/**
 * @file    usart_openmv.c
 * @brief   MaixCAM2 摄像头串口接收实现
 *
 * 接收状态机 (RX 中断驱动):
 *   STATE 0 (找同步字1): 等待 0xAA
 *   STATE 1 (找同步字2): 收到 0xAA 后等待 0x55
 *   STATE 2 (收数据):   收满 9 字节 → XOR 校验 → 解析 → 置 g_ball_new = 1
 *
 * 错误处理:
 *   - 同步字不匹配 → 重置状态机重新同步
 *   - 校验失败 → 丢弃整帧，重新同步
 *   - 不会卡死在中间状态 (无超时机制，靠下一条正确帧自然恢复)
 */

#include "usart_openmv.h"

/* ================================================================
 * 全局变量
 * ================================================================ */
volatile maix_ball_t g_ball;        /* 最新解析结果 */
volatile uint8_t    g_ball_new = 0; /* 新帧标志 */

/* ================================================================
 * 接收状态机 (静态变量，仅中断中使用)
 * ================================================================ */
static uint8_t  rx_buf[MAIX_FRAME_LEN];    /* 帧缓冲区 (9字节) */
static uint8_t  rx_idx = 0;                 /* 当前填充位置 */
static uint8_t  rx_sync = 0;                /* 同步状态: 0=找0xAA, 1=找0x55, 2=收数据 */

/* ================================================================
 * uart_openmv_init
 * ================================================================ */
void uart_openmv_init(void)
{
    /* UART2 硬件已在 SysConfig 中初始化 (波特率 115200, 8N1)
     * 这里只需清除 pending 中断并使能 NVIC */
    NVIC_ClearPendingIRQ(UART_OPENMV_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_OPENMV_INST_INT_IRQN);
}

/* ================================================================
 * UART_OPENMV_INST_IRQHandler  (RX 中断服务程序)
 *
 * 三态状态机:
 *   0 → 等待 0xAA → 1
 *   1 → 等待 0x55 → 2 (否则回 0)
 *   2 → 收满 9 字节 → XOR 校验 → 解析 → 回 0
 * ================================================================ */
void UART_OPENMV_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_OPENMV_INST)) {

    case DL_UART_IIDX_RX: {
        uint8_t ch = DL_UART_receiveData(UART_OPENMV_INST);

        switch (rx_sync) {

        /* ── 状态 0: 等待帧头第1字节 0xAA ── */
        case 0:
            if (ch == MAIX_SYNC1) {       /* 0xAA */
                rx_buf[0] = ch;
                rx_sync = 1;              /* → 找 0x55 */
            }
            /* 不是 0xAA 则丢弃，继续等待 */
            break;

        /* ── 状态 1: 等待帧头第2字节 0x55 ── */
        case 1:
            if (ch == MAIX_SYNC2) {       /* 0x55 */
                rx_buf[1] = ch;
                rx_idx = 2;               /* 从 byte[2] 开始收数据 */
                rx_sync = 2;              /* → 收数据 */
            } else {
                rx_sync = 0;              /* 不是 0x55，可能是假同步，回状态 0 */
            }
            break;

        /* ── 状态 2: 接收剩余 7 字节 (byte[2] ~ byte[8]) ── */
        case 2:
            rx_buf[rx_idx++] = ch;

            /* 收满 9 字节? */
            if (rx_idx >= MAIX_FRAME_LEN) {

                /* ── XOR 校验 ──
                 * CHK = rx_buf[2] ^ rx_buf[3] ^ ... ^ rx_buf[7]
                 * 与帧尾 rx_buf[8] 比较 */
                uint8_t chk = 0;
                for (uint8_t i = 2; i < MAIX_FRAME_LEN - 1; i++) {
                    chk ^= rx_buf[i];
                }

                if (chk == rx_buf[MAIX_FRAME_LEN - 1]) {
                    /* 校验通过 → 解析帧数据 */
                    g_ball.num  = rx_buf[2];
                    g_ball.cx   = rx_buf[3] | ((uint16_t)rx_buf[4] << 8);   /* 小端序 */
                    g_ball.cy   = rx_buf[5] | ((uint16_t)rx_buf[6] << 8);   /* 小端序 */
                    g_ball.conf = rx_buf[7];
                    g_ball_new  = 1;   /* 通知主循环有新数据 */
                }
                /* 校验失败 → 静默丢弃本帧 */

                rx_sync = 0;   /* 准备接收下一帧 */
            }
            break;
        }
        break;
    }

    default:
        break;
    }
}
