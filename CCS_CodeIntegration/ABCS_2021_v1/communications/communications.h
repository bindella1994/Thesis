/*
 * communications.h
 */

#ifndef COMMUNICATIONS_H_
#define COMMUNICATIONS_H_

#define BEACONSIZE 84
#define RADIOHEADERS 10 //sync[3] + index[2] + indexback[2] +  size[1] +TYPE[1] + crc[1]

#define TYPERADIONOOP 		0x00
#define TYPERADIOBEACON 	0x01
#define TYPEMEMORYOPERATION 0x03
#define TYPEEXPERIMENT 	    0x04
#define TYPEFLASHCONF 		0x05
#define TYPERADIOOPERATION 	0x06
#define TYPEDIGITALPIN 		0x10
#define TYPEANALOGPIN 		0x11
#define TYPEFPGAOPERATIONS	0x12
#define TYPEI2COPERATIONS	0x13
#define TYPEUARTOPERATIONS	0x14
#define TYPERESETSATELLITE	0x25
#define TYPETRANSPONDER     0x26
#define TYPEHAMOPERATIONS	0x27
#define TYPEDEBUG			0x28



#include "radio_lithium2.h"
#include "../satellite/satsystem_init.h"

void sendRawRadioPacket(uint8_t *buffer, uint8_t repetitions);
void checkRadio();
void radioInProcessPacket(uint8_t *buffer, uint8_t lenght);
void checkBeacon();


#endif /* COMMUNICATIONS_H_ */
