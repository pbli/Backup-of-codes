/**
 * \file Peer2Peer.c
 *
 * \brief Peer2Peer application implementation
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
 * $Id: Peer2Peer.c 9267 2014-03-18 21:46:19Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"

#ifndef F_CPU
#define F_CPU 8000000ul
#endif



/*- Definitions ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)//define of buffer, if security enable, the size decrease
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t//define these flags
{
  APP_STATE_INITIAL,
  APP_STATE_IDLE,
} AppState_t;

/*- Prototypes -------------------------------------------------------------*/
static void appSendData(void);

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;
static uint8_t SleepFlag=0;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
  appDataReqBusy = false;
  (void)req;
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void)
{
	if (SleepFlag==1)
	{
		NWK_WakeupReq();
		SleepFlag=0;
	}
  if (appDataReqBusy || 0 == appUartBufferPtr)//in req or Ptr is 0(no data)
    return;
  
  memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);//destin, source and num

  appDataReq.dstAddr = 1-APP_ADDR;
  appDataReq.dstEndpoint = APP_ENDPOINT;
  appDataReq.srcEndpoint = APP_ENDPOINT;
  appDataReq.options = NWK_OPT_ENABLE_SECURITY | NWK_IND_OPT_ACK_REQUESTED;
  appDataReq.data = appDataReqBuffer;
  appDataReq.size = appUartBufferPtr;
  appDataReq.confirm = appDataConf;
  NWK_DataReq(&appDataReq);

  appUartBufferPtr = 0;
  appDataReqBusy = true;
}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)//sensor data need to send to rf channel
{
  for (uint16_t i = 0; i < bytes; i++)
  {
    uint8_t byte = HAL_UartReadByte();//read data from rxbuffer, data from sensor
    if (appUartBufferPtr == sizeof(appUartBuffer))//if full, send, if not keep stack
      appSendData();//redundancy

    if (appUartBufferPtr < sizeof(appUartBuffer))
      appUartBuffer[appUartBufferPtr++] = byte;
  }
  SYS_TimerStop(&appTimer);
  SYS_TimerStart(&appTimer);
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer)//after 20ms, send, duiring this 20ms, cpu sleep
{
  appSendData();
  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind)//data from rf channel,not use
{
  for (uint8_t i = 0; i < ind->size; i++)
  ;
  return true;
}

static void appInit(void)
{
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
  PHY_SetBand(APP_BAND);
  PHY_SetModulation(APP_MODULATION);
#endif
  PHY_SetRxState(true);

  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

  HAL_BoardInit();

  appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
  appTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appTimer.handler = appTimerHandler;

}

static void APP_TaskHandler(void)
{
  switch (appState)
  {
    case APP_STATE_INITIAL:
    {
      appInit();
      appState = APP_STATE_IDLE;
    } break;

    case APP_STATE_IDLE:
      break;

    default:
      break;
  }
}

void powercontrol(void)
{
	 power_all_disable();
	 set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	 //power_adc_enable();
	 //power_usart0_enable();
	 //power_spi_enable();
	 power_timer2_enable();
	 power_twi_enable();
	 //power_pga_enable();
	 //power_timer0_enable();
	 //power_timer1_enable();
	 //power_usart1_enable();
	 //power_timer3_enable();
	 //power_timer4_enable();
	 //power_timer5_enable();
	 power_transceiver_enable();
	 power_ram0_enable();
	 power_ram1_enable();
	 power_ram2_enable();
	 power_ram3_enable();
}

void sleep5ms(void)
{
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
	sei();
}

int main()
{
  powercontrol();
  SYS_Init();//hardware, physical layer, nwk init
  HAL_UartInit();//sensor initial
  
  
  while (1)
  {	  
      
	  SYS_TaskHandler();//get data in timer handler if reach sampling time
	  HAL_UartTaskHandler();//store data and send
	  APP_TaskHandler();	
	    
	  if (!NWK_Busy())//sending finish, sleep transceiver and cpu
	  {	  
		if (SleepFlag==0)
		{
			NWK_SleepReq();
			SleepFlag=1;
		}		
		//sleep5ms();
	 }      	
  }
}

