#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif


struct ACEHEADER
{
	WORD  CRC16;        // CRC16 over block
	WORD  HeaderSize;   // size of the block(from HeaderType)
	BYTE  HeaderType;   // archive header type is 0
	WORD  HeaderFlags;
	BYTE  Signature[7]; // '**ACE**'
	BYTE  VerExtract;   // version needed to extract archive
	BYTE  VerCreate;    // version used to create the archive
	BYTE  Host;         // HOST-OS for ACE used to create the archive
	BYTE  VolumeNum;    // which volume of a multi-volume-archive is it?
	DWORD AcrTime;      // date and time in MS-DOS format
	BYTE  Reserved[8];  // 8 bytes reserved for the future
};

const unsigned int MIN_HEADER_LEN = sizeof(ACEHEADER);

//too simple
bool IsAceHeader(const unsigned char* pData, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	const unsigned char* pMaxData = pData+uDataSize-MIN_HEADER_LEN;

	for (const unsigned char* pCurData = pData; pCurData < pMaxData; pCurData++)
	{
		ACEHEADER* pHeader = (ACEHEADER*)pCurData;

		if ( !memcmp(pHeader, "**ACE**", 7) )
		{
			if ( pHeader->HeaderType == 0 )
				return (int)(pCurData-pData);
		}
	}

	return -1;
}
