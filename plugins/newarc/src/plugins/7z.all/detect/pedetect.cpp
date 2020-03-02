#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const size_t MIN_HEADER_LEN = sizeof (IMAGE_DOS_HEADER);

int IsPEHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)Data;

	if (    pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
	     || (size_t)DataSize < pDOSHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS))
		return -1;

	PIMAGE_NT_HEADERS pPEHeader = (PIMAGE_NT_HEADERS)&Data[pDOSHeader->e_lfanew];

	if ( pPEHeader->Signature != IMAGE_NT_SIGNATURE )
		return -1;

	return 0;
}
