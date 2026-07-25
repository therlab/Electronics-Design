#include "pid.h"
#include "bsp_gyro.h"
#include "config.h"
#include <math.h>

extern uint8_t circle_mode;  /* defined in fun.c */
extern uint8_t run_flag;      /* defined in fun.c */
int16_t left_target_speed = 0, right_target_speed = 0;

/* ---- 8-bit digital weighted error ---- */
float compute_weighted_error_digital(uint8_t gray_bits)
{
    static const int16_t w[8] = {
        GRAY_W1, GRAY_W2, GRAY_W3, GRAY_W4,
        GRAY_W5, GRAY_W6, GRAY_W7, GRAY_W8
    };
    int32_t total = 0;
    uint8_t count = 0;
    static float last_err = 0;

    for (uint8_t i = 0; i < 8; i++) {
        if (gray_bits & (1 << i)) { total += w[i]; count++; }
    }
    if (count > 0) { last_err = (float)total / (float)count; return last_err; }
    /* Lost line */
    if (last_err < 0) return -LOST_LINE_SPEED;
    if (last_err > 0) return  LOST_LINE_SPEED;
    return -LOST_LINE_SPEED;
}

float compute_weighted_error(uint16_t normal[8]) { return 0; }

/* ---- Grayscale position PID ---- */
static float g_gray_last = 0, g_gray_int = 0;

float grayscale_pid(float error)
{
    g_gray_int += error;
    if (g_gray_int > GRAY_INT_MAX)  g_gray_int = GRAY_INT_MAX;
    if (g_gray_int < -GRAY_INT_MAX) g_gray_int = -GRAY_INT_MAX;
    float deriv = error - g_gray_last;
    g_gray_last = error;
    return GRAY_KP * error + GRAY_KI * g_gray_int + GRAY_KD * deriv;
}

/* ---- Yaw PID ---- */
static float g_yaw_last = 0;
static float g_yaw_int  = 0;
float yaw_pd(float err)
{
    g_yaw_int += err;
    if (g_yaw_int > YAW_INT_MAX)  g_yaw_int = YAW_INT_MAX;
    if (g_yaw_int < -YAW_INT_MAX) g_yaw_int = -YAW_INT_MAX;
    float d = err - g_yaw_last;
    g_yaw_last = err;
    return YAW_KP * err + YAW_KI * g_yaw_int + YAW_KD * d;
}

/* ---- Speed PI ---- */
static float g_l_int = 0, g_r_int = 0;

float speed_pi_left(float tgt, float act)
{
    float e = tgt - act;
    g_l_int += e;
    if (g_l_int > SPEED_INT_MAX)  g_l_int = SPEED_INT_MAX;
    if (g_l_int < -SPEED_INT_MAX) g_l_int = -SPEED_INT_MAX;
    return SPEED_KP * e + SPEED_KI * g_l_int;
}

float speed_pi_right(float tgt, float act)
{
    float e = tgt - act;
    g_r_int += e;
    if (g_r_int > SPEED_INT_MAX)  g_r_int = SPEED_INT_MAX;
    if (g_r_int < -SPEED_INT_MAX) g_r_int = -SPEED_INT_MAX;
    return SPEED_KP * e + SPEED_KI * g_r_int;
}

void pid_reset_all(void)
{
    g_gray_int = g_gray_last = 0;
    g_yaw_last = 0;
    g_yaw_int  = 0;
    g_l_int = g_r_int = 0;
}
