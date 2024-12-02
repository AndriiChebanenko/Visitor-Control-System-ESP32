#include "driver/gpio.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdio.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "freertos/projdefs.h"
#include "hc_sr_04.h"
#include "lwip/sockets.h"
#include "wifi_f.h"
#include "time_f.h"
#include "tcpserver.h"

#include <string.h>
#include <sys/socket.h>

#define LED_PIN GPIO_NUM_2
#define TRIG_PIN GPIO_NUM_4
#define ECHO_PIN GPIO_NUM_18

static const char *TAG = "VCS";
static char *WIFI_SSID = "Kartoplyanka";
static char *WIFI_PASSWORD = "12345789";
char visitors_data[2000];

typedef struct {
	char *ssid;
	char *password;
} wifi_auth_data_t;

void setup(void);
void wifi_task(void* arg);
void track_visitors_task(void* arg);
void tcpserver_task(void* arg);

TaskHandle_t wifi_task_handle;
TaskHandle_t track_visitors_task_handle;
TaskHandle_t tcpserver_task_handle;

void app_main(void) {
	setup();
	
	wifi_auth_data_t auth_data = {WIFI_SSID, WIFI_PASSWORD};
	xTaskCreate(wifi_task,
				"WiFi management task",
				4096, 
				(void*)&auth_data, 
				3, 
				&wifi_task_handle);
	vTaskDelay(pdMS_TO_TICKS(5000));
	
	int detecting_range[2] = {15, 85};
	xTaskCreate(track_visitors_task, 
				"Tracking visitors and writing time of detecting to datafile", 
				4096, 
				(void*)&detecting_range, 
				2, 
				&track_visitors_task_handle);
				
	xTaskCreate(tcpserver_task, 
				"TCP Server task", 
				4096, 
				NULL, 
				2, 
				&tcpserver_task_handle);
	while (1)
	{
		vTaskDelay(1000);
	}
}

void setup(void) {
	gpio_reset_pin(LED_PIN);
	gpio_reset_pin(TRIG_PIN);
	gpio_reset_pin(ECHO_PIN);
  	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  	gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
  	gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

void wifi_task(void* arg) {
	char *ssid = ((wifi_auth_data_t*)arg)->ssid;
	char *password = ((wifi_auth_data_t*)arg)->password;
	
	ESP_LOGI(TAG, "Establishing connection...");
    ESP_ERROR_CHECK(wifi_f_init());

    esp_err_t ret = wifi_f_connect(ssid, password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    }

    wifi_ap_record_t ap_info;
    ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_ERR_WIFI_CONN) {
        ESP_LOGE(TAG, "Wi-Fi station interface not initialized");
    }
    else if (ret == ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGE(TAG, "Wi-Fi station is not connected");
    } else {
        ESP_LOGI(TAG, "--- Access Point Information ---");
        ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
        ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
        ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
        ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);
    }
    while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void track_visitors_task(void* arg) {
	
	initialize_sntp();
	
	int dist;
	int *range = (int*)arg;
	float range_start = range[0] / 100.0f;
	float range_end = range[1] / 100.0f;
	
	char *data;
	
	int initial_distance = measure_distance_cm(TRIG_PIN, ECHO_PIN);
	ESP_LOGI(TAG, "Initial distance: %d", initial_distance);
	int min_trig_distance = (int)(initial_distance * range_start);
	int max_trig_distance = (int)(initial_distance * range_end);
	ESP_LOGI(TAG, "Min distance: %d, Max distance: %d", min_trig_distance, max_trig_distance);
	
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
		dist = measure_distance_cm(TRIG_PIN, ECHO_PIN);
		if (dist > min_trig_distance && dist < max_trig_distance)
		{
			data = get_current_datetime();
			strcat(data, "\n");
			strcat(visitors_data, data);
			printf("Array: \n%s\n", visitors_data);
			//ESP_LOGI(TAG, "Array: %s", visitors_data);
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}
}

void tcpserver_task(void* arg) {
	ESP_LOGI(TAG, "tcpserver task started");
	char buffer[256];
	int server_socket = tcpserver_init();
	
	while (1) {
        // Приймаємо підключення
        int client_socket = accept(server_socket, NULL, NULL);
        printf("Client connected.\n");

        // Обробка запитів клієнта
        while (1) {
            memset(buffer, 0, sizeof(buffer)); // Очищення буфера
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0) {
                // Клієнт відключився або стався збій
                printf("Client disconnected.\n");
                break;
            }

            buffer[strcspn(buffer, "\n")] = 0; // Видалення символа нового рядка
            printf("Received request: %s\n", buffer);

            // Обробка запиту
            if (strcmp(buffer, "GET FILE") == 0) {
                send(client_socket, visitors_data, strlen(visitors_data), 0); // Надсилаємо дані
                ESP_LOGI("GET COMMAND", "Visitors data sent");
            } else if (strcmp(buffer, "SET MODE") == 0) {
                ESP_LOGI(TAG, "SET MODE command received"); // Встановлюємо режим
            } else {
                char server_message[] = "Unknown command!";
                send(client_socket, server_message, strlen(server_message), 0);
            }
        }
        // Закриття клієнтського сокета, але сервер продовжує працювати
        close(client_socket);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    tcpserver_deinit(server_socket);
}