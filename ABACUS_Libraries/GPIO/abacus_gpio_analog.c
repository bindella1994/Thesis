/*
 * abacus_analog.c
 */

#include "abacus_gpio_analog.h"


/*
 * It reads the analogic 12bit value of the specified port. Available Ports are:
 * - AB_Current: current of MCU
 * - AB_RTC_VBACKUPS: Voltage of external RTC auxiliar battery
 * - AB_H1_22
 * - AB_H1_33
 * - AB_H1_20
 * - AB_H1_31
 * - AB_H1_18
 * - AB_H1_29
 * - AB_H1_16
 * - AB_H1_27
 * - AB_H1_14
 * - AB_H1_25
 */
int32_t abacus_gpio_analogRead(uint8_t port)
{

	uint16_t result;

	// Disable conversion
	ADC12CTL0 &= ~ADC12ENC;

	//Clear ADC12CSTARTADDx field
	ADC12CTL1 &= 0x0FFF;

	switch (port)
	{
	case AB_CURRENT:	//Reads V bat RTC
		ADC12CTL1 |= ADC12CSTARTADD_0;
		// Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT0));
		__no_operation();
		result = ADC12MEM0;
		break;

	case AB_RTC_VBACKUP:		//Reads current sensor
		ADC12CTL1 |= ADC12CSTARTADD_1;
		// Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT1));
		__no_operation();
		result = ADC12MEM1;
		break;

	case AB_H1_22:
		ADC12CTL1 |= ADC12CSTARTADD_2;
		// Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT2));
		__no_operation();
		result = ADC12MEM2;
		break;

	case AB_H1_33:
		ADC12CTL1 |= ADC12CSTARTADD_3;
		// Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT3));
		__no_operation();
		result = ADC12MEM3;
		break;
	case AB_H1_20:
		ADC12CTL1 |= ADC12CSTARTADD_4;
		// Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT4));
		__no_operation();
		result = ADC12MEM4;
		break;

	case AB_H1_31:
		ADC12CTL1 |= ADC12CSTARTADD_5;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT5));
		__no_operation();
		result = ADC12MEM5;
		break;

	case AB_H1_18:
		ADC12CTL1 |= ADC12CSTARTADD_6;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT6));
		__no_operation();
		result = ADC12MEM6;
		break;

	case AB_H1_29:
		ADC12CTL1 |= ADC12CSTARTADD_7;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BIT7));
		__no_operation();
		result = ADC12MEM7;
		break;

	case AB_H1_16:	//ADC12
		ADC12CTL1 |= ADC12CSTARTADD_12;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BITC));
		__no_operation();
		result = ADC12MEM12;
		break;

	case AB_H1_27:
		ADC12CTL1 |= ADC12CSTARTADD_13;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BITD));
		__no_operation();
		result = ADC12MEM13;
		break;

	case AB_H1_14:
		ADC12CTL1 |= ADC12CSTARTADD_14;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BITE));
		__no_operation();
		result = ADC12MEM14;
		break;

	case AB_H1_25:
		ADC12CTL1 |= ADC12CSTARTADD_15;
		//Enable and Start conversion
		ADC12CTL0 |= ADC12ENC + ADC12SC;
		while (!(ADC12IFG & BITF));
		__no_operation();
		result = ADC12MEM15;
		break;

	default:
		result = 0;
		break;
	}
	return result;
}

/*
 * Returns the value of the mA consumed by the ABACUS board
 */
/*float abacus_currentReadFloat()
{
	//Read RAW from ADC12:
	int32_t rawMeasurement = 0;
	uint8_t i;
	for(i = 0; i < 10; i++)
		rawMeasurement += abacus_gpio_analogRead(AB_CURRENT);
	rawMeasurement = rawMeasurement / 10;
	//We have a x2 0.4ohm resistor in parallell(total 0.2) and amplifies
	//20 times.
	//We have a 12bit resolution at ADC12, so from 0 to 4095 each
	//bit is 3.3V/4096 = 0.000806V
	return ((((2.5 * (float)rawMeasurement/4095.0)) / 0.2) / 20.0) * 1000.0;
}*/

/*
 * Returns the value of the mA consumed by the ABACUS board
 */
uint16_t abacus_currentRead()
{
	//Read RAW from ADC12:
	int32_t rawMeasurement = 0;
	uint8_t i;
	for(i = 0; i < 10; i++)
		rawMeasurement += abacus_gpio_analogRead(AB_CURRENT);
	rawMeasurement = rawMeasurement / 10;
	//We have a x2 0.4ohm resistor in parallell(total 0.2) and amplifies
	//20 times.
	//We have a 12bit resolution at ADC12, so from 0 to 4095 each
	//bit is 3.3V/4096 = 0.000806V

	//return ((((2.5 * (float)rawMeasurement/4095.0)) / 0.2) / 20.0) * 1000.0;
	return (rawMeasurement * 1000)/6552;
}
