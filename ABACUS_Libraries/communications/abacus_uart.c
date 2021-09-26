/*
 * abacus_uart.c
 *
 */

#include "abacus_uart.h"
#include <stdio.h>

volatile struct Port uart00;
volatile struct Port uart01;
volatile struct Port uart02;
volatile struct Port uart03;

/*
 * Initialize variables of the UART ports
 */
void abacus_uart_init()
{
    uart00.uart_name = AB_UART00;
    uart00.status = UARTSTATUS_CLOSED;
    uart00.bufferInStart = 0;
    uart00.bufferInEnd = 0;
    uart00.bufferOutStart = 0;
    uart00.bufferOutEnd = 0;
    uart00.triggerEnabled = 0;
    uart00.interruptWhenNoData = 0;

    uart01.uart_name = AB_UART01;
    uart01.status = UARTSTATUS_CLOSED;
    uart01.bufferInStart = 0;
    uart01.bufferInEnd = 0;
    uart01.bufferOutStart = 0;
    uart01.bufferOutEnd = 0;
    uart01.triggerEnabled = 0;
    uart01.interruptWhenNoData = 0;

    uart02.uart_name = AB_UART02;
    uart02.status = UARTSTATUS_CLOSED;
    uart02.bufferInStart = 0;
    uart02.bufferInEnd = 0;
    uart02.bufferOutStart = 0;
    uart02.bufferOutEnd = 0;
    uart02.triggerEnabled = 0;
    uart02.interruptWhenNoData = 0;

    uart03.uart_name = AB_UART03;
    uart03.status = UARTSTATUS_CLOSED;
    uart03.bufferInStart = 0;
    uart03.bufferInEnd = 0;
    uart03.bufferOutStart = 0;
    uart03.bufferOutEnd = 0;
    uart03.triggerEnabled = 0;
    uart03.interruptWhenNoData = 0;
}

/*
 * It opens the uart port
 */
int8_t abacus_uart_open(uint8_t uart_name, uint8_t bauds)
{
    //Sanity check:
    if (uart_name > 4)
        return -1;
    if (bauds > 6)
        return -2;

    //Clock source: CLK = SMCLK
    const uint8_t clockSource = UCSSEL_2;
    uint8_t clockConf[2];
    uint8_t clockModulation;

    //Configuration bytes for speed

    if (AB_IS_1MHZ)
    {
        //Clock is 1MHz
        if (bauds == AB_B9600)
        {
            clockConf[0] = 0x6D;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_2 + UCBRF_0;
        }
        else if (bauds == AB_B19200)
        {
            clockConf[0] = 0x36;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_5 + UCBRF_0;
        }
        else if (bauds == AB_B38400)
        {
            clockConf[0] = 0x1B;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_2 + UCBRF_0;
        }
        else if (bauds == AB_B57600)
        {
            clockConf[0] = 0x12;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_1 + UCBRF_0;
        }
        else if (bauds == AB_B115200)
        {
            clockConf[0] = 0x09;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_1 + UCBRF_0;
        }
    }
    else if (AB_IS_8MHZ)	//Added by Riccardo Di Roberto for the 8MHz
    {
        //Clock is 8Mhz
        if (bauds == AB_B9600)
        {
            clockConf[0] = 0x41;
            clockConf[1] = 0x03;
            clockModulation = UCBRS_2 + UCBRF_0;
        }
        else if (bauds == AB_B19200)
        {
            clockConf[0] = 0xA0;
            clockConf[1] = 0x01;
            clockModulation = UCBRS_6 + UCBRF_0;
        }
        else if (bauds == AB_B38400)
        {
            clockConf[0] = 0xD0;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_3 + UCBRF_0;
        }
        else if (bauds == AB_B57600)
        {
            clockConf[0] = 0x8A;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_7 + UCBRF_0;
        }
        else if (bauds == AB_B115200)
        {
            clockConf[0] = 0x45;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_4 + UCBRF_0;
        }
        else if (bauds == AB_B230400)
        {
            clockConf[0] = 0x22;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_6 + UCBRF_0;
        }
        else if (bauds == AB_B460800)
        {
            clockConf[0] = 0x11;
            clockConf[1] = 0x00;
            clockModulation = UCBRS_3 + UCBRF_0;
        }
    }

    if (uart_name == AB_UART00)		// P3.4,5 = USCI_A0 TXD/RXD
    {
        uart00.status = UARTSTATUS_OPENED;
        uart00.bufferInStart = 0;
        uart00.bufferInEnd = 0;
        uart00.bufferOutStart = 0;
        uart00.bufferOutEnd = 0;
        P3SEL |= 0x30;
        // **Put state machine in reset**
        UCA0CTL1 |= UCSWRST;
        UCA0CTL1 |= clockSource;

        UCA0BR0 = clockConf[0];
        UCA0BR1 = clockConf[1];
        // Modulation
        UCA0MCTL = clockModulation;
        // **Initialize USCI state machine**
        UCA0CTL1 &= ~UCSWRST;
        // Enable RX interrupt
        UCA0IE |= UCRXIE;
    }
    else if (uart_name == AB_UART01)	// P5.6,7 = USCI_A1 TXD/RXD
    {
        uart01.status = UARTSTATUS_OPENED;
        uart01.bufferInStart = 0;
        uart01.bufferInEnd = 0;
        uart01.bufferOutStart = 0;
        uart01.bufferOutEnd = 0;
        P5SEL |= 0xC0;
        // **Put state machine in reset**
        UCA1CTL1 |= UCSWRST;
        UCA1CTL1 |= clockSource;

        UCA1BR0 = clockConf[0];
        UCA1BR1 = clockConf[1];
        // Modulation
        UCA1MCTL = clockModulation;
        // **Initialize USCI state machine**
        UCA1CTL1 &= ~UCSWRST;
        // Enable RX interrupt
        UCA1IE |= UCRXIE;
    }
    else if (uart_name == AB_UART02)	// P9.4,5 = USCI_A2 TXD/RXD
    {
        uart02.status = UARTSTATUS_OPENED;
        uart02.bufferInStart = 0;
        uart02.bufferInEnd = 0;
        uart02.bufferOutStart = 0;
        uart02.bufferOutEnd = 0;
        P9SEL |= 0x30;
        // **Put state machine in reset**
        UCA2CTL1 |= UCSWRST;
        UCA2CTL1 |= clockSource;

        UCA2BR0 = clockConf[0];
        UCA2BR1 = clockConf[1];
        // Modulation
        UCA2MCTL = clockModulation;
        // **Initialize USCI state machine**
        UCA2CTL1 &= ~UCSWRST;
        // Enable RX interrupt
        UCA2IE |= UCRXIE;
    }
    else if (uart_name == AB_UART03)	// P10.4,5 = USCI_A3 TXD/RXD
    {
        uart03.status = UARTSTATUS_OPENED;
        uart03.bufferInStart = 0;
        uart03.bufferInEnd = 0;
        uart03.bufferOutStart = 0;
        uart03.bufferOutEnd = 0;
        P10SEL |= 0x30;
        // **Put state machine in reset**
        UCA3CTL1 |= UCSWRST;
        UCA3CTL1 |= clockSource;

        UCA3BR0 = clockConf[0];
        UCA3BR1 = clockConf[1];
        // Modulation
        UCA3MCTL = clockModulation;
        // **Initialize USCI state machine**
        UCA3CTL1 &= ~UCSWRST;
        // Enable RX interrupt
        UCA3IE |= UCRXIE;
    }

    // Enable interrupts
    __bis_SR_register(GIE);

    //Return success
    return 0;
}

/*
 * Check if the interrupt for a specific port is ON
 */
int8_t abacus_uart_InterruptisOn(uint8_t uart_name)
{
    if (uart_name == AB_UART00)
        return UCA0IE & BIT0;
    else if (uart_name == AB_UART01)
        return UCA1IE & BIT0;
    else if (uart_name == AB_UART02)
        return UCA2IE & BIT0;
    else if (uart_name == AB_UART03)
        return UCA3IE & BIT0;

    return 0;
}
/*
 * Enables the interrupt for a specific port
 */
int8_t abacus_uart_enableInterrupt(uint8_t uart_name)
{
    if (uart_name == AB_UART00)
        UCA0IE |= UCRXIE;
    else if (uart_name == AB_UART01)
        UCA1IE |= UCRXIE;
    else if (uart_name == AB_UART02)
        UCA2IE |= UCRXIE;
    else if (uart_name == AB_UART03)
        UCA3IE |= UCRXIE;

    return 0;
}

/*
 * Disables the interrupt for a specific port
 */
int8_t abacus_uart_disableInterrupt(uint8_t uart_name)
{
    if (uart_name == AB_UART00)
        UCA0IE &= ~UCRXIE;
    else if (uart_name == AB_UART01)
        UCA1IE &= ~UCRXIE;
    else if (uart_name == AB_UART02)
        UCA2IE &= ~UCRXIE;
    else if (uart_name == AB_UART03)
        UCA3IE &= ~UCRXIE;

    return 0;
}

/*
 * It opens the port and enables an interruption function defined by the user
 */
int8_t abacus_uart_open_function(uint8_t uart_name, uint8_t bauds,
                                 void (*interruptFunction)(uint8_t*))
{
    //Open the port with the predefined function
    int8_t result = abacus_uart_open(uart_name, bauds);

    //Enable the trigger function
    if (uart_name == AB_UART00)
    {
        uart00.triggerEnabled = 1;
        uart00.triggerFunction = interruptFunction;
    }
    else if (uart_name == AB_UART01)
    {
        uart01.triggerEnabled = 1;
        uart01.triggerFunction = interruptFunction;
    }
    else if (uart_name == AB_UART02)
    {
        uart02.triggerEnabled = 1;
        uart02.triggerFunction = interruptFunction;
    }
    else if (uart_name == AB_UART03)
    {
        uart03.triggerEnabled = 1;
        uart03.triggerFunction = interruptFunction;
    }

    return result;
}

/*
 * Close uart port
 */
int8_t abacus_uart_close(uint8_t uart_name)
{
    //Enable the trigger function
    if (uart_name == AB_UART00)
    {
        uart00.status = UARTSTATUS_CLOSED;
        uart00.triggerEnabled = 0;
        uart00.bufferInStart = 0;
        uart00.bufferInEnd = 0;
        uart00.bufferOutStart = 0;
        uart00.bufferOutEnd = 0;
        //Disable TX & RX interrupt
        UCA0IE &= ~UCTXIE;
        UCA0IE &= ~UCRXIE;
        //Put on HiZ the pins
        //Set as GPIO P3.4-5
        P3SEL &= ~(BIT4 | BIT5);
        //Set as input
        P3DIR &= ~(BIT4 | BIT5);

    }
    else if (uart_name == AB_UART01)
    {
        uart01.status = UARTSTATUS_CLOSED;
        uart01.triggerEnabled = 0;
        uart01.bufferInStart = 0;
        uart01.bufferInEnd = 0;
        uart01.bufferOutStart = 0;
        uart01.bufferOutEnd = 0;
        //Disable TX & RX interrupt
        UCA1IE &= ~UCTXIE;
        UCA1IE &= ~UCRXIE;
        //Put on HiZ the pins
        //Set as GPIO P5.6-7
        P5SEL &= ~(BIT6 | BIT7);
        //Set as input
        P5DIR &= ~(BIT6 | BIT7);
    }
    else if (uart_name == AB_UART02)
    {
        uart02.status = UARTSTATUS_CLOSED;
        uart02.triggerEnabled = 0;
        uart02.bufferInStart = 0;
        uart02.bufferInEnd = 0;
        uart02.bufferOutStart = 0;
        uart02.bufferOutEnd = 0;
        //Disable TX & RX interrupt
        UCA2IE &= ~UCTXIE;
        UCA2IE &= ~UCRXIE;
        //Put on HiZ the pins
        //Set as GPIO P9.4-5
        P9SEL &= ~(BIT4 | BIT5);
        //Set as input
        P9DIR &= ~(BIT4 | BIT5);
    }
    else if (uart_name == AB_UART03)
    {
        uart03.status = UARTSTATUS_CLOSED;
        uart03.triggerEnabled = 0;
        uart03.bufferInStart = 0;
        uart03.bufferInEnd = 0;
        uart03.bufferOutStart = 0;
        uart03.bufferOutEnd = 0;
        //Disable TX & RX interrupt
        UCA3IE &= ~UCTXIE;
        UCA3IE &= ~UCRXIE;
        //Put on HiZ the pins
        //Set as GPIO P10.4-5
        P10SEL &= ~(BIT4 | BIT5);
        //Set as input
        P10DIR &= ~(BIT4 | BIT5);
    }
    return 0;
}

/*
 * It counts the size of the buffer
 * This function must not be called outside of this file (it is not available
 * in the abacus_uart.h file.
 */
int bufferSize(uint8_t isOut, volatile struct Port *uart)
{
    if (isOut == 0)
    {
        if (uart->bufferInEnd > uart->bufferInStart)
            return (uart->bufferInEnd - uart->bufferInStart);
        else if (uart->bufferInEnd < uart->bufferInStart)
            return (UARTBUFFERLENGHT - uart->bufferInStart + uart->bufferInEnd);
    }
    else
    {
        if (uart->bufferOutEnd > uart->bufferOutStart)
            return (uart->bufferOutEnd - uart->bufferOutStart);
        else if (uart->bufferOutEnd < uart->bufferOutStart)
            return (UARTBUFFERLENGHT - uart->bufferOutStart + uart->bufferOutEnd);
    }
    return 0;
}

/*
 * It returns the size of the non sent bytes yet. if isOut is 1, it will return the
 * output bytes yet to be sent
 */
int abacus_uart_outputBufferCount(uint8_t uart_name, uint8_t isOut)
{
    if (uart_name == AB_UART00)
        return bufferSize(isOut, &uart00);
    else if (uart_name == AB_UART01)
        return bufferSize(isOut, &uart01);
    else if (uart_name == AB_UART02)
        return bufferSize(isOut, &uart02);
    else if (uart_name == AB_UART03)
        return bufferSize(isOut, &uart03);

    //It should never arrive here
    return 0;
}

/*
 * Returns the number of bytes
 * This function must not be called outside of this file (it is not available
 * in the abacus_uart.h file.
 */
int uart_count_caracters(volatile struct Port *uart)
{
    if (uart->status != UARTSTATUS_OPENED)
        return -1;

    //char str[100];
    //sprintf(str, "Buffer end: %d, Buffer start: %d\r\n", uart->bufferInEnd, uart->bufferInStart);
    //abacus_uart_print(AB_UART01, str);

    return bufferSize(0, uart);
}

/*
 * Returns the number of available bytes in the buffer
 */
int8_t abacus_uart_available(uint8_t uart_name)
{
    if (uart_name == AB_UART00)
        return uart_count_caracters(&uart00);
    else if (uart_name == AB_UART01)
        return uart_count_caracters(&uart01);
    else if (uart_name == AB_UART02)
        return uart_count_caracters(&uart02);
    else if (uart_name == AB_UART03)
        return uart_count_caracters(&uart03);

    //It will never arrive here
    return -1;
}

/*
 * Returns the last byte in buffer
 * This function must not be called outside of this file (it is not available
 * in the abacus_uart.h file.
 */
uint8_t uart_read_byte(volatile struct Port *uart)
{
    if (uart->status != UARTSTATUS_OPENED)
        return 0;

    //Buffer was empty. Send last byte as safe
    if (uart->bufferInEnd == uart->bufferInStart)
        return uart->bufferIn[uart->bufferInStart];

    uint8_t byteToSent;
    byteToSent = uart->bufferIn[uart->bufferInStart];

    //Increment buffer pointer
    uart->bufferInStart++;

    //Sanity check
    if (uart->bufferInStart >= UARTBUFFERLENGHT)
        uart->bufferInStart = 0;

    return byteToSent;
}

/*
 * Generic call for uart_read_byte
 */
uint8_t abacus_uart_read(uint8_t uart_name)
{
    if (uart_name == AB_UART00)
        return uart_read_byte(&uart00);
    else if (uart_name == AB_UART01)
        return uart_read_byte(&uart01);
    else if (uart_name == AB_UART02)
        return uart_read_byte(&uart02);
    else if (uart_name == AB_UART03)
        return uart_read_byte(&uart03);

    //It should never arrive here
    return 0;
}

/*
 * This function will try to send one byte if
 * buffer out is empty. It is useful for when TX
 * interrupt is lost due to busy code
 */
/*
 void uart_safe_tx_byte(struct Port *uart)
 {
 if(uart->uart_name == AB_UART00)
 {
 if((UCA0IFG & !UCTXIFG))
 return;

 //Actually everything was already sent!
 if(uart->bufferOutEnd == uart->bufferOutStart)
 return;

 //We have to send the next byte in the buffer
 UCA0TXBUF = uart->bufferOut[uart->bufferOutStart];

 uart->bufferOutStart++;
 if(uart->bufferOutStart >= UARTBUFFERLENGHT)
 uart->bufferOutStart = 0;

 //Reenable interrupts
 UCA0IE |= UCTXIE;
 }
 else if(uart->uart_name == AB_UART01)
 {
 if((UCA1IFG & !UCTXIFG))
 return;

 //Actually everything was already sent!
 if(uart->bufferOutEnd == uart->bufferOutStart)
 return;

 //We have to send the next byte in the buffer
 UCA1TXBUF = uart->bufferOut[uart->bufferOutStart];

 uart->bufferOutStart++;
 if(uart->bufferOutStart >= UARTBUFFERLENGHT)
 uart->bufferOutStart = 0;

 //Reenable interrupts
 UCA1IE |= UCTXIE;
 }
 else if(uart->uart_name == AB_UART02)
 {
 if((UCA2IFG & !UCTXIFG))
 return;

 //Actually everything was already sent!
 if(uart->bufferOutEnd == uart->bufferOutStart)
 return;

 //We have to send the next byte in the buffer
 UCA2TXBUF = uart->bufferOut[uart->bufferOutStart];

 uart->bufferOutStart++;
 if(uart->bufferOutStart >= UARTBUFFERLENGHT)
 uart->bufferOutStart = 0;

 //Reenable interrupts
 UCA2IE |= UCTXIE;
 }
 else if(uart->uart_name == AB_UART03)
 {
 if((UCA3IFG & !UCTXIFG))
 return;

 //Actually everything was already sent!
 if(uart->bufferOutEnd == uart->bufferOutStart)
 return;

 //We have to send the next byte in the buffer
 UCA3TXBUF = uart->bufferOut[uart->bufferOutStart];

 uart->bufferOutStart++;
 if(uart->bufferOutStart >= UARTBUFFERLENGHT)
 uart->bufferOutStart = 0;

 //Reenable interrupts
 UCA3IE |= UCTXIE;
 }
 }
 */

void uart_safe_tx_byte(volatile struct Port *uart)
{
    if (uart->interruptWhenNoData == 0)
        return;

    uart->interruptWhenNoData = 0;
    //Watch out, is not that the buffer is now empty, isn't it?
    if (uart->bufferOutEnd == uart->bufferOutStart)
        return;

    uint8_t byte2write = uart->bufferOut[uart->bufferOutStart];
    uart->bufferOutStart++;
    if (uart->bufferOutStart >= UARTBUFFERLENGHT)
        uart->bufferOutStart = 0;

    //We need to refill the buffer
    if (uart->uart_name == AB_UART00)
    {
        while (UCA0STAT & UCBUSY)
            ;
        UCA0TXBUF = byte2write;
        //Reenable interrupt
        UCA0IE |= UCTXIE;
    }
    else if (uart->uart_name == AB_UART01)
    {
        while (UCA1STAT & UCBUSY)
            ;
        UCA1TXBUF = byte2write;
        //Reenable interrupt
        UCA1IE |= UCTXIE;
    }
    else if (uart->uart_name == AB_UART02)
    {
        while (UCA2STAT & UCBUSY)
            ;
        UCA2TXBUF = byte2write;
        //Reenable interrupt
        UCA2IE |= UCTXIE;
    }
    else if (uart->uart_name == AB_UART03)
    {
        while (UCA3STAT & UCBUSY)
            ;
        UCA3TXBUF = byte2write;
        //Reenable interrupt
        UCA3IE |= UCTXIE;
    }
}

/*
 * This function sends a buffer to the selected port.
 * This function must not be called outside of this file (it is not available
 * in the abacus_uart.h file.
 */
int8_t uart_write(volatile struct Port *uart, uint8_t *buffer, uint16_t lenght)
{
    //Copy next byte to buffer
    unsigned int position = 0;
    if (bufferSize(1, uart) == 0)
    {
        if (uart->uart_name == AB_UART00)
        {
            while (UCA0STAT & UCBUSY)
                ;	//Wait previous TX
            UCA0TXBUF = *buffer;
        }
        else if (uart->uart_name == AB_UART01)
        {
            while (UCA1STAT & UCBUSY)
                ;	//Wait previous TX
            UCA1TXBUF = *buffer;
        }
        else if (uart->uart_name == AB_UART02)
        {
            while (UCA2STAT & UCBUSY)
                ;	//Wait previous TX
            UCA2TXBUF = *buffer;
        }
        else if (uart->uart_name == AB_UART03)
        {
            while (UCA3STAT & UCBUSY)
                ;	//Wait previous TX
            UCA3TXBUF = *buffer;
        }
        buffer++;
        position++;
    }

    while (position != lenght)
    {
        int wait = 1000;
        //Block code to avoid buffer overflow
        //TODO, we need to add a failsafe to overwrite this.
        while (bufferSize(1, uart) == (UARTBUFFERLENGHT - 1))
        {
            //It can happen that with multiple interrupts,
            //TX is not triggered so we have to force the TX...
            if (uart->interruptWhenNoData == 1)
                uart_safe_tx_byte(uart);

            wait--;
            if (wait == 0)
                break;
        }

        //Ok there is space in the buffer, lets go:
        uart->bufferOut[uart->bufferOutEnd] = *buffer;
        position++;

        uart->bufferOutEnd++;
        if (uart->bufferOutEnd >= UARTBUFFERLENGHT)
            uart->bufferOutEnd = 0;

        if (position >= lenght)
            break;

        //Do it as the last thing to avoid buffer overflow
        buffer++;
    }

    //It can happen that the interrupt started while buffer was still empty
    //so now that has been filled, lets trigger it again:
    if (uart->interruptWhenNoData == 1)
        uart_safe_tx_byte(uart);

    return 0;
}

/*
 * Writes a buffer to the port. This is a blocking function if data to send is
 * longer than the buffer length!
 */
int8_t abacus_uart_write(uint8_t uart_name, uint8_t *buffer, uint16_t lenght)
{
    if (uart_name == AB_UART00)
    {
        uart00.noDisableTXInterrupt = 1;
        //Enable TX interrupt
        UCA0IE |= UCTXIE;
        uart_write(&uart00, buffer, lenght);
        uart00.noDisableTXInterrupt = 0;
    }
    else if (uart_name == AB_UART01)
    {
        uart01.noDisableTXInterrupt = 1;
        //Enable TX interrupt
        UCA1IE |= UCTXIE;
        uart_write(&uart01, buffer, lenght);
        uart01.noDisableTXInterrupt = 0;
    }
    else if (uart_name == AB_UART02)
    {
        uart02.noDisableTXInterrupt = 1;
        //Enable TX interrupt
        UCA2IE |= UCTXIE;
        uart_write(&uart02, buffer, lenght);
        uart02.noDisableTXInterrupt = 0;
    }
    else if (uart_name == AB_UART03)
    {
        uart03.noDisableTXInterrupt = 1;
        //Enable TX interrupt
        UCA3IE |= UCTXIE;
        uart_write(&uart03, buffer, lenght);
        uart03.noDisableTXInterrupt = 0;
    }

    return 0;
}

/*
 * Writes a buffer char in the port
 */
int8_t abacus_uart_print(uint8_t uart_name, char *buffer)
{
    //Search for the \0 in the buffer
    unsigned int i = 0;
    while (buffer[i] != '\0')
        i++;
    return abacus_uart_write(uart_name, (uint8_t*) buffer, i);
}

/*
 * Writes an integer in the port
 */
int8_t abacus_uart_print_int(uint8_t uart_name, int16_t number)
{
    char buffer[10];
    sprintf(buffer, "%d", number);
    return abacus_uart_print(uart_name, buffer);
}

/*
 * Writes an unsigned integer in the port
 */
int8_t abacus_uart_print_uint(uint8_t uart_name, uint16_t number)
{
    char buffer[10];
    sprintf(buffer, "%d%d", (number / 10), (number % 10));
    return abacus_uart_print(uart_name, buffer);
}

/*
 * Writes the value to the uart in hex
 */
int8_t abacus_uart_print_uint8_hex(uint8_t uart_name, uint8_t number)
{
    char buffer[5];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[4] = '\0';
    if (number < 10)
    {
        buffer[2] = '0';
        buffer[3] = number + 48;
    }
    else if (number < 16)
    {
        buffer[2] = '0';
        buffer[3] = number + 55; //-10 +65;
    }
    else
    {
        buffer[2] = number >> 4;
        if (buffer[2] < 10)
            buffer[2] += 48;
        else
            buffer[2] += 55;

        buffer[3] = number & 0x0F;
        if (buffer[3] < 10)
            buffer[3] += 48;
        else
            buffer[3] += 55;

    }

    return abacus_uart_print(uart_name, buffer);
}

/*
 * Writes a float in the port
 */
/*int8_t abacus_uart_print_float(int uart_name, float number)
 {
 char buffer[15];
 sprintf(buffer, "%d.%d", (int)number, ((int)(number*100.0)%100));
 return abacus_uart_print(uart_name, buffer);
 }*/

/*
 * Writes a unsigned long in the port
 */
int8_t abacus_uart_print_ulong(uint8_t uart_name, uint32_t number)
{
    char buffer[15];
    int length = 13;
//	sprintf(buffer, "%d%d%d",
//			(number/100000000UL),
//			(number%100000000UL)/10000UL,
//			((number%100000000UL)%10000UL));
    do
    {
        buffer[length--] = (number % 10) + 48;
        number = number / 10;
    }
    while (number);
    buffer[14]='\0';
    return abacus_uart_print(uart_name, &buffer[length+1]);
    //return abacus_uart_write(uart_name,(uint8_t*) buffer[length+1],14-length);
}

/*
 * It blocks the execution of the program until UART is empty
 */
int8_t abacus_uart_waitUntilTxFinished(uint8_t uart_name)
{
    while (abacus_uart_outputBufferCount(uart_name, 1) != 0)
        ;
    //Wait for last TX
    __delay_cycles(500);

    //Faster way, just to wait until interrupts are disabled ;)

    /*if(uart_name == AB_UART00)
     while(UCA0IE & UCTXIE);
     else if(uart_name == AB_UART01)
     while(UCA1IE & UCTXIE);
     else if(uart_name == AB_UART02)
     while(UCA2IE & UCTXIE);
     else if(uart_name == AB_UART03)
     while(UCA3IE & UCTXIE);*/

    return 0;
}

//////////////////////////////////////////////////INTERRUPT HANDLERS///////////

uint8_t uart_RX_next_byte(volatile struct Port *uart)
{
    //Add next character to circular buffer
    if (uart->uart_name == AB_UART00)
        uart->bufferIn[uart->bufferInEnd] = UCA0RXBUF;
    else if (uart->uart_name == AB_UART01)
        uart->bufferIn[uart->bufferInEnd] = UCA1RXBUF;
    else if (uart->uart_name == AB_UART02)
        uart->bufferIn[uart->bufferInEnd] = UCA2RXBUF;
    else if (uart->uart_name == AB_UART03)
        uart->bufferIn[uart->bufferInEnd] = UCA3RXBUF;

    uart->bufferInEnd++;
    if (uart->bufferInEnd >= UARTBUFFERLENGHT)
        uart->bufferInEnd = 0;

    //Buffer overflow? delete last character
    if (uart->bufferInEnd == uart->bufferInStart)
        uart->bufferInStart++;

    if (uart->bufferInEnd >= UARTBUFFERLENGHT)
        uart->bufferInEnd = 0;

    if (uart->bufferInStart >= UARTBUFFERLENGHT)
        uart->bufferInStart = 0;

    uint8_t wakeupOnExit = 0;

    //Call trigger function if enabled
    if (uart->triggerEnabled == 1)
        (*(uart->triggerFunction))(&wakeupOnExit);

    // Enable USCI_AX RX interrupt
    if (uart->uart_name == AB_UART00)
        UCA0IE |= UCRXIE;
    else if (uart->uart_name == AB_UART01)
        UCA1IE |= UCRXIE;
    else if (uart->uart_name == AB_UART02)
        UCA2IE |= UCRXIE;
    else if (uart->uart_name == AB_UART03)
        UCA3IE |= UCRXIE;

    return wakeupOnExit;
}

/*
 *  UART00 HANDLER
 */
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    int code_event = __even_in_range(UCA0IV, 4);
    //Interrupts only when receiving or tx
    if (code_event != 2)
        if (code_event != 4)
            return;

    if (code_event == 2)
    {

        if (uart_RX_next_byte(&uart00) == 1)
        {
            //The user wants to exit LPM
            switch (abacusLPMStatus)
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
        return;
    }
    //We are in Event == 4, TX
    if (uart00.bufferOutEnd == uart00.bufferOutStart)
    {
        //Finish sending, disable TX interrupt and bye bye
        if (uart00.noDisableTXInterrupt == 0)
            UCA0IE &= ~UCTXIE;
        else
        {
            uart00.interruptWhenNoData = 1;
            UCA0IE &= ~UCTXIE;
        }
        return;
    }

    //We have to send the next byte in the buffer
    UCA0TXBUF = uart00.bufferOut[uart00.bufferOutStart];

    uart00.bufferOutStart++;
    if (uart00.bufferOutStart >= UARTBUFFERLENGHT)
        uart00.bufferOutStart = 0;

    uart00.interruptWhenNoData = 0;
}

/*
 *  UART01 HANDLER
 */
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    int code_event = __even_in_range(UCA1IV, 4);
    //Interrupts only when receiving or tx
    if (code_event != 2)
        if (code_event != 4)
            return;

    if (code_event == 2)
    {
        if (uart_RX_next_byte(&uart01) == 1)
        {
            //The user wants to exit LPM
            switch (abacusLPMStatus)
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
        return;
    }

    //We are in Event == 4, this is TX
    if (uart01.bufferOutEnd == uart01.bufferOutStart)
    {
        //Finish sending, disable TX interrupt and bye bye
        if (uart01.noDisableTXInterrupt == 0)
            UCA1IE &= ~UCTXIE;
        else
        {
            uart01.interruptWhenNoData = 1;
            UCA1IE &= ~UCTXIE;
        }
        return;
    }

    //We have to send the next byte in the buffer
    UCA1TXBUF = uart01.bufferOut[uart01.bufferOutStart];

    uart01.bufferOutStart++;
    if (uart01.bufferOutStart >= UARTBUFFERLENGHT)
        uart01.bufferOutStart = 0;

    uart01.interruptWhenNoData = 0;
}

/*
 *  UART02 HANDLER
 */
#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void)
{
    int code_event = __even_in_range(UCA2IV, 4);
    //Interrupts only when receiving or tx
    if (code_event != 2)
        if (code_event != 4)
            return;

    if (code_event == 2)
    {
        if (uart_RX_next_byte(&uart02) == 1)
        {
            //The user wants to exit LPM
            switch (abacusLPMStatus)
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
        return;
    }

    //We are in Event == 4, this is tx
    if (uart02.bufferOutEnd == uart02.bufferOutStart)
    {
        //Finish sending, disable TX interrupt and bye bye
        if (uart02.noDisableTXInterrupt == 0)
            UCA2IE &= ~UCTXIE;
        else
        {
            uart02.interruptWhenNoData = 1;
            UCA2IE &= ~UCTXIE;
        }
        return;
    }

    //We have to send the next byte in the buffer
    UCA2TXBUF = uart02.bufferOut[uart02.bufferOutStart];

    uart02.bufferOutStart++;
    if (uart02.bufferOutStart >= UARTBUFFERLENGHT)
        uart02.bufferOutStart = 0;

    uart02.interruptWhenNoData = 0;
}

/*
 *  UART03 HANDLER
 */
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
    int code_event = __even_in_range(UCA3IV, 4);
    //Interrupts only when receiving or transmitting
    if (code_event != 2)
        if (code_event != 4)
            return;

    if (code_event == 2)
    {
        if (uart_RX_next_byte(&uart03) == 1)
        {
            //The user wants to exit LPM
            switch (abacusLPMStatus)
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
        return;
    }

    //We are in Event == 4, this is tx
    if (uart03.bufferOutEnd == uart03.bufferOutStart)
    {
        //Finish sending, disable TX interrupt and bye bye
        if (uart03.noDisableTXInterrupt == 0)
            UCA3IE &= ~UCTXIE;
        else
        {
            uart03.interruptWhenNoData = 1;
            UCA3IE &= ~UCTXIE;
        }
        return;
    }

    //We have to send the next byte in the buffer
    UCA3TXBUF = uart03.bufferOut[uart03.bufferOutStart];

    uart03.bufferOutStart++;
    if (uart03.bufferOutStart >= UARTBUFFERLENGHT)
        uart03.bufferOutStart = 0;

    uart03.interruptWhenNoData = 0;
}
