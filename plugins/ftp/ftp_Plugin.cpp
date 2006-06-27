#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

// ------------------------------------------------------------------------
// FTPNotify
// ------------------------------------------------------------------------
void FTPNotify::Notify( const PFTNNotify p ) { Interface()->Notify( p ); }
// ------------------------------------------------------------------------
// FTPDirList
// ------------------------------------------------------------------------
WORD     FTPDirList::DetectStringType( const PFTPServerInfo Server,char *ListingString, int ListingLength ) { return Interface()->DetectStringType( Server,ListingString,ListingLength ); }
WORD     FTPDirList::GetNumberOfSupportedTypes( void )                                                      { return Interface()->GetNumberOfSupportedTypes(); }
PFTPType FTPDirList::GetType( WORD Index )                                                                  { return Interface()->GetType( Index ); }
WORD     FTPDirList::DetectDirStringType( const PFTPServerInfo Server,CONSTSTR ListingString )              { return Interface()->DetectDirStringType( Server,ListingString); }

//------------------------------------------------------------------------
static FTPInterface Interface;
static BOOL         InterfaceInited = FALSE;

//------------------------------------------------------------------------
HANDLE DECLSPEC idProcStart( CONSTSTR FunctionName,CONSTSTR Format,... )
  {  String str;
     va_list argptr;

     va_start( argptr,Format );
       str.vprintf( Format,argptr );
     va_end( argptr );

 return new FARINProc( FunctionName,str.c_str() );
}

void           DECLSPEC idProcEnd( HANDLE proc )    { delete ((FARINProc*)proc); }
POptionsPlugin DECLSPEC idGetOpt( void )            { return &Opt; }

int DECLSPEC idFtpCmdBlock( int block /*TRUE,FALSE,-1*/ )
  {  FTP *ftp = LastUsedPlugin;
     if ( !ftp || !ftp->hConnect ) return -1;
  return FtpCmdBlock( ftp->hConnect,block );
}

int DECLSPEC idFtpGetRetryCount( void )
  {  FTP *ftp = LastUsedPlugin;
     if ( !ftp || !ftp->hConnect ) return 0;
 return FtpGetRetryCount( ftp->hConnect );
}

PFTPHostPlugin DECLSPEC idGetHostOpt( void )
  {  FTP *ftp = LastUsedPlugin;
     if ( !ftp || !ftp->hConnect ) return NULL;
     return &ftp->hConnect->Host;
}

//------------------------------------------------------------------------
void CreateFTPInterface( void )
  {
     InterfaceInited = TRUE;

     Interface.Magic           = FTP_INTERFACE_MAGIC;
     Interface.SizeOf          = sizeof(Interface);
     Interface.Info            = FP_Info;
     Interface.FSF             = FP_FSF;
     Interface.PluginRootKey   = FP_PluginRootKey;
     Interface.PluginStartPath = FP_PluginStartPath;
     Interface.WinVer          = FP_WinVer;
     Interface.FTPModule       = FP_HModule;

//FAR
     Interface.GetMsg          = FP_GetMsgINT;
     Interface.GetMsgStr       = FP_GetMsgSTR;

//Debug
#if defined(__USE_TRAPLOGER__)
     Interface.Assertion       = __TrapLog;
#else
     Interface.Assertion       = __WinAbort;
#endif
     Interface.SayLog          = FARINProc::Say;
     Interface.LogProcStart    = idProcStart;
     Interface.LogProcEnd      = idProcEnd;

//Reg
     Interface.GetRegKeyFullInt  = FP_GetRegKey;
     Interface.GetRegKeyFullStr  = FP_GetRegKey;
     Interface.GetRegKeyInt      = FP_GetRegKey;
     Interface.GetRegKeyStr      = FP_GetRegKey;

//Std
     Interface.Alloc     = _Alloc;
     Interface.Del       = _Del;
     Interface.Realloc   = _Realloc;
     Interface.PtrSize   = _PtrSize;
     Interface.HeapCheck = _HeapCheck;

     Interface.StrCmp    = StrCmp;
     Interface.StrCpy    = StrCpy;
     Interface.StrCat    = StrCat;
     Interface.strLen    = strLen;
     Interface.StrChr    = StrChr;
     Interface.StrRChr   = StrRChr;

     Interface.Sprintf = Sprintf;
     Interface.SNprintf = SNprintf;
     Interface.VSprintf = VSprintf;
     Interface.VSNprintf = VSNprintf;

//Utilities
     Interface.Message          = Message;
     Interface.MessageV         = MessageV;
     Interface.PointToName      = PointToName;
     Interface.FDigit           = FDigit;
     Interface.FCps             = FCps;
     Interface.FMessage         = FMessage;
     Interface.CheckForEsc      = CheckForEsc;
     Interface.IdleMessage      = IdleMessage;

//FTP related
     Interface.FtpGetRetryCount = idFtpGetRetryCount;
     Interface.FtpCmdBlock      = idFtpCmdBlock;
//Info
     Interface.GetOpt           = idGetOpt;
     Interface.GetHostOpt       = idGetHostOpt;
}

PFTPPluginHolder StdCreator( HMODULE m,PFTPPluginInterface Interface )
  {  PFTPPluginHolder p = new FTPPluginHolder;

     if ( !p->Assign(m,Interface) ) {
       delete p;
       return NULL;
     }

 return p;
}

STRUCT( FTPPluginsInfo )
  DWORD            Magic;
  PFTPPluginHolder Holder;
  PFTPPluginHolder (*Creator)( HMODULE m,PFTPPluginInterface Interface );
  CONSTSTR         Name;
  CONSTSTR         Description;
} StdPlugins[] = {

 /*PLUGIN_xxx*/
  /*PLUGIN_PROGRESS*/ { FTP_PROGRESS_MAGIC, NULL, StdCreator, "ftpProgress.fll", FMSG("Ftp plugin progress dialog") },
  /*PLUGIN_DIRLIST*/  { FTP_DIRLIST_MAGIC,  NULL, StdCreator, "ftpDirList.fll",  FMSG("Ftp plugin directory listing parcer") },
  /*PLUGIN_NOTIFY*/   { FTP_NOTIFY_MAGIC,   NULL, StdCreator, "ftpNotify.fll",   NULL },

{ 0,NULL,NULL,NULL } };

//------------------------------------------------------------------------
BOOL InitPlugins( void )
  {  HMODULE m;
     char    str[ FAR_MAX_PATHSIZE ],*tmp;
     int     n;

     if ( InterfaceInited ) return TRUE;
     CreateFTPInterface();

     for( n = 0; StdPlugins[n].Magic; n++ ) {
       //FAR root
       str[ GetModuleFileName(NULL,str,sizeof(str)) ] = 0;
       tmp = strrchr( str,'\\' ); if (tmp) tmp[1] = 0;
       StrCat( str, StdPlugins[n].Name, sizeof(str) );
       m = LoadLibrary( str );
       if ( !m ) {
         //System-wide
         m = LoadLibrary( StdPlugins[n].Name );
         if ( !m ) {
           //Plugin path
           StrCpy( str, FP_PluginStartPath, sizeof(str) );
           StrCat( str, "\\",               sizeof(str) );
           StrCat( str, StdPlugins[n].Name, sizeof(str) );
           m = LoadLibrary( str );
           if (!m) {
             //Plugin lib path
             StrCpy( str, FP_PluginStartPath, sizeof(str) );
             StrCat( str, "\\Lib\\",            sizeof(str) );
             StrCat( str, StdPlugins[n].Name, sizeof(str) );
             m = LoadLibrary( str );
             if (!m) {
               if ( StdPlugins[n].Description )
                 break;
                else
                 continue;
             }
           }
         }
       }

       BOOL err = TRUE;
       do{
         FTPQueryInterface_t p = (FTPQueryInterface_t)GetProcAddress( m,"FTPQueryInterface" );
         if ( !p ) break;

         PFTPPluginInterface inf = p( &Interface );
         if ( !inf || inf->Magic != StdPlugins[n].Magic )
           break;

         StdPlugins[n].Holder = StdPlugins[n].Creator( m,inf );
         if ( !StdPlugins[n].Holder )
           break;

         err = FALSE;
       }while(0);

       if ( err && StdPlugins[n].Description )
         break;

     }/*for*/

     if ( StdPlugins[n].Magic ) {
       if ( m )
         FreeLibrary(m);

       SNprintf( str,sizeof(str),
                 "Error loading...\n"
                 "FTP plugin: \"%s\"\n"
                 " With name: \"%s\"\n"
                 "    Plugin: %s.\n"
                 "You can not use FTP plugin.",
                 StdPlugins[n].Description, StdPlugins[n].Name,
                 m ? "is not valid FTP plugin" : "can not be found" );
       FP_Info->Message( 0, FMSG_WARNING | FMSG_DOWN | FMSG_LEFTALIGN | FMSG_MB_OK | FMSG_ALLINONE,
                         NULL, (CONSTSTR  const *)str, 0, 0 );

       FreePlugins();
       return FALSE;
     }

 return TRUE;
}

void FreePlugins( void )
  {
     if ( InterfaceInited ) {
       InterfaceInited = FALSE;
       for( int n = 0; StdPlugins[n].Magic; n++ )
         if ( StdPlugins[n].Holder ) {
           StdPlugins[n].Holder->Destroy();
           delete StdPlugins[n].Holder;
           StdPlugins[n].Holder = NULL;
         }
     }
}

PFTPPluginHolder GetPluginHolder( WORD Number )
  {
     Assert( Number < ARRAY_SIZE(StdPlugins)-1 );

 return StdPlugins[Number].Holder;
}

BOOL PluginAvailable( WORD Number )
  {
 return Number < ARRAY_SIZE(StdPlugins)-1 &&
        StdPlugins[Number].Holder;
}

//------------------------------------------------------------------------
BOOL FTPPluginHolder::Assign( HMODULE m,PFTPPluginInterface inf )
  {
    Module    = m;
    Interface = inf;

 return TRUE;
}
void FTPPluginHolder::Destroy( void )
  {
    if ( Module )
      FreeLibrary( Module );
    Module    = NULL;
    Interface = NULL;
}
//------------------------------------------------------------------------
#define CH_OBJ if (!Object) Object = Interface()->CreateObject();

void FTPProgress::Resume( CONSTSTR LocalFileName )                                     { CH_OBJ Interface()->ResumeFile(Object,LocalFileName); }
void FTPProgress::Resume( __int64 size )                                                 { CH_OBJ Interface()->Resume(Object,size); }
BOOL FTPProgress::Callback( int Size )                                                 { CH_OBJ return Interface()->Callback(Object,Size); }
void FTPProgress::Init( HANDLE Connection,int tMsg,int OpMode,PFP_SizeItemList il )        { CH_OBJ Interface()->Init(Object,Connection,tMsg,OpMode,il); }
void FTPProgress::Skip( void )                                                         { CH_OBJ Interface()->Skip(Object); }
void FTPProgress::Waiting( time_t paused )                                             { CH_OBJ Interface()->Waiting(Object,paused); }
void FTPProgress::SetConnection( HANDLE Connection )                                   { CH_OBJ Interface()->SetConnection(Object,Connection); }

void FTPProgress::InitFile( PluginPanelItem *pi, CONSTSTR SrcName, CONSTSTR DestName )
  {
    __int64 sz;
    if ( pi )
      sz = ((__int64)pi->FindData.nFileSizeHigh) * ((__int64)MAX_DWORD) + ((__int64)pi->FindData.nFileSizeLow);
     else
      sz = 0;
    InitFile( sz, SrcName, DestName);
}

void FTPProgress::InitFile( LPFAR_FIND_DATA pi, CONSTSTR SrcName, CONSTSTR DestName )
  {
    __int64 sz;
    if ( pi )
      sz = ((__int64)pi->nFileSizeHigh) * ((__int64)MAX_DWORD) + ((__int64)pi->nFileSizeLow);
     else
      sz = 0;
    InitFile( sz, SrcName, DestName);
}

void FTPProgress::InitFile( __int64 sz, CONSTSTR SrcName, CONSTSTR DestName )
  {
    CH_OBJ
    Interface()->InitFile(Object,sz,SrcName,DestName);
}
