#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const unsigned int MIN_HEADER_LEN = 32; //???

int IsRarHeader(const unsigned char *Data, unsigned int uDataSize)
{
	if ( uDataSize < MIN_HEADER_LEN )
		return -1;

	for (unsigned int i = 0; i < uDataSize-7; i++)
	{
		const unsigned char* D = Data+i;

		if ( (D[0] == 0x52) && (D[1] == 0x45) && (D[2] == 0x7e) && (D[3] == 0x5e) && 
			 (i==0 || (Data[28]==0x52 && Data[29]==0x53 && Data[30]==0x46 && Data[31]==0x58)))
			//if (D[0]==0x52 && D[1]==0x45 && D[2]==0x7e && D[3]==0x5e)
		{
			//OldFormat=TRUE;
			return i;
		}
		// check marker block
		// The marker block is actually considered as a fixed byte sequence: 0x52 0x61 0x72 0x21 0x1a 0x07 0x00
		if (D[0]==0x52 && D[1]==0x61 && D[2]==0x72 && D[3]==0x21 &&
			D[4]==0x1a && D[5]==0x07 && D[6]==0 &&
			D[9]==0x73)                                             // next "archive header"? (Header type: 0x73)
		{
			//if(D[10]&0x80)
				//NeedUsedUnRAR_DLL=TRUE;
			//OldFormat=FALSE;
			return i;
		}
	}
	return -1;
}
