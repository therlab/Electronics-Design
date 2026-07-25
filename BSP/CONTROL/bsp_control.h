/**
 * @file    bsp_control.h
 * @brief   小车运动控制库 — 灰度循迹 / 陀螺仪直走 / 转弯 / 色标序列
 *
 * 依赖:
 *   - BSP/PID/pid.h         (compute_weighted_error_digital, grayscale_pid, yaw_pd)
 *   - BSP/MOTOR/bsp_tb6612.h (motor_control)
 *   - BSP/ENCODER/encoder.h  (encoder_poll, encoder_get_distance_mm, encoder_reset_distance)
 *   - BSP/JY61P_IIC/bsp_gyro.h (get_jy61p_data, YawZ)
 *   - config.h               (所有 PID/速度/距离参数)
 *
 * 使用方式:
 *   empty.c 中只做调度，运动计算全部委托给本库:
 *
 *     switch (state) {
 *     case STATE_GRAY:
 *         control_gray_follow(gray, &left, &right);
 *         if (encoder_get_distance_mm() > GRAY_DIST_MM) { ... 切换状态 ... }
 *         break;
 *     case STATE_GYRO:
 *         control_gyro_straight(cur_yaw, gyro_target, &left, &right);
 *         if (色标触发) { control_color_sequence(); state = STATE_GRAY; }
 *         break;
 *     }
 *     // PWM 限幅 + motor_control(left, right) 仍在 empty.c
 */

#ifndef __BSP_CONTROL_H
#define __BSP_CONTROL_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* ================================================================
 * 1. 灰度循迹 — 根据 8 路灰度传感器计算左右轮 PWM
 * ================================================================ */

/**
 * @brief  灰度 PID 巡线差速计算
 * @param  gray   8 路灰度数字量 (bit0=左1 ... bit7=右8)
 * @param  left   输出: 左轮 PWM (正=前进, 负=后退)
 * @param  right  输出: 右轮 PWM
 * @note   内部调用 compute_weighted_error_digital() + grayscale_pid()
 *         基础速度 = BASE_SPEED, PID 参数见 config.h
 */
void control_gray_follow(uint8_t gray, int16_t *left, int16_t *right);

/* ================================================================
 * 2. 陀螺仪直走 — 航向保持
 * ================================================================ */

/**
 * @brief  陀螺仪 PID 直走差速计算
 * @param  cur_yaw     当前航向角 (度)
 * @param  target_yaw  目标航向角 (度)
 * @param  left        输出: 左轮 PWM
 * @param  right       输出: 右轮 PWM
 * @note   内部调用 yaw_pd() 做航向修正
 *         基础速度 = GYRO_SPEED, PID 参数见 config.h
 *         角度差自动归一化到 [-180, 180]
 */
void control_gyro_straight(float cur_yaw, float target_yaw,
                           int16_t *left, int16_t *right);

/* ================================================================
 * 3. 陀螺仪转弯 — 两段式 PID 原地旋转 (阻塞)
 * ================================================================ */

/**
 * @brief  陀螺仪原地转弯 (阻塞执行)
 * @param  angle  转弯角度 (度, 正=右转, 负=左转)
 * @note   两段式 PID: 粗调 (>TURN_SWITCH_DEG) → 精调 → 退出
 *         超时保护: TURN_TIMEOUT_MS
 *         内部调用 encoder_poll / get_jy61p_data / motor_control
 *         结束时自动 motor_control(0,0)
 */
void control_gyro_turn(float angle);

/* ================================================================
 * 4. 色标检测序列 — 停车 → 倒车 → 停车 → 转弯
 * ================================================================ */

/**
 * @brief  色标触发后的完整动作序列 (阻塞执行)
 *
 *   ① 停车 2 秒
 *   ② 倒退 BACK_DIST_MM (50mm) — 编码器测距
 *   ③ 停车 2 秒
 *   ④ 陀螺仪右转 TURN_90 (90°)
 *   ⑤ 距离归零
 *
 * @note  序列结束后编码器距离已归零
 *         调用者负责: pid_reset_all / 更新 gyro_target / 切换状态
 */
void control_color_sequence(void);

#endif /* __BSP_CONTROL_H */
