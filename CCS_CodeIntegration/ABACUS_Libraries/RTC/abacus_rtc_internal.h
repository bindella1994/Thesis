/*
 * abacus_rtc_internal.h
 */

#ifndef ABACUS_RTC_INTERNAL_H_
#define ABACUS_RTC_INTERNAL_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "abacus_rtc.h"
#include "stdint.h"

#define CALENDAR 1

uint8_t abacus_RTC_internal_isOn();
void abacus_RTC_internal_init(uint8_t mode);
void abacus_RTC_internal_switchOff();
void abacus_RTC_internal_readCalendar(	uint8_t *year,
										uint8_t *month,
										uint8_t *day,
										uint8_t *hour,
										uint8_t *minute,
										uint8_t *second);
void abacus_RTC_internal_sincRTC();

void abacus_RTC_internal_setUnixTime(unsigned long time);
unsigned long abacus_RTC_internal_getUnixTime();

//DA FARE
//void abacus_RTC_internal_checkShift();

#endif /* ABACUS_RTC_INTERNAL_H_ */
