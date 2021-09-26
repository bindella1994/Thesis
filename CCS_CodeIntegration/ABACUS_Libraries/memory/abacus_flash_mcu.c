/*
 * abacus_flash_mcu.c
 *
 */

#include "abacus_flash_mcu.h"

/**
 * Addresses go for FLASH from:
 * 	 0x005C00 to 0x00FFFF -> Bank Aa
 * 	 0x040000 to 0x045BFF -> Bank Ab
 * 	 0x010000 to 0x01FFFF -> Bank B
 * 	 0x020000 to 0x02FFFF -> Bank C
 * 	 0x030000 to 0x03FFFF -> Bank D
 *
 * Addresses go for RAM from:
 * 	 0x001C00 to 0x005BFF
 *
 * Addresses go for INO C-D
 */
int8_t abacus_flash_mcu_checkValidAddress(uint32_t address)
{
    //INFO C-D
    if (address >= 0x001800 && address <= 0x0018FF)
        return 2;

    //RAM
    if (address >= 0x001C00 && address <= 0x005BFF)
        return 1;

    //FLASH first bank Aa
    if (address >= 0x5c00 && address <= 0xffff)
        return 1;

    //FLASH rest of banks
    if (address >= 0x010000 && address <= 0x045bff)
        return 1;

    return 0;
}

/**
 * It returns 1 if address is in bank A. 0 otherwise
 */
uint8_t abacus_flash_mcu_isBankAAddress(uint32_t address)
{
    //FLASH firs bank Aa
    if (address >= 0x5c00 && address <= 0xffff)
        return 1;

    if (address >= 0x040000 && address <= 0x045bff)
        return 1;

    return 0;
}

/**
 * It writes data to the desired address. It will write in byte mode for
 * simplicity shake, however it is twice slower than in word mode.
 * It returns -1 if start address or end address are out of possible address
 * It returns -2 if start bank is different from end bank (only bank A applies)
 * Careful with WDT when using this function
 */
int8_t abacus_flash_mcu_write_data(uint32_t address, uint8_t *data,
                                   uint32_t lenght)
{
    //Make sure it is a valid address
    if (abacus_flash_mcu_checkValidAddress(address) == 0)
        return -1;
    if (abacus_flash_mcu_checkValidAddress(address + lenght) == 0)
        return -1;

    uint32_t i = 0;

    //We don't like that we start on Bank A and end in anotherone?
    uint8_t isBankA = abacus_flash_mcu_isBankAAddress(address);
    if (isBankA != 0 && abacus_flash_mcu_isBankAAddress(address + lenght) == 0)
        return -2;

    if (isBankA == 0 && abacus_flash_mcu_isBankAAddress(address + lenght) != 0)
        return -2;

    //BankA needs extra handling
    if (isBankA != 0)
        if ((FCTL3 & LOCKA) != 0)
            // Clear LOCK & set LOCKA
            FCTL3 = FWKEY + LOCKA;
        else
            FCTL3 = FWKEY;
    else
        FCTL3 = FWKEY;

    //WRT flag
    FCTL1 = FWKEY + WRT;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) address;

    for (i = 0; i < lenght; i++)
    {
        //write byte to flash
        *flashPointer = data[i];
        //Move to next byte
        flashPointer++;
    }

    // Clear delete flags
    FCTL1 = FWKEY;

    if (isBankA != 0)
        // Set LOCK & LOCKA bit
        if ((FCTL3 & LOCKA) == 0)
            FCTL3 = FWKEY + LOCKA + LOCK;
        else
            FCTL3 = FWKEY + LOCK;
    else
        FCTL3 = FWKEY + LOCK;

    return 0;
}

int8_t abacus_flash_mcu_wait_while_busy()
{
    while ((FCTL3 & BUSY) != 0){
        //waiting
    }
    return 0;

}

/**
 * It reads data from the mcu flash or RAM memory,
 * it returns -1 if address is out of boundaries.
 */
int8_t abacus_flash_mcu_read_data(uint32_t address, uint8_t *data,
                                  uint32_t lenght)
{
    //Make sure it is a valid address
    //if(abacus_flash_mcu_checkValidAddress(address) == 0)
    //	return -1;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) address;

    uint32_t i;

    for (i = 0; i < lenght; i++)
    {
        data[i] = *flashPointer;
        flashPointer++;
    }
    return 0;
}

/**
 * It calculates the CRC of a specified memory lenght.
 * It does not kick the WDT, so be careful.
 */
int16_t abacus_flash_mcu_crc(uint32_t startAddress, uint32_t lenght)
{
    //Make sure it is a valid address
    //if(abacus_flash_mcu_checkValidAddress(startAddress) == 0)
    //	return -1;

    uint16_t crc;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) startAddress;

    uint32_t i;
    uint8_t resetCRC = 1;

    for (i = 0; i < lenght; i++)
    {
        uint8_t data = *flashPointer;
        flashPointer++;
        //Calculate crc16:
        calculate_crc16_8bit(&data, 1, &crc, resetCRC);
        //Only first time is needed to reset
        resetCRC = 0;
    }

    return crc;

}

/**
 * It calculates the CRC of a specified memory lenght.
 * It kicks the WDT! you have to send the WDT command to kick it
 */
int16_t abacus_flash_mcu_crc_kickwdt(uint32_t startAddress, uint32_t lenght,
                                     uint16_t wdt_command)
{
    //Make sure it is a valid address
    //if(abacus_flash_mcu_checkValidAddress(startAddress) == 0)
    //	return -1;

    uint16_t crc;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) startAddress;

    uint32_t i;
    uint8_t resetCRC = 1;

    for (i = 0; i < lenght; i++)
    {
        uint8_t data = *flashPointer;
        flashPointer++;
        //Calculate crc16:
        calculate_crc16_8bit(&data, 1, &crc, resetCRC);
        //Only first time is needed to reset
        resetCRC = 0;
        //kick the infamous WDT
        WDTCTL = wdt_command;
    }

    return crc;
}

/**
 * It dumps the contents from the selected to the selected UART
 */
int8_t abacus_flash_mcu_dump_memory(uint8_t uart, uint32_t startAddress,
                                    uint32_t lenght)
{
    //Make sure it is a valid address
    //if(abacus_flash_mcu_checkValidAddress(startAddress) == 0)
    //	return -1;

    uint8_t *flashPointer;
    uint32_t endPoint = startAddress + lenght;
    flashPointer = (uint8_t*) startAddress;
    if (lenght == 0)
        endPoint = 0x0045bff;

    uint8_t buffer[1];
    uint32_t pointer = startAddress;
    while (pointer < endPoint)
    {
        buffer[0] = *flashPointer;
        abacus_uart_write(uart, buffer, 1);
        flashPointer++;
        pointer++;
    }
    return 0;
}

/**
 * It will erase the memory in segments of 512 bytes, no Bank or Mass erase
 * supported on libraries as those operations must be handled with great care
 * WDT is switched of during this operation. Only valid memory addresses.
 * Careful with WDT when using this function
 */
int8_t abacus_flash_mcu_erase(uint32_t address, uint32_t size)
{
    //Make sure it is a valid address
    if (abacus_flash_mcu_checkValidAddress(address) == 0)
        return -1;

    uint32_t i = 0;

    //Sync start address to segment start:
    uint32_t addressSync = (address / 512);
    addressSync = addressSync * 512;

    //Retune size
    size = size + address - addressSync;

    //How many segments to delete:
    uint32_t iterations = size / 512 + 1;

    if (size % 512 == 0)
        iterations--;

    //BankA needs extra handling
    uint8_t isBankA = abacus_flash_mcu_isBankAAddress(addressSync);

    if (isBankA != 0)
        if ((FCTL3 & LOCKA) != 0)
            // Clear LOCK & set LOCKA, locka is a toggle bit, careful handling!
            FCTL3 = FWKEY + LOCKA;
        else
            FCTL3 = FWKEY;
    else
        FCTL3 = FWKEY;

    //ERASE bit to 1 MERAS to 0
    FCTL1 = FWKEY + ERASE;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) addressSync;

    for (i = 0; i < iterations; i++)
    {
        //Dummy write, code is stopped until delete is completed
        *flashPointer = 0;
        //Move to next segment
        flashPointer += 512;
        //ERASE bit to 1 MERAS to 0
        FCTL1 = FWKEY + ERASE;
    }

    // Clear delete flags
    FCTL1 = FWKEY;

    if (isBankA != 0)
        // Set LOCK & LOCKA bit
        if ((FCTL3 & LOCKA) == 0)
            FCTL3 = FWKEY + LOCKA + LOCK;
        else
            FCTL3 = FWKEY + LOCK;
    else
        FCTL3 = FWKEY + LOCK;

    return 0;
}

/**
 * It will erase the memory in segments of 512 bytes, no Bank or Mass erase
 * supported on libraries as those operations must be handled with great care
 * WDT is switched of during this operation. Only valid memory addresses.
 * Careful with WDT when using this function
 */
int8_t abacus_infoflash_mcu_erase(uint32_t address)
{
    //Make sure it is a valid address
    if (abacus_flash_mcu_checkValidAddress(address) != 2)
        return -1;

    //uint32_t i = 0;

    //Sync start address to segment start:
    uint32_t addressSync = (address / 128);
    addressSync = addressSync * 128;

    FCTL3 = FWKEY;

    //ERASE bit to 1 MERAS to 0
    FCTL1 = FWKEY + ERASE;

    uint8_t *flashPointer;
    flashPointer = (uint8_t*) addressSync;

    //Dummy write, code is stopped until delete is completed
    *flashPointer = 0;

    //ERASE bit to 1 MERAS to 0
    FCTL1 = FWKEY + ERASE;

    // Clear delete flags
    FCTL1 = FWKEY;

    FCTL3 = FWKEY + LOCK;

    return 0;
}

