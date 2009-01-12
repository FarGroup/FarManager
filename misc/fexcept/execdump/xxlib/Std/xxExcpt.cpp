#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HOOKS_USED__) && defined(__EXCEPTIONS_USED__) && !defined(__VCL__)

#pragma option -vGc- -vGt- -vGd-

#include <except.h>

//---------------------------------------------------------------------------
// BCB 5.5.1 console|WIN32 app`s
//---------------------------------------------------------------------------
//+ DATA
static LPTOP_LEVEL_EXCEPTION_FILTER OldHandler        = NULL;
static FILE                        *File              = NULL;
static DWORD                        __ExcceptionFlags = STK_FULL;
static BOOL                         InsideExcept      = FALSE;
static HVFPrintProc_t               FileProc;

static void DoAbort( void )
  {
    FILELog( "Programm terminated in cause of exception" );
    TerminateProcess( GetCurrentProcess(),0 );
}

static int idPrint( const char *PrintfFormat,... )
  {  va_list  a;
     int      sz;

     va_start( a,PrintfFormat );
       sz = FileProc( File,PrintfFormat,a );
     va_end( a );

 return sz;
}

static void UnhandledException()
  {  CONSTSTR m;

    m = GetLogFullFileName();
    if ( !m || !m[0] )
      return;

    FileProc = SetFileLogProc(NULL);
    SetFileLogProc( FileProc );
    if ( !FileProc )
      return;

    if ( m[0] == '-' && m[1] == 0 )
      File = NULL;
     else {
      File = fopen( m,"a+" );
      if ( !File )
        return;
     }

    idPrint( "\n--------------------------------------------------\n"
             "!!Unhandled exception \"%s\" at [%s]:%d\nCaller stack:\n"
             "--------------------------------------------------\n",
            __throwExceptionName,
            __throwFileName, __throwLineNumber );

    MakeStackWalkListIP( idPrint, 2, (DWORD)UnhandledException, _EBP, __ExcceptionFlags );

    if (File) {
      fclose(File);
      File = NULL;
    }
    abort();
}

static LONG WINAPI idExceptionFilter( PEXCEPTION_POINTERS ExceptionInfo )
  {
//protect agains recurse exceptions
    if ( InsideExcept ) DoAbort();
    InsideExcept = TRUE;

//Process WIN32 exceptions
    if ( isOSException(ExceptionInfo->ExceptionRecord->ExceptionCode) &&
         IS_FLAG(__ExcceptionFlags,STK_ENABLED) ) {
    //Log
      if ( IS_FLAG(__ExcceptionFlags,STK_FILELOG) )
        File = fopen( GetLogFullFileName(),"a+" );
       else
        File = NULL;
    //Info
      MakeExceptionInfo( idPrint,ExceptionInfo->ExceptionRecord,ExceptionInfo->ContextRecord );
    //Stack
      idPrint( "Caller stack:\n"
               "--------------------------------------------------\n" );
      MakeStackWalkListIP( idPrint, 0,
                           ExceptionInfo->ContextRecord->Eip, ExceptionInfo->ContextRecord->Ebp,
                           __ExcceptionFlags );
    //Close file
      if (File) {
        fclose(File);
        File = NULL;
      }
    //Terminate
      DoAbort();
    }

//Call old handler

  //If no handler - terminate
    if ( !OldHandler )
      DoAbort();

   InsideExcept = FALSE;

  //Let old handler work
 return OldHandler(ExceptionInfo);
}

DECL_HOOKPROC( LPTOP_LEVEL_EXCEPTION_FILTER, WINAPI, SetUnhandledExceptionFilter, (LPTOP_LEVEL_EXCEPTION_FILTER cb) )
  {
//Set new handler
     if ( cb && cb != idExceptionFilter ) {
       PSymbol sym;

       sym = LocateSymbol((DWORD)cb,TRUE);
       if ( sym )
         FILELog( "ExceptionFilter override to: %p at %s::\"%s\"+%d\n",
                  cb,
                  sym->Owner->Name.Text(),sym->Name,
                  ((DWORD)cb)-sym->Addr );
        else
         FILELog( "ExceptionFilter override to: %p at <have no symbols>\n",cb );

       //Swap old handler, return to caller current handler
       LPTOP_LEVEL_EXCEPTION_FILTER p = OldHandler;
       OldHandler = cb;
       return p;
     } else
//Set empty handler
     if ( !cb ) {
       OldHandler = NULL;
       return NULL;
     } else
//Reset current handler
       return OldHandler; //CALL_HOOKED(SetUnhandledExceptionFilter) (cb);
}

void MYRTLEXP SetExceptionHandlerCPP( DWORD Flags )
  {
    __ExcceptionFlags = Flags;
//Set unhandled exception filter
    std::set_unexpected( UnhandledException );
//Set Win32 exception handler
    OldHandler = SetUnhandledExceptionFilter(idExceptionFilter);
    DEF_HOOKPROC( SetUnhandledExceptionFilter );
}

#endif  //__EXCEPTIONS_USED__
