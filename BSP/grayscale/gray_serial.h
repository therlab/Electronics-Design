/**
 * @file    gray_serial.h
 * @brief   8-channel grayscale serial driver (CLK+DAT via auxiliary board)
 *
 * Reads auxiliary board via CLK+DAT serial protocol.
 * CLK=PB7 (output), DAT=PB8 (input).
 * Returns 8-bit value: bit0=sensor1(left) ... bit7=sensor8(right).
 * Each bit: 1=white line, 0=black ground.
 */

#ifndef GRAY_SERIAL_H
#define GRAY_SERIAL_H
#include "ti_msp_dl_config.h"
#include <stdint.h>

uint8_t gray_serial_read(void);

#endif
