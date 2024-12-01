#pragma once

#include "esp_err.h"

esp_err_t wifi_f_init(void);
esp_err_t wifi_f_connect(char* wifi_ssid, char* wifi_password);
esp_err_t wifi_f_disconnect(void);
esp_err_t wifi_f_deinit(void);