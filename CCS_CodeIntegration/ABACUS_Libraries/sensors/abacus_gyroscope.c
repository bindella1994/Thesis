/*
 * abacus_gyroscope.c
 *
 */

 #include "abacus_gyroscope.h"

uint8_t addressGyroscope_;

/*
 * Initialization of gyroscope
 */
int8_t abacus_sensors_gyro_init(uint8_t address)
{
	if(address == AB_ABACUSVERSION2013 || address == AB_ADDRESS_GYRO2013)
		addressGyroscope_ = AB_ADDRESS_GYRO2013;
	else
		addressGyroscope_ = AB_ADDRESS_GYRO2014;

	int8_t result = 0;
	uint8_t buffer[2];

	buffer[0] = 0x20;	//Registry address
	buffer[1] = 0x07;	// 0x03 = 100Hz data rate, 12.5Hz cut off, Normal mode, all axis enabled and OFF
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	buffer[0] = 0x21;	//Registry address
	buffer[1] = 0x20;	// 0x20 = Normal mode, 8Hz High Pass filter cut off
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	buffer[0] = 0x22;	//Registry address
	buffer[1] = 0x00;	//Interrupts disabled
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	buffer[0] = 0x23;	//Registry address
	buffer[1] = 0x00;	//Continious update, 245dps
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	buffer[0] = 0x24;	//Registry address
	buffer[1] = 0x40;	//FIFO depths limited to FIFO threshold
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	buffer[0] = 0x2E;	//Registry address
	buffer[1] = 0x4A;	//stream mode + watermark level = 10 samples
						//(but DRDY/INT2 NOT CONNECTED IN THIS ABACUSHW!!!
	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);

	abacus_sensors_gyro_power_on();
	//abacus_sensors_gyro_setRate(AB_GYRO_RATE_2000DPS);

	return result;
}

int8_t abacus_sensors_gyro_isOn()
{
	// Local variable
	uint8_t buffertmp[1];

	buffertmp[0] = abacus_i2c_readRegister(AB_I2C_BUS00, addressGyroscope_, L3G4200CONFIGREG);
	if(buffertmp[0] & BIT3)
		return 1; //ON
	else
		return 0; //OFF
}

/*
 * It powers on the gyroscope
 */
int8_t abacus_sensors_gyro_power_on()
{
	uint8_t registryAddress = 0x20;
	uint8_t registryConf;

	//Ask gyro to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, addressGyroscope_, &registryConf, 1, 0);

	registryConf |= BIT3;

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);
}

/*
 * It switches off gyroscope
 */
int8_t abacus_sensors_gyro_power_off()
{
	uint8_t registryAddress = 0x20;
	uint8_t registryConf;

	//Ask gyro to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, addressGyroscope_, &registryConf, 1, 0);

	registryConf &= ~BIT3;

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);
}

/*
 * It reads the temperature of the gyroscope
 */
int8_t abacus_sensors_gyro_readTemperature(int8_t *temp)
{
	uint8_t registryAddress = 0x26;
	int8_t result = 0;

	uint8_t tempe;

	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &registryAddress, 1, 0);
	result += abacus_i2c_requestFrom(AB_I2C_BUS00, addressGyroscope_, &tempe, 1, 0);
	*temp = (int8_t) tempe;
	return result;
}

/*
 * It reads the temperature of the gyroscope
 */
int8_t abacus_sensors_gyro_readTemperatureToUint8(uint8_t *pointer)
{
	uint8_t registryAddress = 0x26;
	int8_t result = 0;

	result += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &registryAddress, 1, 0);
	result += abacus_i2c_requestFrom(AB_I2C_BUS00, addressGyroscope_, pointer, 1, 0);

	return result;
}

/*
 * Returns the sensors values
 */
int8_t abacus_sensors_gyro_read(int *x, int *y, int *z)
{
	/*uint8_t pointerAddress = 0x28;	//Read 6 bytes from 0x28 to 0x2D
	int8_t i2cResult = 0;
	uint8_t buffer[6];

	//Ask in streaming
	i2cResult += abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &pointerAddress, 1);
	i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00, addressGyroscope_, buffer, 6);

	//Lest significant byte came first:
	char2int(buffer, x);
	char2int(&buffer[2], y);
	char2int(&buffer[4], z);

	return i2cResult;*/

	uint8_t address;
	uint8_t buffer[6];
	uint8_t *pBuffer = buffer;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x28; address < 0x2E; address++)
	{
		abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
				addressGyroscope_,
				pBuffer,
				1,
				0);
		pBuffer++;
	}

	//Fill the x y z variables with the read results
	pBuffer = buffer;
	char2int(pBuffer, x);
	pBuffer += 2;
	char2int(pBuffer, y);
	pBuffer += 2;
	char2int(pBuffer, z);

	return i2cResult;
}

/*
 * It reads the value of all axis. Registry from 0x32 to 0x37, each axis is
 * 2 bytes. It returns the RAW data.
 */
int8_t abacus_sensors_gyro_readToUint8(uint8_t *pointer)
{
	uint8_t address;
	//uint8_t buffer[6];
	//uint8_t *pBuffer = buffer;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x28; address < 0x2E; address++)
	{
		abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
											addressGyroscope_,
											pointer,
											1,
											0);
		pointer++;
	}

	return i2cResult;
}

/*
 * Calibration values
 */
int8_t abacus_sensors_gyro_setZero(uint8_t axis, uint8_t offset)
{
	//TODO
	return -1;
}

/*
 * Sets the gyro scale. Possible values are:
 * 	AB_GYRO_RATE_245DPS
 * 	AB_GYRO_RATE_500DPS
 * 	AB_GYRO_RATE_2000DPS
 */
int8_t abacus_sensors_gyro_setRate(uint8_t rate)
{
	uint8_t buffer[2];
	buffer[0] = 0x23;	//Registry address
	switch (rate)
	{
	case AB_GYRO_RATE_245DPS:
		buffer[1] = 0x00;
		break;
	case AB_GYRO_RATE_500DPS:
		buffer[1] = 0x10;
		break;
	case AB_GYRO_RATE_2000DPS:
		buffer[1] = 0x30;
		break;
	default:
		buffer[1] = 0x00;	//245
		break;
	}
	return abacus_i2c_write(AB_I2C_BUS00, addressGyroscope_, buffer, 2, 0);
}

