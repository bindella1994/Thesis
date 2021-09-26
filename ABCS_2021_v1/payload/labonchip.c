/*
 * labonchip.c
 *
 *  Created on: 8 gen 2021
 *      Author: Augusto Nascetti
 */

#include <memory/memory.h>
#include "labonchip.h"

uint8_t currentExperimentID;
uint8_t experimentStatus[NUMBER_OF_EXPERIMENTS];
uint16_t plannedStartTime[NUMBER_OF_EXPERIMENTS]; //in seconds
uint16_t nextDeltaStep;
struct Experiment labonchipExperiment_[NUMBER_OF_EXPERIMENTS];
uint32_t startPauseTime;
uint8_t standardActions[MAX_NUMBER_OF_STEPS] = { INITEXP, PUMPi, MEASURE,
FINISHEXP, FINISHEXP, FINISHEXP }; // STANDARDSTEPS = 4

uint16_t standardDurations[MAX_NUMBER_OF_STEPS] = { 20, 60, 600, 20, 0, 0 };
uint8_t eventWetInLogged;
uint8_t eventWetOutLogged;

void checkExperiment()
{
    uint32_t timeNow = abacus_millis();
    uint16_t wetRefValue;
    uint16_t wetNowValue;
    uint8_t activePump;

//    if (satelliteStatus_.status != EXPERIMENTMODE)
//        return;

    /**************************************************
     * NO EXPERIMENT RUNNING   (start new experiment?)
     **************************************************/
    if (satelliteStatus_.experimentRunning == TODO)
    { //&& NORMALMODE (&& BAT>x && y<T<z)
      //no experiment running
      //check if it's time for a new experiment
      //1) check the next experiment to do
        int i;
        uint8_t expFound = 0;
        uint16_t nextStartTime = 0xFFFF;
        for (i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
        {
            if (experimentStatus[i] == TODO)
            {
                if (plannedStartTime[i] < nextStartTime)
                {
                    nextStartTime = plannedStartTime[i];
                    satelliteStatus_.currentExperimentIndex = i;
                    expFound = 1;
                }
            }
        }
        if (expFound == 0)
        {
            // check if there are ABORTED EXPERIMENTS
            uint16_t nextStartTime = 0xFFFF;
            for (i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
            {
                if (experimentStatus[i] == ABORTED)
                {
                    if (plannedStartTime[i] < nextStartTime)
                    {
                        nextStartTime = plannedStartTime[i];
                        satelliteStatus_.currentExperimentIndex = i;
                        expFound = 1;
                    }
                }
            }
            if (expFound == 0)
                return;      // no more experiments to do
        }
        // time to start the new experiment?
        // (should work without changes also for aborted experiments)
        if (timeNow
                > ((uint32_t) plannedStartTime[satelliteStatus_.currentExperimentIndex]
                        * 1000UL))
        {
            //start next experiment
            //reset the experiment event variable
            satelliteStatus_.experimentEvent = 0;
            satelliteStatus_.currentExperimentStep = INITEXP; // step 0 is always the START
            // the change of step is done in the running-experiment part
            currentExperimentID =
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].experimentID;
            satelliteStatus_.experimentRunning = RUNNING;
            experimentStatus[satelliteStatus_.currentExperimentIndex] = RUNNING;

            //BEGINSTATE is 0
            //in theory there is no need to do this because each step STATUS is initialized at 0
            labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
            BEGINSTATE;

            satelliteStatus_.experimentEvent = BEGIN;

        }
        else
        {
            // no experiment running
            // and still not time to start a new one
            // return to jump next code
            return;
        }
    } // end if experiment TODO

    /**************************************************
     * EXPERIMENT RUNNING
     **************************************************/
    if (satelliteStatus_.experimentRunning == RUNNING)
    {
        // don't need a 'wake up' because during experiment there will be
        // triggers from MARIE

        // there is a running experiment (eventually one just started now)
        // is it time for a new step?
        uint8_t stepStatus;
        stepStatus =
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep];

        switch (stepStatus)
        {
        case BEGINSTATE:
            // BEGIN STATE of EACH STEP
            satelliteStatus_.startTimeCurrentExpStep = timeNow;
            satelliteStatus_.expPauseInterval = 0;
            nextDeltaStep = EXPDELTASTEP;
            labonchipExperiment_[satelliteStatus_.currentExperimentIndex].elapsedTimeinStep =
                    0;

            //go to the next SM state
            labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
            EXECUTINGSTATE;
            break;

        case EXECUTINGSTATE:
            // EXECUTE ACTION

            // store the time spent in current step taking into account of any pause
            labonchipExperiment_[satelliteStatus_.currentExperimentIndex].elapsedTimeinStep =
                    timeNow - satelliteStatus_.startTimeCurrentExpStep
                            - satelliteStatus_.expPauseInterval;

            // Check battery (eventually pause the experiment?)

            switch (labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepAction[satelliteStatus_.currentExperimentStep])
            {
            case INITEXP:
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].startUnixTime =
                        timeNow;
                currentExperimentID =
                        labonchipExperiment_[satelliteStatus_.currentExperimentIndex].experimentID;

                // Configure beacon and telemetry timers
                //setSatelliteStatus(EXPERIMENTMODE);

                // SWITCH MARIE INTERRUPT
                switchMarie(SWON);

                // Check memory
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].expStartAddress =
                        satellite_memory.marieStartFreeAddress;

                // Enable MARIE INTERRUPT
                MarieStatus_.marieInterrupt = 1U;

                // read wet sensor ref resistance
                wetRefValue = readWetSensor();

                // Check errors
                if (checkExpIssues())
                {
                    // report error
                    //experimentStatus[satelliteStatus_.currentExperimentIndex]=INITERROR;
                    satelliteStatus_.experimentRunning = ABORTED;  //4 aborted
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[0] =
                    INITERROR;
                    satelliteStatus_.experimentEvent = EXPERROR;
                }

                //go to the next step without waiting for timeout
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                ENDSTATE;
                break;

            case PUMP1:
            case PUMP2:
            case PUMP3:
            case PUMP4:
            case PUMP5:
            case PUMP6:
                // Switch ON the pump
                activePump =
                        labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepAction[satelliteStatus_.currentExperimentStep];

                // this prevents repeating activation if pump is on (but it wouldn't be a problem)
                // but it also allows to restart the pump after returning from a pause
                if (!pumpIsON(activePump))
                    switchPump(activePump,SWON);

                // Read wet sensor
                wetNowValue = readWetSensor();

                // Check battery (eventually pause the experiment if lowbattery (SAFEMODE),
                // and if less than 50% of pumping time elapsed 500UL=1000 (s to ms) / 2 (=50% of elapsed time)
                // and if not wet
                if ((satelliteConfiguration_.status == SAFEMODE)
                        && (labonchipExperiment_[satelliteStatus_.currentExperimentIndex].elapsedTimeinStep
                                < (uint32_t) labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepDuration[satelliteStatus_.currentExperimentStep]
                                        * 500UL)
                        && (wetNowValue >= wetRefValue))
                {
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                    PAUSED;
                    // Switch OFF the pump
                    switchPump(activePump,SWOFF);

                    //calculate the duration of the pause
                    startPauseTime = timeNow;
                    satelliteStatus_.experimentRunning = PAUSED;
                }

                // check if the inlet is wet
                if (wetNowValue < (0.8) * wetRefValue)
                {
                    //log succeffull wetting
                    memory_logEvent_noPayload(timeNow, WETOK);
                    // Switch OFF the pump
                    switchPump(activePump, SWOFF);
                    //go to the next step without waiting for timeout
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                    ENDSTATE;
                }

                // step timeout?
                if ((labonchipExperiment_[satelliteStatus_.currentExperimentIndex].elapsedTimeinStep
                        >= (uint32_t) labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepDuration[satelliteStatus_.currentExperimentStep]
                                * 1000UL))
                {
                    // Switch OFF the pump
                    switchPump(activePump,SWOFF);
                    // go to the next step state
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                    ENDSTATE;
                }
                break;

            case MEASURE:
                // something to do?
                // probably not (or synchronous telemetry acquisition every DELTASTEP?)

                // step timeout?
                if ((labonchipExperiment_[satelliteStatus_.currentExperimentIndex].elapsedTimeinStep
                        >= (uint32_t) labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepDuration[satelliteStatus_.currentExperimentStep]
                                * 1000UL))
                {
                    // go to the next step state
                    labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                    ENDSTATE;
                }
                break;

            case FINISHEXP:
                // finish saving data
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].expEndAddress =
                        satellite_memory.marieStartFreeAddress; // -1 page
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].endUnixTime =
                        timeNow;
                satelliteStatus_.experimentEvent = COMPLETED;
                satelliteStatus_.experimentRunning = TODO;
                //go to the next step without waiting for timeout
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                ENDSTATE;
                break;

            default:
                break;
            }

            break;

        case ENDSTATE:
            labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
            COMPLETED;

            if (satelliteStatus_.currentExperimentStep
                    >= labonchipExperiment_[satelliteStatus_.currentExperimentIndex].numberOfSteps)
            {
                //EXPERIMENT FINISHED
                //this should be not necessary as these thing are done
                //in the EXECUTING STATE of the FINISHEXP step
                experimentStatus[satelliteStatus_.currentExperimentIndex] =
                        COMPLETED;
                satelliteStatus_.experimentEvent = COMPLETED;
                satelliteStatus_.experimentRunning = TODO;
            }
            else
            {
                // go to the next step
                satelliteStatus_.currentExperimentStep++;
                //BEGINSTATE is 0
                //in theory there is no need to do this because each step STATUS is initialized at 0
                labonchipExperiment_[satelliteStatus_.currentExperimentIndex].stepStatus[satelliteStatus_.currentExperimentStep] =
                BEGINSTATE;
            }
            break;

        default:
            break;
        }
    } // end if experiment running

    /**************************************************
     * EXPERIMENT PAUSED
     **************************************************/
    if (satelliteStatus_.experimentRunning == PAUSED)
    {
        /*case MAXPAUSE:
         // not restored Normal opMode
         satelliteStatus_.experimentRunning = 1;
         break;*/

        uint32_t currentPauseTime = timeNow - startPauseTime;
        if (currentPauseTime > EXTRAPAUSEACQUISITIONTIME)
        {
            // after 2minutes in pause stop the storage of MARIE DATA
            // or just sore one every 10 (but check the timestamp in MARIE FW)

            // disable MARIE interrupt
            MarieStatus_.marieInterrupt = 0U;
            // NOTE: now there will be not the wakeup from MARIE!
        }

        // power GOOD?
        if ((satelliteConfiguration_.status == EXPERIMENTMODE)) // ||(satelliteStatus_.status == NORMALMODE))
        {
            satelliteStatus_.experimentEvent = EXPRESUME;
            if (MarieStatus_.marieInterrupt == 0U)
                MarieStatus_.marieInterrupt = 1U;
            // restored Normal opMode
            satelliteStatus_.expPauseInterval += currentPauseTime;
            satelliteStatus_.experimentRunning = RUNNING;
        }
    } // end if experiment paused

    /**************************************************
     * EXPERIMENT ABORTED
     **************************************************/

    if (satelliteStatus_.experimentRunning == ABORTED)
    {
        //HANDLE ABORTED EXPERIMENts
        // Enable MARIE INTERRUPT
        MarieStatus_.marieInterrupt = 0U;
        switchMarie(SWOFF);
        experimentStatus[satelliteStatus_.currentExperimentIndex] = ABORTED;
        satelliteStatus_.experimentRunning = TODO;

    }
}

int setupExperiment(uint8_t experimentIndex, uint8_t id, uint8_t numberofsteps,
                    uint8_t *stepactions, uint16_t *stepdurations)
{
    if (experimentIndex >= NUMBER_OF_EXPERIMENTS)
        return -1;
    if (numberofsteps >= MAX_NUMBER_OF_STEPS)
        return -2;
    labonchipExperiment_[experimentIndex].experimentID = id;
    labonchipExperiment_[experimentIndex].numberOfSteps = numberofsteps;
    labonchipExperiment_[experimentIndex].startUnixTime = 0UL;
    labonchipExperiment_[experimentIndex].endUnixTime = 0UL;
    labonchipExperiment_[experimentIndex].elapsedTimeinStep = 0UL;
    labonchipExperiment_[experimentIndex].expStartAddress = MEMORY_MARIE_START;
    labonchipExperiment_[experimentIndex].expEndAddress = MEMORY_MARIE_START;
    int i;
    for (i = 1; i < numberofsteps; i++)
    {
        labonchipExperiment_[experimentIndex].stepStatus[i] = TODO;
        labonchipExperiment_[experimentIndex].stepAction[i] = *stepactions;
        stepactions++;
        labonchipExperiment_[experimentIndex].stepDuration[i] = *stepdurations;
        stepdurations++;
    }
    return 0;
}

int setupStandardExperiment(uint8_t experimentIndex)
{
    if (experimentIndex >= NUMBER_OF_EXPERIMENTS)
        return -1;
    labonchipExperiment_[experimentIndex].experimentID = experimentIndex;
    labonchipExperiment_[experimentIndex].numberOfSteps = STANDARDSTEPS;
    labonchipExperiment_[experimentIndex].startUnixTime = 0UL;
    labonchipExperiment_[experimentIndex].endUnixTime = 0UL;
    labonchipExperiment_[experimentIndex].elapsedTimeinStep = 0UL;
    labonchipExperiment_[experimentIndex].expStartAddress = MEMORY_MARIE_START;
    labonchipExperiment_[experimentIndex].expEndAddress = MEMORY_MARIE_START;
    int i;
    for (i = 1; i < MAX_NUMBER_OF_STEPS; i++)
    {
        labonchipExperiment_[experimentIndex].stepStatus[i] = TODO;
        if (standardActions[i] == PUMPi)
            labonchipExperiment_[experimentIndex].stepAction[i] =
                    experimentIndex + 1; // experiment 0 pump 1, ecc.
        else
            labonchipExperiment_[experimentIndex].stepAction[i] =
                    standardActions[i];

        labonchipExperiment_[experimentIndex].stepDuration[i] =
                standardDurations[i];
    }
    return 0;
}

void setupAllExperiments()
{
    int i;
    int res = 0;
    for (i = 1; i < NUMBER_OF_EXPERIMENTS; i++)
    {
        res += setupStandardExperiment(i);
    }
}

uint8_t checkExpIssues(void){
    // check if everything is OK
    // before starting
    return 0; // replace with the corrct return value
}

uint8_t pumpIsON(uint8_t pumpNum)
{
    // check it the pump is activated
    return 0; // replace with the corrct return value
}

void switchPump(uint8_t pumpNum, uint8_t onoff)
{
    if (onoff == SWON)
    {
        // Switch ON
    }
    else
    {
        // Switch OFF
    }
}

void switchMarie(uint8_t onoff)
{
    if (onoff == SWON)
    {
        // Switch ON
        // open UART?
        // check current?
        // other things to do?

    }
    else
    {
        // Switch OFF
        // close UART?
        // check current?
        // other things to do?
    }
}
uint16_t readWetSensor()
{
    //read wet sensor
    return 0; // replace with the corrct return value
}
