#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

struct LZH_Level0 {
/* 00 */ BYTE  HeadSize;      // Header Size in Bytes
/* 01 */ BYTE  CheckSum;      // Header Checksum
/* 02 */ BYTE  HeadID[3];     // Header ID Code
/* 05 */ BYTE  Method;        // Compression Method
/* 06 */ BYTE  free1;
/* 07 */ DWORD PackSize;      // Packed File Size
/* 11 */ DWORD UnpSize;       // Original File Size not decompressed yet
/* 15 */ WORD  FTime;         // File Time an Date Stamp
/* 17 */ WORD  FDate;         // File Time an Date Stamp
/* 19 */ BYTE  FAttr;         // File Attributes
/* 20 */ BYTE  FLevel;        // level = 0x00
/*
Level 0
   21      1 byte   Filename / path length in bytes (f)
   22     (f)bytes  Filename / path
   22+(f)  2 bytes  CRC-16 of original file
   24+(f) (n)bytes  Compressed data
*/
};

BOOL CheckLZHHeader(struct LZH_Level0 *lzh)
{
  return (lzh->HeadID[0]=='-' && lzh->HeadID[1]=='l' && (lzh->HeadID[2]=='h' || lzh->HeadID[2]=='z') &&
         ((lzh->Method>='0' && lzh->Method<='9') || lzh->Method=='d' || lzh->Method=='s') &&
         lzh->free1 == '-' && lzh->FLevel <= 2);
}


int IsLzhHeader(const unsigned char *Data,int DataSize)
{
  for (int I=0;I<DataSize-5;I++)
  {
    struct LZH_Level0 *lzh=(struct LZH_Level0*)(Data+I);
    if(CheckLZHHeader(lzh))
    {
      const unsigned char *D=Data+I;
      if(lzh->FLevel == 0 && D[21] > 0 && D[22] != 0 && (lzh->HeadSize-2-(int)D[21]) == sizeof(struct LZH_Level0)-1)
      {
        return I;
      }

      if((lzh->FLevel == 1) || (lzh->FLevel == 2)) // !TODO:
      {
        return I;
      }
    }
  }
  return -1;
}
