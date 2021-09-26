/*
 * Scrubbing.c
 *
 *  Created on: 22 ago 2021
 *      Author: bindella
 */

#include "PersistentRam/PersistentRam.h"
#include "BackupRom/BackupRom.h"
#include "satellite/satsystem_init.h"

#define ROM_BACKUP_AVAILABLE 0
uint8_t scrubbingRoutine()
{

    uint32_t i;
    uint8_t firstCondition = 0;
    for ( i=0; i<PERSISTENT_RAM_LENGTH/8; ++i) {
        if(scrub_recovery((uint8_t*)ramBackup_.persistentRam[8*i],i)!=0x0) {
            firstCondition = 0xff;
            break;
        }
    }
    if( firstCondition && ramBackup_.isPersistentRamWritten ) {
        restoreFromPersistentRam((uint8_t*)&ramBackup_.persistentRam[0],(uint8_t*)&satelliteStatus_,PERSISTENT_RAM_LENGTH);
    }
    else
    {
        /*
#if ROM_BACKUP_AVAILABLE
        if( isRomBackupPossible((uint32_t)ROM_BACKUP_STARTING_POINTER,(uint32_t)PERSISTENT_RAM_BYTE_SIZE) )
        {
            restoreDataFromRomBackup((uint32_t)MEMORY_TO_BACKUP_STARTING_POINTER, (uint32_t)ROM_BACKUP_STARTING_POINTER, (uint32_t)PERSISTENT_RAM_BYTE_SIZE);
        }
#endif
*/
        backupInPersistentRam((uint8_t*)&ramBackup_.persistentRam[0], (uint8_t*)&satelliteStatus_, PERSISTENT_RAM_LENGTH);
    }

    return 0;
}


uint8_t periodicScrubPersistentRAM(){
    uint32_t r;
    for ( r = 0; r< PERSISTENT_RAM_LENGTH/8 ; ++r ) {
        scrub_recovery((uint8_t*)&ramBackup_.persistentRam[8*r],PERSISTENT_RAM_LENGTH/8);
    }
}
