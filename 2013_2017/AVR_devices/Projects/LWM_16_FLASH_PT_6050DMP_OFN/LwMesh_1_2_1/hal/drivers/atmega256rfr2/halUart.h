/**
 * \file halUart.h
 *
 * \brief ATmega256rfr2 UART interface
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
 * $Id: halUart.h 9267 2014-03-18 21:46:19Z ataradov $
 *
 */

#ifndef _HAL_UART_H_
#define _HAL_UART_H_

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <sysConfig.h>
#include "I2C_master.h"
#include "spi.h"


/*- Prototypes -------------------------------------------------------------*/
void HAL_UartInit(void);
uint8_t HAL_UartReadByte(void);
void HAL_UartBytesReceived(uint16_t bytes);
void HAL_UartTaskHandler(void);


#endif // _HAL_UART_H_
typedef struct//move rx so can writeto rxbuffer
{
	uint16_t  head;
	uint16_t  tail;
	uint16_t  size;
	uint16_t  bytes;
	uint8_t   *data;
} FifoBuffer_t;

static volatile FifoBuffer_t rxFifo;

static  volatile bool newData;
uint32_t C[7];//p1 prom
uint32_t CC[7];//p2 prom

uint32_t TimerFlag;


char result[11];//converted digit of each number, at most 3 digits
uint8_t timerMark;
uint8_t  pageNmH;
uint8_t  pageNmL;
uint8_t  pageWNm;



void Sensor_ini(void);
void OFN_ini(void);
void Read_OFN(void);

void PSensor_init(void);

void sendData(int32_t Data);
void AddDat2Buf(uint8_t Income);
void sendCachedFlashdata(void);
void Store2Fl(void);
void ReadSendFl(void);
void FlashIni(void);




