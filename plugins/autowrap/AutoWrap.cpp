#include "AutoWrap.hpp"
#include "WrapReg.hpp"
#include "WrapLng.hpp"
#include "DlgBuilder.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

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

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
wchar_t *PluginRootKey;

struct Options
{
  wchar_t FileMasks[512];
  wchar_t ExcludeFileMasks[512];
  int RightMargin;
  int Wrap;
} Opt;

const wchar_t *GetCommaWord(const wchar_t *Src, wchar_t *Word);

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid, MsgId);
}

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

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  FSF=*Info->FSF;
  ::Info.FSF=&FSF;
  PluginRootKey = (wchar_t *)malloc(lstrlen(Info->RootKey)*sizeof(wchar_t) + sizeof(L"\\AutoWrap"));
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,L"\\AutoWrap");

  Opt.Wrap=GetRegKey(L"",L"Wrap",0);
  Opt.RightMargin=GetRegKey(L"",L"RightMargin",75);
  GetRegKey(L"",L"FileMasks",Opt.FileMasks,L"*.*",ARRAYSIZE(Opt.FileMasks));
  GetRegKey(L"",L"ExcludeFileMasks",Opt.ExcludeFileMasks,L"",ARRAYSIZE(Opt.ExcludeFileMasks));
}

void WINAPI ExitFARW()
{
  free(PluginRootKey);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAutoWrap);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

HANDLE WINAPI OpenPluginW(int OpenFrom,const GUID* Guid,INT_PTR Item)
{
  PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MAutoWrap, nullptr);
  Builder.AddCheckbox(MEnableWrap, &Opt.Wrap);
  FarDialogItem *RightMargin = Builder.AddIntEditField(&Opt.RightMargin, 3);
  Builder.AddTextAfter(RightMargin, MRightMargin);
  Builder.AddSeparator();
  Builder.AddText(MFileMasks);
  Builder.AddEditField(Opt.FileMasks, ARRAYSIZE(Opt.FileMasks), 65);
  Builder.AddText(MExcludeFileMasks);
  Builder.AddEditField(Opt.ExcludeFileMasks, ARRAYSIZE(Opt.ExcludeFileMasks), 65);
  Builder.AddOKCancel(MOk, MCancel);
  if (Builder.ShowDialog())
  {
    SetRegKey(L"",L"Wrap",Opt.Wrap);
    SetRegKey(L"",L"RightMargin",Opt.RightMargin);
    SetRegKey(L"",L"FileMasks",Opt.FileMasks);
    SetRegKey(L"",L"ExcludeFileMasks",Opt.ExcludeFileMasks);
  }
  return INVALID_HANDLE_VALUE;
}


int WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec)
{
  if (!Opt.Wrap)
    return FALSE;

  static int Reenter=FALSE;

  if (Reenter || Rec->EventType!=KEY_EVENT || !Rec->Event.KeyEvent.bKeyDown || Rec->Event.KeyEvent.wVirtualKeyCode==VK_F1)
    return FALSE;

  struct EditorInfo startei;
  Info.EditorControl(0,ECTL_GETINFO,0,(INT_PTR)&startei);

  struct EditorGetString prevegs;
  prevegs.StringNumber=-1;
  Info.EditorControl(0,ECTL_GETSTRING,0,(INT_PTR)&prevegs);

  Reenter=TRUE;
  Info.EditorControl(0,ECTL_PROCESSINPUT,0,(INT_PTR)Rec);
  Reenter=FALSE;

  for (int Pass=1;;Pass++)
  {
    EditorInfo ei;
    Info.EditorControl(0,ECTL_GETINFO,0,(INT_PTR)&ei);
    LPWSTR FileName=nullptr;
    size_t FileNameSize=Info.EditorControl(0,ECTL_GETFILENAME,0,0);
    if(FileNameSize)
    {
      FileName=new wchar_t[FileNameSize];
      if(FileName)
      {
        Info.EditorControl(0,ECTL_GETFILENAME,0,(INT_PTR)FileName);
      }
    }
    if (Pass==1 && *Opt.FileMasks)
    {
      if (ei.CurLine!=startei.CurLine)
      {
        if(FileName)
          delete[] FileName;
        return TRUE;
      }
      bool Found=false;
      wchar_t FileMask[MAX_PATH];
      const wchar_t *MaskPtr=Opt.FileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=nullptr)
      {
        if (FSF.ProcessName(FileMask,FileName,0,PN_CMPNAME|PN_SKIPPATH))
        {
          Found=true;
          break;
        }
      }
      if (!Found)
      {
        if(FileName)
          delete[] FileName;
        return TRUE;
      }
      MaskPtr=Opt.ExcludeFileMasks;
      while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=nullptr)
      {
        if (FSF.ProcessName(FileMask,FileName,0,PN_CMPNAME|PN_SKIPPATH))
        {
          Found=false;
          break;
        }
      }
      if (!Found)
      {
        if(FileName)
          delete[] FileName;
        return TRUE;
      }
    }
    if(FileName)
      delete[] FileName;

    struct EditorGetString egs;
    egs.StringNumber=ei.CurLine;
    Info.EditorControl(0,ECTL_GETSTRING,0,(INT_PTR)&egs);

    bool TabPresent=wmemchr(egs.StringText,L'\t',egs.StringLength)!=nullptr;
    int TabLength=egs.StringLength;
    if (TabPresent)
    {
      struct EditorConvertPos ecp;
      ecp.StringNumber=-1;
      ecp.SrcPos=egs.StringLength;
      Info.EditorControl(0,ECTL_REALTOTAB,0,(INT_PTR)&ecp);
      TabLength=ecp.DestPos;
    }

    if ((Pass!=1 || prevegs.StringLength!=egs.StringLength) &&
        TabLength>=Opt.RightMargin && ei.CurPos>=egs.StringLength)
    {
      int SpacePos=-1;
      for (int I=egs.StringLength-1;I>0;I--)
      {
        if (egs.StringText[I]==L' ' || egs.StringText[I]==L'\t')
        {
          SpacePos=I;
          int TabPos=I;
          if (TabPresent)
          {
            struct EditorConvertPos ecp;
            ecp.StringNumber=-1;
            ecp.SrcPos=I;
            Info.EditorControl(0,ECTL_REALTOTAB,0,(INT_PTR)&ecp);
            TabPos=ecp.DestPos;
          }
          if (TabPos<Opt.RightMargin)
            break;
        }
      }

      if (SpacePos<=0)
        break;

      bool SpaceOnly=true;
      for (int I=0;I<SpacePos;I++)
      {
        if (egs.StringText[I]!=L' ' && egs.StringText[I]!=L'\t')
        {
          SpaceOnly=false;
          break;
        }
      }

      if (SpaceOnly)
        break;

      struct EditorSetPosition esp;
      memset(&esp,-1,sizeof(esp));
      esp.CurPos=SpacePos+1;
      Info.EditorControl(0,ECTL_SETPOSITION,0,(INT_PTR)&esp);
      int Indent=TRUE;
      if (!Info.EditorControl(0,ECTL_INSERTSTRING,0,(INT_PTR)&Indent))
        break;
      if (ei.CurPos<SpacePos)
      {
        esp.CurLine=ei.CurLine;
        esp.CurPos=ei.CurPos;
        Info.EditorControl(0,ECTL_SETPOSITION,0,(INT_PTR)&esp);
      }
      else
      {
        egs.StringNumber=ei.CurLine+1;
        Info.EditorControl(0,ECTL_GETSTRING,0,(INT_PTR)&egs);
        esp.CurLine=ei.CurLine+1;
        esp.CurPos=egs.StringLength;
        Info.EditorControl(0,ECTL_SETPOSITION,0,(INT_PTR)&esp);
      }
      Info.EditorControl(0,ECTL_REDRAW,0,0);
    }
    else
      break;
  }
  return TRUE;
}

const wchar_t *GetCommaWord(const wchar_t *Src, wchar_t *Word)
{
  if (*Src==L'\0')
    return nullptr;

  int WordPos=0;
  bool SkipBrackets=false;

  for (; *Src!=L'\0'; Src++,WordPos++)
  {
    if (*Src==L'[' && wcschr(Src+1,L']')!=nullptr)
      SkipBrackets=true;
    if (*Src==L']')
      SkipBrackets=false;
    if (*Src==L',' && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (iswspace(*Src))
        Src++;
      return Src;
    }
    else
    {
      Word[WordPos]=*Src;
    }
  }
  Word[WordPos]=0;

  return Src;
}
