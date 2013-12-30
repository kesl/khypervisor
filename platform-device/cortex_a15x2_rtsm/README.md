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

# bmguset + bmgest

## Building the bmguest
<blockquote>
<pre>
$ cd platform-device/cortex_a15x2_rtsm/guestos/bmguest/
$ make
</pre>
</blockquote>

## Building the k-hypervisor for bmguest
<blockquote>
<pre>
$ cd platform-device/cortex_a15x2_rtsm
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest0.bin
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest1.bin
$ make
</pre>
</blockquote>

## Testing bmguest on RTSM
1. run rtsm
<pre>
RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/RTSM_VE_Cortex-A15x1-A7x1 --cadi-server
</pre>
2. run maxview
<pre>
ARM/FastModelsTools_8.2/bin/maxview
</pre>
3. load hvc-man-switch.axf

# bmguset + linux guest

## Building the linux guset
1. Download linux
<pre>
$ git submodule init
$ git submodule update
</pre>
2. Initial setup for linux
<pre>
$ cd guestos/linux
$ git checkout 7d1f9aeff1ee4a20b1aeb377dd0f579fe9647619 -b 3.8
$ git apply ../../patch/linux-fastmodels-config-add-minimal-linux-config.patch

</pre>
3. Building linux-arndale
<pre>
$ make ARCH=arm vexpress_minhw_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
$ cat host-a15.dtb >> arch/arm/boot/zImage
</pre>

## Building the k-hypervisor for linux guset
<blockquote>
<pre>
$ cd platform-device/cortex_a15x2_arndale
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/guest0.bin
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest1.bin
$ make LINUX=y
</pre>
</blockquote>

## Testing minimal linux on RTSM board using git submodule
1. run rtsm
<pre>
RTSM_A15-A7x14_VE/models/Linux64_GCC-4.1/RTSM_VE_Cortex-A15x1-A7x1 --cadi-server
</pre>
2. run maxview
<pre>
ARM/FastModelsTools_8.2/bin/maxview
</pre>
3. load hvc-man-switch.axf
