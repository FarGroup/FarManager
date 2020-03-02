#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const size_t MIN_HEADER_LEN = 1024 + 512;

int IsHfsHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

  for (int i = 0; i < 1024; i++)
    if (Data[i] != 0)
      return -1;

  Data += 1024;

  if (Data[0] != 'H' || (Data[1] != '+' && Data[1] != 'X'))
    return -1;

	return 0;
}
