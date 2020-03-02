#include <Rtl.Base.h>

void StrDeleteArray (TCHAR** &Strings, int Count)
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

TCHAR *StrDuplicate (const TCHAR *String, int Length)
{
	if (String && Length)
	{
		if (Length == -1)
			Length = StrLength(String)+1;

		TCHAR *result = StrCreate (Length);
		_tcscpy (result, String);

		return result;
	}

	return NULL;
}

TCHAR** StrDuplicateArray (TCHAR** &Strings, int Count)
{
	if (Count && Strings)
	{
		TCHAR **result = StrCreateArray (Count);

		for (int i = 0; i < Count; i++)
			result[i] = StrDuplicate (Strings[i]);

		return result;
	}

	return NULL; 
}

TCHAR *StrCreate (int nLength)
{
	TCHAR *lpResult = (TCHAR*)malloc ((nLength+1)*sizeof(TCHAR));
	memset (lpResult, 0, (nLength+1)*sizeof(TCHAR));
	return lpResult;
}

void StrDelete (TCHAR* &String)
{
	free (String);
	String = NULL;
}

TCHAR*  StrReplace (TCHAR* String1, const TCHAR *String2)
{
	StrFree (String1);
	return StrDuplicate (String2);
}

TCHAR** StrCreateArray (int nCount)
{
	TCHAR **pResult = NULL;

	if ( nCount )
	{
		pResult = (TCHAR**)malloc (nCount*sizeof(TCHAR*));
		memset (pResult, 0, nCount*sizeof(TCHAR*));
	}

	return pResult;
}

unsigned int StrLength(const TCHAR* s) 
{
	return (unsigned int)_tcslen(s);
}
