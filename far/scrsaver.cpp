/*
scrsaver.cpp

ScreenSaver

*/

/* Revision: 1.01 10.10.2000 $ */

/*
Modify:
  10.10.2000 SVS
    ! Незначительное уточнение в строке:
      if (Star[I].Type!=STAR_NONE && (Step%Star[I].Speed)==0)
                                     ^                  ^
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

static void ShowSaver(int Step);

// from scrsaver.cpp
static struct
{
  int X;
  int Y;
  int Type;
  int Color;
  int Speed;
} Star[16];

int ScreenSaver(int EnableExit)
{
  INPUT_RECORD rec;
  clock_t WaitTime;

  if (ScreenSaverActive)
    return(1);
  ChangePriority ChPriority(THREAD_PRIORITY_IDLE);
  for (WaitTime=clock();clock()-WaitTime<500;)
  {
    if (PeekInputRecord(&rec))
      return(1);
    Sleep(100);
  }
  ScreenSaverActive=TRUE;
  {
    SaveScreen SaveScr;
    SetCursorType(0,10);
    randomize();
    SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);

    for (int I=0;I<sizeof(Star)/sizeof(Star[0]);I++)
      Star[I].Type=STAR_NONE;

    int Step=0;
    while (!PeekInputRecord(&rec))
    {
      clock_t CurTime=clock();

      if (EnableExit && Opt.InactivityExit && Opt.InactivityExitTime>0 &&
          CurTime-StartIdleTime>Opt.InactivityExitTime*60000 &&
          CtrlObject->ModalManager.GetModalCount()==0)
      {
        CtrlObject->ExitMainLoop(FALSE);
        return(0);
      }
      Sleep(50);
      ShowSaver(Step++);
    }
  }
  ScreenSaverActive=FALSE;
  FlushInputBuffer();
  StartIdleTime=clock();
  return(1);
}


static void ShowSaver(int Step)
{
  int I;
  for (I=0;I<sizeof(Star)/sizeof(Star[0]);I++)
    if (Star[I].Type!=STAR_NONE && (Step%Star[I].Speed)==0)
    {
      SetColor(F_LIGHTCYAN|B_BLACK);
      GotoXY(Star[I].X/100,Star[I].Y/100);
      Text(" ");
      int dx=Star[I].X/100-ScrX/2;
      Star[I].X+=dx*10+((dx<0) ? -1:1);
      int dy=Star[I].Y/100-ScrY/2;
      Star[I].Y+=dy*10+((dy<0) ? -1:1);
      if (Star[I].X<0 || Star[I].X/100>ScrX || Star[I].Y<0 || Star[I].Y/100>ScrY)
        Star[I].Type=STAR_NONE;
      else
      {
        SetColor(Star[I].Color|B_BLACK);
        GotoXY(Star[I].X/100,Star[I].Y/100);
        if (abs(dx)>3*ScrX/8 || abs(dy)>3*ScrY/8)
        {
          if (Star[I].Type==STAR_PLANET)
          {
            SetColor(Star[I].Color|FOREGROUND_INTENSITY|B_BLACK);
            Text("■");
          }
          else
          {
            SetColor(F_WHITE|B_BLACK);
            Text("");
          }
        }
        else
          if (abs(dx)>ScrX/7 || abs(dy)>ScrY/7)
          {
            if (Star[I].Type==STAR_PLANET)
            {
              SetColor(Star[I].Color|FOREGROUND_INTENSITY|B_BLACK);
              Text("");
            }
            else
            {
              if (abs(dx)>ScrX/4 || abs(dy)>ScrY/4)
                SetColor(F_LIGHTCYAN|B_BLACK);
              else
                SetColor(F_CYAN|B_BLACK);
              Text("∙");
            }
          }
          else
          {
            if (Star[I].Type==STAR_PLANET)
            {
              SetColor(Star[I].Color|B_BLACK);
              Text("°");
            }
            else
            {
              SetColor(F_CYAN|B_BLACK);
              Text("·");
            }
          }
      }

    }
  for (I=0;I<sizeof(Star)/sizeof(Star[0]);I++)
    if (Star[I].Type==STAR_NONE)
    {
      static const int Colors[]={F_MAGENTA,F_RED,F_BLUE};
      Star[I].Type=random(77)<3 ? STAR_PLANET:STAR_NORMAL;
      Star[I].X=(ScrX/2-ScrX/4+random(ScrX/2))*100;
      Star[I].Y=(ScrY/2-ScrY/4+random(ScrY/2))*100;
      Star[I].Color=Colors[random(sizeof(Colors)/sizeof(Colors[0]))];
      Star[I].Speed=(Star[I].Type==STAR_PLANET) ? 1:2;
      break;
    }
}
