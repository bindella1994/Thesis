/*
 * abacus_rtc.c
 *
 */

 #include "abacus_rtc.h"


/*
 * Initialization of the RTC
 */
int8_t abacus_RTC_init()
{
	//Try to contact RTC
	//TODO
	//Get the date of the RTC
	//TODO
	//Was the date reset to 0-0-0 00:00:00 ?
	//TODO

	return 0;
}

/*
 * Enables or disables the trickleCharge of the RTC. Enable only if you are
 * keeping the time using a capacitor, supercap or rechargable battery.
 * Disable if if you are using a non rechargeable battery!
 */
int8_t abacus_RTC_setTrickleCharge(uint8_t enable)
{
	uint8_t i2cError = 0;
	uint8_t buffer[2];
	if(enable == 0)
	{
		//Disable trickleCharge

		//Disable TCH2 witch
		buffer[0] = 0x08;	//Register address
		buffer[1] = 0x00;	//Register data
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

		//Disable TCHE switch
		buffer[0] = 0x09;	//Register address
		buffer[1] = 0x0A;	//Register data
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	}
	else
	{
		//Enable trickleCharge

		//Enable TCHE switch
		buffer[0] = 0x09;	//Register address
		buffer[1] = 0x05;	//Register data
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

		//Enable TCH2 witch
		buffer[0] = 0x08;	//Register address
		buffer[1] = 0x20;	//Register data
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);
	}

	//Return i2c accumulated errors if any
	return i2cError;
}

/*
 * It interrogates the RTC and returns the time reported by the RTC
 */
int8_t abacus_RTC_getTime(uint8_t *year,
						  uint8_t *month,
						  uint8_t *day,
						  uint8_t *hour,
						  uint8_t *minute,
						  uint8_t *second)
{
	uint8_t buffer[2];
	uint8_t i2cError = 0;

	//All dates and times are encoded in BCD, so tricky conversions ahead ;)

	//Get year:
	buffer[0] = 0x06;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*year = (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x0F);

	//Get month:
	buffer[0] = 0x05;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*month = (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x01);

	//Get day of month:
	buffer[0] = 0x04;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*day= (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x03);

	//Get hour:
	buffer[0] = 0x02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*hour= (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x03);

	//Get minutes:
	buffer[0] = 0x01;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*minute= (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x07);

	//Get seconds:
	buffer[0] = 0x00;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	i2cError += abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 1, 0);
	*second= (buffer[0] & 0x0F) + 10 * ((buffer[0] >> 4) & 0x07);

	//Return i2c accumulated errors if any
	return i2cError;
}

/*
 * It interrogates the RTC and returns the time reported by the RTC in
 * UnixTime format.
 */
unsigned long abacus_RTC_getUnixTime()
{
	uint8_t year, month, day, hour, minute, second;

	//Get RTC time
	abacus_RTC_getTime(&year,
					   &month,
					   &day,
					   &hour,
					   &minute,
					   &second);

	//Convert and return from date to UnixTime:

	return abacus_convertToUnixTime(year,
					   month,
					   day,
					   hour,
					   minute,
					   second);
}

/*
 * It sets the new RTC time from a defined UnixTime
 */
int8_t abacus_RTC_setUnixTime(unsigned long time)
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

	//Send date to RTC

	return abacus_RTC_setTime(year,
						   month,
						   day,
						   hour,
						   minute,
						   second);
}

/*
 * It sets the new RTC time from the variables
 */
int8_t abacus_RTC_setTime(uint8_t year,
						  uint8_t month,
						  uint8_t day,
						  uint8_t hour,
						  uint8_t minute,
						  uint8_t second)
{
	uint8_t buffer[2], digit01, digit02;
	uint8_t i2cError = 0;

	//All dates and times are encoded in BCD, so tricky conversions ahead ;)

	//Set year:
	buffer[0] = 0x06;
	digit01 = (((year / 10) & 0x0F) << 4);
	digit02 = (year % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Set month:
	buffer[0] = 0x05;
	digit01 = (((month / 10) & 0x01) << 4);
	digit02 = (month % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Set day of month:
	buffer[0] = 0x04;
	digit01 = (((day / 10) & 0x03) << 4);
	digit02 = (day % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Set hour:
	buffer[0] = 0x02;
	digit01 = (((hour / 10) & 0x03) << 4);
	digit02 = (hour % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Set minutes:
	buffer[0] = 0x01;
	digit01 = (((minute / 10) & 0x07) << 4) ;
	digit02 = (minute % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Set seconds:
	buffer[0] = 0x00;
	digit01 = (((second / 10) & 0x07) << 4);
	digit02 = (second % 10) & 0x0F;
	buffer[1] = digit01 + digit02;
	i2cError += abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_RTC, buffer, 2, 0);

	//Return i2c accumulated errors if any
	return i2cError;
}




///////////////////////////////////////////////////////////////////////////////
// Functions to handle Unix Time format times with human readable versions

//Leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

/*
 * Calculates the Unixtime of the selected date
 */
unsigned long abacus_convertToUnixTime(uint8_t year,
									 uint8_t month,
									 uint8_t day,
									 uint8_t hour,
									 uint8_t minute,
									 uint8_t second)
{
	// assemble time elements into unsigned long

	if(year > 69)
		year = year - 70;
	else
		year = year + 30;

	int i;
	unsigned long seconds;
	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds = year * (SECS_PER_DAY * 365);
	for (i = 0; i < year; i++)
	{
		if (LEAP_YEAR(i))
			seconds +=  SECS_PER_DAY;   // add extra days for leap years
	}

	// add days for this year, months start from 1
	for (i = 1; i < month; i++)
	{
		if ( (i == 2) && LEAP_YEAR(year))
			seconds += SECS_PER_DAY * 29;
		else
			seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
	}

	seconds += (day - 1) * SECS_PER_DAY;
	seconds += hour * SECS_PER_HOUR;
	seconds += minute * SECS_PER_MIN;
	seconds += second;

	return seconds;
}


/*
 * Break the given unixtime into time components
 * This is a more compact version of the C library localtime function
 * Note that year is offset from 1970 !!!
 * Source: http://forum.arduino.cc/index.php?topic=56310.5
 */
void abacus_convertFromUnixTime(unsigned long unixtime,
								uint8_t *year,
								uint8_t *month,
								uint8_t *day,
								uint8_t *hour,
								uint8_t *minute,
								uint8_t *second)
{
	uint8_t monthLength;

	*second = unixtime % 60;
	unixtime /= 60; // now it is minutes
	*minute = unixtime % 60;
	unixtime /= 60; // now it is hours
	*hour = unixtime % 24;
	unixtime /= 24; // now it is days
	//uint8_t Wday = ((unixtime + 4) % 7) + 1;  // Sunday is day 1

	uint8_t years = 0;
	unsigned long days = 0;
	while((unsigned)(days += (LEAP_YEAR(years) ? 366 : 365)) <= unixtime)
	{
		years++;
	}
	// years is offset from 1970

	days -= LEAP_YEAR(years) ? 366 : 365;
	unixtime -= days; // now it is days in this year, starting at 0

	days = 0;
	*month = 0;
	monthLength = 0;
	for (*month = 0; *month < 12; *month = *month + 1)
	{
		if (*month == 1)
		{
			// February
			if (LEAP_YEAR(years))
				monthLength = 29;
			else
				monthLength = 28;
		}
		else
			monthLength = monthDays[*month];

		if (unixtime >= monthLength)
			unixtime -= monthLength;
		else
			break;
	}
	if(years < 30)
		*year = years + 70;
	else
		*year = years - 30;

	*month = *month + 1;  // jan is month 1
	*day = unixtime + 1;     // day of month
}
