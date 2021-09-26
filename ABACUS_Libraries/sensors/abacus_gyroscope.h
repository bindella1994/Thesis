/*
 * abacus_gyroscope.h
 *
 */

#ifndef ABACUS_GYROSCOPE_H_
#define ABACUS_GYROSCOPE_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

#define AB_ADDRESS_GYRO2014 0x6B  //Sensor address ABACUS2014
#define AB_ADDRESS_GYRO2013 0x69	//Sensor address ABACUS2013
#define L3G4200CONFIGREG 0X20

#define AB_GYRO_RATE_245DPS		0
#define AB_GYRO_RATE_500DPS		1
#define AB_GYRO_RATE_2000DPS	2

extern uint8_t addressGyroscope_;

int8_t abacus_sensors_gyro_init(uint8_t address);

int8_t abacus_sensors_gyro_isOn();
int8_t abacus_sensors_gyro_power_on();
int8_t abacus_sensors_gyro_power_off();
int8_t abacus_sensors_gyro_readTemperature(int8_t *temp);
int8_t abacus_sensors_gyro_read(int *x, int *y, int *z);
int8_t abacus_sensors_gyro_setZero(uint8_t axis, uint8_t offset);
int8_t abacus_sensors_gyro_setPowerMode(uint8_t mode);
int8_t abacus_sensors_gyro_setRate(uint8_t rate);

int8_t abacus_sensors_gyro_readToUint8(uint8_t *pointer);
int8_t abacus_sensors_gyro_readTemperatureToUint8(uint8_t *temp);
#endif /* ABACUS_GYROSCOPE_H_ */
