#include <k-hypervisor-config.h>
#include <memory.h>
#include <arch_types.h>
#include <log/print.h>
#include <log/uart_print.h>

static struct memory_ops *_memory_ops;

/**
 * @brief Hyp mode general-purpose storage allocator.
 *
 * Obtains the storage from hyp mode heap memory.
 * - First, search free block list.
 * - Second, if free block list has no list or not enough size of list,
 *   Obtains storage from heap memory.
 * - Third, there are list has enough size, return list's pointer.
 *
 * @param size Size of space
 * @return Allocated space
 */
void *memory_alloc(unsigned long size)
{
    if (_memory_ops->alloc)
        return _memory_ops->alloc(size);

    return 0;
}

/**
 * @brief Free allocated memory.
 *
 * Freed the header of the allocated block.
 *
 * @param addr The address of target block.
 * @return void
 */
void memory_free(void *ap)
{
    if (_memory_ops->free)
        _memory_ops->free(ap);
}

hvmm_status_t memory_save(void)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_memory_ops->save)
        ret =  _memory_ops->save();

    return ret;
}

hvmm_status_t memory_restore(vmid_t vmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_memory_ops->restore)
        ret = _memory_ops->restore(vmid);

    return ret;
}

hvmm_status_t memory_init(struct memmap_desc **guest0,
                struct memmap_desc **guest1)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    _memory_ops = _memory_module.ops;

    if (_memory_ops->init) {
        ret = _memory_ops->init(guest0, guest1);
        if (ret)
            printh("host initial failed:'%s'\n", _memory_module.name);
    }

    return ret;
}
