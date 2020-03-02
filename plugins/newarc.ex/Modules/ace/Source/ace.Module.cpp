#include "ace.h"

// {F39E8435-EF9F-4C89-B882-F4ACA6035CFD}
MY_DEFINE_GUID(CLSID_ModuleACE, 0xf39e8435, 0xef9f, 0x4c89, 0xb8, 0x82, 0xf4, 0xac, 0xa6, 0x3, 0x5c, 0xfd);

AceModule::AceModule()
{
	m_pPlugin = NULL;
	m_pPluginInfo = NULL;
}

bool AceModule::Load()
{
	bool bResult = false;

	m_pPlugin = new AcePlugin;

	if ( m_pPlugin )
	{
		string strModuleName = Info.ModuleName;

		CutToSlash(strModuleName);
		strModuleName += _T("unacev2.dll");

		if ( m_pPlugin->Load(strModuleName) )
			bResult = true;
		else
		{
			CutToSlash(strModuleName);
			strModuleName += _T("acev2.dll");

			if ( m_pPlugin->Load(strModuleName) )
				bResult = true;
		}
	}

	if ( bResult )
	{
		m_pPluginInfo = new ArchivePluginInfo;
		memset(m_pPluginInfo, 0, sizeof(ArchivePluginInfo));

		m_pPluginInfo->uid = m_pPlugin->GetUID();
		m_pPluginInfo->lpModuleName = StrDuplicate(m_pPlugin->GetModuleName());
		m_pPluginInfo->uFormats = m_pPlugin->GetNumberOfFormats();
		m_pPluginInfo->pFormats = m_pPlugin->GetFormats();
		m_pPluginInfo->dwFlags = APF_SUPPORT_SINGLE_FORMAT_QUERY;
	}

	return bResult;
}

AceModule::~AceModule()
{
	if ( m_pPlugin )
		delete m_pPlugin;

	if ( m_pPluginInfo )
	{
		StrFree((void*)m_pPluginInfo->lpModuleName);
		delete m_pPluginInfo;
	}
}

const GUID& AceModule::GetUID()
{
	return CLSID_ModuleACE;
}

const ArchivePluginInfo* AceModule::GetPlugins()
{
	return m_pPluginInfo;
}

unsigned int AceModule::GetNumberOfPlugins()
{
	return 1;
}

bool AceModule::QueryArchive(const QueryArchiveStruct* pQAS, ArchiveQueryResult* pResult)
{
	return m_pPlugin->QueryArchive(
			pQAS->lpFileName, 
			pQAS->pBuffer,
			pQAS->dwBufferSize,
			pResult
			);
}

AceArchive* AceModule::OpenCreateArchive(
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	return m_pPlugin->OpenCreateArchive(
			lpFileName,
			hCallback,
			pfnCallback,
			bCreate
			);
}

void AceModule::CloseArchive(AceArchive* pArchive)
{
	m_pPlugin->CloseArchive(pArchive);
}