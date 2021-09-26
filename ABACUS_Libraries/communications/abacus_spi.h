/*
 * abacus_spi.h
 *
 *  Created on: 06/mar/2014
 *      Author: PC12
 */

#ifndef ABACUS_SPI_H_
#define ABACUS_SPI_H_

#include "msp430x54xa.h"
#include "stdint.h"
#include "../abacus.h"
#include "../abacus_utils.h"


#define SPIBUFFERLENGHT 256

struct SPIPort
{
	uint8_t spi_name;
	uint8_t lastAddress;
	uint8_t isOnError;
};


extern struct SPIPort spi00;
extern struct SPIPort spi01;

void abacus_spi_init();

int8_t abacus_spi_write_instruction(uint8_t busSelect, uint8_t instruction);
int8_t abacus_spi_write_read(uint8_t busSelect,
							 uint8_t *bufferOut, unsigned int bufferOutLenght,
							 uint8_t *bufferIn,  unsigned int bufferInLenght);
uint16_t abacus_spi_write_read_calculateCRC16(uint8_t busSelect,
							 uint8_t *bufferOut, unsigned int bufferOutLenght,
							 uint16_t *crc16,  unsigned long bufferInLenght);

uint16_t abacus_spi_write_read_calculateCRC16_kickwdt(uint8_t busSelect,
							 uint8_t *bufferOut, unsigned int bufferOutLenght,
							 uint16_t *crc16,  unsigned long bufferInLenght,
							 uint16_t wdt_command);

#endif /* ABACUS_SPI_H_ */
