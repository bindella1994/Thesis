/*
 * telemetry.c
 */

#include "telemetry.h"

struct SatelliteTelemetry satelliteTelemetry_;

/*
 * It generates a 46 bytes telemetry
 */
void checkTelemetry()
{
	//Check if we have to read the telemetry status:
	uint32_t timeNow = abacus_millis();
	if(		(satelliteStatus_.telemetryLastTimeRead
			+ (uint32_t)satelliteConfiguration_.telemetryReadInterval * 1000UL)
					< timeNow)
		//Time to read Sensors
		updateSensors();

	//Time to save them to memory?
	if(		(satelliteStatus_.telemetryLastTimeSaved
				+ (uint32_t)satelliteConfiguration_.telemetrySaveInterval * 1000UL)
						< timeNow)
		//Time to save Sensors
		saveSensors();
}


/*
 * It averages an analog input as many times as desired
 */
uint8_t averageSensor(uint8_t port, uint8_t times)
{
	uint8_t i;
	uint32_t result = 0;
	for(i = 0; i < times; i++)
		result += abacus_gpio_analogRead(port);
	result = result / times;	//Final value
	result = result / 16;		//Reduce resolution to 8 bit
	return (uint8_t)result;		//cast to 8 bit
}

/*
 * It reads all the sensors
 */
void updateSensors()
{
	//writeDown time when we last read sensors
	uint32_t timeNow = abacus_millis();
	satelliteStatus_.telemetryLastTimeRead = timeNow;

	//If there are any other ongoing operations ignore and exit
	if( satelliteStatus_.memoryNoSleep == 1)
		return;

	//UnixTime
	satelliteTelemetry_.unixTime = getUnixTimeNow();

	//Round temperature:
	satelliteTelemetry_.tempMCU = (int8_t)abacus_sensors_temperatureTranslate(abacus_sensors_temperatureCPU_read());

	//Round temperature:
	satelliteTelemetry_.tempFPGA = (int8_t)abacus_sensors_temperatureTranslate(abacus_sensors_temperatureFPGA_read());

	//Magnetometers:
	if(abacus_sensors_magnetometer_read(&satelliteTelemetry_.magX,
			&satelliteTelemetry_.magY,
			&satelliteTelemetry_.magZ) == 0)
		//Clear error
		satelliteStatus_.errors &= ~ERRORI2C01;
	else
		//Set error
		satelliteStatus_.errors |= ERRORI2C01;

	//Gyro:
	abacus_sensors_gyro_read(&satelliteTelemetry_.gyroX,
			&satelliteTelemetry_.gyroY,
			&satelliteTelemetry_.gyroZ);

	//Abacus Current
	satelliteTelemetry_.abacusCurrent = abacus_gpio_analogRead(AB_CURRENT);

	//float current = abacus_currentRead();

	//Read radio Temperature:
	if(radio_readTelemetry(&radioLithium.telemetry) == 0)
	{
		satelliteTelemetry_.tempRadio = (int8_t)radioLithium.telemetry.msp430_temp;
		//Clear error
		satelliteStatus_.errors &= ~ERRORRADIO;
	}
	else
		satelliteStatus_.errors |= ERRORRADIO;

/*
	//Read bombaBateryVoltage
	//Divided by 16 it removes LS 4 bits
	satelliteTelemetry_.bombaBatBigVolt = (uint8_t)(abacus_gpio_analogRead(AB_H1_33)/ 16);	//Max 1.8V
	satelliteTelemetry_.bombaBatTimerVolt = (uint8_t)(abacus_gpio_analogRead(AB_H1_14)/ 16);	//Max 2.3V

	//Read external temperatures
	abacus_gpio_digitalWrite(TEMPEN_PIN, AB_HIGH);
	satelliteTelemetry_.temp01 = averageSensor(AB_H1_20, 5);
	satelliteTelemetry_.temp02 = averageSensor(AB_H1_22, 5);
	satelliteTelemetry_.temp03 = averageSensor(AB_H1_18, 5);
	abacus_gpio_digitalWrite(TEMPEN_PIN, AB_LOW);

	//Read Gommspace EPS data:
	uint8_t epsRawTelemetry[20];
	if(eps_getHkTelemetry(epsRawTelemetry) == 0)
		satelliteStatus_.errors &= ~ERROREPS;
	else
		satelliteStatus_.errors |= ERROREPS;

	char2uint(epsRawTelemetry, &satelliteTelemetry_.eps_Vbat);
	char2uint(&epsRawTelemetry[2], &satelliteTelemetry_.eps_currentSun);
	char2uint(&epsRawTelemetry[4], &satelliteTelemetry_.eps_currentOut);
	char2uint(&epsRawTelemetry[6], &satelliteTelemetry_.eps_batTemperature);
	char2uint(&epsRawTelemetry[8], &satelliteTelemetry_.eps_Vpanel01);
	char2uint(&epsRawTelemetry[10], &satelliteTelemetry_.eps_current01);
	char2uint(&epsRawTelemetry[12], &satelliteTelemetry_.eps_Vpanel02);
	char2uint(&epsRawTelemetry[14], &satelliteTelemetry_.eps_current02);
	char2uint(&epsRawTelemetry[16], &satelliteTelemetry_.eps_Vpanel03);
	char2uint(&epsRawTelemetry[18], &satelliteTelemetry_.eps_current03);
*/


	//Check errors
	if(abacus_gpio_expander.errorDetected == 0)
		satelliteStatus_.errors &= ~ERRORGPIOEXP;
	else
		satelliteStatus_.errors |= ERRORGPIOEXP;

	
	//Set errors telemetry:
	satelliteTelemetry_.telemetryErrors = satelliteStatus_.errors;

	//Set satellite status:
	satelliteTelemetry_.status = satelliteConfiguration_.status;

	//Kick Gomspace wdt
	//eps_resetWdt();
}

/*
 * It saves the sensors to memory
 */
void saveSensors()
{
	if(satelliteStatus_.memory->noSleep == 1)
		return;

	//writedown time when we last saved sensors on memory
	uint32_t timeNow = abacus_millis();
	satelliteStatus_.telemetryLastTimeSaved = timeNow;

	//If there are any other ongoing operations ignore and exit


	uint8_t sensorBuffer[SENSORS_SIZE_STREAM];	//Usually 48 bytes
	uint8_t *sensorP;
	sensorP = sensorBuffer;

	ulong2char(&satelliteTelemetry_.unixTime, sensorP);
	sensorP += 4;

	*sensorP = satelliteTelemetry_.tempMCU;
	sensorP++;
	*sensorP = satelliteTelemetry_.tempFPGA;
	sensorP++;

	int2char(&satelliteTelemetry_.magX, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.magY, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.magZ, sensorP);
	sensorP += 2;

	int2char(&satelliteTelemetry_.gyroX, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.gyroY, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.gyroZ, sensorP);
	sensorP += 2;

	uint2char(&satelliteTelemetry_.abacusCurrent, sensorP);
	sensorP += 2;

	*sensorP = satelliteTelemetry_.tempRadio;
	sensorP++;
	// *sensorP = satelliteTelemetry_.bombaBatBigVolt;
	// sensorP++;
	// *sensorP = satelliteTelemetry_.bombaBatTimerVolt;
	// sensorP++;

	*sensorP = satelliteTelemetry_.temp01;
	sensorP++;

	*sensorP = satelliteTelemetry_.temp02;
	sensorP++;

	*sensorP = satelliteTelemetry_.temp03;
	sensorP++;

	uint2char(&satelliteTelemetry_.eps_Vbat, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_currentSun, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_currentOut, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_Vpanel01, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_Vpanel02, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_Vpanel03, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_current01, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_current02, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_current03, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_batTemperature, sensorP);
	sensorP += 2;



	uint2char(&satelliteTelemetry_.telemetryErrors, sensorP);
	sensorP += 2;

	*sensorP = satelliteTelemetry_.status;

	//Save the buffer to the memory
	memory_sensors_write(sensorBuffer);

}

/*
 * It puts in a buffer all the telemetry of the satellite
 */
void saveExtendedTelemetry(uint8_t *buffer, uint8_t *size)
{
	uint32_t timeNow = abacus_millis();

	uint8_t *sensorP;
	sensorP = buffer;

	uint2char(&satelliteConfiguration_.numberReboots, sensorP);
	sensorP += 2;

	ulong2char(&timeNow, sensorP);
	sensorP += 4;

	ulong2char(&satelliteTelemetry_.unixTime, sensorP);
	sensorP += 4;

	*sensorP = satelliteTelemetry_.tempMCU;
	sensorP++;
	*sensorP = satelliteTelemetry_.tempFPGA;
	sensorP++;

	int16_t x, y, z;
	if(abacus_sensors_acc_read(&x, &y, &z) == 0)
		//Clear error
		satelliteStatus_.errors &= ~ERRORI2C01;
	else
		//Set error
		satelliteStatus_.errors |= ERRORI2C01;

	int2char(&x, sensorP);
	sensorP += 2;
	int2char(&y, sensorP);
	sensorP += 2;
	int2char(&z, sensorP);
	sensorP += 2;

	int2char(&satelliteTelemetry_.magX, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.magY, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.magZ, sensorP);
	sensorP += 2;

	int2char(&satelliteTelemetry_.gyroX, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.gyroY, sensorP);
	sensorP += 2;
	int2char(&satelliteTelemetry_.gyroZ, sensorP);
	sensorP += 2;

	uint2char(&satelliteTelemetry_.abacusCurrent, sensorP);
	sensorP += 2;

	*sensorP = satelliteTelemetry_.tempRadio;
	sensorP++;
	*sensorP = satelliteTelemetry_.bombaBatBigVolt;
	sensorP++;
	*sensorP = satelliteTelemetry_.bombaBatTimerVolt;
	sensorP++;

	*sensorP = satelliteTelemetry_.temp01;
	sensorP++;

	*sensorP = satelliteTelemetry_.temp02;
	sensorP++;

	*sensorP = satelliteTelemetry_.temp03;
	sensorP++;

	uint2char(&satelliteTelemetry_.eps_Vbat, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_currentSun, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_currentOut, sensorP);
	sensorP += 2;

	uint2char(&satelliteTelemetry_.eps_Vpanel01, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_Vpanel02, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_Vpanel03, sensorP);
	sensorP += 2;

	uint2char(&satelliteTelemetry_.eps_current01, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_current02, sensorP);
	sensorP += 2;
	uint2char(&satelliteTelemetry_.eps_current03, sensorP);
	sensorP += 2;

	uint2char(&satelliteTelemetry_.eps_batTemperature, sensorP);
	sensorP += 2;

	
	uint2char(&satelliteTelemetry_.telemetryErrors, sensorP);
	sensorP += 2;

	*sensorP = satelliteTelemetry_.status;

	*size = 62;
}
