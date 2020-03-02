#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned char sevenzip_signature[] = {'7' , 'z', 0xBC, 0xAF, 0x27, 0x1C};

struct SevenZipHeader {
	char Signature[6];
	BYTE btVersionMajor;
	BYTE btVersionMinor;
	DWORD dwHeaderCRC;
	unsigned __int64 uNextHeaderOffset;
	unsigned __int64 uNextHeaderSize;
	DWORD dwNextHeaderCRC;
	};

const size_t MIN_HEADER_LEN = sizeof (SevenZipHeader);

unsigned long CRC32(
		unsigned long crc,
		const char *buf,
		unsigned int len
		)
{
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		unsigned long c;
		int n, k;

		for (n = 0; n < 256; n++)
		{
			c = (unsigned long)n;
			for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
				crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;
	while (len-- > 0) {
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ 0xffffffffL;
}


static inline BOOL IsValidHeader (const unsigned char *Data)
{
	SevenZipHeader *header = (SevenZipHeader*)Data;

	DWORD crc = CRC32 (0, (const char*)&header->uNextHeaderOffset, sizeof (header->uNextHeaderOffset));
	crc = CRC32 (crc, (const char*)&header->uNextHeaderSize, sizeof (header->uNextHeaderSize));
	crc = CRC32 (crc, (const char*)&header->dwNextHeaderCRC, sizeof (header->dwNextHeaderCRC));

	return !memcmp (&header->Signature, &sevenzip_signature, sizeof (sevenzip_signature)) && (crc == header->dwHeaderCRC);
}

int Is7zHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	const unsigned char *MaxData=Data+DataSize-MIN_HEADER_LEN;
	//const unsigned char *DataEnd=Data+DataSize;

	for (const unsigned char *CurData=Data; CurData<MaxData; CurData++)
	{
		if ( IsValidHeader (CurData) )
			return (int)(CurData-Data);
	}
	return -1;
}
