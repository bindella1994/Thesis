/*
 * configuration.c
 */

#include "configuration.h"

#pragma PERSISTENT(persistent_RAM)
uint8_t persistent_RAM[PERSISTENT_RAM_LENGTH] = { 0x00 };

/*
 * It loads the configuration from the persistent memory
 */
int8_t satsystem_loadConfiguration(struct SatelliteConfiguration *configuration,
                                   uint8_t *bufferOrigin)
{

    if (bufferOrigin[0] != PERSRAMMAGICWORD)	//Magic word
    {
        //Ok, at this point nothing was previously saved to internal flash.
        //So we load default configuration;
        satsystem_loadDefaultConfiguration(configuration);

        //Save new configuration!
        satsystem_saveConfiguration(configuration, bufferOrigin);

        return 2;
    }

    //numberReboots
    char2uint(&bufferOrigin[1], &configuration->numberReboots);
    //Last time on
    char2ulong(&bufferOrigin[3], &configuration->lastTimeOn);

    //Calculate the CRC16 of the flash where the configuration exists:
    uint16_t crc, newCrc;
    calculate_crc16_8bit(&bufferOrigin[7],
    PERSISTENT_RAM_LENGTH - 2 - 7,
                         &crc, 1);
    char2uint(&bufferOrigin[PERSISTENT_RAM_LENGTH - 2], &newCrc);

    if (crc != newCrc)
    {
        //Flash configuration is corrupted

        //Generate error event
        //memory_logEvent_noPayload(getUnixTimeNow(),EVENT_FLASHCONFCRCERROR);

        //Load default configuration
        satsystem_loadDefaultConfiguration(configuration);
        //Dont reset last time on or number of resets:
        //numberReboots
        //char2uint(&bufferOrigin[1], &configuration->numberReboots);
        //Last time on
        //char2ulong(&bufferOrigin[3], &configuration->lastTimeOn);
        //Save new configuration!
        satsystem_saveConfiguration(configuration, bufferOrigin);
        return 1;
    }

    //CRC is correct, load values:
    configuration->doNotInitMemory = bufferOrigin[7];
    configuration->busAndSensConfig = bufferOrigin[8];

    configuration->status = bufferOrigin[9];
    configuration->weAreOnOrbit = bufferOrigin[10];

    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        configuration->radioAutomaticBeaconData[i] = bufferOrigin[11 + i];
    }

    configuration->radioAutomaticBeaconInterval = bufferOrigin[61];
    configuration->radioTelemetryBeaconInterval = bufferOrigin[62];
    char2uint(&bufferOrigin[63], &configuration->radioMinimumInterval);
    configuration->radioDataMaxSize = bufferOrigin[65];
    configuration->radioOutputPower = bufferOrigin[66];
    configuration->radioPowerAutomaticControl = bufferOrigin[67];

    char2uint(&bufferOrigin[68], &configuration->minVoltageSafeMode);

    configuration->fpgaOnBoot = bufferOrigin[70];

    char2uint(&bufferOrigin[71], &configuration->shadowVoltageLimit);

    configuration->telemetryReadInterval = bufferOrigin[73];
    configuration->telemetrySaveInterval = bufferOrigin[74];

    configuration->radioRepetitions = bufferOrigin[75];

//    configuration->debugUart = bufferOrigin[76];
//    configuration->radioUart = bufferOrigin[77];
//    configuration->marieUart = bufferOrigin[78];
//    configuration->pl2Uart = bufferOrigin[79];

    configuration->debugIsOn = bufferOrigin[76];
    configuration->watchDogInterval = bufferOrigin[77];

    //Failsafe for the watch dog:
    if (!(configuration->watchDogInterval == WDT_MRST_LONG
            || configuration->watchDogInterval == WDT_MRST_EXTRALONG
            || configuration->watchDogInterval == WDT_MRST_XXL))
    {
        configuration->watchDogInterval = (uint8_t) WDT_MRST_EXTRALONG;	//Avoid extraneous WDT configurations
    }

    configuration->radioAmateurOn = bufferOrigin[78];
    char2uint(&bufferOrigin[79], &configuration->radioAmateurMaxTx);
    char2uint(&bufferOrigin[81], &configuration->radioAmateurMinVoltage);

    for (i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
    {
        configuration->experimentStatus[i] = bufferOrigin[83 + i];
    }

    configuration->memory->selectedMainMemory = bufferOrigin[89];
    char2ulong(&bufferOrigin[90], &configuration->memory->confStartFreeAddress);
    char2ulong(&bufferOrigin[94],
               &configuration->memory->sensorsStartFreeAddress);
    char2ulong(&bufferOrigin[98],
               &configuration->memory->sensorsEndFreeAddress);
    char2ulong(&bufferOrigin[102], &configuration->memory->sensorsStoredPage);
    char2ulong(&bufferOrigin[106], &configuration->memory->sensorsStoredPacket);
    char2ulong(&bufferOrigin[110],
               &configuration->memory->marieStartFreeAddress);
    char2ulong(&bufferOrigin[114], &configuration->memory->marieEndFreeAddress);
    char2ulong(&bufferOrigin[118], &configuration->memory->marieStoredPage);
    char2ulong(&bufferOrigin[122],
               &configuration->memory->eventStartFreeAddress);
    char2ulong(&bufferOrigin[126], &configuration->memory->eventEndFreeAddress);
    char2ulong(&bufferOrigin[130], &configuration->memory->eventStoredPacket);
    char2ulong(&bufferOrigin[134], &configuration->memory->readAddress);
    char2ulong(&bufferOrigin[138],
               &configuration->memory->lastMarieBeaconAddress);
    configuration->memory->errors = bufferOrigin[142];

    //Used 143 bytes (0-142)
    // total now 160 bytes
    //bytes 158-159 are CRC
    return 0;
}

/*
 * Loads default NORMALMODE configuration
 */
int8_t satsystem_loadDefaultConfiguration(
        struct SatelliteConfiguration *configuration)
{

    strcpy((char*) configuration->radioAutomaticBeaconData,
           "AstroBioCubeSat School Of Aerospace Engineering");
    uint8_t i;
    for (i = strlen((char*) configuration->radioAutomaticBeaconData); i < 50;
            i++)
        configuration->radioAutomaticBeaconData[i] = 0;

    configuration->radioAutomaticBeaconInterval = 13;	//Every 32.5 seconds
    //configuration->radioAutomaticBeaconInterval = 0;	//Never
    configuration->radioTelemetryBeaconInterval = 10;	//Every 10 seconds
    configuration->radioMinimumInterval = 100;			//100ms
    configuration->radioDataMaxSize = 250;
    configuration->radioOutputPower = 137;	//This is between 2 and 2.5 watts
    configuration->radioPowerAutomaticControl = 1;
    configuration->minVoltageSafeMode = 6400;	//6.4V?
    configuration->fpgaOnBoot = 0;
    configuration->radioRepetitions = 1;
    configuration->telemetryReadInterval = 10;
    configuration->telemetrySaveInterval = 60;
    configuration->shadowVoltageLimit = 8000;	//mV
    configuration->debugIsOn = 1;
    configuration->watchDogInterval = (uint8_t) WDT_MRST_LONG;//Set to 500ms by default
    configuration->doNotInitMemory = 0;	//By default initializate it
    configuration->radioAmateurOn = 1;
    configuration->radioAmateurMaxTx = 200;
    configuration->radioAmateurMinVoltage = 7200;	//7.2V?
    return 1;
}

/*
 * Loads default SAFEMODE configuration
 */
int8_t satsystem_loadDefaultSafeConfiguration(
        struct SatelliteConfiguration *configuration)
{

    strcpy((char*) configuration->radioAutomaticBeaconData,
           "AstroBioCubeSat - Safe Mode Operation");
    uint8_t i;
    for (i = strlen((char*) configuration->radioAutomaticBeaconData); i < 50;
            i++)
        configuration->radioAutomaticBeaconData[i] = 0;

    configuration->radioAutomaticBeaconInterval = 40;   //Every 100 seconds
    //configuration->radioAutomaticBeaconInterval = 0;  //Never
    configuration->radioTelemetryBeaconInterval = 60;   //Every 60 seconds
    configuration->radioMinimumInterval = 200;          //100ms
    configuration->radioDataMaxSize = 250;
    configuration->radioOutputPower = 137;  //This is between 2 and 2.5 watts
    configuration->radioPowerAutomaticControl = 1;
    configuration->minVoltageSafeMode = 6400;   //6.4V?
    configuration->fpgaOnBoot = 0;
    configuration->radioRepetitions = 1;
    configuration->telemetryReadInterval = 10;
    configuration->telemetrySaveInterval = 60;
    configuration->shadowVoltageLimit = 8000;   //mV
    configuration->debugIsOn = 1;
    configuration->watchDogInterval = (uint8_t) WDT_MRST_LONG; //Set to 500ms by default
    configuration->doNotInitMemory = 0; //By default initializate it
    configuration->radioAmateurOn = 0;
    configuration->radioAmateurMaxTx = 200;
    configuration->radioAmateurMinVoltage = 7200;   //7.2V?
    return 1;
}

/*
 * Loads default EXPERIMENTMODE configuration
 */
int8_t satsystem_loadDefaultExpConfiguration(
        struct SatelliteConfiguration *configuration)
{

    strcpy((char*) configuration->radioAutomaticBeaconData,
           "AstroBioCubeSat - Lab-on-chip Experiment Running");
    uint8_t i;
    for (i = strlen((char*) configuration->radioAutomaticBeaconData); i < 50;
            i++)
        configuration->radioAutomaticBeaconData[i] = 0;

    configuration->radioAutomaticBeaconInterval = 24;   //Every 60 seconds
    //configuration->radioAutomaticBeaconInterval = 0;  //Never
    configuration->radioTelemetryBeaconInterval = 30;   //Every 30 seconds
    configuration->radioMinimumInterval = 200;          //100ms
    configuration->radioDataMaxSize = 250;
    configuration->radioOutputPower = 137;  //This is between 2 and 2.5 watts
    configuration->radioPowerAutomaticControl = 1;
    configuration->minVoltageSafeMode = 6400;   //6.4V?
    configuration->fpgaOnBoot = 0;
    configuration->radioRepetitions = 1;
    configuration->telemetryReadInterval = 10;
    configuration->telemetrySaveInterval = 60;
    configuration->shadowVoltageLimit = 8000;   //mV
    configuration->debugIsOn = 1;
    configuration->watchDogInterval = (uint8_t) WDT_MRST_LONG; //Set to 500ms by default
    configuration->doNotInitMemory = 0; //By default initializate it
    configuration->radioAmateurOn = 0;
    configuration->radioAmateurMaxTx = 200;
    configuration->radioAmateurMinVoltage = 7200;   //7.2V?
    return 1;
}

/*
 * Saves the internal configuration of the satellite in the internal flash of
 * the MCU
 */
int8_t satsystem_saveConfiguration(struct SatelliteConfiguration *configuration,
                                   uint8_t *bufferDestination)
{
    uint2char(&satelliteConfiguration_.numberReboots, &bufferDestination[1]);
    configuration_saveLastTimeOn(satelliteConfiguration_.lastTimeOn);

    bufferDestination[7] = configuration->status;
    bufferDestination[8] = configuration->busAndSensConfig;

    bufferDestination[9] = configuration->doNotInitMemory;
    bufferDestination[10] = configuration->weAreOnOrbit;
    uint8_t i;
    for (i = 0; i < 50; i++)
        bufferDestination[11 + i] = configuration->radioAutomaticBeaconData[i];

    bufferDestination[61] = configuration->radioAutomaticBeaconInterval;
    bufferDestination[62] = configuration->radioTelemetryBeaconInterval;
    uint2char(&configuration->radioMinimumInterval, &bufferDestination[63]);
    bufferDestination[65] = configuration->radioDataMaxSize;
    bufferDestination[66] = configuration->radioOutputPower;
    bufferDestination[67] = configuration->radioPowerAutomaticControl;

    uint2char(&configuration->minVoltageSafeMode, &bufferDestination[68]);
    bufferDestination[70] = configuration->fpgaOnBoot;

    uint2char(&configuration->shadowVoltageLimit, &bufferDestination[71]);

    bufferDestination[73] = configuration->telemetryReadInterval;
    bufferDestination[74] = configuration->telemetrySaveInterval;

    bufferDestination[75] = configuration->radioRepetitions;

//    bufferDestination[76] = configuration->debugUart = DEBUGUART;
//    bufferDestination[77] = configuration->radioUart = RADIOUART;
//    bufferDestination[78] = configuration->marieUart = MARIEUART;
//    bufferDestination[79] = configuration->pl2Uart = PL2UART;

    bufferDestination[76] = configuration->debugIsOn;
    if (!(configuration->watchDogInterval == WDT_MRST_LONG
            || configuration->watchDogInterval == WDT_MRST_EXTRALONG
            || configuration->watchDogInterval == WDT_MRST_XXL))
    {
        configuration->watchDogInterval = (uint8_t) WDT_MRST_EXTRALONG;	//Avoid extraneous WDT configurations
    }
    bufferDestination[77] = configuration->watchDogInterval;

    bufferDestination[78] = configuration->radioAmateurOn;
    uint2char(&configuration->radioAmateurMaxTx, &bufferDestination[79]);
    uint2char(&configuration->radioAmateurMinVoltage, &bufferDestination[81]);

    for (i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
    {
        bufferDestination[83 + i] = configuration->experimentStatus[i];
    }

    configuration->memory->selectedMainMemory = bufferDestination[89];
    ulong2char(&configuration->memory->confStartFreeAddress, &bufferDestination[90]);
    ulong2char(&configuration->memory->sensorsStartFreeAddress,
               &bufferDestination[94]);
    ulong2char(&configuration->memory->sensorsEndFreeAddress,
               &bufferDestination[98]);
    ulong2char(&configuration->memory->sensorsStoredPage, &bufferDestination[102]);
    ulong2char(&configuration->memory->sensorsStoredPacket, &bufferDestination[106]);
    ulong2char(&configuration->memory->marieStartFreeAddress,
               &bufferDestination[110]);
    ulong2char(&configuration->memory->marieEndFreeAddress, &bufferDestination[114]);
    ulong2char(&configuration->memory->marieStoredPage, &bufferDestination[118]);
    ulong2char(&configuration->memory->eventStartFreeAddress,
               &bufferDestination[122]);
    ulong2char(&configuration->memory->eventEndFreeAddress, &bufferDestination[126]);
    ulong2char(&configuration->memory->eventStoredPacket, &bufferDestination[130]);
    ulong2char(&configuration->memory->readAddress, &bufferDestination[134]);
    ulong2char(&configuration->memory->lastMarieBeaconAddress,
               &bufferDestination[138]);
    configuration->memory->errors = bufferDestination[142];

    //Used 143 bytes (0-142)

    for (i = 143; i < PERSISTENT_RAM_LENGTH; i++)
        bufferDestination[i] = 0x00;

    //Update the CRC:
    uint16_t crc;
    calculate_crc16_8bit(&bufferDestination[7],
    PERSISTENT_RAM_LENGTH - 2 - 7,
                         &crc, 1);
    //Save CRC in the last 2 bytes:
    uint2char(&crc, &bufferDestination[PERSISTENT_RAM_LENGTH - 2]);

    //Set magic world, we have a new configuration saved
    bufferDestination[0] = PERSRAMMAGICWORD;

    return 0;
}

/*
 * Increments in 1 the number of resets that abacus performed
 */
int8_t configuration_saveNewReset()
{
    //Increase configuration
    satelliteConfiguration_.numberReboots++;
    //Save to internal flash
    uint2char(&satelliteConfiguration_.numberReboots, &persistent_RAM[1]);

    return 0;
}

/*
 * Saves the last time abacus was on.
 */
int8_t configuration_saveLastTimeOn(uint32_t time)
{
    //Save to internal flash
    ulong2char(&time, &persistent_RAM[3]);

    return 0;
}

/*
 * Loads the semi-permanent data. This is not saved during powercycles only resets
 */
int8_t loadPartialPermanentSettings()
{

    if (persistent_RAM[0] == PERSRAMMAGICWORD)	//Magic word
    {
        //Load from internal flash the number of reboots:
        char2uint(&persistent_RAM[1], &satelliteConfiguration_.numberReboots);
        //Load the status of the last time memory was initiated
        satelliteConfiguration_.doNotInitMemory = persistent_RAM[7];
        satelliteConfiguration_.busAndSensConfig = persistent_RAM[8];

        return 0;
    }

    //Load with default values
    satelliteConfiguration_.numberReboots = 0;
    satelliteConfiguration_.doNotInitMemory = 0;
    satelliteConfiguration_.busAndSensConfig = 0xFF;

    return 0;
}

/*
 * Loads the semi-permanent data. This is not saved during powercycles only resets
 */
int8_t savePartialPermanentSettings()
{
    persistent_RAM[0] = PERSRAMMAGICWORD;
    //Load from internal flash the number of reboots:
    uint2char(&satelliteConfiguration_.numberReboots, &persistent_RAM[1]);
    //Load the status of the last time memory was initiated
    persistent_RAM[7] = satelliteConfiguration_.doNotInitMemory;
    persistent_RAM[8] = satelliteConfiguration_.busAndSensConfig;

    return 0;
}

