#include "driver/gptimer.h"
#include "driver/gptimer_types.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <esp_log.h>
#include <esp_sntp.h>

#include "time_f.h"

static const char *TAG = "Time";

void initialize_sntp(void) {
	ESP_LOGI(TAG, "Initializing SNTP...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2023 - 1900)) {
        ESP_LOGE(TAG, "Failed to synchronize time");
    } else {
        ESP_LOGI(TAG, "Time synchronized successfully");
    }
}

char* get_current_date(void) {
    static char date_str[36];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    return date_str;
}

char* get_current_time(int16_t timezone) {
    static char time_str[11];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", (timeinfo.tm_hour + timezone) % 24, timeinfo.tm_min, timeinfo.tm_sec);
    return time_str;
}

char* get_current_datetime(int16_t timezone) {
	static char datetime_str[48];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    sprintf(datetime_str, "%02d:%02d:%02d %02d/%02d/%04d",
    						(timeinfo.tm_hour + timezone) % 24, timeinfo.tm_min, timeinfo.tm_sec,
    						timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    return datetime_str;
}

gptimer_handle_t gptimer_init(void) {
    gptimer_handle_t timer_handle = NULL;

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 500000
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer_handle));
    ESP_ERROR_CHECK(gptimer_enable(timer_handle));
    ESP_ERROR_CHECK(gptimer_start(timer_handle));

    return timer_handle;
}

void gptimer_delay_us(gptimer_handle_t timer_handle, uint32_t delay_us) {
    uint64_t start_count = 0;
    gptimer_get_raw_count(timer_handle, &start_count);

    uint64_t current_count = 0;

    do {
        gptimer_get_raw_count(timer_handle, &current_count);
    } while ((current_count - start_count) < delay_us);
}

void gptimer_delay_ms(gptimer_handle_t timer_handle, uint32_t delay_ms) {
	gptimer_delay_us(timer_handle, delay_ms * 1000);
}

void gptimer_deinit(gptimer_handle_t timer_handle) {
	ESP_ERROR_CHECK(gptimer_stop(timer_handle));
	ESP_ERROR_CHECK(gptimer_del_timer(timer_handle));
}