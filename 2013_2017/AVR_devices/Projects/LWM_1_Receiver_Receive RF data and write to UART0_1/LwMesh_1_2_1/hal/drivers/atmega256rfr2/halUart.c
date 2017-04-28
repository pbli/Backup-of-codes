/**
 * \file halUart.c
 *
 * \brief ATmega256rfr2 UART implementation
 *
 * Copyright (C) 2012-2014, Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * Modification and other use of this code is subject to Atmel's Limited
 * License Agreement (license.txt).
 *
 * $Id: halUart.c 9267 2014-03-18 21:46:19Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdbool.h>
#include "hal.h"
#include "halUart.h"
#include "config.h"
#include "sysTimer.h"
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>


/*- Definitions ------------------------------------------------------------*/
#ifndef HAL_UART_TX_FIFO_SIZE
#define HAL_UART_TX_FIFO_SIZE  10
#endif

#ifndef HAL_UART_RX_FIFO_SIZE
#define HAL_UART_RX_FIFO_SIZE  10
#endif

#if HAL_UART_CHANNEL == 0
  #define UBRRxH            UBRR0H
  #define UBRRxL            UBRR0L
  #define UCSRxA            UCSR0A
  #define UCSRxB            UCSR0B
  #define UCSRxC            UCSR0C
  #define UDRx              UDR0
  #define USARTx_UDRE_vect  USART0_UDRE_vect
  #define USARTx_RX_vect    USART0_RX_vect
#elif HAL_UART_CHANNEL == 1
  #define UBRRxH            UBRR1H
  #define UBRRxL            UBRR1L
  #define UCSRxA            UCSR1A
  #define UCSRxB            UCSR1B
  #define UCSRxC            UCSR1C
  #define UDRx              UDR1
  #define USARTx_UDRE_vect  USART1_UDRE_vect
  #define USARTx_RX_vect    USART1_RX_vect
#else
  #error Unsupported UART channel
#endif


static uint8_t txData[HAL_UART_TX_FIFO_SIZE+1];
static SYS_Timer_t hrtbtTimer;



 

 // This function is used to convert the character to its ASCII value.


static void hrtbtTimerHandler(SYS_Timer_t *timer)//read I2C data and write to buffer
{
	HAL_GPIO_LED_toggle(); 
	_delay_ms(100); 
	HAL_GPIO_LED_toggle(); //no this line, can not 
}

void HAL_UartInit(uint32_t baudrate)
{
  uint32_t brr = ((uint32_t)F_CPU * 2) / (16 * baudrate) - 1;//Fosc=2*Fcpu

  UBRRxH = (brr >> 8) & 0xff;
  UBRRxL = (brr & 0xff);
  UCSRxA = (1 << U2X1);
  UCSRxB = (1 << TXEN1) | (1 << RXEN1) | (1 << RXCIE1);
  UCSRxC = (3 << UCSZ10);

  txFifo.data = txData;
  txFifo.size = HAL_UART_TX_FIFO_SIZE;
  txFifo.bytes = 0;
  txFifo.head = 0;
  txFifo.tail = 0;


  udrEmpty = true;
  hrtbtTimer.interval = 1000;//ms
  hrtbtTimer.mode = SYS_TIMER_PERIODIC_MODE;
  hrtbtTimer.handler = hrtbtTimerHandler;
  SYS_TimerStart(&hrtbtTimer);

}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartWriteByte(uint8_t byte)
{
  if (txFifo.bytes == txFifo.size)//update tx buffer with rf data
    return;

  txFifo.data[txFifo.tail++] = byte;
  if (txFifo.tail == txFifo.size)
    txFifo.tail = 0;
  txFifo.bytes++;
}


/*************************************************************************//**
*****************************************************************************/
ISR(USARTx_UDRE_vect)//end of send txbuffer data to screen
{
  udrEmpty = true;
  UCSRxB &= ~(1 << UDRIE1);
}


/*************************************************************************//**
*****************************************************************************/
void HAL_UartTaskHandler(void)
{
  if (txFifo.bytes && udrEmpty)//have data from rf in txbuffer and no data in UART
  {
    uint8_t byte;

    byte = txFifo.data[txFifo.head++];
    if (txFifo.head == txFifo.size)
      txFifo.head = 0;
    txFifo.bytes--;
	

    ATOMIC_SECTION_ENTER //send to computer
      UDRx = byte;
      UCSRxB |= (1 << UDRIE1);
      udrEmpty = false;
    ATOMIC_SECTION_LEAVE
  }
}
