#include <Rtl.Base.h>

void StrDeleteArray (char** &Strings, int Count)
{
	if (Strings && Count)
	{
		for (int i = 0; i < Count; i++)
		{
			free (Strings[i]);
		}

		free (Strings);
		Strings = NULL;
	}
}

char *StrDuplicate (const char *String, int Length)
{
	if (String && Length)
	{
		if (Length == -1)
			Length = StrLength(String)+1;

		char *result = StrCreate (Length);
		strcpy (result, String);

		return result;
	}

	return NULL;
}

char** StrDuplicateArray (char** &Strings, int Count)
{
	if (Count && Strings)
	{
		char **result = StrCreateArray (Count);

		for (int i = 0; i < Count; i++)
			result[i] = StrDuplicate (Strings[i]);

		return result;
	}

	return NULL; 
}

char *StrCreate (int nLength)
{
	char *lpResult = (char*)malloc (nLength+1);
	memset (lpResult, 0, nLength+1);
	return lpResult;
}

void StrDelete (char* &String)
{
	free (String);
	String = NULL;
}

char*  StrReplace (char* String1, const char *String2)
{
	StrFree (String1);
	return StrDuplicate (String2);
}

char** StrCreateArray (int nCount)
{
	char **pResult = NULL;

	if ( nCount )
	{
		pResult = (char**)malloc (nCount*sizeof(char*));
		memset (pResult, 0, nCount*sizeof(char*));
	}

	return pResult;
}
