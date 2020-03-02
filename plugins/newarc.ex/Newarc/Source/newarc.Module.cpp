#include "newarc.h"

const TCHAR* __stdcall ArchiveModule::GetMsg(INT_PTR nModuleNumber, int nID)
{
	ArchiveModule *pModule = (ArchiveModule*)nModuleNumber;

	if( pModule->m_pLanguage )
	{
		const TCHAR* lpResult = pModule->m_pLanguage->GetMsg(nID);

		if ( lpResult )
			return lpResult;
	}

	return _T("NO LNG STRING");
}

ArchiveModule::ArchiveModule(ArchiveModuleManager* pManager)
{
	m_pManager = pManager; 
	m_pLanguage = nullptr;
}

bool ArchiveModule::Load(const TCHAR *lpModuleName, const TCHAR *lpLanguage)
{
	m_strModuleName = lpModuleName;
	m_hModule = LoadLibrary(lpModuleName);

	if ( m_hModule )
	{
		// global

		m_pfnModuleEntry = (MODULEENTRY)GetProcAddress(m_hModule, "ModuleEntry");

		if ( m_pfnModuleEntry )
		{
			StartupInfo _si;
			FARSTANDARDFUNCTIONS _FSF;

			_si.Info = Info;

			_FSF  = FSF;

			_si.Info.FSF = &_FSF;

#ifdef UNICODE
			_si.Info.ModuleName = lpModuleName;
#else
			_tcscpy (_si.Info.ModuleName, lpModuleName);
#endif

			_si.Info.ModuleNumber = (INT_PTR)this;
			_si.Info.GetMsg = GetMsg;

			ReloadLanguage(lpLanguage); //хм, а не рано ли

			if ( m_pfnModuleEntry(FID_INITIALIZE, (void*)&_si) == NAERROR_SUCCESS )
			{
				ArchiveModuleInfo info;
				memset(&info, 0, sizeof(ArchiveModuleInfo));

				if ( m_pfnModuleEntry(FID_GETARCHIVEMODULEINFO, (void*)&info) == NAERROR_SUCCESS )
				{
					m_dwFlags = info.dwFlags;
					m_uid = info.uid;

					for (unsigned int i = 0; i < info.uPlugins; i++)
						m_pPlugins.add(new ArchivePlugin(this, &info.pPlugins[i]));
				
					return true;
				}
			}
		}
	}

	return false;
}

bool ArchiveModule::QueryCapability(DWORD dwFlags) const
{
	return (m_dwFlags & dwFlags) == dwFlags;
}

const GUID& ArchiveModule::GetUID() const
{
	return m_uid;
}

const TCHAR* ArchiveModule::GetModuleName() const
{
	return m_strModuleName;
}


ArchiveModule::~ArchiveModule()
{
	if ( m_pfnModuleEntry )
		m_pfnModuleEntry(FID_FINALIZE, NULL);

	if ( m_hModule )
		FreeLibrary(m_hModule);

	if ( m_pLanguage )
		delete m_pLanguage;
}

ArchivePlugin* ArchiveModule::GetPlugin(const GUID& uid)
{
	for (unsigned int i = 0; i < m_pPlugins.count(); i++)
		if ( m_pPlugins[i]->GetUID() == uid )
			return m_pPlugins[i];

	return NULL;
}

Array<ArchivePlugin*>& ArchiveModule::GetPlugins()
{
	return m_pPlugins;
}

int ArchiveModule::GetPlugins(Array<ArchivePlugin*>& plugins)
{
	for (unsigned int i = 0; i < m_pPlugins.count(); i++)
		plugins.add(m_pPlugins[i]);

	return 0;
}


int ArchiveModule::GetFormats(Array<ArchiveFormat*>& formats)
{
	for (unsigned int i = 0; i < m_pPlugins.count(); i++)
		m_pPlugins[i]->GetFormats(formats);

	return 0;
}

ArchiveFormat* ArchiveModule::GetFormat(const GUID& uidPlugin, const GUID& uidFormat)
{
	ArchivePlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->GetFormat(uidFormat);

	return NULL;
}

int ArchiveModule::QueryArchives(
		const GUID* puidPlugin,
		const GUID* puidFormat,
		const TCHAR *lpFileName,
		const unsigned char *pBuffer,
		DWORD dwBufferSize,
		Array<ArchiveFormat*>& result
		)
{
	QueryArchiveStruct QAS;

	DWORD dwFlags = 0;

	if ( puidPlugin )
	{
		QAS.uidPlugin = *puidPlugin;
		dwFlags |= QUERY_FLAG_PLUGIN_UID_READY;
	}

	if ( puidFormat )
	{
		QAS.uidFormat = *puidFormat;
		dwFlags |= QUERY_FLAG_FORMAT_UID_READY;
	}

	QAS.lpFileName = lpFileName;
	QAS.pBuffer = pBuffer;
	QAS.dwBufferSize = dwBufferSize;

	do { 

		QAS.dwFlags = dwFlags;
		QAS.bResult = false;

		m_pfnModuleEntry(FID_QUERYARCHIVE, &QAS);

		if ( QAS.bResult )
		{
			ArchiveFormat* pFormat = GetFormat(QAS.uidPlugin, QAS.uidFormat);

			if ( pFormat )
				result.add(pFormat);
		}

	} while ( (QAS.dwFlags & QUERY_FLAG_MORE_ARCHIVES) == QUERY_FLAG_MORE_ARCHIVES );			


	return 0;
}

HANDLE ArchiveModule::OpenCreateArchive(
		const GUID& uidPlugin,
		const GUID& uidFormat,
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	OpenCreateArchiveStruct OAS;

	OAS.hResult = NULL;

	OAS.uidPlugin = uidPlugin;
	OAS.uidFormat = uidFormat;
	OAS.lpFileName = lpFileName;
	OAS.hCallback = hCallback;
	OAS.pfnCallback = pfnCallback;
	OAS.bCreate = bCreate;

	if ( m_pfnModuleEntry(bCreate?FID_CREATEARCHIVE:FID_OPENARCHIVE, &OAS) == NAERROR_SUCCESS )
		return OAS.hResult;

	return NULL;
}


void ArchiveModule::CloseArchive(const GUID& uidPlugin, HANDLE hArchive)
{
	CloseArchiveStruct CAS;

	CAS.hArchive = hArchive;
	CAS.uidPlugin = uidPlugin;
	m_pfnModuleEntry(FID_CLOSEARCHIVE, (void*)&CAS);
}

bool ArchiveModule::GetDefaultCommand(
		const GUID& uidPlugin, 
		const GUID& uidFormat, 
		int nCommand, 
		string& strCommand,
		bool& bEnabled
		)
{
	GetDefaultCommandStruct GDC;

	GDC.nCommand = nCommand;
	GDC.lpCommand = NULL;
	GDC.bEnabled = false;
	GDC.uidPlugin = uidPlugin;
	GDC.uidFormat = uidFormat;


	if ( (m_pfnModuleEntry(FID_GETDEFAULTCOMMAND, (void*)&GDC) == NAERROR_SUCCESS) && GDC.bResult )
	{
		strCommand = GDC.lpCommand;
		bEnabled = GDC.bEnabled;
		return true;
	}

	strCommand = NULL;
	bEnabled = false;

	return false;
}


void ArchiveModule::ReloadLanguage(
		const TCHAR* lpLanguage
		)
{
	string strPath = m_strModuleName;
	CutToSlash (strPath);

	Language* pLanguage = nullptr;
	Language* pEnglishLanguage = nullptr;

	string strMask = strPath+_T("*.lng");
	
	WIN32_FIND_DATA FindData;
	bool bResult = false;

	HANDLE hSearch = FindFirstFile(
			strMask,
			(WIN32_FIND_DATA*)&FindData
			);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {
			string strFileName = strPath+FindData.cFileName;

			pLanguage = new Language();

			if ( pLanguage->LoadFromFile(strFileName) )
			{
				string strLanguage = pLanguage->GetLanguage();

				if ( strLanguage == lpLanguage )
				{
					if ( m_pLanguage )
						delete m_pLanguage;

					m_pLanguage = pLanguage;

					bResult = true;
				}
				else
				{
					if ( strLanguage == _T("English") ) //case??
					{
						if ( !pEnglishLanguage ) //нам два не надо
							pEnglishLanguage = pLanguage;
						else
							delete pLanguage;
					}
					else
						delete pLanguage;
				}
			}
			else
				delete pLanguage;

		} while ( !bResult && FindNextFile(hSearch, (WIN32_FIND_DATA*)&FindData) );

		FindClose (hSearch);
	}

	if ( pEnglishLanguage )
	{
		if ( !bResult )
		{
			if ( m_pLanguage )
				delete m_pLanguage;

			m_pLanguage = pEnglishLanguage;
		}
		else
			delete pEnglishLanguage;
	}
}



int ArchiveModule::Extract(
		HANDLE hArchive,
		const ArchiveItemArray& items, 
		const TCHAR* lpDestDiskPath, 
		const TCHAR* lpPathInArchive
		)
{
	ExtractStruct ES;

	ES.hArchive = hArchive;
	ES.pItems = items.data();
	ES.uItemsNumber = items.count();
	ES.lpDestPath = lpDestDiskPath;
	ES.lpCurrentPath = lpPathInArchive;

	if ( m_pfnModuleEntry (FID_EXTRACT, (void*)&ES) == NAERROR_SUCCESS )
		return ES.nResult;

	return RESULT_ERROR;
}

int ArchiveModule::AddFiles(
		HANDLE hArchive,
		const ArchiveItemArray& items,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpPathInArchive,
		const TCHAR* lpConfig
		)
{
	AddStruct AS;

	AS.hArchive = hArchive;
	AS.lpSourcePath = lpSourceDiskPath;
	AS.lpCurrentPath = lpPathInArchive;
	AS.pItems = items.data();
	AS.uItemsNumber = items.count();
	AS.pConfig = lpConfig;

	if ( m_pfnModuleEntry (FID_ADD, (void*)&AS) == NAERROR_SUCCESS )
		return AS.nResult;

	return RESULT_ERROR;
}


int ArchiveModule::Test(
		HANDLE hArchive,
		const ArchiveItemArray& items
		)
{
	TestStruct TS;

	TS.hArchive = hArchive;
	TS.pItems = items.data();
	TS.uItemsNumber = items.count();

	if ( m_pfnModuleEntry(FID_TEST, (void*)&TS) == NAERROR_SUCCESS )
		return TS.nResult;

	return RESULT_ERROR;
}

int ArchiveModule::Delete(
		HANDLE hArchive,
		const ArchiveItemArray& items
		)
{
	DeleteStruct DS;

	DS.hArchive = hArchive;
	DS.pItems = items.data();
	DS.uItemsNumber = items.count();

	if ( m_pfnModuleEntry (FID_DELETE, (void*)&DS) == NAERROR_SUCCESS )
		return DS.nResult;

	return RESULT_ERROR;
}

bool ArchiveModule::StartOperation(
		HANDLE hArchive,
		int nOperation,
		bool bInternal
		)
{
	OperationStruct SOS;

	SOS.hArchive = hArchive;
	SOS.nOperation = nOperation;
	SOS.bInternal = bInternal;

	if ( m_pfnModuleEntry(FID_STARTOPERATION, (void*)&SOS) == NAERROR_SUCCESS )
		return SOS.bResult || !bInternal; //ignore result for external for now

	return false;
}

bool ArchiveModule::EndOperation(
		HANDLE hArchive,
		int nOperation,
		bool bInternal
		)
{
	OperationStruct EOS;

	EOS.hArchive = hArchive;
	EOS.nOperation = nOperation;
	EOS.bInternal = bInternal;

	if ( m_pfnModuleEntry(FID_ENDOPERATION, (void*)&EOS) == NAERROR_SUCCESS )
		return EOS.bResult;

	return false;
}


bool ArchiveModule::GetArchiveFormat(
		HANDLE hArchive,
		GUID* puid
		)
{
	GetArchiveFormatStruct GAF;
	
	GAF.hArchive = hArchive;

	if ( (m_pfnModuleEntry(FID_GETARCHIVEFORMAT, (void*)&GAF) == NAERROR_SUCCESS) && GAF.bResult )
	{	
		*puid = GAF.uid;
		return true;
	}

	return false;
}

bool ArchiveModule::FreeArchiveItem(
		HANDLE hArchive,
		ArchiveItem* pItem
		)
{
	FreeArchiveItemStruct FAI;

	FAI.hArchive = hArchive;
	FAI.pItem = pItem;

	if ( m_pfnModuleEntry(FID_FREEARCHIVEITEM, (void*)&FAI) == NAERROR_SUCCESS )
		return FAI.bResult;

	return false;
}

int ArchiveModule::GetArchiveItem(
		HANDLE hArchive,
		ArchiveItem* pItem
		)
{
	GetArchiveItemStruct GAI;

	GAI.hArchive = hArchive;
	GAI.pItem = pItem;

	if ( m_pfnModuleEntry(FID_GETARCHIVEITEM, (void*)&GAI) == NAERROR_SUCCESS )
		return GAI.nResult;

	return 1;
}

int ArchiveModule::GetArchiveInfo(HANDLE hArchive, bool& bMultiVolume, const ArchiveInfoItem** pItems)
{
	ArchiveInfoStruct AIS;

	AIS.hArchive = hArchive;
	AIS.bResult = false;

	if ( (m_pfnModuleEntry(FID_GETARCHIVEINFO, (void*)&AIS) == NAERROR_SUCCESS) && AIS.bResult )
	{
		*pItems = AIS.pInfo;
		bMultiVolume = AIS.bMultiVolume;

		return AIS.nInfoItems;
	}

	return 0;
}

bool ArchiveModule::ConfigureFormat(const GUID& uidPlugin, const GUID& uidFormat, const TCHAR* lpInitialConfig, string& strResultConfig)
{
	ConfigureFormatStruct CFS;

	CFS.uidFormat = uidFormat;
	CFS.uidPlugin = uidPlugin;
	CFS.lpConfig = lpInitialConfig;

	if ( m_pfnModuleEntry(FID_CONFIGUREFORMAT, (void*)&CFS) == NAERROR_SUCCESS )
	{
		strResultConfig = CFS.lpResult;

		FreeConfigResultStruct FCR;
		FCR.lpResult = CFS.lpResult;

		m_pfnModuleEntry(FID_FREECONFIGRESULT, &FCR);

		return CFS.bResult;
	}

	return false;
}

void ArchiveModule::Configure()
{
	ConfigureStruct CF;
	m_pfnModuleEntry(FID_CONFIGURE, (void*)&CF);
}

