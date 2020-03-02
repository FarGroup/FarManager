#include "UnicodeAnsi.hpp"

//как-то ANSI тут неумно звучит

wchar_t* AnsiToUnicode(const char* lpSrc, int CodePage)
{
	if ( !lpSrc )
		return NULL;

	int nLength = MultiByteToWideChar(CodePage, 0, lpSrc, -1, NULL, 0);

	wchar_t* lpResult = (wchar_t*)malloc((nLength+1)*sizeof(wchar_t));

	MultiByteToWideChar(CodePage, 0, lpSrc, -1, lpResult, nLength);

	return lpResult;
}

char* UnicodeToAnsi(const wchar_t* lpSrc, int CodePage)
{
	if ( !lpSrc )
		return NULL;

	int nLength = WideCharToMultiByte(CodePage, 0, lpSrc, -1, NULL, 0, NULL, NULL);

	char* lpResult = (char*)malloc(nLength+1);

	WideCharToMultiByte(CodePage, 0, lpSrc, -1, lpResult, nLength, NULL, NULL);

	return lpResult;
}

char* UnicodeToUTF8(const wchar_t* lpSrc)
{
	return UnicodeToAnsi(lpSrc, CP_UTF8); //ааа!!!! ToAnsi!!!
}

char* AnsiToUTF8(const char* lpSrc, int CodePage)
{
	if ( !lpSrc )
		return nullptr;

	wchar_t* lpUnicode = AnsiToUnicode(lpSrc, CodePage);

	char* lpResult = UnicodeToUTF8(lpUnicode);

	free(lpUnicode);

	return lpResult;
}

wchar_t* UTF8ToUnicode(const char* lpSrc)
{
	return AnsiToUnicode(lpSrc, CP_UTF8); //aaa!!!
}

char* UTF8ToAnsi(const char* lpSrc, int CodePage)
{
	if ( !lpSrc  )
		return nullptr;

	wchar_t* lpUnicode = AnsiToUnicode(lpSrc, CP_UTF8);

	char* lpResult = UnicodeToAnsi(lpUnicode, CP_OEMCP);

	free(lpUnicode);

	return lpResult;
}