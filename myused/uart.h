#ifndef uart_h
#define uart_h


#include "ti_msp_dl_config.h"
#include "stdio.h"

extern volatile unsigned char uart_data;

void uart0_init(void);

void delay_us(unsigned long __us);
void delay_ms(unsigned long ms);
//#define delay_us(x)         delay_cycles(16000000/1000000*x) 
//#define delay_ms(x)         delay_cycles(16000000/1000*x) 

void uart0_send_char(char ch);
void uart0_send_string(char* str);

#endif

