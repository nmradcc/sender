/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strsep.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
//#include <sys/cdefs.h>
/* __FBSDID("$FreeBSD: src/sys/libkern/strsep.c,v 1.7 2004/04/07 20:46:10 imp Exp $"); */

//#include <sys/param.h>
//#include <libkern/libkern.h>
#include <string.h>

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char * strsepold(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


char * strsep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;
	int inQuote = 0;
	char quote[] = "\r\n\"";

	if ((s = *stringp) == NULL)
	{
		return (NULL);
	}

	if(*s == '"')
	{
		inQuote = 1;
		s++;
	}

	for (tok = s;;)
	{
		c = *s++;
		if(inQuote)
		{
			spanp = quote;	// only look to eol and the end quote
		}
		else
		{
			spanp = delim;
		}
		do
		{
			if ((sc = *spanp++) == c)
			{
				if (c == 0)
				{
					s = NULL;
				}
				else
				{
					s[-1] = 0;
				}
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


char* strsepAtt(char** stringp, const char* delim)
{
	char* s;
	const char* spanp;
	char c, sc;
	char* tok;
	int inQuote = 0;

	if ((s = *stringp) == NULL)
	{
		return (NULL);
	}

	for (tok = s;;)
	{
		c = *s++;
		if(c == '"')
		{
			if(inQuote)
			{
				inQuote = 0;
			}
			else
			{
				inQuote = 1;
			}
		}
		else
		{
			spanp = delim;
			do
			{
				sc = *spanp++;
				if(inQuote)
				{
					if(sc == '\r' || sc == '\n')	// ignore all of the other deliminators
					{
						if (sc == c)
						{
							if (c == 0)
							{
								s = NULL;
							}
							else
							{
								s[-1] = 0;
							}
							*stringp = s;
							return (tok);
						}
					}
				}
				else
				{
					if (sc == c)
					{
						if (c == 0)
						{
							s = NULL;
						}
						else
						{
							s[-1] = 0;
						}
						*stringp = s;
						return (tok);
					}
				}
			} while (sc != 0);
		}
	}
	/* NOTREACHED */
}


#ifdef NOT_USED
char* strsep2(char **stringp, const char delim, const char *eol)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
	{
		return (NULL);
	}
	for (tok = s;;)
	{
		c = *s++;
		spanp = eol;
		do
		{
			if(c == delim)
			{

			}
			if ((sc = *spanp++) == c)
			{
				if (c == 0)
				{
					s = NULL;
				}
				else
				{
					s[-1] = 0;
				}
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif

