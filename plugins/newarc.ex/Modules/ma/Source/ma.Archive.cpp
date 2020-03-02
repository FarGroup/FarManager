#include "ma.h"

MaArchive::MaArchive(
		MaPlugin* pPlugin, 
		const GUID& uid, 
		const TCHAR *lpFileName, 
		HANDLE hCallback, 
		ARCHIVECALLBACK pfnCallback
		)
{
	m_uid = uid;
	m_pPlugin = pPlugin;

	m_strFileName = lpFileName;
	m_hArchive = NULL;

	m_hCallback = hCallback;
	m_pfnCallback = pfnCallback;

	m_bArchiveInfoAdded = false;
	m_bItemInfoAdded = false;
}

const GUID& MaArchive::GetUID()
{
	return m_uid;
}

MaArchive::~MaArchive()
{
	for (unsigned int i = 0; i < m_pArchiveInfo.count(); i++)
	{
		StrFree((void*)m_pArchiveInfo[i].lpName);
		StrFree((void*)m_pArchiveInfo[i].lpValue);
	}
}


bool MaArchive::StartOperation(int nOperation, bool bInternal)
{
	if ( nOperation == OPERATION_LIST )
		return m_pPlugin->OpenArchive(m_strFileName);

	return false;
}

bool MaArchive::EndOperation(int nOperation, bool bInternal)
{
	if ( nOperation == OPERATION_LIST )
	{
		ArcInfo info;
		m_pPlugin->CloseArchive(&info);

		if ( !m_bArchiveInfoAdded )
		{
			string strValue;
			ArchiveInfoItem* item;
			
			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("SFX size"));
			
			strValue.Format(_T("%d"), info.SFXSize);
			item->lpValue = StrDuplicate(strValue);

			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Volume number"));

			strValue.Format(_T("%d"), info.Volume);
			item->lpValue = StrDuplicate(strValue);

			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Comment presend"));
			item->lpValue = StrDuplicate(info.Comment?_T("Yes"):_T("No"));

			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Locked"));
			item->lpValue = StrDuplicate(info.Lock?_T("Yes"):_T("No"));
		
			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("AV present"));
			item->lpValue = StrDuplicate(((info.Flags & AF_AVPRESENT) == AF_AVPRESENT)?_T("Yes"):_T("No"));

			m_bArchiveInfoAdded = true;
		}
	}

	return true;
}

int MaArchive::GetArchiveItem(ArchiveItem* pItem)
{
	ArcItemInfo info;

	int nResult = m_pPlugin->GetArchiveItem(pItem, &info);

	if ( (nResult == NAERROR_SUCCESS) && !m_bItemInfoAdded )
	{
		string strValue;
		ArchiveInfoItem* item;
		TCHAR* lpTemp;

		if ( *info.HostOS )
		{
			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Host OS"));
			
#ifdef UNICODE
			lpTemp = AnsiToUnicode(info.HostOS);
			item->lpValue = StrDuplicate(lpTemp);
			free(lpTemp);
#else
			item->lpValue = StrDuplicate(info.HostOS);
#endif
		}

		if ( *info.Description )
		{
			item = m_pArchiveInfo.add();
			item->lpName = StrDuplicate(_T("Description"));
			
#ifdef UNICODE
			lpTemp = AnsiToUnicode(info.Description);
			item->lpValue = StrDuplicate(lpTemp);
			free(lpTemp);
#else
			item->lpValue = StrDuplicate(info.Description);
#endif
		}

		item = m_pArchiveInfo.add();
		item->lpName = StrDuplicate(_T("Solid"));
		item->lpValue = StrDuplicate(info.Solid?_T("Yes"):_T("No"));

		item = m_pArchiveInfo.add();
		item->lpName = StrDuplicate(_T("Encrypted"));
		item->lpValue = StrDuplicate(info.Solid?_T("Yes"):_T("No"));

		item = m_pArchiveInfo.add();
		item->lpName = StrDuplicate(_T("Dictionary size"));

		strValue.Format(_T("%d Kb"), info.DictSize);
		item->lpValue = StrDuplicate(strValue);

		item = m_pArchiveInfo.add();
		item->lpName = StrDuplicate(_T("Unpacker version"));

		strValue.Format(_T("%1d.%1d"), HIBYTE(info.UnpVer), LOBYTE(info.UnpVer));
		item->lpValue = StrDuplicate(strValue);

		m_bItemInfoAdded = true;
	}

	return nResult;
}

void MaArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	return m_pPlugin->FreeArchiveItem(pItem);
}


LONG_PTR MaArchive::Callback(int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}

int MaArchive::GetArchiveInfo(const ArchiveInfoItem** pItems)
{
	if ( m_pArchiveInfo.count() )
	{
		*pItems = m_pArchiveInfo.data();
		return m_pArchiveInfo.count();
	}

	return 0;
}

