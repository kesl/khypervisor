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
OBJS 		= boot.o monitor.o guest.o c_start.o semihosting.o string.o semi_loader.o $(LIBFDTOBJS)
KERNEL		= uImage

IMAGE		= linux-system.axf
SEMIIMG 	= linux-system-semi.axf
LD_SCRIPT	= model.lds.S


CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(IMAGE) $(SEMIIMG)

# Build just the semihosting wrapper
semi: $(SEMIIMG)

clean distclean:
	rm -f $(IMAGE) $(SEMIIMG) \
	model.lds modelsemi.lds $(OBJS) $(KERNEL)

$(KERNEL): $(KERNEL_SRC)/arch/arm/boot/uImage
	cp $< $@

$(IMAGE): $(OBJS) model.lds $(KERNEL) $(FILESYSTEM) Makefile
	$(LD) -o $@ $(OBJS) --script=model.lds

$(SEMIIMG): $(OBJS) modelsemi.lds
	$(LD) -o $@ $(OBJS) --script=modelsemi.lds

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

modelsemi.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -DSEMIHOSTING=1 -E -P -C -o $@ $<

$(KERNEL_SRC)/arch/arm/boot/uImage: force
	$(MAKE) -C $(KERNEL_SRC) -j4 uImage

# Pass any target we don't know about through to the kernel makefile.
# This is a convenience rule so we can say 'make menuconfig' etc here.
# Note that any rules in this file must have a command or be marked as
# .PHONY.
%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all semi clean distclean config.mk config-default.mk
