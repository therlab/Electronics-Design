/**
 * @file    bsp_tb6612.c
 * @brief   TB6612 4路电机驱动实现 (4轮小车)
 *
 * 4路电机控制逻辑:
 *   speed > 0: IN1=1, IN2=0 → 正转 (前进)
 *   speed < 0: IN1=0, IN2=1 → 反转 (后退)
 *   speed = 0: IN1=0, IN2=0 → 刹车
 *
 * PWM 通道分配:
 *   前左 F_L → TIMA0 CH2 (PA15), IN1=PA7,  IN2=PA12
 *   前右 F_R → TIMA0 CH0 (PB14), IN1=PA13, IN2=PA16
 *   后左 B_L → TIMA0 CH1 (PB9),  IN1=PB16, IN2=PA14
 *   后右 B_R → TIMA0 CH3 (PA23), IN1=PB13, IN2=PB15
 *
 * 左侧 = F_L + B_L (同速), 右侧 = F_R + B_R (同速)
 *
 * !!! 调试注意 !!!
 *   如果某个轮子转向反了, 把对应 FWD/REV 宏里的 IN1/IN2 互换即可.
 *   例如: F_L 前进时轮子实际往后转, 就把 F_L_IN1(1);F_L_IN2(0)
 *   改成                          F_L_IN1(0);F_L_IN2(1)
 */

#include "bsp_tb6612.h"

#define PWM_MAX   1000   /* TIMA0 period=1000 */

/* ================================================================
 * 方向宏 — 全部默认: IN1=1,IN2=0=前进
 * 如果某轮子方向反了, 在下面把对应 FWD/REV 的 IN1/IN2 对调!
 * ================================================================ */
/* 前左 F_L — IN1/IN2 对调 (物理接线反相) */
#define F_L_DIR(fwd)  do { \
    if ((fwd) > 0)      { F_L_IN1(0); F_L_IN2(1); } \
    else if ((fwd) < 0) { F_L_IN1(1); F_L_IN2(0); } \
    else                { F_L_IN1(0); F_L_IN2(0); } \
} while(0)

/* 前右 F_R — 默认方向 (IN1=1,IN2=0=前进) */
#define F_R_DIR(fwd)  do { \
    if ((fwd) > 0)      { F_R_IN1(1); F_R_IN2(0); } \
    else if ((fwd) < 0) { F_R_IN1(0); F_R_IN2(1); } \
    else                { F_R_IN1(0); F_R_IN2(0); } \
} while(0)

/* 后左 B_L — 默认方向 (物理接线匹配: IN1=1,IN2=0=前进) */
#define B_L_DIR(fwd)  do { \
    if ((fwd) > 0)      { B_L_IN1(1); B_L_IN2(0); } \
    else if ((fwd) < 0) { B_L_IN1(0); B_L_IN2(1); } \
    else                { B_L_IN1(0); B_L_IN2(0); } \
} while(0)

/* 后右 B_R — 默认方向 (IN1=1,IN2=0=前进) */
#define B_R_DIR(fwd)  do { \
    if ((fwd) > 0)      { B_R_IN1(1); B_R_IN2(0); } \
    else if ((fwd) < 0) { B_R_IN1(0); B_R_IN2(1); } \
    else                { B_R_IN1(0); B_R_IN2(0); } \
} while(0)

/**
 * @brief  4轮小车差速控制
 * @param  left   左侧速度 (前左+后左同速): 正=前进, 负=后退
 * @param  right  右侧速度 (前右+后右同速): 正=前进, 负=后退
 */
void motor_control(int16_t left, int16_t right)
{
    int16_t pwm;

    /* ====== 前左 F_L: PWM=CH2(PA15) ====== */
    F_L_DIR(left);
    pwm = (left < 0) ? -left : left;
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, (uint32_t)pwm, DL_TIMER_CC_2_INDEX);

    /* ====== 后左 B_L: PWM=CH1(PB9) ====== */
    B_L_DIR(left);
    pwm = (left < 0) ? -left : left;
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, (uint32_t)pwm, DL_TIMER_CC_1_INDEX);

    /* ====== 前右 F_R: PWM=CH0(PB14) ====== */
    F_R_DIR(right);
    pwm = (right < 0) ? -right : right;
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, (uint32_t)pwm, DL_TIMER_CC_0_INDEX);

    /* ====== 后右 B_R: PWM=CH3(PA23) ====== */
    B_R_DIR(right);
    pwm = (right < 0) ? -right : right;
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, (uint32_t)pwm, DL_TIMER_CC_3_INDEX);
}
