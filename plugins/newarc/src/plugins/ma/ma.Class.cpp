#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "ma.class.h"
#include <debug.h>

MY_DEFINE_GUID (CLSID_TemplateMA, 0x83FEFEBE, 0x3EE3, 0x4CD5, 0xB2, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

unsigned long CRC32(
		unsigned long crc,
		const char *buf,
		unsigned int len
		)
{
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		unsigned long c;
		int n, k;

		for (n = 0; n < 256; n++)
		{
			c = (unsigned long)n;
			for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
				crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;
	while (len-- > 0) {
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ 0xffffffffL;
}


const char *GUID2STR (const GUID &uid)
{
	static char szGUID[64];
	LPOLESTR string;

	StringFromIID (uid, &string);

	//int length = wcslen (string)+1;
	//char *result = StrCreate (length);

	memset (&szGUID, 0, sizeof (szGUID));

	WideCharToMultiByte (CP_OEMCP, 0, string, -1, (char*)&szGUID, sizeof (szGUID), NULL, NULL);

	CoTaskMemFree (string);

	return (const char*)&szGUID;
}


bool GetGUIDFromModule (const MaModule *pModule, WORD wFormat, GUID *puid)
{
	if ( puid && pModule )
	{
		memcpy (puid, &CLSID_TemplateMA, sizeof (GUID));

		memcpy ((unsigned char*)puid+10, &pModule->m_dwCRC, 4);
		memcpy ((unsigned char*)puid+14, &wFormat, 2);

		return true;
	}

	return false;
}


MaModule::MaModule (
		const char *lpFileName
		)
{
	m_lpModuleName = StrDuplicate (lpFileName);

	FSF.LStrupr (m_lpModuleName);

	m_Info.create (ARRAY_OPTIONS_DELETE);

	const char *lpName = FSF.PointToName (m_lpModuleName);

	m_dwCRC = CRC32 (0, lpName, StrLength(lpName));

	m_hModule = LoadLibraryEx (
			lpFileName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnLoadFormatModule=(PLUGINLOADFORMATMODULE)GetProcAddress(m_hModule,"LoadFormatModule");
		m_pfnIsArchive=(PLUGINISARCHIVE)GetProcAddress(m_hModule,"IsArchive");
		m_pfnOpenArchive=(PLUGINOPENARCHIVE)GetProcAddress(m_hModule,"OpenArchive");
		m_pfnGetArcItem=(PLUGINGETARCITEM)GetProcAddress(m_hModule,"GetArcItem");
		m_pfnCloseArchive=(PLUGINCLOSEARCHIVE)GetProcAddress(m_hModule,"CloseArchive");
		m_pfnGetFormatName=(PLUGINGETFORMATNAME)GetProcAddress(m_hModule,"GetFormatName");
		m_pfnGetDefaultCommands=(PLUGINGETDEFAULTCOMMANDS)GetProcAddress(m_hModule,"GetDefaultCommands");
		m_pfnSetFarInfo=(PLUGINSETFARINFO)GetProcAddress(m_hModule,"SetFarInfo");
		m_pfnGetSFXPos=(PLUGINGETSFXPOS)GetProcAddress(m_hModule,"GetSFXPos");
	}

	if ( LoadedOK() )
	{
		if (m_pfnLoadFormatModule)
			m_pfnLoadFormatModule(lpFileName);

		if (m_pfnSetFarInfo)
		{
			PluginStartupInfo _Info;
			FARSTANDARDFUNCTIONS _FSF;

			_Info = Info;
			_FSF  = FSF;
			_Info.FSF = &_FSF;

			strcpy (_Info.ModuleName, lpFileName);
			m_pfnSetFarInfo(&_Info);
		}
	}
}

bool MaModule::LoadedOK ()
{
	if ( !(m_hModule && m_pfnIsArchive && m_pfnOpenArchive && m_pfnGetArcItem) )
		return false;

	return true;
}

MaModule::~MaModule ()
{
	FreeLibrary (m_hModule);
	StrFree (m_lpModuleName);

	m_Info.free ();
}

MaModules::MaModules ()
{
	memset (&m_PluginInfo, 0, sizeof (m_PluginInfo));

	char *lpPluginsPath = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpPluginsPath);

	strcat (lpPluginsPath, "Formats");

	m_Modules.create(ARRAY_OPTIONS_DELETE);

	FSF.FarRecursiveSearch(lpPluginsPath,"*.fmt",(FRSUSERFUNC)LoadFmtModules,FRS_RECUR,this);

	StrFree(lpPluginsPath);

	m_PluginInfo.pFormatInfo = (ArchiveFormatInfo *) realloc (m_PluginInfo.pFormatInfo, sizeof (ArchiveFormatInfo) * m_PluginInfo.nFormats);

	GUID uid;
	int index = 0;

	for (int i = 0; i < m_Modules.count(); i++)
	{
		MaModule *pModule = m_Modules[i];

		for (int j = 0; j < pModule->m_Info.count(); j++)
		{
			GetGUIDFromModule (pModule, j, &uid);

			m_PluginInfo.pFormatInfo[index].lpName = pModule->m_Info[j]->Name;
			m_PluginInfo.pFormatInfo[index].lpDefaultExtention = pModule->m_Info[j]->DefaultExtention;
			m_PluginInfo.pFormatInfo[index].uid = uid;

			index++;
		}
	}
}

MaModules::~MaModules ()
{
	if ( m_PluginInfo.pFormatInfo )
		free (m_PluginInfo.pFormatInfo);

	m_Modules.free ();
}

MaModule *MaModules::IsArchive (QueryArchiveStruct *pQAS, int *nModuleNum)
{
	DWORD MinSFXSize = 0xFFFFFFFF;
	DWORD CurSFXSize;
	MaModule *TrueArc = NULL;

    //*nModuleNum = -1;

	for (int i=0; i<m_Modules.count(); i++)
	{
		if ( m_Modules[i]->m_pfnIsArchive (pQAS->lpFileName, (const unsigned char *)pQAS->lpBuffer, pQAS->dwBufferSize) )
		{
			CurSFXSize = 0;

			if ( m_Modules[i]->m_pfnGetSFXPos )
				CurSFXSize = m_Modules[i]->m_pfnGetSFXPos ();

			if ( CurSFXSize <= MinSFXSize )
			{
				MinSFXSize = CurSFXSize;
				TrueArc = m_Modules[i];
               	//*nModuleNum = m_ExtraPluginInfo[i]->BaseNumber;
			}
		}
	}

	return TrueArc;
}

int WINAPI MaModules::LoadFmtModules (const WIN32_FIND_DATA *pFindData,
		const char *lpFullName,
		MaModules *pModules
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	MaModule *pModule = new MaModule (lpFullName);

	if ( !pModule )
		return TRUE;

	if ( !pModule->LoadedOK () )
	{
		delete pModule;
		return TRUE;
	}

	if ( pModule->m_pfnGetFormatName )
	{
		int index = 0;

		while ( true )
		{
			MaPluginInfo *info = new MaPluginInfo;

			*info->Name = 0;
			*info->DefaultExtention = 0;

			if ( pModule->m_pfnGetFormatName (index, info->Name, info->DefaultExtention) )
			{
				pModule->m_Info.add (info);
				index++;
				pModules->m_PluginInfo.nFormats++;
			}
			else
			{
				delete info;
				break;
			}
		}
	}

	pModules->m_Modules.add (pModule);

	return TRUE;
}

void MaModules::GetArchivePluginInfo (ArchivePluginInfo *ai)
{
	ai->nFormats = m_PluginInfo.nFormats;
	ai->pFormatInfo = m_PluginInfo.pFormatInfo;
}

bool MaModules::GetDefaultCommand (const GUID &uid, int nCommand, char *lpCommand)
{
	unsigned char *buffer = (unsigned char*)&uid;

	WORD index = *(WORD*)(buffer+14);
	DWORD crc = *(DWORD*)(buffer+10);

	MaModule *pResultModule = NULL;

	for (int i = 0; i < m_Modules.count(); i++)
	{
		if ( m_Modules[i]->m_dwCRC == crc )
		{
			pResultModule = m_Modules[i];
			break;
		}
	}

	if ( pResultModule && pResultModule->m_pfnGetDefaultCommands )
		return (bool)pResultModule->m_pfnGetDefaultCommands (
				index,
				nCommand,
				lpCommand
				);

	return false;
}

MaArchive::MaArchive (MaModule *pModule, int nModuleNum, const char *lpFileName)
{
	m_pModule = pModule;

	m_lpFileName = StrDuplicate (lpFileName);

	m_nArcType = 0;

	m_nModuleNum = nModuleNum;
}

MaArchive::~MaArchive ()
{
	StrFree (m_lpFileName);
}

int MaArchive::ConvertResult (int nResult)
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

bool __stdcall MaArchive::pOpenArchive ()
{
	m_nArcType = 0;
	return m_pModule->m_pfnOpenArchive (m_lpFileName, &m_nArcType);
}

void __stdcall MaArchive::pCloseArchive ()
{
	struct ArcInfo MaArcInfo;

	if (m_pModule->m_pfnCloseArchive)
		m_pModule->m_pfnCloseArchive (&MaArcInfo);
}

int __stdcall MaArchive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	struct ArcItemInfo MaItemInfo;
	memset (&MaItemInfo, 0, sizeof (MaItemInfo));

	int nResult = m_pModule->m_pfnGetArcItem (&pItem->pi, &MaItemInfo);

	if ( nResult==GETARC_SUCCESS )
	{
		if ( MaItemInfo.Encrypted )
			pItem->dwFlags |= AIF_CRYPTED;
		if ( MaItemInfo.Solid )
			pItem->dwFlags |= AIF_SOLID;

		return E_SUCCESS;
	}

	return ConvertResult (nResult);
}

void __stdcall MaArchive::pGetArchiveType (GUID *puid)
{
	GetGUIDFromModule (m_pModule, m_nArcType, puid);
}
