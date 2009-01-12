#include <all_lib.h>
#define HEX_INTERFACE 1
#include "ExcDump.h"

//----------------------------------------------------------------------------
static HPrintProc_t   LogProcedure;

//----------------------------------------------------------------------------
static CONSTSTR RTL_CALLBACK idLogName( void ) { return "-"; }

//----------------------------------------------------------------------------
static int RTL_CALLBACK idFileLog( FILE * /*f*/, CONSTSTR Fmt, va_list argptr )
{
    if (!LogProcedure) return 0;

    static char Buff[ 1000 ];

    VSNprintf(Buff, sizeof(Buff), Fmt, argptr);
    return LogProcedure("%s", Buff);
}

//----------------------------------------------------------------------------
HEX_DECL HPrintProc_t HEX_SPEC HEX_SetLogProc( HPrintProc_t PrintProcedure )
{
    HPrintProc_t p = LogProcedure;
    LogProcedure = PrintProcedure;
    return p;
}

//----------------------------------------------------------------------------
HEX_DECL BOOL HEX_SPEC HEX_StackWalkListAddress( HPrintProc_t PrintProcedure,
                                                 DWORD ip, DWORD bp, DWORD Flags,
                                                 DWORD SkipFrames )
{
    return MakeStackWalkListIP( PrintProcedure, SkipFrames, ip, bp, Flags );
}

//----------------------------------------------------------------------------
HEX_DECL BOOL HEX_SPEC HEX_StackWalkListContext( HPrintProc_t PrintProcedure,
                                                 CONTEXT *ctx, DWORD Flags,
                                                 DWORD SkipFrames )
{
    return MakeStackWalkListCTX( PrintProcedure, ctx, Flags, SkipFrames );
}

//----------------------------------------------------------------------------
HEX_DECL BOOL HEX_SPEC HEX_StackWalkListCurrent( HPrintProc_t PrintProcedure,
                                                 DWORD Flags,DWORD SkipFrames )
{
    return MakeStackWalkListCUR( PrintProcedure, SkipFrames, Flags );
}

//----------------------------------------------------------------------------
HEX_DECL void HEX_SPEC HEX_ExceptionInfo( HPrintProc_t PrintProcedure,
                                          PEXCEPTION_RECORD record,
                                          PCONTEXT context )
{
    MakeExceptionInfo( PrintProcedure, record, context );
}

//----------------------------------------------------------------------------
HEX_DECL void  HEX_SPEC HEX_Initialize( void )
{
     LogProcedure = NULL;
}

//----------------------------------------------------------------------------
HEX_DECL void  HEX_SPEC HEX_Cleanup( void )
{
    LogProcedure = NULL;
    CallAtExit();
}

//----------------------------------------------------------------------------
HEX_DECL PCHEX_DumpInfo HEX_SPEC HEX_QueryInterface( void )
{
    static const HEX_DumpInfo Info = {
       HEX_VERSION,
       HEX_Initialize,
       HEX_Cleanup,
       HEX_StackWalkListAddress,
       HEX_StackWalkListContext,
       HEX_StackWalkListCurrent,
       HEX_ExceptionInfo,
       HEX_SetLogProc
    };

    return &Info;
}

//----------------------------------------------------------------------------
#if defined(__BORLAND)
BOOL WINAPI DllEntryPoint( HINSTANCE hinst, DWORD reason, LPVOID ptr )
#else
#if defined(__MSOFT)
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID ptr )
#else
  #error "Define plugin DLL entry point procedure for your  compiller"
#endif
#endif
{
    if ( reason == DLL_PROCESS_ATTACH ) {
       SetFileLogProc( idFileLog );
       SetLogNameProc( idLogName );
    }
    return TRUE;
}

//----------------------------------------------------------------------------
