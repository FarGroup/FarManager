#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

#define FTPHOST_DVERSION                 (2740)
#define FTPHOST_DVERSION_SERVERTYPE      (FTPHOST_DVERSION + sizeof(WORD))
#define FTPHOST_DVERSION_SERVERTYPE_CODE (FTPHOST_DVERSION_SERVERTYPE + sizeof(BOOL))

#define FTPHOST_VERSION_LAST             FTPHOST_DVERSION_SERVERTYPE_CODE

#define FTPHOST_VERSION      Message("1.%d", FTPHOST_VERSION_LAST )

#undef PROC
#undef Log
#if 0
  #define Log(v)  INProc::Say v
  #define PROC(v) INProc _inp v ;
#else
  #define PROC(v)
  #define Log(v)
#endif

//---------------------------------------------------------------------------------
BOOL WarnOldFormat( PFTPHost h )
  {  static CONSTSTR items[] = {
       FMSG( MOldHostFormatTitle ),
       FMSG( MOldHostFormat1 ),
       NULL,
       FMSG( "\x1" ),
       FMSG( MOldHostFormat2 ),
       FMSG( MOldHostFormat3 ),
       FMSG( "\x1" ),
       FMSG(MOldHostConvert), FMSG(MCancel)
     };

     items[2] = h->HostName;

 return FMessage( FMSG_WARNING | FMSG_LEFTALIGN, "WarnOldFmt", items, ARRAY_SIZE(items), 2 ) == 0;
}

//---------------------------------------------------------------------------------
typedef BOOL (*HexToPassword_cb)( char *HexStr,char *Password );

void PasswordToHex(char *Password,char *HexStr)
  {  int  n;
     BYTE pwd[FTP_PWD_LEN];

    MakeCryptPassword( Password,pwd );

    StrCpy(HexStr,"hex:");
    for( n = 0,HexStr += 4; n < FTP_PWD_LEN && pwd[n]; Password++,n++ ) {
      Sprintf(HexStr,"%02x",pwd[n] );
      HexStr+=2;
    }
}
BYTE HexToNum(char Hex)
  {
    if ( Hex >= '0' && Hex <= '9' )
      return (BYTE)(Hex-'0');

  return (BYTE)(ToUpper(Hex)-'A'+10);
}

BOOL HexToPassword_OLD( char *HexStr,char *Password )
  {
    if ( strncmp(HexStr,"hex:",4) != 0 )
      return FALSE;

    HexStr+=4;
    while (*HexStr) {
      *Password = HexToNum(HexStr[0])*16+HexToNum(HexStr[1]);
      Password++;
      HexStr+=2;
    }
    *Password = 0;

 return TRUE;
}

BOOL HexToPassword_2740( char *HexStr,char *Password )
  {  BYTE pwd[FTP_PWD_LEN];
     int  n;

    if ( strncmp(HexStr,"hex:",4) != 0 )
      return FALSE;

    for ( n = 0, HexStr+=4;
          n < FTP_PWD_LEN &&
          *HexStr;
          n++, HexStr+=2 )
      pwd[n] = HexToNum(HexStr[0])*16 + HexToNum(HexStr[1] );

    pwd[n] = 0;

    DecryptPassword( pwd,Password );

 return TRUE;
}

inline BOOL HexToPassword_CUR( char *HexStr,char *Password ) { return HexToPassword_2740(HexStr,Password); }

//---------------------------------------------------------------------------------
void FTPHost::Init( void )
  {
     Log(( "FTPHost::Init %p",this ));

     memset( this,0,sizeof(*this) );
     Size         = sizeof(*this);

     ExtCmdView   = Opt.ExtCmdView;
     IOBuffSize   = Opt.IOBuffSize;
     PassiveMode  = Opt.PassiveMode;
     ProcessCmd   = Opt.ProcessCmd;
     UseFirewall  = Opt.Firewall[0] != 0;
     FFDup        = Opt.FFDup;
     UndupFF      = Opt.UndupFF;
     DecodeCmdLine= TRUE;
     ServerType   = FTP_TYPE_DETECT;

     TStrCpy( ListCMD,  "LIST -la" );
}

void FTPHost::Assign( PFTPHost p )
  {
    Assert( p );
    Assert( p != this );
    memcpy( this,p,sizeof(*this) );
}

BOOL FTPHost::Cmp( PFTPHost p )
  {
 return strcmp(RegKey,p->RegKey) == 0;
}

BOOL FTPHost::CmpConnected( PFTPHost p )
  {
 return StrCmp( p->Host,      Host,      -1, FALSE) == 0 &&
        StrCmp( p->User,      User,      -1, TRUE)  == 0 &&
        StrCmp( p->Password,  Password,  -1, TRUE)  == 0;
}

void FTPHost::MkUrl( String& str,CONSTSTR Path,CONSTSTR nm,BOOL sPwd )
  {  bool defPara = StrCmp(User,"anonymous") == 0 &&
                    StrCmp(Password,Opt.DefaultPassword) == 0;

    if ( !defPara && User[0] ) {
      if ( Password[0] )
        str.printf( "ftp://%s:%s@%s",
                    User,
                    (sPwd || IS_FLAG(Opt.PwdSecurity,SEC_PLAINPWD)) ? Password : "",
                    Host );
       else
        str.printf( "ftp://%s@%s",User,Host );
    } else
      str.printf( "ftp://%s",Host );

    if ( Path && *Path ) {
      if ( *Path != '/' )
        str.Add( '/' );
      str.cat( Path );
    }

    if ( nm && *nm ) {
      int len = str.Length()-1;
      if (len >= 0 && str[len] != '/' && str[len] != '\\' )
        str.Add( '/' );
      str.Add( nm );
    }

    FixFTPSlash( str );
}

char *FTPHost::MkINIFile( char *DestName,CONSTSTR Path,CONSTSTR DestPath )
  {  PROC(( "FTPHost::MkINIFile","{%s,%s} [%s] [%s]",RegKey,Host,Path,DestPath ))
     char *m,*m1;

     StrCpy( DestName,DestPath );

     if ( Path ) {
       while( *Path == '\\' || *Path == '/' ) Path++;
         AddEndSlash( DestName,'\\',FAR_MAX_PATHSIZE );

         // Add from "Hosts\Folder\Item0" "Folder\Item0" part
         StrCat( DestName, RegKey + 6 /*the sizeof("Hosts\\")*/ + strLen(Path), FAR_MAX_PATHSIZE );

         // Remove trailing "\Item0"
         m = strrchr( DestName,'\\' );
         if (m) *m = 0;
     }

     AddEndSlash( DestName,'\\',FAR_MAX_PATHSIZE );

     //Correct bad characters and add possible to DestName
     BOOL  bad,
           curBad;

     m   = DestName + strLen(DestName);    //ptr to add at
     m1  = HostName[0] ? HostName : Host;  //Source to translate
     bad = TRUE;                           //Do not add `bad` an start

     for ( int i = m - DestName + 1;
           i < FAR_MAX_PATHSIZE-4 && //Until not full Dest
           *m1;                   //And source exist
           m1++ ) {

       //Is bad
       curBad = ((BYTE)*m1) < ((BYTE)0x20) ||
                strchr( Folder ? ":/\\\"\'|><^*?" : ":/\\.@\"\'|><^*?",*m1) != 0;

       //Already bad. Add only one.
       if ( curBad ) {
         if (!bad) {
           *m++ = '_';
           bad = TRUE;
           i++;
         }
       } else {
       //Add correct char
         *m++ = *m1;
         bad = FALSE;
         i++;
       }
     }
     *m = 0;

     //Add extension
     if (!Folder)
       StrCat( DestName,".ftp" );

     Log(( "rc: [%s]",DestName ));

 return DestName;
}

CONSTSTR FTPHost::MkHost( CONSTSTR Path,CONSTSTR Name )
  {  PROC(( "FTPHost::MkHost","[%s] [%s]",Path,Name ))
     static char key[FAR_MAX_REG];

    TStrCpy( key,"Hosts" );

    if ( Path ) {
      if ( StrNCmpI( Path,"Hosts",5 ) == 0 )
        Path += 5;
      while( *Path == '\\' ) Path++;
      if ( Path[0] ) {
        TAddEndSlash( key, '\\' );
        TStrCat( key, Path );
      }
    }

    if ( Name && Name[0] ) {
      while( *Name == '\\' || *Name == '/' ) Name++;
      if ( Name[0] ) {
        TAddEndSlash( key, '\\' );
        TStrCat( key, Name );
      }
    }

    DelEndSlash( key,'\\' );

    Log(( "rc=%s",key ));

 return key;
}

BOOL FTPHost::CheckHost( CONSTSTR Path,CONSTSTR Name )
  {
 return FP_CheckRegKey( MkHost(Path,Name) );
}

BOOL FTPHost::CheckHostFolder( CONSTSTR Path,CONSTSTR Name )
  {  CONSTSTR m = MkHost(Path,Name);

 return FP_CheckRegKey(m) &&
        FP_GetRegKey(m,"Folder",0) != 0;
}

char *FindLastBefore( char *str,char ch,char before )
  {  char *m,
          *m1 = StrChr(str,before);
     char *prev = str;

     while( 1 ) {
       m  = strchr(prev,ch);
       if ( !m || (m1 && m1 <= m) )
         break;
       prev = m+1;
     }

 return prev == str ? NULL : (prev-1);
}

BOOL FTPHost::SetHostName( CONSTSTR hnm,CONSTSTR usr,CONSTSTR pwd )
  {  PROC(( "FTPHost::SetHostName","h:[%s], u:[%s], p:[%s]",hnm,usr,pwd ))
     char *m = (char*)hnm,
          *mHost;

    if ( !hnm || !hnm[0] )
      return FALSE;

    User[0]     = 0;
    Password[0] = 0;
    StrCpy( HostName,hnm,sizeof(HostName) );
//ftp.xx
    m = StrNCmpI(m,"ftp://",6) == 0 ? (m+6) : m;
    m = StrNCmpI(m,"http://",7) == 0 ? (m+7) : m;
    mHost = m;

    m = FindLastBefore( mHost,'@','/' );

    if (m) {
//xx@ftp.xx
      StrCpy( Host,m+1,sizeof(Host) );
      StrCpy( User,mHost,Min(m-mHost+1,(int)sizeof(User)) );
      m = FindLastBefore( User,':','/' );
      if (m) {
//xx:xx@ftp.xx
        StrCpy( Password,m+1 );
        m[0] = 0;
      }
    } else
      StrCpy( Host,mHost );
//Home
    m = strchr( Host,'/' );
    if (m) {
      StrCpy( Home,m,sizeof(Home) );
      int Length = strLen(Home);
      if ( Length>1 && Home[Length-1]=='/' ) Home[Length-1] = 0;
      *m = 0;
      FixFTPSlash( Home );
    } else
      Home[0] = 0;
//User
    if (usr && usr[0])
      StrCpy( User,usr,sizeof(User) );

//Psw
    if (pwd && pwd[0])
      StrCpy( Password,pwd,sizeof(Password) );

 return HostName[0] != 0;
}

void AddPath( char *buff,CONSTSTR path )
  {
    if ( path && path[0] ) {
      AddEndSlash( buff,'\\',FAR_MAX_PATHSIZE );
      while( *path == '\\' || *path == '/' ) path++;
      StrCat( buff,path,FAR_MAX_PATHSIZE );
    }
    DelEndSlash( buff,'\\' );
}

void FTPHost::MakeFreeKey( CONSTSTR Hosts )
  {  PROC(( "FTPHost::MakeFreeKey","%s",Hosts ))
     char str[ FAR_MAX_PATHSIZE ];
     char key[ FAR_MAX_PATHSIZE ];
     int  n;

     StrCpy( RegKey, MkHost(NULL,Hosts) );
     Log(( "BaseReg: [%s]",RegKey ));

     if ( Folder ) {
       AddPath( RegKey,Host );
       Log(( "folder rc: %s",RegKey ));
       return;
     }

     for ( n = 0; 1 ; n++ ) {
       StrCpy( key, RegKey,                sizeof(key) );
       StrCat( key, Message("\\Item%d",n), sizeof(key) );

       FP_GetRegKey( key,"HostName",str,NULL,sizeof(str) );
       if ( str[0] == 0 ) {
         strcpy( RegKey,key );
         break;
       } else
         Log(( "Item [%s] exist",key ));
     }
     Log(( "host rc: %s",RegKey ));
}
//---------------------------------------------------------------------------------
BOOL FTPHost::Read( CONSTSTR nm )
  {  PROC(( "FTPHost::Read","%s",nm ))
     char *m;
     BYTE  psw[ FAR_MAX_PATHSIZE ];
     char  usr[ FAR_MAX_PATHSIZE ];
     char  hnm[ FAR_MAX_PATHSIZE ];
     char  pwd[ FAR_MAX_PATHSIZE ];

    //! Init called from `EnumHost::GetNextHost`
    //Init();

    if ( nm && nm[0] )
      StrCpy( RegKey,nm,sizeof(RegKey) );
    Log(( "RegKey: %s",RegKey ));

    Folder = FP_GetRegKey(RegKey,"Folder",0);
    if ( Folder ) {
      oldFmt       = FALSE;
      User[0]      = 0;
      Password[0]  = 0;
      Home[0]      = 0;
      FP_GetRegKey( RegKey,"Description", HostDescr, NULL,sizeof(HostDescr) );
      StrCpy( Host,strrchr(RegKey,'\\')+1 );
      StrCpy( HostName,Host );
      return TRUE;
    }

    m = strrchr(RegKey,'\\');
    if (!m) m = RegKey; else m++;

    if ( !FP_GetRegKey(RegKey,"HostName",hnm,"",sizeof(hnm) ) )
      return FALSE;
    oldFmt = hnm[0] == 0;

    if ( !oldFmt )
      m = hnm;

    FP_GetRegKey( RegKey, "User",     usr, NULL, sizeof(usr) );
    FP_GetRegKey( RegKey, "Password", psw, NULL, sizeof(psw) );
    if ( psw[0] )
      DecryptPassword(psw,pwd);
     else
      pwd[0] = 0;

    SetHostName( m,usr,pwd );

    FP_GetRegKey(RegKey,"Description",HostDescr, NULL,sizeof(HostDescr));
    FP_GetRegKey(RegKey,"Table",      HostTable, NULL,sizeof(HostTable));
    ProcessCmd    = FP_GetRegKey(RegKey,"ProcessCmd",        TRUE );
    AskLogin      = FP_GetRegKey(RegKey,"AskLogin",          FALSE );
    PassiveMode   = FP_GetRegKey(RegKey,"PassiveMode",       FALSE );
    UseFirewall   = FP_GetRegKey(RegKey,"UseFirewall",       FALSE );
    AsciiMode     = FP_GetRegKey(RegKey,"AsciiMode",         FALSE );
    ExtCmdView    = FP_GetRegKey(RegKey,"ExtCmdView",        Opt.ExtCmdView );
    ExtList       = FP_GetRegKey(RegKey,"ExtList",           FALSE );
    ServerType    = FP_GetRegKey(RegKey,"ServerType",        FTP_TYPE_DETECT );
    CodeCmd       = FP_GetRegKey(RegKey,"CodeCmd",           TRUE );
    FP_GetRegKey  ( RegKey,"ListCMD",ListCMD, "LIST -la",sizeof(ListCMD) );
    IOBuffSize    = FP_GetRegKey(RegKey,"IOBuffSize",        Opt.IOBuffSize );
    FFDup         = FP_GetRegKey(RegKey,"FFDup",             Opt.FFDup );
    UndupFF       = FP_GetRegKey(RegKey,"UndupFF",           Opt.UndupFF );
    DecodeCmdLine = FP_GetRegKey(RegKey,"DecodeCmdLine",     TRUE );
    SendAllo      = FP_GetRegKey(RegKey,"SendAllo",          FALSE );
    UseStartSpaces = FP_GetRegKey(RegKey,"UseStartSpaces", TRUE );

    IOBuffSize = Max(FTR_MINBUFFSIZE,IOBuffSize);

 return TRUE;
}

BOOL FTPHost::Write( CONSTSTR nm )
  {  PROC(( "FTPHost::Write","%s",nm ))
     BOOL rc;

    if ( oldFmt &&
         !WarnOldFormat(this) )
      return FALSE;

    Log(( "RegKey=[%s]",RegKey ));
    FP_DeleteRegKey(RegKey);

    if (!RegKey[0] || oldFmt)
      MakeFreeKey(nm);

    rc = TRUE;

    if ( !Folder ) {
      BYTE psw[ FTP_PWD_LEN ];

      Log(( "pwd: [%s]", Password ));
      if ( Password[0] )
        MakeCryptPassword(Password,psw);
       else
        psw[0] = 0;

      Log(( "pwdc: [%s]", psw ));
      rc = FP_SetRegKey(RegKey,"HostName",      HostName ) &&
           FP_SetRegKey(RegKey,"User",          User ) &&
           FP_SetRegKey(RegKey,"Password",      psw,sizeof(psw) ) &&
           FP_SetRegKey(RegKey,"Table",         HostTable  )      &&
           FP_SetRegKey(RegKey,"AskLogin",      AskLogin   )      &&
           FP_SetRegKey(RegKey,"PassiveMode",   PassiveMode)      &&
           FP_SetRegKey(RegKey,"UseFirewall",   UseFirewall)      &&
           FP_SetRegKey(RegKey,"AsciiMode",     AsciiMode  )      &&
           FP_SetRegKey(RegKey,"ExtCmdView",    ExtCmdView )      &&
           FP_SetRegKey(RegKey,"ExtList",       ExtList )         &&
           FP_SetRegKey(RegKey,"ServerType",    ServerType )      &&
           FP_SetRegKey(RegKey,"ListCMD",       ListCMD )         &&
           FP_SetRegKey(RegKey,"ProcessCmd",    ProcessCmd )      &&
           FP_SetRegKey(RegKey,"CodeCmd",       CodeCmd )         &&
           FP_SetRegKey(RegKey,"IOBuffSize",    IOBuffSize )      &&
           FP_SetRegKey(RegKey,"FFDup",         FFDup )           &&
           FP_SetRegKey(RegKey,"UndupFF",       UndupFF )         &&
           FP_SetRegKey(RegKey,"DecodeCmdLine", DecodeCmdLine )   &&
           FP_SetRegKey(RegKey,"SendAllo",      SendAllo )        &&
           FP_SetRegKey(RegKey,"UseStartSpaces", UseStartSpaces );
    }

    rc = rc &&
         FP_SetRegKey(RegKey,"Description", HostDescr  ) &&
         FP_SetRegKey(RegKey,"Folder",      Folder     );

 return rc;
}
//---------------------------------------------------------------------------------
BOOL FTPHost::ReadINI( CONSTSTR nm )
  {  PROC(( "FTPHost::ReadINI","%s",nm ))
     char   hex[FAR_MAX_PATHSIZE*2],
            hst[FAR_MAX_PATHSIZE],
            usr[FAR_MAX_PATHSIZE],
            pwd[FAR_MAX_PATHSIZE];

     HexToPassword_cb DecodeProc = NULL;

    Init();

    GetPrivateProfileString("FarFTP","Version","",hst,sizeof(hst),nm );
    if ( hst[0] == '1' ) {
      size_t sz;
      if ( hst[1] == '.' && (sz=atoi(hst+2)) != 0 )
        switch( sz ) {
          case FTPHOST_DVERSION:
          case FTPHOST_DVERSION_SERVERTYPE:
          case FTPHOST_DVERSION_SERVERTYPE_CODE:
                    DecodeProc = HexToPassword_2740;
                  break;
        }
    } else
      DecodeProc = HexToPassword_OLD;

    if ( !DecodeProc ) {
      CONSTSTR msgs[] = {
        FMSG( MError ),
        FMSG( MNoVersion_txt1 ),
        FMSG( MNoVersion_txt2 ),
        FMSG( MOk ) };
      FMessage( FMSG_WARNING,NULL,msgs,ARRAY_SIZE(msgs),1 );
      DecodeProc = HexToPassword_CUR;
    }

    GetPrivateProfileString("FarFTP","Url","",hst,sizeof(hst),nm );
    if ( !hst[0] )
      return FALSE;

    usr[0] = '1'; usr[1] = 0;
    hex[0] = '2'; hex[1] = 0;

    GetPrivateProfileString( "FarFTP", "User",     "", usr, sizeof(usr), nm );
    GetPrivateProfileString( "FarFTP", "Password", "", hex, sizeof(hex), nm );

    if ( !DecodeProc(hex,pwd) ||
         !SetHostName(hst,usr,pwd) )
      return FALSE;

    GetPrivateProfileString("FarFTP","Description","",HostDescr,sizeof(HostDescr),nm);
    AskLogin    = GetPrivateProfileInt("FarFTP","AskLogin",    0,nm);
    AsciiMode   = GetPrivateProfileInt("FarFTP","AsciiMode",   0,nm);
    PassiveMode = GetPrivateProfileInt("FarFTP","PassiveMode", Opt.PassiveMode,nm);
    UseFirewall = GetPrivateProfileInt("FarFTP","UseFirewall", *Opt.Firewall!=0,nm);
    ExtCmdView  = GetPrivateProfileInt("FarFTP","ExtCmdView",  Opt.ExtCmdView,nm );
    ExtList     = GetPrivateProfileInt("FarFTP","ExtList",     FALSE,nm );
    ServerType  = GetPrivateProfileInt("FarFTP","ServerType",  Opt.ServerType,nm );
    ProcessCmd  = GetPrivateProfileInt("FarFTP","ProcessCmd",  TRUE,nm );
    CodeCmd     = GetPrivateProfileInt("FarFTP","CodeCmd",     TRUE,nm );
    GetPrivateProfileString("FarFTP","ListCMD",ListCMD,ListCMD,sizeof(ListCMD),  nm);
    IOBuffSize  = GetPrivateProfileInt("FarFTP","IOBuffSize",  Opt.IOBuffSize,   nm );
    FFDup       = GetPrivateProfileInt("FarFTP","FFDup",       Opt.FFDup,        nm );
    UndupFF     = GetPrivateProfileInt("FarFTP","UndupFF",     Opt.UndupFF,      nm );
    DecodeCmdLine = GetPrivateProfileInt("FarFTP","DecodeCmdLine", TRUE,         nm );
    SendAllo    = GetPrivateProfileInt("FarFTP","SendAllo",    FALSE,            nm );
    UseStartSpaces = GetPrivateProfileInt("FarFTP","UseStartSpaces",    TRUE,            nm );
    GetPrivateProfileString("FarFTP","CharTable","",HostTable,sizeof(HostTable), nm);

    IOBuffSize = Max(FTR_MINBUFFSIZE,IOBuffSize);

 return TRUE;
}
BOOL FTPHost::WriteINI( CONSTSTR nm )
  {  PROC(( "FTPHost::WriteINI","%s",nm ))
     char HexStr[FAR_MAX_PATHSIZE*2];
     BOOL res;

//CreateDirectory
     char *m = StrRChr(nm,SLASH_CHAR );
     if ( m ) {
       StrCpy( HexStr,nm,sizeof(HexStr) );
       m = StrRChr(HexStr,SLASH_CHAR );
       *m = 0;
       if ( !DoCreateDirectory(HexStr) )
         return FALSE;
     }

//Write INI
     WritePrivateProfileString(NULL,NULL,NULL,nm);
     PasswordToHex( Password,HexStr );

     res = WritePrivateProfileString( "FarFTP","Version",            FTPHOST_VERSION,nm   )           &&
           WritePrivateProfileString( "FarFTP","Url",                HostName,nm   )                  &&
           WritePrivateProfileString( "FarFTP","User",               User,nm       )                  &&
           WritePrivateProfileString( "FarFTP","Password",           HexStr,nm     )                  &&
           WritePrivateProfileString( "FarFTP","Description",        HostDescr,nm  )                  &&
           WritePrivateProfileString( "FarFTP","AskLogin",           Message("%d",AskLogin),nm   )    &&
           WritePrivateProfileString( "FarFTP","AsciiMode",          Message("%d",AsciiMode),nm  )    &&
           WritePrivateProfileString( "FarFTP","PassiveMode",        Message("%d",PassiveMode),nm)    &&
           WritePrivateProfileString( "FarFTP","UseFirewall",        Message("%d",UseFirewall),nm)    &&
           WritePrivateProfileString( "FarFTP","ExtCmdView",         Message("%d",ExtCmdView),nm)     &&
           WritePrivateProfileString( "FarFTP","ExtList",            Message("%d",ExtList),nm)        &&
           WritePrivateProfileString( "FarFTP","ServerType",         Message("%d",ServerType),nm)     &&
           WritePrivateProfileString( "FarFTP","CodeCmd",            Message("%d",CodeCmd),nm)        &&
           WritePrivateProfileString( "FarFTP","ListCMD",            "LIST -la",nm )                  &&
           WritePrivateProfileString( "FarFTP","IOBuffSize",         Message("%d",IOBuffSize),nm)     &&
           WritePrivateProfileString( "FarFTP","FFDup",              Message("%d",FFDup),nm)          &&
           WritePrivateProfileString( "FarFTP","UndupFF",            Message("%d",UndupFF),nm)        &&
           WritePrivateProfileString( "FarFTP","DecodeCmdLine",      Message("%d",DecodeCmdLine),nm)  &&
           WritePrivateProfileString( "FarFTP","SendAllo",           Message("%d",SendAllo),nm)       &&
           WritePrivateProfileString( "FarFTP","UseStartSpaces",  Message("%d",UseStartSpaces),nm) &&
           WritePrivateProfileString( "FarFTP","ProcessCmd",         Message("%d",ProcessCmd),nm)        &&
           WritePrivateProfileString( "FarFTP","CharTable", HostTable,nm  );
     if (res)
       WritePrivateProfileString(NULL,NULL,NULL,nm);
 return res;
}
