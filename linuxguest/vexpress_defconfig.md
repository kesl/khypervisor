# Linux Kernel Default Configuration: vexpress_defconfig
## Hardware Support Related Configuration Flags
- CONFIG_ARCH_VEXPRESS
- CONFIG_SMP
- CONFIG_MTD_ARM_INTEGRATOR - CFI/P720T
- CONFIG_NETDEVICES/CONFIG_NET_ETHERNET/CONFIG_SMSC911X
- CONFIG_SERIO_AMBAKMI
- CONFIG_SERIAL_AMBA_PL011
- CONFIG_FB_ARMCLCD
- CONFIG_SND_ARMAACI
- CONFIG_USB/CONFIG_USB_ISP1760_HCD
- CONFIG_MMC/CONFIG_MMC_ARMMMCI
- CONFIG_RTC_DRV_PL031

## Test Configuration 1
- Disable SATA/PATA
- Disable Network Device Support
- Disable Sound Card Support
- Disable USB Support
- Disable MMC/SD/SDIO

## Test Configuration 2
- Applied Test Configuration 1
- Disabled CONFIG_FB_ARMCLCD
- Disabled CONFIG_SMP

### Hardware Resources
- Memory: 0x80000000 ~ 2GB

- GIC: 0x2c001000 ~ +0x8000
    - IRQ : interrupts = <1 9 0xf04>
    - Virtual Maintenance Interrupt - GICD_PPISR[9] - PPI6(CA15) - ID25

- Timer:
    - Hypervisor Timer - GICD_PPISR[10] - PPI5(CA15) - ID26
    - Virtual Timer - GICD_PPISR[11] - PPI4(CA15) - ID27
    - Secure Physical Timer GICD_PPISR[13] - PPI1(CA15) - ID29
    - Non-secure Physical Timer GICD_PPISR[14] - PPI2(CA15) - ID30

- Timer: SP804
	- vexpress 0x1C110000 ~ 64KB, IRQ34
    - vexpress 0x1c120000 ~ 64KB, IRQ35
    - 0x3 00110000 ~, 0x1000
    - interrupt: 2
    - 0x3 00120000 ~, 0x1000
    - interrupt: 3
    
- Motherboard (rtsm_ve-cortex_a15x1.dts)
	- 0x08000000 ~ 0x04000000
    - 0x14000000 ~ 0x04000000
    - 0x18000000 ~ 0x04000000
    - 0x1c000000 ~ 0x04000000
    - 0x0c000000 ~ 0x04000000
    - 0x10000000 ~ 0x04000000
    - Flash: 0x00000000 ~ , 0x04000000
    - flash: 0x4 00000000 ~, 0x04000000
	- vram: 0x2 00000000 ~, 0x00800000
- Ethernet: SMSC91c111, Unused
	- 0x2 02000000 ~, 0x10000
    - Interrupt: 15
- sysreg, gpio-controller
	- 0x3 00010000 ~, 0x1000
- sysctl: SP810
	- 0x3 00020000 ~, 0x1000
- aaci: PL041
	- 0x3 00040000 ~, 0x1000
    - interrupt: 11
- mmci: PL180
	- 0x3 00050000 ~, 0x1000
    - interrupt: 9, 10
- kmi: PL050
	- 0x3 00060000 ~, 0x1000
    - interrupt: 12
    - 0x3 00070000 ~, 0x1000
    - interrupt: 13
- UART: PL011
	- 0x3 00090000 ~, 0x1000
    - interrupt: 5
	- 0x3 000a0000 ~, 0x1000
    - interrupt: 6
	- 0x3 000b0000 ~, 0x1000
    - interrupt: 7
	- 0x3 000c0000 ~, 0x1000
    - interrupt: 8
- wdt: Watchdog SP805
	- 0x3 000f0000 ~, 0x1000
    - interrupt: 0
- rtc: PL031
	- 0x3 00170000 ~, 0x1000
    - interrupt: 4
- clcd: PL111, Unused
	- 0x3 001f0000 ~, 0x1000
	- interrupt: 14
    - framebuffer: 0x18000000 ~ 0x01000000 (VRAM)

### $ cat /proc/iomem
<pre>
1c010000-1c010fff : 1c010000.sysreg
1c040000-1c040fff : /motherboard/iofpga@3,00000000/aaci@040000
1c050000-1c050fff : /motherboard/iofpga@3,00000000/mmci@050000
1c060000-1c060fff : /motherboard/iofpga@3,00000000/kmi@060000
  1c060000-1c060fff : kmi-pl050
1c070000-1c070fff : /motherboard/iofpga@3,00000000/kmi@070000
  1c070000-1c070fff : kmi-pl050
1c090000-1c090fff : /motherboard/iofpga@3,00000000/uart@090000
  1c090000-1c090fff : uart-pl011
1c0a0000-1c0a0fff : /motherboard/iofpga@3,00000000/uart@0a0000
  1c0a0000-1c0a0fff : uart-pl011
1c0b0000-1c0b0fff : /motherboard/iofpga@3,00000000/uart@0b0000
  1c0b0000-1c0b0fff : uart-pl011
1c0c0000-1c0c0fff : /motherboard/iofpga@3,00000000/uart@0c0000
  1c0c0000-1c0c0fff : uart-pl011
1c0f0000-1c0f0fff : /motherboard/iofpga@3,00000000/wdt@0f0000
1c110000-1c110fff : /motherboard/iofpga@3,00000000/timer@110000
1c120000-1c120fff : /motherboard/iofpga@3,00000000/timer@120000
1c170000-1c170fff : /motherboard/iofpga@3,00000000/rtc@170000
  1c170000-1c170fff : rtc-pl031
1c1f0000-1c1f0fff : /motherboard/iofpga@3,00000000/clcd@1f0000
a0000000-dfffffff : System RAM
  a0008000-a03693c7 : Kernel code
  a056e000-a05a9c73 : Kernel data
</pre>

### $ cat /proc/interrupts
<pre>
# cat interrupts 
           CPU0       
 34:    2439979       GIC  timer
 36:          0       GIC  rtc-pl031
 37:       3037       GIC  uart-pl011
 44:          9       GIC  kmi-pl050
 45:        118       GIC  kmi-pl050
Err:          0
</pre>

## Hardware Componenets
- SP804 Timer
- PL031 RTC
- GIC
- PL310 RTC
- PL011 UART
- PL050
- PL111 CLCD (Unused)

## ARM Errata
- ARM Errata 720789
- PL310 Errata 753970
- VEXPRESS Cortex A5/A9 Errata

### CONFIG_EXPERIMENTAL=y
- should be disabled?
- Necessary for
    - 'Supplement the appended DTB with traditional ATAG information'
    - 'Kernel hacking' -> 'Show Timeing information on printks'

### ~~CONFIG\_LOCALVERSION_AUTO~~ is not set

### CONFIG_SYSVIPC=y
- sw feature
- not hardware feature

### CONFIG_IKCONFIG=y
- Kernel .config support
- not hardware feature

### CONFIG\_IKCONFIG_PROC=y
- /proc/config.gz accesses .config
- not hardware feature

### CONFIG\_LOG_BUF_SHIFT=14
- Log buffer size: 2^14 bytes
- not hardware feature

### CONFIG\_CGROUPS=y
- Control Group Support
- group processes for use with process control subsystems
- not hardware feature
- Related Features
    - Cpuset

### CONFIG_CPUSETS=y
- Control Group
- not hardware feature

### ~~CONFIG\_UTS_NS~~ is not set
### ~~CONFIG\_IPC_NS~~ is not set
### ~~CONFIG\_USER_NS~~ is not set
### ~~CONFIG\_PID_NS~~ is not set
### ~~CONFIG\_NET_NS~~ is not set

### CONFIG_BLK_DEV_INITRD=y
- Initial RAM Filesystem and RAM disk

### ~CONFIG_CC_OPTIMIZE_FOR_SIZE~ is not set
### CONFIG_PROFILING=y
- Profiling support
- not hardware feature

### CONFIG_OPROFILE=y
- Experimental OProfile system profiling
- no hardware feature

### CONFIG_MODULES=y
- no hardware feature

### CONFIG_MODULE_UNLOAD=y
- no hardware feature

### ~~CONFIG_LBDAF~~ is not set
### ~~CONFIG_BLK_DEV_BSG~~ is not set
### ~~CONFIG_IOSCHED_DEADLINE~~ is not set
### ~~CONFIG_IOSCHED_CFQ~~ is not set
### CONFIG_ARCH_VEXPRESS=y
- Our target platform

### CONFIG_ARCH_VEXPRESS_CA9X4=y
- Versatile Express Cortex-A9x4 tile

### ~~CONFIG_SWP_EMULATE~~ is not set

### CONFIG_SMP=y
- Can be disabled for single core?

### CONFIG_VMSPLIT_2G=y
- Unclear why set

### CONFIG_HOTPLUG_CPU=y
- Valid only with SMP supported?

### CONFIG_AEABI=y
- Use ARM EABI to compile the kernel

### CONFIG_ZBOOT_ROM_TEXT=0x0
- Compressed ROM boot loader base address
- Physical address of ROM-able zImage
- Valid only if CONFIG_ZBOOT is enabled

### CONFIG_ZBOOT_ROM_BSS=0x0
- physical address of read/write area of ROM-able zImage
- Valid only if CONFIG_ZBOOT is enabled

### CONFIG_CMDLINE="root=/dev/nfs nfsroot=10.1.69.3:/work/nfsroot ip=dhcp console=ttyAMA0 mem=128M"
### CONFIG_VFP=y
- ARM VFP Support
- Runs VFP applications

### CONFIG_NEON=y
- NEON Advanced SIMD Extension Support

### ~~CONFIG_CORE_DUMP_DEFAULT_ELF_HEADERS~~ is not set
### CONFIG_NET=y
- Networking Support
- no hardware feature

### CONFIG_PACKET=y
- Direct Packet protocol support (tcpdump)
- no hardware feature

### CONFIG_UNIX=y
- Unix Domain Socket
- no hardware feature

### CONFIG_INET=y
- TCP/IP
- no hardware feature

### CONFIG_IP_PNP=y
- Kernel Level Autoconfiguration of IP Addresses of Devices

### CONFIG_IP_PNP_DHCP=y
- DHCP support for kernel level autoconfiguration of IP addresses

### CONFIG_IP_PNP_BOOTP=y
- BOOTP Protocol
- Useful for NFS root file system

### ~~CONFIG_INET_LRO~~ is not set
### ~~CONFIG_IPV6~~ is not set
### ~~CONFIG_WIRELESS~~ is not set
### CONFIG_UEVENT_HELPER_PATH="/sbin/hotplug"
- Not clear what's 'uevent'

## CONFIG_MTD=y
- Flash, RAM, SSD ...

### CONFIG_MTD_CONCAT=y
### CONFIG_MTD_PARTITIONS=y
### CONFIG_MTD_CMDLINE_PARTS=y
### CONFIG_MTD_CHAR=y
### CONFIG_MTD_BLOCK=y
### CONFIG_MTD_CFI=y
### CONFIG_MTD_CFI_INTELEXT=y
### CONFIG_MTD_CFI_AMDSTD=y
### CONFIG_MTD_ARM_INTEGRATOR=y
- CFI Flash Device Mapped on ARM Integrator/P720T

## CONFIG_MISC_DEVICES=y
### ~~CONFIG_SCSI_PROC_FS~~ is not set
### CONFIG_BLK_DEV_SD=y
- SCSI Disk Block Device Support

### ~~CONFIG_SCSI_LOWLEVEL~~ is not set
### CONFIG_ATA=y
- SATA/PATA Disk Support

### ~~CONFIG_SATA_PMP~~ is not set


## CONFIG_NETDEVICES=y
- Network Devices

### CONFIG_NET_ETHERNET=y
- Ethernet Devices

### CONFIG_SMSC911X=y
- SMSC911X hardware driver

### ~~CONFIG_NETDEV_1000~~ is not set
### ~~CONFIG_NETDEV_10000~~ is not set
### ~~CONFIG_WLAN~~ is not set

## CONFIG_INPUT_EVDEV=y
- Event Interface
- /dev/input/eventX

### ~~CONFIG_SERIO_SERPORT~~ is not set
### CONFIG_SERIO_AMBAKMI=y
- hardware driver
- AMBA KMI keyboard controller hardware driver

### CONFIG_SERIAL_AMBA_PL011=y
- UART hardware driver

### CONFIG_SERIAL_AMBA_PL011_CONSOLE=y
- console support

### CONFIG_LEGACY_PTY_COUNT=16
- up to 16 PTYs
- no direct hardware driver

### ~~CONFIG_HW_RANDOM~~ is not set
### ~~CONFIG_HWMON~~ is not set

## CONFIG_FB=y
- Framebuffer

### CONFIG_FB_ARMCLCD=y
- ARM PrimeCell PL110 hardware support

### CONFIG_FRAMEBUFFER_CONSOLE=y
- Console on LCD

### CONFIG_LOGO=y
- Bootup logo on framebuffer

### ~~CONFIG_LOGO_LINUX_MONO~~ is not set
### ~~CONFIG_LOGO_LINUX_VGA16~~ is not set

## CONFIG_SOUND=y
- Sound card support

### CONFIG_SND=y
- ALSA

### CONFIG_SND_MIXER_OSS=y
- OSS Mixer API

### CONFIG_SND_PCM_OSS=y
- OSS PCM Interface Support?

### ~~CONFIG_SND_DRIVERS~~ is not set
### CONFIG_SND_ARMAACI=y
- ARM PrimeCell PL041 AC Link hardware support

## HID Devices
### CONFIG_HID_DRAGONRISE=y
- Game Controller, should be disabled?

### CONFIG_HID_GYRATION=y
- Remote Control, should be disabled?

### CONFIG_HID_TWINHAN=y
- IR Remote Control, should be disabled?

### CONFIG_HID_NTRIG=y
- N-Trig touch screen, should be disabled?

### CONFIG_HID_PANTHERLORD=y
- try with disabled

### CONFIG_HID_PETALYNX=y
- try with disabled

### CONFIG_HID_SAMSUNG=y
- try with disabled

### CONFIG_HID_SONY=y
- try with disabled

### CONFIG_HID_SUNPLUS=y
- try with disabled

### CONFIG_HID_GREENASIA=y
- try with disabled

### CONFIG_HID_SMARTJOYPLUS=y
- try with disabled

### CONFIG_HID_TOPSEED=y
- try with disabled

### CONFIG\_HID_THRUSTMASTER=y
- try with disabled

### CONFIG\_HID_ZEROPLUS=y
- try with disabled

## CONFIG_USB=y
- try disabled

### CONFIG_USB_ANNOUNCE_NEW_DEVICES=y
### ~~CONFIG_USB_DEVICE_CLASS~~ is not set
### CONFIG_USB_MON=y
- try disabled

### CONFIG_USB_ISP1760_HCD=y
- try disabled

### CONFIG_USB_STORAGE=y
- try disabled

## CONFIG_MMC=y
- try disabled

### CONFIG_MMC_ARMMMCI=y
- try disabled

### CONFIG_RTC_CLASS=y
- RTC driver class support

### CONFIG_RTC_DRV_PL031=y
- ARM PrimeCell Real Time Clock PL031 hardware support

## File Systems

### CONFIG_EXT2_FS=y
### CONFIG_EXT3_FS=y
### ~~CONFIG_EXT3_DEFAULTS_TO_ORDERED~~ is not set
### ~~CONFIG_EXT3_FS_XATTR~~ is not set
### CONFIG_VFAT_FS=y
### CONFIG_TMPFS=y
### CONFIG_JFFS2_FS=y
### CONFIG_CRAMFS=y
### CONFIG_NFS_FS=y
### CONFIG_NFS_V3=y
### CONFIG_ROOT_NFS=y
### ~~CONFIG_RPCSEC_GSS_KRB5~~ is not set

### CONFIG_NLS_CODEPAGE_437=y
### CONFIG_NLS_ISO8859_1=y

### CONFIG_MAGIC_SYSRQ=y
- Linux Magic System Request Key Hacks

### CONFIG_DEBUG_FS=y
### CONFIG_DEBUG_KERNEL=y
### CONFIG_DETECT_HUNG_TASK=y
### ~~CONFIG_SCHED_DEBUG~~ is not set
### CONFIG_DEBUG_INFO=y
### ~~CONFIG_RCU_CPU_STALL_DETECTOR~~ is not set
### CONFIG_DEBUG_USER=y
### CONFIG_DEBUG_ERRORS=y
### CONFIG_DEBUG_LL=y
### CONFIG_EARLY_PRINTK=y
### ~~CONFIG_CRYPTO_ANSI_CPRNG~~ is not set
### ~~CONFIG_CRYPTO_HW~~ is not set
