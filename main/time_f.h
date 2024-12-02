#pragma once

#include <stdint.h>

void initialize_sntp(void);
char* get_current_date(void);
char* get_current_time(void);
char* get_current_datetime(void);
void timer_delay_us(uint32_t us);
void timer_delay_ms(uint32_t ms);