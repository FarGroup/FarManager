#pragma once
#include "Rtl.Base.h"

inline bool IsEOL(TCHAR c)
{
	return c == _T('\n') || c == _T('\r');
}

inline bool IsEmpty (TCHAR c)
{
	return c == _T(' ') || c == _T('\t');
}

//#define StrCreateArray(i) (i?((char**)malloc (i << 2)):NULL)
//#define StrCreate(i) (char*)malloc(i)
#define StrFree(s) free(s)
extern unsigned int StrLength(const TCHAR* s); 
extern TCHAR** StrCreateArray (int nCount);
extern TCHAR*  StrCreate (int nLength);
extern void   StrDeleteArray    (TCHAR** &Strings, int Count);
extern TCHAR** StrDuplicateArray (TCHAR** &Strings, int Count); 

extern TCHAR*  StrDuplicate      (const TCHAR* String, int Length = -1);
extern void   StrDelete         (TCHAR* &String);
extern TCHAR*  StrReplace        (TCHAR* String1, const TCHAR* String2);

//extern char *strdup (const char *lpSrc);
//extern char *strncpy (char * dest,const char *src, int maxlen);
