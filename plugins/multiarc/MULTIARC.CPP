#include "multiarc.hpp"
#include "arcread.cpp"
#include "ArcPut.cpp"
#include "arcproc.cpp"
#include "arcmix.cpp"
#include "arccfg.cpp"
#include "arccmd.cpp"
#include "arcplg.cpp"
#include "arcreg.cpp"


int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1044);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  #ifdef __USE_OWN_RTL__
  heapNew=GetProcessHeap();
  #endif
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  FarVER=::Info.AdvControl(::Info.ModuleNumber,ACTL_GETFARVERSION,NULL);

  ::FSF.sprintf(PluginRootKey,"%s\\MultiArc",Info->RootKey);
  if (ArcPlugin==NULL)
    ArcPlugin=new ArcPlugins(Info->ModuleName);
  Opt.HideOutput=GetRegKey(HKEY_CURRENT_USER,"","HideOutput",0);
  Opt.ProcessShiftF1=GetRegKey(HKEY_CURRENT_USER,"","ProcessShiftF1",1);
  Opt.UseLastHistory=GetRegKey(HKEY_CURRENT_USER,"","UseLastHistory",0);

  //Opt.DeleteExtFile=GetRegKey(HKEY_CURRENT_USER,"","DeleteExtFile",1);
  //Opt.AddExtArchive=GetRegKey(HKEY_CURRENT_USER,"","AddExtArchive",0);
  Opt.AutoOffExactArcName=GetRegKey(HKEY_CURRENT_USER,"","AutoOffExactArcName",1);
  Opt.ExactArcName=GetRegKey(HKEY_CURRENT_USER, "", "ExactArcName", 0);

  GetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames,
            "descript.ion,files.bbs",sizeof(Opt.DescriptionNames));
  Opt.ReadDescriptions=GetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",0);
  Opt.UpdateDescriptions=GetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",0);
  Opt.UserBackground=GetRegKey(HKEY_CURRENT_USER,"","Background",0);

  #ifdef _NEW_ARC_SORT_
  strcpy(IniFile, Info->ModuleName);
  *((int *)(IniFile+strlen(IniFile)-3))=0x696E69; // :)
  #endif
}


HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,
                                     int DataSize)
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
    int ReadCode=Plugin->ReadArchive(Name);
    if (ReadCode==0)
      return INVALID_HANDLE_VALUE;
    if (ReadCode==-1)
      return (HANDLE)-2;
  }
  return hPlugin;
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
  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=(char*)GetMsg(MCfgLine0);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
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
  strcpy(MenuItems[0].Text,GetMsg(MCfgLine1));
  strcpy(MenuItems[1].Text,GetMsg(MCfgLine2));
  MenuItems[0].Selected=TRUE;

  do{
    ItemNumber=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,
                       GetMsg(MCfgLine0),NULL,"Config",NULL,NULL,MenuItems,
                       sizeof(MenuItems)/sizeof(MenuItems[0]));
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
          ConfigCommands(ArcFormat);
        break;
      }
    }
    MenuItems[0].Selected=FALSE;
    MenuItems[1].Selected=FALSE;
    MenuItems[ItemNumber].Selected=TRUE;
  }while(1);

  return FALSE;
}
