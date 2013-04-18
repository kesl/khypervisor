/*
 * Copyright (c) 2012 Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Linaro Limited nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 */

#ifndef SEMI_LOADER_H
#define SEMI_LOADER_H

#define ATAG_NONE 0x00000000
#define ATAG_CORE 0x54410001
#define ATAG_MEM 0x54410002
#define ATAG_INITRD2 0x54420005
#define ATAG_CMDLINE 0x54410009

struct atag_header {
	unsigned size;
	unsigned tag;
};

struct atag_core {
	unsigned flags;
	unsigned pagesize;
	unsigned rootdev;
};

struct atag_mem {
	unsigned size;
	unsigned start;
};

struct atag_initrd2 {
	unsigned start;
	unsigned size;
};

static const char uboot_image_header_magic[] = {
	0x27, 0x05, 0x19, 0x56
};
#define UBOOT_IMAGE_HEADER_SIZE 0x40

#define ZIMAGE_MAGIC_OFFSET 36
#define ZIMAGE_MAGIC 0x016f2818UL

#define PHYS_OFFSET 0x80000000
#define PHYS_SIZE 0x80000000 /* can limit on kernel cmdline if necessary */
#define ATAGS_OFFSET 0x100
#define TEXT_OFFSET 0x8000
#define INITRD_OFFSET 0xD00000 /* qemu uses the same random offset */

#define FDT_SIZE_MAX 0x10000	/* maximum size allowed for device tree blob */

#define SEMI_CMDLINE_MAX 0x1000	/* maximum semihosting command line length */

/*
 * Align number <n> or pointer <p> up to the next boundary of size <size>.
 * <size> must be a power of two.
 */
#define ALIGN_INT(n, size) (((n) + ((size) - 1)) & ~((size) - 1))
#define ALIGN(p, size) ((void *)ALIGN_INT((unsigned)(p), size))

struct loader_info {
	unsigned kernel_size;	/* nonzero indicates preloaded kernel size */
	unsigned initrd_start;	/* start of preloaded initrd, if any */
	unsigned initrd_size;
	unsigned cmdline_start;	/* start of cmdline buffer */
	unsigned cmdline_size;

	/* The remaining fields are set by the loader: */

	/* There could be a built-in FDT, but currently that it not supported */
	unsigned fdt_start;	/* start of device tree blob, if any */
	unsigned fdt_size;

	unsigned atags_start;
	unsigned kernel_entry;	/* kernel entry point */
};

void load_kernel(struct loader_info *info);

/* removed __boot_kernel() - simon */

#endif /* ! SEMI_LOADER_H */
