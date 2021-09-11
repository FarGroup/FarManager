#ifndef __FAR_PLUGINS_STD_HEADERS
#define __FAR_PLUGINS_STD_HEADERS

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

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
#include <winsock2.h>
#include <wininet.h>
#include <mmsystem.h>


#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

//- MACROSES
#define MAX_DWORD                 ((DWORD)0xFFFFFFFFUL)
#define MAX_WORD                  ((WORD)0xFFFFU)
#define MAX_BYTE                  ((WORD)0xFFU)

template <class T> inline T Max(T a,T b) { return (a>b)?a:b; }
template <class T> inline T Min(T a,T b) { return (a<b)?a:b; }
template <class T> inline T Abs(T v) { return (v>0)?v:(-v); }
template <class T> inline BOOL Between(T val,T a,T b) { return (val >= a && val <= b)?TRUE:FALSE; }
template <class T> void Swap(T& a,T& b) { T tmp = a; a = b; b = tmp; }

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

//- ASSERT
#if !defined(__FP_NOT_FUNCTIONS__)
//[fstd_asrt.cpp]
extern void         __cdecl __WinAbort(LPCSTR msg,...);
//[fstd_err.cpp]
extern const char  *__cdecl __WINError(void);
//[fstd_exit.cpp]
typedef void (__cdecl *AbortProc)(void);

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
inline void          GET_TIME(DWORD& var)                 { var = timeGetTime(); }
inline double CMP_TIME(DWORD evar, DWORD bvar) { return ((double)evar - (double)bvar) / 1000.; }

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
//string extension
inline char             ToUpperI(char c) { return (char)((c) + 'A' - 'a'); }
inline char             ToLowerI(char c) { return (char)((c) + 'a' - 'A'); }
extern char    WINAPI ToUpper(char c);
extern char    WINAPI ToLower(char c);

extern int     WINAPI StrCmp(LPCSTR str,LPCSTR str1,int maxlen = -1, BOOL isCaseSens = TRUE);
inline int              StrCmpI(LPCSTR str,LPCSTR str1)                  { return StrCmp(str,str1,-1,FALSE); }
inline int              StrNCmp(LPCSTR str,LPCSTR str1,int maxlen)       { return StrCmp(str,str1,maxlen,TRUE); }
inline int              StrNCmpI(LPCSTR str,LPCSTR str1,int maxlen)      { return StrCmp(str,str1,maxlen,FALSE); }

extern char   *WINAPI StrCpy(char *dest,LPCSTR src,int dest_sz);
extern char   *WINAPI StrCat(char *dest,LPCSTR src,int dest_sz);
extern int     WINAPI StrPosChr(LPCSTR str,char ch,int pos = 0);
extern int     WINAPI StrPosStr(LPCSTR str,LPCSTR s,int pos = 0);

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
extern LPCSTR __cdecl Message(LPCSTR patt,...);
extern LPCSTR WINAPI    MessageV(LPCSTR patt,va_list a);

//Utils
extern LPCSTR WINAPI FCps(char *buff,double val);   // Create CPS value string (Allways 3+1+3+1 length)
extern LPSTR    WINAPI AddLastSlash(char *path, char Slash = '\\');
extern LPSTR    WINAPI DelLastSlash(char *path, char Slash = '\\');
extern LPCSTR WINAPI FPath(LPCSTR nm, char Slash = '\\');
extern LPCSTR WINAPI FName(LPCSTR nm, char Slash = '\\');
extern LPCSTR WINAPI FExtOnly(LPCSTR nm, char Slash = '\\');
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

		static void __cdecl Say(LPCSTR s,...);
};

extern int FP_LogErrorStringLength;

/** @brief Procedure callback for specify name of current log file
*/
extern void               __cdecl FP_FILELog(LPCSTR msg,...);              ///< Writes text to current log file.
extern LPCSTR           WINAPI    FP_GetLogFullFileName(void);               ///< Returns full current file log.
/**@} Log*/

#endif
