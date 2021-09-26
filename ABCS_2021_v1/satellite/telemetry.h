/*
 * telemetry.h
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <memory/memory.h>
#include "abacus.h"
#include "GPIO/abacus_gpio.h"

#include "configuration.h"

#define TELEMETRYLENGHT 46

struct SatelliteTelemetry
{
	uint32_t unixTime;
	int8_t tempMCU;
	int8_t tempFPGA;

	int16_t magX;
	int16_t magY;
	int16_t magZ;

	int16_t gyroX;
	int16_t gyroY;
	int16_t gyroZ;

	uint16_t abacusCurrent;

	uint8_t tempRadio;
	uint8_t bombaBatBigVolt;	//This not on memory
	uint8_t bombaBatTimerVolt;	//This not on memory

	uint8_t temp01;
	uint8_t temp02;
	uint8_t temp03;

	uint16_t eps_Vbat;
	uint16_t eps_currentSun;
	uint16_t eps_currentOut;
	uint16_t eps_Vpanel01;
	uint16_t eps_Vpanel02;
	uint16_t eps_Vpanel03;
	uint16_t eps_current01;
	uint16_t eps_current02;
	uint16_t eps_current03;
	uint16_t eps_batTemperature;

	uint16_t telemetryErrors;
	uint8_t status;
};

extern struct SatelliteTelemetry satelliteTelemetry_;

void checkTelemetry();
void updateSensors();
void saveSensors();
void saveExtendedTelemetry(uint8_t *buffer, uint8_t *size);


#endif /* TELEMETRY_H_ */
