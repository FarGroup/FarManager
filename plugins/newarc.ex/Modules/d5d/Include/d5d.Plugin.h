#pragma once

#define ARCHIVE_OPERATION_ADD 1
#define ARCHIVE_OPERATION_DELETE 2

struct D5DArchiveHandle {
	int nFilesCount;
};

class D5DPlugin {

public:

	DWORD m_dwDUDIVersion; //interface version
	DWORD m_dwVersion; //driver version

	bool m_bSpecialCase; //а вот это для глюков оригинального плагина

	GUID m_uid;

	HMODULE m_hModule;

	ArchiveFormatInfo* m_pFormatInfo;

	PBYTE m_pfnGetMsgThunk;
	PBYTE m_pfnPercentThunk;
	PBYTE m_pfnMessageBoxThunk;

public:

	string m_strModuleName;

	//эти не менялись
	DUDIVERSION m_pfnDUDIVersion;
	GETNUMVERSION m_pfnGetNumVersion;

	CLOSEFORMAT m_pfnCloseFormat;
	GETCURRENTDRIVERINFO m_pfnGetCurrentDriverInfo;
	GETENTRY m_pfnGetEntry;
	GETERRORINFO m_pfnGetErrorInfo;
	GETDRIVERINFO m_pfnGetDriverInfo;
	ISFORMAT m_pfnIsFormat;

	//эти новые
	DUDIVERSIONEX m_pfnDUDIVersionEx;
	EXTRACTFILETOSTREAM m_pfnExtractFileToStream; //not supported, TStream needed
	INITPLUGINEX5 m_pfnInitPluginEx5;

	//насрать, что имена одинаковые. на самом деле это могут быть разные функции в разных
	//версиях DUDI

	INITPLUGIN m_pfnInitPlugin;
	INITPLUGIN3 m_pfnInitPlugin3;

	EXTRACTFILE m_pfnExtractFile;
	EXTRACTFILE2 m_pfnExtractFile2;


	READFORMAT m_pfnReadFormat;
	READFORMAT2 m_pfnReadFormat2;

	//схватка 3-х якодзун
	ABOUTBOX m_pfnAboutBox;
	ABOUTBOX2 m_pfnAboutBox2;
	ABOUTBOX3 m_pfnAboutBox3;

	CONFIGUREBOX m_pfnConfigureBox;
	CONFIGUREBOX2 m_pfnConfigureBox2;
	CONFIGUREBOX3 m_pfnConfigureBox3;

public:

	D5DPlugin(const GUID& uid);
	~D5DPlugin();

	bool Load(const TCHAR* lpModuleName);

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	unsigned int GetNumberOfFormats();
	const ArchiveFormatInfo* GetFormats();

	int QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);

	D5DArchive* OpenArchive(const GUID& uidFormat, const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	void CloseArchive(D5DArchive* pArchive);

//d5d

	int ReadFormat(const TCHAR* lpFileName);
	void CloseFormat();

	int GetEntry(int& nIndex, ArchiveItem* pItem);
	void FreeEntry(ArchiveItem* pItem);

	int Extract(const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpDeskDiskPath, const TCHAR* lpPathInArchive);

	int About();
	int Configure();

	void InitPlugin(); 
	DWORD GetNumVersion();

private:

	void __stdcall GetMsg(ShortString& src, ShortString& dest);
	void __stdcall Percent(unsigned char p);
	void __stdcall MessageBox(const char* lpTitle, const char* lpMessage);

private:

	int ConvertResult(int nResult);
};
