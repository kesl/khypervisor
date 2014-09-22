# Hypervisor for ARMv7 Virtualization Extensions

## Initializing a Build Environment

### Download CodeBendch Lite Compiler for u-boot

- Compiler version: Sourcery CodeBench Lite 2013.05-23(gcc version: 4.7.3)

1. Access CodeBench page in Mentor Graphics(R) site
    http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/
2. Find ARM Processor part in the page and Click Download the EABI Release
    http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/request?id=e023fac2-e611-476b-a702-90eabb2aeca8&downloadlite=scblite2012&fmpath=/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/form
3. Fill the form about your information(name, email, etc), then you will receive the download URL of your email

### Download Tool chain for k-hypervisor
- ARM Toolchain Shipped with DS-5: <i>arm-linux-gnueabihf-</i>
- or running apt-get install like this
<pre>
sudo apt-get install gcc-arm-linux-gnueabihf
</pre>

# Download hypervisor
<pre>
$ git clone https://github.com/kesl/khypervisor.git
$ cd khypervisor
$ git submodule init
$ git submodule update
$ ./scripts/apply_patch.sh
</pre>

# How to test android + bmguest
<pre>
$ source platform-device/cortex_a15x2_rtsm/build/android_bmguest.sh
$ ./script/build_android_vexpress_mmc.sh
$ make
$ ./script/run_android_rtsm.sh
</pre>

# How to test bmguest + bmguest

## Make a build in one step continuous integration
Go to "Testing bmguest on RTSM"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_rtsm/build/bmguest_bmguest.sh
$ make
</pre>

## Building the bmguest
<pre>
$ cd platform-device/cortex_a15x2_rtsm/guestos/bmguest/
$ make
</pre>

## Building the guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest0.bin
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>
</pre>

## Building the k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm
$ make
</pre>

## Testing bmguest on RTSM
1. use rtsm
<pre>
RTSM_VE_Cortex-A15x1-A7x1 -a coretile.cluster0.cpu0=hvc-man-switch.axf
</pre>
2. use rtsm with maxview
<pre>
RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/RTSM_VE_Cortex-A15x1-A7x1 --cadi-server
</pre>
 3. run maxview
<pre>
ARM/FastModelsTools_8.2/bin/maxview
</pre>
 4. load hvc-man-switch.axf

# How to test bmguset + linux guest

## Make a build in one step continuous integration
Go to "Testing bmguest+linuxguest on RTSM"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_rtsm/build/linux_bmguest.sh
$ make
</pre>

## Building the bmguest
<pre>
$ cd platform-device/cortex_a15x2_rtsm/guestos/bmguest/
$ make
</pre>

## Building the linux guset
1. Building linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm/guestos/linux
$ cp ../../linuxguest/fs.cpio .
$ cp ../../linuxguest/host-a15.dtb .
$ make ARCH=arm vexpress_minhw_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
$ cat host-a15.dtb >> arch/arm/boot/zImage
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/zImage
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm/guestos/guestloader
$ make LINUX=y
$ cp guestloader.bin ../../guestimages/guest0.bin
</pre>
3. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_rtsm/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Building the k-hypervisor
<pre>
$ cd platform-device/cortex_a15x2_rtsm
$ make
</pre>

## Testing bmguest on RTSM
1. use rtsm
<pre>
RTSM_VE_Cortex-A15x1-A7x1 -a coretile.cluster0.cpu0=hvc-man-switch.axf
</pre>
2. use rtsm with maxview
<pre>
RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/RTSM_VE_Cortex-A15x1-A7x1 --cadi-server
</pre>
 3. run maxview
<pre>
ARM/FastModelsTools_8.2/bin/maxview
</pre>
 4. load hvc-man-switch.axf
