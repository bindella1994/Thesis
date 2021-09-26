/*
 * abacus_accelerometer.h
 *
 */

#ifndef ABACUS_ACCELEROMETER_H_
#define ABACUS_ACCELEROMETER_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

#define AB_ADDRESS_ACC 0x53  //Sensor address
#define ADXL345CONFIGREG 0x2D

int8_t abacus_sensors_acc_init();

int8_t abacus_sensors_acc_isOn();
int8_t abacus_sensors_acc_power_on();
int8_t abacus_sensors_acc_power_off();
int8_t abacus_sensors_acc_read(int *x, int *y, int *z);
int8_t abacus_sensors_acc_setOffset(uint8_t axis, uint8_t offset);
int8_t abacus_sensors_acc_setPowerMode(uint8_t mode);
int8_t abacus_sensors_acc_setRate(uint8_t rate);
int8_t abacus_sensors_acc_setRange(uint8_t range);
int8_t abacus_sensors_acc_setIntEnDis(uint8_t value);
int8_t abacus_sensors_acc_setIntMap(uint8_t value);

int8_t abacus_sensors_acc_readToUint8(uint8_t *pointer);

#endif /* ABACUS_ACCELEROMETER_H_ */
