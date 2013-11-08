#include <context.h>
#include <print.h>
#include <vdev.h>
#include <hvmm_trace.h>

#define MAX_VDEV    5

static vdev_info_t vdev_list[MAX_VDEV];

void vdev_init(void)
{
    int i = 0;
    for (i = 0; i < MAX_VDEV; i++) {
        vdev_list[i].name = 0;
        vdev_list[i].base = 0;
        vdev_list[i].size = 0;
        vdev_list[i].handler = 0x0;
    }
}

hvmm_status_t vdev_reg_device(vdev_info_t *new_vdev)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    int i = 0;

    HVMM_TRACE_ENTER();
    for (i = 0; i < MAX_VDEV; i++) {
        if (vdev_list[i].handler == 0x0 ) {
            vdev_list[i].name = new_vdev->name;
            vdev_list[i].base = new_vdev->base;
            vdev_list[i].size = new_vdev->size;
            vdev_list[i].handler = new_vdev->handler;
            printh("vdev:Registered vdev '%s' at index %d\n", vdev_list[i].name, i);

            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }

    if ( result != HVMM_STATUS_SUCCESS ) {
        printh("vdev:Failed registering vdev '%s', max %d full \n", new_vdev->name, MAX_VDEV);
    }

    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vdev_emulate(uint32_t fipa, uint32_t wnr, vdev_access_size_t access_size, uint32_t srt, struct arch_regs *regs) 
{
    hvmm_status_t result = HVMM_STATUS_NOT_FOUND;
    int i = 0;
    uint32_t offset;
    uint8_t isize = 4;

    HVMM_TRACE_ENTER();
    if ( regs->cpsr & 0x20 ) {
        /* Thumb */
        isize = 2;
    }

    for (i = 0; i < MAX_VDEV; i++){
        if ( vdev_list[i].base == 0 ) break;

        offset = fipa - vdev_list[i].base;
        if ( fipa >= vdev_list[i].base && offset < vdev_list[i].size && vdev_list[i].handler != 0) {
            /* fipa is in the rage: base ~ base + size */
            printh("vdev: found %s for fipa %x srt:%x gpr[srt]:%x write:%d\n", vdev_list[i].name, fipa, srt, regs->gpr[srt], wnr );
            result = vdev_list[i].handler(wnr, offset, &(regs->gpr[srt]), access_size);
            if ( wnr == 0 ) {
                printh("vdev: result:%x\n", regs->gpr[srt] );
            }

            /* Update PC regardless handling result */
            regs->pc += isize;
            break;
        } else {
            printh("vdev: fipa %x base %x not matched\n", fipa, vdev_list[i].base );
        }
    }
    HVMM_TRACE_EXIT();

    return result;
}
