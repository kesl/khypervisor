# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.


# Include config file (prefer config.mk, fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

OBJS 		= guest.o c_start.o string.o exception.o gic.o uart_print.o io_emulation_exam.o

GUESTIMG 	= bmguest.axf
GUESTBIN	= bmguest.bin
LD_SCRIPT	= model.lds.S
INCLUDES    = -I. -I../common/include

CPPFLAGS    += $(INCLUDES)
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

# Guest type is GUEST_HYPMON by default or specified as an environment variable before running 'make'
#ifneq ($(strip $(GUESTTYPE)), "")
#GUESTTYPE	= GUEST_HYPMON
#endif
GUESTTYPE?=GUEST_HYPMON
# Possible values supported are:
#GUESTTYPE	= GUEST_SECMON
#GUESTTYPE	= GUEST_HYPMON

GUESTCONFIGS	=

ifeq ($(GUESTTYPE),GUEST_HYPMON)
GUESTCONFIGS	= -D__MONITOR_CALL_HVC__ -DLDS_PHYS_OFFSET='0x80000000' -DLDS_GUEST_OFFSET='0x80000000' -DLDS_GUEST_STACK='0x8F000000'
else
# if GUESTTYPE == GUEST_SECMON, 
#	"smc #0" will be used to switch instead of hvc #??
GUESTCONFIGS	= -DLDS_PHYS_OFFSET='0xE0000000' -DLDS_GUEST_OFFSET='0xE0000000' -DLDS_GUEST_STACK='0xEF000000'
endif
GUESTCONFIGS += -DNUM_ITERATIONS=300 -DGUEST_LABEL='"[guest0] "'
GUESTCONFIGS += -DLDS_$(GUESTTYPE)=1


#GUESTCONFIGS	= -D__MONITOR_CALL_HVC__
# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(GUESTBIN)
	@echo "================================================================="
	@echo "  BUILT GUEST TYPE:$(GUESTTYPE) "
	@echo "  GUESTCONFIGS FLAGS:$(GUESTCONFIGS) "
	@echo "================================================================="
	@echo "  Entry point physical address:"
	@nm $(GUESTIMG) | grep _guest_start
	@echo "================================================================="
	@echo "Copy $(GUESTBIN) to securemode-switching/ to load it as the guest"
	@echo "Example: $$ cp $(GUESTBIN) ../securemode-switching"
	@echo "================================================================="
	@echo "  Usage: $$ GUESTTYPE=<GUEST_HYPMON | GUEST_SECMON> make clean all"
	@echo "  Example:"
	@echo "  - Building a Hyp Monitor guest: $$ GUESTTYPE=GUEST_HYPMON make clean all"
	@echo "  - Building a Secure Monitor guest: $$ GUESTTYPE=GUEST_SECMON make clean all"
	@echo "================================================================="

# Build just the semihosting wrapper

clean distclean:
	rm -f $(GUESTIMG) $(GUESTBIN) \
	model.lds $(OBJS) 

$(GUESTIMG): $(OBJS) model.lds
	$(LD) -o $@ $(OBJS) --script=model.lds

$(GUESTBIN): $(GUESTIMG)
	$(OBJCOPY) -O binary -S $< $@

guest.o: guest.S
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
