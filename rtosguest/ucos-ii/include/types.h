
#ifndef _TYPES_H_4782374832742374327423
#define _TYPES_H_4782374832742374327423

typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef unsigned char	uchar;
typedef unsigned int	uint;

#ifndef __cplusplus
typedef int				bool;
#define	true			1
#define false			0
#endif

typedef enum {
	VAR_LONG=32,
	VAR_SHORT=16,
	VAR_CHAR=8
} VAR_TYPE;

#ifndef NULL
#define NULL (void *)0
#endif

#endif

