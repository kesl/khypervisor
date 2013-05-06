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
#include "semihosting.h"

int semi_open(char const *filename, int mode)
{
	struct {
		char const *filename;
		int mode;
		int filename_length;
	} args;

	args.filename = filename;
	args.mode = mode;
	args.filename_length = strlen(filename);

	return __semi_call(SYS_OPEN, &args);
}

int semi_close(int fd)
{
	return __semi_call(SYS_CLOSE, &fd);
}

int semi_write0(char const *string)
{
	return __semi_call(SYS_WRITE0, string);
}

int semi_read(int fd, char *buffer, int length)
{
	struct {
		int fd;
		char *buffer;
		int length;
	} args;

	args.fd = fd;
	args.buffer = buffer;
	args.length = length;

	return __semi_call(SYS_READ, &args);
}

int semi_flen(int fd)
{
	return __semi_call(SYS_FLEN, &fd);
}

int semi_get_cmdline(char *buffer, int size, int *length)
{
	int result;
	struct {
		char *buffer;
		int size;
	} args;

	args.buffer = buffer;
	args.size = size;

	result = __semi_call(SYS_GET_CMDLINE, &args);
	if(result)
		return result;

	if(length)
		*length = args.size;

	return 0;
}

int semi_reportexc(int reason)
{
	return __semi_call(SYS_REPORTEXC, (void *)reason);
}

void semi_exit(void)
{
	semi_reportexc(REPORTEXC_REASON_APP_EXIT);
	while(1); /* should not be reached */
}

void semi_fatal(char const *message)
{
	semi_write0(message);
	semi_exit();
}

int semi_load_file(void **dest, unsigned *size, char const *filename)
{
	int result = -1;	/* fail by default */
	int fd = -1;
	int filesize;

	fd = semi_open(filename, OPEN_RDONLY);
	if(fd == -1) {
		semi_write0("Cannot open file: ");
		goto out;
	}

	filesize = semi_flen(fd);
	if(filesize == -1) {
		semi_write0("Cannot get file size for: ");
		goto out;
	}

	if(semi_read(fd, *dest, filesize)) {
		semi_write0("Could not read: ");
		goto out;
	}

	result = 0;	/* success */
	*dest = (char *)*dest + filesize;

out:
	if(fd != -1)
		semi_close(fd);

	if(result) {	/* print context for the error message */
		semi_write0(filename);
		semi_write0("\n");
	} else
		if(size)
			*size = filesize;

	return result;
}
