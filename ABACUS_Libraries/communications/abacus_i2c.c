/*
 * abacus_i2c.c
 *
 */

 #include "abacus_i2c.h"

struct I2cPort i2c00;
struct I2cPort i2c01;

/*
 * It initializates the 2 I2C buses used in ABACUS:
 * - AB_I2C_BUS00: is UCB0 and it is connected to the sensors and has
 *                 optional external to 3.3V or 5V
 * - AB_I2C_BUS01: is UCB1 and it is connected to the GPIO expander and
 * 				   has the option to external bus in 3.3V
 */
void abacus_i2c_init()
{
	//Start with BUS00

	i2c00.i2c_name = AB_I2C_BUS00;
	i2c00.isOnError = 0;
	i2c00.inRepeatedStartCondition = 0;
	i2c00.isSlave = 0;
	//Assign I2C pins to USCI_B0
	P3SEL |= 0x06;
	//Enable SW reset
	UCB0CTL1 |= UCSWRST;
	//I2C Master, synchronous mode
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
	//Use SMCLK, keep SW reset
	UCB0CTL1 = UCSSEL_2 + UCSWRST;
	//fSCL = SMCLK/64
	UCB0BR0 = 64;
	UCB0BR1 = 0;

	//Clear SW reset, resume operation
	UCB0CTL1 &= ~UCSWRST;
	//Without TX interrupt
	//UCB0IE |= UCTXIE;
	//Without RX interrupt
	//UCB0IE |= UCRXIE;

	//Now lets go to BUS01

	i2c01.i2c_name = AB_I2C_BUS01;
	i2c01.isOnError = 0;
	i2c01.inRepeatedStartCondition = 0;
	i2c01.isSlave = 0;

	//Assign I2C pins to USCI_B3
	P10SEL |= 0x06;
	//Enable SW reset
	UCB3CTL1 |= UCSWRST;
	//I2C Master, synchronous mode
	UCB3CTL0 = UCMST + UCMODE_3 + UCSYNC;
	//Use SMCLK, keep SW reset
	UCB3CTL1 = UCSSEL_2 + UCSWRST;
	//fSCL = SMCLK/64
	UCB3BR0 = 64;
	UCB3BR1 = 0;

	//Clear SW reset, resume operation
	UCB3CTL1 &= ~UCSWRST;
	//Without TX interrupt
	//UCB3IE |= UCTXIE;
	//Without RX interrupt
	//UCB3IE |= UCRXIE;


}

/**
 * It sets the speed of the i2c bus. Speed will be Fmcu/(inputValue). So the
 * higher the input value the slower it will be.
 * Maximum speed will be of Fmcu/4. (4 lowbyte, 0 highbyte)
 * ABACUS default speed is Fmcu/64. (64 lowbyte, 0 highbyte)
 * GOMMSpace EPS requires lower speeds, like Fmcu/256. (0 lowbyte, 1 highbyte)
 * of
 */
void abacus_i2c_setSpeed(uint8_t busSelect,
		uint8_t highbyte,
		uint8_t lowbyte)
{
	if(busSelect == AB_I2C_BUS00)
	{
		//AB_I2C_BUS00
		//Enable SW reset
		UCB0CTL1 |= UCSWRST;

		//fSCL = SMCLK/12
		UCB0BR0 = lowbyte;
		UCB0BR1 = highbyte;

		//Clear SW reset, resume operation
		UCB0CTL1 &= ~UCSWRST;

		return;
	}

	//Enable SW reset
	UCB3CTL1 |= UCSWRST;

	//fSCL = SMCLK/(highbyte_lowbyte)
	UCB3BR0 = lowbyte;
	UCB3BR1 = highbyte;

	//Clear SW reset, resume operation
	UCB3CTL1 &= ~UCSWRST;

}

/**
 * This is a private function. Used to start the transmission in master mode
 */
int8_t i2c_begin_transmission(uint8_t busSelect,
							  uint8_t address,
							  uint8_t asReceiver)
{
	if(busSelect == AB_I2C_BUS00)
	{
		//AB_I2C_BUS00

		//Check that the FPGA is not using it otherwise return with error:
		if(abacus_fpga_i2cGetStatus() != 0)
			return -1;

		//Reserve the I2C:
		abacus_fpga_i2cReserve();

		//Check that the FPGA is not using it otherwise return with error:
		if(abacus_fpga_i2cGetStatus() != 0)
		{
			abacus_fpga_i2cRelease();
			return -1;
		}


		//Make sure stop condition got sent:
		uint16_t counter = 0;
		while (UCB0CTL1 & UCTXSTP)
		{
			counter++;
			if(counter > 30000)
			{
				i2c00.isOnError = 1;
				//I2C stop condition
				UCB0CTL1 |= UCTXSTP;
				abacus_i2c_init();
				return -2;	//Very error
			}
		}

		//Set the address:
		UCB0I2CSA = address;
		i2c00.lastAddress = address;

		if(asReceiver == 0)
			UCB0CTL1 |= UCTR;	//As TX
		else
		{
			//Flush any previous byte:
			uint8_t foo = UCB0RXBUF;
			UCB0CTL1 &= ~UCTR;	//As Rx
		}

		//Generate START condition on bus
		UCB0CTL1 |= UCTXSTT;

		return 0;
	}
	//AB_I2C_BUS01

	//Make sure stop condition got sent:
	uint16_t counter = 0;
	while (UCB3CTL1 & UCTXSTP)
	{
		counter++;
		if(counter > 30000)
		{
			//I2C stop condition
			UCB3CTL1 |= UCTXSTP;
			i2c01.isOnError = 1;
			abacus_i2c_init();
			return -2;	//Very error
		}
	}

	//Set the address:
	UCB3I2CSA = address;
	i2c01.lastAddress = address;

	if(asReceiver == 0)
		UCB3CTL1 |= UCTR;	//As TX
	else
	{
		//Flush any previous byte:
		uint8_t foo = UCB3RXBUF;
		UCB3CTL1 &= ~UCTR;	//As Rx
	}

	//Generate START condition on bus
	UCB3CTL1 |= UCTXSTT;
	
	return 0;
}

/**
 * This is a private function
 * First byte works differently than the other bytes because it also handles
 * the flags of sending the address.
 */
int8_t i2c_write_firstbyte(uint8_t busSelect, uint8_t *buffer)
{
	//address is being transmitted as this time, we have to write something
	//in buffer now in orther to prevent bus being stalled:
	if(busSelect == AB_I2C_BUS00)
	{
		// Wait for UCTXIF
		while (UCB0IFG & !UCTXIFG);
		
		//Fill the tx buffer
		UCB0TXBUF = *buffer;
		
		//Now wait for start condition flag is set to 0. This happens when address
		//Is finished being sent. It might hang if there is a malfunction on the i2c
		//bus like a shortcircuit
		uint16_t counter = 0;
		while (UCB0CTL1 & UCTXSTT)
		{
			counter++;
			if(counter > 30000)
			{
				i2c00.isOnError = 1;
				return -2;	//Very error
			}
		}
		
		//Ok, we need to check if there was acknowledge
		if ( (UCB0IFG & UCNACKIFG) )
		{
			// We got NACK!
			//I2C stop condition
			UCB0CTL1 |= UCTXSTP;
			i2c00.isOnError = 1;
			//Release i2C for the FPGA
			abacus_fpga_i2cRelease();
			return -1;
		}
		
		//Ok, everything worked well, at this point first byte is being transmitted
		i2c00.isOnError = 0;
		return 0;
	}
	
	// Wait for UCTXIF
	while (UCB3IFG & !UCTXIFG);

	//Fill the tx buffer
	UCB3TXBUF = *buffer;

	//Now wait for start condition flag is set to 0. This happens when address
	//Is finished being sent. It might hang if there is a malfunction on the i2c
	//bus like a shortcircuit
	uint16_t counter = 0;
	while (UCB3CTL1 & UCTXSTT)
	{
		counter++;
		if(counter > 30000)
		{
			i2c01.isOnError = 1;
			return -2;	//Very error
		}
	}

	//Ok, we need to check if there was adknowledge
	if ( (UCB3IFG & UCNACKIFG) )
	{
		// We got NACK!
		//I2C stop condition
		UCB3CTL1 |= UCTXSTP;
		i2c01.isOnError = 1;
		return -1;
	}

	//Ok, everything worked well, at this point first byte is being transmitted
	i2c01.isOnError = 0;
	return 0;
}
						

/**
 * This is a private function
 * It sends the buffer to the selected bus. This is a blocking function!
 * If it fails it will return -1 and activate bus flag error.
 */
int8_t abacus_i2c_write(uint8_t busSelect,
						uint8_t address,
						uint8_t *buffer,
						uint16_t lenght,
						uint8_t repeatedStart)
{
	//Going into a repeated start?
	if(busSelect == AB_I2C_BUS00 && i2c00.inRepeatedStartCondition == 1)
	{
		//Repeated start on bus 00
		UCB0CTL1 |= UCTR;	//As TX
		UCB0CTL1 |= UCTXSTT;	//Repeated start condition
	}
	else if(busSelect == AB_I2C_BUS01 && i2c01.inRepeatedStartCondition == 1)
	{
		//Repeated start on bus 01
		UCB3CTL1 |= UCTR;	//As TX
		UCB3CTL1 |= UCTXSTT;	//Repeated start condition
	}
	else
	{
		//Starting from scracth

		//Begin transmission
		if(i2c_begin_transmission(busSelect, address, 0) != 0)
			return -2;	//I2C was busy by the FPGA

		//Send the first byte and return with error if unack
		if(i2c_write_firstbyte(busSelect, buffer) != 0)
			return -1;
	
		//First byte was already sent
		buffer++;
		lenght--;

		//First byte is being transmitted at this point. We need to stop
		//Sending bytes or continue if buffer is not yet empty
	}
	
	if(busSelect == AB_I2C_BUS00)
	{
		//AB_I2C_BUS00

		//Now we send the bytes:
		while(lenght)
		{
			//UCTXIFG is set again as soon as the data is transferred
			//from the buffer into the shift register
			while (!(UCB0IFG & UCTXIFG))
			{
				if ( (UCB0IFG & UCNACKIFG) )
				{
					// We got NACK!
					//I2C stop condition
					UCB0CTL1 |= UCTXSTP;
					i2c00.isOnError = 1;
					i2c00.inRepeatedStartCondition = 0;
					//Release i2C for the FPGA
					abacus_fpga_i2cRelease();
					return -1;
				}
			}

			//Fill again the tx buffer
	    	UCB0TXBUF = *buffer;

		    lenght--;
		    if(lenght != 0)
		    	buffer++;
		    
		}
		
		// Wait for TX buffer to empty
		while (!(UCB0IFG & UCTXIFG))
			if ( (UCB0IFG & UCNACKIFG) )
			{
				// We got NACK!
				//I2C stop condition
				UCB0CTL1 |= UCTXSTP;
				i2c00.isOnError = 1;
				i2c00.inRepeatedStartCondition = 0;
				//Release i2C for the FPGA
				abacus_fpga_i2cRelease();
				return -1;
			}

		if(repeatedStart == 0)
		{
			//I2C stop condition
			UCB0CTL1 |= UCTXSTP;

			//Set flag by software:
			UCB0IFG &= ~UCTXIFG;

			//Clear bus errors
			i2c00.isOnError = 0;
			i2c00.inRepeatedStartCondition = 0;

			while (UCB0CTL1 & UCTXSTP);

			//Release i2C for the FPGA
			abacus_fpga_i2cRelease();

			return 0;
		}

		//A repeated start is needed in next transaction!
		i2c00.inRepeatedStartCondition = 1;

		return 1;
	}

	//AB_I2C_BUS01

	//Now we send the bytes:
	while(lenght)
	{
		//UCTXIFG is set again as soon as the data is transferred
		//from the buffer into the shift register
		//while (UCB3IFG & !UCTXIFG)
		while(!(UCB3IFG & UCTXIFG))
		{
			if ( (UCB3IFG & UCNACKIFG) )
			{
				// We got NACK!
				//I2C stop condition
				UCB3CTL1 |= UCTXSTP;
				i2c01.isOnError = 1;
				i2c01.inRepeatedStartCondition = 0;
				return -1;
			}
		}

		//Fill again the tx buffer
    	UCB3TXBUF = *buffer;

	    lenght--;
	    if(lenght != 0)
	    	buffer++;

	}

	// Wait for TX buffer to empty
	//while (UCB3IFG & !UCTXIFG)
	while(!(UCB3IFG & UCTXIFG))
		if ( (UCB3IFG & UCNACKIFG) )
		{
			// We got NACK!
			//I2C stop condition
			UCB3CTL1 |= UCTXSTP;
			i2c01.isOnError = 1;
			i2c01.inRepeatedStartCondition = 0;
			return -1;
		}

	if(repeatedStart == 0)
	{
		//I2C stop condition
		UCB3CTL1 |= UCTXSTP;

		//Set flag by software:
		UCB3IFG &= ~UCTXIFG;

		//Clear bus errors
		i2c01.isOnError = 0;

		//Wait for repeated start to be sent
		//while (UCB3CTL1 & UCTXSTP);
		i2c01.inRepeatedStartCondition = 0;

		while (UCB3CTL1 & UCTXSTP);

		return 0;
	}

	//A repeated start is needed in next transaction!
	i2c01.inRepeatedStartCondition = 1;

	return 1;
}


/*
 * It sends read request to slave and saves the incoming data to buffer.
 * This is a blocking function!!
 */
int8_t abacus_i2c_requestFrom(uint8_t busSelect,
							  uint8_t address,
							  uint8_t *buffer,
							  uint16_t lenght,
							  uint8_t repeatedStart)
{

	//Going into a repeated start?
	if(busSelect == AB_I2C_BUS00 && i2c00.inRepeatedStartCondition == 1)
	{
		//Repeated start on bus 00

		uint8_t foo = UCB0RXBUF;	//Flush any previous byte:
		UCB0CTL1 &= ~UCTR;	//As Rx
		UCB0CTL1 |= UCTXSTT;	//Repeated start condition
	}
	else if(busSelect == AB_I2C_BUS01 && i2c01.inRepeatedStartCondition == 1)
	{
		//Repeated start on bus 01

		uint8_t foo = UCB3RXBUF;	//Flush any previous byte:
		UCB3CTL1 &= ~UCTR;	//As Rx
		UCB3CTL1 |= UCTXSTT;	//Repeated start condition
	}
	else
	{
		//Begin transmission as reader and return if slave does not answer
		if(i2c_begin_transmission(busSelect, address, 1) != 0)
			return -2;	//Bus was in use by the FPGA
	}

	int wait = 30000;

	if(busSelect == AB_I2C_BUS00)
	{
		//Wait for the start condition to be send:
		uint16_t counter = 0;
		while (UCB0CTL1 & UCTXSTT)
		{
			counter++;
			if(counter > 30000)
			{
				i2c00.isOnError = 1;
				i2c00.inRepeatedStartCondition = 0;
				UCB0CTL1 |= UCTXSTP;
				return -2;	//Very error
			}
		}

		if ( (UCB0IFG & UCNACKIFG) )
		{
			// We got NACK!
			//I2C stop condition
			UCB0CTL1 |= UCTXSTP;
			i2c00.isOnError = 1;
			i2c00.inRepeatedStartCondition = 0;
			//Release i2C for the FPGA
			abacus_fpga_i2cRelease();
			return -1;
		}

		// Only one byte to be received?
		if (lenght == 1)
		{
			if(repeatedStart == 0)
				UCB0CTL1 |= UCTXSTP + UCTXNACK;  // Generate I2C stop condition and NACK
			else
			{
				UCB0CTL1 |= UCTXNACK;  // Send NACK
				i2c00.inRepeatedStartCondition = 1;
			}
		}

		//Receive each byte only when something available
		while(lenght)
		{

			//wait for receive flag to be set
			if((UCB0IFG & UCRXIFG))
			{
				//Move byte from hardware buffer
		        *buffer = UCB0RXBUF;
				lenght--;

				//Move pointer buffer
				if(lenght != 0)
					buffer++;

				// Only one byte left?
				if (lenght == 1)
				{
					if(repeatedStart == 0)
						UCB0CTL1 |= UCTXSTP + UCTXNACK;  // Generate I2C stop condition and NACK
					else
					{
						UCB0CTL1 |= UCTXNACK;  // Send NACK
						i2c00.inRepeatedStartCondition = 1;
					}
				}
			}
			else
			{
				if(--wait == 0)
				{
					//if this transaction took too long, bail
					lenght = 0;
					UCB0CTL1 |= UCTXSTP;
					i2c00.isOnError = 1;
					i2c00.inRepeatedStartCondition = 0;
					//Release i2C for the FPGA
					abacus_fpga_i2cRelease();
					//Exit with errors
					return -1;
				}
			}
		}

		i2c00.isOnError = 0;

		if(repeatedStart == 0)
		{
			//Wait for repeated start to be sent
			while (UCB0CTL1 & UCTXSTP);
			i2c00.inRepeatedStartCondition = 0;

			//Release i2C for the FPGA
			abacus_fpga_i2cRelease();

			//Exit without errors
			return 0;
		}

		return 1;	//Repeated start! :-D
	}

	//Wait for the start condition to be send:
	uint16_t counter = 0;
	while (UCB3CTL1 & UCTXSTT)
	{
		counter++;
		if(counter > 30000)
		{
			i2c01.isOnError = 1;
			i2c01.inRepeatedStartCondition = 0;
			UCB3CTL1 |= UCTXSTP;
			return -2;	//Very error
		}
	}

	if ( (UCB3IFG & UCNACKIFG) )
	{
		// We got NACK!
		//I2C stop condition
		UCB3CTL1 |= UCTXSTP;
		i2c01.isOnError = 1;
		i2c01.inRepeatedStartCondition = 0;
		return -1;
	}

	// Only one byte to be received?
	if (lenght == 1)
	{
		if(repeatedStart == 0)
		{
			UCB3CTL1 |= UCTXNACK;  // Send NACK
			UCB3CTL1 |= UCTXSTP;  // Generate I2C stop condition
		}
		else
		{
			UCB3CTL1 |= UCTXNACK;  // Send NACK
			i2c01.inRepeatedStartCondition = 1;
		}
	}


	//Receive each byte only when something available
	while(lenght)
	{

		//wait for receive flag to be set
		if((UCB3IFG & UCRXIFG))
		{
			//Move byte from hardware buffer
	        *buffer = UCB3RXBUF;
			lenght--;

			//Move pointer buffer
			if(lenght != 0)
				buffer++;

			// Only one byte left?
			if (lenght == 1)
			{
				if(repeatedStart == 0)
				{
					UCB3CTL1 |= UCTXNACK;  // Send NACK
					UCB3CTL1 |= UCTXSTP;  // Generate I2C stop condition
				}
				else
				{
					UCB3CTL1 |= UCTXNACK;  // Send NACK
					i2c01.inRepeatedStartCondition = 1;
				}
			}
		}
		else
		{
			if(--wait == 0)
			{
				//if this transaction took too long, bail
				lenght = 0;
				UCB3CTL1 |= UCTXSTP;
				i2c01.isOnError = 1;
				i2c01.inRepeatedStartCondition = 0;
				//Exit with errors
				return -1;
			}
		}
	}

	i2c01.isOnError = 0;

	//Exit without errors
	if(repeatedStart == 0)
	{
		i2c01.inRepeatedStartCondition = 0;
		//Wait for repeated start to be sent
		while (UCB3CTL1 & UCTXSTP);
		return 0;
	}
	else
		return 1;
}

/**
 * It puts the I2C in read mode to the selected address and returns a single
 * 8bit register from a selected address.
 */
uint8_t abacus_i2c_readRegister(uint8_t busSelect,
		uint8_t address,
		uint8_t reg)
{

	abacus_i2c_write(busSelect, address, &reg, 1, 0);
	uint8_t buffer[1];
	abacus_i2c_requestFrom(busSelect, address, buffer, 1, 0);

	return (buffer[0]);
}

/**
 * It puts the I2C in read mode to the selected address and returns
 * 2 bytes register from a selected address in the buffer pointer.
 */
/*void abacus_i2c_read16bitRegister(uint8_t busSelect,
		uint8_t address,
		uint8_t reg,
		uint8_t *buffer)
{

	abacus_i2c_write(busSelect, address, &reg, 1, 0);
	abacus_i2c_requestFrom(busSelect, address, buffer, 2, 0);
}*/


/* Yet to implement the slave functions for ABACUS i2c
/////
//Slave functions:
int8_t abacus_i2c_slaveInit(uint8_t busSelect,
		uint8_t address,
		void (*triggerFunctionRequest)(uint8_t*),
		void (*triggerFunctionReceive)(uint8_t*))
{
	//TODO
}

int16_t abacus_i2c_slaveAvailable(uint8_t busSelect)
{
	//TODO
}

uint8_t abacus_i2c_slaveRead(uint8_t busSelect)
{
	//TODO
}

int8_t abacus_i2c_slaveWrite(uint8_t busSelect,
		uint8_t *buffer,
		uint16_t lenght)
{
	//TODO
}
*/
