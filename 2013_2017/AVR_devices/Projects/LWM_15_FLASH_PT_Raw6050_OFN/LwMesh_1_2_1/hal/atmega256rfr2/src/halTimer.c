/**
 * \file halTimer.c
 *
 * \brief ATmega256rfr2 timer implementation
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
 * $Id: halTimer.c 9267 2014-03-18 21:46:19Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "hal.h"
#include "halTimer.h"

/*- Definitions ------------------------------------------------------------*/
#define TIMER_PRESCALER     8

/*- Variables --------------------------------------------------------------*/
volatile uint8_t halTimerIrqCount;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerInit(void)
{
  halTimerIrqCount = 0;
  //Timer2 250ms
  ////Disable timer2 interrupts
  //TIMSK2  = 0;
  ////use inner clock
  //ASSR =1<<AS2;
  ////set initial counter value
  //TCNT2=0;
  ////set prescaller 1024
  //TCCR2B = 0x03; //1/32768*256*8=15.625 each timer has action on rtx
  //while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB)));
  ////clear interrupt flags
  //TIFR2  = (1<<TOV2);
  ////enable TOV2 interrupt
  //TIMSK2  = (1<<TOIE2);
  
  //Timer 2 CTC mode 10ms
  OCR2A=164; //1/32768*1*164=5.0049ms, good enough 20ms
  ASSR =1<<AS2;//clock source
  while (ASSR & (1<<TCR2BUB));
  TCCR2A|=1<<WGM21;//CTC mode
  TCCR2B = 0x01;//prescaler is 1
  TIMSK2|=(1<<OCIE2A);//enable
  

  //OCR4A = ((F_CPU /1000ul) / TIMER_PRESCALER) * HAL_TIMER_INTERVAL;
  //TCCR4B = (1 << WGM12);              // CTC mode
  //TCCR4B |= (1 << CS11);              // Prescaler 8
  //TIMSK4 |= (1 << OCIE4A);            // Enable TC4 interrupt
}

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerDelay(uint16_t us)// delay function not used, so needs no correctioin
{
  PRAGMA(diag_suppress=Pa082);

  OCR4B = TCNT4 + us;
  if (OCR4B > OCR4A)
    OCR4B -= OCR4A;

  TIFR4 = (1 << OCF4B);
  while (0 == (TIFR4 & (1 << OCF4B)));

  PRAGMA(diag_default=Pa082);
}

/*************************************************************************//**
*****************************************************************************/
ISR(TIMER2_COMPA_vect)//TIMER2_OVF_vect TIMER4_COMPA_vect
{
  halTimerIrqCount++;
}
