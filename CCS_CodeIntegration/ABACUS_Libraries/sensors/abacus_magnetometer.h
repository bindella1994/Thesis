/*
 * abacus_magnetometer.h
 *
 */

#ifndef ABACUS_MAGNETOMETER_H_
#define ABACUS_MAGNETOMETER_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

#define AB_ADDRESS_MAGNETOMETER 0x1E  //Sensor address
#define HMC5883MODEREG 0x02

int8_t abacus_sensors_magnetometer_init();

int8_t abacus_sensors_magnetometer_isOn();
int8_t abacus_sensors_magnetometer_power_on();
int8_t abacus_sensors_magnetometer_power_off();
int8_t abacus_sensors_magnetometer_read(int *x, int *y, int *z);
int8_t abacus_sensors_magnetometer_setRate(uint8_t rate);
int8_t abacus_sensors_magnetometer_setGain(uint8_t gain);
int8_t abacus_sensors_magnetometer_setSampleAveraging(uint8_t value);
int8_t abacus_sensors_magnetometer_setMeasurementMode(uint8_t value);

int8_t abacus_sensors_magnetometer_readToUint8(uint8_t *pointer);

#endif /* ABACUS_MAGNETOMETER_H_ */
