# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.

# Usage: make
# Example:
# 	$ make LINUX=y 	; # build for linux kernel loading
#	$ make			; # build for bmguest loading

# Include config file (prefer config.mk, fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

HYPERVISOR_SOURCE_DIR=../../hypervisor
COMMON_SOURCE_DIR=../common

BOOTLOADER	= boot.S

OBJS 		= boot.o	\
	main.o				\
	$(HYPERVISOR_SOURCE_DIR)/monitor.o				\
	$(HYPERVISOR_SOURCE_DIR)/monitor_secure.o		\
	$(HYPERVISOR_SOURCE_DIR)/monitor_imp.o			\
	$(HYPERVISOR_SOURCE_DIR)/mm.o					\
	$(HYPERVISOR_SOURCE_DIR)/vmm.o					\
	$(HYPERVISOR_SOURCE_DIR)/lpae.o					\
	$(HYPERVISOR_SOURCE_DIR)/timer.o				\
	$(HYPERVISOR_SOURCE_DIR)/context.o				\
	$(HYPERVISOR_SOURCE_DIR)/gic/gic.o				\
	$(HYPERVISOR_SOURCE_DIR)/interrupt.o			\
	$(HYPERVISOR_SOURCE_DIR)/smp.o					\
	$(HYPERVISOR_SOURCE_DIR)/scheduler.o			\
	$(HYPERVISOR_SOURCE_DIR)/generic_timer/generic_timer.o	\
	$(HYPERVISOR_SOURCE_DIR)/gic/vgic.o				\
	$(HYPERVISOR_SOURCE_DIR)/trap.o					\
	$(HYPERVISOR_SOURCE_DIR)/sched_policy.o			\
	$(HYPERVISOR_SOURCE_DIR)/trap_dabort.o			\
	$(HYPERVISOR_SOURCE_DIR)/vdev/vdev.o			\
	$(HYPERVISOR_SOURCE_DIR)/vdev/vdev_sample.o		\
	$(HYPERVISOR_SOURCE_DIR)/virq.o					\
	$(HYPERVISOR_SOURCE_DIR)/vdev/vdev_gicd.o		\
	$(HYPERVISOR_SOURCE_DIR)/gic/slotpirq.o			\
	$(HYPERVISOR_SOURCE_DIR)/virqmap.o				\
	$(HYPERVISOR_SOURCE_DIR)/vdev/vdev_timer.o

OBJS 		+=	$(COMMON_SOURCE_DIR)/test/tests.o	\
	$(COMMON_SOURCE_DIR)/test/tests_gic_timer.o		\
	$(COMMON_SOURCE_DIR)/test/tests_vdev.o			\
	$(COMMON_SOURCE_DIR)/test/tests_malloc.o		\
	$(COMMON_SOURCE_DIR)/test/tests_vdev_timer.o		

OBJS 		+=	$(COMMON_SOURCE_DIR)/log/string.o	\
	$(COMMON_SOURCE_DIR)/log/format.o				\
	$(COMMON_SOURCE_DIR)/log/print.o				\

LD_SCRIPT	= model.lds.S

OBJS 		+= drivers/uart/uart_print.o

INCLUDES    = -I $(HYPERVISOR_SOURCE_DIR)/include -I $(HYPERVISOR_SOURCE_DIR)
INCLUDES    += -I. -I $(COMMON_SOURCE_DIR)
INCLUDES    +=  -I ./include

GUEST0BIN	= ./guestimages/guest0.bin
GUEST1BIN	= ./guestimages/guest1.bin

SEMIIMG 	= hvc-man-switch.axf
MONITORMAP	= monitor.map


CONFIG_FLAGS	= -DBAREMETAL_GUEST 

ifeq ($(LINUX), y)
    CONFIG_FLAGS	+= -DLINUX_GUEST
	OBJS += $(HYPERVISOR_SOURCE_DIR)/loadlinux.o
endif

CPPFLAGS	+= $(CONFIG_FLAGS) $(INCLUDES)

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
NM		= $(CROSS_COMPILE)nm

#for armflash.bin
OBJCOPY		= $(CROSS_COMPILE)objcopy
DD		= dd
HYPBIN		= hvc-man-switch.bin
FLASHIMG	= armflash.bin

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: semi
# Build just the semihosting wrapper
semi: $(SEMIIMG) $(MONITORMAP)

$(MONITORMAP): $(SEMIIMG)
	$(NM) $< > $@
	
clean distclean:
	rm -f $(MONITORMAP) $(SEMIIMG) $(HYPBIN) \
	model.lds modelsemi.lds $(OBJS) 

$(SEMIIMG): $(OBJS) modelsemi.lds
	$(LD) -o $@ $(OBJS) --script=modelsemi.lds

$(HYPBIN): $(SEMIIMG)
	$(OBJCOPY) -O binary $(SEMIIMG) $(HYPBIN) 

$(GUEST0BIN): 
	@echo "========================================"
	@echo "ERROR: Copy $@ from guestos/bmguest/ after building it"
	@echo "======================================="
	# Our intention is to cause an error and abort Makefile processing
	# Not actually removing the file
	@rm $@
	
$(GUEST1BIN): 
	@echo "========================================"
	@echo "ERROR: Copy $@ from guestos/bmguest/ after building it"
	@echo "======================================="
	# Our intention is to cause an error and abort Makefile processing
	# Not actually removing the file
	@rm $@ 


boot.o: $(BOOTLOADER)
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

monitor.o: monitor.S
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -Wall -O2 -ffreestanding -I. -c -o $@ $<

model.lds: $(LD_SCRIPT) Makefile 
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

modelsemi.lds: $(LD_SCRIPT) Makefile $(GUEST0BIN) $(GUEST1BIN)
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

