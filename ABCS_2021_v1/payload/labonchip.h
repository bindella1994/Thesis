/*
 * labonchip.h
 *
 *  Created on: 8 gen 2021
 *      Author: Augusto Nascetti
 */

#ifndef PAYLOAD_LABONCHIP_H_
#define PAYLOAD_LABONCHIP_H_
#include "abacus.h"
#include "../satellite/configuration.h"
#include "../satellite/abcs_status.h"

#define NUMBER_OF_EXPERIMENTS 6
#define MAX_NUMBER_OF_STEPS 6 //max number of step of an experiment
#define STANDARDSTEPS 4  // number os steps of the default experiment
#define EXPDELTASTEP  5   // 5s delta time steps
#define EXTRAPAUSEACQUISITIONTIME 120000UL //2 minutes before stopping MARIE storage (to prevent memory full)

// experiment status
#define TODO        0
#define RUNNING     1
#define PAUSED      2
#define COMPLETED   3
#define ABORTED     4
#define INITERROR   5

// step states
#define BEGINSTATE      0
#define EXECUTINGSTATE  1
#define ENDSTATE        2
#define COMPLETED       3

// step/experiment EVENTS
#define NONE            (0x0000)
#define BEGIN           (0x0001)
#define TIMEDELTASTEP   (0x0002)
#define STEPTIMEOUT     (0x0004)
#define WETINOK         (0x0008)
#define WETOUTOK        (0x0010)
#define EXPSTANDBY      (0x0020)
#define EXPRESUME       (0x0040)
#define EXPERROR        (0x0080)

// step actions
#define INITEXP     0
#define PUMP1       1
#define PUMP2       2
#define PUMP3       3
#define PUMP4       4
#define PUMP5       5
#define PUMP6       6
#define MEASURE     7
#define PAUSE       8
#define FINISHEXP      9
#define PUMPi       10
#define NEXTSTEP    11


#define SWON  0xFF
#define SWOFF 0x00
#define WETOK 0xFF
/* maybe is useless
 * better use some var for currentExperiment and its startTime
 * and eventually define an array of struct Experiment
 struct ExperimentSet {
 //uint8_t numberOfEpxeriments;
 uint8_t currentExperiment;
 uint32_t startTimeCurrentExp;
 uint8_t experimentStatus[NUMBER_OF_EXPERIMENTS];
 uint32_t startTime[NUMBER_OF_EXPERIMENTS];
 };
 */

struct Experiment
{
    uint8_t experimentID;
    uint8_t numberOfSteps;
    uint32_t startUnixTime;  //ACTUAL START TIME
    uint32_t endUnixTime;
    uint32_t expStartAddress; //STORAGE ADDRESS IN FLASH
    uint32_t expEndAddress;
    uint32_t elapsedTimeinStep;
    uint8_t stepAction[MAX_NUMBER_OF_STEPS];
    uint8_t stepStatus[MAX_NUMBER_OF_STEPS];
    uint16_t stepDuration[MAX_NUMBER_OF_STEPS]; //step duration in seconds (min 1s, max 18h)
};

extern uint8_t currentExperimentID;
extern uint8_t experimentStatus[NUMBER_OF_EXPERIMENTS];
extern uint32_t startTime[NUMBER_OF_EXPERIMENTS];
extern struct Experiment labonchipExperiment_[NUMBER_OF_EXPERIMENTS];

void checkExperiment();
int setupExperiment(uint8_t experimentIndex, uint8_t id, uint8_t numberofsteps,
                    uint8_t *stepactions, uint16_t *stepdurations);
int setupStandardExperiment(uint8_t experimentIndex);
void setupAllExperiments();
uint8_t checkExpIssues(void);
uint8_t pumpIsON(uint8_t pumpNum);
void switchPump(uint8_t pumpNum, uint8_t onoff);
void switchMarie(uint8_t onoff);
uint16_t readWetSensor();
#endif /* PAYLOAD_LABONCHIP_H_ */
