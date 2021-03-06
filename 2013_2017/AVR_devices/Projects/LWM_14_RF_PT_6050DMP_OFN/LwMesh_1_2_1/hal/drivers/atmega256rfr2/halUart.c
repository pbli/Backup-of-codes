/**
 * \file halUart.c
 *
 * \brief ATmega256rfr2 UART implementation
 a lot modification is made here, almost all my codes is here
 i know there is some professional way....but i am an old man
 *
 * Copyright (C) 2012-2014, Atmel Corporation. All rights reserved.
//- Includes ---------------------------------------------------------------*/



#include <stdbool.h>
#include <avr/io.h>
#include "hal.h"
#include "halUart.h"
#include "config.h"
#include "sysTimer.h"
#include "I2C_master.h"



//build in delay function
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <util/delay.h>
#include <math.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include "mpu.h"


/*- Definitions ------------------------------------------------------------*/
#ifndef HAL_UART_TX_FIFO_SIZE
#define HAL_UART_TX_FIFO_SIZE  10
#endif

#ifndef HAL_UART_RX_FIFO_SIZE
#define HAL_UART_RX_FIFO_SIZE  10
#endif

#ifndef HAL_UART_CHANNEL
#define HAL_UART_CHANNEL       0
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

static uint8_t rxData[HAL_UART_RX_FIFO_SIZE+1];
static SYS_Timer_t Pengbo50msTimer;
static SYS_Timer_t Pengbo15msTimer1;
static SYS_Timer_t Pengbo15msTimer2;
#define addGy 0xD2
//two pressure sensor:1110110X EC, ED 1110111X EE, EF
#define addP1 0xEC//EC
#define addP2 0xEE
#define addOFN 0xAE 
//#define OFN
//#define Psensor
#define cpg 16384 
static void Pengbo50msTimerHandler(SYS_Timer_t *timer)//use 25hz timer to read fifo
{
	sendData(101);

	TimerMarker+=1;//add one in each interrupt
	uint32_t timerrange=1;
	if (TimerMarker==timerrange)//72000 = 1 hour
	{  
		 _i2c_write_byte(addGy,0x6B,0x80);//reset gyro
		_i2c_write_byte(addGy,0x6B,0x00);//wait about 50ms
	}
	else if (TimerMarker==timerrange+1)//start mpu
	{
		mympu_open(10);//10 hz
	}
	else if(TimerMarker==timerrange+2)
	{
		
		
		TimerMarker-=1;
		int ret=mympu_update();	//0 means new package, check package quicker, 20hz
		if (ret==0)
		{
		timerindex+=1;
		}
		if (timerindex==2)// has two new pack, then go and send, 5hz.
		{
			timerindex=0;
			Fst=1;
			for (int i=0;i<4;i++)
			sendData(mympu.q[i]*1000000);//send Q*1000000//not class, it is structure


			//for (int i=0;i<3;i++)
			//sendData(mympu.Drpy[i]);
			
			//for (int i=0;i<3;i++){
			  	//sendData((mympu.rpy[i]*10/10.0f-mympu.Drpy[i]*10/10.0f)*100);
			//}
			//for (int i=0;i<3;i++){
			//sendData(mympu.acclw[i]);
			//}

			#ifdef Psensor
			I2C_start(addP1);  I2C_writebyte(0x58); I2C_stop();//wake up pressure sensors for temperature reading
			I2C_start(addP2);  I2C_writebyte(0x58); I2C_stop();
			#endif
			
			SYS_TimerStop(&Pengbo15msTimer1);//wait 10ms timer
			SYS_TimerStart(&Pengbo15msTimer1);
			
		}
		
	}

	
	
}

static void Pengbo15msTimer1Handler(SYS_Timer_t *timer)
{
#ifdef Psensor
	D12=0;D22=0;tempD=0;
	
	I2C_start(addP1); I2C_writebyte(0x00);	I2C_stop();
	I2C_start(addP1+1); tempD= I2C_read_ack(); D12=tempD;
	tempD=I2C_read_ack(); D12=D12<<8|tempD;
	tempD=I2C_read_nack(); D12=D12<<8|tempD;  I2C_stop();
	
	
	I2C_start(addP2); I2C_writebyte(0x00);	I2C_stop();
	I2C_start(addP2+1); tempD= I2C_read_ack(); D22=tempD;
	tempD=I2C_read_ack(); D22=D22<<8|tempD;
	tempD=I2C_read_nack(); D22=D22<<8|tempD;  I2C_stop();
	
	I2C_start(addP1);  I2C_writebyte(0x48); I2C_stop();//wake up pressure
	I2C_start(addP2);  I2C_writebyte(0x48); I2C_stop();
#endif

	(void)timer;
	
	SYS_TimerStop(&Pengbo15msTimer2);//wait 10ms timer
	SYS_TimerStart(&Pengbo15msTimer2);

}

static void Pengbo15msTimer2Handler(SYS_Timer_t *timer)
{
#ifdef Psensor
	D11=0;D21=0;tempD=0;//read pressure
	I2C_start(addP1);  I2C_writebyte(0x00); I2C_stop();
	I2C_start(addP1+1);  tempD= I2C_read_ack(); D11=tempD;
	tempD=I2C_read_ack();  D11=D11<<8|tempD;
	tempD=I2C_read_nack();  D11=D11<<8|tempD;
	I2C_stop();
	
	I2C_start(addP2);  I2C_writebyte(0x00); I2C_stop();
	I2C_start(addP2+1);  tempD= I2C_read_ack(); D21=tempD;
	tempD=I2C_read_ack();  D21=D21<<8|tempD;
	tempD=I2C_read_nack();  D21=D21<<8|tempD;
	I2C_stop();
	
	
	dT=0;OFF=0;SENS=0;P=0;//calibration of temperature and pressure
	dT   = D12 - ((uint32_t)C[5] << 8);
	OFF  = ((int64_t)C[2] << 18) + (((int64_t)dT * (int64_t)C[4]) >> 5);
	SENS = ((int64_t)C[1] << 17) + (((int64_t)dT * (int64_t)C[3]) >> 7);
	TEMP = (int64_t)dT * (int64_t)C[6] / 8388608 + 2000;//Temperature /0.01
	P    = ((int64_t)D11 * SENS / 2097152 - OFF) / 32768;//P Pa
	sendData(P);
	
	dT=0;OFF=0;SENS=0;P=0;
	dT   = D22 - ((uint32_t)CC[5] << 8);
	OFF  = ((int64_t)CC[2] << 18) + (((int64_t)dT * (int64_t)CC[4]) >> 5);
	SENS = ((int64_t)CC[1] << 17) + (((int64_t)dT * (int64_t)CC[3]) >> 7);
	P  = ((int64_t)D21 * SENS / 2097152 - OFF) / 32768;//P Pa
	sendData(P);
	sendData(TEMP);
	sendData(TEMP);

	
	
#endif

#ifdef OFN
    Read_OFN();
#endif
	(void)timer;
}






void HAL_UartInit(void)
{
  rxFifo.data = rxData;
  rxFifo.size = HAL_UART_RX_FIFO_SIZE;
  rxFifo.bytes = 0;
  rxFifo.head = 0;
  rxFifo.tail = 0;

  newData = false;
  
  Pengbo50msTimer.interval = 50;//0.050049
  Pengbo50msTimer.mode = SYS_TIMER_PERIODIC_MODE;
  Pengbo50msTimer.handler = Pengbo50msTimerHandler;
  
  Pengbo15msTimer1.interval=15;//in this version with DMP, the time for reset needs 15ms
  Pengbo15msTimer1.mode=SYS_TIMER_INTERVAL_MODE;
  Pengbo15msTimer1.handler=Pengbo15msTimer1Handler;
  
  Pengbo15msTimer2.interval=15;
  Pengbo15msTimer2.mode=SYS_TIMER_INTERVAL_MODE;
  Pengbo15msTimer2.handler=Pengbo15msTimer2Handler;
  I2C_init();
  Sensor_ini();  
  SYS_TimerStart(&Pengbo50msTimer);//start timer after finish of initialization  
  TimerMarker=0;
}


//read data from rxbuffer
uint8_t HAL_UartReadByte(void)
{
  uint8_t byte;

  PRAGMA(diag_suppress=Pa082);
  ATOMIC_SECTION_ENTER
    byte = rxFifo.data[rxFifo.head++];//read data from rxbuffer
    if (rxFifo.head == rxFifo.size)
      rxFifo.head = 0;
    rxFifo.bytes--;
  ATOMIC_SECTION_LEAVE
  PRAGMA(diag_default=Pa082);

  return byte;
}

//******************

void Sensor_ini(){
#ifdef Psensor
PSensor_init();
#endif
#ifdef OFN
	OFN_ini();	
#endif
    _i2c_write_byte(addGy,0x6B,0x40);//sleep gyro

}


//***************

//new data put in txbuffer and send if full
void HAL_UartTaskHandler(void)
{
    uint16_t bytes;
    bool new;

    ATOMIC_SECTION_ENTER
      new = newData;
      newData = false;
      bytes = rxFifo.bytes;
    ATOMIC_SECTION_LEAVE

    if (new)
      HAL_UartBytesReceived(bytes);//bytes from sensor
}


// Pressure init
void PSensor_init(void){ 
  
  uint8_t tempD=0;//sensor 1
  I2C_start(addP1);//address***
  I2C_writebyte(0x1E);//reset
  I2C_stop();
  _delay_ms(10);//delay 10+ms for reset
  for (int i=0;i<6;i++)
  {
  I2C_start(addP1);
  I2C_writebyte(0xA2+i*2);
  I2C_stop();
  I2C_start(addP1+1);//***
  C[i+1] = I2C_read_ack();
  tempD=I2C_read_nack();
  I2C_stop();
  C[i+1]=C[i+1]*256+(uint32_t)tempD;
  }  
  
   //sensor 2
  
  I2C_start(addP2);//address***
  I2C_writebyte(0x1E);//reset
  I2C_stop();
  _delay_ms(10);//cpu sleep a while for data ready
  for (int i=0;i<6;i++)
  { 
  I2C_start(addP2);//
  I2C_writebyte(0xA2+i*2);
  I2C_stop();
  I2C_start(addP2+1);//***
  CC[i+1] = I2C_read_ack();
  tempD=I2C_read_nack();
  I2C_stop();
  CC[i+1]=CC[i+1]*256+(uint32_t)tempD;
  }   
}

void OFN_ini(){
	//power on reset, at least 20 us
	uint8_t prodID;
	HAL_GPIO_LED_clr();
	_delay_us(50);
	HAL_GPIO_LED_set();
	prodID=_i2c_read_byte(addOFN,0x00);
	sendData(prodID);
	//disable all properties, just count the distance
	_i2c_write_byte(addOFN,0x60,0x00);
	_i2c_write_byte(addOFN,0x61,0x00);	
}

void Read_OFN(){
		uint8_t flag;
		flag=_i2c_read_byte(addOFN,0x02);
		if (!(flag&0x10))
		{
			OFNvalue[0]=(int8_t)_i2c_read_byte(addOFN,0x03);
			OFNvalue[1]=(int8_t)_i2c_read_byte(addOFN,0x04);
		}
		
		sendData(OFNvalue[0]);
		sendData(OFNvalue[1]);
		
}

//change anything to string and add to rxbuffer, much simple than before
void sendData(int32_t Data){
    //AddDat2Buf(13);//return
	//AddDat2Buf(32);//space
	AddDat2Buf(10);//newline
	if (Fst==1)
	{
		AddDat2Buf(113);
		AddDat2Buf(10);
		Fst=0;
	}
	ltoa(Data,result,10);
	for (int i=0;i<11; i++)
	{	
		if (result[i]!='\0')
		{
			AddDat2Buf(result[i]);	
		}
		else
		break;
		
	}		
 }
 
 void AddDat2Buf(uint8_t Income){//the sampled data is stored in rxfifo

 if (rxFifo.bytes == rxFifo.size)
	 return;
	 rxFifo.data[rxFifo.tail++] = Income;
	 if (rxFifo.tail == rxFifo.size)
	 rxFifo.tail = 0;
	 rxFifo.bytes++;
	 newData = true;
	 
 }



