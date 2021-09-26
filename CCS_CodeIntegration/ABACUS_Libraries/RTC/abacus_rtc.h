/*
 * abacus_rtc.h
 *
 */

#ifndef ABACUS_RTC_H_
#define ABACUS_RTC_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

#define AB_ADDRESS_RTC 0x68  //RTC address

//BQ32000 USEFUL REGISTERS
#define BQ32000SECONDS 0x00
#define BQ32000CFG2 0x09
#define BQ32000TCH2 0x08
#define BQ32000SFR 0x22


int8_t abacus_RTC_init();
int8_t abacus_RTC_setTrickleCharge(uint8_t enable);

unsigned long abacus_RTC_getUnixTime();
int8_t abacus_RTC_getTime(uint8_t *year,
						  uint8_t *month,
						  uint8_t *day,
						  uint8_t *hour,
						  uint8_t *minute,
						  uint8_t *second);

int8_t abacus_RTC_setUnixTime(unsigned long time);
int8_t abacus_RTC_setTime(uint8_t year,
						  uint8_t month,
						  uint8_t day,
						  uint8_t hour,
						  uint8_t minute,
						  uint8_t second);


unsigned long abacus_convertToUnixTime(uint8_t year,
									 uint8_t month,
									 uint8_t day,
									 uint8_t hour,
									 uint8_t minute,
									 uint8_t second);

void abacus_convertFromUnixTime(unsigned long unixtime,
								uint8_t *year,
								uint8_t *month,
								uint8_t *day,
								uint8_t *hour,
								uint8_t *minute,
								uint8_t *second);


#endif /* ABACUS_RTC_H_ */
