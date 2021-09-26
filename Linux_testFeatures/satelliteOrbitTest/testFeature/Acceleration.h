#ifndef ACCELERATION_H_
#define ACCELERATION_H_

#include "stdint.h"
//il valore (modulo quadro) massimo da confrontare con il valore corrente di accelerazione (servirà un valore molto più basso)
#define THRESHOLD_ACCELERATION 9.81/2 // [ m*sec^-2 ]
#define ACCELERATION_BUFFER_SIZE 32
//uint32_t PrevTime=RTC_abacus_millis();

void setAbsAcceleration();

float getAbsAcceleration(int32_t x, int32_t y, int32_t z);

void addAcceleration(const float* a);

float getMeanAcceleration();

uint8_t checkAccelerationAtStartTime();

uint8_t isSatelliteInSpace();


typedef struct CircularBuffer
{
float buffer[ACCELERATION_BUFFER_SIZE];
float mean3DAcceleration;
uint8_t index;
uint8_t full;
}CircularBuffer;

CircularBuffer accelerationBuffer;


#endif

