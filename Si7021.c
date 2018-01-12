/*
 ============================================================================
 Name        : Si7021.c
 Author      : JhonToroM
 Version     :
 Copyright   : Your copyright notice
 Description : This Application prints Temperature and Humidity values using
 	 	 	    Si7021 sensor.

 ============================================================================
 */

/*================================================
BBB_expansion_header_P9_pins     MPU6050 pins
===================================================
P9_19                              SCL
P9_20                              SDA
P9_3                               VCC 3.3v
P9_1                               GND
==================================================== */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

/* This is the I2C slave address of Si7021 sensor */
#define Si7021_SLAVE_ADDR 				0x40


/*Si7021 register addresses */
#define Si7021_RH_HoldMaster_Mode		0xE5
#define Si7021_RH_NoHondMaster_Mode 	0xF5
#define Si7021_Temp_HoldMaster_Mode		0xE3
#define Si7021_TempP_NoHoldMaster_Mode	0xF3
#define Si7021_Temp_Pre_RHmeasurement	0xE0
#define Si7021_Reset					0xFE

/* */
#define Si7021_USER1_READ				0xE7
#define Si7021_USER1_WRITE				0xE6

/* this is the linux OS device file for the I2C-2 controller of the SOC */
#define I2C_DEVICE_FILE "/dev/i2c-2"

int fd;




/*write a 8bit "data" to the sensor at the address indicated by "addr" */
int Si7021_write(uint8_t addr, uint8_t data)
{
  int ret;
  char buf[2];
  buf[0]=addr;
  buf[1]=data;
  ret = write(fd,buf,2);
  if (ret <= 0)
  {
      perror("write failed\n");
      return -1;
  }
  return 0;
}

/*read "len" many bytes from "addr" of the sensor in to the adresss indicated by "pBuffer" */
int Si7021_read(uint8_t base_addr, char *pBuffer,uint32_t len)
{
  int ret;
  char buf[2];
  buf[0]=base_addr;
  ret = write(fd,buf,1);
  if (ret <= 0)
  {
      perror("write address failed\n");
      return -1;
  }
  //usleep(5000);
  ret = read(fd,pBuffer,len);
  if(ret <= 0)
  {
      perror("read failed\n");
  }
  return 0;
}

void Si7021_read_temp(short *pBuffer)
{
	char Temp_buffer[2];
	short x;
	Si7021_read(Si7021_Temp_Pre_RHmeasurement,Temp_buffer,2);
	x =  ( (Temp_buffer[0] << 8) + (Temp_buffer[1]) );
	*pBuffer = x;
	//printf("Temperatura raw value : %d \n",x);
}

void Si7021_read_RH(short *pBuffer)
{
	char RH_buffer[2];
	short x;
	Si7021_read(Si7021_RH_HoldMaster_Mode,RH_buffer,2);
    x = ( (RH_buffer[0] << 8) + (RH_buffer[1]) );
    *pBuffer = x;
	//printf("RH raw value : %d \n",x);
}

void Si7021_init()
{
	/* Set Resolution */
	// 0x00 = 14 bit temp, 12 bit RH (default)
	// 0x01 = 12 bit temp, 8 bit RH
	// 0x80 = 13 bit temp, 10 bit RH
	// 0x81 = 11 bit temp, 11 bit RH
	char Resolution;
	char *Resolution_pt = &Resolution;

	Si7021_read(Si7021_USER1_READ,Resolution_pt,1);
	printf("Actual Configuration Val: %d \n", Resolution);
	if (Resolution != 0x3A){
		Si7021_write(Si7021_USER1_WRITE,0x3A);
	}
}


int main(void)
{
	double Temp,RH;
	short raw_Temp = 0;
	short raw_RH = 0;
	short *Temp_pt = &raw_Temp;
	short *RH_pt = &raw_RH;

	printf("Lectura de TEMPERATURA/HUMEDAD Sensor Si7021 \n");

	/* First lets open the I2C device file */

	if ((fd = open(I2C_DEVICE_FILE,O_RDWR)) < 0){
		perror("Failed to open I2C device fail. \n");
		return -1;
	}

	/* Set the I2C slave address using ioctl I2C_SLAVE command */

	if(ioctl(fd,I2C_SLAVE,Si7021_SLAVE_ADDR) < 0){
		perror("Failed to acquire bus access and|or talk to SLAVE.\n");
		close(fd);
		return -1;
	}

	/* Init configuration of slave (Set Resolution) */

	Si7021_init();

	/* Read raw values*/

	Si7021_read_RH(RH_pt);
	Si7021_read_temp(Temp_pt);
	printf("Temperatura raw value : %d \n",raw_Temp);
	printf("RH raw value : %d \n",raw_RH);
	/* Convert values */

	RH = ( ((125*raw_RH) / 65536) - 6);
	Temp = ( ((175.72*raw_Temp)/65536) - 46.85);

	printf("Temp:%0.2f    RH:%0.2f  \n",Temp,RH);


}
