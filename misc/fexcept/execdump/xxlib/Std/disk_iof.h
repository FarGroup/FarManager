#ifndef __FILE_IO_UNI
#define __FILE_IO_UNI

#if !defined(SEEK_CUR)
  #define SEEK_CUR    1
  #define SEEK_END    2
  #define SEEK_SET    0
#endif

#if !defined(R_OK)
  #define R_OK        4
  #define W_OK        2
  #define F_OK        0
  #define X_OK        1
#endif

#if !defined( S_ISDIR )
  #define S_ISDIR(v) IS_FLAG(v,S_IFDIR)
#endif

enum seekTypes {
  seekBEGIN = SEEK_SET,
  seekCUR   = SEEK_CUR,
  seekEND   = SEEK_END
};

enum accTypes {
  accREAD     = R_OK,
  accWRITE    = W_OK,
  accEXIST    = F_OK,
  accEXECUTE  = X_OK
};

enum FileType {
  ftUnknown,
  ftDisk,
  ftDevice,
  ftPipe
};

enum _FIO_IO_Operation {
 FIO_IO_READ       = 0,
 FIO_IO_WRITE      = 1,
 FIO_IO_ERROR      = 2
};

#if defined(__GNUC__)
   typedef int ATTR_TYPE;
   #define flDirectory      S_IFDIR
   #define flAttrCount      10
   #define ALL_FILES        "*"
   #define SLASH_CHAR       '/'
   #define SLASH_STR        "/"
   #define MAX_PATH_SIZE    MAXPATHLEN
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   #define FIO_DEF_ATTR     0640

   #define _creat      creat

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      open( fname,omode )
   #define FIO_CREAT( fname,attr)       creat( fname,attr )
   #define FIO_READ( f,buff,count)      read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     write( f,buff,count )
   #define FIO_CLOSE( f)                close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) != -1)
   #define FIO_STAT( f,st )             (lstat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,u,g )           (chown( f,u,g ) == 0)
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   getuid()
   #define FIO_GETGID                   getgid()
   #define FIO_ALLFILES                 0xFFFF
#else
#if defined(__QNX__)
   typedef int ATTR_TYPE;
   #define flDirectory      _S_IFDIR
   #define flAttrCount      10
   #define ALL_FILES        "*"
   #define SLASH_CHAR       '/'
   #define SLASH_STR        "/"
   #define MAX_PATH_SIZE    (_MAX_PATH + FILENAME_MAX + 1)
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   #define FIO_DEF_ATTR     0640

   #define _creat      creat

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      open( fname,omode )
   #define FIO_CREAT( fname,attr)       creat( fname,attr )
   #define FIO_READ( f,buff,count)      read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     write( f,buff,count )
   #define FIO_CLOSE( f)                close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) != -1)
   #define FIO_STAT( f,st )             (lstat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,u,g )           (chown( f,u,g ) == 0)
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   getuid()
   #define FIO_GETGID                   getgid()
   #define FIO_ALLFILES                 0xFFFF
#else
#if defined(__REALDOS__)
   #define flDirectory      FA_DIREC
   #define flAttrCount      6
   #define ALL_FILES        "*.*"
   #define SLASH_CHAR       '\\'
   #define SLASH_STR        "\\"
   #define MAX_PATH_SIZE    (MAXPATH+1)
   #define MAX_FILE_COUNT   500
   #define MAX_CMD_SIZE     256
   typedef mode_t ATTR_TYPE;
   #define FIO_DEF_ATTR     0

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      _open( fname,omode )
   #define FIO_CREAT( fname,attr)       _creat( fname,attr )
   #define FIO_READ( f,buff,count)      _read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _write( f,buff,count )
   #define FIO_CLOSE( f)                _close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)
#else
#if defined(__PROTDOS__)
   #define flDirectory      FA_DIREC
   #define flAttrCount      6
   #define ALL_FILES        "*.*"
   #define SLASH_CHAR       '\\'
   #define SLASH_STR        "\\"
   #define MAX_PATH_SIZE    (MAXPATH+1)
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   typedef mode_t ATTR_TYPE;
   #define FIO_DEF_ATTR     0

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      _open( fname,omode )
   #define FIO_CREAT( fname,attr)       _creat( fname,attr )
   #define FIO_READ( f,buff,count)      _read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _write( f,buff,count )
   #define FIO_CLOSE( f)                _close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)
#else
#if defined(__SCWIN32__)
   #define ALL_FILES        "*.*"
   #define SLASH_CHAR       '\\'
   #define SLASH_STR        "\\"
   #define MAX_PATH_SIZE    (MAXPATH+1)
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   #define flDirectory      FILE_ATTRIBUTE_DIRECTORY
   #define flAttrCount      8
   typedef DWORD ATTR_TYPE;
   #define FIO_DEF_ATTR     0

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      _open( fname,omode )
   #define FIO_CREAT( fname,attr)       _creat( fname,attr )
   #define FIO_READ( f,buff,count)      _read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _write( f,buff,count )
   #define FIO_CLOSE( f)                _close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)
#else
#if defined(__BCWIN32__)
   #define ALL_FILES                    "*.*"
   #define SLASH_CHAR                   '\\'
   #define SLASH_STR                    "\\"
   #define MAX_PATH_SIZE                (MAXPATH+1)
   #define MAX_FILE_COUNT               5000
   #define MAX_CMD_SIZE                 1024
   #define flDirectory                  FILE_ATTRIBUTE_DIRECTORY
   #define flAttrCount                  8
   typedef DWORD                        ATTR_TYPE;
   #define FIO_DEF_ATTR                 0

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      Stub_rtl_open( fname,omode )
   #define FIO_CREAT( fname,attr)       Stub_rtl_creat( fname,attr )
   #define FIO_READ( f,buff,count)      _rtl_read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _rtl_write( f,buff,count )
   #define FIO_CLOSE( f)                _rtl_close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)

   #define VERSION_WIN_NT               (GetVersion() < 0x80000000UL)
   #define VERSION_WIN_9X               (GetVersion() >= 0x80000000UL)

   HDECLSPEC int MYRTLEXP Stub_rtl_open(const char *filename, int oflags);
   HDECLSPEC int MYRTLEXP Stub_rtl_creat(const char *path, int attrib);
#else
#if defined(__BCWIN16__)
   #define ALL_FILES                    "*.*"
   #define SLASH_CHAR                   '\\'
   #define SLASH_STR                    "\\"
   #define MAX_PATH_SIZE                (MAXPATH+1)
   #define MAX_FILE_COUNT               500
   #define MAX_CMD_SIZE                 256
   #define flDirectory                  FA_DIREC
   #define flAttrCount                  6
   typedef unsigned                     ATTR_TYPE;
   #define FIO_DEF_ATTR                 0

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      _rtl_open( fname,omode )
   #define FIO_CREAT( fname,attr)       _rtl_creat( fname,attr )
   #define FIO_READ( f,buff,count)      _rtl_read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _rtl_write( f,buff,count )
   #define FIO_CLOSE( f)                _rtl_close( f )
   #define FIO_SEEK( f,off,from)        lseek ( f,off,from )
   #define FIO_TELL( f )                tell( f )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)

   #define VERSION_WIN_NT               (GetVersion() < 0x80000000UL)
   #define VERSION_WIN_9X               (GetVersion() >= 0x80000000UL)
#else
#if defined(__MSOFT) || defined(__INTEL)
   #define ALL_FILES        "*.*"
   #define SLASH_CHAR       '\\'
   #define SLASH_STR        "\\"
   #define MAX_PATH_SIZE    (_MAX_PATH+1)
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   #define flDirectory      FILE_ATTRIBUTE_DIRECTORY
   #define flAttrCount      8
   typedef DWORD ATTR_TYPE;
   #define FIO_DEF_ATTR     (_S_IREAD|_S_IWRITE)

   #undef O_BINARY
   #define O_BINARY        _O_BINARY

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      _open( fname,omode )
   #define FIO_CREAT( fname,attr)       _creat( fname,S_IWRITE )
   #define FIO_READ( f,buff,count)      _read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     _write( f,buff,count )
   #define FIO_CLOSE( f)                _close( f )
   #define FIO_SEEK( f,off,from)        _lseek ( f,off,from )
   #define FIO_TELL( f )                _tell( f )
   #define FIO_TRUNC( f,size )          (_chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (_access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (_chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    _chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)

   #define VERSION_WIN_NT               (GetVersion() < 0x80000000UL)
   #define VERSION_WIN_9X               (GetVersion() >= 0x80000000UL)
#else
#if defined(__DMC)
   #define ALL_FILES        "*.*"
   #define SLASH_CHAR       '\\'
   #define SLASH_STR        "\\"
   #define MAX_PATH_SIZE    (_MAX_PATH+1)
   #define MAX_FILE_COUNT   5000
   #define MAX_CMD_SIZE     1024
   #define flDirectory      FILE_ATTRIBUTE_DIRECTORY
   #define flAttrCount      8
   typedef DWORD ATTR_TYPE;
   #define FIO_DEF_ATTR     (_S_IREAD|_S_IWRITE)

   #undef O_BINARY
   #define O_BINARY        _O_BINARY

   #define FIO_EOF( f )                 eof( f )
   #define FIO_OPEN( fname,omode )      open( fname,omode )
   #define FIO_CREAT( fname,attr)       creat( fname,attr )
   #define FIO_READ( f,buff,count)      read( f,buff,count )
   #define FIO_WRITE( f,buff,count)     write( f,buff,count )
   #define FIO_CLOSE( f)                close( f )
   #define FIO_SEEK( f,off,from)        _lseek ( f,off,from )
   #define FIO_TELL( f )                _lseek( f, 0L, SEEK_CUR )
   #define FIO_TRUNC( f,size )          (chsize( f,size ) == 0)
   #define FIO_ACCESS( f,m )            (access( f,m ) == 0)
   #define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
   #define FIO_STAT( f,st )             (stat(f,st) == 0)
   #define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
   #define FIO_CHOWN( f,g,u )           1
   #define FIO_CHDIR                    chdir
   #define FIO_GETUID                   1
   #define FIO_GETGID                   1
   #define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)

   #define VERSION_WIN_NT               (GetVersion() < 0x80000000UL)
   #define VERSION_WIN_9X               (GetVersion() >= 0x80000000UL)
#else
#if defined(__TEC32__)
   #define ALL_FILES        "*"
   #define SLASH_CHAR       '/'
   #define SLASH_STR        "/"
   #define MAX_PATH_SIZE    256
   #define MAX_FILE_COUNT   100
   #define MAX_CMD_SIZE     256
   #define flDirectory      1
   #define flAttrCount      2
   typedef BYTE ATTR_TYPE;
   #define FIO_DEF_ATTR     0

   #undef O_BINARY
   #define O_BINARY

   /* Has no file IO

   #define FIO_EOF( f )
   #define FIO_OPEN( fname,omode )
   #define FIO_CREAT( fname,attr)
   #define FIO_READ( f,buff,count)
   #define FIO_WRITE( f,buff,count)
   #define FIO_CLOSE( f)
   #define FIO_SEEK( f,off,from)
   #define FIO_TELL( f )
   #define FIO_TRUNC( f,size )
   #define FIO_ACCESS( f,m )
   #define FIO_CHMOD( f,at )
   #define FIO_STAT( f,st )
   #define FIO_HSTAT( f,st )
   #define FIO_CHOWN( f,g,u )
   #define FIO_CHDIR
   #define FIO_GETUID
   #define FIO_GETGID
   #define FIO_ALLFILES
   */
#else
   #error ERR_PLATFORM
#endif //TEC32
#endif //DMC
#endif //MSOFT
#endif //BC WIN32
#endif //BC WIN16
#endif //SCWIN32
#endif //PROTDOS
#endif //REALDOS
#endif //QNX
#endif //GNUC

#if defined(__cplusplus)
/* Class to hold FIO handler.
   Auto close handle at destroy.
*/
struct FIO_Save {
  int File;

  FIO_Save( void )  { File = -1; }
  FIO_Save( int f ) { File = f; }
  ~FIO_Save()       { Close(); }

  void Close( void ) { if ( File != -1 ) FIO_CLOSE(File); File = -1; }

  operator int( void ) { return File; }
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  struct FIO_Save&
#endif
    operator =( int f )  { Close(); File = f; return *this; }
};
#endif

#endif
