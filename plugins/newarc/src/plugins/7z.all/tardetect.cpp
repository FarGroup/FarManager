#include "7z.h"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

#if defined(__GNUC__)
#define _strtoi64 strtoul
#endif


struct posix_header
{                               /* byte offset */
  char name[100];               /*   0 = 0x000 */
  char mode[8];                 /* 100 = 0x064 */
  char uid[8];                  /* 108 = 0x06C */
  char gid[8];                  /* 116 = 0x074 */
  char size[12];                /* 124 = 0x07C */
  char mtime[12];               /* 136 = 0x088 */
  char chksum[8];               /* 148 = 0x094 */
  char typeflag;                /* 156 = 0x09C */
  char linkname[100];           /* 157 = 0x09D */
  char magic[6];                /* 257 = 0x101 */
  char version[2];              /* 263 = 0x107 */
  char uname[32];               /* 265 = 0x109 */
  char gname[32];               /* 297 = 0x129 */
  char devmajor[8];             /* 329 = 0x149 */
  char devminor[8];             /* 337 = 0x151 */
  char prefix[155];             /* 345 = 0x159 */
                                /* 500 = 0x1F4 */
};

#define TMAGIC   "ustar"    // ustar and a null
#define TMAGLEN  6
#define GNUTMAGIC  "GNUtar"    // 7 chars and a null
#define GNUTMAGLEN 7
#define TVERSION "00"       // 00 and no null
#define TVERSLEN 2

/* OLDGNU_MAGIC uses both magic and version fields, which are contiguous.
   Found in an archive, it indicates an old GNU header format, which will be
   hopefully become obsolescent.  With OLDGNU_MAGIC, uname and gname are
   valid, though the header is not truly POSIX conforming.  */
#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */


enum archive_format
{
  DEFAULT_FORMAT,       /* format to be decided later */
  V7_FORMAT,            /* old V7 tar format */
  OLDGNU_FORMAT,        /* GNU format as per before tar 1.12 */
  POSIX_FORMAT,         /* restricted, pure POSIX format */
  GNU_FORMAT            /* POSIX format with GNU extensions */
};

enum archive_format TarArchiveFormat;

__int64 GetOctal(const char *Str)
{
  char *endptr;
  return _strtoi64(Str,&endptr,8);
}

int IsTarHeader (const unsigned char *Data, int DataSize)
{
  size_t I;
  struct posix_header *Header;

  if (DataSize<sizeof(struct posix_header))
    return -1;

  Header=(struct posix_header *)Data;

  if(!lstrcmp (Header->magic, TMAGIC))
    TarArchiveFormat = POSIX_FORMAT;
  else if(!lstrcmp (Header->magic, OLDGNU_MAGIC))
    TarArchiveFormat = OLDGNU_FORMAT;
  else
    TarArchiveFormat = V7_FORMAT;

  for (I=0; Header->name[I] && I < sizeof(Header->name); I++)
    if (Header->name[I] < ' ')
      return -1;

  //for (I=0; I < (&Header->typeflag - &Header->mode[0]); I++)
  for (I=0; I < sizeof(Header->mode); I++)
  {
    int Mode=Header->mode[I];
    if (Mode > '7' || Mode < '0' && Mode && Mode != ' ')
      return -1;
  }

  for (I=0; Header->mtime[I] && I < sizeof(Header->mtime); I++)
    if (Header->mtime[I] < ' ')
      return -1;

  __int64 Sum=256;
  for(I=0; I <= 147; I++)
    Sum+=Data[I];

  for(I=156; I < 512; I++)
    Sum+=Data[I];

  return (Sum==GetOctal(Header->chksum)?0:-1);
}
