#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned char nsis_signature[] = {0xEF, 0xBE, 0xAD, 0xDE, 0x4E, 0x75, 0x6C, 0x6C, 0x73, 0x6F, 0x66, 0x74, 0x49, 0x6E, 0x73, 0x74};

struct NSISHeader //обязан быть на позиции в файле, кратной 512 байт (перед ним идет exe stub)
{
	DWORD dwFlags;
	unsigned char Signature[16];
	// points to the header+sections+entries+stringtable in the datablock
	DWORD dwHeaderLength;
	DWORD dwArchiveSize;
};

const size_t MIN_HEADER_LEN = sizeof (NSISHeader);


static inline BOOL IsValidHeader (const unsigned char *Data)
{
	NSISHeader *header = (NSISHeader*)Data;
	return !memcmp (&header->Signature, &nsis_signature, sizeof (nsis_signature));
}

int IsNSISHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	const unsigned char *MaxData=Data+DataSize-MIN_HEADER_LEN;

	for (const unsigned char *CurData=Data; CurData<MaxData; CurData++)
	{
		int position = (int)(CurData-Data);

		if ( !(position%512) && IsValidHeader (CurData) )
			return position;
	}
	return -1;
}
