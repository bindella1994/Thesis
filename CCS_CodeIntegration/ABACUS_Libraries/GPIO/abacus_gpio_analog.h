/*
 * abacus_analog.h
 *
 */

#ifndef ABACUS_GPIO_ANALOG_H_
#define ABACUS_GPIO_ANALOG_H_

#include "msp430x54xa.h"
#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

int32_t abacus_gpio_analogRead(uint8_t port);
uint16_t abacus_currentRead();

#endif /* ABACUS_ANALOG_H_ */
