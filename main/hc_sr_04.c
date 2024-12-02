/*
 * hc_sr_04.c
 *
 *  Created on: 29 лист. 2024 р.
 *      Author: andri
 */
 
#include "hc_sr_04.h"
#include "time_f.h"

//static const char *TAG = "hc-sr-04";

/*unsigned long IRAM_ATTR micros() {
    return (unsigned long) (esp_timer_get_time());
}

void IRAM_ATTR delayMicroseconds(uint32_t us) {
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}*/

int measure_distance_cm(gpio_num_t trig_pin, gpio_num_t echo_pin) {
	gpio_set_level(trig_pin, 0);
	int64_t t0, t1, dt;
	timer_delay_us(2);
	gpio_set_level(trig_pin, 1);
	timer_delay_us(10);
	gpio_set_level(trig_pin, 0);
	while (!gpio_get_level(echo_pin)) {}
	t0 = esp_timer_get_time();
	while (gpio_get_level(echo_pin)) {}
	t1 = esp_timer_get_time();
	dt = t1 - t0;
	int result = (int)((dt * (SOUND_VELOCITY / 10000.0f)) / 2.0f);
	return result;
}