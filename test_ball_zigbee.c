/**
 * @file    test_ball_zigbee.c
 * @brief   通过 Zigbee 接收 MaixCAM2 钢珠数据 → OLED 显示
 *
 * ================================================================
 * 接线 (和之前一样, 不需要改):
 * ================================================================
 *   MSPM0 UART0 PA10(TX) → DL-20 Zigbee RX
 *   MSPM0 UART0 PA11(RX) ← DL-20 Zigbee TX
 *   OLED I2C0 PA28(SDA) PA31(SCL)
 *
 * ================================================================
 * MaixCAM2 端接线:
 * ================================================================
 *   MaixCAM2 A21(TX) → DL-20 Zigbee RX
 *   MaixCAM2 GND      → DL-20 GND
 *   MaixCAM2 3.3V     → DL-20 VCC
 *   (运行 send_ball_zigbee.py)
 *
 * ================================================================
 * 协议帧 (9 字节, 9600bps 通过 DL-20 传输):
 * ================================================================
 *   [0]=0xAA [1]=0x55 [2]=num [3]=cxL [4]=cxH
 *   [5]=cyL [6]=cyH [7]=conf [8]=XOR_CHK
 *
 * ================================================================
 * Keil 编译方法:
 * ================================================================
 *   把此文件加入工程 (替换 empty.c), 原 empty.c 暂时移除编译即可
 */

#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "uart.h"
#include "tim.h"
#include "oled_hardware_i2c.h"
#include "delay.h"
#include "bsp_zigbee.h"

/* ================================================================
 * 帧协议常量
 * ================================================================ */
#define FRAME_LEN   9
#define SYNC1       0xAA
#define SYNC2       0x55

/* ================================================================
 * 外部变量
 * ================================================================ */
extern volatile unsigned long tick_ms;

/* ================================================================
 * 模块变量
 * ================================================================ */
static uint8_t  g_oled_text[32];       /* OLED 显示缓冲 */
static uint32_t last_hb_tick = 0;      /* 心跳计时 */
static uint32_t frame_cnt     = 0;     /* 成功收到的帧数 */
static uint32_t bad_cnt       = 0;     /* 校验失败的帧数 */

/* 最新解析结果 */
static uint8_t  ball_num  = 0;
static uint16_t ball_cx   = 0;
static uint16_t ball_cy   = 0;
static uint8_t  ball_conf = 0;
static uint8_t  ball_new  = 0;         /* 新帧标志 */

/* ================================================================
 * 帧解析状态机 (从 zigbee_read() 逐字节喂入)
 * ================================================================ */
static uint8_t  rx_buf[FRAME_LEN];
static uint8_t  rx_idx = 0;
static uint8_t  rx_sync = 0;           /* 0=找0xAA, 1=找0x55, 2=收数据 */

/**
 * @brief  喂一个字节给帧解析器
 * @return 1=解析到完整有效帧 (数据在 ball_xxx 中), 0=未完成/无效
 */
static uint8_t feed_parser(uint8_t ch)
{
    switch (rx_sync) {

    case 0:   /* 等 0xAA */
        if (ch == SYNC1) {
            rx_buf[0] = ch;
            rx_sync = 1;
        }
        return 0;

    case 1:   /* 等 0x55 */
        if (ch == SYNC2) {
            rx_buf[1] = ch;
            rx_idx = 2;
            rx_sync = 2;
        } else {
            rx_sync = 0;   /* 假同步, 回退 */
        }
        return 0;

    case 2:   /* 收剩余 7 字节 */
        rx_buf[rx_idx++] = ch;
        if (rx_idx >= FRAME_LEN) {
            rx_sync = 0;

            /* XOR 校验: byte[2] ^ ... ^ byte[7] == byte[8] */
            uint8_t chk = 0;
            for (uint8_t i = 2; i < FRAME_LEN - 1; i++) {
                chk ^= rx_buf[i];
            }

            if (chk == rx_buf[FRAME_LEN - 1]) {
                ball_num  = rx_buf[2];
                ball_cx   = rx_buf[3] | ((uint16_t)rx_buf[4] << 8);
                ball_cy   = rx_buf[5] | ((uint16_t)rx_buf[6] << 8);
                ball_conf = rx_buf[7];
                ball_new  = 1;
                frame_cnt++;
                return 1;
            } else {
                bad_cnt++;
                return 0;
            }
        }
        return 0;
    }
    return 0;
}

/* ================================================================
 * main
 * ================================================================ */
int main(void)
{
    /* ── 初始化 ── */
    SYSCFG_DL_init();
    uart0_init();
    OLED_Init();
    OLED_Clear();
    delay_ms(100);

    zigbee_init();

    /* ── 启动画面 ── */
    OLED_ShowString(0, 0, (uint8_t *)"Ball ZigBee Test", 8);
    OLED_ShowString(0, 1, (uint8_t *)"Waiting...", 8);
    delay_ms(1500);
    OLED_Clear();

    /* ── 开机心跳 ── */
    zigbee_printf("=== Ball ZigBee Test Start ===\r\n");

    /* ════════════════════════════════════════════════════════════════
     * 主循环
     * ════════════════════════════════════════════════════════════════ */
    while (1) {

        /* ============================================================
         * 1. Zigbee 轮询 + 帧解析
         * ============================================================ */
        zigbee_poll();

        while (zigbee_available() > 0) {
            uint8_t ch = zigbee_read();
            feed_parser(ch);
        }

        /* ============================================================
         * 2. 心跳: 每 1 秒发状态
         * ============================================================ */
        if (tick_ms - last_hb_tick >= 1000) {
            last_hb_tick = tick_ms;
            zigbee_printf("Ball: N=%u CX=%u CY=%u CONF=%u F=%lu B=%lu\r\n",
                ball_num, ball_cx, ball_cy, ball_conf, frame_cnt, bad_cnt);
        }

        /* ============================================================
         * 3. OLED 显示 (每 200ms 刷新)
         * ============================================================ */
        {
            static uint32_t disp_tick = 0;
            if (tick_ms - disp_tick >= 200) {
                disp_tick = tick_ms;

                /* 行1: 标题 + 帧计数 */
                sprintf((char *)g_oled_text, "Ball F:%lu    ",
                    frame_cnt);
                OLED_ShowString(0, 0, g_oled_text, 8);

                /* 行2: 钢珠数量和置信度 */
                if (ball_num > 0) {
                    sprintf((char *)g_oled_text, "Num:%u Conf:%u%%   ",
                        ball_num, ball_conf);
                } else {
                    sprintf((char *)g_oled_text, "No ball          ");
                }
                OLED_ShowString(0, 1, g_oled_text, 8);

                /* 行3: 中心坐标 */
                sprintf((char *)g_oled_text, "CX:%u CY:%u       ",
                    ball_cx, ball_cy);
                OLED_ShowString(0, 2, g_oled_text, 8);

                /* 行4: 校验失败 / 运行秒 */
                sprintf((char *)g_oled_text, "Bad:%lu T:%lus  ",
                    bad_cnt, (unsigned int)(tick_ms / 1000));
                OLED_ShowString(0, 3, g_oled_text, 8);

                ball_new = 0;
            }
        }

        delay_ms(10);
    }
}
