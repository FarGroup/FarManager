/*
grabber.cpp

Screen grabber

*/

/* Revision: 1.05 14.03.2001 $ */

/*
Modify:
  14.03.2001 SVS
    - Неправильно воспроизводился макрос в режиме грабления экрана.
      При воспроизведении клавиша Home перемещала курсор в координаты
      0,0 консоли.
      Не было учтено режима выполнения макроса.
  07.02.2001 SVS
    - Бага с грабилкой... Забыли ввести проверку на "вынос" координат мыши за
      пределы экрана.
  17.10.2000 tran
    !  screen grabber (Alt-Ins) при добавлении к содержимому clipboard (Ctrl-Gray+)
       не вставляет <CR> в конце, из-за чего несколько фрагментов "слипаются"
       на стыках. возможно имеет смысл также вставлять <CR> в начале фрагмента
  14.08.2000 tran
    - trap при проигрывании макроса с клавишами типа Shift...
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
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

Grabber::Grabber()
{
  SaveScr=new SaveScreen;

  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_OTHER);

  /* $ 14.08.2000 tran
     хоть и не решает баг, но уже на трапается */
  memset(&GArea,0,sizeof(GArea));
  memset(&PrevArea,0,sizeof(PrevArea));
  /* tran 14.08.2000 $ */

  int Visible,Size;
  GetCursorType(Visible,Size);
  if (Visible)
    GetCursorPos(GArea.CurX,GArea.CurY);
  else
  {
    GArea.CurX=0;
    GArea.CurY=0;
  }
  GArea.X1=-1;

  SetCursorType(TRUE,60);

  PrevArea=GArea;
  ResetArea=TRUE;

  Process();
  delete SaveScr;
}


Grabber::~Grabber()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
}


void Grabber::CopyGrabbedArea(int Append)
{
  if (GArea.X1==-1)
    return;
  int X1,Y1,X2,Y2;
  X1=Min(GArea.X1,GArea.X2);
  X2=Max(GArea.X1,GArea.X2);
  Y1=Min(GArea.Y1,GArea.Y2);
  Y2=Max(GArea.Y1,GArea.Y2);
  int BufSize=(X2-X1+3)*(Y2-Y1+1);
  CHAR_INFO *CharBuf=new CHAR_INFO[BufSize];
  char *CopyBuf=new char[BufSize];
  GetText(X1,Y1,X2,Y2,CharBuf);
  *CopyBuf=0;
  for (int I=0;I<Y2-Y1+1;I++)
  {
    if (I>0)
      strcat(CopyBuf,"\r\n");
    for (int J=0;J<X2-X1+1;J++)
      strncat(CopyBuf,&CharBuf[I*(X2-X1+1)+J].Char.AsciiChar,1);
    for (int K=strlen(CopyBuf)-1;K>=0 && CopyBuf[K]==' ';K--)
      CopyBuf[K]=0;
  }
  if (Append)
  {
    char *AppendBuf=PasteFromClipboard();
    int add=0;
    if (AppendBuf!=NULL)
    {
      int DataSize=strlen(AppendBuf);
      if ( AppendBuf[DataSize-1]!='\n' )
      {
        add=2;
      }
      AppendBuf=(char *)realloc(AppendBuf,DataSize+BufSize+add);
      memcpy(AppendBuf+DataSize+add,CopyBuf,BufSize);
      if ( add )
        memcpy(AppendBuf+DataSize,"\r\n",2);
      /* $ 13.07.2000 SVS
         раз вызывали new[], то нужно вызывать delete[]
      */
      delete[] CopyBuf;
      /* SVS $ */
      CopyBuf=AppendBuf;
    }
  }
  CopyToClipboard(CopyBuf);
  /* $ 13.07.2000 SVS
     раз вызывали new[], то нужно вызывать delete[]
  */
  delete[] CopyBuf;
  delete[] CharBuf;
  /* SVS $ */
}


void Grabber::DisplayObject()
{
  MoveCursor(GArea.CurX,GArea.CurY);
  if (PrevArea.X1!=GArea.X1 || PrevArea.X2!=GArea.X2 ||
      PrevArea.Y1!=GArea.Y1 || PrevArea.Y2!=GArea.Y2)
  {
    int X1,Y1,X2,Y2;
    X1=Min(GArea.X1,GArea.X2);
    X2=Max(GArea.X1,GArea.X2);
    Y1=Min(GArea.Y1,GArea.Y2);
    Y2=Max(GArea.Y1,GArea.Y2);
    if (X1>Min(PrevArea.X1,PrevArea.X2) || X2<Max(PrevArea.X1,PrevArea.X2) ||
        Y1>Min(PrevArea.Y1,PrevArea.Y2) || Y2<Max(PrevArea.Y1,PrevArea.Y2))
      SaveScr->RestoreArea(FALSE);
    if (GArea.X1!=-1)
    {
      CHAR_INFO *CharBuf=new CHAR_INFO[(X2-X1+1)*(Y2-Y1+1)];
      CHAR_INFO *PrevBuf=SaveScr->GetBufferAddress();
      GetText(X1,Y1,X2,Y2,CharBuf);
      for (int X=X1;X<=X2;X++)
        for (int Y=Y1;Y<=Y2;Y++)
        {
          int NewColor;
          if ((PrevBuf[X+Y*(ScrX+1)].Attributes & B_LIGHTGRAY)==B_LIGHTGRAY)
            NewColor=B_BLACK|F_LIGHTGRAY;
          else
            NewColor=B_LIGHTGRAY|F_BLACK;
          int Pos=(X-X1)+(Y-Y1)*(X2-X1+1);
          CharBuf[Pos].Attributes=(CharBuf[Pos].Attributes & ~0xff) | NewColor;
        }
      PutText(X1,Y1,X2,Y2,CharBuf);
      /* $ 13.07.2000 SVS
         раз вызывали new[], то нужно вызывать delete[]
      */
      delete[] CharBuf;
      /* SVS $ */
    }
    PrevArea=GArea;
  }
}


int Grabber::ProcessKey(int Key)
{
  /* $ 14.03.2001 SVS
    [-] Неправильно воспроизводился макрос в режиме грабления экрана.
        При воспроизведении клавиша Home перемещала курсор в координаты
        0,0 консоли.
    Не было учтено режима выполнения макроса.
  */
  if ((ShiftPressed || CtrlObject->Macro.IsExecuting() && (Key&KEY_SHIFT)) &&
     Key!=KEY_NONE && ResetArea)
  {
    GArea.X1=GArea.X2=GArea.CurX;
    GArea.Y1=GArea.Y2=GArea.CurY;
    ResetArea=FALSE;
  }
  else
    if (Key!=KEY_NONE && Key!=KEY_SHIFT &&
        (!ShiftPressed || (CtrlObject->Macro.IsExecuting() && !(Key&KEY_SHIFT))))
      ResetArea=TRUE;
  /* SVS $ */

  switch(Key)
  {
    case KEY_ESC:
      SetExitCode(0);
      break;
    case KEY_ENTER:
    case KEY_CTRLINS:
    case KEY_CTRLADD:
      CopyGrabbedArea(Key==KEY_CTRLADD);
      SetExitCode(1);
      break;
    case KEY_LEFT:
      if (GArea.CurX>0)
        GArea.CurX--;
      break;
    case KEY_RIGHT:
      if (GArea.CurX<ScrX)
        GArea.CurX++;
      break;
    case KEY_UP:
      if (GArea.CurY>0)
        GArea.CurY--;
      break;
    case KEY_DOWN:
      if (GArea.CurY<ScrY)
        GArea.CurY++;
      break;
    case KEY_HOME:
      GArea.CurX=0;
      break;
    case KEY_END:
      GArea.CurX=ScrX;
      break;
    case KEY_PGUP:
      GArea.CurY=0;
      break;
    case KEY_PGDN:
      GArea.CurY=ScrY;
      break;
    case KEY_CTRLHOME:
      GArea.CurX=GArea.CurY=0;
      break;
    case KEY_CTRLEND:
      GArea.CurX=ScrX;
      GArea.CurY=ScrY;
      break;
    case KEY_CTRLLEFT:
      if ((GArea.CurX-=10)<0)
        GArea.CurX=0;
      break;
    case KEY_CTRLRIGHT:
      if ((GArea.CurX+=10)>ScrX)
        GArea.CurX=ScrX;
      break;
    case KEY_CTRLUP:
      if ((GArea.CurY-=5)<0)
        GArea.CurY=0;
      break;
    case KEY_CTRLDOWN:
      if ((GArea.CurY+=5)>ScrY)
        GArea.CurY=ScrY;
      break;
    case KEY_SHIFTLEFT:
      if (GArea.X1>0)
        GArea.X1--;
      GArea.CurX=GArea.X1;
      break;
    case KEY_SHIFTRIGHT:
      if (GArea.X1<ScrX)
        GArea.X1++;
      GArea.CurX=GArea.X1;
      break;
    case KEY_SHIFTUP:
      if (GArea.Y1>0)
        GArea.Y1--;
      GArea.CurY=GArea.Y1;
      break;
    case KEY_SHIFTDOWN:
      if (GArea.Y1<ScrY)
        GArea.Y1++;
      GArea.CurY=GArea.Y1;
      break;
    case KEY_SHIFTHOME:
      GArea.CurX=GArea.X1=0;
      break;
    case KEY_SHIFTEND:
      GArea.CurX=GArea.X1=ScrX;
      break;
    case KEY_SHIFTPGUP:
      GArea.CurY=GArea.Y1=0;
      break;
    case KEY_SHIFTPGDN:
      GArea.CurY=GArea.Y1=ScrY;
      break;
  }
  DisplayObject();
  return(TRUE);
}


int Grabber::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (MouseEvent->dwEventFlags==DOUBLE_CLICK ||
      MouseEvent->dwEventFlags==0 && (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED))
  {
    ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  if (!LButtonPressed)
    return(FALSE);
  /* $ 07.02.2001 SVS
     Бага с грабилкой...
     Забыли ввести проверку на вынос координат за пределы экрана.
  */
  GArea.CurX=(MouseX<ScrX?(MouseX<0?0:MouseX):ScrX);
  GArea.CurY=(MouseY<ScrY?(MouseY<0?0:MouseY):ScrY);
  /* SVS $ */
  if (MouseEvent->dwEventFlags==0)
    ResetArea=TRUE;
  else
    if (MouseEvent->dwEventFlags==MOUSE_MOVED)
    {
      if (ResetArea)
      {
        GArea.X2=GArea.CurX;
        GArea.Y2=GArea.CurY;
        ResetArea=FALSE;
      }
      GArea.X1=GArea.CurX;
      GArea.Y1=GArea.CurY;
    }
  DisplayObject();
  return(TRUE);
}
