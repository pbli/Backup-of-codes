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
#include "hal.h"
#include "halUart.h"
#include "config.h"
#include "sysTimer.h"
#include "I2C_master.h"
#include <avr/io.h>



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
static uint8_t dataforflash[26];// D11-D22 4*3=12  Gy

static SYS_Timer_t Pengbo50msTimer;
static SYS_Timer_t Pengbo15msTimer1;
static SYS_Timer_t Pengbo15msTimer2;
#define addGy 0xD2
//two pressure sensor:1110110X EC, ED 1110111X EE, EF
#define addP1 0xEC//EC
#define addP2 0xEE
#define addOFN 0xAE 
#define CNumberPerSample 26
//#define OFN
//#define Psensor
//#define IMU
//#define Erase  //erase chip before test
#define Read   //read data after test or erase
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
NOTE:sleep need care
*/ 


static void Pengbo50msTimerHandler(SYS_Timer_t *timer)//use 25hz timer to read fifo
{	
#ifdef Read
      ReadSendFl();  
#else
    uint32_t timerrange=10;//3hours
	TimerFlag+=1;	
	if (TimerFlag==timerrange)
	{
		_i2c_write_byte(addGy,0x6B,0x80);//reset gyro
		_i2c_write_byte(addGy,0x6B,0x00);//wake up and wait next interrupt,it will be about 50
	}
	
	if (TimerFlag==timerrange+1)
	{
		mympu_open(20);
	}
	
	if (TimerFlag==timerrange+2)
	{
		TimerFlag-=1;		
		mympu_update(timerMark);
		timerMark+=1;
		if (timerMark==5)
		{
			float accW1;
			float accW2;
			float accW3;
			accW1=(mympu.acclw[0]+mympu.acclw[3]+mympu.acclw[6]+mympu.acclw[9]+mympu.acclw[12])/5.0f;
			accW2=(mympu.acclw[1]+mympu.acclw[4]+mympu.acclw[7]+mympu.acclw[10]+mympu.acclw[13])/5.0f;
			accW3=(mympu.acclw[2]+mympu.acclw[5]+mympu.acclw[8]+mympu.acclw[11]+mympu.acclw[14])/5.0f;
			
		
			char aw[4];
		
			memcpy(aw,&accW1,sizeof(float));
			for (int i=0;i<4;i++)
			{
			dataforflash[i+12]=aw[i];
			}
		
			memcpy(aw,&accW2,sizeof(float));
			for (int i=0;i<4;i++)
			{
			dataforflash[i+16]=aw[i];
			}
		
			memcpy(aw,&accW3,sizeof(float));
			for (int i=0;i<4;i++)
			{
			dataforflash[i+20]=aw[i];
			}
		
			timerMark=0;

			#ifdef Psensor
			I2C_start(addP1);  I2C_writebyte(0x58); I2C_stop();//wake up pressure sensors for temperature reading
			I2C_start(addP2);  I2C_writebyte(0x58); I2C_stop();
		    #endif
		
			SYS_TimerStop(&Pengbo15msTimer1);//wait 10ms timer
			SYS_TimerStart(&Pengbo15msTimer1);
		}		
	}
#endif  
	
}
static void Pengbo15msTimer1Handler(SYS_Timer_t *timer)
{
#ifdef Psensor
	I2C_start(addP1); I2C_writebyte(0x00);	I2C_stop();
	I2C_start(addP1+1); dataforflash[0]= I2C_read_ack();
	dataforflash[1]=I2C_read_ack(); 
	dataforflash[2]=I2C_read_nack();I2C_stop();


	I2C_start(addP2); I2C_writebyte(0x00);	I2C_stop();
	I2C_start(addP2+1); dataforflash[3]= I2C_read_ack();
	dataforflash[4]=I2C_read_ack();
	dataforflash[5]=I2C_read_nack(); I2C_stop();

	
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
	I2C_start(addP1);  I2C_writebyte(0x00); I2C_stop();
	I2C_start(addP1+1);  dataforflash[6]= I2C_read_ack();
	dataforflash[7]=I2C_read_ack();
	dataforflash[8]=I2C_read_nack();
	I2C_stop();
	
	I2C_start(addP2);  I2C_writebyte(0x00); I2C_stop();
	I2C_start(addP2+1);  dataforflash[9]= I2C_read_ack();
	dataforflash[10]=I2C_read_ack();
	dataforflash[11]=I2C_read_nack();
	I2C_stop();
#endif

#ifdef OFN
	Read_OFN();
#endif	
	//Store2Fl();
	sendCachedFlashdata();
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
	pageWNm=0;
	pageNmH=0;
	pageNmL=0;
	timerMark=0;
	TimerFlag=0;
  
	Pengbo50msTimer.interval = 50;//0.0050049
#ifdef Read
    Pengbo50msTimer.interval = 1000;
#endif
	Pengbo50msTimer.mode = SYS_TIMER_PERIODIC_MODE;
	Pengbo50msTimer.handler = Pengbo50msTimerHandler;
    //need 15ms for DMP version
	Pengbo15msTimer1.interval=15;
	Pengbo15msTimer1.mode=SYS_TIMER_INTERVAL_MODE;
	Pengbo15msTimer1.handler=Pengbo15msTimer1Handler;
  
	Pengbo15msTimer2.interval=15;
	Pengbo15msTimer2.mode=SYS_TIMER_INTERVAL_MODE;
	Pengbo15msTimer2.handler=Pengbo15msTimer2Handler;
	I2C_init();
	spi_init_master();	
	//different sensors are different
	C[0]=0;C[1]=31860;C[2]=27726;C[3]=37783;C[4]=16107;C[5]=33809;C[6]=28079;
	CC[0]=0;CC[1]=31995;CC[2]=28332;CC[3]=37992;CC[4]=16711;CC[5]=34495;CC[6]=27927;
	Sensor_ini();  
	SYS_TimerStart(&Pengbo50msTimer);//start timer after finish of initialization 
	//P sensor Prom values, need to change after new sensor, address need to be matched. EE and EC should be still EE and EC

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
	FlashIni();
#ifdef OFN
	OFN_ini();	
#endif
#ifdef IMU
	_i2c_write_byte(addGy,0x6B,0x40);//sleep gyro
#endif
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
  C[i+1]=C[i+1]<<8|(uint32_t)tempD;
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
  CC[i+1]=CC[i+1]<<8|(uint32_t)tempD;
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
		dataforflash[24]=_i2c_read_byte(addOFN,0x03);
		dataforflash[25]=_i2c_read_byte(addOFN,0x04);
	}
}

//change anything to string and add to rxbuffer, much simple than before
void sendData(int32_t Data){
    uint8_t space=' ';
	AddDat2Buf(space);
	ltoa(Data,result,10);
	for (int i=0;i<10; i++)
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

void sendCachedFlashdata(){
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
	
	uint8_t Li[4];
	float awx;
	Li[0]=dataforflash[12];Li[1]=dataforflash[13];Li[2]=dataforflash[14];Li[3]=dataforflash[15];
	memcpy(&awx,Li,sizeof(float));
	sendData(awx);
	Li[0]=dataforflash[16];Li[1]=dataforflash[17];Li[2]=dataforflash[18];Li[3]=dataforflash[19];
	memcpy(&awx,Li,sizeof(float));
	sendData(awx);
	Li[0]=dataforflash[20];Li[1]=dataforflash[21];Li[2]=dataforflash[22];Li[3]=dataforflash[23];
	memcpy(&awx,Li,sizeof(float));
	sendData(awx);


	sendData((int8_t)(dataforflash[24]));
	sendData((int8_t)(dataforflash[25]));
}

void ReadSendFl()
{
	uint8_t pagelimit=256/CNumberPerSample;
	select();
	spi_transfer(0xab);//power up
	deselect();
	_delay_us(5);
	
	select();
	spi_transfer(0x03);
	spi_transfer(pageNmH);//page number
	spi_transfer(pageNmL);
	spi_transfer(pageWNm*CNumberPerSample);//
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
		pageWNm=0;//for new page, reset the value of offset
	}
	deselect();
	_delay_ms(1);
	
	select();//put it to power down
	spi_transfer(0xB9);//power down
	deselect();
	sendCachedFlashdata(); //This part calculation will overflow?
}

void Store2Fl()
{
	uint8_t pagelimit=256/CNumberPerSample;
	
	select();    spi_transfer(0xab);//power up
	deselect();  _delay_us(5);
		
	select();	spi_transfer(0x06);//write enable
	deselect();	_delay_us(5);
		
	select();
	spi_transfer(0x2);//write
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
		pageWNm=0;//for new page, reset the value of offset

	}
	select();//put it to power down
	spi_transfer(0xB9);//power down
	deselect();
		

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

	}
	#endif
	select();//put it to power down
	spi_transfer(0xB9);
	deselect();
}