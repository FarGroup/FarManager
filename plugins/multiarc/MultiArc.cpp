#include "MultiArc.hpp"
#include "marclng.hpp"

int WINAPI _export GetMinFarVersion(void)
{
  return FARMANAGERVERSION;
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  FSF=*Info->FSF;
  ::Info.FSF=&FSF;

  ::Info.FSF->Reserved[0]=(DWORD_PTR)malloc;
  ::Info.FSF->Reserved[1]=(DWORD_PTR)free;

  FSF.sprintf(PluginRootKey,"%s\\MultiArc",Info->RootKey);

  if (ArcPlugin==NULL)
    ArcPlugin=new ArcPlugins(Info->ModuleName);

  Opt.HideOutput=GetRegKey(HKEY_CURRENT_USER,"","HideOutput",0);
  Opt.ProcessShiftF1=GetRegKey(HKEY_CURRENT_USER,"","ProcessShiftF1",1);
  Opt.UseLastHistory=GetRegKey(HKEY_CURRENT_USER,"","UseLastHistory",0);

  //Opt.DeleteExtFile=GetRegKey(HKEY_CURRENT_USER,"","DeleteExtFile",1);
  //Opt.AddExtArchive=GetRegKey(HKEY_CURRENT_USER,"","AddExtArchive",0);

  //Opt.AutoResetExactArcName=GetRegKey(HKEY_CURRENT_USER,"","AutoResetExactArcName",1);
  //Opt.ExactArcName=GetRegKey(HKEY_CURRENT_USER, "", "ExactArcName", 0);
  Opt.AdvFlags=GetRegKey(HKEY_CURRENT_USER, "", "AdvFlags", 2);

  GetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames,
            "descript.ion,files.bbs",sizeof(Opt.DescriptionNames));
  Opt.ReadDescriptions=GetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",0);
  Opt.UpdateDescriptions=GetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",0);
  //Opt.UserBackground=GetRegKey(HKEY_CURRENT_USER,"","Background",0); // $ 06.02.2002 AA
  Opt.OldUserBackground=0; // $ 02.07.2002 AY
  Opt.AllowChangeDir=GetRegKey(HKEY_CURRENT_USER,"","AllowChangeDir",0);

  GetRegKey(HKEY_CURRENT_USER,"","Prefix1",Opt.CommandPrefix1,"ma",sizeof(Opt.CommandPrefix1));

  #ifdef _NEW_ARC_SORT_
  lstrcpy(IniFile, Info->ModuleName);
  *((int *)(IniFile+lstrlen(IniFile)-3))=0x696E69; // :)
  #endif
  Opt.PriorityClass=2; // default: NORMAL

  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
}


HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  if (ArcPlugin==NULL)
    return INVALID_HANDLE_VALUE;
  int ArcPluginNumber=-1;
  if (Name==NULL)
  {
    if (!Opt.ProcessShiftF1)
      return INVALID_HANDLE_VALUE;
  }
  else
  {
    ArcPluginNumber=ArcPlugin->IsArchive(Name,Data,DataSize);
    if (ArcPluginNumber==-1)
      return INVALID_HANDLE_VALUE;
  }
  HANDLE hPlugin=new PluginClass(ArcPluginNumber);
  if (hPlugin==NULL)
    return INVALID_HANDLE_VALUE;
  if (Name!=NULL)
  {
    PluginClass *Plugin=(PluginClass *)hPlugin;
    if (!Plugin->PreReadArchive(Name))
      return INVALID_HANDLE_VALUE;
  }
  return hPlugin;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom, INT_PTR Item)
{
  if (ArcPlugin==NULL)
    return INVALID_HANDLE_VALUE;

  bool known_command = false;
  int pref_len = -1;
  if (OpenFrom == OPEN_SHORTCUT)
	  known_command = true;
  else if (OpenFrom == OPEN_COMMANDLINE)
  {
	  char *cmd = (char *)Item;
	  pref_len = lstrlen(Opt.CommandPrefix1);
	  known_command = (FSF.LStrnicmp(Opt.CommandPrefix1,cmd,pref_len) == 0 && cmd[pref_len] == ':');
  }

  if (known_command)
  {
    ++pref_len;
    if (!((char *)Item)[pref_len])
      return INVALID_HANDLE_VALUE;
    char oldfilename[NM+2];
    lstrcpyn(oldfilename,&((char *)Item)[pref_len],sizeof(oldfilename));
    FSF.Unquote(oldfilename);
    FSF.ExpandEnvironmentStr(oldfilename,oldfilename,NM);
    char filename[NM];
    char *fnp;
    DWORD bl = GetFullPathName(oldfilename,NM,filename,&fnp);
    if (bl>NM || bl==0)
      return INVALID_HANDLE_VALUE;
    HANDLE h = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (h == INVALID_HANDLE_VALUE)
      return INVALID_HANDLE_VALUE;
    DWORD size = (DWORD)Info.AdvControl(Info.ModuleNumber,ACTL_GETPLUGINMAXREADDATA,(void *)0);
    unsigned char *data;
    data = (unsigned char *) malloc(size*sizeof(unsigned char));
    if (!data)
      return INVALID_HANDLE_VALUE;
    DWORD datasize;
    BOOL b = ReadFile(h,data,size,&datasize,NULL);
    CloseHandle(h);
    if (!(b&&datasize))
    {
      free(data);
      return INVALID_HANDLE_VALUE;
    }
    h = OpenFilePlugin(filename,data,datasize);
    free(data);
    if ((h==(HANDLE)-2) || h==INVALID_HANDLE_VALUE)
      return INVALID_HANDLE_VALUE;

    if (OpenFrom == OPEN_SHORTCUT)
    {
      ((PluginClass *)h)->ReadArchive(filename);
    }
    return h;
  }
  return INVALID_HANDLE_VALUE;
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (PluginClass *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->GetFindData(pPanelItem,pItemsNumber,OpMode);
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  Plugin->FreeFindData(PanelItem,ItemsNumber);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->SetDirectory(Dir,OpMode);
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->DeleteFiles(PanelItem,ItemsNumber,OpMode);
}


int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode);
}


int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->PutFiles(PanelItem,ItemsNumber,Move,OpMode);
}


void WINAPI _export ExitFAR()
{
  delete ArcPlugin;
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_FULLCMDLINE;
  static const char *PluginCfgStrings[1];
  PluginCfgStrings[0]=(char*)GetMsg(MCfgLine0);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=ARRAYSIZE(PluginCfgStrings);
  static char CommandPrefix[sizeof(Opt.CommandPrefix1)];
  lstrcpy(CommandPrefix,Opt.CommandPrefix1);
  Info->CommandPrefix=CommandPrefix;
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  Plugin->GetOpenPluginInfo(Info);
}


int WINAPI _export ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->ProcessHostFile(PanelItem,ItemsNumber,OpMode);
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return Plugin->ProcessKey(Key,ControlState);

}

int WINAPI _export Configure(int ItemNumber)
{
  struct FarMenuItem MenuItems[2];
  memset(MenuItems,0,sizeof(MenuItems));
  lstrcpy(MenuItems[0].Text,GetMsg(MCfgLine1));
  lstrcpy(MenuItems[1].Text,GetMsg(MCfgLine2));
  MenuItems[0].Selected=TRUE;

  do{
    ItemNumber=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,
                       GetMsg(MCfgLine0),NULL,"Config",NULL,NULL,MenuItems,
                       ARRAYSIZE(MenuItems));
    switch(ItemNumber)
    {
      case -1:
        return FALSE;
      case 0:
        ConfigGeneral();
        break;
      case 1:
      {
        char ArcFormat[100];
        *ArcFormat=0;
        while(PluginClass::SelectFormat(ArcFormat))
          ;//ConfigCommands(ArcFormat);
        break;
      }
    }
    MenuItems[0].Selected=FALSE;
    MenuItems[1].Selected=FALSE;
    MenuItems[ItemNumber].Selected=TRUE;
  }while(1);

  //return FALSE;
}
