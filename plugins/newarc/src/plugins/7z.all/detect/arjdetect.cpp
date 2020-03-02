#include <windows.h>
#include <limits.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

#ifndef FNAME_MAX
#define FNAME_MAX           512
#endif
#define FIRST_HDR_SIZE    30
#define COMMENT_MAX     2048
#define HEADERSIZE_MAX   (FIRST_HDR_SIZE + 10 + FNAME_MAX + COMMENT_MAX)

#define CRC_MASK        0xFFFFFFFFL
static DWORD crctable[UCHAR_MAX + 1];
static BOOL CRCInit=FALSE;
DWORD CRC;

#define UPDATE_CRC(r,c) r=crctable[((BYTE)(r)^(BYTE)(c))&0xff]^(r>>CHAR_BIT)
#define CRCPOLY         0xEDB88320L

static void make_crctable(void)
{
  UINT i, j;
  DWORD r;

  for (i = 0; i <= UCHAR_MAX; i++)
  {
    r = i;
    for (j = CHAR_BIT; j > 0; j--)
    {
      if (r & 1)
          r = (r >> 1) ^ CRCPOLY;
      else
          r >>= 1;
    }
    crctable[i] = r;
  }
}

static void crc_buf(const char *str, int len)
{
    while (len--)
        UPDATE_CRC(CRC, *str++);
}

int IsArjHeader(const unsigned char *Data,int DataSize)
{
  if(!CRCInit)
  {
    make_crctable();
    CRCInit=TRUE;
  }

  for (int I=0;I<DataSize-11;I++)
  {
    const unsigned char *D=Data+I;
    WORD HeaderSize;
    if (D[0]==0x60 && D[1]==0xEA &&
       (HeaderSize=*(WORD*)(D+2)) <= HEADERSIZE_MAX &&
       D[7]<0x10 &&
       D[10]==2 &&
       I+4+HeaderSize < DataSize-11)
    {
      CRC=CRC_MASK;
      crc_buf((char *)D+4, (int) HeaderSize);
      if ((CRC ^ CRC_MASK) == *(DWORD*)(D+4+HeaderSize))
      {
        return I;
      }
    }
  }
  return -1;
}
