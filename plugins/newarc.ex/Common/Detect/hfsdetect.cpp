#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned int MIN_HEADER_LEN = 1024+512;

int IsHfsHeader(const unsigned char* pData, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	for (unsigned int i = 0; i < 1024; i++)
		if ( pData[i] != 0)
			return -1;

	pData += 1024;

	if ( pData[0] != 'H' || (pData[1] != '+' && pData[1] != 'X'))
		return -1;

	return 0;
}
