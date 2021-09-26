/*
 * abacus_accelerometer.c
 *
 */

 #include "abacus_accelerometer.h"


/*
 * It powers up and configures the acc sensor
 */
int8_t abacus_sensors_acc_init()
{
	//Configure datarate, offsets, ranges and data formats
	//TODO

	return abacus_sensors_acc_power_on();
}

int8_t abacus_sensors_acc_isOn()
{
	// Local variable
	uint8_t buffertmp[1];

	buffertmp[0] = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_ACC, ADXL345CONFIGREG);
	if(buffertmp[0] & BIT3)
		return 1;	//ON
	else
		return 0;	//OFF
}

/*
 * Sets the accelerometer to measuring mode
 */
int8_t abacus_sensors_acc_power_on()
{
	uint8_t buffer[2];
	buffer[0] = 0x2D;	//Registry address
	buffer[1] = 0x08;	//Registry data
	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 * Sets the accelerometer to standby
 */
int8_t abacus_sensors_acc_power_off()
{
	uint8_t buffer[2];
	buffer[0] = 0x2D;	//Registry address
	buffer[1] = 0x00;	//Registry data
	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 * It reads the value of all axis. Registry from 0x32 to 0x37, each axis is
 * 2 bytes. It returns the RAW data.
 */
int8_t abacus_sensors_acc_read(int *x, int *y, int *z)
{
	uint8_t address;
	uint8_t buffer[6];
	uint8_t *pBuffer = buffer;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x32; address < 0x38; address++)
	{
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
											AB_ADDRESS_ACC,
											pBuffer,
											1, 0);
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
int8_t abacus_sensors_acc_readToUint8(uint8_t *pointer)
{
	uint8_t address;
	//uint8_t buffer[6];
	//uint8_t *pBuffer = buffer;

	int8_t i2cResult = 0;

	//Read the 6 registry
	for(address = 0x32; address < 0x38; address++)
	{
		abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, &address, 1, 0);
		i2cResult += abacus_i2c_requestFrom(AB_I2C_BUS00,
											AB_ADDRESS_ACC,
											pointer,
											1, 0);
		pointer++;
	}

	return i2cResult;
}

/*
 *
 */
int8_t abacus_sensors_acc_setOffset(uint8_t axis, uint8_t offset)
{
	uint8_t buffer[2];

	if(axis == 0)			//X
		buffer[0] = 0x1E;
	else if(axis == 0)		//Y
		buffer[0] = 0x1F;
	else					//Z
		buffer[0] = 0x20;

	buffer[1] = offset;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 *
 * mode: 0 -> Normal operation.
 *       1 -> Reduced power operation.
 */
int8_t abacus_sensors_acc_setPowerMode(uint8_t mode)
{
	uint8_t registryAddress = 0x2C;
	uint8_t registryConf;

	//Ask Acc to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryConf, 1, 0);

	if(mode == 0x00)
		registryConf &= ~BIT4;
	else
		registryConf |= BIT4;

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 *
 */
int8_t abacus_sensors_acc_setRate(uint8_t rate)
{
	uint8_t registryAddress = 0x2C;
	uint8_t registryConf;

	//Ask Acc to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryConf, 1, 0);

	//We reset the last 4 bits:
	registryConf &= 0xF0;
	//Set the 4 bits of rate
	registryConf |= rate;

	//Build the command to send
	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 *
 * D1  D0  (Range setting)
 * 0   0  --> 2g
 * 0   1  --> 4g
 * 1   0  --> 8g
 * 1   1  --> 16g
 */
int8_t abacus_sensors_acc_setRange(uint8_t range)
{
	uint8_t registryAddress = 0x31;	//DATA_FORMAT registry
	uint8_t registryConf;

	//Ask Acc to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, AB_ADDRESS_ACC, &registryConf, 1, 0);

	//We reset the last 2 bits:
	registryConf &= 0xFC;
	//Set the 4 bits of rate
	registryConf |= range;

	//Build the command to send
	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, AB_ADDRESS_ACC, buffer, 2, 0);
}

/*
 * Not yet supported by ABACUS
 */
int8_t abacus_sensors_acc_setInterrupt(uint8_t value)
{
	//TODO
	return -1;
}

/*
 * Not yet supported by ABACUS
 */
int8_t abacus_sensors_acc_setInterruptSource(uint8_t value)
{
	//TODO
	return -1;
}
