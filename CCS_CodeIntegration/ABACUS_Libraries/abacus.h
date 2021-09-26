/*
 * abacus.h
 *
 */

#ifndef ABACUS_H_
#define ABACUS_H_

#include <msp430.h>
#include "communications/abacus_uart.h"
#include "communications/abacus_i2c.h"
#include "communications/abacus_spi.h"
#include "sensors/abacus_temperature.h"
#include "sensors/abacus_accelerometer.h"
#include "sensors/abacus_gyroscope.h"
#include "sensors/abacus_magnetometer.h"
#include "memory/abacus_flash.h"
#include "GPIO/abacus_gpio_analog.h"
#include "GPIO/abacus_gpio_exp.h"
#include "GPIO/abacus_gpio.h"
#include "RTC/abacus_rtc.h"
#include "RTC/abacus_rtc_internal.h"
#include "FPGA/abacus_fpga.h"
#include "timer/abacus_timer.h"

extern uint8_t abacusLPMStatus;

int abacus_init(uint8_t abacus_version, uint8_t abacus_clockSpeed);
int abacus_init_quiet();
void abacus_init_clock(uint8_t abacus_clockSpeed);
int abacus_enter_LPM(uint8_t lpmSelection);
int abacus_set_WDT();


//Definitions of names of ABACUS:
#define AB_ABACUSVERSION2013	0
#define AB_ABACUSVERSION2014	1

#define AB_UART00 0
#define AB_UART01 1
#define AB_UART02 2
#define AB_UART03 4

#define AB_CLOCK1MHZ 0
#define AB_CLOCK8MHZ 1

#define AB_B9600   0
#define AB_B19200  1
#define AB_B38400  2
#define AB_B57600  3
#define AB_B115200 4
#define AB_B230400 5
#define AB_B460800 6

//BUS01 is the one to external cubesat standard
#define AB_I2C_BUS00	0
#define AB_I2C_BUS01	1

//BUS00 is the MCU flash, BUS01 is the FPGA flash
#define AB_SPI_BUS00	0
#define AB_SPI_BUS01	1

//Flash memories for MCU and FPGA
#define AB_FLASH_MCU	0
#define AB_FLASH_FPGA	1

//Low power mode configuration
#define AB_LPM0	0
#define AB_LPM1	1
#define AB_LPM2	2
#define AB_LPM3	3
#define AB_LPM4	4
#define AB_LPM5	5
#define AB_AM	6


//Definitions of GPIOs of the connector H1 and H2
#define AB_RTC_VBACKUP	0
#define AB_CURRENT		1
#define AB_H1_3   3
#define AB_H1_4   4
#define AB_H1_5   5
#define AB_H1_6   6
#define AB_H1_8   8
#define AB_H1_10 10
#define AB_H1_12 12
#define AB_H1_13 13
#define AB_H1_14 14
#define AB_H1_15 15
#define AB_H1_16 16
#define AB_H1_18 18
#define AB_H1_19 19
#define AB_H1_20 20
#define AB_H1_22 22
#define AB_H1_25 25
#define AB_H1_27 27
#define AB_H1_28 28
#define AB_H1_29 29
#define AB_H1_30 30
#define AB_H1_31 31
#define AB_H1_33 33
#define AB_H1_34 34
#define AB_H1_35 35
#define AB_H1_36 36
#define AB_H1_37 37
#define AB_H1_38 38
#define AB_H1_39 39
#define AB_H1_40 40
#define AB_H2_1  54
#define AB_H2_2  53
#define AB_H2_3  56
#define AB_H2_4  55
#define AB_H2_5  58
#define AB_H2_6  57
#define AB_H2_7  60
#define AB_H2_8  59
#define AB_H2_9  62
#define AB_H2_10 61
#define AB_H2_11 64
#define AB_H2_12 63
#define AB_H2_13 66
#define AB_H2_14 65
#define AB_H2_15 68
#define AB_H2_16 67

#define AB_LOW    0
#define AB_HIGH   1
#define AB_INPUT  0
#define AB_OUTPUT 1

#define AB_PULLDISABLE 0
#define AB_PULLDOWN    1
#define AB_PULLUP      2

#define AB_LOW2HIGH    0
#define AB_HIGH2LOW    1

#define AB_LED_ON	(P11OUT |= BIT0)
#define AB_LED_OFF	(P11OUT &= ~BIT0)

#define AB_IS_1MHZ (UCSCTL1 == DCORSEL_2)
#define AB_IS_8MHZ (UCSCTL1 == DCORSEL_5)

#endif /* ABACUS_H_ */
