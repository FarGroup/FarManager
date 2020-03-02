#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned char iso_signature[] = {'C', 'D', '0', '0', '1', 0x1};

const size_t MIN_HEADER_LEN = 0x8000+sizeof (iso_signature)+1;

int IsIsoHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	if (!memcmp (Data+0x8000+1, &iso_signature, sizeof (iso_signature)))
			return 0;

	return -1;
}
