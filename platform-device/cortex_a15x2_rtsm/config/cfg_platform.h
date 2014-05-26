/*
 *  BOARD param
 */
#define CFG_BOARD_RTSM_VE_CA15
#define CFG_GENERIC_CA15
#define CFG_CNTFRQ          1000000000
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
/**
 * label, ipa, pa, size, attr
 */
#define CFG_GUEST0_DEVICE_MEMORY \
{ \
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
{ "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, \
        LPAED_STAGE2_MEMATTR_DM }, \
{ "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
{ "rtc", 0x1C170000, 0x1C170000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },         \
{ "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },        \
{ "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC, \
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, LPAED_STAGE2_MEMATTR_DM }, \
{ 0, 0, 0, 0, 0 } \
}

#define CFG_GUEST1_DEVICE_MEMORY \
{ \
{ "uart", 0x1C090000, 0x1C0B0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM }, \
{ "sp804", 0x1C110000, 0x1C120000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },\
{ "gicc", 0x2C000000 | GIC_OFFSET_GICC, \
   CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, LPAED_STAGE2_MEMATTR_DM },\
{0, 0, 0, 0, 0} \
}

