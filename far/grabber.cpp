/*
grabber.cpp

Screen grabber

*/

#include "headers.hpp"
#pragma hdrstop

#include "grabber.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "frame.hpp"

#if defined(USE_WFUNC)
extern WCHAR Oem2Unicode[];
#endif

Grabber::Grabber()
{
  Frame *pFrame = FrameManager->GetCurrentFrame ();

  pFrame->Lock();

  SaveScr=new SaveScreen;

  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_OTHER);

  /* $ 14.08.2000 tran
     хоть и не решает баг, но уже на трапаетс€ */
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
  VerticalBlock=FALSE;

  Process();

  delete SaveScr;

  pFrame->Unlock();
  FrameManager->RefreshFrame ();
}


Grabber::~Grabber()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
}


void Grabber::CopyGrabbedArea(int Append, int VerticalBlock)
{
  if (GArea.X1 < 0)
    return;
  int X1,Y1,X2,Y2;
  X1=Min(GArea.X1,GArea.X2);
  X2=Max(GArea.X1,GArea.X2);
  Y1=Min(GArea.Y1,GArea.Y2);
  Y2=Max(GArea.Y1,GArea.Y2);
  int GWidth=X2-X1+1,GHeight=Y2-Y1+1;
  int BufSize=(GWidth+3)*GHeight;
  CHAR_INFO *CharBuf=new CHAR_INFO[BufSize], *PtrCharBuf;
  char *CopyBuf=(char *)xf_malloc(BufSize), *PtrCopyBuf;
  WORD Chr;
  GetText(X1,Y1,X2,Y2,CharBuf,BufSize*sizeof(CHAR_INFO));
  *CopyBuf=0;
  PtrCharBuf=CharBuf;
  PtrCopyBuf=CopyBuf;
  for (int I=0;I<GHeight;I++)
  {
    if (I>0)
    {
      *PtrCopyBuf++='\r';
      *PtrCopyBuf++='\n';
      *PtrCopyBuf=0;
    }

    for (int J=0;J<GWidth;J++, ++PtrCharBuf)
    {
      WORD Chr2=Opt.UseUnicodeConsole?PtrCharBuf->Char.UnicodeChar:(WORD)PtrCharBuf->Char.AsciiChar;
      Chr=GetVidChar(*PtrCharBuf);

           if(Chr2 == 0xBB) Chr=Opt.UseUnicodeConsole?'>':0xBB;
      else if(Chr2 == 0xAB) Chr=Opt.UseUnicodeConsole?'<':0xAB;
      else if(Opt.CleanAscii)
      {
        switch(Chr2)
        {
          case '.':  Chr='.'; break;
          case 0x07: Chr='*'; break;
          case 0x10: Chr='>'; break;
          case 0x11: Chr='<'; break;
          case 0x18:
          case 0x19: Chr='|'; break;
          case 0x1E:
          case 0x1F: Chr='X'; break;
          case 0xFF: Chr=' '; break;
          default:
            if(Chr2 < 0x20)
              Chr='.';
            else if(Chr2 < 0x100)
              Chr=Chr2;
            break;
        }
      }

      if (Opt.NoGraphics && Chr2 >=0xB3 && Chr2 <= 0xDA)
      {
        switch(Chr2)
        {
          case 0xB3:
          case 0xBA: Chr='|'; break;
          case 0xC4: Chr='-'; break;
          case 0xCD: Chr='='; break;
          default:   Chr='+'; break;
        }
      }

      *PtrCopyBuf++=static_cast<char>(Chr);
      *PtrCopyBuf=0;
    }
    for (int K=(int)strlen(CopyBuf)-1;K>=0 && CopyBuf[K]==' ';K--)
      CopyBuf[K]=0;
    PtrCopyBuf=CopyBuf+strlen(CopyBuf);
  }
  if (Append)
  {
    char *AppendBuf=PasteFromClipboard();
    int add=0;
    if (AppendBuf!=NULL)
    {
      int DataSize=(int)strlen(AppendBuf);
      if ( AppendBuf[DataSize-1]!='\n' )
      {
        add=2;
      }
      AppendBuf=(char *)xf_realloc(AppendBuf,DataSize+BufSize+add);
      memcpy(AppendBuf+DataSize+add,CopyBuf,BufSize);
      if ( add )
        memcpy(AppendBuf+DataSize,"\r\n",2);

      xf_free(CopyBuf);
      CopyBuf=AppendBuf;
    }
  }
  if(VerticalBlock)
    CopyFormatToClipboard(FAR_VerticalBlock,CopyBuf);
  else
    CopyToClipboard(CopyBuf);

  if(CopyBuf)
    xf_free(CopyBuf);
  delete[] CharBuf;
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
      GetText(X1,Y1,X2,Y2,CharBuf,sizeof(CHAR_INFO)*(X2-X1+1)*(Y2-Y1+1));

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

      delete[] CharBuf;
    }
    if (GArea.X1==-2)
    {
      SaveScr->RestoreArea(FALSE);
      GArea.X1=GArea.X2;
    }
    PrevArea=GArea;
  }
}


int Grabber::ProcessKey(int Key)
{
  /* $ 14.03.2001 SVS
    [-] Ќеправильно воспроизводилс€ макрос в режиме граблени€ экрана.
        ѕри воспроизведении клавиша Home перемещала курсор в координаты
        0,0 консоли.
    Ќе было учтено режима выполнени€ макроса.
  */
  SetCursorType(TRUE,60);

  if(CtrlObject->Macro.IsExecuting())
  {
    if ((Key&KEY_SHIFT) && ResetArea)
      Reset();
    else if(!(Key&KEY_SHIFT))
      ResetArea=TRUE;
  }
  else
  {
    if ((ShiftPressed || Key!=KEY_SHIFT) && (Key&KEY_SHIFT) && Key!=KEY_NONE && ResetArea)
      Reset();
    else if (Key!=KEY_IDLE && Key!=KEY_NONE && Key!=KEY_SHIFT && !ShiftPressed && !(Key&KEY_SHIFT))
      ResetArea=TRUE;
  }
  /* SVS $ */

  switch(Key)
  {
    case KEY_CTRLU:
      Reset();
      GArea.X1=-2;
      break;
    case KEY_ESC:
      SetExitCode(0);
      break;
    case KEY_NUMENTER:
    case KEY_ENTER:
    case KEY_CTRLINS:   case KEY_CTRLNUMPAD0:
    case KEY_CTRLADD:
      CopyGrabbedArea(Key == KEY_CTRLADD,VerticalBlock);
      SetExitCode(1);
      break;
    case KEY_LEFT:      case KEY_NUMPAD4:   case '4':
      if (GArea.CurX>0)
        GArea.CurX--;
      break;
    case KEY_RIGHT:     case KEY_NUMPAD6:   case '6':
      if (GArea.CurX<ScrX)
        GArea.CurX++;
      break;
    case KEY_UP:        case KEY_NUMPAD8:   case '8':
      if (GArea.CurY>0)
        GArea.CurY--;
      break;
    case KEY_DOWN:      case KEY_NUMPAD2:   case '2':
      if (GArea.CurY<ScrY)
        GArea.CurY++;
      break;
    case KEY_HOME:      case KEY_NUMPAD7:   case '7':
      GArea.CurX=0;
      break;
    case KEY_END:       case KEY_NUMPAD1:   case '1':
      GArea.CurX=ScrX;
      break;
    case KEY_PGUP:      case KEY_NUMPAD9:   case '9':
      GArea.CurY=0;
      break;
    case KEY_PGDN:      case KEY_NUMPAD3:   case '3':
      GArea.CurY=ScrY;
      break;
    case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
      GArea.CurX=GArea.CurY=0;
      break;
    case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
      GArea.CurX=ScrX;
      GArea.CurY=ScrY;
      break;
    case KEY_CTRLLEFT:      case KEY_CTRLNUMPAD4:
    case KEY_CTRLSHIFTLEFT: case KEY_CTRLSHIFTNUMPAD4:
      if ((GArea.CurX-=10)<0)
        GArea.CurX=0;
      if(Key == KEY_CTRLSHIFTLEFT || Key == KEY_CTRLSHIFTNUMPAD4)
        GArea.X1=GArea.CurX;
      break;
    case KEY_CTRLRIGHT:      case KEY_CTRLNUMPAD6:
    case KEY_CTRLSHIFTRIGHT: case KEY_CTRLSHIFTNUMPAD6:
      if ((GArea.CurX+=10)>ScrX)
        GArea.CurX=ScrX;
      if(Key == KEY_CTRLSHIFTRIGHT || Key == KEY_CTRLSHIFTNUMPAD6)
        GArea.X1=GArea.CurX;
      break;
    case KEY_CTRLUP:        case KEY_CTRLNUMPAD8:
    case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8:
      if ((GArea.CurY-=5)<0)
        GArea.CurY=0;
      if(Key == KEY_CTRLSHIFTUP || Key == KEY_CTRLSHIFTNUMPAD8)
        GArea.Y1=GArea.CurY;
      break;
    case KEY_CTRLDOWN:      case KEY_CTRLNUMPAD2:
    case KEY_CTRLSHIFTDOWN: case KEY_CTRLSHIFTNUMPAD2:
      if ((GArea.CurY+=5)>ScrY)
        GArea.CurY=ScrY;
      if(Key == KEY_CTRLSHIFTDOWN || Key == KEY_CTRLSHIFTNUMPAD8)
        GArea.Y1=GArea.CurY;
      break;
    case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
      if (GArea.X1>0)
        GArea.X1--;
      GArea.CurX=GArea.X1;
      break;
    case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
      if (GArea.X1<ScrX)
        GArea.X1++;
      GArea.CurX=GArea.X1;
      break;
    case KEY_SHIFTUP:    case KEY_SHIFTNUMPAD8:
      if (GArea.Y1>0)
        GArea.Y1--;
      GArea.CurY=GArea.Y1;
      break;
    case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
      if (GArea.Y1<ScrY)
        GArea.Y1++;
      GArea.CurY=GArea.Y1;
      break;
    case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
      GArea.CurX=GArea.X1=0;
      break;
    case KEY_SHIFTEND:   case KEY_SHIFTNUMPAD1:
      GArea.CurX=GArea.X1=ScrX;
      break;
    case KEY_SHIFTPGUP:  case KEY_SHIFTNUMPAD9:
      GArea.CurY=GArea.Y1=0;
      break;
    case KEY_SHIFTPGDN:  case KEY_SHIFTNUMPAD3:
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
     Ѕага с грабилкой...
     «абыли ввести проверку на вынос координат за пределы экрана.
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
  //VerticalBlock=MouseEvent->dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED);
  DisplayObject();
  return(TRUE);
}

void Grabber::Reset()
{
  GArea.X1=GArea.X2=GArea.CurX;
  GArea.Y1=GArea.Y2=GArea.CurY;
  ResetArea=FALSE;
  //DisplayObject();
}

BOOL RunGraber(void)
{
  if (!InGrabber)
  {
    InGrabber=TRUE;
    WaitInMainLoop=FALSE;
    FlushInputBuffer();
    Grabber Grabber;
    InGrabber=FALSE;
    return TRUE;
  }
  return FALSE;
}
