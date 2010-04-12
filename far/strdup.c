#include "headers.hpp"
#pragma hdrstop

#ifdef __cplusplus
extern "C"
{
#endif

	void *__cdecl xf_malloc(size_t __size);

#ifdef __cplusplus
}
#endif

/***
*char *_strdup(string) - duplicate string into malloc'd memory
*
*Purpose:
*       Allocates enough storage via malloc() for a copy of the
*       string, copies the string into the new memory, and returns
*       a pointer to it.
*
*Entry:
*       char *string - string to copy into new memory
*
*Exit:
*       returns a pointer to the newly allocated storage with the
*       string in it.
*
*       returns NULL if enough memory could not be allocated, or
*       string was NULL.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl xf_strdup(const char * string)
{
	if (string)
	{
		char *memory;

		if ((memory = xf_malloc(strlen(string) + 1)) != NULL)
			return strcpy(memory,string);
	}

	return(NULL);
}
