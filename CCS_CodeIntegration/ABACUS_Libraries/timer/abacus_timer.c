/*
 * abacus_timer.c
 */

#include "abacus_timer.h"

volatile struct SleepTimer sleepTimer_;
volatile uint32_t timeSinceBoot_;


/*
 *
 */
void abacus_timerA1_enable(uint16_t sec, void (*interruptFunction)(int*))
{
	sleepTimer_.seconds = sec;
	sleepTimer_.counterInterrupt = 0;
	sleepTimer_.interruptFunctionEnabled = 1;
	sleepTimer_.interruptFunction = interruptFunction;

	//Stop just in case...
	TA1CTL = TASSEL_2 + ID_3 + MC_0 + TACLR;
	//TACCR0 interrupt enabled
	TA1CCTL0 = CCIE;
	//this count corresponds to 1sec with /16
	if (AB_IS_8MHZ)
	{
		TA1CCR0 = TIMER1S_8MHZ;
		//Prescaler 2 divided by 8
		TA1EX0 = TAIDEX_7;
		//Clock source SMCLK, Up to CCR0, Clear TA, Divider /8
		TA1CTL = TASSEL_2 + MC_1 + TACLR + ID__8;
	}
	else
	{
		TA1CCR0 = TIMER1S_1MHZ;
		//Prescaler 2 divided by 1
		TA1EX0 = TAIDEX_0;
		//Clock source SMCLK, Up to CCR0, Clear TA, Divider /8
		TA1CTL = TASSEL_2 + MC_1 + TACLR + ID__8;
	}


}

/*
 *
 */
void abacus_timerA1_disable()
{
	//Stop timer:
	TA1CCR0 = 0;
	//Interrupt disabled
	TA1CCTL0 &= ~CCIE;
	//Diusable interrupt function
	sleepTimer_.interruptFunctionEnabled = 0;
}

void abacus_timerA1_pause()
{
	TA1CCTL0 &= ~CCIE;      // TACCR0 interrupt disabled
	TA1CCR0 = 0;       // pause
}

void abacus_timerA1_resume()
{
	TA1CCTL0 = CCIE;
	if (AB_IS_8MHZ)
		TA1CCR0 = TIMER1S_8MHZ;
	else
		TA1CCR0 = TIMER1S_1MHZ;
}

/*
 * It makes abacus sleep x secs.
 */
void abacus_sleep_sec(uint32_t sec)
{
	sleepTimer_.target = sec;
	sleepTimer_.counter = 0;

	//Stop just in case...
	TA0CTL = TASSEL_2 + ID_3 + MC_0 + TACLR;
	//TACCR0 interrupt enabled
	TA0CCTL0 = CCIE;
	//this count corresponds to 1sec with /16
	if (AB_IS_8MHZ)
	{
		TA0CCR0 = TIMER1S_8MHZ;
		//Prescaler 2 divided by 8
		TA0EX0 = TAIDEX_7;
		//Clock source SMCLK, Up to CCR0 and back to 0, Clear TA, Divider /8
		TA0CTL = TASSEL_2 + MC_1 + TACLR + ID__8;
	}
	else
	{
		TA0CCR0 = TIMER1S_1MHZ;
		//Prescaler 2 divided by 1
		TA0EX0 = TAIDEX_0;
		//Clock source SMCLK, Up to CCR0, Clear TA, Divider /8
		TA0CTL = TASSEL_2 + MC_1 + TACLR + ID__8;
	}



	//Enter low power mode
	abacus_enter_LPM(AB_LPM0);

	//Stop timer:
	TA0CCR0 = 0;

	//Enable watchdog for PUC (power up clear)
	//WDTCTL = WDTPW | (uint16_t)unisatConfiguration_.watchDogInterval;
}

/*
 * lets it make sleep msecs
 */
void abacus_sleep_msec(uint32_t msec)
{
	sleepTimer_.target = msec;
	sleepTimer_.counter = 0;

	//Stop just in case...
	TA0CTL = TASSEL_2 + ID_3 + MC_0 + TACLR;
	//TACCR0 interrupt enabled
	TA0CCTL0 = CCIE;
	//this count corresponds to 1msec
	if (AB_IS_8MHZ)
		TA0CCR0 = TIMER1MS_8MHZ;
	else
		TA0CCR0 = TIMER1MS_1MHZ;

	//Prescaler 2 divided by 1
	TA0EX0 = TAIDEX_0;

	//Clock source SMCLK, Up to CCR0, Clear TA, Divider 0
	TA0CTL = TASSEL_2 + MC_1 + TACLR + ID__1;

	//Enter low power mode
	abacus_enter_LPM(AB_LPM0);

	//Stop timer:
	TA0CCR0 = 0;

	//Enable watchdog for PUC
	//WDTCTL = WDTPW | (uint16_t)unisatConfiguration_.watchDogInterval;
}

/*
 * Use it only for values of more than 10uSec, otherwise it is definitely not
 * reliable enough at 1.045MHz. Resolution is 10usec.
 */
void abacus_sleep_usec(uint32_t usec)
{
	sleepTimer_.target = usec/100UL;
	sleepTimer_.counter = 0;

	//Stop just in case...
	TA0CTL = TASSEL_2 + ID_3 + MC_0 + TACLR;
	//TACCR0 interrupt enabled
	TA0CCTL0 = CCIE;
	//this count corresponds to 100usec

	if (AB_IS_8MHZ)
		TA0CCR0 = TIMER100US_8MHZ;
	else
		TA0CCR0 = TIMER100US_1MHZ;
	//Clock source SMCLK, Up to CCR0, Clear TA, Divider 0
	TA0CTL = TASSEL_2 + MC_1 + TACLR + ID__1;

	//Enter low power mode
	abacus_enter_LPM(AB_LPM0);

	//Stop timer:
	TA0CCR0 = 0;

	//Enable watchdog for PUC
	//WDTCTL = WDTPW | (uint16_t)unisatConfiguration_.watchDogInterval;
}

/*
 * Initiates the millis timer on timer B
 */
void abacus_millis_init()
{
	//Iniatialize counter:
	timeSinceBoot_ = 0;

	//Stop just in case...
	TB0CTL = TBSSEL_2 + ID_3 + MC_0 + TBCLR;
	//Select 16bit

	//TBCCR0 interrupt enabled
	TB0CCTL0 = CCIE;
	//this count corresponds to 4sec with /64
	if (AB_IS_8MHZ)
		TB0CCR0 = TIMER1S_8MHZ;
	else
		TB0CCR0 = TIMER4S_1MHZ;
	//Prescaler 2 divided by 8
	TB0EX0 = TBIDEX_7;
	//Clock source SMCLK, Up to CCR0, Clear TB, Divider /8
	TB0CTL = TBSSEL_2 + MC_1 + TBCLR + ID__8;
}


/*
 * It returns the number of elapses milliseconds since the satellite was
 * switched on
 */
uint32_t abacus_millis()
{
	//The overflow will happen after 49 days:
	//4294967296 is the maximum timeSinceBoot_ that the variable can store

	uint32_t result = 0;
	if (AB_IS_8MHZ)
	{
		result = ((uint32_t)(TB0R) * 500UL) / (uint32_t)TIMER1S_8MHZ;
		if(result >= 496)
			return timeSinceBoot_ + result - 500;
		else
			return timeSinceBoot_ + result;
	}
	else if(AB_IS_1MHZ)
	{
		//We have to first multiply by 4000 and then divide. Otherwise we lose
		//resolution
		result = ((uint32_t)(TB0R) * 4000UL) / (uint32_t)TIMER4S_1MHZ;
		if(result >= 3996)
			return timeSinceBoot_ + result - 4000;
		else
			return timeSinceBoot_ + result;
	}
	return result;
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
	//This counter is used only for count ms since switch on! careful!
	if (AB_IS_8MHZ)
		timeSinceBoot_ += 500UL;
	else if(AB_IS_1MHZ)
		timeSinceBoot_ += 4000UL;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	//This counter is used only for sleeping in the code. Careful!!
	if(sleepTimer_.counter == sleepTimer_.target)
	{
		__bic_SR_register_on_exit(LPM0_bits); // wake up CPU on exit
		sleepTimer_.counter = 0;
	}
	else
	{
		sleepTimer_.counter++;
	}
}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	//This counter is used for personal function trigger
	if(sleepTimer_.counterInterrupt == sleepTimer_.seconds)
	{
		sleepTimer_.counterInterrupt = 0;
		//Call exit function
		int wakeupOnExit = 0;
		//Call the associated function
		if(sleepTimer_.interruptFunctionEnabled == 1)
			(*(sleepTimer_.interruptFunction))(&wakeupOnExit);
		else
			wakeupOnExit = 1;

		if(wakeupOnExit == 1)
		{
			//The user wants to exit LPM
			switch(abacusLPMStatus)
			{
			case AB_LPM0:
				__bic_SR_register_on_exit(LPM0_bits);
				break;
			case AB_LPM1:
				__bic_SR_register_on_exit(LPM1_bits);
				break;
			case AB_LPM2:
				__bic_SR_register_on_exit(LPM2_bits);
				break;
			case AB_LPM3:
				__bic_SR_register_on_exit(LPM3_bits);
				break;
			case AB_LPM4:
				__bic_SR_register_on_exit(LPM4_bits);
				break;
			case AB_LPM5:
				//__bic_SR_register_on_exit(LPM5_bits);
				break;
			default:
				break;
			}
		}

	}
	else
	{
		sleepTimer_.counterInterrupt++;
	}
}



