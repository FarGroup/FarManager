#include <initguid.h>
#include "Network.hpp"
#include "version.hpp"

//-----------------------------------------------------------------------------
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

//-----------------------------------------------------------------------------
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  InitializeNetFunction();

  HANDLE hPlugin=new NetBrowser;
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  NetBrowser *Browser=(NetBrowser *)hPlugin;

  if(OInfo->OpenFrom==OPEN_COMMANDLINE)
  {
    wchar_t Path[MAX_PATH] = L"\\\\";

    int I=0;
    wchar_t *cmd=(wchar_t *)OInfo->Data;
    wchar_t *p=wcschr(cmd, L':');
    if (!p || !*p)
    {
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
    *p++ = L'\0';
    bool netg;
    if (!lstrcmpi(cmd, L"netg"))
      netg = true;
    else if (!lstrcmpi(cmd, L"net"))
      netg = false;
    else
    {
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
    cmd = p;
    if(lstrlen(FSF.Trim(cmd)))
    {
      if (cmd [0] == L'/')
        cmd [0] = L'\\';
      if (cmd [1] == L'/')
        cmd [1] = L'\\';
      if (!netg && !Opt.NavigateToDomains)
      {
        if(cmd[0] == L'\\' && cmd[1] != L'\\')
          I=1;
        else if(cmd[0] != L'\\' && cmd[1] != L'\\')
          I=2;
      }
      lstrcpy(Path+I, cmd);

      FSF.Unquote(Path);
      // Expanding environment variables.
      {
          wchar_t PathCopy[MAX_PATH];
          lstrcpy(PathCopy, Path);
          ExpandEnvironmentStrings(PathCopy, Path, ARRAYSIZE(Path));
      }
      Browser->SetOpenFromCommandLine (Path);
    }
  }
  /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
      mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
      in later versions. Please DON'T use it in your plugins. */
  else if (OInfo->OpenFrom == OPEN_FILEPANEL)
  {
    if (!Browser->SetOpenFromFilePanel ((wchar_t *) OInfo->Data))
    {
      // we don't support upwards browsing from NetWare shares -
      // it doesn't work correctly
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
  }
  else {
    if (IsFirstRun && Opt.LocalNetwork)
      Browser->GotoLocalNetwork();
  }
  IsFirstRun = FALSE;

  wchar_t szCurrDir[MAX_PATH];
  if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir))
  {
    if (*szCurrDir == L'\\' && GetSystemDirectory(szCurrDir, ARRAYSIZE(szCurrDir)))
    {
      szCurrDir[2] = L'\0';
      SetCurrentDirectory(szCurrDir);
    }
  }
  return(hPlugin);
}

//-----------------------------------------------------------------------------
void WINAPI ClosePanelW(HANDLE hPlugin)
{
  delete (NetBrowser *)hPlugin;
}

//-----------------------------------------------------------------------------
int WINAPI GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
}

//-----------------------------------------------------------------------------
void WINAPI FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->FreeFindData(PanelItem,ItemsNumber);
}

//-----------------------------------------------------------------------------
void WINAPI GetOpenPanelInfoW(HANDLE hPlugin,struct OpenPanelInfo *Info)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->GetOpenPanelInfo(Info);
}

//-----------------------------------------------------------------------------
int WINAPI SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->SetDirectory(Dir,OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                                 int ItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessKeyW(HANDLE hPlugin,const INPUT_RECORD *Rec)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->ProcessKey(Rec));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessEventW(HANDLE hPlugin,int Event,void *Param)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return Browser->ProcessEvent (Event, Param);
}

//-----------------------------------------------------------------------------
