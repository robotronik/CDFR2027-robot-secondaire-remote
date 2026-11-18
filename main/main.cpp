#include <cstring>
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "asserv_i2c.hpp"
#include "teleop.hpp"

static const char* TAG = "teleop";

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[] asm("_binary_index_html_end");

static QueueHandle_t cmd_q;

static void start_ap() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wc = {};
    strcpy((char*)wc.ap.ssid, "cdfr2023-teleop");
    strcpy((char*)wc.ap.password, "robotronik2023");
    wc.ap.ssid_len = strlen("cdfr2023-teleop");
    wc.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wc.ap.max_connection = 1;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "AP started: cdfr2023-teleop at 192.168.4.1");
}

static esp_err_t root_get(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
}

static esp_err_t ws_handler(httpd_req_t* req) {
    if (req->method == HTTP_GET) return ESP_OK;  // handshake
    httpd_ws_frame_t f = {};
    esp_err_t err = httpd_ws_recv_frame(req, &f, 0);  // length only
    if (err != ESP_OK || f.len != 1) return ESP_FAIL;  // payload stays unread: close the socket, else the stream desyncs
    uint8_t c;
    f.payload = &c;
    if (httpd_ws_recv_frame(req, &f, 1) != ESP_OK) return ESP_FAIL;
    xQueueSend(cmd_q, &c, 0);
    return ESP_OK;
}

static void start_http() {
    httpd_handle_t srv = nullptr;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.lru_purge_enable = true;
    ESP_ERROR_CHECK(httpd_start(&srv, &cfg));
    httpd_uri_t root = {};
    root.uri = "/";
    root.method = HTTP_GET;
    root.handler = root_get;
    ESP_ERROR_CHECK(httpd_register_uri_handler(srv, &root));
    httpd_uri_t ws = {};
    ws.uri = "/ws";
    ws.method = HTTP_GET;
    ws.handler = ws_handler;
    ws.is_websocket = true;
    ESP_ERROR_CHECK(httpd_register_uri_handler(srv, &ws));
    ESP_LOGI(TAG, "httpd started");
}

extern "C" void app_main(void) {
    static AsservI2C robot(GPIO_NUM_8, GPIO_NUM_9);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    if (!robot.interface_version_matches())
        ESP_LOGW(TAG, "asserv version mismatch: reflash the STM32 from cdfr2023-programme-base-roulante-asservissment against the same ../librairie-commune");

    robot.set_coordinates(0, 0, 0);
    int16_t x, y, theta;
    robot.get_coordinates(x, y, theta);
    ESP_LOGI(TAG, "coordinates %d %d %d", x, y, theta);

    cmd_q = xQueueCreate(8, 1);
    start_ap();
    start_http();

    Pose target{0, 0, 0};
    bool armed = false;
    int64_t last_rx = 0;
    auto stop_all = [&] {
        robot.stop();
        robot.get_coordinates(target.x, target.y, target.theta);
        armed = false;
    };
    for (;;) {
        char c;
        if (xQueueReceive(cmd_q, &c, pdMS_TO_TICKS(50))) {
            last_rx = esp_timer_get_time();
            switch (c) {
                case 'F':
                case 'B':
                    target = next_target(target, c);
                    robot.go_to_point(target.x, target.y, Rotation::SHORTEST,
                                      c == 'F' ? Direction::FORWARD : Direction::BACKWARD);
                    armed = true;
                    break;
                case 'L':
                case 'R':
                    target = next_target(target, c);
                    robot.consigne_angulaire(target.theta, Rotation::SHORTEST);
                    armed = true;
                    break;
                case 'S': stop_all(); break;
                default: break;  // 'P' heartbeat
            }
        }
        if (armed && esp_timer_get_time() - last_rx > 300000) {
            ESP_LOGW(TAG, "no message for 300 ms: safety stop");
            stop_all();
        }
    }
}
