include ../../config-default.mk

#
# Main options
#
#CROSS_COMPILE	= arm-linux-gnueabihf-
#ARCH			= arm
CPPFLAGS		= -mcpu=cortex-a15 -marm -g
