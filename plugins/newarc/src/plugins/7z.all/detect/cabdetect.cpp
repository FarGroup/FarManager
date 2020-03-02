#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

typedef BYTE u1;
typedef WORD u2;
typedef DWORD u4;

struct CFHEADER
{
  u4 signature;
  u4 reserved1;
  u4 cbCabinet;
  u4 reserved2;
  u4 coffFiles;
  u4 nFiles;
  u1 versionMinor;
  u1 versionMajor;
  u2 cFolders;
  u2 cFiles;
  u2 flags;
  u2 setID;
  u2 iCabinet;
};

int IsCabHeader(const unsigned char *Data,int DataSize)
{
  int I;

  for( I=0; I <= (int)(DataSize-sizeof(struct CFHEADER)); I++ )
  {
    const unsigned char *D=Data+I;
    if (D[0]=='M' && D[1]=='S' && D[2]=='C' && D[3]=='F')
    {
      struct CFHEADER *Header=(struct CFHEADER *)(Data+I);
      if (Header->cbCabinet>sizeof(Header) && Header->coffFiles>sizeof(Header) &&
          Header->coffFiles<0xffff && Header->versionMajor>0 &&
          Header->versionMajor<0x10 && Header->cFolders>0)
      {
        return I;
      }
    }
  }
  return -1;
}
