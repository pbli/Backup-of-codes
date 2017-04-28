/**
 * \file template.c
 *
 * \brief Empty application template
 *
 * Copyright (C) 2012-2013, Atmel Corporation. All rights reserved.
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
 * $Id: template.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*- Definitions ------------------------------------------------------------*/
static uint8_t message;
static NWK_DataReq_t nwkDataReq;


/*- Types ------------------------------------------------------------------*/
// Put your type definitions here

/*- Prototypes -------------------------------------------------------------*/
// Put your function prototypes here

/*- Variables --------------------------------------------------------------*/
// Put your variables here

/*- Implementations --------------------------------------------------------*/

/*static void appDataConf(NWK_DataReq_t *req)
{
	if (NWK_SUCCESS_STATUS == req->status)
		// frame successfully sent
	else
		// not success
}*/

static void sendFrame(void)
{
	nwkDataReq.dstAddr = 0;
	nwkDataReq.dstEndpoint = 1;
	nwkDataReq.srcEndpoint = 5;
	nwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
	nwkDataReq.data = &message;
	nwkDataReq.size = sizeof(message);
	//nwkDataReq.confirm = appDataConf;
	NWK_DataReq(&nwkDataReq);
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
  
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
 // uint8_t i;
  uint8_t j;
    
  SYS_Init();

  while (1)
  {   
    
	SYS_TaskHandler();
	for (j=0; j<10000; j++)
	{

		message = j;
		
		sendFrame();
	}
	
	
	
	//APP_TaskHandler();
  }
}
