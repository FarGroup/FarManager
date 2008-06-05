#pragma once
#include "Rtl.Base.h"

inline bool IsEOL(char c)
{
	return c == '\n' || c == '\r';
}

inline bool IsEmpty (char c)
{
	return c == ' ' || c == '\t';
}

//#define StrCreateArray(i) (i?((char**)malloc (i << 2)):NULL)
//#define StrCreate(i) (char*)malloc(i)
#define StrFree(s) free(s)
#define StrLength(s) (unsigned int)strlen(s)

extern char** StrCreateArray (int nCount);
extern char*  StrCreate (int nLength);
extern void   StrDeleteArray    (char** &Strings, int Count);
extern char** StrDuplicateArray (char** &Strings, int Count); 

extern char*  StrDuplicate      (const char* String, int Length = -1);
extern void   StrDelete         (char* &String);
extern char*  StrReplace        (char* String1, const char *String2);

