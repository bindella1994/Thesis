/*
 * abacus_flash_mcu.h
 */

#ifndef ABACUS_FLASH_MCU_H_
#define ABACUS_FLASH_MCU_H_

#include "msp430x54xa.h"
#include "stdint.h"
#include "../abacus.h"

int8_t abacus_flash_mcu_checkValidAddress(uint32_t address);

int8_t abacus_flash_mcu_write_data(uint32_t address,
		uint8_t *data,
		uint32_t lenght);

int8_t abacus_flash_mcu_wait_while_busy();

int8_t abacus_flash_mcu_read_data(uint32_t address,
		uint8_t *data,
		uint32_t lenght);

int16_t abacus_flash_mcu_crc(uint32_t startAddress,
		uint32_t lenght);

int16_t abacus_flash_mcu_crc_kickwdt(uint32_t startAddress,
		uint32_t lenght,
		uint16_t wdt_command);

int8_t abacus_flash_mcu_dump_memory(uint8_t uart,
		uint32_t startAddress,
		uint32_t lenght);

int8_t abacus_flash_mcu_erase(uint32_t address,
		uint32_t size);

int8_t abacus_infoflash_mcu_erase(uint32_t address);



#endif /* ABACUS_FLASH_MCU_H_ */
