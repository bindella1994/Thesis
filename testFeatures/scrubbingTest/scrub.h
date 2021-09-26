/*
 * scrub.h
 *
 *  Created on: 22 set 2021
 *      Author: laboratorio
 */

#ifndef SCRUB_H_
#define SCRUB_H_


/*
 *
 * Funzione di calcolo della parit� Verticale, genera un byte a partire da un blocco di otto byte
 *
*/


#include "scrub.h"
#include <stdint.h>
#define PERSISTENT_RAM_LENGTH 80
#define NUMBER_OF_EXPERIMENTS 6
#define PERSRAMMAGICWORD 0x07

struct parity
{
    int8_t h1[PERSISTENT_RAM_LENGTH/8];
    int8_t v1[PERSISTENT_RAM_LENGTH/8];
    int16_t d1[PERSISTENT_RAM_LENGTH/8];

    int8_t h2[PERSISTENT_RAM_LENGTH/8];
    int8_t v2[PERSISTENT_RAM_LENGTH/8];
    int16_t d2[PERSISTENT_RAM_LENGTH/8];
};

struct persistent_RAM
{
       uint8_t Memory[PERSISTENT_RAM_LENGTH];
};


struct SatelliteStatus
{

    uint32_t totalMissionMinutes;
    uint32_t timeAtBoot;

    uint8_t eventTest;
    uint16_t errors;
    uint8_t interruptCause;
    uint8_t isOnSun;
    uint32_t lastTimeOnSun;
    uint32_t lastTimeOnShadow;
    uint32_t lastTimeRTC;
    uint32_t lastTimeRTCunixTime;

    // Telemetry
    uint32_t telemetryLastTimeBeacon;
    uint32_t telemetryLastTimeTx;
    uint32_t telemetryLastTimeRead;
    uint32_t telemetryLastTimeSaved;

    // LABONCHIP
    uint8_t experimentRunning;
    uint8_t experimentEvent;
    uint8_t currentExperimentIndex;
    uint8_t currentExperimentStep;
    uint32_t startTimeCurrentExpStep;
    uint32_t expPauseInterval;

    // COMM
    uint16_t radioPacketIndex;
    uint16_t radioAckPacketIndex;
    uint32_t lastTimeRxGround;
    uint32_t lastTimeRadioReboot;
    uint16_t amateurPackets;
    uint16_t amateurTxPackets;

    //MEMORY
    uint8_t ignoreMemory;
};

int16_t Calc_VParity(int16_t Data[]);

int16_t Calc_HParity(int16_t Data[]);

int16_t Calc_DParity(int16_t Data[]);

int8_t scrubbing_bytes(int16_t Data[8],int8_t index);

void find_position(int16_t a, int16_t *ptr);

void initialize(int16_t *pt, int16_t size);

int16_t count_errors(int16_t a);

void toggle_bit(int16_t *pt,int16_t len,int16_t i,int16_t j);

void scrubbing_parity_generation(int16_t Data[8],int8_t stage,int8_t index );

void setScrubParity(uint8_t* index_pointer, uint8_t index_struct );

int8_t scrub_recovery(uint8_t* index_pointer, uint8_t index_struct );

void backupInPersistentRam (uint8_t* persistentStartingPointer,
                           uint8_t* memoryStartingPointer,
                           uint32_t length);
void restoreFromPersistentRam ( uint8_t* persistentStartingPointer,
                                uint8_t* memoryStartingPointer,
                                uint32_t length);
void printPersistentRam();

void printParity();

void insertSingleCorruption(unsigned char* startingPointer, unsigned int length, unsigned char log );

int numberOfErrors(uint8_t* startingPointer);


void initSatelliteStatus(uint8_t* startingPointer, uint32_t length);

void copyPersistentRamForFinalTest(uint8_t* startingPointer,uint8_t* copyStartingPointer, uint32_t length);

#endif /* SCRUB_H_ */
