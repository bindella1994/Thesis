/*
 * communications.c
 */

#include "communications.h"

#pragma NOINIT(bufferOut)
uint8_t bufferOut[250]; //Overall variable to save memory. It will be used by everyone

/*
 * It sends the buffer. Data must be already present and starting at position 7
 * with the size of the data to be sent.
 * Here only SYNC bytes, index in, index out and CRC is written
 */
void sendRawRadioPacket(uint8_t *buffer, uint8_t repetitions)
{
    buffer[0] = 'S';
    buffer[1] = 'I';
    buffer[2] = 'A';

    //Index
    uint2char(&satelliteStatus_.radioPacketIndex, &buffer[3]);
    satelliteStatus_.radioPacketIndex++;

    //Send ACK?
    uint2char(&satelliteStatus_.radioAckPacketIndex, &buffer[5]);

    uint16_t packetSize = buffer[8] + RADIOHEADERS - 1;	//Minus 1 of CRC

    //Calculate CRC
    uint16_t crc;

    calculate_crc16_8bit(buffer, packetSize, &crc, 1);

    buffer[packetSize] = (uint8_t) 0x00FF & crc;
    //buffer[packetSize] = (uint8_t)0xDB;

    uint8_t i;

    //Failsafe
    if (repetitions == 0)
        repetitions = 1;

    //Do not transmit as fast as what I tell you
    uint32_t timeNow = abacus_millis();
    uint32_t timeNext = satelliteStatus_.telemetryLastTimeTx
            + (uint32_t) satelliteConfiguration_.radioMinimumInterval;
    if (timeNext > timeNow)
        abacus_sleep_msec(timeNext - timeNow);

    //Send the packet as many times as requested
    for (i = 0; i < repetitions; i++)
    {
        radio_txData(buffer, packetSize + 1);
        //Wait for the end of tx?
        if ((i + 1) != repetitions)
        {
            //Kick the WDT
            WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
            radio_lockUntilAvailable();
        }
    }

    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    radio_lockUntilAvailable();

    //Save time when it was transmitted:
    satelliteStatus_.telemetryLastTimeTx = abacus_millis();

    //If debug is on, send it also to debug UART
    if (satelliteConfiguration_.debugIsOn == 0)
        return;

    abacus_uart_write(DEBUGUART, buffer, packetSize + 1);
}

/*
 * Check what did the radio sent.
 */
void checkRadio()
{
    int8_t radioData = radio_available();

    uint32_t timeNow = abacus_millis();
    if (satelliteStatus_.lastTimeRxGround + 172800000UL < timeNow)
    {
        if (satelliteStatus_.lastTimeRadioReboot + 172800000UL < timeNow)
        {
            //Reboot radio:
            satelliteStatus_.lastTimeRadioReboot = timeNow;
            //Software:
            radio_reset(0);
            //Hardware:
            radio_reset(1);
            //Reinit radio:
            radio_init(RADIOUART,
                       satelliteConfiguration_.radioAutomaticBeaconInterval,
                       satelliteConfiguration_.radioAutomaticBeaconData,
                       satelliteConfiguration_.radioOutputPower);
        }
        if (satelliteStatus_.lastTimeRxGround + 604800000UL < timeNow)
        {
            //After 7 days of not listening ground. Reboot!
            //Reset writing an incorrect value to WDTCTL
            memory_logEvent_noPayload(getUnixTimeNow(), EVENT_REBOOTNOLISTEN);
            WDTCTL = 0x0000;
        }
    }

    if (radioData == 0)
        return;	//Nothing available from radio

    if (radioData == 1)
    {
        //We have an ACK or NACK on the radio
        uint8_t code;
        radio_readAckNack(&code);
        /*if(radio_readAckNack(&code) == 0)
         {
         abacus_uart_print(DEBUGUART, "Radio: ACK, code:");
         abacus_uart_print_int(DEBUGUART, (int) code);
         abacus_uart_print(DEBUGUART, "\r\n");
         }
         else
         {
         abacus_uart_print(DEBUGUART, "Radio: NACK, code:");
         abacus_uart_print_int(DEBUGUART, (int) code);
         abacus_uart_print(DEBUGUART, "\r\n");
         }*/
    }
    else if (radioData == 2)
    {
        //We have a data from radio:
        uint8_t code;
        uint8_t buffer2[280];
        uint8_t *bufferPointer = &buffer2[16];
        uint8_t lenght, lenghtData;
        uint32_t destAddress, sourceAddress;
        uint8_t controlField, protocolId;
        if (radio_readData(buffer2, &lenght, &code) == 0)
        {
            if (code == 0x04)
            {
                radio_x25_decode(buffer2, &lenght, &destAddress, &sourceAddress,
                                 &controlField, &protocolId, bufferPointer,
                                 &lenghtData);
                radioInProcessPacket(bufferPointer, lenght);
            }
        }
        else
        {
            //abacus_uart_print(DEBUGUART, "Radio: data unknown error!\r\n");
        }
    }
}

/*
 * Check if it is time to send the beacon, and if it is... transmit it!!
 */
void checkBeacon()
{
    //Check if we have to read the telemetry status:
    uint32_t timeNow = abacus_millis();
    uint32_t internalRTC = abacus_RTC_internal_getUnixTime();

    if ((satelliteStatus_.telemetryLastTimeBeacon
            + (uint32_t) satelliteConfiguration_.radioTelemetryBeaconInterval
                    * 1000UL) > timeNow)
    {
        //No need to send beacon yet
        return;
    }

    satelliteStatus_.telemetryLastTimeBeacon = timeNow;

    //If there are any other ongoing operations ignore and exit
    //if(satelliteStatus_.camera->noSleep == 1 || satelliteStatus_.memory->noSleep == 1)
    //	return;

    uint8_t *sensorP;
    sensorP = &bufferOut[RADIOHEADERS - 3];

    //Type of packet
    *sensorP = TYPERADIOBEACON;
    sensorP++;

    //Size of packet
    *sensorP = BEACONSIZE;
    sensorP++;

    ulong2char(&timeNow, sensorP);
    sensorP += 4;

    ulong2char(&internalRTC, sensorP);
    sensorP += 4;

    ulong2char(&satelliteStatus_.totalMissionMinutes, sensorP);
    sensorP += 4;

    ulong2char(&satelliteStatus_.timeAtBoot, sensorP);
    sensorP += 4;

    ulong2char(&satelliteStatus_.lastTimeRxGround, sensorP);
    sensorP += 4;

    *sensorP = satelliteStatus_.experimentRunning;
    sensorP++;

    *sensorP = satelliteStatus_.currentExperimentIndex;
    sensorP++;

    *sensorP = satelliteStatus_.currentExperimentStep;
    sensorP++;

    uint2char(&satelliteConfiguration_.numberReboots, sensorP);
    sensorP += 2;

    *sensorP = satelliteConfiguration_.doNotInitMemory;
    sensorP++;

    *sensorP = satelliteConfiguration_.busAndSensConfig;
    sensorP++;

    *sensorP = satelliteConfiguration_.status;
    sensorP++;

    uint2char(&satelliteStatus_.radioAckPacketIndex, sensorP);
    sensorP += 2;

    *sensorP = satelliteConfiguration_.radioAmateurOn;
    sensorP++;

    uint2char(&satelliteStatus_.amateurPackets, sensorP);
    sensorP += 2;

    uint2char(&satelliteConfiguration_.memory->errors, sensorP);
    sensorP += 2;

    ulong2char(&satelliteConfiguration_.memory->eventStartFreeAddress, sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->eventEndFreeAddress, sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->sensorsStartFreeAddress,
               sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->sensorsEndFreeAddress, sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->marieStartFreeAddress, sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->marieEndFreeAddress, sensorP);
    sensorP += 4;

    ulong2char(&satelliteConfiguration_.memory->lastMarieBeaconAddress,
               sensorP);
    sensorP += 4;

    ulong2char(&satelliteTelemetry_.unixTime, sensorP);
    sensorP += 4;

    *sensorP = satelliteTelemetry_.tempMCU;
    sensorP++;

    *sensorP = satelliteTelemetry_.tempFPGA;
    sensorP++;

    int2char(&satelliteTelemetry_.magX, sensorP);
    sensorP += 2;
    int2char(&satelliteTelemetry_.magY, sensorP);
    sensorP += 2;
    int2char(&satelliteTelemetry_.magZ, sensorP);
    sensorP += 2;

    int2char(&satelliteTelemetry_.gyroX, sensorP);
    sensorP += 2;
    int2char(&satelliteTelemetry_.gyroY, sensorP);
    sensorP += 2;
    int2char(&satelliteTelemetry_.gyroZ, sensorP);
    sensorP += 2;

    uint2char(&satelliteTelemetry_.abacusCurrent, sensorP);
    sensorP += 2;
    /*
     *sensorP = satelliteTelemetry_.tempRadio;
     sensorP++;

     uint2char(&satelliteTelemetry_.eps_Vbat, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_currentSun, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_currentOut, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_Vpanel01, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_Vpanel02, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_Vpanel03, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_current01, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_current02, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_current03, sensorP);
     sensorP += 2;
     uint2char(&satelliteTelemetry_.eps_batTemperature, sensorP);
     sensorP += 2;

     uint2char(&satelliteTelemetry_.telemetryErrors, sensorP);
     sensorP += 2;
     */
    *sensorP = satelliteTelemetry_.status;

    //Send telemetry via Radio
    sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
}

/*
 * It processes configuration parameters
 */
void radioProcessConfigurationPacket(uint8_t *buffer, uint8_t size)
{
    uint8_t parameterIndex = buffer[0];
    uint32_t newUnixTime;
    switch (parameterIndex)
    {
    case 0x00:
        //RTC update:
        char2ulong(&buffer[1], &newUnixTime);
        abacus_RTC_setUnixTime(newUnixTime);
        //Force update of all registers:
        satelliteStatus_.lastTimeRTC = 0;
        getUnixTimeNow();
        break;
    default:
        break;
    }

    //Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * Memory operations
 */
void radioProcessMemoryOperations(uint8_t *buffer, uint8_t size)
{

    uint32_t temp1, temp2;
    uint16_t temp3;

    //Check if there is already an ongoing operation
    if (buffer[0] > 0 && buffer[0] < 0x0D)
    {
        //Only if memory is ready
        if (satelliteStatus_.memory->statusParallel != 0)
        {
            //Return error
            bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
            bufferOut[RADIOHEADERS - 2] = 2;	//Size
            bufferOut[RADIOHEADERS - 1] = buffer[0];
            bufferOut[RADIOHEADERS] = 0xFF;	//Error
            //Send telemetry via Radio
            sendRawRadioPacket(bufferOut,
                               satelliteConfiguration_.radioRepetitions);
            return;
        }
    }

    switch (buffer[0])
    {
    case 0x00:	//get status
        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = 34 + 1;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];

        ulong2char(&satelliteStatus_.memory->sensorsStartFreeAddress,
                   &bufferOut[RADIOHEADERS]);
        ulong2char(&satelliteStatus_.memory->sensorsEndFreeAddress,
                   &bufferOut[RADIOHEADERS + 4]);
        ulong2char(&satelliteStatus_.memory->marieStartFreeAddress,
                   &bufferOut[RADIOHEADERS + 8]);
        ulong2char(&satelliteStatus_.memory->marieEndFreeAddress,
                   &bufferOut[RADIOHEADERS + 12]);

        ulong2char(&satelliteStatus_.memory->marieStoredPage,
                   &bufferOut[RADIOHEADERS + 16]);
        ulong2char(&satelliteStatus_.memory->lastMarieBeaconAddress,
                   &bufferOut[RADIOHEADERS + 20]);
        ulong2char(&satelliteStatus_.memory->eventStartFreeAddress,
                   &bufferOut[RADIOHEADERS + 24]);
        ulong2char(&satelliteStatus_.memory->eventEndFreeAddress,
                   &bufferOut[RADIOHEADERS + 28]);
        bufferOut[RADIOHEADERS + 32] = satelliteStatus_.memory->errors;

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;

    case 0x01:	//get sensors from to
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        char2ulong(&buffer[1], &satelliteStatus_.memory->parallelStartAddress);
        char2ulong(&buffer[5], &satelliteStatus_.memory->parallelEndAddress);
        break;

    case 0x02:	//clean sensors
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        break;

    case 0x03:	//get events from to
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        char2ulong(&buffer[1], &satelliteStatus_.memory->parallelStartAddress);
        char2ulong(&buffer[5], &satelliteStatus_.memory->parallelEndAddress);
        break;

    case 0x04:	//clean events
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        break;

    case 0x05:	//get CRC part memory
        char2ulong(&buffer[1], &temp1);
        char2ulong(&buffer[5], &temp2);
        temp3 = abacus_flash_crc(satelliteStatus_.memory->selectedMainMemory,
                                 temp1, temp2);
        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = 3;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];
        uint2char(&temp3, &bufferOut[RADIOHEADERS]);
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;

    case 0x0A:	//get Marie
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        char2ulong(&buffer[1], &satelliteStatus_.memory->parallelStartAddress);
        char2ulong(&buffer[5], &satelliteStatus_.memory->parallelEndAddress);
        break;

    case 0x0B:	//clean Marie
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = buffer[0];
        satelliteStatus_.memory->subStatusParallel = 0x00;
        break;

    case 0x0C:	//Bulk erase and reboot counters and maps
        satelliteStatus_.memory->noSleep = 1;
        satelliteStatus_.memory->statusParallel = 0x0D;
        satelliteStatus_.memory->subStatusParallel = 0x00;
        break;

    case 0x0D:	//Reset all the status and remount
        satelliteStatus_.memory->noSleep = 0;
        satelliteStatus_.memory->statusParallel = 0x00;
        satelliteStatus_.memory->subStatusParallel = 0x00;
        memory_init(0);
        break;

    case 0x10:	//Delete a Sector in bruteforce mode
        char2ulong(&buffer[2], &temp1);
        abacus_flash_sector_erase(buffer[1], temp1);
        break;

    case 0x11:	//Read 80bytes in bruteforce mode
        char2ulong(&buffer[2], &temp1);
        abacus_flash_read_data(buffer[1], temp1, &bufferOut[RADIOHEADERS], 80);

        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = 81;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];

        //Send data via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;

    default:
        break;
    }

    //Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;

}

/*
 * Telecommunication packets for setting or reading the internal configuration
 * of the satellite
 */
void radioProcessSetNewConfiguration(uint8_t *buffer, uint8_t size)
{
    switch (buffer[0])
    {
    case 0x00:	//Send current configuration
        bufferOut[RADIOHEADERS - 3] = TYPEFLASHCONF;
        bufferOut[RADIOHEADERS - 2] = 1 + PERSISTENT_RAM_LENGTH;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];
        //Instead of saving data to internal flash, it will make it into the
        //output buffer that is about to be sent
        satsystem_saveConfiguration(&satelliteConfiguration_,
                                    &bufferOut[RADIOHEADERS]);

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    case 0x01:	//Set arriving new configuration
        //Tries to load the configruation from the arriving stream instead form the
        //internal flash
        satsystem_loadConfiguration(&satelliteConfiguration_, &buffer[1]);
        break;
    case 0x02:	//Flash actual configuration to flash

        //satsystem_saveConfiguration(&satelliteConfiguration_, internal_flash);
        memory_writeConf(&satelliteConfiguration_);
        break;

    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * It processes operations for the radio
 */
void radioProcessRadioOperations(uint8_t *buffer, uint8_t size)
{
//uint8_t bufferOut[100];
    switch (buffer[0])
    {
    case 0x00:	//radio reboot software
        radio_reset(0);
        radio_init(RADIOUART,
                   satelliteConfiguration_.radioAutomaticBeaconInterval,
                   satelliteConfiguration_.radioAutomaticBeaconData,
                   satelliteConfiguration_.radioOutputPower);
        break;
    case 0x01:	//radio reboot hardware
        radio_reset(1);
        radio_init(RADIOUART,
                   satelliteConfiguration_.radioAutomaticBeaconInterval,
                   satelliteConfiguration_.radioAutomaticBeaconData,
                   satelliteConfiguration_.radioOutputPower);
        break;
    case 0x02:	//radio set power
        radio_setPowerAmp(buffer[1]);
        break;
    case 0x03:	//Read conf
        radio_readRawStruct(buffer[1], &bufferOut[RADIOHEADERS + 1], buffer[2]);
        bufferOut[RADIOHEADERS - 3] = TYPERADIOOPERATION;
        bufferOut[RADIOHEADERS - 2] = 2 + buffer[2];	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[1];
        bufferOut[RADIOHEADERS] = buffer[2];
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    case 0x04:	//Write conf
        radio_writeRawStruct(buffer[1], &buffer[3], buffer[2]);
        bufferOut[RADIOHEADERS - 3] = TYPERADIOOPERATION;
        bufferOut[RADIOHEADERS - 2] = 2;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[1];
        bufferOut[RADIOHEADERS] = buffer[2];
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    case 0x05:	//Reconfigure beacon and power as init
        radio_init(RADIOUART,
                   satelliteConfiguration_.radioAutomaticBeaconInterval,
                   satelliteConfiguration_.radioAutomaticBeaconData,
                   satelliteConfiguration_.radioOutputPower);
    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * It reads a pin from digital inputs and returns its value
 */
void radioProcessDigitalPin(uint8_t *buffer, uint8_t size)
{
    uint8_t result = 0;
    switch (buffer[0])
    {
    case 0:	//digitalMode
        abacus_gpio_digitalMode(buffer[1], buffer[2]);
        break;
    case 1:	//digitalWrite
        abacus_gpio_digitalWrite(buffer[1], buffer[2]);
        break;
    case 2:	//digitalRead
        result = abacus_gpio_digitalRead(buffer[1]);
        bufferOut[RADIOHEADERS - 3] = TYPEDIGITALPIN;
        bufferOut[RADIOHEADERS - 2] = 2;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[1];
        bufferOut[RADIOHEADERS] = result;
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * It reads an analog pin and returns its value
 * buffer[0] is port, buffer[1] is number of repetitions
 */
void radioProcessAnalogPin(uint8_t *buffer, uint8_t size)
{
    int32_t rawMeasurement = 0;
    uint8_t i;
    for (i = 0; i < buffer[1]; i++)
        rawMeasurement += abacus_gpio_analogRead(buffer[0]);
    rawMeasurement = rawMeasurement / (int32_t) buffer[1];
    uint16_t result = (uint16_t) rawMeasurement;

    bufferOut[RADIOHEADERS - 3] = TYPEANALOGPIN;
    bufferOut[RADIOHEADERS - 2] = 4;	//Size
    bufferOut[RADIOHEADERS - 1] = buffer[0];
    bufferOut[RADIOHEADERS] = buffer[1];

    uint2char(&result, &bufferOut[RADIOHEADERS + 1]);

//Send telemetry via Radio
    sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
}

/*
 * FPGA operations
 */
void radioProcessFPGAOperations(uint8_t *buffer, uint8_t size)
{
//uint8_t result = 0;
    switch (buffer[0])
    {
    case 0:	//Switch on
        abacus_fpga_switchOn();
        abacus_sleep_msec(5);
        abacus_fpga_resetProgram();
        break;
    case 1:	//Switch off
        abacus_fpga_switchOff();
        break;
    case 2:	//bulkErase
        abacus_fpga_switchOn();
        abacus_fpga_prgB_holdUp();
        abacus_flash_bulk_erase(AB_FLASH_FPGA);
        break;
    case 3:	//Flash new program
        //Not done here, done in Memory operations
        break;
    case 4:	//Send test
        abacus_fpga_dataBusSendArray_8bit(&buffer[3], buffer[1]);
        abacus_fpga_dataBusGetArray_8bit(&bufferOut[RADIOHEADERS + 1],
                                         buffer[2]);
        bufferOut[RADIOHEADERS - 3] = TYPEFPGAOPERATIONS;
        bufferOut[RADIOHEADERS - 2] = 2 + buffer[2];	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[1];
        bufferOut[RADIOHEADERS] = buffer[2];
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    case 5:	//Get Status
        bufferOut[RADIOHEADERS - 3] = TYPEFPGAOPERATIONS;
        bufferOut[RADIOHEADERS - 2] = 2;	//Size
        bufferOut[RADIOHEADERS - 1] = abacus_fpga_isOn();

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * It sends or requests i2c data in Raw mode:
 * buffer[0] is write or read
 * buffer[1] is bus name
 * buffer[2] is destination address
 * buffer[3] is size of output
 * buffer[4] is size of input
 * buffer[5] is data to transfer
 *
 */
void radioProcessI2COperations(uint8_t *buffer, uint8_t size)
{
    int8_t errors = 0;
    if (buffer[0] == 0x00)
    {
        //Only send a new configuration nothing to read
        errors += abacus_i2c_write(buffer[1], buffer[2], &buffer[5], buffer[3],
                                   0);

        bufferOut[RADIOHEADERS - 3] = TYPEI2COPERATIONS;
        bufferOut[RADIOHEADERS - 2] = 4;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];
        bufferOut[RADIOHEADERS] = buffer[1];
        bufferOut[RADIOHEADERS + 1] = buffer[2];
        bufferOut[RADIOHEADERS + 2] = errors;

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);

        return;
    }

//Request the data via i2c

    errors += abacus_i2c_write(buffer[1], buffer[2], &buffer[5], buffer[3], 0);

//Give it time to process, otherwise it fails.
    abacus_sleep_msec(5);

    errors += abacus_i2c_requestFrom(buffer[1], buffer[2],
                                     &bufferOut[RADIOHEADERS + 3], buffer[4],
                                     0);

    bufferOut[RADIOHEADERS - 3] = TYPEI2COPERATIONS;
    bufferOut[RADIOHEADERS - 2] = buffer[4] + 4;	//Size
    bufferOut[RADIOHEADERS - 1] = buffer[0];
    bufferOut[RADIOHEADERS] = buffer[1];
    bufferOut[RADIOHEADERS + 1] = buffer[2];
    bufferOut[RADIOHEADERS + 2] = errors;

//Send telemetry via Radio
    sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);

}

/*
 * It sends or requests uart data in Raw mode:
 * buffer[0] is write or read
 * buffer[1] is bus name
 * buffer[2] is size of output
 * buffer[3] is size of input
 * buffer[4] is data to transfer
 */
void radioProcessUARTOperations(uint8_t *buffer, uint8_t size)
{
    switch (buffer[0])
    {
    case 0x00:
        //Only send a new configuration nothing to read
        abacus_uart_write(buffer[1], &buffer[4], buffer[2]);

        bufferOut[RADIOHEADERS - 3] = TYPEUARTOPERATIONS;
        bufferOut[RADIOHEADERS - 2] = 2;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[0];
        bufferOut[RADIOHEADERS] = buffer[1];

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);

        return;
    case 0x01:
        //Request the data via UART //This will not work with UARTs with
        //INTERRUPT enabled

        //Send command
        abacus_uart_write(buffer[1], &buffer[4], buffer[2]);

        //Lock until send:
        abacus_uart_waitUntilTxFinished(buffer[1]);

        //Read command:
        uint8_t i;
        uint16_t failSafe;
        for (i = 0; i < buffer[3]; i++)
        {
            for (failSafe = 0; failSafe < 600; failSafe++)
                if (abacus_uart_available(buffer[1]) != 0)
                    break;
            bufferOut[RADIOHEADERS + i] = abacus_uart_read(buffer[1]);
        }

        bufferOut[RADIOHEADERS - 3] = TYPEUARTOPERATIONS;
        bufferOut[RADIOHEADERS - 2] = buffer[3] + 1;	//Size
        bufferOut[RADIOHEADERS - 1] = buffer[1];	//Bus

        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        break;

    case 0x02:	//Open port
        //Prevent touching radio port:
        if (buffer[1] == RADIOUART)
            break;

        abacus_uart_open(buffer[1], buffer[2]);
        break;

    case 0x03:	//Close port
        //Prevent touching radio port:
        if (buffer[1] == RADIOUART)
            break;

        abacus_uart_close(buffer[1]);
        break;

    default:
        break;
    }

//Send telemetry via Radio
    sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
}

/*
 * Operations for HAM packet
 */
void radioProcessHAMPacket(uint8_t *buffer, uint8_t size)
{
    switch (buffer[0])
    {
    case 0:	//Switch on ham operations
        satelliteConfiguration_.radioAmateurOn = 1;
        satelliteStatus_.amateurPackets = 0;
        satelliteStatus_.amateurTxPackets = 0;
        break;
    case 1:	//Switch off ham operations
        satelliteConfiguration_.radioAmateurOn = 0;
        satelliteStatus_.amateurPackets = 0;
        satelliteStatus_.amateurTxPackets = 0;
        break;
    case 2:	//Reset ham counter
        satelliteStatus_.amateurPackets = 0;
        satelliteStatus_.amateurTxPackets = 0;
        break;
    case 3:	//Read status
        bufferOut[RADIOHEADERS - 3] = TYPEFPGAOPERATIONS;
        bufferOut[RADIOHEADERS - 2] = 5;	//Size
        bufferOut[RADIOHEADERS - 1] = satelliteConfiguration_.radioAmateurOn;
        uint2char(&satelliteStatus_.amateurPackets, &bufferOut[RADIOHEADERS]);
        uint2char(&satelliteStatus_.amateurPackets,
                  &bufferOut[RADIOHEADERS + 2]);
        //Send telemetry via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        return;
    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * Operations for debug packet
 */
void radioProcessDebugPacket(uint8_t *buffer, uint8_t size)
{
    switch (buffer[0])
    {
    case 0:	//Switch on debug
        satelliteConfiguration_.debugIsOn = 1;
        break;
    case 1:	//Switch off debug
        satelliteConfiguration_.debugIsOn = 0;
        break;
    default:
        break;
    }
//Force inmediate answer with beacon:
    satelliteStatus_.telemetryLastTimeBeacon = 0;
}

/*
 * Transponder mode for HAM radio amateurs
 */
void radioInProcessHamPacket(uint8_t *buffer, uint8_t size)
{
    if (satelliteConfiguration_.radioAmateurOn != 1)
        return;

    if (satelliteStatus_.amateurTxPackets
            > satelliteConfiguration_.radioAmateurMaxTx)
        return;

    satelliteConfiguration_.radioAmateurMaxTx++;
    uint8_t lenght = buffer[3];
    if (lenght > 150)
        return;	//Do not overload

//Using the same buffer to send back data:

    buffer[RADIOHEADERS - 3] = TYPETRANSPONDER;
    buffer[RADIOHEADERS - 2] = lenght;	//Size

}

void radioProcessExperimentOp(uint8_t *buffer, uint8_t size)
{

}

/*
 * It processes the input packet
 */
void radioInProcessPacket(uint8_t *buffer, uint8_t lenght)
{
    /*
     * Bytes 0-2: pwd
     * Bytes 3-4: index
     * Byte 5: type
     * Byte 6: size
     * Bytes 7-(6+size): command and params
     * Bytes (7+size)-(8+size): CRC
     */

//Check if it is a command from hams:
    if (buffer[0] == 'H' && buffer[1] == 'A' && buffer[2] == 'M')
    {
        satelliteStatus_.amateurPackets++;
        radioInProcessHamPacket(buffer, lenght);
        //if(satelliteConfiguration_.debugIsOn == 1)
        //	abacus_uart_print(DEBUGUART, "HAM packet received");
        return;
    }

//Check if it is a commamd from home
    if (!(buffer[0] == 'S' && buffer[1] == 'I' && buffer[2] == 'A'))
    {
        //Invalid packet
        //if(satelliteConfiguration_.debugIsOn == 1)
        //	abacus_uart_print(DEBUGUART, "Invalid packet received.");
        return;
    }

//Save when it was last time we received from ground
    satelliteStatus_.lastTimeRxGround = abacus_millis();
    satelliteStatus_.lastTimeRadioReboot = satelliteStatus_.lastTimeRxGround;

//Valid sync bytes received
//get size of payload:
    uint8_t size = buffer[6];
    uint16_t crc, calculatedCrc;
    char2uint(&buffer[size + 7], &crc);

//Check CRC
    calculate_crc16_8bit(buffer, size + 7, &calculatedCrc, 1);

    if (crc != calculatedCrc)
    {
        //Incorrect CRC from ground package

        //Generate event and reject package
        memory_logEvent_noPayload(getUnixTimeNow(), EVENT_RADIOCRCERROR);

        return;
    }

//CRC is correct
//Put the indexBack packet counter:
    uint16_t indexAckBack;
    char2uint(&buffer[3], &indexAckBack);

    if (indexAckBack == satelliteStatus_.radioAckPacketIndex)
    {
        //Packet was duplicated!!! ignore it!
        //if(satelliteConfiguration_.debugIsOn == 1)
        //abacus_uart_print(DEBUGUART, "Duplicated packet received ignoring it");
        return;
    }

    satelliteStatus_.radioAckPacketIndex = indexAckBack;

//uint8_t answerWithBeacon = 0;

//Check type of packet:
    switch (buffer[5])
    {
    case 0:
        //Answer with a telemetry
        satelliteStatus_.telemetryLastTimeBeacon = 0;
        break;
    case TYPEMEMORYOPERATION:
        radioProcessMemoryOperations(&buffer[7], size);
        break;
    case TYPEEXPERIMENT:
        radioProcessExperimentOp(&buffer[7], size);
        break;
    case TYPEFLASHCONF:
        radioProcessSetNewConfiguration(&buffer[7], size);
        break;
    case TYPERADIOOPERATION:
        radioProcessRadioOperations(&buffer[7], size);
        break;
    case TYPEDIGITALPIN:
        radioProcessDigitalPin(&buffer[7], size);
        break;
    case TYPEANALOGPIN:
        radioProcessAnalogPin(&buffer[7], size);
        break;
    case TYPEFPGAOPERATIONS:
        radioProcessFPGAOperations(&buffer[7], size);
        break;
    case TYPEI2COPERATIONS:
        radioProcessI2COperations(&buffer[7], size);
        break;
    case TYPEUARTOPERATIONS:
        radioProcessUARTOperations(&buffer[7], size);
        break;
    case 0x20:
        //Configuration packet received
        radioProcessConfigurationPacket(&buffer[7], size);
        break;
    case TYPERESETSATELLITE:
        //Reset writting an incorrect value to WDTCTL
        memory_logEvent_noPayload(getUnixTimeNow(), EVENT_MANUALREBOOT);
        WDTCTL = 0x0000;
        break;	//It will never arrive here
    case TYPEHAMOPERATIONS:
        //Process packet for HAM operations
        radioProcessHAMPacket(&buffer[7], size);
        break;	//It will never arrive here
    case TYPEDEBUG:
        radioProcessDebugPacket(&buffer[7], size);
        break;
    default:
        //if(satelliteConfiguration_.debugIsOn == 1)
        //	abacus_uart_print(DEBUGUART, "Unknown packet received...");
        break;
    }

}

