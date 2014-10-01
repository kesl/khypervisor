# Configuration file included in Makefile
#
# Copyright (C) 2011 Columbia University. All rights reserved.
# 		     Christoffer Dall <cdall@cs.columbia.edu>
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.
#
# This is a sample configuration file. To make changes, copy this file to
# config.mk and modify that file.
#
# For all systems you can override USE_INITRD and KCMD from the command-line.
#

###########################################################################
# Main options
#
CROSS_COMPILE	?= arm-linux-gnueabihf-
ARCH		?= arm

SYSTEM ?= vexpress

CPPFLAGS	+= -DSMP -D_SMP_
#CPPFLAGS    += -D_VEXPRESS_
CPPFLAGS	+= -mcpu=cortex-a15 -marm
CPPFLAGS	+= -g
