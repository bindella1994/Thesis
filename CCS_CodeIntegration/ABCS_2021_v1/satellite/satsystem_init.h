/*
 * satsystem_init.h
 *
 */

#ifndef SATSYSTEM_INIT_H_
#define SATSYSTEM_INIT_H_


#include <memory/memory.h>
#include "abacus.h"
#include "communications/abacus_uart.h"
#include "GPIO/abacus_gpio.h"
#include "memory/abacus_flash_mcu.h"


#include "../communications/radio_lithium2.h"
#include "configuration.h"
#include "telemetry.h"
#include "timers.h"

#include "debug.h"
#include "../payload/marie.h"

#define MINUTE_PAGES_ADD    0x1880UL // infoC
#define MINUTE_COUNTER_ADD  0x1800UL // infoD
#define MINUTESPER8PAGES    8192
#define MINUTESPERPAGE      1024
#define MINUTESPERBYTE      8


#define PERSISTENT_RAM_LENGTH 160
#define PERSRAMMAGICWORD  0xAD

#define FLASHMAGICWORD          0xDA
#define FLASHMAGICWORDADDR      0x1900UL // infoB

extern struct SatelliteConfiguration satelliteConfiguration_;
extern struct SatelliteStatus satelliteStatus_;
extern uint8_t persistent_RAM[PERSISTENT_RAM_LENGTH];

int8_t satsystem_init();
int8_t init_registers();
void init_mission_timer();

#endif /* SATSYSTEM_INIT_H_ */
