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
KERNEL_SRC	?= ../linux-kvm-arm

# Select system:
# mps:		MPS (Cortex-M3)
# realview_eb:	RealViewPB, EB, etc.
# vexpress:	Versatile Express
# arndale:	arndale board
#SYSTEM ?= arndale
SYSTEM ?= vexpress

###########################################################################
# Turn this on to use an initrd whose contents are in filesystem.cpio.gz
USE_INITRD ?= no
ifeq ($(USE_INITRD),yes)
CPPFLAGS	+= -DUSE_INITRD
FILESYSTEM	?= filesystem.cpio.gz
else
FILESYSTEM =
endif

###########################################################################
# Default NFS root
NFS_ROOT	?= /srv/nfsroot
ifeq ($(origin NFS_SERVER), undefined)
NFS_SERVER	:= $(shell ip addr show scope global | \
		   sed -ne '/inet/{s/ *inet \([^/]*\)\/.*/\1/p;q}')
endif


###########################################################################
# MPS (Cortex-M3) definitions
#
ifeq ($(SYSTEM),mps)
# C-flags
CPPFLAGS	+= -DMACH_MPS -DTHUMB2_KERNEL
CPPFLAGS	+= -march=armv7-m
CPPFLAGS	+= -mthumb -Wa,-mthumb -Wa,-mimplicit-it=always

# Kernel command line
KCMD ?= "rdinit=/bin/sh console=ttyAMA3 mem=4M earlyprintk"
endif # SYSTEM = mps


###########################################################################
# EB, RealviewPB, etc
#
ifeq ($(SYSTEM),realview_eb)

#CPPFLAGS	+= -DSMP
#CPPFLAGS	+= -march=armv7-a -marm
CPPFLAGS	+= -mcpu=cortex-a15
#CPPFLAGS	+= -DTHUMB2_KERNEL

# Default kernel command line, using initrd:
ifeq ($(USE_INITRD),yes)
	KCMD ?= "console=ttyAMA0 mem=256M earlyprintk"
endif
#
# Default kernel command line, without initrd:
ifneq ($(USE_INITRD),yes)
	KCMD ?= "root=/dev/nfs nfsroot=$(NFS_HOST):$(NFS_ROOT) ip=dhcp console=ttyAMA0 mem=256M earlyprintk"
endif
endif # SYSTEM = realvire_eb


###########################################################################
# Versatile Express
#
ifeq ($(SYSTEM),arndale)

CPPFLAGS	+= -DSMP
CPPFLAGS	+= -mcpu=cortex-a15 -marm
CPPFLAGS	+= -g

endif
###########################################################################
# Versatile Express
#
ifeq ($(SYSTEM),vexpress)

CPPFLAGS	+= -DSMP
CPPFLAGS	+= -mcpu=cortex-a15 -marm
CPPFLAGS	+= -g

# Default kernel command line, using initrd:
ifeq ($(USE_INITRD),yes)
	KCMD ?= "console=ttyAMA0 mem=512M mem=512M@0x880000000 earlyprintk ip=dhcp"
endif
#
# Default kernel command line, without initrd:
ifneq ($(USE_INITRD),yes)
	KCMD ?= "console=ttyAMA0 mem=512M mem=512M@0x880000000 earlyprintk root=/dev/nfs nfsroot=$(NFS_SERVER):$(NFS_ROOT),tcp rw ip=dhcp nfsrootdebug"
endif
endif # SYSTEM = vexpress
