#ifndef CSIM_UTIL_H
#define CSIM_UTIL_H

#include <stdlib.h>
#include <stdio.h>

// this is in posix standard but not in STD-C99
// I orginally wrote this for my scripting language: github.com/dvtate/old-yoda
int getline(char** lineptr, size_t* n, FILE* stream) {

	char *bufptr, *p;

	ssize_t size;
	int c;

	// all parameters are required
	if (!lineptr || !stream || !n)
		return -1;

	bufptr	= *lineptr;
	size 	= *n;
	c 		= fgetc(stream);

	if (c == EOF)
		return -1;

	if (bufptr == NULL) {
		bufptr = (char*) malloc(128);
		if (bufptr == NULL)
			return -2;

		size = 128;
	}

	p = bufptr;

	while (c != EOF) {

		if ((p - bufptr) > (size - 2)) {
			size = size + 128;
			bufptr = (char*) realloc(bufptr, size);
			if (bufptr == NULL)
				return -1;
		}

		*p++ = c;
		if (c == '\n' || c == '\0')
			break;

		c = fgetc(stream);

	}

	*p++	 = '\0';
	*lineptr = bufptr;
	*n		 = size;

	return p - bufptr - 1;

}


#endif
