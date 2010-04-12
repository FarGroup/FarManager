#include "headers.hpp"
#pragma hdrstop

#ifdef _MSC_VER
#pragma intrinsic (strcmp)       /* don't use the unsafe version */
#else
#pragma intrinsic -strcmp       /* don't use the unsafe version */
#endif
/*---------------------------------------------------------------------*

Name            _tmktemp used as _mktemp and _wmktemp
                _mktemp   - makes a unique file name
                _wmktemp - makes a unique file name

Usage           char *_mktemp(char *template);
                wchar_t *_wmktemp(wchar_t *template);

Prototype in    dir.h

Description     _tmktemp replaces template by a unique file name
                and returns the address of template.

                The template should be a null-terminated string
                with six trailing X's. These X's are replaced with a unique
                collection of letters plus a dot, so that there are two letters,
                a dot, and three suffix letters in the new file name.

                The first letter is variable: it can be '0' or 'a' to 'z'.
                The remaining four letters are replaced with an ASCII
                base-32 represention of the process ID.  The function
                runs through all possible values for the first letter until
                it finds a name that doesn't correspond to an existing file.

Return value    If template is well-formed, _tmktemp returns the
                address of the template string. Otherwise, it does not create
                or open the file.

*---------------------------------------------------------------------*/

char* farmktemp(char *temp)
{
	register char *cp;
	size_t  len;
	int     i, c, j, k;
	int     pid;
	/* Verify that the template is of the proper form.
	 * Point cp at the start of the XXXXXXXX.
	 */
	len = strlen(temp);

	if (len < 8)
		return(0);

	cp = temp + len - 8;

	if (strcmp(cp, "XXXXXXXX") != 0)
		return(0);

	/* The XXXXXXXX is converted to the following format:
	 *      dpppp.pp
	 * Where pppppp is the low 20 bits of the process ID in mangled form
	 * (ASCII base 32, backwards), and d is '0' or a lower-case letter.
	 */
	pid = MAKEWORD(GetCurrentProcessId(),GetCurrentThreadId());
	cp[4] = '.';

	for (i = 3; i < 8; i++)
	{
		if (i == 4)         /* skip the '.' */
			i = 5;

		c = pid & 0x1f;     /* convert low 5 bits to ASCII */
		cp[i] = c + (c < 10 ? '0' : 'A' - 10);
		pid >>= 5;          /* shift to get next 5 bits */
	}

	/* Try varying the first "letter", starting with '0', then
	 * using the lowercase letters, until we find a file that doesn't
	 * exist, or exhaust all the letters.
	 */
	for (i = 'A'-1; i <= 'Z'; i++)
	{
		cp[0] = i == 'A'-1 ? '0' : i;

		for (j = 'A'-1; j <= 'Z'; j++)
		{
			cp[1] = j == 'A'-1 ? '0' : j;

			for (k = 'A'-1; k <= 'Z'; k++)
			{
				cp[2] = k == 'A'-1 ? '0' : k;

				if (access(temp, 0) == -1)
					return(temp);
			}
		}
	}

	return (NULL);
}
