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

LIBFDTOBJS      = libfdt/fdt.o libfdt/fdt_ro.o libfdt/fdt_wip.o \
		  libfdt/fdt_sw.o libfdt/fdt_rw.o libfdt/fdt_strerror.o
BOOTLOADER	= boot.S
OBJS 		= boot.o monitor.o c_start.o string.o hyp_main.o monitor_imp.o uart_print.o guest.o mmu.o
GUESTBIN	= bmguest.bin

SEMIIMG 	= hvc-man-switch.axf
LD_SCRIPT	= model.lds.S
CONFIG_FLAGS	= -DBAREMETAL_GUEST
CPPFLAGS	+= $(CONFIG_FLAGS)

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(SEMIIMG)

# Build just the semihosting wrapper
semi: $(SEMIIMG)

clean distclean:
	rm -f $(SEMIIMG) \
	model.lds modelsemi.lds $(OBJS) 

$(SEMIIMG): $(OBJS) modelsemi.lds
	$(LD) -o $@ $(OBJS) --script=modelsemi.lds

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

guest.o: guest.S
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -O2 -ffreestanding -I. -Ilibfdt -c -o $@ $<

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
