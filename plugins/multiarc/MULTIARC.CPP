#include "multiarc.hpp"
#include "arcread.cpp"
#include "arcproc.cpp"
#include "arcmix.cpp"
#include "arccfg.cpp"
#include "arccmd.cpp"
#include "arcplg.cpp"
#include "arcreg.cpp"


void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  sprintf(PluginRootKey,"%s\\MultiArc",Info->RootKey);
  if (ArcPlugin==NULL)
    ArcPlugin=new ArcPlugins(Info->ModuleName);
  Opt.HideOutput=GetRegKey(HKEY_CURRENT_USER,"","HideOutput",0);
  Opt.ProcessShiftF1=GetRegKey(HKEY_CURRENT_USER,"","ProcessShiftF1",1);
  GetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames,
            "descript.ion,files.bbs",sizeof(Opt.DescriptionNames));
  Opt.ReadDescriptions=GetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",0);
  Opt.UpdateDescriptions=GetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",0);
}


HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,
                                     unsigned int DataSize)
{
  if (ArcPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  int ArcPluginNumber=-1;
  if (Name==NULL)
  {
    if (!Opt.ProcessShiftF1)
      return(INVALID_HANDLE_VALUE);
  }
  else
  {
    ArcPluginNumber=ArcPlugin->IsArchive(Name,Data,DataSize);
    if (ArcPluginNumber==-1)
      return(INVALID_HANDLE_VALUE);
  }
  HANDLE hPlugin=new PluginClass(ArcPluginNumber);
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  if (Name!=NULL)
  {
    PluginClass *Plugin=(PluginClass *)hPlugin;
    int ReadCode=Plugin->ReadArchive(Name);
    if (ReadCode==0)
      return(INVALID_HANDLE_VALUE);
    if (ReadCode==-1)
      return((HANDLE)-2);
  }
  return(hPlugin);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (PluginClass *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->GetFindData(pPanelItem,pItemsNumber,OpMode));
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  Plugin->FreeFindData(PanelItem,ItemsNumber);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->SetDirectory(Dir,OpMode));
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}


int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode));
}


int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,int OpMode)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->PutFiles(PanelItem,ItemsNumber,Move,OpMode));
}


void WINAPI _export ExitFAR()
{
  delete ArcPlugin;
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  Info->DiskMenuStrings=NULL;
  Info->DiskMenuNumbers=NULL;
  Info->DiskMenuStringsNumber=0;
  Info->PluginMenuStrings=NULL;
  Info->PluginMenuStringsNumber=0;
  static char *PluginCfgStrings[2];
  PluginCfgStrings[0]=GetMsg(MCfgLine1);
  PluginCfgStrings[1]=GetMsg(MCfgLine2);
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
  return(Plugin->ProcessHostFile(PanelItem,ItemsNumber,OpMode));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  PluginClass *Plugin=(PluginClass *)hPlugin;
  return(Plugin->ProcessKey(Key,ControlState));

}

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(ConfigGeneral());
    case 1:
      return(ConfigCommands());
  }
  return(FALSE);
}
