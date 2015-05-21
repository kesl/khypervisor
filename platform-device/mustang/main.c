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
    {       0, 0, 0, 0,  0},
};
/*  label, ipa, pa, size, attr */
static struct memmap_desc guest0_device_md0[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
static struct memmap_desc guest0_device_md1[] = {
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC) - 0x40000000,
            CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K,
            MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest1_device_md0[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
static struct memmap_desc guest1_device_md1[] = {
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC) - 0x40000000,
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K, MEMATTR_DM },
    {0, 0, 0, 0, 0}
};

#ifdef _SMP_
static struct memmap_desc guest2_device_md0[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
static struct memmap_desc guest2_device_md1[] = {
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC) - 0x40000000,
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K, MEMATTR_DM },
    {0, 0, 0, 0, 0}
};

static struct memmap_desc guest3_device_md0[] = {
    { "ns16550", 0x1C020000, 0x1C020000, SZ_64K, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};
static struct memmap_desc guest3_device_md1[] = {
    { "gicc", (CFG_GIC_BASE_PA | GIC_OFFSET_GICC) - 0x40000000,
       CFG_GIC_BASE_PA | GIC_OFFSET_GICV, SZ_128K, MEMATTR_DM },
    {0, 0, 0, 0, 0}
};
#endif

/**
 * @brief Memory map for guest 0.
 */
static struct memmap_desc guest0_memory_md0[] = {
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

static struct memmap_desc guest0_memory_md1[] = {
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 1.
 */
static struct memmap_desc guest1_memory_md0[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

static struct memmap_desc guest1_memory_md1[] = {
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

#ifdef _SMP_
/**
 * @brief Memory map for guest 2.
 */
static struct memmap_desc guest2_memory_md[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 3.
 */
static struct memmap_desc guest3_memory_md[] = {
    /* 256MB */
    {"start", 0x00000000, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};
#endif

/* Memory Map for Guest 0 */
static struct memmap_desc *guest0_mdlist[259] = {
    0,
};

/* Memory Map for Guest 1 */
static struct memmap_desc *guest1_mdlist[259] = {
    0,
};

/* Memory Map for Guest 2 */
static struct memmap_desc *guest2_mdlist[259] = {
    0,
};

/* Memory Map for Guest 3 */
static struct memmap_desc *guest3_mdlist[259] = {
    0,
};

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

    /*
     *  vimm-0, pirq-69, virq-69 = pwm timer driver
     *  vimm-0, pirq-32, virq-32 = WDT: shared driver
     *  vimm-0, pirq-34, virq-34 = SP804: shared driver
     *  vimm-0, pirq-35, virq-35 = SP804: shared driver
     *  vimm-0, pirq-36, virq-36 = RTC: shared driver
     *  vimm-0, pirq-38, virq-37 = UART: dedicated driver IRQ 37 for guest 0
     *  vimm-1, pirq-39, virq-37 = UART: dedicated driver IRQ 37 for guest 1
     *  vimm-2, pirq,40, virq-37 = UART: dedicated driver IRQ 37 for guest 2
     *  vimm-3, pirq,48, virq-37 = UART: dedicated driver IRQ 38 for guest 3 -ch
     *  vimm-0, pirq-43, virq-43 = ACCI: shared driver
     *  vimm-0, pirq-44, virq-44 = KMI: shared driver
     *  vimm-0, pirq-45, virq-45 = KMI: shared driver
     *  vimm-0, pirq-47, virq-47 = SMSC 91C111, Ethernet - etc0
     *  vimm-0, pirq-41, virq-41 = MCI - pl180
     *  vimm-0, pirq-42, virq-42 = MCI - pl180
     */
    DECLARE_VIRQMAP(_guest_virqmap, 0, 6, 6);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 3, 3);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 1, 1);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 16, 16);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 17, 17);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 18, 18);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 19, 19);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 31, 31);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 32, 32);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 33, 33);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 34, 34);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 35, 35);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 36, 36);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 37, 38);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 38, 37);
//    DECLARE_VIRQMAP(_guest_virqmap, 1, 39, 37);
//    DECLARE_VIRQMAP(_guest_virqmap, 2, 40, 37);
//    DECLARE_VIRQMAP(_guest_virqmap, 3, 48, 37);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 41, 41);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 42, 42);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 43, 43);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 44, 44);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 45, 45);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 46, 46);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 47, 47);
//    DECLARE_VIRQMAP(_guest_virqmap, 0, 69, 69);
}

void setup_memory()
{
    int i;
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */
    //guest0_memory_md[0].pa = (uint64_t)((uint64_t) &_guest0_bin_start);
    //guest1_memory_md[0].pa = (uint64_t)((uint64_t) &_guest1_bin_start);
    guest0_mdlist[0] = guest0_device_md0;
    guest1_mdlist[0] = guest1_device_md0;

    guest0_mdlist[1] = guest0_device_md1;
    guest1_mdlist[1] = guest1_device_md1;

    for(i=2; i< 256; i++) {
        guest0_mdlist[i] = guest_md_empty;
        guest1_mdlist[i] = guest_md_empty;
    }

    guest0_mdlist[i] = guest0_memory_md0;
    guest1_mdlist[i++] = guest1_memory_md0;

    guest0_mdlist[i] = guest0_memory_md1;
    guest1_mdlist[i++] = guest1_memory_md1;

    guest0_mdlist[i] = 0;
    guest1_mdlist[i] = 0;

    guest0_memory_md0[0].pa = 0x4000000000ULL;
    guest0_memory_md1[0].pa = 0x4040000000ULL;
    guest1_memory_md0[0].pa = 0x4080000000ULL;
    guest1_memory_md1[0].pa = 0x40C0000000ULL;
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
 *  GENERIC_TIMER_NSP : 13
 *  GENERIC_TIMER_VIR : 14
 *  GENERIC_TIMER_HYP : 15
 *
 *  @note xgen-storm.dtsi, timer
 */
void setup_timer()
{
    _timer_irq = 15; /* GENERIC_TIMER_HYP */
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

    local_irq_enable();
    local_serror_enable();

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

    local_irq_enable();
    local_serror_enable();

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
