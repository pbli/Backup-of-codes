/**
 * \file halUart.c
 *
 * \brief ATmega256rfr2 UART implementation
 *
 * Copyright (C) 2012-2014, Atmel Corporation. All rights reserved.
//- Includes ---------------------------------------------------------------*/



#include <stdbool.h>
#include "hal.h"
#include "halUart.h"
#include "config.h"
#include "sysTimer.h"
#include "I2C_master.h"
#include <avr/io.h>
//buildin delay function
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>

#include <math.h>
#include <stdlib.h>
#include <avr/sleep.h>




static uint8_t rxData[201];

static SYS_Timer_t Pengbo250msTimer;
static SYS_Timer_t Pengbo20msTimer;
static SYS_Timer_t Pengbo10msTimer1;
static SYS_Timer_t Pengbo10msTimer2;




//The ad0 is connected to high, so the address is 1101 001X D2 and D3 for new version pcb, but for previous sparkfun is D0 and D1
#define addGy 0xD2
//two pressure sensor:1110110X EC, ED 1110111X EE, EF
#define addP1 0xEC //EC
#define addP2 0xEE
// address of mag sensor12/0C=00001100 in I2C it will be 00011000 or 00011001 =0x18 and 0x19
#define addMg 0x18
//1010 1110 for write, 10101111 for read
#define addOFN 0xAE 
// select the device you are running
//#define Psensor
#define MPU6050
//#define MPU9250
//#define OFN
#define Gyrawake
#define RFPeriod 1000

// resolution of sensor and amplification 
/*
#define gyrRel 500.0/32768.0

#define accRel 2.0/32768.0

#define magRel 0.15


//offsets
#define Gxoffset -62
#define Gyoffset 48
#define Gzoffset 43
#define Axoffset 180
#define Ayoffset 380
#define Azoffset 15700
#define Mxoffset 0
#define Myoffset 0
#define Mzoffset 0
*/

//just get register values
#define gyrRel 1

#define accRel 1

#define magRel 1

#define Gxoffset 0
#define Gyoffset 0
#define Gzoffset 0

#define Axoffset 0
#define Ayoffset 0
#define Azoffset 0

#define Mxoffset 0
#define Myoffset 0
#define Mzoffset 0



  

  /*************************************************************************//**
  *****************************************************************************/
  
//response of the 250mstimer: active gyro
static void Pengbo250msTimerHandler(SYS_Timer_t *timer)
{
	TimerFlag+=1;	
	if (TimerFlag==2)//14400=1 hour
	{
		//First use number to active sleep mode then use real sensor values during program chip
		_i2c_write_byte(addGy,0x6B,0x00);//wake up gyro
		SYS_TimerStop(&Pengbo20msTimer);//wait 20ms timer
		SYS_TimerStart(&Pengbo20msTimer);
		TimerFlag-=1;	
	}



}
//convert temperature
static void Pengbo20msTimerHandler(SYS_Timer_t *timer)
{
#ifdef Psensor
	I2C_start(addP1);  I2C_write(0x58); I2C_stop();//wake up pressure sensors for temperature reading
	I2C_start(addP2);  I2C_write(0x58); I2C_stop();      
#endif
	(void)timer;
	SYS_TimerStop(&Pengbo10msTimer1);//wait 10ms timer
	SYS_TimerStart(&Pengbo10msTimer1);

}

//Get temperature, convert pressure
static void Pengbo10msTimer1Handler(SYS_Timer_t *timer)
{
#ifdef Psensor

	D12=0;D22=0;tempD=0;
	
	I2C_start(addP1); I2C_write(0x00);	I2C_stop();
	I2C_start(addP1+1); tempD= I2C_read_ack(); D12=tempD;
	tempD=I2C_read_ack(); D12=D12<<8|tempD;
	tempD=I2C_read_nack(); D12=D12<<8|tempD;  I2C_stop();
	
	
	I2C_start(addP2); I2C_write(0x00);	I2C_stop();
	I2C_start(addP2+1); tempD= I2C_read_ack(); D22=tempD;
	tempD=I2C_read_ack(); D22=D22<<8|tempD;
	tempD=I2C_read_nack(); D22=D22<<8|tempD;  I2C_stop();
	
	I2C_start(addP1);  I2C_write(0x48); I2C_stop();//wake up pressure
	I2C_start(addP2);  I2C_write(0x48); I2C_stop();

#endif

	(void)timer;
	SYS_TimerStop(&Pengbo10msTimer2);//wait 10ms timer
	SYS_TimerStart(&Pengbo10msTimer2);

}
static void Pengbo10msTimer2Handler(SYS_Timer_t *timer)
{
#ifdef Psensor

	D11=0;D21=0;tempD=0;//read pressure
	I2C_start(addP1);  I2C_write(0x00); I2C_stop();
	I2C_start(addP1+1);  tempD= I2C_read_ack(); D11=tempD;
	tempD=I2C_read_ack();  D11=D11<<8|tempD;
	tempD=I2C_read_nack();  D11=D11<<8|tempD;
	I2C_stop();
	
	I2C_start(addP2);  I2C_write(0x00); I2C_stop();
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

#endif

#ifdef MPU6050
	ReadGyr();
#endif   

#ifdef MPU9250
	readMag(); 
#endif	

#ifdef OFN
    readOFN();
#endif

	(void)timer;

}



void HAL_UartInit(void)
{
	rxFifo.data = rxData;
	rxFifo.size = 200;
	rxFifo.bytes = 0;
	rxFifo.head = 0;
	rxFifo.tail = 0;
	TimerFlag=0;

	newData = false;
  
	Pengbo250msTimer.interval =RFPeriod;//250*10.0098=250.
	Pengbo250msTimer.mode = SYS_TIMER_PERIODIC_MODE;
	Pengbo250msTimer.handler = Pengbo250msTimerHandler;
 
	Pengbo20msTimer.interval=20;
	Pengbo20msTimer.mode=SYS_TIMER_INTERVAL_MODE;
	Pengbo20msTimer.handler=Pengbo20msTimerHandler;
 
	Pengbo10msTimer1.interval=10;
	Pengbo10msTimer1.mode=SYS_TIMER_INTERVAL_MODE;
	Pengbo10msTimer1.handler=Pengbo10msTimer1Handler;
  
	Pengbo10msTimer2.interval=10;
	Pengbo10msTimer2.mode=SYS_TIMER_INTERVAL_MODE;
	Pengbo10msTimer2.handler=Pengbo10msTimer2Handler;
  
	I2C_init();
	SenIni();
	SYS_TimerStart(&Pengbo250msTimer);//start timer after finish of initialization  
}

void SenIni(){
#ifdef Psensor
	 PSensor_init();
#endif
#ifdef MPU6050
     Gyro_init();
#endif
	 
#ifdef MPU9250
     MagIni();// only for
#endif
#ifdef OFN
     OFNIni();
#endif

}

//read data from rx
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



//new data put in RF buffer and send if full
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


// Pressure ini
void PSensor_init(void){ 
  
  uint8_t tempD=0;//sensor 1
  tempD=I2C_start(addP1);//address***
  I2C_write(0x1E);//reset
  I2C_stop();
  _delay_ms(20);//delay 10+ms for reset
  for (int i=0;i<6;i++)
  {
  I2C_start(addP1);
  I2C_write(0xA2+i*2);
  I2C_stop();
  I2C_start(addP1+1);//***
  C[i+1] = I2C_read_ack();
  tempD=I2C_read_nack();
  I2C_stop();
  C[i+1]=C[i+1]<<8|(uint32_t)tempD;
  }  
  
   //sensor 2
  
  I2C_start(addP2);//address***
  I2C_write(0x1E);//reset
  I2C_stop();
  _delay_ms(10);//cpu sleep a while for data ready
  for (int i=0;i<6;i++)
  { 
  I2C_start(addP2);//
  I2C_write(0xA2+i*2);
  I2C_stop();
  I2C_start(addP2+1);//***
  CC[i+1] = I2C_read_ack();
  tempD=I2C_read_nack();
  I2C_stop();
  CC[i+1]=CC[i+1]<<8|(uint32_t)tempD;
  }   
}


//Gyro_init
void Gyro_init(void){
   
	_i2c_write_byte(addGy,0x6B,0x80);//reset gyro

	_i2c_write_byte(addGy,0x6B,0x00);//wake up and wait a little bit		              
	_delay_ms(40);
	
#ifndef Gyrawake
	_i2c_write_byte(addGy,0x6C,0x07);//set Gyro standby if not wake
#endif
	
	_i2c_write_byte(addGy,0x19,0x09);//set divide to 9, so the sample freq=1khz/(1+9)=100hz

#ifdef Gyrawake
	_i2c_write_byte(addGy,0x1A,0x01);//set dlpf_CFG to 1, so the internal freq=1khz
	
	_i2c_write_byte(addGy,0x1B,0x08);//set gyro to 500dps, fchoice b is 00 sensity: 65.5 per 1dps
#endif

	_i2c_write_byte(addGy,0x1C,0x00);//set acc to + -2g sensitivity: 16384 per g
	
	
#ifdef MPU9250
    _i2c_write_byte(addGy,0x1D,0x01);//set a dlpf to 1, afchoice b is 0, acc to 148hz pass,1k/(1+4)=200hz, just 9250 need
    _i2c_write_byte(addGy,0x37,0x02);//initialization of mag sensor: by pass mode

#endif
	_i2c_write_byte(addGy,0x6B,0x40);//sleep
	
	Goffset[0]=Gxoffset;
	Goffset[1]=Gyoffset;
	Goffset[2]=Gzoffset;
	
	Goffset[3]=Axoffset;
	Goffset[4]=Ayoffset;
	Goffset[5]=Azoffset;
	
}

void ReadGyr(void){//burst read, much quicker
	tempD=0;
	I2C_start(addGy);
	I2C_write(0x3B);
	I2C_stop();
	_delay_ms(1);
	I2C_start(addGy+1);
	tempD=I2C_read_ack();GyroValue[0]=tempD;
	tempD=I2C_read_ack();GyroValue[0]=(signed int)GyroValue[0]<<8|tempD; //ax
	
	tempD=I2C_read_ack();GyroValue[1]=tempD;
	tempD=I2C_read_ack();GyroValue[1]=(signed int)GyroValue[1]<<8|tempD;//ay
	
	tempD=I2C_read_ack();GyroValue[2]=tempD;
	tempD=I2C_read_ack();GyroValue[2]=(signed int)GyroValue[2]<<8|tempD;//az

	#ifdef Gyrawake
	tempD=I2C_read_ack();GyroValue[6]=tempD;
	tempD=I2C_read_ack();GyroValue[6]=(signed int)GyroValue[6]<<8|tempD;//Temp

	tempD=I2C_read_ack();GyroValue[3]=tempD;
	tempD=I2C_read_ack();GyroValue[3]=(signed int)GyroValue[3]<<8|tempD;//gx

	tempD=I2C_read_ack();GyroValue[4]=tempD;
	tempD=I2C_read_ack();GyroValue[4]=(signed int)GyroValue[4]<<8|tempD;//gy

	tempD=I2C_read_ack();GyroValue[5]=tempD;
	tempD=I2C_read_nack();GyroValue[5]=(signed int)GyroValue[5]<<8|tempD;//gz
	#endif
	
	
	I2C_stop();
	_i2c_write_byte(addGy,0x6B,0x40);//sleep gyro
	
	
	for (int j=0;j<3;j++)//acc data
	{
		sendData((GyroValue[j]-Goffset[j])*accRel);
	}

	#ifdef Gyrawake
	for (int k=3;k<6;k++)//gyr data
	{
		sendData((GyroValue[k]-Goffset[k])*gyrRel);
	}
	#endif

	
}


void MagIni(){
	
		_delay_ms(3);
		_i2c_write_byte(addMg,0x0B,0x01);	//reset
			
		_i2c_write_byte(addMg,0x0A,0x0F);	//fuse access to get adjustment	
		MagSenAdj[0]=_i2c_read_byte(addMg,0x10);
		//convert_write_data(MagSenAdj[0]);//x adjust
		MagSenAdj[1]=_i2c_read_byte(addMg,0x11);
		//convert_write_data(MagSenAdj[1]);//y adjust		
		MagSenAdj[2]=_i2c_read_byte(addMg,0x12);
		//convert_write_data(MagSenAdj[2]);//z adjust	
		
		_i2c_write_byte(addMg,0x0A,0x00);		//power down mode
	    //Pengbo_i2c_write_byte(addMg,0x0A,0x12);     //continuous measurement will be 8hz, 16bit mode

		Moffset[0]=Mxoffset;//offsets
		Moffset[1]=Myoffset;
		Moffset[2]=Mzoffset;
		MagRelMdf[0]=magRel*((float)(MagSenAdj[0]-128.0)*0.5/128.0+1.0f);
		MagRelMdf[1]=magRel*((float)(MagSenAdj[1]-128.0)*0.5/128.0+1.0f);
		MagRelMdf[2]=magRel*((float)(MagSenAdj[2]-128.0)*0.5/128.0+1.0f);
		sendData(MagSenAdj[0]);
		sendData(MagSenAdj[1]);
		sendData(MagSenAdj[2]);
	
}

void readMag(){
	   uint8_t ctl;
	   _i2c_write_byte(addMg,0x0A,0x11); //one measurement 16bit, then go power down automatically
	   //data ready
	   if (_i2c_read_byte(addMg,0x02)&0x01)
	   {
		   tempD=0;
		   uint8_t tempDlow=0;
		   
		   I2C_start(addMg);
		   I2C_write(0x03);
		   I2C_stop();
		   _delay_ms(1);
		   I2C_start(addMg+1);
		   
		   tempDlow=I2C_read_ack();//low
		   tempD=I2C_read_ack();MagValue[0]=tempD;//high
		   MagValue[0]=(signed int)MagValue[0]<<8|tempDlow; //Hx
		   
		   tempDlow=I2C_read_ack();//low
		   tempD=I2C_read_ack();MagValue[1]=tempD;
		   MagValue[1]=(signed int)MagValue[1]<<8|tempDlow; //Hy
		   
		   tempDlow=I2C_read_ack();//low
		   tempD=I2C_read_nack();MagValue[2]=tempD;
		   MagValue[2]=(signed int)MagValue[2]<<8|tempDlow; //Hz
		   
		   I2C_stop();
	   }
	   ctl=_i2c_read_byte(addMg,0x09);// one time read, overflow check is useless
	   if (!(ctl&0x08))
	   {
		   for (int i=0;i<3;i++)
		   {
			   sendData((MagValue[i]-Moffset[i])*MagRelMdf[i]);
		   }
	   }
}

void OFNIni(){
	//power on reset, at least 20 us
	uint8_t prodID;
	HAL_GPIO_LED_clr();//reset, low active
	_delay_us(50);
	HAL_GPIO_LED_set();
	prodID=_i2c_read_byte(addOFN,0x00);
	sendData(prodID);
	//disable all properties, just count the distance
	_i2c_write_byte(addOFN,0x60,0x00);
	_i2c_write_byte(addOFN,0x61,0x00);
}
void readOFN(){
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
//I2C
uint8_t _i2c_read_byte(uint8_t SenAdd,uint8_t RegAdd){
		uint8_t flag;
		I2C_start(SenAdd);
		I2C_write(RegAdd);
		I2C_stop();
		_delay_ms(1);
		I2C_start(SenAdd+1);
		flag=I2C_read_nack();
		I2C_stop();
		return flag;	
}

void _i2c_write_byte(uint8_t SenAdd, uint8_t RegAdd,uint8_t value){
	
		I2C_start(SenAdd);
		I2C_write(RegAdd);
		I2C_write(value);
		I2C_stop();		
	
}




void sendData(int32_t Data){	
	uint8_t space=' ';
	AddDat2Buf(space);
	
	ltoa(Data,result,10);	
	for (int i=0;i<11; i++)
	{
		if (result[i]!='\0')
		{
			AddDat2Buf(result[i]);
		}
		else break;
		
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

