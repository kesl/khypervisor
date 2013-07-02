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
        - Implementation Should Begin Shortly
        - Directory name changed from hvc-manualswitching/
- test/
    - Hvc_man_switch.axf binary is divide hypervisor and guest OSes
    - U-boot support 

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

3. Run and Debug on DS-5 or FastModels MaxView + RTSM VE Cortext A15-A7
<pre>
$ maxview &
<i> Under securemode-switching/ directory </i>
$ ./run_rtsm.sh
</pre>
- From Max View Debugger, connect to "the" model running, load securemode-switching/secmon.axf and run to start debugging

Testing Hypervisor Prototype 2 with u-boot
==========================================
1. Build u-boot 
<pre>
$ git clone git://git.denx.de/u-boot.git
$ make vexpress_ca15_tc2_config; make or make vexpress_ca15_tc2
</pre>

2. Build bmguest/
<pre>
same as 1. Build bmguest/ in Testing Secure/Non-secure World Switching
</pre>

3. Build Hypervisor Prototype 2: armflash.bin
<pre>
$ cd monitor
$ make
</pre>

4.Loading armflash.bin on FastModels MaxView + RTSM VE Cortext A15-A7
<pre>
$ RTSM_VE_Cortex-A15x1-A7x1 --cadi-server -C motherboard.flashloader0.fname=$(HYP)/monitor/armflash.bin &
<i> $(HYP) is absolute path of hypervisor prototype 2 </i>
$ maxview &
</pre>

5.Copy hypervisor prototype 2 flash image to main memory 
<pre>
<i> in u-boot prompt </i>
VExpress# cp.b 0x8000000 0xf0000000 0x100000
VExpress# cp.b 0x8100000 0xa0000000 0x100000
VExpress# cp.b 0x8200000 0xb0000000 0x100000
</pre>


Tool chain
=============
- ARM Toolchain Shipped with DS-5

arm-linux-gnueabihf-
