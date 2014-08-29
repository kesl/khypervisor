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

# How to test RTOS guest + linux guest

## Make a build in one step continuous integration
Go to "How to Flash a K-hypervisor to arndale board (RTOS + Linux guest)"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_arndale/build/linux_ucos-ii.sh
$ make
</pre>

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

## Build linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/linux
$ make ARCH=arm arndale_minimal_linux_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/zImage
$ cp ./guestos/ucos-ii/rtos.bin ./guestimages/rtos.bin
</pre>
2. Build guestloader for linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make LINUX=y
$ cp guestloader.bin ../../guestimages/guest0.bin
</pre>
3. Build guestloader for RTOS guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make RTOS=y
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make
</pre>


## How to Flash a K-hypervisor to arndale board (RTOS + Linux guest)

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
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=11153
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
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x60000000 c51 1F40;mmc read 0x90000000 2b91 bb8;go 0xa000004c
</pre>

# How to test bmguest + linux guest

## Make a build in one step continuous integration
Go to "How to Flash a K-hypervisor to arndale board (bmguest + Linux guest)"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_arndale/build/linux_bmguest.sh
$ make
</pre>

## Build bootloader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/u-boot-native
$ make arndale5250 CROSS_COMPILE=arm-none-eabi- -j8
</pre>

## Build RTOS guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/bmguest/
$ make
</pre>

## Build linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/linux
$ make ARCH=arm arndale_minimal_linux_defconfig
$ make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/zImage
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make LINUX=y
$ cp guestloader.bin ../../guestimages/guest0.bin
</pre>
3. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make
</pre>


## How to Flash a K-hypervisor to arndale board (bmguest + Linux guest)

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
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=11153
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
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x60000000 c51 1F40;mmc read 0x90000000 2b91 bb8;go 0xa000004c
</pre>


# How to test bmguest + bmguest

## Make a build in one step continuous integration
Go to "How to Flash a K-hypervisor to arndale board (bmguest + bmguest)"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_arndale/build/bmguest_bmguest.sh
$ make
</pre>

## Build bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/bmguest/
$ make
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest0.bin
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make
</pre>

## Build bootloader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/u-boot-native
$ make arndale5250
</pre>

## How to Flash a K-hypervisor to arndale board (bmguest + bmguest)

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
$ sudo dd if=guestimages/guest1.bin of=/dev/sdX bs=512 seek=6153
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
$ ZIMAGE: ARNDALE # mmc read 0xa0000000 451 800;mmc read 0x60000000 c51 bb8;mmc read 0x90000000 1809 bb8;go 0xa000004c
</pre>




# How to test linaro-android(kitkat) + bmguest

## Download linaro-kernel
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro
$ ./get_linaro_kernel.sh
</pre>

## Make a build in one step continuous integration
Cross compiler : arm-none-linux-gnueabi- (version 4.5.2) (build linaro kernel)

Go to "How to Flash a K-hypervisor to arndale board (bmguest + Linux guest)"
this section, if you done this process first.

<pre>
$ cd khypervisor
$ source platform-device/cortex_a15x2_arndale/build/linaro_bmguest.sh
$ make
</pre>


## Build bootloader
- build bootloader or get builded bootfile

1.build bootloader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/u-boot-native
$ make arndale5250 CROSS_COMPILE=arm-none-eabi- -j8
</pre>

2.get builded bootfile<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro
$ ./get-builded-bootfile.sh
</pre>



## Build guest loader
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/bmguest/
$ make
</pre>

## Build linaro guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro/
$ ./get_linaro_kernel.sh
$ ./build-linaro-kernel.sh
</pre>


## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ cp ./guestos/linaro/arch/arm/boot/zImage ./guestimages/zImage
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for linux guest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make LINUX=y
$ cp guestloader.bin ../../guestimages/guest0.bin
</pre>
3. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Build k-hypervisor
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale
$ make
</pre>

## How to Flash a K-hypervisor to arndale board (linaro android + bmguest)

1. Get linaro tool to flash native android (kitkat)
<pre>
$ sudo add-apt-repository ppa:linaro-maintainers/tools
$ sudo apt-get update
$ sudo apt-get install linaro-image-tools
</pre>

2. get builded bootfile
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro
$ ./get-builded-bootfile.sh
</pre>

3. Get android source (kitkat)
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro/
$ ./get-android.sh
</pre>

4. Flash sdcard
<pre>
$ cd khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro/
$ ./upload-sdcard.sh
"Then in shell input you're sdcard name then enter you're sdcard name (sda,sdc ....)"
 input you're sdcard name
$ sdX
 you're sdcard is sdX
</pre>

5. Insert the SD card and turn it on. When booting the board, press any key(of HostPC Keyboard, focused on serial terminal program window) in 3 seconds for enter the u-boot command mode
<pre>
$ ZIMAGE: ARNDALE #
</pre>

6. Enter the following command
<pre>
$ ZIMAGE: ARNDALE # mmc read 0xb0000000 451 64;mmc read 0x40000000 4B5 1f4a;mmc read 0x80000000 23ff 14;mmc read 0x46400000 2413 1bbc;mmc read 80100000 3fcf A; go 0xb000004c
</pre>

*You can get information(android + bmguest) at khypervisor/platform-device/cortex_a15x2_arndale/guestos/android-linaro/README












