#ifndef ACCELERATION_H_
#define ACCELERATION_H_


//il valore (modulo quadro) massimo da confrontare con il valore corrente di accelerazione (servirà un valore molto più basso)
#define THRESHOLD_ACCELERATION 5 // [ m^2*sec^-4 ]
#define ACCELERATION_BUFFER_SIZE 32
#include "abacus.h"
//uint32_t PrevTime=RTC_abacus_millis();

/*
 *
 * Function to set acceleration in the buffer AccelerationBuffer
 *
 */
void addAcceleration(const int32_t* a);
/*
 *
 * Function to set absolute value of acceleration 
 *
 */
void setAbsAcceleration();
/*
 *
 * Function to get absolute value of acceleration
 *
 */
int32_t getAbsAcceleration();
/*
 *
 * Function to get the mean value of acceleration (mean between ACCELEARTION_BUFFER_SIZE amount of samples)
 *
 */
int32_t getMeanAcceleration();
/*
 *
 * Function to check if satellite has acceleration while code is running. This should check only one time acceleration and stop all the operations if needed for N seconds
 *
 */
uint8_t checkAccelerationAtStartTime();

typedef struct CircularBuffer
{
float buffer[ACCELERATION_BUFFER_SIZE];
float mean3DAcceleration;
uint8_t index;
uint8_t full;
}CircularBuffer;

CircularBuffer accelerationBuffer;


uint8_t isSatelliteInSpace();

#endif
