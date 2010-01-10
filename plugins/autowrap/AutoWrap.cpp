#include "plugin.hpp"
#include "WrapLng.hpp"
#include "AutoWrap.hpp"
#include "CRT/crt.hpp"

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

#include "WrapReg.cpp"
#include "WrapMix.cpp"

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetDataPtr(i) DialogItems[i].Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  FSF=*Info->FSF;
  ::Info.FSF=&FSF;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\AutoWrap"));
  Opt.Wrap=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("Wrap"),0);
  Opt.RightMargin=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("RightMargin"),75);
  GetRegKey(HKEY_CURRENT_USER,_T(""),_T("FileMasks"),Opt.FileMasks,_T("*.*"),ArraySize(Opt.FileMasks));
  GetRegKey(HKEY_CURRENT_USER,_T(""),_T("ExcludeFileMasks"),Opt.ExcludeFileMasks,_T(""),ArraySize(Opt.ExcludeFileMasks));
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  struct InitDialogItem InitItems[]={
    {DI_DOUBLEBOX,3,1,72,11,0,0,0,0,(TCHAR *)MAutoWrap},
    {DI_CHECKBOX,5,2,0,0,1,0,0,0,(TCHAR *)MEnableWrap},
    {DI_EDIT,5,3,7,3,0,0,0,0,_T("")},
    {DI_TEXT,9,3,0,0,0,0,0,0,(TCHAR *)MRightMargin},
    {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
    {DI_TEXT,5,5,0,0,0,0,0,0,(TCHAR *)MFileMasks},
    {DI_EDIT,5,6,70,6,0,0,0,0,_T("")},
    {DI_TEXT,5,7,0,0,0,0,0,0,(TCHAR *)MExcludeFileMasks},
    {DI_EDIT,5,8,70,6,0,0,0,0,_T("")},
    {DI_TEXT,5,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
    {DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
    {DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };

  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
  DialogItems[1].Selected=Opt.Wrap;
#ifndef UNICODE
#define SET_DLGITEM(n,v)  lstrcpy(DialogItems[n].Data, v)
#else
#define SET_DLGITEM(n,v)  DialogItems[n].PtrData = v
#endif
  SET_DLGITEM(6,Opt.FileMasks);
  SET_DLGITEM(8,Opt.ExcludeFileMasks);
#ifdef UNICODE
  wchar_t marstr[32];
  DialogItems[2].PtrData = marstr;
  FSF.sprintf(marstr,L"%d",Opt.RightMargin);
#else
  FSF.sprintf(DialogItems[2].Data,"%d",Opt.RightMargin);
#endif
#ifndef UNICODE
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,13,NULL,DialogItems,
                           ArraySize(DialogItems));
#else
  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,76,13,NULL,DialogItems,
                                ArraySize(DialogItems),0,0,NULL,0);
  if (hDlg == INVALID_HANDLE_VALUE)
    return INVALID_HANDLE_VALUE;

  int ExitCode=Info.DialogRun(hDlg);
#endif
  if (ExitCode==10)
  {
    Opt.Wrap=GetCheck(1);
    Opt.RightMargin=FSF.atoi(GetDataPtr(2));
    lstrcpy(Opt.FileMasks,GetDataPtr(6));
    lstrcpy(Opt.ExcludeFileMasks,GetDataPtr(8));
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("Wrap"),Opt.Wrap);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("RightMargin"),Opt.RightMargin);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("FileMasks"),Opt.FileMasks);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("ExcludeFileMasks"),Opt.ExcludeFileMasks);
  }
#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif
  return(INVALID_HANDLE_VALUE);
}


int WINAPI EXP_NAME(ProcessEditorInput)(const INPUT_RECORD *Rec)
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
    EditorInfo ei;
    Info.EditorControl(ECTL_GETINFO,&ei);
#ifdef UNICODE
    LPWSTR FileName=NULL;
    size_t FileNameSize=Info.EditorControl(ECTL_GETFILENAME,NULL);
    if(FileNameSize)
    {
      FileName=new wchar_t[FileNameSize];
      if(FileName)
      {
        Info.EditorControl(ECTL_GETFILENAME,FileName);
      }
    }
#endif
    if (Pass==1 && *Opt.FileMasks)
    {
      if (ei.CurLine!=startei.CurLine)
      {
#ifdef UNICODE
        if(FileName)
        {
          delete[] FileName;
        }
#endif
        return TRUE;
      }
      int Found=FALSE;
      TCHAR FileMask[MAX_PATH],*MaskPtr=Opt.FileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
        if (Info.CmpName(FileMask,
#ifndef UNICODE
                                  ei.
#endif
                                     FileName,TRUE))
        {
          Found=TRUE;
          break;
        }
      if (!Found)
      {
#ifdef UNICODE
        if(FileName)
        {
          delete[] FileName;
        }
#endif
        return TRUE;
      }
      MaskPtr=Opt.ExcludeFileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
        if (Info.CmpName(FileMask,
#ifndef UNICODE
                                  ei.
#endif
                                     FileName,TRUE))
        {
          Found=FALSE;
          break;
        }
      if (!Found)
      {
#ifdef UNICODE
        if(FileName)
        {
          delete[] FileName;
        }
#endif
        return TRUE;
      }
    }
#ifdef UNICODE
    if(FileName)
    {
      delete[] FileName;
    }
#endif

    struct EditorGetString egs;
    egs.StringNumber=ei.CurLine;
    Info.EditorControl(ECTL_GETSTRING,&egs);

    int TabPresent=_tmemchr(egs.StringText,_T('\t'),egs.StringLength)!=NULL;
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
        if (egs.StringText[I]==_T(' ') || egs.StringText[I]==_T('\t'))
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
        if (egs.StringText[I]!=_T(' ') && egs.StringText[I]!=_T('\t'))
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


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static const TCHAR *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAutoWrap);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);
  Info->PluginConfigStringsNumber=0;
}
