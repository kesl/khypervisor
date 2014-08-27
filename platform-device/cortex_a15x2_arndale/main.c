#include <k-hypervisor-config.h>
#include <guest.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <memory.h>
#include <gic_regs.h>
#include <test/tests.h>
#include <smp.h>
#include <asm_io.h>
#include <drivers/mct/mct_priv.h>

#define PLATFORM_BASIC_TESTS 0

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                        \
        name[id].map[_pirq].virq = _virq;       \
        name[id].map[_virq].pirq = _pirq;       \
    } while (0)

static struct guest_virqmap _guest_virqmap[NUM_GUESTS_STATIC];

static struct memmap_desc guest_md_empty[] = {
    {       0, 0, 0, 0,  0},
};

/*  label, ipa, pa, size, attr */
static struct memmap_desc guest0_device_md[] = {
    { "tmu", 0x10060000, 0x10060000, SZ_4K, MEMATTR_DM },
    { "pl330.0", 0x11C10000, 0x11C10000, SZ_64K, MEMATTR_DM },
    { "pl330.1", 0x121A0000, 0x121A0000, SZ_64K, MEMATTR_DM },
    { "pl330.2", 0x121B0000, 0x121B0000, SZ_64K, MEMATTR_DM },
    { "usb", 0x12000000, 0x12000000, SZ_64K, MEMATTR_DM },
    { "uart.0", 0x12C00000, 0x12C00000, SZ_64K, MEMATTR_DM },
    { "uart.1", 0x12C10000, 0x12C10000, SZ_64K, MEMATTR_DM },
    { "uart.2", 0x12C20000, 0x12C20000, SZ_64K, MEMATTR_DM },
    { "uart.3", 0x12C30000, 0x12C30000, SZ_64K, MEMATTR_DM },
    { "chipid", 0x10000000, 0x10000000, SZ_4K, MEMATTR_DM },
    { "syscon", 0x10050000, 0x10050000, SZ_64K, MEMATTR_DM },
    { "timer", 0x12DD0000, 0x12DD0000, SZ_16K, MEMATTR_DM },
    { "wdt", 0x101D0000, 0x101D0000, SZ_4K, MEMATTR_DM },
    { "sromc", 0x12250000, 0x12250000, SZ_4K, MEMATTR_DM },
    { "hsphy", 0x12130000, 0x12130000, SZ_4K, MEMATTR_DM },
    { "sataphy", 0x12170000, 0x12170000, SZ_4K, MEMATTR_DM},
    { "i2c", 0x121d0000, 0x121d0000, SZ_4K, MEMATTR_DM},
    { "dwmmc0", 0x12200000, 0x12200000, SZ_64K, MEMATTR_DM},
    { "dwmmc1", 0x12210000, 0x12210000, SZ_64K, MEMATTR_DM},
    { "dwmmc2", 0x12220000, 0x12220000, SZ_64K, MEMATTR_DM},
    { "dwmmc3", 0x12230000, 0x12230000, SZ_64K, MEMATTR_DM},
    { "i2c2", 0x12260000, 0x12260000, SZ_4K, MEMATTR_DM},
    { "i2c3", 0x12280000, 0x12280000, SZ_4K, MEMATTR_DM},
    { "i2c4", 0x122e0000, 0x122e0000, SZ_4K, MEMATTR_DM},
    { "systimer", 0x101C0000, 0x101C0000, SZ_4K, MEMATTR_DM },
    { "sysram", 0x02020000, 0x02020000, SZ_4K, MEMATTR_DM },
    { "cmu", 0x10010000, 0x10010000, 144 * SZ_1K, MEMATTR_DM },
    { "pmu", 0x10040000, 0x10040000, SZ_64K, MEMATTR_DM },
    { "combiner", 0x10440000, 0x10440000, SZ_4K, MEMATTR_DM },
    { "gpio1", 0x11400000, 0x11400000, SZ_4K, MEMATTR_DM },
    { "gpio2", 0x13400000, 0x13400000, SZ_4K, MEMATTR_DM },
    { "gpio3", 0x10D10000, 0x10D10000, SZ_256, MEMATTR_DM },
    { "gpio4", 0x03860000, 0x03860000, SZ_256, MEMATTR_DM },
    { "audss", 0x03810000, 0x03810000, SZ_4K, MEMATTR_DM },
    { "ss_phy", 0x12100000, 0x12100000, SZ_4K, MEMATTR_DM },
    { "sysram_ns", 0x0204F000, 0x0204F000, SZ_4K, MEMATTR_DM },
    { "ppmu_cpu", 0x10C60000, 0x10C60000, SZ_8K, MEMATTR_DM },
    { "ppmu_ddr_c", 0x10C40000, 0x10C40000, SZ_8K, MEMATTR_DM },
    { "ppmu_ddr_r1", 0x10C50000, 0x10C50000, SZ_8K, MEMATTR_DM },
    { "ppmu_ddr_l", 0x10CB0000, 0x10CB0000, SZ_8K, MEMATTR_DM },
    { "ppmu_right0_bus", 0x13660000, 0x13660000, SZ_8K, MEMATTR_DM},
    { "fimc_lite0", 0x13C00000, 0x13C00000, SZ_4K, MEMATTR_DM },
    { "fimc_lite1", 0x13C10000, 0x13C10000, SZ_4K, MEMATTR_DM },
    { "fimc_lite2", 0x13C90000, 0x13C90000, SZ_4K, MEMATTR_DM },
    { "mipi_csis0", 0x13C20000, 0x13C20000, SZ_4K, MEMATTR_DM },
    { "mipi_csis1", 0x13C30000, 0x13C30000, SZ_4K, MEMATTR_DM },
    { "fimd", 0x14400000, 0x14400000, SZ_256K, MEMATTR_DM },
    { "i2c5", 0x12C70000, 0x12C70000, SZ_64K, MEMATTR_DM },
    { "i2c6", 0x121D0000, 0x121D0000, SZ_64K, MEMATTR_DM },
    { "MTCADC_ISP", 0x13150000, 0x13150000, SZ_64K, MEMATTR_DM },
    { "i2c0", 0x12C60000, 0x12C60000, SZ_64K, MEMATTR_DM },
    { "usb_ehci", 0x12110000, 0x12110000, SZ_64K, MEMATTR_DM },
    { "usb_ohci", 0x12120000, 0x12120000, SZ_64K, MEMATTR_DM },
    { "usb_ctrl", 0x12130000, 0x12130000, SZ_64K, MEMATTR_DM },
    { "usb_devicelink", 0x12140000, 0x12140000, SZ_64K, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest1_device_md[] = {
    { "uart", 0x12C10000, 0x12C20000, 0x1000, MEMATTR_DM },
    { "uart2", 0x12C20000, 0x12C20000, 0x1000, MEMATTR_DM },
    { "pwm_timer", 0x3FD10000, 0x12DD0000, 0x1000, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

#if _SMP_
static struct memmap_desc guest2_device_md[] = {
    { "uart", 0x12C10000, 0x12C20000, 0x1000, MEMATTR_DM },
    { "pwm_timer", 0x3FD10000, 0x12DD0000, 0x1000, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest3_device_md[] = {
    { "uart", 0x12C10000, 0x12C20000, 0x1000, MEMATTR_DM },
    { "pwm_timer", 0x3FD10000, 0x12DD0000, 0x1000, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
        CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, 0x2000, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
#endif

static struct memmap_desc guest0_memory_md[] = {
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

static struct memmap_desc guest1_memory_md[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x10000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

#if _SMP_
/**
 * @brief Memory map for guest 2.
 */
static struct memmap_desc guest2_memory_md[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x10000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 3.
 */
static struct memmap_desc guest3_memory_md[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x10000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};
#endif

/* Memory Map for Guest 0 */
static struct memmap_desc *guest0_mdlist[] = {
    guest0_device_md,   /* 0x0000_0000 */
    guest0_memory_md,   /* 0x4000_0000 */
    guest_md_empty,
    guest_md_empty,     /* 0xC000_0000 PA:0x40000000*/
    0
};

/* Memory Map for Guest 1 */
static struct memmap_desc *guest1_mdlist[] = {
    guest1_device_md,
    guest1_memory_md,
    guest_md_empty,
    guest_md_empty,
    0
};

#if _SMP_
/* Memory Map for Guest 2 */
static struct memmap_desc *guest2_mdlist[] = {
    guest2_device_md,
    guest2_memory_md,
    guest_md_empty,
    guest_md_empty,
    0
};

/* Memory Map for Guest 3 */
static struct memmap_desc *guest3_mdlist[] = {
    guest3_device_md,
    guest3_memory_md,
    guest_md_empty,
    guest_md_empty,
    0
};
#endif

static uint32_t _timer_irq;

/*
 * Creates a mapping table between PIRQ and VIRQ.vmid/pirq/coreid.
 * Mapping of between pirq and virq is hard-coded.
 */
void setup_interrupt()
{
    int i, j;
    struct virqmap_entry *map;

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        map = _guest_virqmap[i].map;
        for (j = 0; j < MAX_IRQS; j++) {
            map[j].enabled = GUEST_IRQ_DISABLE;
            map[j].virq = VIRQ_INVALID;
            map[j].pirq = PIRQ_INVALID;
        }
    }

    {
       int i;
       for (i =0; i < 1000; i++){
            if(i==25 || i==26)
                continue;
            else
                DECLARE_VIRQMAP(_guest_virqmap, 0, i, i);
       }

    }
#if 0
    DECLARE_VIRQMAP(_guest_virqmap, 0, 16, 16);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 20, 20);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 32, 32);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 33, 33);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 34, 34);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 35, 35);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 36, 36);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 37, 37);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 38, 38);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 39, 39);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 40, 40);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 41, 41);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 42, 42);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 43, 43);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 44, 44);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 45, 45);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 46, 46);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 47, 47);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 48, 48);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 49, 49);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 50, 50);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 51, 51);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 52, 52);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 53, 53);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 54, 54);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 55, 55);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 56, 56);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 57, 57);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 58, 58);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 59, 59);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 60, 60);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 61, 61);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 62, 62);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 63, 63);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 64, 64);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 65, 65);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 66, 66);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 67, 67);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 68, 68);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 69, 69);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 70, 70);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 71, 71);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 72, 72);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 73, 73);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 74, 74);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 75, 75);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 76, 76);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 77, 77);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 78, 78);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 79, 79);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 80, 80);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 81, 81);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 82, 82);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 83, 83);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 84, 84);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 85, 85);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 86, 86);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 87, 87);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 88, 88);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 89, 89);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 90, 90);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 91, 91);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 92, 92);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 93, 93);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 94, 94);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 95, 95);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 96, 96);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 97, 97);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 98, 98);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 99, 99);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 103, 103);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 104, 104);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 107, 107);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 109, 109);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 126, 126);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 147, 147);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 152, 152);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 153, 153);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 156, 156);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 309, 309);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 347, 347);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 447, 447);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 106, 106);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 138, 138);
#endif
}

void setup_memory()
{
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */
    guest0_memory_md[0].pa = (uint64_t)((uint32_t) &_guest0_bin_start);
    guest1_memory_md[0].pa = (uint64_t)((uint32_t) &_guest1_bin_start);

#if _SMP_
    guest2_memory_md[0].pa = (uint64_t)((uint32_t) &_guest2_bin_start);
    guest3_memory_md[0].pa = (uint64_t)((uint32_t) &_guest3_bin_start);
#endif
}

/** @brief Registers generic timer irqs such as hypervisor timer event
 *  (GENERIC_TIMER_HYP), non-secure physical timer event(GENERIC_TIMER_NSP)
 *  and virtual timer event(GENERIC_TIMER_NSP).
 *  Each interrup source is identified by a unique ID.
 *  cf. "Cortex™-A15 Technical Reference Manual" 8.2.3 Interrupt sources
 *
 *  DEVICE : IRQ number
 *  GENERIC_TIMER_HYP : 26
 *  GENERIC_TIMER_NSP : 30
 *  GENERIC_TIMER_VIR : 27
 *
 *  @note "Cortex™-A15 Technical Reference Manual", 8.2.3 Interrupt sources
 */
void setup_timer()
{
    _timer_irq = 26; /* GENERIC_TIMER_HYP */
    mct_init();
}

#ifdef _SMP_
void init_secondary();
#endif

int main_cpu_init()
{
    init_print();

#ifdef _SMP_
    /*
     * In Exnosy5250 platform like a Arndale, secondary cpus have been WFE
     * state by u-boot spl,so hyperviosr should wake up secondary cpu using
     * the sev instruction. This code will be different on each platform.
     */
    dsb_sev();
#endif
    printH("[%s : %d]Starting...Main CPU\n", __func__, __LINE__);
    setup_memory();
    /* Initialize Memory Management */
    if (memory_init(guest0_mdlist, guest1_mdlist))
        printh("[start_guest] virtual memory initialization failed...\n");

    /* Initialize PIRQ to VIRQ mapping */
    setup_interrupt();
    /* Initialize Interrupt Management */
    if (interrupt_init(_guest_virqmap))
        printh("[start_guest] interrupt initialization failed...\n");

#ifdef _SMP_
    printH("wake up...other CPUs\n");
    writel((unsigned int)init_secondary, 0x02020000);
#endif

    /* Initialize Timer */
    setup_timer();
    if (timer_init(_timer_irq))
        printh("[start_guest] timer initialization failed...\n");

    /* Initialize Guests */
    if (guest_init())
        printh("[start_guest] guest initialization failed...\n");

    /* Initialize Virtual Devices */
    if (vdev_init())
        printh("[start_guest] virtual device initialization failed...\n");

    /* Begin running test code for newly implemented features */
    if (basic_tests_run(PLATFORM_BASIC_TESTS))
        printh("[start_guest] basic testing failed...\n");

    /* Print Banner */
    printH("%s", BANNER_STRING);

    /* Switch to the first guest */
    guest_sched_start();

    /* The code flow must not reach here */
    printh("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();

}

#ifdef _SMP_

void secondary_cpu_init(uint32_t cpu)
{
    if (cpu >= CFG_NUMBER_OF_CPUS)
        hyp_abort_infinite();

    printH("[%s : %d] Starting...Secondary CPU\n", __func__, __LINE__);

    /* Initialize Memory Management */
    if (memory_init(guest2_mdlist, guest3_mdlist))
        printh("[start_guest] virtual memory initialization failed...\n");

    printH("[%s : %d] Interrupt Init... for CPU\n", __func__, __LINE__);

    /* Initialize Interrupt Management */
    if (interrupt_init(_guest_virqmap))
        printh("[start_guest] interrupt initialization failed...\n");

    /* Initialize Timer */
    if (timer_init(_timer_irq))
        printh("[start_guest] timer initialization failed...\n");

    /* Initialize Guests */
    if (guest_init())
        printh("[start_guest] guest initialization failed...\n");

    /* Initialize Virtual Devices */
    if (vdev_init())
        printh("[start_guest] virtual device initialization failed...\n");

    /* Switch to the first guest */
    guest_sched_start();

    /* The code flow must not reach here */
    printh("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();
}
#endif


int main(void)
{
#ifdef _SMP_
    uint32_t cpu = smp_processor_id();

    if (!cpu)
        main_cpu_init();
    else
        secondary_cpu_init(cpu);
#else
    main_cpu_init();
#endif
    return 0;
}
