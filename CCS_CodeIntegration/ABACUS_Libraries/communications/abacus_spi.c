/*
 * abacus_spi.c
 *
 */

 #include "abacus_spi.h"

struct SPIPort spi00;
struct SPIPort spi01;

/*
 *
 */
void abacus_spi_init()
{
	//Configuring the SPI for the MCU flash
	spi00.spi_name = AB_SPI_BUS00;

	//Put state machine in reset
	UCB2CTL1 |= UCSWRST;
	// 3-pin, 8-bit SPI master
	UCB2CTL0 |= UCMST + UCSYNC + UCCKPL + UCMSB;
	// Clock polarity high, MSB
	// SMCLK
	UCB2CTL1 |= UCSSEL_2;
	//2
	UCB2BR0 = 0x02;
	UCB2BR1 = 0x00;

	// Select P9 for SPI
	P9SEL |=  BIT1 | BIT2 | BIT3;

	// Pull-up on SOMI
	//P9OUT |= BIT2;

	// SIMO (output)
	P9DIR |=  BIT1;
	// SOMI (input)
	P9DIR &= ~BIT2;
	// CLK (output)
	P9DIR |= BIT3;

	//Initialize USCI state machine
	UCB2CTL1 &= ~UCSWRST;

	//**************************************
	//Configuring the SPI for the FPGA flash
	spi01.spi_name = AB_SPI_BUS01;

	// **Put state machine in reset**
	UCB1CTL1 |= UCSWRST;
	// 3-pin, 8-bit SPI master
	UCB1CTL0 |= UCMST + UCSYNC + UCCKPL + UCMSB;
	// Clock polarity high, MSB
	// SMCLK
	UCB1CTL1 |= UCSSEL_2;
	//2
	UCB1BR0 = 0x02;
	UCB1BR1 = 0x00;

	// Select P5 & P3 for SPI
	P5SEL |=  BIT4 | BIT5;
	P3SEL |=  BIT7;

	// SIMO (output)
	P3DIR |=  BIT7;
	// SOMI (input)
	P5DIR &= ~BIT4;
	// CLK (output)
	P5DIR |= BIT5;

	//Initialize USCI state machine
	UCB1CTL1 &= ~UCSWRST;
}



/*
 *
 */
int8_t abacus_spi_write_instruction(uint8_t busSelect, uint8_t instruction)
{
	if(busSelect == AB_SPI_BUS00)
	{
		//AB_SPI_BUS00

		// Wait for TXBUF ready
		while (!(UCB2IFG & UCTXIFG));
		UCB2TXBUF = instruction;

		// Wait for TX complete
		while (UCB2STAT & UCBUSY);

		return 0;
	}

	//AB_SPI_BUS01

	// Wait for TXBUF ready
	while (!(UCB1IFG & UCTXIFG));
	UCB1TXBUF = instruction;

	// Wait for TX complete
	while (UCB1STAT & UCBUSY);

	return 0;
}

/*
 *
 */
int8_t abacus_spi_write_read(uint8_t busSelect,
							 uint8_t *bufferOut, unsigned int bufferOutLenght,
							 uint8_t *bufferIn,  unsigned int bufferInLenght)
{
	if(busSelect == AB_SPI_BUS00)
	{
		//AB_SPI_BUS00

		//First we send:
		while(bufferOutLenght)
		{
			// Wait for TXBUF ready
			while (!(UCB2IFG & UCTXIFG));
			//Fill buffer
			UCB2TXBUF = *bufferOut;

			bufferOutLenght--;
			if(bufferOutLenght != 0)
				*bufferOut++;
		}
		// Wait for TX complete
		while (UCB2STAT & UCBUSY);

		//Empty the RX buffer:
		uint8_t dummy = UCB2RXBUF;

		//Then we read
		while(bufferInLenght)
		{
			//Send dummy byte to keep the clock running
			UCB2TXBUF = 0;
			// Wait for RX to finish
			while (!(UCB2IFG & UCRXIFG));

			// Store data from last data RX
			*bufferIn = UCB2RXBUF;

			bufferInLenght--;
			if(bufferInLenght != 0)
				*bufferIn++;
		}

		//Return without errors
		return 0;
	}

	//AB_SPI_BUS01

	//First we send:
	while(bufferOutLenght)
	{
		// Wait for TXBUF ready
		while (!(UCB1IFG & UCTXIFG));
		//Fill buffer
		UCB1TXBUF = *bufferOut;

		bufferOutLenght--;
		if(bufferOutLenght != 0)
			*bufferOut++;
	}
	// Wait for TX complete
	while (UCB1STAT & UCBUSY);

	//Empty the RX buffer:
	uint8_t dummy = UCB1RXBUF;

	//Then we read
	while(bufferInLenght)
	{
		//Send dummy byte to keep the clock running
		UCB1TXBUF = 0;

		// Wait for RX to finish
		while (!(UCB1IFG & UCRXIFG));

		// Store data from last data RX
		*bufferIn = UCB1RXBUF;

		bufferInLenght--;
		if(bufferInLenght != 0)
			*bufferIn++;
	}

	//Return without errors
	return 0;
}


/*
 * It calculates the CRC16 of a stream coming from SPI
 */
uint16_t abacus_spi_write_read_calculateCRC16(uint8_t busSelect,
							 uint8_t *bufferOut, unsigned int bufferOutLenght,
							 uint16_t *crc16,  unsigned long bufferInLenght)
{
	uint8_t bufferIn;
	uint8_t resetCRC = 1;
	if(busSelect == AB_SPI_BUS00)
	{
		//AB_SPI_BUS00

		//First we send:
		while(bufferOutLenght)
		{
			// Wait for TXBUF ready
			while (!(UCB2IFG & UCTXIFG));
			//Fill buffer
			UCB2TXBUF = *bufferOut;

			bufferOutLenght--;
			if(bufferOutLenght != 0)
			*bufferOut++;
		}
		// Wait for TX complete
		while (UCB2STAT & UCBUSY);

		//Empty the RX buffer:
		uint8_t dummy = UCB2RXBUF;

		//Then we read
		while(bufferInLenght)
		{
			//Send dummy byte to keep the clock running
			UCB2TXBUF = 0;
			// Wait for RX to finish
			while (!(UCB2IFG & UCRXIFG));

			// Store data from last data RX
			bufferIn = UCB2RXBUF;

			//Calculate crc16:
			calculate_crc16_8bit(&bufferIn, 1, crc16, resetCRC);
			//Only first time is needed to reset
			resetCRC = 0;

			bufferInLenght--;
		}

		//Return the CRC
		return *crc16;
	}

	//AB_SPI_BUS00

	//First we send:
	while(bufferOutLenght)
	{
		// Wait for TXBUF ready
		while (!(UCB1IFG & UCTXIFG));
		//Fill buffer
		UCB1TXBUF = *bufferOut;

		bufferOutLenght--;
		if(bufferOutLenght != 0)
		*bufferOut++;
	}
	// Wait for TX complete
	while (UCB1STAT & UCBUSY);

	//Empty the RX buffer:
	uint8_t dummy = UCB1RXBUF;

	//Then we read
	while(bufferInLenght)
	{
		//Send dummy byte to keep the clock running
		UCB1TXBUF = 0;

		// Wait for RX to finish
		while (!(UCB1IFG & UCRXIFG));

		// Store data from last data RX
		bufferIn = UCB1RXBUF;

		//Calculate crc16:
		calculate_crc16_8bit(&bufferIn, 1, crc16, resetCRC);

		//Only first time is needed to reset
		resetCRC = 0;

		bufferInLenght--;
	}

	//Return the CRC
	return *crc16;
}

/*
 * It calculates the CRC16 of a stream coming from SPI
 */
uint16_t abacus_spi_write_read_calculateCRC16_kickwdt(uint8_t busSelect,
		uint8_t *bufferOut, unsigned int bufferOutLenght,
		uint16_t *crc16,  unsigned long bufferInLenght,
		uint16_t wdt_command)
{
	uint8_t bufferIn;
	uint8_t resetCRC = 1;
	if(busSelect == AB_SPI_BUS00)
	{
		//AB_SPI_BUS00

		//First we send:
		while(bufferOutLenght)
		{
			// Wait for TXBUF ready
			while (!(UCB2IFG & UCTXIFG));
			//Fill buffer
			UCB2TXBUF = *bufferOut;

			bufferOutLenght--;
			if(bufferOutLenght != 0)
				*bufferOut++;
		}
		// Wait for TX complete
		while (UCB2STAT & UCBUSY);

		//Empty the RX buffer:
		uint8_t dummy = UCB2RXBUF;

		//Then we read
		while(bufferInLenght)
		{
			//Send dummy byte to keep the clock running
			UCB2TXBUF = 0;
			// Wait for RX to finish
			while (!(UCB2IFG & UCRXIFG));

			// Store data from last data RX
			bufferIn = UCB2RXBUF;

			//Calculate crc16:
			calculate_crc16_8bit(&bufferIn, 1, crc16, resetCRC);
			//Only first time is needed to reset
			resetCRC = 0;
			//kick the infamous WDT
			WDTCTL = wdt_command;

			bufferInLenght--;
		}

		//Return the CRC
		return *crc16;
	}

	//AB_SPI_BUS00

	//First we send:
	while(bufferOutLenght)
	{
		// Wait for TXBUF ready
		while (!(UCB1IFG & UCTXIFG));
		//Fill buffer
		UCB1TXBUF = *bufferOut;

		bufferOutLenght--;
		if(bufferOutLenght != 0)
			*bufferOut++;
	}
	// Wait for TX complete
	while (UCB1STAT & UCBUSY);

	//Empty the RX buffer:
	uint8_t dummy = UCB1RXBUF;

	//Then we read
	while(bufferInLenght)
	{
		//Send dummy byte to keep the clock running
		UCB1TXBUF = 0;

		// Wait for RX to finish
		while (!(UCB1IFG & UCRXIFG));

		// Store data from last data RX
		bufferIn = UCB1RXBUF;

		//Calculate crc16:
		calculate_crc16_8bit(&bufferIn, 1, crc16, resetCRC);

		//Only first time is needed to reset
		resetCRC = 0;

		//kick the infamous WDT
		WDTCTL = wdt_command;

		bufferInLenght--;
	}

	//Return the CRC
	return *crc16;
}
