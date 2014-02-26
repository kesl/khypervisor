# Usage: make
# Example:
#	$ make		; build for bmguest
#	$ make		; build for linux guest

# Include config file 
include config-default.mk
COMMON_SOURCE_DIR=../../../../common
OBJS += boot.o main.o loadlinux.o drivers/uart.o $(COMMON_SOURCE_DIR)/log/string.o 
GUESTIMG	= guestloader.axf
GUESTBIN	= guestloader.bin
LD_SCRIPT	= model.lds.S
INCLUDES	= -I. -I$(COMMON_SOURCE_DIR) -I$(COMMON_SOURCE_DIR)/include 
CPPFLAGS	+= $(INCLUDES)
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY	= $(CROSS_COMPILE)objcopy
GUESTCONFIGS += -DLDS_PHYS_OFFSET='0x80000000' -DLDS_STACK='0x8F000000'
ifeq ($(LINUX), y)
	GUESTCONFIGS += -DLINUX_GUEST
endif
# Build all wrappers
all: $(GUESTBIN)
clean distclean:
	rm -f $(GUESTIMG) $(GUESTBIN) \
	model.lds $(OBJS) 
$(GUESTIMG): $(OBJS) model.lds
	$(LD) -o $@ $(OBJS) --script=model.lds
$(GUESTBIN): $(GUESTIMG)
	$(OBJCOPY) -O binary -S $< $@
boot.o: boot.S
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -DKCMD='$(KCMD)' -c -o $@ $<
%.o: %.c
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -O2 -ffreestanding -I.  -c -o $@ $<
model.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) $(GUESTCONFIGS) -E -P -C -o $@ $<
force: ;
Makefile: ;
.PHONY: all clean distclean config-default.mk
