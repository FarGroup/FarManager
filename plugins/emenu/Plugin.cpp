#include "plugin.h"
#include <comdef.h>
#include <stddef.h>
#include "resource.h"
#include "MenuDlg.h"
#include "FarMenu.h"
#include "Pidl.h"
#include "shlguid.h"
#include "olethread.h"
#include "farkeys.hpp"
#include "HMenu.h"
#include "pluginreg/pluginreg.hpp"
#include "guid.hpp"
#include <cassert>

wchar_t *PluginRootKey;

#define GetCheck(i) (int)PluginStartupInfo::SendDlgMessage(hDlg,DM_GETCHECK,i,0)

// new version of PSDK do not contains standard smart-pointer declaration
_COM_SMARTPTR_TYPEDEF(IContextMenu, __uuidof(IContextMenu));
_COM_SMARTPTR_TYPEDEF(IContextMenu2, __uuidof(IContextMenu2));
_COM_SMARTPTR_TYPEDEF(IContextMenu3, __uuidof(IContextMenu3));

#ifdef _MSC_VER
#pragma warning(disable : 4290)
#endif
void __stdcall _com_issue_error(HRESULT) throw(_com_error)
{
  assert(0);
}
#ifdef _MSC_VER
#pragma warning(default : 4290)
#endif

CPlugin::~CPlugin(void)
{
}

CPlugin::CPlugin(const PluginStartupInfo *Info)
{
  m_hModule=(HINSTANCE)GetModuleHandle(Info->ModuleName);
  m_pMalloc=NULL;
  NULL_HWND=NULL;
  REG_Key=L"\\RightClick";
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

  PluginRootKey = (wchar_t *)malloc(lstrlen(Info->RootKey)*sizeof(wchar_t) + lstrlen(REG_Key)*sizeof(wchar_t));
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,REG_Key);

  ReadRegValues();

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize=sizeof(osvi);
  if (!GetVersionEx(&osvi)) return;
  m_bWin2K=(5==osvi.dwMajorVersion && 0==osvi.dwMinorVersion && VER_PLATFORM_WIN32_NT==osvi.dwPlatformId);
}

void CPlugin::ReadRegValues()
{
  m_WaitToContinue=GetRegKey(L"",REG_WaitToContinue, 0);
  m_UseGUI=GetRegKey(L"",REG_UseGUI, 2);
  m_DelUsingFar=GetRegKey(L"",REG_DelUsingFar, 0);
  m_ClearSel=GetRegKey(L"",REG_ClearSel, 1);
  m_Silent=GetRegKey(L"",REG_Silent, 0);
  m_enHelptext=(EAdditionalStr)GetRegKey(L"",REG_Helptext, 0);
  m_DifferentOnly=GetRegKey(L"",REG_DifferentOnly, 0);
  m_GuiPos=GetRegKey(L"",REG_GuiPos, 1);
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

int CPlugin::Message(DWORD nFlags, LPCWSTR szHelpTopic, const LPCWSTR* pItems, int nItemsNumber, int nButtonsNumber)
{
  if (m_Silent) return -1;
  return PluginStartupInfo::Message(&MainGuid, nFlags, szHelpTopic, pItems, nItemsNumber, nButtonsNumber);
}

INT_PTR WINAPI CPlugin::CfgDlgProcStatic(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2)
{
  static CPlugin* pThis=NULL;
  if (DN_INITDIALOG==Msg)
  {
    pThis=reinterpret_cast<CPlugin*>(Param2);
  }
  if (pThis) pThis->CfgDlgProc(hDlg, Msg, Param1, Param2);
  return thePlug->DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CPlugin::CfgDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  if (DN_INITDIALOG==Msg)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId, 0==SendDlgMessage(hDlg, DM_GETCHECK, m_nSilentId, 0));
    SendDlgMessage(hDlg, DM_ENABLE, m_nDifferentId, 0==SendDlgMessage(hDlg, DM_GETCHECK, m_nDifferentId-3, 0));
  }
  if (DN_BTNCLICK==Msg && m_nSilentId==Param1)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId, 0==Param2);
  }
  if (DN_BTNCLICK==Msg && Param1>=m_nDifferentId-2 && Param1<m_nDifferentId)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nDifferentId, 0!=Param2);
  }
}

int CPlugin::Configure()
{
  const int nWidth=57;
  const int nHeight=20;
  assert(nHeight<25 && nWidth<80);
  FarDialogItem DlgItems[] = {
  /*0*/  {DI_DOUBLEBOX,   3, 1, nWidth-4, nHeight-2,   {0}, NULL, NULL, 0, 0, GetMsg(LNG_TITLE), 0},

  /*1*/  {DI_CHECKBOX,    5, 2, 0, 0,                  {0}, NULL, NULL, DIF_FOCUS, 0, GetMsg(LNG_SHOWMESS), 0},

  /*2*/  {DI_CHECKBOX,    5, 3, 0, 0,                  {0}, NULL, NULL, DIF_3STATE, 0, GetMsg(LNG_SHOWGUI), 0},

  /*3*/  {DI_CHECKBOX,    5, 4, 0, 0,                  {0}, NULL, NULL, 0, 0, GetMsg(LNG_DELETE_USING_FAR), 0},

  /*4*/  {DI_CHECKBOX,    5, 5, 0, 0,                  {0}, NULL, NULL, 0, 0, GetMsg(LNG_CLEAR_SELECTION), 0},

  /*5*/  {DI_CHECKBOX,    5, 6, 0, 0,                  {0}, NULL, NULL, 0, 0, GetMsg(LNG_SILENT), 0},

  /*6*/  {DI_SINGLEBOX,   5, 7, nWidth-6, 12,          {0}, NULL, NULL, DIF_LEFTTEXT, 0, GetMsg(LNG_ADDITIONAL_INFO), 0},
  /*7*/  {DI_RADIOBUTTON, 6, 8, 0, 0,                  {0}, NULL, NULL, DIF_GROUP, 0, GetMsg(LNG_ADDITIONAL_INFO_NONE), 0},
  /*8*/  {DI_RADIOBUTTON, 6, 9, 0, 0,                  {0}, NULL, NULL, 0, 0, GetMsg(LNG_ADDITIONAL_INFO_HELPTEXT), 0},
  /*9*/  {DI_RADIOBUTTON, 6, 10, 0, 0,                 {0}, NULL, NULL, 0, 0, GetMsg(LNG_ADDITIONAL_INFO_VERBS), 0},
  /*10*/ {DI_CHECKBOX,    6, 11, 0, 0,                 {0}, NULL, NULL, 0, 0, GetMsg(LNG_ADDITIONAL_DIFFERENT_ONLY), 0},

  /*11*/ {DI_SINGLEBOX,   5, 13, nWidth-6, 16,         {0}, NULL, NULL, DIF_LEFTTEXT, 0, GetMsg(LNG_GUI_POSITION), 0},
  /*12*/ {DI_RADIOBUTTON, 6, 14, 0, 0,                 {0}, NULL, NULL, DIF_GROUP, 0, GetMsg(LNG_GUI_MOUSE_CURSOR), 0},
  /*13*/ {DI_RADIOBUTTON, 6, 15, 0, 0,                 {0}, NULL, NULL, 0, 0, GetMsg(LNG_GUI_WINDOW_CENTER), 0},

  /*14*/ {DI_BUTTON,      0, nHeight-3, 0, 0,          {0}, NULL, NULL, DIF_CENTERGROUP|DIF_DEFAULTBUTTON, 0, GetMsg(LNG_SAVE), 0},
  /*15*/ {DI_BUTTON,      0, nHeight-3, 0, 0,          {0}, NULL, NULL, DIF_CENTERGROUP, 0, GetMsg(LNG_CANCEL), 0}
  };
  m_nShowMessId=1;
  m_nSilentId=5;
  m_nDifferentId=10;
  DlgItems[1].Selected=m_WaitToContinue;
  DlgItems[2].Selected=m_UseGUI;
  DlgItems[3].Selected=m_DelUsingFar;
  DlgItems[4].Selected=m_ClearSel;
  DlgItems[5].Selected=m_Silent;
  DlgItems[7].Selected=m_enHelptext==AS_NONE;
  DlgItems[8].Selected=m_enHelptext==AS_HELPTEXT;
  DlgItems[9].Selected=m_enHelptext==AS_VERB;
  DlgItems[10].Selected=m_DifferentOnly;
  DlgItems[12].Selected=m_GuiPos==0;
  DlgItems[13].Selected=m_GuiPos==1;

  int ret = 0;
  HANDLE hDlg = DialogInit(&MainGuid, &DialogGuid, -1, -1, nWidth, nHeight, g_szTopicConfig, DlgItems, ARRAYSIZE(DlgItems), 0, 0, CfgDlgProcStatic, reinterpret_cast<INT_PTR>(this));

  if (hDlg == INVALID_HANDLE_VALUE)
    return ret;

  int ExitCode = DialogRun(hDlg);

  if (ExitCode==14)
  {
    m_WaitToContinue=GetCheck(1);
    m_UseGUI=GetCheck(2);
    m_DelUsingFar=GetCheck(3);
    m_ClearSel=GetCheck(4);
    m_Silent=GetCheck(5);
    if (GetCheck(7)) m_enHelptext=AS_NONE;
    if (GetCheck(8)) m_enHelptext=AS_HELPTEXT;
    if (GetCheck(9)) m_enHelptext=AS_VERB;
    m_DifferentOnly=GetCheck(10);
    if (GetCheck(12)) m_GuiPos=0;
    if (GetCheck(13)) m_GuiPos=1;
    SetRegKey(L"", REG_WaitToContinue, m_WaitToContinue);
    SetRegKey(L"", REG_UseGUI, m_UseGUI);
    SetRegKey(L"", REG_DelUsingFar, m_DelUsingFar);
    SetRegKey(L"", REG_ClearSel, m_ClearSel);
    SetRegKey(L"", REG_Silent, m_Silent);
    SetRegKey(L"", REG_Helptext, m_enHelptext);
    SetRegKey(L"", REG_DifferentOnly, m_DifferentOnly);
    SetRegKey(L"", REG_GuiPos, m_GuiPos);
    ret=1;
  }
  DialogFree(hDlg);
  return ret;
}

void CPlugin::ExitFAR()
{
  OleThread::Stop();
  free(PluginRootKey);
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
    if (m_ClearSel && !Control(FCTL_UPDATEPANEL, 0, NULL))
    {
      assert(0);
    }
  }
  return INVALID_HANDLE_VALUE;
}

CPlugin::EDoMenu CPlugin::OpenPluginBkg(int nOpenFrom, INT_PTR nItem)
{
  LPWSTR szCmdLine=NULL;
  if (OPEN_COMMANDLINE==nOpenFrom)
  {
    LPCWSTR sz=reinterpret_cast<LPCWSTR>(nItem);
    size_t nLen=512;
    do
    {
      delete[] szCmdLine;
      nLen*=2;
      szCmdLine=new wchar_t[nLen];
    } while (ExpandEnvironmentStrings(sz, szCmdLine,(DWORD)nLen) >= nLen-1);
  }
  EDoMenu enDoMenu=DoMenu(szCmdLine);
  delete[] szCmdLine;
  return enDoMenu;
}

CPlugin::EDoMenu CPlugin::DoMenu(LPWSTR szCmdLine)
{
  IShellFolderPtr pDesktop;
  if (FAILED(SHGetDesktopFolder(&pDesktop))) return DOMNU_ERR_SHOW;
  m_pDesktop=pDesktop;
  if (!m_pMalloc) return DOMNU_ERR_SHOW;
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
    nSelItem=oMainMenu.Show(GetMsg(LNG_TITLE), nSelItem, m_GuiPos==0);
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
  CPidl oPidlMyComp(m_pMalloc);
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
  CPidl oPiids(m_pMalloc);
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
  int nItem=oDrivesMenu.Show(szMenuTitle, 0, m_GuiPos==0);
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
    CPidl oDirPidl(m_pMalloc);
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
    CPidl oPidl(m_pMalloc);
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

bool CPlugin::GetFilesFromParams(LPWSTR szCmdLine
                 , LPCWSTR** ppFiles
                 , unsigned* pnFiles
                 , unsigned* pnFolders
                 , auto_sz* pstrCurDir
                 , bool bSkipFirst)
{
  int Size=Control(FCTL_GETPANELDIR,0,NULL);
  wchar_t *CurDir=new wchar_t[Size];
  Control(FCTL_GETPANELDIR,Size,(LONG_PTR)CurDir);
  *pstrCurDir=auto_sz(CurDir,Size);
  delete[] CurDir;
  if (pstrCurDir->Len()) m_fsf.AddEndSlash(*pstrCurDir);
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
       delete[] SelectedItems[i];
    delete[] SelectedItems;
    SelectedItems=NULL;
    SelectedItemsCount=0;
  }
  PanelInfo pi;
  if (!Control(FCTL_GETPANELINFO,0,(LONG_PTR)&pi))
  {
    return false;
  }
  int Size=Control(FCTL_GETPANELDIR,0,NULL);
  wchar_t *CurDir=new wchar_t[Size];
  Control(FCTL_GETPANELDIR,Size,(LONG_PTR)CurDir);
  // preserve space for AddEndSlash
  *pstrCurDir=auto_sz(CurDir,Size);
  delete[] CurDir;

  bool Root=!pi.SelectedItemsNumber;
  if(!Root)
  {
    size_t Size=Control(FCTL_GETSELECTEDPANELITEM,0,NULL);
    if(Size)
    {
      PluginPanelItem *PPI=(PluginPanelItem*)new char[Size];
       if(PPI)
      {
        Control(FCTL_GETSELECTEDPANELITEM,0,(LONG_PTR)PPI);
        Root=(pi.SelectedItemsNumber==1 && !lstrcmp(PPI->FileName,L".."));
        delete[] PPI;
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
    if (pstrCurDir->Len()) m_fsf.AddEndSlash(*pstrCurDir);
    *ppFiles=new LPCWSTR[pi.SelectedItemsNumber];
    SelectedItemsCount=pi.SelectedItemsNumber;
    SelectedItems=new PluginPanelItem*[SelectedItemsCount];
    for (int i=0; i<pi.SelectedItemsNumber; i++)
    {
      SelectedItems[i]=(PluginPanelItem*)new char[Control(FCTL_GETSELECTEDPANELITEM,i,NULL)];
      Control(FCTL_GETSELECTEDPANELITEM,i,(LONG_PTR)SelectedItems[i]);
      LPCWSTR szPath=SelectedItems[i]->FileName;
      PluginPanelItem *PPI=(PluginPanelItem*)new char[Control(FCTL_GETPANELITEM,pi.CurrentItem,NULL)];
      Control(FCTL_GETPANELITEM,pi.CurrentItem,(LONG_PTR)PPI);
      bool Equal=!lstrcmp(PPI->FileName,szPath);
      delete[] PPI;
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

CPlugin::EDoMenu CPlugin::DoMenu(LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* pPiids
                 , LPCWSTR pFiles[], unsigned nFiles
                 , unsigned nFolders, LPCWSTR szCommand/*=NULL*/
                 , EAutoItem enAutoItem/*=AI_NONE*/)
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
    wsprintf(strFiles, L"%d %s", nFiles, GetMsg(LNG_FILES));

    auto_sz strFolders;
    strFolders.Realloc(20);
    wsprintf(strFolders, L"%d %s", nFolders, GetMsg(LNG_FOLDERS));

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
    CFarMenu oTypeMenu(g_szTopicChooseMenuType);
    oTypeMenu.AddItem(GetMsg(LNG_MNU_GUI));
    oTypeMenu.AddItem(GetMsg(LNG_MNU_TEXT));
    while (1)
    {
      bool bGUI;
      if(2!=m_UseGUI)
      {
        bGUI=(0!=m_UseGUI);
      }
      else
      {
        nSelItem=oTypeMenu.Show(strMnuTitle, nSelItem, m_GuiPos==0);
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
      DWORD dwKey=KEY_F6;
      KeySequence Key={0, 1, &dwKey};
      AdvControl(ACTL_POSTKEYSEQUENCE, &Key);
      return DOMENU_CANCELLED;
    }
    if (m_DelUsingFar && lstrcmpA(szVerb, "delete")==0)
    {
      DWORD dwKey=KEY_F8;
      if (GetKeyState(VK_LSHIFT)&0x8000 || GetKeyState(VK_RSHIFT)&0x8000)
      {
        dwKey=KEY_SHIFTDEL;
      }
      else if (GetKeyState(VK_LMENU)&0x8000 || GetKeyState(VK_RMENU)&0x8000)
      {
        dwKey=KEY_ALTDEL;
      }
      KeySequence Key={0, 1, &dwKey};
      AdvControl(ACTL_POSTKEYSEQUENCE, &Key);
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

bool CPlugin::ShowGuiMenu(HMENU hMenu
              , LPCONTEXTMENU pMenu1
              , LPCONTEXTMENU2 pMenu2
              , LPCONTEXTMENU3 pMenu3
              , int* pnCmd)
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
  HWND hFarWnd=(HWND)AdvControl(ACTL_GETFARHWND, 0);
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
  if (FAILED(pContextMenu->GetCommandString(nID, nType, NULL, reinterpret_cast<LPSTR>(szwAddInfo), ARRAYSIZE(szwAddInfo))))
  {
    return false;
  }
  *pstr=szwAddInfo;
  if (AS_HELPTEXT==enAdditionalString) pstr->RemoveTrailing(L'.');
  return !pstr->IsEmpty();
}

bool CPlugin::ShowTextMenu(HMENU hMenu
              , LPCONTEXTMENU pPreferredMenu
              , LPCONTEXTMENU2 pMenu2
              , LPCONTEXTMENU3 pMenu3
              , int* pnCmd
              , LPCWSTR szTitle
              , LPSHELLFOLDER pCurFolder
              , LPCITEMIDLIST* ppiid
              , unsigned nPiidCnt)
{
  int nItems=GetMenuItemCount(hMenu);
  if (nItems<1) return false;
  CFarMenu oFarMenu(g_szTopicContextMenu, nItems);
  for (int i=0; i<nItems; i++)
  {
    auto_sz szItem;
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(mii);
    mii.fMask=MIIM_TYPE|MIIM_ID|MIIM_SUBMENU|MIIM_STATE;
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
    else if (mii.fType&MFT_BITMAP)
    {
      grphid = LNG_MT_BITMAP;
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

          wchar_t *Buf=(wchar_t*)malloc((szSub.Len()+1)*sizeof(wchar_t));
          if (Buf)
          {
            //Получаем из shell32.dll шаблон, по которому формируются эти подсказки,
            //(строковый ресурс #5380, "Opens the document with %s."),
            //и убираем его из сабжевых строк. Для XP/2003.
            *Buf=0;
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
            free(Buf);
          }

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
          szItem+=L"");
        }
      }
    }
    oFarMenu.AddItem(szItem, NULL!=mii.hSubMenu, enCheck, bDisabled);
  }
  for(int nItem=0;;)
  {
    nItem=oFarMenu.Show(szTitle, nItem, m_GuiPos==0);
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
        if (!m_pMalloc) return false;
        CPidl oSendtoPidl(m_pMalloc);
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
  if (!m_pMalloc) return false;
  CPidl oFolderPiids(m_pMalloc);
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
    nItem=oFarMenu.Show(szTitle, nItem, m_GuiPos==0);
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
