#include "UnicodeAnsi.hpp"

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
