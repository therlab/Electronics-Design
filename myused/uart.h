#ifndef uart_h
#define uart_h


#include "ti_msp_dl_config.h"
#include "stdio.h"

/* UART0 RX 环形缓冲区 (ISR 写入, zigbee_poll 读出) */
#define UART_RX_BUF_SIZE  64
extern volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
extern volatile uint16_t uart_rx_head;   /* ISR 写指针 */
extern volatile uint16_t uart_rx_tail;   /* zigbee_poll 读指针 */

void uart0_init(void);

void delay_us(unsigned long __us);
void delay_ms(unsigned long ms);
//#define delay_us(x)         delay_cycles(16000000/1000000*x)
//#define delay_ms(x)         delay_cycles(16000000/1000*x)

void uart0_send_char(char ch);
void uart0_send_string(char* str);

#endif

