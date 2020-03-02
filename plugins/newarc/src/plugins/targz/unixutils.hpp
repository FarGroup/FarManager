#ifndef __UNIXUTILS_HPP__
#define __UNIXUTILS_HPP__

//TAR

#define BLOCKSIZE 512

struct posix_header
{       /* byte offset */
  char name[100];   /*   0 */
  char mode[8];     /* 100 */
  char uid[8];      /* 108 */
  char gid[8];      /* 116 */
  char size[12];    /* 124 */
  char mtime[12];   /* 136 */
  char chksum[8];   /* 148 */
  char typeflag;    /* 156 */
  char linkname[100];   /* 157 */
  char magic[6];    /* 257 */
  char version[2];    /* 263 */
  char uname[32];   /* 265 */
  char gname[32];   /* 297 */
  char devmajor[8];   /* 329 */
  char devminor[8];   /* 337 */
  char prefix[155];   /* 345 */
  char unused_chars[12];   /* 500 */
};

/* Identifies the *next* file on the tape as having a long linkname.  */
#define GNUTYPE_LONGLINK 'K'

/* Identifies the *next* file on the tape as having a long name.  */
#define GNUTYPE_LONGNAME 'L'

#define TMAGIC   "ustar"  /* ustar and a null */
#define TMAGLEN  6

//CPIO

struct old_cpio_header
{
  unsigned short c_magic;
  short c_dev;
  unsigned short c_ino;
  unsigned short c_mode;
  unsigned short c_uid;
  unsigned short c_gid;
  unsigned short c_nlink;
  short c_rdev;
  unsigned short c_mtimes[2];
  unsigned short c_namesize;
  unsigned short c_filesizes[2];
  unsigned long c_mtime;  /* Long-aligned copy of `c_mtimes'. */
  unsigned long c_filesize; /* Long-aligned copy of `c_filesizes'. */
  char *c_name;
};

struct new_cpio_header
{
  unsigned short c_magic;
  unsigned long c_ino;
  unsigned long c_mode;
  unsigned long c_uid;
  unsigned long c_gid;
  unsigned long c_nlink;
  unsigned long c_mtime;
  unsigned long c_filesize;
  long c_dev_maj;
  long c_dev_min;
  long c_rdev_maj;
  long c_rdev_min;
  unsigned long c_namesize;
  unsigned long c_chksum;
  char *c_name;
  char *c_tar_linkname;
};

//TOOLS

//extern void UnixToDos(long time,struct dosdate_t *d,struct time *t);
extern DWORD GetOctal(const char *Str);
extern DWORD GetHex(const char *Str);
extern void UnixTimeToFileTime(DWORD UnixTime,FILETIME *LocalFileTime);
extern char *AdjustTARFileName(char *FileName);

#if _MSC_VER || defined(__MINGW32__)
struct dosdate_t {
   unsigned char day;       /* 1-31 */
   unsigned char month;     /* 1-12 */
   unsigned int  year;      /* 1980 - 2099 */
   unsigned char dayofweek; /* 0 - 6 (0=Sunday) */
};
struct time {
   unsigned char ti_min;      /* minutes */
   unsigned char ti_hour;     /* hours */
   unsigned char ti_hund;     /* hundredths of seconds */
   unsigned char ti_sec;      /* seconds */
};
#endif

#endif
