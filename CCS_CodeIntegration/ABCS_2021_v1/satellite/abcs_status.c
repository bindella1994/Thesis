/*
 * abcs.status.c
 *
 *  Created on: 14 gen 2021
 *      Author: Augusto Nascetti
 */

#include "abcs_status.h"

uint8_t cmdOpMode;
uint8_t opModeChange;

void bootFlightStatus()
{
    // first return the boot status to the debug port
    // TODO with bootloader code version
    // TODO boot sequence
    // total minutes
    // flash memory occupation
    // errors
    //
    // configure the satellite according to the mission phase
    // (check if experiments are completed)
    // Define mission phases
    // 1) first boot in orbit
    // 2) first experiment set (add runtime exp data to beacon)
    // 3) first digipeater on & beacon config with stored exp data
    // 4) second set of experiments
    // 5) digipeater on
    report_status();
}

void checkStatus()
{
    //Check if we have to read the telemetry status:
    uint32_t timeNow = abacus_millis();

    switch (satelliteConfiguration_.status)
    {
    case NORMALMODE:
        //(here experimentRunning=0)
        //consider events that lead to a configuration change
        switch (satelliteStatus_.event)
        {

        case ENTERINGSTATE:
            if (opModeChange == REQUESTED)
            {
                satsystem_loadDefaultConfiguration(&satelliteConfiguration_);
                opModeChange = EXECUTED;
            }
            break;

        case LOWBATTERY:
            satelliteConfiguration_.status = SAFEMODE;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        case HIGHTEMP:
            break;

        case LOWTEMP:
            break;

        case EXPERIMENTSTARTED:
            satelliteConfiguration_.status = EXPERIMENTMODE;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        case CMDCHGCONFIGURATION:
            //it is set in the command execution?
            satelliteConfiguration_.status = cmdOpMode;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;
            /*
             case ERROREPS:
             break;
             case ERRORI2C00:
             break;
             case ERRORI2C01:
             break;
             case ERRORFLASHMCU:
             break;
             case ERRORFLASHFPGA:
             break;
             case ERRORSENSOR:
             break;
             case ERRORRADIO:
             break;
             case ERRORLABONCHIP:
             break;
             */

        default:
            break;
        }
        break;

    case SAFEMODE:
        // here experiment running=0 or 2 (paused)
        //consider events that lead to a configuration change
        switch (satelliteStatus_.event)
        {
        case ENTERINGSTATE:
            if (opModeChange == REQUESTED)
            {
                satsystem_loadDefaultSafeConfiguration(
                        &satelliteConfiguration_);
                opModeChange = EXECUTED;
            }
            break;

        case GOODBATTERY:
            if (satelliteStatus_.experimentRunning == 0)
                satelliteConfiguration_.status = NORMALMODE;
            else if (satelliteStatus_.experimentRunning == 2)
                satelliteConfiguration_.status = EXPERIMENTMODE;

            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        case HIGHTEMP:
            break;

        case LOWTEMP:
            break;

        case EXPERIMENTSTARTED:
            break;

        case CMDCHGCONFIGURATION:
            //it is set in the command execution?
            satelliteConfiguration_.status = cmdOpMode;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        default:
            break;
        }
        break;

    case EXPERIMENTMODE:
        // here experiment running=1
        //consider events that lead to a configuration change
        switch (satelliteStatus_.event)
        {
        case ENTERINGSTATE:
            if (opModeChange == REQUESTED)
            {
                satsystem_loadDefaultExpConfiguration(&satelliteConfiguration_);
                opModeChange = EXECUTED;
            }
            break;

        case LOWBATTERY:
            satelliteConfiguration_.status = SAFEMODE;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        case EXPERIMENTSTARTED:
            break;

        case HIGHTEMP:
            break;

        case LOWTEMP:
            break;

        case CMDCHGCONFIGURATION:
            //it is set in the command execution?
            satelliteConfiguration_.status = cmdOpMode;
            satelliteStatus_.event = ENTERINGSTATE;
            opModeChange = REQUESTED;
            break;

        default:
            break;
        }
        break;

    default:
        break;

    }

}

void report_status()
{
    if (satelliteConfiguration_.debugIsOn == 0)
        return;

    abacus_uart_print(DEBUGUART, "\r\nMagic word: ");
    abacus_uart_print_uint8_hex(DEBUGUART, persistent_RAM[0]);

    abacus_uart_print(DEBUGUART, "\r\nTime at boot: ");
    abacus_uart_print_ulong(DEBUGUART, abacus_millis());

    abacus_uart_print(DEBUGUART, "\r\nTotal Mission Minutes: ");
    abacus_uart_print_ulong(DEBUGUART, satelliteStatus_.totalMissionMinutes);

    abacus_uart_print(DEBUGUART, "\r\nWDT interval: ");
    abacus_uart_print_uint8_hex(DEBUGUART,
                                satelliteConfiguration_.watchDogInterval);

    abacus_uart_print(DEBUGUART, "\r\nNumber of reboots: ");
    abacus_uart_print_uint(DEBUGUART, satelliteConfiguration_.numberReboots);

    abacus_uart_print(DEBUGUART, "\r\nLast Time Rx Ground: ");
    abacus_uart_print_ulong(DEBUGUART, satelliteStatus_.lastTimeRxGround);

    abacus_uart_print(DEBUGUART, "\r\nStatus: ");
    abacus_uart_print_uint8_hex(DEBUGUART, satelliteConfiguration_.status);

    abacus_uart_print(DEBUGUART, "\r\nBus and sensors configuration: ");
    abacus_uart_print_uint8_hex(DEBUGUART,
                                satelliteConfiguration_.busAndSensConfig);

    abacus_uart_print(DEBUGUART, "\r\nDo not init memory status: ");
    abacus_uart_print_uint8_hex(DEBUGUART,
                                satelliteConfiguration_.doNotInitMemory);

    abacus_uart_print(DEBUGUART, "\r\nEvents Start Free Address: \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.eventStartFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nEvents End Free Address:   \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.eventEndFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nEvents stored pages:       \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.eventStoredPacket);

    abacus_uart_print(DEBUGUART, "\r\nSensors Start Free Address:\t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.sensorsStartFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nSensors End Free Address:  \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.sensorsEndFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nSensors stored pages:      \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.sensorsStoredPage);

    abacus_uart_print(DEBUGUART, "\r\nMARIE Start Free Address:  \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.marieStartFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nMARIE End Free Address:    \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.marieEndFreeAddress);

    abacus_uart_print(DEBUGUART, "\r\nMARIE stored pages:        \t");
    abacus_uart_print_ulong(
    DEBUGUART,
                            satellite_memory.marieStoredPage);


}
