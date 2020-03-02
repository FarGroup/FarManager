#ifndef __FAR_PLUGIN_FTP_FTP
#define __FAR_PLUGIN_FTP_FTP

#define FTP_COL_MODE 0
#define FTP_COL_LINK 1

#define FTP_COL_MAX  2

#define FTP_MAXBACKUPS 10

struct FTPUrl
{
	FTPHost       Host;
	String        SrcPath;
	String        DestPath;
	String        Error;
	FAR_FIND_DATA FileName;
	BOOL          Download;
	FTPUrl*       Next;
};

struct FTPCopyInfo
{
	BOOL      asciiMode;
	BOOL      ShowProcessList;
	BOOL      AddToQueque;
	overCode  MsgCode;
	String    DestPath;
	String    SrcPath;   //Used only on queue processing
	BOOL      Download;
	BOOL      UploadLowCase;
	BOOL      FTPRename;

	FTPCopyInfo(void);
};

struct QueueExecOptions
{
	BOOL      RestoreState;
	BOOL      RemoveCompleted;
};

class FTP
{
		friend class FTPCmdBlock;
		String      SelectFile;
		BOOL        ResetCache;
		int         ShowHosts;
		int         SwitchingToFTP;
		char        HostsPath[1024];
		int         StartViewMode;
		int         RereadRequired;
		FTPCurrentStates CurrentState;
		char        IncludeMask[MAX_PATH];
		char        ExcludeMask[MAX_PATH];
		BOOL        PluginColumnModeSet;
		int         ActiveColumnMode;
		BOOL        NeedToSetActiveMode;
		FTPUrl*     UrlsList, *UrlsTail;
		int         QuequeSize;
		overCode    LastMsgCode,
		       OverrideMsgCode;
	public:
		static int  SkipRestoreScreen;

		FTPHost     Host;
		char        PanelTitle[512];
		HANDLE      LongBeep;
		HANDLE      KeepAlivePeriod;
		Connection *hConnect;
		int         CallLevel;
	private:
		int       Connect();
		void      CopyNamesToClipboard(void);
		int       DeleteFilesINT(PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
		int       GetFreeKeyNumber();
		void      GetFullFileName(char *FullName,char *Name);
		void      GetFullKey(char *FullKeyName,LPCSTR Name);
		int       GetHostFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
		void      GetNewKeyName(char *FullKeyName);
		int       HexToNum(int Hex);
		void      HexToPassword(char *HexStr,char *Password);
		void      MakeKeyName(char *FullKeyName,int Number);
		void      PasswordToHex(char *Password,char *HexStr);
		int       PutHostsFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
		void      SaveURL();
		void      SelectFileTable(char *TableName);
		WORD      SelectServerType(WORD Type);
		void      SelectTable();
		void      SetAttributes();
		int       TableNameToValue(char *TableName);
	private:
		BOOL      EditDirectory(String& Name,char *Desc,BOOL newDir);
		void      FTP_FixPaths(LPCSTR base,PluginPanelItem *p,int cn,BOOL FromPlugin);
		void      FTP_FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber,BOOL FromPlugin);
		BOOL      FTP_GetFindData(PluginPanelItem **PanelItem,int *ItemsNumber,BOOL FromPlugin);
		BOOL      FTP_SetDirectory(LPCSTR dir,BOOL FromPlugin);
		int       GetFilesInterface(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
		BOOL      GetHost(int title,FTPHost* p,BOOL ToDescription);
		BOOL      Reread(void);
		int       PutFilesINT(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
		void      SaveUsedDirNFile(void);
		BOOL      ExecCmdLine(LPCSTR str, BOOL Prefix);
		BOOL      ExecCmdLineFTP(LPCSTR str, BOOL Prefix);
		BOOL      ExecCmdLineANY(LPCSTR str, BOOL Prefix);
		BOOL      ExecCmdLineHOST(LPCSTR str, BOOL Prefix);
		BOOL      DoCommand(LPCSTR str, int type, DWORD flags);
		BOOL      DoFtpConnect(int blocked);
	private:
		int       ExpandListINT(struct PluginPanelItem *PanelItem,int ItemsNumber,FP_SizeItemList* il,BOOL FromPlugin,ExpandListCB cb = NULL,LPVOID Param = NULL);
		int       ExpandList(struct PluginPanelItem *PanelItem,int ItemsNumber,FP_SizeItemList* il,BOOL FromPlugin,ExpandListCB cb = NULL,LPVOID Param = NULL);
		BOOL      CopyAskDialog(BOOL Move, BOOL Download,FTPCopyInfo* ci);
		BOOL      ShowFilesList(FP_SizeItemList* il);
		overCode  AskOverwrite(int title,BOOL Download,FAR_FIND_DATA* dest,FAR_FIND_DATA* src,overCode last,bool haveTimes);
		void      BackToHosts(void);
		BOOL      FullConnect();
		void      SaveList(FP_SizeItemList* il);
		BOOL      SetDirectoryStepped(LPCSTR Dir, BOOL update);
		void      InsertToQueue(void);
		LPCSTR  InsertCurrentToQueue(void);
		LPCSTR  InsertAnotherToQueue(void);
		BOOL      CheckDotsBack(const String& OldDir,const String& CmdDir);
		BOOL      FTPCreateDirectory(LPCSTR dir,int OpMode);

	private:
		int       _FtpGetFile(LPCSTR lpszRemoteFile,LPCSTR lpszNewFile,BOOL Reget,int AsciiMode);
		int       _FtpPutFile(LPCSTR lpszLocalFile,LPCSTR lpszNewRemoteFile,BOOL Reput,int AsciiMode);
	public:
		FTP();
		~FTP();
		int       DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
		void      FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
		int       GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode);
		int       GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
		void      GetOpenPluginInfo(struct OpenPluginInfo *Info);
		int       MakeDirectory(String& Name,int OpMode);
		int       ProcessCommandLine(char *CommandLine);
		int       ProcessEvent(int Event,void *Param);
		int       ProcessKey(int Key,unsigned int ControlState);
		int       ProcessShortcutLine(char *Line);
		int       PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
		int       SetDirectory(LPCSTR Dir,int OpMode);

		int       SetDirectoryFAR(LPCSTR _Dir,int OpMode);

		void      Invalidate(void);
		void      GetCurPath(char *buff,int bsz);
		void      GetCurPath(String& buff);

		void      LongBeepEnd(BOOL DoNotBeep = FALSE);
		void      LongBeepCreate(void);
		BOOL      HostsMode(void)
		{
			return ShowHosts && !SwitchingToFTP;
		}
		BOOL      FTPMode(void)
		{
			return !HostsMode() && hConnect;
		}

		static FTP *Backups[ FTP_MAXBACKUPS ];
		static int  BackupCount;

		void      SetBackupMode(void);
		void      SetActiveMode(void);
		BOOL      isBackup(void);
		void      DeleteFromBackup(void);
		void      AddToBackup(void);

		LPCSTR  CloseQuery(void);

		FTPUrl*   UrlItem(int num, FTPUrl* *prev);
		void      UrlInit(FTPUrl* p);
		void      DeleteUrlItem(FTPUrl* p, FTPUrl* prev);
		BOOL      EditUrlItem(FTPUrl* p);

		void      AddToQueque(FAR_FIND_DATA* FileName, LPCSTR Path, BOOL Download);
		void      AddToQueque(FTPUrl* p,int pos = -1);
		void      ListToQueque(FP_SizeItemList* il,FTPCopyInfo* ci);
		void      ClearQueue(void);

		void      SetupQOpt(QueueExecOptions* op);
		BOOL      WarnExecuteQueue(QueueExecOptions* op);
		void      QuequeMenu(void);
		void      ExecuteQueue(QueueExecOptions* op);
		void      ExecuteQueueINT(QueueExecOptions* op);

		void Call(void);
		void End(int rc = -156);
};


extern void MakeCryptPassword(LPCSTR Src,BYTE Dest[FTP_PWD_LEN]);
extern void DecryptPassword(BYTE Src[FTP_PWD_LEN],char *Dest);

#endif
