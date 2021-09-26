/*
 * abacus_rtc_internal.c
 */

#include "abacus_rtc_internal.h"


/*
 * It returns whether the internal RTC is On or not.
 */
uint8_t abacus_RTC_internal_isOn()
{
	return ((RTCCTL01 & RTCMODE)>>8);
}

/*
 * It initiates the internal RTC. Input options are:
 * - CALENDAR: what?
 */
void abacus_RTC_internal_init(uint8_t mode)
{
	switch(mode)
	{
	case CALENDAR:
		RTCCTL01 = RTCMODE;
		break;
	default:
		break;
	}
}

/*
 * It stops the internal RTC
 */
void abacus_RTC_internal_switchOff()
{
	RTCCTL01 = RTCHOLD;
}

/*
 * It returns the date & time of the internal RTC
 */
void abacus_RTC_internal_readCalendar(uint8_t *year,
									  uint8_t *month,
									  uint8_t *day,
									  uint8_t *hour,
									  uint8_t *minute,
									  uint8_t *second)
{
	*second = RTCSEC;
	*minute = RTCMIN;
	*hour = RTCHOUR;
	*day = RTCDAY;
	*month = RTCMON;
	*year = RTCYEARL;
}

/*
 * Synchronizes the internal RTC with external rtc
 */
void abacus_RTC_internal_sincRTC()
{
	uint8_t sec;
	uint8_t min;
	uint8_t hou;
	uint8_t day;
	uint8_t mon;
	uint8_t yea;

	if(abacus_RTC_getTime(&yea, &mon, &day,  &hou, &min, &sec) == 0)
	{
		//Sync only if external RTC replied with ACK :-D
		RTCSEC = sec;
		RTCMIN = min;
		RTCHOUR = hou;
		RTCDAY = day;
		RTCMON = mon;
		RTCYEARL = yea;
	}

}

/*
 * It sets the internal RTC time in Unix Time format
 */
void abacus_RTC_internal_setUnixTime(unsigned long time)
{
	uint8_t year, month, day, hour, minute, second;

	//Convert from Unixtime to human readable format
	abacus_convertFromUnixTime(time,
			&year,
			&month,
			&day,
			&hour,
			&minute,
			&second);

	RTCSEC = second;
	RTCMIN = minute;
	RTCHOUR = hour;
	RTCDAY = day;
	RTCMON = month;
	RTCYEARL = year;

}

/*
 * It returns the RTC internal date/time in Unix time format
 */
unsigned long abacus_RTC_internal_getUnixTime()
{
	return abacus_convertToUnixTime(RTCYEARL,
			RTCMON,
			RTCDAY,
			RTCHOUR,
			RTCMIN,
			RTCSEC);
}

