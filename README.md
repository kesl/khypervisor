Sub-projects
============
- securemode-switching/
    - boot-wrapper based modification for TrstZone Secure/Non-Secure world switching example

- bmguest/
    - Bare Metal Guest Application
    - entry: 0x00000000 
    - UART Print is not working due to the UART address is not reachable (PA@0x1???????)

- monitor/
    - Prototype 1
        - 2 bare metal guests
        - hvc call for manual guest switching
        - LPAE stage 2 address translation
    - Prototype 2
        - Directory name changed from hvc-manualswitching/
        - Implementation On-going
            - Interrupt Handling through GICv2
            - Generic Timer and Scheduler (Round-robin)
            - Boot loader

Building Hypervisor (Monitor)
-----------------------------

<code>
$ cd monitor
$ make UBOOT=y ; Build image for U-boot loading: armflash.bin
or
$ make UBOOT=n ; Build ELF image: hvc-man-switch.axf (can be loaded from FastModels as an application)

$ make ; without UBOOT variable is equivalent as UBOOT=n
</code>

Testing Secure/Non-secure World Switching, Prototype 1 Phase 2
==============================================================
1. Build bmguest/
<pre>
$ cd bmguest
$ GUESTTYPE=GUEST_SECMON make
$ cp bmguest.bin ../securemode-switching
</pre>

2. Build Secure Monitor
<pre>
$ cd securemode-switching
$ make semi
</pre>

3. Run and Debug on DS-5 or FastModels MaxView + RTSM VE Cortex A15-A7
<pre>
$ maxview &
<i> Under securemode-switching/ directory </i>
$ ./run_rtsm.sh
</pre>
- From Max View Debugger, connect to "the" model running, load securemode-switching/secmon.axf and run to start debugging

Testing Hypervisor Prototype 2 with u-boot
==========================================
Project branch information: test
u-boot version information
- tag: v2013.07-rc2
- commit hash: e6bf18dba2a21bebf2c421b1c2e188225f6485a1
- target board: vexpress_ca15_tc2
- compiler: Sourcery CodeBench Lite 2013.05-23(gcc version: 4.7.3)
Directory infomation
- HYP: project root directory
- UBOOT: uboot root directory



1. Build u-boot 
<pre>
$ git clone git://git.denx.de/u-boot.git
$ make vexpress_ca15_tc2_config; make 
or 
make vexpress_ca15_tc2
</pre>

2. Build bmguest/
<code>
$ cd bmguest/
$ make GUESTTYPE=GUEST_HYPMON
$ cp bmguest.bin ../monitor
</code>

3. Build Hypervisor Prototype 2: armflash.bin
<pre>
$ cd monitor
$ make UBOOT=y
</pre>

4.Loading armflash.bin on FastModels MaxView + RTSM VE Cortex A15-A7
<pre>
- Loading flash image with RTSM VE 
$ RTSM_VE_Cortex-A15x1-A7x1 --cadi-server -C motherboard.flashloader0.fname=$(HYP)/monitor/armflash.bin &
- Loading u-boot on Maxview
$ maxview &
- From Maxview Debugger, connect to "the" model running, load u-boot and run to start debugging
<i> how to load application code in maxview </i>
Click the file tab-load application code 
select '$(UBOOT)/u-boot'(without file extension; filetype: data)
Click the run icon in toolbar
</pre>

5.Copy hypervisor prototype 2 flash image to main memory 
<pre>
<i> in u-boot prompt </i>
VExpress# cp.b 0x8000000 0xf0000000 0x100000; copy hypervisor from flash@0x800_0000 to DRAM@0xf000_0000
VExpress# cp.b 0x8100000 0xa0000000 0x100000; copy guest os#1 from flash@0x810_0000 to DRAM@0xA000_0000
VExpress# cp.b 0x8200000 0xb0000000 0x100000; copy guest os#2 from flash@0x820_0000 to DRAM@0xB000_0000
VExpress# go 0xf000004c; this address is entry point of hypervisor
</pre>


Tool chain
=============
- ARM Toolchain Shipped with DS-5

arm-linux-gnueabihf-
