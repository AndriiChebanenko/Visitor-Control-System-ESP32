#include "driver/gpio.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <unistd.h>
#include "hal/gpio_types.h"
#include "hc_sr_04.h"
#include "soc/gpio_num.h"

#define LED_PIN GPIO_NUM_2
#define TRIG_PIN GPIO_NUM_4
#define ECHO_PIN GPIO_NUM_18

static const char *TAG = "VCS";

void setup(void);

void app_main(void)
{
	setup();
	int distance = 0;
	for (;;)
	{
		distance = measure_distance_cm(TRIG_PIN, ECHO_PIN);
		ESP_LOGI(TAG, "Distance: %d", distance);
		sleep(2);
	}
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