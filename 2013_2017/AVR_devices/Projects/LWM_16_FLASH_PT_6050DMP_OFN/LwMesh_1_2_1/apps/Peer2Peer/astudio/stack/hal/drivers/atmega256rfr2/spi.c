/*
 * spi.c
 *
 * Created: 2/20/2016 4:49:44 PM
 *  Author: pli3
 */ 
#include "spi.h"
#include <avr/io.h>#include <util/delay.h>
//Initialize SPI Master Device
void select()
{
	PORTE&= ~(1<<2);//output low at PE2, and with 11111011
}
void deselect()
{
	PORTE|= 0xFF;//output low at PE2, all high, or with 11111111
}

void spi_init_master ()
{	DDRB = (1<<1)|(1<<2);              //Set PB2 MOSI, PB1 SCK as Output
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //Enable SPI, Set as Master osci clock /4 16/4=4Mhz
	//future version will connect ss to ground, no need this
	DDRE|= 0x04;  //PE2 as output, others are still input
	
}
//Function to send and receive data
unsigned char spi_transfer (unsigned char dataOut)
{
	SPDR = dataOut;                       //Load data into the buffer
	while(!(SPSR & (1<<SPIF) ));       //Wait until transmission complete
	return(SPDR);                      //Return received data
}


void SPIStore8bit(uint8_t dataIn){
	select();
	spi_transfer(0xab);//power up
	deselect();
	_delay_us(5);
	select();
	spi_transfer(0x06);//write enable
	deselect();
	_delay_us(5);
	
	
	
	
}
void SPIStore16bit(uint8_t dataIn){
	select();
	spi_transfer(0xab);//power up
	deselect();
	_delay_us(5);
	select();
	spi_transfer(0x06);//write enable
	deselect();
	_delay_us(5);
}
void SPIStore32bit(uint8_t dataIn){
	
}