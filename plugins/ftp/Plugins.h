#ifndef __FTP_PLUGINS_HOLDERS
#define __FTP_PLUGINS_HOLDERS

//------------------------------------------------------------------------
/* Number in array of interfaces [Plugin.cpp]
*/
#define PLUGIN_PROGRESS  0
#define PLUGIN_DIRLIST   1
#define PLUGIN_NOTIFY    2

//------------------------------------------------------------------------
struct FTPPluginHolder
{
	HMODULE             Module;
	FTPPluginInterface* Interface;

	virtual BOOL Assign(HMODULE m,FTPPluginInterface* Interface);
	virtual void Destroy(void);
};

extern BOOL             InitPlugins(void);
extern void             FreePlugins(void);
extern FTPPluginHolder* GetPluginHolder(WORD Number);
extern BOOL             PluginAvailable(WORD Number);

//------------------------------------------------------------------------
template <class Cl, WORD Index> struct FTPPlugin
{
	FTPPluginHolder* Holder;

	FTPPlugin(void)
	{
		Holder = GetPluginHolder(Index);
	}
	virtual ~FTPPlugin()
	{
		Holder = NULL;
	}

	Cl Interface(void)
	{
		Assert(Holder);
		Assert(Holder->Interface);
		return (Cl)Holder->Interface;
	}
};
//------------------------------------------------------------------------
struct FTPProgress : public FTPPlugin<ProgressInterface*,PLUGIN_PROGRESS>
{
	HANDLE Object;

	FTPProgress(void)
	{
		Object = NULL;
	}
	~FTPProgress()
	{
		if(Object)
		{
			Interface()->DestroyObject(Object);
			Object = NULL;
		}
	}

	void Resume(LPCSTR LocalFileName);
	void Resume(__int64 size);
	BOOL Callback(int Size);
	void Init(HANDLE Connection,int tMsg,int OpMode,FP_SizeItemList* il);
	void InitFile(__int64 sz, LPCSTR SrcName, LPCSTR DestName);
	void InitFile(PluginPanelItem *pi, LPCSTR SrcName, LPCSTR DestName);
	void InitFile(FAR_FIND_DATA* pi, LPCSTR SrcName, LPCSTR DestName);
	void Skip(void);
	void Waiting(time_t paused);
	void SetConnection(HANDLE Connection);
};

//------------------------------------------------------------------------
struct FTPDirList : public FTPPlugin<DirListInterface*,PLUGIN_DIRLIST>
{

	WORD     DetectStringType(FTPServerInfo* const Server,char *ListingString, int ListingLength);
	WORD     DetectDirStringType(FTPServerInfo* const Server,LPCSTR ListingString);
	WORD     GetNumberOfSupportedTypes(void);
	FTPType* GetType(WORD Index);
};

//------------------------------------------------------------------------
struct FTPNotify : public FTPPlugin<NotifyInterface*,PLUGIN_NOTIFY>
{

	void     Notify(const FTNNotify* p);
};

#endif
