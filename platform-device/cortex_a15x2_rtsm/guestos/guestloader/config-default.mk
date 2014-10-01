#
# Main options
#
CROSS_COMPILE	= arm-linux-gnueabihf-
ARCH			= arm
#CPPFLAGS        += -DVEXPRESS
CPPFLAGS		+= -mcpu=cortex-a15 -marm -g
