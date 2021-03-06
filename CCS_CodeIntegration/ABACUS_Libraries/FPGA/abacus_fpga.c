/*
 * abacus_fpga.c
 *
 */
 
#include "abacus_fpga.h"

void (*triggerFPGAFunction)(int*);
int8_t fpgaInterruptEnabled;

/*
 * It initiates all the pinouts and switches off the FPGA
 */
void abacus_fpga_init()
{
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

	//0010 1101 BIT0, 2, 3, 5 as output
	P1DIR  =  (BIT0 | BIT2 | BIT3 | BIT5);
	//Put the as low
	P1OUT &= ~(BIT0 | BIT2 | BIT3 | BIT5);

	//Enable pulldown on bit 4
	P1REN |= BIT4;
	P1OUT &= ~BIT4;

	//P1.6 interrupts are only enabled by its special function,
	//so nothing donw here

	//Now the FPGA output/input 16bits as inputs and disable pullups/downs
	P4SEL = 0x00;
	P4DIR = 0x00;
	P4REN = 0x00;

	P8SEL = 0x00;
	P8DIR = 0x00;
	//P8REN = 0x00;
	P8REN = 0xFF;
	P8OUT = 0x00;

	//Power off FPGA:
	//abacus_fpga_switchOff();

	//Disable interrupts
	abacus_fpga_interruptDisable();
}

/*
 * It returns 0 if FPGA is off, 1 if is on
 */
int8_t abacus_fpga_isOn()
{
	return !(P2IN & BIT5);
}

/*
 * It switches on the power of the FPGA
 */
int8_t abacus_fpga_switchOn()
{
	FPGA_ON;
	return 0;
}

/*
 * It switches off the power of the FPGA
 */
int8_t abacus_fpga_switchOff()
{
	FPGA_OFF;
	return 0;
}

/*
 * It is a reset implemented in the FPGA. Called Finite State machine reset
 */
int8_t abacus_fpga_FSM_reset()
{
	FPGA_ResetFst_Hi;
	_delay_cycles (20);
	FPGA_ResetFst_Low;
	return 0;
}


/*
 * It resets FPGA switching it off, then On, then reset program
 */
int8_t abacus_fpga_cold_reset()
{
	FPGA_OFF;
	_delay_cycles (20);
	FPGA_ON;

	abacus_fpga_resetProgram();
	return 0;
}

/*
 * It forces the FPGA to reboot and reprogram itself using the
 * progB pin from FPGA
 */
int8_t abacus_fpga_resetProgram()
{
	P10OUT |= BIT3;
	P10OUT &= ~BIT3;
	return 0;
}

/*
 *
 */
int8_t abacus_fpga_prgB_holdDown()
{
	P10OUT &= ~BIT3;
	return 0;
}

/*
 *
 */
int8_t abacus_fpga_prgB_holdUp()
{
	P10OUT |= BIT3;
	return 0;
}

/*
 * It returns 0 if FPGA interrupt is off, 1 if is on
 */
int8_t abacus_fpga_interruptisOn()
{
	return (P1IE & BIT1);
}
/*
 * It enables the interrupt on port P1.6 and adds the function to be called
 * during the interrupt
 */
int8_t abacus_fpga_interruptEnable(void (*interruptFunction)(int*))
{
	triggerFPGAFunction = interruptFunction;
	fpgaInterruptEnabled = 1;
	//Enable interrupts on port

	//Low to High edge
	P1IES &= ~BIT1;
	//Clear previous interrupts
	P1IFG &= ~BIT1;
	//Enable interrupt
	P1IE |= BIT1;
	return 0;
}

int8_t abacus_fpga_interruptDisable()
{
	fpgaInterruptEnabled = 0;
	//Disable interrupt
	P1IE &= ~BIT1;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * It returns 0 if the FPGA is not using the I2C
 */
int8_t abacus_fpga_i2cGetStatus()
{
	//Busy only if it is on
	if(abacus_fpga_isOn())
		return FPGA_I2CBus_Busy;
	//It is off, so return 0
	return 0;
}

/*
 * It reserves the I2C so FPGA will not use it
 */
int8_t abacus_fpga_i2cReserve()
{
	//Check if is busy, return error if so
	if(FPGA_I2CBus_Busy)
		return -1;

	//Reserve bus
	FPGA_Reserve_I2CBus;

	//FPGA is much more faster so recheck and release if so
	if(FPGA_I2CBus_Busy)
	{
		FPGA_Release_I2CBus;
		return -1;
	}

	return 0;
}

/*
 * Releases the semaphore for the I2C
 */
int8_t abacus_fpga_i2cRelease()
{
	//Release I2C
	FPGA_Release_I2CBus;
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
int8_t abacus_fpga_dataBusReserve()
{
	//Check if is busy, return error if so
	if(FPGA_Data_Ready)
		return -1;

	//Reserve bus
	FPGA_Reserve_DataBus;

	//FPGA is much more faster so recheck and release if so
	if(FPGA_Data_Ready)
	{
		FPGA_Release_DataBus;
		return -1;
	}

	//Set low impedance
	abacus_fpga_dataBusSetLowZ();
	return 0;
}

/*
 *
 */
int8_t abacus_fpga_dataBusRelease()
{
	//Set high impedance
	abacus_fpga_dataBusSetHighZ();

	FPGA_Release_DataBus;
	return 0;
}


int8_t abacus_fpga_dataBusSetHighZ()
{
	FPGA_LOWBYTE_asInput;
	FPGA_HIGHBYTE_asInput;
	return 0;
}

int8_t abacus_fpga_dataBusSetLowZ()
{
	FPGA_LOWBYTE_asOutput;
	FPGA_HIGHBYTE_asOutput;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/*
 *
 */
int8_t abacus_fpga_dataBusSendData(uint16_t data)
{
	FPGA_LOWBYTE_OUT  = 0xFF & data;
	FPGA_HIGHBYTE_OUT = 0xFF & ((0xFF00 & data) >> 8);
	__delay_cycles (1);
	//Tick
	FPGA_CkSyncToFPGA_High;
	_delay_cycles (1);
	//Tock
	FPGA_CkSyncToFPGA_Low;
	return 0;
}

/*
 * Send data array
 */
int8_t abacus_fpga_dataBusSendArray(uint16_t *buffer, uint16_t lenght)
{
	while(abacus_fpga_dataBusReserve() != 0);

	while(lenght)
	{
		abacus_fpga_dataBusSendData(*buffer);
		buffer++;
		lenght--;
	}

	abacus_fpga_dataBusRelease();
	return 0;
}

/*
 * Send data array uint8
 */
int8_t abacus_fpga_dataBusSendArray_8bit(uint8_t *buffer, uint16_t length)
{

	while(abacus_fpga_dataBusReserve() != 0);

	while(length)
	{
		FPGA_HIGHBYTE_OUT  = *buffer;
		buffer++;
		length--;
		if (length==0)
		{	
			// there was an odd number of bytes in the buffer
			FPGA_LOWBYTE_OUT = 0;
		} else
		{
			FPGA_LOWBYTE_OUT = *buffer;
			buffer++;
			length--;
		}
		__delay_cycles (1);
		//Tick
		FPGA_CkSyncToFPGA_High;
		_delay_cycles (1);
		//Tock
		FPGA_CkSyncToFPGA_Low;
	}

	abacus_fpga_dataBusRelease();
	return 0;
}




/*
 *
 */
int8_t abacus_fpga_dataBusGetData(uint16_t *buffer)
{
	//Tick
	FPGA_CkSyncToFPGA_High;
	_delay_cycles (2);

	*buffer = (FPGA_HIGHBYTE_IN << 8) + FPGA_LOWBYTE_IN;

	//Tock
	FPGA_CkSyncToFPGA_Low;
	return 0;
}

/*
 * get data array
 */
int8_t abacus_fpga_dataBusGetArray(uint16_t *buffer, uint16_t lenght)
{
	while(!FPGA_Data_Ready);

	while(lenght)
	{
		abacus_fpga_dataBusGetData(buffer);
		buffer++;
		lenght--;
	}

	return 0;
}


/*
 * get data array 8bit
 */
int8_t abacus_fpga_dataBusGetArray_8bit(uint8_t *buffer, uint16_t lenght)
{
	
	while(!FPGA_Data_Ready);

	while(lenght)
	{
		//Tick
		FPGA_CkSyncToFPGA_High;
		_delay_cycles (2);

		*buffer = FPGA_HIGHBYTE_IN;
		buffer++;
		lenght--;

		*buffer = FPGA_LOWBYTE_IN;
		buffer++;
		lenght--;
		//Tock
		FPGA_CkSyncToFPGA_Low;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	//Clear interrupt
	P1IFG &= ~BIT1;
	int wakeupOnExit = 0;
	//Call the associated function
	if(fpgaInterruptEnabled == 1)
		(*(triggerFPGAFunction))(&wakeupOnExit);

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
