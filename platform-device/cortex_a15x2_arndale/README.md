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

# How to test RTOSguset + linux guest

## Build bootloader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/u-boot-native
$ make arndale5250 CROSS_COMPILE=arm-none-eabi- -j8
</pre>

## Build RTOS guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/ucos-ii/
$ make CROSS_COMPILE=arm-none-eabi-
</pre>

## Build linux guset
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/linux
$ make ARCH=arm arndale_minimal_linux_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
</pre>

## Build k-hypervisor
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/guest0.bin
$ cp ./guestos/ucos-ii/rtos.bin ./guestimages/guest1.bin
</pre>
2. Initial setup for linux
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make LINUX=y RTOS=y 
</pre>


## How to Flash a K-hypervisor to arndale board

1. Copy the binaries to SD card (X is number of SD card parition)	
	<br>
	Download arndale-bl1.bin here : <a href="http://releases.linaro.org/12.12/components/kernel/arndale-bl1/arndale-bl1.bin">arndale-bl1.bin download</a>

	<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ sudo dd if=arndale-bl1.bin of=/dev/sdX bs=512 seek=1
$ sudo dd if=./u-boot-native/spl/smdk5250-spl.bin of=/dev/sdX bs=512 seek=17
$ sudo dd if=./u-boot-native/u-boot.bin of=/dev/sdX bs=512 seek=49
$ sudo dd if=hvc-man-switch.bin of=/dev/sdX bs=512 seek=1105
$ sudo dd if=guestimages/guest0.bin of=/dev/sdX bs=512 seek=3153
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=2129
</pre>

2. Setting serial port and run minicom, open 2 minicoms
<pre>
$ minicom -s
"serial device for hypervisor & rtos sets /dev/ttyS0"
$ minicom -s
"serial device for linuxguest sets /dev/ttyS1"
</pre>

3. Insert the SD card and turn it on. When booting the board, press any key(of HostPC Keyboard, focused on serial terminal program window) in 3 seconds for enter the u-boot command mode
<pre>
$ ZIMAGE: ARNDALE # 
</pre>

4. Enter the following command
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x90000000 851 400;mmc read 0x80008000 c51 2000;go 0xa000004c
</pre>

# How to test bmguset + bmgest
## Build bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/bmguest/
$ make
</pre>

## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest0.bin
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/guest1.bin
$ make
</pre>

## Build bootloader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/u-boot-native
$ make arndale5250
</pre>

## How to Flash a K-hypervisor to arndale board

1. Copy the binaries to SD card (X is number of SD card parition)	
	<br>
	Download arndale-bl1.bin here : <a href="http://releases.linaro.org/12.12/components/kernel/arndale-bl1/arndale-bl1.bin">arndale-bl1.bin download</a>

	<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ sudo dd if=arndale-bl1.bin of=/dev/sdX bs=512 seek=1
$ sudo dd if=./u-boot-native/spl/smdk5250-spl.bin of=/dev/sdX bs=512 seek=17
$ sudo dd if=./u-boot-native/u-boot.bin of=/dev/sdX bs=512 seek=49
$ sudo dd if=hvc-man-switch.bin of=/dev/sdX bs=512 seek=1105
$ sudo dd if=guestimages/guest0.bin of=/dev/sdX bs=512 seek=3153
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=2129
</pre>

2. Setting serial port and run minicom, open 2 minicoms
<pre>
$ minicom -s
"serial device for hypervisor & rtos sets /dev/ttyS0"
$ minicom -s
"serial device for linuxguest sets /dev/ttyS1"
</pre>

3. Insert the SD card and turn it on. When booting the board, press any key(of HostPC Keyboard, focused on serial terminal program window) in 3 seconds for enter the u-boot command mode
<pre>
$ ZIMAGE: ARNDALE # 
</pre>

4. Enter the following command
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800; mmc read 0x60000000 851 400; mmc read 0x90000000 851 400; go 0xa000004c
</pre>

