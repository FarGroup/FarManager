/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugins.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "plugin.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "global.hpp"
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
#include "BlockExtKey.hpp"

#ifdef _MSC_VER
#pragma warning(disable:4509)
#endif


const wchar_t *FmtPluginsCache_PluginDW=L"PluginsCache\\Plugin%d";
const wchar_t *FmtPluginsCache_PluginDExportW=L"PluginsCache\\Plugin%d\\Exports";
const wchar_t *FmtDiskMenuStringDW=L"DiskMenuString%d";
const wchar_t *FmtDiskMenuNumberDW=L"DiskMenuNumber%d";
const wchar_t *FmtPluginMenuStringDW=L"PluginMenuString%d";
const wchar_t *FmtPluginConfigStringDW=L"PluginConfigString%d";



static const wchar_t *RKN_PluginsCacheW=L"PluginsCache";

static int _cdecl PluginsSort(const void *el1,const void *el2);


PluginManager::PluginManager()
{
  PluginsData=NULL;
  PluginsCount=0;
  CurPluginItem=NULL;
  Reserved=0;
  CurEditor=NULL;
  CurViewer=NULL;
}



PluginManager::~PluginManager()
{
	CurPluginItem=NULL;

	Plugin *pPlugin;

	for (int i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		pPlugin->Unload ();

		delete pPlugin;
	}

	xf_free (PluginsData);
}

int PluginManager::AddPlugin (Plugin *pPlugin)
{
	Plugin **NewPluginsData=(Plugin**)xf_realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));

	if ( !NewPluginsData )
		return FALSE;

	PluginsData = NewPluginsData;
	PluginsData[PluginsCount]=pPlugin;
	PluginsCount++;

	return TRUE;
}

int PluginManager::RemovePlugin (Plugin *pPlugin)
{
	for (int i = 0; i < PluginsCount; i++)
	{
		if ( PluginsData[i] == pPlugin )
		{
			delete pPlugin;

			memmove (&PluginsData[i], &PluginsData[i+1], (PluginsCount-i-1)*sizeof (Plugin*));
			PluginsCount--;

			return TRUE;
		}
	}

	return FALSE;
}


int PluginManager::LoadPlugin (
		const wchar_t *lpwszModuleName,
		const FAR_FIND_DATA_EX *fdata,
		Plugin **pLoadedPlugin
		)
{
	if ( pLoadedPlugin )
		*pLoadedPlugin = NULL;

	Plugin *pPlugin = GetPlugin (lpwszModuleName);

	if ( !pPlugin )
	{
		pPlugin = new Plugin(this, lpwszModuleName);

		if ( !pPlugin )
			return FALSE;

		AddPlugin (pPlugin);
	}

	BOOL bResult = pPlugin->LoadFromCache();

    if ( !bResult && !Opt.LoadPlug.PluginsCacheOnly )
    {
    	bResult = pPlugin->Load();

    	if ( !bResult )
    		RemovePlugin (pPlugin);
	}

	if ( bResult )
	{
		far_qsort(PluginsData, PluginsCount, sizeof(*PluginsData), PluginsSort);

		if ( pLoadedPlugin )
			*pLoadedPlugin = pPlugin;

		return TRUE;
	}

	return FALSE;
}

int PluginManager::UnloadPlugin (Plugin *pPlugin, DWORD dwException, bool bRemove)
{
	int nResult = FALSE;

	if ( pPlugin && (dwException != EXCEPT_EXITFAR) ) //схитрим, если упали в EXITFAR, не полезем в рекурсию, мы и так в Unload
	{
		//какие-то непонятные действия...

		CurPluginItem=NULL;

		Frame *frame;

		if ( (frame = FrameManager->GetBottomFrame()) != NULL )
			frame->Unlock();

		if ( Flags.Check(PSIF_DIALOG) ) // BugZ#52 exception handling for floating point incorrect
		{
			Flags.Clear(PSIF_DIALOG);
			FrameManager->DeleteFrame();
		}

		//

		bool bPanelPlugin = pPlugin->IsPanelPlugin();

		if ( dwException != (DWORD)-1 )
			nResult = pPlugin->Unload(true);
		else
			nResult = pPlugin->Unload(false);

		if ( bPanelPlugin /*&& bUpdatePanels*/ )
		{
			CtrlObject->Cp()->ActivePanel->SetCurDir(L".",TRUE);

			Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
			ActivePanel->Update(UPDATE_KEEP_SELECTION);
			ActivePanel->Redraw();

			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}

		if ( bRemove )
			RemovePlugin (pPlugin);
	}

	return nResult;
}

Plugin *PluginManager::GetPlugin (const wchar_t *lpwszModuleName)
{
	string strFileName1, strFileName2;

	ConvertNameToShortW (lpwszModuleName, strFileName1);

	Plugin *pPlugin;

	for (int i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		ConvertNameToShortW (pPlugin->m_strModuleName, strFileName2);

		if ( !LocalStricmpW (strFileName1, strFileName2) )
			return pPlugin;
	}

	return NULL;
}

void PluginManager::LoadPlugins()
{
  BlockExtKey blockExtKey;
  Flags.Clear(PSIF_PLUGINSLOADDED);

  if (Opt.LoadPlug.PluginsCacheOnly)  // $ 01.09.2000 tran  '/co' switch
  {
     LoadPluginsFromCache();
     return;
  }

  if(Opt.LoadPlug.MainPluginDir || !Opt.LoadPlug.strCustomPluginsPath.IsEmpty() || (Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty()))
  {
    ScanTree ScTree(FALSE,TRUE);
    UserDefinedListW PluginPathList;  // хранение списка каталогов

    string strPluginsDir;
    string strFullName;
    FAR_FIND_DATA_EX FindData;

    PluginPathList.SetParameters(0,0,ULF_UNIQUE);
    // сначала подготовим список
    if(Opt.LoadPlug.MainPluginDir) // только основные и персональные?
    {
      strPluginsDir.Format (L"%s%s",(const wchar_t*)g_strFarPath, PluginsFolderNameW);
      PluginPathList.AddItem(strPluginsDir);
      // ...а персональные есть?
      if(Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty() && !(Opt.Policies.DisabledOptions&FFPOL_PERSONALPATH))
        PluginPathList.AddItem(Opt.LoadPlug.strPersonalPluginsPath);
    }
    else if( !Opt.LoadPlug.strCustomPluginsPath.IsEmpty() ) // только "заказные" пути?
      PluginPathList.AddItem(Opt.LoadPlug.strCustomPluginsPath);

    const wchar_t *NamePtr;
    PluginPathList.Reset();
    // теперь пройдемся по всему ранее собранному списку
    while(NULL!=(NamePtr=PluginPathList.GetNext()))
    {
      // расширяем значение пути
      apiExpandEnvironmentStrings (NamePtr,strFullName);
      Unquote(strFullName); //??? здесь ХЗ
      if(!PathMayBeAbsoluteW(strFullName))
      {
        strPluginsDir = g_strFarPath;
        strPluginsDir += strFullName;
        strFullName = strPluginsDir;
      }
      // Получим реальное значение полного длинного пути с учетом символических связей.
      ConvertNameToRealW(strFullName,strFullName);
      ConvertNameToLongW(strFullName,strFullName);

      strPluginsDir = strFullName;

      if( strPluginsDir.IsEmpty() ) // Хмм... а нужно ли ЭТО условие после такой модернизации алгоритма загрузки?
        continue;

      // ставим на поток очередной путь из списка...
      ScTree.SetFindPathW(strPluginsDir,L"*.*");

      // ...и пройдемся по нему
      while (ScTree.GetNextNameW(&FindData,strFullName))
      {
        if ( CmpNameW(L"*.dll",FindData.strFileName,FALSE) &&
        	 (FindData.dwFileAttributes & FA_DIREC)==0 )
        	LoadPlugin (strFullName, &FindData);
      } // end while
    }
  }

  Flags.Set(PSIF_PLUGINSLOADDED);
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginManager::LoadPluginsFromCache()
{
  /*
    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0]
    "Name"="C:\\PROGRAM FILES\\FAR\\Plugins\\ABOOK\\AddrBook.dll"
    "ID"="e400a14def00a37ea900"
    "DiskMenuString0"="Address Book"
    "PluginMenuString0"="Address Book"
    "PluginConfigString0"="Address Book"
    "PluginConfigString1"="Address: E-Mail"
    "PluginConfigString2"="Address: Birthday"
    "PluginConfigString3"="Address: Phone number"
    "PluginConfigString4"="Address: Fidonet"
    "CommandPrefix"=""
    "Flags"=dword:00000000

    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0\Exports]
    "OpenPlugin"=dword:00000001
    "OpenFilePlugin"=dword:00000000
    "SetFindList"=dword:00000000
    "ProcessEditorInput"=dword:00000000
    "ProcessEditorEvent"=dword:00000000
  */
	string strRegKey;
	string strModuleName;

	int i = 0;

	while ( EnumRegKey(RKN_PluginsCacheW, i, strRegKey) )
	{
		GetRegKey(strRegKey, L"Name", strModuleName, L"");

		LoadPlugin (strModuleName);

		i++;
	}

	Flags.Set(PSIF_PLUGINSLOADDED);
}
/* tran $ */

int _cdecl PluginsSort(const void *el1,const void *el2)
{
	Plugin *Plugin1=*((Plugin**)el1);
	Plugin *Plugin2=*((Plugin**)el2);
	return (LocalStricmpW(PointToName(Plugin1->m_strModuleName),PointToName(Plugin2->m_strModuleName)));
}



HANDLE PluginManager::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	ConsoleTitle ct(Opt.ShowCheckingFile?UMSG(MCheckingFileInPlugin):NULL);

	Plugin *pPlugin = NULL;

	for (int i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		if ( Opt.ShowCheckingFile )
			ct.Set(L"%s - [%s]...",UMSG(MCheckingFileInPlugin),wcsrchr(pPlugin->m_strModuleName,L'\\')+1);

		HANDLE hPlugin = pPlugin->OpenFilePlugin (Name, Data, DataSize, OpMode);

		if ( (hPlugin == (HANDLE)-2) || (hPlugin == INVALID_HANDLE_VALUE) )
			return hPlugin;

        PluginHandle *handle = new PluginHandle;
        handle->hPlugin = hPlugin;
        handle->pPlugin = pPlugin;

        return (HANDLE)handle;
    }

    return INVALID_HANDLE_VALUE;
}


HANDLE PluginManager::OpenFindListPlugin (const PluginPanelItemW *PanelItem, int ItemsNumber)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	Plugin *pPlugin;

	for (int i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];

		HANDLE hPlugin = pPlugin->OpenPlugin(OPEN_FINDLIST, 0);

		if ( hPlugin != INVALID_HANDLE_VALUE )
		{
			if ( pPlugin->SetFindList (hPlugin, PanelItem, ItemsNumber) )
			{
		        PluginHandle *handle = new PluginHandle;
		        handle->hPlugin = hPlugin;
	    	    handle->pPlugin = pPlugin;

	        	return (HANDLE)handle;
			}

			pPlugin->ClosePlugin (hPlugin);
		}
	}

    return INVALID_HANDLE_VALUE;
}


void PluginManager::ClosePlugin(HANDLE hPlugin)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;

	ph->pPlugin->ClosePlugin(ph->hPlugin);

	delete ph;
}


int PluginManager::ProcessEditorInput (INPUT_RECORD *Rec)
{
	Plugin *pPlugin = NULL;;

    for (int i = 0; i < PluginsCount; i++)
    {
    	pPlugin = PluginsData[i];

    	if ( pPlugin->ProcessEditorInput(Rec) )
    		return TRUE;
	}

	return FALSE;
}


int PluginManager::ProcessEditorEvent(int Event,void *Param)
{
	int nResult = 0;

	if ( CtrlObject->Plugins.CurEditor )
	{
		Plugin *pPlugin = NULL;

        for (int i = 0; i < PluginsCount; i++)
        {
        	pPlugin = PluginsData[i];
        	nResult = pPlugin->ProcessEditorEvent(Event, Param);
		}
	}

	return nResult;
}


int PluginManager::ProcessViewerEvent(int Event, void *Param)
{
	int nResult = 0;

	Plugin *pPlugin = NULL;

	for (int i = 0; i < PluginsCount; i++)
	{
		pPlugin = PluginsData[i];
		nResult = pPlugin->ProcessViewerEvent (Event, Param);
	}

	return nResult;
}


int PluginManager::GetFindData (
		HANDLE hPlugin,
		PluginPanelItemW **pPanelData,
		int *pItemsNumber,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle *)hPlugin;

	return ph->pPlugin->GetFindData (ph->hPlugin, pPanelData, pItemsNumber, OpMode);
}


void PluginManager::FreeFindData (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber
		)
{
	PluginHandle *ph = (PluginHandle *)hPlugin;
	ph->pPlugin->FreeFindData (ph->hPlugin, PanelItem, ItemsNumber);
}


int PluginManager::GetVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItemW **pPanelData,
		int *pItemsNumber,
		const wchar_t *Path
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	PluginHandle *ph = (PluginHandle*)hPlugin;

	*pItemsNumber=0;

	return ph->pPlugin->GetVirtualFindData(ph->hPlugin, pPanelData, pItemsNumber, Path);
}


void PluginManager::FreeVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->FreeVirtualFindData (ph->hPlugin, PanelItem, ItemsNumber);
}


int PluginManager::SetDirectory (
		HANDLE hPlugin,
		const wchar_t *Dir,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;

	return ph->pPlugin->SetDirectory(ph->hPlugin, Dir, OpMode);
}


int PluginManager::GetFile (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		const wchar_t *DestPath,
		string &strResultName,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen *SaveScr=NULL;

	int Found=FALSE;
	KeepUserScreen=FALSE;

	if ((OpMode & OPM_FIND)==0)
		SaveScr = new SaveScreen; //???

	UndoGlobalSaveScrPtr UndSaveScr(SaveScr);

    int GetCode = ph->pPlugin->GetFiles(ph->hPlugin, PanelItem, 1, 0, DestPath, OpMode);

	string strFindPath;

	strFindPath = DestPath;

	AddEndSlash(strFindPath);
	strFindPath += L"*.*";

	FAR_FIND_DATA_EX fdata;

    HANDLE FindHandle;
    if ((FindHandle=apiFindFirstFile(strFindPath,&fdata))!=INVALID_HANDLE_VALUE)
    {
      int Done=0;
      while (!Done)
      {
        if ((fdata.dwFileAttributes & FA_DIREC)==0)
          break;
        Done=!apiFindNextFile(FindHandle,&fdata);
      }
      FindClose(FindHandle);
      if (!Done)
      {
        strResultName = DestPath;
        AddEndSlash(strResultName);
        strResultName += fdata.strFileName;
        if (GetCode!=1)
        {
          SetFileAttributesW(strResultName,FILE_ATTRIBUTE_NORMAL);
          DeleteFileW(strResultName); //BUGBUG
        }
        else
          Found=TRUE;
      }
    }

    ReadUserBackgound(SaveScr);

    delete SaveScr;

	return Found;
}


int PluginManager::DeleteFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	PluginHandle *ph = (PluginHandle*)hPlugin;

    SaveScreen SaveScr;
    KeepUserScreen=FALSE;

    int Code = ph->pPlugin->DeleteFiles(ph->hPlugin, PanelItem, ItemsNumber, OpMode);

    if ( Code )
	    ReadUserBackgound(&SaveScr); //???

    return Code;
}


int PluginManager::MakeDirectory (
		HANDLE hPlugin,
		const wchar_t *Name,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;

    SaveScreen SaveScr;
    KeepUserScreen=FALSE;

    int Code = ph->pPlugin->MakeDirectory(ph->hPlugin, Name, OpMode);

    if ( Code != -1 ) //???BUGBUG
		ReadUserBackgound(&SaveScr);

    return Code;
}


int PluginManager::ProcessHostFile (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

    SaveScreen SaveScr;
    KeepUserScreen=FALSE;

    int Code = ph->pPlugin->ProcessHostFile(ph->hPlugin, PanelItem, ItemsNumber, OpMode);

    if ( Code ) //BUGBUG
	    ReadUserBackgound(&SaveScr);

    return Code;
}


int PluginManager::GetFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int Move,
		const wchar_t *DestPath,
		int OpMode
		)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    PluginHandle *ph=(PluginHandle*)hPlugin;

	return ph->pPlugin->GetFiles(ph->hPlugin, PanelItem, ItemsNumber, Move, DestPath, OpMode);
}


int PluginManager::PutFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

    SaveScreen SaveScr;
    KeepUserScreen=FALSE;

    int Code = ph->pPlugin->PutFiles (ph->hPlugin, PanelItem, ItemsNumber, Move, OpMode);

    if ( Code ) //BUGBUG
	    ReadUserBackgound(&SaveScr);

    return Code;
}


// этой функции вообще быть не должно!!!
int PluginManager::GetPluginInfo (Plugin *pPlugin, PluginInfoW *Info)
{
	if ( !pPlugin )
		MessageBox (0, "null ", "asD", MB_OK);

	pPlugin->GetPluginInfo (Info);
	return TRUE; //BUGBUG
}


void PluginManager::GetOpenPluginInfo (
		HANDLE hPlugin,
		OpenPluginInfoW *Info
		)
{
 	if ( !Info )
		return;

	memset(Info, 0, sizeof(*Info));

	PluginHandle *ph = (PluginHandle*)hPlugin;

	ph->pPlugin->GetOpenPluginInfo (ph->hPlugin, Info);

    if ( Info->CurDir == NULL) //хмм...
		Info->CurDir = L"";
}


int PluginManager::ProcessKey (
		HANDLE hPlugin,
		int Key,
		unsigned int ControlState
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->ProcessKey(ph->hPlugin, Key, ControlState);
}


int PluginManager::ProcessEvent (
		HANDLE hPlugin,
		int Event,
		void *Param
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;

	return ph->pPlugin->ProcessEvent(ph->hPlugin, Event, Param);
}


int PluginManager::Compare (
		HANDLE hPlugin,
		const PluginPanelItemW *Item1,
		const PluginPanelItemW *Item2,
		unsigned int Mode
		)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->Compare(ph->hPlugin, Item1, Item2, Mode);

	//BUGBUG, was return -3
}

void PluginManager::ConfigureCurrent(Plugin *pPlugin, int INum)
{
	if ( pPlugin->Configure(INum) )
	{
		int PMode[2];

		PMode[0]=CtrlObject->Cp()->LeftPanel->GetMode();
		PMode[1]=CtrlObject->Cp()->RightPanel->GetMode();

		for(int I=0; I < sizeof(PMode)/sizeof(PMode[0]); ++I)
		{
			if(PMode[I] == PLUGIN_PANEL)
			{
				Panel *pPanel=(I==0?CtrlObject->Cp()->LeftPanel:CtrlObject->Cp()->RightPanel);
				pPanel->Update(UPDATE_KEEP_SELECTION);
				pPanel->SetViewMode(pPanel->GetViewMode());
				pPanel->Redraw();
			}
		}
	}

    pPlugin->SaveToCache ();
}

struct PluginMenuItemData {
	Plugin *pPlugin;
	int nItem;
};

/* $ 29.05.2001 IS
   ! При настройке "параметров внешних модулей" закрывать окно с их
     списком только при нажатии на ESC
*/
void PluginManager::Configure(int StartPos)
{
  // Полиция 4 - Параметры внешних модулей
  if(Opt.Policies.DisabledOptions&FFPOL_MAINMENUPLUGINS)
    return;

  {
    VMenu PluginList(UMSG(MPluginConfigTitle),NULL,0,ScrY-4);
    PluginList.SetFlags(VMENU_WRAPMODE);
    PluginList.SetHelp(L"PluginsConfig");

    for(;;)
    {
      int I, J;
      BOOL NeedUpdateItems=TRUE;
      BOOL Done=FALSE;
      int MenuItemNumber=0;

      string strFirstHotKey;
      int HotKeysPresent=EnumRegKey(L"PluginHotkeys",0,strFirstHotKey);

      if(NeedUpdateItems)
      {
        PluginList.ClearDone();
        PluginList.DeleteItems();
        PluginList.SetPosition(-1,-1,0,0);
        MenuItemNumber=0;

        LoadIfCacheAbsent();

        string strHotRegKey, strHotKey;

        for (I=0;I<PluginsCount;I++)
        {
          Plugin *pPlugin = PluginsData[I];

          if (pPlugin->WorkFlags.Check(PIWF_CACHED))
          {
            string strRegKey, strValue;
            int RegNumber=pPlugin->GetCacheNumber();
            if (RegNumber==-1)
              continue;
            else
              for (J=0;;J++)
              {
                strHotKey=L"";
                if (GetHotKeyRegKey(pPlugin,J,strHotRegKey))
                  GetRegKey(strHotRegKey,L"ConfHotkey",strHotKey,L"");
                MenuItemEx ListItem;
                ListItem.Clear ();
                strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
                strValue.Format (FmtPluginConfigStringDW,J);
                string strName;
                if ( !GetRegKey(strRegKey,strValue,strName,L"") )
                  break;
                if (!HotKeysPresent)
                  ListItem.strName = strName;
                else
                  if ( !strHotKey.IsEmpty() )
                    ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                  else
                    ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                MenuItemNumber++;

                PluginMenuItemData item;

                item.pPlugin = pPlugin;
                item.nItem = J;

                PluginList.SetUserData((void*)&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
              }
          }
          else
          {
            PluginInfoW Info;
            if (!GetPluginInfo(pPlugin,&Info))
              continue;
            for (J=0;J<Info.PluginConfigStringsNumber;J++)
            {
              strHotKey=L"";
              if (GetHotKeyRegKey(pPlugin,J,strHotRegKey))
                GetRegKey(strHotRegKey,L"ConfHotkey",strHotKey,L"");
              MenuItemEx ListItem;

              ListItem.Clear();
              string strName = NullToEmptyW(Info.PluginConfigStrings[J]);
              if (!HotKeysPresent)
                ListItem.strName = strName;
              else
                if ( !strHotKey.IsEmpty() )
                  ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                else
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
              //ListItem.SetSelect(MenuItemNumber++ == StartPos);
              MenuItemNumber++;

              PluginMenuItemData item;

              item.pPlugin = pPlugin;
              item.nItem = J;
              PluginList.SetUserData((void*)&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
            }
          }
        }
        PluginList.AssignHighlights(FALSE);
        PluginList.SetBottomTitle(UMSG(MPluginHotKeyBottom));

        PluginList.ClearDone();
        PluginList.SortItems(0,3);
        PluginList.SetSelectPos(StartPos,1);
        NeedUpdateItems=FALSE;
      }


      string strPluginModuleName;

      PluginList.Show();

      while (!PluginList.Done())
      {
        int SelPos=PluginList.GetSelectPos();

        PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(NULL,0,SelPos);
        string strRegKey;
        switch(PluginList.ReadInput())
        {
          case KEY_SHIFTF1:

            strPluginModuleName = item->pPlugin->m_strModuleName;
            if(!FarShowHelp(strPluginModuleName,L"Config",FHELP_SELFHELP|FHELP_NOSHOWERROR) &&
               !FarShowHelp(strPluginModuleName,L"Configure",FHELP_SELFHELP|FHELP_NOSHOWERROR))
            {
              //strcpy(PluginModuleName,PluginsData[Data[0]].ModuleName);
              FarShowHelp(strPluginModuleName,NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR);
            }
            break;
          case KEY_F4:
            if (PluginList.GetItemCount() > 0 && SelPos<MenuItemNumber && GetHotKeyRegKey(item->pPlugin, item->nItem,strRegKey))
            {
              BlockExtKey blockExtKey;
              string strName00;

              int nOffset = HotKeysPresent?3:0;

              strName00 = (const wchar_t*)PluginList.GetItemPtr()->strName+nOffset;
              RemoveExternalSpacesW(strName00);

              string strHotKey;

              if(GetMenuHotKeyW(strHotKey,1,
                        (const wchar_t *)MPluginHotKeyTitle,
                        (const wchar_t *)MPluginHotKey,
                        strName00,
                        NULL,strRegKey,L"ConfHotkey"))
              {
                PluginList.Hide();
                NeedUpdateItems=TRUE;
                StartPos=SelPos;
                PluginList.SetExitCode(SelPos);
//              PluginList.Hide();
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

      if(!NeedUpdateItems)
      {
        StartPos=PluginList.Modal::GetExitCode();
        PluginList.Hide();
        if (StartPos<0)
          break;

        PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(NULL,0,StartPos);
        ConfigureCurrent(item->pPlugin, item->nItem);
      }
    }
  }
}
/* IS $ */


///int PluginManager::CommandsMenu(int Editor,int Viewer,int StartPos,char *HistoryName)
int PluginManager::CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName)
{
  int MenuItemNumber=0;
  int Editor,Viewer;

  Editor = ModalType==MODALTYPE_EDITOR;
  Viewer = ModalType==MODALTYPE_VIEWER;

  string strRegKey;

//  {
    VMenu PluginList(UMSG(MPluginCommandsMenuTitle),NULL,0,ScrY-4);
    PluginList.SetFlags(VMENU_WRAPMODE);
    PluginList.SetHelp(L"Plugins");
    BOOL NeedUpdateItems=TRUE;
    BOOL Done=FALSE;


    while(!Done)
    {

      string strFirstHotKey;
      int HotKeysPresent=EnumRegKey(L"PluginHotkeys",0,strFirstHotKey);

      if(NeedUpdateItems)
      {
        PluginList.ClearDone();
        PluginList.DeleteItems();
        PluginList.SetPosition(-1,-1,0,0);

        LoadIfCacheAbsent();

        for (int I=0;I<PluginsCount;I++)
        {
          Plugin *pPlugin = PluginsData[I];

          string strHotRegKey, strHotKey;
          if (pPlugin->WorkFlags.Check(PIWF_CACHED))
          {
            string strRegKey, strValue;
            int RegNumber=pPlugin->GetCacheNumber();
            if (RegNumber==-1)
              continue;
            else
            {
              strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
              int IFlags=GetRegKey(strRegKey,L"Flags",0);
              if (Editor && (IFlags & PF_EDITOR)==0 ||
                Viewer && (IFlags & PF_VIEWER)==0 ||
                !Editor && !Viewer && (IFlags & PF_DISABLEPANELS))
                continue;
              for (int J=0;;J++)
              {
                strHotKey=L"";
                if (GetHotKeyRegKey(pPlugin,J,strHotRegKey))
                  GetRegKey(strHotRegKey,L"Hotkey",strHotKey,L"");
                MenuItemEx ListItem;
                ListItem.Clear();
                strValue.Format (FmtPluginMenuStringDW,J);
                string strName;
                if ( !GetRegKey(strRegKey,strValue,strName,L"") )
                  break;
                if (!HotKeysPresent)
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);//strcpy(ListItem.Name,Name);
                else
                  if ( !strHotKey.IsEmpty() )
                    ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""),(const wchar_t*)strName);
                  else
                    ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                  //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                  MenuItemNumber++;

                  PluginMenuItemData item;

                  item.pPlugin = pPlugin;
                  item.nItem = J;

                  PluginList.SetUserData((void*)&item,sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
              }
            }
          }
          else
          {
            PluginInfoW Info;
            if (!GetPluginInfo(pPlugin,&Info))
              continue;
            if (Editor && (Info.Flags & PF_EDITOR)==0 ||
              Viewer && (Info.Flags & PF_VIEWER)==0 ||
              !Editor && !Viewer && (Info.Flags & PF_DISABLEPANELS))
              continue;
            for (int J=0;J<Info.PluginMenuStringsNumber;J++)
            {
              strHotKey=L"";
              if (GetHotKeyRegKey(pPlugin,J,strHotRegKey))
                GetRegKey(strHotRegKey,L"Hotkey",strHotKey,L"");
              MenuItemEx ListItem;
              ListItem.Clear();
              string strName = NullToEmptyW(Info.PluginMenuStrings[J]);
              if (!HotKeysPresent)
                ListItem.strName.Format (L"   %s", (const wchar_t*)strName);//strcpy(ListItem.Name,Name);
              else
                if ( !strHotKey.IsEmpty() )
                  ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                else
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                MenuItemNumber++;

                PluginMenuItemData item;

                item.pPlugin = pPlugin;
                item.nItem = J;

                PluginList.SetUserData((void*)&item,sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
            }
          }
        }

        PluginList.AssignHighlights(FALSE);
        PluginList.SetBottomTitle(UMSG(MPluginHotKeyBottom));

        PluginList.SortItems(0,3);
        PluginList.SetSelectPos(StartPos,1);
        NeedUpdateItems=FALSE;
      }

      PluginList.Show();

      while (!PluginList.Done())
      {
        int SelPos=PluginList.GetSelectPos();

        PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(NULL,0,SelPos);

        switch(PluginList.ReadInput())
        {
        case KEY_SHIFTF1:
          {
            // Вызываем нужный топик, который передали в CommandsMenu()
            FarShowHelp(item->pPlugin->m_strModuleName,HistoryName,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
            break;
          }
        case KEY_ALTF11:
          WriteEvent(FLOG_PLUGINSINFO);
          break;
        case KEY_F4:
          if (PluginList.GetItemCount() > 0 && SelPos<MenuItemNumber && GetHotKeyRegKey(item->pPlugin, item->nItem, strRegKey))
          {
            BlockExtKey blockExtKey;
            string strName00;

            int nOffset = HotKeysPresent?3:0;

            strName00 = (const wchar_t*)PluginList.GetItemPtr()->strName+nOffset;
            RemoveExternalSpacesW(strName00);

            string strHotKey;

            if(GetMenuHotKeyW(strHotKey,1,(const wchar_t *)MPluginHotKeyTitle,(const wchar_t *)MPluginHotKey,strName00,NULL,strRegKey,L"Hotkey"))
            {
              PluginList.Hide();
              NeedUpdateItems=TRUE;
              StartPos=SelPos;
              PluginList.SetExitCode(SelPos);
//            PluginList.Hide();
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

      if(!NeedUpdateItems && PluginList.Done())
        break;
    }

    int ExitCode=PluginList.Modal::GetExitCode();

    PluginList.Hide();
    if (ExitCode<0)
      return(FALSE);
    ScrBuf.Flush();

    PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(NULL,0,ExitCode);

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

    int OpenCode=OPEN_PLUGINSMENU;
    if (Editor)
      OpenCode=OPEN_EDITOR;
    if (Viewer)
      OpenCode=OPEN_VIEWER;

    HANDLE hPlugin=OpenPlugin(item->pPlugin,OpenCode,item->nItem);

    if (hPlugin!=INVALID_HANDLE_VALUE && !Editor && !Viewer)
    {
      if ( ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL) )
      {
        ClosePlugin (hPlugin);
        return(FALSE);
      }


      Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
      NewPanel->SetPluginMode(hPlugin,L"");
      NewPanel->Update(0);
      NewPanel->Show();
      NewPanel->SetFocus();
    }
    if (Editor && CurEditor){
      CurEditor->SetPluginTitle(NULL);
    }

  return(TRUE);
}

int PluginManager::GetHotKeyRegKey(Plugin *pPlugin,int ItemNumber,string &strRegKey)
{
#if 1
// Старый вариант
/*
FarPath
C:\Program Files\Far\

ModuleName                                             PluginName
---------------------------------------------------------------------------------------
C:\Program Files\Far\Plugins\MultiArc\MULTIARC.DLL  -> Plugins\MultiArc\MULTIARC.DLL
C:\MultiArc\MULTIARC.DLL                            -> DLL
---------------------------------------------------------------------------------------
*/
  size_t FarPathLength=g_strFarPath.GetLength();
  strRegKey=L"";
  if (FarPathLength < pPlugin->m_strModuleName.GetLength())
  {
    string strPluginName;

//    strcpy(PluginName,PluginsData[PluginNumber].ModuleName+FarPathLength);
    strPluginName = (const wchar_t*)pPlugin->m_strModuleName+(wcsncmp(pPlugin->m_strModuleName,g_strFarPath,FarPathLength)?0:FarPathLength);

    wchar_t *Ptr = strPluginName.GetBuffer ((int)strPluginName.GetLength()+20);

    while ( *Ptr )
    {
      if (*Ptr==L'\\')
        *Ptr=L'/';

      Ptr++;
    }

    if (ItemNumber>0)
      swprintf(Ptr,L"%%%d",ItemNumber);

    strPluginName.ReleaseBuffer ();

    strRegKey.Format (L"PluginHotkeys\\%s",(const wchar_t*)strPluginName);
    return(TRUE);
  }
  return(FALSE);
#else
// Новый вариант, с полными путями
  *RegKey=0;
  char PluginName[NM], *Ptr;
  strcpy(PluginName,PluginsData[PluginNumber]->ModuleName);
  for (Ptr=PluginName;*Ptr;++Ptr)
  {
    if (*Ptr=='\\')
      *Ptr='/';
  }
  if (ItemNumber>0)
    sprintf(Ptr,"%%%d",ItemNumber);

  sprintf(RegKey,"PluginHotkeys\\%s",PluginName);

#if 1
/*  // конвертация - потом убрать
  unsigned int FarPathLength=strlen(FarPath);
  if (FarPathLength < strlen(PluginsData[PluginNumber]->ModuleName))
  {
    char OldRevKey[NM];
    sprintf(OldRevKey,"PluginHotkeys\\%s",PluginName+FarPathLength);
    if(CheckRegKey(OldRevKey))
    {
      char ConfHotkey[32];
      char Hotkey[32];
      GetRegKey(OldRevKey,"ConfHotkey",ConfHotkey,"",sizeof(ConfHotkey));
      GetRegKey(OldRevKey,"Hotkey",Hotkey,"",sizeof(Hotkey));
      if(*ConfHotkey)
        SetRegKey(RegKey,"ConfHotkey",ConfHotkey);
      if(*Hotkey)
        SetRegKey(RegKey,"Hotkey",Hotkey);
      DeleteRegKey(OldRevKey); //??? А нужно?
    }
  }*/
#endif
  return(TRUE);
#endif
}

/* $ 21.08.2002 IS
   + Параметр PluginTextSize, чтобы знать, сколько брать
*/
int PluginManager::GetDiskMenuItem(Plugin *pPlugin,int PluginItem,
                int &ItemPresent,int &PluginTextNumber,string &strPluginText)
{
  LoadIfCacheAbsent();

  if (pPlugin->WorkFlags.Check(PIWF_CACHED))
  {
    string strRegKey, strValue;
    int RegNumber=pPlugin->GetCacheNumber();
    if (RegNumber==-1)
      ItemPresent=0;
    else
    {
      strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
      strValue.Format (FmtDiskMenuStringDW,PluginItem);
      GetRegKey(strRegKey,strValue,strPluginText,L"");
      strValue.Format (FmtDiskMenuNumberDW,PluginItem);
      GetRegKey(strRegKey,strValue,PluginTextNumber,0);
      ItemPresent=!strPluginText.IsEmpty();
    }
    return(TRUE);
  }

  PluginInfoW Info;
  if (!GetPluginInfo(pPlugin,&Info) || Info.DiskMenuStringsNumber <= PluginItem)
    ItemPresent=FALSE;
  else
  {
    if (Info.DiskMenuNumbers)
      PluginTextNumber=Info.DiskMenuNumbers[PluginItem];
    else
      PluginTextNumber=0;
    strPluginText = Info.DiskMenuStrings[PluginItem];

    ItemPresent=TRUE;
  }
  return(TRUE);
}
/* IS $ */

int PluginManager::UseFarCommand(HANDLE hPlugin,int CommandType)
{
  OpenPluginInfoW Info;
  GetOpenPluginInfo(hPlugin,&Info);
  if ((Info.Flags & OPIF_REALNAMES)==0)
    return(FALSE);

  PluginHandle *ph = (PluginHandle*)hPlugin;

  switch(CommandType)
  {
    case PLUGIN_FARGETFILE:
    case PLUGIN_FARGETFILES:
      return(ph->pPlugin->pGetFiles==NULL || (Info.Flags & OPIF_EXTERNALGET));
    case PLUGIN_FARPUTFILES:
      return(ph->pPlugin->pPutFiles==NULL || (Info.Flags & OPIF_EXTERNALPUT));
    case PLUGIN_FARDELETEFILES:
      return(ph->pPlugin->pDeleteFiles==NULL || (Info.Flags & OPIF_EXTERNALDELETE));
    case PLUGIN_FARMAKEDIRECTORY:
      return(ph->pPlugin->pMakeDirectory==NULL || (Info.Flags & OPIF_EXTERNALMKDIR));
  }
  return(TRUE);
}


void PluginManager::ReloadLanguage()
{
  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
      PData->Lang.Close();
  }
  DiscardCache();
}


void PluginManager::DiscardCache()
{
  for (int I=0;I<PluginsCount;I++)
  {
    Plugin *pPlugin = PluginsData[I];
    pPlugin->Load();
  }
  DeleteKeyTree(RKN_PluginsCacheW);
}


void PluginManager::LoadIfCacheAbsent()
{
  if (!CheckRegKey(RKN_PluginsCacheW))
    for (int I=0;I<PluginsCount;I++)
    {
      Plugin *pPlugin = PluginsData[I];
      pPlugin->Load();
    }
}


int PluginManager::ProcessCommandLine(const wchar_t *CommandParam,Panel *Target)
{
  size_t PrefixLength=0;

  string strCommand=CommandParam;

  UnquoteExternal(strCommand);
  RemoveLeadingSpacesW(strCommand);

  while (1)
  {
    wchar_t Ch=strCommand.At(PrefixLength);
    if (Ch==0 || IsSpaceW(Ch) || Ch==L'/' || PrefixLength>64)
      return(FALSE);
    if (Ch==L':' && PrefixLength>0)
      break;
    PrefixLength++;
  }

  LoadIfCacheAbsent();

  string strPrefix;

  wchar_t *Prefix = strPrefix.GetBuffer((int)PrefixLength);

  xwcsncpy(Prefix,strCommand,PrefixLength);

  strPrefix.ReleaseBuffer ();

  Plugin *PluginPos=NULL;

  DWORD PluginFlags = 0;
  string strPluginPrefix;

  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->WorkFlags.Check(PIWF_CACHED))
    {
      int RegNumber=PData->GetCacheNumber();
      if (RegNumber!=-1)
      {
        string strRegKey;
        strRegKey.Format (FmtPluginsCache_PluginDW, RegNumber);
        GetRegKey(strRegKey,L"CommandPrefix",strPluginPrefix, L"");
        PluginFlags=GetRegKey(strRegKey,L"Flags",0);
      }
      else
        continue;
    }
    else
    {
      PluginInfoW Info;
      if (GetPluginInfo(PData,&Info))
      {
        strPluginPrefix = NullToEmptyW(Info.CommandPrefix);
        PluginFlags = Info.Flags;
      }
      else
        continue;
    }
    if ( strPluginPrefix.IsEmpty() )
      continue;

    const wchar_t *PrStart = strPluginPrefix;

    PrefixLength=strPrefix.GetLength ();
    while(1)
    {
      const wchar_t *PrEnd = wcschr(PrStart, L':');
      size_t Len=PrEnd==NULL ? wcslen(PrStart):(PrEnd-PrStart);
      if(Len<PrefixLength)Len=PrefixLength;
      if (LocalStrnicmpW(Prefix, PrStart, (int)Len)==0)
      {
        PluginPos=PData;
        break;
      }
      if (PrEnd == NULL)
        break;
      PrStart = ++PrEnd;
    }
    if (PluginPos)
      break;
  }

  if (PluginPos==NULL)
    return(FALSE);
  if ( !PluginPos->Load() || PluginPos->pOpenPlugin==NULL)
    return(FALSE);
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *CurPanel=(Target)?Target:ActivePanel;
  if (CurPanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return(FALSE);
  CtrlObject->CmdLine->SetString(L"");

  string strPluginCommand;

  strPluginCommand = (const wchar_t*)strCommand+(PluginFlags & PF_FULLCMDLINE ? 0:PrefixLength+1);
  RemoveTrailingSpacesW(strPluginCommand);
  HANDLE hPlugin=OpenPlugin(PluginPos,OPEN_COMMANDLINE,(INT_PTR)(const wchar_t*)strPluginCommand); //BUGBUG
  if (hPlugin!=INVALID_HANDLE_VALUE)
  {
    Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CurPanel,FILE_PANEL,TRUE,TRUE);
    NewPanel->SetPluginMode(hPlugin,L"");
    NewPanel->Update(0);
    NewPanel->Show();
    if(!Target || Target == ActivePanel)
      NewPanel->SetFocus();
  }
  return(TRUE);
}


void PluginManager::ReadUserBackgound(SaveScreen *SaveScr)
{
  FilePanels *FPanel=CtrlObject->Cp();
  FPanel->LeftPanel->ProcessingPluginCommand++;
  FPanel->RightPanel->ProcessingPluginCommand++;
  if (KeepUserScreen)
  {
    if(SaveScr)
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
int PluginManager::CallPlugin(DWORD SysID,int OpenFrom, void *Data)
{
  Plugin *pPlugin = FindPlugin(SysID);

  if ( pPlugin )
  {
    if (pPlugin->pOpenPlugin && !ProcessException)
    {
      HANDLE hNewPlugin=OpenPlugin(pPlugin,OpenFrom,(INT_PTR)Data);

      if (hNewPlugin!=INVALID_HANDLE_VALUE &&
         (OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL))
      {
        int CurFocus=CtrlObject->Cp()->ActivePanel->GetFocus();
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CtrlObject->Cp()->ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,L"");
        if (Data && *(const wchar_t *)Data)
          SetDirectory(hNewPlugin,(const wchar_t *)Data,0);
        /* $ 04.04.2001 SVS
           Код закомментирован! Попытка исключить ненужные вызовы в CallPlugin()
           Если что-то не так - раскомментировать!!!
        */
//        NewPanel->Update(0);
        if (CurFocus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible())
          NewPanel->SetFocus();
//        NewPanel->Show();
        /* SVS $ */
      }
      return TRUE;
    }
  }
  return FALSE;
}

Plugin *PluginManager::FindPlugin(DWORD SysID)
{
  if(SysID != 0 && SysID != 0xFFFFFFFFUl) // не допускается 0 и -1
  {
    Plugin *PData;
    for (int I=0;I<PluginsCount;I++)
    {
      PData = PluginsData[I];
      if (PData->SysID == SysID)
        return PData;
    }
  }
  return NULL;
}

HANDLE PluginManager::OpenPlugin(Plugin *pPlugin,int OpenFrom,INT_PTR Item)
{
	HANDLE hPlugin = pPlugin->OpenPlugin (OpenFrom, Item);

	if ( hPlugin != INVALID_HANDLE_VALUE )
	{
		PluginHandle *handle = new PluginHandle;

		handle->hPlugin = hPlugin;
		handle->pPlugin = pPlugin;

		return (HANDLE)handle;
	}

	return hPlugin;
}
/* SVS $ */

/* $ 23.10.2000 SVS
   Функция TestPluginInfo - проверка на вшивость переданных плагином данных
*/
#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL PluginManager::TestPluginInfo(Plugin *Item,PluginInfoW *Info)
{
  if(!Opt.ExceptRules)
    return TRUE;

  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->DiskMenuStringsNumber > 0 && !Info->DiskMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->DiskMenuStringsNumber; I++)
      memcpy(Buf,Info->DiskMenuStrings[I],1);

    if(Info->PluginMenuStringsNumber > 0 && !Info->PluginMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->PluginMenuStringsNumber; I++)
     memcpy(Buf,Info->PluginMenuStrings[I],1);

    if(Info->PluginConfigStringsNumber > 0 && !Info->PluginConfigStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    else for (I=0; I<Info->PluginConfigStringsNumber; I++)
      memcpy(Buf,Info->PluginConfigStrings[I],1);

    if (Info->CommandPrefix)
      memcpy(Buf,Info->CommandPrefix,1);

    I=TRUE;
  }
  EXCEPT(xfilter(EXCEPT_GETPLUGININFO_DATA,GetExceptionInformation(),Item,1))
  {
     UnloadPlugin(Item,EXCEPT_GETPLUGININFO_DATA); // тест не пройден, выгружаем его
     I=FALSE;
//     ProcessException=FALSE;
  }
  return I;
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif
/* SVS $ */

/* $ 31.10.2000 SVS
   Функция TestOpenPluginInfo - проверка на вшивость переданных плагином данных
*/
#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL PluginManager::TestOpenPluginInfo(Plugin *Item,OpenPluginInfoW *Info)
{
  if(!Opt.ExceptRules)
    return TRUE;

  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->HostFile) memcpy(Buf,Info->HostFile,1);
    if(Info->CurDir) memcpy(Buf,Info->CurDir,1);
    if(Info->Format) memcpy(Buf,Info->Format,1);
    if(Info->PanelTitle) memcpy(Buf,Info->PanelTitle,1);

    if(Info->InfoLinesNumber > 0 && !Info->InfoLines)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->InfoLinesNumber; I++)
      memcpy(Buf,&Info->InfoLines[I],1);

    if(Info->DescrFilesNumber > 0 && !Info->DescrFiles)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->DescrFilesNumber; I++)
      memcpy(Buf,Info->DescrFiles[I],1);

    if(Info->PanelModesNumber > 0 && !Info->PanelModesArray)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    for (I=0; I<Info->PanelModesNumber; I++)
      memcpy(Buf,&Info->PanelModesArray[I],1);

    if(Info->KeyBar) memcpy(Buf,Info->KeyBar,1);
    if(Info->ShortcutData) memcpy(Buf,Info->ShortcutData,1);
    I=TRUE;
  }
  EXCEPT(xfilter(EXCEPT_GETOPENPLUGININFO_DATA,GetExceptionInformation(),Item,1))
  {
     UnloadPlugin(Item,EXCEPT_GETOPENPLUGININFO_DATA); // тест не пройден, выгружаем его
     I=FALSE;
//     ProcessException=FALSE;
  }
  return I;
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif
/* SVS $ */
