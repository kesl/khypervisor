#include "loadlinux.h"
#include "string.h"
#include "print.h"


static struct atag *params; /* used to point at the current tag */

void setup_core_tag( void * address, long pagesize )
{
    params = (struct atag *)address;         /* Initialise parameters to start at given address */

    params->hdr.tag = ATAG_CORE;            /* start with the core tag */
    params->hdr.size = tag_size(atag_core); /* size the tag */
    params->u.core.flags = 1;               /* ensure read-only */
    params->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
    params->u.core.rootdev = 0;             /* zero root device (typicaly overidden from commandline )*/

    params = tag_next(params);              /* move pointer to next tag */
}

void setup_revision_tag( void )
{
    params->hdr.tag = ATAG_REVISION;
    params->hdr.size = tag_size (atag_revision);
    params->u.revision.rev = 0xcfdfdfdf;
    params = tag_next (params);
}

void setup_cmdline_tag( const char * line )
{
    int linelen = strlen(line);

    if(!linelen)
        return;                             /* do not insert a tag for an empty commandline */

    params->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
    params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

    strcpy(params->u.cmdline.cmdline,line); /* place commandline into tag */

    params = tag_next(params);              /* move pointer to next tag */
}

void setup_mem_tag( uint32_t start, uint32_t len )
{
    printh("setup_mem_tag start :  %x len : %x\n", start, len);
    params->hdr.tag = ATAG_MEM;             /* Memory tag */
    params->hdr.size = tag_size(atag_mem);  /* size tag */

    params->u.mem.start = start;            /* Start of memory area (physical address) */
    params->u.mem.size = len;               /* Length of area */

    params = tag_next(params);              /* move pointer to next tag */
}

void setup_end_tag( void )
{
    params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    params->hdr.size = 0;                   /* zero length */
}

void setup_tags( uint32_t *src )
{
    char *commandline = "root=/dev/ram rw earlyprintk console=ttyAMA0 mem=256M rdinit=/sbin/init";
    setup_core_tag(src+(0x100/4), 4096);       /* standard core tag 4k pagesize */
    setup_cmdline_tag(commandline);    /* commandline setting root device */
    setup_revision_tag();
    setup_mem_tag((uint32_t)(src-(0x20000000/4)), 0x10000000);
    /* end of tags */
    setup_end_tag();
}
