/*
keyboard.cpp

Функций, имеющие отношение к клавитуре

*/

/* Revision: 1.03 05.01.2001 $ */

/*
Modify:
  05.01.2001 SVS
    - База в "вычислителе" клавиш :-(
  04.01.2001 SVS
    + Переделка алгоритмов декодирования клавиш...
    + TranslateKeyToVK
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

/* ----------------------------------------------------------------- */
static struct TTable_KeyToVK{
  int Key;
  int VK;
} Table_KeyToVK[]={
   {KEY_BREAK,         VK_CANCEL},
   {KEY_BS,            VK_BACK},
   {KEY_TAB,           VK_TAB},
   {KEY_NUMPAD5,       VK_CLEAR},
   {KEY_ENTER,         VK_RETURN},

   {KEY_ESC,           VK_ESCAPE},

   {KEY_SPACE,         VK_SPACE},
   {KEY_PGUP,          VK_PRIOR},
   {KEY_PGDN,          VK_NEXT},
   {KEY_END,           VK_END},
   {KEY_HOME,          VK_HOME},
   {KEY_LEFT,          VK_LEFT},
   {KEY_UP,            VK_UP},
   {KEY_RIGHT,         VK_RIGHT},
   {KEY_DOWN,          VK_DOWN},

   {KEY_INS,           VK_INSERT},
   {KEY_DEL,           VK_DELETE},

   {KEY_LWIN,          VK_LWIN},
   {KEY_RWIN,          VK_RWIN},
   {KEY_APPS,          VK_APPS},

   {KEY_MULTIPLY,      VK_MULTIPLY},
   {KEY_ADD,           VK_ADD},
   {KEY_SUBTRACT,      VK_SUBTRACT},
   {KEY_DIVIDE,        VK_DIVIDE},

   {KEY_F1,            VK_F1},
   {KEY_F2,            VK_F2},
   {KEY_F3,            VK_F3},
   {KEY_F4,            VK_F4},
   {KEY_F5,            VK_F5},
   {KEY_F6,            VK_F6},
   {KEY_F7,            VK_F7},
   {KEY_F8,            VK_F8},
   {KEY_F9,            VK_F9},
   {KEY_F10,           VK_F10},
   {KEY_F11,           VK_F11},
   {KEY_F12,           VK_F12},
};


struct TFKey3{
  DWORD Key;
  int   Len;
  char *Name;
};

static struct TFKey3 FKeys1[]={
  { KEY_CTRLALTSHIFTRELEASE, 19, "CtrlAltShiftRelease"},
  { KEY_CTRLALTSHIFTPRESS,   17, "CtrlAltShiftPress"},
  { KEY_BACKSLASH,            9, "BackSlash"},
  { KEY_SUBTRACT,             8, "Subtract"},
  { KEY_MULTIPLY,             8, "Multiply"},
  { KEY_DIVIDE,               6, "Divide"},
  { KEY_BREAK,                5, "Break"},
  { KEY_CLEAR,                5, "Clear"},
  { KEY_NUMPAD5,              5, "Clear"},
  { KEY_RIGHT,                5, "Right"},
  { KEY_ENTER,                5, "Enter"},
  { KEY_SPACE,                5, "Space"},
  { KEY_PGUP,                 4, "PgUp"},
  { KEY_HOME,                 4, "Home"},
  { KEY_LEFT,                 4, "Left"},
  { KEY_DOWN,                 4, "Down"},
  { KEY_PGDN,                 4, "PgDn"},
  { KEY_APPS,                 4, "Apps"},
  { KEY_LWIN,                 4 ,"LWin"},
  { KEY_RWIN,                 4 ,"RWin"},
  { KEY_END,                  3, "End"},
  { KEY_INS,                  3, "Ins"},
  { KEY_DEL,                  3, "Del"},
  { KEY_ADD,                  3, "Add"},
  { KEY_F10,                  3, "F10"},
  { KEY_F11,                  3, "F11"},
  { KEY_F12,                  3, "F12"},
  { KEY_TAB,                  3, "Tab"},
  { KEY_ESC,                  3, "Esc"},
  { KEY_F1,                   2, "F1"},
  { KEY_F2,                   2, "F2"},
  { KEY_F3,                   2, "F3"},
  { KEY_F4,                   2, "F4"},
  { KEY_F5,                   2, "F5"},
  { KEY_F6,                   2, "F6"},
  { KEY_F7,                   2, "F7"},
  { KEY_F8,                   2, "F8"},
  { KEY_F9,                   2, "F9"},
  { KEY_UP,                   2, "Up"},
  { KEY_BS,                   2, "BS"},
  { KEY_BRACKET,              1, "["},
  { KEY_BACKBRACKET,          1, "]"},
  { KEY_COMMA,                1, ","},
  { KEY_QUOTE,                1, "\""},
  { KEY_DOT,                  1, "."},
  { KEY_SLASH,                1, "/"},
};

static struct TFKey3 ModifKeyName[]={
  { KEY_CTRL   ,4 ,"Ctrl"},
  { KEY_RCTRL  ,5 ,"RCtrl"},
  { KEY_ALT    ,3 ,"Alt"},
  { KEY_RALT   ,4 ,"RAlt"},
  { KEY_SHIFT  ,5 ,"Shift"},
//  { KEY_LCTRL  ,5 ,"LCtrl"},
//  { KEY_LALT   ,4 ,"LAlt"},
//  { KEY_LSHIFT ,6 ,"LShift"},
//  { KEY_RSHIFT ,6 ,"RShift"},
};

/* ----------------------------------------------------------------- */
/* tran 31.08.2000 $
  FarInputRecordToKey */
int WINAPI InputRecordToKey(INPUT_RECORD *r)
{
  if(r)
    return CalcKeyCode(r,TRUE);
  return KEY_NONE;
}
/* tran 31.08.2000 $ */


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
                if ((DWORD)Key==KEY_NONE || (DWORD)Key!=KEY_SHIFT && tmprec.Event.KeyEvent.bKeyDown)
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
//SysLog("1) CalcKey=0x%08X",CalcKey);
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
//SysLog("2) CalcKey=0x%08X",CalcKey);

  if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
    CalcKey=KEY_NONE;
//SysLog("3) CalcKey=0x%08X\n",CalcKey);
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

static char *GetShiftKeyName(char *Name, DWORD Key,int& Len)
{
  if((Key&KEY_RCTRL) == KEY_RCTRL)   strcat(Name,ModifKeyName[1].Name);
  else if(Key&KEY_CTRL)              strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_LCTRL)             strcat(Name,ModifKeyName[3].Name);

  if((Key&KEY_RALT) == KEY_RALT)     strcat(Name,ModifKeyName[3].Name);
  else if(Key&KEY_ALT)               strcat(Name,ModifKeyName[2].Name);
//  else if(Key&KEY_LALT)    strcat(Name,ModifKeyName[6].Name);

  if(Key&KEY_SHIFT)                  strcat(Name,ModifKeyName[4].Name);
//  else if(Key&KEY_LSHIFT)  strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_RSHIFT)  strcat(Name,ModifKeyName[1].Name);

  Len=strlen(Name);
  return Name;
}

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!
*/
int WINAPI KeyNameToKey(char *Name)
{
   if(!Name || !*Name)
     return -1;

   // Это макроклавиша?
   if(Name[0] == '$' && Name[1])
     return KeyNameMacroToKey(Name);

   int I, Pos, Len=strlen(Name);
   DWORD Key=0;

   // пройдемся по всем модификаторам
   for(Pos=I=0; Pos < Len && I < sizeof(ModifKeyName)/sizeof(ModifKeyName[0]); ++I)
   {
     if(!memicmp(Name+Pos,ModifKeyName[I].Name,ModifKeyName[I].Len))
     {
       Pos+=ModifKeyName[I].Len;
       Key|=ModifKeyName[I].Key;
     }
   }

   // если что-то осталось - преобразуем.
   if(Pos < Len)
   {
     // сначала - FKeys1
     for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
       if (!memicmp(Name+Pos,FKeys1[I].Name,FKeys1[I].Len))
       {
         Key|=FKeys1[I].Key;
         break;
       }
     if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]))
     {
       WORD Chr=(WORD)Name[Pos];
       if (Chr > 0 && Chr < 256)
       {
         Key|=Chr;
         if (Key&(0xFF000000-KEY_SHIFT))
         {
           if(Chr >= 'a' && Chr <= 'z' || Chr >= 'а' && Chr <= 'п')
             Key&=~0x20;
           else if(Chr >= 'р' && Chr <= 'я')
             Key=(Key|0x10)&(~0x60);
//SysLog("KeyNameToKey=%x (%c) %x (%c)",Key,Key&0xFF,Chr,Chr);
         }
       }
     }
   }

   return (!Key)? -1: Key;
}
/* SVS $*/

BOOL WINAPI KeyToText(int Key0,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;

  char KeyText[66];
  int I, Len;
  DWORD Key=(DWORD)Key0, FKey=(DWORD)Key0&0xFFFF;

  if(Key&KEY_MACRO_BASE)
    return KeyMacroToText(Key0,KeyText0,Size);

  memset(KeyText,0,sizeof(KeyText));

  strcpy(KeyText,GetShiftKeyName(KeyText,Key,Len));

  for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
    if (FKey==FKeys1[I].Key)
    {
      strcat(KeyText,FKeys1[I].Name);
      break;
    }
  if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]))
  {
    FKey=(Key&0xFF)&(~0x20);
    if (FKey >= 'A' && FKey <= 'Z')
      KeyText[Len]=Key&0xFF;
    else if ((Key&0xFF) > 0 && (Key&0xFF) < 256)
      KeyText[Len]=Key&0xFF;
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
//SysLog("KeyToText() 0x%08X %s",Key,KeyText);
  return TRUE;
}
/* SVS 10.09.2000 $ */
/* SVS $ */


int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState)
{
  int FKey  =Key&0x0000FFFF;
  int FShift=Key&0xFF000000;
  int I;

  VirtKey=0;
  ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
               (FShift&KEY_ALT?PKF_ALT:0)|
               (FShift&KEY_CTRL?PKF_CONTROL:0);

  for(I=0; I < sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]); ++I)
    if (FKey==Table_KeyToVK[I].Key)
    {
      VirtKey=Table_KeyToVK[I].VK;
      break;
    }

  if(I  == sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]))
  {
    if (FKey>='0' && FKey<='9' || FKey>='A' && FKey<='Z')
      VirtKey=FKey;
  }
  return(VirtKey!=0);
}


// GetAsyncKeyState(VK_RSHIFT)
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
  DWORD Modif=(CtrlPressed?KEY_CTRL:0)|
        (AltPressed?KEY_ALT:0)|
        (ShiftPressed?KEY_SHIFT:0);

  if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
    return(Modif+KEY_F1+((KeyCode-VK_F1)<<8));

//  if(CtrlPressed || AltPressed || ShiftPressed)
  {
    int NotShift=!CtrlPressed && !AltPressed && !ShiftPressed;
    switch(KeyCode)
    {
      case VK_INSERT:
      case VK_NUMPAD0:
        if (NotShift && KeyCode == VK_NUMPAD0)
          return '0';
        return(Modif|KEY_INS);
      case VK_DOWN:
      case VK_NUMPAD2:
        if (NotShift && KeyCode == VK_NUMPAD2)
          return '2';
        return(Modif|KEY_DOWN);
      case VK_LEFT:
      case VK_NUMPAD4:
        if (NotShift && KeyCode == VK_NUMPAD4)
          return '4';
        return(Modif|KEY_LEFT);
      case VK_RIGHT:
      case VK_NUMPAD6:
        if (NotShift && KeyCode == VK_NUMPAD6)
          return '6';
        return(Modif|KEY_RIGHT);
      case VK_UP:
      case VK_NUMPAD8:
        if (NotShift && KeyCode == VK_NUMPAD8)
          return '8';
        return(Modif|KEY_UP);
      case VK_END:
      case VK_NUMPAD1:
        if (NotShift && KeyCode == VK_NUMPAD1)
          return '1';
        return(Modif|KEY_END);
      case VK_HOME:
      case VK_NUMPAD7:
        if (NotShift && KeyCode == VK_NUMPAD7)
          return '7';
        return(Modif|KEY_HOME);
      case VK_NEXT:
      case VK_NUMPAD3:
        if (NotShift && KeyCode == VK_NUMPAD3)
          return '3';
        return(Modif|KEY_PGDN);
      case VK_PRIOR:
      case VK_NUMPAD9:
        if (NotShift && KeyCode == VK_NUMPAD9)
          return '9';
        return(Modif|KEY_PGUP);
      case VK_CLEAR:
      case VK_NUMPAD5:
        if (NotShift)
        {
          if(KeyCode == VK_NUMPAD5)
            return '5';
        }
        return(Modif|KEY_NUMPAD5);
      case VK_DECIMAL:
        if(NotShift)
          return (CtrlState & NUMLOCK_ON) ? '.':KEY_DEL;
      case VK_DELETE:
        if(NotShift)
        {
          if ((CtrlState & NUMLOCK_ON) && (CtrlState & ENHANCED_KEY)==0 &&
             AsciiChar=='.')
            return('.');
          return(KEY_DEL);
        }
        return(Modif|KEY_DEL);
      case VK_APPS:
        return(Modif|KEY_APPS);
      case VK_LWIN:
        return(Modif|KEY_LWIN);
      case VK_RWIN:
        return(Modif|KEY_RWIN);
      case VK_RETURN:
        if (ShiftPressed && RealKey && !ShiftPressedLast && !CtrlPressed && !AltPressed)
          return(KEY_ENTER);
        return(Modif|KEY_ENTER);
      case VK_BACK:
        return(Modif|KEY_BS);
      case VK_SPACE:
        return(Modif|KEY_SPACE);
      case VK_TAB:
        return(Modif|KEY_TAB);
      case VK_ADD:
        return(Modif|KEY_ADD);
      case VK_SUBTRACT:
        return(Modif|KEY_SUBTRACT);
      case VK_MULTIPLY:
        if (ShiftPressed && !CtrlPressed && !AltPressed)
          return('*');
        return(Modif|KEY_MULTIPLY);
      case VK_ESCAPE:
        return(Modif|KEY_ESC);
    }
  }

  if (CtrlPressed && AltPressed)
  {
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL|KEY_ALT+KeyCode);
    if (AsciiChar)
      return(KEY_CTRL|KEY_ALT+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_ALT+KeyCode);
  }

  if (AltPressed && ShiftPressed)
  {
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_ALTSHIFT+KeyCode);
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
      return(KEY_ALT+KEY_SHIFT+AsciiChar);
    }
    if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_ALT+KEY_SHIFT+KeyCode);
  }

  if (CtrlPressed && ShiftPressed)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRLSHIFT0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLSHIFTA+KeyCode-'A');
    switch(KeyCode)
    {
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
    }
    if (AsciiChar)
      return(KEY_CTRL|KEY_SHIFT+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_SHIFT+KeyCode);
  }

  if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_RCTRL0+KeyCode-'0');
  }

  if (!CtrlPressed && !AltPressed && !ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_DIVIDE);
      case KEY_BREAK:
        return(KEY_BREAK);
    }
  }


  if (CtrlPressed)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRL0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL+KeyCode);
    switch(KeyCode)
    {
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

    if (KeyCode)
    {
      if (!RealKey && KeyCode==VK_CONTROL)
        return(KEY_NONE);
      return(KEY_CTRL+KeyCode);
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

    switch(KeyCode)
    {
      case 0xbc:
        return(KEY_ALTCOMMA);
      case 0xbe:
        return(KEY_ALTDOT);
    }
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
//SysLog("AltPressed) KeyCode=%x AsciiChar=%x LocalUpper(AsciiChar)=%x (%x)", KeyCode, AsciiChar,LocalUpper(AsciiChar),LocalUpper(AsciiChar)+KEY_ALT);
      return(LocalUpper(AsciiChar)+KEY_ALT);
    }
    if (KeyCode==VK_CAPITAL)
      return(KEY_NONE);
    if (!RealKey && KeyCode==VK_MENU)
      return(KEY_NONE);
    return(KEY_ALT+KeyCode);
  }

  if (AsciiChar)
    return(AsciiChar);
  return(KEY_NONE);
}

