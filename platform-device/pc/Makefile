# Makefile
#
# Usage: make
# Example:
#   $ make	# build for pc version of khypervisor

# Include config file (prefer config.mk fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

.SUFFIXES: .c .o

PROJECT_ROOT_DIR=../..
HYPERVISOR_SOURCE_DIR=$(PROJECT_ROOT_DIR)/hypervisor
COMMON_SOURCE_DIR=$(PROJECT_ROOT_DIR)/common

OBJS		= main.o	\
	timer.o

SRCS		= main.c	\
	timer.c

INCLUDES	= -I $(PROJECT_ROOT_DIR)

INCLUDES	+= -I $(COMMON_SOURCE_DIR)	\
	-I $(COMMON_SOURCE_DIR)/include

INCLUDES	+= -I $(HYPERVISOR_SOURCE_DIR)	\
	-I $(HYPERVISOR_SOURCE_DIR)/include	\
	-I $(HYPERVISOR_SOURCE_DIR)/hal/arm32ve/include

CPPFLAGS	+= $(CONFIG_FLAGS) $(INCLUDES)

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
NM		= $(CROSS_COMPILE)nm

TARGET = pc

all: $(TARGET)
	
$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -Wall -O0 -o $@ $(OBJS)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean config.mk config-default.mk
