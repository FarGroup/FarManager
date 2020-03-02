#include <windows.h>
#include <limits.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

struct CFHEADER
{
	BYTE signature[4];    /* cabinet file signature */
	DWORD reserved1;        /* reserved */
	DWORD cbCabinet;        /* size of this cabinet file in bytes */
	DWORD reserved2;        /* reserved */
	DWORD coffFiles;        /* offset of the first CFFILE entry */
	DWORD reserved3;        /* reserved */
	BYTE versionMinor;    /* cabinet file format version, minor */
	BYTE versionMajor;    /* cabinet file format version, major */
	WORD cFolders;        /* number of CFFOLDER entries in this cabinet */
	WORD cFiles;            /* number of CFFILE entries in this cabinet */
	WORD flags;            /* cabinet file option indicators */
	WORD setID;            /* must be the same for all cabinets in a set */
	WORD iCabinet;        /* number of this cabinet file in a set */
};

const unsigned int MIN_HEADER_LEN = sizeof(CFHEADER);

int IsCabHeader(const unsigned char* pData, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	const unsigned char* pMaxData = pData+uDataSize-MIN_HEADER_LEN;

	for (const unsigned char* pCurData = pData; pCurData < pMaxData; pCurData++)
	{
		CFHEADER* pHeader = (CFHEADER*)pCurData;

		if ( (pHeader->signature[0] == 'M') && 
			 (pHeader->signature[1] == 'S') && 
			 (pHeader->signature[2] == 'C') && 
			 (pHeader->signature[3] == 'F') )
		{
			if ( (pHeader->cbCabinet > sizeof(CFHEADER)) && 
				 (pHeader->coffFiles > sizeof(CFHEADER)) &&
				 (pHeader->coffFiles < 0xFFFF) && 
				 (pHeader->versionMajor > 0) &&
				 (pHeader->versionMajor < 0x10) && 
				 (pHeader->cFolders > 0) )
				return (int)(pCurData-pData);
		}
	}

	return -1;
}
