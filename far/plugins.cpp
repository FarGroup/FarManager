/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugins.hpp"
#include "keys.hpp"
#include "flink.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "farexcpt.hpp"
#include "fileedit.hpp"
#include "RefreshFrameManager.hpp"
#include "plugapi.hpp"
#include "TaskBar.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "processname.hpp"
#include "interf.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "FarGuid.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"

static const wchar_t *PluginsFolderName=L"Plugins";

static int _cdecl PluginsSort(const void *el1,const void *el2);

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

			for (k = 0; k < 8; k++) c = (c >> 1) ^(c & 1 ? 0xedb88320L : 0);

			crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;

	while (len-- > 0)
	{
		crc = crc_table[(crc ^(*buf++)) & 0xff] ^(crc >> 8);
	}

	return crc ^ 0xffffffffL;
}

enum
{
	CRC32_GETGLOBALINFOW   = 0x633EC0C4,
};

DWORD ExportCRC32W[] =
{
	CRC32_GETGLOBALINFOW,
};

#ifndef NO_WRAPPER
enum
{
	CRC32_SETSTARTUPINFO   = 0xF537107A,
	CRC32_GETPLUGININFO    = 0xDB6424B4,
	CRC32_OPENPLUGIN       = 0x601AEDE8,
	CRC32_OPENFILEPLUGIN   = 0xAC9FF5CD,
	CRC32_EXITFAR          = 0x04419715,
	CRC32_SETFINDLIST      = 0x7A74A2E5,
	CRC32_CONFIGURE        = 0x4DC1BC1A,
	CRC32_GETMINFARVERSION = 0x2BBAD952,
};

DWORD ExportCRC32[] =
{
	CRC32_SETSTARTUPINFO,
	CRC32_GETPLUGININFO,
	CRC32_OPENPLUGIN,
	CRC32_OPENFILEPLUGIN,
	CRC32_EXITFAR,
	CRC32_SETFINDLIST,
	CRC32_CONFIGURE,
	CRC32_GETMINFARVERSION
};
#endif // NO_WRAPPER

enum PluginType
{
	NOT_PLUGIN,
	UNICODE_PLUGIN,
#ifndef NO_WRAPPER
	OEM_PLUGIN,
#endif // NO_WRAPPER
};

PluginType IsModulePlugin2(
    PBYTE hModule
)
{
	DWORD dwExportAddr;
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pPEHeader;
	__try
	{

		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return NOT_PLUGIN;

		pPEHeader = (PIMAGE_NT_HEADERS)&hModule[pDOSHeader->e_lfanew];

		if (pPEHeader->Signature != IMAGE_NT_SIGNATURE)
			return NOT_PLUGIN;

		if (!(pPEHeader->FileHeader.Characteristics & IMAGE_FILE_DLL))
			return NOT_PLUGIN;

		if (pPEHeader->FileHeader.Machine!=
#ifdef _WIN64
#ifdef _M_IA64
		        IMAGE_FILE_MACHINE_IA64
#else
		        IMAGE_FILE_MACHINE_AMD64
#endif
#else
		        IMAGE_FILE_MACHINE_I386
#endif
		   )
			return NOT_PLUGIN;

		dwExportAddr = pPEHeader->OptionalHeader.DataDirectory[0].VirtualAddress;

		if (!dwExportAddr)
			return NOT_PLUGIN;

		PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pPEHeader);

		for (int i = 0; i < pPEHeader->FileHeader.NumberOfSections; i++)
		{
			if ((pSection[i].VirtualAddress == dwExportAddr) ||
			        ((pSection[i].VirtualAddress <= dwExportAddr) && ((pSection[i].Misc.VirtualSize+pSection[i].VirtualAddress) > dwExportAddr)))
			{
				int nDiff = pSection[i].VirtualAddress-pSection[i].PointerToRawData;
				PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)&hModule[dwExportAddr-nDiff];
				DWORD* pNames = (DWORD *)&hModule[pExportDir->AddressOfNames-nDiff];
#ifndef NO_WRAPPER
				bool bOemExports=false;
#endif // NO_WRAPPER
				for (DWORD n = 0; n < pExportDir->NumberOfNames; n++)
				{
					const char *lpExportName = (const char *)&hModule[pNames[n]-nDiff];
					DWORD dwCRC32 = CRC32(0, lpExportName, (unsigned int)strlen(lpExportName));

					// а это вам не фиг знает что, это вам оптимизация, типа 8-)
					for (size_t j = 0; j < ARRAYSIZE(ExportCRC32W); j++)
						if (dwCRC32 == ExportCRC32W[j])
							return UNICODE_PLUGIN;

#ifndef NO_WRAPPER
					if (!bOemExports && Opt.LoadPlug.OEMPluginsSupport)
						for (size_t j = 0; j < ARRAYSIZE(ExportCRC32); j++)
							if (dwCRC32 == ExportCRC32[j])
								bOemExports=true;
#endif // NO_WRAPPER
				}
#ifndef NO_WRAPPER
				if (bOemExports)
					return OEM_PLUGIN;
#endif // NO_WRAPPER
			}
		}

		return NOT_PLUGIN;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NOT_PLUGIN;
	}
}

PluginType IsModulePlugin(const string& lpModuleName)
{
	PluginType bResult = NOT_PLUGIN;
	HANDLE hModuleFile = apiCreateFile(
	                         lpModuleName,
	                         GENERIC_READ,
	                         FILE_SHARE_READ,
	                         nullptr,
	                         OPEN_EXISTING,
	                         0
	                     );

	if (hModuleFile != INVALID_HANDLE_VALUE)
	{
		HANDLE hModuleMapping = CreateFileMapping(
		                            hModuleFile,
		                            nullptr,
		                            PAGE_READONLY,
		                            0,
		                            0,
		                            nullptr
		                        );

		if (hModuleMapping)
		{
			PBYTE pData = (PBYTE)MapViewOfFile(hModuleMapping, FILE_MAP_READ, 0, 0, 0);

			if (pData)
			{
				bResult = IsModulePlugin2(pData);
				UnmapViewOfFile(pData);
			}

			CloseHandle(hModuleMapping);
		}

		CloseHandle(hModuleFile);
	}

	return bResult;
}

class PluginSearch: public AncientPlugin
{
	private:
		GUID m_Guid;
		PluginSearch();
	public:
		PluginSearch(const GUID& Id): m_Guid(Id) {}
		~PluginSearch() {}
		const GUID& GetGUID(void) const { return m_Guid; }
};

PluginTree::PluginTree(): Tree<AncientPlugin*>()
{
}

PluginTree::~PluginTree()
{
	clear();
}

long PluginTree::compare(Node<AncientPlugin*>* first,AncientPlugin** second)
{
	return memcmp(&((*(first->data))->GetGUID()),&((*second)->GetGUID()),sizeof(GUID));
}

AncientPlugin** PluginTree::query(const GUID& value)
{
	PluginSearch plugin(value);
	AncientPlugin* get=&plugin;
	return Tree<AncientPlugin*>::query(&get);
}

PluginManager::PluginManager():
	PluginsData(nullptr),
	PluginsCount(0),
#ifndef NO_WRAPPER
	OemPluginsCount(0),
#endif // NO_WRAPPER
	PluginsCache(nullptr),
	CurPluginItem(nullptr),
	CurEditor(nullptr),
	CurViewer(nullptr)
{
	PluginsCache=new PluginTree;
}

//хак чтоб SCTL_* могли работать при ExitFarW.
PluginManager *PluginManagerForExitFar=nullptr;

PluginManager::~PluginManager()
{
	CurPluginItem=nullptr;
	Plugin *pPlugin;

	PluginManagerForExitFar = this;
	for (size_t i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];
		pPlugin->Unload(true);
		if (PluginsCache)
		{
			PluginsCache->remove((AncientPlugin**)&pPlugin);
		}
		delete pPlugin;
		PluginsData[i] = nullptr;
	}
	PluginManagerForExitFar = nullptr;

	delete PluginsCache;
	PluginsCache=nullptr;

	if(PluginsData)
	{
		xf_free(PluginsData);
	}
}

bool PluginManager::AddPlugin(Plugin *pPlugin)
{
	if (PluginsCache)
	{
		AncientPlugin** item=new AncientPlugin*(pPlugin);
		item=PluginsCache->insert(item);
		if(*item!=pPlugin) return false;
	}
	Plugin **NewPluginsData=(Plugin**)xf_realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));

	if (!NewPluginsData)
		return false;

	PluginsData = NewPluginsData;
	PluginsData[PluginsCount]=pPlugin;
	PluginsCount++;
#ifndef NO_WRAPPER
	if(pPlugin->IsOemPlugin())
	{
		OemPluginsCount++;
	}
#endif // NO_WRAPPER
	return true;
}

bool PluginManager::UpdateId(Plugin *pPlugin, const GUID& Id)
{
	if (PluginsCache)
	{
		PluginsCache->remove((AncientPlugin**)&pPlugin);
		pPlugin->SetGuid(Id);
		AncientPlugin** item=new AncientPlugin*(pPlugin);
		item=PluginsCache->insert(item);
		if(*item!=pPlugin) return false;
	}
	return true;
}

bool PluginManager::RemovePlugin(Plugin *pPlugin)
{
	if (PluginsCache)
	{
		PluginsCache->remove((AncientPlugin**)&pPlugin);
	}
	for (size_t i = 0; i < PluginsCount; i++)
	{
		if (PluginsData[i] == pPlugin)
		{
#ifndef NO_WRAPPER
			if(pPlugin->IsOemPlugin())
			{
				OemPluginsCount--;
			}
#endif // NO_WRAPPER
			delete pPlugin;
			memmove(&PluginsData[i], &PluginsData[i+1], (PluginsCount-i-1)*sizeof(Plugin*));
			PluginsCount--;
			return true;
		}
	}

	return false;
}


Plugin* PluginManager::LoadPlugin(
    const string& lpwszModuleName,
    const FAR_FIND_DATA_EX &FindData,
    bool LoadToMem
)
{
	Plugin *pPlugin = nullptr;

	switch (IsModulePlugin(lpwszModuleName))
	{
		case UNICODE_PLUGIN: pPlugin = new Plugin(this, lpwszModuleName); break;
#ifndef NO_WRAPPER
		case OEM_PLUGIN: pPlugin = new wrapper::PluginA(this, lpwszModuleName); break;
#endif // NO_WRAPPER
		default: return nullptr;
	}

	if (!pPlugin)
		return nullptr;

	bool bResult=false,bDataLoaded=false;

	if (!LoadToMem)
		bResult = pPlugin->LoadFromCache(FindData);

	if (!bResult && (pPlugin->CheckWorkFlags(PIWF_PRELOADED) || !Opt.LoadPlug.PluginsCacheOnly))
	{
		bResult = bDataLoaded = pPlugin->LoadData();
	}

	if (bResult && !AddPlugin(pPlugin))
	{
		pPlugin->Unload(true);
		delete pPlugin;
		return nullptr;
	}

	if (bDataLoaded)
	{
		bResult = pPlugin->Load();
	}

	return pPlugin;
}

HANDLE PluginManager::LoadPluginExternal(const string& lpwszModuleName, bool LoadToMem)
{
	Plugin *pPlugin = GetPlugin(lpwszModuleName);

	if (pPlugin)
	{
		if ((LoadToMem || pPlugin->bPendingRemove) && !pPlugin->Load())
		{
			UnloadedPlugins.Push(&pPlugin);
			return nullptr;
		}
	}
	else
	{
		FAR_FIND_DATA_EX FindData;

		if (apiGetFindDataEx(lpwszModuleName, FindData))
		{
			pPlugin = LoadPlugin(lpwszModuleName, FindData, LoadToMem);
			if (!pPlugin)
				return nullptr;
			far_qsort(PluginsData, PluginsCount, sizeof(*PluginsData), PluginsSort);
		}
	}
	return pPlugin;
}

int PluginManager::UnloadPlugin(Plugin *pPlugin, DWORD dwException)
{
	int nResult = FALSE;

	if (pPlugin && (dwException != EXCEPT_EXITFAR))   //схитрим, если упали в EXITFAR, не полезем в рекурсию, мы и так в Unload
	{
		//какие-то непонятные действия...
		CurPluginItem=nullptr;

		for(int i = FrameManager->GetModalStackCount()-1; i >= 0; --i)
		{
			Frame *frame = FrameManager->GetModalFrame(i);
			if((frame->GetType()==MODALTYPE_DIALOG && static_cast<Dialog*>(frame)->GetPluginOwner() == pPlugin) || frame->GetType()==MODALTYPE_HELP)
			{
				frame->Lock();
				if(i)
				{
					FrameManager->GetModalFrame(i-1)->Lock();
				}
				FrameManager->DeleteFrame(frame);
				FrameManager->PluginCommit();
			}
		}

		bool bPanelPlugin = pPlugin->IsPanelPlugin();

		if (dwException != (DWORD)-1)
			nResult = pPlugin->Unload(true);
		else
			nResult = pPlugin->Unload(false);

		pPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (bPanelPlugin /*&& bUpdatePanels*/)
		{
			CtrlObject->Cp()->ActivePanel->SetCurDir(L".",TRUE);
			Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
			ActivePanel->Update(UPDATE_KEEP_SELECTION);
			ActivePanel->Redraw();
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}

		UnloadedPlugins.Push(&pPlugin);
	}

	return nResult;
}

bool PluginManager::IsPluginUnloaded(Plugin* pPlugin)
{
	for(Plugin** p  = UnloadedPlugins.First(); p; p = UnloadedPlugins.Next(p))
	{
		if(*p == pPlugin)
		{
			return true;
		}
	}
	return false;
}

int PluginManager::UnloadPluginExternal(HANDLE hPlugin)
{
	//BUGBUG нужны проверки на легальность выгрузки
	int nResult = FALSE;
	Plugin* pPlugin = reinterpret_cast<Plugin*>(hPlugin);
	if(pPlugin->Active())
	{
		nResult = TRUE;
	}
	else
	{
		nResult = pPlugin->Unload(true);
	}
	if(!IsPluginUnloaded(pPlugin))
	{
		UnloadedPlugins.Push(&pPlugin);
	}
	return nResult;
}

Plugin *PluginManager::GetPlugin(const wchar_t *lpwszModuleName)
{
	Plugin *pPlugin;

	for (size_t i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		if (!StrCmpI(lpwszModuleName, pPlugin->GetModuleName()))
			return pPlugin;
	}

	return nullptr;
}

Plugin *PluginManager::GetPlugin(size_t PluginNumber)
{
	if (PluginNumber < PluginsCount)
		return PluginsData[PluginNumber];

	return nullptr;
}

void PluginManager::LoadPlugins()
{
	TaskBar TB(false);
	Flags.Clear(PSIF_PLUGINSLOADDED);

	if (Opt.LoadPlug.PluginsCacheOnly)  // $ 01.09.2000 tran  '/co' switch
	{
		LoadPluginsFromCache();
	}
	else if (Opt.LoadPlug.MainPluginDir || !Opt.LoadPlug.strCustomPluginsPath.IsEmpty() || (Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty()))
	{
		ScanTree ScTree(FALSE,TRUE,Opt.LoadPlug.ScanSymlinks);
		UserDefinedList PluginPathList(ULF_UNIQUE);  // хранение списка каталогов
		string strPluginsDir;
		string strFullName;
		FAR_FIND_DATA_EX FindData;

		// сначала подготовим список
		if (Opt.LoadPlug.MainPluginDir) // только основные и персональные?
		{
			strPluginsDir=g_strFarPath+PluginsFolderName;
			PluginPathList.AddItem(strPluginsDir);

			// ...а персональные есть?
			if (Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty() && !(Opt.Policies.DisabledOptions&FFPOL_PERSONALPATH))
				PluginPathList.AddItem(Opt.LoadPlug.strPersonalPluginsPath);
		}
		else if (!Opt.LoadPlug.strCustomPluginsPath.IsEmpty())  // только "заказные" пути?
		{
			PluginPathList.AddItem(Opt.LoadPlug.strCustomPluginsPath);
		}

		const wchar_t *NamePtr;
		PluginPathList.Reset();

		// теперь пройдемся по всему ранее собранному списку
		while (nullptr!=(NamePtr=PluginPathList.GetNext()))
		{
			// расширяем значение пути
			apiExpandEnvironmentStrings(NamePtr,strFullName);
			Unquote(strFullName); //??? здесь ХЗ

			if (!IsAbsolutePath(strFullName))
			{
				strPluginsDir = g_strFarPath;
				strPluginsDir += strFullName;
				strFullName = strPluginsDir;
			}

			// Получим реальное значение полного длинного пути
			ConvertNameToFull(strFullName,strFullName);
			ConvertNameToLong(strFullName,strFullName);
			strPluginsDir = strFullName;

			if (strPluginsDir.IsEmpty())  // Хмм... а нужно ли ЭТО условие после такой модернизации алгоритма загрузки?
				continue;

			// ставим на поток очередной путь из списка...
			ScTree.SetFindPath(strPluginsDir,L"*");

			// ...и пройдемся по нему
			while (ScTree.GetNextName(&FindData,strFullName))
			{
				if (CmpName(L"*.dll",FindData.strFileName,false) && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					LoadPlugin(strFullName, FindData, false);
				}
			} // end while
		}
	}

	Flags.Set(PSIF_PLUGINSLOADDED);
	far_qsort(PluginsData, PluginsCount, sizeof(*PluginsData), PluginsSort);
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginManager::LoadPluginsFromCache()
{
	string strModuleName;

	for (DWORD i=0; PlCacheCfg->EnumPlugins(i, strModuleName); i++)
	{
		ReplaceSlashToBSlash(strModuleName);

		FAR_FIND_DATA_EX FindData;

		if (apiGetFindDataEx(strModuleName, FindData))
			LoadPlugin(strModuleName, FindData, false);
	}
}

int _cdecl PluginsSort(const void *el1,const void *el2)
{
	Plugin *Plugin1=*((Plugin**)el1);
	Plugin *Plugin2=*((Plugin**)el2);
	return (StrCmpI(PointToName(Plugin1->GetModuleName()),PointToName(Plugin2->GetModuleName())));
}

HANDLE PluginManager::OpenFilePlugin(
	const string* Name,
	int OpMode,	//!!! potential future error: OPERATION_MODES is __int64
	OPENFILEPLUGINTYPE Type
)
{
	struct PluginInfo
	{
		PluginHandle Handle;
		HANDLE Analyse;
	};
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	ConsoleTitle ct(Opt.ShowCheckingFile?MSG(MCheckingFileInPlugin):nullptr);
	HANDLE hResult = nullptr;
	PluginInfo *pResult = nullptr;
	PluginInfo *pAnalyse = nullptr;
	TPointerArray<PluginInfo> items;
	string strFullName;

	if (Name)
	{
		ConvertNameToFull(*Name,strFullName);
		Name = &strFullName;
	}

	bool ShowMenu = Opt.PluginConfirm.OpenFilePlugin==BSTATE_3STATE? !(Type == OFP_NORMAL || Type == OFP_SEARCH) : Opt.PluginConfirm.OpenFilePlugin != 0;
	bool ShowWarning = (OpMode==0);
	 //у анси плагинов OpMode нет.
	if(Type==OFP_ALTERNATIVE) OpMode|=OPM_PGDN;
	if(Type==OFP_COMMANDS) OpMode|=OPM_COMMANDS;

	Plugin *pPlugin = nullptr;

	File file;
	AnalyseInfo Info={sizeof(Info), Name? Name->CPtr() : nullptr, nullptr, 0, (OPERATION_MODES)OpMode};
	bool DataRead = false;
	for (size_t i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		if (!pPlugin->HasOpenFilePlugin() && !(pPlugin->HasAnalyse() && pPlugin->HasOpenPanel()))
			continue;

		if(Name && !DataRead)
		{
			if (file.Open(*Name, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
			{
				Info.Buffer = new BYTE[Opt.PluginMaxReadData];
				if (Info.Buffer)
				{
					DWORD DataSize = 0;
					if (file.Read(Info.Buffer, Opt.PluginMaxReadData, DataSize))
					{
						Info.BufferSize = DataSize;
						DataRead = true;
					}
				}
				file.Close();
			}
			if(!DataRead)
			{
				if(ShowWarning)
				{
					Message(MSG_WARNING|MSG_ERRORTYPE, 1, L"", MSG(MOpenPluginCannotOpenFile), *Name, MSG(MOk));
				}
				break;
			}
		}

		HANDLE hPlugin;

		if (pPlugin->HasOpenFilePlugin())
		{
			if (Opt.ShowCheckingFile)
				ct << MSG(MCheckingFileInPlugin) << L" - [" << PointToName(pPlugin->GetModuleName()) << L"]..." << fmt::Flush();

			hPlugin = pPlugin->OpenFilePlugin(Name? Name->CPtr() : nullptr, (BYTE*)Info.Buffer, Info.BufferSize, OpMode);

			if (hPlugin == PANEL_STOP)   //сразу на выход, плагин решил нагло обработать все сам (Autorun/PictureView)!!!
			{
				hResult = PANEL_STOP;
				break;
			}

			if (hPlugin)
			{
				PluginInfo *handle=items.addItem();
				handle->Handle.hPlugin = hPlugin;
				handle->Handle.pPlugin = pPlugin;
				handle->Analyse = nullptr;
			}
		}
		else
		{
			HANDLE analyse=pPlugin->Analyse(&Info);
			if (analyse)
			{
				PluginInfo *handle=items.addItem();
				handle->Handle.pPlugin = pPlugin;
				handle->Handle.hPlugin = nullptr;
				handle->Analyse = analyse;
			}
		}

		if (items.getCount() && !ShowMenu)
			break;
	}

	if (items.getCount() && (hResult != PANEL_STOP))
	{
		bool OnlyOne = (items.getCount() == 1) && !(Name && Opt.PluginConfirm.OpenFilePlugin && Opt.PluginConfirm.StandardAssociation && Opt.PluginConfirm.EvenIfOnlyOnePlugin);

		if(!OnlyOne && ShowMenu)
		{
			VMenu menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
			menu.SetPosition(-1, -1, 0, 0);
			menu.SetHelp(L"ChoosePluginMenu");
			menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
			MenuItemEx mitem;

			for (size_t i = 0; i < items.getCount(); i++)
			{
				PluginInfo *handle = items.getItem(i);
				mitem.Clear();
				mitem.strName = handle->Handle.pPlugin->GetTitle();
				menu.SetUserData(&handle, sizeof(handle), menu.AddItem(&mitem));
			}

			if (Opt.PluginConfirm.StandardAssociation && Type == OFP_NORMAL)
			{
				mitem.Clear();
				mitem.Flags |= MIF_SEPARATOR;
				menu.AddItem(&mitem);
				mitem.Clear();
				mitem.strName = MSG(MMenuPluginStdAssociation);
				menu.AddItem(&mitem);
			}

			menu.Show();

			while (!menu.Done())
			{
				menu.ReadInput();
				menu.ProcessInput();
			}

			if (menu.GetExitCode() == -1)
				hResult = PANEL_STOP;
			else
			{
				void* pItem = menu.GetUserData(nullptr, 0);
				if (pItem)
				{
					pResult = *static_cast<PluginInfo**>(pItem);
				}
			}
		}
		else
		{
			pResult = items.getItem(0);
		}

		if (pResult && pResult->Handle.hPlugin == nullptr)
		{
			pAnalyse = pResult;
			OpenAnalyseInfo oainfo={sizeof(OpenAnalyseInfo),&Info,pResult->Analyse};
			HANDLE h = pResult->Handle.pPlugin->Open(OPEN_ANALYSE, FarGuid, (intptr_t)&oainfo);

			if (h == PANEL_STOP)
			{
				hResult = PANEL_STOP;
				pResult = nullptr;
			}
			else if (h)
			{
				pResult->Handle.hPlugin = h;
			}
			else
			{
				pResult = nullptr;
			}
		}
	}

	if(Info.Buffer)
	{
		delete[] (BYTE*)Info.Buffer;
	}

	for (size_t i = 0; i < items.getCount(); i++)
	{
		PluginInfo *handle = items.getItem(i);

		if (handle != pResult)
		{
			if (handle->Handle.hPlugin)
				handle->Handle.pPlugin->ClosePanel(handle->Handle.hPlugin);
		}
		if (handle != pAnalyse)
		{
			if(handle->Analyse)
				handle->Handle.pPlugin->CloseAnalyse(handle->Analyse);
		}
	}

	if (pResult)
	{
		PluginHandle* pDup=new PluginHandle;
		pDup->hPlugin=pResult->Handle.hPlugin;
		pDup->pPlugin=pResult->Handle.pPlugin;
		hResult=static_cast<HANDLE>(pDup);
	}

	return hResult;
}

HANDLE PluginManager::OpenFindListPlugin(const PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *pResult = nullptr;
	TPointerArray<PluginHandle> items;
	Plugin *pPlugin=nullptr;

	for (size_t i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		if (!pPlugin->HasSetFindList())
			continue;

		HANDLE hPlugin = pPlugin->Open(OPEN_FINDLIST, FarGuid, 0);

		if (hPlugin)
		{
			PluginHandle *handle=items.addItem();
			handle->hPlugin = hPlugin;
			handle->pPlugin = pPlugin;
		}

		if (items.getCount() && !Opt.PluginConfirm.SetFindList)
			break;
	}

	if (items.getCount())
	{
		if (items.getCount()>1)
		{
			VMenu menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
			menu.SetPosition(-1, -1, 0, 0);
			menu.SetHelp(L"ChoosePluginMenu");
			menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
			MenuItemEx mitem;

			for (size_t i=0; i<items.getCount(); i++)
			{
				PluginHandle *handle = items.getItem(i);
				mitem.Clear();
				mitem.strName = handle->pPlugin->GetTitle();
				menu.AddItem(&mitem);
			}

			menu.Show();

			while (!menu.Done())
			{
				menu.ReadInput();
				menu.ProcessInput();
			}

			int ExitCode=menu.GetExitCode();

			if (ExitCode>=0)
			{
				pResult=items.getItem(ExitCode);
			}
		}
		else
		{
			pResult=items.getItem(0);
		}
	}

	if (pResult)
	{
		if (!pResult->pPlugin->SetFindList(pResult->hPlugin, PanelItem, ItemsNumber))
		{
			pResult=nullptr;
		}
	}

	for (size_t i=0; i<items.getCount(); i++)
	{
		PluginHandle *handle=items.getItem(i);

		if (handle!=pResult)
		{
			if (handle->hPlugin)
				handle->pPlugin->ClosePanel(handle->hPlugin);
		}
	}

	if (pResult)
	{
		PluginHandle* pDup=new PluginHandle;
		pDup->hPlugin=pResult->hPlugin;
		pDup->pPlugin=pResult->pPlugin;
		pResult=pDup;
	}

	return pResult;
}


void PluginManager::ClosePanel(HANDLE hPlugin)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ph->pPlugin->ClosePanel(ph->hPlugin);
	delete ph;
}


int PluginManager::ProcessEditorInput(INPUT_RECORD *Rec)
{
	for (size_t i = 0; i < PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		if (pPlugin->HasProcessEditorInput() && pPlugin->ProcessEditorInput(Rec))
			return TRUE;
	}

	return FALSE;
}


int PluginManager::ProcessEditorEvent(int Event,void *Param,int EditorID)
{
	int nResult = 0;

	if (CtrlObject->Plugins->CurEditor)
	{
		Plugin *pPlugin = nullptr;

		for (size_t i = 0; i < PluginsCount; i++)
		{
			pPlugin = PluginsData[i];

			if (pPlugin->HasProcessEditorEvent())
				nResult = pPlugin->ProcessEditorEvent(Event, Param, EditorID);
		}
	}

	return nResult;
}


int PluginManager::ProcessViewerEvent(int Event, void *Param,int ViewerID)
{
	int nResult = 0;

	for (size_t i = 0; i < PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		if (pPlugin->HasProcessViewerEvent())
			nResult = pPlugin->ProcessViewerEvent(Event, Param, ViewerID);
	}

	return nResult;
}

int PluginManager::ProcessDialogEvent(int Event, FarDialogEvent *Param)
{
	for (size_t i=0; i<PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		if (pPlugin->HasProcessDialogEvent() && pPlugin->ProcessDialogEvent(Event,Param))
			return TRUE;
	}

	return FALSE;
}

#if defined(MANTIS_0000466)
int PluginManager::ProcessMacro(const GUID& guid,ProcessMacroInfo *Info)
{
	int nResult = 0;

#if 0
	for (int i=0; i<PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		if (pPlugin->HasProcessMacro())
			if ((nResult = pPlugin->ProcessMacro(Info)) != 0)
				break;
	}
#else
	Plugin *pPlugin = FindPlugin(guid);

	if (pPlugin && pPlugin->HasProcessMacro())
	{
		nResult = pPlugin->ProcessMacro(Info);
	}
#endif

	return nResult;
}
#endif

#if defined(MANTIS_0001687)
int PluginManager::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	int nResult = 0;

	for (size_t i=0; i<PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		if (pPlugin->HasProcessConsoleInput())
		{
			int n = pPlugin->ProcessConsoleInput(Info);
			if (n == 1)
			{
				nResult = 1;
				break;
			}
			else if (n == 2)
			{
				nResult = 2;
			}
		}
	}

	return nResult;
}
#endif


int PluginManager::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelData,
    size_t *pItemsNumber,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle *)hPlugin;
	*pItemsNumber = 0;
	return ph->pPlugin->GetFindData(ph->hPlugin, pPanelData, pItemsNumber, OpMode);
}


void PluginManager::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	PluginHandle *ph = (PluginHandle *)hPlugin;
	ph->pPlugin->FreeFindData(ph->hPlugin, PanelItem, ItemsNumber);
}


int PluginManager::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelData,
    size_t *pItemsNumber,
    const wchar_t *Path
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	*pItemsNumber=0;
	return ph->pPlugin->GetVirtualFindData(ph->hPlugin, pPanelData, pItemsNumber, Path);
}


void PluginManager::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->FreeVirtualFindData(ph->hPlugin, PanelItem, ItemsNumber);
}


int PluginManager::SetDirectory(
    HANDLE hPlugin,
    const wchar_t *Dir,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->SetDirectory(ph->hPlugin, Dir, OpMode);
}


int PluginManager::GetFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    const wchar_t *DestPath,
    string &strResultName,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen *SaveScr=nullptr;
	int Found=FALSE;
	KeepUserScreen=FALSE;

	if (!(OpMode & OPM_FIND))
		SaveScr = new SaveScreen; //???

	UndoGlobalSaveScrPtr UndSaveScr(SaveScr);
	int GetCode = ph->pPlugin->GetFiles(ph->hPlugin, PanelItem, 1, 0, &DestPath, OpMode);
	string strFindPath;
	strFindPath = DestPath;
	AddEndSlash(strFindPath);
	strFindPath += L"*";
	FAR_FIND_DATA_EX fdata;
	FindFile Find(strFindPath);
	bool Done = true;
	while(Find.Get(fdata))
	{
		if(!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			Done = false;
			break;
		}
	}

	if (!Done)
	{
		strResultName = DestPath;
		AddEndSlash(strResultName);
		strResultName += fdata.strFileName;

		if (GetCode!=1)
		{
			apiSetFileAttributes(strResultName,FILE_ATTRIBUTE_NORMAL);
			apiDeleteFile(strResultName); //BUGBUG
		}
		else
			Found=TRUE;
	}

	ReadUserBackgound(SaveScr);
	delete SaveScr;
	return Found;
}


int PluginManager::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen SaveScr;
	KeepUserScreen=FALSE;
	int Code = ph->pPlugin->DeleteFiles(ph->hPlugin, PanelItem, ItemsNumber, OpMode);

	if (Code)
		ReadUserBackgound(&SaveScr); //???

	return Code;
}


int PluginManager::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen SaveScr;
	KeepUserScreen=FALSE;
	int Code = ph->pPlugin->MakeDirectory(ph->hPlugin, Name, OpMode);

	if (Code != -1)   //???BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	KeepUserScreen=FALSE;
	int Code = ph->pPlugin->ProcessHostFile(ph->hPlugin, PanelItem, ItemsNumber, OpMode);

	if (Code)   //BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph=(PluginHandle*)hPlugin;
	return ph->pPlugin->GetFiles(ph->hPlugin, PanelItem, ItemsNumber, Move, DestPath, OpMode);
}


int PluginManager::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    int OpMode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	KeepUserScreen=FALSE;
	int Code = ph->pPlugin->PutFiles(ph->hPlugin, PanelItem, ItemsNumber, Move, OpMode);

	if (Code)   //BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}

void PluginManager::GetOpenPanelInfo(
    HANDLE hPlugin,
    OpenPanelInfo *Info
)
{
	if (!Info)
		return;

	ClearStruct(*Info);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ph->pPlugin->GetOpenPanelInfo(ph->hPlugin, Info);

	if (!Info->CurDir)  //хмм...
		Info->CurDir = L"";

	if ((Info->Flags & OPIF_REALNAMES) && (CtrlObject->Cp()->ActivePanel->GetPluginHandle() == hPlugin) && *Info->CurDir && ParsePath(Info->CurDir)!=PATH_UNKNOWN)
		apiSetCurrentDirectory(Info->CurDir, false);
}


int PluginManager::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->ProcessKey(ph->hPlugin, Rec, Pred);
}


int PluginManager::ProcessEvent(
    HANDLE hPlugin,
    int Event,
    void *Param
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->ProcessPanelEvent(ph->hPlugin, Event, Param);
}


int PluginManager::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    unsigned int Mode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->Compare(ph->hPlugin, Item1, Item2, Mode);
}

void PluginManager::ConfigureCurrent(Plugin *pPlugin, const GUID& Guid)
{
	if (pPlugin->Configure(Guid))
	{
		int PMode[2];
		PMode[0]=CtrlObject->Cp()->LeftPanel->GetMode();
		PMode[1]=CtrlObject->Cp()->RightPanel->GetMode();

		for (size_t I=0; I < ARRAYSIZE(PMode); ++I)
		{
			if (PMode[I] == PLUGIN_PANEL)
			{
				Panel *pPanel=(I?CtrlObject->Cp()->RightPanel:CtrlObject->Cp()->LeftPanel);
				pPanel->Update(UPDATE_KEEP_SELECTION);
				pPanel->SetViewMode(pPanel->GetViewMode());
				pPanel->Redraw();
			}
		}
		pPlugin->SaveToCache();
	}
}

struct PluginMenuItemData
{
	Plugin *pPlugin;
	GUID Guid;
};

/* $ 29.05.2001 IS
   ! При настройке "параметров внешних модулей" закрывать окно с их
     списком только при нажатии на ESC
*/
void PluginManager::Configure(int StartPos)
{
	// Полиция 4 - Параметры внешних модулей
	if (Opt.Policies.DisabledOptions&FFPOL_MAINMENUPLUGINS)
		return;

	int PrevMacroMode = CtrlObject->Macro.GetMode();
	CtrlObject->Macro.SetMode(MACRO_MENU);

	{
		VMenu PluginList(MSG(MPluginConfigTitle),nullptr,0,ScrY-4);
		PluginList.SetFlags(VMENU_WRAPMODE);
		PluginList.SetHelp(L"PluginsConfig");

		for (;;)
		{
			bool NeedUpdateItems = true;
			bool HotKeysPresent = PlHotkeyCfg->HotkeysPresent(PluginsHotkeysConfig::CONFIG_MENU);

			if (NeedUpdateItems)
			{
				PluginList.ClearDone();
				PluginList.DeleteItems();
				PluginList.SetPosition(-1,-1,0,0);
				LoadIfCacheAbsent();
				string strHotKey, strName;
				GUID guid;

				for (size_t I=0; I<PluginsCount; I++)
				{
					Plugin *pPlugin = PluginsData[I];
					bool bCached = pPlugin->CheckWorkFlags(PIWF_CACHED)?true:false;
					unsigned __int64 id = 0;

					PluginInfo Info = {sizeof(Info)};
					if (bCached)
					{
						id = PlCacheCfg->GetCacheID(pPlugin->GetCacheName());
					}
					else
					{
						if (!pPlugin->GetPluginInfo(&Info))
							continue;
					}

					for (size_t J=0; ; J++)
					{
						if (bCached)
						{
							string strGuid;

							if (!PlCacheCfg->GetPluginsConfigMenuItem(id, J, strName, strGuid))
								break;
							if (!StrToGuid(strGuid,guid))
								break;
						}
						else
						{
							if (J >= Info.PluginConfig.Count)
								break;

							strName = Info.PluginConfig.Strings[J];
							guid = Info.PluginConfig.Guids[J];
						}

						GetPluginHotKey(pPlugin,guid,PluginsHotkeysConfig::CONFIG_MENU,strHotKey);
						MenuItemEx ListItem;
						ListItem.Clear();

#ifndef NO_WRAPPER
						if (pPlugin->IsOemPlugin())
							ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
						if (!HotKeysPresent)
							ListItem.strName = strName;
						else if (!strHotKey.IsEmpty())
							ListItem.strName.Format(L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), strName.CPtr());
						else
							ListItem.strName.Format(L"   %s", strName.CPtr());

						PluginMenuItemData item;
						item.pPlugin = pPlugin;
						item.Guid = guid;
						PluginList.SetUserData(&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
					}
				}

				PluginList.AssignHighlights(FALSE);
				PluginList.SetBottomTitle(MSG(MPluginHotKeyBottom));
				PluginList.ClearDone();
				PluginList.SortItems(0,HotKeysPresent?3:0);
				PluginList.SetSelectPos(StartPos,1);
				NeedUpdateItems = false;
			}

			string strPluginModuleName;
			PluginList.Show();

			while (!PluginList.Done())
			{
				CtrlObject->Macro.SetMode(MACRO_MENU);
				DWORD Key=PluginList.ReadInput();
				int SelPos=PluginList.GetSelectPos();
				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,SelPos);

				switch (Key)
				{
					case KEY_SHIFTF1:
						if (item)
						{
							strPluginModuleName = item->pPlugin->GetModuleName();
							if (!pluginapi::apiShowHelp(strPluginModuleName,L"Config",FHELP_SELFHELP|FHELP_NOSHOWERROR) &&
							        !pluginapi::apiShowHelp(strPluginModuleName,L"Configure",FHELP_SELFHELP|FHELP_NOSHOWERROR))
							{
								pluginapi::apiShowHelp(strPluginModuleName,nullptr,FHELP_SELFHELP|FHELP_NOSHOWERROR);
							}
						}
						break;

					case KEY_F3:
						if (item)
						{
							ShowPluginInfo(item->pPlugin, item->Guid);
						}
						break;

					case KEY_F4:
						if (item)
						{
							string strTitle;
							int nOffset = HotKeysPresent?3:0;
							strTitle = PluginList.GetItemPtr()->strName.CPtr()+nOffset;
							RemoveExternalSpaces(strTitle);

							if (SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::CONFIG_MENU, strTitle))
							{
								PluginList.Hide();
								NeedUpdateItems = true;
								StartPos = SelPos;
								PluginList.SetExitCode(SelPos);
								PluginList.Show();
								break;
							}
						}
						break;

					default:
						PluginList.ProcessInput();
						break;
				}
			}

			if (!NeedUpdateItems)
			{
				StartPos=PluginList.Modal::GetExitCode();
				PluginList.Hide();

				if (StartPos<0)
					break;

				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,StartPos);
				ConfigureCurrent(item->pPlugin, item->Guid);
			}
		}
	}

	CtrlObject->Macro.SetMode(PrevMacroMode);
}

int PluginManager::CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName)
{
	if (ModalType == MODALTYPE_DIALOG)
	{
		if (static_cast<Dialog*>(FrameManager->GetCurrentFrame())->CheckDialogMode(DMODE_NOPLUGINS))
		{
			return 0;
		}
	}

	int PrevMacroMode = CtrlObject->Macro.GetMode();
	CtrlObject->Macro.SetMode(MACRO_MENU);

	bool Editor = ModalType==MODALTYPE_EDITOR;
	bool Viewer = ModalType==MODALTYPE_VIEWER;
	bool Dialog = ModalType==MODALTYPE_DIALOG;

	PluginMenuItemData item;

	{
		VMenu PluginList(MSG(MPluginCommandsMenuTitle),nullptr,0,ScrY-4);
		PluginList.SetFlags(VMENU_WRAPMODE);
		PluginList.SetHelp(L"PluginCommands");
		bool NeedUpdateItems = true;
		bool Done = false;

		while (!Done)
		{
			bool HotKeysPresent = PlHotkeyCfg->HotkeysPresent(PluginsHotkeysConfig::PLUGINS_MENU);

			if (NeedUpdateItems)
			{
				PluginList.ClearDone();
				PluginList.DeleteItems();
				PluginList.SetPosition(-1,-1,0,0);
				LoadIfCacheAbsent();
				string strHotKey, strName;
				GUID guid;

				for (size_t I=0; I<PluginsCount; I++)
				{
					Plugin *pPlugin = PluginsData[I];
					bool bCached = pPlugin->CheckWorkFlags(PIWF_CACHED)?true:false;
					UINT64 IFlags;
					unsigned __int64 id = 0;

					PluginInfo Info = {sizeof(Info)};
					if (bCached)
					{
						id = PlCacheCfg->GetCacheID(pPlugin->GetCacheName());
						IFlags = PlCacheCfg->GetFlags(id);
					}
					else
					{
						if (!pPlugin->GetPluginInfo(&Info))
							continue;

						IFlags = Info.Flags;
					}

					if ((Editor && !(IFlags & PF_EDITOR)) ||
					        (Viewer && !(IFlags & PF_VIEWER)) ||
					        (Dialog && !(IFlags & PF_DIALOG)) ||
					        (!Editor && !Viewer && !Dialog && (IFlags & PF_DISABLEPANELS)))
						continue;

					for (size_t J=0; ; J++)
					{
						if (bCached)
						{
							string strGuid;

							if (!PlCacheCfg->GetPluginsMenuItem(id, J, strName, strGuid))
								break;
							if (!StrToGuid(strGuid,guid))
								break;
						}
						else
						{
							if (J >= Info.PluginMenu.Count)
								break;

							strName = Info.PluginMenu.Strings[J];
							guid = Info.PluginMenu.Guids[J];
						}

						GetPluginHotKey(pPlugin,guid,PluginsHotkeysConfig::PLUGINS_MENU,strHotKey);
						MenuItemEx ListItem;
						ListItem.Clear();
#ifndef NO_WRAPPER
						if (pPlugin->IsOemPlugin())
							ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
						if (!HotKeysPresent)
							ListItem.strName = strName;
						else if (!strHotKey.IsEmpty())
							ListItem.strName.Format(L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), strName.CPtr());
						else
							ListItem.strName.Format(L"   %s", strName.CPtr());

						PluginMenuItemData item;
						item.pPlugin = pPlugin;
						item.Guid = guid;
						PluginList.SetUserData(&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
					}
				}

				PluginList.AssignHighlights(FALSE);
				PluginList.SetBottomTitle(MSG(MPluginHotKeyBottom));
				PluginList.SortItems(0,HotKeysPresent?3:0);
				PluginList.SetSelectPos(StartPos,1);
				NeedUpdateItems = false;
			}

			PluginList.Show();

			while (!PluginList.Done())
			{
				CtrlObject->Macro.SetMode(MACRO_MENU);
				DWORD Key=PluginList.ReadInput();
				int SelPos=PluginList.GetSelectPos();
				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,SelPos);

				switch (Key)
				{
					case KEY_SHIFTF1:
						// Вызываем нужный топик, который передали в CommandsMenu()
						if (item)
							pluginapi::apiShowHelp(item->pPlugin->GetModuleName(),HistoryName,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
						break;

					case KEY_ALTF11:
					case KEY_RALTF11:
						WriteEvent(FLOG_PLUGINSINFO);
						break;


					case KEY_F3:
						if (item)
						{
							ShowPluginInfo(item->pPlugin, item->Guid);
						}
						break;

					case KEY_F4:
						if (item)
						{
							string strTitle;
							int nOffset = HotKeysPresent?3:0;
							strTitle = PluginList.GetItemPtr()->strName.CPtr()+nOffset;
							RemoveExternalSpaces(strTitle);

							if (SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::PLUGINS_MENU, strTitle))
							{
								PluginList.Hide();
								NeedUpdateItems = true;
								StartPos = SelPos;
								PluginList.SetExitCode(SelPos);
								PluginList.Show();
							}
						}
						break;

					case KEY_ALTSHIFTF9:
					case KEY_RALTSHIFTF9:
					{
						if (item)
						{
							PluginList.Hide();
							NeedUpdateItems = true;
							StartPos = SelPos;
							PluginList.SetExitCode(SelPos);
							Configure();
							PluginList.Show();
						}
						break;
					}

					case KEY_SHIFTF9:
					{
						if (item)
						{
							NeedUpdateItems = true;
							StartPos=SelPos;

							if (item->pPlugin->HasConfigure())
								ConfigureCurrent(item->pPlugin, item->Guid);

							PluginList.SetExitCode(SelPos);
							PluginList.Show();
						}

						break;
					}

					default:
						PluginList.ProcessInput();
						break;
				}
			}

			if (!NeedUpdateItems && PluginList.Done())
				break;
		}

		int ExitCode=PluginList.Modal::GetExitCode();
		PluginList.Hide();

		if (ExitCode<0)
		{
			CtrlObject->Macro.SetMode(PrevMacroMode);
			return FALSE;
		}

		ScrBuf.Flush();
		item = *(PluginMenuItemData*)PluginList.GetUserData(nullptr,0,ExitCode);
	}

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	int OpenCode=OPEN_PLUGINSMENU;
	intptr_t Item=0;
	OpenDlgPluginData pd={sizeof(OpenDlgPluginData)};

	if (Editor)
	{
		OpenCode=OPEN_EDITOR;
	}
	else if (Viewer)
	{
		OpenCode=OPEN_VIEWER;
	}
	else if (Dialog)
	{
		OpenCode=OPEN_DIALOG;
		pd.hDlg=(HANDLE)FrameManager->GetCurrentFrame();
		Item=(intptr_t)&pd;
	}

	HANDLE hPlugin=Open(item.pPlugin,OpenCode,item.Guid,Item);

	if (hPlugin && !Editor && !Viewer && !Dialog)
	{
		if (ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		{
			ClosePanel(hPlugin);
			return FALSE;
		}

		Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
		NewPanel->SetPluginMode(hPlugin,L"",true);
		NewPanel->Update(0);
		NewPanel->Show();
	}

	// restore title for old plugins only.
#ifndef NO_WRAPPER
	if (item.pPlugin->IsOemPlugin() && Editor && CurEditor)
	{
		CurEditor->SetPluginTitle(nullptr);
	}
#endif // NO_WRAPPER
	CtrlObject->Macro.SetMode(PrevMacroMode);
	return TRUE;
}

void PluginManager::GetHotKeyPluginKey(Plugin *pPlugin, string &strPluginKey)
{
	/*
	FarPath
	C:\Program Files\Far\

	ModuleName                                             PluginName
	---------------------------------------------------------------------------------------
	C:\Program Files\Far\Plugins\MultiArc\MULTIARC.DLL  -> Plugins\MultiArc\MULTIARC.DLL
	C:\MultiArc\MULTIARC.DLL                            -> C:\MultiArc\MULTIARC.DLL
	---------------------------------------------------------------------------------------
	*/
	strPluginKey = pPlugin->GetHotkeyName();
#ifndef NO_WRAPPER
	size_t FarPathLength=g_strFarPath.GetLength();
	if (pPlugin->IsOemPlugin() && FarPathLength < pPlugin->GetModuleName().GetLength() && !StrCmpNI(pPlugin->GetModuleName(), g_strFarPath, (int)FarPathLength))
		strPluginKey.LShift(FarPathLength);
#endif // NO_WRAPPER
}

void PluginManager::GetPluginHotKey(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, string &strHotKey)
{
	string strPluginKey;
	strHotKey.Clear();
	GetHotKeyPluginKey(pPlugin, strPluginKey);
	strHotKey = PlHotkeyCfg->GetHotkey(strPluginKey, GuidToStr(Guid), HotKeyType);
}

bool PluginManager::SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, const wchar_t *DlgPluginTitle)
{
	string strPluginKey;
	GetHotKeyPluginKey(pPlugin, strPluginKey);
	string strGuid = GuidToStr(Guid);
	string strHotKey = PlHotkeyCfg->GetHotkey(strPluginKey, strGuid, HotKeyType);

	DialogBuilder Builder(MPluginHotKeyTitle, L"SetHotKeyDialog");
	Builder.AddText(MPluginHotKey);
	Builder.AddTextAfter(Builder.AddFixEditField(&strHotKey, 1), DlgPluginTitle);
	Builder.AddOKCancel();
	if(Builder.ShowDialog())
	{
		if (!strHotKey.IsEmpty() && strHotKey.At(0) != L' ')
			PlHotkeyCfg->SetHotkey(strPluginKey, strGuid, HotKeyType, strHotKey);
		else
			PlHotkeyCfg->DelHotkey(strPluginKey, strGuid, HotKeyType);
		return true;
	}
	return false;
}

void PluginManager::ShowPluginInfo(Plugin *pPlugin, const GUID& Guid)
{
	string strPluginGuid = GuidToStr(pPlugin->GetGUID());
	string strItemGuid = GuidToStr(Guid);
	string strPluginPrefix;
	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned __int64 id = PlCacheCfg->GetCacheID(pPlugin->GetCacheName());
		strPluginPrefix = PlCacheCfg->GetCommandPrefix(id);
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (pPlugin->GetPluginInfo(&Info))
		{
			strPluginPrefix = Info.CommandPrefix;
		}
	}
	const int Width = 36;
	DialogBuilder Builder(MPluginInformation, L"ShowPluginInfo");
	Builder.AddText(MPluginModuleTitle);
	Builder.AddConstEditField(pPlugin->GetTitle(), Width);
	Builder.AddText(MPluginDescription);
	Builder.AddConstEditField(pPlugin->GetDescription(), Width);
	Builder.AddText(MPluginAuthor);
	Builder.AddConstEditField(pPlugin->GetAuthor(), Width);
	Builder.AddText(MPluginVersion);
	Builder.AddConstEditField(pPlugin->GetVersionString(), Width);
	Builder.AddText(MPluginModulePath);
	Builder.AddConstEditField(pPlugin->GetModuleName(), Width);
	Builder.AddText(MPluginGUID);
	Builder.AddConstEditField(strPluginGuid, Width);
	Builder.AddText(MPluginItemGUID);
	Builder.AddConstEditField(strItemGuid, Width);
	Builder.AddText(MPluginPrefix);
	Builder.AddConstEditField(strPluginPrefix, Width);
	Builder.AddOK();
	Builder.ShowDialog();
}

char* BufReserve(char*& Buf, size_t Count, size_t& Rest, size_t& Size)
{
	char* Res = nullptr;

	if (Buf)
	{
		if (Rest >= Count)
		{
			Res = Buf;
			Buf += Count;
			Rest -= Count;
		}
		else
		{
			Buf += Rest;
			Rest = 0;
		}
	}

	Size += Count;
	return Res;
}


wchar_t* StrToBuf(const string& Str, char*& Buf, size_t& Rest, size_t& Size)
{
	size_t Count = (Str.GetLength() + 1) * sizeof(wchar_t);
	wchar_t* Res = reinterpret_cast<wchar_t*>(BufReserve(Buf, Count, Rest, Size));
	if (Res)
	{
		wcscpy(Res, Str);
	}
	return Res;
}


void ItemsToBuf(PluginMenuItem& Menu, TArray<string>& NamesArray, TArray<string>& GuidsArray, char*& Buf, size_t& Rest, size_t& Size)
{
	Menu.Count = NamesArray.getSize();
	Menu.Strings = nullptr;
	Menu.Guids = nullptr;

	if (Menu.Count)
	{
		wchar_t** Items = reinterpret_cast<wchar_t**>(BufReserve(Buf, Menu.Count * sizeof(wchar_t*), Rest, Size));
		GUID* Guids = reinterpret_cast<GUID*>(BufReserve(Buf, Menu.Count * sizeof(GUID), Rest, Size));
		Menu.Strings = Items;
		Menu.Guids = Guids;

		for (size_t i = 0; i < Menu.Count; ++i)
		{
			wchar_t* pStr = StrToBuf(*NamesArray.getItem(i), Buf, Rest, Size);
			if (Items)
			{
				Items[i] = pStr;
			}

			if (Guids)
			{
				GUID Guid;
				if (StrToGuid(*GuidsArray.getItem(i), Guid))
				{
					Guids[i] = Guid;
				}
			}
		}
	}
}

size_t PluginManager::GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize)
{
	if(IsPluginUnloaded(pPlugin)) return 0;
	string Prefix;
#if defined(MANTIS_0000466)
	string MacroFunc;
#endif
	PLUGIN_FLAGS Flags = 0;
	TArray<string> MenuNames, MenuGuids, DiskNames, DiskGuids, ConfNames, ConfGuids;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned __int64 id = PlCacheCfg->GetCacheID(pPlugin->GetCacheName());
		Flags = PlCacheCfg->GetFlags(id);
		Prefix = PlCacheCfg->GetCommandPrefix(id);
#if defined(MANTIS_0000466)
		MacroFunc = PlCacheCfg->GetMacroFunctions(id);
#endif

		string Name, Guid;

		for(int i = 0; PlCacheCfg->GetPluginsMenuItem(id, i, Name, Guid); ++i)
		{
			MenuNames.addItem(Name);
			MenuGuids.addItem(Guid);
		}

		for(int i = 0; PlCacheCfg->GetPluginsMenuItem(id, i, Name, Guid); ++i)
		{
			DiskNames.addItem(Name);
			DiskGuids.addItem(Guid);
		}

		for(int i = 0; PlCacheCfg->GetPluginsMenuItem(id, i, Name, Guid); ++i)
		{
			ConfNames.addItem(Name);
			ConfGuids.addItem(Guid);
		}
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (pPlugin->GetPluginInfo(&Info))
		{
			Flags = Info.Flags;
			Prefix = Info.CommandPrefix;
#if defined(MANTIS_0000466)
			MacroFunc = Info.MacroFunctions;
#endif

			for (size_t i = 0; i < Info.PluginMenu.Count; i++)
			{
					MenuNames.addItem(Info.PluginMenu.Strings[i]);
					MenuGuids.addItem(GuidToStr(Info.PluginMenu.Guids[i]));
			}

			for (size_t i = 0; i < Info.DiskMenu.Count; i++)
			{
				DiskNames.addItem(Info.DiskMenu.Strings[i]);
				DiskGuids.addItem(GuidToStr(Info.DiskMenu.Guids[i]));
			}

			for (size_t i = 0; i < Info.PluginConfig.Count; i++)
			{
				ConfNames.addItem(Info.PluginConfig.Strings[i]);
				ConfGuids.addItem(GuidToStr(Info.PluginConfig.Guids[i]));
			}
		}
	}

	FarGetPluginInformation Temp;
	char* Buffer = nullptr;
	size_t Rest = 0;

	if (pInfo)
	{
		Rest = BufferSize - sizeof(FarGetPluginInformation);
		Buffer = reinterpret_cast<char*>(pInfo+1);
	}
	else
	{
		pInfo = &Temp;
	}

	size_t Size = sizeof(FarGetPluginInformation);

	pInfo->ModuleName = StrToBuf(pPlugin->GetModuleName(), Buffer, Rest, Size);

	pInfo->Flags = 0;

	if (pPlugin->m_hModule)
	{
		pInfo->Flags |= FPF_LOADED;
	}
#ifndef NO_WRAPPER
	if (pPlugin->IsOemPlugin())
	{
		pInfo->Flags |= FPF_ANSI;
	}
#endif // NO_WRAPPER

	pInfo->GInfo.StructSize = sizeof(GlobalInfo);
	pInfo->GInfo.Guid = pPlugin->GetGUID();
	pInfo->GInfo.Version = pPlugin->GetVersion();
	pInfo->GInfo.Title = StrToBuf(pPlugin->strTitle, Buffer, Rest, Size);
	pInfo->GInfo.Description = StrToBuf(pPlugin->strDescription, Buffer, Rest, Size);
	pInfo->GInfo.Author = StrToBuf(pPlugin->strAuthor, Buffer, Rest, Size);

	pInfo->PInfo.StructSize = sizeof(PluginInfo);
	pInfo->PInfo.Flags = Flags;
	pInfo->PInfo.CommandPrefix = StrToBuf(Prefix, Buffer, Rest, Size);
#if defined(MANTIS_0000466)
	pInfo->PInfo.MacroFunctions = StrToBuf(MacroFunc, Buffer, Rest, Size);
#endif

	ItemsToBuf(pInfo->PInfo.DiskMenu, DiskNames, DiskGuids, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo.PluginMenu, MenuNames, MenuGuids, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo.PluginConfig, ConfNames, ConfGuids, Buffer, Rest, Size);

	return Size;
}

bool PluginManager::GetDiskMenuItem(
     Plugin *pPlugin,
     size_t PluginItem,
     bool &ItemPresent,
     wchar_t& PluginHotkey,
     string &strPluginText,
     GUID &Guid
)
{
	LoadIfCacheAbsent();

	ItemPresent = false;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		string strGuid;
		if (PlCacheCfg->GetDiskMenuItem(PlCacheCfg->GetCacheID(pPlugin->GetCacheName()), PluginItem, strPluginText, strGuid))
			if (StrToGuid(strGuid,Guid))
				ItemPresent = true;
		ItemPresent = ItemPresent && !strPluginText.IsEmpty();
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};

		if (!pPlugin->GetPluginInfo(&Info) || Info.DiskMenu.Count <= PluginItem)
		{
			ItemPresent = false;
		}
		else
		{
			strPluginText = Info.DiskMenu.Strings[PluginItem];
			Guid = Info.DiskMenu.Guids[PluginItem];
			ItemPresent = true;
		}
	}
	if (ItemPresent)
	{
		string strHotKey;
		GetPluginHotKey(pPlugin,Guid,PluginsHotkeysConfig::DRIVE_MENU,strHotKey);
		PluginHotkey = strHotKey.At(0);
	}

	return true;
}

int PluginManager::UseFarCommand(HANDLE hPlugin,int CommandType)
{
	OpenPanelInfo Info;
	GetOpenPanelInfo(hPlugin,&Info);

	if (!(Info.Flags & OPIF_REALNAMES))
		return FALSE;

	PluginHandle *ph = (PluginHandle*)hPlugin;

	switch (CommandType)
	{
		case PLUGIN_FARGETFILE:
		case PLUGIN_FARGETFILES:
			return(!ph->pPlugin->HasGetFiles() || (Info.Flags & OPIF_EXTERNALGET));
		case PLUGIN_FARPUTFILES:
			return(!ph->pPlugin->HasPutFiles() || (Info.Flags & OPIF_EXTERNALPUT));
		case PLUGIN_FARDELETEFILES:
			return(!ph->pPlugin->HasDeleteFiles() || (Info.Flags & OPIF_EXTERNALDELETE));
		case PLUGIN_FARMAKEDIRECTORY:
			return(!ph->pPlugin->HasMakeDirectory() || (Info.Flags & OPIF_EXTERNALMKDIR));
	}

	return TRUE;
}


void PluginManager::ReloadLanguage()
{
	Plugin *PData;

	for (size_t I=0; I<PluginsCount; I++)
	{
		PData = PluginsData[I];
		PData->CloseLang();
	}

	DiscardCache();
}


void PluginManager::DiscardCache()
{
	for (size_t I=0; I<PluginsCount; I++)
	{
		Plugin *pPlugin = PluginsData[I];
		pPlugin->Load();
	}

	PlCacheCfg->DiscardCache();
}


void PluginManager::LoadIfCacheAbsent()
{
	if (PlCacheCfg->IsCacheEmpty())
	{
		for (size_t I=0; I<PluginsCount; I++)
		{
			Plugin *pPlugin = PluginsData[I];
			pPlugin->Load();
		}
	}
}

//template parameters must have external linkage
struct PluginData
{
	Plugin *pPlugin;
	UINT64 PluginFlags;
};

int PluginManager::ProcessCommandLine(const wchar_t *CommandParam,Panel *Target)
{
	size_t PrefixLength=0;
	string strCommand=CommandParam;
	UnquoteExternal(strCommand);
	RemoveLeadingSpaces(strCommand);

	for (;;)
	{
		wchar_t Ch=strCommand.At(PrefixLength);

		if (!Ch || IsSpace(Ch) || Ch==L'/' || PrefixLength>64)
			return FALSE;

		if (Ch==L':' && PrefixLength>0)
			break;

		PrefixLength++;
	}

	LoadIfCacheAbsent();
	string strPrefix(strCommand,PrefixLength);
	string strPluginPrefix;
	TPointerArray<PluginData> items;

	for (size_t I=0; I<PluginsCount; I++)
	{
		UINT64 PluginFlags=0;

		if (PluginsData[I]->CheckWorkFlags(PIWF_CACHED))
		{
			unsigned __int64 id = PlCacheCfg->GetCacheID(PluginsData[I]->GetCacheName());
			strPluginPrefix = PlCacheCfg->GetCommandPrefix(id);
			PluginFlags = PlCacheCfg->GetFlags(id);
		}
		else
		{
			PluginInfo Info = {sizeof(Info)};

			if (PluginsData[I]->GetPluginInfo(&Info))
			{
				strPluginPrefix = Info.CommandPrefix;
				PluginFlags = Info.Flags;
			}
			else
				continue;
		}

		if (strPluginPrefix.IsEmpty())
			continue;

		const wchar_t *PrStart = strPluginPrefix;
		PrefixLength=strPrefix.GetLength();

		for (;;)
		{
			const wchar_t *PrEnd = wcschr(PrStart, L':');
			size_t Len=PrEnd ? (PrEnd-PrStart):StrLength(PrStart);

			if (Len<PrefixLength)Len=PrefixLength;

			if (!StrCmpNI(strPrefix, PrStart, (int)Len))
			{
				if (PluginsData[I]->Load() && PluginsData[I]->HasOpenPanel())
				{
					PluginData *pD=items.addItem();
					pD->pPlugin=PluginsData[I];
					pD->PluginFlags=PluginFlags;
					break;
				}
			}

			if (!PrEnd)
				break;

			PrStart = ++PrEnd;
		}

		if (items.getCount() && !Opt.PluginConfirm.Prefix)
			break;
	}

	if (!items.getCount())
		return FALSE;

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *CurPanel=(Target)?Target:ActivePanel;

	if (CurPanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		return FALSE;

	PluginData* PData=nullptr;

	if (items.getCount()>1)
	{
		VMenu menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
		menu.SetPosition(-1, -1, 0, 0);
		menu.SetHelp(L"ChoosePluginMenu");
		menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
		MenuItemEx mitem;

		for (size_t i=0; i<items.getCount(); i++)
		{
			mitem.Clear();
			mitem.strName=PointToName(items.getItem(i)->pPlugin->GetModuleName());
			menu.AddItem(&mitem);
		}

		menu.Show();

		while (!menu.Done())
		{
			menu.ReadInput();
			menu.ProcessInput();
		}

		int ExitCode=menu.GetExitCode();

		if (ExitCode>=0)
		{
			PData=items.getItem(ExitCode);
		}
	}
	else
	{
		PData=items.getItem(0);
	}

	if (PData)
	{
		CtrlObject->CmdLine->SetString(L"");
		string strPluginCommand=strCommand.CPtr()+(PData->PluginFlags & PF_FULLCMDLINE ? 0:PrefixLength+1);
		RemoveTrailingSpaces(strPluginCommand);
		HANDLE hPlugin=Open(PData->pPlugin,OPEN_COMMANDLINE,FarGuid,(intptr_t)strPluginCommand.CPtr()); //BUGBUG

		if (hPlugin)
		{
			Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CurPanel,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,L"",!Target || Target == ActivePanel);
			NewPanel->Update(0);
			NewPanel->Show();
		}
	}

	return TRUE;
}


void PluginManager::ReadUserBackgound(SaveScreen *SaveScr)
{
	FilePanels *FPanel=CtrlObject->Cp();
	FPanel->LeftPanel->ProcessingPluginCommand++;
	FPanel->RightPanel->ProcessingPluginCommand++;

	if (KeepUserScreen)
	{
		if (SaveScr)
			SaveScr->Discard();

		RedrawDesktop Redraw;
	}

	FPanel->LeftPanel->ProcessingPluginCommand--;
	FPanel->RightPanel->ProcessingPluginCommand--;
}


/* $ 27.09.2000 SVS
  Функция CallPlugin - найти плагин по ID и запустить
  в зачаточном состоянии!
*/
#ifdef FAR_LUA
int PluginManager::CallPlugin(const GUID& SysID,int OpenFrom, void *Data,void **Ret)
#else
int PluginManager::CallPlugin(const GUID& SysID,int OpenFrom, void *Data,int *Ret)
#endif
{
	if (FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_DIALOG)
	{
		if (static_cast<Dialog*>(FrameManager->GetCurrentFrame())->CheckDialogMode(DMODE_NOPLUGINS))
		{
			return FALSE;
		}
	}

	Plugin *pPlugin = FindPlugin(SysID);

	if (pPlugin)
	{
		if (pPlugin->HasOpenPanel() && !ProcessException)
		{
			HANDLE hNewPlugin=Open(pPlugin,OpenFrom,FarGuid,(intptr_t)Data);
			bool process=false;

			if (OpenFrom == OPEN_FROMMACRO)
			{
				// <????>
				;
				// </????>
			}
			else
			{
				process=OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL;
			}

			if (hNewPlugin && process)
			{
				int CurFocus=CtrlObject->Cp()->ActivePanel->GetFocus();
				Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CtrlObject->Cp()->ActivePanel,FILE_PANEL,TRUE,TRUE);
				NewPanel->SetPluginMode(hNewPlugin,L"",CurFocus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());

				if (Data && *(const wchar_t *)Data)
					SetDirectory(hNewPlugin,(const wchar_t *)Data,0);

				// $ 04.04.2001 SVS
				//	Код закомментирован! Попытка исключить ненужные вызовы в CallPlugin()
				//	Если что-то не так - раскомментировать!!!

				//NewPanel->Update(0);
				//NewPanel->Show();
			}

			if (Ret)
			{
				PluginHandle *handle=(PluginHandle *)hNewPlugin;
#ifdef FAR_LUA
				*Ret = hNewPlugin?handle->hPlugin:nullptr;
#else
				*Ret = (hNewPlugin && handle->hPlugin) ? 1 : 0;
#endif
				delete handle;
			}

			return TRUE;
		}
	}
	return FALSE;
}

// поддержка макрофункций plugin.call, plugin.cmd, plugin.config и т.п
int PluginManager::CallPluginItem(const GUID& Guid, CallPluginInfo *Data, int *Ret/*=nullptr*/)
{
	BOOL Result=FALSE;

	if (!ProcessException)
	{
		int curType = FrameManager->GetCurrentFrame()->GetType();

		if (curType==MODALTYPE_DIALOG)
		{
			if (static_cast<Dialog*>(FrameManager->GetCurrentFrame())->CheckDialogMode(DMODE_NOPLUGINS))
			{
				return FALSE;
			}
		}

		bool Editor = curType==MODALTYPE_EDITOR;
		bool Viewer = curType==MODALTYPE_VIEWER;
		bool Dialog = curType==MODALTYPE_DIALOG;

		if (Data->CallFlags & CPT_CHECKONLY)
		{
			Data->pPlugin = FindPlugin(Guid);
			if (Data->pPlugin && Data->pPlugin->Load())
			{
				// Разрешен ли вызов данного типа в текущей области (предварительная проверка)
				switch ((Data->CallFlags & CPT_MASK))
				{
					case CPT_MENU:
						if (!Data->pPlugin->HasOpenPanel())
							return FALSE;
						break;
					case CPT_CONFIGURE:
						if (curType!=MODALTYPE_PANELS)
						{
							//TODO: Автокомплит не влияет?
							return FALSE;
						}
						if (!Data->pPlugin->HasConfigure())
							return FALSE;
						break;
					case CPT_CMDLINE:
						if (curType!=MODALTYPE_PANELS)
						{
							//TODO: Автокомплит не влияет?
							return FALSE;
						}
						//TODO: OpenPanel или OpenFilePlugin?
						if (!Data->pPlugin->HasOpenPanel())
							return FALSE;
						break;
					case CPT_INTERNAL:
						//TODO: Уточнить функцию
						if (!Data->pPlugin->HasOpenPanel())
							return FALSE;
						break;
				}

				UINT64 IFlags;
				PluginInfo Info = {sizeof(Info)};
				if (!Data->pPlugin->GetPluginInfo(&Info))
					return FALSE;
				else
					IFlags = Info.Flags;

				PluginMenuItem *MenuItems=nullptr;

				// Разрешен ли вызов данного типа в текущей области
				switch ((Data->CallFlags & CPT_MASK))
				{
					case CPT_MENU:
						if ((Editor && !(IFlags & PF_EDITOR)) ||
								(Viewer && !(IFlags & PF_VIEWER)) ||
								(Dialog && !(IFlags & PF_DIALOG)) ||
								(!Editor && !Viewer && !Dialog && (IFlags & PF_DISABLEPANELS)))
							return FALSE;
						MenuItems = &Info.PluginMenu;
						break;
					case CPT_CONFIGURE:
						MenuItems = &Info.PluginConfig;
						break;
					case CPT_CMDLINE:
						if (!Info.CommandPrefix || !*Info.CommandPrefix)
							return FALSE;
						break;
					case CPT_INTERNAL:
						break;
				}

				if ((Data->CallFlags & CPT_MASK)==CPT_MENU || (Data->CallFlags & CPT_MASK)==CPT_CONFIGURE)
				{
					bool ItemFound = false;
					if (Data->ItemGuid==nullptr)
					{
						if (MenuItems->Count==1)
						{
							Data->FoundGuid=MenuItems->Guids[0];
							Data->ItemGuid=&Data->FoundGuid;
							ItemFound=true;
						}
					}
					else
					{
						for (size_t i = 0; i < MenuItems->Count; i++)
						{
							if (memcmp(Data->ItemGuid, &(MenuItems->Guids[i]), sizeof(GUID)) == 0)
							{
								Data->FoundGuid=*Data->ItemGuid;
								Data->ItemGuid=&Data->FoundGuid;
								ItemFound=true;
								break;
							}
						}
					}
					if (!ItemFound)
						return FALSE;
				}
			}

			Result=TRUE;
		}
		else
		{
			if (!Data->pPlugin)
				return FALSE;

			HANDLE hPlugin=nullptr;
			Panel *ActivePanel=nullptr;

			switch ((Data->CallFlags & CPT_MASK))
			{
				case CPT_MENU:
				{
					ActivePanel=CtrlObject->Cp()->ActivePanel;
					int OpenCode=OPEN_PLUGINSMENU;
					intptr_t Item=0;
					OpenDlgPluginData pd={sizeof(OpenDlgPluginData)};

					if (Editor)
					{
						OpenCode=OPEN_EDITOR;
					}
					else if (Viewer)
					{
						OpenCode=OPEN_VIEWER;
					}
					else if (Dialog)
					{
						OpenCode=OPEN_DIALOG;
						pd.hDlg=(HANDLE)FrameManager->GetCurrentFrame();
						Item=(intptr_t)&pd;
					}

					hPlugin=Open(Data->pPlugin,OpenCode,Data->FoundGuid,Item);

					Result=TRUE;
					break;
				}

				case CPT_CONFIGURE:
					CtrlObject->Plugins->ConfigureCurrent(Data->pPlugin,Data->FoundGuid);
					return TRUE;

				case CPT_CMDLINE:
				{
					ActivePanel=CtrlObject->Cp()->ActivePanel;
					string command=Data->Command; // Нужна копия строки
					hPlugin=Open(Data->pPlugin,OPEN_COMMANDLINE,FarGuid,(intptr_t)command.CPtr());

					Result=TRUE;
					break;
				}
				case CPT_INTERNAL:
					//TODO: бывший CallPlugin
					//WARNING: учесть, что он срабатывает без переключения MacroState
					break;
			}

			if (hPlugin && !Editor && !Viewer && !Dialog)
			{
				//BUGBUG: Закрытие панели? Нужно ли оно?
				//BUGBUG: В ProcessCommandLine зовется перед Open, а в CPT_MENU - после
				if (ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
				{
					ClosePanel(hPlugin);
					return FALSE;
				}

				Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
				NewPanel->SetPluginMode(hPlugin,L"",true);
				NewPanel->Update(0);
				NewPanel->Show();
			}

			// restore title for old plugins only.
			#ifndef NO_WRAPPER
			if (Data->pPlugin->IsOemPlugin() && Editor && CurEditor)
			{
				CurEditor->SetPluginTitle(nullptr);
			}
			#endif // NO_WRAPPER
		}
	}

	return Result;
}

Plugin *PluginManager::FindPlugin(const GUID& SysID)
{
	Plugin **result=nullptr;
	if(PluginsCache) result=(Plugin**)PluginsCache->query(SysID);
	return result?*result:nullptr;
}

HANDLE PluginManager::Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item)
{
	HANDLE hPlugin = pPlugin->Open(OpenFrom, Guid, Item);
	if (hPlugin)
	{
		PluginHandle *handle = new PluginHandle;
		handle->hPlugin = hPlugin;
		handle->pPlugin = pPlugin;
		return handle;
	}

	return hPlugin;
}

void PluginManager::GetCustomData(FileListItem *ListItem)
{
	NTPath FilePath(ListItem->strName);

	for (size_t i=0; i<PluginsCount; i++)
	{
		Plugin *pPlugin = PluginsData[i];

		wchar_t *CustomData = nullptr;

		if (pPlugin->HasGetCustomData() && pPlugin->GetCustomData(FilePath.CPtr(), &CustomData))
		{
			if (!ListItem->strCustomData.IsEmpty())
				ListItem->strCustomData += L" ";
			ListItem->strCustomData += CustomData;

			if (pPlugin->HasFreeCustomData())
				pPlugin->FreeCustomData(CustomData);
		}
	}
}

const GUID& PluginManager::GetGUID(HANDLE hPlugin)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->GetGUID();
}

void PluginManager::RefreshPluginsList()
{
	if(!UnloadedPlugins.Empty())
	{
		for(Plugin** p = UnloadedPlugins.First(); p; p = UnloadedPlugins.Next(p))
		{
			(*p)->Unload(true);
			RemovePlugin(*p);
		}
		UnloadedPlugins.Clear();
	}
}

void PluginManager::UndoRemove(Plugin* plugin)
{
	for(Plugin** p = UnloadedPlugins.First(); p; p = UnloadedPlugins.Next(p))
	{
		if(*p == plugin)
		{
			UnloadedPlugins.Delete(p);
			break;
		}
	}
}
