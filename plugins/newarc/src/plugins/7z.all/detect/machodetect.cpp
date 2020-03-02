#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const size_t MIN_HEADER_LEN = 4;

int IsMachoHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	DWORD sig = *((DWORD *)Data);

	if (sig == 0xCEFAEDFE || sig == 0xCFFAEDFE || sig == 0xFEEDFACE || sig == 0xFEEDFACF)
		return 0;

	return -1;
}
