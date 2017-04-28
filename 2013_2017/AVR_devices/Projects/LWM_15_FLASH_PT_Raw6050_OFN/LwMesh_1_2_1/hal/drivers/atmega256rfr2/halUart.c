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



/*- Definitions ------------------------------------------------------------*/

static uint8_t rxData[201];

static SYS_Timer_t Pengbo250msTimer;
static SYS_Timer_t Pengbo20msTimer;
static SYS_Timer_t Pengbo10msTimer1;
static SYS_Timer_t Pengbo10msTimer2;
static uint8_t dataforflash[26];// D11-D22 4*3=12  Gyro16=2*6=12 2 OFN



//The ad0 is connected to high, so the address is 1101 001X D2 and D3 for new version pcb, but for previous sparkfun is D0 and D1
#define addGy 0xD2
//two pressure sensor:1110110X EC, ED 1110111X EE, EF
#define addP1 0xEC//EC
#define addP2 0xEE
#define addOFN 0xAE
#define CNumberPerSample 26
#define OFN
//#define Erase
//#define Read

/* 
1. Chip Erase: Erase is the task
   After finish soldering the chip,uncomment erase and read,chip will be erased and the all 255 will be read out after finish.
   Note: if P and Gyro not connected, need comment related single sensor initialization function
   Note: if P is connected, get the C and CC and put in UARTini for easy read after test.
2. Animal test Program loading: load program is the task
   comment read and erase, others be the test codes, do not connect sensors, load program,the program will be blocked at
   initialization and there will be no write or overwrite in loading and power up. 
3. Animal test data: store data
   After connect all parts and battery, the program will store the data in the flash
4. Read data
   after animal test,in order to no overwrite, do not connect P sensor, it will block at initialization
   and then uncomment read, reload the new program, and connect pressure sensor,it will not erase or write,just will read data out
   Note: if hard to connect pressure sensor, just comment Psensor Ini, but the initial values of CC and C should be right.
   Note2: if gyro and OFN are not connected, need comment related functions in Ini.
NOTE: sleep function also need care
*/ 


  

  /*************************************************************************//**
  *****************************************************************************/
  
//response of the 250mstimer: active gyro
static void Pengbo250msTimerHandler(SYS_Timer_t *timer)
{
	#ifdef Read
	SendStoreReadsend(3);//read from flash and send
	#else
	TimerFlag+=1;	
	if (TimerFlag==240)//4=1s
	{
		//First use number to active sleep mode then use real sensor values during program chip
		_i2c_write_byte(addGy,0x6B,0x00);//wake up gyro
		SYS_TimerStop(&Pengbo20msTimer);//wait 20ms timer
		SYS_TimerStart(&Pengbo20msTimer);
		TimerFlag-=1;	
	}
	#endif

}
//convert temperature
static void Pengbo20msTimerHandler(SYS_Timer_t *timer)
{
	I2C_start(addP1);  I2C_write(0x58); I2C_stop();//wake up pressure sensors for temperature reading
	I2C_start(addP2);  I2C_write(0x58); I2C_stop();      
	(void)timer;
	
	SYS_TimerStop(&Pengbo10msTimer1);//wait 10ms timer
	SYS_TimerStart(&Pengbo10msTimer1);

}

//Get temperature, convert pressure
static void Pengbo10msTimer1Handler(SYS_Timer_t *timer)
{
	
	I2C_start(addP1); I2C_write(0x00);	I2C_stop();
	I2C_start(addP1+1); dataforflash[0]= I2C_read_ack();
	dataforflash[1]=I2C_read_ack(); 
	dataforflash[2]=I2C_read_nack(); I2C_stop();
	
	
	I2C_start(addP2); I2C_write(0x00);	I2C_stop();
	I2C_start(addP2+1); dataforflash[3]= I2C_read_ack(); 
	dataforflash[4]=I2C_read_ack();
	dataforflash[5]=I2C_read_nack(); I2C_stop();
	
	I2C_start(addP1);  I2C_write(0x48); I2C_stop();//wake up pressure
	I2C_start(addP2);  I2C_write(0x48); I2C_stop();
	(void)timer;
	
	SYS_TimerStop(&Pengbo10msTimer2);//wait 10ms timer
	SYS_TimerStart(&Pengbo10msTimer2);

}
static void Pengbo10msTimer2Handler(SYS_Timer_t *timer)
{
	
	I2C_start(addP1);  I2C_write(0x00); I2C_stop();
	I2C_start(addP1+1);  dataforflash[6]= I2C_read_ack();
	dataforflash[7]=I2C_read_ack();  
	dataforflash[8]=I2C_read_nack(); 
	I2C_stop();
	
	I2C_start(addP2);  I2C_write(0x00); I2C_stop();
	I2C_start(addP2+1);  dataforflash[9]= I2C_read_ack();
	dataforflash[10]=I2C_read_ack();  
	dataforflash[11]=I2C_read_nack(); 
	I2C_stop();
	
	ReadGyr();
	
#ifdef OFN
	Read_OFN();
#endif
    SendStoreReadsend(2); //each 250 ms, perform some action: 1 send, 2 store
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
	pageWNm=0;
	pageNmH=0x00;
	pageNmL=0x00;
	  
  
	Pengbo250msTimer.interval = 250;//250*10.0098=250.
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
	spi_init_master();  
	//P sensor Prom values, need to change after new sensor, address need to be matched. EE and EC should be still EE and EC
	C[0]=0;C[1]=32096;C[2]=29403;C[3]=37974;C[4]=17363;C[5]=33993;C[6]=27957;
	CC[0]=0;CC[1]=32015;CC[2]=28015;CC[3]=38145;CC[4]=16296;CC[5]=34519;CC[6]=27987;
	SenIni();
	SYS_TimerStart(&Pengbo250msTimer);//start timer after finish of initialization  
	
	
}

void SenIni(){
	PSensor_init();
	Gyro_init();
#ifdef OFN
	OFN_ini();
#endif
    FlashIni();

	  
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
  I2C_start(addP1);//address***
  I2C_write(0x1E);//reset
  I2C_stop();
  _delay_ms(10);//delay 10+ms for reset
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
	
	_i2c_write_byte(addGy,0x19,0x04);//set divide to 24, so the sample freq=1khz/(4+1)=200hz

	_i2c_write_byte(addGy,0x1A,0x01);//set dlpf_CFG to 1, so the internal freq=1khz
	
	_i2c_write_byte(addGy,0x1B,0x08);//set gyro to 500dps, fchoice b is 00 sensity: 65.5 per 1dps
	
	_i2c_write_byte(addGy,0x1C,0x00);//set acc to + -2g sensitivity: 16384 per g
	
	_i2c_write_byte(addGy,0x1D,0x01);//set a dlpf to 1, afchoice b is 0, acc to 148hz pass,1k/(1+4)=200hz
	
	_i2c_write_byte(addGy,0x6B,0x40);//sleep
	
	
}

void ReadGyr(void){//burst read, much quicker
	I2C_start(addGy);
	I2C_write(0x3B);
	I2C_stop();
	_delay_ms(1);
	I2C_start(addGy+1);
	for (int j=12;j<18;j++)//ax to az
	{
		dataforflash[j]=I2C_read_ack();
	}

	for (int k=18;k<20;k++)//temperature
	{
		dataforflash[k]=I2C_read_ack();
	}
	
	for (int l=18;l<23;l++)//gx-gz, temperature overwrite
	{
		dataforflash[l]=I2C_read_ack();
	}
	dataforflash[23]=I2C_read_nack();
	
	I2C_stop();
	_i2c_write_byte(addGy,0x6B,0x40);//sleep gyro
	
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
		dataforflash[24]=_i2c_read_byte(addOFN,0x03);
		dataforflash[25]=_i2c_read_byte(addOFN,0x04);
	}
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

void sendFlashdata(){
	uint32_t D11;//pressure sensor value
	uint32_t D12;
	uint32_t D21;//pressure sensor value
	uint32_t D22;
	int32_t dT;//calibration
	int64_t OFF;
	int64_t SENS;
	int32_t P;//pressure
	int32_t TEMP;//Temperature
	D12=((uint32_t)dataforflash[0]<<16)|((uint32_t)dataforflash[1]<<8)|dataforflash[2];
	D22=((uint32_t)dataforflash[3]<<16)|((uint32_t)dataforflash[4]<<8)|dataforflash[5];
    D11=((uint32_t)dataforflash[6]<<16)|((uint32_t)dataforflash[7]<<8)|dataforflash[8];
	D21=((uint32_t)dataforflash[9]<<16)|((uint32_t)dataforflash[10]<<8)|dataforflash[11];
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
	
	int16_t Li;
	Li=(signed int)dataforflash[12]<<8|dataforflash[13];//ax
	sendData(Li);
	Li=(signed int)dataforflash[14]<<8|dataforflash[15];//ay
	sendData(Li);
	Li=(signed int)dataforflash[16]<<8|dataforflash[17];//az
	sendData(Li);
	Li=(signed int)dataforflash[18]<<8|dataforflash[19];//gx
	sendData(Li);
	Li=(signed int)dataforflash[20]<<8|dataforflash[21];//gy
	sendData(Li);
	Li=(signed int)dataforflash[22]<<8|dataforflash[23];//gz
	sendData(Li);
	sendData((int8_t)dataforflash[24]);
	sendData((int8_t)dataforflash[25]);
}


void SendStoreReadsend(uint8_t Action)// 1 send, 2 store 3 read send
{
	uint8_t pagelimit=256/CNumberPerSample;
	if (Action==1)//send data via rf channel
	{
		sendFlashdata();
	}
	
	else if (Action==2)
	{
		select();    spi_transfer(0xab);//power up
        deselect();  _delay_us(5);
		
		select();	spi_transfer(0x06);//write enable
		deselect();	_delay_us(5);
		
		select();
		spi_transfer(0x02);//write
		spi_transfer(pageNmH);//page number
		spi_transfer(pageNmL);
		spi_transfer(pageWNm*CNumberPerSample);
		for (int i=0;i<CNumberPerSample;i++)//26
		{
			spi_transfer(dataforflash[i]);
		}
		deselect();
		_delay_ms(6);
		
		pageWNm+=1;//write 1 time at current page
		if (pageWNm==pagelimit)//this page is full, update page number
		{
			if (pageNmL==255)//lower full, need move to higher block/sector
			{
				pageNmH+=1;
			}
			pageNmL+=1;//next page
			pageWNm=0;//for new pa
		}
		
		select();//put it to power down
		spi_transfer(0xB9);//power down
		deselect();		
	}
	
	else if (Action==3)
	{
		select();
		spi_transfer(0xab);//power up
		deselect();
		_delay_us(5);
		
		
		select();
		spi_transfer(0x03);//power up
		spi_transfer(pageNmH);//page number
		spi_transfer(pageNmL);
		spi_transfer(pageWNm*CNumberPerSample);
		for (int i=0;i<CNumberPerSample;i++)//26
		{
			dataforflash[i]=spi_transfer(0);	
		}
		pageWNm+=1;//write 1 time at current page
		if (pageWNm==pagelimit)//this page is full, update page number
		{
			if (pageNmL==255)//lower full, need move to higher block/sector
			{
				pageNmH+=1;
			}
			pageNmL+=1;//next page
			pageWNm=0;//for new pa
		}
		deselect();
		_delay_ms(1);
		
		select();//put it to power down
		spi_transfer(0xB9);//power down
		deselect();
		
        sendFlashdata();	
    }
}


void FlashIni(){
#ifdef Erase
	{
		select();    spi_transfer(0xab);//power up
		deselect();  _delay_us(5);
		
		select();	spi_transfer(0x06);//write enable
		deselect();	_delay_us(5);

		select();
		spi_transfer(0x60);//erease 4
		deselect();
		_delay_ms(420000);//delay 420s
		
		sendData(1000);

	}
#endif
	select();//put it to power down
	spi_transfer(0xB9);
	deselect();
}
		