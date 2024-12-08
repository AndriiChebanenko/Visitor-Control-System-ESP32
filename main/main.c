#include "driver/gpio.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "lwip/sockets.h"

#include "wifi_f.h"
#include "time_f.h"
#include "tcpserver.h"
#include "ultrasonic.h"
#include "main_log_props.h"

#include <string.h>
#include <sys/socket.h>

#define LED_PIN GPIO_NUM_2
#define TRIG_PIN GPIO_NUM_5
#define ECHO_PIN GPIO_NUM_18
#define BUZZ_PIN GPIO_NUM_19

#define MAX_DISTANCE_CM 500

typedef struct {
	char *ssid;
	char *password;
} wifi_auth_data_t;

typedef enum {
	COUNTER,
	ALARM
} workmode_t;

typedef enum {
	GET_DATA,
	GET_MODE,
	CHANGE_MODE,
	CHECK_ALARM,
	TURN_ALARM_OFF,
	UNDEFINED
} command_t;

#ifdef MAIN_LOG_ON
	static const char *TAG = "VCS";
#endif

static char *WIFI_SSID = "Kartoplyanka";
static char *WIFI_PASSWORD = "12345789";
char visitors_data[2000];
workmode_t workmode = COUNTER;
static uint8_t alarm_triggered = 0;

void setup(void);
void wifi_task(void* arg);
void track_visitors_task(void* arg);
void tcpserver_task(void* arg);
void buzzer_task(void* arg);
void change_workmode(void);
command_t determine_command(char* command);
void blink_led(int blinks_number, int delay_ms);

TaskHandle_t wifi_task_handle;
TaskHandle_t track_visitors_task_handle;
TaskHandle_t tcpserver_task_handle;
TaskHandle_t buzzer_task_handle;

ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIG_PIN,
        .echo_pin = ECHO_PIN
    };

void app_main(void) {
	setup();
	
	wifi_auth_data_t auth_data = {WIFI_SSID, WIFI_PASSWORD};
	xTaskCreate(wifi_task,
				"WiFi management task",
				4096, 
				(void*)&auth_data, 
				3, 
				&wifi_task_handle);
	
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
	
	const int buzzer_freq = 5000;			
	xTaskCreate(buzzer_task, 
				"Buzzer task", 
				2048, 
				(void*)&buzzer_freq, 
				2, 
				&buzzer_task_handle);
	while (1)
	{
		vTaskDelay(1000);
	}
}

void setup(void) {
    ultrasonic_init(&sensor);
    
	gpio_reset_pin(LED_PIN);
	gpio_reset_pin(BUZZ_PIN);
	
  	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  	gpio_set_direction(BUZZ_PIN, GPIO_MODE_OUTPUT);
}

void wifi_task(void* arg) {
	char *ssid = ((wifi_auth_data_t*)arg)->ssid;
	char *password = ((wifi_auth_data_t*)arg)->password;
	
	#ifdef MAIN_LOG_ON
		ESP_LOGI(TAG, "Establishing connection...");
	#endif
	
    ESP_ERROR_CHECK(wifi_f_init());
    esp_err_t ret = wifi_f_connect(ssid, password);
    
	#ifdef MAIN_LOG_ON
    	if (ret != ESP_OK) {
        	ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    	}
	#endif

    wifi_ap_record_t ap_info;
    ret = esp_wifi_sta_get_ap_info(&ap_info);
    
	#ifdef MAIN_LOG_ON
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
	#endif
    
    xTaskNotifyGive(track_visitors_task_handle);
    xTaskNotifyGive(tcpserver_task_handle);
    
    while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void track_visitors_task(void* arg) {
	
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	initialize_sntp();
	
	float dist;
	int *range = (int*)arg;
	float range_start = range[0] / 100.0f;
	float range_end = range[1] / 100.0f;
	
	char *data;
	
	float initial_distance;
	ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &initial_distance);
	blink_led(1, 500);
	
	#ifdef MAIN_LOG_ON
		ESP_LOGI(TAG, "Initial distance: %.2f", initial_distance);
	#endif	
	
	float min_trig_distance = initial_distance * range_start;
	float max_trig_distance = initial_distance * range_end;
	
	#ifdef MAIN_LOG_ON	
		ESP_LOGI(TAG, "Min distance: %.2f, Max distance: %.2f", min_trig_distance, max_trig_distance);
	#endif	
	
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
		ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &dist);
		if (dist > min_trig_distance && dist < max_trig_distance)
		{
			switch (workmode) {
			case COUNTER:
				data = get_current_datetime(2);
				strcat(data, "\n");
				strcat(visitors_data, data);
				blink_led(1, 70);
				#ifdef MAIN_LOG_ON
					printf("Array: \n%s\n", visitors_data);
				#endif				
				break;
			case ALARM:
				alarm_triggered = 1;
				break;
			}						
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}
}

void tcpserver_task(void* arg) {
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	
	char buffer[256];
	int server_socket = tcpserver_init();
	
	const char change_workmode_counter_message[] = "Workmode set: COUNTER";
	const char change_workmode_alarm_message[] = "Workmode set: ALARM";
	const char alarm_triggered_message[] = "Alarm triggered!";
	const char unknown_message[] = "Unknown command";
	const char mode_counter_message[] = "Mode: COUNTER";
	const char mode_alarm_message[] = "Mode: ALARM";
	const char alarm_turned_off_message[] = "Alarm turned off";
	
	while (1) {
        // Приймаємо підключення
        int client_socket = accept(server_socket, NULL, NULL); 
        printf("Client connected.\n");

        while (1) {
            memset(buffer, 0, sizeof(buffer)); // Очищення буфера
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0) {         // Клієнт відключився або стався збій           
				#ifdef MAIN_LOG_ON                
                	printf("Client disconnected.\n");
				#endif                
                break;
            }
            
            printf("Received request: %s\n", buffer);

            command_t command = determine_command(buffer);
            switch (command) {
				case GET_DATA:
                	send(client_socket, visitors_data, strlen(visitors_data), 0); // Надсилаємо дані
                	#ifdef MAIN_LOG_ON
                		ESP_LOGI("GET COMMAND", "Visitors data sent");
                	#endif
                	break;
                case GET_MODE:
                	if (workmode == COUNTER) {
						send(client_socket, mode_counter_message, strlen(mode_counter_message), 0);
					} else {
						send(client_socket, mode_alarm_message, strlen(mode_alarm_message), 0);
					}
					break;
                case CHANGE_MODE:
                	change_workmode();
                	switch (workmode) {
						case COUNTER:
							send(client_socket, change_workmode_counter_message, strlen(change_workmode_counter_message), 0);
							alarm_triggered = 0;
							break;
						case ALARM:
							send(client_socket, change_workmode_alarm_message, strlen(change_workmode_alarm_message), 0);
							break;
					}
					break;
				case CHECK_ALARM:
					if (alarm_triggered) {
						send(client_socket, alarm_triggered_message, strlen(alarm_triggered_message), 0);					
					}
					else
						send(client_socket, "OK", 3, 0);
					break;
				case TURN_ALARM_OFF:
					alarm_triggered = 0;
					send(client_socket, alarm_turned_off_message, strlen(alarm_turned_off_message), 0);
					break;
				default:
					send(client_socket, unknown_message, strlen(unknown_message), 0);
					break;
        	}
    	}
    	// Закриття клієнтського сокета, але сервер продовжує працювати
        close(client_socket);        
        vTaskDelay(pdMS_TO_TICKS(100));
	}
	tcpserver_deinit(server_socket);
}

void buzzer_task(void *arg) {
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(250));
		if (alarm_triggered && workmode == ALARM) {
			gpio_set_level(BUZZ_PIN, 1);
			vTaskDelay(pdMS_TO_TICKS(250));
			gpio_set_level(BUZZ_PIN, 0);
		}
	}
}

void change_workmode(void) {
	if (workmode == COUNTER)
		workmode = ALARM;
	else
 		workmode = COUNTER;
 	#ifdef MAIN_LOG_ON
 		ESP_LOGI(TAG, "Workmode changed on: %s", (workmode == COUNTER)?"COUNTER":"ALARM");
 	#endif
}

command_t determine_command(char* command) {
	if (strcmp(command, "GET DATA") == 0) return GET_DATA;
	else if (strcmp(command, "GET MODE") == 0) return GET_MODE;
	else if (strcmp(command, "CHANGE MODE") == 0) return CHANGE_MODE;
	else if (strcmp(command, "CHECK ALARM") == 0) return CHECK_ALARM;
	else if (strcmp(command, "TURN ALARM OFF") == 0) return TURN_ALARM_OFF;
	else return UNDEFINED;
}

void blink_led(int blinks_number, int delay_ms) {
	if (blinks_number > 0) {
		for (int i = 0; i < blinks_number; i++) {
			gpio_set_level(LED_PIN, 1);
			vTaskDelay(pdMS_TO_TICKS(delay_ms));
			gpio_set_level(LED_PIN, 0);
			vTaskDelay(pdMS_TO_TICKS(delay_ms));
		}
	}
}