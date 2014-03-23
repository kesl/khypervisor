#include <guest.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <mm.h>
#include <test/tests.h>
#include <smp.h>

#define PLATFORM_BASIC_TESTS 0

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                \
        name[_pirq].virq = _virq;       \
        name[_pirq].vmid = id;          \
    } while (0)

static struct virqmap_entry _virqmap[MAX_IRQS];

/*
 * Creates a mapping table between PIRQ and VIRQ.vmid/pirq/coreid.
 * Mapping of between pirq and virq is hard-coded.
 */
void setup_interrupt()
{
    int i;

    for (i = 0; i < MAX_IRQS; i++) {
        _virqmap[i].vmid = VMID_INVALID;
        _virqmap[i].virq = 0;
        _virqmap[i].pirq = 0;
    }

    /*
     * NOTE(wonseok):
     * referenced by
     * https://github.com/kesl/khypervisor/wiki/Hardware-Resources
     * -of-Guest-Linux-on-FastModels-RTSM_VE-Cortex-A15x1
     * */
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
    DECLARE_VIRQMAP(_virqmap, 0, 1, 1);
    DECLARE_VIRQMAP(_virqmap, 0, 31, 31);
    DECLARE_VIRQMAP(_virqmap, 0, 33, 33);
    DECLARE_VIRQMAP(_virqmap, 0, 16, 16);
    DECLARE_VIRQMAP(_virqmap, 0, 17, 17);
    DECLARE_VIRQMAP(_virqmap, 0, 18, 18);
    DECLARE_VIRQMAP(_virqmap, 0, 19, 19);
    DECLARE_VIRQMAP(_virqmap, 0, 69, 69);
    DECLARE_VIRQMAP(_virqmap, 0, 32, 32);
    DECLARE_VIRQMAP(_virqmap, 0, 34, 34);
    DECLARE_VIRQMAP(_virqmap, 0, 35, 35);
    DECLARE_VIRQMAP(_virqmap, 0, 36, 36);
    DECLARE_VIRQMAP(_virqmap, 0, 38, 37);
    DECLARE_VIRQMAP(_virqmap, 1, 39, 37);
    DECLARE_VIRQMAP(_virqmap, 0, 43, 43);
    DECLARE_VIRQMAP(_virqmap, 0, 44, 44);
    DECLARE_VIRQMAP(_virqmap, 0, 45, 45);
}

void setup_memory()
{
    /* TODO: Here is guest memory setup */
}

int main_cpu_init()
{
    setup_memory();
    /* Initialize Memory Management */
    if (hvmm_mm_init())
        printh("[start_guest] virtual memory initialization failed...\n");

    /* Initialize PIRQ to VIRQ mapping */
    setup_interrupt();
    /* Initialize Interrupt Management */
    if (interrupt_init(_virqmap))
        printh("[start_guest] interrupt initialization failed...\n");

    /* Initialize Timer */
    if (timer_init(TIMER_SCHED))
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

int main(void)
{
    uint32_t cpu = smp_processor_id();

    init_print();
    printh("[%s : %d] Starting...\n", __func__, __LINE__);

    if (!cpu)
        main_cpu_init();
    else
        printh("Second CPU Starting...\n", __func__, __LINE__);

    return 0;
}
