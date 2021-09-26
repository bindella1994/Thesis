/*
 * configuration.h
 *
 *  Created on: 02/mag/2014
 *      Author: Aitor
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <memory/memory.h>
#include "stdint.h"
#include <msp430.h>
#include <strings.h>
#include "abacus.h"
#include "satsystem_init.h"
#include "..\payload\labonchip.h"

#define EVENT_BOOT                      0xAA
#define EVENT_RXCOMMAND_CONFIGURATION   0x01
#define EVENT_ERRORFLASHMCU             0x02
#define EVENT_ERRORFLASHFPGA            0x03
#define EVENT_ERRORI2C00                0x04
#define EVENT_ERRORI2C01                0x05
#define EVENT_ERRORTSENSMCU             0x06
#define EVENT_ERRORTSENSFPGA            0x07
#define EVENT_ERRORGYRO                 0x08
#define EVENT_ERRORMAG                  0x09
#define EVENT_ERRORACCL                 0x0A
#define EVENT_ERRORRADIO                0x0B
#define EVENT_ERRORFPGA                 0x0C

#define EVENT_FPGAFLASHPROGRAMMED       0x10
#define EVENT_FPGAREBOOT                0x11
#define EVENT_FPGASHUTDOWN              0x12
#define EVENT_FPGAON                    0x13

#define EVENT_ERROREPS                  0x20
#define EVENT_EPSSUN                    0x21
#define EVENT_EPSSHADOW                 0x22
#define EVENT_BATTERYLOW                0x23
#define EVENT_BATTERYERROR              0x24

#define EVENT_FLASHCONFCRCERROR         0x30

#define EVENT_RADIOTIMEOUT              0x40
#define EVENT_RADIONACK                 0x41
#define EVENT_RADIOCRCERROR             0x42

#define EVENT_CHANGESTATUS              0x50
#define EVENT_NORMALOPERATIONS          0x51

#define EVENT_MANUALREBOOT              0x60
#define EVENT_REBOOTCLEAR               0x61
#define EVENT_REBOOTNOLISTEN            0x62

#define EVENT_ALLSECTORERASEDEVENT      0x70
#define EVENT_ALLSECTORSERASEDMARIE     0x71
#define EVENT_ALLSECTORSERASEDSENSOR    0x72
#define EVENT_ERROR_ALLSECTORERASEEVENT 0x73
#define EVENT_ERROR_ALLSECTORERASEMARIE 0x74
#define EVENT_ERROR_ALLSECTORERASESENSOR    0x75

#define EVENT_ERROR_SECTORERASEEVENT    0x76
#define EVENT_ERROR_SECTORERASEMARIE    0x77
#define EVENT_ERROR_SECTORERASESENSOR   0x78
#define EVENT_SECTORERASEDMARIE         0x79
#define EVENT_SECTORERASEDSENSOR        0x7A


#define ERRORRADIO 			0x0001
#define ERRORI2C00 			0x0004
#define ERRORI2C01 			0x0008
#define ERRORFLASHMCU 		0x0010
#define ERRORFLASHFPGA 	    0x0020
#define ERRORTSENSMCU 		0x0040
#define ERRORTSENSFPGA 	    0x0080
#define ERRORGYRO           0X0100
#define ERRORMAG            0X0200
#define ERRORACCL           0X0400
#define ERRORFPGA           0X0800
#define ERRORGPIOEXP 		0x1000
#define ERROREPS 			0x2000

#define NOINTERRUPT			0
#define INTERRUPTACTUATOR 	1
#define INTERRUPTRADIOIN 	2
#define INTERRUPTRADIORESET	3
#define INTERRUPTDEBUG		4
#define INTERRUPTTIMER		5

#define RADIOUART		AB_UART00
#define MARIEUART		AB_UART01
#define PL2UART         AB_UART02
#define DEBUGUART		AB_UART03

// WDT for reset
#define WDT_MRST_LONG 		(WDTCNTCL + WDTIS1 + WDTIS0)  	// 500ms
#define WDT_MRST_EXTRALONG 	(WDTCNTCL + WDTIS1)    			// 8s
#define WDT_MRST_XXL 		(WDTCNTCL + WDTIS0)          	// 128s
#define WDT_MDLY_LONG       (WDTTMSEL + WDTCNTCL + WDTIS1 + WDTIS0) /* DELAY MODE (ISR) 500ms interval*/

// ABCS H1/H2 PINOUT
#define PUMP_SWITCH_1   AB_H1_34
#define PUMP_SWITCH_2   AB_H1_35
#define PUMP_1_4        AB_H1_36
#define PUMP_2_5        AB_H1_37
#define PUMP_3_6        AB_H1_38
#define WET_EN1         AB_H1_28
#define WET_EN2         AB_H1_30
#define HEATER_EN       AB_H1_8
#define POWER_CYCLE     AB_H1_10
#define RADIO_RESET     AB_H1_14
#define RADFET_EN1      AB_H1_13
#define RADFET_EN2      AB_H1_15
#define MARIE_EN        AB_H1_19
//#define PL2_EN          AB_H1_25
#define RADIOINTERRUPT  AB_H1_25

#define HEADERBUFFERDEBUG 4U
#define CRCBUFFERDEBUG 0U

struct SatelliteConfiguration
{

    uint16_t numberReboots;
    uint32_t lastTimeOn;
    uint8_t doNotInitMemory; //use half byte for MCU flash and half for FPG flash
    uint8_t busAndSensConfig; // BIT 0 I2C00
                              // BIT 1 I2C01
                              // BIT 2 TSENSMCU
                              // BIT 3 TSENSFPGA
                              // BIT 4 GYRO
                              // BIT 5 MAG
                              // BIT 6 ACC
                              // BIT 7 FPGA
    uint8_t status;
    uint8_t weAreOnOrbit;

    uint8_t radioAutomaticBeaconData[50];
    uint8_t radioAutomaticBeaconInterval;   //LSB is 2.5s
    uint8_t radioTelemetryBeaconInterval;   //in seconds
    uint16_t radioMinimumInterval;          //in ms
    uint8_t radioDataMaxSize;               //in bytes
    uint8_t radioOutputPower;               //From 0 to 255;

    uint8_t radioPowerAutomaticControl;
    uint16_t minVoltageSafeMode;

    uint8_t fpgaOnBoot;

    uint16_t shadowVoltageLimit;

    uint8_t telemetryReadInterval;  //Seconds
    uint8_t telemetrySaveInterval;  //Seconds

    uint8_t radioRepetitions;

//    uint8_t debugUart;
//    uint8_t radioUart;
//    uint8_t marieUart;
//    uint8_t pl2Uart;

    uint8_t debugIsOn;

    uint8_t watchDogInterval;

    uint8_t radioAmateurOn;
    uint16_t radioAmateurMaxTx;
    uint16_t radioAmateurMinVoltage;

    uint8_t experimentStatus[NUMBER_OF_EXPERIMENTS];

    // MEMORY CONFIGURATION
    struct MemorySatellite *memory;
    struct Radio_configuration *radio;
    struct SatelliteTelemetry *telemetry;
    //struct EPS_configuration *eps;
};

struct SatelliteStatus
{

    uint32_t totalMissionMinutes;
    uint32_t timeAtBoot;
    uint32_t minutePagesAddress;
    uint32_t minuteCounterAddress;

    uint8_t event;
    uint16_t errors;
    uint8_t interruptCause;
    uint8_t isOnSun;
    uint32_t lastTimeOnSun;
    uint32_t lastTimeOnShadow;
    uint32_t lastTimeRTC;
    uint32_t lastTimeRTCunixTime;
    uint32_t lastTimeMinuteCounter;

    // Telemetry
    uint32_t telemetryLastTimeBeacon;
    uint32_t telemetryLastTimeTx;
    uint32_t telemetryLastTimeRead;
    uint32_t telemetryLastTimeSaved;

    // LABONCHIP
    uint8_t experimentRunning;
    uint8_t experimentEvent;
    uint8_t currentExperimentIndex;
    uint8_t currentExperimentStep;
    uint32_t startTimeCurrentExpStep;
    uint32_t expPauseInterval;

    // COMM
    uint16_t radioPacketIndex;
    uint16_t radioAckPacketIndex;
    uint32_t lastTimeRxGround;
    uint32_t lastTimeRadioReboot;
    uint16_t amateurPackets;
    uint16_t amateurTxPackets;

    // MEMORY
    struct MemorySatellite *memory;
    uint8_t ignoreMemory;
    uint8_t memoryNoSleep;

    // OPERATIONS
    uint8_t fcnOpReq;
    uint8_t indexFcnOpReq;
    uint8_t memOpReq;

    uint8_t memSubOp;
    uint8_t indexMemOpReq;

    uint8_t groundStationDebug;
    uint8_t lowBatteryFlag;
    uint8_t batteryVoltage;/*SOLAR_CHARGING..DISCHARING STATUS*/

};

int8_t satsystem_loadConfiguration(struct SatelliteConfiguration *configuration,
                                   uint8_t *bufferOrigin);
int8_t satsystem_loadDefaultConfiguration(
        struct SatelliteConfiguration *configuration);

int8_t satsystem_loadDefaultSafeConfiguration(
        struct SatelliteConfiguration *configuration);

int8_t satsystem_loadDefaultExpConfiguration(
        struct SatelliteConfiguration *configuration);

int8_t satsystem_saveConfiguration(struct SatelliteConfiguration *configuration,
                                   uint8_t *bufferDestination);
int8_t configuration_saveNewReset();
int8_t configuration_saveLastTimeOn(uint32_t time);

int8_t loadPartialPermanentSettings();
int8_t savePartialPermanentSettings();

#endif /* CONFIGURATION_H_ */
