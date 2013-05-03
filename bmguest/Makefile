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

OBJS 		= guest.o c_start.o semihosting.o string.o 

GUESTIMG 	= bmguest.axf
GUESTBIN	= bmguest.bin
LD_SCRIPT	= model.lds.S

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(GUESTBIN)
	@echo "================================================================="
	@echo "Copy $(GUESTBIN) to securemode-switching/ to load it as the guest"
	@echo "Example: $$ cp $(GUESTBIN) ../securemode-switching"
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
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -O2 -ffreestanding -I.  -c -o $@ $<

model.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<


force: ;

Makefile: ;

.PHONY: all clean distclean config.mk config-default.mk
