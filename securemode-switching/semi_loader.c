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

#include <string.h>
#include "libfdt.h"
#include "semihosting.h"
#include "semi_loader.h"

static void _print_info(char const **strings)
{
	char const *string;

	semi_write0("[bootwrapper] ");
	while((string = *strings++))
		semi_write0(string);
}

#define info(strings...) do {					\
	char const *__info_strings[] = { strings, NULL };	\
								\
	_print_info(__info_strings);				\
} while(0)

#define warn(strings...) info("WARNING: ", strings)
#define error(strings...) info("ERROR: ", strings)

#define fatal(strings...) do {			\
	error(strings);				\
	semi_fatal("[bootwrapper] BOOT FAILED\n");	\
} while(0)

#define CMDLINE_KERNEL "--kernel"
#define CMDLINE_INITRD "--initrd"
#define CMDLINE_NOINITRD "--no-initrd"
#define CMDLINE_DTB "--dtb"
#define CMDLINE_FDT "--fdt"	/* deprecated */
#define CMDLINE_REST "-- "

static void _usage_fatal(void)
{
	info("Usage: [" CMDLINE_KERNEL " <kernel filename>] ["
		CMDLINE_NOINITRD "|" CMDLINE_INITRD " <initrd filename>] ["
		CMDLINE_DTB " <dtb filename>] ["
		CMDLINE_REST "<kernel boot arguments>]\n");
	fatal("Incorrect bootwrapper command-line.\n");
}

#define usage_fatal(strings...) do {		\
	error(strings);				\
	_usage_fatal();				\
} while(0)

static void atag_append(void **dest, unsigned tag, void const *data, unsigned size)
{
	char *d = *dest;
	unsigned padded_size = ALIGN_INT(size, 4) + 8;
	struct atag_header header = {
		padded_size >> 2,
		tag
	};

	if(tag == ATAG_NONE)
		header.size = 0;

	memcpy(d, &header, sizeof header);
	memcpy(d + 8, data, size);

	if(padded_size > size + 8)
		memset(d + 8 + size, 0, padded_size - (size + 8));

	*dest = d + padded_size;
}

static int _fdt_make_node(void *fdt, int parentoffset, const char *name)
{
	int e;

	e = fdt_subnode_offset(fdt, parentoffset, name);
	if(e != -FDT_ERR_NOTFOUND)
		return e;

	return fdt_add_subnode(fdt, parentoffset, name);
}

static void _fdt_address_and_size_cells(void *fdt, int *addrcells, int *sizecells)
{
	int e;
	uint32_t const *p;

	if(!(p = fdt_getprop(fdt, 0, "#address-cells", &e)))
		goto libfdt_error;
	if(e != 4)
		goto size_error;
	*addrcells = fdt32_to_cpu(*p);
	if(!(p = fdt_getprop(fdt, 0, "#size-cells", &e)))
		goto libfdt_error;
	if(e != 4)
		goto size_error;
	*sizecells = fdt32_to_cpu(*p);

	/*
	 * Sanity-check address sizes, since addresses and sizes which do
	 * not take up exactly 4 or 8 bytes are not supported.
	 */
	if ((*addrcells != 1 && *addrcells != 2) ||
	    (*sizecells != 1 && *sizecells != 2))
		goto size_error;

	return;

libfdt_error:
	fatal("libfdt: ", fdt_strerror(e), ", while looking for #address-cells/#size-cells\n");

size_error:
	fatal("Unexpected/invalid #address-cells/#size-cells in device tree\n");
}

static void update_fdt(void **dest, struct loader_info *info)
{
	int e;
	int _chosen;
	void *fdt;
	uint32_t const *p;
	int addrcells, sizecells;

	if(!info->fdt_start)
		return;

	fdt = ALIGN(*dest, 4);
	if((e = fdt_open_into((void *)info->fdt_start, fdt, FDT_SIZE_MAX)) < 0)
		goto libfdt_error;

	_fdt_address_and_size_cells(fdt, &addrcells, &sizecells);

	/*
	 * Add a memory node, but only if there isn't one already.  If
	 * there is already a memory node with non-zero size, it was put
	 * in the DT on purpose and should take precedence over our
	 * guesses.  Otherwise, make a memory node with the appropriate
	 * parameters.
	 */
	{
		int offset, depth = 0;
		int _memory;
		uint32_t reg[4];

		for(offset = fdt_next_node(fdt, 0, &depth); offset >= 0;
				offset = fdt_next_node(fdt, offset, &depth)) {
			char const *name;

			if(depth != 1)
				continue;

			name = fdt_get_name(fdt, offset, (void *)0);
			if(!strcmp(name, "memory") ||
					!strncmp(name, "memory@", 7)) {
				p = fdt_getprop(fdt, offset, "reg", &e);
				if(e < 0)
					goto libfdt_error;

				/* Check whether the <size> part of the <addr>,<size> tuple is nonzero */
				if(fdt32_to_cpu(p[addrcells]) != 0)
					goto no_add_memory;
				if(sizecells == 2 && fdt32_to_cpu(p[addrcells + 1]) != 0)
					goto no_add_memory;
			}
		}

		if((e = _fdt_make_node(fdt, 0, "memory")) < 0)
			goto libfdt_error;
		_memory = e;

		/* This assumes PHYS_OFFSET and PHYS_SIZE are 32 bits, though
		 * the fdt cells we put them in may not be.
		 */
		reg[0] = reg[1] = reg[2] = reg[3] = 0;
		reg[addrcells - 1] = cpu_to_fdt32(PHYS_OFFSET);
		reg[addrcells + sizecells - 1] = cpu_to_fdt32(PHYS_SIZE);
		
		if((e = fdt_setprop(fdt, _memory, "reg", &reg,
				    sizeof(reg[0]) * (addrcells + sizecells))) < 0)
			goto libfdt_error;

		if((e = fdt_setprop_string(fdt, _memory, "device_type",
				"memory")) < 0)
			goto libfdt_error;
	}

no_add_memory:
	/* populate the "chosen" node */

	if((e = _fdt_make_node(fdt, 0, "chosen")) < 0)
		goto libfdt_error;
	_chosen = e;

	e = fdt_setprop_string(fdt, _chosen, "bootargs",
				(char const *)info->cmdline_start);
	if(e < 0)
		goto libfdt_error;

	if(info->initrd_start) {
		uint32_t initrd_end = info->initrd_start + info->initrd_size;
		/* It's not documented whether these cells should honour
		 * #address-cells. Currently the kernel accepts them as being
		 * addresses of either size, so we leave them as 32 bits for now.
		 */
		if((e = fdt_setprop_cell(fdt, _chosen, "linux,initrd-start",
					info->initrd_start)) < 0)
			goto libfdt_error;

		if((e = fdt_setprop_cell(fdt, _chosen, "linux,initrd-end",
					initrd_end)) < 0)
			goto libfdt_error;
	}

	/* success */

	/* clean up */
	fdt_pack(fdt);
	info->fdt_start = (unsigned)fdt;
	info->fdt_size = fdt_totalsize(fdt);
	info("FDT updated.\n");

	return;
		
libfdt_error:
	fatal("libfdt: ", fdt_strerror(e), ", while updating device tree\n");
}

/* For accessing 32-bit device ports */
#define io32(p) (*(volatile uint32_t *)(p))

static void init_cci(unsigned cci)
{
	info("Initialising CCI\n");

	/*
	 * Ideally, the CCI device tree binding would include suitable
	 * information so we can correctly configure the CCI, but for
	 * now we'll just hard-code settings for the present A15xA7
	 * models.
	 */

	/* Turn on CCI snoops and DVM messages */
	io32(cci+0x4000) = 0x3;   /* A15 cluster */
	io32(cci+0x5000) = 0x3;   /* A7 cluster */

	/* Wait while change pending bit of status register is set */
	while(io32(cci+0xc) & 0x1)
		{}
}

static void configure_from_fdt(struct loader_info *info)
{
	void *fdt = (void *)info->fdt_start;
	uint32_t const *p;
	int addrcells, sizecells;
	int offset, len;

	if(!fdt)
		return;

	_fdt_address_and_size_cells(fdt, &addrcells, &sizecells);

	/* See if there is a CCI device to initialise */
	offset = fdt_node_offset_by_compatible(fdt, 0, "arm,cci");
	if (offset >= 0) {
		p = fdt_getprop(fdt, offset, "reg", &len);
		if(len != (addrcells + sizecells) * 4)
			info("Failed parsing device-tree node for CCI\n");
		else {
			/*
			 * p[addrcells - 1] is the least significant 32-bits of
			 * the address for the CCI. On 32-bit CPUs any additional
			 * address bits had  better be zero otherwise we can't
			 * access it as we don't enable the MMU.
			 */
			init_cci(fdt32_to_cpu(p[addrcells - 1]));
		}
	}

	return;
}

static int is_space(char c)
{
	return c == ' ';
}

static void skip_space(char **s)
{
	char *t = *s;

	for(t = *s; is_space(*t); t++);
	*s = t;
}

static void find_space(char **s)
{
	char *t = *s;

	for(t = *s; *t && !is_space(*t); t++);
	*s = t;
}

static int match_word(char **s, char const *string)
{
	unsigned l;

	l = strlen(string);
	if(strncmp(*s, string, l))
		return 0;

	*s += l;
	skip_space(s);
	return 1;
}

/*
 * Match an option with a mandatory argument.
 * On success, a pointer to the argument is returned, with leading and
 * trailing whitespace stripped.
 */
static char *match_option(char **s, char const *option_string)
{
	char *arg;

	if(!match_word(s, option_string))
		return (void *)0;

	if(!**s)
		usage_fatal("Option requires as argument: ",
			option_string, "\n");

	/* otherwise, *s now points to the argument */
	arg = *s;

	find_space(s);		/* find the end of the argument */
	if(**s) {
		*(*s)++ = '\0';	/* null-terminate if necessary */
		skip_space(s);	/* skip any remaining space */
	}

	return arg;
}

static void load_file_essential(void **dest, char const *filename,
				unsigned *size, char const *failmsg)
{
	if(semi_load_file(dest, size, filename))
		fatal(failmsg, ": \"", filename, "\"\n");
}

/* is_uboot_image_format - to check an image is in uboot image format or not
 * @start: start address of the image
 * @size:  size of the image
 *
 * Returns:
 *  0: no
 *  1: yes
 */
static int is_uboot_image_format(const char *start, const unsigned size)
{
	if(size <= UBOOT_IMAGE_HEADER_SIZE)
		return 0;

	return !memcmp(start, uboot_image_header_magic,
					 sizeof uboot_image_header_magic);
}

/* Move the kernel if necessary, based on the image type: */
static void correct_kernel_location(struct loader_info *info)
{
	char *const text_start = (char *)(PHYS_OFFSET + TEXT_OFFSET);
	char *const text_end = text_start + info->kernel_size;
	char *const uImage_payload = text_start + UBOOT_IMAGE_HEADER_SIZE;
	unsigned long *const zImage_magic_p = (unsigned long *)(
		uImage_payload + ZIMAGE_MAGIC_OFFSET);

	/*
	 * If the image is not a uImage, then it is a raw Image or zImage,
	 * and no action is necessary:
	 */
	if(!is_uboot_image_format(text_start, info->kernel_size))
		return;

	warn("Ignoring uImage meta-data\n");

	/*
	 * If the uImage payload is a zImage, the position-independent
	 * nature of the zImage header means that no relocation is
	 * needed.  Instead, just enter at the start of the loaded
	 * zImage header:
	 */
	if(text_end >= (char *)&zImage_magic_p[1]
			&& *zImage_magic_p == ZIMAGE_MAGIC) {
		info->kernel_entry += UBOOT_IMAGE_HEADER_SIZE;
		return;
	}

	/*
	 * Otherwise, move the payload to replace the uImage header, and
	 * leave the entry point unmodified.
	 */
	memmove(text_start, uImage_payload,
		info->kernel_size - UBOOT_IMAGE_HEADER_SIZE);
}

static void correct_initrd_location(struct loader_info *info)
{
	/*
	 * if initrd image is in u-boot image format,
	 * move initrd_start and initrd_size to ignore the header
	 */
	if(is_uboot_image_format((char *)info->initrd_start,
							info->initrd_size)) {
		warn("Ignoring uInitrd meta-data\n");
		info->initrd_start += UBOOT_IMAGE_HEADER_SIZE;
		info->initrd_size -= UBOOT_IMAGE_HEADER_SIZE;
	}

	return;
}

static char semi_cmdline[SEMI_CMDLINE_MAX];

static char *kernel_arg = (void *)0;
static char *initrd_arg = (void *)0;
static char *fdt_arg = (void *)0;
static char *dtb_arg = (void *)0;
static char *cmdline_arg = (void *)0;
static char *noinitrd_arg = (void *)0;

static const struct {
	char const *option_string;
	char **argp;
	enum { OPT_ARG, OPT_BOOL, OPT_REST } action;
} options[] = {
	{ CMDLINE_KERNEL,	&kernel_arg,	OPT_ARG		},
	{ CMDLINE_INITRD,	&initrd_arg,	OPT_ARG		},
	{ CMDLINE_NOINITRD,	&noinitrd_arg,	OPT_BOOL	},
	{ CMDLINE_FDT,		&fdt_arg,	OPT_ARG		},
	{ CMDLINE_DTB,		&dtb_arg,	OPT_ARG		},
	{ CMDLINE_REST,		&cmdline_arg,	OPT_REST	},
};

void load_kernel(struct loader_info *info)
{
	unsigned i;
	char *cmdline = semi_cmdline;
	int cmdline_length;
	void *phys = (char *)(PHYS_OFFSET + TEXT_OFFSET);
	void *atagp = (char *)(PHYS_OFFSET + ATAGS_OFFSET);

	union {
		struct atag_core core;
		struct atag_mem mem;
		struct atag_initrd2 initrd;
	} atag;

	/* Fetch the command line: */

	if(semi_get_cmdline(semi_cmdline, sizeof semi_cmdline,
			    &cmdline_length) ||
				cmdline_length >= sizeof semi_cmdline) {
		warn("Failed to get semihosting command line, using built-in defaults\n");
		cmdline_length = 0;
	}
	cmdline[cmdline_length] = '\0';

	/* Parse the arguments (if any): */

	skip_space(&cmdline);

	while(*cmdline) {
		for(i = 0; i < sizeof options / sizeof *options; i++) {
			char *arg;

			switch(options[i].action) {
			case OPT_BOOL:
				if(!match_word(&cmdline,
						options[i].option_string))
					continue;

				*options[i].argp = cmdline; /* non-NULL */
				goto next_arg;

			case OPT_REST:
				if(!match_word(&cmdline,
						options[i].option_string))
					continue;

			*options[i].argp = cmdline;
			goto args_done;

			case OPT_ARG:
				arg = match_option(&cmdline,
					options[i].option_string);
				if(!arg)
					continue;

				if(*options[i].argp)
					usage_fatal("Duplicate option ",
						options[i].option_string);

				/* otherwise, option was parsed successfully: */
				*options[i].argp = arg;
				goto next_arg;
			}
		} /* for(i) */

		/* Failed to match any expected option: */
		usage_fatal("Invalid option(s): ", cmdline);

	next_arg: ;
	} /* while(*cmdline) */

args_done:
	if(initrd_arg && noinitrd_arg)
		usage_fatal("Option --initrd conflicts with --no-initrd.\n");

	if(fdt_arg) {
		warn("--fdt is deprecated.  Please use --dtb instead.\n");

		if(dtb_arg)
			usage_fatal("--fdt conflicts with --dtb.\n");
		else
			dtb_arg = fdt_arg;
	}

	/*
	 * Now, proceed to load images and set up ATAGs.
	 * For simplicity, ATAGs are generated even if there is a DTB
	 */

	/* built-in FDT not supported, for now */
	info->fdt_start = info->fdt_size = 0;

	info->atags_start = (unsigned)atagp;

	memset(&atag.core, 0, sizeof atag.core);
	atag_append(&atagp, ATAG_CORE, &atag.core, sizeof atag.core);

	/* create the essential ATAGs */

	atag.mem.start = PHYS_OFFSET;
	atag.mem.size = PHYS_SIZE;
	atag_append(&atagp, ATAG_MEM, &atag.mem, sizeof atag.mem);

	/* load the kernel */

	info->kernel_entry = (unsigned)phys;

	if(kernel_arg) {
		load_file_essential(&phys, kernel_arg, &info->kernel_size,
			"Failed to load kernel image");
		info("Loaded kernel: ", kernel_arg, "\n");
	} else if(info->kernel_size) {
		info("Using built-in kernel\n");
		phys += info->kernel_size;
	} else
		usage_fatal("Expected " CMDLINE_KERNEL "\n");

	/* move the kernel to the correct place, if necessary */

	correct_kernel_location(info);

	phys = (char *)(PHYS_OFFSET + INITRD_OFFSET);

	/* load the initrd */

	atag.initrd.size = 0;

	if(initrd_arg) {
		unsigned start = (unsigned)phys;

		load_file_essential(&phys, initrd_arg, NULL,
			"Failed to load initrd image");
		info("Loaded initrd: ", initrd_arg, "\n");

		info->initrd_start = start;
		info->initrd_size = (unsigned)phys - start;

		correct_initrd_location(info);

		atag.initrd.start = info->initrd_start;
		atag.initrd.size = info->initrd_size;

	} else if(info->initrd_size) {
		if(noinitrd_arg) {
			info->initrd_size = 0;
			info("Built-in initrd discarded, as requested\n");
		} else {
			info("Using built-in initrd\n");

			atag.initrd.start = info->initrd_start;
			atag.initrd.size = info->initrd_size;
		}
	} else
		info->initrd_size = 0;

	if(atag.initrd.size)
		atag_append(&atagp, ATAG_INITRD2, &atag.initrd, sizeof atag.initrd);

	/* load the FDT, if specified */

	if(dtb_arg) {
		phys = ALIGN(phys, 4);
		info->fdt_start = (unsigned)phys;

		load_file_essential(&phys, dtb_arg, NULL,
			"Failed to load device tree blob");
		info("Loaded FDT: ", dtb_arg, "\n");

		info->fdt_size = (unsigned)phys - info->fdt_start;
	}

	/*
	 * The FDT will get modified to reflect bootargs, initrd and memory
	 * configuration later.
	 */

	/* set the command line */

	if(cmdline_arg) {
		info->cmdline_start = (unsigned)cmdline_arg;
		info->cmdline_size = strlen(cmdline_arg) + 1;
	} else if(info->cmdline_size)
		info("Using built-in kernel bootargs\n");

	/* cmdline_size is presumed to include a NUL terminator: */
	if(info->cmdline_size) {
		atag_append(&atagp, ATAG_CMDLINE,
			(char *)info->cmdline_start, info->cmdline_size);
		info("Kernel bootargs: ", (char *)info->cmdline_start, "\n");
	}

	atag_append(&atagp, ATAG_NONE, 0, 0);

	update_fdt(&phys, info);

	configure_from_fdt(info);
}
