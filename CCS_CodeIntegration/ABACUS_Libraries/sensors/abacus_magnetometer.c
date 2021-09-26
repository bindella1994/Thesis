/*
 * abacus_magnetometer.c
 *
 */

#include "abacus_magnetometer.h"


/*
 * It initializates the sensor with the default values
 */
int8_t abacus_sensors_magnetometer_init()
{
	//Normal mode without bias
	abacus_sensors_magnetometer_setMeasurementMode(0);
	//0.75Hz
	abacus_sensors_magnetometer_setRate(0);
	//LSB: 230 (Recommended +-8.1 Ga)
	abacus_sensors_magnetometer_setGain(7);
	//Average 8 measurements
	abacus_sensors_magnetometer_setSampleAveraging(3);
	//Power on!
	return abacus_sensors_magnetometer_power_on();
}

int8_t abacus_sensors_magnetometer_isOn()
{
	// Local variable
	uint8_t buffertmp[1];

	buffertmp[0] = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, HMC5883MODEREG);
	if(buffertmp[0] & BIT1)
		return 0; //OFF
	else
		return 1; //ON
}

/*
 * It puts the magnetometer in continuous-mearusement mode
 */
int8_t abacus_sensors_magnetometer_power_on()
{
	uint8_t buffer[2];	//Configuration register B
	buffer[0] = 0x02;	////Mode register
	buffer[1] = 0x00;	//Last two bits set to zero, the others to zero too

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

/*
 * It puts the magnetometer in idle status
 */
int8_t abacus_sensors_magnetometer_power_off()
{
	uint8_t buffer[2];	//Configuration register B
	buffer[0] = 0x02;	////Mode register
	buffer[1] = 0x03;	//Last two bits set to one, the others to zero

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

/*
 * It returns the RAW values of the magnetometer
 */
int8_t abacus_sensors_magnetometer_read(int *x, int *y, int *z)
{
	uint8_t address;
	uint8_t buffer[6];
	uint8_t *pBuffer = buffer;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x08; address > 0x02; address--)
	{
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
				AB_ADDRESS_MAGNETOMETER,
				pBuffer,
				1,
				0);
		pBuffer++;
	}

	//Careful, We have done it first YY, then ZZ, then XX
	//Fill the x y z variables with the read results
	pBuffer = buffer;
	char2int(pBuffer, y);
	pBuffer += 2;
	char2int(pBuffer, z);
	pBuffer += 2;
	char2int(pBuffer, x);

	return i2cResult;
}

/*
 * It reads the value of all axis. Registry from 0x32 to 0x37, each axis is
 * 2 bytes. It returns the RAW data.
 */
int8_t abacus_sensors_magnetometer_readToUint8(uint8_t *pointer)
{
	uint8_t address;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x08; address > 0x02; address--)
	{
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
											AB_ADDRESS_MAGNETOMETER,
											pointer,
											1, 0);
		pointer++;
	}

	return i2cResult;
}

/*
 * Sets the gain of the magnetometer:
 * Value 0: LSB:1370 (Recommended +-0.88Ga)
 * Value 1: LSB:1090 (Recommended +-1.3 Ga)
 * Value 2: LSB: 820 (Recommended +-1.9 Ga)
 * Value 3: LSB: 660 (Recommended +-2.5 Ga)
 * Value 4: LSB: 440 (Recommended +-4.0 Ga)
 * Value 5: LSB: 390 (Recommended +-4.7 Ga)
 * Value 6: LSB: 330 (Recommended +-5.6 Ga)
 * Value 7: LSB: 230 (Recommended +-8.1 Ga)
 */
int8_t abacus_sensors_magnetometer_setGain(uint8_t gain)
{
	//Sanity check
	if(gain > 7)
		return -2;

	uint8_t buffer[2];
	buffer[0] = 0x01;	//Configuration register B
	buffer[1] = (gain << 5) & 0XE0;	//Are the BITS 5 to 7 the others are set to ZERO

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

/*
 * Sets the sample averaging of the magenetometer
 * Value 0: 1
 * Value 1: 2
 * Value 2: 4
 * Value 3: 8
 */
int8_t abacus_sensors_magnetometer_setSampleAveraging(uint8_t value)
{
	//Sanity check
	if(value > 3)
		return -2;
	uint8_t registryAddress = 0x00;	//Register A
	uint8_t registryConf;

	//Ask to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryConf, 1, 0);

	//Bits 5 & 6 and 7 to zero 0xx1 1111
	registryConf = (registryConf & 0x1F) | (value << 5);

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

/*
 * Sets the data output rate of the magenetometer
 * Value 0: 0.75Hz
 * Value 1:  1.5Hz
 * Value 2:    3Hz
 * Value 3:  7.5Hz
 * Value 4:   15Hz
 * Value 5:   30Hz
 * Value 6:   75Hz
 */
int8_t abacus_sensors_magnetometer_setRate(uint8_t rate)
{
	//Sanity check
	if(rate > 6)
		return -2;
	uint8_t registryAddress = 0x00;	//Register A
	uint8_t registryConf;

	//Ask to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryConf, 1, 0);

	//Bits 2, 3 & 4 and 7 to zero allways 011x xx11
	registryConf = (registryConf & 0x63) | (rate << 2);

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

/*
 * Sets the data output rate of the magenetometer
 * Value 0: Normal mode
 * Value 1: Positive bias
 * Value 2: Negative biaas
 */
int8_t abacus_sensors_magnetometer_setMeasurementMode(uint8_t value)
{
	//Sanity check
	if(value > 2)
		return -2;
	uint8_t registryAddress = 0x00;	//Register A
	uint8_t registryConf;

	//Ask to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, &registryConf, 1, 0);

	//Bits 0 & 1 and 7 to zero allways 0111 11xx
	registryConf = (registryConf & 0x7C) | value;

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, buffer, 2, 0);
}

