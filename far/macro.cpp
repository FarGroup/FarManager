/*
macro.cpp

Макросы

*/

/* Revision: 1.03 23.07.2000 $ */

/*
Modify:
  23.07.2000 SVS
    + Клавиши:
       Ctrl- Shift- Alt- CtrlShift- AltShift- CtrlAlt- Apps :-)
       KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

KeyMacro::KeyMacro()
{
  Macros=NULL;
  MacrosNumber=0;
  StartMacroPos=0;
  Recording=FALSE;
  Executing=FALSE;
  RecBuffer=NULL;
  RecBufferSize=0;
  InternalInput=FALSE;
  ReadMacros(MACRO_SHELL);
  ReadMacros(MACRO_VIEWER);
  ReadMacros(MACRO_EDITOR);
  ReadMacros(MACRO_DIALOG);
  ReadMacros(MACRO_SEARCH);
  ReadMacros(MACRO_DISKS);
  ReadMacros(MACRO_MAINMENU);
  ReadMacros(MACRO_HELP);
  ReadMacros(MACRO_OTHER);
  Mode=MACRO_SHELL;
  LockScr=NULL;
}


KeyMacro::~KeyMacro()
{
  /* $ 13.07.2000 SVS
     ни кто не вызывал запрос памяти через new :-)
  */
  for (int I=0;I<MacrosNumber;I++)
    free(Macros[I].Buffer);
  free(Macros);
  /* ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
  delete[] RecBuffer;
  /* SVS $ */
  delete LockScr;
}


int KeyMacro::ProcessKey(int Key)
{
  if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE)
    return(FALSE);
  if (Recording)
  {
    if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT)
    {
      int MacroKey;

      {
        SaveScreen SaveScr;
        SetCursorType(FALSE,0);
        Message(0,0,MSG(MDefineMacroTitle),MSG(MDefineMacro));
        InternalInput=TRUE;
        do {
          INPUT_RECORD rec;
          MacroKey=GetInputRecord(&rec);
        } while (MacroKey>=KEY_NONE);
        InternalInput=FALSE;
      }

      int DisableOutput=TRUE,EmptyCommandLine=FALSE,NotEmptyCommandLine=FALSE;
      int RunAfterStart=FALSE;

      if (Key==KEY_CTRLSHIFTDOT || Recording==2)
      {
        InternalInput=TRUE;
        if (!GetMacroSettings(DisableOutput,RunAfterStart,EmptyCommandLine,NotEmptyCommandLine))
          MacroKey=KEY_ESC;
        InternalInput=FALSE;
      }

      if (MacroKey==KEY_ESC)
        delete RecBuffer;
      else
      {
        int Pos;
        for (Pos=0;Pos<MacrosNumber;Pos++)
          if (Macros[Pos].Key==MacroKey && Macros[Pos].Mode==StartMode)
            break;
        if (Pos==MacrosNumber)
        {
          Macros=(struct MacroRecord *)realloc(Macros,sizeof(*Macros)*(MacrosNumber+1));
          if (Macros==NULL)
          {
            MacrosNumber=0;
            return(FALSE);
          }
          MacrosNumber++;
        }
        else
          delete Macros[Pos].Buffer;
        Macros[Pos].Key=MacroKey;
        Macros[Pos].Buffer=RecBuffer;
        Macros[Pos].BufferSize=RecBufferSize;
        Macros[Pos].Mode=StartMode;
        Macros[Pos].DisableOutput=DisableOutput;
        Macros[Pos].RunAfterStart=RunAfterStart;
        Macros[Pos].EmptyCommandLine=EmptyCommandLine;
        Macros[Pos].NotEmptyCommandLine=NotEmptyCommandLine;
      }

      Recording=FALSE;
      RecBuffer=NULL;
      RecBufferSize=0;
      if (Opt.AutoSaveSetup)
        SaveMacros();
      ScrBuf.RestoreMacroChar();
      return(TRUE);
    }
    else
    {
      if (Key>=KEY_NONE)
        return(FALSE);
      RecBuffer=(int *)realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+1));
      if (RecBuffer==NULL)
        return(FALSE);
      RecBuffer[RecBufferSize]=Key;
      RecBufferSize++;
      return(FALSE);
    }
  }
  else
    if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT)
    {
      delete LockScr;
      LockScr=NULL;

      if (Mode==MACRO_SHELL && !WaitInMainLoop)
        StartMode=MACRO_OTHER;
      else
        StartMode=Mode;
      Recording=(Key==KEY_CTRLSHIFTDOT) ? 2:1;
      delete RecBuffer;
      RecBuffer=NULL;
      RecBufferSize=0;
      ScrBuf.ResetShadow();
      ScrBuf.Flush();
      return(TRUE);
    }
    else
    {
      if (!Executing)
      {
        int CurMode=(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode;
        for (int I=0;I<MacrosNumber;I++)
          if (LocalUpper(Macros[I].Key)==LocalUpper(Key) &&
              Macros[I].Mode==CurMode && Macros[I].BufferSize>0)
          {
            int CmdLength=CtrlObject->CmdLine.GetLength();
            if (Macros[I].EmptyCommandLine && CmdLength!=0 ||
                Macros[I].NotEmptyCommandLine && CmdLength==0)
              return(FALSE);
            if (Macros[I].DisableOutput)
              LockScr=new LockScreen;
            Executing=TRUE;
            ExecMacroPos=I;
            ExecKeyPos=0;
            return(TRUE);
          }
      }
      return(FALSE);
    }
}


int KeyMacro::GetKey()
{
  if (InternalInput || !Executing)
    return(FALSE);
  if (ExecKeyPos>=Macros[ExecMacroPos].BufferSize ||
      Macros[ExecMacroPos].Buffer==NULL)
  {
    delete LockScr;
    LockScr=NULL;
    Executing=FALSE;
    return(FALSE);
  }
  int Key=Macros[ExecMacroPos].Buffer[ExecKeyPos++];
  return(Key);
}


int KeyMacro::PeekKey()
{
  if (InternalInput || !Executing || ExecKeyPos>=Macros[ExecMacroPos].BufferSize)
    return(0);
  int Key=Macros[ExecMacroPos].Buffer[ExecKeyPos];
  return(Key);
}


void KeyMacro::SaveMacros()
{
  for (int I=0;I<MacrosNumber;I++)
  {
    char *TextBuffer=NULL;
    int TextBufferSize=0;
    char KeyText[50],RegKeyName[50],*Subkey;
    KeyToText(Macros[I].Key,KeyText);
    switch(Macros[I].Mode)
    {
      case MACRO_SHELL:
        Subkey="Shell";
        break;
      case MACRO_VIEWER:
        Subkey="Viewer";
        break;
      case MACRO_EDITOR:
        Subkey="Editor";
        break;
      case MACRO_DIALOG:
        Subkey="Dialog";
        break;
      case MACRO_SEARCH:
        Subkey="Search";
        break;
      case MACRO_DISKS:
        Subkey="Disks";
        break;
      case MACRO_MAINMENU:
        Subkey="MainMenu";
        break;
      case MACRO_HELP:
        Subkey="Help";
        break;
      case MACRO_OTHER:
        Subkey="Other";
        break;
    }
    sprintf(RegKeyName,"KeyMacros\\%s\\%s",Subkey,KeyText);

    if (Macros[I].BufferSize==0)
    {
      DeleteRegKey(RegKeyName);
      continue;
    }
    for (int J=0;J<Macros[I].BufferSize;J++)
    {
      char MacroKeyText[50];
      KeyToText(Macros[I].Buffer[J],MacroKeyText);
      int NewSize=TextBufferSize+strlen(MacroKeyText);
      if (TextBufferSize!=0)
        NewSize++;
      TextBuffer=(char *)realloc(TextBuffer,NewSize+1);
      if (TextBuffer==NULL)
        continue;
      sprintf(TextBuffer+TextBufferSize,"%s%s",
              TextBufferSize==0 ? "":" ",MacroKeyText);
      TextBufferSize=NewSize;
    }
    SetRegKey(RegKeyName,"Sequence",TextBuffer);
    if (Macros[I].DisableOutput)
      SetRegKey(RegKeyName,"DisableOutput",1);
    else
      DeleteRegValue(RegKeyName,"DisableOutput");
    if (Macros[I].RunAfterStart)
      SetRegKey(RegKeyName,"RunAfterFARStart",1);
    else
      DeleteRegValue(RegKeyName,"RunAfterFARStart");
    if (Macros[I].EmptyCommandLine)
      SetRegKey(RegKeyName,"EmptyCommandLine",1);
    else
      DeleteRegValue(RegKeyName,"EmptyCommandLine");
    if (Macros[I].NotEmptyCommandLine)
      SetRegKey(RegKeyName,"NotEmptyCommandLine",1);
    else
      DeleteRegValue(RegKeyName,"NotEmptyCommandLine");
    delete TextBuffer;
  }
}


void KeyMacro::KeyToText(int Key,char *KeyText)
{
  if (Key>=KEY_F1 && Key<=KEY_F12)
  {
    sprintf(KeyText,"F%d",Key-KEY_F1+1);
    return;
  }
  if (Key>=KEY_CTRLF1 && Key<=KEY_CTRLF12)
  {
    sprintf(KeyText,"CtrlF%d",Key-KEY_CTRLF1+1);
    return;
  }
  if (Key>=KEY_ALTF1 && Key<=KEY_ALTF12)
  {
    sprintf(KeyText,"AltF%d",Key-KEY_ALTF1+1);
    return;
  }
  if (Key>=KEY_SHIFTF1 && Key<=KEY_SHIFTF12)
  {
    sprintf(KeyText,"ShiftF%d",Key-KEY_SHIFTF1+1);
    return;
  }
  if (Key>=KEY_CTRLA && Key<=KEY_CTRLZ)
  {
    sprintf(KeyText,"Ctrl%c",Key-KEY_CTRLA+'A');
    return;
  }
  if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    sprintf(KeyText,"Ctrl%c",Key-KEY_CTRL0+'0');
    return;
  }
  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    sprintf(KeyText,"RCtrl%c",Key-KEY_RCTRL0+'0');
    return;
  }
  if (Key>=KEY_CTRLSHIFTF1 && Key<=KEY_CTRLSHIFTF12)
  {
    sprintf(KeyText,"CtrlShiftF%d",Key-KEY_CTRLSHIFTF1+1);
    return;
  }
  if (Key>=KEY_ALTSHIFTF1 && Key<=KEY_ALTSHIFTF12)
  {
    sprintf(KeyText,"AltShiftF%d",Key-KEY_ALTSHIFTF1+1);
    return;
  }
  if (Key>=KEY_CTRLALTF1 && Key<=KEY_CTRLALTF12)
  {
    sprintf(KeyText,"CtrlAltF%d",Key-KEY_CTRLALTF1+1);
    return;
  }
  if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
  {
    sprintf(KeyText,"CtrlShift%c",Key-KEY_CTRLSHIFT0+'0');
    return;
  }
  if (Key>=KEY_CTRLSHIFTA && Key<=KEY_CTRLSHIFTZ)
  {
    sprintf(KeyText,"CtrlShift%c",Key-KEY_CTRLSHIFTA+'A');
    return;
  }
  if (Key>=KEY_ALTSHIFTA && Key<=KEY_ALTSHIFTZ)
  {
    sprintf(KeyText,"AltShift%c",Key-KEY_ALTSHIFTA+'A');
    return;
  }
  if (Key>=KEY_CTRLALTA && Key<=KEY_CTRLALTZ)
  {
    sprintf(KeyText,"CtrlAlt%c",Key-KEY_CTRLALTA+'A');
    return;
  }
  if (Key>=KEY_ALT0 && Key<=KEY_ALT9)
  {
    sprintf(KeyText,"Alt%c",Key-KEY_ALT0+'0');
    return;
  }
  if (Key>=KEY_ALTA && Key<=KEY_ALTZ)
  {
    sprintf(KeyText,"Alt%c",Key-KEY_ALTA+'A');
    return;
  }
  /* $ 23.07.2000 SVS
     + KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
  */
  static int KeyCodes[]={
    KEY_BS,KEY_TAB,KEY_ENTER,KEY_ESC,KEY_SPACE,KEY_HOME,KEY_END,KEY_UP,
    KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_PGUP,KEY_PGDN,KEY_INS,KEY_DEL,KEY_NUMPAD5,
    KEY_CTRLBRACKET,KEY_CTRLBACKBRACKET,KEY_CTRLCOMMA,KEY_CTRLDOT,KEY_CTRLBS,
    KEY_CTRLQUOTE,KEY_CTRLSLASH,
    KEY_CTRLENTER,KEY_CTRLTAB,KEY_CTRLSHIFTINS,KEY_CTRLSHIFTDOWN,
    KEY_CTRLSHIFTLEFT,KEY_CTRLSHIFTRIGHT,KEY_CTRLSHIFTUP,KEY_CTRLSHIFTEND,
    KEY_CTRLSHIFTHOME,KEY_CTRLSHIFTPGDN,KEY_CTRLSHIFTPGUP,
    KEY_CTRLSHIFTSLASH,KEY_CTRLSHIFTBACKSLASH,
    KEY_CTRLSHIFTSUBTRACT,KEY_CTRLSHIFTADD,KEY_CTRLSHIFTENTER,KEY_ALTADD,
    KEY_ALTSUBTRACT,KEY_ALTMULTIPLY,KEY_ALTDOT,KEY_ALTCOMMA,KEY_ALTINS,
    KEY_ALTDEL,KEY_ALTBS,KEY_ALTHOME,KEY_ALTEND,KEY_ALTPGUP,KEY_ALTPGDN,
    KEY_ALTUP,KEY_ALTDOWN,KEY_ALTLEFT,KEY_ALTRIGHT,
    KEY_CTRLDOWN,KEY_CTRLLEFT,KEY_CTRLRIGHT,KEY_CTRLUP,
    KEY_CTRLEND,KEY_CTRLHOME,KEY_CTRLPGDN,KEY_CTRLPGUP,KEY_CTRLBACKSLASH,
    KEY_CTRLSUBTRACT,KEY_CTRLADD,KEY_CTRLMULTIPLY,KEY_CTRLCLEAR,KEY_ADD,
    KEY_SUBTRACT,KEY_MULTIPLY,KEY_BREAK,KEY_SHIFTINS,KEY_SHIFTDEL,
    KEY_SHIFTEND,KEY_SHIFTHOME,KEY_SHIFTLEFT,KEY_SHIFTUP,KEY_SHIFTRIGHT,
    KEY_SHIFTDOWN,KEY_SHIFTPGUP,KEY_SHIFTPGDN,KEY_SHIFTENTER,KEY_SHIFTTAB,
    KEY_SHIFTADD,KEY_SHIFTSUBTRACT,KEY_CTRLINS,KEY_CTRLDEL,KEY_CTRLSHIFTDOT,
    KEY_CTRLSHIFTTAB,KEY_DIVIDE,KEY_CTRLSHIFTBS,KEY_ALT,KEY_CTRL,KEY_SHIFT,
    KEY_RALT,KEY_RCTRL,KEY_CTRLSHIFTBRACKET,KEY_CTRLSHIFTBACKBRACKET,
    KEY_ALTSHIFTINS,KEY_ALTSHIFTDOWN,KEY_ALTSHIFTLEFT,KEY_ALTSHIFTRIGHT,
    KEY_ALTSHIFTUP,KEY_ALTSHIFTEND,KEY_ALTSHIFTHOME,KEY_ALTSHIFTPGDN,
    KEY_ALTSHIFTPGUP,KEY_ALTSHIFTENTER,
    KEY_CTRLALTINS,KEY_CTRLALTDOWN,KEY_CTRLALTLEFT,KEY_CTRLALTRIGHT,
    KEY_CTRLALTUP,KEY_CTRLALTEND,KEY_CTRLALTHOME,KEY_CTRLALTPGDN,
    KEY_CTRLALTPGUP,KEY_CTRLALTENTER,KEY_SHIFTBS,KEY_APPS,
    KEY_CTRLAPPS,KEY_ALTAPPS,KEY_SHIFTAPPS,
    KEY_CTRLSHIFTAPPS,KEY_ALTSHIFTAPPS,KEY_CTRLALTAPPS,
    KEY_LWIN,KEY_RWIN
  };
  static char *KeyNames[]={
    "BS","Tab","Enter","Esc","Space","Home","End","Up",
    "Down","Left","Right","PgUp","PgDn","Ins","Del","Clear",
    "Ctrl[","Ctrl]","Ctrl,","Ctrl.","CtrlBS",
    "Ctrl\"","Ctrl/",
    "CtrlEnter","CtrlTab","CtrlShiftIns","CtrlShiftDown",
    "CtrlShiftLeft","CtrlShiftRight","CtrlShiftUp","CtrlShiftEnd",
    "CtrlShiftHome","CtrlShiftPgDn","CtrlShiftPgUp",
    "CtrlShiftSlash","CtrlShiftBackSlash",
    "CtrlShiftSubtract","CtrlShiftAdd","CtrlShiftEnter","AltAdd",
    "AltSubtract","AltMultiply","Alt.","Alt,","AltIns",
    "AltDel","AltBS","AltHome","AltEnd","AltPgUp","AltPgDn",
    "AltUp","AltDown","AltLeft","AltRight",
    "CtrlDown","CtrlLeft","CtrlRight","CtrlUp",
    "CtrlEnd","CtrlHome","CtrlPgDn","CtrlPgUp","CtrlBackSlash",
    "CtrlSubtract","CtrlAdd","CtrlMultiply","CtrlClear","Add",
    "Subtract","Multiply","Break","ShiftIns","ShiftDel",
    "ShiftEnd","ShiftHome","ShiftLeft","ShiftUp","ShiftRight",
    "ShiftDown","ShiftPgUp","ShiftPgDn","ShiftEnter","ShiftTab",
    "ShiftAdd","ShiftSubtract","CtrlIns","CtrlDel","CtrlShiftDot",
    "CtrlShiftTab","Divide","CtrlShiftBS","Alt","Ctrl","Shift",
    "RAlt","RCtrl","CtrlShift[","CtrlShift]",
    "AltShiftIns","AltShiftDown","AltShiftLeft","AltShiftRight",
    "AltShiftUp","AltShiftEnd","AltShiftHome","AltShiftPgDn",
    "AltShiftPgUp","AltShiftEnter",
    "CtrlAltIns","CtrlAltDown","CtrlAltLeft","CtrlAltRight",
    "CtrlAltUp","CtrlAltEnd","CtrlAltHome","CtrlAltPgDn","CtrlAltPgUp",
    "CtrlAltEnter","ShiftBS",
    "Apps","CtrlApps","AltApps","ShiftApps",
    "CtrlShiftApps","AltShiftApps","CtrlAltApps",
    "LWin","RWin"
  };
  /* SVS $ */
  int I;

  for (I=0;I<sizeof(KeyCodes)/sizeof(KeyCodes[0]);I++)
    if (Key==KeyCodes[I])
    {
      strcpy(KeyText,KeyNames[I]);
      return;
    }
  if (Key<256)
    sprintf(KeyText,"%c",Key);
  else
    if (Key>KEY_CTRL_BASE && Key<KEY_END_CTRL_BASE)
      sprintf(KeyText,"Ctrl%c",Key-KEY_CTRL_BASE);
    else
      if (Key>KEY_ALT_BASE && Key<KEY_END_ALT_BASE)
        sprintf(KeyText,"Alt%c",Key-KEY_ALT_BASE);
      else
        if (Key>KEY_CTRLSHIFT_BASE && Key<KEY_END_CTRLSHIFT_BASE)
          sprintf(KeyText,"CtrlShift%c",Key-KEY_CTRLSHIFT_BASE);
        else
          if (Key>KEY_ALTSHIFT_BASE && Key<KEY_END_ALTSHIFT_BASE)
            sprintf(KeyText,"AltShift%c",Key-KEY_ALTSHIFT_BASE);
          else
            if (Key>KEY_CTRLALT_BASE && Key<KEY_END_CTRLALT_BASE)
              sprintf(KeyText,"CtrlAlt%c",Key-KEY_CTRLALT_BASE);
            else
              *KeyText=0;
  for (I=0;KeyText[I]!=0;I++)
    if (KeyText[I]=='\\')
    {
      strcpy(KeyText+I,"BackSlash");
      break;
    }
}


void KeyMacro::ReadMacros(int ReadMode)
{
  char KeyNames[2048][32];
  char Buffer[32768];
  int I;

  for (I=0;I<sizeof(KeyNames)/sizeof(KeyNames[0]);I++)
    KeyToText(I,KeyNames[I]);
  for (I=0;;I++)
  {
    char RegKeyName[50],KeyText[50],*Subkey;
    switch(ReadMode)
    {
      case MACRO_SHELL:
        Subkey="Shell";
        break;
      case MACRO_VIEWER:
        Subkey="Viewer";
        break;
      case MACRO_EDITOR:
        Subkey="Editor";
        break;
      case MACRO_DIALOG:
        Subkey="Dialog";
        break;
      case MACRO_SEARCH:
        Subkey="Search";
        break;
      case MACRO_DISKS:
        Subkey="Disks";
        break;
      case MACRO_MAINMENU:
        Subkey="MainMenu";
        break;
      case MACRO_HELP:
        Subkey="Help";
        break;
      case MACRO_OTHER:
        Subkey="Other";
        break;
    }
    char UpKeyName[100];
    sprintf(UpKeyName,"KeyMacros\\%s",Subkey);
    if (!EnumRegKey(UpKeyName,I,RegKeyName,sizeof(RegKeyName)))
      break;
    char *KeyNamePtr=strrchr(RegKeyName,'\\');
    if (KeyNamePtr!=NULL)
      strcpy(KeyText,KeyNamePtr+1);
    else
      *KeyText=0;
    int KeyCode=-1;
    for (int J=0;J<sizeof(KeyNames)/sizeof(KeyNames[0]);J++)
      if (stricmp(KeyText,KeyNames[J])==0)
      {
        KeyCode=J;
        break;
      }
    if (KeyCode==-1)
      continue;
    Macros=(struct MacroRecord *)realloc(Macros,sizeof(*Macros)*(MacrosNumber+1));
    if (Macros==NULL)
    {
      MacrosNumber=0;
      break;
    }
    struct MacroRecord *CurMacro=&Macros[MacrosNumber];
    CurMacro->Key=KeyCode;
    CurMacro->Buffer=NULL;
    CurMacro->BufferSize=0;
    CurMacro->Mode=ReadMode;
    GetRegKey(RegKeyName,"Sequence",Buffer,"",sizeof(Buffer));
    CurMacro->DisableOutput=GetRegKey(RegKeyName,"DisableOutput",0);
    CurMacro->RunAfterStart=GetRegKey(RegKeyName,"RunAfterFARStart",0);
    CurMacro->EmptyCommandLine=GetRegKey(RegKeyName,"EmptyCommandLine",0);
    CurMacro->NotEmptyCommandLine=GetRegKey(RegKeyName,"NotEmptyCommandLine",0);
    char *BufPtr=Buffer;
    while (1)
    {
      while (isspace(*BufPtr))
        BufPtr++;
      if (*BufPtr==0)
        break;
      char *CurBufPtr=BufPtr;
      while (*BufPtr && !isspace(*BufPtr))
        BufPtr++;
      int Length=BufPtr-CurBufPtr;
      char CurKeyText[50];
      memcpy(CurKeyText,CurBufPtr,Length);
      CurKeyText[Length]=0;
      int KeyCode=-1;
      for (int J=0;J<sizeof(KeyNames)/sizeof(KeyNames[0]);J++)
        if (Length==1 && strcmp(CurKeyText,KeyNames[J])==0 ||
            Length>1 && stricmp(CurKeyText,KeyNames[J])==0)
        {
          KeyCode=J;
          break;
        }
      if (KeyCode!=-1)
      {
        CurMacro->Buffer=(int *)realloc(CurMacro->Buffer,sizeof(*CurMacro->Buffer)*(CurMacro->BufferSize+1));
        if (CurMacro->Buffer==NULL)
          return;
        CurMacro->Buffer[CurMacro->BufferSize]=KeyCode;
        CurMacro->BufferSize++;
      }
    }
    MacrosNumber++;
  }
}


int KeyMacro::GetMacroSettings(int &DisableOutput,int &RunAfterStart,
              int &EmptyCommandLine,int &NotEmptyCommandLine)
{
  static struct DialogData MacroSettingsDlgData[]={
    DI_DOUBLEBOX,3,1,62,10,0,0,0,0,(char *)MMacroSettingsTitle,
    DI_CHECKBOX,5,2,0,0,1,1,0,0,(char *)MMacroSettingsDisableOutput,
    DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MMacroSettingsRunAfterStart,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_RADIOBUTTON,5,5,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnoreCommandLine,
    DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MMacroSettingsEmptyCommandLine,
    DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MMacroSettingsNotEmptyCommandLine,
    DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(MacroSettingsDlgData,MacroSettingsDlg);

  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]));
  Dlg.SetPosition(-1,-1,66,12);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  if (Dlg.GetExitCode()!=8)
    return(FALSE);
  DisableOutput=MacroSettingsDlg[1].Selected;
  RunAfterStart=MacroSettingsDlg[2].Selected;
  EmptyCommandLine=MacroSettingsDlg[5].Selected;
  NotEmptyCommandLine=MacroSettingsDlg[6].Selected;
  return(TRUE);
}


void KeyMacro::RunStartMacro()
{
  if (StartMacroPos==-1)
    return;
  while (StartMacroPos<MacrosNumber)
  {
    int CurPos=StartMacroPos++;
    if (Macros[CurPos].Mode==MACRO_SHELL && Macros[CurPos].BufferSize>0 &&
        Macros[CurPos].RunAfterStart)
    {
      int CmdLength=CtrlObject->CmdLine.GetLength();
      if (Macros[CurPos].EmptyCommandLine && CmdLength!=0 ||
          Macros[CurPos].NotEmptyCommandLine && CmdLength==0)
        return;
      if (Macros[CurPos].DisableOutput)
        LockScr=new LockScreen;
      Executing=TRUE;
      ExecMacroPos=CurPos;
      ExecKeyPos=0;
      return;
    }
  }
  StartMacroPos=-1;
}
