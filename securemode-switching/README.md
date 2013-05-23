Build bmguest.bin first!
========================
<pre>
$ cd ../bmguest
$ GUESTTYPE=GUEST_SECMON make clean all
$ cp bmguest.bin ../securemode-switching
$ cd ../securemode-switching
$ make semi
</pre>

Building Secure Mode Switching Demo
===================================

Simply typing 'make' will not successfully make the Secure Mode Switching demo. Please give 'semi' as the argument to 'make' command.

<pre>
$ make semi
</pre>

Now you have the binary: secmon.axf

Running Secure Mode Switching Demo
==================================

<pre>
$ ./run_rtsm.sh
</pre>
- This runs the binary image, secmon.axf, in RTSM_VE_Cortex-A15x1-A7x1 Fast Model. Launch FastModels MaxView Debugger and connect, or
- Load and connect to a Debug Configuration from DS-5 Debugger. The Debug Configuration must be created to
    - Load the binary image, secmon.axf
    - RTSM_VE Bare Metal Debugging, with Cortex A15x1-A7x1

The Secure Monitor prints log using semi hosting like below:

<pre>
simon@simon-desktop:~/Virtualization/hypervisor/securemode-switching$ ./run_rtsm.sh 
terminal_0: Listening for serial connection on port 5000
terminal_1: Listening for serial connection on port 5001
terminal_2: Listening for serial connection on port 5002
terminal_3: Listening for serial connection on port 5003
INFO: mmc: no image file connected
[bootwrapper] Starting...
[sec] hello
[sec] hello
[sec] hello
[sec] hello
</pre>

On the other hand, the guest, bmguest.bin, prints log to UART. So the logs from the guest will be separately visible on an xterm, if you are running the FastModels/RTSM on a Linux with X11 Desktop.
<pre>
[guest0] starting...
[guest0] iteration 0x00000000
[guest0] iteration 0x00000001
[guest0] iteration 0x00000002
[guest0] done
</pre>

