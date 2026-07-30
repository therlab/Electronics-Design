/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_MOTOR */
#define PWM_MOTOR_INST                                                     TIMA0
#define PWM_MOTOR_INST_IRQHandler                               TIMA0_IRQHandler
#define PWM_MOTOR_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define PWM_MOTOR_INST_CLK_FREQ                                          4000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_MOTOR_C0_PORT                                             GPIOB
#define GPIO_PWM_MOTOR_C0_PIN                                     DL_GPIO_PIN_14
#define GPIO_PWM_MOTOR_C0_IOMUX                                  (IOMUX_PINCM31)
#define GPIO_PWM_MOTOR_C0_IOMUX_FUNC                 IOMUX_PINCM31_PF_TIMA0_CCP0
#define GPIO_PWM_MOTOR_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_MOTOR_C1_PORT                                             GPIOB
#define GPIO_PWM_MOTOR_C1_PIN                                      DL_GPIO_PIN_9
#define GPIO_PWM_MOTOR_C1_IOMUX                                  (IOMUX_PINCM26)
#define GPIO_PWM_MOTOR_C1_IOMUX_FUNC                 IOMUX_PINCM26_PF_TIMA0_CCP1
#define GPIO_PWM_MOTOR_C1_IDX                                DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_PWM_MOTOR_C2_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C2_PIN                                     DL_GPIO_PIN_15
#define GPIO_PWM_MOTOR_C2_IOMUX                                  (IOMUX_PINCM37)
#define GPIO_PWM_MOTOR_C2_IOMUX_FUNC                 IOMUX_PINCM37_PF_TIMA0_CCP2
#define GPIO_PWM_MOTOR_C2_IDX                                DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_PWM_MOTOR_C3_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C3_PIN                                     DL_GPIO_PIN_23
#define GPIO_PWM_MOTOR_C3_IOMUX                                  (IOMUX_PINCM53)
#define GPIO_PWM_MOTOR_C3_IOMUX_FUNC                 IOMUX_PINCM53_PF_TIMA0_CCP3
#define GPIO_PWM_MOTOR_C3_IDX                                DL_TIMER_CC_3_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMA1)
#define TIMER_0_INST_IRQHandler                                 TIMA1_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMA1_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                          (3999U)




/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C0
#define I2C_OLED_INST_IRQHandler                                 I2C0_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C0_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOA
#define GPIO_I2C_OLED_SDA_PIN                                     DL_GPIO_PIN_28
#define GPIO_I2C_OLED_IOMUX_SDA                                   (IOMUX_PINCM3)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                    IOMUX_PINCM3_PF_I2C0_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOA
#define GPIO_I2C_OLED_SCL_PIN                                     DL_GPIO_PIN_31
#define GPIO_I2C_OLED_IOMUX_SCL                                   (IOMUX_PINCM6)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                    IOMUX_PINCM6_PF_I2C0_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                            4000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                  (9600)
#define UART_0_IBRD_4_MHZ_9600_BAUD                                         (26)
#define UART_0_FBRD_4_MHZ_9600_BAUD                                          (3)
/* Defines for UART_OPENMV */
#define UART_OPENMV_INST                                                   UART2
#define UART_OPENMV_INST_FREQUENCY                                      32000000
#define UART_OPENMV_INST_IRQHandler                             UART2_IRQHandler
#define UART_OPENMV_INST_INT_IRQN                                 UART2_INT_IRQn
#define GPIO_UART_OPENMV_RX_PORT                                           GPIOA
#define GPIO_UART_OPENMV_TX_PORT                                           GPIOA
#define GPIO_UART_OPENMV_RX_PIN                                   DL_GPIO_PIN_24
#define GPIO_UART_OPENMV_TX_PIN                                   DL_GPIO_PIN_21
#define GPIO_UART_OPENMV_IOMUX_RX                                (IOMUX_PINCM54)
#define GPIO_UART_OPENMV_IOMUX_TX                                (IOMUX_PINCM46)
#define GPIO_UART_OPENMV_IOMUX_RX_FUNC                 IOMUX_PINCM54_PF_UART2_RX
#define GPIO_UART_OPENMV_IOMUX_TX_FUNC                 IOMUX_PINCM46_PF_UART2_TX
#define UART_OPENMV_BAUD_RATE                                           (115200)
#define UART_OPENMV_IBRD_32_MHZ_115200_BAUD                                 (17)
#define UART_OPENMV_FBRD_32_MHZ_115200_BAUD                                 (23)





/* Port definition for Pin Group GPIO_I2C */
#define GPIO_I2C_PORT                                                    (GPIOA)

/* Defines for SCL: GPIOA.0 with pinCMx 1 on package pin 33 */
#define GPIO_I2C_SCL_PIN                                         (DL_GPIO_PIN_0)
#define GPIO_I2C_SCL_IOMUX                                        (IOMUX_PINCM1)
/* Defines for SDA: GPIOA.1 with pinCMx 2 on package pin 34 */
#define GPIO_I2C_SDA_PIN                                         (DL_GPIO_PIN_1)
#define GPIO_I2C_SDA_IOMUX                                        (IOMUX_PINCM2)
/* Defines for F_L_IN1: GPIOA.7 with pinCMx 14 on package pin 49 */
#define GPIO_MOTOR_F_L_IN1_PORT                                          (GPIOA)
#define GPIO_MOTOR_F_L_IN1_PIN                                   (DL_GPIO_PIN_7)
#define GPIO_MOTOR_F_L_IN1_IOMUX                                 (IOMUX_PINCM14)
/* Defines for F_L_IN2: GPIOA.12 with pinCMx 34 on package pin 5 */
#define GPIO_MOTOR_F_L_IN2_PORT                                          (GPIOA)
#define GPIO_MOTOR_F_L_IN2_PIN                                  (DL_GPIO_PIN_12)
#define GPIO_MOTOR_F_L_IN2_IOMUX                                 (IOMUX_PINCM34)
/* Defines for F_R_IN1: GPIOA.13 with pinCMx 35 on package pin 6 */
#define GPIO_MOTOR_F_R_IN1_PORT                                          (GPIOA)
#define GPIO_MOTOR_F_R_IN1_PIN                                  (DL_GPIO_PIN_13)
#define GPIO_MOTOR_F_R_IN1_IOMUX                                 (IOMUX_PINCM35)
/* Defines for F_R_IN2: GPIOA.16 with pinCMx 38 on package pin 9 */
#define GPIO_MOTOR_F_R_IN2_PORT                                          (GPIOA)
#define GPIO_MOTOR_F_R_IN2_PIN                                  (DL_GPIO_PIN_16)
#define GPIO_MOTOR_F_R_IN2_IOMUX                                 (IOMUX_PINCM38)
/* Defines for B_L_IN1: GPIOB.16 with pinCMx 33 on package pin 4 */
#define GPIO_MOTOR_B_L_IN1_PORT                                          (GPIOB)
#define GPIO_MOTOR_B_L_IN1_PIN                                  (DL_GPIO_PIN_16)
#define GPIO_MOTOR_B_L_IN1_IOMUX                                 (IOMUX_PINCM33)
/* Defines for B_L_IN2: GPIOA.14 with pinCMx 36 on package pin 7 */
#define GPIO_MOTOR_B_L_IN2_PORT                                          (GPIOA)
#define GPIO_MOTOR_B_L_IN2_PIN                                  (DL_GPIO_PIN_14)
#define GPIO_MOTOR_B_L_IN2_IOMUX                                 (IOMUX_PINCM36)
/* Defines for B_R_IN1: GPIOB.13 with pinCMx 30 on package pin 1 */
#define GPIO_MOTOR_B_R_IN1_PORT                                          (GPIOB)
#define GPIO_MOTOR_B_R_IN1_PIN                                  (DL_GPIO_PIN_13)
#define GPIO_MOTOR_B_R_IN1_IOMUX                                 (IOMUX_PINCM30)
/* Defines for B_R_IN2: GPIOB.15 with pinCMx 32 on package pin 3 */
#define GPIO_MOTOR_B_R_IN2_PORT                                          (GPIOB)
#define GPIO_MOTOR_B_R_IN2_PIN                                  (DL_GPIO_PIN_15)
#define GPIO_MOTOR_B_R_IN2_IOMUX                                 (IOMUX_PINCM32)
/* Port definition for Pin Group GPIO_KEY */
#define GPIO_KEY_PORT                                                    (GPIOB)

/* Defines for KEY1: GPIOB.17 with pinCMx 43 on package pin 14 */
#define GPIO_KEY_KEY1_PIN                                       (DL_GPIO_PIN_17)
#define GPIO_KEY_KEY1_IOMUX                                      (IOMUX_PINCM43)
/* Defines for KEY2: GPIOB.18 with pinCMx 44 on package pin 15 */
#define GPIO_KEY_KEY2_PIN                                       (DL_GPIO_PIN_18)
#define GPIO_KEY_KEY2_IOMUX                                      (IOMUX_PINCM44)
/* Defines for KEY3: GPIOB.19 with pinCMx 45 on package pin 16 */
#define GPIO_KEY_KEY3_PIN                                       (DL_GPIO_PIN_19)
#define GPIO_KEY_KEY3_IOMUX                                      (IOMUX_PINCM45)
/* Defines for KEY4: GPIOB.20 with pinCMx 48 on package pin 17 */
#define GPIO_KEY_KEY4_PIN                                       (DL_GPIO_PIN_20)
#define GPIO_KEY_KEY4_IOMUX                                      (IOMUX_PINCM48)
/* Defines for KEY5: GPIOB.21 with pinCMx 49 on package pin 18 */
#define GPIO_KEY_KEY5_PIN                                       (DL_GPIO_PIN_21)
#define GPIO_KEY_KEY5_IOMUX                                      (IOMUX_PINCM49)
/* Defines for KEY6: GPIOB.22 with pinCMx 50 on package pin 19 */
#define GPIO_KEY_KEY6_PIN                                       (DL_GPIO_PIN_22)
#define GPIO_KEY_KEY6_IOMUX                                      (IOMUX_PINCM50)
/* Port definition for Pin Group Serial_Gray */
#define Serial_Gray_PORT                                                 (GPIOB)

/* Defines for DAT: GPIOB.8 with pinCMx 25 on package pin 60 */
#define Serial_Gray_DAT_PIN                                      (DL_GPIO_PIN_8)
#define Serial_Gray_DAT_IOMUX                                    (IOMUX_PINCM25)
/* Defines for CLK: GPIOB.7 with pinCMx 24 on package pin 59 */
#define Serial_Gray_CLK_PIN                                      (DL_GPIO_PIN_7)
#define Serial_Gray_CLK_IOMUX                                    (IOMUX_PINCM24)
/* Defines for L_A: GPIOB.24 with pinCMx 52 on package pin 23 */
#define GPIO_Encoder_L_A_PORT                                            (GPIOB)
#define GPIO_Encoder_L_A_PIN                                    (DL_GPIO_PIN_24)
#define GPIO_Encoder_L_A_IOMUX                                   (IOMUX_PINCM52)
/* Defines for L_B: GPIOA.25 with pinCMx 55 on package pin 26 */
#define GPIO_Encoder_L_B_PORT                                            (GPIOA)
#define GPIO_Encoder_L_B_PIN                                    (DL_GPIO_PIN_25)
#define GPIO_Encoder_L_B_IOMUX                                   (IOMUX_PINCM55)
/* Defines for R_A: GPIOB.25 with pinCMx 56 on package pin 27 */
#define GPIO_Encoder_R_A_PORT                                            (GPIOB)
#define GPIO_Encoder_R_A_PIN                                    (DL_GPIO_PIN_25)
#define GPIO_Encoder_R_A_IOMUX                                   (IOMUX_PINCM56)
/* Defines for R_B: GPIOA.26 with pinCMx 59 on package pin 30 */
#define GPIO_Encoder_R_B_PORT                                            (GPIOA)
#define GPIO_Encoder_R_B_PIN                                    (DL_GPIO_PIN_26)
#define GPIO_Encoder_R_B_IOMUX                                   (IOMUX_PINCM59)
/* Port definition for Pin Group GPIO_Color */
#define GPIO_Color_PORT                                                  (GPIOA)

/* Defines for LEFT: GPIOA.8 with pinCMx 19 on package pin 54 */
#define GPIO_Color_LEFT_PIN                                      (DL_GPIO_PIN_8)
#define GPIO_Color_LEFT_IOMUX                                    (IOMUX_PINCM19)
/* Defines for RIGHT: GPIOA.9 with pinCMx 20 on package pin 55 */
#define GPIO_Color_RIGHT_PIN                                     (DL_GPIO_PIN_9)
#define GPIO_Color_RIGHT_IOMUX                                   (IOMUX_PINCM20)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_MOTOR_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_I2C_OLED_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_OPENMV_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
