#ifndef __ARMV8_PROCESSOR_H__
#define __ARMV8_PROCESSOR_H__
#include "arch_types.h"
#include "stringify.h"

#define read_sr64(name)       ({ uint64_t rval; asm volatile (\
                              " mrs %0, "#name \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sr64(val, name) asm volatile(\
                              "msr "#name", %0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_sr32(name)       ({ unint32_t rval; asm volatile (\
                              " mrs %0, "__stringify(name) \
                                : "=r" (rval) " " "memory", "cc"); rval; })

#define write_sr32(val, name) asm volatile(\
                              "msr "#name", %0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#endif
