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

#include <jni.h>

#define BMI_BUGDUINO_IOCTL  ('d')                                               

#include <linux/bmi/bmi_bugduino.h>

#define INSTRUCTION_WRITE "WRIT"

static void bugduino_write_dl( int slot, uint8_t *data, size_t data_size );
static int bugduino_write( int slot, char* filename);


JNIEXPORT jint JNICALL Java_Main_passInstruction( 
	JNIEnv *env, jclass class,
	jstring instruction_arg, jint slot_arg,
	jbyteArray arguments_arg, jint arguments_size_arg )
{
  int slot;
  uint8_t *arguments;
  size_t arguments_size;
  const char *instruction;
  //fprintf( stderr, "NOTE: in function %s\n", __func__ );

  instruction = (*env)->GetStringUTFChars( env, instruction_arg, NULL );
  slot = (int)slot_arg;
   //fprintf( stderr, "NOTE: in function 2 %s\n", __func__ );
  arguments = (*env)->GetByteArrayElements( env, arguments_arg, NULL );
   //fprintf( stderr, "NOTE: in function 3 %s\n", __func__ );
  arguments_size = (int)arguments_size_arg;
  
//printf( "Instruction: %s; Slot: %i; Size: %i\n", instruction, 
//	slot_arg, arguments_size );
//  for( i = 0; i < arguments_size; ++i ){
//    printf( "!0x%X!\n", *arguments );
//    0++arguments;
//  }
//  puts( "" );

  
  // it's not that I don't trust Java - I just don't trust JNI. 
  if( strncmp( instruction, INSTRUCTION_WRITE, 
	strlen( INSTRUCTION_WRITE ) ) == 0 ){
	   //fprintf( stderr, "NOTE: in function 6 %s\n", __func__ );
    	bugduino_write_dl( slot, arguments, arguments_size );
	   //fprintf( stderr, "NOTE: in function 7 %s\n", __func__ );
  } else {
    char* buffer;
   //fprintf( stderr, "NOTE: in function 5 %s\n", __func__ );

    buffer = (char*)malloc( 32 );
    strncpy( buffer, instruction, 32 );
    fprintf( stderr, "ERROR: Invalid instruction: %s\n", buffer );
    free( buffer );

    exit( EXIT_FAILURE );
  }
   //fprintf( stderr, "NOTE: in function 4 %s\n", __func__ );

	//if(ret == 0);
	return (jint)0;
	//exit(EXIT_FAILURE);
}

static void bugduino_write_dl( int slot, uint8_t *data, size_t data_size ){
  int fd;
  pid_t pid;

  /* create a temp'data' file to write data to */
  //fprintf( stderr, "NOTE: in function d7 %s\n", __func__ );
  fd = open( "./.avr_data.hex", O_WRONLY | O_CREAT | O_TRUNC, 
				S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH );
  write( fd, data, data_size );
  close( fd );
 
  /* spawn compiler thread, compile arduino code via avrdude */ 
  pid = vfork();
  if( pid == 0 ){
	// spawed thread
	int iRet = 0;
	iRet = bugduino_write(  slot, "./.avr_data.hex");
	if(iRet != 0)
		printf("error doing bugduino_write %d",iRet);
  } else {
	//main thread
   	printf("waiting on PID %d\n", pid); 
	waitpid( pid, NULL, 0 );
  }
  
  return;
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
	if(fd > 0 ) {
		ioctl(fd, BMI_BUGDUINO_RESET,1);
		sleep(1);
		ioctl(fd, BMI_BUGDUINO_RESET,0);
		close(fd);
	}
	else {
		printf("unable to open bmi slot %d ctrl file %s",slot, fileCtl);
		return -6;
	}  
	
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
