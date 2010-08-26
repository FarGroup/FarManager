#include "plugin.h"
#include <comdef.h>
#include <stddef.h>
#include "resource.h"
#include "Reg.cpp"
#include "MenuDlg.h"
#include "FarMenu.h"
#include "Pidl.h"
#include "shlguid.h"
#include "olethread.h"
#include "farkeys.hpp"
#include "HMenu.h"
#include <cassert>

#ifndef UNICODE
#define GetCheck(i) DlgItems[i].Selected
#else
#define GetCheck(i) (int)PluginStartupInfo::SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#endif

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
  REG_Key=_T("\\RightClick");
  REG_WaitToContinue=_T("WaitToContinue");
  REG_UseGUI=_T("UseGUI");
  REG_DelUsingFar=_T("DelUsingFar");
  REG_ClearSel=_T("ClearSelection");
  REG_Silent=_T("Silent");
  REG_Helptext=_T("Helptext");
  REG_DifferentOnly=_T("DifferentOnly");
  REG_GuiPos=_T("GuiPos");
#ifndef UNICODE
  auto_sz::SetOem();
#else
  SelectedItems=NULL;
  SelectedItemsCount=0;
#endif

  *(PluginStartupInfo*)this=*Info;
  m_fsf=*Info->FSF;

  lstrcpy(g_PluginKey, Info->RootKey);
  lstrcat(g_PluginKey, REG_Key);
  ReadRegValues();

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize=sizeof(osvi);
  if (!GetVersionEx(&osvi)) return;
  if (osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
  {
    m_bWin9x=true;
    m_bWin95=(osvi.dwMajorVersion==4 && !osvi.dwMinorVersion);
    m_bWin98=(osvi.dwMajorVersion==4 && osvi.dwMinorVersion<0x5a);
    m_bWinME=((osvi.dwMajorVersion>4)
      || (osvi.dwMajorVersion==4 && osvi.dwMinorVersion>=0x5a));
  }
  m_bWinNT=(4==osvi.dwMajorVersion
    && 0==osvi.dwMinorVersion
    && VER_PLATFORM_WIN32_NT==osvi.dwPlatformId);
  m_bWin2K=(5==osvi.dwMajorVersion
    && 0==osvi.dwMinorVersion
    && VER_PLATFORM_WIN32_NT==osvi.dwPlatformId);
}

void CPlugin::ReadRegValues()
{
  m_WaitToContinue=GetRegKey(REG_WaitToContinue, 0);
  m_UseGUI=GetRegKey(REG_UseGUI, 2);
  m_DelUsingFar=GetRegKey(REG_DelUsingFar, 0);
  m_ClearSel=GetRegKey(REG_ClearSel, 1);
  m_Silent=GetRegKey(REG_Silent, 0);
  m_enHelptext=(EAdditionalStr)GetRegKey(REG_Helptext, 0);
  m_DifferentOnly=GetRegKey(REG_DifferentOnly, 0);
  m_GuiPos=GetRegKey(REG_GuiPos, 1);
}

void CPlugin::GetPluginInfo(PluginInfo *Info)
{
  m_PluginMenuString=GetMsg(LNG_TITLE);
  m_PluginConfigString=GetMsg(LNG_TITLE);
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_FULLCMDLINE;
  Info->DiskMenuStringsNumber=0;
  Info->PluginMenuStrings=&m_PluginMenuString;
  Info->PluginMenuStringsNumber=1;
  Info->PluginConfigStrings=&m_PluginConfigString;
  Info->PluginConfigStringsNumber=1;
  Info->CommandPrefix=PFX_RCLK _T(":") PFX_RCLK_TXT _T(":")
    PFX_RCLK_GUI _T(":") PFX_RCLK_CMD _T(":") PFX_RCLK_ITEM;
}

LPCTSTR g_szTopicMain=_T("Main");
LPCTSTR g_szTopicConfig=_T("Config");
LPCTSTR g_szTopicChooseMenuType=_T("ChooseMenuType");
LPCTSTR g_szTopicContextMenu=_T("ContextMenu");
LPCTSTR g_szTopicMyComp=_T("MyComp");
LPCTSTR g_szTopicError0=_T("Error0");
LPCTSTR g_szTopicError1=_T("Error1");
LPCTSTR g_szTopicError2=_T("Error2");
LPCTSTR g_szTopicClose=_T("Close");

LPCTSTR CPlugin::GetMsg(int nMsgId)
{
  return PluginStartupInfo::GetMsg(ModuleNumber, nMsgId);
}

int CPlugin::Menu(int nX, int nY, int nMaxHeight, DWORD nFlags
          , LPCTSTR szTitle, LPCTSTR szBottom, LPCTSTR szHelpTopic
          , const int* pnBreakKeys, int* pnBreakCode
          , const FarMenuItem* pItems, int nItemsNumber)
{
  return PluginStartupInfo::Menu(ModuleNumber, nX, nY, nMaxHeight, nFlags
    , szTitle, szBottom, szHelpTopic, pnBreakKeys, pnBreakCode
    , pItems, nItemsNumber);
}

#ifndef UNICODE
int CPlugin::DialogEx(int X1, int Y1, int X2, int Y2, LPCTSTR szHelpTopic
    , FarDialogItem* pItem, int nItemsNumber, DWORD nReserved, DWORD nFlags
    , FARWINDOWPROC DlgProc, LONG_PTR pParam)
{
  return PluginStartupInfo::DialogEx(ModuleNumber, X1, Y1, X2, Y2, szHelpTopic
    , pItem, nItemsNumber, nReserved, nFlags, DlgProc, pParam);
}
#else
HANDLE CPlugin::DialogInit(int X1, int Y1, int X2, int Y2, LPCTSTR szHelpTopic
    , FarDialogItem* pItem, int nItemsNumber, DWORD nReserved, DWORD nFlags
    , FARWINDOWPROC DlgProc, LONG_PTR pParam)
{
  return PluginStartupInfo::DialogInit(ModuleNumber, X1, Y1, X2, Y2, szHelpTopic
    , pItem, nItemsNumber, nReserved, nFlags, DlgProc, pParam);
}

int CPlugin::DialogRun(HANDLE hDlg)
{
  return PluginStartupInfo::DialogRun(hDlg);
}

void CPlugin::DialogFree(HANDLE hDlg)
{
  PluginStartupInfo::DialogFree(hDlg);
}
#endif

int CPlugin::Message(DWORD nFlags, LPCTSTR szHelpTopic, const LPCTSTR* pItems
    , int nItemsNumber, int nButtonsNumber)
{
  if (m_Silent) return -1;
  return PluginStartupInfo::Message(ModuleNumber, nFlags, szHelpTopic, pItems
    , nItemsNumber, nButtonsNumber);
}

#ifndef UNICODE
int CPlugin::Control(int nCommand, void* pParam)
#else
int CPlugin::Control(int nCommand, int Param1,INT_PTR Param2)
#endif
{
#ifndef UNICODE
  return PluginStartupInfo::Control(INVALID_HANDLE_VALUE, nCommand, pParam);
#else
  return PluginStartupInfo::Control(INVALID_HANDLE_VALUE,nCommand,Param1,Param2);
#endif
}

INT_PTR CPlugin::AdvControl(int nCommand, void *pParam)
{
  return PluginStartupInfo::AdvControl(ModuleNumber, nCommand, pParam);
}

LONG_PTR WINAPI CPlugin::CfgDlgProcStatic(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
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
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId
      , 0==SendDlgMessage(hDlg, DM_GETCHECK, m_nSilentId, 0));
    SendDlgMessage(hDlg, DM_ENABLE, m_nDifferentId
      , 0==SendDlgMessage(hDlg, DM_GETCHECK, m_nDifferentId-3, 0));
  }
  if (DN_BTNCLICK==Msg && m_nSilentId==Param1)
  {
    SendDlgMessage(hDlg, DM_ENABLE, m_nShowMessId, 0==Param2);
  }
  if (DN_BTNCLICK==Msg && Param1>=m_nDifferentId-2
    && Param1<m_nDifferentId)
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
  /*0*/ {DI_DOUBLEBOX, 3, 1, nWidth-4, nHeight-2, 0, 0, 0, 0, {0}},

  /*1*/ {DI_CHECKBOX, 5, 2, 0, 0, TRUE, 0, 0, 0, {0}},

  /*2*/ {DI_CHECKBOX, 5, 3, 0, 0, 0, 0, DIF_3STATE, 0, {0}},

  /*3*/ {DI_CHECKBOX, 5, 4, 0, 0, 0, 0, 0, 0, {0}},

  /*4*/ {DI_CHECKBOX, 5, 5, 0, 0, 0, 0, 0, 0, {0}},

  /*5*/ {DI_CHECKBOX, 5, 6, 0, 0, 0, 0, 0, 0, {0}},

  /*6*/ {DI_SINGLEBOX, 5, 7, nWidth-6, 12, 0, 0, DIF_LEFTTEXT, 0, {0}},
  /*7*/ {DI_RADIOBUTTON, 6, 8, 0, 0, 0, 1, DIF_GROUP, 0, {0}},
  /*8*/ {DI_RADIOBUTTON, 6, 9, 0, 0, 0, 0, 0, 0, {0}},
  /*9*/ {DI_RADIOBUTTON, 6, 10, 0, 0, 0, 0, 0, 0, {0}},
  /*10*/  {DI_CHECKBOX, 6, 11, 0, 0, 0, 0, 0, 0, {0}},

  /*11*/  {DI_SINGLEBOX, 5, 13, nWidth-6, 16, 0, 0, DIF_LEFTTEXT, 0, {0}},
  /*12*/  {DI_RADIOBUTTON, 6, 14, 0, 0, 0, 1, DIF_GROUP, 0, {0}},
  /*13*/  {DI_RADIOBUTTON, 6, 15, 0, 0, 0, 0, 0, 0, {0}},

//  /*  */  {DI_TEXT, 0, nHeight-4, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, 0, {0}},

  /*14*/  {DI_BUTTON, 0, nHeight-3, 0, 0, 0, 0, DIF_CENTERGROUP, 1, {0}},
  /*15*/  {DI_BUTTON, 0, nHeight-3, 0, 0, 0, 0, DIF_CENTERGROUP, 0, {0}}
  };
  m_nShowMessId=1;
  m_nSilentId=5;
  m_nDifferentId=10;
#ifndef UNICODE
#define SET_DLGITEM(n,v)  lstrcpy(DlgItems[n].Data, v)
#else
#define SET_DLGITEM(n,v)  DlgItems[n].PtrData = v
#endif
  SET_DLGITEM(0, GetMsg(LNG_TITLE));
  SET_DLGITEM(m_nShowMessId, GetMsg(LNG_SHOWMESS));
  SET_DLGITEM(2, GetMsg(LNG_SHOWGUI));
  SET_DLGITEM(3, GetMsg(LNG_DELETE_USING_FAR));
  SET_DLGITEM(4, GetMsg(LNG_CLEAR_SELECTION));
  SET_DLGITEM(m_nSilentId, GetMsg(LNG_SILENT));
  SET_DLGITEM(6, GetMsg(LNG_ADDITIONAL_INFO));
  SET_DLGITEM(7, GetMsg(LNG_ADDITIONAL_INFO_NONE));
  SET_DLGITEM(8, GetMsg(LNG_ADDITIONAL_INFO_HELPTEXT));
  SET_DLGITEM(9, GetMsg(LNG_ADDITIONAL_INFO_VERBS));
  SET_DLGITEM(10, GetMsg(LNG_ADDITIONAL_DIFFERENT_ONLY));
  SET_DLGITEM(11, GetMsg(LNG_GUI_POSITION));
  SET_DLGITEM(12, GetMsg(LNG_GUI_MOUSE_CURSOR));
  SET_DLGITEM(13, GetMsg(LNG_GUI_WINDOW_CENTER));
  SET_DLGITEM(14, GetMsg(LNG_SAVE));
  SET_DLGITEM(15, GetMsg(LNG_CANCEL));
#undef SET_DLGITEM
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
#ifndef UNICODE
  int ExitCode = DialogEx(-1, -1, nWidth, nHeight, g_szTopicConfig, DlgItems,
                   ArraySize(DlgItems), 0, 0, CfgDlgProcStatic, reinterpret_cast<LONG_PTR>(this));
#else
  HANDLE hDlg = DialogInit(-1, -1, nWidth, nHeight, g_szTopicConfig, DlgItems,
                   ArraySize(DlgItems), 0, 0, CfgDlgProcStatic, reinterpret_cast<LONG_PTR>(this));

  if (hDlg == INVALID_HANDLE_VALUE)
    return ret;

  int ExitCode = DialogRun(hDlg);
#endif

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
    SetRegKey(REG_WaitToContinue, m_WaitToContinue);
    SetRegKey(REG_UseGUI, m_UseGUI);
    SetRegKey(REG_DelUsingFar, m_DelUsingFar);
    SetRegKey(REG_ClearSel, m_ClearSel);
    SetRegKey(REG_Silent, m_Silent);
    SetRegKey(REG_Helptext, m_enHelptext);
    SetRegKey(REG_DifferentOnly, m_DifferentOnly);
    SetRegKey(REG_GuiPos, m_GuiPos);
    ret=1;
  }
#ifdef UNICODE
  DialogFree(hDlg);
#endif
  return ret;
}

void CPlugin::ExitFAR()
{
  OleThread::Stop();
}

HANDLE CPlugin::OpenPlugin(int nOpenFrom, INT_PTR nItem)
{
  LPCTSTR MsgItems[2]={GetMsg(LNG_TITLE)};
  bool bSuccess=false;
  SetFileApisToANSI();
  switch (OleThread::OpenPlugin(nOpenFrom, (int)nItem))
  {
  case DOMNU_ERR_DIFFERENT_FOLDERS:
    {
      MsgItems[1]=GetMsg(LNG_ERR_DIFFERENT_FOLDERS);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError0, MsgItems
        , ArraySize(MsgItems), 0);
    }
    break;
  case DOMNU_ERR_SHOW:
    {
      MsgItems[1]=GetMsg(LNG_ERR_SHOW);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError1, MsgItems
        , ArraySize(MsgItems), 0);
    }
    break;
  case DOMNU_ERR_INVOKE:
    {
      MsgItems[1]=GetMsg(LNG_ERR_INVOKE);
      Message(FMSG_WARNING|FMSG_MB_OK, g_szTopicError2, MsgItems
        , ArraySize(MsgItems), 0);
    }
    break;
  case DOMNU_OK:
    if (m_WaitToContinue)
    {
      MsgItems[1]=GetMsg(LNG_CLOSE);
      Message(FMSG_MB_OK, g_szTopicClose, MsgItems, ArraySize(MsgItems), 0);
    }
    bSuccess=true;
    break;
  case DOMENU_CANCELLED:
    break;
  default:
    assert(0);
  }
  SetFileApisToOEM();
  if (bSuccess)
  {
    if (m_ClearSel && !Control(FCTL_UPDATEPANEL,
#ifdef UNICODE
                                                0,
#endif
                                                  NULL))
    {
      assert(0);
    }
  }
  return INVALID_HANDLE_VALUE;
}

CPlugin::EDoMenu CPlugin::OpenPluginBkg(int nOpenFrom, INT_PTR nItem)
{
  LPTSTR szCmdLine=NULL;
  if (OPEN_COMMANDLINE==nOpenFrom)
  {
    LPCTSTR sz=reinterpret_cast<LPCTSTR>(nItem);
    size_t nLen=512;
    do
    {
      delete[] szCmdLine;
      nLen*=2;
      szCmdLine=new TCHAR[nLen];
    } while (
#ifndef UNICODE
             m_fsf.ExpandEnvironmentStr   // in OEM
#else
             ExpandEnvironmentStrings
#endif
             (sz, szCmdLine,
#ifdef UNICODE
                             (DWORD)
#endif
                                     nLen) >= nLen-1);
  }
  EDoMenu enDoMenu=DoMenu(szCmdLine);
  delete[] szCmdLine;
  return enDoMenu;
}

CPlugin::EDoMenu CPlugin::DoMenu(LPTSTR szCmdLine)
{
  IShellFolderPtr pDesktop;
  if (FAILED(SHGetDesktopFolder(&pDesktop))) return DOMNU_ERR_SHOW;
  m_pDesktop=pDesktop;
  if (!m_pMalloc) return DOMNU_ERR_SHOW;
  if (szCmdLine)
  {
    int UseGUISav=m_UseGUI;
    LPTSTR szParams=NULL;
    EAutoItem enAutoItem=AI_NONE;
    if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK _T(":"), ArraySize(PFX_RCLK)))
    {
      szParams=szCmdLine+ArraySize(PFX_RCLK);
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_TXT _T(":"), ArraySize(PFX_RCLK_TXT)))
    {
      szParams=szCmdLine+ArraySize(PFX_RCLK_TXT);
      m_UseGUI=0;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_GUI _T(":"), ArraySize(PFX_RCLK_GUI)))
    {
      szParams=szCmdLine+ArraySize(PFX_RCLK_GUI);
      m_UseGUI=1;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_CMD _T(":"), ArraySize(PFX_RCLK_CMD)))
    {
      szParams=szCmdLine+ArraySize(PFX_RCLK_CMD);
      m_UseGUI=0;
      enAutoItem=AI_VERB;
    }
    else if (0==m_fsf.LStrnicmp(szCmdLine, PFX_RCLK_ITEM _T(":"), ArraySize(PFX_RCLK_ITEM)))
    {
      szParams=szCmdLine+ArraySize(PFX_RCLK_ITEM);
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
  if (FAILED(m_pDesktop->BindToObject(oPidlMyComp, NULL, IID_IShellFolder
    , reinterpret_cast<void**>(&pMyComputer))))
  {
    return DOMNU_ERR_SHOW;
  }
  IEnumIDListPtr pEnum;
  if (FAILED(pMyComputer->EnumObjects(NULL_HWND, SHCONTF_FOLDERS
    |SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN, &pEnum)))
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
  LPCTSTR sz=oDrivesMenu[nItem];
  return DoMenu(pMyComputer, oPiids.GetArray()+nItem, &sz, 0, 1);
}

CPlugin::EDoMenu CPlugin::MenuForPanelOrCmdLine(LPTSTR szCmdLine/*=NULL*/
                      , EAutoItem enAutoItem/*=AI_NONE*/)
{
  EDoMenu enRet=DOMNU_ERR_SHOW;
  LPCTSTR* pParams=NULL;
  LPCTSTR* pFiles=NULL;
  LPCTSTR szCommand=NULL;
  auto_sz strCommand;
  do
  {
    unsigned nFiles=0, nFolders=0;
    auto_sz strCurDir;
    bool bGetFromPanel=true;
    if (szCmdLine && GetFilesFromParams(szCmdLine, &pParams
      , &nFiles, &nFolders, &strCurDir, enAutoItem!=AI_NONE))
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
#ifndef UNICODE
        strCommand.Oem2Ansi();
#endif
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
      LPCTSTR szFName=m_fsf.PointToName(pFiles[i]);
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
        if (*szFName==_T('\0'))
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
//__asm int 3;
    if (FAILED(m_pDesktop->ParseDisplayName(NULL_HWND, NULL, strFilesDir, &nCount
      , &oDirPidl, NULL)))
    {
      enRet=DOMNU_ERR_SHOW;
      break;
    }
    IShellFolderPtr pCurFolder;
    if (FAILED(m_pDesktop->BindToObject(oDirPidl, NULL, IID_IShellFolder
      , reinterpret_cast<void**>(&pCurFolder))))
    {
      enRet=DOMNU_ERR_SHOW;
      break;
    }
    CPidl oPidl(m_pMalloc);
    for (i=0; i<nFiles+nFolders; i++)
    {
      LPITEMIDLIST pidl;
      auto_sz szFile(pFiles[i]);
      if (szFile.Len()==2 && _T(':')==szFile[1])
      {
        // диск (c:)
        szFile+=_T("\\");
      }
      HRESULT hr=0;
      if (FAILED(hr=pCurFolder->ParseDisplayName(NULL_HWND, NULL, szFile
        , &nCount, &pidl, NULL)))
      {
        enRet=DOMNU_ERR_SHOW;
        break;
      }
      oPidl.Add(pidl);
    }
    if (oPidl.Count()==nFolders+nFiles)
    {
      enRet=DoMenu(pCurFolder, oPidl.GetArray(), pFiles, nFiles
        , nFolders, szCommand, enAutoItem);
    }
  } while (0);
  delete[] pParams;
  return enRet;
}

bool CPlugin::GetFilesFromParams(LPTSTR szCmdLine
                 , LPCTSTR** ppFiles
                 , unsigned* pnFiles
                 , unsigned* pnFolders
                 , auto_sz* pstrCurDir
                 , bool bSkipFirst)
{
#ifndef UNICODE
  PanelInfo pi;
  if (!Control(FCTL_GETPANELSHORTINFO, &pi))
    return false;
  *pstrCurDir=auto_sz(pi.CurDir, lstrlen(pi.CurDir)+1);
#else
  int Size=Control(FCTL_GETPANELDIR,0,NULL);
  wchar_t *CurDir=new wchar_t[Size];
  Control(FCTL_GETPANELDIR,Size,(LONG_PTR)CurDir);
  *pstrCurDir=auto_sz(CurDir,Size);
  delete[] CurDir;
#endif
  if (pstrCurDir->Len()) m_fsf.AddEndSlash(*pstrCurDir);
  unsigned nCnt=ParseParams(szCmdLine);
  if (!nCnt) return false;
  *ppFiles=new LPCTSTR[nCnt];
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

unsigned CPlugin::ParseParams(LPTSTR szParams, LPCTSTR* pFiles/*=NULL*/)
{
  unsigned nCnt=0;
  bool bStartNew=true;
  bool bInsideQuotes=false;
  for (; *szParams; szParams++)
  {
    if (IsSpace(*szParams) && !bInsideQuotes)
    {
      bStartNew=true;
      if (pFiles) *szParams=_T('\0');
    }
    else
    {
      if (_T('\"')==*szParams)
      {
        bStartNew=true;
        if (pFiles) *szParams=_T('\0');
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

bool CPlugin::GetFilesFromPanel(LPCTSTR** ppFiles, unsigned* pnFiles
    , unsigned* pnFolders, auto_sz* pstrCurDir)
{
#ifdef UNICODE
  if(SelectedItems && SelectedItemsCount)
  {
   for(int i=0;i<SelectedItemsCount;i++)
    if(SelectedItems[i])
     delete[] SelectedItems[i];
   delete[] SelectedItems;
   SelectedItems=NULL;
   SelectedItemsCount=0;
  }
#endif
  PanelInfo pi;
#ifndef UNICODE
  if (!Control(FCTL_GETPANELINFO, &pi))
#else
  if (!Control(FCTL_GETPANELINFO,0,(LONG_PTR)&pi))
#endif
  {
    return false;
  }
#ifndef UNICODE
  // preserve space for AddEndSlash
  *pstrCurDir = auto_sz(pi.CurDir, lstrlen(pi.CurDir)+1);
#else
  int Size=Control(FCTL_GETPANELDIR,0,NULL);
  wchar_t *CurDir=new wchar_t[Size];
  Control(FCTL_GETPANELDIR,Size,(LONG_PTR)CurDir);
  // preserve space for AddEndSlash
  *pstrCurDir=auto_sz(CurDir,Size);
  delete[] CurDir;
#endif

#ifndef UNICODE
  if (!pi.SelectedItemsNumber ||
    (1==pi.SelectedItemsNumber && 0==lstrcmp(pi.SelectedItems[0].FindData.cFileName, ".."))
    )
#else
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
        Root=(pi.SelectedItemsNumber==1 && !lstrcmp(PPI->FindData.lpwszFileName,L".."));
        delete[] PPI;
      }
    }
  }
  if(Root)
#endif
  {
    *pnFolders=1;
    LPCTSTR szFName=m_fsf.PointToName(*pstrCurDir);
    if (*szFName==_T('\0'))
    {
      // это бывает для дисков (c:, c:\)
      szFName=*pstrCurDir;
    }
    static TCHAR szFNameTmp[1024];
    lstrcpy(szFNameTmp,szFName);
    (*ppFiles)=new LPCTSTR[1];
    (*ppFiles)[0]=szFNameTmp;
    pstrCurDir->Trunc(szFName-pstrCurDir->operator LPCTSTR());
  }
  else
  {
    if (pstrCurDir->Len()) m_fsf.AddEndSlash(*pstrCurDir);
    *ppFiles=new LPCTSTR[pi.SelectedItemsNumber];
#ifdef UNICODE
    SelectedItemsCount=pi.SelectedItemsNumber;
    SelectedItems=new PluginPanelItem*[SelectedItemsCount];
#endif
    for (int i=0; i<pi.SelectedItemsNumber; i++)
    {
#ifndef UNICODE
      LPCTSTR szPath=pi.SelectedItems[i].FindData.cFileName;
      if (!lstrcmp(pi.PanelItems[pi.CurrentItem].FindData.cFileName, szPath))
#else
      SelectedItems[i]=(PluginPanelItem*)new char[Control(FCTL_GETSELECTEDPANELITEM,i,NULL)];
      Control(FCTL_GETSELECTEDPANELITEM,i,(LONG_PTR)SelectedItems[i]);
      LPCTSTR szPath=SelectedItems[i]->FindData.lpwszFileName;
      PluginPanelItem *PPI=(PluginPanelItem*)new char[Control(FCTL_GETPANELITEM,pi.CurrentItem,NULL)];
      Control(FCTL_GETPANELITEM,pi.CurrentItem,(LONG_PTR)PPI);
      bool Equal=!lstrcmp(PPI->FindData.lpwszFileName,szPath);
      delete[] PPI;
      if(Equal)
#endif
      {
        (*ppFiles)[i]=(*ppFiles)[0];
        (*ppFiles)[0]=szPath;
      }
      else
      {
        (*ppFiles)[i]=szPath;
      }
      if (FILE_ATTRIBUTE_DIRECTORY
#ifndef UNICODE
        & pi.SelectedItems[i].FindData.dwFileAttributes)
#else
        & SelectedItems[i]->FindData.dwFileAttributes)
#endif
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
                 , LPCTSTR pFiles[], unsigned nFiles
                 , unsigned nFolders, LPCTSTR szCommand/*=NULL*/
                 , EAutoItem enAutoItem/*=AI_NONE*/)
{
  assert(nFolders+nFiles);
  auto_sz strMnuTitle;
  if (nFolders+nFiles==1)
  {
    strMnuTitle=pFiles[0];
    if (nFolders)
    {
      strMnuTitle.RemoveTrailing(_T('\\'));
      strMnuTitle+=_T("\\");
    }
  }
  else
  {
    auto_sz strFiles;
    strFiles.Realloc(20);
    wsprintf(strFiles, _T("%d %s"), nFiles, GetMsg(LNG_FILES));

    auto_sz strFolders;
    strFolders.Realloc(20);
    wsprintf(strFolders, _T("%d %s"), nFolders, GetMsg(LNG_FOLDERS));

    if (nFiles)
    {
      strMnuTitle+=strFiles;
    }
    if (nFolders)
    {
      if (nFiles) strMnuTitle+=_T(", ");
      strMnuTitle+=strFolders;
    }
  }
  IContextMenuPtr pCMenu1;

  if (FAILED(pCurFolder->GetUIObjectOf(NULL_HWND, nFolders+nFiles, pPiids
    , IID_IContextMenu, 0, reinterpret_cast<void**>(&pCMenu1))))
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
  if (!pPreferredMenu || FAILED(pPreferredMenu->QueryContextMenu(oHMenu
    , 0, MENUID_CMDOFFSET, 0x7FFF, CMF_CANRENAME|(GetKeyState(VK_SHIFT)&0xF000?EMENU_CMF_EXTENDEDVERBS:0))))
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
        if (!GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET
          , AS_VERB, &strAutoItem, FALSE))
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
      if (strAutoItem.CompareExcluding(szCommand, _T('&')))
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
        if (!ShowTextMenu(oHMenu, pPreferredMenu, pCMenu2, pCMenu3, &nCmd, strMnuTitle
          , pCurFolder, pPiids, nFolders+nFiles))
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
    if (FAILED(pPreferredMenu->GetCommandString(nId, GCS_VERBA, NULL, szVerb
      , ArraySize(szVerb))))
    {
      szVerb[0]=_T('\0');
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
  HWND hWnd=CreateDialogParam(m_hModule, MAKEINTRESOURCE(IDD_NULL), NULL
    , (DLGPROC)MenuDlgProc, (LPARAM)&DlgParam);
  assert(hWnd);
  if (!hWnd) return false;
  RECT rc;
  if (!GetWindowRect(hWnd, &rc)
    || !SetWindowPos(hWnd, HWND_TOPMOST, pt.x-(rc.right-rc.left)/2
      , pt.y-(rc.bottom-rc.top)/2
      , 0, 0, SWP_NOSIZE))
  {
    assert(0);
  }
  *pnCmd=TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON
    , pt.x, pt.y, 0, hWnd, NULL);
  if (!DestroyWindow(hWnd))
  {
    assert(0);
  }
  return true;
}

bool CPlugin::GetAdditionalString(IContextMenu* pContextMenu, UINT nID
                  , EAdditionalStr enAdditionalString
                  , auto_sz* pstr, BOOL bToOEM/*=TRUE*/)
{
  UINT nType;
  switch (enAdditionalString)
  {
  case AS_HELPTEXT:
    nType=m_bWin9x?GCS_HELPTEXTA:GCS_HELPTEXTW;
    break;
  case AS_VERB:
    nType=m_bWin9x?GCS_VERBA:GCS_VERBW;
    break;
  default:
    return false;
  }
  WCHAR szwAddInfo[200]=L"\0";
  if (FAILED(pContextMenu->GetCommandString(nID, nType, NULL
    , reinterpret_cast<LPSTR>(szwAddInfo), ArraySize(szwAddInfo))))
  {
    return false;
  }
  if (m_bWin9x)
  {
    *pstr=reinterpret_cast<LPCTSTR>(szwAddInfo);
#ifndef UNICODE
    if (bToOEM) pstr->Ansi2Oem();
#endif
  }
  else
  {
    *pstr=szwAddInfo;
  }
  if (AS_HELPTEXT==enAdditionalString) pstr->RemoveTrailing(_T('.'));
  return !pstr->IsEmpty();
}

bool CPlugin::ShowTextMenu(HMENU hMenu
              , LPCONTEXTMENU pPreferredMenu
              , LPCONTEXTMENU2 pMenu2
              , LPCONTEXTMENU3 pMenu3
              , int* pnCmd
              , LPCTSTR szTitle
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
#ifndef UNICODE
      szItem.Ansi2Oem();
#endif
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
            || GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, AS_VERB, &szSub, FALSE))
        {
          szItem=_T("{");

          TCHAR *Buf=(TCHAR*)malloc((szSub.Len()+1)*sizeof(TCHAR));
          if (Buf)
          {
            //Получаем из shell32.dll шаблон, по которому формируются эти подсказки,
            //(строковый ресурс #5380, "Opens the document with %s."),
            //и убираем его из сабжевых строк. Для XP/2003.
            *Buf=0;
            LoadString(GetModuleHandle(_T("shell32.dll")),5380,Buf,int(szSub.Len()+1));
            int i=0;
            while(Buf[i] && Buf[i]!=_T('%'))
              i++;
            if (Buf[i] == _T('%') && !_tcsncmp(Buf,szSub,i))
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
            szRight = ((LPCTSTR)szSub)+szSub.Len()/2;
            if (szLeft.CompareExcluding(szRight, _T('&')))
            {
              //if (szLeft.Len() > szRight.Len())
                //szSub = szLeft;
              //else
                szSub = szRight;
            }
          }

          szItem+=szSub;
          szItem+=_T("}");
        }
      }
      if (0==szItem.Len()) szItem = GetMsg(grphid);
    }
    if (m_enHelptext != AS_NONE) {
      auto_sz szAddInfo;
      if (GetAdditionalString(pPreferredMenu, mii.wID-MENUID_CMDOFFSET, m_enHelptext, &szAddInfo))
      {
        if (!m_DifferentOnly || !szItem.CompareExcluding(szAddInfo, _T('&')))
        {
          szItem+=_T(" (");
          szItem+=szAddInfo;
          szItem+=_T(")");
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
      if ( ((m_bWin95 || m_bWinNT) && MENUID_SENDTO_WIN95NT4==mii.wID)
        || ((m_bWin98 || !m_bWin9x) && MENUID_SENDTO_WIN98==mii.wID)
        || (m_bWinME && MENUID_SENDTO_WINME==mii.wID) )
      {
        if (!m_pMalloc) return false;
        CPidl oSendtoPidl(m_pMalloc);
        if (FAILED(SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO
          , &oSendtoPidl)))
        {
          return false;
        }
        IDropTargetPtr pDropTarget;
        bShowMenuRes=ShowFolder(m_pDesktop, oSendtoPidl, pnCmd
          , szTitle, &pDropTarget);
        if (!bShowMenuRes)
        {
          return false;
        }
        if (*pnCmd!=MENUID_CANCELLED)
        {
          IDataObjectPtr pDataObject;
          if (FAILED(pCurFolder->GetUIObjectOf(
            NULL_HWND
            , nPiidCnt
            , ppiid
            , IID_IDataObject
            , NULL
            , reinterpret_cast<void**>(&pDataObject))))
          {
            return false;
          }
          POINTL pt={0};
          DWORD nEffect=DROPEFFECT_COPY;
          if (FAILED(pDropTarget->DragEnter(pDataObject, MK_LBUTTON
            , pt, &nEffect)))
          {
            return false;
          }
          if (FAILED(pDropTarget->Drop(pDataObject, MK_LBUTTON
            , pt, &nEffect)))
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
          if (NOERROR!=pMenu3->HandleMenuMsg(WM_INITMENUPOPUP
            , (WPARAM)mii.hSubMenu, nItem))
          {
          }
        }
        else if (pMenu2)
        {
          if (NOERROR!=pMenu2->HandleMenuMsg(WM_INITMENUPOPUP
            , (WPARAM)mii.hSubMenu, nItem))
          {
          }
        }
        bShowMenuRes=ShowTextMenu(mii.hSubMenu, pPreferredMenu, pMenu2
          , pMenu3, pnCmd, szTitle, pCurFolder, ppiid, nPiidCnt);
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

bool CPlugin::ShowFolder(LPSHELLFOLDER pParentFolder, LPCITEMIDLIST piid
             , int* pnCmd, LPCTSTR szTitle, LPDROPTARGET* ppDropTarget)
{
  IShellFolderPtr pFolder;
  if (FAILED(pParentFolder->BindToObject(piid, NULL, IID_IShellFolder
    , reinterpret_cast<void**>(&pFolder))))
  {
    return false;
  }
  IEnumIDListPtr pEnum;
  if (FAILED(pFolder->EnumObjects(NULL_HWND, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS
    |SHCONTF_INCLUDEHIDDEN, &pEnum)))
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
      if (FAILED(pFolder->GetUIObjectOf(
        NULL_HWND
        , 1
        , &pSelPiid
        , IID_IDropTarget
        , NULL
        , (LPVOID*)ppDropTarget)))
      {
        return false;
      }
      *pnCmd=MENUID_SENDTO_DONE;
      break;
    }
  }
  return true;
}
