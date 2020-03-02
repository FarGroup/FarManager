#include "ma.h"

MaPlugin::MaPlugin(const GUID& uid)
{
	m_uid = uid;
	m_hModule = NULL;

	m_pfnLoadFormatModule = nullptr;
	m_pfnIsArchive = nullptr;
	m_pfnOpenArchive = nullptr;
	m_pfnGetArcItem = nullptr;
	m_pfnCloseArchive = nullptr;
	m_pfnGetFormatName = nullptr;
	m_pfnGetDefaultCommands = nullptr;
	m_pfnSetFarInfo = nullptr;
	m_pfnGetSFXPos = nullptr;
}

bool MaPlugin::Load(
		const TCHAR* lpModuleName, 
		oldfar::PluginStartupInfo* pInfo, 
		oldfar::FARSTANDARDFUNCTIONS* pFSF
		)
{
	m_strModuleName = lpModuleName;

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnLoadFormatModule = (PLUGINLOADFORMATMODULE)GetProcAddress(m_hModule, "LoadFormatModule");
		m_pfnIsArchive = (PLUGINISARCHIVE)GetProcAddress(m_hModule, "IsArchive");
		m_pfnOpenArchive = (PLUGINOPENARCHIVE)GetProcAddress(m_hModule, "OpenArchive");
		m_pfnGetArcItem = (PLUGINGETARCITEM)GetProcAddress(m_hModule, "GetArcItem");
		m_pfnCloseArchive = (PLUGINCLOSEARCHIVE)GetProcAddress(m_hModule, "CloseArchive");
		m_pfnGetFormatName = (PLUGINGETFORMATNAME)GetProcAddress(m_hModule, "GetFormatName");
		m_pfnGetDefaultCommands = (PLUGINGETDEFAULTCOMMANDS)GetProcAddress(m_hModule, "GetDefaultCommands");
		m_pfnSetFarInfo = (PLUGINSETFARINFO)GetProcAddress(m_hModule, "SetFarInfo");
		m_pfnGetSFXPos = (PLUGINGETSFXPOS)GetProcAddress(m_hModule, "GetSFXPos");

		if ( m_pfnGetFormatName && m_pfnIsArchive && m_pfnOpenArchive && m_pfnGetArcItem ) 
		{
#ifdef UNICODE
			char* lpFileNameA = UnicodeToAnsi(lpModuleName);
#else
			const char* lpFileNameA = lpModuleName;
#endif
			if ( m_pfnLoadFormatModule )
				m_pfnLoadFormatModule(lpFileNameA);

			if ( m_pfnSetFarInfo )
			{
				oldfar::PluginStartupInfo _Info;
				oldfar::FARSTANDARDFUNCTIONS _FSF;
				
				_Info = *pInfo;
				_FSF  = *pFSF;
				_Info.FSF = &_FSF;
				
				strcpy (_Info.ModuleName, lpFileNameA);
				m_pfnSetFarInfo(&_Info);
			}

#ifdef UNICODE
			free(lpFileNameA);
#endif

			int index = 0;
			char Name[100];
			char DefaultExtention[100];  

			while ( true )
			{
				*Name = 0;
				*DefaultExtention = 0;

				if ( m_pfnGetFormatName(index, Name, DefaultExtention) )
				{
					ArchiveFormatInfo info;
					//memset(&info, 0, sizeof(ArchiveFormatInfo));

					info.dwFlags = AFF_SUPPORT_DEFAULT_COMMANDS;

					string strName;

#ifdef UNICODE
					strName.SetData(Name, CP_OEMCP);
#else
					strName = Name;
#endif
					//oops, unicode & ansi
					
					strName += _T(" [fmt]");


					info.lpName = StrDuplicate(strName);
#ifdef UNICODE
					info.lpDefaultExtention = AnsiToUnicode(DefaultExtention);
#else
					info.lpDefaultExtention = StrDuplicate(DefaultExtention);
#endif

					info.uid = CreateFormatUID(m_uid, info.lpName);
					
					m_pFormatInfo.add(info);

					index++;
				}
				else
					break;
			}

			return true;
		}
	}

	return false;
}

const GUID& MaPlugin::GetUID()
{
	return m_uid;
}

const TCHAR* MaPlugin::GetModuleName()
{
	return m_strModuleName;
}

const ArchiveFormatInfo* MaPlugin::GetFormats()
{
	return m_pFormatInfo.data();
}

unsigned int MaPlugin::GetNumberOfFormats()
{
	return m_pFormatInfo.count();
}

MaPlugin::~MaPlugin()
{
	for (unsigned int i = 0; i < m_pFormatInfo.count(); i++)
	{
		StrFree((void*)m_pFormatInfo[i].lpName);
		StrFree((void*)m_pFormatInfo[i].lpDefaultExtention);
	}

	FreeLibrary (m_hModule);
}

int MaPlugin::QueryArchives(const unsigned char* pBuffer, DWORD dwBufferSize, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result)
{
	MaArchive* pResult = NULL;

	OEM_NAME_CREATE_CONST(lpFileName);

	if ( m_pfnIsArchive(lpFileNameA, pBuffer, dwBufferSize) )
	{
		//попробуем сделать страшное...
		int nArcType = -1;

		if ( m_pfnOpenArchive(lpFileNameA, &nArcType) )
		{
			if ( (nArcType >= 0) && (nArcType < m_pFormatInfo.count()) )
			{
				ArchiveQueryResult* pResult = new ArchiveQueryResult;

				pResult->uidPlugin = m_uid;
				pResult->uidFormat = m_pFormatInfo[nArcType].uid;

				result.add(pResult);
			}

			struct ArcInfo MaArcInfo;

			if ( m_pfnCloseArchive )
				m_pfnCloseArchive (&MaArcInfo);
		}
	}

	OEM_NAME_DELETE_CONST(lpFileName);

	return NULL;
}


MaArchive* MaPlugin::OpenArchive(const GUID& uid, const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback)
{
	return new MaArchive(this, uid, lpFileName, hCallback, pfnCallback);
}

void MaPlugin::CloseArchive(MaArchive* pArchive)
{
	delete pArchive;
}

bool MaPlugin::GetDefaultCommand(const GUID& uid, int nCommand, const TCHAR** ppCommand)
{
	for (unsigned int i = 0; i < m_pFormatInfo.count(); i++)
	{
		if ( m_pFormatInfo[i].uid == uid )
		{
			char szBuffer[1024];

			if ( m_pfnGetDefaultCommands(i, nCommand, szBuffer) )
			{
				m_strCommandBuffer.SetData(szBuffer, CP_OEMCP);
				*ppCommand = m_strCommandBuffer;
				return true;
			}

			break;
		}
	}

	return false;
}


bool MaPlugin::OpenArchive(const TCHAR* lpFileName)
{
	OEM_NAME_CREATE_CONST(lpFileName);

	int nType = 0;

	bool bResult = m_pfnOpenArchive(lpFileNameA, &nType);

	OEM_NAME_DELETE_CONST(lpFileName);
	
	return bResult;
}

void MaPlugin::CloseArchive(ArcInfo* pInfo)
{
	if ( m_pfnCloseArchive)
		m_pfnCloseArchive(pInfo);
}

int MaPlugin::GetArchiveItem(ArchiveItem* pItem, ArcItemInfo* pInfo)
{
	oldfar::PluginPanelItem item;
	memset(&item, 0, sizeof(oldfar::PluginPanelItem)); //модули глючат и считают, что тут нули

	ArcItemInfo MaItemInfo;
	memset(&MaItemInfo, 0, sizeof(ArcItemInfo));

	int nResult = m_pfnGetArcItem(&item, &MaItemInfo);

	if ( nResult == GETARC_SUCCESS )
	{
#ifdef UNICODE
		pItem->lpFileName = AnsiToUnicode(item.FindData.cFileName);
		pItem->lpAlternateFileName = AnsiToUnicode(item.FindData.cAlternateFileName);
#else
		pItem->lpFileName = StrDuplicate(item.FindData.cFileName);
		pItem->lpAlternateFileName = StrDuplicate(item.FindData.cFileName);
#endif

		pItem->nFileSize = ((unsigned __int64)item.FindData.nFileSizeHigh << 32)+(unsigned __int64)item.FindData.nFileSizeLow;
		pItem->dwFileAttributes = item.FindData.dwFileAttributes;

		pItem->nPackSize = ((unsigned __int64)item.PackSizeHigh << 32)+(unsigned __int64)item.PackSize;
		pItem->dwCRC32 = item.CRC32;

		if ( MaItemInfo.Encrypted )
			pItem->dwFlags |= AIF_CRYPTED;

		if ( MaItemInfo.Solid )
			pItem->dwFlags |= AIF_SOLID;

		if ( pInfo )
			memcpy(pInfo, &MaItemInfo, sizeof(ArcItemInfo));

		return E_SUCCESS;
	}

	return ConvertResult (nResult);
}

void MaPlugin::FreeArchiveItem(ArchiveItem* pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);
}


int MaPlugin::ConvertResult(int nResult)
{
	switch (nResult)
	{
		case GETARC_EOF:
			return E_EOF;
		case GETARC_SUCCESS:
			return E_SUCCESS;
		case GETARC_BROKEN:
			return E_BROKEN;
		case GETARC_UNEXPEOF:
			return E_UNEXPECTED_EOF;
		case GETARC_READERROR:
			return E_READ_ERROR;
	}

	return E_UNEXPECTED_EOF;
}
