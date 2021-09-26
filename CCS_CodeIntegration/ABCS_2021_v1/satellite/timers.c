/*
 * timers.c
 */

#include "timers.h"

/*
 * included in the new libraries
 struct SleepTimer sleepTimer_;
 uint32_t timeSinceBoot_;
 */

/*
 * It checks if the MCU can go to sleep or it is doing something else
 */
uint8_t timer_goToSleep()
{
    //Check if we can go to sleep
    if (satelliteStatus_.memoryNoSleep != 0)
        return 0;	//No sleep we are working hard!

    //Ok we can go to sleep, bug for how many milliseconds?
    int32_t nextTimeMillis, nextTimeMillisTemp;

    uint32_t nowTime = abacus_millis();

    //Check when next beacon has to be sent
    nextTimeMillis = satelliteStatus_.telemetryLastTimeBeacon
            + (uint32_t) satelliteConfiguration_.radioTelemetryBeaconInterval
                    * 1000UL - nowTime;

    if (nextTimeMillis < 1)
        return 0;	//No time to sleep!!

    //Check when telemetry must be read next time
    nextTimeMillisTemp = satelliteStatus_.telemetryLastTimeRead
            + (uint32_t) satelliteConfiguration_.telemetryReadInterval * 1000UL
            - nowTime;

    if (nextTimeMillisTemp < 1)
        return 0;	//No time to sleep!!

    if (nextTimeMillisTemp < nextTimeMillis)
        nextTimeMillis = nextTimeMillisTemp;

    //Check when telemetry must be saved to memory next time
    //Time to save them to memory?
    nextTimeMillisTemp = satelliteStatus_.telemetryLastTimeSaved
            + (uint32_t) satelliteConfiguration_.telemetrySaveInterval * 1000UL
            - nowTime;

    if (nextTimeMillisTemp < 1)
        return 0;	//No time to sleep!!

    if (nextTimeMillisTemp < nextTimeMillis)
        nextTimeMillis = nextTimeMillisTemp;

    //Failsafe
    //(and useful to register the 1 minute tick of flight timer in INFO MCU FlASH)
    if (nextTimeMillis > 60000)	//More than 1 minute? no way!
    {
        abacus_sleep_msec(60000);
        return 0;
    }
    else if (nextTimeMillis < 0)	//Negative? no way!
    {
        return 0;
    }
    else
        abacus_sleep_msec(nextTimeMillis);

    return 1;
}

void check_minute_counter()
{
    //Check if we have to register a new 0 in the minute counter
    uint32_t timeNow = abacus_millis();
    uint8_t countMinutesByte;
    uint8_t countMinutesPage;

    if ((satelliteStatus_.lastTimeMinuteCounter + 60000UL) > timeNow)
    {
        //No need to do anything
        return;
    }

    satelliteStatus_.lastTimeMinuteCounter = timeNow;

    //if byte is not 'full' (0x00) then put next bit to 0
    //else if there are bytes available in the page
    //           go to the next byte and write 1st bit to 0
    //     else count a new full page* and erase counter memory
    // *use the same logic of the minute counter
    //
    //if all full erase both memories and generate an event

    satelliteStatus_.totalMissionMinutes++;

    abacus_flash_mcu_read_data(satelliteStatus_.minuteCounterAddress,
                               &countMinutesByte, 1UL);
    if (countMinutesByte > 0)
    {
        // there are still bits available at that address
        countMinutesByte = countMinutesByte << 1;
        abacus_flash_mcu_write_data(satelliteStatus_.minuteCounterAddress,
                                    &countMinutesByte, 1UL);
    }
    else
    {
        if (satelliteStatus_.minuteCounterAddress
                < (MINUTE_COUNTER_ADD + 128UL - 1UL))  //it's not the last byte
        {
            // minute counter memory is not full
            satelliteStatus_.minuteCounterAddress += 1UL;
            countMinutesByte = 0xFE; // 1111 1110
            abacus_flash_mcu_write_data(satelliteStatus_.minuteCounterAddress,
                                        &countMinutesByte, 1UL);
        }
        else
        {
            // minute counter memory is full
            abacus_flash_mcu_read_data(satelliteStatus_.minutePagesAddress,
                                       &countMinutesPage, 1UL);
            if (countMinutesPage > 0)
            {
                // there are still bits available at that address
                countMinutesPage = countMinutesPage << 1;
                abacus_flash_mcu_write_data(satelliteStatus_.minutePagesAddress,
                                            &countMinutesPage, 1UL);
            }
            else
            {
                if (satelliteStatus_.minutePagesAddress
                        < (MINUTE_PAGES_ADD + 128UL - 1UL)) //it's not the last byte
                {
                    satelliteStatus_.minutePagesAddress += 1UL;
                    countMinutesPage = 0xFE; // 1111 1110
                    abacus_flash_mcu_write_data(
                            satelliteStatus_.minutePagesAddress,
                            &countMinutesPage, 1UL);
                    //wait
                    abacus_flash_mcu_wait_while_busy();

                    abacus_infoflash_mcu_erase(MINUTE_COUNTER_ADD);
                } else {
                    // log an event MINUTE MEMORY FULL
                    abacus_infoflash_mcu_erase(MINUTE_PAGES_ADD);
                    //wait
                    abacus_flash_mcu_wait_while_busy();

                    abacus_infoflash_mcu_erase(MINUTE_COUNTER_ADD);
                }
            }
        }
    }

}

uint32_t getUnixTimeNow()
{
    uint32_t timeNow = abacus_millis();
    if (timeNow > satelliteStatus_.lastTimeRTC + 60000
            || satelliteStatus_.lastTimeRTC == 0)
    {
        //Time to ask time to RTC and sync everything:
        satelliteStatus_.lastTimeRTC = timeNow;
        satelliteStatus_.lastTimeRTCunixTime = abacus_RTC_getUnixTime();
        //abacus_uart_print(satelliteConfiguration_.debugUart, "RTC sync\r\n");
    }

    return satelliteStatus_.lastTimeRTCunixTime
            + (timeNow - satelliteStatus_.lastTimeRTC) / 1000UL;
}


