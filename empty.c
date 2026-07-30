/**
 * @file    empty.c
 * @brief   灰度循迹 + 色标停车 + 6键调参 (加权平均法)
 *          参数在 config.h 里改, 改完 Rebuild 生效
 *          上电 STOP 状态, KEY4 启停, KEY5/6 辅助调参
 */

#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "uart.h"
#include "tim.h"
#include "oled_hardware_i2c.h"
#include "bsp_tb6612.h"
#include "pid.h"
#include "delay.h"
#include "gray_serial.h"
#include "config.h"
#include "w25q64.h"

extern volatile unsigned long tick_ms;

/* ══════ 按键 (PB17/18/19 + PB20/21/22, 内部上拉, 按下=0) ══════ */
static void keys_init(void) {
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY2_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY3_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY4_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY5_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY6_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

static uint8_t key_read(GPIO_Regs *port, uint32_t pin) {
    return (DL_GPIO_readPins(port, pin) == 0) ? 1 : 0;
}

static uint8_t key_debounced(GPIO_Regs *port, uint32_t pin) {
    uint8_t a = key_read(port, pin);
    delay_ms(10);
    uint8_t b = key_read(port, pin);
    return (a == b) ? a : 0;
}

/* ══════ 参数名列表 ══════ */
static const char *pname[] = {"KP","KD","KI","SPD"};
static uint8_t pidx = 0;

/* ══════ W25Q64 参数存储 ══════ */
#define W25_MAGIC  0xBEEFCAFE
#define W25_ADDR   0x000000

typedef struct {
    float kp, kd, ki;
    int   spd;
    uint32_t magic;
} w25_params_t;

static float g_kp = GRAY_KP;
static float g_kd = GRAY_KD;
static float g_ki = GRAY_KI;
static int   g_spd = BASE_SPEED;

static void w25_params_load(void) {
    w25_params_t p;
    w25q64_read(W25_ADDR, (uint8_t *)&p, sizeof(p));
    if (p.magic == W25_MAGIC) {
        g_kp = p.kp; g_kd = p.kd;
        g_ki = p.ki; g_spd = p.spd;
    }
}

static void w25_params_save(void) {
    w25_params_t p = {g_kp, g_kd, g_ki, g_spd, W25_MAGIC};
    w25q64_erase_sector(W25_ADDR);
    w25q64_write(W25_ADDR, (uint8_t *)&p, sizeof(p));
}

/* ══════ 运行时 PID (使用W25Q64加载的参数) ══════ */
static float g_pid_last = 0;
static float g_pid_int  = 0;

static float run_pid(float error)
{
    g_pid_int += error;
    if (g_pid_int > GRAY_INT_MAX)  g_pid_int =  GRAY_INT_MAX;
    if (g_pid_int < -GRAY_INT_MAX) g_pid_int = -GRAY_INT_MAX;
    float deriv = error - g_pid_last;
    g_pid_last = error;
    return g_kp * error + g_ki * g_pid_int + g_kd * deriv;
}

int main(void)
{
    SYSCFG_DL_init();

    w25q64_init();
    w25_params_load();
    keys_init();

    OLED_Init();
    OLED_Clear();

    /* 显示参数来源 */
    {
        w25_params_t p;
        w25q64_read(W25_ADDR, (uint8_t *)&p, sizeof(p));
        if (p.magic == W25_MAGIC) {
            OLED_ShowString(0, 2, (uint8_t*)"Flash loaded", 8);
        } else {
            OLED_ShowString(0, 2, (uint8_t*)"Defaults", 8);
        }
    }
    delay_ms(500);
    OLED_Clear();

    char     t[22];
    uint32_t count = 0;
    uint8_t  stopped = 1;          /* 上电即 STOP */
    int16_t  left = 0, right = 0;
    float    err = 0;
    float    correction = 0;

    /* 显示初始 STOP 画面 */
    OLED_ShowString(0, 0, (uint8_t*)"STOP [4]->GO", 16);
    snprintf(t, sizeof(t), "%cKP%.1f %cKD%.1f",
        (pidx == 0) ? '>' : ' ', (double)g_kp,
        (pidx == 1) ? '>' : ' ', (double)g_kd);
    OLED_ShowString(0, 2, (uint8_t*)t, 16);
    snprintf(t, sizeof(t), "%cKI%.2f %cV%d",
        (pidx == 2) ? '>' : ' ', (double)g_ki,
        (pidx == 3) ? '>' : ' ', g_spd);
    OLED_ShowString(0, 4, (uint8_t*)t, 16);
    OLED_ShowString(0, 6, (uint8_t*)"G:........", 16);

    while (1)
    {
        /* ══════ 按键处理 (每2次循环≈10ms) ══════ */
        static uint8_t key_tick = 0;
        if (++key_tick >= 2) {
            key_tick = 0;

            /* 消抖: 连续2次读到相同值才确认 */
            static uint8_t k1_db = 0, k2_db = 0, k3_db = 0;
            static uint8_t k4_db = 0, k5_db = 0, k6_db = 0;
            uint8_t r1 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY1_PIN) == 0);
            uint8_t r2 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY2_PIN) == 0);
            uint8_t r3 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY3_PIN) == 0);
            uint8_t r4 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY4_PIN) == 0);
            uint8_t r5 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY5_PIN) == 0);
            uint8_t r6 = (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_KEY6_PIN) == 0);
            uint8_t k1 = (r1 && k1_db) ? 1 : 0; k1_db = r1;
            uint8_t k2 = (r2 && k2_db) ? 1 : 0; k2_db = r2;
            uint8_t k3 = (r3 && k3_db) ? 1 : 0; k3_db = r3;
            uint8_t k4 = (r4 && k4_db) ? 1 : 0; k4_db = r4;
            uint8_t k5 = (r5 && k5_db) ? 1 : 0; k5_db = r5;
            uint8_t k6 = (r6 && k6_db) ? 1 : 0; k6_db = r6;

            /* KEY4 启停 toggle */
            static uint8_t k4_was = 0;
            if (k4 && !k4_was) {
                stopped = !stopped;
                if (stopped) {
                    motor_control(0, 0);
                    g_pid_int = 0; g_pid_last = 0;
                }
                OLED_Clear();
            }
            k4_was = k4;

            /* 参数调节 (仅在 STOP 状态) */
            if (stopped) {
                static uint8_t k1_was = 0;
                if (k1 && !k1_was) { pidx = (pidx + 1) % 4; }
                k1_was = k1;

                static uint8_t k1_hold = 0;
                if (k1) {
                    if (++k1_hold > 80) {  /* ~1s */
                        k1_hold = 0;
                        g_kp = GRAY_KP; g_kd = GRAY_KD;
                        g_ki = GRAY_KI; g_spd = BASE_SPEED;
                        g_pid_int = 0; g_pid_last = 0;
                    }
                } else { k1_hold = 0; }

                static uint8_t k2_hold = 0;
                if (k2) {
                    if (++k2_hold > 160) {  /* ~2s */
                        k2_hold = 0;
                        w25_params_save();
                        OLED_ShowString(0, 0, (uint8_t*)"SAVED!", 16);
                        delay_ms(800);
                    }
                } else { k2_hold = 0; }

                static uint8_t k2_was = 0, k5_was = 0;
                if ((k2 && !k2_was) || (k5 && !k5_was)) {
                    switch (pidx) {
                    case 0: g_kp += 0.5f; if (g_kp > 20) g_kp = 20; break;
                    case 1: g_kd += 0.5f; if (g_kd > 20) g_kd = 20; break;
                    case 2: g_ki += 0.05f; if (g_ki > 5) g_ki = 5; break;
                    case 3: g_spd += 25; if (g_spd > 800) g_spd = 800; break;
                    }
                    g_pid_int = 0; g_pid_last = 0;
                }
                k2_was = k2; k5_was = k5;

                static uint8_t k3_was = 0, k6_was = 0;
                if ((k3 && !k3_was) || (k6 && !k6_was)) {
                    switch (pidx) {
                    case 0: g_kp -= 0.5f; if (g_kp < 0.5f) g_kp = 0.5f; break;
                    case 1: g_kd -= 0.5f; if (g_kd < 0) g_kd = 0; break;
                    case 2: g_ki -= 0.05f; if (g_ki < 0) g_ki = 0; break;
                    case 3: g_spd -= 25; if (g_spd < 50) g_spd = 50; break;
                    }
                    g_pid_int = 0; g_pid_last = 0;
                }
                k3_was = k3; k6_was = k6;
            }
        }

        uint8_t gray = gray_serial_read();

        /* ══════ 循迹 (RUN 模式) ══════ */
        if (!stopped)
        {
            err = compute_weighted_error_digital(gray);
            correction = run_pid(err);

            left  = (int16_t)(g_spd - correction);
            right = (int16_t)(g_spd + correction);

            if (left  > 1000) left  = 1000;
            if (left  < -1000) left = -1000;
            if (right > 1000) right = 1000;
            if (right < -1000) right = -1000;

            motor_control(left, right);
        }

        count++;

        /* ══════ OLED 刷新 ══════ */
        if (count % OLED_DIV == 0)
        {
            /* 灰度传感器位图 */
            char gray_bits[9];
            for (int i = 0; i < 8; i++)
                gray_bits[7 - i] = (gray & (1 << i)) ? '1' : '0';
            gray_bits[8] = '\0';

            if (!stopped) {
                OLED_ShowString(0, 0, (uint8_t*)"RUN", 16);
            } else {
                OLED_ShowString(0, 0, (uint8_t*)"STOP [4]->GO", 16);
            }

            /* Line 1: KP + KD */
            snprintf(t, sizeof(t), "%cKP%.1f %cKD%.1f",
                (pidx == 0) ? '>' : ' ',
                (double)g_kp,
                (pidx == 1) ? '>' : ' ',
                (double)g_kd);
            OLED_ShowString(0, 2, (uint8_t*)t, 16);

            /* Line 2: KI + SPD */
            snprintf(t, sizeof(t), "%cKI%.2f %cV%d",
                (pidx == 2) ? '>' : ' ',
                (double)g_ki,
                (pidx == 3) ? '>' : ' ',
                g_spd);
            OLED_ShowString(0, 4, (uint8_t*)t, 16);

            /* Line 3: 传感器状态 */
            snprintf(t, sizeof(t), "G:%s", gray_bits);
            OLED_ShowString(0, 6, (uint8_t*)t, 16);
        }

        delay_ms(LOOP_DELAY_MS);
    }
}
