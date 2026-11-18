#include "esp_log.h"
#include "nvs_flash.h"
#include "asserv_i2c.hpp"

static const char* TAG = "teleop";

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
}
