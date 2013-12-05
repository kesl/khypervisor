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

#define CFG_MACHINE_NUMBER 2272  //RTSM

#define CFG_GUEST0_DEVICE_MEMORY \
{ "sysreg", 0x1C010000, 0x1C010000,   0x1000, LPAED_STAGE2_MEMATTR_DM },        \
{ "sysctl", 0x1C020000, 0x1C020000,   0x1000, LPAED_STAGE2_MEMATTR_DM },        \
{ "aaci", 0x1C040000, 0x1C040000,   0x1000, LPAED_STAGE2_MEMATTR_DM },          \
{ "mmci", 0x1C050000, 0x1C050000,   0x1000, LPAED_STAGE2_MEMATTR_DM },          \
{ "kmi", 0x1C060000, 0x1C060000,   0x1000, LPAED_STAGE2_MEMATTR_DM },           \
{ "kmi2", 0x1C070000, 0x1C070000,   0x1000, LPAED_STAGE2_MEMATTR_DM },          \
{ "v2m_serial0", 0x1C090000, 0x1C0A0000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, \
{ "v2m_serial1", 0x1C0A0000, 0x1C090000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, \
{ "v2m_serial2", 0x1C0B0000, 0x1C0B0000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, \
{ "v2m_serial3", 0x1C0C0000, 0x1C0C0000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, \
{ "wdt", 0x1C0F0000, 0x1C0F0000,   0x1000, LPAED_STAGE2_MEMATTR_DM },           \
{ "v2m_timer01(sp804)", 0x1C110000, 0x1C110000,   0x1000, LPAED_STAGE2_MEMATTR_DM },    \
{ "v2m_timer23", 0x1C120000, 0x1C120000,   0x1000, LPAED_STAGE2_MEMATTR_DM },           \
{ "rtc", 0x1C170000, 0x1C170000,   0x1000, LPAED_STAGE2_MEMATTR_DM },                   \
{ "clcd", 0x1C1F0000, 0x1C1F0000,   0x1000, LPAED_STAGE2_MEMATTR_DM },                  \
{ "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC, CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, LPAED_STAGE2_MEMATTR_DM }
/* UNMAP {  "gicd", 0x2C001000, 0x2C001000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, */

#define CFG_GUEST1_DEVICE_MEMORY \
{ "uart", 0x1C090000, 0x1C0B0000, 0x1000, LPAED_STAGE2_MEMATTR_DM },    \
{ "sp804", 0x1C110000, 0x1C120000, 0x1000, LPAED_STAGE2_MEMATTR_DM },   \
{ "gicc", 0x2C000000 | GIC_OFFSET_GICC, CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, LPAED_STAGE2_MEMATTR_DM }
/* UNMAP {  "gicd", 0x2C001000, 0x2C001000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, */

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

#define CFG_GUEST0_VIRQMAP(name) \
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
    DECLARE_VIRQMAP(name, 0, 45, 45);


#define CFG_GUEST1_VIRQMAP(name)
