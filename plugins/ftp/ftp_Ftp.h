#ifndef __FAR_PLUGIN_FTP_FTP
#define __FAR_PLUGIN_FTP_FTP

#define FTP_COL_MODE 0
#define FTP_COL_LINK 1

#define FTP_COL_MAX  2

#define FTP_MAXBACKUPS 10

STRUCT( FTPUrl )
  FTPHost       Host;
  String        SrcPath;
  String        DestPath;
  String        Error;
  FAR_FIND_DATA FileName;
  BOOL          Download;
  PFTPUrl       Next;
};

STRUCT( FTPCopyInfo )
  BOOL      asciiMode;
  BOOL      ShowProcessList;
  BOOL      AddToQueque;
  overCode  MsgCode;
  String    DestPath;
  String    SrcPath;   //Used only on queue processing
  BOOL      Download;
  BOOL      UploadLowCase;
  BOOL      FTPRename;

  FTPCopyInfo( void );
};

STRUCT( QueueExecOptions )
  BOOL      RestoreState;
  BOOL      RemoveCompleted;
};

CLASS( FTP )
    friend class FTPCmdBlock;
    String      SelectFile;
    BOOL        ResetCache;
    int         ShowHosts;
    int         SwitchingToFTP;
    char        HostsPath[1024];
    int         StartViewMode;
    int         RereadRequired;
    FTPCurrentStates CurrentState;
    char        IncludeMask[ FAR_MAX_PATHSIZE ];
    char        ExcludeMask[ FAR_MAX_PATHSIZE ];
    BOOL        PluginColumnModeSet;
    int         ActiveColumnMode;
    BOOL        NeedToSetActiveMode;
    PFTPUrl     UrlsList, UrlsTail;
    int         QuequeSize;
    overCode    LastMsgCode,
                OverrideMsgCode;
  public:
    FTPHost     Host;
    char        PanelTitle[512];
    HANDLE      LongBeep;
    HANDLE      KeepAlivePeriod;
    Connection *hConnect;
    int         CallLevel;
  private:
    int       Connect();
    void      CopyNamesToClipboard( void );
    int       DeleteFilesINT(PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int       GetFreeKeyNumber();
    void      GetFullFileName(char *FullName,char *Name);
    void      GetFullKey(char *FullKeyName,CONSTSTR Name);
    int       GetHostFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
    void      GetNewKeyName(char *FullKeyName);
    int       HexToNum(int Hex);
    void      HexToPassword(char *HexStr,char *Password);
    void      MakeKeyName(char *FullKeyName,int Number);
    void      PasswordToHex(char *Password,char *HexStr);
    int       PutHostsFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    void      SaveURL();
    void      SelectFileTable(char *TableName);
    WORD      SelectServerType( WORD Type );
    void      SelectTable();
    void      SetAttributes();
    int       TableNameToValue(char *TableName);
  private:
    BOOL      EditDirectory( String& Name,char *Desc,BOOL newDir );
    void      FTP_FixPaths( CONSTSTR base,PluginPanelItem *p,int cn,BOOL FromPlugin );
    void      FTP_FreeFindData( PluginPanelItem *PanelItem,int ItemsNumber,BOOL FromPlugin );
    BOOL      FTP_GetFindData( PluginPanelItem **PanelItem,int *ItemsNumber,BOOL FromPlugin );
    BOOL      FTP_SetDirectory( CONSTSTR dir,BOOL FromPlugin );
    int       GetFilesInterface(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
    BOOL      GetHost( int title,PFTPHost p,BOOL ToDescription );
    BOOL      Reread( void );
    int       PutFilesINT(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    void      SaveUsedDirNFile( void );
    BOOL      ExecCmdLine( CONSTSTR str, BOOL Prefix );
    BOOL      ExecCmdLineFTP( CONSTSTR str, BOOL Prefix );
    BOOL      ExecCmdLineANY( CONSTSTR str, BOOL Prefix );
    BOOL      ExecCmdLineHOST( CONSTSTR str, BOOL Prefix );
    BOOL      DoCommand( CONSTSTR str, int type, DWORD flags );
    BOOL      DoFtpConnect( int blocked );
  private:
    int       ExpandListINT(struct PluginPanelItem *PanelItem,int ItemsNumber,PFP_SizeItemList il,BOOL FromPlugin,ExpandListCB cb = NULL,LPVOID Param = NULL );
    int       ExpandList(struct PluginPanelItem *PanelItem,int ItemsNumber,PFP_SizeItemList il,BOOL FromPlugin,ExpandListCB cb = NULL,LPVOID Param = NULL );
    BOOL      CopyAskDialog( BOOL Move, BOOL Download,PFTPCopyInfo ci );
    BOOL      ShowFilesList( PFP_SizeItemList il );
    overCode  AskOverwrite( int title,BOOL Download,LPFAR_FIND_DATA dest,LPFAR_FIND_DATA src,overCode last,bool haveTimes );
    void      BackToHosts( void );
    BOOL      FullConnect();
    void      SaveList( PFP_SizeItemList il );
    BOOL      SetDirectoryStepped( CONSTSTR Dir, BOOL update );
    void      InsertToQueue( void );
    CONSTSTR  InsertCurrentToQueue( void );
    CONSTSTR  InsertAnotherToQueue( void );
    BOOL      CheckDotsBack( const String& OldDir,const String& CmdDir );
    BOOL      FTPCreateDirectory( CONSTSTR dir,int OpMode );

  private:
    int       _FtpGetFile( CONSTSTR lpszRemoteFile,CONSTSTR lpszNewFile,BOOL Reget,int AsciiMode );
    int       _FtpPutFile( CONSTSTR lpszLocalFile,CONSTSTR lpszNewRemoteFile,BOOL Reput,int AsciiMode );
  public:
    FTP();
    ~FTP();
    int       DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    void      FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    int       GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
    int       GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void      GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int       MakeDirectory( String& Name,int OpMode);
    int       ProcessCommandLine(char *CommandLine);
    int       ProcessEvent(int Event,void *Param);
    int       ProcessKey(int Key,unsigned int ControlState);
    int       ProcessShortcutLine(char *Line);
    int       PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    int       SetDirectory(CONSTSTR Dir,int OpMode);

    int       SetDirectoryFAR(CONSTSTR _Dir,int OpMode);

    void      Invalidate( void );
    void      GetCurPath( char *buff,int bsz );
    void      GetCurPath( String& buff );

    void      LongBeepEnd( BOOL DoNotBeep = FALSE );
    void      LongBeepCreate( void );
    BOOL      HostsMode( void )         { return ShowHosts && !SwitchingToFTP; }
    BOOL      FTPMode( void )           { return !HostsMode() && hConnect; }

    static FTP *Backups[ FTP_MAXBACKUPS ];
    static int  BackupCount;

    void      SetBackupMode( void );
    void      SetActiveMode( void );
    BOOL      isBackup( void );
    void      DeleteFromBackup( void );
    void      AddToBackup( void );

    CONSTSTR  CloseQuery( void );

    PFTPUrl   UrlItem( int num, PFTPUrl *prev );
    void      UrlInit( PFTPUrl p );
    void      DeleteUrlItem( PFTPUrl p, PFTPUrl prev );
    BOOL      EditUrlItem( PFTPUrl p );

    void      AddToQueque( LPFAR_FIND_DATA FileName, CONSTSTR Path, BOOL Download );
    void      AddToQueque( PFTPUrl p,int pos = -1 );
    void      ListToQueque( PFP_SizeItemList il,PFTPCopyInfo ci );
    void      ClearQueue( void );

    void      SetupQOpt( PQueueExecOptions op );
    BOOL      WarnExecuteQueue( PQueueExecOptions op );
    void      QuequeMenu( void );
    void      ExecuteQueue( PQueueExecOptions op );
    void      ExecuteQueueINT( PQueueExecOptions op );

    void Call( void );
    void End( int rc = -156 );
};


extern void MakeCryptPassword( CONSTSTR Src,BYTE Dest[FTP_PWD_LEN] );
extern void DecryptPassword( BYTE Src[FTP_PWD_LEN],char *Dest);

#endif
