/*
interf.cpp

Консольные функции ввода-вывода

*/

/* Revision: 1.09 08.09.2000 $ */

/*
Modify:
  08.09.2000 SVS
    + KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL в функции CalcKeyCode
  24.08.2000 SVS
    + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
    + Добавление на реакцию KEY_CTRLALTSHIFTPRESS & KEY_CTRLALTSHIFTRELEASE
  23.08.2000 SVS
    + Код для средней клавиши мыши :-) (ну есть у меня дома эта хрень...)
  09.08.2000 KM
    ! Изменена MakeShadow таким образом, что теперь при достижении
      окном (диалогом) правой или нижней границы экрана и при выходе за неё
      тень продолжает рисоваться, что при нынешней идеологии движения окон
      более правильно.
  23.07.2000 SVS
    ! Немного оптимизации в функциях Box(), HiText() :-)
    + Клавиши (CalcKeyCode):
       Ctrl- Shift- Alt- CtrlShift- AltShift- CtrlAlt- Apps :-)
       KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
    + Text(int X, int Y, int Color, char *Str);
    + Text(int X, int Y, int Color, int MsgId);
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  06.07.2000 SVS
    - Временная отмена патча 11 (NT Console resize bug) до лучших времен :-)
  28.06.2000 tran
    - NT Console resize bug
      adding resize support in GetInputRecord for modal window like
      Viewer, Editor, Help
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

BOOL __stdcall CtrlHandler(DWORD CtrlType);
static void InitRecodeOutTable();

static HANDLE hConOut,hConInp;
static int CurX,CurY;
static int CurColor;
static int AltValue=0,ReturnAltValue;

static int ShiftPressedLast=FALSE,AltPressedLast=FALSE,CtrlPressedLast=FALSE;
static int RightAltPressedLast=FALSE,RightCtrlPressedLast=FALSE;
static clock_t PressedLastTime,KeyPressedLastTime;

static int OutputCP;
static unsigned char RecodeOutTable[256];
static int InitCurVisible,InitCurSize;

static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?

void InitConsole()
{
  OutputCP=GetConsoleOutputCP();
  InitRecodeOutTable();
  hConOut=CreateFile("CONOUT$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  ScrBuf.SetHandle(hConOut);
  hConInp=CreateFile("CONIN$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  SetConsoleCtrlHandler(CtrlHandler,TRUE);
  GetConsoleMode(hConInp,&InitialConsoleMode);
  GetRealCursorType(InitCurVisible,InitCurSize);
  GetRegKey("Interface","Mouse",Opt.Mouse,1);
  SetFarConsoleMode();
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
  GetVideoMode();
  ScrBuf.FillBuf();
  // было sizeof(Palette)
  memcpy(Palette,DefaultPalette,SizeArrayPalette);
}


void ReopenConsole()
{
  HANDLE hOldOut=hConOut;
  hConOut=CreateFile("CONOUT$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  ScrBuf.SetHandle(hConOut);
  SetStdHandle(STD_OUTPUT_HANDLE,hConOut);
  CloseHandle(hOldOut);
}


void CloseConsole()
{
  ScrBuf.Flush();
  SetRealCursorType(InitCurVisible,InitCurSize);
  ChangeConsoleMode(InitialConsoleMode);
  CloseHandle(hConInp);
  CloseHandle(hConOut);
}


void SetFarConsoleMode()
{
  int Mode=ENABLE_WINDOW_INPUT;
  if (Opt.Mouse)
    Mode|=ENABLE_MOUSE_INPUT;
  ChangeConsoleMode(Mode);
}


void InitRecodeOutTable()
{
  int I;
  for (I=0;I<sizeof(RecodeOutTable)/sizeof(RecodeOutTable[0]);I++)
    RecodeOutTable[I]=I;
  if (Opt.CleanAscii)
  {
    for (int I=0;I<32;I++)
      RecodeOutTable[I]='.';
    RecodeOutTable[7]='*';
    RecodeOutTable[24]=RecodeOutTable[25]='|';
    RecodeOutTable[30]='X';
    RecodeOutTable[31]='X';
    RecodeOutTable[255]=' ';
  }
  if (Opt.NoGraphics)
  {
    for (int I=179;I<=218;I++)
      RecodeOutTable[I]='+';
    RecodeOutTable[179]=RecodeOutTable[186]='|';
    RecodeOutTable[196]='-';
    RecodeOutTable[205]='=';
  }
}


void ChangeConsoleMode(int Mode)
{
  DWORD CurrentConsoleMode;
  GetConsoleMode(hConInp,&CurrentConsoleMode);
  if (CurrentConsoleMode!=Mode)
    SetConsoleMode(hConInp,Mode);
}


void FlushInputBuffer()
{
  FlushConsoleInputBuffer(hConInp);
}


void ChangeVideoMode(int NumLines,int NumColumns)
{
  int xSize=NumColumns,ySize=NumLines;
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
  SMALL_RECT srWindowRect; /* hold the new console size */
  COORD coordScreen;

  GetConsoleScreenBufferInfo(hConOut, &csbi);
  /* get the largest size we can size the console window to */
  coordScreen = GetLargestConsoleWindowSize(hConOut);
  /* define the new console window size and scroll position */
  srWindowRect.Right = (SHORT) (Min(xSize, coordScreen.X) - 1);
  srWindowRect.Bottom = (SHORT) (Min(ySize, coordScreen.Y) - 1);
  srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
  /* define the new console buffer size */
  coordScreen.X = xSize;
  coordScreen.Y = ySize;
  /* if the current buffer is larger than what we want, resize the */
  /* console window first, then the buffer */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
  {
    SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
    SetConsoleScreenBufferSize(hConOut, coordScreen);
  }
  /* if the current buffer is smaller than what we want, resize the */
  /* buffer first, then the console window */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)
  {
    SetConsoleScreenBufferSize(hConOut, coordScreen);
    SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
  }
  GetVideoMode();
}


void GetVideoMode()
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  memset(&csbi,0,sizeof(csbi));
  GetConsoleScreenBufferInfo(hConOut,&csbi);
  ScrX=csbi.dwSize.X-1;
  ScrY=csbi.dwSize.Y-1;
  ScrBuf.AllocBuf(csbi.dwSize.X,csbi.dwSize.Y);
}


BOOL __stdcall CtrlHandler(DWORD CtrlType)
{
  if (CtrlType==CTRL_C_EVENT || CtrlType==CTRL_BREAK_EVENT)
  {
    if (CtrlType==CTRL_BREAK_EVENT)
      WriteInput(KEY_BREAK);
    if (CtrlObject->LeftPanel!=NULL && CtrlObject->LeftPanel->GetMode()==PLUGIN_PANEL)
      CtrlObject->Plugins.ProcessEvent(CtrlObject->LeftPanel->GetPluginHandle(),FE_BREAK,(void *)CtrlType);
    if (CtrlObject->RightPanel!=NULL && CtrlObject->RightPanel->GetMode()==PLUGIN_PANEL)
      CtrlObject->Plugins.ProcessEvent(CtrlObject->RightPanel->GetPluginHandle(),FE_BREAK,(void *)CtrlType);
    return(TRUE);
  }
  CloseFAR=TRUE;
  if (CurrentEditor!=NULL && CurrentEditor->IsFileModified() ||
      CtrlObject->ModalManager.IsAnyModalModified(FALSE))
    return(TRUE);
  return(FALSE);
}


void GotoXY(int X,int Y)
{
  if (X<0) X=0;
  if (Y<0) Y=0;
  CurX=X;
  CurY=Y;
}


int WhereX()
{
  return(CurX);
}


int WhereY()
{
  return(CurY);
}


void MoveCursor(int X,int Y)
{
  ScrBuf.MoveCursor(X,Y);
}


void GetCursorPos(int& X,int& Y)
{
  ScrBuf.GetCursorPos(X,Y);
}


void SetCursorType(int Visible,int Size)
{
  if (Size==-1 || !Visible)
    Size=IsWindowed() ? 15:10;

  ScrBuf.SetCursorType(Visible,Size);
}


void GetCursorType(int &Visible,int &Size)
{
  ScrBuf.GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
  COORD C;
  C.X=X;
  C.Y=Y;
  SetConsoleCursorPosition(hConOut,C);
}


void GetRealCursorPos(int& X,int& Y)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConOut,&csbi);
  X=csbi.dwCursorPosition.X;
  Y=csbi.dwCursorPosition.Y;
}


void SetRealCursorType(int Visible,int Size)
{
  CONSOLE_CURSOR_INFO cci;
  cci.dwSize=Size;
  cci.bVisible=Visible;
  SetConsoleCursorInfo(hConOut,&cci);
}


void GetRealCursorType(int &Visible,int &Size)
{
  CONSOLE_CURSOR_INFO cci;
  GetConsoleCursorInfo(hConOut,&cci);
  Size=cci.dwSize;
  Visible=cci.bVisible;
}

/* $ 23.07.2000 SVS
   + две полных функции Text
*/
void Text(int X, int Y, int Color, char *Str)
{
  int Length=strlen(Str);
  CurColor=FarColorToReal(Color);
  if (X<0) X=0;
  if (Y<0) Y=0;
  CurX=X;
  CurY=Y;
  if (CurX+Length>ScrX)
    Length=ScrX-CurX+1;
  if (Length<=0)
    return;
  CHAR_INFO CharBuf[1024]={0};
  for (int I=0;I<Length;I++)
  {
    CharBuf[I].Char.AsciiChar=RecodeOutTable[Str[I]];
    CharBuf[I].Attributes=CurColor;
  }
  ScrBuf.Write(CurX,CurY,CharBuf,Length);
  CurX+=Length;
}

void Text(char *Str)
{
  Text(CurX,CurY,CurColor,Str);
}

void Text(int X, int Y, int Color,int MsgId)
{
  Text(X,Y,Color,MSG(MsgId));
}

void Text(int MsgId)
{
  Text(CurX,CurY,CurColor,MSG(MsgId));
}
/* SVS $ */

void VText(char *Str)
{
  int Length=strlen(Str);
  if (CurY+Length>ScrY)
    Length=ScrY-CurY+1;
  if (Length<=0)
    return;
  int StartCurX=CurX;
  for (int I=0;I<Length;I++)
  {
    GotoXY(CurX,CurY);
    mprintf("%c",Str[I]);
    CurY++;
    CurX=StartCurX;
  }
}


void HiText(char *Str,int HiColor)
{
  char TextStr[300];
  int SaveColor;
  char *ChPtr;
  strncpy(TextStr,Str,sizeof(TextStr));
  if ((ChPtr=strchr(TextStr,'&'))==NULL)
    Text(TextStr);
  else
  {
    *ChPtr=0;
    Text(TextStr);
    if (ChPtr[1])
    {
      /* $ 23.07.2000 SVS
         Немного оптимизации :-)
      */
      char Chr[2];
      SaveColor=CurColor;
      SetColor(HiColor);
      Chr[0]=ChPtr[1]; Chr[1]=0;
      Text(Chr);
      SetColor(SaveColor);
      Text(ChPtr+2);
      /* SVS $ */
    }
  }
}


void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color)
{
  char FillStr[256];
  int I;
  SetColor(Color);
  if (X1<0) X1=0;
  if (Y1<0) Y1=0;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;
  memset(FillStr,Ch,X2-X1+1);
  FillStr[X2-X1+1]=0;
  for (I=Y1;I<=Y2;I++)
  {
    GotoXY(X1,I);
    Text(FillStr);
  }
}


void MakeShadow(int X1,int Y1,int X2,int Y2)
{
  int I;
  CHAR_INFO *CharBuf=new CHAR_INFO[(X2-X1+1)*(Y2-Y1+1)];
  /* $ 09.08.2000 KM
     Данное изменение позволяет отрисовывать тень, даже
     если край окна вышел за границы экрана
  */
  if (X1<0 || Y1<0)  // || X2>ScrX || Y2>ScrY)
    return;
  /* KM $ */
  GetText(X1,Y1,X2,Y2,CharBuf);
  for (I=0;I<(X2-X1+1)*(Y2-Y1+1);I++)
    CharBuf[I].Attributes&=~0xf8;
  PutText(X1,Y1,X2,Y2,CharBuf);
  /* $ 13.07.2000 SVS
     раз уж вызвали new[], то и нужно delete[]
  */
  delete[] CharBuf;
  /* SVS $ */
}


void mprintf(char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  char OutStr[1024];
  vsprintf(OutStr,fmt,argptr);
  Text(OutStr);
  va_end(argptr);
}

void mprintf(int MsgId,...)
{
  va_list argptr;
  va_start(argptr,MsgId);
  char OutStr[1024];
  vsprintf(OutStr,MSG(MsgId),argptr);
  Text(OutStr);
  va_end(argptr);
}

void vmprintf(char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  char OutStr[1024];
  vsprintf(OutStr,fmt,argptr);
  VText(OutStr);
  va_end(argptr);
}

void SetColor(int Color)
{
  CurColor=FarColorToReal(Color);
}


int GetColor()
{
  return(CurColor);
}


void ScrollScreen(int Count)
{
  char *ScreenBuf=new char[(ScrX+1)*(ScrY+1)*4+10];
  if (!ScreenBuf || Count<=0)
    return;
  GetText(0,Count,ScrX,ScrY,ScreenBuf);
  PutText(0,0,ScrX,ScrY-Count,ScreenBuf);
  SetScreen(0,ScrY+1-Count,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);
  /* $ 13.07.2000 SVS
     раз уж вызвали new[], то и нужно delete[]
  */
  delete[] ScreenBuf;
  /* SVS $ */
}


void GetText(int X1,int Y1,int X2,int Y2,void *Dest)
{
  ScrBuf.Read(X1,Y1,X2,Y2,(CHAR_INFO *)Dest);
}


void PutText(int X1,int Y1,int X2,int Y2,void *Src)
{
  int Width=X2-X1+1;
  for (int I=0;I<Y2-Y1+1;I++)
    ScrBuf.Write(X1,Y1+I,((PCHAR_INFO)Src)+Width*I,Width);
}



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

void ShowTime(int ShowAlways)
{
  char ClockText[10];
  static SYSTEMTIME lasttm={0,0,0,0,0,0,0,0};
  SYSTEMTIME tm;
  GetLocalTime(&tm);
  CHAR_INFO ScreenClockText[5];
  GetText(ScrX-4,0,ScrX,0,ScreenClockText);
  if (ShowAlways==2)
  {
    memset(&lasttm,0,sizeof(lasttm));
    return;
  }
  if (!ShowAlways && lasttm.wMinute==tm.wMinute && lasttm.wHour==tm.wHour &&
      ScreenClockText[2].Char.AsciiChar==':' || ScreenSaverActive)
    return;
  lasttm=tm;
  sprintf(ClockText,"%02d:%02d",tm.wHour,tm.wMinute);
  GotoXY(ScrX-4,0);
  SetColor(COL_CLOCK);
  Text(ClockText);
  ScrBuf.Flush();
  static int RegChecked=FALSE;
  if (!RegChecked && clock()>10000)
  {
    RegChecked=TRUE;
    struct RegInfo Reg;
    Reg.Done=0;
    _beginthread(CheckReg,0x10000,&Reg);
    while (!Reg.Done)
      Sleep(0);
    if (*Reg.RegName)
    {
      unsigned char Add=158,Xor1=211;
      for (int I=0;Reg.RegName[I];I++)
      {
        Add+=Reg.RegName[I];
        Xor1^=(RegName[I]<<I)^I;
      }
      if ((Add & 0xf)!=ToHex(Reg.RegCode[1]) || ((Add>>3) & 0xf)!=ToHex(Reg.RegCode[2]))
      {
        void *ErrRegFnPtr=((char *)ErrRegFn-111);
        _beginthread((void (cdecl *)(void *))((char *)ErrRegFnPtr+111),0x10000,NULL);
      }
      if (RegVer!=3 && (((Xor1>>5) & 0xf)!=ToHex(Reg.RegCode[4]) || (~Xor1 & 0xf)!=ToHex(Reg.RegCode[6])))
      {
        void *ErrRegFnPtr=((char *)ErrRegFn-50);
        _beginthread((void (cdecl *)(void *))((char *)ErrRegFnPtr+50),0x10000,NULL);
      }
    }
  }
}

void BoxText(char *Str)
{
  if (OutputCP!=437 && OutputCP!=866)
    for (int I=0;Str[I]!=0;I++)
      switch(Str[I])
      {
        case 199:
        case 182:
          Str[I]=186;
          break;
        case 207:
        case 209:
          Str[I]=205;
          break;
      }
  Text(Str);
}

/*
   Отрисовка прямоугольника.
*/
/* $ 23.07.2000 SVS
   Немного оптимизации :-)
*/
void Box(int x1,int y1,int x2,int y2,int Color,int Type)
{
  char OutStr[512];
  static char ChrBox[2][6]={
    {'─','│','┌','└','┘','┐'},
    {'═','║','╔','╚','╝','╗'},
  };

  if (x1>=x2 || y1>=y2)
    return;

  SetColor(Color);
  Type=(Type==SINGLE_BOX || Type==SHORT_SINGLE_BOX)?0:1;

  memset(OutStr,ChrBox[Type][0],sizeof(OutStr));
  OutStr[x2-x1]=0;

  OutStr[0]=ChrBox[Type][2];
  OutStr[x2-x1]=ChrBox[Type][5];
  GotoXY(x1,y1);
  mprintf("%.*s",x2-x1+1,OutStr);

  OutStr[0]=ChrBox[Type][3];
  OutStr[x2-x1]=ChrBox[Type][4];
  GotoXY(x1,y2);
  mprintf("%.*s",x2-x1+1,OutStr);

  memset(OutStr,ChrBox[Type][1],sizeof(OutStr));
  OutStr[y2-y1]=0;

  GotoXY(x1,y1+1);
  vmprintf("%.*s",y2-y1-1,OutStr);
  GotoXY(x2,y1+1);
  vmprintf("%.*s",y2-y1-1,OutStr);
}
/* SVS $ */

void PutRealText(int X1,int Y1,int X2,int Y2,void *Src)
{
  COORD Size,Corner;
  SMALL_RECT Coord;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;
  if (X1>X2) X1=X2;
  if (Y1>Y2) Y1=Y2;
  Size.X=X2-X1+1;
  Size.Y=Y2-Y1+1;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=X1;
  Coord.Top=Y1;
  Coord.Right=X2;
  Coord.Bottom=Y2;
  WriteConsoleOutput(hConOut,(PCHAR_INFO)Src,Size,Corner,&Coord);
}

void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest)
{
  COORD Size,Corner;
  SMALL_RECT Coord;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;
  if (X1>X2) X1=X2;
  if (Y1>Y2) Y1=Y2;
  Size.X=X2-X1+1;
  Size.Y=Y2-Y1+1;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=X1;
  Coord.Top=Y1;
  Coord.Right=X2;
  Coord.Bottom=Y2;
  ReadConsoleOutput(hConOut,(PCHAR_INFO)Dest,Size,Corner,&Coord);
}
