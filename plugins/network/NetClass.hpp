#ifndef __NETCLASS_HPP__
#define __NETCLASS_HPP__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif // _MSC_VER

#include <plugin.hpp>
#ifdef NETWORK_LOGGING
#include <stdio.h>
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// winnt.h
#ifndef FILE_ATTRIBUTE_VIRTUAL
#define FILE_ATTRIBUTE_VIRTUAL 0x00010000
#endif

class NetResourceList
{
private:
	NETRESOURCE* ResList;
	unsigned int ResCount;

public:
	NetResourceList();
	~NetResourceList();

public:
	NetResourceList& operator=(NetResourceList& other);
	static wchar_t* CopyText(const wchar_t* Text);

	static void InitNetResource(NETRESOURCE& Res);
	static void DeleteNetResource(NETRESOURCE& Res);
	static void CopyNetResource(NETRESOURCE& Dest, const NETRESOURCE& Src);

	void Clear();
	BOOL Enumerate(DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCE lpNetResource);

	void Push(NETRESOURCE& Res);
	NETRESOURCE* Top();
	void Pop();

	unsigned int Count() { return ResCount; }

	NETRESOURCE& operator[](int index) { return ResList[index]; }
};

struct NameAndPassInfo
{
	wchar_t* Title;
	wchar_t* Name;
	wchar_t* Password;
	LPBOOL pRemember;
	wchar_t* szFavoritePath;
};

class NetBrowser
{
private:
	NetResourceList NetList; // list of resources in the current folder
	wchar_t NetListRemoteName[MAX_PATH]; // remote name of the resource stored in NetList
	NetResourceList ConnectedList; // list of resources mapped to local drives
	NetResourceList RootResources; // stack of resources above the current level
	// (used in non-MS Windows networks only)
	NETRESOURCE CurResource; // NETRESOURCE describing the current location
	NETRESOURCE* PCurResource; // points to CurResource or nullptr (if at root)

	BOOL ChangeDirSuccess;
	BOOL OpenFromFilePanel;
	int ReenterGetFindData;
	wchar_t CmdLinePath[MAX_PATH]; // path passed when invoking us from command line
	wchar_t m_PanelMode[2]; // current start panel mode

#ifdef NETWORK_LOGGING
		static FILE *LogFile;
		static int LogFileRef;
		static void LogNetResource(NETRESOURCE &Res);
		static void OpenLogFile(const wchar_t *lpFileName);
		static void CloseLogfile();
#endif

private:
	void RemoveItems();
#ifdef NETWORK_LOGGING
		void LogData(const wchar_t* Data);
#endif
	static void DisconnectFromServer(NETRESOURCE* nr);
	BOOL ChangeToDirectory(const wchar_t* Dir, OPERATION_MODES opmodes, bool IsExplicit);
	void ManualConnect();
	BOOL CancelConnection(const wchar_t* RemoteName);
	BOOL GetDriveToDisconnect(const wchar_t* RemoteName, wchar_t* LocalName);
	BOOL ConfirmCancelConnection(wchar_t* LocalName, wchar_t* RemoteName, int& UpdateProfile);
	BOOL NeedConfirmCancelConnection();
	BOOL HandsOffDisconnectDrive(const wchar_t* LocalName);
	void GetLocalName(wchar_t* RemoteName, wchar_t* LocalName);
	static int GetNameAndPassword(NameAndPassInfo* passInfo);
	void GetRemoteName(NETRESOURCE* NetRes, wchar_t* RemoteName);
	BOOL EnumerateNetList();
	void GetHiddenShares();
	void GetFreeLetter(DWORD& DriveMask, wchar_t* DiskName);
	BOOL IsReadable(const wchar_t* Remote);
	int GotoComputer(const wchar_t* Dir);
	void SetCursorToShare(wchar_t* Share);
	BOOL MapNetworkDrive(const wchar_t* RemoteName, BOOL AskDrive, BOOL Permanent);
	BOOL AskMapDrive(wchar_t* NewLocalName, BOOL& Permanent);
	void FileNames2Clipboard(BOOL ToCommandLine);

public:
	NetBrowser();
	~NetBrowser();

public:
	void CreateFavSubFolder();
	int GetFindData(PluginPanelItem** pPanelItem, size_t* pItemsNumber, OPERATION_MODES OpMode);
	void FreeFindData(PluginPanelItem* PanelItem, int ItemsNumber);
	void GetOpenPanelInfo(OpenPanelInfo* Info);
	int SetDirectory(const wchar_t* Dir, OPERATION_MODES OpMode);
	int DeleteFiles(PluginPanelItem* PanelItem, int ItemsNumber, OPERATION_MODES OpMode);
	int ProcessKey(const INPUT_RECORD* Rec);
	int ProcessEvent(intptr_t Event, void* Param);
	void SetOpenFromCommandLine(wchar_t* ShareName);
	BOOL SetOpenFromFilePanel(wchar_t* ShareName);
	void GotoLocalNetwork();

	BOOL GotoFavorite(wchar_t* lpPath);
	BOOL EditFavorites();

	static int AddConnection(NETRESOURCE* nr, int Remember = TRUE);
	static int AddConnectionExplicit(NETRESOURCE* nr, int Remember = TRUE);
	static int AddConnectionWithLogon(NETRESOURCE* nr, wchar_t* Name, wchar_t* Password, int Remember = TRUE);
	static int AddConnectionFromFavorites(NETRESOURCE* nr, int Remember = TRUE);

	static BOOL GetResourceInfo(wchar_t* SrcName, LPNETRESOURCE DstNetResource);
	static BOOL GetResourceParent(NETRESOURCE& SrcRes, LPNETRESOURCE DstNetResource);
	static BOOL IsMSNetResource(const NETRESOURCE& Res);
	static BOOL IsResourceReadable(NETRESOURCE& Res);
	//static BOOL GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent);
};

extern NetResourceList* CommonRootResources;
extern BOOL SavedCommonRootResources;

#endif // __NETCLASS_HPP__
