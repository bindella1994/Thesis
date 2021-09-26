/*
 * abacus_uart.h
 *
 */

#ifndef ABACUS_UART_H_
#define ABACUS_UART_H_
#include "msp430x54xa.h"
#include "stdint.h"
#include "../abacus.h"

#define UARTBUFFERLENGHT 100
#define UARTSTATUS_CLOSED 0
#define UARTSTATUS_OPENED 1

struct Port
{
	uint8_t uart_name;
	uint8_t bauds;
	uint8_t status;
	uint8_t interruptWhenNoData;

	uint8_t bufferIn[UARTBUFFERLENGHT];
	uint16_t bufferInStart;
	uint16_t bufferInEnd;

	uint8_t noDisableTXInterrupt;
	uint8_t bufferOut[UARTBUFFERLENGHT];
	uint16_t bufferOutStart;
	uint16_t bufferOutEnd;

	uint8_t triggerEnabled;
	void (*triggerFunction)(uint8_t*);
};


extern volatile struct Port uart00;
extern volatile struct Port uart01;
extern volatile struct Port uart02;
extern volatile struct Port uart03;


void abacus_uart_init();
int8_t abacus_uart_open(uint8_t uart_name,
		uint8_t bauds);

int8_t abacus_uart_open_function(uint8_t uart_name,
		uint8_t bauds,
		void (*interruptFunction)(uint8_t*));

int16_t abacus_uart_outputBufferCount(uint8_t uart_name,
		uint8_t isOut);

int8_t abacus_uart_InterruptisOn(uint8_t uart_name);
int8_t abacus_uart_enableInterrupt(uint8_t uart_name);
int8_t abacus_uart_disableInterrupt(uint8_t uart_name);
int8_t abacus_uart_close(uint8_t uart_name);
int8_t abacus_uart_available(uint8_t uart_name);
int8_t abacus_uart_waitUntilTxFinished(uint8_t uart_name);
uint8_t abacus_uart_read(uint8_t uart_name);
int8_t abacus_uart_write(uint8_t uart_name,
		uint8_t *buffer,
		unsigned int lenght);

int8_t abacus_uart_print(uint8_t uart_name, char *buffer);
int8_t abacus_uart_print_int(uint8_t uart_name, int16_t number);
int8_t abacus_uart_print_uint(uint8_t uart_name, uint16_t number);
int8_t abacus_uart_print_uint8_hex(uint8_t uart_name, uint8_t number);
//int8_t abacus_uart_print_float(uint8_t uart_name, float number);
int8_t abacus_uart_print_ulong(uint8_t uart_name, uint32_t number);

#endif /* ABACUS_UART_H_ */
