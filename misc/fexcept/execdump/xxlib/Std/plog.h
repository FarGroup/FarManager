#ifndef __MY_LOGPROCS
#define __MY_LOGPROCS

#if !defined(__TEC32__)
HDECLSPEC CONSTSTR         MYRTLEXP    GetLogFullFileName( void );
HDECLSPEC HGetStringProc_t MYRTLEXP    SetLogNameProc( HGetStringProc_t p );
#endif

#if !defined(__TEC32__)
HDECLSPEC int              MYRTLEXP_PT FILELog( CONSTSTR msg,... );
HDECLSPEC int              MYRTLEXP    FILELogV( CONSTSTR msg,va_list a );
HDECLSPEC int              MYRTLEXP_PT FILELogFile( CONSTSTR File,CONSTSTR msg,... );
HDECLSPEC int              MYRTLEXP    FILELogFileV( CONSTSTR File,CONSTSTR msg,va_list a );
HDECLSPEC HVFPrintProc_t   MYRTLEXP    SetFileLogProc( HVFPrintProc_t UserFunction );
#else
  #define FILELog     printf
  #define FILELogV    vprintf
#endif
// ----------------------------------------------------------------------------
//[plog_s.cpp]
#if !defined(__TEC32__)
HDECLSPEC int MYRTLEXP_PT _SayCL( const char *msg,... );
HDECLSPEC int MYRTLEXP_PT _SayC( const char *msg,... );
HDECLSPEC int MYRTLEXP_PT _SayL( const char *msg,... );
#endif

#define SayCL _SayCL
#define SayL  _SayL
#define SayC  _SayC

#if !defined(__TEC32__)
HDECLSPEC BOOL FlushLOGFile;
HDECLSPEC BOOL SayLogQuiet;
#endif


// ----------------------------------------------------------------------------
#define LOG_PROC(v) INProc _inp v ;
#define LOG_LOG(v)  INProc::Say v ;

CLASS( INProc )
  static  int Counter;
  CONSTSTR Name;
 public:
   INProc( CONSTSTR ProcedureName,CONSTSTR ProcedureParametersFormat /*=NULL*/,... );
   INProc( CONSTSTR ProcedureName );
   ~INProc();

   static void Say( CONSTSTR Format,... );
};

// ----------------------------------------------------------------------------
#if defined(__FILELOG__)
  #undef PLOG
  #define PLOG(v) FILELog v
#endif
#if defined(__NOPLOG__) || !defined(PLOG)
  #undef PLOG
  #define PLOG(v)
#endif
#if defined(__DBGFILELOG__) && defined(__DEBUG__)
  #undef PLOG
  #define PLOG(v) FILELog v
#endif

#endif
