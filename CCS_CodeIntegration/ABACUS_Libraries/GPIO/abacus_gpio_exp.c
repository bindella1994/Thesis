/*
 * abacus_gpio_exp.c
 *
 */

#include "abacus_gpio_exp.h"

struct GPIOExpander abacus_gpio_expander;

uint8_t maskIoExpanderPort(uint8_t port)
{
	switch(port)
	{
		case AB_H2_1:
			return 1;
		case AB_H2_2:
			return 0;
		case AB_H2_3:
			return 3;
		case AB_H2_4:
			return 2;
		case AB_H2_5:
			return 5;
		case AB_H2_6:
			return 4;
		case AB_H2_7:
			return 7;
		case AB_H2_8:
			return 6;
		case AB_H2_9:
			return 9;
		case AB_H2_10:
			return 8;
		case AB_H2_11:
			return 11;
		case AB_H2_12:
			return 10;
		case AB_H2_13:
			return 13;
		case AB_H2_14:
			return 12;
		case AB_H2_15:
			return 15;
		case AB_H2_16:
			return 14;
		default:
			return 0;
	}
}

uint8_t maskPort2IoExpander(uint8_t ioExpander)
{
	switch(ioExpander)
	{
	case 1:
		return AB_H2_1;
	case 0:
		return AB_H2_2;
	case 3:
		return AB_H2_3;
	case 2:
		return AB_H2_4;
	case 5:
		return AB_H2_5;
	case 4:
		return AB_H2_6;
	case 7:
		return AB_H2_7;
	case 6:
		return AB_H2_8;
	case 9:
		return AB_H2_9;
	case 8:
		return AB_H2_10;
	case 11:
		return AB_H2_11;
	case 10:
		return AB_H2_12;
	case 13:
		return AB_H2_13;
	case 12:
		return AB_H2_14;
	case 15:
		return AB_H2_15;
	case 14:
		return AB_H2_16;
	default:
		return 0;
	}
}

uint8_t portBitMap(uint8_t port)
{
	switch(port)
	{
		case 0:
			return BIT0;
		case 1:
			return BIT1;
		case 2:
			return BIT2;
		case 3:
			return BIT3;
		case 4:
			return BIT4;
		case 5:
			return BIT5;
		case 6:
			return BIT6;
		case 7:
			return BIT7;
		default:
			return 0;
	}
}

/*
 * It reads all the registers of the GPIO expander
 */
uint8_t abacus_gpio_GPIOExpLoad()
{
	uint8_t registryAddress = 0x00;

	//Set P2.1 as input (this is for the interrupts)
	//As GPIO
	P2SEL &= ~BIT1;
	//Disable Pullups/downs
	P2REN &= ~BIT1;
	//As input
	P2DIR &= ~BIT1;

	//Set P7.1 as output (for reseting gpio expander)
	//As GPIO
	P7SEL &= ~BIT1;
	//As output
	P7DIR |= BIT1;

	//Set low to reset
	P7OUT &= ~BIT1;
	_delay_cycles (20);
	//Set high to start using it
	P7OUT |= BIT1;


	registryAddress = 0x00;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.inputPort0, 1, 0);
	registryAddress = 0x01;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.inputPort1, 1, 0);

	registryAddress = 0x02;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outputPort0, 1, 0);
	registryAddress = 0x03;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outputPort1, 1, 0);

	registryAddress = 0x04;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.polarityInversionPort0, 1, 0);
	registryAddress = 0x05;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.polarityInversionPort1, 1, 0);

	registryAddress = 0x06;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.configurationPort0, 1, 0);
	registryAddress = 0x07;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.configurationPort1, 1, 0);

	registryAddress = 0x40;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outpudDriveStrenghtPort0_0, 1, 0);
	registryAddress = 0x41;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outpudDriveStrenghtPort0_1, 1, 0);
	registryAddress = 0x42;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outpudDriveStrenghtPort1_0, 1, 0);
	registryAddress = 0x43;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outpudDriveStrenghtPort1_1, 1, 0);

	registryAddress = 0x44;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.inputLatchPort0, 1, 0);
	registryAddress = 0x45;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.inputLatchPort1, 1, 0);

	registryAddress = 0x46;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.pullupdownEnablePort0, 1, 0);
	registryAddress = 0x47;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.pullupdownEnablePort1, 1, 0);

	registryAddress = 0x48;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.pullupdownSelectionPort0, 1, 0);
	registryAddress = 0x49;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.pullupdownSelectionPort1, 1, 0);

	registryAddress = 0x4A;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptMaskPort0, 1, 0);
	registryAddress = 0x4B;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptMaskPort1, 1, 0);

	registryAddress = 0x4C;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptStatusPort0, 1, 0);
	registryAddress = 0x4D;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptStatusPort1, 1, 0);

	registryAddress = 0x4F;
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.outputPortConf, 1, 0);

	abacus_gpio_expander.ioExpanderInterruptEnabled = 0;

	return 0;
}

/*
 * This function sets Low or high a bit in the GPIOs of the H1 and H2 ports
 * Options are:
 * AB_HIGH
 * AB_LOW
 */
uint8_t abacus_gpio_GPIOExpDigitalWrite(uint8_t port, uint8_t signal)
{
	//It is a pin from the GPIO expander
	uint8_t bufferI2c[2];
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t registryAddress = 0x02;

	if(maskPort > 7)
	{
		maskPort -= 8;
		registryAddress = 0x03;

		if(signal == AB_LOW)
			abacus_gpio_expander.outputPort1 &= ~portBitMap(maskPort);
		else
			abacus_gpio_expander.outputPort1 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.outputPort1;
	}
	else
	{
		if(signal == AB_LOW)
			abacus_gpio_expander.outputPort0 &= ~portBitMap(maskPort);
		else
			abacus_gpio_expander.outputPort0 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.outputPort0;
	}

	bufferI2c[0] = registryAddress;

	//Send the i2c data
	return abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);
}

/*
 * This function reads the inputs of the H1 and H2 ports
 */
uint8_t abacus_gpio_GPIOExpDigitalRead(uint8_t port)
{
	//It is a pin from the GPIO expander
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t registryAddress = 0x00;

	if(maskPort > 7)
	{
		maskPort -= 8;
		registryAddress = 0x01;
		abacus_gpio_expander.outputPort1 = abacus_gpio_GPIOExpReadPort(registryAddress);
		if(abacus_gpio_expander.outputPort1 & portBitMap(maskPort))
			return AB_HIGH;
		else
			return AB_LOW;
	}
	else
	{
		abacus_gpio_expander.outputPort0 = abacus_gpio_GPIOExpReadPort(registryAddress);
		if(abacus_gpio_expander.outputPort0 & portBitMap(maskPort))
			return AB_HIGH;
		else
			return AB_LOW;
	}

}

/*
 * This function sets the pins of H1 and H2 ports to output or input
 * Options are:
 * AB_INPUT
 * AB_OUTPUT
 */
uint8_t abacus_gpio_GPIOExpDigitalMode(uint8_t port, uint8_t signal)
{
	//It is a pin from the GPIO expander
	uint8_t bufferI2c[2];
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t registryAddress = 0x06;

	if(maskPort > 7)
	{
		maskPort -= 8;
		registryAddress = 0x07;

		if(signal == AB_OUTPUT)
			abacus_gpio_expander.configurationPort1 &= ~portBitMap(maskPort);
		else
			abacus_gpio_expander.configurationPort1 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.configurationPort1;
	}
	else
	{
		if(signal == AB_OUTPUT)
			abacus_gpio_expander.configurationPort0 &= ~portBitMap(maskPort);
		else
			abacus_gpio_expander.configurationPort0 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.configurationPort0;
	}

	bufferI2c[0] = registryAddress;

	//Send the i2c data
	return abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);

}

/*
 * This function sets the pullup and downs of the H1 and H2. Options are:
 *  AB_PULLDISABLE
 *  AB_PULLDOWN
 *  AB_PULLUP
 */
uint8_t abacus_gpio_GPIOExpDigitalPullupdown(uint8_t port, uint8_t conf)
{
	//It is a pin from the GPIO expander
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t bufferI2c[2];

	if(maskPort > 7)
	{
		maskPort -= 8;

		if(conf == AB_PULLDOWN)
		{
			abacus_gpio_expander.pullupdownEnablePort1 |= portBitMap(maskPort);
			abacus_gpio_expander.pullupdownSelectionPort1 &= ~portBitMap(maskPort);
		}
		else if(conf == AB_PULLUP)
		{
			abacus_gpio_expander.pullupdownEnablePort1 |= portBitMap(maskPort);
			abacus_gpio_expander.pullupdownSelectionPort1 &= ~portBitMap(maskPort);
		}
		else
		{
			abacus_gpio_expander.pullupdownEnablePort1 &= ~portBitMap(maskPort);
		}
		//Send the i2c data
		bufferI2c[0] = 0x47;
		bufferI2c[1] = abacus_gpio_expander.pullupdownEnablePort1;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);
		bufferI2c[0] = 0x49;
		bufferI2c[1] = abacus_gpio_expander.pullupdownSelectionPort1;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);
	}
	else
	{
		if(conf == AB_PULLDOWN)
		{
			abacus_gpio_expander.pullupdownEnablePort0 |= portBitMap(maskPort);
			abacus_gpio_expander.pullupdownSelectionPort0 &= ~portBitMap(maskPort);
		}
		else if(conf == AB_PULLUP)
		{
			abacus_gpio_expander.pullupdownEnablePort0 |= portBitMap(maskPort);
			abacus_gpio_expander.pullupdownSelectionPort0 &= ~portBitMap(maskPort);
		}
		else
		{
			abacus_gpio_expander.pullupdownEnablePort0 &= ~portBitMap(maskPort);
		}
		//Send the i2c data
		bufferI2c[0] = 0x46;
		bufferI2c[1] = abacus_gpio_expander.pullupdownEnablePort0;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);
		bufferI2c[0] = 0x48;
		bufferI2c[1] = abacus_gpio_expander.pullupdownSelectionPort0;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);
	}

	return 0;
}

/*
 * It enables the interrupts
 */
uint8_t abacus_gpio_GPIOExpDigitalEnableInterrupt(uint8_t port, void (*interruptFunction)(int*))
{
	//It is a pin from the GPIO expander
	uint8_t bufferI2c[2];
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t registryAddress = 0x4A;

	if(maskPort > 7)
	{
		maskPort -= 8;
		registryAddress = 0x4B;
		abacus_gpio_expander.interruptMaskPort1 &= ~portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.interruptMaskPort1;
	}
	else
	{
		abacus_gpio_expander.interruptMaskPort0 &= ~portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.interruptMaskPort0;
	}

	bufferI2c[0] = registryAddress;

	//Send the i2c data
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);

	//Set Function:
	abacus_gpio_expander.triggerIOExpanderFunction = interruptFunction;
	abacus_gpio_expander.ioExpanderInterruptEnabled = 1;

	//Now enable interrupts on MCU pin
	//Enable interrupt
	P2IE |= BIT1;
	//Low to High edge
	P2IES &= ~BIT1;
	//Clear previous interrupts
	P2IFG &= ~BIT1;

	return 0;
}

/*
 * It disables the interrupts
 */
uint8_t abacus_gpio_GPIOExpDigitalDisableInterrupt(uint8_t port)
{
	//It is a pin from the GPIO expander
	uint8_t bufferI2c[2];
	uint8_t maskPort = maskIoExpanderPort(port);
	uint8_t registryAddress = 0x4A;

	if(maskPort > 7)
	{
		maskPort -= 8;
		registryAddress = 0x4B;
		abacus_gpio_expander.interruptMaskPort1 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.interruptMaskPort1;
	}
	else
	{
		abacus_gpio_expander.interruptMaskPort0 |= portBitMap(maskPort);
		bufferI2c[1] = abacus_gpio_expander.interruptMaskPort0;
	}

	bufferI2c[0] = registryAddress;

	//Send the i2c data
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, bufferI2c, 2, 0);

	if(abacus_gpio_expander.interruptMaskPort0 == 0xFF
			&& abacus_gpio_expander.interruptMaskPort1 == 0xFF )
	{
		//Disable all interrupts
		abacus_gpio_expander.ioExpanderInterruptEnabled = 0;
		//Now enable interrupts on MCU pin
		//Disable interrupt
		P2IE &= ~BIT1;
	}

	return 0;
}

/*
 * Returns the register of the status of the GPIO expander ports:
 * H2.1 to H2.16
 */
uint8_t abacus_gpio_GPIOExpDigitalGetInterruptRegister(uint8_t *port0, uint8_t *port1)
{
	*port0 = abacus_gpio_expander.interruptStatusPort0;
	*port1 = abacus_gpio_expander.interruptStatusPort1;
	//Clear flags
	abacus_gpio_expander.interruptStatusPort0 = 0;
	abacus_gpio_expander.interruptStatusPort1 = 0;
	return 0;
}

uint8_t abacus_gpio_GPIOExpReadPort(uint8_t registryAddress)
{
	uint8_t registryConf = 0;

	//Ask
	abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
	abacus_gpio_expander.errorDetected = abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryConf, 1, 0);

	return registryConf;
}


