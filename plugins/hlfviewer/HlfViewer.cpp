#define STRICT
#define _FAR_USE_FARFINDDATA
#include <CRT/crt.hpp>
#include <plugin.hpp>
#include <farkeys.hpp>
#include "HlfViewer.hpp"
#include "lang.hpp"

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

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetDataPtr(i) DialogItems[i].Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

#include "reg.cpp"
#include "mix.cpp"
#include "FarEditor.cpp"

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  IsOldFar=TRUE;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\HlfViewer"));
  if(Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    IsOldFar=FALSE;
    PointToName=::FSF.PointToName;
    GetPathRoot=::FSF.GetPathRoot;
    AddEndSlash=::FSF.AddEndSlash;
    FarSprintf=::FSF.sprintf;
    GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditorKey,KeyNameFromReg,_T("F1"),ArraySize(KeyNameFromReg)-1);
    Opt.Key=::FSF.FarNameToKey(KeyNameFromReg);
    Opt.ProcessEditorInput=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ProcessEditorInput,1);
    Opt.Style=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Style,0);
  }
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  if(IsOldFar)
    return (INVALID_HANDLE_VALUE);

  if(OpenFrom==OPEN_COMMANDLINE)
  {
    if(lstrlen((TCHAR *)Item))
    {
      static TCHAR cmdbuf[1024], FileName[NM], *ptrTopic, *ptrName;

      lstrcpy(cmdbuf,(TCHAR *)Item);
      FSF.Trim(cmdbuf);
      ptrName=ptrTopic=cmdbuf;
      if(*cmdbuf==_T('\"'))
      {
        ptrName++;
        ptrTopic++;
        while(*ptrTopic!=_T('\"') && *ptrTopic)
          ptrTopic++;
      }
      else
      {
        while(*ptrTopic!=_T(' ') && *ptrTopic)
          ptrTopic++;
      }

      int hasTopic = (*ptrTopic == _T(' '));

      *ptrTopic=0;
      GetFullPathName(ptrName,NM,FileName,&ptrName);

      if (hasTopic)
      {
        ptrTopic++;
        if(lstrlen(ptrTopic))
          FSF.Trim(ptrTopic);
        else
          ptrTopic=NULL;
      }
      else
        ptrTopic = NULL;

      ShowHelp(FileName,ptrTopic,true);
    }
    else
      Info.ShowHelp(Info.ModuleName,HlfId.cmd,FHELP_SELFHELP);
  }
  else if(OpenFrom == OPEN_EDITOR)
  {
    if (IsHlf())
      ShowCurrentHelpTopic();
    else
    {
      const TCHAR *Items[] = { GetMsg (MTitle), GetMsg (MNotAnHLF), GetMsg (MOk) };
      Info.Message (Info.ModuleNumber, 0,
        NULL, Items, 3, 1);
    }
  }

  return(INVALID_HANDLE_VALUE);
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  if(IsOldFar)
    return;

  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;

  static const TCHAR * PluginMenuStrings[1], *PluginConfigStrings[1];

  PluginConfigStrings[0]=GetMsg(MTitle);
  Info->PluginConfigStrings=PluginConfigStrings;
  Info->PluginConfigStringsNumber=ArraySize(PluginConfigStrings);

  if(Opt.ProcessEditorInput)
  {
    Info->PluginMenuStrings=0;
    Info->PluginMenuStringsNumber=0;
  }
  else
  {
    PluginMenuStrings[0]=GetMsg(MShowHelpTopic);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);
  }

  Info->CommandPrefix=_T("HLF");
}

int WINAPI EXP_NAME(ProcessEditorInput)(const INPUT_RECORD *Rec)
{
  BOOL Result=FALSE;
  if(!IsOldFar && Opt.ProcessEditorInput)
  {
    if(Rec->EventType==KEY_EVENT && Rec->Event.KeyEvent.bKeyDown &&
       FSF.FarInputRecordToKey((INPUT_RECORD *)Rec)==Opt.Key)
    {
#ifdef UNICODE
      ei.FileNameSize=0;
#endif
      Info.EditorControl(ECTL_GETINFO,&ei);
#ifdef UNICODE
      if(ei.FileNameSize)
      {
        ei.FileName=new wchar_t[ei.FileNameSize];
        if(ei.FileName)
        {
          Info.EditorControl(ECTL_GETINFO,&ei);
        }
      }
#endif
      if(CheckExtension(ei.FileName))
      {
        ShowCurrentHelpTopic();
        Result=TRUE;
      }
    }
  }
#ifdef UNICODE
  if(ei.FileName)
  {
    delete[] ei.FileName;
  }
#endif
  return Result;
}

void ShowCurrentHelpTopic()
{
#ifdef UNICODE
  ei.FileNameSize=0;
#endif
  Info.EditorControl(ECTL_GETINFO,&ei);
#ifdef UNICODE
  if(ei.FileNameSize)
  {
    ei.FileName=new wchar_t[ei.FileNameSize];
    if(ei.FileName)
    {
      Info.EditorControl(ECTL_GETINFO,&ei);
    }
  }
#endif
  switch(Opt.Style)
  {
    case 1:
      if(!(ei.CurState&ECSTATE_SAVED))
        ShowHelpFromTempFile();
      else
        ShowHelp(ei.FileName,FindTopic());
      break;
    case 2:
      if(!(ei.CurState&ECSTATE_SAVED))
        Info.EditorControl(ECTL_SAVEFILE, NULL);
    default:
      ShowHelp(ei.FileName,FindTopic());
      break;
  }
#ifdef UNICODE
  if(ei.FileName)
  {
    delete[] ei.FileName;
  }
#endif
}

void ShowHelpFromTempFile()
{
  struct EditorSaveFile esf;
#ifdef UNICODE
  wchar_t fname[MAX_PATH];
  esf.FileName = fname;
  if(FSF.MkTemp(fname, ArraySize(fname)-4,_T("HLF")))
#else
  if(FSF.MkTemp(esf.FileName,_T("HLF")))
#endif
  {
    lstrcat((TCHAR*)esf.FileName,_T(".hlf"));
  /*
    esf.FileEOL=NULL;
    Info.EditorControl(ECTL_SAVEFILE, &esf);
    ShowHelp(esf.FileName, FindTopic());
    DeleteFile(esf.FileName);
  */
    struct EditorGetString egs;
    struct EditorInfo ei;
    DWORD Count;

    HANDLE Handle=CreateFile(esf.FileName, GENERIC_WRITE, FILE_SHARE_READ,
                             NULL, CREATE_NEW, 0, NULL);
    if(Handle != INVALID_HANDLE_VALUE)
    {
#ifdef UNICODE
      ei.FileNameSize=0;
#endif
      Info.EditorControl(ECTL_GETINFO,&ei);
      for(egs.StringNumber=0; egs.StringNumber<ei.TotalLines; egs.StringNumber++)
      {
        Info.EditorControl(ECTL_GETSTRING, &egs);
        WriteFile(Handle, egs.StringText, egs.StringLength, &Count, NULL);
        WriteFile(Handle, _T("\r\n"), 2*sizeof(TCHAR), &Count, NULL);
      }
      CloseHandle(Handle);
      ShowHelp(esf.FileName, FindTopic());
      DeleteFile(esf.FileName);
    }
  }
  else
    ; //??
}

#if 0
int WINAPI EXP_NAME(ProcessEditorEvent)(int Event, void * /*Param*/)
{
  if(Event==EE_READ || Event==EE_SAVE)
  {
    GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditorKey,KeyNameFromReg,_T("F1"),ArraySize(KeyNameFromReg)-1);
    Opt.Key=FSF.FarNameToKey(KeyNameFromReg);
    Opt.ProcessEditorInput=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ProcessEditorInput,1);
    Opt.Style=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Style,0);
  }
  return 0;
}
#endif

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  if(IsOldFar)
    return FALSE;

  GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditorKey,KeyNameFromReg,_T("F1"),ArraySize(KeyNameFromReg)-1);

  static struct InitDialogItem InitItems[]=
  {
  /*00*/{DI_DOUBLEBOX,3,1,70,10,0,0,0,0,(TCHAR *)MTitle},
  /*01*/{DI_CHECKBOX,5,2,0,0,TRUE,0,0,0,(TCHAR *)MProcessEditorInput},
  /*02*/{DI_FIXEDIT,10,2,68,2,0,0,0,0,KeyNameFromReg},
  /*03*/{DI_TEXT,0,3,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /*04*/{DI_TEXT,5,4,0,0,0,0,0,0,(TCHAR *)MStyle},
  /*05*/{DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(TCHAR *)MStr1},
  /*06*/{DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(TCHAR *)MStr2},
  /*07*/{DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(TCHAR *)MStr3},
  /*08*/{DI_TEXT,0,8,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /*09*/{DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
  /*10*/{DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };
  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
  int ret=FALSE;

  DialogItems[1].Selected=Opt.ProcessEditorInput=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ProcessEditorInput,1);
  DialogItems[2].X1+=lstrlen(GetMsg(MProcessEditorInput));
  Opt.Style=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Style,0);
  DialogItems[5].Selected=DialogItems[6].Selected=DialogItems[7].Selected=0;
  DialogItems[5+(Opt.Style>2?0:Opt.Style)].Selected=1;

#ifndef UNICODE
  int ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,74,12, HlfId.Config,
                             DialogItems,ArraySize(InitItems));
#else
  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,74,12, HlfId.Config,
                            DialogItems,ArraySize(InitItems),0,0,NULL,0);

  if (hDlg == INVALID_HANDLE_VALUE)
    return ret;

  int ExitCode = Info.DialogRun(hDlg);
#endif

  if(ExitCode==9)
  {
    Opt.ProcessEditorInput=GetCheck(1);
    if((Opt.Key=FSF.FarNameToKey(GetDataPtr(2)))==-1) Opt.Key=KEY_F1;
    FSF.FarKeyToName(Opt.Key,KeyNameFromReg,ArraySize(KeyNameFromReg)-1);
    Opt.Style=(GetCheck(6)!=0)+((GetCheck(7)!=0)<<1);

    SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ProcessEditorInput,Opt.ProcessEditorInput);
    SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditorKey,KeyNameFromReg);
    SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Style,Opt.Style);
    ret=TRUE;
  }

#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif

  return ret;
}
