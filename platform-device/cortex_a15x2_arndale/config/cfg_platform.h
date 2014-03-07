/*
 *  BOARD param
 */
#define CFG_BOARD_ARNDALE
#define CFG_EXYNOS5250
#define CFG_CNTFRQ          24000000
#define CFG_UART2         0x12C00000
#define CFG_UART1         0x12C10000
#define CFG_UART0         0x12C20000

/*
 *  SOC param
 */
#define CFG_GIC_BASE_PA   0x10480000

#define CFG_MACHINE_NUMBER 4274

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
    { "pl330.0", 0x11C10000, 0x11C10000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },         \
    { "pl330.1", 0x121A0000, 0x121A0000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },         \
    { "pl330.2", 0x121B0000, 0x121B0000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },         \
    { "uart.0", 0x12C00000, 0x12C00000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },          \
    { "uart.1", 0x12C10000, 0x12C10000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },          \
    { "uart.2", 0x12C20000, 0x12C20000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },          \
    { "uart.3", 0x12C30000, 0x12C30000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },          \
    { "chipid", 0x10000000, 0x10000000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },           \
    { "syscon", 0x10050000, 0x10050000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },          \
    { "timer", 0x12DD0000, 0x12DD0000, SZ_16K, LPAED_STAGE2_MEMATTR_DM },           \
    { "wdt", 0x101D0000, 0x101D0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },              \
    { "sromc", 0x12250000, 0x12250000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "hsphy", 0x12130000, 0x12130000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "systimer", 0x101C0000, 0x101C0000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },         \
    { "sysram", 0x02020000, 0x02020000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },           \
    { "cmu", 0x10010000, 0x10010000, 144 * SZ_1K, LPAED_STAGE2_MEMATTR_DM },        \
    { "pmu", 0x10040000, 0x10040000, SZ_64K, LPAED_STAGE2_MEMATTR_DM },             \
    { "combiner", 0x10440000, 0x10440000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },         \
    { "gpio1", 0x11400000, 0x11400000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "gpio2", 0x13400000, 0x13400000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "gpio3", 0x10D10000, 0x10D10000, SZ_256, LPAED_STAGE2_MEMATTR_DM },           \
    { "gpio4", 0x03860000, 0x03860000, SZ_256, LPAED_STAGE2_MEMATTR_DM },           \
    { "audss", 0x03810000, 0x03810000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "hsphy", 0x12130000, 0x12130000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },            \
    { "ss_phy", 0x12100000, 0x12100000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },           \
    { "sysram_ns", 0x0204F000, 0x0204F000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },        \
    { "ppmu_cpu", 0x10C60000, 0x10C60000, SZ_8K, LPAED_STAGE2_MEMATTR_DM },         \
    { "ppmu_ddr_c", 0x10C40000, 0x10C40000, SZ_8K, LPAED_STAGE2_MEMATTR_DM },       \
    { "ppmu_ddr_r1", 0x10C50000, 0x10C50000, SZ_8K, LPAED_STAGE2_MEMATTR_DM },      \
    { "ppmu_ddr_l", 0x10CB0000, 0x10CB0000, SZ_8K, LPAED_STAGE2_MEMATTR_DM },       \
    { "ppmu_right0_bus", 0x13660000, 0x13660000, SZ_8K, LPAED_STAGE2_MEMATTR_DM},   \
    { "fimc_lite0", 0x13C00000, 0x13C00000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },       \
    { "fimc_lite1", 0x13C10000, 0x13C10000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },       \
    { "fimc_lite2", 0x13C90000, 0x13C90000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },       \
    { "mipi_csis0", 0x13C20000, 0x13C20000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },       \
    { "mipi_csis1", 0x13C30000, 0x13C30000, SZ_4K, LPAED_STAGE2_MEMATTR_DM },       \
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC, CFG_GIC_BASE_PA | GIC_OFFSET_GICVI,  0x2000, LPAED_STAGE2_MEMATTR_DM }
    /* UNMAP {  "gicd", 0x2C001000, 0x2C001000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, */ 

#define CFG_GUEST1_DEVICE_MEMORY \
        { "uart", 0x12C10000, 0x12C20000, 0x1000, LPAED_STAGE2_MEMATTR_DM },        \
        { "pwm_timer", 0x3FD10000, 0x12DD0000, 0x1000, LPAED_STAGE2_MEMATTR_DM },   \
        { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC, CFG_GIC_BASE_PA | GIC_OFFSET_GICVI,  0x2000, LPAED_STAGE2_MEMATTR_DM }
        /* UNMAP {  "gicd", 0x2C001000, 0x2C001000,     0x1000, LPAED_STAGE2_MEMATTR_DM }, */

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                \
        name[_pirq].virq = _virq;         \
        name[_pirq].vmid = id;           \
    } while (0)

#define CFG_GUEST_VIRQMAP(name) \
    DECLARE_VIRQMAP(name, 0, 32, 32);   \
    DECLARE_VIRQMAP(name, 0, 33, 33);   \
    DECLARE_VIRQMAP(name, 0, 34, 34);   \
    DECLARE_VIRQMAP(name, 0, 35, 35);   \
    DECLARE_VIRQMAP(name, 0, 36, 36);   \
    DECLARE_VIRQMAP(name, 0, 37, 37);   \
    DECLARE_VIRQMAP(name, 0, 38, 38);   \
    DECLARE_VIRQMAP(name, 0, 39, 39);   \
    DECLARE_VIRQMAP(name, 0, 40, 40);   \
    DECLARE_VIRQMAP(name, 0, 41, 41);   \
    DECLARE_VIRQMAP(name, 0, 42, 42);   \
    DECLARE_VIRQMAP(name, 0, 43, 43);   \
    DECLARE_VIRQMAP(name, 0, 44, 44);   \
    DECLARE_VIRQMAP(name, 0, 45, 45);   \
    DECLARE_VIRQMAP(name, 0, 46, 46);   \
    DECLARE_VIRQMAP(name, 0, 47, 47);   \
    DECLARE_VIRQMAP(name, 0, 48, 48);   \
    DECLARE_VIRQMAP(name, 0, 49, 49);   \
    DECLARE_VIRQMAP(name, 0, 50, 50);   \
    DECLARE_VIRQMAP(name, 0, 51, 51);   \
    DECLARE_VIRQMAP(name, 0, 52, 52);   \
    DECLARE_VIRQMAP(name, 0, 53, 53);   \
    DECLARE_VIRQMAP(name, 0, 54, 54);   \
    DECLARE_VIRQMAP(name, 0, 55, 55);   \
    DECLARE_VIRQMAP(name, 0, 56, 56);   \
    DECLARE_VIRQMAP(name, 0, 57, 57);   \
    DECLARE_VIRQMAP(name, 0, 58, 58);   \
    DECLARE_VIRQMAP(name, 0, 59, 59);   \
    DECLARE_VIRQMAP(name, 0, 60, 60);   \
    DECLARE_VIRQMAP(name, 0, 61, 61);   \
    DECLARE_VIRQMAP(name, 0, 62, 62);   \
    DECLARE_VIRQMAP(name, 0, 63, 63);   \
    DECLARE_VIRQMAP(name, 0, 64, 64);   \
    DECLARE_VIRQMAP(name, 0, 65, 65);   \
    DECLARE_VIRQMAP(name, 0, 66, 66);   \
    DECLARE_VIRQMAP(name, 0, 67, 67);   \
    DECLARE_VIRQMAP(name, 0, 68, 68);   \
    DECLARE_VIRQMAP(name, 0, 69, 69);   \
    DECLARE_VIRQMAP(name, 0, 70, 70);   \
    DECLARE_VIRQMAP(name, 0, 71, 71);   \
    DECLARE_VIRQMAP(name, 0, 72, 72);   \
    DECLARE_VIRQMAP(name, 0, 73, 73);   \
    DECLARE_VIRQMAP(name, 0, 74, 74);   \
    DECLARE_VIRQMAP(name, 0, 75, 75);   \
    DECLARE_VIRQMAP(name, 0, 76, 76);   \
    DECLARE_VIRQMAP(name, 0, 77, 77);   \
    DECLARE_VIRQMAP(name, 0, 78, 78);   \
    DECLARE_VIRQMAP(name, 0, 79, 79);   \
    DECLARE_VIRQMAP(name, 0, 80, 80);   \
    DECLARE_VIRQMAP(name, 0, 81, 81);   \
    DECLARE_VIRQMAP(name, 0, 82, 82);   \
    DECLARE_VIRQMAP(name, 0, 83, 83);   \
    DECLARE_VIRQMAP(name, 0, 84, 84);   \
    DECLARE_VIRQMAP(name, 0, 85, 85);   \
    DECLARE_VIRQMAP(name, 0, 86, 86);   \
    DECLARE_VIRQMAP(name, 0, 87, 87);   \
    DECLARE_VIRQMAP(name, 0, 88, 88);   \
    DECLARE_VIRQMAP(name, 0, 89, 89);   \
    DECLARE_VIRQMAP(name, 0, 90, 90);   \
    DECLARE_VIRQMAP(name, 0, 91, 91);   \
    DECLARE_VIRQMAP(name, 0, 92, 92);   \
    DECLARE_VIRQMAP(name, 0, 93, 93);   \
    DECLARE_VIRQMAP(name, 0, 94, 94);   \
    DECLARE_VIRQMAP(name, 0, 95, 95);   \
    DECLARE_VIRQMAP(name, 0, 96, 96);   \
    DECLARE_VIRQMAP(name, 0, 97, 97);   \
    DECLARE_VIRQMAP(name, 0, 98, 98);   \
    DECLARE_VIRQMAP(name, 0, 99, 99);
