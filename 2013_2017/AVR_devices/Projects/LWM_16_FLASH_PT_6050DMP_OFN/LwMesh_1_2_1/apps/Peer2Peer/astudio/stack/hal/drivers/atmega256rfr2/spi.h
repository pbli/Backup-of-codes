/*
 * spi.h
 *
 * Created: 2/20/2016 4:49:19 PM
 *  Author: pli3
 */ 


#ifndef SPI_H_
#define SPI_H_
#include <stdint.h>
#include <sysConfig.h>
void select(void);
void deselect(void);
unsigned char spi_transfer(unsigned char dataOut);
void spi_init_master (void);
#endif /* SPI_H_ */