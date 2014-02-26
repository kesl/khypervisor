# Usage: make
# Example:
#	$ make	     	; build for bmguest
#	$ make LINUX=y	; build for linux guest

# Include config file
include config-default.mk
COMMON_SOURCE_DIR=../../../../common
OBJS += boot.o main.o loadlinux.o drivers/uart.o $(COMMON_SOURCE_DIR)/log/string.o
GUESTLOADERIMG	= guestloader.axf
GUESTLOADERBIN	= guestloader.bin
LD_SCRIPT	= model.lds.S
INCLUDES	= -I. -I$(COMMON_SOURCE_DIR) -I$(COMMON_SOURCE_DIR)/include
CPPFLAGS	+= $(INCLUDES)
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY	= $(CROSS_COMPILE)objcopy
GUESTLOADERCONFIGS += -DLDS_PHYS_OFFSET='0x80000000' -DLDS_STACK='0x8F000000'
ifeq ($(LINUX), y)
	GUESTLOADERCONFIGS += -DLINUX_GUEST
endif
all: $(GUESTLOADERBIN)
clean distclean:
	rm -f $(GUESTLOADERIMG) $(GUESTLOADERBIN) \
	model.lds $(OBJS)
$(GUESTLOADERIMG): $(OBJS) model.lds
	$(LD) -o $@ $(OBJS) --script=model.lds
$(GUESTLOADERBIN): $(GUESTLOADERIMG)
	$(OBJCOPY) -O binary -S $< $@
boot.o: boot.S
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -DKCMD='$(KCMD)' -c -o $@ $<
%.o: %.c
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -O2 -ffreestanding -I.  -c -o $@ $<
model.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) $(GUESTLOADERCONFIGS) -E -P -C -o $@ $<
force: ;
Makefile: ;
.PHONY: all clean distclean config-default.mk
