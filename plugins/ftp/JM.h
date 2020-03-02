#ifndef __MY_JM
#define __MY_JM

class FTP;

//------------------------------------------------------------------------
//ftp_FTPHost.cpp
struct FTPHost: public FTPHostPlugin
{
//Reg
	BOOL     Folder;
	char     HostName[MAX_PATH],
	   User[FAR_MAX_NAME],
	   Password[FAR_MAX_NAME],
	   HostDescr[FAR_MAX_NAME];
	char     HostTable[100];
	FILETIME LastWrite;

//Other
	char     Host[MAX_PATH];
	char     Home[MAX_PATH];
	char     RegKey[FAR_MAX_REG];
	BOOL     oldFmt;

	void     Init(void);
	void     Assign(FTPHost* p);

	void     MkUrl(String& Buff,LPCSTR Path,LPCSTR Name,BOOL sPwd = FALSE);
	char    *MkINIFile(char *DestName,LPCSTR Path,LPCSTR DestPath);
	BOOL     Cmp(FTPHost* p);
	BOOL     CmpConnected(FTPHost* p);
	void     MakeFreeKey(LPCSTR Hosts);

	BOOL     SetHostName(LPCSTR hnm,LPCSTR usr,LPCSTR pwd);

	BOOL     Read(LPCSTR nm);
	BOOL     Write(LPCSTR Hosts);
	BOOL     ReadINI(LPCSTR nm);
	BOOL     WriteINI(LPCSTR nm);

	static BOOL     CheckHost(LPCSTR Path,LPCSTR Name);
	static BOOL     CheckHostFolder(LPCSTR Path,LPCSTR Name);
	static LPCSTR MkHost(LPCSTR Path,LPCSTR Name);
	static FTPHost* Convert(const PluginPanelItem *p)
	{
		return (p && p->UserData && p->PackSizeHigh == FTP_HOSTID)?((FTPHost*)p->UserData):NULL;
	}
};

//------------------------------------------------------------------------
//ftp_EnumHost.cpp
class EnumHost
{
	public:
		HKEY hEnum;
		char RootKey[ FAR_MAX_REG ];
		int  HostPos;
	public:
		EnumHost(char *HostsPath);
		EnumHost(void)
		{
			hEnum = NULL;
		}
		~EnumHost();

		BOOL Assign(char *HostsPath);
		BOOL GetNextHost(FTPHost* p);
};

//------------------------------------------------------------------------
//ftp_FTPBlock.cpp
class FTPCmdBlock
{
		int   hVis;  /*TRUE, FALSE, -1*/
		FTP  *Handle;
	public:
		FTPCmdBlock(FTP *c,int block = -1);
		~FTPCmdBlock();

		void Block(int block);
		void Reset(void);
};

#endif
