/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */


#pragma once

/*
 * lkk
 * The Eclipse build configuration (in Properties>C/C++ Build>Settings>Preprocessor>Defined Symbols:)
 * defines either BOARD_PCA10040 or BOARD_CUSTOM.
 * The former causes include from SDK of a file custom to the nRF52DK dev board.
 * The latter causes include this file for RedBear Nano or other custom board.
 *
 * The path to this file must also be in the build configuration (in Properties>C/C++ Build>Settings>Preprocessor>Includes
 */
// change the next line to choose a custom board
#if 0

#define REDBEAR_NANO_H__

#define LEDS_NUMBER    1

#define LED_START  19
#define BSP_LED_0  19
#define LED_STOP   19

#define LEDS_MASK      (BSP_LED_0_MASK)
// this means is a sink, pin low means LED is lit
#define LEDS_INV_MASK  LEDS_MASK

#else

#define MY_CUSTOM_BOARD_H__
#define UBLOX_NINA_H__

#define LEDS_NUMBER    1

#define LED_START  28
#define BSP_LED_0  28
#define LED_STOP   28

#define LEDS_MASK      (BSP_LED_0_MASK)
// this means is a source, pin low means LED is not lit
#define LEDS_INV_MASK  ~LEDS_MASK

#endif


#define BUTTONS_LIST {}
#define LEDS_LIST { BSP_LED_0 }

#define BSP_LED_0_MASK    (1<<BSP_LED_0)

// bsp.c assumes BSP_LED_1_MASK always exists
#define BSP_LED_1_MASK    (1<<BSP_LED_0)



// there are no buttons on this board
#define BUTTONS_NUMBER 0
#define BUTTONS_MASK   0x00000000

// UART pins connected to J-Link
#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true


