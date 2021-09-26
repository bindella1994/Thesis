#include "abacus_gpio.h"

struct GPIOdigital abacus_gpio;

int abacus_gpio_init()
{
	// LED init
	// P11.0 set as output (LED)
	P11DIR |= BIT0;
	AB_LED_ON;

	/* Initialize REF module */
	// Enable 2.5V shared reference, disable temperature sensor to save power
	REFCTL0 |= REFMSTR + REFVSEL_2 + REFON + REFTCOFF;

	///////////////////////////////////////////////////////////////////////////
	//Set all the pinout for FPGA

	// P2.5 for FPGA POWER ENABLE (active low); default= 0 =enabled
	P2DIR |= BIT5;
	//FPGA remains off
	P2OUT |= BIT5;

	// P10.3 for FPGA REPROGRAM pin, set as output and keep it low
	P10SEL &= ~BIT3;
	P10DIR |= BIT3;
	P10OUT &= ~BIT3;

	//The control bus:
	//As GPIO
	P1SEL = 0x00;
	//Disable Pullups/downs
	P1REN = 0x00;
	//0010 1101 BIT0, 2, 3, 5 as output
	P1DIR  =  (BIT0 | BIT2 | BIT3 | BIT5);
	//Put the as low
	P1OUT &= ~(BIT0 | BIT2 | BIT3 | BIT5);
	//Enable pulldown on bit 4
	P1REN |= BIT4;
	P1OUT &= ~BIT4;

	//Now the FPGA output/input 16bits as inputs and disable pullups/downs
	P4SEL = 0x00;
	P4DIR = 0x00;
	P4REN = 0x00;

	P8SEL = 0x00;
	P8DIR = 0x00;
	//P8REN = 0x00;
	P8REN = 0xFF;
	P8OUT = 0x00;

	// End of configuration for the FPGA
	///////////////////////////////////////////////////////////////////////////

	P2SEL &= ~(BIT1|BIT2|BIT3|BIT4); // P2.1 .2 .3 .4 as GPIO
	P2DIR &= ~(BIT1|BIT2|BIT3|BIT4); // P2.1 .2 .3 .4 as Input
	P2REN &= ~(BIT1|BIT2|BIT3|BIT4); // P2.1 .2 .3 .4 Pull up/down resistor off

	/* Port 6 Port Select Register */
	// ADC1 to ADC7 init
	P6SEL |= (BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);     // Disable all digital I/O on P6.1 to P6.7 **** MT ****


	P7SEL |= (BIT4|BIT5|BIT6|BIT7);	// Disable all digital I/O on P7.4 to P7.7 (NB: es: P7.6=A14=ADC10 on Abacus) **** MT ****

	// Turn on ADC12, Sampling time on Reference Generator and set to 2.5V
	ADC12CTL0 = ADC12ON + ADC12SHT02 + ADC12REFON + ADC12REF2_5V;
	//  ADC12CTL1 = ADC12SHP;                     // Use sampling timer

	ADC12CTL1 = ADC12SHP/*+ADC12CONSEQ_1*/;     // Use sampling timer  /*+ Sequence-of-channels*/

	//ADC per consumo corrente
	ADC12MCTL0 = ADC12INCH_1 + ADC12SREF_1;     //InputCH1, Vr+=Vref+ and Vr-=AVss
	//ADC per sensori di temperatura esterni
	ADC12MCTL2 = ADC12INCH_2 + ADC12SREF_1;     //InputCH2
	ADC12MCTL3 = ADC12INCH_3 + ADC12SREF_1;		//InputCH3
	ADC12MCTL4 = ADC12INCH_4 + ADC12SREF_1;		//InputCH4
	ADC12MCTL5 = ADC12INCH_5 + ADC12SREF_1;		//InputCH5
	ADC12MCTL6 = ADC12INCH_6 + ADC12SREF_1;		//InputCH6
	ADC12MCTL7 = ADC12INCH_7 + ADC12SREF_1;		//InputCH7
	ADC12MCTL12 = ADC12INCH_12 + ADC12SREF_1;	//InputCH12
	ADC12MCTL13 = ADC12INCH_13 + ADC12SREF_1;	//InputCH13
	ADC12MCTL14 = ADC12INCH_14 + ADC12SREF_1;	//InputCH14
	ADC12MCTL15 = ADC12INCH_15 + ADC12SREF_1;	//InputCH15

	__delay_cycles (1000);                     //wait for reference start up


	/* Port 10 Port Select Register */ //***MT***
	P10SEL &= ~(BIT6|BIT7);	// P10.3 .6 .7 as GPIO
	P10DIR |= BIT6|BIT7;		// P10.3 .6 .7 as output
	P10OUT &= ~BIT6;				// P10.6 = SPI_Switch to switch the SPI bus of the FPGA_Flash between FPGA and uC (default = 0 = FPGA)
	P10OUT &= ~BIT7;				// P10.7 = FPGA_FLASH_SS chip select of the FPGA_FLash by uC (default=0 = don't select)

	/* Port 11 Port Select Register */
	P11SEL &= ~(BIT0|BIT1|BIT2);	// Setto come GPIO

	abacus_gpio.interruptEnabled = 0;

	AB_LED_OFF;

	return 0;
}

/*
 * This function sets Low or high a bit in the GPIOs of the H1 and H2 ports
 * Options are:
 * AB_HIGH
 * AB_LOW
 */
uint8_t abacus_gpio_digitalWrite(uint8_t port, uint8_t signal)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalWrite(port, signal);

	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_3:	//P5.6
		if(signal == AB_LOW)
			P5OUT &= ~BIT6;
		else
			P5OUT |= BIT6;
		break;
	case AB_H1_4:	//P9.4
		if(signal == AB_LOW)
			P9OUT &= ~BIT4;
		else
			P9OUT |= BIT4;
		break;
	case AB_H1_5:	//P5.7
		if(signal == AB_LOW)
			P5OUT &= ~BIT7;
		else
			P5OUT |= BIT7;
		break;
	case AB_H1_6:	//P9.5
		if(signal == AB_LOW)
			P9OUT &= ~BIT5;
		else
			P9OUT |= BIT5;
		break;
	case AB_H1_8:	//P2.0
		if(signal == AB_LOW)
			P2OUT &= ~BIT0;
		else
			P2OUT |= BIT0;
		break;
	case AB_H1_10:	//P7.0
		if(signal == AB_LOW)
			P7OUT &= ~BIT0;
		else
			P7OUT |= BIT0;
		break;
	case AB_H1_12:	//P3.6
		if(signal == AB_LOW)
			P3OUT &= ~BIT6;
		else
			P3OUT |= BIT6;
		break;
	case AB_H1_13:	//P2.2
		if(signal == AB_LOW)
			P2OUT &= ~BIT2;
		else
			P2OUT |= BIT2;
		break;
	case AB_H1_14:	//P7.6
		if(signal == AB_LOW)
			P7OUT &= ~BIT6;
		else
			P7OUT |= BIT6;
		break;
	case AB_H1_15:	//P11.1
		if(signal == AB_LOW)
			P11OUT &= ~BIT1;
		else
			P11OUT |= BIT1;
		break;
	case AB_H1_16:	//P7.4
		if(signal == AB_LOW)
			P7OUT &= ~BIT4;
		else
			P7OUT |= BIT4;
		break;
	case AB_H1_18:	//P6.6
		if(signal == AB_LOW)
			P6OUT &= ~BIT6;
		else
			P6OUT |= BIT6;
		break;
	case AB_H1_19:	//P10.0
		if(signal == AB_LOW)
			P10OUT &= ~BIT0;
		else
			P10OUT |= BIT0;
		break;
	case AB_H1_20:	//P6.4
		if(signal == AB_LOW)
			P6OUT &= ~BIT4;
		else
			P6OUT |= BIT4;
		break;
	case AB_H1_22:	//P6.2
		if(signal == AB_LOW)
			P6OUT &= ~BIT2;
		else
			P6OUT |= BIT2;
		break;
	case AB_H1_25:	//P7.7
		if(signal == AB_LOW)
			P7OUT &= ~BIT7;
		else
			P7OUT |= BIT7;
		break;
	case AB_H1_27:	//P7.5
		if(signal == AB_LOW)
			P7OUT &= ~BIT5;
		else
			P7OUT |= BIT5;
		break;
	case AB_H1_28:	//P5.0
		if(signal == AB_LOW)
			P5OUT &= ~BIT0;
		else
			P5OUT |= BIT0;
		break;
	case AB_H1_29:	//P6.7
		if(signal == AB_LOW)
			P6OUT &= ~BIT7;
		else
			P6OUT |= BIT7;
		break;
	case AB_H1_30:	//P5.1
		if(signal == AB_LOW)
			P5OUT &= ~BIT1;
		else
			P5OUT |= BIT1;
		break;
	case AB_H1_31:	//P6.5
		if(signal == AB_LOW)
			P6OUT &= ~BIT5;
		else
			P6OUT |= BIT5;
		break;
	case AB_H1_33:	//P6.3
		if(signal == AB_LOW)
			P6OUT &= ~BIT3;
		else
			P6OUT |= BIT3;
		break;
	case AB_H1_34:	//P11.2
		if(signal == AB_LOW)
			P11OUT &= ~BIT2;
		else
			P11OUT |= BIT2;
		break;
	case AB_H1_35:	//P3.3
		if(signal == AB_LOW)
			P3OUT &= ~BIT3;
		else
			P3OUT |= BIT3;
		break;
	case AB_H1_36:	//P2.7
		if(signal == AB_LOW)
			P2OUT &= ~BIT7;
		else
			P2OUT |= BIT7;
		break;
	case AB_H1_37:	//P3.0
		if(signal == AB_LOW)
			P3OUT &= ~BIT0;
		else
			P3OUT |= BIT0;
		break;
	case AB_H1_38:	//P2.6
		if(signal == AB_LOW)
			P2OUT &= ~BIT6;
		else
			P2OUT |= BIT6;
		break;
	case AB_H1_39:	//P10.4
		if(signal == AB_LOW)
			P10OUT &= ~BIT4;
		else
			P10OUT |= BIT4;
		break;
	case AB_H1_40:	//P10.5
		if(signal == AB_LOW)
			P10OUT &= ~BIT5;
		else
			P10OUT |= BIT5;
		break;
	default:
		break;
	}
	return 0;
}

/*
 * This function reads the inputs of the H1 and H2 ports
 */
uint8_t abacus_gpio_digitalRead(uint8_t port)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalRead(port);

	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_3:	//P5.6
		if(P5IN & BIT6)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_4:	//P9.4
		if(P9IN & BIT4)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_5:	//P5.7
		if(P5IN & BIT7)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_6:	//P9.5
		if(P9IN & BIT5)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_8:	//P2.0
		if(P2IN & BIT0)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_10:	//P7.0
		if(P7IN & BIT0)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_12:	//P3.6
		if(P3IN & BIT6)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_13:	//P2.2
		if(P2IN & BIT2)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_14:	//P7.6
		if(P7IN & BIT6)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_15:	//P11.1
		if(P11IN & BIT1)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_16:	//P7.4
		if(P7IN & BIT4)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_18:	//P6.6
		if(P6IN & BIT6)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_19:	//P10.0
		if(P10IN & BIT0)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_20:	//P6.4
		if(P6IN & BIT4)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_22:	//P6.2
		if(P2IN & BIT2)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_25:	//P7.7
		if(P7IN & BIT7)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_27:	//P7.5
		if(P7IN & BIT5)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_28:	//P5.0
		if(P5IN & BIT0)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_29:	//P6.7
		if(P6IN & BIT7)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_30:	//P5.1
		if(P5IN & BIT1)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_31:	//P6.5
		if(P6IN & BIT5)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_33:	//P6.3
		if(P6IN & BIT3)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_34:	//P11.2
		if(P11IN & BIT2)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_35:	//P3.3
		if(P3IN & BIT3)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_36:	//P2.7
		if(P2IN & BIT7)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_37:	//P3.0
		if(P3IN & BIT0)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_38:	//P2.6
		if(P2IN & BIT6)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_39:	//P10.4
		if(P10IN & BIT4)
			return AB_LOW;
		else
			return AB_HIGH;
	case AB_H1_40:	//P10.5
		if(P10IN & BIT5)
			return AB_LOW;
		else
			return AB_HIGH;
	default:
		return AB_LOW;
	}
}

/*
 * This function sets the pins of H1 and H2 ports to output or input
 * Options are:
 * AB_INPUT
 * AB_OUTPUT
 */
uint8_t abacus_gpio_digitalMode(uint8_t port, uint8_t signal)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalMode(port, signal);

	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_3:	//P5.6
		if(signal == AB_INPUT)
		{
			P5SEL &= ~BIT6;
			P5DIR &= ~BIT6;
		}
		else
		{
			P5SEL &= ~BIT6;
			P5DIR |= BIT6;
		}
		break;
	case AB_H1_4:	//P9.4
		if(signal == AB_INPUT)
		{
			P9SEL &= ~BIT4;
			P9DIR &= ~BIT4;
		}
		else
		{
			P9SEL &= ~BIT4;
			P9DIR |= BIT4;
		}
		break;
	case AB_H1_5:	//P5.7
		if(signal == AB_INPUT)
		{
			P5SEL &= ~BIT7;
			P5DIR &= ~BIT7;
		}
		else
		{
			P5SEL &= ~BIT7;
			P5DIR |= BIT7;
		}
		break;
	case AB_H1_6:	//P9.5
		if(signal == AB_INPUT)
		{
			P9SEL &= ~BIT5;
			P9DIR &= ~BIT5;
		}
		else
		{
			P9SEL &= ~BIT5;
			P9DIR |= BIT5;
		}
		break;
	case AB_H1_8:	//P2.0
		if(signal == AB_INPUT)
		{
			P2SEL &= ~BIT0;
			P2DIR &= ~BIT0;
		}
		else
		{
			P2SEL &= ~BIT0;
			P2DIR |= BIT0;
		}
		break;
	case AB_H1_10:	//P7.0
		if(signal == AB_INPUT)
		{
			P7SEL &= ~BIT0;
			P7DIR &= ~BIT0;
		}
		else
		{
			P7SEL &= ~BIT0;
			P7DIR |= BIT0;
		}
		break;
	case AB_H1_12:	//P3.6
		if(signal == AB_INPUT)
		{
			P3SEL &= ~BIT6;
			P3DIR &= ~BIT6;
		}
		else
		{
			P3SEL &= ~BIT6;
			P3DIR |= BIT6;
		}
		break;
	case AB_H1_13:	//P2.2
		if(signal == AB_INPUT)
		{
			P2SEL &= ~BIT2;
			P2DIR &= ~BIT2;
		}
		else
		{
			P2SEL &= ~BIT2;
			P2DIR |= BIT2;
		}
		break;
	case AB_H1_14:	//P7.6
		if(signal == AB_INPUT)
		{
			P7SEL &= ~BIT6;
			P7DIR &= ~BIT6;
		}
		else
		{
			P7SEL &= ~BIT6;
			P7DIR |= BIT6;
		}
		break;
	case AB_H1_15:	//P11.1
		if(signal == AB_INPUT)
		{
			P11SEL &= ~BIT1;
			P11DIR &= ~BIT1;
		}
		else
		{
			P11SEL &= ~BIT1;
			P11DIR |= BIT1;
		}
		break;
	case AB_H1_16:	//P7.4
		if(signal == AB_INPUT)
		{
			P7SEL &= ~BIT4;
			P7DIR &= ~BIT4;
		}
		else
		{
			P7SEL &= ~BIT4;
			P7DIR |= BIT4;
		}
		break;
	case AB_H1_18:	//P6.6
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT6;
			P6DIR &= ~BIT6;
		}
		else
		{
			P6SEL &= ~BIT6;
			P6DIR |= BIT6;
		}
		break;
	case AB_H1_19:	//P10.0
		if(signal == AB_INPUT)
		{
			P10SEL &= ~BIT0;
			P10DIR &= ~BIT0;
		}
		else
		{
			P10SEL &= ~BIT0;
			P10DIR |= BIT0;
		}
		break;
	case AB_H1_20:	//P6.4
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT4;
			P6DIR &= ~BIT4;
		}
		else
		{
			P6SEL &= ~BIT4;
			P6DIR |= BIT4;
		}
		break;
	case AB_H1_22:	//P6.2
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT2;
			P6DIR &= ~BIT2;
		}
		else
		{
			P6SEL &= ~BIT2;
			P6DIR |= BIT2;
		}
		break;
	case AB_H1_25:	//P7.7
		if(signal == AB_INPUT)
		{
			P7SEL &= ~BIT7;
			P7DIR &= ~BIT7;
		}
		else
		{
			P7SEL &= ~BIT7;
			P7DIR |= BIT7;
		}
		break;
	case AB_H1_27:	//P7.5
		if(signal == AB_INPUT)
		{
			P7SEL &= ~BIT5;
			P7DIR &= ~BIT5;
		}
		else
		{
			P7SEL &= ~BIT5;
			P7DIR |= BIT5;
		}
		break;
	case AB_H1_28:	//P5.0
		if(signal == AB_INPUT)
		{
			P5SEL &= ~BIT0;
			P5DIR &= ~BIT0;
		}
		else
		{
			P5SEL &= ~BIT0;
			P5DIR |= BIT0;
		}
		break;
	case AB_H1_29:	//P6.7
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT7;
			P6DIR &= ~BIT7;
		}
		else
		{
			P6SEL &= ~BIT7;
			P6DIR |= BIT7;
		}
		break;
	case AB_H1_30:	//P5.1
		if(signal == AB_INPUT)
		{
			P5SEL &= ~BIT1;
			P5DIR &= ~BIT1;
		}
		else
		{
			P5SEL &= ~BIT1;
			P5DIR |= BIT1;
		}
		break;
	case AB_H1_31:	//P6.5
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT5;
			P6DIR &= ~BIT5;
		}
		else
		{
			P6SEL &= ~BIT5;
			P6DIR |= BIT5;
		}
		break;
	case AB_H1_33:	//P6.3
		if(signal == AB_INPUT)
		{
			P6SEL &= ~BIT3;
			P6DIR &= ~BIT3;
		}
		else
		{
			P6SEL &= ~BIT3;
			P6DIR |= BIT3;
		}
		break;
	case AB_H1_34:	//P11.2
		if(signal == AB_INPUT)
		{
			P11SEL &= ~BIT2;
			P11DIR &= ~BIT2;
		}
		else
		{
			P11SEL &= ~BIT2;
			P11DIR |= BIT2;
		}
		break;
	case AB_H1_35:	//P3.3
		if(signal == AB_INPUT)
		{
			P3SEL &= ~BIT3;
			P3DIR &= ~BIT3;
		}
		else
		{
			P3SEL &= ~BIT3;
			P3DIR |= BIT3;
		}
		break;
	case AB_H1_36:	//P2.7
		if(signal == AB_INPUT)
		{
			P2SEL &= ~BIT7;
			P2DIR &= ~BIT7;
		}
		else
		{
			P2SEL &= ~BIT7;
			P2DIR |= BIT7;
		}
		break;
	case AB_H1_37:	//P3.0
		if(signal == AB_INPUT)
		{
			P3SEL &= ~BIT0;
			P3DIR &= ~BIT0;
		}
		else
		{
			P3SEL &= ~BIT0;
			P3DIR |= BIT0;
		}
		break;
	case AB_H1_38:	//P2.6
		if(signal == AB_INPUT)
		{
			P2SEL &= ~BIT6;
			P2DIR &= ~BIT6;
		}
		else
		{
			P2SEL &= ~BIT6;
			P2DIR |= BIT6;
		}
		break;
	case AB_H1_39:	//P10.4
		if(signal == AB_INPUT)
		{
			P10SEL &= ~BIT4;
			P10DIR &= ~BIT4;
		}
		else
		{
			P10SEL &= ~BIT4;
			P10DIR |= BIT4;
		}
		break;
	case AB_H1_40:	//P10.5
		if(signal == AB_INPUT)
		{
			P10SEL &= ~BIT5;
			P10DIR &= ~BIT5;
		}
		else
		{
			P10SEL &= ~BIT5;
			P10DIR |= BIT5;
		}
		break;
	default:
		break;
	}
	return 0;
}

/*
 * This function sets the pullup and downs of the H1 and H2. Options are:
 *  AB_PULLDISABLE
 *  AB_PULLDOWN
 *  AB_PULLUP
 */
uint8_t abacus_gpio_digitalPullupdown(uint8_t port, uint8_t conf)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalPullupdown(port, conf);


	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_3:	//P5.6
		if(conf == AB_PULLUP)
		{
			P5REN |= BIT6;
			P5OUT |= BIT6;
		}
		else if(conf == AB_PULLDOWN)
		{
			P5REN |= BIT6;
			P5OUT &= ~BIT6;
		}
		else
		{
			P5REN &= ~BIT6;
		}
		break;
	case AB_H1_4:	//P9.4
		if(conf == AB_PULLUP)
		{
			P9REN |= BIT4;
			P9OUT |= BIT4;
		}
		else if(conf == AB_PULLDOWN)
		{
			P9REN |= BIT4;
			P9OUT &= ~BIT4;
		}
		else
		{
			P9REN &= ~BIT4;
		}
		break;
	case AB_H1_5:	//P5.7
		if(conf == AB_PULLUP)
		{
			P5REN |= BIT7;
			P5OUT |= BIT7;
		}
		else if(conf == AB_PULLDOWN)
		{
			P5REN |= BIT7;
			P5OUT &= ~BIT7;
		}
		else
		{
			P5REN &= ~BIT7;
		}
		break;
	case AB_H1_6:	//P9.5
		if(conf == AB_PULLUP)
		{
			P9REN |= BIT5;
			P9OUT |= BIT5;
		}
		else if(conf == AB_PULLDOWN)
		{
			P9REN |= BIT5;
			P9OUT &= ~BIT5;
		}
		else
		{
			P9REN &= ~BIT5;
		}
		break;
	case AB_H1_8:	//P2.0
		if(conf == AB_PULLUP)
		{
			P2REN |= BIT0;
			P2OUT |= BIT0;
		}
		else if(conf == AB_PULLDOWN)
		{
			P2REN |= BIT0;
			P2OUT &= ~BIT0;
		}
		else
		{
			P2REN &= ~BIT0;
		}
		break;
	case AB_H1_10:	//P7.0
		if(conf == AB_PULLUP)
		{
			P7REN |= BIT0;
			P7OUT |= BIT0;
		}
		else if(conf == AB_PULLDOWN)
		{
			P7REN |= BIT0;
			P7OUT &= ~BIT0;
		}
		else
		{
			P7REN &= ~BIT0;
		}
		break;
	case AB_H1_12:	//P3.6
		if(conf == AB_PULLUP)
		{
			P3REN |= BIT6;
			P3OUT |= BIT6;
		}
		else if(conf == AB_PULLDOWN)
		{
			P3REN |= BIT6;
			P3OUT &= ~BIT6;
		}
		else
		{
			P3REN &= ~BIT6;
		}
		break;
	case AB_H1_13:	//P2.2
		if(conf == AB_PULLUP)
		{
			P2REN |= BIT2;
			P2OUT |= BIT2;
		}
		else if(conf == AB_PULLDOWN)
		{
			P2REN |= BIT2;
			P2OUT &= ~BIT2;
		}
		else
		{
			P2REN &= ~BIT2;
		}
		break;
	case AB_H1_14:	//P7.6
		if(conf == AB_PULLUP)
		{
			P7REN |= BIT6;
			P7OUT |= BIT6;
		}
		else if(conf == AB_PULLDOWN)
		{
			P7REN |= BIT6;
			P7OUT &= ~BIT6;
		}
		else
		{
			P7REN &= ~BIT6;
		}
		break;
	case AB_H1_15:	//P11.1
		if(conf == AB_PULLUP)
		{
			P11REN |= BIT1;
			P11OUT |= BIT1;
		}
		else if(conf == AB_PULLDOWN)
		{
			P11REN |= BIT1;
			P11OUT &= ~BIT1;
		}
		else
		{
			P11REN &= ~BIT1;
		}
		break;
	case AB_H1_16:	//P7.4
		if(conf == AB_PULLUP)
		{
			P7REN |= BIT4;
			P7OUT |= BIT4;
		}
		else if(conf == AB_PULLDOWN)
		{
			P7REN |= BIT4;
			P7OUT &= ~BIT4;
		}
		else
		{
			P7REN &= ~BIT4;
		}
		break;
	case AB_H1_18:	//P6.6
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT6;
			P6OUT |= BIT6;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT6;
			P6OUT &= ~BIT6;
		}
		else
		{
			P6REN &= ~BIT6;
		}
		break;
	case AB_H1_19:	//P10.0
		if(conf == AB_PULLUP)
		{
			P10REN |= BIT0;
			P10OUT |= BIT0;
		}
		else if(conf == AB_PULLDOWN)
		{
			P10REN |= BIT0;
			P10OUT &= ~BIT0;
		}
		else
		{
			P10REN &= ~BIT0;
		}
		break;
	case AB_H1_20:	//P6.4
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT4;
			P6OUT |= BIT4;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT4;
			P6OUT &= ~BIT4;
		}
		else
		{
			P6REN &= ~BIT4;
		}
		break;
	case AB_H1_22:	//P6.2
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT2;
			P6OUT |= BIT2;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT2;
			P6OUT &= ~BIT2;
		}
		else
		{
			P6REN &= ~BIT2;
		}
		break;
	case AB_H1_25:	//P7.7
		if(conf == AB_PULLUP)
		{
			P7REN |= BIT7;
			P7OUT |= BIT7;
		}
		else if(conf == AB_PULLDOWN)
		{
			P7REN |= BIT7;
			P7OUT &= ~BIT7;
		}
		else
		{
			P7REN &= ~BIT7;
		}
		break;
	case AB_H1_27:	//P7.5
		if(conf == AB_PULLUP)
		{
			P7REN |= BIT5;
			P7OUT |= BIT5;
		}
		else if(conf == AB_PULLDOWN)
		{
			P7REN |= BIT5;
			P7OUT &= ~BIT5;
		}
		else
		{
			P7REN &= ~BIT5;
		}
		break;
	case AB_H1_28:	//P5.0
		if(conf == AB_PULLUP)
		{
			P5REN |= BIT0;
			P5OUT |= BIT0;
		}
		else if(conf == AB_PULLDOWN)
		{
			P5REN |= BIT0;
			P5OUT &= ~BIT0;
		}
		else
		{
			P5REN &= ~BIT0;
		}
		break;
	case AB_H1_29:	//P6.7
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT7;
			P6OUT |= BIT7;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT7;
			P6OUT &= ~BIT7;
		}
		else
		{
			P6REN &= ~BIT7;
		}
		break;
	case AB_H1_30:	//P5.1
		if(conf == AB_PULLUP)
		{
			P5REN |= BIT1;
			P5OUT |= BIT1;
		}
		else if(conf == AB_PULLDOWN)
		{
			P5REN |= BIT1;
			P5OUT &= ~BIT1;
		}
		else
		{
			P5REN &= ~BIT1;
		}
		break;
	case AB_H1_31:	//P6.5
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT5;
			P6OUT |= BIT5;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT5;
			P6OUT &= ~BIT5;
		}
		else
		{
			P6REN &= ~BIT5;
		}
		break;
	case AB_H1_33:	//P6.3
		if(conf == AB_PULLUP)
		{
			P6REN |= BIT3;
			P6OUT |= BIT3;
		}
		else if(conf == AB_PULLDOWN)
		{
			P6REN |= BIT3;
			P6OUT &= ~BIT3;
		}
		else
		{
			P6REN &= ~BIT3;
		}
		break;
	case AB_H1_34:	//P11.2
		if(conf == AB_PULLUP)
		{
			P11REN |= BIT2;
			P11OUT |= BIT2;
		}
		else if(conf == AB_PULLDOWN)
		{
			P11REN |= BIT2;
			P11OUT &= ~BIT2;
		}
		else
		{
			P11REN &= ~BIT2;
		}
		break;
	case AB_H1_35:	//P3.3
		if(conf == AB_PULLUP)
		{
			P3REN |= BIT3;
			P3OUT |= BIT3;
		}
		else if(conf == AB_PULLDOWN)
		{
			P3REN |= BIT3;
			P3OUT &= ~BIT3;
		}
		else
		{
			P3REN &= ~BIT3;
		}
		break;
	case AB_H1_36:	//P2.7
		if(conf == AB_PULLUP)
		{
			P2REN |= BIT7;
			P2OUT |= BIT7;
		}
		else if(conf == AB_PULLDOWN)
		{
			P2REN |= BIT7;
			P2OUT &= ~BIT7;
		}
		else
		{
			P2REN &= ~BIT7;
		}
		break;
	case AB_H1_37:	//P3.0
		if(conf == AB_PULLUP)
		{
			P3REN |= BIT0;
			P3OUT |= BIT0;
		}
		else if(conf == AB_PULLDOWN)
		{
			P3REN |= BIT0;
			P3OUT &= ~BIT0;
		}
		else
		{
			P3REN &= ~BIT0;
		}
		break;
	case AB_H1_38:	//P2.6
		if(conf == AB_PULLUP)
		{
			P2REN |= BIT6;
			P2OUT |= BIT6;
		}
		else if(conf == AB_PULLDOWN)
		{
			P2REN |= BIT6;
			P2OUT &= ~BIT6;
		}
		else
		{
			P2REN &= ~BIT6;
		}
		break;
	case AB_H1_39:	//P10.4
		if(conf == AB_PULLUP)
		{
			P10REN |= BIT4;
			P10OUT |= BIT4;
		}
		else if(conf == AB_PULLDOWN)
		{
			P10REN |= BIT4;
			P10OUT &= ~BIT4;
		}
		else
		{
			P10REN &= ~BIT4;
		}
		break;
	case AB_H1_40:	//P10.5
		if(conf == AB_PULLUP)
		{
			P10REN |= BIT5;
			P10OUT |= BIT5;
		}
		else if(conf == AB_PULLDOWN)
		{
			P10REN |= BIT5;
			P10OUT &= ~BIT5;
		}
		else
		{
			P10REN &= ~BIT5;
		}
		break;
	default:
		break;
	}
	return 0;
}

/*
 * It enables the interrupt of the selected port. Please be aware you can only
 * use one function for H2 and one function for H1. H2 all pins have interrupts
 * but H1 only ports H1.8, H1.13, H1.36 & H1.38 have interrupts. On H2 all
 * transitions are interrupt trigger for H1:
 * - AB_LOW2HIGH for Low to High only on port H1
 * - AB_HIGH2LOW for High to Low only on port H1
 * Returns -1 if port was not interruptible
 */
int8_t abacus_gpio_digitalEnableInterrupt(uint8_t port, void (*interruptFunction)(int*), uint8_t direction)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalEnableInterrupt(port, interruptFunction);

	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_8:	//P2.0
		if(direction == AB_LOW2HIGH)
			P2IES &= ~BIT0;
		else
			P2IES |= BIT0;
		P2IE  |= BIT0;
		//Clear interrupt
		P2IFG &= ~BIT0;
		abacus_gpio.triggerFunction = interruptFunction;
		abacus_gpio.interruptEnabled = 1;
		break;
	case AB_H1_13:	//P2.2
		if(direction == AB_LOW2HIGH)
			P2IES &= ~BIT2;
		else
			P2IES |= BIT2;
		P2IE  |= BIT2;
		//Clear interrupt
		P2IFG &= ~BIT2;
		abacus_gpio.triggerFunction = interruptFunction;
		abacus_gpio.interruptEnabled = 1;
		break;
	case AB_H1_36:	//P2.7
		if(direction == AB_LOW2HIGH)
			P2IES &= ~BIT7;
		else
			P2IES |= BIT7;
		P2IE  |= BIT7;
		//Clear interrupt
		P2IFG &= ~BIT7;
		abacus_gpio.triggerFunction = interruptFunction;
		abacus_gpio.interruptEnabled = 1;
		break;
	case AB_H1_38:	//P2.6
		if(direction == AB_LOW2HIGH)
			P2IES &= ~BIT6;
		else
			P2IES |= BIT6;
		P2IE  |= BIT6;
		//Clear interrupt
		P2IFG &= ~BIT6;
		abacus_gpio.triggerFunction = interruptFunction;
		abacus_gpio.interruptEnabled = 1;
		break;
	default:
		return -1;	//Return with error
	}
	return 0;
}

/*
 * It disables the interrupts
 */
int8_t abacus_gpio_digitalDisableInterrupt(uint8_t port)
{
	if(port >= AB_H2_2 && port <= AB_H2_15)
		return abacus_gpio_GPIOExpDigitalDisableInterrupt(port);

	//Ok, it is an internal GPIO of the MCU
	switch(port)
	{
	case AB_H1_8:	//P2.0
		P2IE &= ~BIT0;
		break;
	case AB_H1_13:	//P2.2
		P2IE &= ~BIT2;
		break;
	case AB_H1_36:	//P2.7
		P2IE &= ~BIT7;
		break;
	case AB_H1_38:	//P2.6
		P2IE &= ~BIT6;
		break;
	default:
		return -1;	//Return with error
	}

	if(!(P2IE & 0xC5))	//All interrupts disabled? then set to zero
		abacus_gpio.interruptEnabled = 0;

	return 0;
}

/*
 * It returns the name of the last port that generated an interrupt on H1
 */
uint8_t abacus_gpio_digitalGetLastInterruptPort()
{
	return abacus_gpio.interruptPort;
}
