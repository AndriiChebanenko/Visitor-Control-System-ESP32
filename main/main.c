#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <unistd.h>
#include "hal/gpio_types.h"
#include "hc_sr_04.h"
#include "soc/gpio_num.h"
#include "wifi_f.h"
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <sys/socket.h>

#define LED_PIN GPIO_NUM_2
#define TRIG_PIN GPIO_NUM_4
#define ECHO_PIN GPIO_NUM_18

static const char *TAG = "VCS";
static char *WIFI_SSID = "Tenda_5151A0";
static char *WIFI_PASSWORD = "63521153";

void setup(void);
void connect_wifi(char *ssid, char *password);
void create_tcpclient(int port, char *ip);
void timer_delay_ms(uint32_t ms);

TaskHandle_t task_1_handle;
TaskHandle_t task_2_handle;

void app_main(void)
{
	setup();
	connect_wifi(WIFI_SSID, WIFI_PASSWORD);
	int distance = 0;
	for (int i = 0; i < 4; i++)
	{
		distance = measure_distance_cm(TRIG_PIN, ECHO_PIN);
		ESP_LOGI(TAG, "Distance: %d", distance);
		sleep(2);
	}
	create_tcpclient(8083, "10.10.10.251");
}

void setup(void)
{
	gpio_reset_pin(LED_PIN);
	gpio_reset_pin(TRIG_PIN);
	gpio_reset_pin(ECHO_PIN);
  	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  	gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
  	gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

void connect_wifi(char *ssid, char *password)
{
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
}

void create_tcpclient(int port, char *ip)
{
	// create a socket
  int network_socket;
  network_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  // specify an address for the socket
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(ip);

  int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  // check for error with the connection
  if (connection_status == -1){
    printf("There was an error making a connection to the remote socket \n\n");
  }
  
  // receive data from the server
  char server_response[256];
  recv(network_socket, &server_response, sizeof(server_response), 0);

  // print out the server's response
  printf("The server sent the data: %s\n", server_response);

  // and then close the socket
  close(network_socket);
}

void timer_delay_ms(uint32_t ms)
{
	timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_NONE,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_DIS,
        .divider = 160000
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ms);
    timer_start(TIMER_GROUP_0, TIMER_0);

    while (1) {
        uint64_t counter_val;
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &counter_val);
        if (counter_val >= ms) {
            break;
        }
    }
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

void task_1(void *arg)
{
	int i = 0;
	while (true)
	{
		printf("task_1 i: %d", i++);
		timer_delay_ms(2000);
	}
}

void task_2(void *arg)
{
	int d = (int)(*arg);
	while (true)
	{
		printf("task_2 d: %d", d);
		timer_delay_ms(2000);
	}
}