/*
 *  BOARD param
 */
#define CFG_BOARD_RTSM_VE_CA15
#define CFG_GENERIC_CA15
#define CFG_CNTFRQ          100000000
#define CFG_UART2         0x1C0B0000
#define CFG_UART1         0x1C0A0000
#define CFG_UART0         0x1C090000
/*
 *  SOC param
 */
#define CFG_GIC_BASE_PA   0x2c000000

#define CFG_MACHINE_NUMBER 2272

#define SZ_1                0x00000001
#define SZ_2                0x00000002
#define SZ_4                0x00000004
#define SZ_8                0x00000008
#define SZ_16               0x00000010
#define SZ_32               0x00000020
#define SZ_64               0x00000040
#define SZ_128              0x00000080
#define SZ_256              0x00000100
#define SZ_512              0x00000200

#define SZ_1K               0x00000400
#define SZ_2K               0x00000800
#define SZ_4K               0x00001000
#define SZ_8K               0x00002000
#define SZ_16K              0x00004000
#define SZ_32K              0x00008000
#define SZ_64K              0x00010000
#define SZ_128K             0x00020000
#define SZ_256K             0x00040000
#define SZ_512K             0x00080000

#define SZ_1M               0x00100000
#define SZ_2M               0x00200000
#define SZ_4M               0x00400000
#define SZ_8M               0x00800000
#define SZ_16M              0x01000000
#define SZ_32M              0x02000000
#define SZ_64M              0x04000000
#define SZ_128M             0x08000000
#define SZ_256M             0x10000000
#define SZ_512M             0x20000000

#define SZ_1G               0x40000000
#define SZ_2G               0x80000000

#define CFG_GUEST0_DEVICE_MEMORY \
do {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },   \
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },   \
    { "kmi", 0x1C060000, 0x1C060000,  SZ_4K, LPAED_STAGE2_MEMATTR_DM },   \
    { "kmi2", 0x1C070000, 0x1C070000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },   \
    { "v2m_serial0", 0x1C090000, 0x1C0A0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "v2m_serial1", 0x1C0A0000, 0x1C090000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },         \
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K,
                                                    LPAED_STAGE2_MEMATTR_DM }, \
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },         \
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },        \
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, LPAED_STAGE2_MEMATTR_DM } \
    } while (0)

#define CFG_GUEST1_DEVICE_MEMORY \
do {
    { "uart", 0x1C090000, 0x1C0B0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
    { "sp804", 0x1C110000, 0x1C120000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },\
    { "gicc", 0x2C000000 | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, LPAED_STAGE2_MEMATTR_DM } \
    } while (0)
#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                \
        name[_pirq].virq = _virq;       \
        name[_pirq].vmid = id;          \
    } while (0)

/*
 *  vimm-0, pirq-69, virq-69 = pwm timer driver
 *  vimm-0, pirq-32, virq-32 = WDT: shared driver
 *  vimm-0, pirq-34, virq-34 = SP804: shared driver
 *  vimm-0, pirq-35, virq-35 = SP804: shared driver
 *  vimm-0, pirq-36, virq-36 = RTC: shared driver
 *  vimm-0, pirq-38, virq-37 = UART: dedicated driver IRQ 37 for guest 0
 *  vimm-1, pirq-39, virq-37 = UART: dedicated driver IRQ 37 for guest 1
 *  vimm-0, pirq-43, virq-43 = ACCI: shared driver
 *  vimm-0, pirq-44, virq-44 = KMI: shared driver
 *  vimm-0, pirq-45, virq-45 = KMI: shared driver
 */

#define CFG_GUEST_VIRQMAP(name) \
do { \
    DECLARE_VIRQMAP(name, 0, 1, 1);     \
    DECLARE_VIRQMAP(name, 0, 31, 31);   \
    DECLARE_VIRQMAP(name, 0, 33, 33);   \
    DECLARE_VIRQMAP(name, 0, 16, 16);   \
    DECLARE_VIRQMAP(name, 0, 17, 17);   \
    DECLARE_VIRQMAP(name, 0, 18, 18);   \
    DECLARE_VIRQMAP(name, 0, 19, 19);   \
    DECLARE_VIRQMAP(name, 0, 69, 69);   \
    DECLARE_VIRQMAP(name, 0, 32, 32);   \
    DECLARE_VIRQMAP(name, 0, 34, 34);   \
    DECLARE_VIRQMAP(name, 0, 35, 35);   \
    DECLARE_VIRQMAP(name, 0, 36, 36);   \
    DECLARE_VIRQMAP(name, 0, 38, 37);   \
    DECLARE_VIRQMAP(name, 1, 39, 37);   \
    DECLARE_VIRQMAP(name, 0, 43, 43);   \
    DECLARE_VIRQMAP(name, 0, 44, 44);   \
    DECLARE_VIRQMAP(name, 0, 45, 45);   \
    } while (0)
