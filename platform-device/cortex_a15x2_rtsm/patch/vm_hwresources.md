_Initial Draft Oct. 10th 2013_

# Linux Kernel Version
- git: git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
- Version: v3.8-rc4 (tag name)
- <code>  $ git checkout v3.8-rc4 </code>
## Base Configuration
- vexpress_defconfig
For example, you can start customizing configuration by,
<code> CROSS_COMPILE=arm-linux- ARCH=arm make vexpress_defconfig </code>

## Configuring for minimizing device drivers (hardware components to use)
Based on vexpress_defconfig,

- Test Configuration 1
    - Disable SATA/PATA
    - Disable Network Device Support
    - Disable Sound Card Support
    - Disable USB Support
    - Disable MMC/SD/SDIO

- Test Configuration 2 (Disable further)
    - Applied Test Configuration 1
    - Disabled CONFIG_FB_ARMCLCD
    - Disabled CONFIG_SMP


# Hardware Resources

## Physical IRQs to be forwarded
- IRQ32 : Watchdog SP805
- IRQ34 : Timer SP804*
- IRQ35 : Timer SP804
- IRQ36 : RTC PL031*
- IRQ38 : _VIRQ37_ UART PL001* (UART1 plays UART0 )
- IRQ41 : MMCI PL180
- IRQ42 : MMCI PL180
- IRQ43 : AACI PL041
- IRQ44 : KMI PL050*
- IRQ45 : KMI PL050*
- *** IRQs monitored occuring while run-time

## Minimum Hardware Resources Must be Supported by the Hypervisor
- Memory: 0x80000000 ~ 2GB
- GIC Distributor:
    - 0x2c001000
- Timer: SP804
	- 0x1C110000 ~ 64KB, IRQ34
    - 0x1c120000 ~ 64KB, IRQ35
- sysreg, gpio-controller
	- 0x3 00010000 ~, 0x1000
- sysctl: SP810
	- 0x3 00020000 ~, 0x1000
- aaci: PL041
	- 0x3 00040000 ~, 0x1000, IRQ43
    - interrupt: 11
- UART: PL011
	- 0x3 00090000 ~, 0x1000, IRQ37
    - interrupt: 5
	- 0x3 000a0000 ~, 0x1000, IRQ38
    - interrupt: 6
	- 0x3 000b0000 ~, 0x1000, IRQ39
    - interrupt: 7
	- 0x3 000c0000 ~, 0x1000, IRQ40
    - interrupt: 8
- wdt: Watchdog SP805
	- 0x3 000f0000 ~, 0x1000, IRQ32
    - interrupt: 0
- rtc: PL031
	- 0x3 00170000 ~, 0x1000, IRQ36
    - interrupt: 4
- mmci: PL180
	- 0x3 00050000 ~, 0x1000, IRQ41, IRQ42
    - interrupt: 9, 10
- kmi: PL050
	- 0x3 00060000 ~, 0x1000, IRQ44
    - interrupt: 12
    - 0x3 00070000 ~, 0x1000, IRQ45
    - interrupt: 13

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
