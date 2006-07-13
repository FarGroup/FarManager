#include <FarPluginBase.h>
#include "ma.class.h"

MaModule::MaModule (
		const char *lpFileName
		)
{
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
}

MaModules::MaModules ()
{
	memset (&m_PluginInfo, 0, sizeof (m_PluginInfo));

	char *lpPluginsPath = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpPluginsPath);

	strcat (lpPluginsPath, "Formats");

	m_Modules.Create(10);
	m_ExtraPluginInfo.Create(10);

	FSF.FarRecursiveSearch(lpPluginsPath,"*.fmt",(FRSUSERFUNC)LoadFmtModules,FRS_RECUR,this);

	StrFree(lpPluginsPath);

	m_PluginInfo.pFormatInfo = (ArchiveFormatInfo *) realloc (m_PluginInfo.pFormatInfo, sizeof (ArchiveFormatInfo) * m_PluginInfo.nFormats);

	for (int i=0; i<m_ExtraPluginInfo.GetCount (); i++)
	{
		for (int j=0; j<m_ExtraPluginInfo[i]->Count; j++)
		{
			m_PluginInfo.pFormatInfo[m_ExtraPluginInfo[i]->pExtraPluginInfo[j].FormatNumber].lpName=m_ExtraPluginInfo[i]->pExtraPluginInfo[j].Name;
			m_PluginInfo.pFormatInfo[m_ExtraPluginInfo[i]->pExtraPluginInfo[j].FormatNumber].lpDefaultExtention=m_ExtraPluginInfo[i]->pExtraPluginInfo[j].DefaultExtention;
		}
	}
}

MaModules::~MaModules ()
{
	if ( m_PluginInfo.pFormatInfo )
	{
		free (m_PluginInfo.pFormatInfo);
	}

	for (int i=0; i<m_ExtraPluginInfo.GetCount(); i++)
		free (m_ExtraPluginInfo[i]->pExtraPluginInfo);

	m_ExtraPluginInfo.Free ();
	m_Modules.Free ();
}

MaModule *MaModules::IsArchive (QueryArchiveStruct *pQAS, int *nModuleNum)
{
	DWORD MinSFXSize = 0xFFFFFFFF;
	DWORD CurSFXSize;
	MaModule *TrueArc = NULL;

    *nModuleNum = -1;

	for (int i=0; i<m_Modules.GetCount(); i++)
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
               	*nModuleNum = m_ExtraPluginInfo[i]->BaseNumber;
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

    ExtraPluginInfoArray *pExtraPluginInfoArray = (ExtraPluginInfoArray *)malloc (sizeof (ExtraPluginInfoArray));

    memset (pExtraPluginInfoArray, 0, sizeof (ExtraPluginInfoArray));

    pExtraPluginInfoArray->BaseNumber = pModules->m_PluginInfo.nFormats;

    ExtraPluginInfo *pExtraPluginInfo = NULL;

	if ( pModule->m_pfnGetFormatName )
	{
	    char Format[100], DefExt[NM];

	    *Format=0;
	    *DefExt=0;

		for (int i=0; pModule->m_pfnGetFormatName(i,Format,DefExt); i++)
		{
			pExtraPluginInfo = (ExtraPluginInfo *) realloc (pExtraPluginInfo, sizeof (ExtraPluginInfo) * (i+1));

			strcpy(pExtraPluginInfo[i].Name,Format);

			strcpy(pExtraPluginInfo[i].DefaultExtention,DefExt);

			pExtraPluginInfo[i].FormatNumber = pModules->m_PluginInfo.nFormats++;

			pExtraPluginInfoArray->Count++;

			*Format=0;
			*DefExt=0;
		}

	}

	if ( !pExtraPluginInfoArray->Count )
	{
		pExtraPluginInfo = (ExtraPluginInfo *) malloc (sizeof (ExtraPluginInfo));

        pExtraPluginInfo->FormatNumber = pModules->m_PluginInfo.nFormats++;

		pExtraPluginInfoArray->Count++;
	}

	pExtraPluginInfoArray->pExtraPluginInfo = pExtraPluginInfo;
	pModules->m_ExtraPluginInfo.Add (pExtraPluginInfoArray);

	pModules->m_Modules.Add (pModule);

	return TRUE;
}

void MaModules::GetArchivePluginInfo (ArchivePluginInfo *ai)
{
	ai->nFormats = m_PluginInfo.nFormats;
	ai->pFormatInfo = m_PluginInfo.pFormatInfo;
}

bool MaModules::GetDefaultCommand (int nFormat, int nCommand, char *lpCommand)
{
	int nModuleNum=-1, nTypeNum=-1;

	for (int i=0; i<m_ExtraPluginInfo.GetCount (); i++)
	{
		if ( m_ExtraPluginInfo[i]->BaseNumber > nFormat )
		{
			nModuleNum = i-1;
			break;
		}
		if ( m_ExtraPluginInfo[i]->BaseNumber == nFormat )
		{
			nModuleNum = i;
			nTypeNum = 0;
			break;
		}
	}

	if (nModuleNum == -1)
		nModuleNum = m_ExtraPluginInfo.GetCount()-1;

	if (nModuleNum != -1 && nTypeNum == -1)
	{
		for (int i=0; i<m_ExtraPluginInfo[nModuleNum]->Count; i++)
		{
			if ( m_ExtraPluginInfo[nModuleNum]->pExtraPluginInfo[i].FormatNumber == nFormat )
			{
				nTypeNum = i;
				break;
			}
		}
	}

	if (nModuleNum != -1 && nTypeNum != -1)
		if ( m_Modules[nModuleNum]->m_pfnGetDefaultCommands )
			return m_Modules[nModuleNum]->m_pfnGetDefaultCommands (
										nTypeNum,
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

int __stdcall MaArchive::pGetArchiveType ()
{
	return (m_nModuleNum+m_nArcType);
}
