.EXPORT_ALL_VARIABLES:

DIRS = arch/arm drivers init lib
TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
NM = nm

INCLUDES = -I. -I$(TOPDIR)/include

CFLAGS	= $(INCLUDES) -g -nostdinc -Wall -fno-builtin -fomit-frame-pointer -fPIC -fno-stack-protector
LDFLAGS = -static -nostdlib -nostartfiles -nodefaultlibs 
OCFLAGS = -O binary -R .note -R .comment -S
ELF32_LDFLAGS = -Wl,-T,ld-script
BIN_FILE = rtos
#CPPFLAGS	+= -march=armv7-a -marm
CFLAGS	+= -mcpu=cortex-a15 -marm
#CPPFLAGS	+= -DTHUMB2_KERNEL

OBJ_FILES = arch/arm/arm.o drivers/drivers.o init/init.o
LIBS = lib/lib.a kernel/kernel.a

all:
	for i in $(DIRS) ; do make -C $$i || exit $? ; done
	$(CC) $(LDFLAGS) -o $(BIN_FILE).elf $(ELF32_LDFLAGS) $(OBJ_FILES) $(LIBS) -lgcc
	$(OBJCOPY) $(OCFLAGS) $(BIN_FILE).elf $(BIN_FILE).bin
	$(NM) $(BIN_FILE).elf  |sort > $(BIN_FILE).map
	$(OBJDUMP) -D $(BIN_FILE).elf  > $(BIN_FILE).dis

force:

clean:
	for i in $(DIRS) ; do make -C $$i clean; done 
	rm $(BIN_FILE).elf 
	rm $(BIN_FILE).map
	rm $(BIN_FILE).dis
	rm $(BIN_FILE).bin
