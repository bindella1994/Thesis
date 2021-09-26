/*
 * abacus.c
 *
 */

#include "abacus.h"

uint8_t abacusLPMStatus;

/**
 * Initiates all the variables and hardware of the board
 */
int abacus_init(uint8_t abacus_version, uint8_t abacus_clockSpeed)
{
	//Set internal clock
	abacus_init_clock(abacus_clockSpeed);

	//Init GPIOs
	abacus_gpio_init();

	//Init FPGA (not really needed, it is done also in the gpio init)
	//abacus_fpga_init();

	//Init timers:
	abacus_millis_init();

	//Init comms
	//Init UART ports
	abacus_uart_init();
	//Init i2c ports
	abacus_i2c_init();
	//Init SPI ports
	abacus_spi_init();

	//Init memories:
	abacus_flashMCU_init();
	abacus_flashFPGA_init();

	//Init RTC
	abacus_RTC_init();

	//Init sensors:
	abacus_sensors_temperature_init();
	abacus_sensors_acc_init();
	abacus_sensors_gyro_init(abacus_version);
	abacus_sensors_magnetometer_init();

	abacus_gpio_GPIOExpLoad();

	abacusLPMStatus = AB_AM;

	return 0;
}

/**
 * It only initiates the GPIOs and other critical things. Use instead of
 * abacus_init() if you know what you are doing
 *
 */
int abacus_quiet_init()
{
	//Init GPIOs
	abacus_gpio_init();

	abacusLPMStatus = AB_AM;

	return 0;
}

/**
 * You can select the internal clock to 8MHz or 1MHz
 * This will affect timers and serial ports speeds but is already taken care
 * of in the libraries
 * Implemented by Riccardo di Roberto
 */
void abacus_init_clock(uint8_t abacus_clockSpeed)
{
	//Set DCO FLL reference = REFO
	UCSCTL3 |= SELREF_2;
	//Set ACLK = REFO
	UCSCTL4 |= SELA_2;
	//Disable the FLL control loop
	__bis_SR_register(SCG0);
	//Set lowest possible DCOx, MODx
	UCSCTL0 = 0x0000;

	if(abacus_clockSpeed == AB_CLOCK8MHZ)
	{
		//Select DCO range 8MHz operation
		UCSCTL1 = DCORSEL_5;
		//Set DCO multiplier:
		//(N + 1) * FLLRef = Fdco
		//245 * 32768 = 8MHz
		//Set FLL Div = fDCOCLK/2
		UCSCTL2 = FLLD_1 + 244;
		//Enable FLL Control loop
		__bic_SR_register(SCG0);
		//MCLK cycles for DCO to settle
		//32 * 32 * 8MHz / 32,768Hz = 250880 = MCLK
		__delay_cycles(250880);
	}
	else
	{
		//Any other conf will go to defaults 1MHz
		//It is not 1MHz but 1,048576MHz

		//Select DCO range 1,048576MHz operation
		UCSCTL1 = DCORSEL_2;
		//Set DCO multiplier:
		//(N + 1) * FLLRef = Fdco
		//32 * 32768 = 1,048576MHz
		//Set FLL Div = fDCOCLK/2
		UCSCTL2 = FLLD_1 + 31;
		//Enable FLL Control loop
		__bic_SR_register(SCG0);
		//MCLK cycles for DCO to settle
		//32 * 32 * 1MHz / 32,768Hz = 32768 = MCLK
		__delay_cycles(32768);
	}
}

/*
 * It puts the MCU into low power mode
 */
int abacus_enter_LPM(uint8_t lpmSelection)
{
	// Stop watchdog timer
   	WDTCTL = WDTPW | WDTHOLD;
   	//__bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed
   	abacusLPMStatus = lpmSelection;

   	switch(lpmSelection)
   	{
   	case AB_LPM0:
   		__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
   		break;
   	case AB_LPM1:
   		__bis_SR_register(LPM1_bits + GIE);
   		break;
   	case AB_LPM2:
   		__bis_SR_register(LPM2_bits + GIE);
   		break;
   	case AB_LPM3:
   		__bis_SR_register(LPM3_bits + GIE);
   		break;
   	case AB_LPM4:
   		__bis_SR_register(LPM4_bits + GIE);
   		break;
   	case AB_LPM5:
   		//__bic_SR_register_on_exit(LPM5_bits);
   		break;
   	default:
   		break;
   	}

   	__no_operation();
   	abacusLPMStatus = AB_AM;

   	//Reenable wdt
   	//TODO
    //WDTCTL = abacusWDTtime;

	return 0;
}

