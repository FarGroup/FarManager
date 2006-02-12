#include "plugin.hpp"
#include "WrapLng.hpp"
#include "AutoWrap.hpp"

#if defined(__GNUC__)

#include "crt.hpp"

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

#include "wrapreg.cpp"
#include "wrapmix.cpp"

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  FSF=*Info->FSF;
  ::Info.FSF=&FSF;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,"\\AutoWrap");
  Opt.Wrap=GetRegKey(HKEY_CURRENT_USER,"","Wrap",0);
  Opt.RightMargin=GetRegKey(HKEY_CURRENT_USER,"","RightMargin",75);
  GetRegKey(HKEY_CURRENT_USER,"","FileMasks",Opt.FileMasks,"*.*",sizeof(Opt.FileMasks));
  GetRegKey(HKEY_CURRENT_USER,"","ExcludeFileMasks",Opt.ExcludeFileMasks,"",sizeof(Opt.ExcludeFileMasks));
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,11,0,0,0,0,(char *)MAutoWrap,
    DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MEnableWrap,
    DI_EDIT,5,3,7,3,0,0,0,0,"",
    DI_TEXT,9,3,0,0,0,0,0,0,(char *)MRightMargin,
    DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,5,0,0,0,0,0,0,(char *)MFileMasks,
    DI_EDIT,5,6,70,6,0,0,0,0,"",
    DI_TEXT,5,7,0,0,0,0,0,0,(char *)MExcludeFileMasks,
    DI_EDIT,5,8,70,6,0,0,0,0,"",
    DI_TEXT,5,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=Opt.Wrap;
  lstrcpy(DialogItems[6].Data,Opt.FileMasks);
  lstrcpy(DialogItems[8].Data,Opt.ExcludeFileMasks);
  wsprintf(DialogItems[2].Data,"%d",Opt.RightMargin);
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,13,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode==10)
  {
    Opt.Wrap=DialogItems[1].Selected;
    Opt.RightMargin=FSF.atoi(DialogItems[2].Data);
    lstrcpy(Opt.FileMasks,DialogItems[6].Data);
    lstrcpy(Opt.ExcludeFileMasks,DialogItems[8].Data);
    SetRegKey(HKEY_CURRENT_USER,"","Wrap",Opt.Wrap);
    SetRegKey(HKEY_CURRENT_USER,"","RightMargin",Opt.RightMargin);
    SetRegKey(HKEY_CURRENT_USER,"","FileMasks",Opt.FileMasks);
    SetRegKey(HKEY_CURRENT_USER,"","ExcludeFileMasks",Opt.ExcludeFileMasks);
  }
  return(INVALID_HANDLE_VALUE);
}


int WINAPI _export ProcessEditorInput(const INPUT_RECORD *Rec)
{
  if (!Opt.Wrap)
    return(FALSE);

  static int Reenter=FALSE;

  if (Reenter || Rec->EventType!=KEY_EVENT || !Rec->Event.KeyEvent.bKeyDown || Rec->Event.KeyEvent.wVirtualKeyCode==VK_F1)
    return(FALSE);

  struct EditorInfo startei;
  Info.EditorControl(ECTL_GETINFO,&startei);

  struct EditorGetString prevegs;
  prevegs.StringNumber=-1;
  Info.EditorControl(ECTL_GETSTRING,&prevegs);

  Reenter=TRUE;
  Info.EditorControl(ECTL_PROCESSINPUT,(void*)Rec);
  Reenter=FALSE;

  for (int Pass=1;;Pass++)
  {
    struct EditorInfo ei;
    Info.EditorControl(ECTL_GETINFO,&ei);

    if (Pass==1 && *Opt.FileMasks)
    {
      if (ei.CurLine!=startei.CurLine)
        return(TRUE);
      int Found=FALSE;
      char FileMask[NM],*MaskPtr=Opt.FileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
        if (Info.CmpName(FileMask,ei.FileName,TRUE))
        {
          Found=TRUE;
          break;
        }
      if (!Found)
        return(TRUE);
      MaskPtr=Opt.ExcludeFileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
        if (Info.CmpName(FileMask,ei.FileName,TRUE))
        {
          Found=FALSE;
          break;
        }
      if (!Found)
        return(TRUE);
    }

    struct EditorGetString egs;
    egs.StringNumber=ei.CurLine;
    Info.EditorControl(ECTL_GETSTRING,&egs);

    int TabPresent=memchr(egs.StringText,'\t',egs.StringLength)!=NULL;
    int TabLength=egs.StringLength;
    if (TabPresent)
    {
      struct EditorConvertPos ecp;
      ecp.StringNumber=-1;
      ecp.SrcPos=egs.StringLength;
      Info.EditorControl(ECTL_REALTOTAB,&ecp);
      TabLength=ecp.DestPos;
    }

    if ((Pass!=1 || prevegs.StringLength!=egs.StringLength) &&
        TabLength>=Opt.RightMargin && ei.CurPos>=egs.StringLength)
    {
      int SpacePos=-1;
      int I;
      for (I=egs.StringLength-1;I>0;I--)
        if (egs.StringText[I]==' ' || egs.StringText[I]=='\t')
        {
          SpacePos=I;
          int TabPos=I;
          if (TabPresent)
          {
            struct EditorConvertPos ecp;
            ecp.StringNumber=-1;
            ecp.SrcPos=I;
            Info.EditorControl(ECTL_REALTOTAB,&ecp);
            TabPos=ecp.DestPos;
          }
          if (TabPos<Opt.RightMargin)
            break;
        }

      if (SpacePos<=0)
        break;

      int SpaceOnly=TRUE;
      for (I=0;I<SpacePos;I++)
        if (egs.StringText[I]!=' ' && egs.StringText[I]!='\t')
        {
          SpaceOnly=FALSE;
          break;
        }

      if (SpaceOnly)
        break;

      struct EditorSetPosition esp;
      memset(&esp,-1,sizeof(esp));
      esp.CurPos=SpacePos+1;
      Info.EditorControl(ECTL_SETPOSITION,&esp);
      int Indent=TRUE;
      if (!Info.EditorControl(ECTL_INSERTSTRING,&Indent))
        break;
      if (ei.CurPos<SpacePos)
      {
        esp.CurLine=ei.CurLine;
        esp.CurPos=ei.CurPos;
        Info.EditorControl(ECTL_SETPOSITION,&esp);
      }
      else
      {
        egs.StringNumber=ei.CurLine+1;
        Info.EditorControl(ECTL_GETSTRING,&egs);
        esp.CurLine=ei.CurLine+1;
        esp.CurPos=egs.StringLength;
        Info.EditorControl(ECTL_SETPOSITION,&esp);
      }
      Info.EditorControl(ECTL_REDRAW,NULL);
    }
    else
      break;
  }
  return(TRUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAutoWrap);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}
