# Usage: make
# Example:
#	$ make	     	; build for bmguest
#	$ make LINUX=y	; build for linux guest

# Include config file
include config-default.mk
COMMON_SOURCE_DIR=../../../../common
COMMON_LOADER_DIR=$(COMMON_SOURCE_DIR)/guest/loader
OBJS += boot.o main.o drivers/uart.o drivers/pl011.o drivers/timer.o\
	$(COMMON_SOURCE_DIR)/log/print.o \
	$(COMMON_SOURCE_DIR)/log/format.o \
	$(COMMON_SOURCE_DIR)/log/string.o \
	$(COMMON_SOURCE_DIR)/guest/core/exception.o \
	$(COMMON_SOURCE_DIR)/guest/core/gic.o \
	$(COMMON_SOURCE_DIR)/guest/test/test_vtimer.o \
	$(COMMON_LOADER_DIR)/linuxloader.o \
	$(COMMON_LOADER_DIR)/guestloader_common.o \
	$(COMMON_LOADER_DIR)/cli.o \
	$(COMMON_LOADER_DIR)/monitor/monitor_cli.o \
	$(COMMON_LOADER_DIR)/monitor/guest_monitor.o
GUESTLOADERIMG	= guestloader.axf
GUESTLOADERBIN	= guestloader.bin
LD_SCRIPT	= model.lds.S
INCLUDES	= -I. -I$(COMMON_SOURCE_DIR) -I$(COMMON_SOURCE_DIR)/include \
			 -I$(COMMON_LOADER_DIR) -I$(COMMON_SOURCE_DIR)/guest \
			 -I$(COMMON_SOURCE_DIR)/guest/core -I../../ \
			 -I$(COMMON_LOADER_DIR)/monitor

CPPFLAGS	+= $(INCLUDES)
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY	= $(CROSS_COMPILE)objcopy
GUESTLOADERCONFIGS = -DBM_GUEST
ifeq ($(LINUX), y)
GUESTLOADERCONFIGS = -DLINUX_GUEST
GUESTBIN	= ../../guestimages/zImage
else
GUESTBIN	= ../../guestimages/bmguest.bin
endif
ifeq ($(MONITOR), y)
GUESTLOADERCONFIGS += -DMONITOR_GUEST
endif
GUESTLOADERCONFIGS += -DGUEST_PATH=$(GUESTBIN)
ifeq ($(ANDROID), y)
GUESTLOADERCONFIGS += -DINITRD_PATH=../android_boot/uInitrd -DUSE_ANDROID_INITRD
endif
#GUEST_NUMBER = "GUEST0"
GUESTLOADERCONFIGS += -DGUEST_NUMBER=$(GUEST_NUMBER)
all: $(GUESTLOADERBIN)
clean distclean:
	rm -f $(GUESTLOADERIMG) $(GUESTLOADERBIN) \
	model.lds $(OBJS)
$(GUESTLOADERIMG): $(OBJS) model.lds
	$(LD) -o $@ $(OBJS) --script=model.lds
$(GUESTLOADERBIN): $(GUESTLOADERIMG)
	$(OBJCOPY) -O binary -S $< $@
ifeq ($(LINUX), y)
$(GUESTBIN):
	@echo "ERROS: Copy $@ from guestos/linux/arch/arm/boot/ after building it"
else
$(GUESTBIN):
	@echo "ERROS: Copy $@ from guestos/bmguest/ after building it"
endif
boot.o: boot.S
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -DKCMD='$(KCMD)' -c -o $@ $<
%.o: %.c
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -O2 -ffreestanding -I.  -c -o $@ $<
model.lds: $(LD_SCRIPT) Makefile $(GUESTBIN)
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -E -P -C -o $@ $<
force: ;
Makefile: ;
.PHONY: all clean distclean config-default.mk
