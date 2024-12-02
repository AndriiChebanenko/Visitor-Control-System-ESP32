#pragma once

#include <stdint.h>
#include <driver/gptimer.h>

void initialize_sntp(void);
char* get_current_date(void);
char* get_current_time(void);
char* get_current_datetime(void);
//void timer_delay_us(uint32_t us);
//void timer_delay_ms(uint32_t ms);
gptimer_handle_t gptimer_init(void);
void gptimer_delay_us(gptimer_handle_t timer_handle, uint32_t delay_us);
void gptimer_delay_ms(gptimer_handle_t timer_handle, uint32_t delay_ms);
void gptimer_deinit(gptimer_handle_t timer_handle);