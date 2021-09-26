/*
 * debug.c
 */

#include "debug.h"

#pragma NOINIT(bufferDebug_)
uint8_t bufferDebug_[250];

uint8_t bufferDebugPosition_;
uint8_t debugNewPacketArrived_;

void debugExternalInterrupt(uint8_t *exitLowPower)
{
    //Exit low power only if a new packet arrived
    if (abacus_uart_available(DEBUGUART) != 0)
    {
        uint8_t newByte = abacus_uart_read(DEBUGUART);
        //abacus_uart_write(satelliteConfiguration_.debugUart, &newByte, 1);
        if (bufferDebugPosition_ == 0)
            if (newByte != 'S')
                return;	//Out of sync
        if (bufferDebugPosition_ == 1)
            if (newByte != 'I')
            {
                bufferDebugPosition_ = 0;
                return;	//Out of sync
            }
        if (bufferDebugPosition_ == 2)
            if (newByte != 'A')
            {
                bufferDebugPosition_ = 0;
                return; //Out of sync
            }
        //We are on sync
        bufferDebug_[bufferDebugPosition_] = newByte;
        uint8_t size;
        if (bufferDebugPosition_ > 5)
            size = bufferDebug_[6];

        bufferDebugPosition_++;

        if (bufferDebugPosition_ >= 9 + size)
        {
            //End of package arrived
            debugNewPacketArrived_ = 1;
            *exitLowPower = 1;
            return;
        }
    }
    *exitLowPower = 0;
}

/*
 * It checks if there are commands coming from the Debug UART. The
 * communications are the same as the radio comms
 */
void checkDebugCommand()
{
    if (debugNewPacketArrived_ == 1)
    {
        radioInProcessPacket(bufferDebug_, bufferDebugPosition_);

        //Reset buffer
        bufferDebugPosition_ = 0;

        debugNewPacketArrived_ = 0;
    }

}

