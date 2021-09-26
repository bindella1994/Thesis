/*
 * abacus_P2Interrupts.c
 */

#include "abacus_P2Interrupts.h"

///////////////////////////////////////////////////////////////////////////////
// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	int wakeupOnExit = 0;

	if(P2IFG & BIT1)
	{
		//Ok, we got an interrupt from the GPIO expander

		//Clear interrupt
		P2IFG &= ~BIT1;

		//Read registers of the interrupts to check who is guilty
		uint8_t registryAddress = 0x4C;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
		abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptStatusPort0, 1, 0);
		registryAddress = 0x4D;
		abacus_i2c_write(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &registryAddress, 1, 0);
		abacus_i2c_requestFrom(AB_I2C_BUS01, AB_ADDRESS_GPIOEXPANDER, &abacus_gpio_expander.interruptStatusPort1, 1, 0);

		//Interrupt is cleared :)
		//Set which port was the culprit:
		if(abacus_gpio_expander.interruptStatusPort0 != 0)
		{
			if((abacus_gpio_expander.interruptStatusPort0 & BIT0))
				abacus_gpio.interruptPort = AB_H2_2;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT1))
				abacus_gpio.interruptPort = AB_H2_1;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT2))
				abacus_gpio.interruptPort = AB_H2_4;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT3))
				abacus_gpio.interruptPort = AB_H2_3;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT4))
				abacus_gpio.interruptPort = AB_H2_6;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT5))
				abacus_gpio.interruptPort = AB_H2_5;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT6))
				abacus_gpio.interruptPort = AB_H2_8;
			else if((abacus_gpio_expander.interruptStatusPort0 & BIT7))
				abacus_gpio.interruptPort = AB_H2_7;
		}
		else
		{
			if((abacus_gpio_expander.interruptStatusPort1 & BIT0))
				abacus_gpio.interruptPort = AB_H2_10;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT1))
				abacus_gpio.interruptPort = AB_H2_9;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT2))
				abacus_gpio.interruptPort = AB_H2_12;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT3))
				abacus_gpio.interruptPort = AB_H2_11;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT4))
				abacus_gpio.interruptPort = AB_H2_14;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT5))
				abacus_gpio.interruptPort = AB_H2_13;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT6))
				abacus_gpio.interruptPort = AB_H2_16;
			else if((abacus_gpio_expander.interruptStatusPort1 & BIT7))
				abacus_gpio.interruptPort = AB_H2_15;
		}

		int wakeupOnExit = 0;

		//Call the associated function
		if(abacus_gpio_expander.ioExpanderInterruptEnabled == 1)
			(*(abacus_gpio_expander.triggerIOExpanderFunction))(&wakeupOnExit);

	}

	if(P2IFG & BIT0)
	{
		//Ok, we got an interrupt at H1_8

		//Clear interrupt
		P2IFG &= ~BIT0;
		abacus_gpio.interruptPort = AB_H1_8;

		//Call the associated function
		if(abacus_gpio.interruptEnabled == 1)
			(*(abacus_gpio.triggerFunction))(&wakeupOnExit);
	}

	if(P2IFG & BIT2)
	{
		//Ok, we got an interrupt at H1_13

		//Clear interrupt
		P2IFG &= ~BIT2;
		abacus_gpio.interruptPort = AB_H1_13;

		//Call the associated function
		if(abacus_gpio.interruptEnabled == 1)
			(*(abacus_gpio.triggerFunction))(&wakeupOnExit);
	}

	if(P2IFG & BIT6)
	{
		//Ok, we got an interrupt at H1.38

		//Clear interrupt
		P2IFG &= ~BIT6;
		abacus_gpio.interruptPort = AB_H1_38;

		//Call the associated function
		if(abacus_gpio.interruptEnabled == 1)
			(*(abacus_gpio.triggerFunction))(&wakeupOnExit);
	}


	if(P2IFG & BIT7)
	{
		//Ok, we got an interrupt at H1.36

		//Clear interrupt
		P2IFG &= ~BIT7;
		abacus_gpio.interruptPort = AB_H1_36;

		//Call the associated function
		if(abacus_gpio.interruptEnabled == 1)
			(*(abacus_gpio.triggerFunction))(&wakeupOnExit);
	}

	//TODO the interrupts of the ACC, GYRO and Termometer?


	if(wakeupOnExit == 1)
	{
		//The user wants to exit LPM
		switch(abacusLPMStatus)
		{
		case AB_LPM0:
			__bic_SR_register_on_exit(LPM0_bits);
			break;
		case AB_LPM1:
			__bic_SR_register_on_exit(LPM1_bits);
			break;
		case AB_LPM2:
			__bic_SR_register_on_exit(LPM2_bits);
			break;
		case AB_LPM3:
			__bic_SR_register_on_exit(LPM3_bits);
			break;
		case AB_LPM4:
			__bic_SR_register_on_exit(LPM4_bits);
			break;
		case AB_LPM5:
			//__bic_SR_register_on_exit(LPM5_bits);
			break;
		default:
			break;
		}
	}
}


