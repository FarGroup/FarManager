#ifndef __MY_H_EXCEPTION_DUMP
#define __MY_H_EXCEPTION_DUMP

/** @defgroup Main ExcDump
    @{
*/
/** @mainpage ExcDump library

    @section Usage Using library

    You may use library in two ways:
      - Statically link your programm with ExcDump library.
      - Dynamically load library at moment you need it functions.

    @note In case you use dynamic loading \b you \b must call HEX_Cleanup() function manually before unload library DLL.
    @note In case you use HEX_DumpInfo structure to access library functions you should \b note that library uses
          \b 1-byte alignment structures.

    @section Init  Initialization and cleanup library

    You may or may not manual initialize library, but you must manual call cleanup procedure if
    you dynamically load library.

    \t \br Name             \bc Description \er
      \ir HEX_Initialize()   \c Initialize library \er
      \ir HEX_Cleanup()      \c Cleanup library \er
    \et

    @section Functions Library functions
    \t \br Name                      \bc Description \er
      \ir HEX_SetLogProc()            \c Set user output procedure for library internal logs \er
      \ir HEX_StackWalkListAddress()  \c Stack walk for address \er
      \ir HEX_StackWalkListContext()  \c Stack walk for context \er
      \ir HEX_StackWalkListCurrent()  \c Stack walk for current address \er
      \ir HEX_ExceptionInfo()         \c Output detailed exception information \er
    \et

    @section Types Library types
    \t \br Name        \bc Description \er
      \ir HPrintProc_t              \c Type for user output callback fully compatible with API printf function prototype. \er
      \ir HT_SetLogProc_t           \c Type for HEX_SetLogProc() function. \er
      \ir HT_StackWalkListAddress_t \c Type for HEX_StackWalkListAddress() function. \er
      \ir HT_StackWalkListContext_t \c Type for HEX_StackWalkListContext() function. \er
      \ir HT_StackWalkListCurrent_t \c Type for HEX_StackWalkListCurrent() function. \er
      \ir HT_ExceptionInfo_t        \c Type for HEX_ExceptionInfo() function. \er
      \ir HT_Initialize_t           \c Type for HEX_Initialize() function. \er
      \ir HT_Cleanup_t              \c Type for HEX_Cleanup() function. \er
      \ir HT_QueryInterface_t       \c Type for HEX_QueryInterface() function. \er
    \et

    @section Structures Library structures
    \t \br Name        \bc Description \er
      \ir HEX_DumpInfo  \c Structure used to hold whole set of library functions. \er
    \et

    @section STK_xxx
    @section StackFlags walk flags
    \t \br Name                \bc Description \er
      \ir STK_READCODE        \c Read code around each stack frame \er
      \ir STK_LOADEXPORTS     \c Load export symbols for every module in stack items list \er
      \ir STK_LOADLOCALS      \c Load MAP symbols for every module in stack items list \er
      \ir STK_ENABLED         \c Exception filter enabled \er
      \ir STK_FILELOG         \c Write exception info to log file \er
      \ir STK_DISASM          \c Disassembly stack frame codes \er
      \ir STK_COMMENTCHAR     \c Add DASM_SYM_COMMENT char at start of comment \er
      \ir STK_COMMENTIRECTION \c Add DASM_SYM_UP\DOWN chars in comment \er
      \ir STK_COMMENTSYMBOL   \c Add symbol information to comments \er
      \ir STK_COMMENTDUMP     \c Add data dump to comments \er
      \ir STK_STACKDATA       \c Read and draw stack data for every procedure \er
      \ir STK_STACKSYSTEM     \c Show stack-walk for system modules \er
      \ir STK_LOADIMPORTS     \c Load imports symbols \er
    \et

    @section StackDef walk default values
    \t \br Name                \bc Flags set \bc Description \er
      \ir STK_LOADSYMBOLS       \c STK_LOADIMPORTS, STK_LOADEXPORTS, STK_LOADLOCALS \c
                                   Set of flags you may use to load all available symbols found by library. \er
      \ir STK_FULL              \c STK_ENABLED, STK_FILELOG, STK_LOADSYMBOLS, STK_READCODE, STK_DISASM,
                                   STK_COMMENTCHAR, STK_COMMENTIRECTION, STK_COMMENTSYMBOL, STK_COMMENTDUMP, STK_STACKDATA \c
                                   Full set of stack-walk flags. \er
      \ir STK_DEFAULT           \c STK_FULL \c
                                   Default set of flags. \er
    \et
*/

#if defined(__BORLANDC__)
  #pragma nopackwarning
  #pragma pack(push,1);
#else
#if defined(__SC__) || defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100))
  #pragma pack(1)
#else
#if defined(_MSC_VER)
  #pragma pack(push,1)
#else
  #pragma pack(__push,1);
#endif
#endif
#endif

#if defined(HEX_INTERFACE)
  #define HEX_DECL extern "C"
  #define HEX_SPEC __declspec(dllexport) WINAPI
#else
  #define HEX_DECL extern "C"
  #define HEX_SPEC WINAPI

  #if !defined(__STD_LIB_ALL)
    #if defined( __BORLANDC__ )
        #define RTL_CALLBACK _USERENTRY
    #else
        #define RTL_CALLBACK   _cdecl
    #endif

    /** @brief User write callback
        Callback used be HEX procedures to output data. \n
        The type is designed to be fully compatible with standard C RTL printf function.
    */
    typedef int (RTL_CALLBACK *HPrintProc_t)( const char *PrintfFormat,... );

    #define STK_READCODE         0x00000001ul  ///< Read code around each stack frame
    #define STK_LOADEXPORTS      0x00000002ul  ///< Load export symbols for every module in stack items list
    #define STK_LOADLOCALS       0x00000004ul  ///< Load MAP symbols for every module in stack items list
    #define STK_ENABLED          0x00000008ul  ///< Exception filter enabled
    #define STK_FILELOG          0x00000010ul  ///< Write exception info to log file
    #define STK_DISASM           0x00000020ul  ///< Disassembly stack frame codes
    #define STK_COMMENTCHAR      0x00000040ul  ///< Add DASM_SYM_COMMENT char at start of comment
    #define STK_COMMENTIRECTION  0x00000080ul  ///< Add DASM_SYM_UP\DOWN chars in comment
    #define STK_COMMENTSYMBOL    0x00000100ul  ///< Add symbol information to comments
    #define STK_COMMENTDUMP      0x00000200ul  ///< Add data dump to comments
    #define STK_STACKDATA        0x00000400ul  ///< Read and draw stack data for every procedure
    #define STK_STACKSYSTEM      0x00000800ul  ///< Show stack-walk for system modules
    #define STK_LOADIMPORTS      0x00001000ul  ///< Load imports symbols

    #define STK_LOADSYMBOLS      (STK_LOADIMPORTS|STK_LOADEXPORTS|STK_LOADLOCALS)
    #define STK_FULL             (STK_ENABLED | STK_FILELOG |                   \
                                  STK_LOADSYMBOLS | STK_READCODE | STK_DISASM | \
                                  STK_COMMENTCHAR | STK_COMMENTIRECTION |       \
                                  STK_COMMENTSYMBOL | STK_COMMENTDUMP | STK_STACKDATA)
    #define STK_DEFAULT          STK_FULL
  #endif
#endif

typedef struct HEX_DumpInfo *PHEX_DumpInfo;

typedef HPrintProc_t  (WINAPI *HT_SetLogProc_t)( HPrintProc_t PrintProcedure );
typedef BOOL          (WINAPI *HT_StackWalkListAddress_t)( HPrintProc_t PrintProcedure, DWORD ip, DWORD bp, DWORD Flags, DWORD SkipFrames );
typedef BOOL          (WINAPI *HT_StackWalkListContext_t)( HPrintProc_t PrintProcedure, CONTEXT *ctx, DWORD Flags, DWORD SkipFrames );
typedef BOOL          (WINAPI *HT_StackWalkListCurrent_t)( HPrintProc_t PrintProcedure, DWORD Flags, DWORD SkipFrames );
typedef void          (WINAPI *HT_ExceptionInfo_t)( HPrintProc_t PrintProcedure, PEXCEPTION_RECORD record, PCONTEXT context );
typedef void          (WINAPI *HT_Initialize_t)( void );
typedef void          (WINAPI *HT_Cleanup_t)( void );
typedef PHEX_DumpInfo (WINAPI *HT_QueryInterface_t)( void );

//----------------------------------------
//  Init \ Cleanup
//----------------------------------------
/** @brief Initializes HEX library.

    You may use this function to manually initialize library but it is not necessary.
    All library functions automatically initialize library on first call. \n
    Some library function may take a long time at first run, so you may use this function to initialize
    library once and speed up all function calls.
*/
HEX_DECL void          HEX_SPEC HEX_Initialize( void );

/** @brief Clean library

    Destroy all objects and free all memory used by library. \n
    You must call this function before unload library DLL in case you dynamically load it.
*/
HEX_DECL void          HEX_SPEC HEX_Cleanup( void );

//----------------------------------------
//  Create a human-readable view of unwind stack
//----------------------------------------
/** @brief Set callback used by library to output log information
    @param  PrintProcedure User printf-like callback
    @return Previously log procedure

    Set callback procedure to be used for output HEX library log information. \n
    HEX library will write information about loaded modules used in stack walk process and informations
    about symbols read; \n
    Set log procedure to NULL to disable library logging.
*/
HEX_DECL HPrintProc_t  HEX_SPEC HEX_SetLogProc( HPrintProc_t PrintProcedure );

/** @brief Stack-walk for specified address
    @param PrintProcedure   User callback used to output stack walk text information;
    @param ip               Value for EIP code addtess;
    @param bp               Value for EBP code address;
    @param Flags            Set of STK_xxx flags.
    @param SkipFrames       Number of frames to skip before output information. \n
                            Use this paramater if you use set of nested wrapers around real place of
                            interest to skip wrapers frames.
    @return TRUE if stack walk completed successfully.

    Use this procedure if you have dirrect address to stack walk.
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListAddress( HPrintProc_t PrintProcedure,
                                                          DWORD ip, DWORD bp, DWORD Flags, DWORD SkipFrames );

/** @brief Stack-walk for context record
    @param PrintProcedure   User callback used to output stack walk text information;
    @param ctx              Win API CONTEXT stucture contains information about place of interest. \n
                            Only EIP and EBP fields are used.
    @param Flags            Set of STK_xxx flags.
    @param SkipFrames       Number of frames to skip before output information. \n
                            Use this paramater if you use set of nested wrapers around real place of
                            interest to skip wrapers frames.
    @return TRUE if stack walk completed successfully.

    Use this procedure if you have CONTEXT record available.
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListContext( HPrintProc_t PrintProcedure,
                                                          CONTEXT *ctx, DWORD Flags, DWORD SkipFrames );

/** @brief Stack-walk for current execution point
    @param PrintProcedure   User callback used to output stack walk text information;
    @param Flags            Set of STK_xxx flags.
    @param SkipFrames       Number of frames to skip before output information. \n
                            Use this paramater if you use set of nested wrapers around real place of
                            interest to skip wrapers frames.
    @return TRUE if stack walk completed successfully.

    Use this procedure if you need to stack walking for current execution point. \n
    This procedure very usefull to create assertion macro looking like:
    \code
 #define Assert( p ) do{ if (!(p)) { \
                           printf( "Assertion...\nConditin: \"%s\"\nAt file: \"%s:%d\"", #p , __FILE__ , __LINE__ ); \
                           HEX_StackWalkListCurrent( printf, STK_DEFAULT, 0 ); \
                         } \
                     }while(0)
    \endcode
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListCurrent( HPrintProc_t PrintProcedure,DWORD Flags,DWORD SkipFrames );

/** @brief Creates detailed exception description
    @param PrintProcedure User callback used to output information.
    @param record         Optional exception record structure.
    @param context        Optional xeception context structure.

    Function output detailed information on execption. \n
    Both structure painters may be optional and must be set to NULL if not interested.
    Information will be printed for specified structures only.
*/
HEX_DECL void          HEX_SPEC HEX_ExceptionInfo( HPrintProc_t PrintProcedure,
                                                   PEXCEPTION_RECORD record /*=NULL*/, PCONTEXT context /*=NULL*/ );

//----------------------------------------
//  Interface
//----------------------------------------
#define HEX_VERSION         0x08072004

/** @brief HEX Interface structure.

    Contains pointers to all library functions. \n
    You can use this structure to easy access to library functionss on dynamic library load.
*/
struct HEX_DumpInfo {
  DWORD                     Version;              ///< Allways contains HEX_VERSION
  HT_Initialize_t           Initialize;           ///< Addres of HEX_Initialize() function.
  HT_Cleanup_t              Cleanup;              ///< Addres of HEX_Cleanup() function.
  HT_StackWalkListAddress_t StackWalkListAddress; ///< Addres of HEX_StackWalkListAddress() function.
  HT_StackWalkListContext_t StackWalkListContext; ///< Addres of HEX_StackWalkListContext() function.
  HT_StackWalkListCurrent_t StackWalkListCurrent; ///< Addres of HEX_StackWalkListCurrent() function.
  HT_ExceptionInfo_t        ExceptionInfo;        ///< Addres of HEX_ExceptionInfo() function.
  HT_SetLogProc_t           SetLogProc;           ///< Addres of HEX_SetLogProc() function.
};

/** @brief Interface exported function

    You may use this function to retrive whole HEX interface in one call. \n
    This function usefull if you not link library statically, but load it dynamically at runtime.
*/
HEX_DECL PHEX_DumpInfo HEX_SPEC HEX_QueryInterface( void );

#if defined(__BORLANDC__)
  #pragma nopackwarning
  #pragma pack(pop);
#else
#if defined(__SC__) || defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100))
  #pragma pack()
#else
#if defined(_MSC_VER)
  #pragma pack(pop)
#else
  #pragma pack(__pop)
#endif
#endif
#endif

/**@}*/
#endif