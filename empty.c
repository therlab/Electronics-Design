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

/* ══════ 色标 (右边: PA9) ══════ */
static uint8_t color_right_read(void)
{
    return (DL_GPIO_readPins(GPIO_Color_PORT, GPIO_Color_RIGHT_PIN) != 0) ? 1 : 0;
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
    OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
    snprintf(t, sizeof(t), ">KP: %.1f", (double)g_kp);
    OLED_ShowString(0, 1, (uint8_t*)t, 16);
    snprintf(t, sizeof(t), " KD: %.1f", (double)g_kd);
    OLED_ShowString(0, 2, (uint8_t*)t, 16);
    snprintf(t, sizeof(t), " KI: %.2f", (double)g_ki);
    OLED_ShowString(0, 3, (uint8_t*)t, 16);

    while (1)
    {
        /* ══════ 按键处理 (每20次循环≈100ms) ══════ */
        static uint8_t key_tick = 0;
        if (++key_tick >= 20) {
            key_tick = 0;

            uint8_t k1 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY1_PIN);
            uint8_t k2 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY2_PIN);
            uint8_t k3 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY3_PIN);
            uint8_t k4 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY4_PIN);
            uint8_t k5 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY5_PIN);
            uint8_t k6 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY6_PIN);

            /* ──── KEY4 启停 toggle ──── */
            static uint8_t k4_was = 0;
            if (k4 && !k4_was) {
                stopped = !stopped;
                if (stopped) {
                    motor_control(0, 0);
                    g_pid_int = 0; g_pid_last = 0;
                    OLED_Clear();
                    OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                } else {
                    OLED_Clear();
                }
            }
            k4_was = k4;

            /* ──── 参数调节 (仅在 STOP 状态) ──── */
            if (stopped) {
                /* KEY1 短按: 切换参数 */
                static uint8_t k1_was = 0;
                if (k1 && !k1_was) { pidx = (pidx + 1) % 4; }
                k1_was = k1;

                /* KEY1 长按: 恢复默认 (~1s) */
                static uint8_t k1_hold = 0;
                if (k1) {
                    if (++k1_hold > 10) {
                        k1_hold = 0;
                        g_kp = GRAY_KP; g_kd = GRAY_KD;
                        g_ki = GRAY_KI; g_spd = BASE_SPEED;
                        g_pid_int = 0; g_pid_last = 0;
                    }
                } else { k1_hold = 0; }

                /* KEY2 长按: 保存 (~2s) */
                static uint8_t k2_hold = 0;
                if (k2) {
                    if (++k2_hold > 20) {
                        k2_hold = 0;
                        w25_params_save();
                        OLED_ShowString(0, 0, (uint8_t*)"SAVED!", 16);
                        delay_ms(800);
                        OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                    }
                } else { k2_hold = 0; }

                /* KEY2 / KEY5 短按: 参数+ */
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

                /* KEY3 / KEY6 短按: 参数- */
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
        uint8_t color = color_right_read();

        /* ══════ 色标触发停车 ══════ */
        if (!stopped && color)
        {
            motor_control(0, 0);
            stopped = 1;
            OLED_Clear();
            OLED_ShowString(0, 0, (uint8_t*)"COLOR HIT!", 16);
            OLED_ShowString(0, 1, (uint8_t*)"STOPPED", 16);
            snprintf(t, sizeof(t), "KP%.1f KD%.1f KI%.2f",
                (double)g_kp, (double)g_kd, (double)g_ki);
            OLED_ShowString(0, 2, (uint8_t*)t, 8);
            snprintf(t, sizeof(t), "SPD:%d", g_spd);
            OLED_ShowString(0, 3, (uint8_t*)t, 8);
        }

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
            if (!stopped) {
                /* ──── RUN 状态 OLED ──── */
                char tag0 = (pidx == 0) ? '>' : ' ';
                char tag1 = (pidx == 1) ? '>' : ' ';
                char tag2 = (pidx == 2) ? '>' : ' ';
                char tag3 = (pidx == 3) ? '>' : ' ';
                OLED_ShowString(0, 0, (uint8_t*)" RUN", 16);
                snprintf(t, sizeof(t), "%cKP: %.1f", tag0, (double)g_kp);
                OLED_ShowString(0, 1, (uint8_t*)t, 16);
                snprintf(t, sizeof(t), "%cKD: %.1f", tag1, (double)g_kd);
                OLED_ShowString(0, 2, (uint8_t*)t, 16);
                /* 第3行: KI 或 SPD 轮转 */
                if (pidx >= 2) {
                    /* 显示 KI 和 SPD (后者在 pidx=3时高亮) */
                    snprintf(t, sizeof(t), "%cKI:%.2f %cSPD:%d",
                        tag2, (double)g_ki, tag3, g_spd);
                } else {
                    snprintf(t, sizeof(t), "%cKI: %.2f", tag2, (double)g_ki);
                }
                OLED_ShowString(0, 3, (uint8_t*)t, 16);
            } else {
                /* ──── STOP 状态 OLED (不覆盖 COLOR HIT 首次显示) ──── */
                /* COLOR HIT 触发后 stopped=1, 已显示 COLOR HIT 画面 */
                /* 后续循环才刷新为正常 STOP 调参画面 */
                static uint8_t stop_refresh = 0;
                if (color && stop_refresh == 0) {
                    stop_refresh = 1;  /* 跳过首次, 保留 COLOR HIT 画面 */
                }
                if (++stop_refresh >= 3) { /* 延迟后开始刷新 STOP 画面 */
                    stop_refresh = 3;
                    char tag0 = (pidx == 0) ? '>' : ' ';
                    char tag1 = (pidx == 1) ? '>' : ' ';
                    char tag2 = (pidx == 2) ? '>' : ' ';
                    char tag3 = (pidx == 3) ? '>' : ' ';
                    OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                    snprintf(t, sizeof(t), "%cKP: %.1f", tag0, (double)g_kp);
                    OLED_ShowString(0, 1, (uint8_t*)t, 16);
                    snprintf(t, sizeof(t), "%cKD: %.1f", tag1, (double)g_kd);
                    OLED_ShowString(0, 2, (uint8_t*)t, 16);
                    if (pidx >= 2) {
                        snprintf(t, sizeof(t), "%cKI:%.2f %cSPD:%d",
                            tag2, (double)g_ki, tag3, g_spd);
                    } else {
                        snprintf(t, sizeof(t), "%cKI: %.2f", tag2, (double)g_ki);
                    }
                    OLED_ShowString(0, 3, (uint8_t*)t, 16);
                }
            }
        }

        delay_ms(LOOP_DELAY_MS);
    }
}
