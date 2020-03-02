#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned char iso_signature[] = {'C', 'D', '0', '0', '1', 0x1};
const unsigned char udf_signature[] = {'N', 'S', 'R', '0'};

const size_t MIN_HEADER_LEN = 0x8000+sizeof (iso_signature)+1;

int IsUdfHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	if (memcmp (Data+0x8000+1, &iso_signature, sizeof (iso_signature)))
		return -1;

	const unsigned char *MaxData=Data+DataSize-sizeof(udf_signature);

	for (const unsigned char *CurData=Data+MIN_HEADER_LEN; CurData<MaxData; CurData++)
	{
		if ( !memcmp (CurData, &udf_signature, sizeof (udf_signature)) )
			return 0;
	}

	return -1;
}
