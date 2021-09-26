/*
 * abacus_flash.c
 *
 */

 #include "abacus_flash.h"

struct FlashMemory memoryFlashMCU;
struct FlashMemory memoryFlashFPGA;
 
/*
 *
 */
void spi_chipSelect(uint8_t memorySelect, uint8_t enable)
{
	if(memorySelect == AB_FLASH_MCU)
	{
		//AB_SPI_BUS00

		if(enable == 1)
			P9OUT &= ~BIT0;
		else
			P9OUT |= BIT0;

		return;
	}

	//AB_FLASH_FPGA

	if(enable == 1)
	{
		//Enable switch
		P10OUT |= BIT6;
		__delay_cycles (10);
		//Enable CS (inverted)
		P10OUT |= BIT7;
	}
	else
	{
		//Switch and CS
		//Disable CS
		P10OUT &= ~BIT7;
		__delay_cycles (10);
		//Disable switch
		P10OUT &= ~BIT6;

	}
}

/*
 * It enables or disables the hardware write protection of the MCU flash memory
 * connected in the pin P9.6. it is a -W.
 */
void flash_MCU_writeProtection(uint8_t enable)
{
	if(enable == 1)
		P9OUT |= BIT6;
	else
		P9OUT &= ~BIT6;

}

/*
 * It boots up the MCU flash memory and configures the control pins
 * It also reads the MCU flash properties
 */
void abacus_flashMCU_init()
{
	// Select P9.0 for CS FLASH 1
	P9SEL &=  ~BIT0;

	// Disable CS, W, Hold FLASH 1
	P9OUT |= BIT0 + BIT6 + BIT7;

	//Declare all of them as outputs:
	P9DIR |= BIT0 + BIT6 + BIT7;

	//Get the 3 byte identification
	uint8_t instruction = FLASH_RDID;
	abacus_flash_write_read_instruction(AB_FLASH_MCU,
										&instruction, 1,
										memoryFlashMCU.flash_identification, 3);
}

/*
 * It initializes the pinouts of the MCU to control the FPGA Flash
 * but it does not interrogate it because it might be in use by the FPGA
 */
void abacus_flashFPGA_init()
{
	// Disable SWITCH for FLASH 2
	P10OUT &= ~BIT6;
	// Disable CS for FLASH 2 (it is logic inverted because it is through
	// a mosfet Watch out!
	P10OUT &= ~BIT7;

	// SWITCH (output) & CS for FLASH 2
	P10DIR |=  BIT6 + BIT7;

	//Get the 3 byte identification
	//Not implemented because at boot time this memory is accessed at boot time
	//by the FPGA. Sorry!
}

/*
 * Writes a single instruction to the memory
 */
int8_t abacus_flash_write_instruction(uint8_t memorySelect,
									  uint8_t instruction)
{
	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;

	//Chip select up
	spi_chipSelect(memorySelect, 1);
	//Send a single byte
	int8_t result = abacus_spi_write_instruction(busSelect, instruction);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	return result;
}

/*
 * It serializes the address of 4 bytes in 3 bytes and in reverse order used
 * in this strange flash memories...
 */
void flash_prepareCommandAddress(uint32_t address, uint8_t *outputBuffer)
{
	//Put the address and instruction in 4 bytes:
	//1 byte is FLASH_READ command
	//2 byte is third byte of address
	//3 byte is second byte of address
	//4 byte is first byte of address
	ulong2char(&address, outputBuffer);
	//At this point only bytes 0, 1 and 2 are usable
	//0 goes to 3, 1 goes to 2, 2 goes to 1
	outputBuffer[3] = outputBuffer[0];
	//Dont lose the data at position 2
	outputBuffer[0] = outputBuffer[2];
	outputBuffer[2] = outputBuffer[1];
	outputBuffer[1] = outputBuffer[0];
}


/*
 * Writes a buffer to the memory and reads the desired amount of data
 */
int8_t abacus_flash_write_read_instruction(uint8_t memorySelect,
									  uint8_t *instruction,
									  uint16_t instructionLenght,
									  uint8_t *data,
									  uint16_t dataLenght)
{
	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;

	//Chip select up
	spi_chipSelect(memorySelect, 1);
	//Send the data and read the output
	int8_t result = abacus_spi_write_read(busSelect,
										  instruction,
										  instructionLenght,
										  data,
										  dataLenght);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	return result;
}

/*
 * It checks the registers of the memory to see if there is WIP (Work in
 * progress) on going. For instance just after writing or erasing sectors
 */
int8_t abacus_flash_isWorkInProgress(uint8_t memorySelect)
{
	uint8_t instruction = FLASH_RDSR;
	uint8_t buffer;
	abacus_flash_write_read_instruction(memorySelect, &instruction, 1, &buffer, 1);

	//buffer now contains a byte with the following bits:
	//0: WIP, 1: WEL, 2: BP0, 3: BP1, 4: BP2

	if(buffer & BIT0)
		return 1;

	return 0;
}

/*
 * It writes on the register the configuration to unprotect all the memory:
 */
int8_t abacus_flash_unprotectMemory(uint8_t memorySelect)
{
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;
	else
		//Disable the hardware write protection:
		flash_MCU_writeProtection(1);

	//Send command to unprotect all areas:
	uint8_t instruction[2];
	instruction[0] = FLASH_WRSR;
	//b0, b1, b5 & b6 are readonly. We want all the other bits to be 0
	instruction[1] = 0x00;

	//Chip select up
	spi_chipSelect(memorySelect, 1);
	//Send the data but we dont want to read anything back
	int8_t result = abacus_spi_write_read(busSelect,
										  instruction,
										  2,
										  instruction,
										  0);
	if(memorySelect == AB_FLASH_MCU)
	//	//Disable the hardware write protection:
		flash_MCU_writeProtection(0);

	return result;
}

/*
 * It writes data into the flash memory
 */
int8_t abacus_flash_write_data(uint8_t memorySelect,
							   uint32_t address,
							   uint8_t *data,
							   uint32_t lenght)
{
	//Check that the complete data will fall inside a single page, otherwise
	//return error -3
	uint32_t startPage = address/256;
	uint32_t endPage = (address + lenght - 1)/256;
	if(startPage != endPage)
		return -3;

	//Return process could not be completed because it is busy
	if(abacus_flash_isWorkInProgress(memorySelect) == 1)
		return -2;

	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;
	else
		//Enable the hardware write protection:
		flash_MCU_writeProtection(1);


	//Send write enable instruction
	spi_chipSelect(memorySelect, 1);
	abacus_flash_write_instruction(memorySelect, FLASH_WREN);
	spi_chipSelect(memorySelect, 0);

	//Put the address and instruction in 4 bytes:
	uint8_t outputBuffer[4];
	flash_prepareCommandAddress(address, outputBuffer);
	//Add instruction
	outputBuffer[0] = FLASH_PP;

	//Chip select up
	spi_chipSelect(memorySelect, 1);

	//Send the command and address and read nothing
	int8_t result = abacus_spi_write_read(busSelect,
										  outputBuffer,
										  4,
										  outputBuffer,
										  0);

	//Send the data and read nothing
	result += abacus_spi_write_read(busSelect,
										  data,
										  lenght,
										  outputBuffer,
										  0);

	//Chip select down
	spi_chipSelect(memorySelect, 0);

	if(memorySelect == AB_FLASH_MCU)
		//Disable the hardware write protection:
		flash_MCU_writeProtection(0);

	return result;
}

/*
 * It writes data taking into account the page handling!!
 */
int8_t abacus_flash_write_dataEasy(uint8_t memorySelect,
							   uint32_t address,
							   uint8_t *data,
							   uint32_t lenght)
{
	uint32_t startPage = address/256;
	uint32_t endPage = (address + lenght - 1)/256;
	if(startPage == endPage)
		//Fortunately no need to write in more than one page :)))))
		return abacus_flash_write_data(memorySelect, address, data, lenght);

	//Oh no!!! we have to write in several different pages:
	uint32_t i; //
	uint32_t tempStartAddress = address;
	uint32_t tempEndAddress = (startPage + 1) * 256 - 1;
	int8_t error = 0;

	for(i = 0; i <= (endPage - startPage); i++)
	{
		//Wait until flash is ready
		abacus_flash_wait_while_busy(memorySelect, 50000U, 0);

		uint32_t size = tempEndAddress - tempStartAddress + 1;

		//Write to abacus
		error += abacus_flash_write_data(memorySelect,
								tempStartAddress,
								data,
								size);

		//If error, abort
		if(error != 0)
			return error;

		//Move pointer
		data += size;

		//Calculate new Start and end addresses
		tempStartAddress = tempEndAddress + 1;
		tempEndAddress = tempEndAddress + 256;
		if(tempEndAddress > (address + lenght))
			//Recalculate end address:
			tempEndAddress = address + lenght - 1;

	}

	return error;
}

/*
 * It locks the MCU until flash is ready. returns -1 if it did not
 * get ready in the specified time...
 */
int8_t abacus_flash_wait_while_busy(uint8_t memorySelect,
								uint16_t cycles,
								int8_t toInfinity)
{
	if(toInfinity == 1)
	{
		while(abacus_flash_isWorkInProgress(memorySelect) == 1);
		return 0;
	}

	uint16_t i;
	for(i = 0; i < cycles; i++)
	{
		if(abacus_flash_isWorkInProgress(memorySelect) == 1)
			continue;
		return 0;
	}
	return -1;	//Failed to wait
}

/*
 * It reads a data from the specified memory, address and lenght
 * into the data buffer
 */
int8_t abacus_flash_read_data(uint8_t memorySelect,
							  uint32_t address,
							  uint8_t *data,
							  uint32_t lenght)
{
	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;

	//Put the address and instruction in 4 bytes:
	uint8_t outputBuffer[4];
	flash_prepareCommandAddress(address, outputBuffer);
	//Add instruction
	outputBuffer[0] = FLASH_READ;

	//Chip select up
	spi_chipSelect(memorySelect, 1);

	//Send the data and read the output
	int8_t result = abacus_spi_write_read(busSelect,
										  outputBuffer,
										  4,
										  data,
										  lenght);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	return 0;
}

/*
 * It dumps all the data of the flash memory to the selected UART.
 * Be carefull, at AB_B115200 it will take 20 minutes...
 */
int8_t abacus_flash_dump_memory(uint8_t memorySelect,
							    uint8_t uart,
							    uint32_t startAddress,
							    uint32_t lenght)
{
	uint8_t errors = 0;
	uint32_t pointer = startAddress;
	uint32_t endPoint = startAddress + lenght;
	if(lenght == 0)
		endPoint = 0x00FFFFFF;
	uint8_t buffer[256];
	while(pointer < endPoint)
	{
		//Wait until UART buffer is empty,
		//otherwise interrupts destroy reading from SPI
		abacus_uart_waitUntilTxFinished(uart);

		//Read from SPI
		errors += abacus_flash_read_data(memorySelect, pointer, buffer, 256);
		//Write in uart
		if((pointer + 256) > endPoint)
			abacus_uart_write(uart, buffer, endPoint - pointer);
		else
			abacus_uart_write(uart, buffer, 256);
		pointer += 256;
	}
	return errors;
}

/*
 * It calculates the CRC16 of the flash memory. You can indicate the start
 * address and the lenght
 */
uint16_t abacus_flash_crc(uint8_t memorySelect,
						uint32_t startAddress,
						uint32_t lenght)
{
	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;

	//Put the address and instruction in 4 bytes:
	uint8_t outputBuffer[4];
	flash_prepareCommandAddress(startAddress, outputBuffer);

	//Add instruction
	outputBuffer[0] = FLASH_READ;

	//Chip select up
	spi_chipSelect(memorySelect, 1);

	uint16_t crc;

	//Send the data and read the output
	uint16_t result = abacus_spi_write_read_calculateCRC16(busSelect,
										  outputBuffer,
										  4,
										  &crc,
										  lenght);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	return crc;
}

uint16_t abacus_flash_crc_kickwdt(uint8_t memorySelect,
						uint32_t startAddress,
						uint32_t lenght,
						uint16_t wdt_command)
{
	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;

	//Put the address and instruction in 4 bytes:
	uint8_t outputBuffer[4];
	flash_prepareCommandAddress(startAddress, outputBuffer);

	//Add instruction
	outputBuffer[0] = FLASH_READ;

	//Chip select up
	spi_chipSelect(memorySelect, 1);

	uint16_t crc;

	//Send the data and read the output
	uint16_t result = abacus_spi_write_read_calculateCRC16_kickwdt(busSelect,
			outputBuffer,
			4,
			&crc,
			lenght,
			wdt_command);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	return crc;
}



/*
 * It erases a specified sector of the flash memory
 */
int8_t abacus_flash_sector_erase(uint8_t memorySelect,
							  uint32_t address)
{
	if(abacus_flash_isWorkInProgress(memorySelect) == 1)
		return -2;	//Return process could not be completed because it is busy

	//By default use the bus of the MCU
	uint8_t busSelect = AB_SPI_BUS00;
	if(memorySelect == AB_FLASH_FPGA)
		busSelect = AB_SPI_BUS01;
	else
		//Enable the hardware write protection:
		flash_MCU_writeProtection(1);

	//Send write enable instruction
	spi_chipSelect(memorySelect, 1);
	abacus_flash_write_instruction(memorySelect, FLASH_WREN);
	spi_chipSelect(memorySelect, 0);

	//Put the address and instruction in 4 bytes:
	uint8_t outputBuffer[4];
	flash_prepareCommandAddress(address, outputBuffer);
	//Add instruction
	outputBuffer[0] = FLASH_SE;

	//Chip select up
	spi_chipSelect(memorySelect, 1);
	//Send the data but we dont want to read anything back
	int8_t result = abacus_spi_write_read(busSelect,
											outputBuffer,
											4,
											outputBuffer,
											0);
	//Chip select down
	spi_chipSelect(memorySelect, 0);

	if(memorySelect == AB_FLASH_MCU)
		//Disable the hardware write protection:
		flash_MCU_writeProtection(0);

	return 0;
}

/*
 * It formats all the memory (all set to 0xFF). It takes
 * 40 seconds to complete but this instruction is non blocking
 * Be aware that if you are flashing the FPGA, you should make a progB_UP()
 * first, otherwise the FPGA prevents the flash to be programmed.
 */
int8_t abacus_flash_bulk_erase(uint8_t memorySelect)
{
	if(abacus_flash_isWorkInProgress(memorySelect) == 1)
		return -2;	//Return process could not be completed because it is busy

	//By default use the bus of the MCU
	if(memorySelect == AB_FLASH_MCU)
		//Enable the hardware write protection:
		flash_MCU_writeProtection(1);

	//If AB_FLASH_FPGA, you should externally progrBholdup() until the end

	//Send write enable instruction
	spi_chipSelect(memorySelect, 1);
	abacus_flash_write_instruction(memorySelect, FLASH_WREN);
	spi_chipSelect(memorySelect, 0);

	//Send bulk erase command
	spi_chipSelect(memorySelect, 1);
	abacus_flash_write_instruction(memorySelect, FLASH_BE);
	spi_chipSelect(memorySelect, 0);

	if(memorySelect == AB_FLASH_MCU)
		//Disable the hardware write protection:
		flash_MCU_writeProtection(0);

	return 0;
}

