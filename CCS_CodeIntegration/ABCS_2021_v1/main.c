#include <msp430.h>
#include <stdio.h>
#include "abacus.h"
#include "bootloader/bootloader.h"
#include "satellite/satsystem_init.h"
#include "SelfDiag/selfDiag.h"
#include "SatelliteAcceleration/Acceleration.h"
#pragma CODE_SECTION(main,".text_bootloader")

/*
 * TO DO
 * 1) define status register
 * 2) define FLASH memory usage and map (experiments, telemetry, hamradio...)
 * 3) define opmodes and change criteria (baud rate, power...)
 * 4) define experiments_set and experiment structures
 * 5) organize memory scrubs and spare boot codes
 * 6) define controlled reboots and actions in case of errors
 * 7) define events (refer to STECCO code)
 *
 * */

/*
 * Main routine
 * After initialization the satellite checks if there are tasks to do 
 * and eventually goes to sleep.
 * If there is an interrupt that awakes the satellite the loop is executed
 * checking what has to be done before going back to sleep
 */
int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

//    //Check memory status
    int8_t bootResult = bootloaderStart();


    uint8_t scrubMemory = scrubbingRoutine();
//    //Hardware initialization
//    //with diagnostics
//    //WDT enabled here
    satsystem_init();
//
    AB_LED_OFF;
//
//    //Check flight status and boot into preprogrammed sequence
    bootFlightStatus();
//
    int8_t systemDiagnostic = selfDiagInit();
//    //Enter main loop

    uint8_t isSatelliteInOrbit = isSatelliteInSpace();

    while (1)
    {
//        //Kick the WDT again
        WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Data from MARIE? Store them in FLASH
        //*** really need to do it here?
        //*** it could be done without wakeup probably
        //checkMarie();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Check commands on debug UART or on Radio UART
        //checkRadio();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Check commands on debug UART or on Radio UART
        //checkDebugCommand();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Check satellite status and update if necessary
        //checkStatus();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Time to do a new step of an experiment?
        //checkExperiment();

        //Kick the WDT again
       // WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Digipeater action?
        //checkRadioHam();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Time to read telemetry?
        //checkTelemetry();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //Time to send beacon?
        //checkBeacon();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //checkMemoryOp();

        //Kick the WDT again
        //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

        //check_minute_counter();

        //AB_LED_OFF;
//
        //timer_goToSleep();
//
        if (satelliteConfiguration_.debugIsOn == 1)
            AB_LED_ON;
    }
//
//    //It will never arrive here fortunately
//    //return 0;
}

