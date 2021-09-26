/*
 * debug.h
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "communications/abacus_uart.h"
#include "abacus.h"
#include "configuration.h"

//extern uint8_t bufferDebug_[250];
extern uint8_t bufferDebugPosition_;
extern uint8_t debugNewPacketArrived_;

void debugExternalInterrupt(uint8_t *exitLowPower);
void checkDebugCommand();

#endif /* DEBUG_H_ */
