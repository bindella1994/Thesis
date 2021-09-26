/*
 * abacus_timer.h
 *
 * TIMER A1 is used for user defined functions
 * TIMER A0 is used for accurate sleep of s, ms and us
 * TIMER B0 is used for counting time in ms since ABACUS boot
 */

#ifndef ABACUS_TIMER_H_
#define ABACUS_TIMER_H_

/*
#define TIMER50MS_1MHZ 52250 //50ms without prescalers
#define TIMER1MS_1MHZ 1045 //1ms without prescalers
#define TIMER100US_1MHZ 104 //104us without prescalers
#define TIMER1S_1MHZ 65313 //1s using /8 and /2 = /16
#define TIMER4S_1MHZ 65313 //1s using /8 and /8 = /64
*/

#define TIMER1MS_1MHZ 1048 //1ms without prescalers
#define TIMER100US_1MHZ 105 //104us without prescalers
#define TIMER1S_1MHZ 65535 //1s using /8 and /2 = /16	//Max value is 65535, just enough
#define TIMER4S_1MHZ 65535 //1s using /8 and /8 = /64	//Max value is 65535, just enough

#define TIMER1MS_8MHZ 8000 //1ms without prescalers
#define TIMER100US_8MHZ 800 //104us without prescalers
#define TIMER1S_8MHZ 62500 //1s using /8 and /8 = /64


#include "../abacus.h"


struct SleepTimer
{
	uint32_t target;
	uint32_t counter;

	uint16_t seconds;
	uint16_t counterInterrupt;
	uint8_t interruptFunctionEnabled;
	void (*interruptFunction)(int*);
};

extern volatile struct SleepTimer sleepTimer_;
extern volatile uint32_t timeSinceBoot_;

void abacus_timerA1_enable(uint16_t sec, void (*interruptFunction)(int*));
void abacus_timerA1_disable();
void abacus_timerA1_pause();
void abacus_timerA1_resume();

void abacus_sleep_sec(uint32_t sec);
void abacus_sleep_msec(uint32_t msec);
void abacus_sleep_usec(uint32_t usec);

void abacus_millis_init();
uint32_t abacus_millis();

#endif /* ABACUS_TIMER_H_ */
