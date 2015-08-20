#include <k-hypervisor-config.h>
#include <guest.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <memory.h>
#include <gic_regs.h>
#include <test/tests.h>
#include <smp.h>

#define DEBUG
#include "hvmm_trace.h"
#include <log/uart_print.h>


#define PLATFORM_BASIC_TESTS 4

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                        \
        name[id].map[_pirq].virq = _virq;       \
        name[id].map[_virq].pirq = _pirq;       \
    } while (0) 

static struct guest_virqmap _guest_virqmap[NUM_GUESTS_STATIC];

/**
 * \defgroup Guest_memory_map_descriptor
 *
 * Descriptor setting order
 * - label
 * - Intermediate Physical Address (IPA)
 * - Physical Address (PA)
 * - Size of memory region
 * - Memory Attribute
 * @{
 */
static struct memmap_desc guest_md_empty[] = {
    { 0, 0, 0, 0,  0},
};
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest0_device_md[] = {
    { "sata1", 0x1F21C000, 0x1F21C000, SZ_4K, MEMATTR_DM },
    /* sata 2 */
    { "sata2", 0x1A400000, 0x1A400000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F220000, 0x1F220000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F22A000, 0x1F22A000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F22C000, 0x1F22C000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F22D000, 0x1F22D000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F22E000, 0x1F22E000, SZ_4K, MEMATTR_DM },
    { "sata2", 0x1F227000, 0x1F227000, SZ_4K, MEMATTR_DM },
    /* sata 3 */
    { "sata3", 0x1A800000, 0x1A800000, SZ_4K, MEMATTR_DM },
    { "sata3", 0x1F230000, 0x1F230000, SZ_4K, MEMATTR_DM },
    { "sata3", 0x1F23A000, 0x1F23A000, SZ_4K, MEMATTR_DM },
    { "sata3", 0x1F23C000, 0x1F23C000, SZ_4K, MEMATTR_DM },
    { "sata3", 0x1F23D000, 0x1F23D000, SZ_4K, MEMATTR_DM },
    { "sata3", 0x1F23E000, 0x1F23E000, SZ_4K, MEMATTR_DM },
    /* pcie */
    { "pcie0", 0x1F2B0000, 0x1F2B0000, SZ_64K, MEMATTR_DM },
    //{ "pcie", 0xE0D0000000, 0xE0D0000000, SZ_256K, MEMATTR_DM },
    //{ "pcie0", 0x1F2B2000, 0x1F2B2000, SZ_4K, MEMATTR_DM },
    //{ "pcie0", 0x1F2BC000, 0x1F2BC000, SZ_4K, MEMATTR_DM },
    /* ethernet */
    { "ethernet", 0x17020000, 0x17020000, 0xE000, MEMATTR_DM },
    { "ethernet", 0x17030000, 0x17030000, SZ_4K, MEMATTR_DM },
    { "ethernet", 0x10000000, 0x10000000, SZ_4K, MEMATTR_DM },
    /* sdhc */
    { "sdhc", 0x1C000000, 0x1C000000, SZ_4K, MEMATTR_DM },
    { "sdhc", 0x1F2A0000, 0x1F2A0000, SZ_4K, MEMATTR_DM },
    /* clk */
    { "clk", 0x17000000, 0x17000000, SZ_8K, MEMATTR_DM },
    { "clk", 0x1702C000, 0x1702C000, SZ_4K, MEMATTR_DM },
    { "clk", 0x1703C000, 0x1703C000, SZ_4K, MEMATTR_DM },
    /* eda */
    { "eda", 0x78800000, 0x78800000, SZ_4K, MEMATTR_DM },
    { "eda", 0x7E200000, 0x7E200000, SZ_4K, MEMATTR_DM },
    { "eda", 0x7E700000, 0x7E700000, SZ_4K, MEMATTR_DM },
    { "eda", 0x7E720000, 0x7E720000, SZ_4K, MEMATTR_DM },
    /* edacmc */
    { "edacmc0", 0x7E800000, 0x7E800000, SZ_4K, MEMATTR_DM },
    { "edacmc1", 0x7E840000, 0x7E840000, SZ_4K, MEMATTR_DM },
    { "edacmc2", 0x7E880000, 0x7E880000, SZ_4K, MEMATTR_DM },
    { "edacmc3", 0x7E8C0000, 0x7E8C0000, SZ_4K, MEMATTR_DM },
    /* edapmd */
    { "edapmd", 0x1054A000, 0x1054A000, SZ_4K, MEMATTR_DM },
    { "edapmd0", 0x7C000000, 0x7C000000, SZ_2M, MEMATTR_DM },
    { "edapmd1", 0x7C200000, 0x7C200000, SZ_2M, MEMATTR_DM },
    { "edapmd2", 0x7C400000, 0x7C400000, SZ_2M, MEMATTR_DM },
    { "edapmd3", 0x7C600000, 0x7C600000, SZ_2M, MEMATTR_DM },
    /* eda cl3,soc */
    { "edacl3", 0x7E600000, 0x7E600000, SZ_4K, MEMATTR_DM },
    { "edasoc", 0x7E930000, 0x7E930000, SZ_4K, MEMATTR_DM },

    { "rtc", 0x10510000, 0x10510000, SZ_4K, MEMATTR_DM },
    { "i2c", 0x10512000, 0x10512000, SZ_4K, MEMATTR_DM },
    { "gpio", 0x1701C000, 0x1701C000, SZ_4K, MEMATTR_DM },
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { "reset", 0x17000000, 0x17000000, SZ_4K, MEMATTR_DM },
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC),
        CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K,
        MEMATTR_DM },
   { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest1_device_md[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC),
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K,
       MEMATTR_DM },
    { "reset", 0x17000014, 0x17000014, SZ_4K, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

#ifdef _SMP_
static struct memmap_desc guest2_device_md[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC),
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K,
       MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest3_device_md[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC),
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K,
       MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
#endif

/**
 * @brief Memory map for guest 0.
 */
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest0_memory_md[] = {
    {"start", CFG_MEMMAP_PHYS_START, 0, SZ_2G,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB},
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 1.
 */
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest1_memory_md[] = {
    {"start", CFG_MEMMAP_PHYS_START, 0, SZ_2G,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB},
    {0, 0, 0, 0,  0},
};

#ifdef _SMP_
/**
 * @brief Memory map for guest 2.
 */
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest2_memory_md[] = {
    /* 256MB */
    {"start", CFG_MEMMAP_PHYS_START, 0, SZ_1G,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB},
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 3.
 */
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest3_memory_md[] = {
    /* 256MB */
    {"start", CFG_MEMMAP_PHYS_START, 0, SZ_1G,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB},
    {0, 0, 0, 0,  0},
};
#endif

/* Memory Map for Guest 0 */
static struct memmap_desc guest0_mdlist[259];
/* Memory Map for Guest 1 */
static struct memmap_desc guest1_mdlist[259];
/* Memory Map for Guest 2 */
static struct memmap_desc guest2_mdlist[259];
/* Memory Map for Guest 3 */
static struct memmap_desc guest3_mdlist[259];
/** @}*/

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

    for(i=32; i< MAX_IRQS; i++)
        DECLARE_VIRQMAP(_guest_virqmap, 0, i, i);
}

void setup_memory()
{
    int i, j;
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */
    //guest0_memory_md[0].pa = (uint64_t)((uint64_t) &_guest0_bin_start);
    //guest1_memory_md[0].pa = (uint64_t)((uint64_t) &_guest1_bin_start);
    guest0_memory_md[0].pa = 0x4000000000ULL;
    guest1_memory_md[0].pa = 0x4080000000ULL;

    for(i=0; guest0_device_md[i].label; i++)
        guest0_mdlist[i] = guest0_device_md[i];

    for(j=0; guest0_memory_md[j].label; i++,j++)
        guest0_mdlist[i] = guest0_memory_md[j];

    guest0_mdlist[i] = guest_md_empty[0];

    for(i=0; guest1_device_md[i].label; i++)
        guest1_mdlist[i] = guest1_device_md[i];

    for(j=0; guest1_memory_md[j].label; i++,j++)
        guest1_mdlist[i] = guest1_memory_md[j];

    guest1_mdlist[i] = guest_md_empty[0];

#if _SMP_
    guest2_memory_md[0].pa = (uint64_t)((uint64_t) &_guest2_bin_start);
    guest3_memory_md[0].pa = (uint64_t)((uint64_t) &_guest3_bin_start);
#endif
}

/** @brief Registers generic timer irqs such as hypervisor timer event
 *  (GENERIC_TIMER_HYP), non-secure physical timer event(GENERIC_TIMER_NSP)
 *  and virtual timer event(GENERIC_TIMER_NSP).
 *  Each interrup source is identified by a unique ID.
 *
 *  DEVICE : armv8 IRQ number
 *  GENERIC_TIMER_NSP : 29
 *  GENERIC_TIMER_VIR : 30
 *  GENERIC_TIMER_HYP : 31
 *
 *  @note xgen-storm.dtsi, timer
 */
void setup_timer()
{
    _timer_irq = 31; /* GENERIC_TIMER_HYP */
}

uint8_t secondary_smp_pen;

#define print_feature_64(val1, val2) \
{ \
    uint64_t feature_1, feature_2; \
    feature_1 = read_sr64(val1); \
    feature_2 = read_sr64(val2); \
    uart_print_hex64(feature_1); \
    printH(" "); \
    uart_print_hex64(feature_2); \
    printH("\n"); \
}

#define print_feature_32(val1, val2) \
{ \
    uint32_t feature_1, feature_2; \
    feature_1 = read_sr32(val1); \
    feature_2 = read_sr32(val2); \
    uart_print_hex64(feature_1); \
    printH(" "); \
    uart_print_hex64(feature_2); \
    printH("\n"); \
}
void feature_check()
{
    uint32_t currentEL = read_sr32(currentEL);
    printH("current EL : %x\n", currentEL>>2);
    printH("[feature check]\n");

    printH("aarch64 : \n");
    printH("    Auxiliary :");
    print_feature_64(id_aa64afr0_el1, id_aa64afr1_el1);
    printH("    Debug:");
    print_feature_64(id_aa64dfr0_el1, id_aa64dfr1_el1);
    printH("    Instruction Set Attribute:");
    print_feature_64(id_aa64isar0_el1, id_aa64isar1_el1);
    printH("    Memory Model:");
    print_feature_64(id_aa64mmfr0_el1, id_aa64mmfr1_el1);
    printH("    Processor :");
    print_feature_64(id_aa64pfr0_el1, id_aa64pfr1_el1);

    //printH("aarch32 : \n");
}
void main_cpu_init()
{
    uart_init();

    init_print();
    printH("[%s : %d] Starting...Main CPU\n", __func__, __LINE__);

    feature_check();
    /* Initialize Memory Management */
    setup_memory();
    if (memory_init(guest0_mdlist, guest1_mdlist))
        printh("[start_guest] virtual memory initialization failed...\n");

    /* Initialize PIRQ to VIRQ mapping */
    setup_interrupt();
    /* Initialize Interrupt Management */
    if (interrupt_init(_guest_virqmap))
        printh("[start_guest] interrupt initialization failed...\n");

#ifdef _SMP_
    printH("wake up...other CPUs\n");
    secondary_smp_pen = 1;
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
    printH("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();
}

void secondary_cpu_init(uint64_t cpu)
{
    if (cpu >= CFG_NUMBER_OF_CPUS)
        hyp_abort_infinite();

    init_print();
    printH("[%s : %d] Starting...CPU : #%d\n", __func__, __LINE__, cpu);

    /* Initialize Memory Management */
    if (memory_init(guest2_mdlist, guest3_mdlist))
        printh("[start_guest] virtual memory initialization failed...\n");

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

int main(void)
{
    uint64_t cpu = smp_processor_id();

    if (cpu)
        secondary_cpu_init(cpu);
    else
        main_cpu_init();

    return 0;
}
