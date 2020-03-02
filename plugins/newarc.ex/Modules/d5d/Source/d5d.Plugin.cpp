#include "d5d.h"

TCHAR* ShortStringToTCHAR(const ShortString& str)
{
	TCHAR* lpResult = NULL;
	char* lpBuffer = (char*)malloc(str.Length+1);

	lpBuffer[str.Length] = 0;
	memcpy(lpBuffer, str.Data, str.Length);

#ifdef UNICODE
	lpResult = AnsiToUnicode(lpBuffer, CP_ACP);
	free(lpBuffer);
#else
	lpResult = lpBuffer;
	CharToOemA(lpResult, lpResult);
#endif

	return lpResult;
}

extern D5DLanguage lng;

/*
void __stdcall GetLanguageString(ShortString& src, ShortString& dst)
{
	const char* lpString = lng.GetMessage(src.Data);

	dst.Length = strlen(lpString);
	strcpy((char*)dst.Data, lpString);
}
*/
/*

//как это работает - одному богу известно
void __stdcall ShowMessageBox(const char* lpStr1, const char* lpStr2)
{
	MessageBoxA(0, lpStr2, lpStr1, MB_OK);
}

void __stdcall Percent(unsigned char p)
{
	//MessageBoxA(0, "percent", "xxx", MB_OK);
}

/*
__declspec (naked) void GetLanguageStringThunk()
{
	__asm {
		push edx;
		push eax;
		call GetLanguageString;
		ret;
	};
}
*/
/*
__declspec (naked) void ShowMessageBoxThunk()
{
	__asm {
		push edx;
		push eax;
		call ShowMessageBox;
		ret;
	};
}

__declspec (naked) void PercentThunk()
{
	__asm {
		push eax;
		call Percent;
		ret;
	};
}
*/

D5DPlugin::D5DPlugin(const GUID& uid)
{
	m_pfnDUDIVersion = 0;
	m_pfnGetNumVersion = 0;
	m_pfnCloseFormat = 0;
	m_pfnGetCurrentDriverInfo = 0;
	m_pfnGetEntry = 0;
	m_pfnGetErrorInfo = 0;
	m_pfnGetDriverInfo = 0;
	m_pfnIsFormat = 0;
	m_pfnDUDIVersionEx = 0;
	m_pfnExtractFileToStream = 0; //not supported, TStream needed
	m_pfnInitPluginEx5 = 0;
	m_pfnInitPlugin = 0;
	m_pfnInitPlugin3 = 0;
	m_pfnExtractFile = 0;
	m_pfnExtractFile2 = 0;
	m_pfnReadFormat = 0;
	m_pfnReadFormat2 = 0;
	m_pfnAboutBox = 0;
	m_pfnAboutBox2 = 0;
	m_pfnAboutBox3 = 0;
	m_pfnConfigureBox = 0;
	m_pfnConfigureBox2 = 0;
	m_pfnConfigureBox3 = 0;

	m_uid = uid;

	CreateClassThunkRegister(D5DPlugin, GetMsg, m_pfnGetMsgThunk);
	CreateClassThunkRegister(D5DPlugin, Percent, m_pfnPercentThunk);
	CreateClassThunkRegister(D5DPlugin, MessageBox, m_pfnMessageBoxThunk);

	int i = 0;
}

void __stdcall D5DPlugin::GetMsg(ShortString& src, ShortString& dst)
{
	const char* lpString = lng.GetMessage(src.Data);

	dst.Length = strlen(lpString);
	strcpy((char*)dst.Data, lpString);
}


void __stdcall D5DPlugin::Percent(unsigned char p)
{
}

void __stdcall D5DPlugin::MessageBox(const char* lpStr1, const char* lpStr2)
{
	::MessageBoxA(0, lpStr2, lpStr1, MB_OK);
}


bool D5DPlugin::Load(const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnDUDIVersion = (DUDIVERSION)GetProcAddress(m_hModule, "DUDIVersion");
		m_pfnDUDIVersionEx = (DUDIVERSIONEX)GetProcAddress(m_hModule, "DUDIVersionEx");

		if ( m_pfnDUDIVersion ) //нет ножек, нет варенья
		{
			m_dwDUDIVersion = m_pfnDUDIVersion();

			if ( m_pfnDUDIVersionEx )
				m_dwDUDIVersion = m_pfnDUDIVersionEx(4); //no rtf in messageboxes, v5!! 

			if ( (m_dwDUDIVersion >= 1) && (m_dwDUDIVersion <= 5) ) //про 6 мы ничего пока не знаем
			{
			 	m_pfnIsFormat = (ISFORMAT)GetProcAddress(m_hModule, "IsFormat");
				m_pfnCloseFormat = (CLOSEFORMAT)GetProcAddress(m_hModule, "CloseFormat");
				m_pfnGetCurrentDriverInfo = (GETCURRENTDRIVERINFO)GetProcAddress(m_hModule, "GetCurrentDriverInfo");
				m_pfnGetEntry = (GETENTRY)GetProcAddress(m_hModule, "GetEntry");
				m_pfnGetErrorInfo = (GETERRORINFO)GetProcAddress(m_hModule, "GetErrorInfo");
				m_pfnGetNumVersion = (GETNUMVERSION)GetProcAddress(m_hModule, "GetNumVersion");
				m_pfnGetDriverInfo = (GETDRIVERINFO)GetProcAddress(m_hModule, "GetDriverInfo");

				//в оригинальном коде еще и в загрузчике глюки. но это ничего ))

				if ( m_dwDUDIVersion == 1 )
				{
					m_pfnReadFormat = (READFORMAT)GetProcAddress(m_hModule, "ReadFormat");
					m_pfnAboutBox = (ABOUTBOX)GetProcAddress(m_hModule, "AboutBox");
					m_pfnConfigureBox = (CONFIGUREBOX)GetProcAddress(m_hModule, "ConfigBox");
					m_pfnExtractFile = (EXTRACTFILE)GetProcAddress(m_hModule, "ExtractFile");
				}
				else

				if ( m_dwDUDIVersion == 2 )
				{
					m_pfnReadFormat2 = (READFORMAT2)GetProcAddress(m_hModule, "ReadFormat");
					m_pfnAboutBox2 = (ABOUTBOX2)GetProcAddress(m_hModule, "AboutBox");
					m_pfnConfigureBox2 = (CONFIGUREBOX2)GetProcAddress(m_hModule, "ConfigBox");

					m_pfnExtractFile = (EXTRACTFILE)GetProcAddress(m_hModule, "ExtractFile");
					m_pfnInitPlugin = (INITPLUGIN)GetProcAddress(m_hModule, "InitPlugin");
				}
				else
				{
					if ( m_dwDUDIVersion == 3 )
						m_pfnExtractFile = (EXTRACTFILE)GetProcAddress(m_hModule, "ExtractFile");
					else //4, 5
					{
						m_pfnExtractFile2 = (EXTRACTFILE2)GetProcAddress(m_hModule, "ExtractFile");
						m_pfnExtractFileToStream = (EXTRACTFILETOSTREAM)GetProcAddress(m_hModule, "ExtractFileToStream");
					}

					m_pfnReadFormat2 = (READFORMAT2)GetProcAddress(m_hModule, "ReadFormat");
					m_pfnAboutBox3 = (ABOUTBOX3)GetProcAddress(m_hModule, "AboutBox");
					m_pfnConfigureBox3 = (CONFIGUREBOX3)GetProcAddress(m_hModule, "ConfigBox");
					m_pfnInitPlugin3 = (INITPLUGIN3)GetProcAddress(m_hModule, "InitPlugin");

					if ( m_dwDUDIVersion == 5 )
						m_pfnInitPluginEx5 = (INITPLUGINEX5)GetProcAddress(m_hModule, "InitPluginEx5");
				}
			}
		}

		if ( m_pfnDUDIVersion && m_pfnGetDriverInfo ) //loaded ok??
		{
			m_dwVersion = GetNumVersion();

			m_bSpecialCase = false;

			//upcase, no?
			if ( !FSF.LStricmp(_T("drv_zip.d5d"), FSF.PointToName(m_strModuleName)) && (m_dwDUDIVersion == 4) )
				m_bSpecialCase = true;

			InitPlugin();

			/*
			DriverInfo* pInfo = new DriverInfo; //только не в стеке, ааа!!!

			m_pfnGetDriverInfo(pInfo);

			for (int i = 0; i < pInfo->NumFormats; i++)
			{
				ArchiveFormatInfo* format = m_pFormatInfo.add();

				format->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT;

				format->lpName = ShortStringToTCHAR(pInfo->Formats[i].Name); //не, имя есть, но там такое полотно...

				TCHAR* p = (TCHAR*)_tcschr(format->lpName, _T('('));

				if ( p )
				{
					p--;
					*p = 0;
				}

				format->lpDefaultExtention = ShortStringToTCHAR(pInfo->Formats[i].Extensions);

				format->uid = CreateFormatUID(m_uid, format->lpName);
			}

			delete pInfo;
			*/
			m_pFormatInfo = new ArchiveFormatInfo;
			memset(m_pFormatInfo, 0, sizeof(ArchiveFormatInfo));

			m_pFormatInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_HAS_NO_DEFAULT_COMMANDS|AFF_NEED_EXTERNAL_NOTIFICATIONS;

			string strName = FSF.PointToName(m_strModuleName);

			strName += _T(" [D5D]");

			m_pFormatInfo->lpName = StrDuplicate(strName);
			m_pFormatInfo->uid = CreateFormatUID(m_uid, strName);


			return true;
		}
	}

	return false;
}

const GUID& D5DPlugin::GetUID()
{
	return m_uid;
}

const TCHAR* D5DPlugin::GetModuleName()
{
	return m_strModuleName;
}

unsigned int D5DPlugin::GetNumberOfFormats()
{
	return 1;//m_pFormatInfo.count();
}

const ArchiveFormatInfo* D5DPlugin::GetFormats()
{
	return m_pFormatInfo; //m_pFormatInfo.data();
}

DWORD D5DPlugin::GetNumVersion()
{
	if ( m_pfnGetNumVersion )
		return m_pfnGetNumVersion();

	return 0;
}

void D5DPlugin::InitPlugin()
{
	AnsiGuard guard;

	string strModulePath = Info.ModuleName;
	CutToSlash(strModulePath);

	ANSI_NAME_CREATE_EX(strModulePath, lpModulePath);

	ShortString str;
	str.Length = strlen(lpModulePathA);
	strcpy((char*)&str.Data, lpModulePathA);

	if ( m_pfnInitPlugin )
		m_pfnInitPlugin((PERCENTCALLBACK)m_pfnPercentThunk, (LANGUAGECALLBACK)m_pfnGetMsgThunk, str);
	else

	if ( m_pfnInitPlugin3 )
		m_pfnInitPlugin3((PERCENTCALLBACK)m_pfnPercentThunk, (LANGUAGECALLBACK)m_pfnGetMsgThunk, str, GetConsoleWindow(), NULL);

	if ( m_pfnInitPluginEx5 )
		m_pfnInitPluginEx5((MESSAGEBOXCALLBACK)m_pfnMessageBoxThunk); //и все упадет
	//m_pfnInitPluginEx5 не поддерживается. где я им AnsiString возьму

	ANSI_NAME_DELETE(lpModulePath);
}

D5DArchive* D5DPlugin::OpenArchive(const GUID& uidFormat, const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback)
{
	return new D5DArchive(this, uidFormat, lpFileName, hCallback, pfnCallback);
}

void D5DPlugin::CloseArchive(D5DArchive* pArchive)
{
	delete pArchive;
}


D5DPlugin::~D5DPlugin()
{
	ReleaseThunk(m_pfnGetMsgThunk);
	ReleaseThunk(m_pfnPercentThunk);
	ReleaseThunk(m_pfnMessageBoxThunk);

	if ( m_pFormatInfo )
	{
		StrFree((void*)m_pFormatInfo->lpName);
		StrFree((void*)m_pFormatInfo->lpDefaultExtention);

		delete m_pFormatInfo;
	}

	if ( m_hModule )
		FreeLibrary(m_hModule);
}


int D5DPlugin::ReadFormat(const TCHAR* lpFileName)
{
	AnsiGuard guard;

	int nResult = -2;

	ANSI_NAME_CREATE(lpFileName);

	if ( m_pfnReadFormat )
	{
		ShortString dFileName;

		dFileName.Length = strlen(lpFileNameA);
		strcpy((char*)&dFileName.Data, lpFileNameA); //BUGBUG

		nResult = m_pfnReadFormat(dFileName, (PERCENTCALLBACK)m_pfnPercentThunk, true);
	}
	else

	if ( m_pfnReadFormat2 )
	{
		ShortString dFileName;

		dFileName.Length = strlen(lpFileNameA);
		strcpy((char*)&dFileName.Data, lpFileNameA); //BUGBUG

		if ( m_bSpecialCase )
			nResult = ((READFORMAT)m_pfnReadFormat2)(dFileName, (PERCENTCALLBACK)m_pfnPercentThunk, true);
		else
			nResult = m_pfnReadFormat2(dFileName, true);
	}

	ANSI_NAME_DELETE(lpFileName);

	//тут надо получить информацию о том, что же это было

	return nResult;
}

void D5DPlugin::CloseFormat()
{
	if ( m_pfnCloseFormat )
		m_pfnCloseFormat();
}

int D5DPlugin::ConvertResult(int nResult)
{
/*	switch ( nResult )
	{
		case 0:
			return E_SUCCESS;
		case E_END_ARCHIVE:
			return E_EOF;
		case E_BAD_ARCHIVE:
			return E_BROKEN;
		case E_BAD_DATA:
			return E_UNEXPECTED_EOF;
		case E_EREAD:
			return E_READ_ERROR;
	}
*/
	return E_UNEXPECTED_EOF;
}


int D5DPlugin::GetEntry(int& nIndex, ArchiveItem* pItem)
{
	AnsiGuard guard;

	int nResult = 0;

	if ( m_pfnGetEntry )
	{
		FormatEntry* entry = new FormatEntry;

		while ( nIndex )
		{
			m_pfnGetEntry(entry);

			//бред, приходят пустые файлы, количество больше реального
			if ( entry->FileName.Length > 0 )
            //if ( (entry->Offset >= 0) && (entry->Size > 0) ) //взято из оригинального кода, но херня
			{
				pItem->lpFileName = ShortStringToTCHAR(entry->FileName);
				pItem->nFileSize = entry->Size;

				pItem->UserData = (DWORD_PTR)entry;

				nIndex--;

				return E_SUCCESS;
			}

			nIndex--;
		}

		delete entry;
	}

	return E_EOF;
}


void D5DPlugin::FreeEntry(ArchiveItem* pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);

	delete (FormatEntry*)pItem->UserData;
}


int D5DPlugin::QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
	AnsiGuard guard;

	bool bResult = false;

	ANSI_NAME_CREATE(lpFileName);

	ShortString dFileName;

	dFileName.Length = strlen(lpFileNameA);

	strcpy((char*)&dFileName.Data, lpFileNameA); //BUGBUG, check 255

	if ( m_pfnIsFormat )
		bResult = m_pfnIsFormat(dFileName, false);
 	
 	//else //вот тут надо подумать, может открыть/закрыть

	ANSI_NAME_DELETE(lpFileName);

	if ( bResult )
	{
		ArchiveQueryResult* pResult = new ArchiveQueryResult;

		pResult->uidFormat = m_pFormatInfo->uid;
		pResult->uidPlugin = m_uid;

		result.add(pResult);
	}

	return 0;
}



int D5DPlugin::Extract(
		const ArchiveItem* pItems, 
		int nItemsNumber, 
		const TCHAR* lpDestDiskPath, 
		const TCHAR* lpPathInArchive
		)
{
	AnsiGuard guard;

	string strFullName;

	for (int i = 0; i < nItemsNumber; i++)
	{
		FormatEntry* entry = (FormatEntry*)pItems[i].UserData;

		strFullName = lpDestDiskPath;
		strFullName += pItems[i].lpFileName;

		ANSI_NAME_CREATE_EX(pItems[i].lpFileName, pNameBuffer);
		ANSI_NAME_CREATE_EX(strFullName, pBuffer);
		
		ShortString justName;

		justName.Length = strlen(pNameBufferA);
		strcpy((char*)&justName.Data, pNameBufferA);

		ShortString name;

		name.Length = strlen(pBufferA);
		strcpy((char*)&name.Data, pBufferA); //BUGBUG

		bool bResult = false;

		CurrentDriverInfo* pInfo = new CurrentDriverInfo;
		
		m_pfnGetCurrentDriverInfo(pInfo);

		if ( pInfo->ExtractInternal )
		{
			apiCreateDirectoryForFile(strFullName);

		/*	for (int i = 0; i < justName.Length; i++)
				if ( justName.Data[i] == '\\' )
					justName.Data[i] == '/';
					*/
			if ( m_pfnExtractFile )
				m_pfnExtractFile(name, justName, entry->Offset, entry->Size, entry->DataX, entry->DataY);
			else
			
			if ( m_pfnExtractFile2 )
				m_pfnExtractFile2(name, justName, entry->Offset, entry->Size, entry->DataX, entry->DataY, true);
		}
		else
		{
			__debug(_T("d5d internal, please report"));
		}

		ANSI_NAME_DELETE(pBuffer);
		ANSI_NAME_DELETE(pNameBuffer);
	}

	return 1;
}


int D5DPlugin::Configure()
{
	AnsiGuard guard;

	if ( m_pfnConfigureBox )
		m_pfnConfigureBox(GetConsoleWindow(), (LANGUAGECALLBACK)m_pfnGetMsgThunk);
	else

	if ( m_pfnConfigureBox2 )
		m_pfnConfigureBox2(GetConsoleWindow());
	else

	if ( m_pfnConfigureBox3 )
		m_pfnConfigureBox3();


	return 0;
}

int D5DPlugin::About()
{
	AnsiGuard guard;

	if ( m_pfnAboutBox )
		m_pfnAboutBox(GetConsoleWindow(), (LANGUAGECALLBACK)m_pfnGetMsgThunk);
	else

	if ( m_pfnAboutBox2 )
		m_pfnAboutBox2(GetConsoleWindow());
	else

	if ( m_pfnAboutBox3 )
		m_pfnAboutBox3();


	return 0;
}

