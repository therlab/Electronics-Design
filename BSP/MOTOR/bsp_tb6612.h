/**
 * @file    bsp_tb6612.h
 * @brief   TB6612 4路电机驱动 (4轮小车, 左/右侧各两个电机)
 *
 * 4路电机引脚分配：
 *   前左 F_L: PWM=PA15 (TIMA0_CH2), IN1=PA7,  IN2=PA12
 *   前右 F_R: PWM=PB14 (TIMA0_CH0), IN1=PA13, IN2=PA16
 *   后左 B_L: PWM=PB9  (TIMA0_CH1), IN1=PB16, IN2=PA14
 *   后右 B_R: PWM=PA23 (TIMA0_CH3), IN1=PB13, IN2=PB15
 *
 * 控制逻辑: 左侧(前左+后左)同速, 右侧(前右+后右)同速
 */

#ifndef _BSP_TB6612_H
#define _BSP_TB6612_H

#include "ti_msp_dl_config.h"
#include "stdio.h"

/*================ 前左电机(F_L)控制宏 ================*/
/* IN1=PA7: 1=正转, 0=停止/反转 */
#define F_L_IN1(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_F_L_IN1_PORT,    \
                               GPIO_MOTOR_F_L_IN1_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_F_L_IN1_PORT,    \
                               GPIO_MOTOR_F_L_IN1_PIN)))
/* IN2=PA12: 1=反转, 0=停止/正转 */
#define F_L_IN2(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_F_L_IN2_PORT,    \
                               GPIO_MOTOR_F_L_IN2_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_F_L_IN2_PORT,    \
                               GPIO_MOTOR_F_L_IN2_PIN)))

/*================ 前右电机(F_R)控制宏 ================*/
/* IN1=PA13 */
#define F_R_IN1(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_F_R_IN1_PORT,    \
                               GPIO_MOTOR_F_R_IN1_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_F_R_IN1_PORT,    \
                               GPIO_MOTOR_F_R_IN1_PIN)))
/* IN2=PA16 */
#define F_R_IN2(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_F_R_IN2_PORT,    \
                               GPIO_MOTOR_F_R_IN2_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_F_R_IN2_PORT,    \
                               GPIO_MOTOR_F_R_IN2_PIN)))

/*================ 后左电机(B_L)控制宏 ================*/
/* IN1=PB16 */
#define B_L_IN1(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_B_L_IN1_PORT,    \
                               GPIO_MOTOR_B_L_IN1_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_B_L_IN1_PORT,    \
                               GPIO_MOTOR_B_L_IN1_PIN)))
/* IN2=PA14 */
#define B_L_IN2(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_B_L_IN2_PORT,    \
                               GPIO_MOTOR_B_L_IN2_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_B_L_IN2_PORT,    \
                               GPIO_MOTOR_B_L_IN2_PIN)))

/*================ 后右电机(B_R)控制宏 ================*/
/* IN1=PB13 */
#define B_R_IN1(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_B_R_IN1_PORT,    \
                               GPIO_MOTOR_B_R_IN1_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_B_R_IN1_PORT,    \
                               GPIO_MOTOR_B_R_IN1_PIN)))
/* IN2=PB15 */
#define B_R_IN2(X)   ((X) ? (DL_GPIO_setPins(GPIO_MOTOR_B_R_IN2_PORT,    \
                               GPIO_MOTOR_B_R_IN2_PIN))                     \
                           : (DL_GPIO_clearPins(GPIO_MOTOR_B_R_IN2_PORT,    \
                               GPIO_MOTOR_B_R_IN2_PIN)))

/*================ 电机控制函数 ================*/

/**
 * @brief  4轮小车电机控制 (差速转向)
 * @param  left   左侧速度 (F_L + B_L): 正=前进, 负=后退, 范围 -1000 ~ +1000
 * @param  right  右侧速度 (F_R + B_R): 正=前进, 负=后退, 范围 -1000 ~ +1000
 * @note   左侧两电机同速, 右侧两电机同速 (滑移转向)
 *         PWM通道: F_L=CH2, F_R=CH0, B_L=CH1, B_R=CH3
 */
void motor_control(int16_t left, int16_t right);

#endif /* _BSP_TB6612_H */
