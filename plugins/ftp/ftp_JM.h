#ifndef __MY_JM
#define __MY_JM

class FTP;

//------------------------------------------------------------------------
//ftp_FTPHost.cpp
STRUCTBASE( FTPHost, public FTPHostPlugin )
//Reg
  BOOL     Folder;
  char     HostName[FAR_MAX_PATHSIZE],
           User[FAR_MAX_NAME],
           Password[FAR_MAX_NAME],
           HostDescr[FAR_MAX_NAME];
  char     HostTable[100];
  FILETIME LastWrite;

//Other
  char     Host[FAR_MAX_PATHSIZE];
  char     Home[FAR_MAX_PATHSIZE];
  char     RegKey[FAR_MAX_REG];
  BOOL     oldFmt;

  void     Init( void );
  void     Assign( PFTPHost p );

  void     MkUrl( String& Buff,CONSTSTR Path,CONSTSTR Name,BOOL sPwd = FALSE );
  char    *MkINIFile( char *DestName,CONSTSTR Path,CONSTSTR DestPath );
  BOOL     Cmp( PFTPHost p );
  BOOL     CmpConnected( PFTPHost p );
  void     MakeFreeKey( CONSTSTR Hosts );

  BOOL     SetHostName( CONSTSTR hnm,CONSTSTR usr,CONSTSTR pwd );

  BOOL     Read( CONSTSTR nm );
  BOOL     Write( CONSTSTR Hosts );
  BOOL     ReadINI( CONSTSTR nm );
  BOOL     WriteINI( CONSTSTR nm );

  static BOOL     CheckHost( CONSTSTR Path,CONSTSTR Name );
  static BOOL     CheckHostFolder( CONSTSTR Path,CONSTSTR Name );
  static CONSTSTR MkHost( CONSTSTR Path,CONSTSTR Name );
  static PFTPHost Convert( const PluginPanelItem *p ) { return (p && p->UserData && p->PackSizeHigh == FTP_HOSTID)?((PFTPHost)p->UserData):NULL; }
};

//------------------------------------------------------------------------
//ftp_EnumHost.cpp
class EnumHost {
  public:
    HKEY hEnum;
    char RootKey[ FAR_MAX_REG ];
    int  HostPos;
  public:
    EnumHost( char *HostsPath );
    EnumHost( void ) { hEnum = NULL; }
    ~EnumHost();

    BOOL Assign( char *HostsPath );
    BOOL GetNextHost( PFTPHost p );
};

//------------------------------------------------------------------------
//ftp_FTPBlock.cpp
CLASS( FTPCmdBlock )
    int   hVis;  /*TRUE, FALSE, -1*/
    FTP  *Handle;
  public:
    FTPCmdBlock( FTP *c,int block = -1 );
    ~FTPCmdBlock();

    void Block( int block );
    void Reset( void );
};

#endif
