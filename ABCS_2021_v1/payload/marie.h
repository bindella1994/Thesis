/*
 * marie.h
 *
 *  Created on: Sep 23, 2020
 *      Author: ghori
 */

#ifndef INCLUDES_MARIE_H_
#define INCLUDES_MARIE_H_

#include <memory/memory.h>
#include "../satellite/configuration.h"
#include "../communications/communications.h"

#define MARIEPKGSIZE    256U
#define SIZEMESSAGETOMARIE 64U //controllare il valore

struct MarieStatus
{
    uint8_t marieTimer;
    uint8_t marieInterrupt;
    unsigned int numberMARIEPacket;
};
struct MarieStatus MarieStatus_;

extern uint8_t messageToMarie_[SIZEMESSAGETOMARIE];
//extern uint8_t bufferOut_[DIMBUFFEROUT+RADIOHEADERSIZE + PAYLOADCHECKSUMSIZE];

void init_Marie();

void initMarieStatus();

void marieDataRx(uint8_t *exitLowPower);
void checkMarie(void);
void sendCmdMarie(uint8_t *buffer);
#endif /* INCLUDES_MARIE_H_ */
