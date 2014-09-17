#include <vdev.h>
#include <hvmm_trace.h>
#define DEBUG
#include <log/print.h>
#include <smp.h>

#define MAX_VDEV    256

static struct vdev_module *_vdev_module[VDEV_LEVEL_MAX][MAX_VDEV];
static int _vdev_size[VDEV_LEVEL_MAX];

/**
 * \brief Register the virtual deivce \a module. Level \a level is
 * composed of three types(high, middle and low priority). This function
 * will be called initial state by per virtual device.
 *
 * \retval 0 if virtual device is registed correctly
 * \retval -3 This is an internal error
 */
hvmm_status_t vdev_register(int level, struct vdev_module *module)
{
    int i;
    hvmm_status_t result = HVMM_STATUS_BUSY;

    for (i = 0; i < MAX_VDEV; i++) {
        if (!_vdev_module[level][i]) {
            _vdev_module[level][i] = module;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }

    if (result != HVMM_STATUS_SUCCESS) {
        printh("vdev : Failed registering vdev '%s', max %d full\n",
                module->name, MAX_VDEV);
    }

    return result;
}

/**
 * \brief Lookup the virtual deivce address, using the archtecture specific
 * information \a info and current archtecture specific register \a regs.
 *
 * \retval virtual device number
 * \retval -1 This is an internal error.
 */
int32_t vdev_find(int level, struct arch_vdev_trigger_info *info,
        struct arch_regs *regs)
{
    int32_t i;
    int32_t vdev_num = VDEV_NOT_FOUND;
    struct vdev_module *vdev;

    for (i = 0; i < _vdev_size[level]; i++) {
        vdev = _vdev_module[level][i];
        if (!vdev) {
            printh("vdev : Could not get module, level : %d, i : %d\n",
                    level, i);
            break;
        }
        if (!vdev->ops) {
            printh("vdev : Could not get operation, level : %d, i : %d\n",
                    level, i);
            break;
        }
        if (!vdev->ops->check)
            continue;
        if (!vdev->ops->check(info, regs)) {
            vdev_num = i;
            break;
        }
    }

    return vdev_num;
}

int32_t vdev_read(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs)
{
    int32_t size = 0;
    struct vdev_module *vdev = _vdev_module[level][num];

    if (!vdev) {
        printh("vdev : Could not get module, level : %d, i : %d\n",
                level, num);
        return VDEV_ERROR;
    }

    if (!vdev->ops) {
        printh("vdev : Could not get operation, level : %d, i : %d\n",
                level, num);
        return VDEV_ERROR;
    }

    if (vdev->ops->read)
        size = vdev->ops->read(info, regs);

    return size;
}

int32_t vdev_write(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs)
{
    int32_t size = 0;
    struct vdev_module *vdev = _vdev_module[level][num];

    if (!vdev) {
        printh("vdev : Could not get module, level : %d, i : %d\n",
                level, num);
        return VDEV_ERROR;
    }

    if (!vdev->ops) {
        printh("vdev : Could not get operation, level : %d, i : %d\n",
                level, num);
        return VDEV_ERROR;
    }

    if (vdev->ops->write)
        size = vdev->ops->write(info, regs);

    return size;
}

hvmm_status_t vdev_post(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct vdev_module *vdev = _vdev_module[level][num];

    if (!vdev) {
        printh("vdev : Could not get module, level : %d, i : %d\n",
                level, num);
        return HVMM_STATUS_UNKNOWN_ERROR;
    }

    if (!vdev->ops) {
        printh("vdev : Could not get operation, level : %d, i : %d\n",
                level, num);
        return HVMM_STATUS_UNKNOWN_ERROR;
    }

    if (vdev->ops->post)
        result = vdev->ops->post(info, regs);

    return result;
}

hvmm_status_t vdev_save(vmid_t vmid)
{
    int i, j;
    struct vdev_module *vdev;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* TODO : change one level iteration */
    for (i = 0; i < VDEV_LEVEL_MAX; i++) {
        for (j = 0; j < _vdev_size[i]; j++) {
            vdev = _vdev_module[i][j];
            if (!vdev->ops->save)
                continue;

            result = vdev->ops->save(vmid);
            if (result) {
                printh("vdev : save error, name : %s\n", vdev->name);
                return result;
            }
        }
    }

    return result;
}

hvmm_status_t vdev_restore(vmid_t vmid)
{
    int i, j;
    struct vdev_module *vdev;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* TODO : change one level iteration */
    for (i = 0; i < VDEV_LEVEL_MAX; i++) {
        for (j = 0; j < _vdev_size[i]; j++) {
            vdev = _vdev_module[i][j];
            if (!vdev->ops->restore)
                continue;

            result = vdev->ops->restore(vmid);
            if (result) {
                printh("vdev : save error, name : %s\n", vdev->name);
                return result;
            }
        }
    }

    return result;
}

hvmm_status_t vdev_module_initcall(initcall_t fn)
{
    return  fn();
}

/**
 * \brief Initailize Virtual Device Framework. The init function should
 * only be used in the entire system.
 *
 * \retval 0 on success
 * \retval -1 This is an internal error.
 */
hvmm_status_t vdev_init(void)
{
    int i, j;
    initcall_t *fn;
    struct vdev_module *vdev;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();

    if (!cpu) {
        for (fn = __vdev_module_high_start; fn < __vdev_module_high_end; fn++) {
            if (vdev_module_initcall(*fn)) {
                printh("vdev : high initial call error\n");
                return HVMM_STATUS_UNKNOWN_ERROR;
            }
            _vdev_size[VDEV_LEVEL_HIGH]++;
        }

        for (fn = __vdev_module_high_end; fn < __vdev_module_middle_end; fn++) {
            if (vdev_module_initcall(*fn)) {
                printh("vdev : middle initial call error\n");
                return HVMM_STATUS_UNKNOWN_ERROR;
            }
            _vdev_size[VDEV_LEVEL_MIDDLE]++;
        }

        for (fn = __vdev_module_middle_end; fn < __vdev_module_low_end; fn++) {
            if (vdev_module_initcall(*fn)) {
                printh("vdev : low initial call error\n");
                return HVMM_STATUS_UNKNOWN_ERROR;
            }
            _vdev_size[VDEV_LEVEL_LOW]++;
        }
    }

    for (i = 0; i < VDEV_LEVEL_MAX; i++) {
        for (j = 0; j < _vdev_size[i]; j++) {
            vdev = _vdev_module[i][j];
            if (!vdev->ops) {
                printh("vdev : Could not get operation, level : %d, i : %d\n",
                        i, j);
                return HVMM_STATUS_UNKNOWN_ERROR;
            }

            if (!vdev->ops->init)
                continue;

            result = vdev->ops->init();
            if (result) {
                printh("vdev : Initial error, name : %s\n", vdev->name);
                return result;
            }
        }
    }

    return result;
}
