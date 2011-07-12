#KERN_INC_DIR=/home/mhorowitz/BUG_Stuff/2.0_kernel/bl-linux-omap-latest/bl-linux-omap/include/

KERN_INC_DIR?=/home/farmckon/dev_bl/linaro_kern/include/
JDK_INC_DIR?=/usr/lib/jvm/java-6-openjdk/include/
CC?=gcc

HAS_KERN_DIR=$(shell if [ -d $(KERN_INC_DIR) ]; then echo 'true' ; fi)
HAS_JDK_DIR=$(shell if [ -d $(JDK_INC_DIR) ]; then echo 'true' ; fi)


all:
ifneq ('$(HAS_JDK_DIR)','true')
	$(error "no jdk include dir')
endif	
ifneq ('$(HAS_KERN_DIR)','true')
	$(error "no kern include dir')
endif	

	$(CC) backend.c -shared -o libjavabugduino.so \
	-Wall -I$(JDK_INC_DIR) -I$(KERN_INC_DIR)
	$(CC) cmdline_bugduino_push.c -o program_bugduino \
	-Wall -I$(KERN_INC_DIR)

clean:
	rm program_bugduino libjavabugduino.so
