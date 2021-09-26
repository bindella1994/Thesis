/*
 * satsystem_init.c
 *
 */

#include "satsystem_init.h"

struct SatelliteConfiguration satelliteConfiguration_;
struct SatelliteStatus satelliteStatus_;

/*
 * It initiates the CPU of the satellite
 */
int8_t satsystem_init()
{

    //Init status register
    init_registers();

    //Load default configuration just in case
    satsystem_loadDefaultConfiguration(&satelliteConfiguration_);

    //Enable watchdog for PUC
    WDTCTL = WDTPW | (uint16_t) satelliteConfiguration_.watchDogInterval;

    /*
     * Scrubbing of the persistent_RAM (check here if contains valid data)
     */

    //It loads if there were problems with memory and loads the number of PUC resets
    loadPartialPermanentSettings();

    //Update that a reset was produced in the internal flash of MCU
    //This only saves without power cycles, only wdt PUC
    configuration_saveNewReset();

    //Start timer for counting elapsed time:
    //abacus_millis_init(); //INITIALIZED in abacus_init

    //Standard abacus init
    abacus_init(AB_ABACUSVERSION2014, AB_CLOCK8MHZ);

    AB_LED_ON;

    //Mission timer counts the total minutes of mission
    //saved in internal flash (1bit/minute)
    init_mission_timer();

    //Kick the WDT again
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

    //Set external H1 and H2 pinouts

    //Set all the PAYLOAD pins

    //First pulldown
    abacus_gpio_digitalWrite(PUMP_SWITCH_1, AB_LOW);
    abacus_gpio_digitalWrite(PUMP_SWITCH_2, AB_LOW);
    abacus_gpio_digitalWrite(WET_EN1, AB_LOW);
    abacus_gpio_digitalWrite(WET_EN2, AB_LOW);
    abacus_gpio_digitalWrite(RADFET_EN1, AB_LOW);
    abacus_gpio_digitalWrite(RADFET_EN2, AB_LOW);
    abacus_gpio_digitalWrite(MARIE_EN, AB_LOW);
    abacus_gpio_digitalWrite(PUMP_1_4, AB_LOW);
    abacus_gpio_digitalWrite(PUMP_2_5, AB_LOW);
    abacus_gpio_digitalWrite(PUMP_3_6, AB_LOW);
    abacus_gpio_digitalWrite(HEATER_EN, AB_LOW);
    abacus_gpio_digitalWrite(POWER_CYCLE, AB_LOW);
    //PL2_EN pin AB_H1_25 currently used for Radio Interrupt (form ground)
    //abacus_gpio_digitalWrite(PL2_EN, AB_LOW);

    //Set all to outputs
    abacus_gpio_digitalMode(PUMP_SWITCH_1, AB_OUTPUT);
    abacus_gpio_digitalMode(PUMP_SWITCH_2, AB_OUTPUT);
    abacus_gpio_digitalMode(WET_EN1, AB_OUTPUT);
    abacus_gpio_digitalMode(WET_EN2, AB_OUTPUT);
    abacus_gpio_digitalMode(RADFET_EN1, AB_OUTPUT);
    abacus_gpio_digitalMode(RADFET_EN2, AB_OUTPUT);
    abacus_gpio_digitalMode(MARIE_EN, AB_OUTPUT);
    abacus_gpio_digitalMode(PUMP_1_4, AB_OUTPUT);
    abacus_gpio_digitalMode(PUMP_2_5, AB_OUTPUT);
    abacus_gpio_digitalMode(PUMP_3_6, AB_OUTPUT);
    abacus_gpio_digitalMode(HEATER_EN, AB_OUTPUT);
    abacus_gpio_digitalMode(POWER_CYCLE, AB_OUTPUT);
    //PL2_EN pin AB_H1_25 currently used for Radio Interrupt (form ground)
    //abacus_gpio_digitalMode(PL2_EN, AB_OUTPUT);

    //Do not configure yet the interrupts, that is done
    //depending on the status of the satellite, it will be done
    //on the function bootFlightStatus

    //Original code safe init of external flash
    //Load configuration from MCU flash
    //Mount MCU flash memory
    if (satelliteConfiguration_.doNotInitMemory == 0)
    {
        //If it fails and wdt reboots, memory will not be initializated next time
        satelliteConfiguration_.doNotInitMemory = 1;
        savePartialPermanentSettings();
        memory_init(1);
        //If it fails and wdt reboots, memory will not be initializated next time
        satelliteConfiguration_.doNotInitMemory = 0;
        savePartialPermanentSettings();
        satelliteStatus_.ignoreMemory = 0;
    }
    else
    {
        //Toggle, next time it will definitely try again (forcing a PUC from earth):
        satelliteConfiguration_.doNotInitMemory = 0;
        savePartialPermanentSettings();
        satelliteStatus_.ignoreMemory = 1;
    }

    //Start internal RTC:
    abacus_RTC_internal_init(CALENDAR);

    //Sync internal RTC with external:
    //abacus_RTC_internal_sincRTC();
    //Set internal RTC to the mission time
    uint32_t missionSecs;
    missionSecs = satelliteStatus_.totalMissionMinutes * 60UL;
    abacus_RTC_internal_setUnixTime(missionSecs);

    //Set time of boot
    satelliteStatus_.timeAtBoot = missionSecs; // it was getUnixTimeNow();

    //Write event that reset was produced.
    //memory_logEvent_noPayload(satelliteStatus_.timeAtBoot, EVENT_BOOT);

    //Init EPS
    //eps_init(0x09);

    //Init radio
    radio_init(RADIOUART, satelliteConfiguration_.radioAutomaticBeaconInterval,
               satelliteConfiguration_.radioAutomaticBeaconData,
               satelliteConfiguration_.radioOutputPower);

    //Kick the WDT again
    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

    //Init debug port:
    abacus_uart_open_function(DEBUGUART,
    AB_B9600,
                              debugExternalInterrupt);
    abacus_uart_enableInterrupt(DEBUGUART);

    //Init payload COM port
    init_Marie();
    initMarieStatus();

    //Init Payload 2 COM port
    //init_PL2();

    //Switch on FPGA?
    if (satelliteConfiguration_.fpgaOnBoot == 1)
    {
        abacus_fpga_switchOn();
        abacus_sleep_msec(5);
        abacus_fpga_resetProgram();
        //Kick the WDT again
        WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;

    }

    uint16_t i;
    for (i = 0; i < 5; i++)	//5 times = 1 seconds
    {
        AB_LED_ON;
        abacus_sleep_msec(100);
        AB_LED_OFF;
        abacus_sleep_msec(100);
    }

    // Default opmode at start
    if (satelliteStatus_.errors == 0)
        satelliteConfiguration_.status = NORMALMODE;
    else
        satelliteConfiguration_.status = SAFEMODE;

    return satelliteStatus_.errors;
}

/*
 * Initiates the registers of the satellite
 */
int8_t init_registers()
{
    // init only those registers that will be cleared to zero anyway
    // (i.e. indepentent on the type of boot)
    satelliteStatus_.event = 0;
    satelliteStatus_.errors = 0;
    satelliteStatus_.interruptCause = NOINTERRUPT;

    satelliteStatus_.radioAckPacketIndex = 0;
    satelliteStatus_.radioPacketIndex = 0;

    satelliteStatus_.telemetryLastTimeBeacon = 0;
    satelliteStatus_.telemetryLastTimeTx = 0;
    satelliteStatus_.telemetryLastTimeSaved = 0;
    satelliteStatus_.telemetryLastTimeRead = 0;

    satelliteStatus_.lastTimeRxGround = 0;
    satelliteStatus_.lastTimeRadioReboot = 0;
    satelliteStatus_.lastTimeMinuteCounter = 0;
    bufferDebugPosition_ = 0;
    debugNewPacketArrived_ = 0;
    return 0;
}

void init_mission_timer()
{
    //count the minute counter bits in internal flash
    uint32_t minutes = 0;
    uint8_t found = 0;
    uint8_t countMinutesByte;
    uint8_t countMinutePages;
    uint32_t minutePagesAddr = MINUTE_PAGES_ADD;
    uint32_t minuteCounterAddr = MINUTE_COUNTER_ADD;

    // count how many 17hours blocks were counted
    while (found == 0)
    {
        abacus_flash_mcu_read_data(minutePagesAddr, &countMinutePages, 1UL);
        if (countMinutePages == 0x00)
        {
            //MINUTESPER8PAGES = 8192
            //if minutes are stored in INFOB
            //0x00 means 8 bits written which means 8 full pages of 128*8 bits
            //--> 8*128*8=8192
            minutes += MINUTESPER8PAGES;
            minutePagesAddr++;
            if (minutePagesAddr >= (MINUTE_PAGES_ADD + 128UL))
                found = 1; //we reached the 2 years limit (in case of use of infoB and infoC)
        }
        else
        {
            while ((countMinutePages & BIT0) == 0)
            {
                //MINUTESPERPAGE = 1024 = 17.0667 hours = 17hours and 4 minutes
                //(if minutes are stored in INFOB 128*8)
                minutes += MINUTESPERPAGE;
                countMinutePages >>= 1;
            }
            found = 1;
        }
    }
    satelliteStatus_.minutePagesAddress = minutePagesAddr;

    found = 0;
    // count how many extra minutes were counted
    while (found == 0)
    {
        abacus_flash_mcu_read_data(minuteCounterAddr, &countMinutesByte, 1UL);
        if (countMinutesByte == 0x00)
        {
            // MINUTESPERBYTE = 8 (one per bit)
            minutes += MINUTESPERBYTE;
            minuteCounterAddr++;
            if (minuteCounterAddr == (MINUTE_COUNTER_ADD + 128UL))
                found = 1; //the page is full (it will be erased at the next 1minute write)
        }
        else
        {
            while ((countMinutesByte & BIT0) == 0)
            {
                minutes++; //1 minute per bit
                countMinutesByte >>= 1;
            }
            found = 1;
        }
    }
    satelliteStatus_.minuteCounterAddress = minuteCounterAddr;

    //update the total mission time
    satelliteStatus_.totalMissionMinutes = minutes;
}

