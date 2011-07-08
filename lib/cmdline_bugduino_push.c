#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#define BMI_BUGDUINO_IOCTL  ('d')                                               

#include <linux/bmi/bmi_bugduino.h>
//#include "push_bugduino_code.h"

#define INSTRUCTION_WRITE "WRIT"

static int bugduino_write( int slot, char* filename);


int main(int argc, char **argv)
{
#define MAX_dataSz (1024*1024)
	uint8_t data[MAX_dataSz];
	uint8_t dataSz = 0; 
	int fd = -1;
	int target_slot =2 ; //default target slot
	
	if(argc <= 1) { 
		printf("usage: arduino_avr_hex <target_bmi>\n");
		return 0;
	} 

	fd= open(argv[1],'r');
 	dataSz = read(fd,(void*)data, MAX_dataSz);
	if(dataSz == MAX_dataSz) {
		printf("file too large!\n");
		return -1;
	}
	else if (dataSz <= 0) {
		printf("No data in file, or read error\n");
		return -1;
	}
	target_slot = atoi(argv[2]);
	if(target_slot > 3 || target_slot < 0) {
		printf("illegal target slot %d", target_slot);
		return -3;
	}
	//BUG: On Bugbase YT programming on slot zero fails.
	// no time to debug now, an annoyance but not a blocker
	if (target_slot == 0) {  
		printf("BETA: currently programming via slot 0 is disabledi\n");
		return -2;
	}	
	
	printf("trying to compile and write to bug\n");	
	bugduino_write(target_slot, argv[1]);
	return 0; 

}

/** 
 * write a chunk of hex data to an arduino for programming.
 * @slot - target slot with the bugduino on it
 * @data - data to program to the bugduino
 * @dataSz - size of data to program in bytes 
*/

static int bugduino_write( int slot, char* filename)
{
	char uart[254];
	char fileCtl[256];
	int fd = 0;
	if(slot < 0 || slot > 3) {
		printf("Slot %d out of range\n.",slot);
		return -5;
	}

	// generate our ioctl file, and our uart file by slot #
	snprintf(uart, 256,"/dev/ttyBMI%d",slot);
	snprintf(fileCtl,256,"/dev/bmi_bugduino_slot%d",slot); 
	fd = open(fileCtl,'w');

	//control block for actual writing of data
	//via the onboard uart.
	{
		int iRet =0;
		char* uart_prefix = "-P";
		char  uart_option[256];
		char  file_option[256];
		snprintf( uart_option, 256, "%s%s", uart_prefix, uart );
		printf("uart option %s%s\n", uart_prefix, uart );
 		printf("trying to write via uart %s\n",uart_option); 
		
		snprintf(file_option, 256, "-Uflash:w:%s", filename);

	if(fd > 0 ) {
		printf("doing reset now\n");
		ioctl(fd, BMI_BUGDUINO_RESET,1);
		sleep(1);
		ioctl(fd, BMI_BUGDUINO_RESET,0);
		close(fd);
	}
	else {
		printf("unable to open bmi slot %d ctrl file %s",slot, fileCtl);
		return -6;
	}  
	
		iRet = execl( "/usr/bin/avrdude", "/usr/bin/avrdude", 
			"-pm328p",
			"-cstk500v1",
			uart_option,
			"-b57600",
			"-D",
			file_option,
			NULL );
		return iRet;
	}
	return 0;
}

