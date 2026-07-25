#ifndef _PID_H
#define _PID_H
#include "ti_msp_dl_config.h"
#include "config.h"
#include <stdint.h>

extern uint8_t circle_mode;
extern uint8_t run_flag;
extern int16_t left_target_speed;
extern int16_t right_target_speed;

float compute_weighted_error_digital(uint8_t gray_bits);
float compute_weighted_error(uint16_t normal[8]);
float grayscale_pid(float error);
float yaw_pd(float error_angle);
float speed_pi_left(float target, float actual);
float speed_pi_right(float target, float actual);
void pid_reset_all(void);

#endif
