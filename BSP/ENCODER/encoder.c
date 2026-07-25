/**
 * @file    encoder.c
 * @brief   Encoder pulse counting via GPIO polling in timer ISR
 *          L_A=PB24 L_B=PA25 R_A=PB25 R_B=PA26
 *
 * Uses 100ms timer ISR for polling — simpler and more reliable than interrupts.
 */

#include "encoder.h"

static volatile int32_t g_enc_l = 0;
static volatile int32_t g_enc_r = 0;
static int32_t g_enc_l_last = 0;
static int32_t g_enc_r_last = 0;
static int16_t g_enc_l_spd = 0;
static int16_t g_enc_r_spd = 0;

static uint8_t g_last_la = 0;
static uint8_t g_last_ra = 0;

void encoder_init(void)
{
    /* L_A PB24: input, pull-up + hysteresis */
    DL_GPIO_initDigitalInputFeatures(GPIO_Encoder_L_A_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
    /* L_B PA25: input, pull-up + hysteresis */
    DL_GPIO_initDigitalInputFeatures(GPIO_Encoder_L_B_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
    /* R_A PB25: input, pull-up + hysteresis */
    DL_GPIO_initDigitalInputFeatures(GPIO_Encoder_R_A_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
    /* R_B PA26: input, pull-up + hysteresis */
    DL_GPIO_initDigitalInputFeatures(GPIO_Encoder_R_B_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
}

void encoder_poll(void)
{
    uint8_t la = DL_GPIO_readPins(GPIO_Encoder_L_A_PORT, GPIO_Encoder_L_A_PIN) ? 1 : 0;
    uint8_t ra = DL_GPIO_readPins(GPIO_Encoder_R_A_PORT, GPIO_Encoder_R_A_PIN) ? 1 : 0;

    /* Left: 简化——A相每次跳变计数+1，不判方向 */
    if (la != g_last_la) {
        g_last_la = la;
        g_enc_l++;
    }

    /* Right: 简化——A相每次跳变计数+1，不判方向 */
    if (ra != g_last_ra) {
        g_last_ra = ra;
        g_enc_r++;
    }
}

void encoder_update_speed(void)
{
    int32_t l_d = g_enc_l - g_enc_l_last;
    int32_t r_d = g_enc_r - g_enc_r_last;
    g_enc_l_last = g_enc_l;
    g_enc_r_last = g_enc_r;
    g_enc_l_spd = (int16_t)(0.7f * g_enc_l_spd + 0.3f * l_d);
    g_enc_r_spd = (int16_t)(0.7f * g_enc_r_spd + 0.3f * r_d);
}

int16_t encoder_get_left_speed(void)  { return g_enc_l_spd; }
int16_t encoder_get_right_speed(void) { return g_enc_r_spd; }
int32_t encoder_get_left_count(void)  { return g_enc_l; }
int32_t encoder_get_right_count(void) { return g_enc_r; }

float encoder_get_distance_mm(void) {
    /* Use absolute count for calibration (ignore direction sign) */
    int32_t l = g_enc_l > 0 ? g_enc_l : -g_enc_l;
    int32_t r = g_enc_r > 0 ? g_enc_r : -g_enc_r;
    return ((l + r) / 2.0f) * MM_PER_COUNT;
}
void encoder_reset_distance(void) {
    g_enc_l = 0; g_enc_r = 0;
    g_enc_l_last = 0; g_enc_r_last = 0;
    g_enc_l_spd = 0; g_enc_r_spd = 0;
}
