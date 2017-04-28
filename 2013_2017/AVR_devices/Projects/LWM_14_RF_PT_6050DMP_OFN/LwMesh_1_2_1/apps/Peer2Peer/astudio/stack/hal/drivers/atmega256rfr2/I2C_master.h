
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define I2C_MASTER_H

#define I2C_READ 0x01
#define I2C_WRITE 0x00
//basic i2c functions
void I2C_init(void);
uint8_t I2C_start(uint8_t address);
uint8_t I2C_writebyte(uint8_t data);
uint8_t I2C_read_ack(void);
uint8_t I2C_read_nack(void);
void I2C_stop(void);
//byte write and byte read
void _i2c_write_byte(uint8_t SenAdd, uint8_t RegAdd,uint8_t value);
uint8_t _i2c_read_byte(uint8_t SenAdd,uint8_t RegAdd);
//burst write and read
uint8_t i2c_read(uint8_t devAddr, uint8_t regAddr, uint8_t number,uint8_t *data);
uint8_t i2c_write(uint8_t devAddr, uint8_t regAddr,uint8_t number, uint8_t *data);