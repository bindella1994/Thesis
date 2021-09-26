/*
 * marie.c
 *
 *  Created on: Sep 23, 2020
 *      Author: ghori
 */

#include "marie.h"

uint8_t bufferMarie_[320U]; //controllare il valore !!! use macro to define size
//unsigned int bufferMariePosition_;
uint8_t marieNewPacketArrived_;
uint16_t bufferMariePosition_;

uint8_t messageToMarie_[SIZEMESSAGETOMARIE];
//extern uint8_t bufferOut_[DIMBUFFEROUT+RADIOHEADERSIZE + PAYLOADCHECKSUMSIZE];

void init_Marie()
{
    //Set MARIE port
    abacus_uart_open_function(MARIEUART,
    AB_B57600, marieDataRx);
    abacus_uart_disableInterrupt(MARIEUART); //abiliterò l'interrupt se ricevo il comando M1 dalla debug uart
}


void initMarieStatus()
{
    //TO DO
}

void marieDataRx(uint8_t *exitLowPower)
{
    //Exit low power only if a new packet arrived
    if (abacus_uart_available(MARIEUART) != 0U)
    {

        uint8_t newByte = abacus_uart_read(MARIEUART);

        // Faccio questa cosa per scartare il primo byte che mi arriva quando avvio la comunicazione con MARIE
        if (MarieStatus_.marieInterrupt == 1U)
        {
            MarieStatus_.marieInterrupt = 2U;
            return;
        }

        //We are on sync
        bufferMarie_[bufferMariePosition_] = newByte; //array index may be outside bounds : [0..319]
        bufferMariePosition_++;

        if (bufferMariePosition_ >= MARIEPKGSIZE)
        {
            //End of package arrived
            marieNewPacketArrived_ = 1;
            MarieStatus_.numberMARIEPacket++;
            *exitLowPower = 1U;

            return;
        }
    }
    *exitLowPower = 0U;
}

void checkMarie(void)
{

    if (marieNewPacketArrived_)
    {
        //SatelliteEvent_.marie = 1U;

        memory_marie_saveData(bufferMarie_);
        marieNewPacketArrived_ = 0U;

        //Reset buffer
        bufferMariePosition_ = 0U;
    }
}

void sendCmdMarie(uint8_t *buffer)
{
    uint8_t i;
    uint8_t bufToMarie[64];
    for (i = 0U; i < 16U; i++)
    {
        bufToMarie[i] = buffer[i];
    }
    for (i = 16U; i < 64U; i++)
    {
        bufToMarie[i] = 0xffU;
    }

    abacus_uart_write(MARIEUART, bufToMarie, 64U);
}

