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
            - Generic Timer and Scheduler
            - Boot loader

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


Tool chain
=============
- ARM Toolchain Shipped with DS-5

arm-linux-gnueabihf-
