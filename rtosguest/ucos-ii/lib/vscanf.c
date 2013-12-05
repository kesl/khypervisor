#include	"includes.h"

typedef int boolean_t;

int
_doscan(const unsigned char *fmt, va_list vp,
	int (*getc)(void *getc_arg),
	void (*ungetc)(int c, void *getc_arg),
	void *getc_arg)
{
	register	int c;
	boolean_t	neg;
	boolean_t	discard;
	int		vals = 0;

	while ((c = *fmt++) != 0) {

	    if (c != '%') {
	        if (isspace(c))
		{
		/*	while (isspace(c = getc(getc_arg)));
			ungetc(c, getc_arg);*/
			continue;
		}
	    	else if (c == getc(getc_arg))
			continue;
		else
			break;	/* mismatch */
	    }
	    while (isspace(c = getc(getc_arg)));
		ungetc(c, getc_arg);

	    discard = 0;

	more_fmt:

	    c = *fmt++;

	    switch (c) {

	    case 'd':
	    {
		long n = 0;

		c = getc(getc_arg);

		neg =  c == '-';
		if (neg) c = getc(getc_arg);

		while (c >= '0' && c <= '9') {
		    n = n * 10 + (c - '0');
		    c = getc(getc_arg);
		}
		ungetc(c, getc_arg);

		if (neg) n = -n;

		/* done, store it away */
		if (!discard)
		{
			int *p = va_arg(vp, int *);
			*p = n;
		}

	        break;
	    }

	    case 'o':
	    {
		long n = 0;

		c = getc(getc_arg);

		neg =  c == '-';
		if (neg) c = getc(getc_arg);

		while (c >= '0' && c <= '7') {
		    n = n * 8 + (c - '0');
		    c = getc(getc_arg);
		}
		ungetc(c, getc_arg);

		if (neg) n = -n;

		/* done, store it away */
		if (!discard)
		{
			int *p = va_arg(vp, int *);
			*p = n;
		}

		break;
	    }

	    case 'x':
	    {
		long n = 0;

		c = getc(getc_arg);

		neg =  c == '-';
		if (neg) c = getc(getc_arg);

		while (1)
		{
		    if ((c >= '0') && (c <= '9'))
			n = n * 16 + (c - '0');
		    else if ((c >= 'a') && (c <= 'f'))
			n = n * 16 + (c - 'a' + 10);
		    else if ((c >= 'A') && (c <= 'F'))
			n = n * 16 + (c - 'A' + 10);
		    else
			break;
		    c = getc(getc_arg);
		}
		ungetc(c, getc_arg);	/* retract lookahead */

		if (neg) n = -n;

		/* done, store it away */
		if (!discard)
		{
			int *p = va_arg(vp, int *);
			*p = n;
		}

	        break;
	    }

	    case 's':
	    {
		char *buf = NULL;

		if (!discard)
			buf = va_arg(vp, char *);

		c = getc(getc_arg);
		while (!isspace(c))
		{
		    if (!discard)
		    	*buf++ = c;
		    c = getc(getc_arg);
		}
		ungetc(c, getc_arg);	/* retract lookahead */

		if (!discard)
		    *buf = 0;

		break;
	    }

	    case '*':
	    	discard = 1;
	    case 'l':
		goto more_fmt;

	    default:
	        break;
	    }

	    if (!discard)
		vals++;
	}

	return vals;
}

static int readchar( void *arg)
{
	return *(*(unsigned char**)arg)++;
}

static void unchar( int c, void * arg)
{
	(*(unsigned char**)arg)--;
}

int vsscanf(s, fmt, args)
	char *s;
	const char *fmt;
	va_list args;
{
	return _doscan(fmt, args, readchar, unchar, &s);
}

int sscanf(char *s, const char *fmt, ...)
{
	int nmatch = 0;
        va_list	args;

	va_start(args, fmt);
	nmatch = vsscanf(s, fmt, args);
	va_end(args);

	return nmatch;
}
