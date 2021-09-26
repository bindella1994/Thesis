/*
 * abacus_gpio_exp.h
 *
 */

#ifndef ABACUS_GPIO_EXP_H_
#define ABACUS_GPIO_EXP_H_

#include "msp430x54xa.h"
#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

//I2C address for sensor1 close to MSP430
#define AB_ADDRESS_GPIOEXPANDER 0x20

struct GPIOExpander
{
	uint8_t inputPort0;
	uint8_t inputPort1;
	uint8_t outputPort0;
	uint8_t outputPort1;
	uint8_t polarityInversionPort0;
	uint8_t polarityInversionPort1;

	uint8_t configurationPort0;
	uint8_t configurationPort1;

	uint8_t outpudDriveStrenghtPort0_0;
	uint8_t outpudDriveStrenghtPort0_1;
	uint8_t outpudDriveStrenghtPort1_0;
	uint8_t outpudDriveStrenghtPort1_1;

	uint8_t inputLatchPort0;
	uint8_t inputLatchPort1;

	uint8_t pullupdownEnablePort0;
	uint8_t pullupdownEnablePort1;

	uint8_t pullupdownSelectionPort0;
	uint8_t pullupdownSelectionPort1;

	uint8_t interruptMaskPort0;
	uint8_t interruptMaskPort1;

	uint8_t interruptStatusPort0;
	uint8_t interruptStatusPort1;

	uint8_t outputPortConf;

	void (*triggerIOExpanderFunction)(int*);
	uint8_t ioExpanderInterruptEnabled;

	uint8_t errorDetected;
};

extern struct GPIOExpander abacus_gpio_expander;

uint8_t abacus_gpio_GPIOExpLoad();
uint8_t abacus_gpio_GPIOExpDigitalWrite(uint8_t port, uint8_t signal);
uint8_t abacus_gpio_GPIOExpDigitalRead(uint8_t port);
uint8_t abacus_gpio_GPIOExpDigitalMode(uint8_t port, uint8_t signal);
uint8_t abacus_gpio_GPIOExpDigitalPullupdown(uint8_t port, uint8_t conf);
uint8_t abacus_gpio_GPIOExpDigitalEnableInterrupt(uint8_t port, void (*interruptFunction)(int*));
uint8_t abacus_gpio_GPIOExpDigitalDisableInterrupt(uint8_t port);
uint8_t abacus_gpio_GPIOExpDigitalGetInterruptRegister(uint8_t *port0, uint8_t *port1);
uint8_t abacus_gpio_GPIOExpReadPort(uint8_t registryAddress);

uint8_t maskIoExpanderPort(uint8_t port);
uint8_t maskPort2IoExpander(uint8_t ioExpander);
uint8_t portBitMap(uint8_t port);

#endif /* ABACUS_GPIO_EXP_H_ */
