/*
keymix.cpp

Функций, имеющие отношение к клавитуре

*/

/* Revision: 1.01 26.12.2000 $ */

/*
Modify:
  26.12.2000 SVS
    + вызов KeyMacroToText() (функция определена в macro.cpp)
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля, после чего можно смело
      ваять интерфейс пвсевдоклавиатурного драйвера :-)
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

static int AltValue=0,ReturnAltValue;
static int ShiftPressedLast=FALSE,AltPressedLast=FALSE,CtrlPressedLast=FALSE;
static int RightAltPressedLast=FALSE,RightCtrlPressedLast=FALSE;
static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?
static clock_t PressedLastTime,KeyPressedLastTime;


/* tran 31.08.2000 $
  FarInputRecordToKey */
int WINAPI InputRecordToKey(INPUT_RECORD *r)
{
  if(r)
    return CalcKeyCode(r,TRUE);
  return KEY_NONE;
}
/* tran 31.08.2000 $ */

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!
*/
int WINAPI KeyNameToKey(char *Name)
{
   char KeyName[33];

   if(Name)
     for (int I=0; I < KEY_LAST_BASE;++I)
     {
       if(KeyToText(I,KeyName))
         if(!strcmp(Name,KeyName))
           return I;
     }
   return -1;
}
/* SVS $*/

int IsMouseButtonPressed()
{
  INPUT_RECORD rec;
  if (PeekInputRecord(&rec))
    GetInputRecord(&rec);
  Sleep(20);
  if (LButtonPressed)
    return(1);
  if (RButtonPressed)
    return(2);
  /* $ 23.08.2000 SVS
     + Дополнительно - для средней клавиши мыши
  */
  if(MButtonPressed)
    return(3);
  /* SVS $ */
  return(0);
}

int GetInputRecord(INPUT_RECORD *rec)
{
  static int LastEventIdle=FALSE;
  DWORD ReadCount;
  unsigned int LoopCount=0,CalcKey;
  unsigned int ReadKey=0;

  if (CtrlObject!=NULL)
  {
    int MacroKey=CtrlObject->Macro.GetKey();
    if (MacroKey)
    {
      ScrBuf.Flush();
      memset(rec,0,sizeof(*rec));
      return(MacroKey);
    }
    if (CtrlObject->ActivePanel!=NULL && !CmdMode)
      CtrlObject->Macro.RunStartMacro();
    MacroKey=CtrlObject->Macro.GetKey();
    if (MacroKey)
    {
      ScrBuf.Flush();
      memset(rec,0,sizeof(*rec));
      return(MacroKey);
    }
  }

  int EnableShowTime=Opt.Clock && (WaitInMainLoop || CtrlObject!=NULL &&
                     CtrlObject->Macro.GetMode()==MACRO_SEARCH);

  if (EnableShowTime)
    ShowTime(1);

  ScrBuf.Flush();

  if (!LastEventIdle)
    StartIdleTime=clock();
  LastEventIdle=FALSE;
  SetFarConsoleMode();
  while (1)
  {
    PeekConsoleInput(hConInp,rec,1,&ReadCount);

    if (ReadCount!=0)
      break;

    ScrBuf.Flush();

    Sleep(20);

    if (CloseFAR)
    {
      CloseFAR=FALSE;
      CtrlObject->ModalManager.IsAnyModalModified(TRUE);
    }

    if ((LoopCount & 15)==0)
    {
      clock_t CurTime=clock();
      clock_t TimeAfterExec=CurTime-StartExecTime;
      if (EnableShowTime)
        ShowTime(0);
      if (WaitInMainLoop)
      {
        if (Opt.InactivityExit && Opt.InactivityExitTime>0 &&
            CurTime-StartIdleTime>Opt.InactivityExitTime*60000 &&
            CtrlObject->ModalManager.GetModalCount()==0)
        {
          CtrlObject->ExitMainLoop(FALSE);
          return(KEY_NONE);
        }
        if ((LoopCount & 63)==0)
        {
          static int Reenter=0;
          if (!Reenter)
          {
            Reenter++;
            int X,Y;
            GetRealCursorPos(X,Y);
            if (X==0 && Y==ScrY && CtrlObject->CmdLine.IsVisible())
            {
              while (1)
              {
                INPUT_RECORD tmprec;
                int Key=GetInputRecord(&tmprec);
                if (Key==KEY_NONE || Key!=KEY_SHIFT && tmprec.Event.KeyEvent.bKeyDown)
                  break;
              }

              CtrlObject->SetScreenPositions();
              ScrBuf.ResetShadow();
              ScrBuf.Flush();
            }
            Reenter--;
          }
          static int UpdateReenter=0;
          if (!UpdateReenter && CurTime-KeyPressedLastTime>700)
          {
            UpdateReenter=TRUE;
            CtrlObject->LeftPanel->UpdateIfChanged();
            CtrlObject->RightPanel->UpdateIfChanged();
            UpdateReenter=FALSE;
          }
        }
      }
      if (StartExecTime!=0 && TimeAfterExec>2000)
      {
        StartExecTime=0;
        if (!IsWindowed() && !Opt.Mouse)
        {
          SetConsoleMode(hConInp,ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
          SetConsoleMode(hConInp,ENABLE_MOUSE_INPUT);
        }
        SetFarTitle(LastFarTitle);
      }
      if (Opt.ScreenSaver && Opt.ScreenSaverTime>0 &&
          CurTime-StartIdleTime>Opt.ScreenSaverTime*60000)
        if (!ScreenSaver(WaitInMainLoop))
          return(KEY_NONE);
      if (!WaitInMainLoop && LoopCount==64)
      {
        LastEventIdle=TRUE;
        memset(rec,0,sizeof(*rec));
        rec->EventType=KEY_EVENT;
        return(KEY_IDLE);
      }
    }
    LoopCount++;
  }
  clock_t CurClock=clock();
  if (rec->EventType==FOCUS_EVENT)
  {
    ShiftPressedLast=FALSE;
    CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressedLast=RightAltPressedLast=FALSE;
    PressedLastTime=0;
  }
  if (rec->EventType==KEY_EVENT)
  {
    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;
    if (AltPressed && (CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
      DetectWindowedMode();
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    KeyPressedLastTime=CurClock;

    /* $ 24.08.2000 SVS
       + Добавление на реакцию KEY_CTRLALTSHIFTRELEASE
    */
    if(IsKeyCASPressed && (!CtrlPressed || !AltPressed || !ShiftPressed))
    {
      IsKeyCASPressed=FALSE;
      return(KEY_CTRLALTSHIFTRELEASE);
    }
    /* SVS $ */
  }

  ReturnAltValue=FALSE;
  CalcKey=CalcKeyCode(rec,TRUE);
  if (ReturnAltValue)
  {
    if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
      CalcKey=KEY_NONE;
    return(CalcKey);
  }
  int GrayKey=(CalcKey==KEY_ADD || CalcKey==KEY_SUBTRACT || CalcKey==KEY_MULTIPLY);
  if ((CalcKey>=' ' && CalcKey<256 || CalcKey==KEY_BS || GrayKey) &&
      CalcKey!=KEY_DEL && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
  {
    ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
    if (ReadKey==13 && CalcKey!=KEY_ENTER)
      ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
    rec->Event.KeyEvent.uChar.AsciiChar=ReadKey;
  }
  else
    ReadConsoleInput(hConInp,rec,1,&ReadCount);

  if (EnableShowTime)
    ShowTime(1);

  if (rec->EventType==WINDOW_BUFFER_SIZE_EVENT)
  {
    if (WaitInMainLoop)
    {
      GetVideoMode();
      CtrlObject->SetScreenPositions();
    }
    /* $ 28.06.2000 tran
       NT Console resize support for Editor, Viewer, Help */
    else
    {
      GetVideoMode();
      Modal * CurModal=CtrlObject->ModalManager.ActiveModal;
      if (CurModal)
      {
        /* 06.07.2000 SVS
          Временная отмена патча 11 (NT Console resize bug) до лучших времен :-)
        */
        // CtrlObject->SetScreenPositions();
        // CurModal->SetScreenPosition();
        /* SVS $ */
      }
    }
    /* tran $ */
    return(KEY_NONE);
  }

  if (rec->EventType==KEY_EVENT)
  {
    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;
    DWORD KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    if (KeyCode==VK_F16 && ReadKey==VK_F16 || KeyCode==0)
      return(KEY_NONE);

    if (!rec->Event.KeyEvent.bKeyDown &&
        (KeyCode==VK_SHIFT || KeyCode==VK_CONTROL || KeyCode==VK_MENU) &&
        CurClock-PressedLastTime<500)
    {
      int Key=-1;
      if (ShiftPressedLast && KeyCode==VK_SHIFT)
        Key=KEY_SHIFT;
      if (KeyCode==VK_CONTROL)
        if (CtrlPressedLast)
          Key=KEY_CTRL;
        else
          if (RightCtrlPressedLast)
            Key=KEY_RCTRL;
      if (KeyCode==VK_MENU)
        if (AltPressedLast)
          Key=KEY_ALT;
        else
          if (RightAltPressedLast)
            Key=KEY_RALT;
      if (Key!=-1 && CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(Key))
        Key=KEY_NONE;
      if (Key!=-1)
        return(Key);
    }

    ShiftPressedLast=FALSE;
    CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressedLast=RightAltPressedLast=FALSE;

    ShiftPressedLast=(KeyCode==VK_SHIFT && rec->Event.KeyEvent.bKeyDown) ||
         (KeyCode==VK_RETURN && ShiftPressed && !rec->Event.KeyEvent.bKeyDown);

    if (!ShiftPressedLast)
      if (KeyCode==VK_CONTROL && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_CTRL_PRESSED)
          RightCtrlPressedLast=TRUE;
        else
          CtrlPressedLast=TRUE;
      }

    if (!ShiftPressedLast && !CtrlPressedLast && !RightCtrlPressedLast)
    {
      if (KeyCode==VK_MENU && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_ALT_PRESSED)
          RightAltPressedLast=TRUE;
        else
          AltPressedLast=TRUE;
        PressedLastTime=CurClock;
      }
    }
    else
      PressedLastTime=CurClock;

    if (KeyCode==VK_SHIFT || KeyCode==VK_MENU || KeyCode==VK_CONTROL ||
        KeyCode==VK_NUMLOCK || KeyCode==VK_SCROLL)
    {
      /* $ 24.08.2000 SVS
         + Добавление на реакцию KEY_CTRLALTSHIFTPRESS
      */
      switch(KeyCode)
      {
        case VK_SHIFT:
        case VK_MENU:
        case VK_CONTROL:
          if(!IsKeyCASPressed && CtrlPressed && AltPressed && ShiftPressed)
          {
            IsKeyCASPressed=TRUE;
            return (KEY_CTRLALTSHIFTPRESS);
          }
      }
      /* SVS $ */
      return(KEY_NONE);
    }
    Panel::EndDrag();
  }
  if (rec->EventType==MOUSE_EVENT)
  {
    // проверка на Swap клавиш мыши
    static int SwapButton=GetSystemMetrics(SM_SWAPBUTTON);
    if (SwapButton && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && !IsWindowed())
    {
      DWORD CtrlState=rec->Event.MouseEvent.dwButtonState;
      if (CtrlState & FROM_LEFT_1ST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=RIGHTMOST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~RIGHTMOST_BUTTON_PRESSED;
      if (CtrlState & RIGHTMOST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=FROM_LEFT_1ST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~FROM_LEFT_1ST_BUTTON_PRESSED;
    }
    DWORD CtrlState=rec->Event.MouseEvent.dwButtonState;
    /* $ 23.08.2000 SVS
       + Дополнительно - для средней клавиши мыши
    */
    MButtonPressed=(CtrlState & FROM_LEFT_2ND_BUTTON_PRESSED);
    /* SVS $ */
    LButtonPressed=(CtrlState & FROM_LEFT_1ST_BUTTON_PRESSED);
    RButtonPressed=(CtrlState & RIGHTMOST_BUTTON_PRESSED);
    PrevMouseX=MouseX;
    PrevMouseY=MouseY;
    MouseX=rec->Event.MouseEvent.dwMousePosition.X;
    MouseY=rec->Event.MouseEvent.dwMousePosition.Y;
  }
  if (ReadKey!=0 && !GrayKey)
    CalcKey=ReadKey;

  if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
    CalcKey=KEY_NONE;

  return(CalcKey);
}

int PeekInputRecord(INPUT_RECORD *rec)
{
  DWORD ReadCount;
  ScrBuf.Flush();
  PeekConsoleInput(hConInp,rec,1,&ReadCount);
  if (ReadCount==0)
    return(0);
  return(CalcKeyCode(rec,TRUE));
}


/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
     Если KeyWait = -1 - как и раньше
*/
void WaitKey(int KeyWait)
{
  while (1)
  {
    INPUT_RECORD rec;
    int Key=GetInputRecord(&rec);
    if(KeyWait == -1)
    {
      if (Key!=KEY_NONE && Key!=KEY_IDLE)
        break;
    }
    else if(Key == KeyWait)
      break;
  }
}
/* SVS $ */


void WriteInput(int Key)
{
  INPUT_RECORD rec;
  DWORD WriteCount;
  rec.EventType=KEY_EVENT;
  rec.Event.KeyEvent.bKeyDown=1;
  rec.Event.KeyEvent.wRepeatCount=1;
  rec.Event.KeyEvent.wVirtualKeyCode=rec.Event.KeyEvent.wVirtualScanCode=Key;
  if (Key>255)
    Key=0;
  rec.Event.KeyEvent.uChar.UnicodeChar=rec.Event.KeyEvent.uChar.AsciiChar=Key;
  rec.Event.KeyEvent.dwControlKeyState=0;
  WriteConsoleInput(hConInp,&rec,1,&WriteCount);
}


int CheckForEsc()
{
  INPUT_RECORD rec;
  int Key;
  if (!CtrlObject->Macro.IsExecuting() && PeekInputRecord(&rec) &&
      ((Key=GetInputRecord(&rec))==KEY_ESC || Key==KEY_BREAK))
    return(TRUE);
  return(FALSE);
}


/* $ 25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
*/
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
   Size=0 - по максимуму!
*/
/* $ 10.09.2000 SVS
  ! KeyToText возвращает BOOL
*/
BOOL WINAPI KeyToText(int Key0,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;
  int I;
  char KeyText[32];
  int fmtNum, Key=Key0;
  char *fmtKey[]={
  /* 00 */ "F%d",
  /* 01 */ "CtrlF%d",
  /* 02 */ "AltF%d",
  /* 03 */ "ShiftF%d",
  /* 04 */ "Ctrl%c",
  /* 05 */ "RCtrl%c",
  /* 06 */ "CtrlShiftF%d",
  /* 07 */ "AltShiftF%d",
  /* 08 */ "CtrlAltF%d",
  /* 09 */ "CtrlShift%c",
  /* 10 */ "AltShift%c",
  /* 11 */ "CtrlAlt%c",
  /* 12 */ "Alt%c",
  /* 13 */ "%s",
  /* 14 */ "%c",
  };

  if (Key>=KEY_F1 && Key<=KEY_F12)
  {
    fmtNum=0;
    Key=Key0-KEY_F1+1;
  }
  else if (Key>=KEY_CTRLF1 && Key<=KEY_CTRLF12)
  {
    fmtNum=1;
    Key=Key0-KEY_CTRLF1+1;
  }
  else if (Key>=KEY_ALTF1 && Key<=KEY_ALTF12)
  {
    fmtNum=2;
    Key=Key0-KEY_ALTF1+1;
  }
  else if (Key>=KEY_SHIFTF1 && Key<=KEY_SHIFTF12)
  {
    fmtNum=3;
    Key=Key0-KEY_SHIFTF1+1;
  }
  else if (Key>=KEY_CTRLA && Key<=KEY_CTRLZ)
  {
    fmtNum=4;
    Key=Key0-KEY_CTRLA+'A';
  }
  else if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    fmtNum=4;
    Key=Key0-KEY_CTRL0+'0';
  }
  else if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    fmtNum=5;
    Key=Key0-KEY_RCTRL0+'0';
  }
  else if (Key>=KEY_CTRLSHIFTF1 && Key<=KEY_CTRLSHIFTF12)
  {
    fmtNum=6;
    Key=Key0-KEY_CTRLSHIFTF1+1;
  }
  else if (Key>=KEY_ALTSHIFTF1 && Key<=KEY_ALTSHIFTF12)
  {
    fmtNum=7;
    Key=Key0-KEY_ALTSHIFTF1+1;
  }
  else if (Key>=KEY_CTRLALTF1 && Key<=KEY_CTRLALTF12)
  {
    fmtNum=8;
    Key=Key0-KEY_CTRLALTF1+1;
  }
  else if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
  {
    fmtNum=9;
    Key=Key0-KEY_CTRLSHIFT0+'0';
  }
  else if (Key>=KEY_CTRLSHIFTA && Key<=KEY_CTRLSHIFTZ)
  {
    fmtNum=9;
    Key=Key0-KEY_CTRLSHIFTA+'A';
  }
  else if (Key>=KEY_ALTSHIFTA && Key<=KEY_ALTSHIFTZ)
  {
    fmtNum=10;
    Key=Key0-KEY_ALTSHIFTA+'A';
  }
  else if (Key>=KEY_CTRLALTA && Key<=KEY_CTRLALTZ)
  {
    fmtNum=11;
    Key=Key0-KEY_CTRLALTA+'A';
  }
  else if (Key>=KEY_ALT0 && Key<=KEY_ALT9)
  {
    fmtNum=12;
    Key=Key0-KEY_ALT0+'0';
  }
  else if (Key>=KEY_ALTA && Key<=KEY_ALTZ)
  {
    fmtNum=12;
    Key=Key0-KEY_ALTA+'A';
  }
  else
  {
    /* $ 23.07.2000 SVS
       + KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
    */
    /* $ 08.09.2000 SVS
       + KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL
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
      KEY_LWIN,KEY_RWIN,
      KEY_CTRLALTSHIFTPRESS,KEY_CTRLALTSHIFTRELEASE,
      KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL,
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
      "LWin","RWin",
      "CtrlAltShiftPress","CtrlAltShiftRelease",
      "CtrlShiftDel", "AltShiftDel",
    };
    /* SVS 08.09.2000 $ */
    /* SVS $ */

    for (I=0;I<sizeof(KeyCodes)/sizeof(KeyCodes[0]);I++)
      if (Key==KeyCodes[I])
      {
        strcpy(KeyText,KeyNames[I]);
        break;
      }
    if(I  == sizeof(KeyCodes)/sizeof(KeyCodes[0]))
    {
      if(KeyMacroToText(Key,KeyText,Size))
         Key=-1;
      else if (Key<256)
      {
        fmtNum=14;
        Key=Key0;
      }
      else if (Key>KEY_CTRL_BASE && Key<KEY_END_CTRL_BASE)
      {
        fmtNum=4;
        Key=Key0-KEY_CTRL_BASE;
      }
      else if (Key>KEY_ALT_BASE && Key<KEY_END_ALT_BASE)
      {
        fmtNum=12;
        Key=Key0-KEY_ALT_BASE;
      }
      else if (Key>KEY_CTRLSHIFT_BASE && Key<KEY_END_CTRLSHIFT_BASE)
      {
        fmtNum=9;
        Key=Key0-KEY_CTRLSHIFT_BASE;
      }
      else if (Key>KEY_ALTSHIFT_BASE && Key<KEY_END_ALTSHIFT_BASE)
      {
        fmtNum=10;
        Key=Key0-KEY_ALTSHIFT_BASE;
      }
      else if (Key>KEY_CTRLALT_BASE && Key<KEY_END_CTRLALT_BASE)
      {
        fmtNum=11;
        Key=Key0-KEY_CTRLALT_BASE;
      }
      else
      {
        Key=-1;
        *KeyText=0;
      }
    }
    else
      Key=-1;
  }
  if(Key != -1)
  {
    sprintf(KeyText,fmtKey[fmtNum],Key);
    for (I=0;KeyText[I]!=0;I++)
      if (KeyText[I]=='\\')
      {
        strcpy(KeyText+I,"BackSlash");
        break;
      }
  }

  if(!KeyText[0])
  {
    *KeyText0='\0';
    return FALSE;
  }
  if(Size > 0)
    strncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);
  return TRUE;
}
/* SVS 10.09.2000 $ */
/* SVS $ */

int CalcKeyCode(INPUT_RECORD *rec,int RealKey)
{
  unsigned int ScanCode,KeyCode,CtrlState,AsciiChar;
  CtrlState=rec->Event.KeyEvent.dwControlKeyState;
  ScanCode=rec->Event.KeyEvent.wVirtualScanCode;
  KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
  AsciiChar=rec->Event.KeyEvent.uChar.AsciiChar;

  if (!RealKey)
  {
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
  }

  if (rec->EventType!=KEY_EVENT)
    return(KEY_NONE);
  if (!rec->Event.KeyEvent.bKeyDown)
    if (KeyCode==VK_MENU && AltValue!=0)
    {
      FlushInputBuffer();
      ReturnAltValue=TRUE;
      AltValue&=255;
      rec->Event.KeyEvent.uChar.AsciiChar=AltValue;
      return(AltValue);
    }
    else
      return(KEY_NONE);

  if ((CtrlState & 9)==9)
    if (AsciiChar!=0)
      return(AsciiChar);
    else
      CtrlPressed=0;

  if (Opt.AltGr && CtrlPressed && (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED))
    if (AsciiChar=='\\')
      return(KEY_CTRLBACKSLASH);

  if (KeyCode==VK_MENU)
    AltValue=0;

  if (AsciiChar==0 && !CtrlPressed && !AltPressed)
  {
    if (KeyCode==0xc0)
      return(ShiftPressed ? '~':'`');
    if (KeyCode==0xde)
      return(ShiftPressed ? '"':'\'');
  }

  if (AsciiChar<32 && (CtrlPressed || AltPressed))
  {
    switch(KeyCode)
    {
      case 0xbc:
        AsciiChar=',';
        break;
      case 0xbe:
        AsciiChar='.';
        break;
      case 0xdb:
        AsciiChar='[';
        break;
      case 0xdc:
        AsciiChar='\\';
        break;
      case 0xdd:
        AsciiChar=']';
        break;
      case 0xde:
        AsciiChar='\"';
        break;
    }
  }

  /* $ 24.08.2000 SVS
     "Персональные 100 грамм" :-)
  */
  if(CtrlPressed && AltPressed && ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_SHIFT:
      case VK_MENU:
      case VK_CONTROL:
        return (IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE);
    }
  }
  /* SVS $*/

  if (CtrlPressed && AltPressed)
  {
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_CTRLALTF1+KeyCode-VK_F1);
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLALTA+KeyCode-'A');
    switch(KeyCode)
    {
      case VK_INSERT:
      case VK_NUMPAD0:
        return(KEY_CTRLALTINS);
      case VK_DOWN:
      case VK_NUMPAD2:
        return(KEY_CTRLALTDOWN);
      case VK_LEFT:
      case VK_NUMPAD4:
        return(KEY_CTRLALTLEFT);
      case VK_RIGHT:
      case VK_NUMPAD6:
        return(KEY_CTRLALTRIGHT);
      case VK_UP:
      case VK_NUMPAD8:
        return(KEY_CTRLALTUP);
      case VK_END:
      case VK_NUMPAD1:
        return(KEY_CTRLALTEND);
      case VK_HOME:
      case VK_NUMPAD7:
        return(KEY_CTRLALTHOME);
      case VK_NEXT:
      case VK_NUMPAD3:
        return(KEY_CTRLALTPGDN);
      case VK_PRIOR:
      case VK_NUMPAD9:
        return(KEY_CTRLALTPGUP);
      case VK_RETURN:
        return(KEY_CTRLALTENTER);
      case VK_APPS:
        return(KEY_CTRLALTAPPS);
    }
    if (AsciiChar)
      return(KEY_CTRLALT_BASE+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRLALT_BASE+KeyCode);
  }

  if (AltPressed && ShiftPressed)
  {
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_ALTSHIFTF1+KeyCode-VK_F1);
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_ALTSHIFTA+KeyCode-'A');
    switch(KeyCode)
    {
      case VK_INSERT:
      case VK_NUMPAD0:
        return(KEY_ALTSHIFTINS);
      case VK_DOWN:
      case VK_NUMPAD2:
        return(KEY_ALTSHIFTDOWN);
      case VK_LEFT:
      case VK_NUMPAD4:
        return(KEY_ALTSHIFTLEFT);
      case VK_RIGHT:
      case VK_NUMPAD6:
        return(KEY_ALTSHIFTRIGHT);
      case VK_UP:
      case VK_NUMPAD8:
        return(KEY_ALTSHIFTUP);
      case VK_END:
      case VK_NUMPAD1:
        return(KEY_ALTSHIFTEND);
      case VK_HOME:
      case VK_NUMPAD7:
        return(KEY_ALTSHIFTHOME);
      case VK_NEXT:
      case VK_NUMPAD3:
        return(KEY_ALTSHIFTPGDN);
      case VK_PRIOR:
      case VK_NUMPAD9:
        return(KEY_ALTSHIFTPGUP);
      case VK_RETURN:
        return(KEY_ALTSHIFTENTER);
      case VK_APPS:
        return(KEY_ALTSHIFTAPPS);
      /* $ 08.09.2000 SVS
         + KEY_ALTSHIFTDEL
      */
      case VK_DELETE:
      case VK_DECIMAL:
        return(KEY_ALTSHIFTDEL);
      /* SVS $ */
    }
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
      return(KEY_ALTSHIFT_BASE+AsciiChar);
    }
    if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_ALTSHIFT_BASE+KeyCode);
  }

  if (CtrlPressed && ShiftPressed)
  {
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_CTRLSHIFTF1+KeyCode-VK_F1);
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRLSHIFT0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLSHIFTA+KeyCode-'A');
    switch(KeyCode)
    {
      case VK_INSERT:
      case VK_NUMPAD0:
        return(KEY_CTRLSHIFTINS);
      case VK_DOWN:
      case VK_NUMPAD2:
        return(KEY_CTRLSHIFTDOWN);
      case VK_LEFT:
      case VK_NUMPAD4:
        return(KEY_CTRLSHIFTLEFT);
      case VK_RIGHT:
      case VK_NUMPAD6:
        return(KEY_CTRLSHIFTRIGHT);
      case VK_UP:
      case VK_NUMPAD8:
        return(KEY_CTRLSHIFTUP);
      case VK_END:
      case VK_NUMPAD1:
        return(KEY_CTRLSHIFTEND);
      case VK_HOME:
      case VK_NUMPAD7:
        return(KEY_CTRLSHIFTHOME);
      case VK_NEXT:
      case VK_NUMPAD3:
        return(KEY_CTRLSHIFTPGDN);
      case VK_PRIOR:
      case VK_NUMPAD9:
        return(KEY_CTRLSHIFTPGUP);
      case VK_SUBTRACT:
        return(KEY_CTRLSHIFTSUBTRACT);
      case VK_ADD:
        return(KEY_CTRLSHIFTADD);
      case VK_RETURN:
        return(KEY_CTRLSHIFTENTER);
      case 0xbe:
        return(KEY_CTRLSHIFTDOT);
      case 0xdb:
        return(KEY_CTRLSHIFTBRACKET);
      case 0xdd:
        return(KEY_CTRLSHIFTBACKBRACKET);
      case 0xbf:
        return(KEY_CTRLSHIFTSLASH);
      case 0xdc:
        return(KEY_CTRLSHIFTBACKSLASH);
      case VK_TAB:
        return(KEY_CTRLSHIFTTAB);
      case VK_BACK:
        return(KEY_CTRLSHIFTBS);
      case VK_APPS:
        return(KEY_CTRLSHIFTAPPS);
      /* $ 08.09.2000 SVS
         + KEY_CTRLSHIFTDEL
      */
      case VK_DELETE:
      case VK_DECIMAL:
        return(KEY_CTRLSHIFTDEL);
      /* SVS $ */
    }
    if (AsciiChar)
      return(KEY_CTRLSHIFT_BASE+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRLSHIFT_BASE+KeyCode);
  }

  if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_RCTRL0+KeyCode-'0');
  }

  if (!CtrlPressed && !AltPressed && !ShiftPressed)
  {
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_F1+KeyCode-VK_F1);

    switch(KeyCode)
    {
      case VK_ESCAPE:
        return(KEY_ESC);
      case VK_BACK:
        return(KEY_BS);
      case VK_TAB:
        return(KEY_TAB);
      case VK_RETURN:
        return(KEY_ENTER);
      case VK_SPACE:
        return(KEY_SPACE);
      case VK_LEFT:
        return(KEY_LEFT);
      case VK_RIGHT:
        return(KEY_RIGHT);
      case VK_UP:
        return(KEY_UP);
      case VK_DOWN:
        return(KEY_DOWN);
      case VK_HOME:
        return(KEY_HOME);
      case VK_END:
        return(KEY_END);
      case VK_NEXT:
        return(KEY_PGDN);
      case VK_PRIOR:
        return(KEY_PGUP);
      case VK_INSERT:
        return(KEY_INS);
      case VK_CLEAR:
        return(KEY_NUMPAD5);
      case VK_DECIMAL:
        return (CtrlState & NUMLOCK_ON) ? '.':KEY_DEL;
      case VK_DELETE:
        if ((CtrlState & NUMLOCK_ON) && (CtrlState & ENHANCED_KEY)==0 &&
            AsciiChar=='.')
          return('.');
        return(KEY_DEL);
      case VK_ADD:
        return(KEY_ADD);
      case VK_SUBTRACT:
        return(KEY_SUBTRACT);
      case VK_MULTIPLY:
        return(KEY_MULTIPLY);
      case VK_DIVIDE:
        return(KEY_DIVIDE);
      case VK_APPS:
        return(KEY_APPS);
      case VK_LWIN:
        return(KEY_LWIN);
      case VK_RWIN:
        return(KEY_RWIN);
      case KEY_BREAK:
        return(KEY_BREAK);
    }
  }


  if (CtrlPressed)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRL0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLA+KeyCode-'A');
    switch(KeyCode)
    {
      case VK_TAB:
        return(KEY_CTRLTAB);
      case VK_BACK:
        return(KEY_CTRLBS);
      case VK_RETURN:
        return(KEY_CTRLENTER);
      case VK_INSERT:
      case VK_NUMPAD0:
        return(KEY_CTRLINS);
      case VK_DELETE:
      case VK_DECIMAL:
        return(KEY_CTRLDEL);
      case 0xbc:
        return(KEY_CTRLCOMMA);
      case 0xbe:
        return(KEY_CTRLDOT);
      case 0xbf:
        return(KEY_CTRLSLASH);
      case 0xdb:
        return(KEY_CTRLBRACKET);
      case 0xdc:
        return(KEY_CTRLBACKSLASH);
      case 0xdd:
        return(KEY_CTRLBACKBRACKET);
      case 0xde:
        return(KEY_CTRLQUOTE);
    }

    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_CTRLF1+KeyCode-VK_F1);
    else
      switch(KeyCode)
      {
        case VK_DOWN:
        case VK_NUMPAD2:
          return(KEY_CTRLDOWN);
        case VK_LEFT:
        case VK_NUMPAD4:
          return(KEY_CTRLLEFT);
        case VK_CLEAR:
        case VK_NUMPAD5:
          return(KEY_CTRLCLEAR);
        case VK_RIGHT:
        case VK_NUMPAD6:
          return(KEY_CTRLRIGHT);
        case VK_UP:
        case VK_NUMPAD8:
          return(KEY_CTRLUP);
        case VK_END:
        case VK_NUMPAD1:
          return(KEY_CTRLEND);
        case VK_HOME:
        case VK_NUMPAD7:
          return(KEY_CTRLHOME);
        case VK_NEXT:
        case VK_NUMPAD3:
          return(KEY_CTRLPGDN);
        case VK_PRIOR:
        case VK_NUMPAD9:
          return(KEY_CTRLPGUP);
        case VK_SUBTRACT:
          return(KEY_CTRLSUBTRACT);
        case VK_ADD:
          return(KEY_CTRLADD);
        case VK_MULTIPLY:
          return(KEY_CTRLMULTIPLY);
        case VK_APPS:
          return(KEY_CTRLAPPS);
      }
    if (KeyCode)
    {
      if (!RealKey && KeyCode==VK_CONTROL)
        return(KEY_NONE);
      return(KEY_CTRL_BASE+KeyCode);
    }
  }
  if (AltPressed)
  {
    static int InGrabber=FALSE;
    if (AltValue==0 && !InGrabber && (KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0))
    {
      InGrabber=TRUE;
      WaitInMainLoop=FALSE;
      Grabber Grabber;
      InGrabber=FALSE;
      return(KEY_NONE);
    }
    static int ScanCodes[]={82,79,80,81,75,76,77,71,72,73};
    for (int I=0;I<sizeof(ScanCodes)/sizeof(ScanCodes[0]);I++)
      if (ScanCodes[I]==ScanCode && (CtrlState & ENHANCED_KEY)==0)
      {
        if (RealKey)
          AltValue=AltValue*10+I;
        return(KEY_NONE);
      }
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_ALTF1+KeyCode-VK_F1);
    else
      switch(KeyCode)
      {
        case VK_BACK:
          return(KEY_ALTBS);
        case VK_INSERT:
        case VK_NUMPAD0:
          return(KEY_ALTINS);
        case VK_DELETE:
        case VK_DECIMAL:
          return(KEY_ALTDEL);
        case 0xbc:
          return(KEY_ALTCOMMA);
        case 0xbe:
          return(KEY_ALTDOT);
        case VK_ADD:
          return(KEY_ALTADD);
        case VK_SUBTRACT:
          return(KEY_ALTSUBTRACT);
        case VK_MULTIPLY:
          return(KEY_ALTMULTIPLY);
        case VK_HOME:
          return(KEY_ALTHOME);
        case VK_END:
          return(KEY_ALTEND);
        case VK_PRIOR:
          return(KEY_ALTPGUP);
        case VK_NEXT:
          return(KEY_ALTPGDN);
        case VK_UP:
          return(KEY_ALTUP);
        case VK_DOWN:
          return(KEY_ALTDOWN);
        case VK_LEFT:
          return(KEY_ALTLEFT);
        case VK_RIGHT:
          return(KEY_ALTRIGHT);
        case VK_APPS:
          return(KEY_ALTAPPS);
    }
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
      return(LocalUpper(AsciiChar)+KEY_ALT_BASE);
    }
    if (KeyCode==VK_CAPITAL)
      return(KEY_NONE);
    if (!RealKey && KeyCode==VK_MENU)
      return(KEY_NONE);
    return(KEY_ALT_BASE+KeyCode);
  }
  if (ShiftPressed)
  {
    if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
      return(KEY_SHIFTF1+KeyCode-VK_F1);
    else
      switch(KeyCode)
      {
        case VK_TAB:
          return(KEY_SHIFTTAB);
        case VK_RETURN:
          if (RealKey && !ShiftPressedLast)
            return(KEY_ENTER);
          return(KEY_SHIFTENTER);
        case VK_BACK:
          return(KEY_SHIFTBS);
        case VK_INSERT:
        case VK_NUMPAD0:
          return(KEY_SHIFTINS);
        case VK_DELETE:
        case VK_DECIMAL:
          return(KEY_SHIFTDEL);
        case VK_END:
        case VK_NUMPAD1:
          return(KEY_SHIFTEND);
        case VK_DOWN:
        case VK_NUMPAD2:
          return(KEY_SHIFTDOWN);
        case VK_NEXT:
        case VK_NUMPAD3:
          return(KEY_SHIFTPGDN);
        case VK_LEFT:
        case VK_NUMPAD4:
          return(KEY_SHIFTLEFT);
        case VK_RIGHT:
        case VK_NUMPAD6:
          return(KEY_SHIFTRIGHT);
        case VK_HOME:
        case VK_NUMPAD7:
          return(KEY_SHIFTHOME);
        case VK_UP:
        case VK_NUMPAD8:
          return(KEY_SHIFTUP);
        case VK_PRIOR:
        case VK_NUMPAD9:
          return(KEY_SHIFTPGUP);
        case VK_SUBTRACT:
          return(KEY_SHIFTSUBTRACT);
        case VK_ADD:
          return(KEY_SHIFTADD);
        case VK_APPS:
          return(KEY_SHIFTAPPS);
      }
  }
  if (AsciiChar)
    return(AsciiChar);
  return(KEY_NONE);
}
