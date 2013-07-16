#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#include "arch_types.h"

#define u32 uint32_t
#define u64 uint64_t
#define u8 uint8_t

extern u64 do_udiv64(u64 dividend, u64 divisor, u64 * remainder);

static inline u64 udiv64(u64 value, u64 divisor)
{
	u64 r;
	return do_udiv64(value, divisor, &r);
}

static inline u64 umod64(u64 value, u64 divisor)
{
	u64 r;
	do_udiv64(value, divisor, &r);
	return r;
}


extern u32 do_udiv32(u32 dividend, u32 divisor, u32 * remainder);

static inline u32 udiv32(u32 value, u32 divisor)
{
	u32 r;
	return do_udiv32(value, divisor, &r);
}

static inline u32 umod32(u32 value, u32 divisor)
{
	u32 r;
	do_udiv32(value, divisor, &r);
	return r;
}



#endif
