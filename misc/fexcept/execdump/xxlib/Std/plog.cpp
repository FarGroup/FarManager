#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__SCWIN32__)
extern time_t time( time_t * );
#endif

/**********************************************************************
 PUBLIC DATA
 **********************************************************************/
BOOL FlushLOGFile      = TRUE;

/**********************************************************************
 LOCAL DATA
 **********************************************************************/
static CONSTSTR RTL_CALLBACK PLOGMkFile( void );
static int      RTL_CALLBACK FILELogFPrintfV( FILE *f, CONSTSTR Fmt,va_list argptr );

static CRITICAL_SECTION PLOG_cs;
static HGetStringProc_t PLOGGetLogFileName = PLOGMkFile;
static HVFPrintProc_t   PLOGFileProc       = FILELogFPrintfV;
static char             Buff[ 512 ];
static FILE            *OutFile = NULL;
static char             header_str[ 512 ];

/**********************************************************************
 LOCAL PROCS
 **********************************************************************/
static CONSTSTR RTL_CALLBACK PLOGMkFile( void )
  {
 return "PLog.log";
}

static int RTL_CALLBACK FILELogFPrintfV( FILE *f, CONSTSTR Fmt,va_list argptr )
  {
 return f ? vfprintf( f, Fmt, argptr ) : 0;
}

static BOOL LOGInit( void )
  {  static BOOL inited = FALSE;

  if (inited) return FALSE;
  InitializeCriticalSection(&PLOG_cs);
  inited=TRUE;
#if defined(__QNX__)
  setbuf( stdout,NULL );
#endif
 return TRUE;
}

static int RTL_CALLBACK tprintf( CONSTSTR Fmt,... )
  {
      if ( !PLOGFileProc )
        return 0;

      while( *Fmt == '\n' || *Fmt == '\r' ) Fmt++;

      if ( !Fmt[0] )
        return 0;

      //User message
      va_list argptr;
      int     rc;
      va_start(argptr,Fmt);
       if ( OutFile )
         fprintf( OutFile, "%s: ", header_str );
       rc = PLOGFileProc( OutFile, Fmt, argptr );
      va_end(argptr);

 return rc;
}

/**********************************************************************
 NAMES
 **********************************************************************/
HGetStringProc_t MYRTLEXP SetLogNameProc( HGetStringProc_t p )
  {  HGetStringProc_t o = PLOGGetLogFileName;
     PLOGGetLogFileName = p;
 return o;
}

CONSTSTR MYRTLEXP GetLogFullFileName( void )
  {  static char str[MAX_PATH_SIZE] = "";
     CONSTSTR m;

     if (!PLOGGetLogFileName)
       return "";

     m = PLOGGetLogFileName();
     if (!m || !m[0])
       return "";

     //Dummy
     if ( m[0] == '-' && m[1] == 0 ) {
       str[0] = '-';
       str[1] = 0;
     } else
     //Absolute
     if ( IsAbsolutePath(m) )
       StrCpy( str,m,sizeof(str) );
      else {
     //Relative
       CONSTSTR tmp = GetStartupDir();
       if ( !tmp[0] )
         tmp = GetCurDir();
       SNprintf( str,sizeof(str),"%s%s",tmp,m );
      }

 return str;
}

/**********************************************************************
 FILES
 **********************************************************************/
HVFPrintProc_t MYRTLEXP SetFileLogProc( HVFPrintProc_t p )
  {  HVFPrintProc_t o = PLOGFileProc;
     PLOGFileProc = p;
 return o;
}

int MYRTLEXP_PT FILELog( CONSTSTR msg,... )
  {  va_list argptr;
     int     rc;

     if ( !msg || !msg[0] ) return 0;

     va_start( argptr, msg );
       rc = FILELogFileV( GetLogFullFileName(),msg,argptr );
     va_end( argptr );

 return rc;
}

int MYRTLEXP  FILELogV( CONSTSTR msg,va_list a )
  {
 return FILELogFileV( GetLogFullFileName(), msg, a );
}

int FILELogFile( CONSTSTR File,CONSTSTR msg,... )
  {  va_list argptr;
     int     rc;

     if ( !msg || !msg[0] ) return 0;

     va_start( argptr, msg );
       rc = FILELogFileV( File,msg,argptr );
     va_end( argptr );

 return rc;
}

int MYRTLEXP FILELogFileV( CONSTSTR File,CONSTSTR msg,va_list argptr )
  {  BOOL    first;
     DWORD   err = FIO_ERRORN;
     int     rc = 0;
     char   *m;
     int     sz, nsz;

     if ( !msg )
       return 0;

     first = LOGInit();

     if ( first )
       Buff[0] = 0;

     EnterCriticalSection(&PLOG_cs);
  do{
     m = (char*)File;
     if (!m || !m[0]) break;

     if ( m[0] == '-' && m[1] == 0 )
       OutFile = NULL;
      else
     if ( !OutFile ) {
       OutFile = fopen( m,first?"w":"a" );
       if ( !OutFile ) break;
       if ( first )
         fprintf( OutFile,"DATE TIME / HINST / ERROR : <MESSAGE>\n" );
     }

     m  = header_str;
     sz = ARRAY_SIZE(header_str);
    //Time
#if defined(__HWIN32__)
     SYSTEMTIME st;

     GetLocalTime( &st );
     nsz = SNprintf( m, sz, " %02d-%02d-%4d %02d:%02d:%02d:%04d",
                    st.wDay, st.wMonth, st.wYear,
                    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
     sz -= nsz; m += nsz;
#else
     time_t  tm_t;
     tm     *tm;
     time(&tm_t);
     tm = localtime(&tm_t);
     nsz = Sprintf( m, " %02d-%02d-%4d %02d:%02d:%02d",
                   tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
                   tm->tm_hour, tm->tm_min, tm->tm_sec );
     sz -= nsz; m += nsz;
#endif

   //ID
     nsz = SNprintf( m,sz," %6d",GetHInstance() );
     sz -= nsz; m += nsz;

#if defined(__HWIN__)
     nsz = SNprintf( m,sz,"_%08X",GetCurrentThreadId() );
     sz -= nsz; m += nsz;
#endif

    //Error
     SNprintf( m,sz," %06d",err );

    //Message
     rc = BufferedPrintfV( Buff,sizeof(Buff),tprintf,msg,argptr );
     if ( Buff[0] )
       BufferedPrintf( Buff,sizeof(Buff),tprintf,"\n" );

  }while(0);

  if (OutFile && FlushLOGFile) {
    fclose(OutFile);
    OutFile = NULL;
  }

  LeaveCriticalSection(&PLOG_cs);
  FIO_SETERRORN( err );

 return rc;
}
