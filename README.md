# Hypervisor for ARMv7 Virtualization Extensions

## Sub-projects
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
            - Generic Timer and Scheduler (Round-robin)
            - Boot loader

## Building Hypervisor (Monitor)
<blockquote>
<pre>
$ cd monitor
$ make UBOOT=y ; Build image for U-boot loading: armflash.bin
or
$ make UBOOT=n ; Build ELF image: hvc-man-switch.axf (can be loaded from FastModels as an application)

$ make ; without UBOOT variable is equivalent as UBOOT=n
</pre>
</blockquote>



## Download CodeBendch Lite Compiler for u-boot

- Compiler version: Sourcery CodeBench Lite 2013.05-23(gcc version: 4.7.3)

1. Access CodeBench page in Mentor Graphics(R) site
	http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/
2. Find ARM Processor part in the page and Click Download the EABI Release
	http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/request?id=e023fac2-e611-476b-a702-90eabb2aeca8&downloadlite=scblite2012&fmpath=/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/form
3. Fill the form about your information(name, email, etc), then you will receive the download URL of your email

## Testing Hypervisor Prototype 2 automatically with u-boot
- Branch infomation: proto2
- Commit hash: f1513c60a63e96fbd2ee60864f5b38d65631191e
- Compiler: Sourcery CodeBench Lite 2013.05-23(gcc version: 4.7.3)
- Patch location: $(ROOT)/patch/u-boot_fastmodel.patch

1. Download u-boot as git submodule
<pre>
$ git submodule init
$ git submodule update
</pre>

2. Apply patch file in u-boot root directory
<pre>
$ patch -p1 < ../patch/u-boot_fastmodel.patch
then you can see the message as below
patching file boards.cfg
patching file include/configs/vexpress_rtsm_ca15.h
patching file include/configs/vexpress_rtsm_common.h
</pre>

3. Build the u-boot 
<pre>
$ make vexpress_rtsm_ca15
</pre>

4. If you want to loading the u-boot with khypervisor proto 2, you can see chapter "Testing Hypervisor Prototype 2 Manually with u-boot" in this README.md

Testing u-boot on ARNDALE board using git submodule
==========================================
1. Download u-boot-arndale
<pre>
$ git checkout arndale
$ git submodule init
$ git submodule update
$ cd u-boot-arndale
</pre>
2. patching a autoboot for u-boot-arndale
<pre>
$ git checkout lue_arndale_13.1
$ patch -p1 < ../patch/u-boot_arndale.patch
</pre>
3. Building u-boot-arndale
<pre>
$ make arndale5250
</pre>
4. refusing SD card for arndale(X is number of SD card parition)

<a href="http://releases.linaro.org/12.12/components/kernel/arndale-bl1/arndale-bl1.bin">arndale-bl1.bin download</a>

<pre>
$ sudo dd if=arndale-bl1.bin of=/dev/sdX bs=512 seek=1
$ sudo dd if=spl/smdk5250-spl.bin of=/dev/sdX bs=512 seek=17
$ sudo dd if=u-boot.bin of=/dev/sdX bs=512 seek=49
$ sudo dd if=hvc-man-switch.bin of=/dev/sdX bs=512 seek=1105
$ sudo dd if=bmguest.bin of=/dev/sdX bs=512 seek=2129
</pre>
5. Loading khypervisor to arndale board using CODEVISOR debugger

<blockquote>
<pre>
- You can use "load memory from file" menu in CVD tool 
- Loading khypervisor execution file to proper memory address
</pre>
</blockquote>

## Testing minimal linux on ARNDALE board using git submodule

1. Download linux-arndale
<pre>
$ git submodule init
$ git submodule update
</pre>
2. Initial setup for linux-arndale
<pre>
$ cd linux-arndale
$ git checkout origin/exynos-jb -b exynos-jb
$ git apply ../patch/linux-arndale-config-add-minimal-linux-config.patch
$ git apply ../patch/arndale-change-kernel-sdram-address-uart-port-2-1.patch
$ git clone https://github.com/android/platform_prebuilt prebuilt
</pre>
3. Building linux-arndale
<pre>
$ make ARCH=arm arndale_minimal_linux_defconfig
$ make CROSS_COMPILE=./prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi- ARCH=arm -j8
</pre>
4. Loading khypervisor to arndale board using CODEVISOR debugger
<pre>
<CODEVISOR CMD>$ d.load.binary 0x40008000
<U-BOOT>$ bootm 40008000
</pre>

<blockquote>
<pre>
- You can use "load memory from file" menu in CVD tool 
- Loading khypervisor execution file to proper memory address
</pre>
</blockquote>

## Testing minimal linux on RTSM fastmodels using git submodule
1. Download linux-rtsm
<pre>
$ git submodule init
$ git submodule update
</pre>
2. Initial setup for linux
<pre>
$ cd linux-rtsm
$ git checkout 7d1f9aeff1ee4a20b1aeb377dd0f579fe9647619 -b 3.8
$ git apply ../patch/linux-fastmodels-config-add-minimal-linux-config.patch
$ cp ../linuxguest/fs.cpio ./
$ cp ../linuxguest/host-a15.dtb ./
</pre>
3. Building linux-arndale
<pre>
$ make ARCH=arm vexpress_minhw_defconfig
$ CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm make -j8
$ cat host-a15.dtb >> arch/arm/boot/zImage
4. Loading khypervisor to RTSM fastmodels
<pre>
$ cp arch/arm/boot/zImage ../monitor/zImage
$ cd ../monitor
$ make LINUX=y
</pre>
5. Run and Debug on DS-5 or FastModels MaxView + RTSM VE Cortex A15-A7
<pre>
$ maxview &
 Under monitor/ directory 
$ ./run_rtsm.sh
</pre>



## Testing Hypervisor Prototype 2 with u-boot

- Project branch information: test
- u-boot version information
    - tag: v2013.07-rc2
    - commit hash: e6bf18dba2a21bebf2c421b1c2e188225f6485a1
    - target board: vexpress_ca15_tc2
    - compiler: Sourcery CodeBench Lite 2013.05-23(gcc version: 4.7.3)
- Directory infomation
    - HYP: project root directory
    - UBOOT: uboot root directory


1. Build u-boot 
<pre>
$ git clone git://git.denx.de/u-boot.git
$ make vexpress_ca15_tc2_config; make 
or 
make vexpress_ca15_tc2
</pre>

2. Build bmguest/
<pre>
$ cd bmguest/
$ make GUESTTYPE=GUEST_HYPMON
$ cp bmguest.bin ../monitor
</pre>

3. Build Hypervisor Prototype 2: armflash.bin
<pre>
$ cd monitor
$ make UBOOT=y
</pre>

4. Loading armflash.bin on FastModels MaxView + RTSM VE Cortex A15-A7

    - Loading flash image with RTSM VE
<pre>
$ RTSM_VE_Cortex-A15x1-A7x1 --cadi-server -C motherboard.flashloader0.fname=$(HYP)/monitor/armflash.bin &
</pre>

    - Loading u-boot on Maxview
<pre>
$ maxview &
</pre>

    - From Maxview Debugger, connect to "the" model running, load u-boot and run to start debugging

    - <i> how to load application code in maxview </i>
        - Click the file tab-load application code 
        - select '$(UBOOT)/u-boot'(without file extension; filetype: data)
        - Click the run icon in toolbar

5. Copy hypervisor prototype 2 flash image to main memory 

    - <i> In u-boot prompt </i>
<pre>
    VExpress# cp.b 0x8000000 0xf0000000 0x100000; copy hypervisor from flash@0x800_0000 to DRAM@0xf000_0000
    VExpress# cp.b 0x8100000 0xa0000000 0x100000; copy guest os#1 from flash@0x810_0000 to DRAM@0xA000_0000
    VExpress# cp.b 0x8200000 0xb0000000 0x100000; copy guest os#2 from flash@0x820_0000 to DRAM@0xB000_0000
    VExpress# go 0xf000004c; this address is entry point of hypervisor
</pre>

## Testing Hypervisor Prototype 2 for arndale
1. configure SYSTEM variable in monitor/config-default.mk
<pre>
SYSTEM ?= arndale
</pre>
2. Build Hypervisor Prototype 2
<pre>
$ cd monitor
$ make UBOOT=y
</pre>
3. Loading Image using Codevisor debugger 
<pre>
Bootloader will be supported.
</pre>



## Tool chain
- ARM Toolchain Shipped with DS-5: <i>arm-linux-gnueabihf-</i>

## Testing Secure/Non-secure World Switching, Prototype 1 Phase 2
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

3. Run and Debug on DS-5 or FastModels MaxView + RTSM VE Cortex A15-A7
<pre>
$ maxview &
<i> Under securemode-switching/ directory </i>
$ ./run_rtsm.sh
</pre>
<blockquote> From Max View Debugger, connect to "the" model running, load securemode-switching/secmon.axf and run to start debugging</blockquote>
