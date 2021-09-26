/*
 * abacus_i2c.h
 *
 */

#ifndef ABACUS_I2C_H_
#define ABACUS_I2C_H_
#include "msp430x54xa.h"
#include "stdint.h"
#include "../abacus.h"

#define I2CBUFFERLENGHT 50

struct I2cPort
{
	uint8_t i2c_name;
	uint8_t lastAddress;
	uint8_t isOnError;
	uint8_t inRepeatedStartCondition;

	//Slave variables
	uint8_t isSlave;
	uint8_t addressSlave;
	void (*triggerFunctionRequest)(uint8_t*);
	void (*triggerFunctionReceive)(uint8_t*);
};


extern struct I2cPort i2c00;
extern struct I2cPort i2c01;

void abacus_i2c_init();

void abacus_i2c_setSpeed(uint8_t busSelect,
		uint8_t highbyte,
		uint8_t lowbyte);

/////
// Master functions
int8_t abacus_i2c_write(uint8_t busSelect,
		uint8_t address,
		uint8_t *buffer,
		uint16_t lenght,
		uint8_t repeatedStart);

int8_t abacus_i2c_requestFrom(uint8_t busSelect,
		uint8_t address,
		uint8_t *buffer,
		uint16_t lenght,
		uint8_t repeatedStart);

uint8_t abacus_i2c_readRegister(uint8_t busSelect,
		uint8_t address,
		uint8_t reg);

/*void abacus_i2c_read16bitRegister(uint8_t busSelect,
		uint8_t address,
		uint8_t reg,
		uint8_t *buffer);
*/

/* Yet to implement the slave functions for ABACUS i2c
/////
//Slave functions:
int8_t abacus_i2c_slaveInit(uint8_t busSelect,
		uint8_t address,
		void (*triggerFunctionRequest)(uint8_t*),
		void (*triggerFunctionReceive)(uint8_t*));

int16_t abacus_i2c_slaveAvailable(uint8_t busSelect);

uint8_t abacus_i2c_slaveRead(uint8_t busSelect);

int8_t abacus_i2c_slaveWrite(uint8_t busSelect,
		uint8_t *buffer,
		uint16_t lenght);
*/

#endif /* ABACUS_I2C_H_ */
