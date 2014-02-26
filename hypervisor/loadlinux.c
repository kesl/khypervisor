#include "loadlinux.h"
#include <log/string.h>
#include <log/print.h>

/* list of possible tags */
#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009

#define tag_next(t)     ((struct atag *)((uint32_t *)(t) + (t)->hdr.size))
#define tag_size(type)  ((sizeof(struct atag_header) \
            + sizeof(struct type)) >> 2)

/* structures for each atag */

struct atag_header {
    uint32_t size; /* length of tag in words including this header */
    uint32_t tag;  /* tag type */
};
struct atag_core {
    uint32_t flags;
    uint32_t pagesize;
    uint32_t rootdev;
};

struct atag_mem {
    uint32_t size;
    uint32_t start;
};
struct atag_serialnr {
    uint32_t low;
    uint32_t high;
};

struct atag_revision {
    uint32_t rev;
};
struct atag_cmdline {
    char cmdline[1];
};
struct atag_ramdisk {
    uint32_t flags;      /* bit 0 = load, bit 1 = prompt */
    uint32_t size;       /* decompressed ramdisk size in _kilo_ bytes */
    uint32_t start;      /* starting block of floppy-based RAM disk image */
};
struct atag_videotext {
    uint8_t x;           /* width of display */
    uint8_t y;           /* height of display */
    uint16_t video_page;
    uint8_t video_mode;
    uint8_t video_cols;
    uint16_t video_ega_bx;
    uint8_t video_lines;
    uint8_t video_isvga;
    uint16_t video_points;
};
struct atag_initrd2 {
    uint32_t start;      /* physical start address */
    uint32_t size;       /* size of compressed ramdisk image in bytes */
};
struct atag_videolfb {
    uint16_t lfb_width;
    uint16_t lfb_height;
    uint16_t lfb_depth;
    uint16_t lfb_linelength;
    uint32_t lfb_base;
    uint32_t lfb_size;
    uint8_t red_size;
    uint8_t red_pos;
    uint8_t green_size;
    uint8_t green_pos;
    uint8_t blue_size;
    uint8_t blue_pos;
    uint8_t rsvd_size;
    uint8_t rsvd_pos;
};
struct atag {
    struct atag_header hdr;
    union {
        struct atag_core core;
        struct atag_mem mem;
        struct atag_videotext videotext;
        struct atag_ramdisk ramdisk;
        struct atag_initrd2 initrd2;
        struct atag_serialnr serialnr;
        struct atag_revision revision;
        struct atag_videolfb videolfb;
        struct atag_cmdline cmdline;
    } u;
};

static struct atag *_params; /* used to point at the current tag */

static void setup_core_tag(void *address, long pagesize)
{
    /* Initialise parameters to start at given address */
    _params = (struct atag *)address;
    /* start with the core tag */
    _params->hdr.tag = ATAG_CORE;
    _params->hdr.size = tag_size(atag_core); /* size the tag */
    _params->u.core.flags = 1;               /* ensure read-only */
    _params->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
    /* zero root device (typicaly overidden from commandline )*/
    _params->u.core.rootdev = 0;
    /* move pointer to next tag */
    _params = tag_next(_params);
}

static void setup_revision_tag(void)
{
    _params->hdr.tag = ATAG_REVISION;
    _params->hdr.size = tag_size(atag_revision);
    _params->u.revision.rev = 0xcfdfdfdf;
    _params = tag_next(_params);
}

static void setup_cmdline_tag(const char *line)
{
    int linelen = strlen(line);
    if (!linelen)
        return;    /* do not insert a tag for an empty commandline */

    _params->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
    _params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;
    strcpy(_params->u.cmdline.cmdline, line); /* place commandline into tag */
    _params = tag_next(_params);              /* move pointer to next tag */
}

static void setup_mem_tag(uint32_t start, uint32_t len)
{
    printh("setup_mem_tag start :  %x len : %x\n", start, len);
    _params->hdr.tag = ATAG_MEM;             /* Memory tag */
    _params->hdr.size = tag_size(atag_mem);  /* size tag */
    /* Start of memory area (physical address) */
    _params->u.mem.start = start;
    _params->u.mem.size = len;               /* Length of area */
    _params = tag_next(_params);              /* move pointer to next tag */
}

static void setup_end_tag(void)
{
    _params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    _params->hdr.size = 0;                   /* zero length */
}

void loadlinux_setup_tags(uint32_t *src)
{
    char *commandline =
            "root=/dev/ram rw earlyprintk console=ttyAMA0 "
            "mem=256M rdinit=/sbin/init";
    /* standard core tag 4k pagesize */
    setup_core_tag(src + (0x100 / 4), 4096);
    /* commandline setting root device */
    setup_cmdline_tag(commandline);
    setup_revision_tag();
    setup_mem_tag((uint32_t)(src - (0x20000000 / 4)), 0x10000000);
    /* end of tags */
    setup_end_tag();
}
