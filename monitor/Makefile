# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.

# Usage: make [UBOOT=y]
# Example:
# 	$ make UBOOT=y 	; # build for U-boot loading and armflash.bin generated
#
# 	$ make UBOOT=n 	; # build ELF image for FastModels loading: hvc-man-switch.axf
#	$ make 		; same as UBOOT=n

# Include config file (prefer config.mk, fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

LIBFDTOBJS      = libfdt/fdt.o libfdt/fdt_ro.o libfdt/fdt_wip.o \
		  libfdt/fdt_sw.o libfdt/fdt_rw.o libfdt/fdt_strerror.o
BOOTLOADER	= boot.S
OBJS 		= boot.o c_start.o string.o hmain.o \
              monitor.o monitor_secure.o monitor_imp.o monitor_imp_secure.o \
              mm.o timer.o \
              context.o devices/gic/gic.o interrupt.o smp.o scheduler.o devices/generic_timer/generic_timer.o stdio.o\
              devices/gic/vgic.o
LD_SCRIPT	= model.lds.S
INCLUDES    = -I./include

ifeq ($(SYSTEM), arndale)
OBJS 		+= devices/uart-exynos5250/uart_print.o devices/mct/mct.o
INCLUDES    +=  -I ./plat/exynos5250/arndale/
else # For VEXPRESS CA15
OBJS 		+= ./devices/uart-pl011/uart_print.o
INCLUDES    += -I ./plat/generic_ca15/rtsmve_ca15/
endif



GUESTBIN	= bmguest.bin

SEMIIMG 	= hvc-man-switch.axf
MONITORMAP	= monitor.map
#end of magicyaba

# Turn off BAREMETAL_GUEST flag to not embed bmguest.bin
ifeq ($(UBOOT),y)
CONFIG_FLAGS	= 
else
CONFIG_FLAGS	= -DBAREMETAL_GUEST 
endif

CPPFLAGS	+= $(CONFIG_FLAGS) $(INCLUDES)

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
NM		= $(CROSS_COMPILE)nm
#magicyaba 
#for armflash.bin
OBJCOPY		= $(CROSS_COMPILE)objcopy
DD		= dd
HYPBIN		= hvc-man-switch.bin
FLASHIMG	= armflash.bin
#magicyaba

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
ifeq ($(UBOOT),y)
all: $(FLASHIMG)
else
all: semi
endif
# Build just the semihosting wrapper
semi: $(SEMIIMG) $(MONITORMAP)

$(MONITORMAP): $(SEMIIMG)
	$(NM) $< > $@
	
clean distclean:
	rm -f $(MONITORMAP) $(SEMIIMG) $(HYPBIN) $(FLASHIMG) \
	model.lds modelsemi.lds $(OBJS) 

$(SEMIIMG): $(OBJS) modelsemi.lds
	$(LD) -o $@ $(OBJS) --script=modelsemi.lds

$(HYPBIN): $(SEMIIMG)
	$(OBJCOPY) -O binary $(SEMIIMG) $(HYPBIN) 

$(FLASHIMG): $(HYPBIN) $(GUESTBIN)
	$(DD) if=/dev/zero of=$(FLASHIMG) bs=1 count=3M
	$(DD) if=$(HYPBIN) of=$(FLASHIMG) conv=notrunc bs=1
	$(DD) if=$(GUESTBIN) of=$(FLASHIMG) conv=notrunc bs=1 seek=1M
	$(DD) if=$(GUESTBIN) of=$(FLASHIMG) conv=notrunc bs=1 seek=2M

$(GUESTBIN): 
	@echo "========================================"
	@echo "ERROR: Copy $@ from bmguest/ after building it"
	@echo "======================================="
	# Our intention is to cause an error and abort Makefile processing
	# Not actually removing the file
	@rm $@ 

boot.o: $(BOOTLOADER)
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

monitor.o: monitor.S
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -Wall -O2 -ffreestanding -I. -Ilibfdt -c -o $@ $<

model.lds: $(LD_SCRIPT) Makefile 
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

modelsemi.lds: $(LD_SCRIPT) Makefile $(GUESTBIN)
	$(CC) $(CPPFLAGS) -DSEMIHOSTING=1 -E -P -C -o $@ $<

# Pass any target we don't know about through to the kernel makefile.
# This is a convenience rule so we can say 'make menuconfig' etc here.
# Note that any rules in this file must have a command or be marked as
# .PHONY.
%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all semi clean distclean config.mk config-default.mk
