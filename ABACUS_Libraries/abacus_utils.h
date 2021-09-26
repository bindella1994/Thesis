/*
 * abacus_utils.h
 */

#ifndef ABACUS_UTILS_H_
#define ABACUS_UTILS_H_

#include <msp430.h>
#include "stdint.h"

void char2int(uint8_t *input, int *output);
void char2uint(uint8_t *input, unsigned int *output);
void char2ulong(uint8_t *input, unsigned long *output);
void ulong2char(unsigned long *input, uint8_t *output);
void uint2char(unsigned int *input, uint8_t *output);
void int2char(int *input, uint8_t *output);
uint16_t calculate_crc8(uint8_t *p, uint8_t len, uint16_t *crc);
int16_t calculate_crc16(uint16_t *p, uint16_t len, uint16_t *crc, uint8_t init);
int16_t calculate_crc16_8bit(uint8_t *p, uint16_t len, uint16_t *crc, uint8_t init);


#endif /* ABACUS_UTILS_H_ */
