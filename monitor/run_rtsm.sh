#!/bin/sh
LD_LIBRARY_PATH=/usr/local/RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/ RTSM_VE_Cortex-A15x1-A7x1 --cadi-server -a coretile.cluster0.cpu0=hvc-man-switch.axf
#modeldebugger /home/simon/ARM/FastModelsPortfolio_8.0/examples/RTSM_VE/Build_Cortex-A15x1/Linux64-Release-GCC-4.4/cadi_system_Linux64-Release-GCC-4.4.so /home/simon/Virtualization/boot-wrapper/linux-system-semi.axf
