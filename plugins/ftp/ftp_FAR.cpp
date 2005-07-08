#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"
#include <mem.inc>

CONSTSTR DECLSPEC FP_GetPluginName( void )         { return "FarFtp.dll"; }
CONSTSTR DECLSPEC FP_GetPluginLogName( void )      { return "farftp.log"; }
BOOL     DECLSPEC FP_PluginStartup( DWORD Reason ) { return TRUE; }

//------------------------------------------------------------------------
Options       Opt;
FTP          *FTPPanels[3] = { 0 };
FTP          *LastUsedPlugin = NULL;
BOOL          SocketStartup  = FALSE;
int           SocketInitializeError = 0;
AbortProc     ExitProc;
char          DialogEditBuffer[ DIALOG_EDIT_SIZE ];

typedef char  FTPDistString[ FAR_MAX_PATHSIZE ];
FTPDistString DiskStrings[ 1+FTP_MAXBACKUPS ];
CONSTSTR      DiskMenuStrings[ 1+FTP_MAXBACKUPS ];
int           DiskMenuNumbers[ 1+FTP_MAXBACKUPS ];

//------------------------------------------------------------------------
FTP *DECLSPEC OtherPlugin( FTP *p )
  {
    if ( !p )
      return p;

    if ( FTPPanels[0] == p )
      return FTPPanels[1];
     else
    if ( FTPPanels[1] == p )
      return FTPPanels[0];

 return NULL;
}

int DECLSPEC PluginPanelNumber( FTP *p )
  {
    if ( p ) {
      if ( FTPPanels[0] == p )
        return 1;
       else
      if ( FTPPanels[1] == p )
        return 2;
    }

 return 0;
}

int DECLSPEC PluginUsed( void )
  {
  return PluginPanelNumber( LastUsedPlugin );
}

//------------------------------------------------------------------------
void RTL_CALLBACK CloseUp( void )
  {  int n;

     if ( FTP::BackupCount ) {
       Log(( "CloseUp.FreeBackups" ));
       for( n = 0; n < FTP::BackupCount; n++ )
         delete FTP::Backups[n];
       FTP::BackupCount = NULL;
     }
     Log(( "CloseUp.FreePlugins" ));
     FreePlugins();

     Log(( "CloseUp.Delete Opt data" ));
     for ( n = 0; n < 12; n++ )
       _Del( Opt.Months[n] );
     memset( Opt.Months, 0, sizeof(Opt.Months) );

     if ( SocketStartup ) {
       Log(( "CloseUp.CloseWSA" ));
       WSACleanup();
     }

     if ( ExitProc )
       ExitProc();
}

void AddPlugin( FTP *ftp )
  {
    if ( !FTPPanels[0] )
      FTPPanels[0] = ftp;
     else
    if ( !FTPPanels[1] )
      FTPPanels[1] = ftp;
     else
    if ( !FTPPanels[2] )
      FTPPanels[2] = ftp;
     else
      Assert( !"More then two plugins in a time !!" );
}

void RemovePlugin( FTP *ftp )
  {  PROC(( "RemovePlugin","%p",ftp ))

    if ( FTPPanels[0] == ftp )
      FTPPanels[0] = NULL;
     else
    if ( FTPPanels[1] == ftp )
      FTPPanels[1] = NULL;
     else
    if ( FTPPanels[2] == ftp )
      FTPPanels[2] = NULL;

    if ( FTPPanels[2] ) {
      AddPlugin( FTPPanels[2] );
      FTPPanels[2] = NULL;
    }

    CONSTSTR rejectReason;
    if ( ftp->isBackup() ) {
      ftp->SetBackupMode();
      return;
    }

    CONSTSTR itms[] = {
      FMSG(MRejectTitle),
      FMSG(MRejectCanNot),
      NULL,
      FMSG(MRejectAsk1),
      FMSG(MRejectAsk2),
      FMSG(MRejectIgnore), FMSG(MRejectSite)
    };

    do{
      if ( (rejectReason=ftp->CloseQuery()) == NULL ) break;

      itms[2] = rejectReason;
      if ( FMessage( FMSG_LEFTALIGN|FMSG_WARNING,"CloseQueryReject",itms,ARRAY_SIZE(itms),2) != 1 )
        break;

      ftp->AddToBackup();
      if ( !ftp->isBackup() ) {
        SayMsg( FMSG(MRejectFail) );
        break;
      }

      return;
    }while(0);
    delete ftp;
}

//------------------------------------------------------------------------
FAR_EXTERN void FAR_DECLSPEC ExitFAR( void )
  {
     //FP_Info = NULL; FP_GetMsg( "temp" );
     CallAtExit();
}

FAR_EXTERN void FAR_DECLSPEC SetStartupInfo(const struct PluginStartupInfo *Info)
  {
     LastUsedPlugin = NULL;
     FP_SetStartupInfo( Info,"FTP" );

     ExitProc = AtExit( CloseUp );
     memset( &Opt, 0, sizeof(Opt) );

     for( int n = 0; n < FTP_MAXBACKUPS; n++ )
       DiskMenuStrings[n] = DiskStrings[n];
     memset( DiskMenuNumbers, 0, sizeof(DiskMenuNumbers) );

     PROC(( "SetStartupInfo",NULL ))
     ReadCfg();
     LogCmd( "FTP plugin loaded", ldInt );
}

FAR_EXTERN void FAR_DECLSPEC GetPluginInfo(struct PluginInfo *Info)
  {  LastUsedPlugin = NULL;
     PROC(("GetPluginInfo","%p",Info))

  static CONSTSTR PluginMenuStrings[1];
  static CONSTSTR PluginCfgStrings[1];
  static char     MenuString[ FAR_MAX_PATHSIZE ];
  static char     CfgString[ FAR_MAX_PATHSIZE ];

  SNprintf( MenuString,     sizeof(MenuString),     "%s", FP_GetMsg(MFtpMenu) );
  SNprintf( DiskStrings[0], sizeof(DiskStrings[0]), "%s", FP_GetMsg(MFtpDiskMenu) );
  SNprintf( CfgString,      sizeof(CfgString),      "%s", FP_GetMsg(MFtpMenu) );

  PFTPHost p;
  int      n,
           uLen = 0,
           hLen = 0;
  char     str[ FAR_MAX_PATHSIZE ];
  PFTP     ftp;

  for( n = 0; n < FTP::BackupCount; n++ ) {
    ftp = FTP::Backups[n];
    if ( !ftp->FTPMode() )
      continue;
    p = &ftp->Host;
    uLen = Max( uLen, strLen(p->User) );
    hLen = Max( hLen, strLen(p->Host) );
  }

  for( n = 0; n < FTP::BackupCount; n++ ) {
    ftp = FTP::Backups[n];
    ftp->GetCurPath( str,sizeof(str) );
    if ( ftp->FTPMode() ) {
      p = &ftp->Host;
      SNprintf( DiskStrings[1+n], sizeof(DiskStrings[0]),
                "FTP: %-*s %-*s %s",
                uLen, p->User, hLen, p->Host, str );
    } else
      SNprintf( DiskStrings[1+n], sizeof(DiskStrings[0]), "FTP: %s", str );
  }

  DiskMenuNumbers[0]   = Opt.DisksMenuDigit;
  PluginMenuStrings[0] = MenuString;
  PluginCfgStrings[0]  = CfgString;

  Info->StructSize                = sizeof(*Info);
  Info->Flags                     = 0;

  Info->DiskMenuStrings           = DiskMenuStrings;
  Info->DiskMenuNumbers           = DiskMenuNumbers;
  Info->DiskMenuStringsNumber     = Opt.AddToDisksMenu ? (1+FTP::BackupCount) : 0;

  Info->PluginMenuStrings         = PluginMenuStrings;
  Info->PluginMenuStringsNumber   = Opt.AddToPluginsMenu ? (sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;

  Info->PluginConfigStrings       = PluginCfgStrings;
  Info->PluginConfigStringsNumber = sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);

  Info->CommandPrefix             = FTP_CMDPREFIX;
}

FAR_EXTERN int FAR_DECLSPEC Configure(int ItemNumber)
  {  LastUsedPlugin = NULL;
     PROC(("Configure","%d",ItemNumber))

     switch(ItemNumber) {
       case 0: if ( !Config() )
                 return FALSE;
     }

     //Update panels

  return TRUE;
}

FAR_EXTERN HANDLE FAR_DECLSPEC OpenPlugin(int OpenFrom,int Item)
  {  LastUsedPlugin = NULL;
     PROC(("OpenPlugin","%d,%d",OpenFrom,Item))
     FTP *Ftp;

     ReadCfg();
     if ( !InitPlugins() )
       return INVALID_HANDLE_VALUE;

     if ( Item == 0 || Item > FTP::BackupCount )
       Ftp = new FTP;
      else {
       Ftp = FTP::Backups[ Item-1 ];
       Ftp->SetActiveMode();
      }

     AddPlugin(Ftp);

     Ftp->Call();
     Log(( "FTP handle: %p",Ftp ));
     do{
       if (OpenFrom==OPEN_SHORTCUT) {
         if (!Ftp->ProcessShortcutLine((char *)Item))
           break;
         Ftp->End();
       } else
       if (OpenFrom==OPEN_COMMANDLINE) {
         if (!Ftp->ProcessCommandLine((char *)Item))
           break;
       }

       Ftp->End();
       return (HANDLE)Ftp;
     }while(0);

     RemovePlugin(Ftp);

 return INVALID_HANDLE_VALUE;
}

FAR_EXTERN void FAR_DECLSPEC ClosePlugin(HANDLE hPlugin)
  {  FTP *p = (FTP*)hPlugin;

     PROC(("ClosePlugin","%p",hPlugin))

     LastUsedPlugin = p;
     RemovePlugin(p);
}

FAR_EXTERN int FAR_DECLSPEC GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{  FPOpMode _op(OpMode);
   PFTP     p = (FTP*)hPlugin;

   p->Call();
     int rc = p->GetFindData(pPanelItem,pItemsNumber,OpMode);
   p->End(rc);

 return rc;
}

FAR_EXTERN void FAR_DECLSPEC FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
  {  PFTP     p = (FTP*)hPlugin;

    p->Call();
      p->FreeFindData(PanelItem,ItemsNumber);
    p->End();
}

FAR_EXTERN void FAR_DECLSPEC GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
  {  PFTP     p = (FTP*)hPlugin;

   p->Call();
     p->GetOpenPluginInfo(Info);
   p->End();
}

FAR_EXTERN int FAR_DECLSPEC SetDirectory(HANDLE hPlugin,CONSTSTR Dir,int OpMode)
{  FPOpMode _op(OpMode);
   PFTP     p = (FTP*)hPlugin;

   p->Call();
     int rc = p->SetDirectoryFAR(Dir,OpMode);
   p->End(rc);

 return rc;
}

FAR_EXTERN int FAR_DECLSPEC GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{  FPOpMode _op(OpMode);
   PFTP     p = (FTP*)hPlugin;
   if ( !p || !DestPath || !DestPath[0] )
     return FALSE;

   String s( DestPath );
   p->Call();
     int rc = p->GetFiles(PanelItem,ItemsNumber,Move,s,OpMode);
   p->End(rc);
  return rc;
}

FAR_EXTERN int FAR_DECLSPEC PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
  {  FPOpMode _op(OpMode);
     PFTP     p = (FTP*)hPlugin;

   p->Call();
     int rc = p->PutFiles(PanelItem,ItemsNumber,Move,OpMode);
   p->End(rc);
  return rc;
}

FAR_EXTERN int FAR_DECLSPEC DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
  {  FPOpMode _op(OpMode);
     PFTP     p = (FTP*)hPlugin;

   p->Call();
     int rc = p->DeleteFiles(PanelItem,ItemsNumber,OpMode);
   p->End(rc);
 return rc;
}

FAR_EXTERN int FAR_DECLSPEC MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
  {  FPOpMode _op(OpMode);

     if ( !hPlugin )
       return FALSE;

     PFTP     p = (FTP*)hPlugin;
     String   s( Name ? Name : "" );

     p->Call();
       int rc = p->MakeDirectory(s,OpMode);
     p->End(rc);

 return rc;
}

FAR_EXTERN int FAR_DECLSPEC ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
  {  PFTP     p = (FTP*)hPlugin;

   p->Call();
     int rc = p->ProcessKey(Key,ControlState);
   p->End(rc);

 return rc;
}

FAR_EXTERN int FAR_DECLSPEC ProcessEvent(HANDLE hPlugin,int Event,void *Param)
  {  PFTP     p = (FTP*)hPlugin;
     LastUsedPlugin = p;

#if defined(__FILELOG__)
   static CONSTSTR evts[] = { "CHANGEVIEWMODE", "REDRAW", "IDLE", "CLOSE", "BREAK", "COMMAND" };
     PROC(( "FAR.ProcessEvent","%p,%s[%08X]",
            hPlugin,
            (Event < ARRAY_SIZE(evts)) ? evts[Event] : Message("<unk>%d",Event),Param))
#endif

   p->Call();
     int rc = p->ProcessEvent(Event,Param);
   p->End(rc);
 return rc;
}

FAR_EXTERN int FAR_DECLSPEC Compare( HANDLE hPlugin,const PluginPanelItem *i,const PluginPanelItem *i1,unsigned int Mode )
  {
     PFTPHost p  = FTPHost::Convert(i),
              p1 = FTPHost::Convert(i1);
     int      n;

     if ( !i || !i1 || !p || !p1 )
       return -2;

#define CMP( v,v1 ) (CompareString( LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,v,-1,v1,-1 ) - 2)

     switch( Mode ) {
       case   SM_EXT: n = CMP( p->Home,p1->Home );           break;
       case SM_DESCR: n = CMP( p->HostDescr,p1->HostDescr ); break;
       case SM_OWNER: n = CMP( p->User,p1->User );           break;
             default: n = CMP( p->Host,p1->Host );           break;
     }

    do{
     if (n) break;

     n = CMP( p->Host,p1->Host );
     if (n) break;

     n = CMP( p->User,p1->User );
     if (n) break;

     n = CMP( p->HostDescr,p1->HostDescr );
     if (n) break;
   }while( 0 );

   if (n)
     return (n>0)?1:(-1);
    else
     return 0;
}
