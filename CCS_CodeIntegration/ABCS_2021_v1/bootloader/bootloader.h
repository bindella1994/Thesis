
#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define BOOTLOADER_RAM_START_ADD 0x5900
#define BOOTLOADER_FLASH_START_ADD 0xFC80
#define BOOTLOADER_RAM_SIZE 0x0300
#define MCUFLASHBANKA 0x005c00
#define MCUFLASHBANKB 0x010000


#define MCUMEMORY_CONF_ADD         0x040000
#define MCUMEMORY_CONF_SIZE        0x001000
#define MCUMEMORY_PROGRAMINTEGRITY_ADD 0X041000

#define MEMORY_DOWNLOAD_ADD     0x030000
#define MEMORY_BACKUP_ADD       0x020000

#define MEMORY_PROGRAM_A_ADD    0x005C00
#define MEMORY_PROGRAM_A_SIZE   0x00A400
#define MEMORY_PROGRAM_B_ADD    0x010000
#define MEMORY_PROGRAM_B_SIZE   0x00FFFF

#define MEMORY_MAGICWORD        0x07


#include "memory/abacus_flash.h"
#include "memory/abacus_flash_mcu.h"
#include "abacus_utils.h"

int8_t bootloaderStart();

void bootloaderTriggerRam(uint32_t startAddress, uint8_t simulation);
void bootloaderFlash(uint32_t startAddress, uint8_t simulation);

//Nuovo
// WDT for reset
#define WDT_MRST_LONG       (WDTCNTCL + WDTIS1 + WDTIS0)    // 500ms
#define WDT_MRST_EXTRALONG  (WDTCNTCL + WDTIS1)             // 8s
#define WDT_MRST_XXL        (WDTCNTCL + WDTIS0)             // 128s
/*
#define AB_ADDRESS_ACC_ID               0xE3
#define AB_ADDRESS_GYR_ID               0xD3                //11010011 saranno bit o byte?


#define AB_ADDRESS_A_MAG_ID              0x48             //0 1 0 0 1 0 0 0
#define AB_ADDRESS_B_MAG_ID              0x34              //0 0 1 1 0 1 0 0
#define AB_ADDRESS_C_MAG_ID              0x33              //0 0 1 1 0 0 1 1


// RTC IS 8 BYTE ID AND IS DIFFERENT FOR 2 VERSION, I WILL WRITE BOTH OF THEM AND WE SHOULD DECOMMENT THE RIGHT ONE
//EUI-48™ Node Address 0xff 0xff 0x00 0x04 0xA3 0x12 0x34 0x56
#define AB_ADDRESS_A_RTC_ID              0xff
#define AB_ADDRESS_B_RTC_ID              0xff
#define AB_ADDRESS_C_RTC_ID              0x00
#define AB_ADDRESS_D_RTC_ID              0x04
#define AB_ADDRESS_E_RTC_ID              0xA3
#define AB_ADDRESS_F_RTC_ID              0x12
#define AB_ADDRESS_G_RTC_ID              0x34
#define AB_ADDRESS_H_RTC_ID              0x56
*/
/*
//EUI-64™ Node Address: 0x00 0x04 0xA3 0x12 0x34 0x56 0x78 0x9A
#define AB_ADDRESS_A_RTC_ID              0x00
#define AB_ADDRESS_B_RTC_ID              0x04
#define AB_ADDRESS_C_RTC_ID              0xA3
#define AB_ADDRESS_D_RTC_ID              0x12
#define AB_ADDRESS_E_RTC_ID              0x34
#define AB_ADDRESS_F_RTC_ID              0x56
#define AB_ADDRESS_G_RTC_ID              0x78
#define AB_ADDRESS_H_RTC_ID              0x9A
*/




#endif /* BOOTLOADER_H_ */
