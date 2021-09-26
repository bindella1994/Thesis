/*
 * radio_lithium_astrodev.h
 *
 *  Created on: 24/apr/2014
 *      Author: Aitor
 */

#ifndef RADIO_LITHIUM_STRUCTURES_H_
#define RADIO_LITHIUM_STRUCTURES_H_

#include "stdint.h"

//Configuration Commands
#define LITHIUM_NO_OP_COMMAND 0x01
#define LITHIUM_RESET_SYSTEM 0x02
#define LITHIUM_TRANSMIT_DATA 0x03
#define LITHIUM_RECEIVE_DATA 0x04
#define LITHIUM_GET_TRANSCEIVER_CONFIG 0x05
#define LITHIUM_SET_TRANSCEIVER_CONFIG 0x06
#define LITHIUM_TELEMETRY_QUERY 0x07
#define LITHIUM_WRITE_FLASH 0x08
#define LITHIUM_RF_CONFIG 0x09
#define LITHIUM_BEACON_DATA 0x10
#define LITHIUM_BEACON_CONFIG 0x11
#define LITHIUM_READ_FIRMWARE_REVISION 0x12
#define LITHIUM_WRITE_OVER_AIR_KEY 0x13
#define LITHIUM_FIRMWARE_UPDATE 0x14
#define LITHIUM_FIRMWARE_PACKET 0x15
#define LITHIUM_FAST_PA_SET 0x20

//Configuration parameters
#define LITHIUM_BAUD_RATE_9600 0
#define LITHIUM_BAUD_RATE_19200 1
#define LITHIUM_BAUD_RATE_38400 2
#define LITHIUM_BAUD_RATE_76800 3
#define LITHIUM_BAUD_RATE_115200 4
#define LITHIUM_RF_BAUD_RATE_1200 0
#define LITHIUM_RF_BAUD_RATE_9600 1
#define LITHIUM_RF_BAUD_RATE_19200 2
#define LITHIUM_RF_BAUD_RATE_38400 3
#define LITHIUM_RF_MODULATION_GFSK 0
#define LITHIUM_RF_MODULATION_AFSK 1
#define LITHIUM_RF_MODULATION_BPSK 2
#define LITHIUM_TELEMETRY_DUMP_COMMAND 0x30
#define LITHIUM_PING_RETURN_COMMAND 0x31
#define LITHIUM_CODE_UPLOAD_COMMAND 0x32
#define LITHIUM_RADIO_RESET_COMMAND 0x33
#define LITHIUM_PIN_TOGGLE_COMMAND 0x34

struct Lithium_configuration
{
	uint8_t interface_baud_rate; 	//Radio Interface Baud Rate (9600=0x00)
	uint8_t tx_power_amp_level; 	//Tx Power Amp level (min = 0x00 max = 0xFF)
	uint8_t rx_rf_baud_rate; 		//Radio RX RF Baud Rate (9600=0x00)
	uint8_t tx_rf_baud_rate; 		//Radio TX RF Baud Rate (9600=0x00)
	uint8_t rx_modulation;			//(0x00 = GFSK);
	uint8_t tx_modulation;			//(0x00 = GFSK);
	uint32_t rx_freq; 				//Channel Rx Frequency (ex: 45000000)
	uint32_t tx_freq; 				//Channel Tx Frequency (ex: 45000000)
	unsigned char source[6]; 		//AX25 Mode Source Call Sign (default NOCALL)
	unsigned char destination[6]; 	//AX25 Mode Destination Call Sign (default CQ)
	uint16_t tx_preamble; 			//AX25 Mode Tx Preamble Byte Length (0x00 = 20 flags)
	uint16_t tx_postamble; 			//AX25 Mode Tx Postamble Byte Length (0x00 = 20 flags)
	uint16_t function_config; 		//Radio Configuration Discrete Behaviors
	uint16_t function_config2; //Radio Configuration Discrete Behaviors #2
};

//The Telemetry Structure
struct Lithium_telemetry
{
	uint16_t op_counter;
	int16_t msp430_temp;
	uint8_t time_count[3];
	uint8_t rssi;
	uint32_t bytes_received;
	uint32_t bytes_transmitted;
};

//The Low Level RF Structure
struct Lithium_rf_configuration
{
	uint8_t front_end_level; 		//0 to 63 Value
	uint8_t tx_power_amp_level; 	//0 to 255 value, non-linear
	uint32_t tx_frequency_offset; 	//Up to 20 kHz
	uint32_t rx_frequency_offset; 	//Up to 20 kHz
};

//The Beacon Structure
struct Lithium_beacon_configuration
{
	uint8_t beacon_interval; 		//value of 0 is off, 2.5 sec delay per LSB
};

#endif /* RADIO_LITHIUM_STRUCTURES_H_ */
