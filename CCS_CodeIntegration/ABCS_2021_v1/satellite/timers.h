/*
 * timers.h
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#define TIMER50MS 52250 //50ms without prescalers
#define TIMER1MS 1045 //1ms without prescalers
#define TIMER100US 104 //104us without prescalers
#define TIMER1S 65313 //1s using /8 and /2 = /16
#define TIMER4S 65313 //1s using /8 and /8 = /64

#include "abacus.h"
#include "configuration.h"
#include "satsystem_init.h"

/*
 * included in the new libraries
 * struct SleepTimer
{
	uint32_t target;
	uint32_t counter;

	uint16_t seconds;
	uint16_t counterInterrupt;
	uint8_t interruptFunctionEnabled;
	void (*interruptFunction)(int*);
};

extern struct SleepTimer sleepTimer_;
extern uint32_t timeSinceBoot_;
*/
void abacus_timerA1_enable(uint16_t sec, void (*interruptFunction)(int*));
void abacus_timerA1_disable();
void abacus_timerA1_pause();
void abacus_timerA1_resume();

void abacus_sleep_sec(uint32_t sec);
void abacus_sleep_msec(uint32_t msec);
void abacus_sleep_usec(uint32_t usec);

void check_minute_counter();

//now in libraries
//void abacus_millis_init();
//uint32_t abacus_millis();

uint8_t timer_goToSleep();

uint32_t getUnixTimeNow();





#endif /* TIMERS_H_ */
