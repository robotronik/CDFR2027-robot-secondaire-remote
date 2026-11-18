#pragma once
#include <cstring>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "asservissement_interface.h"

class AsservI2C : public asservissement_interface {
public:
    AsservI2C(gpio_num_t sda, gpio_num_t scl) {
        i2c_master_bus_config_t bus_cfg = {};
        bus_cfg.i2c_port = I2C_NUM_0;
        bus_cfg.sda_io_num = sda;
        bus_cfg.scl_io_num = scl;
        bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_cfg.glitch_ignore_cnt = 7;
        bus_cfg.flags.enable_internal_pullup = true;
        esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_);
        if (err != ESP_OK) { ESP_LOGE(TAG, "bus init: %s", esp_err_to_name(err)); return; }

        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = 42;
        dev_cfg.scl_speed_hz = 100000;
        err = i2c_master_bus_add_device(bus_, &dev_cfg, &dev_);
        if (err != ESP_OK) { ESP_LOGE(TAG, "add device: %s", esp_err_to_name(err)); dev_ = nullptr; }
    }

private:
    static constexpr const char* TAG = "asserv_i2c";
    static constexpr int RETRIES = 3;
    static constexpr int TIMEOUT_MS = 50;
    i2c_master_bus_handle_t bus_ = nullptr;
    i2c_master_dev_handle_t dev_ = nullptr;

    void I2cSendData(uint8_t command, uint8_t* data, int length) override {
        if (!dev_) return;
        uint8_t buf[64];
        if (length < 0 || length + 1 > (int)sizeof(buf)) { ESP_LOGE(TAG, "send %u: bad length %d", command, length); return; }
        buf[0] = command;
        if (length) memcpy(buf + 1, data, length);
        esp_err_t err = ESP_FAIL;
        for (int i = 0; i < RETRIES && err != ESP_OK; i++)
            err = i2c_master_transmit(dev_, buf, length + 1, TIMEOUT_MS);
        if (err != ESP_OK) ESP_LOGE(TAG, "send cmd %u failed: %s", command, esp_err_to_name(err));
    }

    void I2cReceiveData(uint8_t command, uint8_t* data, int length) override {
        esp_err_t err = ESP_FAIL;
        if (dev_) {
            for (int i = 0; i < RETRIES && err != ESP_OK; i++) {
                err = i2c_master_transmit(dev_, &command, 1, TIMEOUT_MS);
                if (err != ESP_OK) continue;
                esp_rom_delay_us(200);
                err = i2c_master_receive(dev_, data, length, TIMEOUT_MS);
            }
        }
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "recv cmd %u failed: %s", command, esp_err_to_name(err));
            memset(data, 0, length);
        }
    }
};
