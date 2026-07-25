/**
 * @file    empty.c
 * @brief   Amy 64pin 4轮小车 — 顶层调度
 *
 * ================================================================
 * 运行流程:
 * ================================================================
 *   上电 → 4秒倒计时 →
 *
 *   STATE_GRAY (灰度巡线) → 走够 GRAY_DIST_MM → STATE_GYRO
 *   STATE_GYRO (陀螺仪直走) → 色标触发 → 倒车转弯 → STATE_GRAY
 *   ↑________________________________________________________↓
 *
 * ================================================================
 * 运动控制: BSP/CONTROL/bsp_control.c  (四个核心函数)
 * ================================================================
 *   control_gray_follow()     — 灰度 PID → 左右 PWM
 *   control_gyro_straight()   — 陀螺仪航向保持 → 左右 PWM
 *   control_gyro_turn()       — 陀螺仪两段式转弯 (阻塞)
 *   control_color_sequence()  — 色标序列: 停车→倒车→停车→转弯 (阻塞)
 *
 * ================================================================
 * 并发:
 * ================================================================
 *   Zigbee (UART0 9600): 每1秒心跳, 主循环轮询接收
 *   MaixCAM2 (UART2 115200): 中断接收钢珠帧 → g_ball_new
 *   OLED: 4行 — 状态/Zigbee/钢珠/灰度+色标
 *
 * ================================================================
 * 硬件速查:
 * ================================================================
 *   电机: F_L(PA15) F_R(PB14) B_L(PB9) B_R(PA23)
 *   灰度: CLK=PB7 DAT=PB8     色标: L=PA8 R=PA9
 *   陀螺仪: I2C PA28/PA31      编码器: PB24 PA25 PB25 PA26
 *   Zigbee: UART0 PA10(TX) PA11(RX) 9600
 *   MaixCAM2: UART2 PA21(TX) PA24(RX) 115200
 */

#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "uart.h"
#include "tim.h"
#include "oled_hardware_i2c.h"
#include "bsp_tb6612.h"
#include "pid.h"
#include "encoder.h"
#include "delay.h"
#include "gray_serial.h"
#include "bsp_gyro.h"
#include "bsp_zigbee.h"
#include "bsp_control.h"
#include "usart_openmv.h"
#include "config.h"

/* ================================================================
 * 外部变量
 * ================================================================ */
extern volatile unsigned long tick_ms;

/* ================================================================
 * 模块变量
 * ================================================================ */
uint8_t g_oled_text[32];                     /* OLED 显示缓冲 */

static uint32_t disp_cnt        = 0;         /* OLED 刷新分频 */
static uint8_t  gyro_cnt        = 0;         /* 陀螺仪读取分频 */
static float    cur_yaw         = 0;         /* 当前航向 */
static float    gyro_target     = 0;         /* 直走目标航向 */
static uint32_t last_zb_tick    = 0;         /* Zigbee 上次发送时间 */

/* ================================================================
 * 状态机
 * ================================================================ */
enum { STATE_GRAY, STATE_GYRO } state;

/* ================================================================
 * main
 * ================================================================ */
int main(void)
{
    /* ─── 初始化 ─── */
    SYSCFG_DL_init();
    uart0_init();
    TIMER_0_init();
    OLED_Init();   OLED_Clear();
    delay_ms(100);

    encoder_init();
    uart_openmv_init();
    zigbee_init();

    /* ─── 4 秒倒计时 ─── */
    {
        uint32_t t0 = tick_ms;
        while (tick_ms - t0 < 4000) {
            sprintf((char *)g_oled_text, "Start in %u s",
                (unsigned int)(4 - (tick_ms - t0) / 1000));
            OLED_ShowString(0, 0, g_oled_text, 8);
            encoder_poll();
            zigbee_poll();
            delay_ms(100);
        }
    }
    OLED_Clear();

    /* ─── 初始状态 ─── */
    state = STATE_GRAY;
    encoder_reset_distance();
    get_jy61p_data();
    gyro_target = YawZ;

    /* ════════════════════════════════════════════════════════════
     * 主循环
     * ════════════════════════════════════════════════════════════ */
    while (1) {

        /* ========================================================
         * 1. Zigbee
         * ======================================================== */
        zigbee_poll();

        /* 接收命令 (协议待定) */
        while (zigbee_available() > 0) {
            uint8_t ch = zigbee_read();
            /* TODO: 解析命令, 例: 'S'=停车 'R'=复位 */
            (void)ch;
        }

        /* 心跳: 每 1 秒发状态 */
        if (tick_ms - last_zb_tick >= 1000) {
            last_zb_tick = tick_ms;
            zigbee_printf("Z%u S%d D%.1f Y%.1f\r\n",
                (unsigned int)zigbee_get_tx_count(),
                (int)state,
                encoder_get_distance_mm(),
                cur_yaw);
        }

        /* ========================================================
         * 2. 传感器更新
         * ======================================================== */
        encoder_poll();

        if (++gyro_cnt >= 10) {
            gyro_cnt = 0;
            get_jy61p_data();
            cur_yaw = YawZ;
        }

        uint8_t gray = gray_serial_read();

        if (g_ball_new) {
            g_ball_new = 0;
            /* 钢珠数据已就绪 (g_ball), 后续在此处理 */
        }

        /* ========================================================
         * 3. 状态机 — 运动控制 (委托给 bsp_control)
         * ======================================================== */
        int16_t left = 0, right = 0;

        switch (state) {

        case STATE_GRAY:
            /* 灰度 PID 巡线 */
            control_gray_follow(gray, &left, &right);

            /* 走够距离 → 切陀螺仪直走 */
            if (encoder_get_distance_mm() > (float)GRAY_DIST_MM) {
                state = STATE_GYRO;
                pid_reset_all();
                get_jy61p_data();
                gyro_target = YawZ;
            }
            break;

        case STATE_GYRO:
            /* 陀螺仪航向保持直走 */
            control_gyro_straight(cur_yaw, gyro_target, &left, &right);

            /* 色标检测 → 倒车转弯序列 → 回灰度巡线 */
            if (DL_GPIO_readPins(GPIO_Color_PORT, GPIO_Color_RIGHT_PIN)) {
                control_color_sequence();    /* 阻塞: 停车→倒车→停车→右转 */
                pid_reset_all();
                get_jy61p_data();
                gyro_target = YawZ;
                state = STATE_GRAY;
            }
            break;
        }

        /* ─── PWM 限幅 ─── */
        if (left  >  1000) left  =  1000;
        if (left  < -1000) left  = -1000;
        if (right >  1000) right =  1000;
        if (right < -1000) right = -1000;

        motor_control(left, right);

        /* ========================================================
         * 4. OLED 显示 (分频)
         * ======================================================== */
        if (++disp_cnt >= OLED_DIV) {
            disp_cnt = 0;

            /* 行1: 状态 + Zigbee 计数 */
            sprintf((char *)g_oled_text, "%s Z:%u/%u        ",
                (state == STATE_GRAY) ? "GRAY" : "GYRO",
                (unsigned int)zigbee_get_tx_count(),
                (unsigned int)zigbee_get_rx_count());
            OLED_ShowString(0, 0, g_oled_text, 8);

            /* 行2: 钢珠信息 */
            if (g_ball.num > 0) {
                sprintf((char *)g_oled_text, "B:%d %d,%d c%d   ",
                    g_ball.num, g_ball.cx, g_ball.cy, g_ball.conf);
            } else {
                sprintf((char *)g_oled_text, "B:---              ");
            }
            OLED_ShowString(0, 1, g_oled_text, 8);

            /* 行3: 距离 + 航向 */
            sprintf((char *)g_oled_text, "D:%.0f Y:%.1f       ",
                encoder_get_distance_mm(), cur_yaw);
            OLED_ShowString(0, 2, g_oled_text, 8);

            /* 行4: 灰度 8 位 + 色标 */
            {
                uint8_t cr = DL_GPIO_readPins(GPIO_Color_PORT,
                                              GPIO_Color_RIGHT_PIN) ? 1 : 0;
                sprintf((char *)g_oled_text, "%d%d%d%d%d%d%d%d C:%d     ",
                    (gray>>0)&1, (gray>>1)&1, (gray>>2)&1, (gray>>3)&1,
                    (gray>>4)&1, (gray>>5)&1, (gray>>6)&1, (gray>>7)&1, cr);
                OLED_ShowString(0, 3, g_oled_text, 8);
            }
        }

        delay_ms(LOOP_DELAY_MS);
    }
}
