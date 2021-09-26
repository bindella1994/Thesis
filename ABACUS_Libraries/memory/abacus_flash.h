/*
 * abacus_flash.h
 *
 */

#ifndef ABACUS_FLASH_H_
#define ABACUS_FLASH_H_

#include "msp430x54xa.h"
#include "stdint.h"
#include "../abacus.h"

#define FLASH_WREN 0x06
#define FLASH_WRDI 0x04
#define FLASH_RDID 0x9F
#define FLASH_RDSR 0x05
#define FLASH_WRSR 0x01
#define FLASH_READ 0x03
#define FLASH_FAST_READ 0x0B
#define FLASH_PP 0x02
#define FLASH_SE 0xD8
#define FLASH_BE 0xC7


struct FlashMemory
{
	uint8_t flash_name;
	uint8_t flash_identification[3];

	uint8_t lastAddress;
	uint8_t isOnError;
};


extern struct FlashMemory memoryFlashMCU;
extern struct FlashMemory memoryFlashFPGA;

void abacus_flashMCU_init();
void abacus_flashFPGA_init();

int8_t abacus_flash_write_instruction(uint8_t memorySelect,
		uint8_t instruction);

int8_t abacus_flash_write_read_instruction(uint8_t memorySelect,
		uint8_t *instruction,
		uint16_t instructionLenght,
		uint8_t *data,
		uint16_t dataLenght);

int8_t abacus_flash_isWorkInProgress(uint8_t memorySelect);

int8_t abacus_flash_unprotectMemory(uint8_t memorySelect);

int8_t abacus_flash_write_data(uint8_t memorySelect,
		uint32_t address,
		uint8_t *data,
		uint32_t lenght);

int8_t abacus_flash_write_dataEasy(uint8_t memorySelect,
		uint32_t address,
		uint8_t *data,
		uint32_t lenght);

int8_t abacus_flash_wait_while_busy(uint8_t memorySelect,
		uint16_t cycles,
		int8_t toInfinity);

int8_t abacus_flash_read_data(uint8_t memorySelect,
		uint32_t address,
		uint8_t *data,
		uint32_t lenght);

uint16_t abacus_flash_crc(uint8_t memorySelect,
		uint32_t startAddress,
		uint32_t lenght);

uint16_t abacus_flash_crc_kickwdt(uint8_t memorySelect,
		uint32_t startAddress,
		uint32_t lenght,
		uint16_t wdt_command);

int8_t abacus_flash_dump_memory(uint8_t memorySelect,
		uint8_t uart,
		uint32_t startAddress,
		uint32_t lenght);

int8_t abacus_flash_sector_erase(uint8_t memorySelect,
		uint32_t address);
int8_t abacus_flash_bulk_erase(uint8_t memorySelect);


#endif /* ABACUS_FLASH_H_ */
