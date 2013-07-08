boot-wrapper: Start Linux kernels under ARM Fast Models

The boot-wrapper is a fairly simple implementation of a boot loader
intended to run under an ARM Fast Model and boot Linux.

License
=======

The boot-wrapper is generally under a 3 clause BSD license
(see LICENSE.txt for details). Note that some source files
are under similar but compatible licenses. In particular
libfdt is dual-license GPL/2-clause-BSD.

Compilation
===========

The expected method of building is to cross-compile on an
x86 box. You'll need an ARM cross-compiler. On Ubuntu you
can get this by installing the packages:
 gcc-4.6-arm-linux-gnueabi binutils-arm-linux-gnueabi
 libc6-armel-cross linux-libc-dev-armel-cross gcc-arm-linux-gnueabi
 libc6-dev-armel-cross cpp-arm-linux-gnueabi

The boot-wrapper can be compiled in two ways:
 (1) as a small standalone binary which uses the model's semihosting
 ABI to load a kernel (and optionally initrd and flattened device tree)
 when you run the model
 (2) with a specific kernel and initrd compiled into the binary;
 this is less flexible but may be useful in some situations

For case (1) you can just run:
 make CROSS_COMPILE=arm-linux-gnueabi- semi
which will build "linux-system-semi.axf".
(As with a Linux kernel cross-compile, the CROSS_COMPILE
variable is set to the prefix of the cross toolchain.
"arm-linux-gnueabi-" matches the prefix used by the Ubuntu
cross toolchain.)

For case (2) you'll need a Linux kernel tree to hand; the
boot-wrapper makefile will automatically look into it to
extract the kernel. By default this tree is assumed to be in
"../linux-kvm-arm". Assuming you have that tree set up and
have built a kernel in it, you can run:
 make CROSS_COMPILE=arm-linux-gnueabi-
which will build "linux-system.axf".

You can configure the makefile system by copying config-default.mk
to config.mk and editing it. This is only likely to be useful for
case (2); see the comments in config-default.mk for more information.

Running
=======

To run a model with a linux-system-semi.axf:

RTSM_VE_Cortex-A15x1 linux-system-semi.axf -C cluster.cpu0.semihosting-cmd_line="--kernel /path/to/zImage [--initrd /path/to/initrd] [--dtb /path/to/dtb] [-- kernel command line arguments]"

The paths to the kernel, initrd and device tree blob should all be
host filesystem paths. The initrd and dtb are both optional. Any text
following '--' is passed to the kernel as its command line; this is
also optional.

You may also want to pass other options to the model (for instance
to enable networking); these are not described here. See the Fast
Models documentation for more information.

Running a linux-system.axf is the same, except that since all
the files are built in there's no need to pass a command line:

RTSM_VE_Cortex-A15x1 linux-system.axf

Passing a command line to linux-system.axf is allowed, and any
kernel/initrd/dtb/commandline specified will override the compiled-in
version.
