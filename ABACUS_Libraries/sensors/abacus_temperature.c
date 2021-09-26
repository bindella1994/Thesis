/*
 * abacus_temperature.c
 *
 */
 
#include "abacus_temperature.h"

int8_t thermometerDS1775_powerOn(uint8_t address);
int8_t thermometerDS1775_powerOff(uint8_t address);
int8_t thermometerDS1775_configure(uint8_t address);
uint16_t thermometerDS1775_readTemperature(uint8_t address);

/*
 * It returns the temperature from the internal temperature sensor of the
 * MCU MSP430. This temperature is usually not very accurate...
 */
int16_t abacus_sensors_temperatureMCU_read()
{
	//TODO

	/*
	 * This is an example for the MSP430G2553
	 * For the MSP430F5438a. Look page 707 of Manual
	 */
	/*
	//1.5V ref,Ref on,64 clocks for sample
	ADC10CTL0 = SREF_1 + REFON + ADC10ON + ADC10SHT_3 ;
	//temp sensor is at 10 and clock/4
	ADC10CTL1 = INCH_10+ ADC10DIV_3;

	//wait 4 ref to settle
	__delay_cycles(1000);
	//enable conversion and start conversion
    ADC10CTL0 |= ENC + ADC10SC;
    //wait..i am converting...
    while(ADC10CTL1 & BUSY);
    //store val in t
    int t = ADC10MEM;
    //disable adc conv
    ADC10CTL0 &= ~ENC;

    //convert and pass
    return(int) ((t * 27069L - 18169625L) >> 16);
    */
	return 0;
}

/*
 * Initiates and configures all temperature sensors
 */
int8_t abacus_sensors_temperature_init()
{
	//Configure sensor CPU
	//TODO
	//Configure sensor FPGA
	//TODO
	//Power on sensor CPU
	abacus_sensors_temperatureCPU_power_on();
	//Power on sensor FPGA
	abacus_sensors_temperatureFPGA_power_on();

	return 0;
}

int8_t abacus_sensors_temperatureCPU_isOn()
{
	// Local variable
	uint8_t buffertmp[1];

	buffertmp[0] = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPCPU, DS1775CONFIGREG);
	if(buffertmp[0]&BIT0)
		return 0; //Sensor is OFF
	else
		return 1; //Sensor is ON
}

/*
 * It powers on the CPU temperature sensor
 */
void abacus_sensors_temperatureCPU_power_on()
{
	thermometerDS1775_powerOn(AB_ADDRESS_TEMPCPU);
}

/*
 * It powers off the CPU temperature sensor
 */
void abacus_sensors_temperatureCPU_power_off()
{
	thermometerDS1775_powerOff(AB_ADDRESS_TEMPCPU);
}

/*
 * It reads the CPU temperature sensor
 */
uint16_t abacus_sensors_temperatureCPU_read()
{
	return thermometerDS1775_readTemperature(AB_ADDRESS_TEMPCPU);;
}

int8_t abacus_sensors_temperatureFPGA_isOn()
{
	// Local variable
	uint8_t buffertmp[1];

	buffertmp[0] = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPFPGA, 0x01);
	if(buffertmp[0]&BIT0)
		return 0; //Sensor is OFF
	else
		return 1; //Sensor is ON
}

/*
 * It powers on the FPGA temperature sensor
 */
void abacus_sensors_temperatureFPGA_power_on()
{
	thermometerDS1775_powerOn(AB_ADDRESS_TEMPFPGA);
}

/*
 * It powers off the FPGA temperature sensor
 */
void abacus_sensors_temperatureFPGA_power_off()
{
	thermometerDS1775_powerOff(AB_ADDRESS_TEMPFPGA);
}

/*
 * It reads the FPGA temperature sensor
 */
uint16_t abacus_sensors_temperatureFPGA_read()
{
	return thermometerDS1775_readTemperature(AB_ADDRESS_TEMPFPGA);;
}

/*
 * It converts the coded temperature of the DS1775 sensors into a human
 * readable value
 */
/*float abacus_sensors_temperatureTranslate(int temperature)
{
	return ((float)temperature * (125.0 / 2000.0));	//This is from datasheet
}*/

/*
 * It converts the coded temperature of the DS1775 sensors into a human
 * readable value in 10ths of Celsius:
 * Example returns 1234 temperature is 12.34C
 */
int16_t abacus_sensors_temperatureTranslate(uint16_t raw)
{
	//From datasheet:
	//temperature * (125.0 / 2000.0)
	return ((raw * 100) / 16);
}

//////Private functions from here//////////////////////////////////////////////

/*
 * Specific function for powering up the temperature sensor
 */
int8_t thermometerDS1775_powerOn(uint8_t address)
{
	uint8_t registryAddress = 0x01;
	uint8_t registryConf;

	//Ask thermometer to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, address, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, address, &registryConf, 1, 0);

	//Set bit 0 to value 0
	registryConf &= ~BIT0;
	//Set maximum resolution:
	registryConf |= BIT5;
	registryConf |= BIT6;

	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, address, buffer, 2, 0);
}

/*
 * Specific function for powering off the temperature sensor
 */
int8_t thermometerDS1775_powerOff(uint8_t address)
{
	uint8_t registryAddress = 0x01;
	uint8_t registryConf;

	//Ask thermometer to get the registry (it is 1 byte)
	abacus_i2c_write(AB_I2C_BUS00, address, &registryAddress, 1, 0);
	abacus_i2c_requestFrom(AB_I2C_BUS00, address, &registryConf, 1, 0);

	//Set bit 0 to value 0
	registryConf |= BIT0;
	uint8_t buffer[2];
	buffer[0] = registryAddress;
	buffer[1] = registryConf;

	return abacus_i2c_write(AB_I2C_BUS00, address, buffer, 2, 0);
}

/*
 * Specific function for reading the temperature sensor
 */
uint16_t thermometerDS1775_readTemperature(uint8_t address)
{
	uint8_t pointerAddress = 0x00;

	//Ask thermometer to get the temperature (it returns 2 bytes)
	//Using i2c with repeated start
	abacus_i2c_write(AB_I2C_BUS00, address, &pointerAddress, 1, 1);
	uint8_t buffer[2];
	abacus_i2c_requestFrom(AB_I2C_BUS00, address, buffer, 2, 0);
	//Most significant byte came first:
	uint8_t temp = buffer[0];
	buffer[0]= buffer[1];
	buffer[1]= temp;

	//Convert buffer to an integer:
	uint16_t result;
	char2uint(buffer, &result);

	//result must move 4 bits right (read datasheet!)
	return (result >> 4);
}

/*
 * Specific function for configuring the temperature sensor
 */
int8_t thermometerDS1775_configure(uint8_t address)
{
	//TODO
	return 0;
}

