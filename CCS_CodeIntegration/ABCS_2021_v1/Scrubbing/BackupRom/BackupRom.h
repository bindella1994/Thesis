/*
 * PersistentRom.h
 *
 *  Created on: 27 giu 2021
 *      Author: Manuel-Desktop
 */

#ifndef BACKUPROM_H_
#define BACKUPROM_H_

#include "../PersistentRam/PersistentRam.h"
#include "memory/abacus_flash.h"
#include "memory/abacus_flash_mcu.h"
#include "abacus_utils.h"
struct RomBackup
{
    uint8_t persistentRam[PERSISTENT_RAM_LENGTH];
    uint16_t crc16BackupData;
}romBackup;

#define ROM_BACKUP_STARTING_POINTER 0x28000
#define ROM_BACKUP_BLOCK_SIZE 512

extern struct RamBackup ramBackup_;

void setCrcInRomBackup();

int16_t getCrcInRomBackup();

uint8_t backupDataInRomBackup(uint32_t dataStartingAddress, uint32_t backupStartingPointer, uint32_t size);

uint8_t restoreDataFromRomBackup(uint32_t dataStartingAddress, uint32_t backupStartingPointer, uint32_t size);

uint8_t isRomBackupUncorructed(uint32_t dataStartingAddress, uint32_t size);

uint8_t isRomBackupPossible(uint32_t dataStartingAddress, uint32_t size);

#endif /* BACKUPROM_H_ */
