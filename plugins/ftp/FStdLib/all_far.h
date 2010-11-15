#ifndef __FAR_PLUGINS_STD_HEADERS
#define __FAR_PLUGINS_STD_HEADERS

/** @page fsdMacro Global macroses
    @ingroup FSTDLib

    Library define some macroses for identify used compiller, platform
    (currently only Win32) and compile enviropment. \n
    During porting to other compillers or targets, please add new macro
    names and change sources used this macroses.

    @section fsdPlatform Platform macroses
    Macroses define on witch platform cources compilled.
     - __HWIN__ Defined on win32 platform.

    @section fsdCompiller Compiller macroses
    Macroses define witch compiller used to compile sources.
     - __MSOFT     Defined if compilled by Visual C compiller.
     - __GNU       Defined if compilled by GCC compiller.

    @section fsdCompillerType Macroses for compiller types
    Macroses define witch type of compiller used to compile sources.
     - __BCWIN32__ Defined if used Borland C/CPP compiller on Win32 platform.
     - __BCB1__    Defined if used Borland C Builder version 1.x.
     - __MSWIN32__ Defined if used Visual C compiller on Win32 platform.
     - __SCWIN32__ Defined if used Symantec C compiller on Win32 platform.
     - __GWIN32__  Defined if used GCC compiller on Win32 platform.

    @section fsdOther Different macroses
    Different macroses globally affected to sources.
     - __DEBUG__         Defined if debug compilling turned ON.
     - __HCONSOLE__      Allways defined on current FAR platform.

    @section fsdMem Memory management
     - __HEAP_MEMORY__   If defined the memory allocator will use HeapXXX windows API functions for
                         memory allocations.
     - __STD_MEMORY__    If defined the memory allocator will use RTL malloc\free for memory allocations.
*/

// --------------------------------------------------------------
//Global compiller types
#undef __MSOFT      //Microsoft
#undef __DMC        //Digital Mars Compoller
#undef __INTEL      //Intel compiller
#undef __GNU        //GCC

//Global targets models
#undef __HWIN__

//! MS 6.0
#if defined(_MSC_VER)
#define __HWIN__     1
#define __MSOFT      1
#define __MSWIN32__  1
#endif

#if defined(__GNUC__) && defined( __WIN32__ )
#define __HWIN__     1
#define __GNU        1
#define __GWIN32__   1
#endif

//- DEBUG
#if defined(_DEBUG)           //BCB debug flag
#undef __DEBUG__
#define __DEBUG__ 1
#endif
#ifdef _DEBUG                 //VC debug flag
#undef __DEBUG__
#define __DEBUG__ 1
#endif
#ifdef NDEBUG                 //VC release
#undef __DEBUG__
#endif
#if defined(NDEBUG)
#undef __DEBUG__
#endif

//! HEADERS
#include <windows.h>

#include <direct.h>             // _chdir
#include <dos.h>                // _argv
#include <io.h>                 // struct ftime, SEEK_xxx
#include <signal.h>             // SIGxxx
#include <sys/stat.h>           // stat
#include <malloc.h>             // alloc,NULL
#include <math.h>               // sqrt
#include <conio.h>              // getch
#include <process.h>            // system,signal
#include <ctype.h>              // isprint
#include <stdio.h>              // sprintf
#include <stdlib.h>             // atexit
#include <string.h>             // strXXX
#include <fcntl.h>              // O_RDWR,xxx
#include <setjmp.h>             // exseptions
#include <assert.h>             // assert
#include <errno.h>              // errors
#include <stdarg.h>             // va_list
#include <time.h>               // timespec
#include <winsock.h>

//- MACROSES
#define MAX_DWORD                 ((DWORD)0xFFFFFFFFUL)
#define MAX_WORD                  ((WORD)0xFFFFU)
#define MAX_BYTE                  ((WORD)0xFFU)

//- TYPES
typedef double        CMP_TIME_TYPE;
typedef DWORD         TIME_TYPE;

// --------------------------------------------------------------
#ifndef Max
#if defined(__cplusplus)
/**@brief Returns maximal value.*/
template <class T> inline T Max(T a,T b) { return (a>b)?a:b; }
#else
#define Max(a,b) (((a)>(b))?(a):(b))
#endif
#endif
#ifndef Min
#if defined(__cplusplus)
/**@brief Returns minimal value.*/
template <class T> inline T Min(T a,T b) { return (a<b)?a:b; }
#else
#define Min(a,b) (((a)<(b))?(a):(b))
#endif
#endif
#ifndef Abs
#if defined(__cplusplus)
/**@brief Returns absolute value.*/
template <class T> inline T Abs(T v) { return (v>0)?v:(-v); }
#else
#define Abs(v) (((v)<0)?(-(v)):(v))
#endif
#endif
#ifndef Between
#if defined(__cplusplus)
/**@brief Check if value between boud values.*/
template <class T> inline BOOL Between(T val,T a,T b) { return (val >= a && val <= b)?TRUE:FALSE; }
#else
#define Between(val,a,b) (((val) >= (a) && (val) <= (b))?TRUE:FALSE)
#endif
#endif
#ifndef Swap
#if defined(__cplusplus)
/**@brief Swapt variables values.*/
template <class T> void Swap(T& a,T& b) { T tmp = a; a = b; b = tmp; }
#else
//??
#endif
#endif

#if !defined( ARRAY_SIZE )
#define ARRAY_SIZE( v )  (sizeof(v) / sizeof( (v)[0] ))
#endif

/** @def SET_HI_WORD( dw,w ) Sets high subword in DWORD value. */
/** @def SET_LO_WORD( dw,w ) Sets low subword in DWORD value. */
#define SET_HI_WORD( dw,w )   ((((DWORD)dw)&0x0000FFFFUL) | (((DWORD)w) << 16))
#define SET_LO_WORD( dw,w )   ((((DWORD)dw)&0xFFFF0000UL) | (((WORD)w)&0xFFFFUL))

#ifndef HI_WORD
#define HI_WORD( dw )       ((WORD)(((DWORD)(dw)) >> 16))
#define LO_WORD( dw )       ((WORD)(((DWORD)(dw))&0x0000FFFFUL))
#endif

#define MK_DWORD( hw,lw )     (((((DWORD)hw)&0x0000FFFFUL) << 16) | (((WORD)lw)&0x0000FFFFUL))

#define SET_HI_BYTE( w,b )    ((((WORD)(w))&0x00FFU) | (((BYTE)(b)) << 8))
#define SET_LO_BYTE( w,b )    ((((WORD)(w))&0xFF00U) | ((BYTE)(b)))
#ifndef HI_BYTE
#define HI_BYTE( w )        ( ((WORD)(w)) >> 8 )
#define LO_BYTE( w )        ( (BYTE)((WORD)(w))&0x00FFU )
#endif
#define MK_WORD( hb,lb )      ((WORD)(((((WORD)(hb))&0x00FFU) << 8) | (((BYTE)(lb))&0x00FFU)))

#define sSET_HI_WORD( dw,w )  ((((int)(dw))&0x0000FFFFUL) | (((int)(w)) << 16))
#define sSET_LO_WORD( dw,w )  ((((int)(dw))&0xFFFF0000UL) | (((short)(w))&0xFFFFUL))
#define sHI_WORD( dw )        ( (short)((dw) >> 16) )
#define sLO_WORD( dw )        ( (short)(dw) )
#define sMK_DWORD( lw,hw )    ((int)(((((int)(hw))&0x0000FFFFUL) << 16) | (((short)(lw))&0xFFFFUL)))


#define IS_BIT( val,num )     IS_FLAG(((DWORD)(val)),1UL<<(num)))
#define IS_FLAG( val,flag )   (((val)&(flag))==(flag))
#define SET_FLAG( val,flag )  (val |= (flag))
#define CLR_FLAG( val,flag )  (val &= ~(flag))
#define SWITCH_FLAG( f,v )    do{ if (IS_FLAG(f,v)) CLR_FLAG(f,v); else SET_FLAG(f,v); }while(0)

//- MK_ID
#define MK_ID( v,v1,v2,v3 ) MK_DWORD( MK_WORD(v3,v2),MK_WORD(v1,v) )

#define HAbort __WinAbort
#define THROW_ERROR(err,fl,nm)      HAbort( "Assertion...\nConditin: \"%s\"\nAt file: \"%s:%d\"",err,fl,nm )

#if defined(__DEBUG__) || defined(__USEASSERT__)
#define Assert( p )       do{ if (!(p)) THROW_ERROR( #p , __FILE__ , __LINE__ ); }while(0)
#else
#define Assert( p )
#endif

// --------------------------------------------------------------
/** @defgroup DiskIO Platform-independed file IO routines
    @{
*/
#if !defined(SEEK_CUR)
#define SEEK_SET    0  ///<Seeks from beginning of file
#define SEEK_CUR    1  ///<Seeks from current position
#define SEEK_END    2  ///<Seeks from end of file
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

/** @brief File pointer move types. */
enum seekTypes
{
	seekBEGIN = SEEK_SET,   ///< Move file pointer from start of file.
	seekCUR   = SEEK_CUR,   ///< Move file pointer from current position.
	seekEND   = SEEK_END    ///< Move file pointer from end of file.
};

/** @brief Type of possible file access type. */
enum accTypes
{
	accREAD    = R_OK,      ///< Check file for \a read posibility
	accWRITE   = W_OK,      ///< Check file for \a write posibility
	accEXIST   = F_OK,      ///< Check file for \a existings
	accEXECUTE = X_OK       ///< Check file for \a execute
};

#if defined(__QNX__)
#define flDirectory                  _S_IFDIR
#define ALL_FILES                    "*"
#define SLASH_CHAR                   '/'
#define SLASH_STR                    "/"
#define MAX_PATH_SIZE                (_MAX_PATH + FILENAME_MAX + 1)
#define MAX_CMD_SIZE                 1024
#define FIO_DEF_ATTR                 0640
typedef int                          ATTR_TYPE;

#define FIO_EOF( f )                 eof( f )
#define FIO_OPEN( fname,omode )      open( fname,omode )
#define FIO_CREAT( fname,attr)       creat( fname,attr )
#define FIO_READ( f,buff,count)      read( f,buff,count )
#define FIO_WRITE( f,buff,count)     write( f,buff,count )
#define FIO_CLOSE( f)                close( f )
#define FIO_SEEK( f,off,from)        lseek ( f,off,from )
#define FIO_TELL( f )                tell( f )
#define FIO_TRUNC( f,size )          chsize( f,size )
#define FIO_ACCESS( f,m )            (access( f,m ) == 0)
#define FIO_CHMOD( f,at )            (chmod(f,at) != -1)
#define FIO_STAT( f,st )             (lstat(f,st) == 0)
#define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
#define FIO_CHOWN( f,u,g )           (chown( f,u,g ) == 0)
#define FIO_CHDIR                    chdir
#define FIO_GETUID                   getuid()
#define FIO_GETGID                   getgid()
#define FIO_ERROR                    strerror( errno )
#define FIO_ERRORN                   ((DWORD)errno)
#define FIO_SETERRORN(v)             errno = (int)(v)
#define FIO_ALLFILES                 0xFFFF
#else
#if defined(__SCWIN32__)
#define ALL_FILES                    "*.*"
#define SLASH_CHAR                   '\\'
#define SLASH_STR                    "\\"
#define MAX_PATH_SIZE                (MAXPATH+1)
#define MAX_CMD_SIZE                 1024
#define flDirectory                  FILE_ATTRIBUTE_DIRECTORY
typedef DWORD                        ATTR_TYPE;
#define FIO_DEF_ATTR                 0

#define FIO_EOF( f )                 eof( f )
#define FIO_OPEN( fname,omode )      _open( fname,omode )
#define FIO_CREAT( fname,attr)       _creat( fname,attr )
#define FIO_READ( f,buff,count)      _read( f,buff,count )
#define FIO_WRITE( f,buff,count)     _write( f,buff,count )
#define FIO_CLOSE( f)                _close( f )
#define FIO_SEEK( f,off,from)        lseek ( f,off,from )
#define FIO_TELL( f )                tell( f )
#define FIO_TRUNC( f,size )          chsize( f,size )
#define FIO_ACCESS( f,m )            (access( f,m ) == 0)
#define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
#define FIO_STAT( f,st )             (stat(f,st) == 0)
#define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
#define FIO_CHOWN( f,g,u )           1
#define FIO_CHDIR                    chdir
#define FIO_GETUID                   1
#define FIO_GETGID                   1
#define FIO_ERROR                    __WINError()
#define FIO_ERRORN                   GetLastError()
#define FIO_SETERRORN(v)             SetLastError( (DWORD)(v) )
#define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)
#else
#if defined(__BORLAND)
#define ALL_FILES                    "*.*"
#define SLASH_CHAR                   '\\'
#define SLASH_STR                    "\\"
#define MAX_PATH_SIZE                (MAXPATH+1)
#define MAX_CMD_SIZE                 1024
#define flDirectory                  FILE_ATTRIBUTE_DIRECTORY
typedef DWORD ATTR_TYPE;
#define FIO_DEF_ATTR                 0

#define FIO_EOF( f )                 eof( f )
#define FIO_OPEN( fname,omode )      _rtl_open( fname,omode )
#define FIO_CREAT( fname,attr)       _rtl_creat( fname,attr )
#define FIO_READ( f,buff,count)      _rtl_read( f,buff,count )
#define FIO_WRITE( f,buff,count)     _rtl_write( f,buff,count )
#define FIO_CLOSE( f)                _rtl_close( f )
#define FIO_SEEK( f,off,from)        lseek ( f,off,from )
#define FIO_TELL( f )                tell( f )
#define FIO_TRUNC( f,size )          chsize( f,size )
#define FIO_ACCESS( f,m )            (access( f,m ) == 0)
#define FIO_CHMOD( f,at )            (chmod(f,at) == 0)
#define FIO_STAT( f,st )             (stat(f,st) == 0)
#define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
#define FIO_CHOWN( f,g,u )           1
#define FIO_CHDIR                    chdir
#define FIO_GETUID                   1
#define FIO_GETGID                   1
#define FIO_ERROR                    __WINError()
#define FIO_ERRORN                   GetLastError()
#define FIO_ERRORN_IO                _doserrno
#define FIO_SETERRORN(v)             SetLastError( (DWORD)(v) )
#define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)
#else
#if defined(__MSOFT) || defined(__INTEL) || defined(__GNU)
#define ALL_FILES                    "*.*"
#define SLASH_CHAR                   '\\'
#define SLASH_STR                    "\\"
#define MAX_PATH_SIZE                (_MAX_PATH+1)
#define MAX_CMD_SIZE                 1024
#define flDirectory                  FILE_ATTRIBUTE_DIRECTORY
typedef DWORD                        ATTR_TYPE;
#define FIO_DEF_ATTR                 0

#undef O_BINARY
#define O_BINARY        _O_BINARY

#define FIO_EOF( f )                 eof( f )
#define FIO_OPEN( fname,omode )      open( fname,omode )
#define FIO_CREAT( fname,attr)       _creat( fname,_S_IREAD|_S_IWRITE )
#define FIO_READ( f,buff,count)      read( f,buff,count )
#define FIO_WRITE( f,buff,count)     write( f,buff,count )
#define FIO_CLOSE( f)                close( f )
#define FIO_SEEK( f,off,from)        lseek ( f,off,from )
#define FIO_TELL( f )                tell( f )
#define FIO_TRUNC( f,size )          chsize( f,size )
#define FIO_ACCESS( f,m )            (_access( f,m ) == 0)
#define FIO_CHMOD( f,at )            (_chmod(f,at) == 0)
#define FIO_STAT( f,st )             (stat(f,st) == 0)
#define FIO_HSTAT( f,st )            (fstat(f,st) == 0)
#define FIO_CHOWN( f,g,u )           1
#define FIO_CHDIR                    _chdir
#define FIO_GETUID                   1
#define FIO_GETGID                   1

#define FIO_ERROR                    __WINError()
#define FIO_ERRORN                   GetLastError()
#define FIO_ERRORN_IO                GetLastError()
#define FIO_SETERRORN(v)             SetLastError( (DWORD)(v) )
#define FIO_ALLFILES                 (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH)

#define EZERO         0             /*0    Error 0                          */
#define EINVFNC       EPERM         /*1    Invalid function number          */
#ifndef ENOFILE
#define ENOFILE       ENOENT        /*2    File not found                   */
#endif
#define ENOPATH       ESRCH         /*3    Path not found                   */
#define ECONTR        E2BIG         /*7    Memory blocks destroyed          */
#define EINVMEM       EBADF         /*9    Invalid memory block address     */
#define EINVENV       ECHILD        /*10    Invalid environment              */
#define EINVFMT       EAGAIN        /*11    Invalid format                   */
#define EINVACC       ENOMEM        /*12    Invalid access code              */
#define EINVDAT       EACCES        /*13    Invalid data                     */
#define EINVDRV       15            /*15    Invalid drive specified          */
#define ECURDIR       EBUSY         /*16    Attempt to remove CurDir         */
#define ENOTSAM       EEXIST        /*17    Not same device                  */
#define ENMFILE       EXDEV         /*18    No more files                    */
#undef  ETXTBSY
#define ETXTBSY       26            /*26    UNIX - not MSDOS                 */
#ifndef EDEADLOCK
#define EDEADLOCK     EDEADLK       /*36    Locking violation                */
#endif
#define ENOTBLK       43            /*43    UNIX - not MSDOS                 */
#define EUCLEAN       47            /*47    UNIX - not MSDOS                 */
#else
#error "Unknown platform. Please correct \"fstdlib.h\" for compiller you use"
#endif //MSOFT
#endif //BORLAND
#endif //SCWIN32
#endif //QNX (WATCOM ??)
/**@} DiskIO*/

//- ASSERT
#if !defined(__FP_NOT_FUNCTIONS__)
//[fstd_asrt.cpp]
extern void         _cdecl __WinAbort(LPCSTR msg,...);
//[fstd_err.cpp]
extern const char  *_cdecl __WINError(void);
//[fstd_exit.cpp]
typedef void (_cdecl *AbortProc)(void);

extern AbortProc WINAPI    AtExit(AbortProc p);
extern void      WINAPI    CallAtExit(void);
#endif

// --------------------------------------------------------------
/** @defgroup TimePeriod Time/Period measurement
    @{

    [fstd_tm.cpp]
    Procedures for get current time, calculate difference and check periods of time.
*/
#define CMP_TIME_FORMAT       "%3.3lf"
inline void          GET_TIME(TIME_TYPE& var)                 { var = timeGetTime(); }
inline CMP_TIME_TYPE CMP_TIME(TIME_TYPE evar, TIME_TYPE bvar) { return ((CMP_TIME_TYPE)evar - (CMP_TIME_TYPE)bvar) / 1000.; }

extern HANDLE   WINAPI FP_PeriodCreate(DWORD ms);
extern BOOL     WINAPI FP_PeriodEnd(HANDLE p);
extern DWORD    WINAPI FP_PeriodTime(HANDLE p);
extern void     WINAPI FP_PeriodDestroy(HANDLE& p);
extern void     WINAPI FP_PeriodReset(HANDLE p);

/** @brief Helper class for time period.
*/
struct FPPeriod
{
		HANDLE Handle;
	public:
		FPPeriod(void)          { Handle = NULL; }
		FPPeriod(DWORD ms)      { Handle = FP_PeriodCreate(ms); }
		~FPPeriod()               { if(Handle) FP_PeriodDestroy(Handle); }

		bool End(void)          { return FP_PeriodEnd(Handle)!=FALSE; }
		void Reset(void)        { FP_PeriodReset(Handle); }
		void Create(DWORD ms)   { Handle = FP_PeriodCreate(ms); }
};
/**@} Period*/

// --------------------------------------------------------------
/** @defgroup StdLib RTL extension\replacement
    @{

    Wrapers for stdlib functions and some usefull extended stdlib functions.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
//Allocators [fstd_mem.inc]
extern LPVOID WINAPI _Alloc(SIZE_T sz);
extern LPVOID WINAPI _Realloc(LPVOID ptr,SIZE_T sz);
extern void   WINAPI _Del(LPVOID ptr);
extern SIZE_T WINAPI _PtrSize(LPVOID ptr);
extern BOOL   WINAPI _HeapCheck(void);

//str
extern int     WINAPI strLen(LPCSTR str);
//string extension
inline char             ToUpperI(char c) { return (char)((c) + 'A' - 'a'); }
inline char             ToLowerI(char c) { return (char)((c) + 'a' - 'A'); }
extern char    WINAPI ToUpper(char c);
extern char    WINAPI ToLower(char c);

extern int     WINAPI StrCmp(LPCSTR str,LPCSTR str1,int maxlen = -1, BOOL isCaseSens = TRUE);
inline int              StrCmpI(LPCSTR str,LPCSTR str1)                  { return StrCmp(str,str1,-1,FALSE); }
inline int              StrNCmp(LPCSTR str,LPCSTR str1,int maxlen)       { return StrCmp(str,str1,maxlen,TRUE); }
inline int              StrNCmpI(LPCSTR str,LPCSTR str1,int maxlen)      { return StrCmp(str,str1,maxlen,FALSE); }

extern char   *WINAPI StrDup(LPCSTR m);

extern char   *WINAPI StrCpy(char *dest,LPCSTR src,int dest_sz = -1);
extern char   *WINAPI StrCat(char *dest,LPCSTR src,int dest_sz = -1);
extern int     WINAPI StrNChr(LPCSTR str,char ch,int maxlen = -1);
extern int     WINAPI strchrCount(LPCSTR str,char ch,int maxlen = -1);
extern int     WINAPI StrPosChr(LPCSTR str,char ch,int pos = 0);
extern int     WINAPI StrPosStr(LPCSTR str,LPCSTR s,int pos = 0);

#define TStrCpy( d, s ) StrCpy( d, s, sizeof(d) )
#define TStrCat( d, s ) StrCat( d, s, sizeof(d) )
#endif

// --------------------------------------------------------------
template <class T> T AtoI(LPCSTR str,T Def)
{
	if(!str || !str[0])
		return Def;

	T Sign = 1;

	if(*str == '-')
	{
		Sign = -1;
		str++;
	}
	else if(*str == '+')
		str++;

	if(!isdigit(*str))
		return Def;

	for(Def = 0; *str && isdigit(*str); str++)
	{
		Def *= 10;
		Def += (T)(*str - '0');
	}

	return Def*Sign;
}
/**@} RTL*/

//Args
#if !defined(__FP_NOT_FUNCTIONS__)
extern void     WINAPI CTArgInit(int argc, char **argv,BOOL CaseSensitive = FALSE);
extern LPSTR    WINAPI CTArgGet(int num);                     //get base argument
extern LPSTR    WINAPI CTArgGetArg(int num);                  //get`s argument that not switch
extern LPSTR    WINAPI CTArgGet(LPCSTR name);               //find "[-/]<name>[=<value>]" key and ret <value> || NULL
extern BOOL     WINAPI CTArgCheck(LPCSTR name);             //check for "[-/]<name>" exist

extern char    *WINAPI Str2Text(LPCSTR name,char *Buff,DWORD BuffSz);
extern char    *WINAPI Text2Str(LPCSTR name,char *Buff,DWORD BuffSz);

//Std columns
extern int      WINAPI StrColCount(LPCSTR str,LPCSTR seps);
extern LPCSTR WINAPI StrGetCol(LPCSTR str,int number,LPCSTR seps);

//Generate 32bit CRC for buffer.
extern DWORD    WINAPI Crc32(DWORD StartCrc/*=0*/, const BYTE *buf,DWORD len);

//[fstd_Mesg.cpp]
extern LPCSTR _cdecl Message(LPCSTR patt,...);
extern LPCSTR WINAPI    MessageV(LPCSTR patt,va_list a);

//Utils
extern LPCSTR WINAPI FCps(char *buff,double val);   // Create CPS value string (Allways 3+1+3+1 length)
extern LPSTR    WINAPI AddLastSlash(char *path, char Slash = SLASH_CHAR);
extern LPSTR    WINAPI DelLastSlash(char *path, char Slash = SLASH_CHAR);
extern LPCSTR WINAPI FPath(LPCSTR nm, char Slash = SLASH_CHAR);
extern LPCSTR WINAPI FName(LPCSTR nm, char Slash = SLASH_CHAR);
extern LPCSTR WINAPI FExtOnly(LPCSTR nm, char Slash = SLASH_CHAR);
#endif

// --------------------------------------------------------------
/** @defgroup Log Logging and debug
    @{

    [fstd_InProc.cpp]
    Helpers for logging and debug purposes.
*/
class FARINProc
{
		static  int Counter;
		LPCSTR Name;
	public:
		FARINProc(LPCSTR nm,LPCSTR s,...);
		~FARINProc();

		static void _cdecl Say(LPCSTR s,...);
};

extern int FP_LogErrorStringLength;

/** @brief Procedure callback for specify name of current log file
*/
extern void               _cdecl FP_FILELog(LPCSTR msg,...);              ///< Writes text to current log file.
extern LPCSTR           WINAPI    FP_GetLogFullFileName(void);               ///< Returns full current file log.
/**@} Log*/

#endif
