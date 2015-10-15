#ifndef __HVMM_TYPES_H__
#define __HVMM_TYPES_H__

#include <arch_types.h>
#define VMID_INVALID    0xFF
#define PIRQ_INVALID    0xFFFFFFFF
#define VIRQ_INVALID    PIRQ_INVALID

typedef enum hvmm_status {
    HVMM_STATUS_SUCCESS = 0,
    HVMM_STATUS_UNKNOWN_ERROR = -1,
    HVMM_STATUS_UNSUPPORTED_FEATURE = -2,
    HVMM_STATUS_BUSY = -3,
    HVMM_STATUS_BAD_ACCESS = -4,
    HVMM_STATUS_NOT_FOUND = -5,
    HVMM_STATUS_IGNORED = -6,
} hvmm_status_t;

typedef enum vcpu_state_t {
    VCPU_RUNNING    = 1,
    VCPU_WAIT       = 2,
    VCPU_BLOCKED    = 3
} vcpu_state_t;


typedef uint8_t vmcbid_t;
typedef uint8_t vcpuid_t;

#endif

