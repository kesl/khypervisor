How to build a baremetal guest
==============================
Makefile usage
--------------
<pre>
  Usage: $ GUESTTYPE=<GUEST_HYPMON | GUEST_SECMON> make clean all
  Example:
  - Building a Hyp Monitor guest: $ GUESTTYPE=GUEST_HYPMON make clean all
  - Building a Secure Monitor guest: $ GUESTTYPE=GUEST_SECMON make clean all
</pre>

1. Building a Hyp monitor guest
------------------------------
- The program (bmguest.bin) runs in Non-secure Supervisor mode as a guest of the Hyp monitor
- Physical Entry point: IPA:0x00000000
- Hyp call (hvc instruction) to request the Hyp monitor to switch the context manually
<pre>
 $ make
 or
 $ GUESTTYPE=GUEST_HYPMON make
arm-unknown-linux-gnueabi-gcc -DSMP -mcpu=cortex-a15 -DVEXPRESS -g -D__MONITOR_CALL_HVC__ -DLDS_PHYS_OFFSET='0x00000000' -DLDS_GUEST_OFFSET='0x00000000' -DLDS_GUEST_STACK='0x0F000000' -DNUM_ITERATIONS=3 -DGUEST_LABEL='"[guest0] "' -DLDS_GUEST_HYPMON=1 -DKCMD='"console=ttyAMA0 mem=512M mem=512M@0x880000000 earlyprintk root=/dev/nfs nfsroot=192.168.0.32:/srv/nfsroot,tcp rw ip=dhcp nfsrootdebug"' -c -o guest.o guest.S
...
arm-unknown-linux-gnueabi-ld -o bmguest.axf guest.o c_start.o string.o uart_print.o --script=model.lds
arm-unknown-linux-gnueabi-objcopy -O binary -S bmguest.axf bmguest.bin
=================================================================
  BUILT GUEST TYPE:GUEST_HYPMON 
  GUESTCONFIGS FLAGS:-D__MONITOR_CALL_HVC__ -DLDS_PHYS_OFFSET='0x00000000' -DLDS_GUEST_OFFSET='0x00000000' -DLDS_GUEST_STACK='0x0F000000' -DNUM_ITERATIONS=3 -DGUEST_LABEL='[guest0] ' -DLDS_GUEST_HYPMON=1 
=================================================================
  Entry point physical address:
00000000 A __hypmon_guest_start
</pre>

2. Building a Secure monitor guest
-------------------------------
- The program (bmguest.bin) runs in Non-secure Supervisor mode along side of the Monitor runs in Secure mode
- Physical Entry point: 0xE0000000
- Secure Monitor Call (smc instruction) to request the Secure Monitor to hand over the execution control to the Secure World
<pre>
 $ GUESTTYPE=GUEST_SECMON make
arm-unknown-linux-gnueabi-gcc -DSMP -mcpu=cortex-a15 -DVEXPRESS -g -DLDS_PHYS_OFFSET='0xE0000000' -DLDS_GUEST_OFFSET='0xE0000000' -DLDS_GUEST_STACK='0xEF000000' -DNUM_ITERATIONS=3 -DGUEST_LABEL='"[guest0] "' -DLDS_GUEST_SECMON=1 -DKCMD='"console=ttyAMA0 mem=512M mem=512M@0x880000000 earlyprintk root=/dev/nfs nfsroot=192.168.0.32:/srv/nfsroot,tcp rw ip=dhcp nfsrootdebug"' -c -o guest.o guest.S
...
=================================================================
  BUILT GUEST TYPE:GUEST_SECMON 
  GUESTCONFIGS FLAGS:-DLDS_PHYS_OFFSET='0xE0000000' -DLDS_GUEST_OFFSET='0xE0000000' -DLDS_GUEST_STACK='0xEF000000' -DNUM_ITERATIONS=3 -DGUEST_LABEL='[guest0] ' -DLDS_GUEST_SECMON=1 
=================================================================
  Entry point physical address:
e0000000 A __secmon_guest_start

</pre>
