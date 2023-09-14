#include <cstdlib>
#include <cwchar>
#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "AlignLng.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

static PluginStartupInfo PsInfo;
static FarStandardFunctions FSF;

static void ReformatBlock(int RightMargin,int SmartMode,int Justify);
static void JustifyBlock(int RightMargin);
static bool JustifyString(int RightMargin,EditorSetString &ess);

static const wchar_t *GetMsg(intptr_t MsgId)
{
  return PsInfo.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}


void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
  PsInfo=*Info;
  FSF=*PsInfo.FSF;
  PsInfo.FSF=&FSF;
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAlign);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}


HANDLE WINAPI OpenW(const OpenInfo *Info)
{
  PluginSettings settings(MainGuid, PsInfo.SettingsControl);

  int RightMargin=settings.Get(0,L"RightMargin",75);
  int Reformat=settings.Get(0,L"Reformat",TRUE);
  int SmartMode=settings.Get(0,L"SmartMode",FALSE);
  int Justify=settings.Get(0,L"Justify",FALSE);

  PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MAlign, nullptr);
  FarDialogItem *RightMarginItem = Builder.AddIntEditField(&RightMargin, 3);
  Builder.AddTextAfter(RightMarginItem, MRightMargin);
  Builder.AddCheckbox(MReformat, &Reformat);
  Builder.AddCheckbox(MSmartMode, &SmartMode);
  Builder.AddCheckbox(MJustify, &Justify);
  Builder.AddOKCancel(MOk, MCancel);

  if (Builder.ShowDialog())
  {
    settings.Set(0,L"Reformat",Reformat);
    settings.Set(0,L"RightMargin",RightMargin);
    settings.Set(0,L"SmartMode",SmartMode);
    settings.Set(0,L"Justify",Justify);
    if (Reformat)
      ReformatBlock(RightMargin,SmartMode,Justify);
    else if (Justify)
      JustifyBlock(RightMargin);
  }

  return nullptr;
}


void ReformatBlock(int RightMargin,int SmartMode,int Justify)
{
  EditorInfo ei={sizeof(EditorInfo)};
  PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);

  if (ei.BlockType!=BTYPE_STREAM || RightMargin<1)
    return;

  EditorSetPosition esp={sizeof(EditorSetPosition),-1,-1,-1,-1,-1,-1};
  esp.CurLine=ei.BlockStartLine;
  esp.CurPos=0;
  PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);

  wchar_t *TotalString={};
  intptr_t TotalLength=0;
  int IndentSize=0x7fffffff;

  while (1)
  {
    EditorGetString egs={sizeof(EditorGetString)};
    egs.StringNumber=-1;
    if (!PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs))
      break;
    if (egs.SelStart==-1 || egs.SelStart==egs.SelEnd)
      break;
    intptr_t ExpandNum=-1;
    PsInfo.EditorControl(-1,ECTL_EXPANDTABS,0,&ExpandNum);
    PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);

    int SpaceLength=0;
    while (SpaceLength<egs.StringLength && egs.StringText[SpaceLength]==L' ')
      SpaceLength++;

    while (egs.StringLength>0 && *egs.StringText==L' ')
    {
      egs.StringText++;
      egs.StringLength--;
    }

    if (egs.StringLength>0)
    {
      if (SpaceLength<IndentSize)
        IndentSize=SpaceLength;

      TotalString=(wchar_t *)realloc(TotalString,(TotalLength+egs.StringLength+2)*sizeof(wchar_t));
      if (TotalLength!=0 && TotalString[TotalLength-1]!=L' ')
        TotalString[TotalLength++]=L' ';

      wmemcpy(TotalString+TotalLength,egs.StringText,egs.StringLength);
      TotalLength+=egs.StringLength;
    }
    if (!PsInfo.EditorControl(-1,ECTL_DELETESTRING,0,{}))
    {
      free(TotalString);
      return;
    }
  }
  if(TotalString)
    TotalString[TotalLength++]=L' ';

  if (IndentSize>=RightMargin)
    IndentSize=RightMargin-1;

  const int MaxIndent=1024;
  if (IndentSize>=MaxIndent)
    IndentSize=MaxIndent-1;

  wchar_t IndentBuf[MaxIndent];
  if (IndentSize>0)
  {
    wmemset(IndentBuf,L' ',IndentSize);
    IndentBuf[IndentSize]=0;
  }

  PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
  PsInfo.EditorControl(-1,ECTL_INSERTSTRING,0,{});
  PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);

  int LastSplitPos=0,PrevSpacePos;
  while (LastSplitPos<TotalLength && TotalString[LastSplitPos]==L' ')
    LastSplitPos++;
  PrevSpacePos=LastSplitPos;

  for (int I=LastSplitPos;I<TotalLength;I++)
  {
    int Length=I-LastSplitPos;
    int LastLength=PrevSpacePos-LastSplitPos;
    if (TotalString[I]==L' ' && Length>RightMargin-IndentSize && LastLength<=RightMargin-IndentSize)
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
          if (TotalString[J]==L' ')
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
            while (PrevSpacePos>LastSplitPos+1 && TotalString[PrevSpacePos-1]==L' ')
              PrevSpacePos--;
            LastLength=PrevSpacePos-LastSplitPos;
          }
      }

      I=PrevSpacePos;

      EditorSetString ess={sizeof(EditorSetString)};
      ess.StringNumber=-1;
      ess.StringText=TotalString+LastSplitPos;
      ess.StringEOL={};
      ess.StringLength=LastLength;
      while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==L' ')
        ess.StringLength--;
      if (!Justify || ess.StringLength>=RightMargin-IndentSize || !JustifyString(RightMargin-IndentSize,ess))
        PsInfo.EditorControl(-1,ECTL_SETSTRING,0,&ess);
      else
        LastLength=RightMargin;

      while (I<TotalLength && TotalString[I]==L' ')
        PrevSpacePos=I++;

      EditorSetPosition SetPos={sizeof(EditorSetPosition),-1,-1,-1,-1,-1,-1};

      if (IndentSize>0)
      {
        SetPos.CurPos=0;
        PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&SetPos);
        PsInfo.EditorControl(-1,ECTL_INSERTTEXT,0,IndentBuf);
      }

      SetPos.CurPos=LastLength+IndentSize;
      PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&SetPos);
      PsInfo.EditorControl(-1,ECTL_INSERTSTRING,0,{});

      LastSplitPos=I;
    }
    if (TotalString[I]==L' ')
      PrevSpacePos=I;
  }
  EditorSetString ess={sizeof(EditorSetString)};
  ess.StringNumber=-1;
  ess.StringText=TotalString+LastSplitPos;
  ess.StringEOL={};
  ess.StringLength=TotalLength-LastSplitPos;
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==L' ')
    ess.StringLength--;
  PsInfo.EditorControl(-1,ECTL_SETSTRING,0,&ess);

  if (IndentSize>0)
  {
    EditorSetPosition SetPos={sizeof(EditorSetPosition), -1};
    PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&SetPos);
    PsInfo.EditorControl(-1,ECTL_INSERTTEXT,0,IndentBuf);
  }

  free(TotalString);

  memset(&esp,-1,sizeof(esp));
  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurPos;
  PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
}


void JustifyBlock(int RightMargin)
{
  EditorInfo ei={sizeof(EditorInfo)};
  PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);

  if (ei.BlockType!=BTYPE_STREAM)
    return;

  EditorGetString egs={sizeof(EditorGetString)};
  egs.StringNumber=ei.BlockStartLine;

  while (1)
  {
    if (!PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs))
      break;
    if (egs.SelStart==-1 || egs.SelStart==egs.SelEnd)
      break;
    int ExpNum=(int)egs.StringNumber;
    if (!PsInfo.EditorControl(-1,ECTL_EXPANDTABS,0,&ExpNum))
      break;
    PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);

    EditorSetString ess={sizeof(EditorSetString)};
    ess.StringNumber=egs.StringNumber;

    ess.StringText=const_cast<wchar_t*>(egs.StringText);
    ess.StringEOL=const_cast<wchar_t*>(egs.StringEOL);
    ess.StringLength=egs.StringLength;

    if (ess.StringLength<RightMargin)
      JustifyString(RightMargin,ess);

    egs.StringNumber++;
  }
}


bool JustifyString(int RightMargin,EditorSetString &ess)
{
  int WordCount=0;
  int I;
  for (I=0;I<ess.StringLength-1;I++)
    if (ess.StringText[I]!=L' ' && ess.StringText[I+1]==L' ')
      WordCount++;
  if (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==L' ')
    WordCount--;
  if (WordCount<=0)
    return false;
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==L' ')
    ess.StringLength--;
  intptr_t TotalAddSize=RightMargin-ess.StringLength;
  intptr_t AddSize=TotalAddSize/WordCount;
  int Reminder=TotalAddSize%WordCount;

  wchar_t *NewString=(wchar_t *)malloc(RightMargin*sizeof(wchar_t));
  wmemset(NewString,L' ',RightMargin);
  wmemcpy(NewString,ess.StringText,ess.StringLength);

  for (I=0;I<RightMargin-1;I++)
  {
    if (NewString[I]!=L' ' && NewString[I+1]==L' ')
    {
      intptr_t MoveSize=AddSize;
      if (Reminder)
      {
        MoveSize++;
        Reminder--;
      }
      if (MoveSize==0)
        break;
      wmemmove(NewString+I+1+MoveSize,NewString+I+1,RightMargin-(I+1+MoveSize));
      while (MoveSize--)
        NewString[I+1+MoveSize]=L' ';
    }
  }

  ess.StringText=NewString;
  ess.StringLength=RightMargin;
  PsInfo.EditorControl(-1,ECTL_SETSTRING,0,&ess);
  free(NewString);
  return true;
}
