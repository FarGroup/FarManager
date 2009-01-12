#ifndef __EXCDUMP_H__
#define __EXCDUMP_H__

/*
    Using library

    You may use library in two ways:
      - Statically link your programm with ExcDump library.
      - Dynamically load library at moment you need it functions.

    note: In case you use dynamic loading you must call HEX_Cleanup() function 
          manually before unload library DLL.
    note: In case you use HEX_DumpInfo structure to access library functions 
          you should \b note that library uses 1-byte alignment structures.

    Init  Initialization and cleanup library

    You may or may not manual initialize library, but you must manual call 
    cleanup procedure if you dynamically load library.

    HEX_Initialize()              Initialize library
    HEX_Cleanup()                 Cleanup library

    HEX_SetLogProc()              Set user output procedure for library 
                                  internal logs
    HEX_StackWalkListAddress()    Stack walk for address
    HEX_StackWalkListContext()    Stack walk for context
    HEX_StackWalkListCurrent()    Stack walk for current address
    HEX_ExceptionInfo()           Output detailed exception information
    

    HPrintProc_t                 Type for user output callback fully 
                                 compatible with API printf function prototype.
    HT_SetLogProc_t              Type for HEX_SetLogProc() function.
    HT_StackWalkListAddress_t    Type for HEX_StackWalkListAddress() function.
    HT_StackWalkListContext_t    Type for HEX_StackWalkListContext() function.
    HT_StackWalkListCurrent_t    Type for HEX_StackWalkListCurrent() function.
    HT_ExceptionInfo_t           Type for HEX_ExceptionInfo() function.
    HT_Initialize_t              Type for HEX_Initialize() function.
    HT_Cleanup_t                 Type for HEX_Cleanup() function.
    HT_QueryInterface_t          Type for HEX_QueryInterface() function.
    

    HEX_DumpInfo                 Structure used to hold whole set of library 
                                 functions.

    StackFlags walk flags:

    STK_READCODE         Read code around each stack frame
    STK_LOADEXPORTS      Load export symbols for every module in stack items list
    STK_LOADLOCALS       Load MAP symbols for every module in stack items list
    STK_ENABLED          Exception filter enabled
    STK_FILELOG          Write exception info to log file
    STK_DISASM           Disassembly stack frame codes
    STK_COMMENTCHAR      Add DASM_SYM_COMMENT char at start of comment
    STK_COMMENTIRECTION  Add DASM_SYM_UP\DOWN chars in comment
    STK_COMMENTSYMBOL    Add symbol information to comments
    STK_COMMENTDUMP      Add data dump to comments
    STK_STACKDATA        Read and draw stack data for every procedure
    STK_STACKSYSTEM      Show stack-walk for system modules
    STK_LOADIMPORTS      Load imports symbols
    STK_EACHDETAILS      Print detailed info for each stack entry                 Y         Y

    StackDef walk default values

    STK_LOADSYMBOLS      Set of flags you may use to load all available symbols
                         found by library.
    STK_FULL             Full set of stack-walk flags.
    STK_DEFAULT          Default set of flags.
*/

#pragma pack(1)

#ifdef __cplusplus
  #define HEX_DECL extern "C"
#else
  #define HEX_DECL
#endif

#if defined(__BORLANDC__) && defined(HEX_INTERFACE)
  #define HEX_SPEC __declspec(dllexport) WINAPI
#else
  #define HEX_SPEC WINAPI
#endif

#ifndef HEX_INTERFACE
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

    #define STK_READCODE         0x00000001ul  // Read code around each stack frame
    #define STK_LOADEXPORTS      0x00000002ul  // Load export symbols for every module in stack items list
    #define STK_LOADLOCALS       0x00000004ul  // Load MAP symbols for every module in stack items list
    #define STK_ENABLED          0x00000008ul  // Exception filter enabled
    #define STK_FILELOG          0x00000010ul  // Write exception info to log file
    #define STK_DISASM           0x00000020ul  // Disassembly stack frame codes
    #define STK_COMMENTCHAR      0x00000040ul  // Add DASM_SYM_COMMENT char at start of comment
    #define STK_COMMENTIRECTION  0x00000080ul  // Add DASM_SYM_UP\DOWN chars in comment
    #define STK_COMMENTSYMBOL    0x00000100ul  // Add symbol information to comments
    #define STK_COMMENTDUMP      0x00000200ul  // Add data dump to comments
    #define STK_STACKDATA        0x00000400ul  // Read and draw stack data for every procedure
    #define STK_STACKSYSTEM      0x00000800ul  // Show stack-walk for system modules
    #define STK_LOADIMPORTS      0x00001000ul  // Load imports symbols
    #define STK_EACHDETAILS      0x00002000ul  // Print detailed info for each stack entry

    #define STK_LOADSYMBOLS      (STK_LOADIMPORTS|STK_LOADEXPORTS|STK_LOADLOCALS)
    #define STK_FULL             (STK_ENABLED | STK_FILELOG |                   \
                                  STK_LOADSYMBOLS | STK_READCODE | STK_DISASM | \
                                  STK_COMMENTCHAR | STK_COMMENTIRECTION |       \
                                  STK_COMMENTSYMBOL | STK_COMMENTDUMP | STK_STACKDATA | \
                                  STK_EACHDETAILS)
    #define STK_DEFAULT          STK_FULL
  #endif
#endif

typedef const struct HEX_DumpInfo *PCHEX_DumpInfo;

typedef HPrintProc_t   (WINAPI *HT_SetLogProc_t)( HPrintProc_t PrintProcedure );
typedef BOOL           (WINAPI *HT_StackWalkListAddress_t)( HPrintProc_t PrintProcedure, DWORD ip, DWORD bp, DWORD Flags, DWORD SkipFrames );
typedef BOOL           (WINAPI *HT_StackWalkListContext_t)( HPrintProc_t PrintProcedure, CONTEXT *ctx, DWORD Flags, DWORD SkipFrames );
typedef BOOL           (WINAPI *HT_StackWalkListCurrent_t)( HPrintProc_t PrintProcedure, DWORD Flags, DWORD SkipFrames );
typedef void           (WINAPI *HT_ExceptionInfo_t)( HPrintProc_t PrintProcedure, PEXCEPTION_RECORD record, PCONTEXT context );
typedef void           (WINAPI *HT_Initialize_t)( void );
typedef void           (WINAPI *HT_Cleanup_t)( void );
typedef PCHEX_DumpInfo (WINAPI *HT_QueryInterface_t)( void );

//----------------------------------------
//  Init \ Cleanup
//----------------------------------------
/* Initializes HEX library.

    You may use this function to manually initialize library but it is not 
    necessary.
    All library functions automatically initialize library on first call.
    Some library function may take a long time at first run, so you may use 
    this function to initialize library once and speed up all function calls.
*/
HEX_DECL void          HEX_SPEC HEX_Initialize( void );

/** Clean library

    Destroy all objects and free all memory used by library.
    You must call this function before unload library DLL in case you 
    dynamically load it.
*/
HEX_DECL void          HEX_SPEC HEX_Cleanup( void );

//----------------------------------------
//  Create a human-readable view of unwind stack
//----------------------------------------
/** Set callback used by library to output log information
    param:  PrintProcedure User printf-like callback
    return: Previously log procedure

    Set callback procedure to be used for output HEX library log information.
    HEX library will write information about loaded modules used in stack walk 
    process and informations about symbols read;
    Set log procedure to NULL to disable library logging.
*/
HEX_DECL HPrintProc_t  HEX_SPEC HEX_SetLogProc( HPrintProc_t PrintProcedure );

/** Stack-walk for specified address
    param: PrintProcedure   User callback used to output stack walk text 
                            information;
    param: ip               Value for EIP code addtess;
    param: bp               Value for EBP code address;
    param: Flags            Set of STK_xxx flags.
    param: SkipFrames       Number of frames to skip before output information.
                            Use this paramater if you use set of nested wrapers
                            around real place of interest to skip wrapers 
                            frames.
    return: TRUE if stack walk completed successfully.

    Use this procedure if you have dirrect address to stack walk.
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListAddress( HPrintProc_t PrintProcedure,
                                                          DWORD ip, DWORD bp, DWORD Flags, DWORD SkipFrames );

/** Stack-walk for context record
    param: PrintProcedure   User callback used to output stack walk text 
                            information;
    param: ctx              Win API CONTEXT stucture contains information about
                            place of interest.
                            Only EIP, EBP and ESP fields are used.
    param: Flags            Set of STK_xxx flags.
    param: SkipFrames       Number of frames to skip before output information.
                            Use this paramater if you use set of nested wrapers
                            around real place of interest to skip wrapers
                            frames.
    return: TRUE if stack walk completed successfully.

    Use this procedure if you have CONTEXT record available.
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListContext( HPrintProc_t PrintProcedure,
                                                          CONTEXT *ctx, DWORD Flags, DWORD SkipFrames );

/** Stack-walk for current execution point
    param: PrintProcedure   User callback used to output stack walk text 
                            information;
    param: Flags            Set of STK_xxx flags.
    param: SkipFrames       Number of frames to skip before output information.
                            Use this paramater if you use set of nested wrapers
                            around real place of interest to skip wrapers 
                            frames.
    return: TRUE if stack walk completed successfully.

    Use this procedure if you need to stack walking for current execution point. \n
    This procedure very usefull to create assertion macro looking like:

 #define Assert( p ) do{ if (!(p)) { \
                           printf( "Assertion...\nConditin: \"%s\"\nAt file: \"%s:%d\"", #p , __FILE__ , __LINE__ ); \
                           HEX_StackWalkListCurrent( printf, STK_DEFAULT, 0 ); \
                         } \
                     }while(0)
*/
HEX_DECL BOOL          HEX_SPEC HEX_StackWalkListCurrent( HPrintProc_t PrintProcedure,DWORD Flags,DWORD SkipFrames );

/** Creates detailed exception description
    param: PrintProcedure User callback used to output information.
    param: record         Optional exception record structure.
    param: context        Optional xeception context structure.

    Function output detailed information on execption.
    Both structure painters may be optional and must be set to NULL if not 
    interested.
    Information will be printed for specified structures only.
*/
HEX_DECL void          HEX_SPEC HEX_ExceptionInfo( HPrintProc_t PrintProcedure,
                                                   PEXCEPTION_RECORD record /*=NULL*/, PCONTEXT context /*=NULL*/ );

//----------------------------------------
//  Interface
//----------------------------------------
#define HEX_VERSION         0x08072004

/** HEX Interface structure.

    Contains pointers to all library functions.
    You can use this structure to easy access to library functionss on 
    dynamic library load.
*/
struct HEX_DumpInfo {
  DWORD                     Version;              // Allways contains HEX_VERSION
  HT_Initialize_t           Initialize;           // Addres of HEX_Initialize() function.
  HT_Cleanup_t              Cleanup;              // Addres of HEX_Cleanup() function.
  HT_StackWalkListAddress_t StackWalkListAddress; // Addres of HEX_StackWalkListAddress() function.
  HT_StackWalkListContext_t StackWalkListContext; // Addres of HEX_StackWalkListContext() function.
  HT_StackWalkListCurrent_t StackWalkListCurrent; // Addres of HEX_StackWalkListCurrent() function.
  HT_ExceptionInfo_t        ExceptionInfo;        // Addres of HEX_ExceptionInfo() function.
  HT_SetLogProc_t           SetLogProc;           // Addres of HEX_SetLogProc() function.
};

/** Interface exported function

    You may use this function to retrive whole HEX interface in one call.
    This function usefull if you not link library statically, but load it 
    dynamically at runtime.
*/
HEX_DECL PCHEX_DumpInfo HEX_SPEC HEX_QueryInterface( void );

#pragma pack()

#endif
