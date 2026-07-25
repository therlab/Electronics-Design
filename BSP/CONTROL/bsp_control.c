/**
 * @file    bsp_control.c
 * @brief   小车运动控制库实现
 *
 * 四个核心函数:
 *   control_gray_follow()      — 灰度巡线 (非阻塞, 仅计算 PWM)
 *   control_gyro_straight()    — 陀螺仪直走 (非阻塞, 仅计算 PWM)
 *   control_gyro_turn()        — 陀螺仪转弯 (阻塞)
 *   control_color_sequence()   — 色标触发序列 (阻塞)
 */

#include "bsp_control.h"
#include "pid.h"
#include "bsp_tb6612.h"
#include "encoder.h"
#include "bsp_gyro.h"
#include "uart.h"
#include "delay.h"
#include "config.h"
#include <math.h>

/* ================================================================
 * 外部变量
 * ================================================================ */
extern volatile unsigned long tick_ms;   /* SysTick 毫秒 (tim.c) */

/* ================================================================
 * control_gray_follow  —  灰度 PID 巡线
 * ================================================================ */
void control_gray_follow(uint8_t gray, int16_t *left, int16_t *right)
{
    /*
     * 灰度传感器 8 路数字量 → 加权误差
     * bit0(G1/最左)=1 表示检测到白线 → 车偏右 → error 为负 → 向左修正
     * 权重在 config.h 定义: GRAY_W1 ~ GRAY_W8
     */
    float error      = compute_weighted_error_digital(gray);
    float correction = grayscale_pid(error);

    /*
     * 差速转向:
     *   correction > 0 (车偏右) → 左轮减速/右轮加速 → 向左修正
     *   correction < 0 (车偏左) → 左轮加速/右轮减速 → 向右修正
     */
    *left  = (int16_t)(BASE_SPEED - correction);
    *right = (int16_t)(BASE_SPEED + correction);
}

/* ================================================================
 * control_gyro_straight  —  陀螺仪航向保持直走
 * ================================================================ */
void control_gyro_straight(float cur_yaw, float target_yaw,
                           int16_t *left, int16_t *right)
{
    /* 角度差归一化到 [-180, 180] */
    float err = target_yaw - cur_yaw;
    while (err >  180) err -= 360;
    while (err < -180) err += 360;

    /*
     * yaw_pd() 内部含 I 项 (YAW_KI=0.05)
     * 注意: correction 符号已反过一次 (陀螺仪 Z 轴旋转方向与车体相反)
     */
    float correction = yaw_pd(err);

    *left  = (int16_t)(GYRO_SPEED - correction);
    *right = (int16_t)(GYRO_SPEED + correction);
}

/* ================================================================
 * control_gyro_turn  —  陀螺仪两段式 PID 原地转弯 (阻塞)
 *
 * PID 切换:
 *   误差 > TURN_SWITCH_DEG (20°) → 粗调 KP=3.8 KD=1.5 SPD=400 (快转)
 *   误差 < 20°                    → 精调 KP=1.0 KD=0.6 SPD=190 (对准)
 *
 * 退出: 误差 < TURN_MIN_ERROR (1.5°) 且连续 TURN_EXIT_CNT (3) 次
 * 超时: TURN_TIMEOUT_MS (2500ms)
 * ================================================================ */
void control_gyro_turn(float angle)
{
    float   yaw_int  = 0;       /* I 项累计 */
    float   yaw_last = 0;       /* 上一次误差 (D 项) */
    uint8_t stable   = 0;       /* 连续稳定计数 */
    uint32_t t0      = tick_ms;

    /* 读当前航向 → 计算目标角度 */
    get_jy61p_data();
    float cur_yaw   = YawZ;
    float target    = cur_yaw - angle;   /* 右转为正 → Yaw 减小 */

    /* 归一化 */
    while (target < -180) target += 360;
    while (target >  180) target -= 360;

    while (1) {
        encoder_poll();
        get_jy61p_data();
        cur_yaw = YawZ;

        /* ── 误差计算 (归一化) ── */
        float err = target - cur_yaw;
        while (err >  180) err -= 360;
        while (err < -180) err += 360;

        /* ── 两段式 PID 参数 ── */
        float kp, ki, kd, min_spd;
        if (fabsf(err) > TURN_SWITCH_DEG) {
            kp = TURN_KP_COARSE;  ki = TURN_KI_COARSE;
            kd = TURN_KD_COARSE;  min_spd = TURN_SPD_COARSE;
        } else {
            kp = TURN_KP_FINE;    ki = TURN_KI_FINE;
            kd = TURN_KD_FINE;    min_spd = TURN_SPD_FINE;
        }

        /* ── PID ── */
        yaw_int += err;
        if (yaw_int >  TURN_INT_MAX) yaw_int =  TURN_INT_MAX;
        if (yaw_int < -TURN_INT_MAX) yaw_int = -TURN_INT_MAX;

        float d   = err - yaw_last;
        yaw_last  = err;
        float out = kp * err + ki * yaw_int + kd * d;

        /* 保证最低马力 */
        if (fabsf(err) > TURN_MIN_ERROR && fabsf(out) < min_spd) {
            out = (out >= 0) ? min_spd : -min_spd;
        }

        /* 差速: 左轮反转 + 右轮正转 → 顺时针 (车体右转) */
        int16_t l = -(int16_t)out + TURN_FWD_BIAS - TURN_LR_COMP;
        int16_t r =  (int16_t)out + TURN_FWD_BIAS + TURN_LR_COMP;

        /* PWM 限幅 */
        if (l >  1000) l =  1000; if (l < -1000) l = -1000;
        if (r >  1000) r =  1000; if (r < -1000) r = -1000;

        motor_control(l, r);

        /* ── 退出判断 ── */
        if (fabsf(err) < TURN_MIN_ERROR) {
            if (++stable >= TURN_EXIT_CNT) break;
        } else {
            stable = 0;
        }

        /* 超时保护 */
        if (tick_ms - t0 > TURN_TIMEOUT_MS) break;

        delay_ms(LOOP_DELAY_MS);
    }

    /* 停车 */
    motor_control(0, 0);
    delay_ms(100);
}

/* ================================================================
 * control_color_sequence  —  色标触发后的完整序列 (阻塞)
 *
 * ① 停车 2 秒
 * ② 倒退 BACK_DIST_MM (编码器测距)
 * ③ 停车 2 秒
 * ④ 陀螺仪右转 TURN_90 (90°)
 * ⑤ 编码器距离归零
 * ================================================================ */
void control_color_sequence(void)
{
    /* ① 停车 2 秒 — 让车完全静止 */
    motor_control(0, 0);
    {
        uint32_t t0 = tick_ms;
        while (tick_ms - t0 < 2000) {
            delay_ms(LOOP_DELAY_MS);
        }
    }

    /* ② 倒退 BACK_DIST_MM — 编码器测距控制 */
    {
        encoder_reset_distance();
        while (encoder_get_distance_mm() < (float)BACK_DIST_MM) {
            encoder_poll();
            motor_control(-GYRO_SPEED, -GYRO_SPEED);
            delay_ms(LOOP_DELAY_MS);
        }
    }

    /* ③ 停车 2 秒 */
    motor_control(0, 0);
    {
        uint32_t t0 = tick_ms;
        while (tick_ms - t0 < 2000) {
            delay_ms(LOOP_DELAY_MS);
        }
    }

    /* ④ 陀螺仪右转 90° */
    control_gyro_turn(TURN_90);

    /* ⑤ 距离归零 (准备下一段巡线) */
    encoder_reset_distance();
}
