#ifndef  F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/twi.h>

#include "I2C_master.h"

#define F_SCL 200000UL // SCL frequency 400k is too fast for mpu
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void I2C_init(void){
	TWBR = TWBR_val;
}

uint8_t I2C_start(uint8_t address){
	// reset TWI control register
	TWCR = 0;
	// transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	
	return 0;
}

uint8_t I2C_writebyte(uint8_t data){
	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	return 0;
}


uint8_t I2C_read_ack(void){
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

uint8_t I2C_read_nack(void){
	
	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

void I2C_stop(void){
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

//multiple byte read
uint8_t i2c_read(uint8_t devAddr, uint8_t regAddr, uint8_t number,uint8_t *data){
	I2C_start(devAddr);//S+ADW
	I2C_writebyte(regAddr);//RA
	I2C_start(devAddr+1);//S+ADR
	for (int i=0;i<number-1;i++)//data+ack
	{
		data[i]=I2C_read_ack();
	}
	data[number-1]=I2C_read_nack();//data+nack
	I2C_stop();//P
	return 0;
		
}

//multiple bytes write 

uint8_t i2c_write(uint8_t devAddr, uint8_t regAddr,uint8_t number, uint8_t *data){
		I2C_start(devAddr);//S+ADW
		I2C_writebyte(regAddr);//RA
		for (int i=0;i<number;i++)//Data
		{
			I2C_writebyte(data[i]);
		}
		I2C_stop();//P
		return 0;
}

uint8_t _i2c_read_byte(uint8_t SenAdd,uint8_t RegAdd){
	uint8_t flag;
	I2C_start(SenAdd);
	I2C_writebyte(RegAdd);
	I2C_start(SenAdd+1);
	flag=I2C_read_nack();
	I2C_stop();
	return flag;
}


void _i2c_write_byte(uint8_t SenAdd, uint8_t RegAdd,uint8_t value){
	
	I2C_start(SenAdd);
	I2C_writebyte(RegAdd);
	I2C_writebyte(value);
	I2C_stop();
	
}

//I2C loweest level functions
//_i2c byte byte write or read
//i2c highest level functions, could perform multiple bytes read and write