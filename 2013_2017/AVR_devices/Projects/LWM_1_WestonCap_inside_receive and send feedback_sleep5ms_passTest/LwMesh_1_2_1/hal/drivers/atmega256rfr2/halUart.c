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





/*- Types ------------------------------------------------------------------*/


/*- Variables --------------------------------------------------------------*/

static uint8_t txData[200+1];
static uint8_t rxData[200+1];

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**/
uint8_t conv_digit_to_ascii(uint32_t val) {
	if (val<= 0x09) {
		val = val + 0x30;
	}
	if ((val >= 0x0A) && (val<= 0x0F)) {
		val = val + 0x37;
	}
	return (uint8_t)val;
}
void convert_result_to_ASCII(uint32_t val) {
	uint32_t x;
	
	x = val / 1000000000;
	if (x) {
		result[0] = conv_digit_to_ascii(x);
		}else{
		result[0] = ' ';
	}
	val = val - x*1000000000;
	
	x = val / 100000000;
	if (x || result[0]!=' ') {
		result[1] = conv_digit_to_ascii(x);
		}else{
		result[1] = ' ';
	}
	val = val - x*100000000;
	
	x = val / 10000000;
	if (x || result[1]!=' ' || result[0]!=' ') {
		result[2] = conv_digit_to_ascii(x);
		}else{
		result[2] = ' ';
	}
	val = val - x*10000000;
	
	x = val / 1000000;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' ') {
		result[3] = conv_digit_to_ascii(x);
		}else{
		result[3] = ' ';
	}
	val = val - x*1000000;
	
	x = val / 100000;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' '|| result[3]!=' ') {
		result[4] = conv_digit_to_ascii(x);
		}else{
		result[4] = ' ';
	}
	val = val - x*100000;
	
	x = val / 10000;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' '|| result[3]!=' '|| result[4]!=' ') {
		result[5] = conv_digit_to_ascii(x);
		}else{
		result[5] = ' ';
	}
	val = val - x*10000;
	
	x = val / 1000;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' '|| result[3]!=' '|| result[4]!=' '|| result[5]!=' ') {
		result[6] = conv_digit_to_ascii(x);
		}else{
		result[6] = ' ';
	}
	val = val - x*1000;
	
	x = val / 100;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' '|| result[3]!=' '|| result[4]!=' '|| result[5]!=' '|| result[6]!=' ') {
		result[7] = conv_digit_to_ascii(x);
		}else{
		result[7] = ' ';
	}
	val = val - x*100;
	
	x = val / 10;
	if (x || result[2]!=' ' || result[1]!=' ' || result[0]!=' '|| result[3]!=' '|| result[4]!=' '|| result[5]!=' '|| result[6]!=' '|| result[7]!=' ') {
		result[8] = conv_digit_to_ascii(x);
		}else{
		result[8] = ' ';
	}
	val = val - x*10;
	
	result[9] = conv_digit_to_ascii(val);
}
/*****************************************************************************/
void convert_write_data(uint32_t data){
	convert_result_to_ASCII(data);// the result is stored in result[10]
	for (int i=0;i<10; i++)
	{
		if (txFifo.bytes == txFifo.size)
		return;
		if (result[i]!=' ') // space+space will not add
		{
			txFifo.data[txFifo.tail++] = result[i];
			if (txFifo.tail == txFifo.size)
			txFifo.tail = 0;
			txFifo.bytes++;
			newData = true;
		}		
	}
	txFifo.data[txFifo.tail++] = ' ';//add space at end
	if (txFifo.tail == txFifo.size)
	txFifo.tail = 0;
	txFifo.bytes++;
}
// This function is used to convert the character to its ASCII value.



void HAL_UartInit(uint32_t baudrate)
{

  txFifo.data = txData;
  txFifo.size = 200;
  txFifo.bytes = 0;
  txFifo.head = 0;
  txFifo.tail = 0;

  rxFifo.data = rxData;
  rxFifo.size = 200;
  rxFifo.bytes = 0;
  rxFifo.head = 0;
  rxFifo.tail = 0;

  newData = false;


}



/*************************************************************************//**
*****************************************************************************/
void HAL_UartWriteByte(uint8_t byte)//get data from outside and write to rxfifo
{
  if (rxFifo.bytes == rxFifo.size)
    return;

  rxFifo.data[rxFifo.tail++] = byte;
  if (rxFifo.tail == rxFifo.size)
    rxFifo.tail = 0;
  rxFifo.bytes++;
}

/*************************************************************************//**
*****************************************************************************/
uint8_t HAL_UartReadByte(void)//get the data need send out from rxfifo
{
  uint8_t byte;

  PRAGMA(diag_suppress=Pa082);
  ATOMIC_SECTION_ENTER
    byte = txFifo.data[txFifo.head++];
    if (txFifo.head == txFifo.size)
      txFifo.head = 0;
    txFifo.bytes--;
  ATOMIC_SECTION_LEAVE
  PRAGMA(diag_default=Pa082);

  return byte;
}




/*************************************************************************//**
*****************************************************************************/
void HAL_UartTaskHandler(void)
{
  if (rxFifo.bytes)// receive data from outside
  {
   HAL_GPIO_LED_toggle(); //light led to indicate activation
   convert_write_data(987654321);//just positive number
  }

  {
    uint16_t bytes;
    bool new;

    ATOMIC_SECTION_ENTER
      new = newData;
      newData = false;
      bytes = txFifo.bytes;
    ATOMIC_SECTION_LEAVE

    if (new)
      HAL_UartBytesReceived(bytes);
  }
}
