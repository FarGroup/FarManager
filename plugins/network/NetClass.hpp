#ifndef __NETCLASS_HPP__
#define __NETCLASS_HPP__

#include <plugin.hpp>
#include <memory>
#include <string>
#include <vector>

#ifdef NETWORK_LOGGING
#include <stdio.h>
#endif

class NetResource
{
public:
	DWORD dwScope{};
	DWORD dwType{};
	DWORD dwDisplayType{};
	DWORD dwUsage{};
	std::unique_ptr<std::wstring> lpLocalName{};
	std::unique_ptr<std::wstring> lpRemoteName{};
	std::unique_ptr<std::wstring> lpComment{};
	std::unique_ptr<std::wstring> lpProvider{};

private:
	void copy(const NetResource& other)
	{
		dwScope = other.dwScope;
		dwType = other.dwType;
		dwDisplayType = other.dwDisplayType;
		dwUsage = other.dwUsage;
		lpLocalName = other.lpLocalName? std::make_unique<std::wstring>(*other.lpLocalName) : nullptr;
		lpRemoteName = other.lpRemoteName? std::make_unique<std::wstring>(*other.lpRemoteName) : nullptr;
		lpComment = other.lpComment? std::make_unique<std::wstring>(*other.lpComment) : nullptr;
		lpProvider = other.lpProvider? std::make_unique<std::wstring>(*other.lpProvider) : nullptr;
	}

public:
	NetResource() = default;

	NetResource(const NetResource& other)
	{
		copy(other);
	}

	NetResource& operator=(const NetResource& other)
	{
		if (this == &other)
			return *this;

		copy(other);

		return *this;
	}

	NetResource(NetResource&& other) noexcept = default;
	NetResource& operator=(NetResource&& other) noexcept = default;

	explicit NetResource(NETRESOURCE& other)
	{
		dwScope = other.dwScope;
		dwType = other.dwType;
		dwDisplayType = other.dwDisplayType;
		dwUsage = other.dwUsage;
		lpLocalName = other.lpLocalName? std::make_unique<std::wstring>(other.lpLocalName) : nullptr;
		lpRemoteName = other.lpRemoteName? std::make_unique<std::wstring>(other.lpRemoteName) : nullptr;
		lpComment = other.lpComment? std::make_unique<std::wstring>(other.lpComment) : nullptr;
		lpProvider = other.lpProvider? std::make_unique<std::wstring>(other.lpProvider) : nullptr;
	}

	static NETRESOURCE getNETRESOURCE(const NetResource* resource)
	{
		NETRESOURCE tmp{};
		if (resource)
		{
			tmp.dwScope = resource->dwScope;
			tmp.dwType = resource->dwType;
			tmp.dwDisplayType = resource->dwDisplayType;
			tmp.dwUsage = resource->dwUsage;
			tmp.lpLocalName = resource->lpLocalName? resource->lpLocalName->data() : nullptr;
			tmp.lpRemoteName = resource->lpRemoteName? resource->lpRemoteName->data() : nullptr;
			tmp.lpComment = resource->lpComment? resource->lpComment->data() : nullptr;
			tmp.lpProvider = resource->lpProvider? resource->lpProvider->data() : nullptr;
		}
		return tmp;
	}
};

class NetResourceList
{
private:
	std::vector<NetResource> ResList;

public:
	NetResourceList() = default;

	void Clear();
	bool Enumerate(DWORD dwScope, DWORD dwType, DWORD dwUsage, NetResource* lpNetResource);

	void Push(NetResource& Res);
	NetResource* Top();
	void Pop();

	size_t Count() { return ResList.size(); }

	NetResource& operator[](size_t index) { return ResList[index]; }
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
	NetResource CurResource; // NETRESOURCE describing the current location
	NetResource* PCurResource; // points to CurResource or nullptr (if at root)

	BOOL ChangeDirSuccess = TRUE;
	BOOL OpenFromFilePanel = FALSE;
	int ReenterGetFindData = 0;
	wchar_t CmdLinePath[MAX_PATH]; // path passed when invoking us from command line
	wchar_t m_PanelMode[2]; // current start panel mode

	void RemoveItems();
	static void DisconnectFromServer(const NetResource* nr);
	BOOL ChangeToDirectory(const wchar_t* Dir, OPERATION_MODES opmodes, bool IsExplicit);
	void ManualConnect();
	BOOL CancelConnection(const wchar_t* RemoteName);
	BOOL GetDriveToDisconnect(const wchar_t* RemoteName, wchar_t* LocalName);
	BOOL ConfirmCancelConnection(wchar_t* LocalName, wchar_t* RemoteName, int& UpdateProfile);
	BOOL NeedConfirmCancelConnection();
	BOOL HandsOffDisconnectDrive(const wchar_t* LocalName);
	void GetLocalName(const std::wstring* RemoteName, wchar_t* LocalName);
	static int GetNameAndPassword(NameAndPassInfo* passInfo);
	static void GetRemoteName(NetResource* NetRes, wchar_t* RemoteName);
	BOOL EnumerateNetList();
	void GetHiddenShares();
	void GetFreeLetter(DWORD& DriveMask, wchar_t* DiskName);
	static BOOL IsReadable(const wchar_t* Remote);
	int GotoComputer(const wchar_t* Dir);
	void SetCursorToShare(wchar_t* Share);
	BOOL MapNetworkDrive(const wchar_t* RemoteName, BOOL AskDrive, BOOL Permanent);
	BOOL AskMapDrive(wchar_t* NewLocalName, BOOL& Permanent);
	void FileNames2Clipboard(BOOL ToCommandLine);

#ifdef NETWORK_LOGGING
	static FILE* LogFile;
	static int LogFileRef;
	static void LogNetResource(const NetResource& Res);
	static void OpenLogFile(const wchar_t* lpFileName);
	static void CloseLogfile();
	void LogData(const wchar_t* Data);
#endif

public:
	NetBrowser();
	~NetBrowser();

	int GetFindData(PluginPanelItem** pPanelItem, size_t* pItemsNumber, OPERATION_MODES OpMode);
	void FreeFindData(PluginPanelItem* PanelItem, size_t ItemsNumber);
	void GetOpenPanelInfo(OpenPanelInfo* Info);
	int SetDirectory(const wchar_t* Dir, OPERATION_MODES OpMode);
	int DeleteFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, OPERATION_MODES OpMode);
	int ProcessKey(const INPUT_RECORD* Rec);
	int ProcessEvent(intptr_t Event, void* Param);
	bool SetOpenFromCommandLine(wchar_t* cmd);
	BOOL SetOpenFromFilePanel(wchar_t* ShareName);
	void GotoLocalNetwork();

	BOOL GotoFavorite(wchar_t* lpPath);
	BOOL EditFavorites();

	static int AddConnection(const NetResource& nr, int Remember = TRUE);
	static int AddConnectionExplicit(const NetResource* connectnr, int Remember = TRUE);
	static int AddConnectionWithLogon(const NetResource* nr, wchar_t* Name, wchar_t* Password, int Remember = TRUE);
	static int AddConnectionFromFavorites(const NetResource* nr, int Remember = TRUE);

	static BOOL GetResourceInfo(wchar_t* SrcName, NetResource& DstNetResource);
	static BOOL GetResourceParent(const NetResource& SrcRes, NetResource* DstNetResource);
	static BOOL IsMSNetResource(const NetResource& Res);
	static BOOL IsResourceReadable(const NetResource& Res);
	//static BOOL GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent);
};

extern NetResourceList* CommonRootResources;
extern BOOL SavedCommonRootResources;

#endif // __NETCLASS_HPP__
