#include <all_far.h>
#pragma hdrstop

#include "Int.h"

// ------------------------------------------------------------------------
// FTPNotify
// ------------------------------------------------------------------------
void FTPNotify::Notify(const FTNNotify* p)
{
	Interface()->Notify(p);
}
// ------------------------------------------------------------------------
// FTPDirList
// ------------------------------------------------------------------------
WORD     FTPDirList::DetectStringType(FTPServerInfo * const Server,char *ListingString, int ListingLength)
{
	return Interface()->DetectStringType(Server,ListingString,ListingLength);
}
WORD     FTPDirList::GetNumberOfSupportedTypes(void)
{
	return Interface()->GetNumberOfSupportedTypes();
}
FTPType* FTPDirList::GetType(WORD Index)
{
	return Interface()->GetType(Index);
}
WORD     FTPDirList::DetectDirStringType(FTPServerInfo * const Server,LPCSTR ListingString)
{
	return Interface()->DetectDirStringType(Server,ListingString);
}

//------------------------------------------------------------------------
static FTPInterface Interface;
static BOOL         InterfaceInited = FALSE;

//------------------------------------------------------------------------
HANDLE __cdecl idProcStart(LPCSTR FunctionName,LPCSTR Format,...)
{
	String str;
	va_list argptr;
	va_start(argptr,Format);
	str.vprintf(Format,argptr);
	va_end(argptr);
	return new FARINProc(FunctionName,str.c_str());
}

void           WINAPI idProcEnd(HANDLE proc)
{
	delete((FARINProc*)proc);
}
OptionsPlugin* WINAPI idGetOpt(void)
{
	return &Opt;
}

int WINAPI idFtpCmdBlock(int block /*TRUE,FALSE,-1*/)
{
	FTP *ftp = LastUsedPlugin;

	if(!ftp || !ftp->hConnect) return -1;

	return FtpCmdBlock(ftp->hConnect,block);
}

int WINAPI idFtpGetRetryCount(void)
{
	FTP *ftp = LastUsedPlugin;

	if(!ftp || !ftp->hConnect) return 0;

	return FtpGetRetryCount(ftp->hConnect);
}

FTPHostPlugin* WINAPI idGetHostOpt(void)
{
	FTP *ftp = LastUsedPlugin;

	if(!ftp || !ftp->hConnect) return NULL;

	return &ftp->hConnect->Host;
}

//------------------------------------------------------------------------
void CreateFTPInterface(void)
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
	Interface.Assertion       = __WinAbort;
	Interface.SayLog          = FARINProc::Say;
	Interface.LogProcStart    = idProcStart;
	Interface.LogProcEnd      = idProcEnd;
//Reg
	Interface.GetRegKeyFullInt  = FP_GetRegKey;
	Interface.GetRegKeyFullStr  = FP_GetRegKey;
	Interface.GetRegKeyInt      = FP_GetRegKey;
	Interface.GetRegKeyStr      = FP_GetRegKey;
//Std
	Interface.StrCmp    = StrCmp;
	Interface.StrCpy    = StrCpy;
	Interface.StrCat    = StrCat;
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

FTPPluginHolder* StdCreator(HMODULE m,FTPPluginInterface* Interface)
{
	FTPPluginHolder* p = new FTPPluginHolder;

	if(!p->Assign(m,Interface))
	{
		delete p;
		return NULL;
	}

	return p;
}

struct FTPPluginsInfo
{
	DWORD            Magic;
	FTPPluginHolder* Holder;
	FTPPluginHolder*(*Creator)(HMODULE m,FTPPluginInterface* Interface);
	LPCSTR         Name;
	LPCSTR         Description;
} StdPlugins[] =
{

	/*PLUGIN_xxx*/
	/*PLUGIN_PROGRESS*/ { FTP_PROGRESS_MAGIC, NULL, StdCreator, "ftpProgress", FMSG("Ftp plugin progress dialog") },
	/*PLUGIN_DIRLIST*/  { FTP_DIRLIST_MAGIC,  NULL, StdCreator, "ftpDirList",  FMSG("Ftp plugin directory listing parcer") },
	/*PLUGIN_NOTIFY*/   { FTP_NOTIFY_MAGIC,   NULL, StdCreator, "ftpNotify",   NULL },

	{ 0,NULL,NULL,NULL }
};

//------------------------------------------------------------------------

BOOL InitPlugins(void)
{
	if(InterfaceInited) return TRUE;
	CreateFTPInterface();

	HMODULE m = NULL;
	char    str[MAX_PATH], *dll_path, *suffix;
	int     n;

	StrCpy(str, dll_path = Interface.Info->ModuleName, ARRAYSIZE(str));
	suffix = dll_path + lstrlen(dll_path) - 4;
	while ( suffix > dll_path && nullptr != strchr(".-_x86432", suffix[-1]) ) --suffix;

	for(n = 0; StdPlugins[n].Magic; n++)
	{
		StrCpy(str, FP_PluginStartPath, ARRAYSIZE(str));
		StrCat(str, "\\",               ARRAYSIZE(str));
		StrCat(str, StdPlugins[n].Name, ARRAYSIZE(str));
		StrCat(str, suffix,             ARRAYSIZE(str));
		str[lstrlen(str)-3] = 'f'; // "*.dll" => "*.fll"
		m = LoadLibrary(str);
		if ( !m )
		{
			StrCpy(str, FP_PluginStartPath, ARRAYSIZE(str));
			StrCat(str, "\\lib\\",          ARRAYSIZE(str));
			StrCat(str, StdPlugins[n].Name, ARRAYSIZE(str));
			StrCat(str, suffix,             ARRAYSIZE(str));
			str[lstrlen(str)-3] = 'f';
			m = LoadLibrary(str);
		}
		if(!m)
		{
			if(StdPlugins[n].Description)
				break;
			else
				continue;
		}

		BOOL err = TRUE;
		do {
			FTPQueryInterface_t p = (FTPQueryInterface_t)GetProcAddress(m,"FTPQueryInterface");
			if(!p)
				break;

			FTPPluginInterface* inf = p(&Interface);
			if( !inf || inf->Magic != StdPlugins[n].Magic )
				break;

			StdPlugins[n].Holder = StdPlugins[n].Creator(m,inf);
			if( !StdPlugins[n].Holder )
				break;

			err = FALSE;
		}
		while(0);
		if ( err && StdPlugins[n].Description )
			break;
	}

	if(StdPlugins[n].Magic)
	{
		if(m)
			FreeLibrary(m);

		_snprintf(str,ARRAYSIZE(str),
		          "Error loading...\n"
		          "FTP plugin: \"%s\"\n"
		          " With name: \"%s\"\n"
		          "    Plugin: %s.\n"
		          "You can not use FTP plugin.",
		          StdPlugins[n].Description, StdPlugins[n].Name,
		          m ? "is not valid FTP plugin" : "can not be found");
		FP_Info->Message(FP_Info->ModuleNumber, FMSG_WARNING | FMSG_DOWN | FMSG_LEFTALIGN | FMSG_MB_OK | FMSG_ALLINONE,
		                 NULL, (LPCSTR  const *)str, 0, 0);
		FreePlugins();
		return FALSE;
	}

	return TRUE;
}

void FreePlugins(void)
{
	if(InterfaceInited)
	{
		InterfaceInited = FALSE;

		for(int n = 0; StdPlugins[n].Magic; n++)
			if(StdPlugins[n].Holder)
			{
				StdPlugins[n].Holder->Destroy();
				delete StdPlugins[n].Holder;
				StdPlugins[n].Holder = NULL;
			}
	}
}

FTPPluginHolder* GetPluginHolder(WORD Number)
{
	Assert(Number < ARRAYSIZE(StdPlugins)-1);
	return StdPlugins[Number].Holder;
}

BOOL PluginAvailable(WORD Number)
{
	return Number < ARRAYSIZE(StdPlugins)-1 &&
	       StdPlugins[Number].Holder;
}

//------------------------------------------------------------------------
BOOL FTPPluginHolder::Assign(HMODULE m,FTPPluginInterface* inf)
{
	Module    = m;
	Interface = inf;
	return TRUE;
}
void FTPPluginHolder::Destroy(void)
{
	if(Module)
		FreeLibrary(Module);

	Module    = NULL;
	Interface = NULL;
}
//------------------------------------------------------------------------
#define CH_OBJ if (!Object) Object = Interface()->CreateObject();

void FTPProgress::Resume(LPCSTR LocalFileName)
{
	CH_OBJ Interface()->ResumeFile(Object,LocalFileName);
}
void FTPProgress::Resume(__int64 size)
{
	CH_OBJ Interface()->Resume(Object,size);
}
BOOL FTPProgress::Callback(int Size)
{
	CH_OBJ return Interface()->Callback(Object,Size);
}
void FTPProgress::Init(HANDLE Connection,int tMsg,int OpMode,FP_SizeItemList* il)
{
	CH_OBJ Interface()->Init(Object,Connection,tMsg,OpMode,il);
}
void FTPProgress::Skip(void)
{
	CH_OBJ Interface()->Skip(Object);
}
void FTPProgress::Waiting(time_t paused)
{
	CH_OBJ Interface()->Waiting(Object,paused);
}
void FTPProgress::SetConnection(HANDLE Connection)
{
	CH_OBJ Interface()->SetConnection(Object,Connection);
}

void FTPProgress::InitFile(PluginPanelItem *pi, LPCSTR SrcName, LPCSTR DestName)
{
	__int64 sz;

	if(pi)
		sz = ((__int64)pi->FindData.nFileSizeHigh) << 32 | pi->FindData.nFileSizeLow;
	else
		sz = 0;

	InitFile(sz, SrcName, DestName);
}

void FTPProgress::InitFile(FAR_FIND_DATA* pi, LPCSTR SrcName, LPCSTR DestName)
{
	__int64 sz;

	if(pi)
		sz = ((__int64)pi->nFileSizeHigh) << 32 | pi->nFileSizeLow;
	else
		sz = 0;

	InitFile(sz, SrcName, DestName);
}

void FTPProgress::InitFile(__int64 sz, LPCSTR SrcName, LPCSTR DestName)
{
	CH_OBJ
	Interface()->InitFile(Object,sz,SrcName,DestName);
}
