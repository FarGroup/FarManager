#include "plugin.hpp"
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


#include "AlignLng.hpp"
#include "Align.hpp"
#include "AlignReg.cpp"
#include "AlignMix.cpp"

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetDataPtr(i) DialogItems[i].Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif


static void ReformatBlock(int RightMargin,int SmartMode,int Justify);
static void JustifyBlock(int RightMargin);
static int JustifyString(int RightMargin,struct EditorSetString &ess);

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}


void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\Align"));
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  struct InitDialogItem InitItems[]={
    {DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(TCHAR *)MAlign},
    {DI_FIXEDIT,5,2,7,3,1,0,0,0,_T("")},
    {DI_TEXT,9,2,0,0,0,0,0,0,(TCHAR *)MRightMargin},
    {DI_CHECKBOX,5,3,0,0,0,0,0,0,(TCHAR *)MReformat},
    {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MSmartMode},
    {DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)MJustify},
    {DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };

  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
  int RightMargin=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("RightMargin"),75);
  int Reformat=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("Reformat"),TRUE);
  int SmartMode=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("SmartMode"),FALSE);
  int Justify=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("Justify"),FALSE);
#ifdef UNICODE
  wchar_t marstr[32];
  DialogItems[1].PtrData = marstr;
  FSF.sprintf(marstr,L"%d",RightMargin);
#else
  FSF.sprintf(DialogItems[1].Data,"%d",RightMargin);
#endif
  DialogItems[3].Selected=Reformat;
  DialogItems[4].Selected=SmartMode;
  DialogItems[5].Selected=Justify;
#ifndef UNICODE
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,NULL,DialogItems,
                           ArraySize(DialogItems));
#else
  HANDLE hDlg=Info.DialogInit(Info.ModuleNumber,-1,-1,76,10,NULL,DialogItems,
                              ArraySize(DialogItems),0,0,NULL,0);
  if (hDlg == INVALID_HANDLE_VALUE)
    return INVALID_HANDLE_VALUE;

  int ExitCode=Info.DialogRun(hDlg);
#endif
  if (ExitCode!=7)
    goto done;
  RightMargin=FSF.atoi(GetDataPtr(1));
  Reformat=GetCheck(3);
  SmartMode=GetCheck(4);
  Justify=GetCheck(5);
  SetRegKey(HKEY_CURRENT_USER,_T(""),_T("Reformat"),Reformat);
  SetRegKey(HKEY_CURRENT_USER,_T(""),_T("RightMargin"),RightMargin);
  SetRegKey(HKEY_CURRENT_USER,_T(""),_T("SmartMode"),SmartMode);
  SetRegKey(HKEY_CURRENT_USER,_T(""),_T("Justify"),Justify);
  if (Reformat)
    ReformatBlock(RightMargin,SmartMode,Justify);
  else
    if (Justify)
      JustifyBlock(RightMargin);
done:
#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif
  return(INVALID_HANDLE_VALUE);
}


void ReformatBlock(int RightMargin,int SmartMode,int Justify)
{
  EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  if (ei.BlockType!=BTYPE_STREAM || RightMargin<1)
    return;

  struct EditorSetPosition esp;
  memset(&esp,-1,sizeof(esp));
  esp.CurLine=ei.BlockStartLine;
  esp.CurPos=0;
  Info.EditorControl(ECTL_SETPOSITION,&esp);

  TCHAR *TotalString=NULL;
  int TotalLength=0,IndentSize=0x7fffffff;

  while (1)
  {
    struct EditorGetString egs;
    egs.StringNumber=-1;
    if (!Info.EditorControl(ECTL_GETSTRING,&egs))
      break;
    if (egs.SelStart==-1 || egs.SelStart==egs.SelEnd)
      break;
    int ExpandNum=-1;
    Info.EditorControl(ECTL_EXPANDTABS,&ExpandNum);
    Info.EditorControl(ECTL_GETSTRING,&egs);

    int SpaceLength=0;
    while (SpaceLength<egs.StringLength && egs.StringText[SpaceLength]==_T(' '))
      SpaceLength++;

    while (egs.StringLength>0 && *egs.StringText==_T(' '))
    {
      egs.StringText++;
      egs.StringLength--;
    }

    if (egs.StringLength>0)
    {
      if (SpaceLength<IndentSize)
        IndentSize=SpaceLength;

      TotalString=(TCHAR *)realloc(TotalString,(TotalLength+egs.StringLength+2)*sizeof(TCHAR));
      if (TotalLength!=0 && TotalString[TotalLength-1]!=_T(' '))
        TotalString[TotalLength++]=_T(' ');

      _tmemcpy(TotalString+TotalLength,egs.StringText,egs.StringLength);
      TotalLength+=egs.StringLength;
    }
    if (!Info.EditorControl(ECTL_DELETESTRING,NULL))
    {
      free(TotalString);
      return;
    }
  }
  if(TotalString)
    TotalString[TotalLength++]=_T(' ');

  if (IndentSize>=RightMargin)
    IndentSize=RightMargin-1;

  const int MaxIndent=1024;
  if (IndentSize>=MaxIndent)
    IndentSize=MaxIndent-1;

  TCHAR IndentBuf[MaxIndent];
  if (IndentSize>0)
  {
    _tmemset(IndentBuf,_T(' '),IndentSize);
    IndentBuf[IndentSize]=0;
  }

  Info.EditorControl(ECTL_SETPOSITION,&esp);
  Info.EditorControl(ECTL_INSERTSTRING,NULL);
  Info.EditorControl(ECTL_SETPOSITION,&esp);

  int LastSplitPos=0,PrevSpacePos;
  while (LastSplitPos<TotalLength && TotalString[LastSplitPos]==_T(' '))
    LastSplitPos++;
  PrevSpacePos=LastSplitPos;

  for (int I=LastSplitPos;I<TotalLength;I++)
  {
    int Length=I-LastSplitPos;
    int LastLength=PrevSpacePos-LastSplitPos;
    if (TotalString[I]==_T(' ') && Length>RightMargin-IndentSize && LastLength<=RightMargin-IndentSize)
    {
      if (LastLength<=0)
      {
        PrevSpacePos=I;
        LastLength=Length;
      }

      if (SmartMode)
      {
        int Space1=-1,Space2=-1;
        for (int J=PrevSpacePos-1;J>LastSplitPos+20;J--)
        {
          if (TotalString[J]==_T(' '))
          {
            if (Space2==-1)
              Space2=J;
            else
            {
              if (Space1==-1)
                Space1=J;
              else
                break;
            }
          }
        }
        if (Space2!=-1 && PrevSpacePos-Space2<4)
          if (Space1==-1 || Space2-Space1>4 || PrevSpacePos-Space2==2)
          {
            PrevSpacePos=Space2;
            while (PrevSpacePos>LastSplitPos+1 && TotalString[PrevSpacePos-1]==_T(' '))
              PrevSpacePos--;
            LastLength=PrevSpacePos-LastSplitPos;
          }
      }

      I=PrevSpacePos;

      struct EditorSetString ess;
      ess.StringNumber=-1;
      ess.StringText=TotalString+LastSplitPos;
      ess.StringEOL=NULL;
      ess.StringLength=LastLength;
      while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==_T(' '))
        ess.StringLength--;
      if (!Justify || ess.StringLength>=RightMargin-IndentSize || !JustifyString(RightMargin-IndentSize,ess))
        Info.EditorControl(ECTL_SETSTRING,&ess);
      else
        LastLength=RightMargin;

      while (I<TotalLength && TotalString[I]==_T(' '))
        PrevSpacePos=I++;

      struct EditorSetPosition esp;
      memset(&esp,-1,sizeof(esp));
      esp.CurLine=-1;

      if (IndentSize>0)
      {
        esp.CurPos=0;
        Info.EditorControl(ECTL_SETPOSITION,&esp);
        Info.EditorControl(ECTL_INSERTTEXT,(void *)IndentBuf);
      }

      esp.CurPos=LastLength+IndentSize;
      Info.EditorControl(ECTL_SETPOSITION,&esp);
      Info.EditorControl(ECTL_INSERTSTRING,NULL);

      LastSplitPos=I;
    }
    if (TotalString[I]==_T(' '))
      PrevSpacePos=I;
  }
  struct EditorSetString ess;
  ess.StringNumber=-1;
  ess.StringText=TotalString+LastSplitPos;
  ess.StringEOL=NULL;
  ess.StringLength=TotalLength-LastSplitPos;
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==_T(' '))
    ess.StringLength--;
  Info.EditorControl(ECTL_SETSTRING,&ess);

  if (IndentSize>0)
  {
    struct EditorSetPosition esp;
    memset(&esp,-1,sizeof(esp));
    esp.CurLine=-1;
    esp.CurPos=0;
    Info.EditorControl(ECTL_SETPOSITION,&esp);
    Info.EditorControl(ECTL_INSERTTEXT,(void *)IndentBuf);
  }

  free(TotalString);

  memset(&esp,-1,sizeof(esp));
  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurPos;
  Info.EditorControl(ECTL_SETPOSITION,&esp);
}


void JustifyBlock(int RightMargin)
{
  EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  if (ei.BlockType!=BTYPE_STREAM)
    return;

  struct EditorGetString egs;
  egs.StringNumber=ei.BlockStartLine;

  while (1)
  {
    if (!Info.EditorControl(ECTL_GETSTRING,&egs))
      break;
    if (egs.SelStart==-1 || egs.SelStart==egs.SelEnd)
      break;
    int ExpNum=egs.StringNumber;
    if (!Info.EditorControl(ECTL_EXPANDTABS,&ExpNum))
      break;
    Info.EditorControl(ECTL_GETSTRING,&egs);

    struct EditorSetString ess;
    ess.StringNumber=egs.StringNumber;

    ess.StringText=(TCHAR*)egs.StringText;
    ess.StringEOL=(TCHAR*)egs.StringEOL;
    ess.StringLength=egs.StringLength;

    if (ess.StringLength<RightMargin)
      JustifyString(RightMargin,ess);

    egs.StringNumber++;
  }
}


int JustifyString(int RightMargin,struct EditorSetString &ess)
{
  int WordCount=0;
  int I;
  for (I=0;I<ess.StringLength-1;I++)
    if (ess.StringText[I]!=_T(' ') && ess.StringText[I+1]==_T(' '))
      WordCount++;
  if (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==_T(' '))
    WordCount--;
  if (WordCount<=0)
    return(FALSE);
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==_T(' '))
    ess.StringLength--;
  int TotalAddSize=RightMargin-ess.StringLength;
  int AddSize=TotalAddSize/WordCount;
  int Reminder=TotalAddSize%WordCount;

  TCHAR *NewString=(TCHAR *)malloc(RightMargin*sizeof(TCHAR));
  _tmemset(NewString,_T(' '),RightMargin);
  _tmemcpy(NewString,ess.StringText,ess.StringLength);

  for (I=0;I<RightMargin-1;I++)
    if (NewString[I]!=_T(' ') && NewString[I+1]==_T(' '))
    {
      int MoveSize=AddSize;
      if (Reminder)
      {
        MoveSize++;
        Reminder--;
      }
      if (MoveSize==0)
        break;
      memmove(NewString+I+1+MoveSize,NewString+I+1,(RightMargin-(I+1+MoveSize))*sizeof(TCHAR));
      while (MoveSize--)
        NewString[I+1+MoveSize]=_T(' ');
    }

  ess.StringText=NewString;
  ess.StringLength=RightMargin;
  Info.EditorControl(ECTL_SETSTRING,&ess);
  free(NewString);
  return(TRUE);
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static const TCHAR *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAlign);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);
  Info->PluginConfigStringsNumber=0;
}
