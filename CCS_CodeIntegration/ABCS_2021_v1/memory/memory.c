/*
 * memory.c
 */
#include "memory.h"

struct MemorySatellite satellite_memory;
uint32_t counts;
uint32_t npacket;

//uint8_t bufferMem[250]; //static variable to save memory? It will be used by everyone...

/*
 * It reads the MCU flash memory and loads indexes to know where to write
 * next sensor, events and pictures
 */
int8_t memory_init(uint8_t readConf)
{   uint16_t errors=0;
    //Ok, abacus has already initiated the SPI

    //Use MCU as default:
    satellite_memory.selectedMainMemory = AB_FLASH_MCU;

    //Read Configuration of Satellite!
    if (readConf == 1)
        if (memory_readConf(&satelliteConfiguration_) != 0)
        {
            //Save new default configuration because it failed
            memory_writeConf(&satelliteConfiguration_);
        }
    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FFUL) | WDTPW | WDTCNTCL;
    //Lets put a very long WDT for this operation
    WDTCTL = WDTPW | (uint16_t) WDT_MRST_XXL;

    errors += memory_sensors_init();

    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FFUL) | WDTPW | WDTCNTCL;

    errors += memory_event_init();
        //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FFUL) | WDTPW | WDTCNTCL;

    errors += memory_marie_init();

    if (errors != 0)
        return -1;
    else
        return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

uint8_t memory_mapFindAddresses(uint32_t addressDataStart,
                                uint32_t addressDataEnd, uint16_t dataSize,
                                uint32_t *startfreeaddress,
                                uint32_t *endfreeaddress)
{
    int8_t errors = 0U;
    uint8_t found = 0U;
    uint8_t buffer=0;
    uint32_t address = addressDataStart;
    //uint16_t i = 0U;

    // Look for 0's. If none the memory is free
    while ((found == 0U) && (address < addressDataEnd))
    {
        errors += abacus_flash_read_data(satellite_memory.selectedMainMemory,
                                         address, &buffer, 1U); //read just 1st byte of sensor's data packet

        if (buffer != 0xFFU)
        {
            found = 1U; // found written memory page
            //set here the (temp) End Free Address of the circular memory
            //if (i == 0U)
            if (address == addressDataStart)
                *endfreeaddress = addressDataEnd;
            else
                *endfreeaddress = address - 1U;
        }
        //i++;
        address += dataSize;
    }

    if (found == 0U)
    {
        //Memory is empty
        *startfreeaddress = addressDataStart;
        *endfreeaddress = addressDataEnd;
        //satellite_memory.sensorsStoredPage = 0UL;
        //satellite_memory.sensorsStoredPacket = 0UL;
        //return 0;
    }
    else
    {
        //Memory is not empty,
        //let's find the start of the Start Free Address of the circular memory
        found = 0U;
        while ((found == 0U) && (address < addressDataEnd))
        {
            errors += abacus_flash_read_data(
                    satellite_memory.selectedMainMemory, address, &buffer, 1U); //read just 1st byte of sensor's data packet
            // search for the 1's
            if (buffer == 0xFFU)
            {
                found = 1U;
                *startfreeaddress = address;
            }
            address += dataSize;
        }

        if (found == 0U)
        {
            // the memory is in the "111111110000" case
            // Start Free Address is MEMORY_SENSORS_ADD
            *startfreeaddress = addressDataStart;
        }
        else
        {
            //let's find the real End Free Address of the circular memory
            found = 0U;
            while ((found == 0U) && (address < addressDataEnd))
            {
                errors += abacus_flash_read_data(
                        satellite_memory.selectedMainMemory, address, &buffer,
                        1U);

                if (buffer != 0xFFU)
                {
                    found = 1U;
                    *endfreeaddress = address - 1U;
                }
                address += dataSize;
            }
            //if found==0 then the temp End Free Address is the real one
            //no need to do anything
        }

    }
    return errors;
}

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////         SENSORS        ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/*
 * Find how much memory has been already used for sensors
 */
uint8_t memory_sensors_init()
{
    uint8_t result=0;
    result = memory_mapFindAddresses(MEMORY_SENSORS_START, MEMORY_SENSORS_END,
    SENSORS_SIZE_STREAM,
                                     &satellite_memory.sensorsStartFreeAddress,
                                     &satellite_memory.sensorsEndFreeAddress);

    uint32_t freeByte, occupiedByte;

    if (satellite_memory.sensorsStartFreeAddress
            < satellite_memory.sensorsEndFreeAddress)
    {
        freeByte = (satellite_memory.sensorsEndFreeAddress
                - satellite_memory.sensorsStartFreeAddress) + 1UL;
        occupiedByte = (uint32_t) (((uint32_t) MEMORY_SENSORS_COMPLETE_SIZE)
                - freeByte);
        satellite_memory.sensorsStoredPacket = (uint32_t) (occupiedByte
                / ((uint32_t) (SENSORS_SIZE_STREAM)));
    }
    else if (satellite_memory.sensorsStartFreeAddress
            > satellite_memory.sensorsEndFreeAddress)
    {
        occupiedByte = (satellite_memory.sensorsStartFreeAddress
                - satellite_memory.sensorsEndFreeAddress) - 1L;
        satellite_memory.sensorsStoredPacket = (uint32_t) (occupiedByte
                / ((uint32_t) SENSORS_SIZE_STREAM));
    }

    return result;
}

/*
 *  It erases all the sensor data. It takes some seconds!
 */
uint8_t memory_sensors_formatAll(void)
{

    uint16_t i;
    uint32_t address = MEMORY_SENSORS_START;

    //Now erase all the sectors except the first because contain sensor memory map
    for (i = 0U; i < MEMORY_SENSORS_SECTORS; i++)
    {
        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     60000U, 1); //abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U, 1U);

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_ALLSECTORERASESENSOR,
                                       address);
            return 255U;
        }

        address += MEMORY_BYTES_PER_SECTOR;

    }

    memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                               EVENT_ALLSECTORSERASEDSENSOR, address);

    satellite_memory.sensorsStartFreeAddress = MEMORY_SENSORS_START;
    satellite_memory.sensorsEndFreeAddress = MEMORY_SENSORS_END;
    satellite_memory.sensorsStoredPacket = 0UL;

    return 0U;
}

/*
 * Writes all sensors to flash
 */
// nel byte 0 di buffer c'il byte con tutti 0 per segnalare che la memoria è stata scritta (indice sparso)
uint8_t memory_sensors_write(uint8_t *buffer)
{
    //Sanity check: Aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e quindi bisogna ricominciare il ciclo.
    if (satellite_memory.sensorsStartFreeAddress
            > (MEMORY_SENSORS_END + 1UL - SENSORS_SIZE_STREAM))
    {
        satellite_memory.sensorsStartFreeAddress = MEMORY_SENSORS_START;
    }

    //Wait while flash is busy:
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 1);

    buffer[0U] = 0xFFU;
    buffer[0U] &= ~1; // sto mettendo il bit 0 (del byte 0U) al valore 0 -> è il flag che indica che quella porzione di memoria è scritta
    int8_t result = abacus_flash_write_dataEasy(
            satellite_memory.selectedMainMemory,
            satellite_memory.sensorsStartFreeAddress, buffer,
            SENSORS_SIZE_STREAM);

    satellite_memory.sensorsStartFreeAddress =
            satellite_memory.sensorsStartFreeAddress + SENSORS_SIZE_STREAM;
    satellite_memory.sensorsStoredPacket += 1UL;

    // Memoria ciclica, ora aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e devo ricominciare il ciclo
    if (satellite_memory.sensorsStartFreeAddress
            > (MEMORY_SENSORS_END + 1UL - SENSORS_SIZE_STREAM))
    {
        satellite_memory.sensorsStartFreeAddress = MEMORY_SENSORS_START;
    }

    // ora devo controllare se sta terminando lo spazio dedicato alla memoria, in questo caso devo formattare dei settori
    if (satellite_memory.sensorsStoredPacket > MEMORY_SENSORS_PACKETSLIMIT)
    {
        // so I erase one sector
        memory_sensors_formatSector();
    }

    return result;
}

uint8_t memory_sensor_checkFormatSector(void)
{
    if (satellite_memory.sensorsStoredPacket > MEMORY_SENSORS_PACKETSLIMIT)
    {
        // so I erase one sector
        uint32_t address = satellite_memory.sensorsEndFreeAddress + 1UL;

        if (address > MEMORY_SENSORS_END)
        {
            address = MEMORY_SENSORS_START;
        }

        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     50000U, 0); // !! HO MESSO A 0

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_SECTORERASESENSOR, address);
            return 255U;
        }

        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_SECTORERASEDSENSOR, address);
        satellite_memory.sensorsEndFreeAddress = address
                + MEMORY_BYTES_PER_SECTOR - 1UL;
        satellite_memory.sensorsStoredPacket =
                satellite_memory.sensorsStoredPacket
                        - MEMORY_SENSORS_PACKETS_PER_SECTOR;
        return 1U;
    }
    return 0U;
}

uint8_t memory_sensors_formatSector(void)
{
    uint32_t address = satellite_memory.sensorsEndFreeAddress + 1UL;

    if (address > MEMORY_SENSORS_END)
    {
        address = MEMORY_SENSORS_START;
    }

    //Wait while flash is busy:
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 0); // !! HO MESSO A 0

    if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address)
            != 0)
    {
        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_ERROR_SECTORERASESENSOR, address);
        return 255U;
    }

    memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                               EVENT_SECTORERASEDSENSOR, address);
    satellite_memory.sensorsEndFreeAddress = address + MEMORY_BYTES_PER_SECTOR
            - 1UL;
    satellite_memory.sensorsStoredPacket = satellite_memory.sensorsStoredPacket
            - (MEMORY_BYTES_PER_SECTOR / SENSORS_SIZE_STREAM);

    return 0U;
}

/*
uint8_t memory_sensor_downloadPacket(void)
{
    bufferMem[0U + RADIOHEADERSIZE] = 'M';
    bufferMem[1U + RADIOHEADERSIZE] = 'T';
    bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
    ulong2char(&npacket, &bufferMem[3U + RADIOHEADERSIZE]);
    ulong2char(&satellite_memory.readAddress,
               &bufferMem[7U + RADIOHEADERSIZE]);
    ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);

    if (abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     40000U, 0) == -1)
        return 255U; //abacus_uart_print(DEBUGUART," MEMORIA OCCUPATA DURANTE LA LETTURA ");

    abacus_flash_read_data(satellite_memory.selectedMainMemory,
                           satellite_memory.readAddress,
                           &bufferMem[15 + RADIOHEADERSIZE],
                           SENSORS_SIZE_STREAM);

    // now I have to set low the thir bit of the flag byte to say I have sent this packet
    radioWrite(bufferMem, 79U);

    return 0U;
}
*/
/*
uint8_t memory_sensors_getInfo(uint8_t *buffer)
{
    buffer[0U] = 'M';
    buffer[1U] = 'T';
    buffer[2U] = 4U;
    ulong2char(&satellite_memory.sensorsStartFreeAddress, &buffer[3U]);
    ulong2char(&satellite_memory.sensorsStoredPacket, &buffer[7U]);
    ulong2char(&satellite_memory.sensorsEndFreeAddress, &buffer[11U]);
    buffer[15U] = satelliteConfiguration_.telemetryReadInterval;

    return 0U;
}
*/
/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////              EVENTS          /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/*
 * Find how much memory has been already used for events
 */
uint8_t memory_event_init()
{
    uint8_t result;
    result = memory_mapFindAddresses(MEMORY_EVENT_START, MEMORY_EVENT_END,
    EVENT_SIZE_STREAM,
                                     &satellite_memory.eventStartFreeAddress,
                                     &satellite_memory.eventEndFreeAddress);

    uint32_t freeByte, occupiedByte;

    if (satellite_memory.eventStartFreeAddress
            < satellite_memory.eventEndFreeAddress)
    {
        freeByte = (satellite_memory.eventEndFreeAddress
                - satellite_memory.eventStartFreeAddress) + 1UL;
        occupiedByte = (uint32_t) (((uint32_t) MEMORY_EVENT_COMPLETE_SIZE)
                - freeByte);
        satellite_memory.eventStoredPacket = (uint32_t) (occupiedByte
                / ((uint32_t) (EVENT_SIZE_STREAM)));
    }
    else if (satellite_memory.eventStartFreeAddress
            > satellite_memory.eventEndFreeAddress)
    {
        occupiedByte = (satellite_memory.eventStartFreeAddress
                - satellite_memory.eventEndFreeAddress) - 1UL;
        satellite_memory.eventStoredPacket = (uint32_t) (occupiedByte
                / ((uint32_t) EVENT_SIZE_STREAM));
    }

    return result;
}

/*
 *  It erases all the event data. It takes some seconds!
 */
uint8_t memory_event_formatAll(void)
{
    uint16_t i;
    uint32_t address = MEMORY_EVENT_START;

    //Now erase all the sectors except the first because contain sensor memory map
    for (i = 0U; i < MEMORY_EVENT_SECTORS; i++)
    {
        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     60000U, 0); // HO MESSO A 0 //abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U, 1U);

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_ALLSECTORERASEEVENT,
                                       address);
            return 255U;
        }

        address += MEMORY_BYTES_PER_SECTOR;

    }

    memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                               EVENT_ALLSECTORERASEDEVENT, address);
    satellite_memory.eventStartFreeAddress = MEMORY_EVENT_START;
    satellite_memory.eventEndFreeAddress = MEMORY_EVENT_END;
    satellite_memory.eventStoredPacket = 0UL;

    return 0U;
}

/*
 * It logs the event.
 */
//int8_t memory_logEvent(uint32_t time, uint8_t code)
//{
//
//    uint8_t bufferEvent[EVENT_SIZE_STREAM];
//    uint8_t *pointer;
//    pointer = bufferEvent;
//    *pointer = 0U; // mi fa da indice
//    pointer++; //1
//
//    uint32_t timeMillis = abacus_millis();
//
//    ulong2char(&timeMillis, pointer);
//    pointer += 4U; //5
//
//    ulong2char(&time, pointer);
//    pointer += 4U; //9
//
//    *pointer = code;
//    pointer++; //10
//
//    // Ho altri 6 byte, magari posso salvare il registro di stato o altre informazioni
//
//    return memory_event_write(bufferEvent);
//}

/*
 * It logs the event, with info passed by a buffer.
 */
//int8_t memory_logEventInfo(uint32_t time, uint8_t code, uint8_t *buffer)
//{
//
//    uint8_t bufferEvent[EVENT_SIZE_STREAM];
//    uint8_t *pointer;
//    pointer = bufferEvent;
//    *pointer = 0U; // counts map
//    pointer++; //1
//
//    *pointer = code;
//    pointer++; //2
//
//    ulong2char(&time, pointer);
//    pointer += 4U; //6
//
//    // Ho altri 10 byte, magari posso salvare il registro di stato o altre informazioni
//
//    *pointer = buffer[0U];
//    *pointer = buffer[1U];
//    *pointer = buffer[2U];
//
//    return memory_event_write(bufferEvent);
//}


/*
 * It logs the event and the data afterwards. you add a payload of information
 * in the data pointer.
 */
int8_t memory_logEvent(uint32_t time,
        uint8_t code,
        uint8_t *data,
        uint8_t lenght)
{
    uint8_t buffer[EVENT_SIZE_STREAM];
    uint8_t *pointer;
    pointer = buffer;

    uint32_t timeMillis = abacus_millis();

    ulong2char(&timeMillis, pointer);
    pointer += 4;

    ulong2char(&time, pointer);
    pointer += 4;

    *pointer = code;
    pointer++;

    uint8_t i;
    for(i = 0; i < (EVENT_SIZE_STREAM - 9); i++)
    {
        if(i < lenght)
            buffer[i + 9] = data[i];
        else
            buffer[i + 9] = 0xFF;
    }

    return memory_event_write(buffer);
}

/*
 * It does not use any payloads after the event
 */
int8_t memory_logEvent_noPayload(uint32_t time, uint8_t code)
{
    return memory_logEvent(time, code, 0, 0);
}

/*
 * It logs the event, with info passed by a buffer.
 */
int8_t memory_logSectorEraseEvent(uint32_t time, uint8_t code, uint32_t address)
{

    uint8_t bufferEvent[EVENT_SIZE_STREAM];
    uint8_t *pointer;
    pointer = bufferEvent;
    *pointer = 0; // counts map
    pointer++; //1

    *pointer = code;
    pointer++; //2

    ulong2char(&time, pointer);
    pointer += 4; //6

    // Ho altri 10 byte, magari posso salvare il registro di stato o altre informazioni

    *pointer = address >> 8;
    *pointer = address >> 16;
    *pointer = address >> 24;

    return memory_event_write(bufferEvent);
}

/*
 * Writes event to flash
 */
uint8_t memory_event_write(uint8_t *buffer)
{
    //Sanity check: Aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e quindi bisogna ricominciare il ciclo.
    if (satellite_memory.eventStartFreeAddress
            > (MEMORY_EVENT_END + 1U - EVENT_SIZE_STREAM))
    {
        satellite_memory.eventStartFreeAddress = MEMORY_EVENT_START;
    }

    //Wait while flash is busy:
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 1);

    buffer[0U] = 0xFFU;
    buffer[0U] &= ~1; // sto mettendo il bit 0 (del byte 0U) al valore 0 -> è il flag che indica che quella porzione di memoria è scritta
    int8_t result = abacus_flash_write_dataEasy(
            satellite_memory.selectedMainMemory,
            satellite_memory.eventStartFreeAddress, buffer,
            EVENT_SIZE_STREAM);

    satellite_memory.eventStartFreeAddress =
            satellite_memory.eventStartFreeAddress + EVENT_SIZE_STREAM;
    satellite_memory.eventStoredPacket += 1;

    // Memoria ciclica, ora aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e devo ricominciare il ciclo
    if (satellite_memory.eventStartFreeAddress
            > (MEMORY_EVENT_END + 1UL - EVENT_SIZE_STREAM))
    {
        satellite_memory.eventStartFreeAddress = MEMORY_EVENT_START;
    }

    // ora devo controllare se sta terminando lo spazio dedicato alla memoria, in questo caso devo formattare dei settori
    if (satellite_memory.eventStoredPacket > MEMORY_EVENT_PACKETSLIMIT)
    {
        // so I erase one sector
        memory_event_formatSector();
    }

    return result;
}

uint8_t memory_event_checkFormatSector(void)
{
    if (satellite_memory.eventStoredPacket > MEMORY_EVENT_PACKETSLIMIT)
    {
        // so I erase one sector
        uint32_t address = satellite_memory.eventEndFreeAddress + 1UL;

        if (address > MEMORY_EVENT_END)
        {
            address = MEMORY_EVENT_START;
        }

        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     50000U, 0); //!! ho messo 0

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_SECTORERASEEVENT, address);
            return 255U;
        }

        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_ERROR_SECTORERASEEVENT, address);
        satellite_memory.eventEndFreeAddress = address + MEMORY_BYTES_PER_SECTOR
                - 1UL;
        satellite_memory.eventStoredPacket = satellite_memory.eventStoredPacket
                - MEMORY_EVENT_PACKETS_PER_SECTOR;
        return 1U;
    }
    return 0U;
}

uint8_t memory_event_formatSector(void)
{
    uint32_t address = satellite_memory.eventEndFreeAddress + 1UL;

    if (address > MEMORY_EVENT_END)
    {
        address = MEMORY_EVENT_START;
    }

    //Wait while flash is busy:
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 1);

    if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address)
            != 0)
    {
        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_ERROR_SECTORERASEEVENT, address);
        return 255U;
    }

    memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                               EVENT_ERROR_SECTORERASEEVENT, address);
    satellite_memory.eventEndFreeAddress = address + MEMORY_BYTES_PER_SECTOR
            - 1UL;
    satellite_memory.eventStoredPacket = satellite_memory.eventStoredPacket
            - (MEMORY_BYTES_PER_SECTOR / EVENT_SIZE_STREAM);

    return 0U;
}
/*
uint8_t memory_event_downloadPacket(void)
{
    bufferMem[0U + RADIOHEADERSIZE] = 'M';
    bufferMem[1U + RADIOHEADERSIZE] = 'E';
    ulong2char(&npacket, &bufferMem[3U + RADIOHEADERSIZE]);
    ulong2char(&satellite_memory.readAddress,
               &bufferMem[7U + RADIOHEADERSIZE]);

    if (abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     40000U, 0) == -1)
        return 255U; //abacus_uart_print(DEBUGUART," MEMORIA OCCUPATA DURANTE LA LETTURA ");

    abacus_flash_read_data(satellite_memory.selectedMainMemory,
                           satellite_memory.readAddress,
                           &bufferMem[15U + RADIOHEADERSIZE],
                           EVENT_SIZE_STREAM);

    radioWrite(bufferMem, 31U);

    return 0U;
}
*/
uint8_t memory_event_getInfo(uint8_t *buffer)
{
    buffer[0U] = 'M';
    buffer[1U] = 'E';
    buffer[2U] = 3U;
    ulong2char(&satellite_memory.eventStartFreeAddress, &buffer[3U]);
    ulong2char(&satellite_memory.eventStoredPacket, &buffer[7U]);
    ulong2char(&satellite_memory.eventEndFreeAddress, &buffer[11U]);

    return 0U;
}

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////    MARIE      ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/*
 * Find how much memory has been already used for Marie
 */
uint8_t memory_marie_init()
{
    uint8_t result;
    result = memory_mapFindAddresses(MEMORY_MARIE_START, MEMORY_MARIE_END,
    MARIE_SIZE_STREAM_DATA,
                                     &satellite_memory.marieStartFreeAddress,
                                     &satellite_memory.marieEndFreeAddress);

    uint32_t freeByte, occupiedByte;

    if (satellite_memory.marieStartFreeAddress
            < satellite_memory.marieEndFreeAddress)
    {
        freeByte = (satellite_memory.marieEndFreeAddress
                - satellite_memory.marieStartFreeAddress) + 1U;
        occupiedByte = MEMORY_MARIE_COMPLETE_SIZE - freeByte;
        satellite_memory.marieStoredPage = occupiedByte / 256UL;
    }
    else if (satellite_memory.marieStartFreeAddress
            > satellite_memory.marieEndFreeAddress)
    {
        occupiedByte = (satellite_memory.marieStartFreeAddress
                - satellite_memory.marieEndFreeAddress) - 1U;
        satellite_memory.marieStoredPage = occupiedByte / 256UL;
    }

    return result;
}

/*
 * Save MARIE data stream
 */
uint8_t memory_marie_saveData(uint8_t *buffer)
{
    buffer[0U] = 0U; // I don't need to do this because the first byte of marie packet is 0, but I want to be sure: I'm writing the memory

    //Sanity check: Aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e quindi bisogna ricominciare il ciclo.
    if (satellite_memory.marieStartFreeAddress
            > (MEMORY_MARIE_END + 1U - MARIE_SIZE_STREAM_DATA))
    {
        satellite_memory.marieStartFreeAddress = MEMORY_MARIE_START;
    }

    //Wait while flash is busy:
    if (abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     40000U, 0) == -1)
    {
        // Memory busy before I'm going to store 256 byte of data //abacus_uart_print(DEBUGUART," MEMORIA OCCUPATA NEL SALVATAGGIO ");
        return 255U;
    }

    buffer[0U] = 0xFFU;
    buffer[0U] &= ~1U; // sto mettendo il bit 0 (del byte 0U) al valore 0 -> è il flag che indica che quella porzione di memoria è scritta
    int8_t result = abacus_flash_write_dataEasy(
            satellite_memory.selectedMainMemory,
            satellite_memory.marieStartFreeAddress, buffer,
            MARIE_SIZE_STREAM_DATA);
    if (result != 0)
    {
        return 255U; // error when I'm writing 256 byte of data
    }

    satellite_memory.marieStartFreeAddress =
            satellite_memory.marieStartFreeAddress + MARIE_SIZE_STREAM_DATA;
    satellite_memory.marieStoredPage += 1;

    // Memoria ciclica, ora aggiorno l'indirizzo di start nel caso sono giunto alla fine della memoria e devo ricominciare il ciclo
    if (satellite_memory.marieStartFreeAddress
            > (MEMORY_MARIE_END + 1U - MARIE_SIZE_STREAM_DATA))
    {
        satellite_memory.marieStartFreeAddress = MEMORY_MARIE_START;
    }

    // ora devo controllare se sta terminando lo spazio dedicato alla memoria, in questo caso devo formattare dei settori
    if (satellite_memory.marieStoredPage > MEMORY_MARIE_PAGESLIMIT)
    {
        // so I erase one sector
        memory_marie_formatSector();
    }

    return result;
}

uint8_t memory_marie_checkFormatSector(void)
{
    if (satellite_memory.marieStoredPage > MEMORY_MARIE_PAGESLIMIT)
    {
        // so I erase one sector
        uint32_t address = satellite_memory.marieEndFreeAddress + 1UL;

        if (address > MEMORY_MARIE_END)
        {
            address = MEMORY_MARIE_START;
        }

        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     50000U, 0U); // !!!!!!ho messo a 0

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_SECTORERASEMARIE, address);
            return 255U;
        }

        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_SECTORERASEDMARIE, address);
        satellite_memory.marieEndFreeAddress = address + MEMORY_BYTES_PER_SECTOR
                - 1U;
        satellite_memory.marieStoredPage = satellite_memory.marieStoredPage
                - MEMORY_PAGES_PER_SECTOR;

        return 1U;
    }
    return 0U;
}

uint8_t memory_marie_formatSector(void)
{
    uint32_t address = satellite_memory.marieEndFreeAddress + 1UL;

    if (address > MEMORY_MARIE_END)
    {
        address = MEMORY_MARIE_START;
    }

    //Wait while flash is busy:
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 0);

    if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address)
            != 0)
    {
        memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                   EVENT_ERROR_SECTORERASEMARIE, address);
        return 255U;
    }

    memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                               EVENT_SECTORERASEDMARIE, address);
    satellite_memory.marieEndFreeAddress = address + MEMORY_BYTES_PER_SECTOR
            - 1U;
    satellite_memory.marieStoredPage = satellite_memory.marieStoredPage
            - MEMORY_PAGES_PER_SECTOR;

    return 0U;
}

/*
 *  It erases all the sensor data. It takes some seconds!
 */
uint8_t memory_marie_formatAll(void)
{
    uint16_t i;
    uint32_t address = MEMORY_MARIE_START;

    //Now erase all the sectors except the first because contain sensor memory map
    for (i = 0U; i < MEMORY_MARIE_SECTORS; i++)
    {
        //Wait while flash is busy:
        abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     60000U, 1); //abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U, 1U);

        if (abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                                      address) != 0)
        {
            memory_logSectorEraseEvent(abacus_RTC_getUnixTime(),
                                       EVENT_ERROR_ALLSECTORERASEMARIE,
                                       address);
            return 255U;
        }

        address += MEMORY_BYTES_PER_SECTOR;

    }

    memory_logEvent_noPayload(abacus_RTC_getUnixTime(), EVENT_ALLSECTORSERASEDMARIE);
    satellite_memory.marieStartFreeAddress = MEMORY_MARIE_START;
    satellite_memory.marieEndFreeAddress = MEMORY_MARIE_END;
    satellite_memory.marieStoredPage = 0U;

    return 0U;
}
/*
uint8_t memory_marie_downloadHalfPage(void)
{
    bufferMem[0U + RADIOHEADERSIZE] = 'M';
    bufferMem[1U + RADIOHEADERSIZE] = 'M';
    ulong2char(&npacket, &bufferMem[3U + RADIOHEADERSIZE]);
    ulong2char(&satellite_memory.readAddress,
               &bufferMem[7U + RADIOHEADERSIZE]);

    if (abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory,
                                     40000U, 0) == -1)
        return 255U; //abacus_uart_print(DEBUGUART," MEMORIA OCCUPATA DURANTE LA LETTURA ");

    abacus_flash_read_data(satellite_memory.selectedMainMemory,
                           satellite_memory.readAddress,
                           &bufferMem[16U + RADIOHEADERSIZE],
                           MARIE_SIZE_STREAM_HALF_DATA);

    radioWrite(bufferMem, (MARIE_SIZE_STREAM_HALF_DATA + 16U));

    return 0U;
}
*/
/*
uint8_t memory_marie_getInfo(uint8_t *buffer)
{
    buffer[0U] = 'M';
    buffer[1U] = 'M';
    buffer[2U] = 3U;
    ulong2char(&satellite_memory.marieStartFreeAddress, &buffer[3U]);
    ulong2char(&satellite_memory.marieStoredPage, &buffer[7U]);
    ulong2char(&satellite_memory.marieEndFreeAddress, &buffer[11U]);

    return 0U;
}
*/
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    CONFIGURATION      /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/*
 * Read all the configurations of abacus 3 times!!! and check the CRC16 of all
 * of them and takes the correct value
 */
int8_t memory_readConf(struct SatelliteConfiguration *conf)
{
    uint8_t buffer[PERSISTENT_RAM_LENGTH];
    //Read the first 100 bytes of memory with the configuration:
    abacus_flash_read_data(satellite_memory.selectedMainMemory, MEMORY_CONF_START,
                           buffer, PERSISTENT_RAM_LENGTH);

    return satsystem_loadConfiguration(conf, buffer);
}

/*
 * Writes the configuraiton of the satellite 3 times and with its own CRC16 in
 * order to avoid loading the wrong data due to radiation events
 * Returns -1 if flash is not working, -2 if loaded correctly but with
 * at least one configuration error, -3 if there was no single memory with correct
 * values. And of course 0 if everything was fine :)
 */
int8_t memory_writeConf(struct SatelliteConfiguration *conf)
{
    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    //Lets put a very long WDT for this operation
    WDTCTL = WDTPW | (uint16_t) WDT_MRST_XXL;//(uint16_t) WDT_MRST_EXTRALONG;

    //First erase page sector:
    abacus_flash_sector_erase(satellite_memory.selectedMainMemory,
                              MEMORY_CONF_START);

    //Then write 3 times the configuration
    uint8_t buffer[PERSISTENT_RAM_LENGTH];

    //It serializes the configuration buffer:
    satsystem_saveConfiguration(conf, buffer);

    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

    //Wait while is busy
    abacus_flash_wait_while_busy(satellite_memory.selectedMainMemory, 50000U,
                                 0);

    //Kick the WDT
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

    //Write the new configuration:
    return abacus_flash_write_dataEasy(satellite_memory.selectedMainMemory,
                                       MEMORY_CONF_START, buffer,
                                       PERSISTENT_RAM_LENGTH);
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    CHECK MEMORY      /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/*
 * Function for parallel handling of the memory operations
 */
void checkMemoryOp()
{
    switch(satelliteStatus_.memory->statusParallel)
    {
    case 0x01:  //get sensors from to
        subOperationSensors();
        break;
    case 0x02:  //clean sensors
        subOperationCleanSensors();
        break;
    case 0x03:  //get events from to
        subOperationEvents();
        break;
    case 0x04:  //clean events
        subOperationCleansEvents();
        break;
    case 0x0A:  //get Marie data
        subOperationMarie();
        break;
    case 0x0B:  //clean Marie memory
        subOperationCleansMarie();
        break;
    case 0x0C:
        subOperationBulkErase();
        break;
    default:
        break;
    }
}


/*
 * Transmits the sensors from address to address.
 */
void subOperationSensors()
{   // dowload sensor memory from startAddr to endAddr
    // referred to the absolute sensor memory map
    // (0 is MEMORY_SENSORS_START)
    uint8_t bufferOut[280];
    uint32_t address;

    //Send packet only once every x ms:
    uint32_t timeNow = abacus_millis();
    if(timeNow
            < (satelliteStatus_.telemetryLastTimeTx + satelliteConfiguration_.radioMinimumInterval))
        return;

    if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
    {

        if(satellite_memory.parallelStartAddress > satellite_memory.parallelEndAddress)
        {
            //Time to end!
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            return;
        }
        address = MEMORY_SENSORS_START + satellite_memory.parallelStartAddress;

        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = satelliteConfiguration_.radioDataMaxSize + 5; //Size 3+2CRC?
        bufferOut[RADIOHEADERS - 1] = satelliteStatus_.memory->statusParallel;
        ulong2char(&satellite_memory.parallelStartAddress, &bufferOut[RADIOHEADERS]);
        //Read memory!
        abacus_flash_read_data(satellite_memory.selectedMainMemory,
                address,
                &bufferOut[RADIOHEADERS + 4],
                satelliteConfiguration_.radioDataMaxSize);
        //Send via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        //Move to next address
        satellite_memory.parallelStartAddress = satellite_memory.parallelStartAddress
                + satelliteConfiguration_.radioDataMaxSize;
        satelliteStatus_.memory->subStatusParallel = 1;
    }
}

/*
 * It formats the memory dedicated to the sensors
 */
void subOperationCleanSensors()
{
    uint32_t address;

    switch(satelliteStatus_.memory->subStatusParallel)
    {
    case 0:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satellite_memory.parallelStartAddress = 0;  //This is used as the counter index of sector
            address = MEMORY_SENSORS_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
            satelliteStatus_.memory->subStatusParallel = 1;
        }
        break;
    case 1:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            if(!(satellite_memory.parallelStartAddress < (MEMORY_SENSORS_COMPLETE_SIZE / MEMORY_BYTES_PER_SECTOR)))
            {
                //Finished with the memory
                satelliteStatus_.memory->subStatusParallel = 2;
                return;
            }
            satellite_memory.parallelStartAddress++;
            address = MEMORY_SENSORS_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
        }
        break;
    case 2: //Check if finished:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            memory_init(0);
            //Memory is ready
        }
        break;
    default:
        break;
    }
}


/*
 * It sends the events information from address to address
 */
void subOperationEvents()
{
    uint8_t bufferOut[280];
    uint32_t address;

    //Send packet only once every x ms:
    uint32_t timeNow = abacus_millis();
    if(timeNow
            < (satelliteStatus_.telemetryLastTimeTx + satelliteConfiguration_.radioMinimumInterval))
        return;

    if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
    {

        if(satellite_memory.parallelStartAddress > satellite_memory.parallelEndAddress)
        {
            //Time to end!
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            return;
        }
        address = MEMORY_EVENT_START + satellite_memory.parallelStartAddress;

        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = satelliteConfiguration_.radioDataMaxSize + 5; //Size
        bufferOut[RADIOHEADERS - 1] = satelliteStatus_.memory->statusParallel;
        ulong2char(&satellite_memory.parallelStartAddress, &bufferOut[RADIOHEADERS]);
        //Read memory!
        abacus_flash_read_data(satellite_memory.selectedMainMemory,
                address,
                &bufferOut[RADIOHEADERS + 4],
                satelliteConfiguration_.radioDataMaxSize);
        //Send via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        //Move to next address
        satellite_memory.parallelStartAddress = satellite_memory.parallelStartAddress
                + satelliteConfiguration_.radioDataMaxSize;
        satelliteStatus_.memory->subStatusParallel = 1;
    }
}


/*
 * It formats the memory dedicated to the sensors
 */
void subOperationCleansEvents()
{
    uint32_t address;

    switch(satelliteStatus_.memory->subStatusParallel)
    {
    case 0:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satellite_memory.parallelStartAddress = 0;  //This is used as the counter index of sector
            address = MEMORY_EVENT_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
            satelliteStatus_.memory->subStatusParallel = 1;
        }
        break;
    case 1:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            if(!(satellite_memory.parallelStartAddress < (MEMORY_EVENT_COMPLETE_SIZE / MEMORY_BYTES_PER_SECTOR)))
            {
                //Finished with the memory
                satelliteStatus_.memory->subStatusParallel = 2;
                return;
            }
            satellite_memory.parallelStartAddress++;
            address = MEMORY_EVENT_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
        }
        break;
    case 2:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            memory_init(0);
            //Memory is ready
        }
        break;
    default:
        break;
    }
}


/*
 * It sends the events information from address to address
 */
void subOperationMarie()
{
    uint8_t bufferOut[280];
    uint32_t address;

    //Send packet only once every x ms:
    uint32_t timeNow = abacus_millis();
    if(timeNow
            < (satelliteStatus_.telemetryLastTimeTx + satelliteConfiguration_.radioMinimumInterval))
        return;

    if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
    {

        if(satellite_memory.parallelStartAddress > satellite_memory.parallelEndAddress)
        {
            //Time to end!
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            return;
        }
        address = MEMORY_MARIE_START + satellite_memory.parallelStartAddress;

        bufferOut[RADIOHEADERS - 3] = TYPEMEMORYOPERATION;
        bufferOut[RADIOHEADERS - 2] = satelliteConfiguration_.radioDataMaxSize + 5; //Size
        bufferOut[RADIOHEADERS - 1] = satelliteStatus_.memory->statusParallel;
        ulong2char(&satellite_memory.parallelStartAddress, &bufferOut[RADIOHEADERS]);
        //Read memory!
        abacus_flash_read_data(satellite_memory.selectedMainMemory,
                address,
                &bufferOut[RADIOHEADERS + 4],
                satelliteConfiguration_.radioDataMaxSize);
        //Send via Radio
        sendRawRadioPacket(bufferOut, satelliteConfiguration_.radioRepetitions);
        //Move to next address
        satellite_memory.parallelStartAddress = satellite_memory.parallelStartAddress
                + satelliteConfiguration_.radioDataMaxSize;
        satelliteStatus_.memory->subStatusParallel = 1;
    }
}


/*
 * It formats the memory dedicated to the sensors
 */
void subOperationCleansMarie()
{
    uint32_t address;

    switch(satelliteStatus_.memory->subStatusParallel)
    {
    case 0:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satellite_memory.parallelStartAddress = 0;  //This is used as the counter index of sector
            address = MEMORY_MARIE_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
            satelliteStatus_.memory->subStatusParallel = 1;
        }
        break;
    case 1:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            if(!(satellite_memory.parallelStartAddress < (MEMORY_MARIE_COMPLETE_SIZE / MEMORY_BYTES_PER_SECTOR)))
            {
                //Finished with the memory
                satelliteStatus_.memory->subStatusParallel = 2;
                return;
            }
            satellite_memory.parallelStartAddress++;
            address = MEMORY_MARIE_START + satellite_memory.parallelStartAddress * MEMORY_BYTES_PER_SECTOR;
            abacus_flash_sector_erase(satellite_memory.selectedMainMemory, address);
        }
        break;
    case 2:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            memory_init(0);
            //Memory is ready
        }
        break;
    default:
        break;
    }
}


/*
 * This is a parallel process to bulk erase all the memory
 */
void subOperationBulkErase()
{
    switch(satelliteStatus_.memory->subStatusParallel)
    {
    case 0:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            abacus_flash_bulk_erase(satellite_memory.selectedMainMemory);
            satelliteStatus_.memory->subStatusParallel = 1;
        }
        break;
    case 1: //Check if finished:
        if(abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 0)
        {
            satelliteStatus_.memory->noSleep = 0;
            satelliteStatus_.memory->statusParallel = 0x00;
            satelliteStatus_.memory->subStatusParallel = 0x00;
            memory_init(0);
            //Memory is ready
        }
        break;
    default:
        break;
    }
}





/* OLD FIACCO COMMANDS

void checkMemoryOp(void)
{
    if (satelliteStatus_ .memOpReq == 0U)
    {
        return;
    }

    uint8_t are_[5U + RADIOHEADERSIZE + PAYLOADCHECKSUMSIZE]; //Ignacio
    are_[RADIOHEADERSIZE] = 'A';
    are_[1U + RADIOHEADERSIZE] = 'R';
    are_[2U + RADIOHEADERSIZE] = 'E';

    uint8_t arc_[5U + RADIOHEADERSIZE + PAYLOADCHECKSUMSIZE]; //Ignacio
    arc_[RADIOHEADERSIZE] = 'A';
    arc_[1U + RADIOHEADERSIZE] = 'R';
    arc_[2U + RADIOHEADERSIZE] = 'C';

    //Send packet only once every x ms:
    uint32_t timeNow = abacus_millis();
    //not needed her, it is checked in communication.c
    //if (timeNow
    //        < (satelliteStatus_.radioLastTimeTx
    //                + satelliteConfiguration_.radioMinimumInterval))
    //{
    //    return;
    //}

    if (abacus_flash_isWorkInProgress(satellite_memory.selectedMainMemory) == 1)
    {
        // I'd like to log this event, at least when groundStationDebug is on
        return;
    }

    switch (satelliteStatus_.memOpReq)
    {
    case 0: //unreachable code or dead code
        break;
    case 1: //Read oldest N packet of TELEMETRY
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress =
                    satellite_memory.sensorsEndFreeAddress + 1UL;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            if (counts < npacket && counts < satellite_memory.sensorsStoredPacket)
            {
                if (satellite_memory.readAddress
                        > MEMORY_SENSORS_END + 1U - SENSORS_SIZE_STREAM)
                {
                    // circular memory, if I reach the end Area but I still have packets to send
                    satellite_memory.readAddress = MEMORY_SENSORS_START;
                }
                // CALL FUNCTION DOWNLOAD PACKET
                memory_sensor_downloadPacket();
                satellite_memory.readAddress += SENSORS_SIZE_STREAM;
                counts++;
                satelliteStatus_.memSubOp = 1U;

            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 2U;
            }
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 2: //Read new N packet of TELEMETRY
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress =
                    satellite_memory.sensorsStartFreeAddress
                            - SENSORS_SIZE_STREAM;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            if (counts < npacket && counts < satellite_memory.sensorsStoredPacket) //se non sono arrivato alla fine
            {
                if (satellite_memory.readAddress < MEMORY_SENSORS_START)
                {
                    satellite_memory.readAddress = MEMORY_SENSORS_END
                            + 1- SENSORS_SIZE_STREAM;
                }
                // CALL FUNCTION DOWNLOAD PACKET
                memory_sensor_downloadPacket();
                satellite_memory.readAddress -= SENSORS_SIZE_STREAM;
                counts++;
                satelliteStatus_.memSubOp = 1U;
            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 2U;
            }
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 3: // Read TELEMETRY packet in specific address
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            satellite_memory.readAddress = valueAfterMemOpReq_[0U]
                    + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            counts = 0U; // in realtà non mi serve
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            // CALL FUNCTION DOWNLOAD PACKET
            memory_sensor_downloadPacket();
            // END OPERATION
            satelliteStatus_.memSubOp = 2U;
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 4: //Read oldest N packet of EVENT
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress = satellite_memory.eventEndFreeAddress
                    + 1UL;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:

            if (counts < npacket && counts < satellite_memory.eventStoredPacket) //se non sono arrivato alla fine
            {
                if (satellite_memory.readAddress
                        > MEMORY_EVENT_END + 1U - EVENT_SIZE_STREAM)
                {
                    satellite_memory.readAddress = MEMORY_EVENT_START;
                }

                bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
                ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
                // CALL FUNCTION DOWNLOAD PACKET
                memory_event_downloadPacket();
                satellite_memory.readAddress += EVENT_SIZE_STREAM;
                counts++;
                satelliteStatus_.memSubOp = 1U;
            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 2U;
            }
            // END OPERATION
            satelliteStatus_.memSubOp = 2U;
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 5: //Read new N packet of EVENT
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress =
                    satellite_memory.eventStartFreeAddress - EVENT_SIZE_STREAM;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            if (counts < npacket && counts < satellite_memory.eventStoredPacket) //se non sono arrivato alla fine
            {
                if (satellite_memory.readAddress < MEMORY_EVENT_START)
                {
                    satellite_memory.readAddress = MEMORY_EVENT_END
                            + 1U- EVENT_SIZE_STREAM;
                }

                bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
                ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
                // CALL FUNCTION DOWNLOAD PACKET
                memory_event_downloadPacket();
                satellite_memory.readAddress -= EVENT_SIZE_STREAM;
                counts++;
                satelliteStatus_.memSubOp = 1U;
            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 2U;
            }
            // END OPERATION
            satelliteStatus_.memSubOp = 2U;
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 6: // Read EVENT packet in specific address
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            satellite_memory.readAddress = valueAfterMemOpReq_[0U]
                    + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            counts = 0U; // in realtà non mi serve
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            // CALL FUNCTION DOWNLOAD PACKET
            memory_event_downloadPacket();
            // END OPERATION
            satelliteStatus_.memSubOp = 2U;
            break;
        case 2:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 7: //Read oldest N packet of MARIE data
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress = satellite_memory.marieEndFreeAddress
                    + 1UL;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            if (counts < npacket && counts < satellite_memory.marieStoredPage) //se non sono arrivato alla fine
            {
                if (satellite_memory.readAddress
                        > MEMORY_MARIE_END + 1U - MARIE_SIZE_STREAM_DATA)
                {
                    satellite_memory.readAddress = MEMORY_MARIE_START;
                }
                bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
                ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
                bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
                // CALL FUNCTION DOWNLOAD PACKET
                memory_marie_downloadHalfPage();
                satellite_memory.readAddress += MARIE_SIZE_STREAM_HALF_DATA;
                satelliteStatus_.memSubOp = 2U;

            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 3U;
            }
            break;
        case 2:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
            memory_marie_downloadHalfPage();
            satellite_memory.readAddress += MARIE_SIZE_STREAM_HALF_DATA;
            counts++;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 3:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 8: //Read new N page of MARIE data
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            npacket = valueAfterMemOpReq_[0U] + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            satellite_memory.readAddress =
                    satellite_memory.marieStartFreeAddress
                            - MARIE_SIZE_STREAM_DATA;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            if (counts < npacket && counts < satellite_memory.marieStoredPage) //se non sono arrivato alla fine
            {
                if (satellite_memory.readAddress < MEMORY_MARIE_START)
                {
                    satellite_memory.readAddress = MEMORY_MARIE_END
                            + 1U- MARIE_SIZE_STREAM_DATA;
                }
                bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
                ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
                bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
                // CALL FUNCTION DOWNLOAD PACKET
                memory_marie_downloadHalfPage();
                satellite_memory.readAddress += MARIE_SIZE_STREAM_HALF_DATA;
                satelliteStatus_.memSubOp = 2U;
            }
            else
            {
                // END OPERATION
                satelliteStatus_.memSubOp = 3U;
            }
            break;
        case 2:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
            memory_marie_downloadHalfPage();
            satellite_memory.readAddress -= MARIE_SIZE_STREAM_DATA; // torno indietro di una pagina, siccome il primo pacchetto che mando è allineato al primo byte di dati di marie
            counts++;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 3:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }

    case 9: //Read MARIE data in a specific address // In realtà potrei usare questa funzione per leggere qualsiasi pagina della memoria
    {
        switch (satelliteStatus_.memSubOp)
        {
        case 0:
            //Acknowledge read memory op Execution
            are_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            are_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(are_, 5U);
            // OPERATION
            satellite_memory.readAddress = valueAfterMemOpReq_[0U]
                    + valueAfterMemOpReq_[1U] * 256UL
                    + valueAfterMemOpReq_[2U] * 256UL * 256UL
                    + valueAfterMemOpReq_[3U] * 256UL * 256UL * 256UL;
            counts = 0U;
            satelliteStatus_.memSubOp = 1U;
            break;
        case 1:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
            // CALL FUNCTION DOWNLOAD PACKET
            memory_marie_downloadHalfPage();
            satellite_memory.readAddress += MARIE_SIZE_STREAM_HALF_DATA;
            satelliteStatus_.memSubOp = 2U;
            break;
        case 2:
            bufferMem[2U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            ulong2char(&counts, &bufferMem[11U + RADIOHEADERSIZE]);
            bufferMem[15 + RADIOHEADERSIZE] = satelliteStatus_.memSubOp;
            memory_marie_downloadHalfPage();
            satelliteStatus_.memSubOp = 3U;
            break;
        case 3:
            // Acknowledge read memory op Completed
            arc_[3U + RADIOHEADERSIZE] = satelliteStatus_.memOpReq;
            arc_[4U + RADIOHEADERSIZE] = satelliteStatus_.indexMemOpReq;
            radioWrite(arc_, 5U);
            satelliteStatus_.memOpReq = 0U;
            satelliteStatus_.memSubOp = 0U;
            break;
        }
        break;
    }
    default:
        satelliteStatus_.memOpReq = 0U;
        satelliteStatus_.memSubOp = 0U;
    }
    return;

}
*/
