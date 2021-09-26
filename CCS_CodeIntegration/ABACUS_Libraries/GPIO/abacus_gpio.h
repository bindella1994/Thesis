/*
 * abacus_gpio_exp.h
 *
 */

#ifndef ABACUS_GPIO_H_
#define ABACUS_GPIO_H_

#include "msp430x54xa.h"
#include "../abacus.h"
#include "../abacus_utils.h"
#include "abacus_gpio_exp.h"
#include "stdint.h"

struct GPIOdigital
{
	uint8_t interruptPort;
	void (*triggerFunction)(int*);
	uint8_t interruptEnabled;
};

extern struct GPIOdigital abacus_gpio;

int abacus_gpio_init();
uint8_t abacus_gpio_digitalWrite(uint8_t port, uint8_t signal);
uint8_t abacus_gpio_digitalRead(uint8_t port);
uint8_t abacus_gpio_digitalMode(uint8_t port, uint8_t signal);
uint8_t abacus_gpio_digitalPullupdown(uint8_t port, uint8_t conf);
int8_t abacus_gpio_digitalEnableInterrupt(uint8_t port, void (*interruptFunction)(int*), uint8_t direction);
int8_t abacus_gpio_digitalDisableInterrupt(uint8_t port);
uint8_t abacus_gpio_digitalGetLastInterruptPort();

#endif /* ABACUS_GPIO_H_ */
