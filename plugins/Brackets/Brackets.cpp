#include <windows.h>
#include "d:\lang\bc5\far\plugin.hpp"
#include "bracklng.hpp"
#include "Brackets.hpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  struct EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  struct EditorGetString egs;
  egs.StringNumber=ei.CurLine;
  Info.EditorControl(ECTL_GETSTRING,&egs);

  if (ei.CurPos>=egs.StringLength)
    return(INVALID_HANDLE_VALUE);

  char Bracket=egs.StringText[ei.CurPos];
  int Direction;

  if (Bracket=='(' || Bracket=='{' || Bracket=='[')
    Direction=1;
  else
    if (Bracket==')' || Bracket=='}' || Bracket==']')
      Direction=-1;
    else
      return(INVALID_HANDLE_VALUE);

  int CurPos=ei.CurPos;

  int MatchCount=1;

  while (1)
  {
    CurPos+=Direction;
    if (CurPos>=egs.StringLength)
    {
      if (++egs.StringNumber >= ei.TotalLines)
        break;
      Info.EditorControl(ECTL_GETSTRING,&egs);
      CurPos=0;
    }
    if (CurPos<0)
    {
      if (--egs.StringNumber < 0)
        break;
      Info.EditorControl(ECTL_GETSTRING,&egs);
      CurPos=egs.StringLength-1;
    }
    if (CurPos>=egs.StringLength || CurPos<0)
      continue;
    char Ch=egs.StringText[CurPos];
    if (Ch==Bracket)
      MatchCount++;
    else
      if (Ch=='(' && Bracket==')' || Ch=='{' && Bracket=='}' ||
          Ch=='[' && Bracket==']' || Ch==')' && Bracket=='(' ||
          Ch=='}' && Bracket=='{' || Ch==']' && Bracket=='[')
        if (--MatchCount==0)
        {
          struct EditorSetPosition esp;
          esp.CurLine=egs.StringNumber;
          esp.CurPos=CurPos;
          esp.CurTabPos=esp.LeftPos=esp.Overtype=-1;
          if (egs.StringNumber<ei.TopScreenLine ||
              egs.StringNumber>=ei.TopScreenLine+ei.WindowSizeY)
          {
            esp.TopScreenLine=esp.CurLine-ei.WindowSizeY/2;
            if (esp.TopScreenLine<0)
              esp.TopScreenLine=0;
          }
          else
            esp.TopScreenLine=-1;

          Info.EditorControl(ECTL_SETPOSITION,&esp);
          break;
        }
  }

  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MBrackets);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}


char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}
