#include "radio_lithium2.h"

struct Radio_configuration radioLithium;

/*
 * It initiates the UART and initial configuration of radio
 */
int8_t radio_init(uint8_t uartName, uint8_t beaconInterval, uint8_t *beaconData,
                  uint8_t power)
{
    //Initialize the radio struct:
    radioLithium.uartName = uartName;
    radioLithium.uartSpeed = AB_B9600;
    radioLithium.triggerRxFunction = radio_interrupt_trigger;
    radioLithium.dataAvailable = 0;
    radioLithium.bufferInPosition = 0;
    radioLithium.lastTimeReceived = 0;

    //Start UART from Abacus
    abacus_uart_open_function(radioLithium.uartName, radioLithium.uartSpeed,
                              radioLithium.triggerRxFunction);

    //Set reset and external event pinouts
    //RADIORESETPIN	AB_H1_14
    abacus_gpio_digitalWrite(RADIO_RESET, AB_HIGH);
    abacus_gpio_digitalMode(RADIO_RESET, AB_OUTPUT);
    abacus_gpio_digitalWrite(RADIO_RESET, AB_HIGH);

    //Enable interrupts
    abacus_uart_enableInterrupt(radioLithium.uartName);

    //Send NOOP to radio
    radio_sendCommand(LITHIUM_NO_OP_COMMAND, radioLithium.bufferOut, 0);

    //Wait for radio to answer
    if (radio_lockUntilAvailable() == -1)
    {
        //log event
        //memory_logEvent_noPayload(getUnixTimeNow(),EVENT_RADIOTIMEOUT);
        return -1;
    }
    else
    {
        uint8_t code;
        if (radio_readAckNack(&code) != 0)
        {
            //log event
            //memory_logEvent_noPayload(getUnixTimeNow(),EVENT_RADIONACK);
        }
    }

    //Send reset to radio?
    //radio_reset(1);

    //Set power
    radio_setPowerAmp(power);

    //Set beaconInterval
    radioLithium.beaconConfiguration.beacon_interval = beaconInterval;
    radio_setBeaconConfiguration(&radioLithium.beaconConfiguration);

    //Set beaconData
    uint8_t lenght = 0;
    for (lenght = 0; lenght < 50; lenght++)
        if (beaconData[lenght] == '\0')
            break;
    radio_setBeaconData(beaconData, lenght);

    //Enable "from earth" external interrupt
    //RADIOEXTPIN	AB_H1_13
    abacus_gpio_digitalMode(RADIOINTERRUPT, AB_INPUT);
    //abacus_gpio_digitalEnableInterrupt(RADIOEXTPIN, radio_interrupt_externalPin, AB_LOW2HIGH);

    return 0;
}

/*
 * Interrupt that is triggered when the radio activates it emergency pin
 */
void radio_interrupt_externalPin(uint8_t *exitLowPower)
{
    //Lock the code to make the WDT jump?
    uint16_t i;
    for (i = 0; i < 1000; i++)
    {
        abacus_sleep_sec(1);
        //Gomspace will reboot after 180sec
    }
    *exitLowPower = 1;
}

/*
 * Reset radio via hardware pin or reset software command
 */
int8_t radio_reset(uint8_t hardware)
{
    if (hardware == 0)
    {
        //Send a software reset
        radio_sendCommand(LITHIUM_RESET_SYSTEM, radioLithium.bufferOut, 0);
        //Wait for radio to answer
        if (radio_lockUntilAvailable() == -1)
        {
            //log event
            /*
             memory_logEvent_noPayload(
             getUnixTimeNow(),
             EVENT_RADIOTIMEOUT);
             */
        }
        else
        {
            uint8_t code;
            //log event
            if (radio_readAckNack(&code) != 0)
            {
                /*
                 memory_logEvent_noPayload(getUnixTimeNow(),
                 EVENT_RADIONACK);
                 }
                 */
            }

            return 0;

        }
    }
    //HARDWARE
    abacus_gpio_digitalWrite(RADIO_RESET, AB_LOW);
    abacus_sleep_msec(100);
    abacus_gpio_digitalWrite(RADIO_RESET, AB_HIGH);

    return 0;
}

/*
 * This function is triggered everytime the UART receives a character.
 * The user must decide if he wants the MCU to exit from lowpower mode (only
 * if low power mode was selected.
 */
void radio_interrupt_trigger(uint8_t *exitLowPower)
{
    if (radioLithium.dataAvailable != 0)
    {
        //Error, user did not yet read the buffer. We have to reset everything
        radioLithium.dataAvailable = 0;
        radioLithium.bufferInPosition = 0;
    }

    //Read whatever it is on the UART buffer
    while (abacus_uart_available(radioLithium.uartName) > 0)
    {
        radioLithium.bufferIn[radioLithium.bufferInPosition] = abacus_uart_read(
                radioLithium.uartName);
        radioLithium.bufferInPosition++;

        if (radioLithium.bufferInPosition > (RADIO_BUFFER_SIZE - 2))
            radioLithium.bufferInPosition = 0;	//Prevent overflow
    }

    //Sync?:
    if (radioLithium.bufferInPosition == 1)
    {
        if (!(radioLithium.bufferIn[0] == 'H'))
        {
            //We are out of sync!!! reset buffer
            radioLithium.bufferInPosition = 0;
        }
    }
    else if (radioLithium.bufferInPosition == 2)
    {
        if (!(radioLithium.bufferIn[0] == 'H' && radioLithium.bufferIn[1] == 'e'))
        {
            //We are out of sync!!! reset buffer
            radioLithium.bufferInPosition = 0;
        }

    }

    //Lenght arrived?
    uint16_t payloadLength = 8;
    if (radioLithium.bufferInPosition > 4)
    {
        if (radioLithium.bufferIn[4] != 0x00)
            payloadLength = 0;
        else
            payloadLength = radioLithium.bufferIn[5];
    }

    //Header arrived?
    if (radioLithium.bufferInPosition == 8)
    {
        //Time to check the checksum of the header
        uint8_t checkSumA, checkSumB;
        //Check the checksums
        radio_calculateChecksums(&radioLithium.bufferIn[2], 4, &checkSumA,
                                 &checkSumB);

        if (!(checkSumA == radioLithium.bufferIn[6]
                && checkSumB == radioLithium.bufferIn[7]))
        {
            //Checksum was incorrect. Go home!
            radioLithium.bufferInPosition = 0;
        }
    }

    //Command without payload?
    if (payloadLength == 0 && radioLithium.bufferInPosition == 8)
    {
        //Received a command without payload:
        radioLithium.dataAvailable = 1;

        //add time
        radioLithium.lastTimeReceived = abacus_millis();

        //Wake up MCU if it was sleeping
        *exitLowPower = 1;
        return;
    }

    //Command with payload?
    if (payloadLength > 0
            && radioLithium.bufferInPosition == (10 + payloadLength))
    {
        //Received a command with payload!
        //Checksums:

        //Time to check the checksum of the header
        uint8_t checkSumA, checkSumB;
        //Check the checksums
        radio_calculateChecksums(&radioLithium.bufferIn[2], 6 + payloadLength,
                                 &checkSumA, &checkSumB);

        if (!(checkSumA == radioLithium.bufferIn[8 + payloadLength + 0]
                && checkSumB == radioLithium.bufferIn[8 + payloadLength + 1]))
        {
            //Checksum was incorrect. Go home!
            radioLithium.bufferInPosition = 0;
        }
        else
        {
            radioLithium.dataAvailable = 2;

            //add time
            radioLithium.lastTimeReceived = abacus_millis();

            //Wake up MCU if it was sleeping
            *exitLowPower = 1;
            return;
        }
    }

    //We don't want to exit from lowpower
    *exitLowPower = 0;
}

/*
 * It sends data to transmit to the radio
 */
int8_t radio_txData(uint8_t *data, uint8_t length)
{
    //Just send the raw command with the correct code:
    return radio_sendCommand(LITHIUM_TRANSMIT_DATA, data, length);
}

/*
 * It sets the power of the amplifier. Value goes from 0 to 255 and it is not
 * linear
 */
int8_t radio_setPowerAmp(uint8_t power)
{
    return radio_sendCommand(LITHIUM_FAST_PA_SET, &power, 1);
}

/*
 * Returns wether there is data available or not:
 * 0: no new packets available
 * 1: an ACK/NACK packet available
 * 2: a data packet available
 */
int8_t radio_available()
{
    return radioLithium.dataAvailable;
}

/*
 * Lock until available or timeout
 */
int8_t radio_lockUntilAvailable()
{
    int counter = 0;
    //Wait for answer from radio
    while (radio_available() == 0)
    {
        //No deadlock if radio is broken please
        if (counter > 10000)
        {
            //Return timeout
            return -1;
        }
        counter++;
    }
    return radio_available();
}

/*
 * This function evaluates the last received packet. If packet was ACK or NACK
 * it will delete the buffered packet. It will not delete the buffer if it is
 * not an ACK/NACK packet
 * Returns:
 * -3 Unknown packet received
 * -2 if it was not an ACK NACK packet
 * -1 if nothing was to read
 * 0  if ACK
 * 1  if NACK
 */
int8_t radio_readAckNack(uint8_t *code)
{
    if (radioLithium.dataAvailable == 0)
        return -1;	//There was no new packets
    if (radioLithium.dataAvailable == 2)
        return -2;	//It was a data packet! error!

    *code = radioLithium.bufferIn[3];

    //Ok packet was an ACK/NACK reset buffer:
    radioLithium.bufferInPosition = 0;
    radioLithium.dataAvailable = 0;

    //Return if it was an ACK or NACK or unknown
    if (radioLithium.bufferIn[4] == 0x0A)
    {
        satelliteStatus_.errors &= ~ERRORRADIO;
        return 0;
    }
    else if (radioLithium.bufferIn[4] == 0xFF)
        return 1;

    //If it is here, packet was an unknown packet
    return -3;
}

/*
 * It returns the buffered payload and only the payload. This function resets
 * the arrived packet. It returns:
 * -2 if it was not a data packet
 * -1 if nothing was to read
 * 0 if data was copied to the array
 */
int8_t radio_readData(uint8_t *data, uint8_t *length, uint8_t *code)
{
    if (radioLithium.dataAvailable == 0)
        return -1;	//There were no new packets
    if (radioLithium.dataAvailable == 1)
        return -2;	//It was an ACK/NACK packet! error!

    *code = radioLithium.bufferIn[3];
    *length = radioLithium.bufferIn[5];

    satelliteStatus_.errors &= ~ERRORRADIO;

    //Copy payload data
    uint16_t i = 0;
    for (i = 0; i < (radioLithium.bufferInPosition - 10); i++)
        data[i] = radioLithium.bufferIn[i + 8];

    //Reset buffers
    radioLithium.bufferInPosition = 0;
    radioLithium.dataAvailable = 0;

    return 0;
}

/*
 * Decodes the x25 packet
 */
int8_t radio_x25_decode(uint8_t *dataInput, uint8_t *dataLength,
                        uint32_t *destAddress, uint32_t *sourceAddress,
                        uint8_t *controlField, uint8_t *protocolId,
                        uint8_t *dataOut, uint8_t *dataOutLength)
{
    dataOut = &(dataInput[16]);
    *dataOutLength = *dataLength - 18;
    //TODO the other fields
    return 0;
}

/*
 * It sends all the bytes to the UART no matter if they are correct or not.
 * You should not use this function unless you know what you are doing.
 */
int8_t radio_sendRawCommand(uint8_t *data, uint16_t length)
{
    int8_t result = abacus_uart_write(radioLithium.uartName, data, length);
    return result;
}

/*
 * Sends a command to the radio
 */
int8_t radio_sendCommand(uint8_t command, uint8_t *data, uint8_t length)
{
    //Prepare the buffer:

    //Add the sync bytes:
    radioLithium.bufferOut[0] = 'H';	//0x48
    radioLithium.bufferOut[1] = 'e';	//0x65

    //Add the header bytes:
    radioLithium.bufferOut[2] = 0x10;	//I_MESSAGE_TYPE
    radioLithium.bufferOut[3] = command;	//TX command
    radioLithium.bufferOut[4] = 0x00;	//Always 0
    radioLithium.bufferOut[5] = length;	//Payload length

    //Add the checksum of the header on 6 and 7 (no sync):
    radio_calculateChecksums(&radioLithium.bufferOut[2], 4,
                             &radioLithium.bufferOut[6],
                             &radioLithium.bufferOut[7]);

    //At this moment total lenght is:
    //2 bytes sync
    //2 bytes command
    //2 bytes lenght
    //2 bytes header checksum
    uint16_t totalLenghtToSend = 8;

    //Is there a payload to send?
    if (length > 0)
    {
        //Add the payload:
        uint16_t i;
        for (i = 0; i < length; i++)
        {
            radioLithium.bufferOut[8 + i] = data[i];
        }

        //The checksum of header and payload (no sync):
        radio_calculateChecksums(&(radioLithium.bufferOut[2]), (6 + length),
                                 &(radioLithium.bufferOut[8 + length + 0]),
                                 &(radioLithium.bufferOut[8 + length + 1]));

        //Buffer is
        //8 bytes header
        //length bytes payload
        //2 bytes checksum
        totalLenghtToSend = (8 + 2 + length);
    }

    //Send to the uart:
    return radio_sendRawCommand(radioLithium.bufferOut, totalLenghtToSend);
}

/*
 * It calculates the Checksum of the data to be sent to the radio as specified
 * in the document "Radio_Interface_Manual_01152013.pdf" page 2.
 * This checksum is compulsory for all the transmitted data or commands.
 */
void radio_calculateChecksums(uint8_t *data, uint16_t length,
                              uint8_t *checksum01, uint8_t *checksum02)
{
    uint8_t lithium_CKS_A = 0;
    uint8_t lithium_CKS_B = 0;
    uint16_t i;
    for (i = 0; i < length; i++)
    {
        lithium_CKS_A = lithium_CKS_A + data[i];
        lithium_CKS_B = lithium_CKS_B + lithium_CKS_A;
    }

    *checksum01 = lithium_CKS_A;
    *checksum02 = lithium_CKS_B;
}

/*
 * It reads a struct in raw mode
 */
int8_t radio_readRawStruct(uint8_t index, uint8_t *buffer, uint8_t size)
{
    //Send command to radio
    radio_sendCommand(index, radioLithium.bufferOut, 0);
    //Wait for radio to answer
    if (radio_lockUntilAvailable() == -1)
    {
        satelliteStatus_.errors |= ERRORRADIO;
        return -1;
    }
    else
    {
        uint8_t length, code;
        radio_readData(buffer, &length, &code);
        return 0;
    }
}

/*
 * It writes the struct in raw mode
 */
int8_t radio_writeRawStruct(uint8_t index, uint8_t *buffer, uint8_t size)
{
    //Send command to radio
    radio_sendCommand(index, buffer, size);
    return 0;
}

/*
 * Reads the general configuration of the radio
 */
int8_t radio_readconfiguration(struct Lithium_configuration *configuration)
{
    //Send command to radio
    radio_sendCommand(LITHIUM_GET_TRANSCEIVER_CONFIG, radioLithium.bufferOut,
                      0);

    //Wait for radio to answer
    if (radio_lockUntilAvailable() == -1)
        satelliteStatus_.errors |= ERRORRADIO;
    else
    {
        uint8_t code;
        uint8_t buffer[36];	//34bytes in reality
        uint8_t length;
        if (radio_readData(buffer, &length, &code) == 0)
        {
            configuration->interface_baud_rate = buffer[0];
            configuration->tx_power_amp_level = buffer[1];
            configuration->rx_rf_baud_rate = buffer[2];
            configuration->tx_rf_baud_rate = buffer[3];
            configuration->rx_modulation = buffer[4];
            configuration->tx_modulation = buffer[5];
            char2ulong(&buffer[6], &configuration->rx_freq);
            char2ulong(&buffer[10], &configuration->tx_freq);
            uint8_t i;
            for (i = 0; i < 6; i++)
            {
                configuration->source[i] = buffer[14 + i];
                configuration->destination[i] = buffer[20 + i];
            }
            char2uint(&buffer[26], &configuration->tx_preamble);
            char2uint(&buffer[28], &configuration->tx_postamble);
            char2uint(&buffer[30], &configuration->function_config);
            char2uint(&buffer[32], &configuration->function_config2);
            return 0;
        }

    }
    return -1;
}

/*
 * It reads the telemetry of the radio.
 */
int8_t radio_readTelemetry(struct Lithium_telemetry *telemetry)
{
    //Send command to radio
    radio_sendCommand(LITHIUM_TELEMETRY_QUERY, radioLithium.bufferOut, 0);

    //Wait for radio to answer
    if (radio_lockUntilAvailable() == -1)
        satelliteStatus_.errors |= ERRORRADIO;
    else
    {
        uint8_t code;
        uint8_t buffer[20];
        uint8_t length;
        if (radio_readData(buffer, &length, &code) == 0)
        {
            char2uint(buffer, &telemetry->op_counter);
            char2int(&buffer[2], &telemetry->msp430_temp);
            telemetry->time_count[0] = buffer[4];
            telemetry->time_count[1] = buffer[5];
            telemetry->time_count[2] = buffer[6];
            telemetry->rssi = buffer[7];

            char2ulong(&buffer[8], &telemetry->bytes_received);
            char2ulong(&buffer[12], &telemetry->bytes_transmitted);
            return 0;
        }

    }
    return -1;
}

/*
 * Sets the configuration of the radio. Please first read it and then
 * overwrite it
 */
int8_t radio_setConfiguration(struct Lithium_configuration *configuration)
{
    uint8_t bufferOut[34];

    bufferOut[0] = configuration->interface_baud_rate;
    bufferOut[1] = configuration->tx_power_amp_level;
    bufferOut[2] = configuration->rx_rf_baud_rate;
    bufferOut[3] = configuration->tx_rf_baud_rate;
    bufferOut[4] = configuration->rx_modulation;
    bufferOut[5] = configuration->tx_modulation;
    ulong2char(&configuration->rx_freq, &bufferOut[6]);
    ulong2char(&configuration->tx_freq, &bufferOut[10]);
    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        bufferOut[14 + i] = configuration->source[i];
        bufferOut[20 + i] = configuration->destination[i];
    }
    uint2char(&configuration->tx_preamble, &bufferOut[26]);
    uint2char(&configuration->tx_postamble, &bufferOut[28]);
    uint2char(&configuration->function_config, &bufferOut[30]);
    uint2char(&configuration->function_config2, &bufferOut[32]);

    return radio_sendCommand(LITHIUM_SET_TRANSCEIVER_CONFIG, bufferOut, 34);
}

/*
 * Sets the low level RF configuration of the radio.
 * 	front_end level from 0 to 63
 * 	tx_power_amp_level from 0 to 255
 * 	tx_frequency_offset from 0 to 20kHz
 * 	rx_frequency_offset from 0 to 20kHz
 */
int8_t radio_setRFconfiguration(
        struct Lithium_rf_configuration *rfConfiguration)
{
    uint8_t bufferOut[10];

    bufferOut[0] = rfConfiguration->front_end_level;
    bufferOut[1] = rfConfiguration->tx_power_amp_level;
    ulong2char(&rfConfiguration->tx_frequency_offset, &bufferOut[2]);
    ulong2char(&rfConfiguration->rx_frequency_offset, &bufferOut[6]);

    return radio_sendCommand(LITHIUM_RF_CONFIG, bufferOut, 10);
}

/*
 * Sets the time interval of the beacon. Each bit is 2.5 seconds, 0 is disabled
 */
int8_t radio_setBeaconConfiguration(
        struct Lithium_beacon_configuration *beaconConfiguration)
{
    uint8_t bufferOut;
    bufferOut = beaconConfiguration->beacon_interval;
    return radio_sendCommand(LITHIUM_BEACON_CONFIG, &bufferOut, 1);
}

/*
 * Sets the data in the beacon. maximum payload size are 255 bytes. but don't
 * go beyond 200 for the gods shake
 */
int8_t radio_setBeaconData(uint8_t *data, uint8_t lenght)
{
    return radio_sendCommand(LITHIUM_BEACON_DATA, data, lenght);
}
