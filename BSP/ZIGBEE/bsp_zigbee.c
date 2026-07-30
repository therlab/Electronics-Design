/**
 * @file    bsp_zigbee.c
 * @brief   Zigbee 无线通信模块实现
 *
 * 工作原理:
 *   - 发送: 直接通过 UART0 发出，阻塞方式
 *   - 接收: 主循环调用 zigbee_poll() 将 uart.c 收到的字节搬运到环形缓冲区
 *   - 上层应用从环形缓冲区读取，防止中断上下文中的数据丢失
 *
 * 两层环形缓冲区:
 *   第1层 (uart.c):      ISR → uart_rx_buf[64]  (硬件到内存)
 *   第2层 (bsp_zigbee.c): poll → rx_ring[64]     (内存到应用)
 *   head = 写指针, tail = 读指针, 满时丢弃最旧的数据
 */

#include "bsp_zigbee.h"
#include "uart.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

/* ================================================================
 * 环形接收缓冲区
 * ================================================================ */
static uint8_t  rx_ring[ZIGBEE_RX_BUF_SIZE];  /* 环形缓冲区 */
static uint16_t rx_head = 0;                   /* 写指针 (中断→poll 写入) */
static uint16_t rx_tail = 0;                   /* 读指针 (应用层读取) */

/* ================================================================
 * 统计计数
 * ================================================================ */
static uint32_t tx_total = 0;   /* 累计发送字节数 */
static uint32_t rx_total = 0;   /* 累计接收字节数 */

/* ================================================================
 * zigbee_init
 * ================================================================ */
void zigbee_init(void)
{
    /* UART0 已在 SysConfig 中硬件初始化 (SYSCFG_DL_UART_0_init)
     * 这里只需使能中断，清空缓冲区 */
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);

    /* 清空环形缓冲区 */
    rx_head = 0;
    rx_tail = 0;
    tx_total = 0;
    rx_total = 0;
}

/* ================================================================
 * zigbee_send_string
 * ================================================================ */
void zigbee_send_string(const char *str)
{
    if (str == NULL) return;

    while (*str != '\0') {
        /* 等待 UART0 发送完成 */
        while (DL_UART_isBusy(UART_0_INST) == true);
        DL_UART_Main_transmitData(UART_0_INST, (uint8_t)*str);
        tx_total++;
        str++;
    }
}

/* ================================================================
 * zigbee_send_bytes
 * ================================================================ */
void zigbee_send_bytes(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) return;

    for (uint16_t i = 0; i < len; i++) {
        while (DL_UART_isBusy(UART_0_INST) == true);
        DL_UART_Main_transmitData(UART_0_INST, data[i]);
        tx_total++;
    }
}

/* ================================================================
 * zigbee_printf
 * ================================================================ */
void zigbee_printf(const char *fmt, ...)
{
    char buf[128];  /* 格式化缓冲区，超出截断 */
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    zigbee_send_string(buf);
}

/* ================================================================
 * zigbee_poll  (每个主循环调用一次)
 *
 * 将 UART0 环形缓冲区中的所有字节搬运到 Zigbee 环形缓冲区。
 * ================================================================ */
void zigbee_poll(void)
{
    while (uart_rx_tail != uart_rx_head) {
        uint8_t ch = uart_rx_buf[uart_rx_tail];
        uart_rx_tail = (uart_rx_tail + 1) % UART_RX_BUF_SIZE;

        /* 写入 Zigbee 环形缓冲区 */
        uint16_t next_head = (rx_head + 1) % ZIGBEE_RX_BUF_SIZE;
        if (next_head != rx_tail) {
            /* 缓冲区未满，正常写入 */
            rx_ring[rx_head] = ch;
            rx_head = next_head;
        } else {
            /* 缓冲区已满，丢弃最旧的一个字节 (tail 前移) */
            rx_tail = (rx_tail + 1) % ZIGBEE_RX_BUF_SIZE;
            rx_ring[rx_head] = ch;
            rx_head = next_head;
        }
        rx_total++;
    }
}

/* ================================================================
 * zigbee_available
 * ================================================================ */
uint16_t zigbee_available(void)
{
    if (rx_head >= rx_tail) {
        return rx_head - rx_tail;
    } else {
        /* 环形缓冲区回绕 */
        return ZIGBEE_RX_BUF_SIZE - rx_tail + rx_head;
    }
}

/* ================================================================
 * zigbee_read
 * ================================================================ */
uint8_t zigbee_read(void)
{
    if (zigbee_available() == 0) {
        return 0;  /* 缓冲区空，返回 0 */
    }

    uint8_t ch = rx_ring[rx_tail];
    rx_tail = (rx_tail + 1) % ZIGBEE_RX_BUF_SIZE;
    return ch;
}

/* ================================================================
 * zigbee_flush_rx
 * ================================================================ */
void zigbee_flush_rx(void)
{
    rx_head = 0;
    rx_tail = 0;
}

/* ================================================================
 * zigbee_get_tx_count / zigbee_get_rx_count
 * ================================================================ */
uint32_t zigbee_get_tx_count(void)
{
    return tx_total;
}

uint32_t zigbee_get_rx_count(void)
{
    return rx_total;
}
