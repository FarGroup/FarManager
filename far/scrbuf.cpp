/*
scrbuf.cpp

Буферизация вывода на экран, весь вывод идет через этот буфер

*/

/* Revision: 1.07 06.06.2001 $ */

/*
Modify:
  06.06.2001 SVS
    ! W-функции юзаем пока только в режиме USE_WFUNC
  06.06.2001 SVS
    ! Под NT применяются W-функции для вывода в консоль
  29.05.2001 tran
    + DIRECT_RT
  12.05.2001 DJ
    ! глобальный ScrBuf переехал сюда
  06.05.2001 DJ
    ! перетрях #include
  09.01.2001 SVS
    ! По наводке ER - в SetCursorType не дергать раньше
      времени установку курсора
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "scrbuf.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"

//#if defined(SYSLOG_OT)
// #define DIRECT_SCREEN_OUT
//#endif

#ifdef DIRECT_RT
extern int DirectRT;
#endif

#if defined(USE_WFUNC)
extern WCHAR Oem2Unicode[];
#endif

ScreenBuf ScrBuf;

ScreenBuf::ScreenBuf()
{
  Buf=Shadow=NULL;
  BufX=BufY=0;
  Flushed=FlushedCurPos=FlushedCurType=TRUE;
  LockCount=0;
  hScreen=NULL;
  UseShadow=FALSE;
}


ScreenBuf::~ScreenBuf()
{
  /* $ 13.07.2000 SVS
     раз уж вызвали new[], то и нужно delete[]
  */
  if(Buf)    delete[] Buf;
  if(Shadow) delete[] Shadow;
  /* SVS $ */
}


/* $ 13.07.2000 SVS
   раз уж вызвали new[], то и нужно delete[]
*/
void ScreenBuf::AllocBuf(int X,int Y)
{
  if (X==BufX && Y==BufY)
    return;
  if(Buf) delete[] Buf;
  if(Shadow) delete[] Shadow;
  Buf=new CHAR_INFO[X*Y];
  Shadow=new CHAR_INFO[X*Y];
#if !defined(ALLOC)
  /* а вот здесь самая интересность: при переопределенных функция запроса
     памяти - память уже проинициализированна нулями ;-)
     поэтому этот кусок пропустится...
  */
  memset(Buf,0,X*Y*sizeof(CHAR_INFO));
  memset(Shadow,0,X*Y*sizeof(CHAR_INFO));
#endif
  BufX=X;
  BufY=Y;
}
/* SVS $ */


void ScreenBuf::FillBuf()
{
  COORD Size,Corner;
  SMALL_RECT Coord;
  Size.X=BufX;
  Size.Y=BufY;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=0;
  Coord.Top=0;
  Coord.Right=BufX-1;
  Coord.Bottom=BufY-1;
#if defined(USE_WFUNC)
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    ReadConsoleOutputW(hScreen,Buf,Size,Corner,&Coord);
  else
    ReadConsoleOutputA(hScreen,Buf,Size,Corner,&Coord);
#else
    ReadConsoleOutput(hScreen,Buf,Size,Corner,&Coord);
#endif
  memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
  UseShadow=TRUE;

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hScreen,&csbi);
  CurX=csbi.dwCursorPosition.X;
  CurY=csbi.dwCursorPosition.Y;
}


void ScreenBuf::Write(int X,int Y,CHAR_INFO *Text,int TextLength)
{
  if (Y>=BufY || TextLength==0)
    return;
  int Pos=Y*BufX;
  for (int I=0;I<TextLength;I++)
  {
    int PosX=I+X;
    if (PosX>=BufX)
      break;
    int CurPos=Pos+PosX;
    Buf[CurPos]=Text[I];
  }
  Flushed=FALSE;

#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif DIRECT_RT
  if ( DirectRT  )
    Flush();
#endif

}


void ScreenBuf::Read(int X1,int Y1,int X2,int Y2,CHAR_INFO *Text)
{
  int Width=X2-X1+1;
  for (int I=0;I<Y2-Y1+1;I++)
    for (int J=0;J<Width;J++)
      Text[I*Width+J]=Buf[(Y1+I)*BufX+(X1+J)];
#if defined(USE_WFUNC)
  if (X1==0 && Y1==0 && CtrlObject!=NULL && CtrlObject->Macro.IsRecording() &&
      GetVidChar(MacroChar)!='R')
#else
  if (X1==0 && Y1==0 && CtrlObject!=NULL && CtrlObject->Macro.IsRecording() &&
      MacroChar.Char.AsciiChar!='R')
#endif
    Text[0]=MacroChar;
}


void ScreenBuf::Flush()
{
  if (LockCount>0)
    return;
  if (CtrlObject!=NULL && CtrlObject->Macro.IsRecording())
  {
#if defined(USE_WFUNC)
    if (GetVidChar(Buf[0])!='R')
      MacroChar=Buf[0];
    SetVidChar(Buf[0],'R');
#else
    if (Buf[0].Char.AsciiChar!='R')
      MacroChar=Buf[0];
    Buf[0].Char.AsciiChar='R';
#endif
    Buf[0].Attributes=FarColorToReal(COL_WARNDIALOGTEXT);
  }
  if (!FlushedCurType && !CurVisible)
  {
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize=CurSize;
    cci.bVisible=CurVisible;
    SetConsoleCursorInfo(hScreen,&cci);
    FlushedCurType=TRUE;
  }
  if (!Flushed)
  {
    Flushed=TRUE;
    if (WaitInMainLoop && Opt.Clock)
      ShowTime(FALSE);
    int WriteX1=BufX-1,WriteY1=BufY-1,WriteX2=0,WriteY2=0;
    int NoChanges=TRUE;
    if (UseShadow)
    {
      for (int I=0;I<BufY;I++)
        for (int J=0;J<BufX;J++)
        {
          int Pos=I*BufX+J;
#if defined(USE_WFUNC)
          if (GetVidChar(Buf[Pos]) == GetVidChar(Shadow[Pos]) ||
              Buf[Pos].Attributes!=Shadow[Pos].Attributes)
#else
          if (Buf[Pos].Char.AsciiChar!=Shadow[Pos].Char.AsciiChar ||
              Buf[Pos].Attributes!=Shadow[Pos].Attributes)
#endif
          {
            if (WriteX1>J)
              WriteX1=J;
            if (WriteX2<J)
              WriteX2=J;
            if (WriteY1>I)
              WriteY1=I;
            if (WriteY2<I)
              WriteY2=I;
            NoChanges=FALSE;
          }
        }
    }
    else
    {
      NoChanges=FALSE;
      WriteX1=0;
      WriteY1=0;
      WriteX2=BufX-1;
      WriteY2=BufY-1;
    }
    if (!NoChanges)
    {
      COORD Size,Corner;
      SMALL_RECT Coord;
      Size.X=BufX;
      Size.Y=BufY;
      Corner.X=WriteX1;
      Corner.Y=WriteY1;
      Coord.Left=WriteX1;
      Coord.Top=WriteY1;
      Coord.Right=WriteX2;
      Coord.Bottom=WriteY2;
#if defined(USE_WFUNC)
      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      {
        // Нужно ли здесь делать перекодировку oem->unicode???
        WriteConsoleOutputW(hScreen,Buf,Size,Corner,&Coord);
      }
      else
        WriteConsoleOutputA(hScreen,Buf,Size,Corner,&Coord);
#else
      WriteConsoleOutput(hScreen,Buf,Size,Corner,&Coord);
#endif
      memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
    }
  }
  if (!FlushedCurPos)
  {
    COORD C;
    C.X=CurX;
    C.Y=CurY;
    SetConsoleCursorPosition(hScreen,C);
    FlushedCurPos=TRUE;
  }
  if (!FlushedCurType && CurVisible)
  {
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize=CurSize;
    cci.bVisible=CurVisible;
    SetConsoleCursorInfo(hScreen,&cci);
    FlushedCurType=TRUE;
  }
  UseShadow=TRUE;
  Flushed=TRUE;
}


void ScreenBuf::Lock()
{
  LockCount++;
}


void ScreenBuf::Unlock()
{
  if (LockCount>0)
    LockCount--;
}


void ScreenBuf::SetHandle(HANDLE hScreen)
{
  ScreenBuf::hScreen=hScreen;
}


void ScreenBuf::ResetShadow()
{
  Flushed=FlushedCurType=FlushedCurPos=UseShadow=FALSE;
}


void ScreenBuf::MoveCursor(int X,int Y)
{
  CurX=X;
  CurY=Y;
  FlushedCurPos=FALSE;
}


void ScreenBuf::GetCursorPos(int& X,int& Y)
{
  X=CurX;
  Y=CurY;
}


void ScreenBuf::SetCursorType(int Visible,int Size)
{
  /* $ 09.01.2001 SVS
     По наводке ER - в SetCursorType не дергать раньше
     времени установку курсора
  */
  if (CurVisible!=Visible || CurSize!=Size)
  {
    CurVisible=Visible;
    CurSize=Size;
    FlushedCurType=FALSE;
  }
  /* SVS $ */
}

void ScreenBuf::GetCursorType(int &Visible,int &Size)
{
  Visible=CurVisible;
  Size=CurSize;
}


void ScreenBuf::RestoreMacroChar()
{
  Write(0,0,&MacroChar,1);
  ResetShadow();
}
