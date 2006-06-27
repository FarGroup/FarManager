#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "farcmds.hpp"
#include "lang.hpp"
#include "crt.hpp"

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
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

FARSTDCOPYTOCLIPBOARD CopyToClipboard;
FARSTDATOI FarAtoi;
FARSTDITOA FarItoa;
FARSTDSPRINTF FarSprintf;
FARSTDUNQUOTE Unquote;
FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
FARSTDPOINTTONAME PointToName;
FARSTDGETPATHROOT GetPathRoot;
FARSTDADDENDSLASH AddEndSlash;
FARSTDMKTEMP MkTemp;
FARSTDRTRIM  FarRTrim;
FARSTDLTRIM  FarLTrim;
FARSTDLTRIM  FarTrim;
FARSTDLOCALSTRICMP LStricmp;
FARSTDLOCALSTRNICMP LStrnicmp;
FARSTDMKLINK MkLink;
FARSTDKEYNAMETOKEY FarNameToKey;
FARSTDQUOTESPACEONLY QuoteSpaceOnly;
FARSTDRECURSIVESEARCH FarRecursiveSearch;
FARSTDLOCALISALPHA FarIsAlpha;

struct RegistryStr REGStr={"Add2PlugMenu","Add2DisksMenu","%s%s%s","Separator",
                           "DisksMenuDigit", "ShowCmdOutput", "CatchMode", "ViewZeroFiles" };
struct HELPIDS HlfId={"Contents","Config"};
static struct PluginStartupInfo Info;
struct PanelInfo PInfo;
char PluginRootKey[80];
char selectItem[NM*5];
//char tempFileNameOut[NM*5],tempFileNameErr[NM*5],FileNameOut[NM*5],FileNameErr[NM*5],
char fullcmd[NM*5],cmd[NM*5];

#include "reg.cpp"
#include "mix.cpp"
#include "OpenCmd.cpp"

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1719);
}


void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psInfo)
{
  Info=*psInfo;

  FarItoa=Info.FSF->itoa;
  FarAtoi=Info.FSF->atoi;
  FarSprintf=Info.FSF->sprintf;
  ExpandEnvironmentStr=Info.FSF->ExpandEnvironmentStr;
  Unquote=Info.FSF->Unquote;
  PointToName=Info.FSF->PointToName;
  GetPathRoot=Info.FSF->GetPathRoot;
  AddEndSlash=Info.FSF->AddEndSlash;
  MkTemp=Info.FSF->MkTemp;
  FarRTrim=Info.FSF->RTrim;
  FarLTrim=Info.FSF->LTrim;
  FarTrim=Info.FSF->Trim;
  LStricmp=Info.FSF->LStricmp;
  LStrnicmp=Info.FSF->LStrnicmp;
  CopyToClipboard=Info.FSF->CopyToClipboard;
  MkLink=Info.FSF->MkLink;
  FarNameToKey=Info.FSF->FarNameToKey;
  QuoteSpaceOnly=Info.FSF->QuoteSpaceOnly;
  FarRecursiveSearch=Info.FSF->FarRecursiveSearch;
  FarIsAlpha=Info.FSF->LIsAlpha;

  lstrcpy(PluginRootKey,Info.RootKey);
  lstrcat(PluginRootKey,"\\FARCmds");
  GetRegKey(HKEY_CURRENT_USER,"",REGStr.Separator,Opt.Separator," ",3);
  Opt.Add2PlugMenu=GetRegKey(HKEY_CURRENT_USER,"",REGStr.Add2PlugMenu,0);
  Opt.Add2DisksMenu=GetRegKey(HKEY_CURRENT_USER,"",REGStr.Add2DisksMenu,0);
  Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,"",REGStr.DisksMenuDigit,0);
  Opt.ShowCmdOutput=GetRegKey(HKEY_CURRENT_USER,"",REGStr.ShowCmdOutput,0);
  Opt.CatchMode=GetRegKey(HKEY_CURRENT_USER,"",REGStr.CatchMode,0);
  Opt.ViewZeroFiles=GetRegKey(HKEY_CURRENT_USER,"",REGStr.ViewZeroFiles,1);
}




HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
//  tempFileNameOut[0]=tempFileNameErr[0]=FileNameOut[0]=FileNameErr[0]=
  fullcmd[0]=cmd[0]=selectItem[0]='\0';
  int _GETPANELINFO=FCTL_GETPANELINFO,
      _SETPANELDIR=FCTL_SETPANELDIR,
      _REDRAWPANEL=FCTL_REDRAWPANEL;

  if(OpenFrom==OPEN_COMMANDLINE)
  {
    OpenFromCommandLine((char *)Item);
  }
  else if(OpenFrom == OPEN_PLUGINSMENU && !Item && PInfo.PanelType != PTYPE_FILEPANEL)
    return INVALID_HANDLE_VALUE;
  else
  {
    lstrcpy(selectItem,PInfo.CurDir);

    if(lstrlen(selectItem))
      AddEndSlash(selectItem);

    lstrcat(selectItem, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);

    _GETPANELINFO=FCTL_GETANOTHERPANELINFO,
    _SETPANELDIR=FCTL_SETANOTHERPANELDIR,
    _REDRAWPANEL=FCTL_REDRAWANOTHERPANEL;
  }

  /*установить курсор на объект*/
  if(lstrlen(selectItem))
  {
    static struct PanelRedrawInfo PRI;
    static char Name[NM], Dir[NM*5];
    int pathlen;

    lstrcpy(Name,PointToName(selectItem));
    pathlen=PointToName(selectItem)-selectItem;

    if(pathlen)
      memcpy(Dir,selectItem,pathlen);

    Dir[pathlen]=0;
    FarTrim(Name);
    FarTrim(Dir);
    Unquote(Name);
    Unquote(Dir);

    if(*Dir) Info.Control(INVALID_HANDLE_VALUE,_SETPANELDIR,&Dir);
    Info.Control(INVALID_HANDLE_VALUE,_GETPANELINFO,&PInfo);

    PRI.CurrentItem=PInfo.CurrentItem;
    PRI.TopPanelItem=PInfo.TopPanelItem;

    for(int J=0; J < PInfo.ItemsNumber; J++)
    {
      if(!LStricmp(Name,PointToName(PInfo.PanelItems[J].FindData.cFileName)))
      {
        PRI.CurrentItem=J;
        PRI.TopPanelItem=J;
        break;
      }
    }
    Info.Control(INVALID_HANDLE_VALUE,_REDRAWPANEL,&PRI);
  }
  else
    Info.Control(INVALID_HANDLE_VALUE,_REDRAWPANEL,NULL);
  return(INVALID_HANDLE_VALUE);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_FULLCMDLINE;

  static char *PluginMenuStrings[1],*PluginConfigStrings[1],
              *DiskMenuStrings[1];

  if(Opt.Add2PlugMenu)
  {
    PluginMenuStrings[0]=(char*)GetMsg(MSetPassiveDir);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }
  else
  {
    Info->PluginMenuStringsNumber=0;
    Info->PluginMenuStrings=0;
  }

  if(Opt.Add2DisksMenu)
  {
   DiskMenuStrings[0]=(char*)GetMsg(MSetPassiveDir);
   Info->DiskMenuStrings=DiskMenuStrings;
   Info->DiskMenuStringsNumber=1;
   Info->DiskMenuNumbers=&Opt.DisksMenuDigit;
  }
  else
  {
   Info->DiskMenuStringsNumber=0;
   Info->DiskMenuStrings=0;
  }

  PluginConfigStrings[0]=(char*)GetMsg(MConfig);
  Info->PluginConfigStrings=PluginConfigStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginConfigStrings)/sizeof(PluginConfigStrings[0]);

  Info->CommandPrefix="far:view:edit:goto:clip:whereis:macro:ln:run";
}

int WINAPI _export Configure( int /*ItemNumber*/ )
{
  struct InitDialogItem InitItems[]=
  { //   Type           X1 Y1 X2 Y2 Fo Se Fl               DB Data
/*00*/ { DI_DOUBLEBOX,   3, 1,69,19, 0, 0, DIF_BOXCOLOR,    0, (char *)MConfig},
/*01*/ { DI_CHECKBOX,    5, 2, 0, 0, 1, 0, 0,               0, (char *)MAddSetPassiveDir2PlugMenu},
/*02*/ { DI_TEXT,        0, 3, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,""},
/*03*/ { DI_CHECKBOX,    5, 4, 0, 0, 0, 0, 0,               0, (char *)MAddToDisksMenu},
/*04*/ { DI_FIXEDIT,     7, 5, 7, 0, 0, 0, DIF_BOXCOLOR|DIF_MASKEDIT,    0, " "},
/*05*/ { DI_TEXT,        9, 5, 0, 0, 0, 0, 0,               0, (char *)MDisksMenuDigit},
/*06*/ { DI_TEXT,        0, 6, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,""},
/*07*/ { DI_RADIOBUTTON, 5, 7, 0, 0, 0, 0, DIF_GROUP,       0, (char *)MHideCmdOutput},
/*08*/ { DI_RADIOBUTTON, 5, 8, 0, 0, 0, 0, 0,               0, (char *)MKeepCmdOutput},
/*09*/ { DI_RADIOBUTTON, 5, 9, 0, 0, 0, 0, 0,               0, (char *)MEchoCmdOutput},
/*10*/ { DI_TEXT,        0,10, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,""},
/*11*/ { DI_RADIOBUTTON, 5,11, 0, 0, 0, 0, DIF_GROUP,       0, (char *)MCatchAllInOne},
/*12*/ { DI_RADIOBUTTON, 5,12, 0, 0, 0, 0, 0,               0, (char *)MCatchStdOutput},
/*13*/ { DI_RADIOBUTTON, 5,13, 0, 0, 0, 0, 0,               0, (char *)MCatchStdError},
/*14*/ { DI_RADIOBUTTON, 5,14, 0, 0, 0, 0, 0,               0, (char *)MCatchSeparate},
/*15*/ { DI_TEXT,        0,15, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,""},
/*16*/ { DI_CHECKBOX,    5,16, 0, 0, 0, 0, 0,               0, (char *)MViewZeroFiles},
/*17*/ { DI_TEXT,        0,17, 0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,""},
/*18*/ { DI_BUTTON,      0,18, 0, 0, 0, 0, DIF_CENTERGROUP, 1, (char *)MOk},
/*19*/ { DI_BUTTON,      0,18, 0, 0, 0, 0, DIF_CENTERGROUP, 0, (char *)MCancel},
  };
  BOOL ret=FALSE;
  struct FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0]))];
  InitDialogItems(InitItems,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));

  DialogItems[4].Mask="9";
  DialogItems[1].Selected=Opt.Add2PlugMenu;
  DialogItems[3].Selected=Opt.Add2DisksMenu;
  DialogItems[7+Opt.ShowCmdOutput].Selected = 1;
  DialogItems[11+Opt.CatchMode].Selected = 1;
  DialogItems[16].Selected = Opt.ViewZeroFiles;
  FarItoa(Opt.DisksMenuDigit,DialogItems[4].Data,10);

  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,73,21,HlfId.Config,
                       DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));
  if(18 == ExitCode)
  {
    Opt.Add2PlugMenu=DialogItems[1].Selected;
    Opt.Add2DisksMenu=DialogItems[3].Selected;
    Opt.DisksMenuDigit=FarAtoi(DialogItems[4].Data);

    Opt.ViewZeroFiles = DialogItems[16].Selected;
    Opt.CatchMode = Opt.ShowCmdOutput = 0;
    for ( int i = 0 ; i < 3 ; i++ )
      if ( DialogItems[7+i].Selected )
      {
        Opt.ShowCmdOutput = i;
        break;
      }
    for ( int j = 0 ; j < 4 ; j++ )
      if ( DialogItems[11+j].Selected )
      {
        Opt.CatchMode = j;
        break;
      }

    SetRegKey(HKEY_CURRENT_USER,"",REGStr.Add2PlugMenu,Opt.Add2PlugMenu);
    SetRegKey(HKEY_CURRENT_USER,"",REGStr.Add2DisksMenu,Opt.Add2DisksMenu);
    SetRegKey(HKEY_CURRENT_USER,"",REGStr.DisksMenuDigit,Opt.DisksMenuDigit);
    SetRegKey(HKEY_CURRENT_USER,"",REGStr.ShowCmdOutput,Opt.ShowCmdOutput);
    SetRegKey(HKEY_CURRENT_USER,"",REGStr.CatchMode,Opt.CatchMode);
    SetRegKey(HKEY_CURRENT_USER,"",REGStr.ViewZeroFiles,Opt.ViewZeroFiles);
    ret=TRUE;
  }
  return ret;
}
