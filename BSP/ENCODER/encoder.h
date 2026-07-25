/**
 * @file    encoder.h
 * @brief   Encoder pulse counting - polling mode
 */

#ifndef ENCODER_H
#define ENCODER_H
#include "ti_msp_dl_config.h"
#include "config.h"
#include <stdint.h>

void encoder_init(void);
void encoder_poll(void);
void encoder_update_speed(void);
int16_t encoder_get_left_speed(void);
int16_t encoder_get_right_speed(void);
int32_t encoder_get_left_count(void);
int32_t encoder_get_right_count(void);
float encoder_get_distance_mm(void);   /* abs distance in mm */
void encoder_reset_distance(void);     /* zero distance counter */

#endif
