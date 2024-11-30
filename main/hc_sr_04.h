#include "driver/gpio.h"
#include "esp_timer.h"
#include "lwip/err.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <inttypes.h>

#define NOP() asm volatile ("nop")

#include "soc/frc_timer_reg.h"
#include <stdint.h>

#ifdef CONFIG_ESP_TIMER_IMPL_FRC2
#define timer_u32() (REG_READ(0x3ff47004));     // FRC2
#elif CONFIG_ESP_TIMER_IMPL_TG0_LAC
#define timer_u32() (REG_READ(0x3ff5F078));     // TG0_LAC
#else
#define timer_u32() (0);                        // SYSTIMER
#endif

#define SOUND_VELOCITY 340

int measure_distance_cm(gpio_num_t trig_pin, gpio_num_t echo_pin);		// Measure distance in cm by HC-SR-04 ultrasonic sensor