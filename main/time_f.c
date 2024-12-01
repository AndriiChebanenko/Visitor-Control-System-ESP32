#include <time.h>
#include <stdio.h>
#include <esp_log.h>
#include <esp_sntp.h>

static const char *TAG = "Time";

void initialize_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Використання глобального NTP-сервера
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 5; // Кількість спроб синхронізації
    const int retry_delay = 2000; // Затримка між спробами в мс
    while (timeinfo.tm_year < (2023 - 1900) && retry > 0) {
        ESP_LOGI(TAG, "Waiting for time to be set... (%d retries left)", retry);
        vTaskDelay(pdMS_TO_TICKS(retry_delay));
        time(&now);
        localtime_r(&now, &timeinfo);
        retry--;
    }

    if (timeinfo.tm_year < (2023 - 1900)) {
        ESP_LOGE(TAG, "Failed to synchronize time");
    } else {
        ESP_LOGI(TAG, "Time synchronized successfully");
    }
}

char* get_current_date(void) {
    static char date[11];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    snprintf(date, sizeof(date), "%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    return date;
}

char* get_current_time(void) {
    static char time_str[9];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return time_str;
}