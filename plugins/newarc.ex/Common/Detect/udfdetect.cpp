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

const unsigned int MIN_HEADER_LEN = 0x8000+sizeof(iso_signature)+1;

int IsUdfHeader(const unsigned char* pData, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	if ( memcmp(pData+0x8000+1, &iso_signature, sizeof(iso_signature)) )
		return -1;

	const unsigned char* pMaxData = pData+uDataSize-sizeof(udf_signature);

	for (const unsigned char* pCurData = pData+MIN_HEADER_LEN; pCurData < pMaxData; pCurData++)
	{
		if ( !memcmp (pCurData, &udf_signature, sizeof(udf_signature)) )
			return 0;
	}

	return -1;
}
