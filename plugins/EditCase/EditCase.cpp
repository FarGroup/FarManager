#include <windows.h>
#include "d:\lang\bc5\far\plugin.hpp"
#include "editlng.hpp"
#include "editcase.hpp"

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  struct FarMenuItem MenuItems[2];
  memset(MenuItems,0,sizeof(MenuItems));
  strcpy(MenuItems[0].Text,GetMsg(MCaseLower));
  strcpy(MenuItems[1].Text,GetMsg(MCaseUpper));
  MenuItems[0].Selected=TRUE;
  int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
                         GetMsg(MCaseConversion),NULL,"Contents",NULL,NULL,
                         MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
  if (MenuCode<0)
    return(INVALID_HANDLE_VALUE);

  struct EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  if (ei.BlockType!=BTYPE_NONE)
  {
    int CurLine=ei.BlockStartLine;
    while (1)
    {
      struct EditorGetString egs;
      egs.StringNumber=CurLine++;
      if (!Info.EditorControl(ECTL_GETSTRING,&egs))
        break;
      if (egs.SelStart==-1)
        break;
      if (egs.SelEnd==-1 || egs.SelEnd>egs.StringLength)
      {
        egs.SelEnd=egs.StringLength;
        if (egs.SelEnd<egs.SelStart)
          egs.SelEnd=egs.SelStart;
      }
      char *NewString=(char *)GlobalAlloc(GMEM_FIXED,egs.StringLength+1);
      memcpy(NewString,egs.StringText,egs.StringLength);

      struct EditorConvertText ect;
      ect.Text=NewString+egs.SelStart;
      ect.TextLength=egs.SelEnd-egs.SelStart;
      Info.EditorControl(ECTL_EDITORTOOEM,&ect);
      for (int I=egs.SelStart;I<egs.SelEnd;I++)
      {
        char Ansi,ReverseOem;
        OemToCharBuff(&NewString[I],&Ansi,1);
        CharToOemBuff(&Ansi,&ReverseOem,1);
        if (IsCharAlpha(Ansi) && ReverseOem==NewString[I])
        {
          if (MenuCode==0)
            NewString[I]=(char)CharLower((LPTSTR)(unsigned char)Ansi);
          else
            NewString[I]=(char)CharUpper((LPTSTR)(unsigned char)Ansi);
          CharToOemBuff(&NewString[I],&NewString[I],1);
        }
      }
      Info.EditorControl(ECTL_OEMTOEDITOR,&ect);

      struct EditorSetString ess;
      ess.StringNumber=egs.StringNumber;
      ess.StringText=NewString;
      ess.StringEOL=egs.StringEOL;
      ess.StringLength=egs.StringLength;
      Info.EditorControl(ECTL_SETSTRING,&ess);

      GlobalFree((HGLOBAL)NewString);
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
  PluginMenuStrings[0]=GetMsg(MCaseConversion);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}


char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}
