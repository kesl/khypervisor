# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.

# Usage: make
# Example:
#	$ make		; build for bmguest
#	$ make		; build for linux guest

# Include config file (prefer config.mk, fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

COMMON_SOURCE_DIR=../../../../common

OBJS = boot.o main.o loadlinux.o

OBJS += drivers/uart.o $(COMMON_SOURCE_DIR)/log/string.o 

GUESTIMG	= guestloader.axf
GUESTBIN	= guestloader.bin
LD_SCRIPT	= model.lds.S
INCLUDES	= -I. -I$(COMMON_SOURCE_DIR) -I$(COMMON_SOURCE_DIR)/include -I$(COMMON_SOURCE_DIR)/log

CPPFLAGS	+= $(INCLUDES)
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY	= $(CROSS_COMPILE)objcopy

# Guest type is GUEST_HYPMON by default or specified as an environment variable before running 'make'
GUESTTYPE?=GUEST_HYPMON

GUESTCONFIGS	=

ifeq ($(LINUX), y)
	GUESTCONFIGS += -DLINUX_GUEST
endif

GUESTCONFIGS += -D__MONITOR_CALL_HVC__ -DLDS_PHYS_OFFSET='0x80000000' -DLDS_GUEST_OFFSET='0x80000000' -DLDS_GUEST_STACK='0x8F000000'
GUESTCONFIGS += -DNUM_ITERATIONS=300 
GUESTCONFIGS += -DLDS_$(GUESTTYPE)=1
# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(GUESTBIN)
	@echo "================================================================="
	@echo "  BUILT GUEST TYPE:$(GUESTTYPE) "
	@echo "  GUESTCONFIGS FLAGS:$(GUESTCONFIGS) "
	@echo "================================================================="
	@echo "  Entry point physical address:"
	@nm $(GUESTIMG) | grep loader_start
	@nm $(GUESTIMG) | grep _binary
	@echo "================================================================="

# Build just the semihosting wrapper

clean distclean:
	rm -f $(GUESTIMG) $(GUESTBIN) \
	model.lds $(OBJS) 

$(GUESTIMG): $(OBJS) model.lds
	$(LD) -o $@ $(OBJS) --script=model.lds

$(GUESTBIN): $(GUESTIMG)
	$(OBJCOPY) -O binary -S $< $@

boot.o: boot.S
	@echo "================================================================="
	@echo "GUESTCONFIGS='$(GUESTCONFIGS)'"
	@echo "================================================================="
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -O2 -ffreestanding -I.  -c -o $@ $<

model.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -E -P -C -o $@ $<

force: ;

Makefile: ;

.PHONY: all clean distclean config.mk config-default.mk
