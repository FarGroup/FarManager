#ifdef __GNUC__
#include <cwchar>
#define swprintf_s(buffer, size, format, ...) swprintf(buffer, format, __VA_ARGS__)
#endif

#include "Plugin.h"
#include <comdef.h>
#include <stddef.h>
#include "resource.h"
#include "MenuDlg.h"
#include "FarMenu.h"
#include "Pidl.h"
#include <shlguid.h>
#include "OleThread.h"
#include "HMenu.h"
#include <PluginSettings.hpp>
#include "guid.hpp"
#include <DlgBuilder.hpp>
#include <cassert>

// new version of PSDK doesn't contain standard smart-pointer declaration
_COM_SMARTPTR_TYPEDEF(IContextMenu, __uuidof(IContextMenu));
_COM_SMARTPTR_TYPEDEF(IContextMenu2, __uuidof(IContextMenu2));
_COM_SMARTPTR_TYPEDEF(IContextMenu3, __uuidof(IContextMenu3));

#ifdef __GNUC__
_COM_SMARTPTR_TYPEDEF(IShellFolder, __uuidof(IShellFolder));
_COM_SMARTPTR_TYPEDEF(IEnumIDList, __uuidof(IEnumIDList));
_COM_SMARTPTR_TYPEDEF(IDropTarget, __uuidof(IDropTarget));
_COM_SMARTPTR_TYPEDEF(IDataObject, __uuidof(IDataObject));
#endif

#ifdef __GNUC__
#define SEH_TRY if(true)
#define SEH_EXCEPT(h) if(false)
#else
#define SEH_TRY __try
#define SEH_EXCEPT(h) __except(h)
#endif

#if _WIN32_WINNT >= 0x0603
# include <versionhelpers.h>
#else
 inline bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
 {
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
		VerSetConditionMask(
		0, VER_MAJORVERSION, VER_GREATER_EQUAL),
		VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;
	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
 }

 inline bool IsWindowsXPOrGreater()
 { return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0); }
#endif

CPlugin::~CPlugin(void)
{
}

CPlugin::CPlugin(const PluginStartupInfo *Info)
{
  m_hModule=(HINSTANCE)GetModuleHandle(Info->ModuleName);
  NULL_HWND=NULL;
  REG_WaitToContinue=L"WaitToContinue";
  REG_UseGUI=L"UseGUI";
  REG_DelUsingFar=L"DelUsingFar";
  REG_ClearSel=L"ClearSelection";
  REG_Silent=L"Silent";
  REG_Helptext=L"Helptext";
  REG_DifferentOnly=L"DifferentOnly";
  REG_GuiPos=L"GuiPos";
  SelectedItems=NULL;
  SelectedItemsCount=0;

  *(PluginStartupInfo*)this=*Info;
  m_fsf=*Info->FSF;

  ReadRegValues();

  m_bWin2K = !IsWindowsXPOrGreater();
}

void CPlugin::ReadRegValues()
{
  PluginSettings settings(MainGuid, SettingsControl);
  m_WaitToContinue=settings.Get(0,REG_WaitToContinue, 0);
  m_UseGUI=settings.Get(0,REG_UseGUI, 2);
  m_DelUsingFar=settings.Get(0,REG_DelUsingFar, 0);
  m_ClearSel=settings.Get(0,REG_ClearSel, 1);
  m_Silent=settings.Get(0,REG_Silent, 0);
  m_enHelptext=(EAdditionalStr)settings.Get(0,REG_Helptext, 0);
  m_DifferentOnly=settings.Get(0,REG_DifferentOnly, 0);
  m_GuiPos=settings.Get(0,REG_GuiPos, 1);
}

void CPlugin::GetPluginInfo(PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_FULLCMDLINE;
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(LNG_TITLE);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
  Info->PluginConfig.Guids=&MenuGuid;
  Info->PluginConfig.Strings=PluginMenuStrings;
  Info->PluginConfig.Count=ARRAYSIZE(PluginMenuStrings);
  Info->CommandPrefix=PFX_RCLK L":" PFX_RCLK_TXT L":" PFX_RCLK_GUI L":" PFX_RCLK_CMD L":" PFX_RCLK_ITEM;
}

LPCWSTR g_szTopicMain=L"Main";
LPCWSTR g_szTopicConfig=L"Config";
LPCWSTR g_szTopicChooseMenuType=L"ChooseMenuType";
LPCWSTR g_szTopicContextMenu=L"ContextMenu";
LPCWSTR g_szTopicMyComp=L"MyComp";
LPCWSTR g_szTopicError0=L"Error0";
LPCWSTR g_szTopicError1=L"Error1";
LPCWSTR g_szTopicError2=L"Error2";
LPCWSTR g_szTopicClose=L"Close";

LPCWSTR CPlugin::GetMsg(int nMsgId)
{
  return PluginStartupInfo::GetMsg(&MainGuid, nMsgId);
}

intptr_t CPlugin::Message(DWORD nFlags, LPCWSTR szHelpTopic, const LPCWSTR* pItems, int nItemsNumber, int nButtonsNumber)
{
  if (m_Silent) return -1;
  return PluginStartupInfo::Message(&MainGuid, nullptr, nFlags, szHelpTopic, pItems, nItemsNumber, nButtonsNumber);
}

INT_PTR WINAPI CPlugin::CfgDlgProcStatic(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
  static CPlugin* pThis=NULL;
  if (DN_INITDIALOG==Msg)
  {
    pThis=reinterpret_cast<CPlugin*>(Param2);
  }
  if (pThis) pThis->CfgDlgProc(hDlg, Msg, Param1, Param2);
  return thePlug->DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CPlugin::CfgDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
  if (DN_INITDIALOG==Msg)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId, (void *)(0==SendDlgMessage(hDlg, DM_GETCHECK, m_nSilentId, 0)));
    SendDlgMessage(hDlg, DM_ENABLE, m_nDifferentId, (void *)(0==SendDlgMessage(hDlg, DM_GETCHECK, m_nDifferentId-3, 0)));
  }
  if (DN_BTNCLICK==Msg && m_nSilentId==Param1)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId, (void *)(0==Param2));
  }
  if (DN_BTNCLICK==Msg && Param1>=m_nDifferentId-2 && Param1<m_nDifferentId)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nDifferentId, (void *)(0!=Param2));
  }
}

int CPlugin::Configure()
{
  int enHelptext = 0;
  if (m_enHelptext==AS_VERB)
    enHelptext=2;
  else if (m_enHelptext==AS_HELPTEXT)
    enHelptext=1;

  PluginDialogBuilder Builder(*this, MainGuid, DialogGuid, LNG_TITLE, g_szTopicConfig, CfgDlgProcStatic, this);

  Builder.AddCheckbox(LNG_SHOWMESS, &m_WaitToContinue);
  m_nShowMessId=Builder.GetLastID();
  Builder.AddCheckbox(LNG_SHOWGUI, &m_UseGUI, 0, true); //3STATE
  Builder.AddCheckbox(LNG_DELETE_USING_FAR, &m_DelUsingFar);
  Builder.AddCheckbox(LNG_CLEAR_SELECTION, &m_ClearSel);
  Builder.AddCheckbox(LNG_SILENT, &m_Silent);
  m_nSilentId=Builder.GetLastID();

  Builder.StartSingleBox(LNG_ADDITIONAL_INFO, true);
  const int AddInfoIDs[] = {LNG_ADDITIONAL_INFO_NONE, LNG_ADDITIONAL_INFO_HELPTEXT, LNG_ADDITIONAL_INFO_VERBS};
  Builder.AddRadioButtons(&enHelptext, 3, AddInfoIDs);
  Builder.AddCheckbox(LNG_ADDITIONAL_DIFFERENT_ONLY, &m_DifferentOnly);
  m_nDifferentId=Builder.GetLastID();
  Builder.EndSingleBox();

  Builder.StartSingleBox(LNG_GUI_POSITION, true);
  const int GuiPosIDs[] = {LNG_GUI_MOUSE_CURSOR, LNG_GUI_WINDOW_CENTER};
  Builder.AddRadioButtons(&m_GuiPos, 2, GuiPosIDs);
  Builder.EndSingleBox();

  Builder.AddOKCancel(LNG_SAVE, LNG_CANCEL);

  if (Builder.ShowDialog())
  {
    if (enHelptext==2)
      m_enHelptext=AS_VERB;
    else if (enHelptext==1)
      m_enHelptext=AS_HELPTEXT;
    else
      m_enHelptext=AS_NONE;
    PluginSettings settings(MainGuid, SettingsControl);
    settings.Set(0, REG_WaitToContinue, m_WaitToContinue);
    settings.Set(0, REG_UseGUI, m_UseGUI);
    settings.Set(0, REG_DelUsingFar, m_DelUsingFar);
    settings.Set(0, REG_ClearSel, m_ClearSel);
    settings.Set(0, REG_Silent, m_Silent);
    settings.Set(0, REG_Helptext, m_enHelptext);
    settings.Set(0, REG_DifferentOnly, m_DifferentOnly);
    settings.Set(0, REG_GuiPos, m_GuiPos);
    return 1;
  }

  return 0;
}

void CPlugin::ExitFAR()
{
  OleThread::Stop();
}

HANDLE CPlugin::OpenPlugin(int nOpenFrom, INT_PTR nItem)
{
  LPCWSTR MsgItems[2]={GetMsg(LNG_TITLE)};
  bool bSuccess=false;
  switch (OleThread::OpenPlugin(nOpenFrom, nItem))
  {
  case DOMNU_ERR_DIFFERENT_FOLDERS:
    {
      MsgItems[1]=GetMsg(LNG_ERR_DIFFERENT_FOLDERS);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError0, MsgItems, ARRAYSIZE(MsgItems), 0);
    }
    break;
  case DOMNU_ERR_SHOW:
    {
      MsgItems[1]=GetMsg(LNG_ERR_SHOW);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError1, MsgItems, ARRAYSIZE(MsgItems), 0);
    }
    break;
  case DOMNU_ERR_INVOKE:
    {
      MsgItems[1]=GetMsg(LNG_ERR_INVOKE);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError2, MsgItems, ARRAYSIZE(MsgItems), 0);
    }
    break;
  case DOMNU_OK:
    if (m_WaitToContinue)
    {
      MsgItems[1]=GetMsg(LNG_CLOSE);
      Message(FMSG_MB_OK, g_szTopicClose, MsgItems, ARRAYSIZE(MsgItems), 0);
    }
    bSuccess=true;
    break;
  case DOMENU_CANCELLED:
    break;
  default:
    assert(0);
  }
  if (bSuccess)
  {
    if (m_ClearSel && !PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL, 0, NULL))
    {
      assert(0);
    }
  }
  return nullptr;
}

CPlugin::EDoMenu CPlugin::OpenPluginBkg(int nOpenFrom, INT_PTR nItem)
{
  LPWSTR szCmdLine=NULL;
  CallMode Mode = CALL_NORMAL;
  switch(nOpenFrom)
  {
  case OPEN_COMMANDLINE:
    {
      LPCWSTR sz=reinterpret_cast<OpenCommandLineInfo*>(nItem)->CommandLine;
      size_t nLen=512;
      do
      {
        delete[] szCmdLine;
        nLen*=2;
        szCmdLine=new wchar_t[nLen];
      }
      while (ExpandEnvironmentStrings(sz, szCmdLine,(DWORD)nLen) >= nLen-1);
    }
    break;

  case OPEN_FILEPANEL:
    Mode = nItem? CALL_APPS : CALL_RIGHTCLICK;
    break;

  case OPEN_LEFTDISKMENU:
  case OPEN_RIGHTDISKMENU:
    {
      struct DiskMenuParam {const wchar_t* CmdLine; BOOL Apps;} *p = reinterpret_cast<DiskMenuParam*>(nItem);
      Mode = p->Apps? CALL_APPS : CALL_RIGHTCLICK;
      szCmdLine=new wchar_t[wcslen(p->CmdLine)+1];
      wcscpy(szCmdLine, p->CmdLine);
    }
    break;

  default:
    Mode = CALL_NORMAL;
    break;
  }
  EDoMenu enDoMenu=DoMenu(szCmdLine, Mode);
  delete[] szCmdLine;
  return enDoMenu;
}

CPlugin::EDoMenu CPlugin::DoMenu(LPWSTR szCmdLine, CallMode Mode)
{
  IShellFolderPtr pDesktop;
  if (FAILED(SHGetDesktopFolder(&pDesktop)))
  	return DOMNU_ERR_SHOW;
  m_pDesktop=pDesktop;

  if (Mode == CALL_RIGHTCLICK || Mode == CALL_APPS)
  {
    int UseGUISav=m_UseGUI;
    int GuiPosSav=m_GuiPos;
    if (m_UseGUI==2)
      m_UseGUI = 0; //если [?] то при right click по дефолту текст меню
    if(Mode == CALL_RIGHTCLICK)
    {
      m_GuiPos=0; //покажем меню там где мышь
    }
    EDoMenu enDoMenu = MenuForPanelOrCmdLine(szCmdLine);
    m_UseGUI=UseGUISav;
    m_GuiPos=GuiPosSav;
    return enDoMenu;
  }

  if (szCmdLine)
  {
    int UseGUISav=m_UseGUI;
    LPWSTR szParams=NULL;
    EAutoItem enAutoItem=AI_NONE;
    if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK L":", ARRAYSIZE(PFX_RCLK)))
    {
      szParams=szCmdLine+ARRAYSIZE(PFX_RCLK);
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_TXT L":", ARRAYSIZE(PFX_RCLK_TXT)))
    {
      szParams=szCmdLine+ARRAYSIZE(PFX_RCLK_TXT);
      m_UseGUI=0;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_GUI L":", ARRAYSIZE(PFX_RCLK_GUI)))
    {
      szParams=szCmdLine+ARRAYSIZE(PFX_RCLK_GUI);
      m_UseGUI=1;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_CMD L":", ARRAYSIZE(PFX_RCLK_CMD)))
    {
      szParams=szCmdLine+ARRAYSIZE(PFX_RCLK_CMD);
      m_UseGUI=0;
      enAutoItem=AI_VERB;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_ITEM L":", ARRAYSIZE(PFX_RCLK_ITEM)))
    {
      szParams=szCmdLine+ARRAYSIZE(PFX_RCLK_ITEM);
      m_UseGUI=0;
      enAutoItem=AI_ITEM;
    }
    if (szParams)
    {
      EDoMenu enDoMenu=MenuForPanelOrCmdLine(szParams, enAutoItem);
      m_UseGUI=UseGUISav;
      return enDoMenu;
    }
  }
  CFarMenu oMainMenu(g_szTopicMain);
  oMainMenu.AddItem(GetMsg(LNG_CONTEXT_MENU));
  oMainMenu.AddItem(GetMsg(LNG_SELECT_DRIVE));
  int nSelItem=0;
  while (1)
  {
    EDoMenu enDoMenu;
    nSelItem=OleThread::ShowMenu(oMainMenu,GetMsg(LNG_TITLE), nSelItem, m_GuiPos==0);
    switch (nSelItem)
    {
    case 0:
      enDoMenu=MenuForPanelOrCmdLine();
      if (DOMENU_BACK!=enDoMenu) return enDoMenu;
      break;
    case 1:
      enDoMenu=SelectDrive();
      if (DOMENU_BACK!=enDoMenu) return enDoMenu;
      break;
    default:
      return DOMENU_CANCELLED;
    }
  }
}

CPlugin::EDoMenu CPlugin::SelectDrive()
{
  CPidl oPidlMyComp;
  if (FAILED(SHGetSpecialFolderLocation(NULL_HWND, CSIDL_DRIVES, &oPidlMyComp)))
  {
    return DOMNU_ERR_SHOW;
  }
  auto_sz szMenuTitle;
  STRRET sr={STRRET_CSTR};
  if (SUCCEEDED(m_pDesktop->GetDisplayNameOf(oPidlMyComp, SHGDN_NORMAL, &sr)))
  {
    szMenuTitle=auto_sz(sr, oPidlMyComp);
  }
  IShellFolderPtr pMyComputer;
  if (FAILED(m_pDesktop->BindToObject(oPidlMyComp, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pMyComputer))))
  {
    return DOMNU_ERR_SHOW;
  }
  IEnumIDListPtr pEnum;
  if (FAILED(pMyComputer->EnumObjects(NULL_HWND, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN, &pEnum)))
  {
    return DOMNU_ERR_SHOW;
  }
  CFarMenu oDrivesMenu(g_szTopicMyComp);
  LPITEMIDLIST piid;
  ULONG nFetched;
  CPidl oPiids;
  unsigned nLastDrivePos=0;
  while (SUCCEEDED(pEnum->Next(1, &piid, &nFetched)) && nFetched)
  {
    sr.uType=STRRET_CSTR;
    if (FAILED(pMyComputer->GetDisplayNameOf(piid, SHGDN_NORMAL, &sr)))
    {
      assert(0);
      continue;
    }
    ULONG nAttr=SFGAO_FILESYSTEM;
    if (FAILED(pMyComputer->GetAttributesOf(1, (LPCITEMIDLIST*)&piid, &nAttr)))
    {
      assert(0);
    }
    auto_sz szMenuItem(sr, piid);
    if (SFGAO_FILESYSTEM==nAttr)
    {
      oDrivesMenu.InsertItem(nLastDrivePos, szMenuItem, false);
      oPiids.Insert(nLastDrivePos, piid);
      ++nLastDrivePos;
    }
    else
    {
      oDrivesMenu.AddItem(szMenuItem, false);
      oPiids.Add(piid);
    }
  }
  int nItem=OleThread::ShowMenu(oDrivesMenu,szMenuTitle, 0, m_GuiPos==0);
  switch (nItem)
  {
  case CFarMenu::SHOW_CANCEL:
    return DOMENU_CANCELLED;
  case CFarMenu::SHOW_BACK:
    return DOMENU_BACK;
  }
  LPCWSTR sz=oDrivesMenu[nItem];
  return DoMenu(pMyComputer, oPiids.GetArray()+nItem, &sz, 0, 1);
}

CPlugin::EDoMenu CPlugin::MenuForPanelOrCmdLine(LPWSTR szCmdLine/*=NULL*/
                      , EAutoItem enAutoItem/*=AI_NONE*/)
{
  EDoMenu enRet=DOMNU_ERR_SHOW;
  LPCWSTR* pParams=NULL;
  LPCWSTR* pFiles=NULL;
  LPCWSTR szCommand=NULL;
  auto_sz strCommand;
  do
  {
    unsigned nFiles=0, nFolders=0;
    auto_sz strCurDir;
    bool bGetFromPanel=true;
    if (szCmdLine && GetFilesFromParams(szCmdLine, &pParams, &nFiles, &nFolders, &strCurDir, enAutoItem!=AI_NONE))
    {
      assert(pParams);
      pFiles=pParams;
      if (enAutoItem==AI_NONE)
      {
        bGetFromPanel=false;
      }
      else
      {
        strCommand=pParams[0];
        szCommand=strCommand;
        pFiles=pParams+1;
        if (nFiles+nFolders>0)
        {
          bGetFromPanel=false;
        }
        else
        {
          delete[] pParams;
          pParams=NULL;
        }
      }
    }
    if (bGetFromPanel)
    {
      if (!GetFilesFromPanel(&pParams, &nFiles, &nFolders, &strCurDir))
      {
        break;
      }
      assert(pParams);
      assert(nFiles+nFolders);
      pFiles=pParams;
    }
    auto_sz strFilesDir;
    bool bMenuAssigned=false;
    bool bDifferentFolders=false;
    unsigned i;
    for (i=0; i<nFiles+nFolders; i++)
    {
      LPCWSTR szFName=m_fsf.PointToName(pFiles[i]);
      if (pFiles[i]==szFName)
      {
        if (!bMenuAssigned)
        {
          strFilesDir=strCurDir;
          bMenuAssigned=true;
        }
        else if (strFilesDir!=strCurDir)
        {
          bDifferentFolders=true;
          break;
        }
      }
      else
      {
        if (*szFName==L'\0')
        {
          // это бывает для дисков (c:, c:\)
          szFName=pFiles[i];
        }
        auto_sz strDir(pFiles[i], szFName-pFiles[i]);
        if (!bMenuAssigned)
        {
          strFilesDir=strDir;
          bMenuAssigned=true;
        }
        else if (strFilesDir!=strDir)
        {
          bDifferentFolders=true;
          break;
        }
        pFiles[i]=szFName;
      }
    }
    if (bDifferentFolders)
    {
      enRet=DOMNU_ERR_DIFFERENT_FOLDERS;
      break;
    }
    CPidl oDirPidl;
    ULONG nCount;
    if (FAILED(m_pDesktop->ParseDisplayName(NULL_HWND, NULL, strFilesDir, &nCount, &oDirPidl, NULL)))
    {
      enRet=DOMNU_ERR_SHOW;
      break;
    }
    IShellFolderPtr pCurFolder;
    if (FAILED(m_pDesktop->BindToObject(oDirPidl, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pCurFolder))))
    {
      enRet=DOMNU_ERR_SHOW;
      break;
    }
    CPidl oPidl;
    for (i=0; i<nFiles+nFolders; i++)
    {
      LPITEMIDLIST pidl;
      auto_sz szFile(pFiles[i]);
      if (szFile.Len()==2 && L':'==szFile[1])
      {
        // диск (c:)
        szFile+=L"\\";
      }
      HRESULT hr=0;
      if (FAILED(hr=pCurFolder->ParseDisplayName(NULL_HWND, NULL, szFile, &nCount, &pidl, NULL)))
      {
        enRet=DOMNU_ERR_SHOW;
        break;
      }
      oPidl.Add(pidl);
    }
    if (oPidl.Count()==nFolders+nFiles)
    {
      enRet=DoMenu(pCurFolder, oPidl.GetArray(), pFiles, nFiles, nFolders, szCommand, enAutoItem);
    }
  } while (0);
  delete[] pParams;
  return enRet;
}

bool CPlugin::GetFilesFromParams(LPWSTR szCmdLine, LPCWSTR** ppFiles, unsigned* pnFiles, unsigned* pnFolders, auto_sz* pstrCurDir, bool bSkipFirst)
{
  int Size=(int)PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,NULL);
  FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[Size];
  dirInfo->StructSize = sizeof(FarPanelDirectory);
  PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,Size,dirInfo);
  *pstrCurDir=auto_sz(dirInfo->Name);
  delete[](char *)dirInfo;
  if (pstrCurDir->Len())
  {
    pstrCurDir->Realloc(pstrCurDir->Len()+2);
    m_fsf.AddEndSlash(*pstrCurDir);
  }
  unsigned nCnt=ParseParams(szCmdLine);
  if (!nCnt) return false;
  *ppFiles=new LPCWSTR[nCnt];
  ParseParams(szCmdLine, *ppFiles);
  for (unsigned i=bSkipFirst?1:0; i<nCnt; i++)
  {
    DWORD dwAttr=GetFileAttributes((*ppFiles)[i]);
    if (DWORD(-1)!=dwAttr && dwAttr&FILE_ATTRIBUTE_DIRECTORY)
      (*pnFolders)++;
    else
      (*pnFiles)++;
  }
  return true;
}

unsigned CPlugin::ParseParams(LPWSTR szParams, LPCWSTR* pFiles/*=NULL*/)
{
  unsigned nCnt=0;
  bool bStartNew=true;
  bool bInsideQuotes=false;
  for (; *szParams; szParams++)
  {
    if (IsSpace(*szParams) && !bInsideQuotes)
    {
      bStartNew=true;
      if (pFiles) *szParams=L'\0';
    }
    else
    {
      if (L'\"'==*szParams)
      {
        bStartNew=true;
        if (pFiles) *szParams=L'\0';
        bInsideQuotes=!bInsideQuotes;
      }
      else if (bStartNew)
      {
        if (pFiles) pFiles[nCnt]=szParams;
        nCnt++;
        bStartNew=false;
      }
    }
  }
  return nCnt;
}

bool CPlugin::GetFilesFromPanel(LPCWSTR** ppFiles, unsigned* pnFiles, unsigned* pnFolders, auto_sz* pstrCurDir)
{
  if(SelectedItems && SelectedItemsCount)
  {
    for(int i=0;i<SelectedItemsCount;i++)
      if(SelectedItems[i])
       delete[](char *)SelectedItems[i];
    delete[] SelectedItems;
    SelectedItems=NULL;
    SelectedItemsCount=0;
  }
  PanelInfo pi = {sizeof(PanelInfo)};
  if (!PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&pi))
  {
    return false;
  }
  int Size=(int)PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,NULL);
  FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[Size];
  dirInfo->StructSize = sizeof(FarPanelDirectory);
  PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,Size,dirInfo);
  // preserve space for AddEndSlash
  *pstrCurDir=auto_sz(dirInfo->Name);
  delete[](char *)dirInfo;

  bool Root=!pi.SelectedItemsNumber;
  if(!Root)
  {
    size_t Size=PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,0,NULL);
    if(Size)
    {
      PluginPanelItem *PPI=(PluginPanelItem*)new char[Size];
       if(PPI)
      {
        FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
        PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,0,&gpi);
        Root=(pi.SelectedItemsNumber==1 && !lstrcmp(PPI->FileName,L".."));
        delete[](char *)PPI;
      }
    }
  }
  if(Root)
  {
    *pnFolders=1;
    LPCWSTR szFName=m_fsf.PointToName(*pstrCurDir);
    if (*szFName==L'\0')
    {
      // это бывает для дисков (c:, c:\)
      szFName=*pstrCurDir;
    }
    static wchar_t szFNameTmp[1024];
    lstrcpy(szFNameTmp,szFName);
    (*ppFiles)=new LPCWSTR[1];
    (*ppFiles)[0]=szFNameTmp;
    pstrCurDir->Trunc(szFName-pstrCurDir->operator LPCWSTR());
  }
  else
  {
    if (pstrCurDir->Len())
    {
      pstrCurDir->Realloc(pstrCurDir->Len()+2);
      m_fsf.AddEndSlash(*pstrCurDir);
    }
    *ppFiles=new LPCWSTR[pi.SelectedItemsNumber];
    SelectedItemsCount=(int)pi.SelectedItemsNumber;
    SelectedItems=new PluginPanelItem*[SelectedItemsCount];
    for (size_t i=0; i<pi.SelectedItemsNumber; i++)
    {
      size_t SelSize = PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,(int)i,NULL);
      SelectedItems[i]=(PluginPanelItem*)new char[SelSize];
      FarGetPluginPanelItem sgpi={sizeof(FarGetPluginPanelItem), SelSize, SelectedItems[i]};
      PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,(int)i,&sgpi);
      LPCWSTR szPath=SelectedItems[i]->FileName;
      size_t Size = PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,(int)pi.CurrentItem,NULL);
      PluginPanelItem *PPI=(PluginPanelItem*)new char[Size];
      FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
      PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM,(int)pi.CurrentItem,&gpi);
      bool Equal=!lstrcmp(PPI->FileName,szPath);
      delete[](char *)PPI;
      if(Equal)
      {
        (*ppFiles)[i]=(*ppFiles)[0];
        (*ppFiles)[0]=szPath;
      }
      else
      {
        (*ppFiles)[i]=szPath;
      }
      if (FILE_ATTRIBUTE_DIRECTORY&SelectedItems[i]->FileAttributes)
      {
        (*pnFolders)++;
      }
      else
      {
        (*pnFiles)++;
      }
    }
  }
  return true;
}// CurDir

CPlugin::EDoMenu CPlugin::DoMenu(LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* pPiids, LPCWSTR pFiles[], unsigned nFiles, unsigned nFolders, LPCWSTR szCommand/*=NULL*/, EAutoItem enAutoItem/*=AI_NONE*/)
{
  assert(nFolders+nFiles);
  auto_sz strMnuTitle;
  if (nFolders+nFiles==1)
  {
    strMnuTitle=pFiles[0];
    if (nFolders)
    {
      strMnuTitle.RemoveTrailing(L'\\');
      strMnuTitle+=L"\\";
    }
  }
  else
  {
    auto_sz strFiles;
    strFiles.Realloc(20);
    wsprintf(strFiles, L"%u %s", nFiles, GetMsg(LNG_FILES));

    auto_sz strFolders;
    strFolders.Realloc(20);
    wsprintf(strFolders, L"%u %s", nFolders, GetMsg(LNG_FOLDERS));

    if (nFiles)
    {
      strMnuTitle+=strFiles;
    }
    if (nFolders)
    {
      if (nFiles) strMnuTitle+=L", ";
      strMnuTitle+=strFolders;
    }
  }
  IContextMenuPtr pCMenu1;

  if (FAILED(pCurFolder->GetUIObjectOf(NULL_HWND, nFolders+nFiles, pPiids, IID_IContextMenu, 0, reinterpret_cast<void**>(&pCMenu1))))
  {
    return DOMNU_ERR_SHOW;
  }
  LPCONTEXTMENU pPreferredMenu=pCMenu1;

  IContextMenu2Ptr pCMenu2(pCMenu1);
  if (pCMenu2)
  {
    pPreferredMenu=pCMenu2;
  }

  IContextMenu3Ptr pCMenu3(pCMenu1);
  if (pCMenu3)
  {
    pPreferredMenu=pCMenu3;
  }

  // Решение дурацкой проблемы с падением в 2000 на paste
  if (m_bWin2K) pCMenu1.Detach();

  CHMenu oHMenu;
  if (!oHMenu) return DOMNU_ERR_SHOW;
  enum {EMENU_CMF_EXTENDEDVERBS=0x00000100}; // rarely used verbs
  if (!pPreferredMenu || FAILED(pPreferredMenu->QueryContextMenu(oHMenu, 0, MENUID_CMDOFFSET, 0x7FFF, CMF_CANRENAME|(GetKeyState(VK_SHIFT)&0xF000?EMENU_CMF_EXTENDEDVERBS:0))))
  {
    return DOMNU_ERR_SHOW;
  }

  int nCmd=0;
  if (szCommand)
  {
    for (int i=0; i<oHMenu.GetMenuItemCount(); i++)
    {
      MENUITEMINFO mii={0};
      mii.cbSize=sizeof(mii);
      mii.fMask=MIIM_ID|MIIM_STATE;
      if (!GetMenuItemInfo(oHMenu, i, TRUE, &mii)) return DOMNU_ERR_INVOKE;
      if (mii.fState&MFS_DISABLED) continue;
      auto_sz strAutoItem;
      switch (enAutoItem)
      {
      case AI_VERB:
        if (!GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, AS_VERB, &strAutoItem))
        {
          continue;
        }
        break;
      case AI_ITEM:
        {
          int nLen=GetMenuString(oHMenu, i, NULL, 0, MF_BYPOSITION);
          if (!nLen) continue;
          ++nLen;
          strAutoItem.Realloc(nLen);
          if (!GetMenuString(oHMenu, i, strAutoItem, nLen, MF_BYPOSITION))
          {
            continue;
          }
        }
        break;
      default:
        assert(0);
        break;
      }
      if (strAutoItem.CompareExcluding(szCommand, L'&'))
      {
        nCmd=mii.wID;
        break;
      }
    }
    if (nCmd<MENUID_CMDOFFSET) return DOMNU_ERR_INVOKE;
  }
  else
  {
    int nSelItem=0;
    CFarMenu oTypeMenu(g_szTopicChooseMenuType, &GuiTextMenuGuid);
    oTypeMenu.AddItem(GetMsg(LNG_MNU_GUI));
    oTypeMenu.AddItem(GetMsg(LNG_MNU_TEXT));
    while (1)
    {
      bool bGUI = false;
      if(2!=m_UseGUI)
      {
        bGUI=(0!=m_UseGUI);
      }
      else
      {
        nSelItem=OleThread::ShowMenu(oTypeMenu,strMnuTitle, nSelItem, m_GuiPos==0);
        switch (nSelItem)
        {
        case CFarMenu::SHOW_CANCEL:
          return DOMENU_CANCELLED;
        case CFarMenu::SHOW_BACK:
          return DOMENU_BACK;
        case 0:
          bGUI=true;
          break;
        case 1:
          bGUI=false;
          break;
        }
      }
      if (bGUI)
      {
        if (!ShowGuiMenu(oHMenu, pCMenu1, pCMenu2, pCMenu3, &nCmd))
        {
          return DOMNU_ERR_SHOW;
        }
      }
      else
      {
        if (!ShowTextMenu(oHMenu, pPreferredMenu, pCMenu2, pCMenu3, &nCmd, strMnuTitle, pCurFolder, pPiids, nFolders+nFiles))
        {
          return DOMNU_ERR_SHOW;
        }
      }
      if (nCmd==MENUID_CANCELLED)
      {
        return DOMENU_CANCELLED;
      }
      if (nCmd!=MENUID_BACK)
      {
        break;
      }
    }
  }

  if (nCmd>=MENUID_CMDOFFSET)
  {
    int nId=nCmd-MENUID_CMDOFFSET;
    CHAR szVerb[100];
    if (FAILED(pPreferredMenu->GetCommandString(nId, GCS_VERBA, NULL, szVerb, ARRAYSIZE(szVerb))))
    {
      szVerb[0]='\0';
    }
    if (lstrcmpA(szVerb, "rename")==0)
    {
      MacroSendMacroText mcmd = {sizeof(MacroSendMacroText), 0, {0}, L"Keys'F6'"};
      MacroControl(NULL, MCTL_SENDSTRING, MSSC_POST, &mcmd);
      return DOMENU_CANCELLED;
    }
    if (m_DelUsingFar && lstrcmpA(szVerb, "delete")==0)
    {
      MacroSendMacroText mcmd = {sizeof(MacroSendMacroText), 0, {0}, L"Keys'F8'"};
      if (GetKeyState(VK_LSHIFT)&0x8000 || GetKeyState(VK_RSHIFT)&0x8000)
      {
        mcmd.SequenceText=L"Keys'ShiftDel'";
      }
      else if (GetKeyState(VK_LMENU)&0x8000 || GetKeyState(VK_RMENU)&0x8000)
      {
        mcmd.SequenceText=L"Keys'AltDel'";
      }
      MacroControl(NULL, MCTL_SENDSTRING, MSSC_POST, &mcmd);
      return DOMENU_CANCELLED;
    }
    else
    {
      CMINVOKECOMMANDINFO cmici;
      cmici.cbSize       = sizeof(cmici);
      cmici.fMask        = 0;
      cmici.hwnd         = NULL_HWND;
      cmici.lpVerb       = (LPCSTR)MAKEINTRESOURCE(nId);
      cmici.lpParameters = NULL;
      cmici.lpDirectory  = NULL;
      cmici.nShow        = SW_SHOWNORMAL;
      cmici.dwHotKey     = 0;
      cmici.hIcon        = NULL;
      if (FAILED(pPreferredMenu->InvokeCommand(&cmici)))
      {
        // return DOMNU_ERR_INVOKE;
        // Иногда здесь возвращается ошибка даже в
        // нормальных ситуациях.
        // Например, если занести это в реестр
//[HKEY_CLASSES_ROOT\AllFileSystemObjects\shellex\ContextMenuHandlers\Copy To]
//@="{C2FBB630-2971-11D1-A18C-00C04FD75D13}"
//[HKEY_CLASSES_ROOT\AllFileSystemObjects\shellex\ContextMenuHandlers\Move To]
//@="{C2FBB631-2971-11D1-A18C-00C04FD75D13}"
        // , а потом вызвать один из добавившихся пунктов
        // и нажать Esc.
      }

      // Эта функция решает проблему, когда
      // после помещения файла в буфер и выхода из FARа
      // буфер портился.
      // проблема точно проявлялась на Build950 и XP
      if (FAILED(OleFlushClipboard()))
      {
        // иногда бывает E_FAIL
        //assert(0);
      }
    }
  }
  return DOMNU_OK;
}

bool CPlugin::ShowGuiMenu(HMENU hMenu, LPCONTEXTMENU pMenu1, LPCONTEXTMENU2 pMenu2, LPCONTEXTMENU3 pMenu3, int* pnCmd)
{
  SMenuDlgParam DlgParam;
  DlgParam.pMenu1=pMenu1;
  DlgParam.pMenu2=pMenu2;
  DlgParam.pMenu3=pMenu3;
  POINT pt={0};
  if (!GetCursorPos(&pt))
  {
    assert(0);
  }
  HWND hFarWnd=(HWND)AdvControl(&MainGuid,ACTL_GETFARHWND, 0, NULL);
  if (m_GuiPos==1)
  {
    RECT rc;
    if (GetWindowRect(hFarWnd, &rc))
    {
      pt.x=(rc.left+rc.right)>>1;
      pt.y=(rc.bottom+rc.top)>>1;
    }
    else
    {
      assert(0);
    }
  }
  // Не устанавливаем родительское окно hFarWnd, т.к.
  // окно мелькает если консольных окон несколько
  HWND hWnd=CreateDialogParam(m_hModule, MAKEINTRESOURCE(IDD_NULL), NULL, (DLGPROC)MenuDlgProc, (LPARAM)&DlgParam);
  assert(hWnd);
  if (!hWnd)
    return false;
  RECT rc;
  if (!GetWindowRect(hWnd, &rc) || !SetWindowPos(hWnd, HWND_TOPMOST, pt.x-(rc.right-rc.left)/2, pt.y-(rc.bottom-rc.top)/2, 0, 0, SWP_NOSIZE))
  {
    assert(0);
  }
  *pnCmd=TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
  if (!DestroyWindow(hWnd))
  {
    assert(0);
  }
  return true;
}

bool CPlugin::GetAdditionalString(IContextMenu* pContextMenu, UINT nID, EAdditionalStr enAdditionalString, auto_sz* pstr)
{
  UINT nType;
  switch (enAdditionalString)
  {
  case AS_HELPTEXT:
    nType=GCS_HELPTEXTW;
    break;
  case AS_VERB:
    nType=GCS_VERBW;
    break;
  default:
    return false;
  }
  WCHAR szwAddInfo[200]=L"\0";
  HRESULT hr;
  SEH_TRY
  {
    hr = pContextMenu->GetCommandString(nID, nType, NULL, reinterpret_cast<LPSTR>(szwAddInfo), ARRAYSIZE(szwAddInfo));
  }
  SEH_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
  {
    hr = E_UNEXPECTED;
  }
  if (FAILED(hr))
  {
    return false;
  }
  *pstr=szwAddInfo;
  if (AS_HELPTEXT==enAdditionalString) pstr->RemoveTrailing(L'.');
  return !pstr->IsEmpty();
}

bool CPlugin::ShowTextMenu(HMENU hMenu, LPCONTEXTMENU pPreferredMenu, LPCONTEXTMENU2 pMenu2, LPCONTEXTMENU3 pMenu3, int* pnCmd, LPCWSTR szTitle, LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* ppiid, unsigned nPiidCnt)
{
  int nItems=GetMenuItemCount(hMenu);
  if (nItems<1) return false;
  CFarMenu oFarMenu(g_szTopicContextMenu, nullptr, nItems);
  for (int i=0; i<nItems; i++)
  {
    auto_sz szItem;
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(mii);
    mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_SUBMENU | MIIM_STATE;
    if (!GetMenuItemInfo(hMenu, i, TRUE, &mii))
    {
      return false;
    }
    if (mii.fType & (MFT_SEPARATOR | MFT_MENUBARBREAK | MFT_MENUBREAK))
    {
      oFarMenu.AddSeparator();
      continue;
    }
    CFarMenu::ECheck enCheck=CFarMenu::UNCHECKED;
    if (mii.fState&MFS_CHECKED)
    {
      if (mii.fType&MFT_RADIOCHECK)
      {
        enCheck=CFarMenu::RADIO;
      }
      else
      {
        enCheck=CFarMenu::CHECKED;
      }
    }
    bool bDisabled=0!=(mii.fState&MFS_DISABLED);
    int grphid = -1;
    if (mii.fType==MFT_STRING)
    {
      int nLen=GetMenuString(hMenu, i, NULL, 0, MF_BYPOSITION);
      if(!nLen) return false;
      ++nLen;
      szItem.Realloc(nLen);
      if (!GetMenuString(hMenu, i, szItem, nLen, MF_BYPOSITION))
      {
        return false;
      }
    }
    else if (mii.fType&MFT_OWNERDRAW)
    {
      grphid = LNG_MT_OWNERDRAWN;
    }
    else
    {
      assert(0);
      continue;
    }
    if (grphid != -1)
    {
      if (m_enHelptext != AS_HELPTEXT /*&& m_enHelptext != AS_VERB*/) {
        auto_sz szSub;
        if (GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, AS_HELPTEXT, &szSub)
            || GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, AS_VERB, &szSub))
        {
          szItem=L"{";

          wchar_t *Buf = new wchar_t[szSub.Len() + 1]();
          //Получаем из shell32.dll шаблон, по которому формируются эти подсказки,
          //(строковый ресурс #5380, "Opens the document with %s."),
          //и убираем его из сабжевых строк. Для XP/2003.
          LoadString(GetModuleHandle(L"shell32.dll"),5380,Buf,int(szSub.Len()+1));
          int i=0;
          while(Buf[i] && Buf[i]!=L'%')
            i++;
          if (Buf[i] == L'%' && !wcsncmp(Buf,szSub,i))
          {
            lstrcpy(Buf,szSub);
            m_fsf.Unquote(Buf+i);
            szSub=Buf+i;
          }
          delete[] Buf;

          //На практике выходит что иногда выходят VERB'ы вида
          //AboutA&bout и т.п., вот тут немного AI чтоб это убрать.
          if (szSub.Len() > 3) //а просто так
          {
            auto_sz szLeft, szRight;

            szLeft = szSub;
            szLeft.Trunc(szLeft.Len()/2);
            szRight = ((LPCWSTR)szSub)+szSub.Len()/2;
            if (szLeft.CompareExcluding(szRight, L'&'))
            {
              //if (szLeft.Len() > szRight.Len())
                //szSub = szLeft;
              //else
                szSub = szRight;
            }
          }

          szItem+=szSub;
          szItem+=L"}";
        }
      }
      if (0==szItem.Len()) szItem = GetMsg(grphid);
    }
    if (m_enHelptext != AS_NONE) {
      auto_sz szAddInfo;
      if (GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, m_enHelptext, &szAddInfo))
      {
        if (!m_DifferentOnly || !szItem.CompareExcluding(szAddInfo, L'&'))
        {
          szItem+=L" (";
          szItem+=szAddInfo;
          szItem+=L")";
        }
      }
    }
    oFarMenu.AddItem(szItem, NULL!=mii.hSubMenu, enCheck, bDisabled);
  }
  for(int nItem=0;;)
  {
    nItem=OleThread::ShowMenu(oFarMenu,szTitle, nItem, m_GuiPos==0);
    switch (nItem)
    {
    case CFarMenu::SHOW_CANCEL:
      *pnCmd=MENUID_CANCELLED;
      return true;
    case CFarMenu::SHOW_BACK:
      *pnCmd=MENUID_BACK;
      return true;
    }
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(mii);
    mii.fMask=MIIM_SUBMENU|MIIM_ID|MIIM_STATE;
    if (!GetMenuItemInfo(hMenu, nItem, TRUE, &mii))
    {
      return false;
    }
    else if (mii.hSubMenu)
    {
      bool bShowMenuRes;
      if (MENUID_SENDTO_WIN98==mii.wID)
      {
        CPidl oSendtoPidl;
        if (FAILED(SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO, &oSendtoPidl)))
        {
          return false;
        }
        IDropTargetPtr pDropTarget;
        bShowMenuRes=ShowFolder(m_pDesktop, oSendtoPidl, pnCmd, szTitle, &pDropTarget);
        if (!bShowMenuRes)
        {
          return false;
        }
        if (*pnCmd!=MENUID_CANCELLED)
        {
          IDataObjectPtr pDataObject;
          if (FAILED(pCurFolder->GetUIObjectOf(NULL_HWND, nPiidCnt, ppiid, IID_IDataObject, NULL, reinterpret_cast<void**>(&pDataObject))))
          {
            return false;
          }
          POINTL pt={0};
          DWORD nEffect=DROPEFFECT_COPY;
          if (FAILED(pDropTarget->DragEnter(pDataObject, MK_LBUTTON, pt, &nEffect)))
          {
            return false;
          }
          if (FAILED(pDropTarget->Drop(pDataObject, MK_LBUTTON, pt, &nEffect)))
          {
            return false;
          }
        }
      }
      else
      {
        // generic submenu selected
        if (pMenu3)
        {
          if (NOERROR!=pMenu3->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM)mii.hSubMenu, nItem))
          {
          }
        }
        else if (pMenu2)
        {
          if (NOERROR!=pMenu2->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM)mii.hSubMenu, nItem))
          {
          }
        }
        bShowMenuRes=ShowTextMenu(mii.hSubMenu, pPreferredMenu, pMenu2, pMenu3, pnCmd, szTitle, pCurFolder, ppiid, nPiidCnt);
      }
      if (!bShowMenuRes)
      {
        return false;
      }
      else if (*pnCmd!=MENUID_CANCELLED && *pnCmd!=MENUID_BACK)
      {
        return true;
      }
    }
    else
    {
      assert(0==(mii.fState&MFS_DISABLED));
      *pnCmd=mii.wID;
      return true;
    }
  }
}

bool CPlugin::ShowFolder(LPSHELLFOLDER pParentFolder, LPCITEMIDLIST piid, int* pnCmd, LPCWSTR szTitle, LPDROPTARGET* ppDropTarget)
{
  IShellFolderPtr pFolder;
  if (FAILED(pParentFolder->BindToObject(piid, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pFolder))))
  {
    return false;
  }
  IEnumIDListPtr pEnum;
  if (FAILED(pFolder->EnumObjects(NULL_HWND, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN, &pEnum)))
  {
    return false;
  }
  CFarMenu oFarMenu;
  CPidl oFolderPiids;
  LPITEMIDLIST piidItem;
  ULONG nFetched;
  while (SUCCEEDED(pEnum->Next(1, &piidItem, &nFetched)) && nFetched)
  {
    STRRET sr;
    sr.uType=STRRET_CSTR;
    if (FAILED(pFolder->GetDisplayNameOf(piidItem, SHGDN_NORMAL, &sr)))
    {
      assert(0);
      continue;
    }
    ULONG nAttr=SFGAO_FOLDER;
    if (FAILED(pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&piidItem, &nAttr)))
    {
      assert(0);
      continue;
    }
    oFolderPiids.Add(piidItem);
    auto_sz szMenuItem(sr, piidItem);
    oFarMenu.AddItem(szMenuItem, 0!=(nAttr&SFGAO_FOLDER));
  }
  for(int nItem=0;;)
  {
    nItem=OleThread::ShowMenu(oFarMenu,szTitle, nItem, m_GuiPos==0);
    switch (nItem)
    {
    case CFarMenu::SHOW_CANCEL:
    case CFarMenu::SHOW_BACK:
      *pnCmd=MENUID_CANCELLED;
      return true;
    }
    ULONG nAttr=SFGAO_FOLDER;
    LPCITEMIDLIST pSelPiid=oFolderPiids.GetAt(nItem);
    if (FAILED(pFolder->GetAttributesOf(1, &pSelPiid, &nAttr)))
    {
      return false;
    }
    if (nAttr&SFGAO_FOLDER)
    {
      if (!ShowFolder(pFolder, pSelPiid, pnCmd, szTitle, ppDropTarget))
      {
        return false;
      }
      if (MENUID_CANCELLED!=*pnCmd) break;
    }
    else
    {
      if (FAILED(pFolder->GetUIObjectOf(NULL_HWND, 1, &pSelPiid, IID_IDropTarget, NULL, (LPVOID*)ppDropTarget)))
      {
        return false;
      }
      *pnCmd=MENUID_SENDTO_DONE;
      break;
    }
  }
  return true;
}
