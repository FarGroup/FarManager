#include "ftplng.hpp"
#include "ftpincl.hpp"
#include "ftp.hpp"

OSVERSIONINFO WinVer;

#include "ftpclass.cpp"
#include "ftpsmall.cpp"
#include "ftpmix.cpp"
#include "ftpreg.cpp"
#include "ftpcfg.cpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  sprintf(PluginRootKey,"%s\\FTP",Info->RootKey);
  Opt.AddToDisksMenu=GetRegKey(HKEY_CURRENT_USER,"","AddToDisksMenu",1);
  Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,"","DisksMenuDigit",2);
  GetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames,
            "00_index.txt,0index,0index.txt",sizeof(Opt.DescriptionNames));
  Opt.ReadDescriptions=GetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",0);
  Opt.UploadLowCase=GetRegKey(HKEY_CURRENT_USER,"","UploadLowCase",0);
  Opt.ShowUploadDialog=GetRegKey(HKEY_CURRENT_USER,"","ShowUploadDialog",1);
  Opt.UpdateDescriptions=GetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",0);
  char CryptedPassword[256];
  GetRegKey(HKEY_CURRENT_USER,"","DefaultPassword",(BYTE *)CryptedPassword,(BYTE *)NULL,sizeof(CryptedPassword));
  FTP::DecryptPassword(CryptedPassword,Opt.DefaultPassword);
  GetRegKey(HKEY_CURRENT_USER,"","Firewall",Opt.Firewall,"",sizeof(Opt.Firewall));
  Opt.PassiveMode=GetRegKey(HKEY_CURRENT_USER,"","PassiveMode",0);
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  HANDLE hPlugin=new FTP;
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  FTP *Ftp=(FTP *)hPlugin;
  if (OpenFrom==OPEN_SHORTCUT)
  {
    if (!Ftp->ProcessShortcutLine((char *)Item))
    {
      delete Ftp;
      return(INVALID_HANDLE_VALUE);
    }
    return(hPlugin);
  }
  if (OpenFrom==OPEN_COMMANDLINE)
  {
    if (!Ftp->ProcessCommandLine((char *)Item))
    {
      delete Ftp;
      return(INVALID_HANDLE_VALUE);
    }
  }
  return(hPlugin);
}


void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (FTP *)hPlugin;
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->GetFindData(pPanelItem,pItemsNumber,OpMode));
}


void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  FTP *Ftp=(FTP *)hPlugin;
  Ftp->FreeFindData(PanelItem,ItemsNumber);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  static char *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MFtpDiskMenu);
  static int DiskMenuNumbers[1];
  Info->DiskMenuStrings=DiskMenuStrings;
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MFtpMenu);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  static char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MFtpMenu);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
  Info->CommandPrefix="ftp";
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  FTP *Ftp=(FTP *)hPlugin;
  Ftp->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->SetDirectory(Dir,OpMode));
}


int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode));
}


int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->PutFiles(PanelItem,ItemsNumber,Move,OpMode));
}


int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}


int WINAPI _export MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->MakeDirectory(Name,OpMode));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->ProcessKey(Key,ControlState));
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->ProcessEvent(Event,Param));
}


int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}


int WINAPI _export Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode)
{
  FTP *Ftp=(FTP *)hPlugin;
  return(Ftp->Compare(Item1,Item2,Mode));
}
