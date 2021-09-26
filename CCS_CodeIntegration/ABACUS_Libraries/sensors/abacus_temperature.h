/*
 * abacus_temperature.h
 *
 */

#ifndef ABACUS_TEMPERATURE_H_
#define ABACUS_TEMPERATURE_H_

#include "../abacus.h"
#include "../abacus_utils.h"
#include "stdint.h"

//I2C address for sensor1 close to MSP430
#define AB_ADDRESS_TEMPCPU 0x48
//I2C address for sensor2 close to FPGA
#define AB_ADDRESS_TEMPFPGA 0x49

#define DS1775CONFIGREG 0x01


int8_t abacus_sensors_temperature_init();

int16_t abacus_sensors_temperatureMCU_read();

int8_t abacus_sensors_temperatureCPU_isOn();
void abacus_sensors_temperatureCPU_power_on();
void abacus_sensors_temperatureCPU_power_off();
uint16_t abacus_sensors_temperatureCPU_read();

int8_t abacus_sensors_temperatureFPGA_isOn();
void abacus_sensors_temperatureFPGA_power_on();
void abacus_sensors_temperatureFPGA_power_off();
uint16_t abacus_sensors_temperatureFPGA_read();

//float abacus_sensors_temperatureTranslate(int temperature);
int16_t abacus_sensors_temperatureTranslate(uint16_t raw);


#endif /* ABACUS_TEMPERATURE_H_ */
