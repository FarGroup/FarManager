#include "Network.hpp"

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


HANDLE WINAPI OpenPlugin(int OpenFrom,int Item)
{
  if(!IsOldFAR)
  {
    InitializeNetFunction();

    HANDLE hPlugin=new NetBrowser;
    if (hPlugin==NULL)
      return(INVALID_HANDLE_VALUE);
    NetBrowser *Browser=(NetBrowser *)hPlugin;

    if(OpenFrom==OPEN_COMMANDLINE)
    {
      char Path[NM]="\\\\";
      int I=0;
      char *cmd=(char *)Item;
      if(lstrlen(FSF.Trim(cmd)))
      {
        if (cmd [0] == '/')
          cmd [0] = '\\';
        if (cmd [1] == '/')
          cmd [1] = '\\';
        if(cmd[0] == '\\' && cmd[1] != '\\')
          I=1;
        else if(cmd[0] != '\\' && cmd[1] != '\\')
          I=2;
        OemToChar (cmd, Path+I);

        FSF.Unquote(Path);
        Browser->SetOpenFromCommandLine (Path);
      }
    }
    /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
       mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
       in later versions. Please DON'T use it in your plugins. */
    else if (OpenFrom == 7)
    {
      if (!Browser->SetOpenFromFilePanel ((char *) Item))
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

    char szCurrDir[MAX_PATH];
    if (GetCurrentDirectory(sizeof(szCurrDir), szCurrDir))
    {
      if (*szCurrDir == '\\' && GetSystemDirectory(szCurrDir, sizeof(szCurrDir)))
      {
        szCurrDir[2] = '\0';
        SetCurrentDirectory(szCurrDir);
      }
    }
    return(hPlugin);
  }
  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  if(!IsOldFAR)
  {
    delete (NetBrowser *)hPlugin;
  }
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
  return FALSE;
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    Browser->FreeFindData(PanelItem,ItemsNumber);
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    Browser->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->SetDirectory(Dir,OpMode));

  return(FALSE);
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                               int ItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
  return(FALSE);
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if(!IsOldFAR)
    return(Browser->ProcessKey(Key,ControlState));
  return(FALSE);
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  if (!IsOldFAR)
    return Browser->ProcessEvent (Event, Param);
  return(FALSE);
}
