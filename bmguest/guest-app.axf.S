
guest-app.axf:     file format elf32-littlearm


Disassembly of section .text:

e0000000 <guest_start>:
	
	@ Guest start code
.global guest_start
guest_start:

	b 	guest_entry
e0000000:	eaffffff 	b	e0000004 <guest_entry>

e0000004 <guest_entry>:
.type guest_start, %function

guest_entry:
	@ stack for guest
	ldr     sp, =guest_stacktop
e0000004:	e59fd008 	ldr	sp, [pc, #8]	; e0000014 <__semi_call+0x8>

	@ And call the C entrypoint
	bl      nrm_loop
e0000008:	eb000002 	bl	e0000018 <nrm_loop>

e000000c <__semi_call>:
        bkpt    0xab
#elif defined(__thumb__)
        @ Otherwise, different SVC numbers for ARM or Thumb mode
        svc    0xab
#else
        svc     0x123456
e000000c:	ef123456 	svc	0x00123456
#endif
        mov pc, lr
e0000010:	e1a0f00e 	mov	pc, lr
	b 	guest_entry
.type guest_start, %function

guest_entry:
	@ stack for guest
	ldr     sp, =guest_stacktop
e0000014:	ef000000 	.word	0xef000000

e0000018 <nrm_loop>:

#include "semihosting.h"

void nrm_loop(void) 
{
e0000018:	e92d4010 	push	{r4, lr}
	semi_write0("[nrm] starting...\n");
e000001c:	e59f0024 	ldr	r0, [pc, #36]	; e0000048 <nrm_loop+0x30>
e0000020:	eb00001d 	bl	e000009c <semi_write0>
e0000024:	e3a0400a 	mov	r4, #10
	int i = 0;
	for( i = 0; i < 10; i++ ) {
		semi_write0("[nrm] hello\n");
e0000028:	e59f001c 	ldr	r0, [pc, #28]	; e000004c <nrm_loop+0x34>
e000002c:	eb00001a 	bl	e000009c <semi_write0>
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> sec_loop() */
e0000030:	e1600070 	smc	0

void nrm_loop(void) 
{
	semi_write0("[nrm] starting...\n");
	int i = 0;
	for( i = 0; i < 10; i++ ) {
e0000034:	e2544001 	subs	r4, r4, #1
e0000038:	1afffffa 	bne	e0000028 <nrm_loop+0x10>
		semi_write0("[nrm] hello\n");
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> sec_loop() */
	}
	semi_write0("[nrm] done\n");
e000003c:	e59f000c 	ldr	r0, [pc, #12]	; e0000050 <nrm_loop+0x38>
}
e0000040:	e8bd4010 	pop	{r4, lr}
	for( i = 0; i < 10; i++ ) {
		semi_write0("[nrm] hello\n");
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> sec_loop() */
	}
	semi_write0("[nrm] done\n");
e0000044:	ea000014 	b	e000009c <semi_write0>
e0000048:	e0000548 	.word	0xe0000548
e000004c:	e000055c 	.word	0xe000055c
e0000050:	e000056c 	.word	0xe000056c

e0000054 <semi_open>:

#include <string.h>
#include "semihosting.h"

int semi_open(char const *filename, int mode)
{
e0000054:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
e0000058:	e24dd014 	sub	sp, sp, #20
		char const *filename;
		int mode;
		int filename_length;
	} args;

	args.filename = filename;
e000005c:	e98d0003 	stmib	sp, {r0, r1}
	args.mode = mode;
	args.filename_length = strlen(filename);
e0000060:	eb0000a2 	bl	e00002f0 <strlen>

	return __semi_call(SYS_OPEN, &args);
e0000064:	e28d1004 	add	r1, sp, #4
		int filename_length;
	} args;

	args.filename = filename;
	args.mode = mode;
	args.filename_length = strlen(filename);
e0000068:	e58d000c 	str	r0, [sp, #12]

	return __semi_call(SYS_OPEN, &args);
e000006c:	e3a00001 	mov	r0, #1
e0000070:	ebffffe5 	bl	e000000c <__semi_call>
}
e0000074:	e28dd014 	add	sp, sp, #20
e0000078:	e8bd8000 	pop	{pc}

e000007c <semi_close>:

int semi_close(int fd)
{
e000007c:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
e0000080:	e24dd00c 	sub	sp, sp, #12
e0000084:	e28d1008 	add	r1, sp, #8
e0000088:	e5210004 	str	r0, [r1, #-4]!
	return __semi_call(SYS_CLOSE, &fd);
e000008c:	e3a00002 	mov	r0, #2
e0000090:	ebffffdd 	bl	e000000c <__semi_call>
}
e0000094:	e28dd00c 	add	sp, sp, #12
e0000098:	e8bd8000 	pop	{pc}

e000009c <semi_write0>:

int semi_write0(char const *string)
{
e000009c:	e1a01000 	mov	r1, r0
	return __semi_call(SYS_WRITE0, string);
e00000a0:	e3a00004 	mov	r0, #4
e00000a4:	eaffffd8 	b	e000000c <__semi_call>

e00000a8 <semi_read>:
}

int semi_read(int fd, char *buffer, int length)
{
e00000a8:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
e00000ac:	e24dd014 	sub	sp, sp, #20
		char *buffer;
		int length;
	} args;

	args.fd = fd;
	args.buffer = buffer;
e00000b0:	e98d0003 	stmib	sp, {r0, r1}
	args.length = length;

	return __semi_call(SYS_READ, &args);
e00000b4:	e28d1004 	add	r1, sp, #4
e00000b8:	e3a00006 	mov	r0, #6
		int length;
	} args;

	args.fd = fd;
	args.buffer = buffer;
	args.length = length;
e00000bc:	e58d200c 	str	r2, [sp, #12]

	return __semi_call(SYS_READ, &args);
e00000c0:	ebffffd1 	bl	e000000c <__semi_call>
}
e00000c4:	e28dd014 	add	sp, sp, #20
e00000c8:	e8bd8000 	pop	{pc}

e00000cc <semi_flen>:

int semi_flen(int fd)
{
e00000cc:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
e00000d0:	e24dd00c 	sub	sp, sp, #12
e00000d4:	e28d1008 	add	r1, sp, #8
e00000d8:	e5210004 	str	r0, [r1, #-4]!
	return __semi_call(SYS_FLEN, &fd);
e00000dc:	e3a0000c 	mov	r0, #12
e00000e0:	ebffffc9 	bl	e000000c <__semi_call>
}
e00000e4:	e28dd00c 	add	sp, sp, #12
e00000e8:	e8bd8000 	pop	{pc}

e00000ec <semi_get_cmdline>:

int semi_get_cmdline(char *buffer, int size, int *length)
{
e00000ec:	e92d4010 	push	{r4, lr}
e00000f0:	e24dd008 	sub	sp, sp, #8
		char *buffer;
		int size;
	} args;

	args.buffer = buffer;
	args.size = size;
e00000f4:	e88d0003 	stm	sp, {r0, r1}

	result = __semi_call(SYS_GET_CMDLINE, &args);
e00000f8:	e3a00015 	mov	r0, #21
e00000fc:	e1a0100d 	mov	r1, sp
{
	return __semi_call(SYS_FLEN, &fd);
}

int semi_get_cmdline(char *buffer, int size, int *length)
{
e0000100:	e1a04002 	mov	r4, r2
	} args;

	args.buffer = buffer;
	args.size = size;

	result = __semi_call(SYS_GET_CMDLINE, &args);
e0000104:	ebffffc0 	bl	e000000c <__semi_call>
	if(result)
e0000108:	e3500000 	cmp	r0, #0
e000010c:	1a000002 	bne	e000011c <semi_get_cmdline+0x30>
		return result;

	if(length)
e0000110:	e3540000 	cmp	r4, #0
		*length = args.size;
e0000114:	159d3004 	ldrne	r3, [sp, #4]
e0000118:	15843000 	strne	r3, [r4]

	return 0;
}
e000011c:	e28dd008 	add	sp, sp, #8
e0000120:	e8bd8010 	pop	{r4, pc}

e0000124 <semi_reportexc>:

int semi_reportexc(int reason)
{
e0000124:	e1a01000 	mov	r1, r0
	return __semi_call(SYS_REPORTEXC, (void *)reason);
e0000128:	e3a00018 	mov	r0, #24
e000012c:	eaffffb6 	b	e000000c <__semi_call>

e0000130 <semi_exit>:
}

void semi_exit(void)
{
	semi_reportexc(REPORTEXC_REASON_APP_EXIT);
e0000130:	e3a00026 	mov	r0, #38	; 0x26
{
	return __semi_call(SYS_REPORTEXC, (void *)reason);
}

void semi_exit(void)
{
e0000134:	e92d4008 	push	{r3, lr}
	semi_reportexc(REPORTEXC_REASON_APP_EXIT);
e0000138:	e3400002 	movt	r0, #2
e000013c:	ebfffff8 	bl	e0000124 <semi_reportexc>
e0000140:	eafffffe 	b	e0000140 <semi_exit+0x10>

e0000144 <semi_fatal>:
	while(1); /* should not be reached */
}

void semi_fatal(char const *message)
{
e0000144:	e92d4008 	push	{r3, lr}
	semi_write0(message);
e0000148:	ebffffd3 	bl	e000009c <semi_write0>
	semi_exit();
e000014c:	ebfffff7 	bl	e0000130 <semi_exit>

e0000150 <semi_load_file>:
}

int semi_load_file(void **dest, unsigned *size, char const *filename)
{
e0000150:	e92d45f8 	push	{r3, r4, r5, r6, r7, r8, sl, lr}
e0000154:	e1a06000 	mov	r6, r0
e0000158:	e1a07001 	mov	r7, r1
	int result = -1;	/* fail by default */
	int fd = -1;
	int filesize;

	fd = semi_open(filename, OPEN_RDONLY);
e000015c:	e1a00002 	mov	r0, r2
e0000160:	e3a01001 	mov	r1, #1
	semi_write0(message);
	semi_exit();
}

int semi_load_file(void **dest, unsigned *size, char const *filename)
{
e0000164:	e1a08002 	mov	r8, r2
	int result = -1;	/* fail by default */
	int fd = -1;
	int filesize;

	fd = semi_open(filename, OPEN_RDONLY);
e0000168:	ebffffb9 	bl	e0000054 <semi_open>
	if(fd == -1) {
e000016c:	e3700001 	cmn	r0, #1
{
	int result = -1;	/* fail by default */
	int fd = -1;
	int filesize;

	fd = semi_open(filename, OPEN_RDONLY);
e0000170:	e1a05000 	mov	r5, r0
	if(fd == -1) {
e0000174:	0a00001e 	beq	e00001f4 <semi_load_file+0xa4>
		semi_write0("Cannot open file: ");
		goto out;
	}

	filesize = semi_flen(fd);
e0000178:	ebffffd3 	bl	e00000cc <semi_flen>
	if(filesize == -1) {
e000017c:	e3700001 	cmn	r0, #1
	if(fd == -1) {
		semi_write0("Cannot open file: ");
		goto out;
	}

	filesize = semi_flen(fd);
e0000180:	e1a04000 	mov	r4, r0
	if(filesize == -1) {
e0000184:	0a00001d 	beq	e0000200 <semi_load_file+0xb0>
		semi_write0("Cannot get file size for: ");
		goto out;
	}

	if(semi_read(fd, *dest, filesize)) {
e0000188:	e1a00005 	mov	r0, r5
e000018c:	e5961000 	ldr	r1, [r6]
e0000190:	e1a02004 	mov	r2, r4
e0000194:	ebffffc3 	bl	e00000a8 <semi_read>
e0000198:	e250a000 	subs	sl, r0, #0
e000019c:	1a00000a 	bne	e00001cc <semi_load_file+0x7c>
		semi_write0("Could not read: ");
		goto out;
	}

	result = 0;	/* success */
	*dest = (char *)*dest + filesize;
e00001a0:	e5963000 	ldr	r3, [r6]

out:
	if(fd != -1)
		semi_close(fd);
e00001a4:	e1a00005 	mov	r0, r5
		semi_write0("Could not read: ");
		goto out;
	}

	result = 0;	/* success */
	*dest = (char *)*dest + filesize;
e00001a8:	e0833004 	add	r3, r3, r4
e00001ac:	e5863000 	str	r3, [r6]

out:
	if(fd != -1)
		semi_close(fd);
e00001b0:	ebffffb1 	bl	e000007c <semi_close>

	if(result) {	/* print context for the error message */
		semi_write0(filename);
		semi_write0("\n");
	} else
		if(size)
e00001b4:	e3570000 	cmp	r7, #0
e00001b8:	01a00007 	moveq	r0, r7
e00001bc:	08bd85f8 	popeq	{r3, r4, r5, r6, r7, r8, sl, pc}
			*size = filesize;
e00001c0:	e5874000 	str	r4, [r7]
e00001c4:	e1a0000a 	mov	r0, sl

	return result;
}
e00001c8:	e8bd85f8 	pop	{r3, r4, r5, r6, r7, r8, sl, pc}
		semi_write0("Cannot get file size for: ");
		goto out;
	}

	if(semi_read(fd, *dest, filesize)) {
		semi_write0("Could not read: ");
e00001cc:	e59f0038 	ldr	r0, [pc, #56]	; e000020c <semi_load_file+0xbc>
e00001d0:	ebffffb1 	bl	e000009c <semi_write0>
	result = 0;	/* success */
	*dest = (char *)*dest + filesize;

out:
	if(fd != -1)
		semi_close(fd);
e00001d4:	e1a00005 	mov	r0, r5
e00001d8:	ebffffa7 	bl	e000007c <semi_close>

	if(result) {	/* print context for the error message */
		semi_write0(filename);
e00001dc:	e1a00008 	mov	r0, r8
e00001e0:	ebffffad 	bl	e000009c <semi_write0>
		semi_write0("\n");
e00001e4:	e59f0024 	ldr	r0, [pc, #36]	; e0000210 <semi_load_file+0xc0>
e00001e8:	ebffffab 	bl	e000009c <semi_write0>
e00001ec:	e3e00000 	mvn	r0, #0
e00001f0:	e8bd85f8 	pop	{r3, r4, r5, r6, r7, r8, sl, pc}
	int fd = -1;
	int filesize;

	fd = semi_open(filename, OPEN_RDONLY);
	if(fd == -1) {
		semi_write0("Cannot open file: ");
e00001f4:	e59f0018 	ldr	r0, [pc, #24]	; e0000214 <semi_load_file+0xc4>
e00001f8:	ebffffa7 	bl	e000009c <semi_write0>
		goto out;
e00001fc:	eafffff6 	b	e00001dc <semi_load_file+0x8c>
	}

	filesize = semi_flen(fd);
	if(filesize == -1) {
		semi_write0("Cannot get file size for: ");
e0000200:	e59f0010 	ldr	r0, [pc, #16]	; e0000218 <semi_load_file+0xc8>
e0000204:	ebffffa4 	bl	e000009c <semi_write0>
		goto out;
e0000208:	eafffff1 	b	e00001d4 <semi_load_file+0x84>
e000020c:	e00005a8 	.word	0xe00005a8
e0000210:	e00005bc 	.word	0xe00005bc
e0000214:	e0000578 	.word	0xe0000578
e0000218:	e000058c 	.word	0xe000058c

e000021c <memcpy>:

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	while (__n--)
e000021c:	e3520000 	cmp	r2, #0
e0000220:	012fff1e 	bxeq	lr
e0000224:	e3a03000 	mov	r3, #0
		*d++ = *s++;
e0000228:	e7d1c003 	ldrb	ip, [r1, r3]
e000022c:	e7c0c003 	strb	ip, [r0, r3]
e0000230:	e2833001 	add	r3, r3, #1

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	while (__n--)
e0000234:	e1520003 	cmp	r2, r3
e0000238:	1afffffa 	bne	e0000228 <memcpy+0xc>
}

void *(memcpy)(void *__dest, __const void *__src, size_t __n)
{
	return __memmove_down(__dest, __src, __n);
}
e000023c:	e12fff1e 	bx	lr

e0000240 <memmove>:

void *(memmove)(void *__dest, __const void *__src, size_t __n)
{
	if(__dest > __src)
e0000240:	e1500001 	cmp	r0, r1
e0000244:	8a000008 	bhi	e000026c <memmove+0x2c>

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	while (__n--)
e0000248:	e3520000 	cmp	r2, #0
e000024c:	012fff1e 	bxeq	lr
e0000250:	e3a03000 	mov	r3, #0
		*d++ = *s++;
e0000254:	e7d1c003 	ldrb	ip, [r1, r3]
e0000258:	e7c0c003 	strb	ip, [r0, r3]
e000025c:	e2833001 	add	r3, r3, #1

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	while (__n--)
e0000260:	e1520003 	cmp	r2, r3
e0000264:	1afffffa 	bne	e0000254 <memmove+0x14>
{
	if(__dest > __src)
		return __memmove_up(__dest, __src, __n);
	else
		return __memmove_down(__dest, __src, __n);
}
e0000268:	e12fff1e 	bx	lr

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;

	while (__n--)
e000026c:	e3520000 	cmp	r2, #0
e0000270:	012fff1e 	bxeq	lr
	return __dest;
}

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;
e0000274:	e2422001 	sub	r2, r2, #1
e0000278:	e0811002 	add	r1, r1, r2

	while (__n--)
		*d-- = *s--;
e000027c:	e451c001 	ldrb	ip, [r1], #-1
	return __dest;
}

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;
e0000280:	e0803002 	add	r3, r0, r2

	while (__n--)
e0000284:	e3520000 	cmp	r2, #0
		*d-- = *s--;
e0000288:	e443c001 	strb	ip, [r3], #-1

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;

	while (__n--)
e000028c:	012fff1e 	bxeq	lr
e0000290:	e2422001 	sub	r2, r2, #1
		*d-- = *s--;
e0000294:	e451c001 	ldrb	ip, [r1], #-1

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;

	while (__n--)
e0000298:	e3520000 	cmp	r2, #0
		*d-- = *s--;
e000029c:	e443c001 	strb	ip, [r3], #-1

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;

	while (__n--)
e00002a0:	1afffffa 	bne	e0000290 <memmove+0x50>
e00002a4:	e12fff1e 	bx	lr

e00002a8 <memchr>:

void *(memchr)(void const *s, int c, size_t n)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(n && *_s != c) {
e00002a8:	e3520000 	cmp	r2, #0
	}

	if(n)
		return (void *)_s;	/* the C library casts const away */
	else
		return (void *)0;
e00002ac:	01a00002 	moveq	r0, r2

void *(memchr)(void const *s, int c, size_t n)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(n && *_s != c) {
e00002b0:	012fff1e 	bxeq	lr
e00002b4:	e5d03000 	ldrb	r3, [r0]
e00002b8:	e1530001 	cmp	r3, r1
e00002bc:	012fff1e 	bxeq	lr
		return __memmove_up(__dest, __src, __n);
	else
		return __memmove_down(__dest, __src, __n);
}

void *(memchr)(void const *s, int c, size_t n)
e00002c0:	e2803001 	add	r3, r0, #1
e00002c4:	e0802002 	add	r2, r0, r2
e00002c8:	ea000003 	b	e00002dc <memchr+0x34>
{
	unsigned char const *_s = (unsigned char const *)s;

	while(n && *_s != c) {
e00002cc:	e4d3c001 	ldrb	ip, [r3], #1
		++_s;
e00002d0:	e2800001 	add	r0, r0, #1

void *(memchr)(void const *s, int c, size_t n)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(n && *_s != c) {
e00002d4:	e15c0001 	cmp	ip, r1
e00002d8:	0a000003 	beq	e00002ec <memchr+0x44>
e00002dc:	e1530002 	cmp	r3, r2
e00002e0:	1afffff9 	bne	e00002cc <memchr+0x24>
	}

	if(n)
		return (void *)_s;	/* the C library casts const away */
	else
		return (void *)0;
e00002e4:	e3a00000 	mov	r0, #0
}
e00002e8:	e12fff1e 	bx	lr
e00002ec:	e12fff1e 	bx	lr

e00002f0 <strlen>:

size_t (strlen)(const char *s)
{
	const char *sc = s;

	while (*sc != '\0')
e00002f0:	e5d03000 	ldrb	r3, [r0]
e00002f4:	e3530000 	cmp	r3, #0
e00002f8:	0a000007 	beq	e000031c <strlen+0x2c>
e00002fc:	e1a02000 	mov	r2, r0
		return (void *)_s;	/* the C library casts const away */
	else
		return (void *)0;
}

size_t (strlen)(const char *s)
e0000300:	e1a03000 	mov	r3, r0
{
	const char *sc = s;

	while (*sc != '\0')
e0000304:	e5f21001 	ldrb	r1, [r2, #1]!
		sc++;
e0000308:	e2833001 	add	r3, r3, #1

size_t (strlen)(const char *s)
{
	const char *sc = s;

	while (*sc != '\0')
e000030c:	e3510000 	cmp	r1, #0
e0000310:	1afffffb 	bne	e0000304 <strlen+0x14>
e0000314:	e0600003 	rsb	r0, r0, r3
e0000318:	e12fff1e 	bx	lr
e000031c:	e1a00003 	mov	r0, r3
		sc++;
	return sc - s;
}
e0000320:	e12fff1e 	bx	lr

e0000324 <memset>:

void *(memset)(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
e0000324:	e3520000 	cmp	r2, #0
e0000328:	012fff1e 	bxeq	lr
e000032c:	e6ef1071 	uxtb	r1, r1
	return sc - s;
}

void *(memset)(void *s, int c, size_t count)
{
	char *xs = s;
e0000330:	e1a03000 	mov	r3, r0
	while (count--)
e0000334:	e2522001 	subs	r2, r2, #1
		*xs++ = c;
e0000338:	e4c31001 	strb	r1, [r3], #1
}

void *(memset)(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
e000033c:	1afffffc 	bne	e0000334 <memset+0x10>
		*xs++ = c;
	return s;
}
e0000340:	e12fff1e 	bx	lr

e0000344 <memcmp>:
int (memcmp)(void const *p1, void const *p2, size_t n)
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
e0000344:	e3520000 	cmp	r2, #0
		*xs++ = c;
	return s;
}

int (memcmp)(void const *p1, void const *p2, size_t n)
{
e0000348:	e52d4004 	push	{r4}		; (str r4, [sp, #-4]!)

		++_p1;
		++_p2;
	}

	return 0;
e000034c:	01a00002 	moveq	r0, r2
int (memcmp)(void const *p1, void const *p2, size_t n)
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
e0000350:	0a000012 	beq	e00003a0 <memcmp+0x5c>
		if(*_p1 < *_p2)
e0000354:	e5d0c000 	ldrb	ip, [r0]
e0000358:	e5d13000 	ldrb	r3, [r1]
e000035c:	e15c0003 	cmp	ip, r3
e0000360:	3a000010 	bcc	e00003a8 <memcmp+0x64>
int (memcmp)(void const *p1, void const *p2, size_t n)
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
e0000364:	92422001 	subls	r2, r2, #1
e0000368:	93a03000 	movls	r3, #0
		if(*_p1 < *_p2)
			return -1;
		else if(*_p1 > *_p2)
e000036c:	9a000006 	bls	e000038c <memcmp+0x48>
e0000370:	ea00000e 	b	e00003b0 <memcmp+0x6c>
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
		if(*_p1 < *_p2)
e0000374:	e5d44001 	ldrb	r4, [r4, #1]
e0000378:	e5dcc001 	ldrb	ip, [ip, #1]
e000037c:	e2833001 	add	r3, r3, #1
e0000380:	e154000c 	cmp	r4, ip
e0000384:	3a000007 	bcc	e00003a8 <memcmp+0x64>
			return -1;
		else if(*_p1 > *_p2)
e0000388:	8a000008 	bhi	e00003b0 <memcmp+0x6c>
int (memcmp)(void const *p1, void const *p2, size_t n)
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
e000038c:	e1530002 	cmp	r3, r2
	while (count--)
		*xs++ = c;
	return s;
}

int (memcmp)(void const *p1, void const *p2, size_t n)
e0000390:	e0804003 	add	r4, r0, r3
e0000394:	e081c003 	add	ip, r1, r3
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
e0000398:	1afffff5 	bne	e0000374 <memcmp+0x30>

		++_p1;
		++_p2;
	}

	return 0;
e000039c:	e3a00000 	mov	r0, #0
}
e00003a0:	e8bd0010 	pop	{r4}
e00003a4:	e12fff1e 	bx	lr
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
		if(*_p1 < *_p2)
			return -1;
e00003a8:	e3e00000 	mvn	r0, #0
e00003ac:	eafffffb 	b	e00003a0 <memcmp+0x5c>
		else if(*_p1 > *_p2)
			return 1;
e00003b0:	e3a00001 	mov	r0, #1
e00003b4:	eafffff9 	b	e00003a0 <memcmp+0x5c>

e00003b8 <strcmp>:
	return 0;
}

int (strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
e00003b8:	e5d03000 	ldrb	r3, [r0]
e00003bc:	e3530000 	cmp	r3, #0
e00003c0:	0a000011 	beq	e000040c <strcmp+0x54>
e00003c4:	e5d12000 	ldrb	r2, [r1]
e00003c8:	e3520000 	cmp	r2, #0
e00003cc:	0a000012 	beq	e000041c <strcmp+0x64>
		if(*s1 < *s2)
e00003d0:	e1530002 	cmp	r3, r2
e00003d4:	3a000012 	bcc	e0000424 <strcmp+0x6c>
			return -1;
		else if(*s1 > *s2)
e00003d8:	91a0c001 	movls	ip, r1
e00003dc:	9a000006 	bls	e00003fc <strcmp+0x44>
e00003e0:	ea00000d 	b	e000041c <strcmp+0x64>
	return 0;
}

int (strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
e00003e4:	e5fc2001 	ldrb	r2, [ip, #1]!
e00003e8:	e3520000 	cmp	r2, #0
e00003ec:	0a00000a 	beq	e000041c <strcmp+0x64>
		if(*s1 < *s2)
e00003f0:	e1530002 	cmp	r3, r2
e00003f4:	3a00000a 	bcc	e0000424 <strcmp+0x6c>
			return -1;
		else if(*s1 > *s2)
e00003f8:	8a00000b 	bhi	e000042c <strcmp+0x74>
	return 0;
}

int (strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
e00003fc:	e5f03001 	ldrb	r3, [r0, #1]!
			return -1;
		else if(*s1 > *s2)
			return 1;

		++s1;
		++s2;
e0000400:	e2811001 	add	r1, r1, #1
	return 0;
}

int (strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
e0000404:	e3530000 	cmp	r3, #0
e0000408:	1afffff5 	bne	e00003e4 <strcmp+0x2c>

		++s1;
		++s2;
	}

	if(!*s1 && !*s2)
e000040c:	e5d10000 	ldrb	r0, [r1]
		return 0;
e0000410:	e3500000 	cmp	r0, #0
e0000414:	13e00000 	mvnne	r0, #0
e0000418:	e12fff1e 	bx	lr
{
	while(*s1 && *s2) {
		if(*s1 < *s2)
			return -1;
		else if(*s1 > *s2)
			return 1;
e000041c:	e3a00001 	mov	r0, #1
e0000420:	e12fff1e 	bx	lr

int (strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
		if(*s1 < *s2)
			return -1;
e0000424:	e3e00000 	mvn	r0, #0
e0000428:	e12fff1e 	bx	lr
		else if(*s1 > *s2)
			return 1;
e000042c:	e3a00001 	mov	r0, #1
		return 0;
	else if(!*s1)
		return -1;
	else
		return 1;
}
e0000430:	e12fff1e 	bx	lr

e0000434 <strncmp>:

int (strncmp)(char const *s1, char const *s2, size_t n)
{
e0000434:	e92d0070 	push	{r4, r5, r6}
	while(*s1 && *s2 && n--) {
e0000438:	e5d03000 	ldrb	r3, [r0]
e000043c:	e3530000 	cmp	r3, #0
e0000440:	0a00002b 	beq	e00004f4 <strncmp+0xc0>
e0000444:	e5d1c000 	ldrb	ip, [r1]
e0000448:	e35c0000 	cmp	ip, #0
e000044c:	0a000028 	beq	e00004f4 <strncmp+0xc0>
e0000450:	e3520000 	cmp	r2, #0
e0000454:	e2426001 	sub	r6, r2, #1
e0000458:	0a000027 	beq	e00004fc <strncmp+0xc8>
		if(*s1 < *s2)
e000045c:	e153000c 	cmp	r3, ip
e0000460:	3a000021 	bcc	e00004ec <strncmp+0xb8>
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
e0000464:	91a02006 	movls	r2, r6
		if(*s1 < *s2)
			return -1;
		else if(*s1 > *s2)
e0000468:	91a05001 	movls	r5, r1
e000046c:	93a0c000 	movls	ip, #0
e0000470:	9a00000a 	bls	e00004a0 <strncmp+0x6c>
e0000474:	ea00001a 	b	e00004e4 <strncmp+0xb0>
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
e0000478:	e5d44001 	ldrb	r4, [r4, #1]
e000047c:	e3540000 	cmp	r4, #0
e0000480:	0a00000c 	beq	e00004b8 <strncmp+0x84>
e0000484:	e156000c 	cmp	r6, ip
e0000488:	e2422001 	sub	r2, r2, #1
e000048c:	0a000009 	beq	e00004b8 <strncmp+0x84>
		if(*s1 < *s2)
e0000490:	e1530004 	cmp	r3, r4
e0000494:	3a000014 	bcc	e00004ec <strncmp+0xb8>
e0000498:	e28cc001 	add	ip, ip, #1
			return -1;
		else if(*s1 > *s2)
e000049c:	8a000010 	bhi	e00004e4 <strncmp+0xb0>
		return -1;
	else
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
e00004a0:	e080300c 	add	r3, r0, ip
{
	while(*s1 && *s2 && n--) {
e00004a4:	e5d33001 	ldrb	r3, [r3, #1]
		return -1;
	else
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
e00004a8:	e081400c 	add	r4, r1, ip
{
	while(*s1 && *s2 && n--) {
e00004ac:	e3530000 	cmp	r3, #0
			return -1;
		else if(*s1 > *s2)
			return 1;

		++s1;
		++s2;
e00004b0:	e2855001 	add	r5, r5, #1
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
e00004b4:	1affffef 	bne	e0000478 <strncmp+0x44>

		++s1;
		++s2;
	}

	if(n == 0 || (!*s1 && !*s2))
e00004b8:	e3520000 	cmp	r2, #0
		return 0;
e00004bc:	01a00002 	moveq	r0, r2

		++s1;
		++s2;
	}

	if(n == 0 || (!*s1 && !*s2))
e00004c0:	0a000005 	beq	e00004dc <strncmp+0xa8>
e00004c4:	e3530000 	cmp	r3, #0
e00004c8:	1a000005 	bne	e00004e4 <strncmp+0xb0>
e00004cc:	e5d53000 	ldrb	r3, [r5]
		return 0;
e00004d0:	e3530000 	cmp	r3, #0
e00004d4:	13e00000 	mvnne	r0, #0
e00004d8:	03a00000 	moveq	r0, #0
	else if(!*s1)
		return -1;
	else
		return 1;
}
e00004dc:	e8bd0070 	pop	{r4, r5, r6}
e00004e0:	e12fff1e 	bx	lr
	if(n == 0 || (!*s1 && !*s2))
		return 0;
	else if(!*s1)
		return -1;
	else
		return 1;
e00004e4:	e3a00001 	mov	r0, #1
e00004e8:	eafffffb 	b	e00004dc <strncmp+0xa8>

int (strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
		if(*s1 < *s2)
			return -1;
e00004ec:	e3e00000 	mvn	r0, #0
e00004f0:	eafffff9 	b	e00004dc <strncmp+0xa8>
		return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
e00004f4:	e1a05001 	mov	r5, r1
e00004f8:	eaffffee 	b	e00004b8 <strncmp+0x84>
e00004fc:	e1a05001 	mov	r5, r1
e0000500:	e1a02006 	mov	r2, r6
e0000504:	eaffffeb 	b	e00004b8 <strncmp+0x84>

e0000508 <strchr>:

char *(strchr)(char const *s, int c)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(*_s && *_s != c)
e0000508:	e5d03000 	ldrb	r3, [r0]
e000050c:	e3530000 	cmp	r3, #0
		++_s;

	if(*_s)
		return (char *)_s;	/* the C library casts const away */
	else
		return (char *)0;
e0000510:	01a00003 	moveq	r0, r3

char *(strchr)(char const *s, int c)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(*_s && *_s != c)
e0000514:	012fff1e 	bxeq	lr
e0000518:	e1530001 	cmp	r3, r1
e000051c:	1a000002 	bne	e000052c <strchr+0x24>
e0000520:	e12fff1e 	bx	lr
e0000524:	e1530001 	cmp	r3, r1
e0000528:	0a000005 	beq	e0000544 <strchr+0x3c>
e000052c:	e5d03001 	ldrb	r3, [r0, #1]
		++_s;
e0000530:	e2800001 	add	r0, r0, #1

char *(strchr)(char const *s, int c)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(*_s && *_s != c)
e0000534:	e3530000 	cmp	r3, #0
e0000538:	1afffff9 	bne	e0000524 <strchr+0x1c>
		++_s;

	if(*_s)
		return (char *)_s;	/* the C library casts const away */
	else
		return (char *)0;
e000053c:	e1a00003 	mov	r0, r3
}
e0000540:	e12fff1e 	bx	lr
e0000544:	e12fff1e 	bx	lr
