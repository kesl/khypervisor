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


# bmguset + linux guest

## Building u-boot

1. Download u-boot
<pre>
$ cd platform-device/cortex_a15x2_arndale
$ git submodule init
$ git submodule update
$ cd u-boot-native
</pre>
2. patching a autoboot for u-boot-native
<pre>
$ git checkout lue_arndale_13.1
$ git apply ../patch/u-boot-bootz.patch
</pre>
3. Building u-boot-arndale
<pre>
$ make arndale5250
</pre>

## Building the linux guset
1. Download linux
<pre>
$ git submodule init
$ git submodule update
</pre>
2. Initial setup for linux
<pre>
$ cd guestos/linux
$ git checkout origin/exynos-jb -b exynos-jb
$ git apply ../../patch/linux-arndale-config-add-minimal-linux-config.patch
$ git apply ../../patch/arndale-change-kernel-sdram-address-uart-port-2-1.patch
</pre>
3. Building linux-arndale
<pre>
$ make ARCH=arm arndale_minimal_linux_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
</pre>

## Building the k-hypervisor for linux guset
1. Copy guest image to guestimages directory
<pre>
$ cd platform-device/cortex_a15x2_arndale
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/guest0.bin
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest1.bin
</pre>
2. Initial setup for linux
<pre>
$ cd platform-device/cortex_a15x2_arndale
$ make LINUX=y
</pre>


## Testing minimal linux on ARNDALE board using git submodule

1. refusing SD card for arndale(X is number of SD card parition)

<a href="http://releases.linaro.org/12.12/components/kernel/arndale-bl1/arndale-bl1.bin">arndale-bl1.bin download</a>

<pre>
$ sudo dd if=./u-boot-native/arndale-bl1.bin of=/dev/sdX bs=512 seek=1
$ sudo dd if=./u-boot-native/spl/smdk5250-spl.bin of=/dev/sdX bs=512 seek=17
$ sudo dd if=./u-boot-native/u-boot.bin of=/dev/sdX bs=512 seek=49
$ sudo dd if=hvc-man-switch.bin of=/dev/sdX bs=512 seek=1105
$ sudo dd if=guestimages/guest0.bin of=/dev/sdX bs=512 seek=3153
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=2129
</pre>

2. Setting serial port and run minicom
<pre>
minicom -s
serial device for hypervisor & bmguest is /dev/ttyS0
serial device for linuxguest is /dev/ttyS1
</pre>

3. When booting the board, press any key(of HostPC Keyboard, focused on serial terminal program window) in 3 seconds for enter the u-boot command mode
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x90000000 851 400;mmc read 0x80008000 c51 2000;go 0xa000004c
</pre>

4. Enter the following command
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x90000000 851 400;mmc read 0x80008000 c51 2000;go 0xa000004c
</pre>

# bmguset + bmgest

## Building the bmguest
<blockquote>
<pre>
$ cd platform-device/cortex_a15x2_arndale/guestos/bmguest/
$ make
</pre>
</blockquote>

## Building the k-hypervisor for bmguest
<blockquote>
<pre>
$ cd platform-device/cortex_a15x2_arndale
$ make
</pre>
</blockquote>


## Building the k-hypervisor for native u-boot

1. Download u-boot
<pre>
$ git submodule init
$ git submodule update
$ cd u-boot-native
</pre>
2. patching a autoboot for u-boot-native
<pre>
$ git checkout lue_arndale_13.1
$ git apply ../patch/u-boot-bootz.patch
</pre>
3. Building u-boot-arndale
<pre>
$ make arndale5250
</pre>


## Testing bmguest on ARNDALE 
1. copy bmguest
<pre>
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest0.bin
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest1.bin
</pre>

2. refusing SD card for arndale(X is number of SD card parition)

<a href="http://releases.linaro.org/12.12/components/kernel/arndale-bl1/arndale-bl1.bin">arndale-bl1.bin download</a>

<pre>
$ sudo dd if=./u-boot-native/arndale-bl1.bin of=/dev/sdX bs=512 seek=1
$ sudo dd if=./u-boot-native/spl/smdk5250-spl.bin of=/dev/sdX bs=512 seek=17
$ sudo dd if=./u-boot-native/u-boot.bin of=/dev/sdX bs=512 seek=49
$ sudo dd if=hvc-man-switch.bin of=/dev/sdX bs=512 seek=1105
$ sudo dd if=guestimages/guest0.bin of=/dev/sdX bs=512 seek=2129
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=3153
</pre>

3. loading on u-boot
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800; mmc read 0x60000000 851 400; mmc read 0x90000000 851 400; go 0xa000004c
</pre>

