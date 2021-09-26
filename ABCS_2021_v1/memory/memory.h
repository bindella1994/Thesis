/*
 * memory.h
 *
 *  Created on: 02/mag/2014
 *      Author: Aitor
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "memory/abacus_flash.h"
#include "../satellite/configuration.h"
#include "../satellite/satsystem_init.h"
#include "../communications/communications.h"
#include "abacus_utils.h"

// Status/Configuration memory
#define MEMORY_CONF_START         0x00000000U
#define MEMORY_CONF_SIZE          262144UL
#define MEMORY_STATUS_END         0x0003FFFFUL
#define CONF_SIZE_STREAM          128U

// Sensor memory
#define MEMORY_SENSORS_START                0x400000UL
#define MEMORY_SENSORS_COMPLETE_SIZE        1310720UL//TEST 5 SETTORI: 5settoriX1024pagineX256byte                    //4194304 // 16 SETTORI
#define MEMORY_SENSORS_END                  0x53FFFFUL//TEST 5 settori                                             //0x7FFFFF//16 settori
#define SENSORS_SIZE_STREAM                 64U
#define SENSORS_SIZE_MAP_STREAM             1U
#define MEMORY_SENSORS_SECTORS              5U//TEST di 5 settori            16
#define MEMORY_SENSORS_PAGESLIMIT           3072U//TEST 3 settori            //11264// 11 settori
#define MEMORY_SENSORS_BYTESLIMIT           786432UL//TEST  3 settori         //2883584//11 settori come limite
#define MEMORY_SENSORS_PACKETSLIMIT         12288U//TEST 3 settori           //45056// 11 settori
#define MEMORY_SENSORS_PACKETS_PER_SECTOR   4096U

// MARIE memory
#define MEMORY_MARIE_START          0xC00000UL
#define MEMORY_MARIE_COMPLETE_SIZE  4194304UL
#define MEMORY_MARIE_END            0xFFFFFFUL
#define MEMORY_MARIE_SECTORS        16U
#define MARIE_SIZE_STREAM           16U // dimensione info nel settore degli indici
#define MARIE_SIZE_STREAM_DATA      256U
#define MARIE_SIZE_STREAM_HALF_DATA 128U
#define MARIE_SIZE_PAGE             256U
#define MEMORY_MARIE_PAGESLIMIT     11264U // 11 settori... alzare il limite fino a 14 se ho 16 settori

// Event Memory
#define MEMORY_EVENT_START              0x240000UL    // settore 9
#define MEMORY_EVENT_END                0X33FFFFUL   // fine del setore 12
#define MEMORY_EVENT_COMPLETE_SIZE      1048576UL     // 4 settori
#define MEMORY_EVENT_SECTORS            4U
#define MEMORY_EVENT_BYTESLIMIT         786432UL     // 3 settori
#define MEMORY_EVENT_PACKETSLIMIT       49152U      // 3 settori
#define MEMORY_EVENT_PACKETS_PER_SECTOR 16384U
#define EVENT_SIZE_STREAM               16U
#define EVENT_SIZE_MAP_STREAM           1U

// General define about memory
#define MEMORY_BYTES_PER_SECTOR 262144UL
#define MEMORY_PAGES_PER_SECTOR 1024U
#define PAYLOADCHECKSUMSIZE         2U
#define RADIOHEADERSIZE              8U

#define DIMBUFFEROUT                255U

struct MemorySatellite
{
    uint8_t selectedMainMemory; //AB_FLASH_MCU normally

    uint32_t confStartFreeAddress;

    uint32_t sensorsStartFreeAddress;
    uint32_t sensorsEndFreeAddress; //last free address
    uint32_t sensorsStoredPage; // vedere se posso risparmiarmi questi 32 bit
    uint32_t sensorsStoredPacket;

    uint32_t marieStartFreeAddress;
    uint32_t marieEndFreeAddress;
    uint32_t marieStoredPage;

    uint32_t eventStartFreeAddress;
    uint32_t eventEndFreeAddress;
    uint32_t eventStoredPacket;

    uint32_t readAddress;

    uint32_t lastMarieBeaconAddress;

    uint16_t errors;

    //Things for parallel handling
    uint8_t statusParallel;
    uint8_t subStatusParallel;
    uint8_t noSleep;
    uint32_t parallelStartAddress;
    uint32_t parallelEndAddress;

};

extern struct MemorySatellite satellite_memory;

void checkMemoryOp(void);

int8_t memory_init(uint8_t readConf);

int8_t memory_readConf(struct SatelliteConfiguration *conf);
int8_t memory_writeConf(struct SatelliteConfiguration *conf);

uint8_t memory_mapFindAddresses(uint32_t addressDataStart,
                                uint32_t addressDataEnd, uint16_t dataSize,
                                uint32_t *startfreeaddress,
                                uint32_t *endfreeaddress);

uint8_t memory_marie_init(void);
uint8_t memory_marie_saveData(uint8_t *buffer);
uint8_t memory_marie_checkFormatSector(void);
uint8_t memory_marie_formatSector(void);
uint8_t memory_marie_formatAll(void);
uint8_t memory_marie_downloadHalfPage(void);
uint8_t memory_marie_getInfo(uint8_t *buffer);

uint8_t memory_sensors_init(void);
uint8_t memory_sensors_formatAll(void);
uint8_t memory_sensors_write(uint8_t *buffer);
uint8_t memory_sensor_checkFormatSector(void);
uint8_t memory_sensors_formatSector(void);
uint8_t memory_sensor_downloadPacket(void);
uint8_t memory_sensors_getInfo(uint8_t *buffer);

uint8_t memory_event_init(void);
uint8_t memory_event_formatAll(void);
//int8_t memory_logEvent(uint32_t time, uint8_t code);
//int8_t memory_logEventInfo(uint32_t time, uint8_t code, uint8_t *buffer);
int8_t memory_logEvent(uint32_t time,
        uint8_t code,
        uint8_t *data,
        uint8_t lenght);
int8_t memory_logEvent_noPayload(uint32_t time, uint8_t code);
int8_t memory_logSectorEraseEvent(uint32_t time, uint8_t code,
                                  uint32_t address);
uint8_t memory_event_write(uint8_t *buffer);
uint8_t memory_event_checkFormatSector(void);
uint8_t memory_event_formatSector(void);
uint8_t memory_event_downloadPacket(void);
uint8_t memory_event_getInfo(uint8_t *buffer);

int8_t memory_fpga_init();
int8_t memory_fpga_checkCRC(uint8_t flashMemory);
uint16_t memory_fpga_checkCRCPart(uint32_t addressStart, uint32_t lenght);
int8_t memory_fpga_format(uint8_t sectors);
int8_t memory_fpga_saveData(uint32_t addressStart, uint8_t *buffer,
                            uint16_t lenght);
int8_t memory_fpga_saveHeader(uint32_t size, uint32_t crc16);
int8_t memory_fpga_reprogram();

void subOperationSensors();
void subOperationCleanSensors();
void subOperationEvents();
void subOperationCleansEvents();
void subOperationMarie();
void subOperationCleansMarie();
void subOperationBulkErase();



#endif /* MEMORY_H_ */
