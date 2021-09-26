/*
 * abacus_fpga.h
 *
 */

#ifndef ABACUS_FPGA_H_
#define ABACUS_FPGA_H_

#include "../abacus.h"
#include "stdint.h"

// P1.0 pin output Reserve data bus putting it low
#define FPGA_Reserve_DataBus (P1OUT |= BIT0) // Set P1.0=1
#define FPGA_Release_DataBus (P1OUT &= ~BIT0) // Set P1.0=0

// P1.1 pin input to check if FPGA is using Data Bus
#define FPGA_Data_Ready (P1IN & BIT1)

// P1.2 pin output for clock sync
#define FPGA_CkSyncToFPGA_High (P1OUT |= BIT2) // Set P1.2=1
#define FPGA_CkSyncToFPGA_Low (P1OUT &= ~BIT2) // Set P1.2=0

// P1.3 pin output reserve I2C bus
#define FPGA_Reserve_I2CBus (P1OUT |= BIT3) // Set P1.3=1
#define FPGA_Release_I2CBus (P1OUT &= ~BIT3) // Set P1.3=0

// P1.4 pin input for checking if I2C is using I2C
#define FPGA_I2CBus_Busy (P1IN & BIT4)

// P1.5 pin output for reseting the fst of FPGA
#define FPGA_ResetFst_Hi	(P1OUT |= BIT5)
#define FPGA_ResetFst_Low	(P1OUT &= ~BIT5)

// P1.6 input pin for interrupt from FPGA


//Pins for switching on or off
#define FPGA_ON (P2OUT &= ~BIT5)
#define FPGA_OFF (P2OUT |= BIT5)

// Set the low and high bytes of databus to input or output
#define FPGA_LOWBYTE_asInput 	(P8DIR = 0x00)
#define FPGA_LOWBYTE_asOutput 	(P8DIR = 0xFF)
#define FPGA_HIGHBYTE_asInput 	(P4DIR = 0x00)
#define FPGA_HIGHBYTE_asOutput	(P4DIR = 0xFF)

// Easy access to low or high bytes (input or output)
#define FPGA_LOWBYTE_OUT 	(P8OUT)
#define FPGA_LOWBYTE_IN		(P8IN)
#define FPGA_HIGHBYTE_OUT	(P4OUT)
#define FPGA_HIGHBYTE_IN	(P4IN)

extern void (*triggerFPGAFunction)(int*);
extern int8_t fpgaInterruptEnabled;

void abacus_fpga_init();

int8_t abacus_fpga_isOn();
int8_t abacus_fpga_switchOn();
int8_t abacus_fpga_switchOff();
int8_t abacus_fpga_FSM_reset();
int8_t abacus_fpga_cold_reset();
int8_t abacus_fpga_resetProgram();
int8_t abacus_fpga_prgB_holdDown();
int8_t abacus_fpga_prgB_holdUp();

int8_t abacus_fpga_i2cGetStatus();
int8_t abacus_fpga_i2cReserve();
int8_t abacus_fpga_i2cRelease();

int8_t abacus_fpga_interruptisOn();
int8_t abacus_fpga_interruptEnable(void (*interruptFunction)(int*));
int8_t abacus_fpga_interruptDisable();


int8_t abacus_fpga_dataBusReserve();
int8_t abacus_fpga_dataBusRelease();
int8_t abacus_fpga_dataBusSetHighZ();
int8_t abacus_fpga_dataBusSetLowZ();
int8_t abacus_fpga_dataBusSendData(uint16_t data);
int8_t abacus_fpga_dataBusSendArray(uint16_t *buffer, uint16_t lenght);
int8_t abacus_fpga_dataBusSendArray_8bit(uint8_t *buffer, uint16_t lenght);
int8_t abacus_fpga_dataBusGetData(uint16_t *buffer);
int8_t abacus_fpga_dataBusGetArray(uint16_t *buffer, uint16_t lenght);
int8_t abacus_fpga_dataBusGetArray_8bit(uint8_t *buffer, uint16_t lenght);

#endif /* ABACUS_FPGA_H_ */
