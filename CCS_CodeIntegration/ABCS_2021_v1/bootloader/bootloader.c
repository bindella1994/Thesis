/*
 * bootloader.c
 *
 *  Created on: 05/giu/2015
 *      Author: Aitor
 */


#include "bootloader.h"


#pragma CODE_SECTION(bootloaderStart,".text_bootloader")
#pragma CODE_SECTION(bootloaderTriggerRam,".text_bootloader")

/**
 * It checks the CRC of the banks A & B where program is stored and
 * saves the CRC on memory address bank A over the configuration address
 * If it fails it will load the memory from backup area (if CRC is also correct)
 * Force indicates if it has to ignore the CRC of the config area and recalculate
 * it, useful if new code has been added
 */
int8_t memory_mcu_checkProgramIntegrity(uint8_t force)
{
    uint16_t crcBankA, crcBankB, crcBankACalc, crcBankBCalc;

    if(force == 0)
    {
        //Read values from config area MEMORY_PROGRAMINTEGRITY_ADD for banks A and B
        uint8_t buffer[5];
        abacus_flash_mcu_read_data(MCUMEMORY_PROGRAMINTEGRITY_ADD, buffer, 5);

        //First Magic word
        if(buffer[0] != MEMORY_MAGICWORD)
            force = 1;  //Force recalculation
        else
        {
            //Second CRC bank A
            char2uint(&buffer[1], &crcBankA);

            //Third CRC bank B
            char2uint(&buffer[3], &crcBankB);
        }
    }

    //Calculate CRC16 of Bank Aa
    crcBankACalc = abacus_flash_mcu_crc(MEMORY_PROGRAM_A_ADD, MEMORY_PROGRAM_A_SIZE);

    //Calculate CRC16 of Bank B
    crcBankBCalc = abacus_flash_mcu_crc(MEMORY_PROGRAM_B_ADD, MEMORY_PROGRAM_B_SIZE);

    //Save into memory the new CRC values
    if(force == 1)
    {
        //Delete the segment and save values:
        abacus_flash_mcu_erase(MCUMEMORY_PROGRAMINTEGRITY_ADD, 512);

        //Serialize the values:
        uint8_t buffer[5];
        buffer[0] = MEMORY_MAGICWORD;
        uint2char(&crcBankACalc, &buffer[1]);
        uint2char(&crcBankBCalc, &buffer[3]);

        //Save to MCU memory
        abacus_flash_mcu_write_data(MCUMEMORY_PROGRAMINTEGRITY_ADD, buffer, 5);

        //Exit indicating saved values
        return 1;
    }

    //Program integrity confirmed?
    if(crcBankACalc == crcBankA && crcBankBCalc == crcBankB)
        return 0;

    //Ups we have a memory corruption
    return -1;
}


/**
 * It checks the integrity of programs downloaded into the MCU memory
 */
int8_t memory_mcu_checkAreaIntegrity(uint32_t startAddress)
{
    //Check how many chunks do we have to analize:
    uint8_t nchunks;

    //First read the magic word
    abacus_flash_mcu_read_data(startAddress, &nchunks, 1);
    if(nchunks != MEMORY_MAGICWORD)
        return -1;  //Nothing was there

    startAddress++;

    //Now read the number of chunks
    abacus_flash_mcu_read_data(startAddress, &nchunks, 1);

    startAddress++;

    //Sanity check
    if(nchunks == 0xFF)
        return -1;

    uint8_t i;

    uint32_t addressData = startAddress + (4 + 4 + 2) * nchunks;

    for(i = 0; i < nchunks; i++)
    {
        uint8_t buffer[10];
        uint32_t addressChunk, sizeChunk;
        uint16_t crcChunk, crcCalculated;
        abacus_flash_mcu_read_data(startAddress, buffer, 10);

        //address chunk 4 bytes
        char2ulong(&buffer[0], &addressChunk);

        //size chunk 4 bytes
        char2ulong(&buffer[4], &sizeChunk);

        //crc chunk 2 bytes
        char2uint(&buffer[8], &crcChunk);

        //calculate CRC of the selected chunk:
        crcCalculated = abacus_flash_mcu_crc(addressData, sizeChunk);

        if(crcCalculated != crcChunk)
            return -1;  //UPS incorrect area

        //Move to next chunk
        startAddress = startAddress + 10;
        addressData = addressData + sizeChunk;
    }

    return 0;
}

/**
 * It checks the integrity of Backup area and Download area (preference to backup)
 * if a valid program is found, it returns 0 and the start address
 */
int8_t memory_mcu_checkAlternativeProgramsIntegrity(uint32_t *goodStartAddress)
{
    //Read backup area:
    uint8_t result = memory_mcu_checkAreaIntegrity(MEMORY_BACKUP_ADD);

    if(result == 0)
    {
        *goodStartAddress = MEMORY_BACKUP_ADD;
        return 0;
    }

    result = memory_mcu_checkAreaIntegrity(MEMORY_DOWNLOAD_ADD);

    if(result == 0)
    {
        *goodStartAddress = MEMORY_DOWNLOAD_ADD;
        return 0;
    }

    return -1;
}

/**
 * It checks the integrity of the program (CRC16). If it fails, it will load the
 * backup program into flash memory. It uses abacus MCU memory functions, however
 * watch out, any other abacus functions do not work yet (no init called yet).
 */
int8_t bootloaderStart()
{
	//Go into a 8MHz version
	//Set DCO FLL reference = REFO
	UCSCTL3 |= SELREF_2;
	//Set ACLK = REFO
	UCSCTL4 |= SELA_2;
	//Disable the FLL control loop
	__bis_SR_register(SCG0);
	//Set lowest possible DCOx, MODx
	UCSCTL0 = 0x0000;

	//Select DCO range 8MHz operation
	UCSCTL1 = DCORSEL_5;
	//Set DCO multiplier:
	//(N + 1) * FLLRef = Fdco
	//245 * 32768 = 8MHz
	//Set FLL Div = fDCOCLK/2
	UCSCTL2 = FLLD_1 + 244;
	//Enable FLL Control loop
	__bic_SR_register(SCG0);
	//MCLK cycles for DCO to settle
	//32 * 32 * 8MHz / 32,768Hz = 250880 = MCLK
	__delay_cycles(250880);


	//Check memory integrity:
	int8_t result = memory_mcu_checkProgramIntegrity(0);
	if(result != -1)
	{
		//Program is just doing fine :)
		return 0;
	}

	//Program is corrupted! check for a backup or download programs:
	uint32_t goodProgramAddress;
	result = memory_mcu_checkAlternativeProgramsIntegrity(&goodProgramAddress);
	if(result == 0)
	{
		//we have a possible candidate!
		bootloaderTriggerRam(goodProgramAddress, 0);

		return 0;
	}

	return -1;
}

/**
 * It will check the integrity of the new program on the selected address
 * and if everything is correct it will load into RAM the code to flash
 * the program! it will finish with a PUC reboot
 */
void bootloaderTriggerRam(uint32_t startAddress, uint8_t simulation)
{
	//Sanity check, check again that there is a valid program on the selected
	//address
	int8_t result = memory_mcu_checkAreaIntegrity(startAddress);
	if(result == -1)
		return;

	//Copy function to memory RAM
	uint8_t *flash_start_ptr;
	uint8_t *ram_start_ptr;

	flash_start_ptr = (uint8_t *)BOOTLOADER_FLASH_START_ADD;
	ram_start_ptr = (uint8_t *)BOOTLOADER_RAM_START_ADD;

	// Copy flash function to RAM
	/*
	memcpy(ram_start_ptr,
			flash_start_ptr,
			BOOTLOADER_RAM_SIZE);*/
	uint16_t i;
	for(i = 0; i < BOOTLOADER_RAM_SIZE; i++)
	{
		*ram_start_ptr = *flash_start_ptr;
		ram_start_ptr++;
		flash_start_ptr++;
	}

	//Call function to memory RAM
	if(simulation == 0)
		bootloaderFlash(startAddress, simulation);
}


#pragma CODE_SECTION(bootloaderFlash,".text_bootloaderRam")
#pragma CODE_SECTION(bootloaderUChar2ULong,".text_bootloaderRam")

/**
 * External function on RAM to convert from byte to unsigned long
 */
void bootloaderUChar2ULong(uint8_t *input, uint32_t *output)
{
  typedef union
  {
    unsigned long longValue;
    uint8_t byteValue[4];
  }
  assocdata;
  assocdata conversion;
  int i;
  for(i = 0; i < 4; i++)
    conversion.byteValue[i] = input[i];

  *output = conversion.longValue;
}

/**
 * This is actually the function that is copied into the RAM! It takes 582 bytes!
 */
void bootloaderFlash(uint32_t startAddress, uint8_t simulation)
{
	// Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	//Here we assume that all CRC were correct, so no more!

	//Disable all interrupts
	__bic_SR_register(GIE);
	__disable_interrupt();

	////////////////////////////////////////
	//Unlock Bank A and delete

	//Wait for memory to be ready
	while(FCTL3 & BUSY);

	// Clear LOCK & set LOCKA
	FCTL3 = FWKEY + LOCKA;
	//Set to erase
	FCTL1 = FWKEY + MERAS;

	//Wait for memory to be ready
	while(FCTL3 & BUSY);

	//Erase segment by segment:
	uint8_t *flashPointer;
	flashPointer = (uint8_t *) MCUFLASHBANKA;

	uint8_t i;

	while(FCTL3 & BUSY);

	//Dummy write, code is stopped until delete is completed
	*flashPointer = 0;

	/*for(i = 0; i < 82; i++)
	{
		//Wait for memory to be ready
		while(FCTL3 & BUSY);

		//Dummy write, code is stopped until delete is completed
		*flashPointer = 0;

		//Move to next segment
		flashPointer += 512;
	}*/

	//Wait for memory to be ready
	while(FCTL3 & BUSY);

	////////////////////////////////////////
	//Unlock Bank B and delete
	//Set to erase in BANK mode
	FCTL1 = FWKEY + MERAS;

	//Set to bank B
	while(FCTL3 & BUSY);
	flashPointer = (uint8_t *) MCUFLASHBANKB;

	//dummy write
	*flashPointer = 0;

	//Wait for memory to be ready
	while(FCTL3 & BUSY);



	////////////////////////////////////////
	//Unlock Bank A and B and set to write
	FCTL1 = FWKEY + WRT;
	//Wait for memory to be ready
	while(FCTL3 & BUSY);

	////////////////////////////////////////
	//Write new code!
	flashPointer = (uint8_t *)startAddress;
	//Jump magic code
	flashPointer++;
	//Read number chunks
	uint8_t nchunks = *flashPointer;
	flashPointer++;
	uint32_t addressInstall;
	uint32_t sizeInstall;

	volatile uint8_t *flashPointerInstall, *flashPointerRead;
	//uint32_t j;
	uint16_t sizeInstall16;

	flashPointerRead = (uint8_t *)startAddress;
	flashPointerRead += 2;
	for(i = 0; i < nchunks; i++)
	{
		flashPointerRead += 10 ;
	}

	for(i = 0; i < nchunks; i++)
	{
		bootloaderUChar2ULong(flashPointer, &addressInstall);
		flashPointer+=4;

		bootloaderUChar2ULong(flashPointer, &sizeInstall );
		sizeInstall16 = (uint16_t) sizeInstall;
		flashPointer+=6;	//Ignore CRC

		flashPointerInstall =  (uint8_t *)addressInstall;

		//I can't use the following because it is 32bit and uses a function on flash;
		//flashPointerRead = (uint8_t *)(startAddress + 1 + 1 + 10 * nchunks + dataCounter);

		uint16_t j = 0;

		for(j = 0; j < sizeInstall16; j++)
		{
			//Wait while busy
			while(FCTL3&BUSY);
			uint8_t byteNew = *flashPointerRead;
			*flashPointerInstall = byteNew;

			flashPointerInstall += 1;
			flashPointerRead += 1;
		}

		while(FCTL3 & BUSY);
	}

	//Relock memory flags:
	// Clear delete flags
	FCTL1 = FWKEY;
	FCTL3 = FWKEY + LOCKA + LOCK;

	//PUC Reboot writting incorrect value to WDT
	WDTCTL = 0xDEAD;
}


