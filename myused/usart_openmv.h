/**
 * @file    usart_openmv.h
 * @brief   MaixCAM2 摄像头串口通信库 (UART2 / UART_OPENMV, 115200bps)
 *
 * 硬件接线:
 *   MaixCAM2 A21 (TX) → MSPM0 PA24 (UART2 RX)
 *   MaixCAM2 A22 (RX) → MSPM0 PA21 (UART2 TX)
 *
 * 协议帧 (9 字节定长, 二进制):
 *   ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
 *   │ 0xAA │ 0x55 │ num  │ cxL  │ cxH  │ cyL  │ cyH  │ conf │ CHK  │
 *   └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
 *    Byte0  Byte1  Byte2  Byte3  Byte4  Byte5  Byte6  Byte7  Byte8
 *
 *   - 0xAA 0x55: 帧头 (同步字)
 *   - num:  检测到的钢珠数量 (0 = 无钢珠)
 *   - cx:   最大钢珠中心 X 坐标 (0~224, 小端序: cxL 低字节, cxH 高字节)
 *   - cy:   最大钢珠中心 Y 坐标 (小端序)
 *   - conf: 置信度 (0~100, 越高越可靠)
 *   - CHK:  校验和 = XOR(byte2, byte3, byte4, byte5, byte6, byte7)
 *
 * 使用说明:
 *   1. 上电后调用 uart_openmv_init() 初始化中断
 *   2. 主循环中检查 g_ball_new 标志:
 *      if (g_ball_new) {
 *          处理 g_ball 数据;
 *          g_ball_new = 0;  // 清除标志
 *      }
 *   3. g_ball 结构体中包含解析好的钢珠数据
 *
 * 相关 SysConfig 配置:
 *   UART2 (UART_OPENMV): 115200, 8N1, 无流控
 *   中断优先级: NVIC priority 1 (比 UART0 高)
 */

#ifndef __USART_OPENMV_H
#define __USART_OPENMV_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* ================================================================
 * 协议常量
 * ================================================================ */
#define MAIX_FRAME_LEN      9       /* 帧总长度 (字节) */
#define MAIX_SYNC1          0xAA    /* 同步字1 */
#define MAIX_SYNC2          0x55    /* 同步字2 */
#define MAIX_IMG_WIDTH      224     /* MaixCAM2 图像宽度 (像素) */
#define MAIX_IMG_HEIGHT     224     /* MaixCAM2 图像高度 (像素) */

/* ================================================================
 * 数据结构
 * ================================================================ */

/**
 * @brief  钢珠检测结果 (一帧解析后的数据)
 * @note   由 RX 中断自动填充，主循环只读
 */
typedef struct {
    uint8_t  num;       /* 当前帧检测到的钢珠数量 (0 = 视野内无钢珠) */
    uint16_t cx;        /* 最大钢珠的质心 X 坐标 (0 ~ 224) */
    uint16_t cy;        /* 最大钢珠的质心 Y 坐标 (0 ~ 224) */
    uint8_t  conf;      /* 置信度百分比 (0 ~ 100)，越高检测越可靠 */
} maix_ball_t;

/* ================================================================
 * 全局变量 (中断中更新，主循环读取)
 * ================================================================ */
extern volatile maix_ball_t g_ball;       /* 最新一帧解析结果 */
extern volatile uint8_t     g_ball_new;   /* 新帧标志: 1=有新数据待处理，主循环读取后清0 */

/* ================================================================
 * API
 * ================================================================ */

/**
 * @brief  初始化 MaixCAM2 串口接收中断
 * @note   在 SYSCFG_DL_init() 之后调用
 *         使能 UART_OPENMV 的 RX 中断，配置 NVIC
 */
void uart_openmv_init(void);

#endif /* __USART_OPENMV_H */
