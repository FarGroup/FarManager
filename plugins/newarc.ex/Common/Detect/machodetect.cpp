#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned int MIN_HEADER_LEN = 4;

int IsMachoHeader(const unsigned char* pData, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	DWORD dwSignature = *((DWORD*)pData);

	if ( (dwSignature == 0xCEFAEDFE) || (dwSignature == 0xCFFAEDFE) || (dwSignature == 0xFEEDFACE) || (dwSignature == 0xFEEDFACF) )
		return 0;

	return -1;
}
