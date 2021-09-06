#ifndef __MY_DEFINES
#define __MY_DEFINES

/********************************************************************
Usefulls
  USEARG(v)                 - prevent `args nether used warning`
  EOL                       - eol string
  Type2StrX( .. )           - create string for procedure parameters
  DECL_GLOBALCALL_PROC      - declare procedure called before `main`
export/import
  __EXPORT                  - declare proc as exported
  __C_EXPORT                - declare proc as "C" exported
  __IMPORT                  - declare proc as imported
  __C_IMPORT                - declare proc as "C" exported
ptr`s
  CT_FAR,CT_NEAR,CT_HUGE    - ptr size modificators
times
  TIME_TYPE                 - type used for GET_TIME
  GET_TIME( var )           - get current TickCount
  CMP_TIME( evar,bvar )     - differs times
  CMP_TIME_TYPE              type for store diff times (double)
  CMP_TIME_FORMAT           - string for print diff times ("%3.2lf")
std
  max,min,abs,between       - std meaning
  SizeOf( Struct,Field )    - calc size of structure field
types
  WORD,DWORD,BYTE,PBYTE,
    BOOL,PVOID,pchar        - WIN-like types
  sDWORD,sWORD,sBYTE        - signed types
  MAX_DWORD                 - maximum DWORD value
  MAX_WORD                  - maximum WORD value
  SET_HI_WORD( dw,w ),
    SET_LO_WORD( dw,w ),
    HI_WORD( dw ),
    LO_WORD( dw )           - access to DWORD parts
  MK_DWORD( lw,hw )         - make DWORD from parts
  SET_HI_BYTE( w,b ),
    SET_LO_BYTE( w,b )
    HI_BYTE( w )
    LO_BYTE( w )            - access to WORD parts
  MK_WORD( lb,hb )          - make WORD from parts
  sSET_HI_WORD( dw,w ),
    sSET_LO_WORD( dw,w ),
    sHI_WORD( dw ),
    sLO_WORD( dw )
  sMK_DWORD( lw,hw )        - signed variants
bit fields
  IS_FLAG( val,flag )       - check bit field
  SET_FLAG( val,flag )      - set bit field
  CLR_FLAG( val,flag )      - clear bit field
struct/class declarations
  PRESTRUCT( cl )           - decl struct and pointer to it
  PRECLASS( cl )            - decl class and pointer to it
  STRUCT( name )            - struct header
  STRUCTBASExx( name,.. )   - struct header for derived structs
  CLASS( name )             - class header
  CLASSBASExxx( name,... )  - class header for derived classes
  TCLASS( t,name )          - template class header
  TCLASSBASExxx( t,name,..) - template class header for derived classes
 *******************************************************************/

#define WIN32_LEAN_AND_MEAN 1

//- CHECKING POSSIBILITY
#if defined(__REALDOS__) && !defined(__LARGE__)
  #error "Use MyLibrary in Large dos memory model only"
#endif

//- PLATROM DEPENDENT DEFINES
// USEARG(v) macro for prevent warning "<var> never use"
#if defined(__TEC32__) || defined(__QNX__)
  #define USEARG(v) v=v;
#else
#if defined(__REALDOS__) || defined(__BORLAND) || defined(__MSOFT) || defined(__INTEL)
  #define USEARG(v) v;
#else
#if defined(__PROTDOS__) || defined(__SCWIN32__) || defined(__DMC)
  #define USEARG(v) v=v;
#else
#if defined(__HUNIX__)
  #define USEARG(v) (void)v;
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
#endif

//EOL - default "end of line"
#if defined(__HUNIX__)
    #define EOL "\n"
#else
    #define EOL "\r\n"
#endif

//va_add( va_arg,size ) - add `size` to va_arg pointer
#if defined(__QNX__)
  #define va_add(ap,sz)     ((ap)[0]+=                                                        \
                              ((sz+sizeof(int)-1)&~(sizeof(int)-1)),                          \
                              (*(char **)((ap)[0]-((sz+sizeof(int)-1)&~(sizeof(int)-1)))))
#else
#if defined(__BORLAND)
  #define ct__size(sz)        ((sz+sizeof(int)-1) & ~(sizeof(int)-1))
  #define va_add(ap,sz)       (*(char *_FAR *)(((*(char _FAR *_FAR *)&(ap))+=(ct__size(sz)))-(ct__size(sz))))
#else
#if defined(__REALDOS__)
  #define ct__size(sz)        ((sz+sizeof(int)-1) & ~(sizeof(int)-1))
  #define va_add(ap, sz)      (*(char *_FAR *)(((*(char _FAR *_FAR *)&(ap))+=ct__size(sz))-(ct__size(sz))))
#else
#if defined(__PROTDOS__)
  #define ct__size(sz)        ((sz + __VA_ALIGN) & ~__VA_ALIGN)
  #define va_add(ap,sz)       (*(char *__SS *)(((ap)+=ct__size(sz))-(ct__size(sz))))
#else
#if defined(__SCWIN32__)
  //??
#else
#if defined(__MSOFT) || defined(__INTEL)
  //??
#else
#if defined(__DMC)
  #define ct__size(sz)        ((sz + __VA_ALIGN) & ~__VA_ALIGN)
  #define va_add(ap,sz)       (*(char *__SS *)(((ap)+=ct__size(sz))-(ct__size(sz))))
#else
#if defined(__HUNIX__)
  //??
#else
#if defined(__TEC32__)
  //??
#else
#error ERR_PLATFORM
#endif  //TEC32
#endif  //DMC
#endif  //UNIX
#endif  //MSOFT
#endif  //SCWIN
#endif  //PROTDOS
#endif  //REALDOS
#endif  //BCWIN
#endif  //QNX

#if defined(EXTERN_C)
  #undef EXTERN_C
#endif

//- DECLSPEC (exports/imports, externs )
#if defined(__HUNIX__) || defined(__TEC32__) || defined(__QNX__) || defined(__HDOS__)
  #define EXTERN                extern
  #define EXTERN_C              extern "C"
  #define DECLSPEC_EXP
  #define DECLSPEC_EXP_PT
  #define DECLSPEC_IMP
  #define DECLSPEC_IMP_PT
  #define DECLSPEC_LOCAL
  #define DECLSPEC_LOCAL_PT
  #define __fastcall
#else
#if defined(__HWIN__)    //SC, BCB, MSC
  #define EXTERN                extern
  #define EXTERN_C              extern "C"
  #define DECLSPEC_EXP_PT       __declspec(dllexport)
  #define DECLSPEC_EXP          __declspec(dllexport)
  #define DECLSPEC_IMP_PT       __declspec(dllimport)
  #define DECLSPEC_IMP          __declspec(dllimport)
  #define DECLSPEC_LOCAL_PT
  #define DECLSPEC_LOCAL
#else
#error ERR_PLATFORM
#endif
#endif

#if defined(__QNX__) || defined(__HDOS__)
  #define __thread
#elif defined(__INTEL)
  #define __thread
#endif

#if defined(__BORLAND) || defined(__MSOFT) || defined(__INTEL)
  //Has __fastcall modifier
#else
  #define __fastcall
#endif

//- PTR modifiers
#if defined(__REALDOS__)
  #define CT_FAR  __far
  #define CT_NEAR __near
  #define CT_HUGE huge
  #ifndef mode_t
    typedef short mode_t;
  #endif
#else
#if defined(__QNX__)
  #define CT_NEAR __near
  #define CT_HUGE
  #if defined(__386__)
    #define CT_FAR
  #else
    #define CT_FAR  __far
  #endif
#else
#if defined(__PROTDOS__)
  #define CT_FAR  __far
  #define CT_NEAR __near
  #define CT_HUGE
#else
#if defined(__HWIN__)
  #define CT_FAR  far
  #define CT_NEAR near
  #define CT_HUGE
#else
#if defined(__TEC32__)
  #define CT_FAR
  #define CT_NEAR
  #define CT_HUGE
#else
#if defined(__HUNIX__)
  #define CT_FAR
  #define CT_NEAR
  #define CT_HUGE
#else
  #error ERR_PLATFORM
#endif //UNIX
#endif //TEC32
#endif //HWIN
#endif //PROTDOS
#endif //QNX
#endif //REALDOS

/************************
Calling convertions
    RTL_CALLBACK     -  atexit, signal
    DLL_CALLBACK     -  function, that can be exported (WINAPI)
 ************************/
#if defined( __BCWIN32__ )
    #define RTL_CALLBACK _USERENTRY
    #define DLL_CALLBACK WINAPI
#else
#if defined( __HWIN__ )
    #define RTL_CALLBACK __cdecl
    #define DLL_CALLBACK WINAPI
#else
#if defined(__QNX__) || defined( __TEC32__ ) || defined( __HDOS__ ) || defined( __HDOS__ )
    #define RTL_CALLBACK __cdecl
    #define DLL_CALLBACK
#else
#if defined(__HUNIX__)
    #define RTL_CALLBACK __cdecl
    #define DLL_CALLBACK
#else
  #error ERR_PLATFORM
#endif //UNIX
#endif //Default
#endif //WIN32
#endif //Borland WIN32

//- Undefined types
#if !defined(__cdecl) && !defined(_MSC_VER) && !defined(__BORLANDC__) // Some does not have `__cdecl`
  #define __cdecl
#endif
#if !defined( INFINITE )
  #define INFINITE MAX_DWORD
#endif
#if !defined( INVALID_HANDLE_VALUE )
  #define INVALID_HANDLE_VALUE ((HANDLE)MAX_DWORD)
#endif

#define SizeOf( Struct,Field )  sizeof( ((Struct*)0)->Field )
#define OffsetOf(__typ,__id)    ((size_t)&(((__typ*)0)->__id))

//- TYPES
#if defined(__HUNIX__ ) || defined(__HDOS__)
    typedef unsigned char        BYTE;
    typedef BYTE                *PBYTE;
    typedef BYTE CT_FAR         *LPBYTE;
    typedef const BYTE CT_FAR   *LPCBYTE;
    typedef void                 VOID;
    typedef VOID                *PVOID;
    typedef const VOID          *PCVOID;
    typedef VOID CT_FAR         *LPVOID;
    typedef const VOID CT_FAR   *LPCVOID;
    typedef unsigned short       WORD;
    typedef WORD                *PWORD;
    typedef const WORD          *PCWORD;
    typedef WORD CT_FAR         *LPWORD;
    typedef const WORD CT_FAR   *LPCWORD;
    typedef unsigned long        DWORD;
    typedef DWORD               *PDWORD;
    typedef const DWORD         *PCDWORD;
    typedef DWORD CT_FAR        *LPDWORD;
    typedef const DWORD CT_FAR  *LPCDWORD;
    typedef DWORD                BOOL;
    typedef unsigned int         UINT;
    typedef const char          *LPCTSTR;
    #define TRUE                 ((BOOL)1)
    #define FALSE                ((BOOL)0)

    #define bool                 BOOL
    #define true                 TRUE
    #define false                FALSE

    #define HASNO_BOOL           1
#else
#if defined(__SCWIN32__)
    #define HASNO_BOOL           1
    #define bool                 BOOL
    #define true                 TRUE
    #define false                FALSE
#else
#if defined( __HWIN16__ )
    typedef const BYTE CT_FAR   *LPCBYTE;
    typedef const VOID CT_FAR   *LPCVOID;
    typedef const WORD          *PCWORD;
    typedef const WORD CT_FAR   *LPCWORD;
    typedef const DWORD         *PCDWORD;
    typedef const DWORD CT_FAR  *LPCDWORD;

    #define HASNO_BOOL           1
    #define bool                 BOOL
    #define true                 TRUE
    #define false                FALSE
#else
#if defined(__MSOFT)
    typedef const BYTE CT_FAR   *LPCBYTE;
#else
#if defined(__DMC)
    typedef const BYTE CT_FAR   *LPCBYTE;
#else
#if defined(__INTEL)
    typedef const BYTE CT_FAR   *LPCBYTE;
#else
  ;
#endif //DOS, UNIX
#endif //DMC
#endif //MSOFT
#endif //BORLAND
#endif //SCWIN
#endif //REALDOS

typedef const char *CONSTSTR;
#ifndef pchar
typedef char       *pchar;
#endif

#if defined(__WINUNICODE__)
  typedef const wchar_t *WCONSTSTR;
  typedef wchar_t       *wpchar;
#endif

#ifndef NULL
    #define NULL 0L
#endif

#if !defined(__HWIN__)
    typedef LPVOID HANDLE;
#endif

#if !defined(__BORLAND) || !defined(__VCL__)
    enum TColor { clNone = 0x7FFF, clDefault = 0xEFFFFFFFL };
#endif

typedef signed long  sDWORD;
typedef signed short sWORD;
typedef signed char  sBYTE;

#if !defined(__HUNIX__)
  typedef int   pid_t;      /* Used for process IDs & group IDs */
  typedef long  nid_t;      /* Used for network IDs         */
  typedef int   timer_t;    /* Used for timer IDs           */
#endif

#ifndef M_PI
    #define M_PI  3.14159265358979323846
#endif

#ifndef USELIB
    #define USELIB(v)
#endif
#ifndef USEUNIT
    #define USEUNIT(v)
#endif

#if !defined( ARRAY_SIZE )
  #define ARRAY_SIZE( v )  (sizeof(v) / sizeof( (v)[0] ))
#endif

#define MAX_DWORD   ((DWORD)0xFFFFFFFFUL)
#define MAX_WORD    ((WORD)0xFFFFU)
#define MAX_BYTE    ((WORD)0xFFU)

//- MK_DWORD, MK_WORD,...
#define SET_HI_WORD( dw,w ) ((((DWORD)dw)&0x0000FFFFUL) | (((DWORD)w) << 16))
#define SET_LO_WORD( dw,w ) ((((DWORD)dw)&0xFFFF0000UL) | (((WORD)w)&0xFFFFUL))

#ifndef HI_WORD
  #define HI_WORD( dw )       ((WORD)(((DWORD)(dw)) >> 16))
  #define LO_WORD( dw )       ((WORD)(((DWORD)(dw))&0x0000FFFFUL))
#endif

#define MK_DWORD( hw,lw )  (((((DWORD)hw)&0x0000FFFFUL) << 16) | (((WORD)lw)&0x0000FFFFUL))

#define SET_HI_BYTE( w,b )  ((((WORD)(w))&0x00FFU) | (((BYTE)(b)) << 8))
#define SET_LO_BYTE( w,b )  ((((WORD)(w))&0xFF00U) | ((BYTE)(b)))
#ifndef HI_BYTE
  #define HI_BYTE( w )      ( ((WORD)(w)) >> 8 )
  #define LO_BYTE( w )      ( (BYTE)((WORD)(w))&0x00FFU )
#endif
#define MK_WORD( hb,lb )   ((WORD)(((((WORD)(hb))&0x00FFU) << 8) | (((BYTE)(lb))&0x00FFU)))

#define sSET_HI_WORD( dw,w ) ((((sDWORD)(dw))&0x0000FFFFUL) | (((sDWORD)(w)) << 16))
#define sSET_LO_WORD( dw,w ) ((((sDWORD)(dw))&0xFFFF0000UL) | (((sWORD)(w))&0xFFFFUL))
#define sHI_WORD( dw )       ( (sWORD)((dw) >> 16) )
#define sLO_WORD( dw )       ( (sWORD)(dw) )
#define sMK_DWORD( hw,lw )   ((sDWORD)(((((sDWORD)(hw))&0x0000FFFFUL) << 16) | (((sWORD)(lw))&0xFFFFUL)))


#define IS_BIT( val,num )     IS_FLAG(((DWORD)(val)),1UL<<(num)))
#define IS_FLAG( val,flag )   (((val)&(flag))==(flag))
#define SET_FLAG( val,flag )  (val |= (flag))
#define CLR_FLAG( val,flag )  (val &= ~(flag))
#define SWITCH_FLAG( f,v )    do{ if (IS_FLAG(f,v)) CLR_FLAG(f,v); else SET_FLAG(f,v); }while(0)

//- MK_ID
#define MK_ID( v1,v,v3,v2 ) MK_DWORD( MK_WORD(v2,v3),MK_WORD(v,v1) )

//- Macroses
#define DECL_GLOBALCALL_PROC(nm) static int GetDummy##nm( void );                      \
                                 static int __IMP_dummy##nm = GetDummy##nm();   \
                                 static int GetDummy##nm( void )

//- Console colors
// CLR( cl,bk ) - color definition
// CLRH(cl,bk)  - hilight color definition

enum clColors {
  CL_Black     = 0,
  CL_Blue      = 1,
  CL_Green     = 2,
  CL_Cyan      = 3,
  CL_Red       = 4,
  CL_Magenta   = 5,
  CL_Yellow    = 6,
  CL_White     = 7
};

//QNX
#if defined(__QNX__)
    #if !defined( CONSOLE_ONLY )
        #define CTC_HILIGHT    TERM_HILIGHT
        #define  CLR( cl,bk )  ( ((0x80+((int)cl))*0x100) | ((0x8+((int)bk))*0x1000) )
    #else
        #define CTC_HILIGHT    0x8UL
        #define  CLR( cl,bk )  (((bk) << 4) | (cl))
    #endif
#else
#if defined(__GNUC__)
    #define CTC_HILIGHT    0x8UL
    #define  CLR( cl,bk )  (((bk) << 4) | (cl))
#else
//DOS, Win32
#if  defined(__REALDOS__) || defined(__PROTDOS__) || defined(__HWIN__)
    #define CTC_HILIGHT    0x8UL
    #define CLR( cl,bk )   (((bk) << 4) | (cl))
#else
    //Dont use
#endif //MSDOS, Win32
#endif //GNUC
#endif //QNX

#define CLRH( cl,bk )  ( CLR(cl,bk) | CTC_HILIGHT )

//- CONSOLE CHARACTERS
#if defined(__QNX__) && defined( CONSOLE_ONLY )
    #define SHADOW_CHAR                      '\xB0' //░
    #define FULL_CHAR                        '\xDB' //█
    #define VERT_CHAR                        '\xB3' //│
    #define DVERT_CHAR                       '\xBA' //║
    #define HORZ_CHAR                        '\xC4' //─
    #define DHORZ_CHAR                       '\xCD' //═
    #define CHECK_CHAR                       '\xFb' //√
    #define SBMENU_CHAR                      '\x10' //
    #define LEFT_CHAR                        '\x11' //
    #define RIGHT_CHAR                       SBMENU_CHAR
    #define SPACE_CHAR                       '\xFA' //·
    #define TAB_CHAR                         '\xFE' //■
    #define DOWN_CHAR                        '\x19' //
    #define SKIP_CHAR                        '\x20' //' '
#else
    #define SHADOW_CHAR                      '\xB0' //░
    #define FULL_CHAR                        '\xDB' //█
    #define VERT_CHAR                        '\xB3' //│
    #define DVERT_CHAR                       '\xBA' //║
    #define HORZ_CHAR                        '\xC4' //─
    #define DHORZ_CHAR                       '\xCD' //═
    #define CHECK_CHAR                       '\xFb' //√
    #define SBMENU_CHAR                      '\x10' //
    #define LEFT_CHAR                        '\x11' //
    #define RIGHT_CHAR                       SBMENU_CHAR
    #define SPACE_CHAR                       '\xFA' //·
    #define TAB_CHAR                         '\xFE' //■
    #define DOWN_CHAR                        '\x19' //
    #define SKIP_CHAR                        '\x20' //' '
#endif

//- Various
#define Type2Str0                  "void"
#define Type2Str1(t1)              #t1
#define Type2Str2(t1,t2)           #t1 "," #t2
#define Type2Str3(t1,t2,t3)        #t1 "," #t2 "," #t3
#define Type2Str4(t1,t2,t3,t4)     #t1 "," #t2 "," #t3 "," #t4

#ifndef Max
  #if defined(__cplusplus)
    template <class T> inline T Max( const T a, const T b ) { return (a>b)?a:b; }
  #else
    #define Max(a,b) (((a)>(b))?(a):(b))
  #endif
#endif
#ifndef Min
  #if defined(__cplusplus)
    template <class T> inline T Min( const T a, const T b ) { return (a<b)?a:b; }
  #else
    #define Min(a,b) (((a)<(b))?(a):(b))
  #endif
#endif
#ifndef Abs
  #if defined(__cplusplus)
    template <class T> inline T Abs( const T v ) { return (v > T(0))?v:(-v); }
  #else
    #define Abs(v) (((v)<0)?(-(v)):(v))
  #endif
#endif
#ifndef Between
  #if defined(__cplusplus)
    template <class T> inline BOOL Between( const T val, const T a, const T b ) { return (val >= a && val <= b)?TRUE:FALSE; }
  #else
    #define Between(val,a,b) (((val) >= (a) && (val) <= (b))?TRUE:FALSE)
  #endif
#endif
#ifndef Swap
  #if defined(__cplusplus)
    template <class T> void Swap( T& a,T& b ) { T tmp = a; a = b; b = tmp; }
  #else
    //??
  #endif
#endif
#ifndef Sign
  #if defined(__cplusplus)
    template <class T> int Sign( const T& a ) { return (a > T(0)) ? (1) : (-1); }
  #else
    #define Sign( a ) ( (a) > 0 ? (1) : (-1) )
  #endif
#endif

//- TIME_TYPE
#if defined(__GNUC__)
   typedef timespec              TIME_TYPE;
   typedef double                CMP_TIME_TYPE;
   #define CMP_TIME_FORMAT       "%3.3lf"
   #define GET_TIME( var )       clock_gettime( CLOCK_REALTIME,&var )
   #define CMP_TIME( e,b )       (((double)(e).tv_sec-(b).tv_sec) + ((double)((e).tv_nsec-(b).tv_nsec) / 1000000000.))
   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return (DWORD)(v*1000); }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return (DWORD)v; }
#else
#if defined(__QNX__)
   typedef timespec              TIME_TYPE;
   typedef double                CMP_TIME_TYPE;
   #define CMP_TIME_FORMAT       "%3.3lf"
   #define GET_TIME( var )       clock_gettime( CLOCK_REALTIME,&var )
   #define CMP_TIME( e,b )       (((double)(e).tv_sec-(b).tv_sec) + ((double)((e).tv_nsec-(b).tv_nsec) / 1000000000.))
   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return v*1000; }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return (DWORD)v; }
#else
#if defined(__HDOS__)
   typedef timeb                 TIME_TYPE;
   typedef DWORD                 CMP_TIME_TYPE;
   #define CMP_TIME_FORMAT       "%3.3lf"
   #define GET_TIME( var )       ftime( &var )
   #define CMP_TIME( e,b)        ((e.time-b.time)*1000+(e.millitm - b.millitm))
   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return v; }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return v/1000; }
#else
#if defined(__TEC32__)
   typedef DWORD                 TIME_TYPE;
   typedef DWORD                 CMP_TIME_TYPE;
   #define GET_TIME( var )       var = GetTickCount()
   #define CMP_TIME( evar,bvar ) (evar - bvar)

   extern DWORD GetTickCount( void );

   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return v; }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return (DWORD)v; }
#else
#if defined(__HWIN16__)
   typedef DWORD                 TIME_TYPE;
   typedef DWORD                 CMP_TIME_TYPE;
   #define GET_TIME( var )       var = GetTickCount()
   #define CMP_TIME( evar,bvar ) (evar - bvar)
   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return v; }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return (DWORD)v; }
#else
#if defined(__HWIN__)
   typedef double                CMP_TIME_TYPE;
   typedef DWORD                 TIME_TYPE;
   #define CMP_TIME_FORMAT       "%3.3lf"
#if defined(__cplusplus)
   inline BOOL   GET_TIME( TIME_TYPE& var ) { var = timeGetTime(); return TRUE; }
   inline double CMP_TIME( const TIME_TYPE& evar, const TIME_TYPE& bvar ) { return ((double)evar - (double)bvar) / 1000.; }
   inline DWORD TIME_DIFF_MS( CMP_TIME_TYPE v ) { return (DWORD)(v*1000); }
   inline DWORD TIME_DIFF_S( CMP_TIME_TYPE v )  { return (DWORD)(v); }
#else
   #define GET_TIME( var )          var = timeGetTime()
   #define CMP_TIME( evar, bvar )   (((double)evar - (double)bvar) / 1000.)
   #define TIME_DIFF_MS( v )        ((DWORD)(v*1000))
   #define TIME_DIFF_S( v )         ((DWORD)(v))
#endif
#else
  #error "Define time variables for this platform !!"
#endif //WIN32
#endif //WIN16
#endif //TEC32
#endif //DOS
#endif //QNX
#endif //UNIX

#include <Global/struct.h>    // BCB package

#if defined(__MYPACKAGE__)
  #include <Global/hdl_pkg.h>    // BCB package
#else
#if defined(__MYEXPRTL__)
  #include <Global/hdl_exp.h>    // Export ALL
#else
#if defined(__MYIMPRTL__)
  #include <Global/hdl_imp.h>    // Import ALL
#else
  #if defined(_RTLDLL) && defined(USEPACKAGES)
    #include <Global/hdl_pkg.h>  // Default packages if used by system
  #else
    #include <Global/hdl_loc.h>  // All locals
  #endif
#endif
#endif
#endif

//Function parameters
#if defined(__cplusplus)
  #define DEFAULT( v )  = v
  #define DEFNULL       = NULL
  #define DEF0          = 0
  #define DEFMAX        = MAX_DWORD
#else
  #define DEFAULT( v )
  #define DEF0
  #define DEFMAX
#endif

#endif
