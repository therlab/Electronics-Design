/**
 * @file    uart.c
 * @brief   UART0 底层驱动 — Zigbee 通信 (9600bps)
 *
 * 功能:
 *   - printf() 重定向到 UART0 (fputc)
 *   - RX 中断接收 (存入 uart_data, 由 bsp_zigbee.c 搬运到环形缓冲区)
 *   - 阻塞式发送 (uart0_send_char / uart0_send_string)
 *   - delay_ms() (委托给 BSP/grayscale/delay.c 的 Tick_delay)
 *
 * 注意:
 *   - RX 中断不 echo (Zigbee 不需要回显，且 echo 会造成回环干扰)
 *   - Zigbee 上层协议请使用 bsp_zigbee.c 的 API
 */

#include "uart.h"

/* UART0 RX 环形缓冲区 */
volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE] = {0};
volatile uint16_t uart_rx_head = 0;
volatile uint16_t uart_rx_tail = 0;

void uart0_init(void)
{
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    uart_rx_head = 0;
    uart_rx_tail = 0;
}

/* delay_us moved to BSP/grayscale/delay.c */
/* delay_ms using Tick_delay from grayscale driver */
extern void Tick_delay(uint32_t ms);

void delay_ms(unsigned long ms)
{
    Tick_delay(ms);
}

/* send single character (blocking) */
void uart0_send_char(char ch)
{
    while (DL_UART_isBusy(UART_0_INST) == true);
    DL_UART_Main_transmitData(UART_0_INST, ch);
}

/* send string */
void uart0_send_string(char* str)
{
    while (*str != 0 && str != 0) {
        uart0_send_char(*str++);
    }
}

/* UART0 RX interrupt: 写入环形缓冲区 */
void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_0_INST)) {
    case DL_UART_IIDX_RX: {
        uint8_t ch = DL_UART_Main_receiveData(UART_0_INST);
        uint16_t next = (uart_rx_head + 1) % UART_RX_BUF_SIZE;
        if (next != uart_rx_tail) {
            uart_rx_buf[uart_rx_head] = ch;
            uart_rx_head = next;
        }
        /* else: 缓冲区满, 丢弃此字节 */
        /* 不 echo: Zigbee 模块不需要回显，echo 会造成回环干扰 */
        break;
    }
    default:
        break;
    }
}

/* printf() redirect to UART0 */
int fputc(int ch, FILE *stream)
{
    while (DL_UART_isBusy(UART_0_INST) == true);
    DL_UART_Main_transmitData(UART_0_INST, ch);
    return ch;
}

#if !defined(__MICROLIB)
#if (__ARMCLIB_VERSION <= 6000000)
struct __FILE { int handle; };
#endif
FILE __stdout;
void _sys_exit(int x) { x = x; }
#endif
