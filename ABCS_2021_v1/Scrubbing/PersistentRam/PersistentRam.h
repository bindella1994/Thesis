#ifndef PERSISTENTRAM_H_
#define PERSISTENTRAM_H_

#include "abacus.h"


#define SCRUBBING_PERSISTENT_RAM_LENGTH 80

void toggle_bit(int16_t Data[8],int16_t len,int16_t i,int16_t j);

int16_t Calc_HParity(int16_t Data[]);

int16_t Calc_VParity(int16_t Data[]);

int16_t Calc_DParity(int16_t Data[]);

int8_t scrubbing_bytes(int16_t Data[8],int8_t index);

int16_t find_position(int16_t a, int16_t *ptr);

void initialize(int16_t *pt, int16_t size);

int16_t count_errors(int16_t a);

void scrubbing_parity_generation(int16_t Data[8],int8_t stage,int8_t index );

void setScrubParity(uint8_t* index_pointer, uint8_t index_struct );

int8_t scrub_recovery(uint8_t* index_pointer, uint8_t index_struct );

void backupInPersistentRam (uint8_t* persistentStartingPointer,
                           uint8_t* memoryStartingPointer,
                           uint32_t length);
void restoreFromPersistentRam ( uint8_t* persistentStartingPointer,
                                uint8_t* memoryStartingPointer,
                                uint32_t length);

void insertSingleCorruption(unsigned char* startingPointer, unsigned int length );

uint8_t cleanPersistentRam(uint16_t *byteElements, uint32_t size);

#define PERSISTENT_RAM_LENGTH 80
#pragma PERSISTENT(parity_)
struct parity
{
    int8_t h1[SCRUBBING_PERSISTENT_RAM_LENGTH/8];
    int8_t v1[SCRUBBING_PERSISTENT_RAM_LENGTH/8];
    int16_t d1[SCRUBBING_PERSISTENT_RAM_LENGTH/8];

    int8_t h2[SCRUBBING_PERSISTENT_RAM_LENGTH/8];
    int8_t v2[SCRUBBING_PERSISTENT_RAM_LENGTH/8];
    int16_t d2[SCRUBBING_PERSISTENT_RAM_LENGTH/8];
}parity_;
#pragma PERSISTENT (ramBackup_)
struct RamBackup
{
    uint8_t persistentRam[PERSISTENT_RAM_LENGTH];
    uint8_t isPersistentRamWritten;
}ramBackup_;


#endif
