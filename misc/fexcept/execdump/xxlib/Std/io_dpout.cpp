#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HOOKS_USED__)

#pragma option -vGc- -vGt- -vGd-

/*******************************************************************
   TYPES
 *******************************************************************/
#define fNUM( v )    ( ( ((int)v) > 100 ) ? fileno(v) : ((int)(v)) )

LOCALSTRUCT( DupFileInfo )
  FILE         *Src;
  FILE         *Dest;
  HVPrintProc_t DestCB;
};

typedef MyRefArray<DupFileInfo> DupFileArrayBase;
LOCALCLASSBASE( DupFileArray, public DupFileArrayBase )
  public:
    DupFileArray( void ) {}

    DEF_SCANER( PDupFileInfo, FindFile, FILE*,
      { return Item->Src == Data || fNUM(Item->Src) == fNUM(Data); } )
};

/*******************************************************************
   DATA
 *******************************************************************/
static DupFileArray Files;
static int          Usage = 0;
static BOOL         Inited = FALSE;
static HANDLE       cs;

/*******************************************************************
   FPRINTF
 *******************************************************************/
DECL_HOOKPROC( int, __cdecl, fprintf, ( FILE *f,const char *fmt,... ) )
  {  va_list ap;
     int     rc;

     va_start(ap,fmt);
       rc = vfprintf( f,fmt,ap );
     va_end(ap);
 return rc;
}

DECL_HOOKPROC( int, __cdecl, vfprintf, ( FILE *f,const char *fmt,va_list ap ) )
  {  PDupFileInfo p;
     int          rc;

     BOOL isSet = WaitForSingleObject(cs,0) == WAIT_TIMEOUT;
     WaitForSingleObject(cs,INFINITE);

     p = Files.FindFile(f);
     if ( p ) {
       if ( p->DestCB ) {
         rc = p->DestCB( fmt,ap );
         if ( rc <= 0 ) return rc;
       } else
       if ( p->Dest )
         vfprintf( p->Dest,fmt,ap );
        else
         return VSNprintf(NULL,0,fmt,ap);

       vfprintf( p->Dest,fmt,ap );
     }

     if(!isSet) SetEvent(cs);

 return CALL_HOOKED(vfprintf)( f,fmt,ap );
}

/*******************************************************************
   PRINTF
 *******************************************************************/
DECL_HOOKPROC( int, __cdecl, printf, ( const char *fmt,... ) )
  {  va_list ap;
     int     rc;
     va_start(ap,fmt);
       rc = vprintf( fmt,ap );
     va_end(ap);
 return rc;
}

DECL_HOOKPROC( int, __cdecl, vprintf, ( const char *fmt,va_list ap ) )
  {  PDupFileInfo p;
     int          rc;
     char         buff[ 1000 ];
     char        *m = buff;

     BOOL isSet = WaitForSingleObject(cs,0) == WAIT_TIMEOUT;
     WaitForSingleObject(cs,INFINITE);

     rc = VSNprintf( NULL,0,fmt,ap );
     p  = Files.FindFile( (FILE*)1 );

     if ( rc < 0 ) rc = -rc;

     if ( p ) {
       if ( p->DestCB ) {
         int rc1 = p->DestCB( fmt,ap );
         if ( rc1 == 0 ) return 0;
       } else
       if ( p->Dest )
         return vfprintf( p->Dest,fmt,ap );
        else
         return rc;
     }

     if(!isSet) SetEvent(cs);

     if ( rc >= sizeof(buff) )
       m = (char*)_Alloc( rc+1 );

     VSNprintf( m,rc+1,fmt,ap );
     //CharToOemBuff( m,m,rc );
     m[rc+1] = 0;

     CALL_HOOKED(printf)( "%s",m );

     if ( m != buff )
       _Del( m );

 return rc;
}

/*******************************************************************
   LOCALS
 *******************************************************************/
static void _DupInit( void )
  {
    if (Inited) return;

    DEF_HOOKPROC( printf );
    DEF_HOOKPROC( fprintf );
    DEF_HOOKPROC( vprintf );
    DEF_HOOKPROC( vfprintf );
    cs = CreateEvent(NULL,TRUE,TRUE,NULL);
    Inited = TRUE;
}

/*******************************************************************
   INTERFACE
 *******************************************************************/
void MYRTLEXP AttachFileOutput( FILE *srcFile,FILE *destFile )
  {  PDupFileInfo p;

    if ( !VERSION_WIN_NT ) return;
    if ( !srcFile ) return;

    _DupInit();

  WaitForSingleObject(cs,INFINITE);
      p = Files.FindFile(srcFile);
      if ( !p ) p = Files.Add( DupFileInfo() );
      p->Src    = srcFile;
      p->Dest   = destFile;
      p->DestCB = NULL;
  SetEvent(cs);
}

void MYRTLEXP AttachFileOutput( FILE *srcFile,HVPrintProc_t cb )
  {  PDupFileInfo p;

    if ( !VERSION_WIN_NT ) return;
    if ( !srcFile ) return;

    _DupInit();

  WaitForSingleObject(cs,INFINITE);
      p = Files.FindFile(srcFile);
      if ( !p ) p = Files.Add( DupFileInfo() );
      p->Src    = srcFile;
      p->Dest   = NULL;
      p->DestCB = cb;
  SetEvent(cs);
}

void MYRTLEXP DetachFileOutput( FILE *srcFile )
  {  PDupFileInfo p;

    if ( !VERSION_WIN_NT ) return;
    if ( !srcFile || !Inited ) return;

  WaitForSingleObject(cs,INFINITE);
      p = Files.FindFile(srcFile);
      if ( p )
        Files.Delete( p );
  SetEvent(cs);
}
#endif
