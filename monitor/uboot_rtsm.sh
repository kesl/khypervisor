#!/bin/sh
LD_LIBRARY_PATH=/usr/local/RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/ RTSM_VE_Cortex-A15x1-A7x1 --cadi-server -C motherboard.flashloader0.fname=$PWD/armflash.bin
