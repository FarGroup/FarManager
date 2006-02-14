#define WIN32_LEAN_AND_MEAN
#define STRICT
#define __STD_STRING
#include "plugin.hpp"

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


#include "AlignLng.hpp"
#include "Align.hpp"
#include "AlignReg.cpp"
#include "AlignMix.cpp"

static void ReformatBlock(int RightMargin,int SmartMode,int Justify);
static void JustifyBlock(int RightMargin);
static int JustifyString(int RightMargin,struct EditorSetString &ess);

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,"\\Align");
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  struct InitDialogItem InitItems[]={
    {DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MAlign},
    {DI_FIXEDIT,5,2,7,3,1,0,0,0,""},
    {DI_TEXT,9,2,0,0,0,0,0,0,(char *)MRightMargin},
    {DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MReformat},
    {DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MSmartMode},
    {DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MJustify},
    {DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel}
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  int RightMargin=GetRegKey(HKEY_CURRENT_USER,"","RightMargin",75);
  int Reformat=GetRegKey(HKEY_CURRENT_USER,"","Reformat",TRUE);
  int SmartMode=GetRegKey(HKEY_CURRENT_USER,"","SmartMode",FALSE);
  int Justify=GetRegKey(HKEY_CURRENT_USER,"","Justify",FALSE);
  FSF.sprintf(DialogItems[1].Data,"%d",RightMargin);
  DialogItems[3].Selected=Reformat;
  DialogItems[4].Selected=SmartMode;
  DialogItems[5].Selected=Justify;
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=7)
    return(INVALID_HANDLE_VALUE);
  RightMargin=FSF.atoi(DialogItems[1].Data);
  Reformat=DialogItems[3].Selected;
  SmartMode=DialogItems[4].Selected;
  Justify=DialogItems[5].Selected;
  SetRegKey(HKEY_CURRENT_USER,"","Reformat",Reformat);
  SetRegKey(HKEY_CURRENT_USER,"","RightMargin",RightMargin);
  SetRegKey(HKEY_CURRENT_USER,"","SmartMode",SmartMode);
  SetRegKey(HKEY_CURRENT_USER,"","Justify",Justify);
  if (Reformat)
    ReformatBlock(RightMargin,SmartMode,Justify);
  else
    if (Justify)
      JustifyBlock(RightMargin);
  return(INVALID_HANDLE_VALUE);
}


void ReformatBlock(int RightMargin,int SmartMode,int Justify)
{
  struct EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  if (ei.BlockType!=BTYPE_STREAM || RightMargin<1)
    return;

  struct EditorSetPosition esp;
  memset(&esp,-1,sizeof(esp));
  esp.CurLine=ei.BlockStartLine;
  esp.CurPos=0;
  Info.EditorControl(ECTL_SETPOSITION,&esp);

  char *TotalString=(char *)GlobalAlloc(GMEM_FIXED,0);
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
    while (SpaceLength<egs.StringLength && egs.StringText[SpaceLength]==' ')
      SpaceLength++;

    while (egs.StringLength>0 && *egs.StringText==' ')
    {
      egs.StringText++;
      egs.StringLength--;
    }

    if (egs.StringLength>0)
    {
      if (SpaceLength<IndentSize)
        IndentSize=SpaceLength;

      TotalString=(char *)GlobalReAlloc((HGLOBAL)TotalString,TotalLength+egs.StringLength+2,GMEM_MOVEABLE);
      if (TotalLength!=0 && TotalString[TotalLength-1]!=' ')
        TotalString[TotalLength++]=' ';

      memcpy(TotalString+TotalLength,egs.StringText,egs.StringLength);
      TotalLength+=egs.StringLength;
    }
    if (!Info.EditorControl(ECTL_DELETESTRING,NULL))
    {
      GlobalFree((HGLOBAL)TotalString);
      return;
    }
  }
  TotalString[TotalLength++]=' ';

  if (IndentSize>=RightMargin)
    IndentSize=RightMargin-1;

  const int MaxIndent=1024;
  if (IndentSize>=MaxIndent)
    IndentSize=MaxIndent-1;

  char IndentBuf[MaxIndent];
  if (IndentSize>0)
  {
    memset(IndentBuf,' ',IndentSize);
    IndentBuf[IndentSize]=0;
  }

  Info.EditorControl(ECTL_SETPOSITION,&esp);
  Info.EditorControl(ECTL_INSERTSTRING,NULL);
  Info.EditorControl(ECTL_SETPOSITION,&esp);

  int LastSplitPos=0,PrevSpacePos;
  while (LastSplitPos<TotalLength && TotalString[LastSplitPos]==' ')
    LastSplitPos++;
  PrevSpacePos=LastSplitPos;

  for (int I=LastSplitPos;I<TotalLength;I++)
  {
    int Length=I-LastSplitPos;
    int LastLength=PrevSpacePos-LastSplitPos;
    if (TotalString[I]==' ' && Length>RightMargin-IndentSize && LastLength<=RightMargin-IndentSize)
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
          if (TotalString[J]==' ')
            if (Space2==-1)
              Space2=J;
            else
              if (Space1==-1)
                Space1=J;
              else
                break;
        if (Space2!=-1 && PrevSpacePos-Space2<4)
          if (Space1==-1 || Space2-Space1>4 || PrevSpacePos-Space2==2)
          {
            PrevSpacePos=Space2;
            while (PrevSpacePos>LastSplitPos+1 && TotalString[PrevSpacePos-1]==' ')
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
      while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==' ')
        ess.StringLength--;
      if (!Justify || ess.StringLength>=RightMargin-IndentSize || !JustifyString(RightMargin-IndentSize,ess))
        Info.EditorControl(ECTL_SETSTRING,&ess);
      else
        LastLength=RightMargin;

      while (I<TotalLength && TotalString[I]==' ')
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
    if (TotalString[I]==' ')
      PrevSpacePos=I;
  }
  struct EditorSetString ess;
  ess.StringNumber=-1;
  ess.StringText=TotalString+LastSplitPos;
  ess.StringEOL=NULL;
  ess.StringLength=TotalLength-LastSplitPos;
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==' ')
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

  GlobalFree((HGLOBAL)TotalString);

  memset(&esp,-1,sizeof(esp));
  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurPos;
  Info.EditorControl(ECTL_SETPOSITION,&esp);
}


void JustifyBlock(int RightMargin)
{
  struct EditorInfo ei;
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

    ess.StringText=(char*)egs.StringText;
    ess.StringEOL=(char*)egs.StringEOL;
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
    if (ess.StringText[I]!=' ' && ess.StringText[I+1]==' ')
      WordCount++;
  if (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==' ')
    WordCount--;
  if (WordCount<=0)
    return(FALSE);
  while (ess.StringLength>0 && ess.StringText[ess.StringLength-1]==' ')
    ess.StringLength--;
  int TotalAddSize=RightMargin-ess.StringLength;
  int AddSize=TotalAddSize/WordCount;
  int Reminder=TotalAddSize%WordCount;

  char *NewString=(char *)GlobalAlloc(GMEM_FIXED,RightMargin);
  memset(NewString,' ',RightMargin);
  memcpy(NewString,ess.StringText,ess.StringLength);

  for (I=0;I<RightMargin-1;I++)
    if (NewString[I]!=' ' && NewString[I+1]==' ')
    {
      int MoveSize=AddSize;
      if (Reminder)
      {
        MoveSize++;
        Reminder--;
      }
      if (MoveSize==0)
        break;
      memmove(NewString+I+1+MoveSize,NewString+I+1,RightMargin-(I+1+MoveSize));
      while (MoveSize--)
        NewString[I+1+MoveSize]=' ';
    }

  ess.StringText=NewString;
  ess.StringLength=RightMargin;
  Info.EditorControl(ECTL_SETSTRING,&ess);
  GlobalFree((HGLOBAL)NewString);
  return(TRUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MAlign);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}
