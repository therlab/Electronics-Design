/**
 * @file    bsp_zigbee.h
 * @brief   Zigbee 无线通信模块 (UART0, 9600bps)
 *
 * 接线:
 *   Zigbee TX  → MSPM0 PA11 (UART0 RX)
 *   Zigbee RX  → MSPM0 PA10 (UART0 TX)
 *
 * 使用说明:
 *   1. 上电后调用 zigbee_init() 初始化
 *   2. 用 zigbee_send_string() / zigbee_send_bytes() 发送数据
 *   3. 主循环中调用 zigbee_poll() 检查是否有新数据到达
 *   4. zigbee_available() 返回 >0 表示有未读字节
 *   5. zigbee_read() 读取一个字节
 *
 * 注意:
 *   - 本模块依赖 uart.c 提供的底层 UART0 驱动 (uart_data, uart0_send_string 等)
 *   - UART0 波特率 9600，由 SysConfig 的 ti_msp_dl_config.c 配置
 *   - RX 中断在 uart.c 的 UART_0_INST_IRQHandler 中处理
 */

#ifndef __BSP_ZIGBEE_H
#define __BSP_ZIGBEE_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* ================================================================
 * 环形接收缓冲区大小 (字节)
 * ================================================================ */
#define ZIGBEE_RX_BUF_SIZE  64

/* ================================================================
 * 公开 API
 * ================================================================ */

/**
 * @brief  Zigbee 模块初始化
 * @note   使能 UART0 中断 (NVIC)
 *         必须在 SysConfig 初始化 (SYSCFG_DL_init) 之后调用
 */
void zigbee_init(void);

/**
 * @brief  发送字符串 (以 '\0' 结尾)
 * @param  str  要发送的字符串指针
 * @note   阻塞发送，字符串不宜过长以免阻塞主循环
 */
void zigbee_send_string(const char *str);

/**
 * @brief  发送指定长度的字节数组
 * @param  data  数据指针
 * @param  len   数据长度 (字节)
 * @note   阻塞发送，逐个字节发出
 */
void zigbee_send_bytes(const uint8_t *data, uint16_t len);

/**
 * @brief  格式化发送 (类似 printf，通过 UART0 发出)
 * @param  fmt  格式化字符串
 * @param  ...  可变参数
 * @note   内部使用 vsprintf，缓冲区 128 字节，超出会截断
 *         示例: zigbee_printf("Z%lu\r\n", count);
 */
void zigbee_printf(const char *fmt, ...);

/**
 * @brief  主循环轮询: 将 uart_data 的最新字节存入环形缓冲区
 * @note   每个主循环调用一次即可
 *         检测 uart_data 变化，有新数据时写入 rx_buf
 */
void zigbee_poll(void);

/**
 * @brief  检查接收缓冲区中是否有未读数据
 * @return 可读取的字节数 (0 = 无数据)
 */
uint16_t zigbee_available(void);

/**
 * @brief  从接收缓冲区读取一个字节
 * @return 读取到的字节 (0x00 ~ 0xFF)
 * @note   调用前请先用 zigbee_available() 确认有数据
 *         缓冲区空时返回 0
 */
uint8_t zigbee_read(void);

/**
 * @brief  清空接收缓冲区
 * @note   丢弃所有未读数据，读写指针归零
 */
void zigbee_flush_rx(void);

/**
 * @brief  获取已发送的总字节数 (统计用)
 * @return 发送字节计数
 */
uint32_t zigbee_get_tx_count(void);

/**
 * @brief  获取已接收的总字节数 (统计用)
 * @return 接收字节计数
 */
uint32_t zigbee_get_rx_count(void);

#endif /* __BSP_ZIGBEE_H */
