#include <guest.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <mm.h>
#include <test/tests.h>
#include <smp.h>

#define PLATFORM_BASIC_TESTS 0

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                        \
        name[id].map[_pirq].virq = _virq;       \
        name[id].map[_virq].pirq = _pirq;       \
    } while (0)


static struct guest_virqmap _guest_virqmap[NUM_GUESTS_STATIC];

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
    if (interrupt_init(_guest_virqmap))
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
