/*
 * PersistentRom.c
 *
 *  Created on: 27 giu 2021
 *      Author: Manuel-Desktop
 */

#include "BackupRom.h"
#include "abacus.h"
#include <math.h>
void setCrcInRomBackup()
{

}
int16_t getCrcInRomBackup()
{
    return romBackup.crc16BackupData;
}
uint8_t backupDataInRomBackup(uint32_t dataStartingAddress, uint32_t backupStartingPointer, uint32_t size)
{
    uint8_t result = 0x00;

    if(abacus_flash_mcu_erase(backupStartingPointer, size))
    {
        abacus_flash_write_data(AB_FLASH_MCU,(uint32_t)backupStartingPointer,(uint8_t*)dataStartingAddress,size);
        uint16_t* crcValue=&romBackup.crc16BackupData;
        calculate_crc16((uint16_t*)backupStartingPointer, size, crcValue, 1);

        result=0xff;


    }
    return result;
}

uint8_t restoreDataFromRomBackup(uint32_t dataStartingAddress, uint32_t backupStartingPointer, uint32_t size)
{
    uint8_t result = 0x00;
    uint16_t crcValue=0xffff;
    calculate_crc16((uint16_t*)backupStartingPointer, size, (uint16_t*)&crcValue, 1);

    if(crcValue==romBackup.crc16BackupData)
    {
        uint8_t* data = (uint8_t*)dataStartingAddress;
        abacus_flash_read_data(AB_FLASH_MCU, dataStartingAddress, data, PERSISTENT_RAM_LENGTH);


        result=0xff;
    }

    return result;
}

uint8_t isRomBackupUncorructed(uint32_t dataStartingAddress, uint32_t size)
{
    uint8_t result = 0x00;
    uint16_t crcValue=0xffff;
    calculate_crc16((uint16_t*)dataStartingAddress, size, (uint16_t*)&crcValue, 1);

    if(crcValue==romBackup.crc16BackupData)
    {
        result=0xff;
    }
    return result;
}

uint8_t isRomBackupPossible(uint32_t dataStartingAddress, uint32_t size)
{
    return ( (romBackup.crc16BackupData != 0x00) && isRomBackupUncorructed(dataStartingAddress,size) );
}
