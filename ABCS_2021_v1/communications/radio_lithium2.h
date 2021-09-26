/*
 * radio_lithium1.h
 *
 */

#ifndef RADIO_LITHIUM2_H_
#define RADIO_LITHIUM2_H_

#define RADIO_BUFFER_SIZE 280	//280 Because maximum payload packet is 255!!!

#include "abacus.h"
#include "radio_lithium_structures.h"
#include <memory/memory.h>


struct Radio_configuration
{
	uint8_t uartName;
	uint8_t uartSpeed;
	uint8_t bufferIn[RADIO_BUFFER_SIZE];
	uint8_t bufferOut[RADIO_BUFFER_SIZE];
	uint8_t dataAvailable;
	uint16_t bufferInPosition;

	unsigned long lastTimeReceived;

	struct Lithium_rf_configuration rfConfiguration;
	struct Lithium_telemetry telemetry;
	struct Lithium_configuration configuration;
	struct Lithium_beacon_configuration beaconConfiguration;

	void (*triggerRxFunction)(uint8_t*);
};

extern struct Radio_configuration radioLithium;


int8_t radio_init(uint8_t uartName,
		uint8_t beaconInterval,
		uint8_t *beaconData,
		uint8_t power);

int8_t radio_reset(uint8_t hardware);

void radio_interrupt_trigger(uint8_t *exitLowPower);

void radio_interrupt_externalPin(uint8_t *exitLowPower);

int8_t radio_txData(uint8_t *data, uint8_t length);
int8_t radio_setPowerAmp(uint8_t power);
int8_t radio_available();
int8_t radio_lockUntilAvailable();
int8_t radio_readData(uint8_t *data, uint8_t *length, uint8_t *code);
int8_t radio_readAckNack(uint8_t *code);
int8_t radio_x25_decode(uint8_t *dataInput,
						uint8_t *dataLength,
						uint32_t *destAddress,
						uint32_t *sourceAddress,
						uint8_t *controlField,
						uint8_t *protocolId,
						uint8_t *dataOut,
						uint8_t *dataOutLength);

int8_t radio_readRawStruct(uint8_t index, uint8_t *buffer, uint8_t size);
int8_t radio_writeRawStruct(uint8_t index, uint8_t *buffer, uint8_t size);

int8_t radio_readconfiguration(struct Lithium_configuration *configuration);
int8_t radio_readTelemetry(struct Lithium_telemetry *telemetry);

int8_t radio_setConfiguration(struct Lithium_configuration *configuration);
int8_t radio_setRFconfiguration(struct Lithium_rf_configuration *rfConfiguration);
int8_t radio_setBeaconConfiguration(struct Lithium_beacon_configuration *beaconConfiguration);
int8_t radio_setBeaconData(uint8_t *data, uint8_t lenght);

int8_t radio_sendRawCommand(uint8_t *data, uint16_t length);
int8_t radio_sendCommand(uint8_t command, uint8_t *data, uint8_t length);
void radio_calculateChecksums(uint8_t *data,
							  uint16_t length,
							  uint8_t *checksum01,
							  uint8_t *checksum02);



#endif /* RADIO_LITHIUM2_H_ */
