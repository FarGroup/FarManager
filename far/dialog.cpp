/*
dialog.cpp

Класс диалога

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


Dialog::Dialog(struct DialogItem *Item,int ItemCount)
{
  CreateObjects=FALSE;
  InitObjects=FALSE;
  DialogTooLong=FALSE;
  Dialog::Item=Item;
  Dialog::ItemCount=ItemCount;
  WarningStyle=0;
  if (CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    CtrlObject->Macro.SetMode(MACRO_DIALOG);
  }
  GetConsoleTitle(OldConsoleTitle,sizeof(OldConsoleTitle));
}


Dialog::~Dialog()
{
  GetDialogObjectsData();
  DeleteDialogObjects();
  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
  Hide();
  ScrBuf.Flush();
  INPUT_RECORD rec;
  PeekInputRecord(&rec);
  SetConsoleTitle(OldConsoleTitle);
}


void Dialog::Show()
{
  if (X1==-1)
  {
    X1=(ScrX-X2+1)/2;
    if (X1<=0)
    {
      DialogTooLong=X2-1;
      X1=0;
      X2=ScrX;
    }
    else
      X2+=X1-1;
  }
  if (Y1==-1)
  {
    Y1=(ScrY-Y2+1)/2;
    if (Y1>1)
      Y1--;
    if (Y1>5)
      Y1--;
    if (Y1<0)
    {
      Y1=0;
      Y2=ScrY;
    }
    else
      Y2+=Y1-1;
  }
  ScreenObject::Show();
}


void Dialog::DisplayObject()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  Shadow();
  if (!InitObjects)
  {
    InitDialogObjects();
    InitObjects=TRUE;
  }
  ShowDialog();
}


/*
Инициализация элементов диалога.
*/
void Dialog::InitDialogObjects()
{
  for (int I=0,TitleSet=0;I<ItemCount;I++)
  {
    struct DialogItem *CurItem=&Item[I];
    if (CurItem->Type==DI_BUTTON && (CurItem->Flags & DIF_NOBRACKETS)==0 &&
        *CurItem->Data!='[')
    {
      char BracketedTitle[200];
      sprintf(BracketedTitle,"[ %s ]",CurItem->Data);
      strcpy(CurItem->Data,BracketedTitle);
    }
    if (!TitleSet && (CurItem->Type==DI_TEXT || CurItem->Type==DI_DOUBLEBOX || CurItem->Type==DI_SINGLEBOX))
      for (int J=0;CurItem->Data[J]!=0;J++)
        if (LocalIsalpha(CurItem->Data[J]))
        {
          SetFarTitle(CurItem->Data+J);
          TitleSet=TRUE;
          break;
        }
  }

  for (int I=0;I<ItemCount;I++)
  {
    struct DialogItem *CurItem=&Item[I];

    if ((CurItem->Flags & DIF_CENTERGROUP) && (I==0 ||
        (Item[I-1].Flags & DIF_CENTERGROUP)==0 || Item[I-1].Y1!=CurItem->Y1))
    {
      int J,Length=0,StartX;
      for (J=I;J<ItemCount && (Item[J].Flags & DIF_CENTERGROUP) && Item[J].Y1==Item[I].Y1;J++)
      {
        Length+=HiStrlen(Item[J].Data);
        if (Item[J].Type==DI_BUTTON && *Item[J].Data!=' ')
          Length+=2;
      }
      if (Item[I].Type==DI_BUTTON && *Item[I].Data!=' ')
        Length-=2;
      StartX=(X2-X1+1-Length)/2;
      if (StartX<0)
        StartX=0;
      for (J=I;J<ItemCount && (Item[J].Flags & DIF_CENTERGROUP) && Item[J].Y1==Item[I].Y1;J++)
      {
        Item[J].X1=StartX;
        StartX+=HiStrlen(Item[J].Data);
        if (Item[J].Type==DI_BUTTON && *Item[J].Data!=' ')
          StartX+=2;
      }
    }
    if (IsEdit(CurItem->Type))
    {
      if (!CreateObjects)
        CurItem->ObjPtr=new Edit;
      Edit *DialogEdit=(Edit *)CurItem->ObjPtr;
      DialogEdit->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                              X1+CurItem->X2,Y1+CurItem->Y2);
      DialogEdit->SetObjectColor(WarningStyle ? COL_WARNDIALOGEDIT:COL_DIALOGEDIT,COL_DIALOGEDITSELECTED);
      if (CurItem->Type==DI_PSWEDIT)
        DialogEdit->SetPasswordMode(TRUE);
      if (CurItem->Type==DI_FIXEDIT)
      {
        DialogEdit->SetMaxLength(CurItem->X2-CurItem->X1+1);
        DialogEdit->SetOvertypeMode(TRUE);
      }
      else
        if (!(CurItem->Flags & DIF_EDITOR))
        {
          DialogEdit->SetEditBeyondEnd(FALSE);
          DialogEdit->SetClearFlag(1);
        }
      DialogEdit->SetString(CurItem->Data);
      if (CurItem->Type==DI_FIXEDIT)
        DialogEdit->SetCurPos(0);
      DialogEdit->FastShow();
    }
  }
  CreateObjects=TRUE;
}


void Dialog::DeleteDialogObjects()
{
  int I;
  for (I=0;I<ItemCount;I++)
    if (IsEdit(Item[I].Type))
    {
      ((Edit *)(Item[I].ObjPtr))->GetString(Item[I].Data,sizeof(Item[I].Data));
      delete (Edit *)(Item[I].ObjPtr);
    }
}

/*
Сохраняет значение из полей редактирования.
При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
  int I;
  for (I=0;I<ItemCount;I++)
    if (IsEdit(Item[I].Type))
    {
      ((Edit *)(Item[I].ObjPtr))->GetString(Item[I].Data,sizeof(Item[I].Data));
      if (ExitCode>=0 && (Item[I].Flags & DIF_HISTORY) && Item[I].Selected && Opt.DialogsEditHistory)
        AddToEditHistory(Item[I].Data,(char *)Item[I].Selected);
    }
}

/*
Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog()
{
  struct DialogItem *CurItem;
  int X,Y;
  int I;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SetScreen(X1,Y1,X2,Y2,' ',WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
  for (I=0;I<ItemCount;I++)
  {
    CurItem=&Item[I];
    switch(CurItem->Type)
    {
      case DI_SINGLEBOX:
      case DI_DOUBLEBOX:
        Box(X1+CurItem->X1,Y1+CurItem->Y1,X1+CurItem->X2,Y1+CurItem->Y2,
            WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX,
            (CurItem->Type==DI_SINGLEBOX) ? SINGLE_BOX:DOUBLE_BOX);
        if (*CurItem->Data)
        {
          char Title[200];
          int X;
          sprintf(Title," %s ",CurItem->Data);
          X=X1+CurItem->X1+(CurItem->X2-CurItem->X1+1-HiStrlen(Title))/2;
          if (CurItem->Flags & DIF_LEFTTEXT && X1+CurItem->X1+1<X)
            X=X1+CurItem->X1+1;
          SetColor(WarningStyle ? COL_WARNDIALOGBOXTITLE:COL_DIALOGBOXTITLE);
          GotoXY(X,Y1+CurItem->Y1);
          HiText(Title,WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT);
        }
        break;
      case DI_TEXT:
        if (CurItem->X1==(unsigned char)-1)
          X=(X2-X1+1-HiStrlen(CurItem->Data))/2;
        else
          X=CurItem->X1;
        if (CurItem->Y1==(unsigned char)-1)
          Y=(Y2-Y1+1)/2;
        else
          Y=CurItem->Y1;
        if (CurItem->Flags & DIF_SETCOLOR)
          SetColor(CurItem->Flags & DIF_COLORMASK);
        else
          if (CurItem->Flags & DIF_BOXCOLOR)
            SetColor(WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX);
          else
            SetColor(WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
        if (CurItem->Flags & DIF_SEPARATOR)
        {
          GotoXY(X1+3,Y1+Y);
          if (DialogTooLong)
            ShowSeparator(DialogTooLong-5);
          else
            ShowSeparator(X2-X1-5);
        }
        GotoXY(X1+X,Y1+Y);
        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(CurItem->Data);
        else
          HiText(CurItem->Data,WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT);
        break;
      case DI_VTEXT:
        if (CurItem->Flags & DIF_BOXCOLOR)
          SetColor(WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX);
        else
          if (CurItem->Flags & DIF_SETCOLOR)
            SetColor(CurItem->Flags & DIF_COLORMASK);
          else
            SetColor(WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);
        VText(CurItem->Data);
        break;
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
        if (CurItem->Focus)
        {
          SetCursorType(1,-1);
          ((Edit *)(CurItem->ObjPtr))->Show();
        }
        else
          ((Edit *)(CurItem->ObjPtr))->FastShow();
        if ((CurItem->Flags & DIF_HISTORY) && Opt.DialogsEditHistory && CurItem->Selected)
        {
          int EditX1,EditY1,EditX2,EditY2;
          ((Edit *)(CurItem->ObjPtr))->GetPosition(EditX1,EditY1,EditX2,EditY2);
          SetColor(COL_DIALOGTEXT);
          GotoXY(EditX2+1,EditY1);
          Text("");
        }
        break;
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
        if (CurItem->Flags & DIF_SETCOLOR)
          SetColor(CurItem->Flags & DIF_COLORMASK);
        else
          SetColor(WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);
        if (CurItem->Type==DI_CHECKBOX)
          mprintf("[%c] ",CurItem->Selected ? 'x':' ');
        else
          if (CurItem->Flags & DIF_MOVESELECT)
            mprintf(" %c ",CurItem->Selected ? '\07':' ');
          else
            mprintf("(%c) ",CurItem->Selected ? '\07':' ');
        HiText(CurItem->Data,WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT);
        if (CurItem->Focus)
        {
          SetCursorType(1,-1);
          MoveCursor(X1+CurItem->X1+1,Y1+CurItem->Y1);
        }
        break;
      case DI_BUTTON:
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);
        if (CurItem->Focus)
        {
          SetCursorType(0,10);
          SetColor(WarningStyle ? COL_WARNDIALOGSELECTEDBUTTON:COL_DIALOGSELECTEDBUTTON);
          HiText(CurItem->Data,WarningStyle ? COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON:COL_DIALOGHIGHLIGHTSELECTEDBUTTON);
        }
        else
        {
          SetColor(WarningStyle ? COL_WARNDIALOGBUTTON:COL_DIALOGBUTTON);
          HiText(CurItem->Data,WarningStyle ? COL_WARNDIALOGHIGHLIGHTBUTTON:COL_DIALOGHIGHLIGHTBUTTON);
        }
        break;
    }
  }
}

/*
Обработка данных от клавиатуры.
Перекрывает BaseInput::ProcessKey.
*/
int Dialog::ProcessKey(int Key)
{
  int FocusPos=0,I;

  if (Key==KEY_NONE || Key==KEY_IDLE)
    return(FALSE);

  for (I=0;I<ItemCount;I++)
    if (Item[I].Focus)
    {
      FocusPos=I;
      break;
    }
  int Type=Item[FocusPos].Type;

  switch(Key)
  {
    case KEY_F1:
      ShowHelp();
      return(TRUE);
    case KEY_TAB:
    case KEY_SHIFTTAB:
      if (Item[FocusPos].Flags & DIF_EDITOR)
      {
        I=FocusPos;
        while (Item[I].Flags & DIF_EDITOR)
          I=ChangeFocus(I,(Key==KEY_TAB) ? 1:-1,TRUE);
      }
      else
      {
        I=ChangeFocus(FocusPos,(Key==KEY_TAB) ? 1:-1,TRUE);
        if (Key==KEY_SHIFTTAB)
          while (I>0 && (Item[I].Flags & DIF_EDITOR)!=0 &&
                 (Item[I-1].Flags & DIF_EDITOR)!=0 &&
                 ((Edit *)Item[I].ObjPtr)->GetLength()==0)
            I--;
      }
      Item[FocusPos].Focus=0;
      Item[I].Focus=1;
      ShowDialog();
      return(TRUE);
    case KEY_CTRLENTER:
      EndLoop=TRUE;
      for (I=0;I<ItemCount;I++)
        if (Item[I].DefaultButton)
        {
          if (!IsEdit(Item[I].Type))
            Item[I].Selected=1;
          ExitCode=I;
          return(TRUE);
        }
    case KEY_ENTER:
      if (Item[FocusPos].Flags & DIF_EDITOR)
      {
        int EditorLastPos;
        for (EditorLastPos=I=FocusPos;I<ItemCount;I++)
          if (IsEdit(Item[I].Type) && (Item[I].Flags & DIF_EDITOR))
            EditorLastPos=I;
          else
            break;
        if (((Edit *)(Item[EditorLastPos].ObjPtr))->GetLength()!=0)
          return(TRUE);
        for (I=EditorLastPos;I>FocusPos;I--)
        {
          char Str[1024];
          int CurPos;
          if (I==FocusPos+1)
            CurPos=((Edit *)(Item[I-1].ObjPtr))->GetCurPos();
          else
            CurPos=0;
          ((Edit *)(Item[I-1].ObjPtr))->GetString(Str,sizeof(Str));
          int Length=strlen(Str);
          ((Edit *)(Item[I].ObjPtr))->SetString(CurPos>=Length ? "":Str+CurPos);
          if (CurPos<Length)
            Str[CurPos]=0;
          ((Edit *)(Item[I].ObjPtr))->SetCurPos(0);
          ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
        }
        if (EditorLastPos>FocusPos)
        {
          ((Edit *)(Item[FocusPos].ObjPtr))->SetCurPos(0);
          ProcessKey(KEY_DOWN);
        }
        else
          ShowDialog();
        return(TRUE);
      }
      EndLoop=TRUE;
      if (Type==DI_BUTTON)
      {
        Item[FocusPos].Selected=1;
        ExitCode=FocusPos;
      }
      else
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            if (!IsEdit(Item[I].Type))
              Item[I].Selected=1;
            ExitCode=I;
          }
      if (ExitCode==-1)
        ExitCode=FocusPos;
      return(TRUE);
    case KEY_ESC:
    case KEY_BREAK:
    case KEY_F10:
      EndLoop=TRUE;
      ExitCode=(Key==KEY_BREAK) ? -2:-1;
      return(TRUE);
    case KEY_ADD:
      if (Type==DI_CHECKBOX && !Item[FocusPos].Selected)
        ProcessKey(KEY_SPACE);
      else
        ProcessKey('+');
      return(TRUE);
    case KEY_SUBTRACT:
      if (Type==DI_CHECKBOX && Item[FocusPos].Selected)
        ProcessKey(KEY_SPACE);
      else
        ProcessKey('-');
      return(TRUE);
    case KEY_SPACE:
      if (Type==DI_BUTTON)
        return(ProcessKey(KEY_ENTER));
      if (Type==DI_CHECKBOX)
      {
        Item[FocusPos].Selected=!Item[FocusPos].Selected;
        ShowDialog();
        return(TRUE);
      }
      if (Type==DI_RADIOBUTTON)
      {
        for (I=FocusPos;;I--)
          if (Item[I].Type==DI_RADIOBUTTON && (Item[I].Flags & DIF_GROUP) ||
              I==0 || Item[I-1].Type!=DI_RADIOBUTTON)
            break;
        do
        {
          Item[I++].Selected=0;
        } while (I<ItemCount && Item[I].Type==DI_RADIOBUTTON && (Item[I].Flags & DIF_GROUP)==0);
        Item[FocusPos].Selected=1;
        ShowDialog();
        return(TRUE);
      }
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      return(TRUE);
    case KEY_HOME:
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      for (I=0;I<ItemCount;I++)
        if (IsEdit(Item[I].Type) || Item[I].Type==DI_BUTTON ||
            Item[I].Type==DI_CHECKBOX || Item[I].Type==DI_RADIOBUTTON)
        {
          Item[FocusPos].Focus=0;
          Item[I].Focus=1;
          ShowDialog();
          return(TRUE);
        }
      return(TRUE);
    case KEY_LEFT:
    case KEY_RIGHT:
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      {
        int MinDist=1000,MinPos;
        for (I=0;I<ItemCount;I++)
          if (I!=FocusPos && (IsEdit(Item[I].Type) || Item[I].Type==DI_CHECKBOX ||
              Item[I].Type==DI_RADIOBUTTON) && Item[I].Y1==Item[FocusPos].Y1)
          {
            int Dist=Item[I].X1-Item[FocusPos].X1;
            if (Key==KEY_LEFT && Dist<0 || Key==KEY_RIGHT && Dist>0)
              if (abs(Dist)<MinDist)
              {
                MinDist=abs(Dist);
                MinPos=I;
              }
          }
          if (MinDist<1000)
          {
            Item[FocusPos].Focus=0;
            Item[MinPos].Focus=1;
            if (Item[MinPos].Flags & DIF_MOVESELECT)
              ProcessKey(KEY_SPACE);
            else
              ShowDialog();
            return(TRUE);
          }
      }
    case KEY_UP:
    case KEY_DOWN:
      {
        int PrevPos=0;
        if (Item[FocusPos].Flags & DIF_EDITOR)
          PrevPos=((Edit *)(Item[FocusPos].ObjPtr))->GetCurPos();
        I=ChangeFocus(FocusPos,(Key==KEY_LEFT || Key==KEY_UP) ? -1:1,FALSE);
        Item[FocusPos].Focus=0;
        Item[I].Focus=1;
        if (Item[I].Flags & DIF_EDITOR)
          ((Edit *)(Item[I].ObjPtr))->SetCurPos(PrevPos);
        if (Item[I].Flags & DIF_MOVESELECT)
          ProcessKey(KEY_SPACE);
        else
          ShowDialog();
      }
      return(TRUE);
    case KEY_END:
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
    case KEY_PGDN:
      if (!(Item[FocusPos].Flags & DIF_EDITOR))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            Item[FocusPos].Focus=0;
            Item[I].Focus=1;
            ShowDialog();
            return(TRUE);
          }
      }
      else
      {
        ProcessKey(KEY_TAB);
        ProcessKey(KEY_UP);
      }
      return(TRUE);
    case KEY_CTRLUP:
    case KEY_CTRLDOWN:
      if (IsEdit(Type) &&
           (Item[FocusPos].Flags & DIF_HISTORY) &&
           Opt.DialogsEditHistory &&
           Item[FocusPos].Selected)
        SelectFromEditHistory((Edit *)(Item[FocusPos].ObjPtr),(char *)Item[FocusPos].Selected);
      return(TRUE);
    default:
      if (IsEdit(Type))
      {
        if (Item[FocusPos].Flags & DIF_EDITOR)
          switch(Key)
          {
            case KEY_CTRLY:
              for (I=FocusPos;I<ItemCount;I++)
                if (Item[I].Flags & DIF_EDITOR)
                {
                  if (I>FocusPos)
                  {
                    char Str[1024];
                    ((Edit *)(Item[I].ObjPtr))->GetString(Str,sizeof(Str));
                    ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
                  }
                  ((Edit *)(Item[I].ObjPtr))->SetString("");
                }
                else
                  break;
              ShowDialog();
              return(TRUE);
            case KEY_DEL:
              if (FocusPos<ItemCount+1 && (Item[FocusPos+1].Flags & DIF_EDITOR))
              {
                int CurPos=((Edit *)(Item[FocusPos].ObjPtr))->GetCurPos();
                int Length=((Edit *)(Item[FocusPos].ObjPtr))->GetLength();
                if (CurPos>=Length)
                {
                  char Str[1024];
                  ((Edit *)(Item[FocusPos].ObjPtr))->GetString(Str,sizeof(Str));
                  int Length=strlen(Str);
                  ((Edit *)(Item[FocusPos+1].ObjPtr))->GetString(Str+Length,sizeof(Str)-Length);
                  ((Edit *)(Item[FocusPos+1].ObjPtr))->SetString(Str);
                  ProcessKey(KEY_CTRLY);
                  ((Edit *)(Item[FocusPos].ObjPtr))->SetCurPos(CurPos);
                  ShowDialog();
                  return(TRUE);
                }
              }
              break;
            case KEY_PGUP:
              ProcessKey(KEY_SHIFTTAB);
              ProcessKey(KEY_DOWN);
              return(TRUE);
          }
        if (((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key))
          return(TRUE);
      }
      if (ProcessHighlighting(Key,FocusPos,FALSE))
        return(TRUE);
      return(ProcessHighlighting(Key,FocusPos,TRUE));
  }
}


/*
Обработка данных от "мыши".
Перекрывает BaseInput::ProcessMouse.
*/
int Dialog::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int FocusPos=0,I;
  int MsX,MsY;

  if (MouseEvent->dwButtonState==0)
    return(FALSE);

  for (I=0;I<ItemCount;I++)
    if (Item[I].Focus)
    {
      FocusPos=I;
      break;
    }

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  if (MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2)
  {
    if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
      ProcessKey(KEY_ESC);
    else
      ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  if (MouseEvent->dwEventFlags==0 && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
  {
    for (I=0;I<ItemCount;I++)
      if (MsX>=X1+Item[I].X1)
      {
        if (IsEdit(Item[I].Type))
        {
          Edit *EditLine=(Edit *)(Item[I].ObjPtr);
          if (EditLine->ProcessMouse(MouseEvent))
          {
            EditLine->SetClearFlag(0);
            Item[FocusPos].Focus=0;
            Item[I].Focus=1;
            ShowDialog();
            return(TRUE);
          }
          else
          {
            int EditX1,EditY1,EditX2,EditY2;
            EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
            if (MsX==EditX2+1 && MsY==EditY1 &&
                (Item[I].Flags & DIF_HISTORY) && Opt.DialogsEditHistory &&
                Item[I].Selected)
            {
              Item[FocusPos].Focus=0;
              Item[I].Focus=1;
              ProcessKey(KEY_CTRLDOWN);
              return(TRUE);
            }
          }
        }
        if (Item[I].Type==DI_BUTTON && MsY==Y1+Item[I].Y1 &&
            MsX < X1+Item[I].X1+HiStrlen(Item[I].Data))
        {
          Item[FocusPos].Focus=0;
          Item[I].Focus=1;
          ShowDialog();
          while (IsMouseButtonPressed())
            ;
          if (MouseX<X1 || MouseX>X1+Item[I].X1+HiStrlen(Item[I].Data)+4 ||
              MouseY!=Y1+Item[I].Y1)
          {
            Item[FocusPos].Focus=1;
            Item[I].Focus=0;
            ShowDialog();
            return(TRUE);
          }
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }

        if ((Item[I].Type==DI_CHECKBOX || Item[I].Type==DI_RADIOBUTTON) &&
            MsY==Y1+Item[I].Y1 &&
            MsX < (X1+Item[I].X1+HiStrlen(Item[I].Data)+4-((Item[I].Flags & DIF_MOVESELECT)!=0)))
        {
          Item[FocusPos].Focus=0;
          Item[I].Focus=1;
          ProcessKey(KEY_SPACE);
          return(TRUE);
        }
      }
  }
  return(FALSE);
}


/*
Изменяет фокус ввода (воздействие клавишами KEY_TAB, KEY_SHIFTTAB,
KEY_UP, KEY_DOWN, а так же Alt-HotKey)
*/
int Dialog::ChangeFocus(int FocusPos,int Step,int SkipGroup)
{
  while (1)
  {
    FocusPos+=Step;
    if (FocusPos>=ItemCount)
      FocusPos=0;
    if (FocusPos<0)
      FocusPos=ItemCount-1;
    int Type=Item[FocusPos].Type;
    if (Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type))
      break;
    if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[FocusPos].Selected))
      break;
  }
  return(FocusPos);
}

/*
Статический метод - преобразует данные об элементах диалога во внутреннее
представление. Аналогичен функции InitDialogItems (см. "Far PlugRinG
Russian Help Encyclopedia of Developer")
*/
void Dialog::DataToItem(struct DialogData *Data,struct DialogItem *Item,
                        int Count)
{
  int I;
  for (I=0;I<Count;I++)
  {
    Item[I].Type=Data[I].Type;
    Item[I].X1=Data[I].X1;
    Item[I].Y1=Data[I].Y1;
    Item[I].X2=Data[I].X2;
    Item[I].Y2=Data[I].Y2;
    Item[I].Focus=Data[I].Focus;
    Item[I].Selected=Data[I].Selected;
    Item[I].Flags=Data[I].Flags;
    Item[I].DefaultButton=Data[I].DefaultButton;
    if ((unsigned int)Data[I].Data<MAX_MSG)
      strcpy(Item[I].Data,MSG((unsigned int)Data[I].Data));
    else
      strcpy(Item[I].Data,Data[I].Data);
    Item[I].ObjPtr=0;
  }
}

/*
Проверяет тип элемента диалога на предмет строки ввода
(DI_EDIT, DI_FIXEDIT, DI_PSWEDIT) и в случае успеха возвращает TRUE
*/
int Dialog::IsEdit(int Type)
{
  return(Type==DI_EDIT || Type==DI_FIXEDIT || Type==DI_PSWEDIT);
}


void Dialog::SelectFromEditHistory(Edit *EditLine,char *HistoryName)
{
  char RegKey[80],KeyValue[80],Str[512];
  sprintf(RegKey,"SavedDialogHistory\\%s",HistoryName);
  {
    // создание пустого вертикального меню
    VMenu HistoryMenu("",NULL,0,8);
    struct MenuItem HistoryItem;
    int EditX1,EditY1,EditX2,EditY2;
    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
    if (EditX2>ScrX)
      EditX2=ScrX;
    HistoryItem.Checked=HistoryItem.Separator=0;
    HistoryMenu.SetFlags(MENU_SHOWAMPERSAND);
    HistoryMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
    HistoryMenu.SetBoxType(SHORT_SINGLE_BOX);

    // заполнение пунктов меню
    int ItemsCount=0;
    for (int I=0;I<16;I++)
    {
      sprintf(KeyValue,"Line%d",I);
      GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
      if (*Str==0)
        continue;
      sprintf(KeyValue,"Locked%d",I);
      int Checked;
      GetRegKey(RegKey,KeyValue,(int)Checked,0);
      HistoryItem.Checked=Checked;
      HistoryItem.Selected=FALSE;
      strncpy(HistoryItem.Name,Str,sizeof(HistoryItem.Name)-1);
      HistoryItem.Name[sizeof(HistoryItem.Name)-1]=0;
      strncpy(HistoryItem.UserData,Str,sizeof(HistoryItem.UserData));
      HistoryItem.UserDataSize=strlen(Str)+1;
      HistoryMenu.AddItem(&HistoryItem);
      ItemsCount++;
    }
    if (ItemsCount==0)
      return;

    HistoryMenu.Show();
    while (!HistoryMenu.Done())
    {
      int Key=HistoryMenu.ReadInput();

      // Del очищает историю команд.
      if (Key==KEY_DEL)
      {
        for (int I=0,Dest=0;I<16;I++)
        {
          sprintf(KeyValue,"Line%d",I);
          GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
          DeleteRegValue(RegKey,KeyValue);
          int Locked;
          sprintf(KeyValue,"Locked%d",I);
          GetRegKey(RegKey,KeyValue,Locked,0);
          DeleteRegValue(RegKey,KeyValue);

          // залоченные пункты истории не удаляются
          if (Locked)
          {
            sprintf(KeyValue,"Line%d",Dest);
            SetRegKey(RegKey,KeyValue,Str);
            sprintf(KeyValue,"Locked%d",Dest);
            SetRegKey(RegKey,KeyValue,TRUE);
            Dest++;
          }
        }
        HistoryMenu.Hide();
        SelectFromEditHistory(EditLine,HistoryName);
        return;
      }

      // Ins защищает пункт истории от удаления.
      if (Key==KEY_INS)
      {
        sprintf(KeyValue,"Locked%d",HistoryMenu.GetSelectPos());
        if (!HistoryMenu.GetSelection())
        {
          HistoryMenu.SetSelection(TRUE);
          SetRegKey(RegKey,KeyValue,1);
        }
        else
        {
          HistoryMenu.SetSelection(FALSE);
          DeleteRegValue(RegKey,KeyValue);
        }
        HistoryMenu.SetUpdateRequired(TRUE);
        HistoryMenu.Redraw();
        continue;
      }
      HistoryMenu.ProcessInput();
    }

    int ExitCode=HistoryMenu.GetExitCode();
    if (ExitCode<0)
      return;
    HistoryMenu.GetUserData(Str,sizeof(Str),ExitCode);
  }
  EditLine->SetString(Str);
  EditLine->SetLeftPos(0);
  Redraw();
}


void Dialog::AddToEditHistory(char *AddStr,char *HistoryName)
{
  int LastLine=15,FirstLine=16;
  if (*AddStr==0)
    return;
  char RegKey[80],SrcKeyValue[80],DestKeyValue[80],Str[512];
  sprintf(RegKey,"SavedDialogHistory\\%s",HistoryName);
  for (int I=0;I<16;I++)
  {
    sprintf(SrcKeyValue,"Locked%d",I);
    int Locked;
    GetRegKey(RegKey,SrcKeyValue,Locked,0);
    if (!Locked)
    {
      FirstLine=I;
      break;
    }
  }
  for (int I=0;I<16;I++)
  {
    sprintf(SrcKeyValue,"Line%d",I);
    GetRegKey(RegKey,SrcKeyValue,Str,"",sizeof(Str));
    if (strcmp(Str,AddStr)==0)
    {
      LastLine=I;
      break;
    }
  }
  if (FirstLine<=LastLine)
  {
    for (int Src=LastLine-1;Src>=FirstLine;Src--)
    {
      int Locked;
      sprintf(SrcKeyValue,"Locked%d",Src);
      GetRegKey(RegKey,SrcKeyValue,Locked,0);
      if (Locked)
        continue;
      for (int Dest=Src+1;Dest<=LastLine;Dest++)
      {
        sprintf(DestKeyValue,"Locked%d",Dest);
        GetRegKey(RegKey,DestKeyValue,Locked,0);
        if (!Locked)
        {
          sprintf(SrcKeyValue,"Line%d",Src);
          GetRegKey(RegKey,SrcKeyValue,Str,"",sizeof(Str));
          sprintf(DestKeyValue,"Line%d",Dest);
          SetRegKey(RegKey,DestKeyValue,Str);
          break;
        }
      }
    }
    char FirstLineKeyValue[20];
    sprintf(FirstLineKeyValue,"Line%d",FirstLine);
    SetRegKey(RegKey,FirstLineKeyValue,AddStr);
  }
}


int Dialog::IsKeyHighlighted(char *Str,int Key,int Translate)
{
  if ((Str=strchr(Str,'&'))==NULL)
    return(FALSE);
  int UpperStrKey=LocalUpper(Str[1]);
  if (Key<256)
    return(UpperStrKey==LocalUpper(Key) ||
           Translate && UpperStrKey==LocalUpper(LocalKeyToKey(Key)));
  if (Key>=KEY_ALT0 && Key<=KEY_ALT9)
    return(Key-KEY_ALT0+'0'==UpperStrKey);
  if (Key>=KEY_ALTA && Key<=KEY_ALT_BASE+255)
  {
    int AltKey=Key-KEY_ALTA+'A';
    return(UpperStrKey==LocalUpper(AltKey) ||
           Translate && UpperStrKey==LocalUpper(LocalKeyToKey(AltKey)));
  }
  return(FALSE);
}


int Dialog::ProcessHighlighting(int Key,int FocusPos,int Translate)
{
  for (int I=0;I<ItemCount;I++)
  {
    if (!IsEdit(Item[I].Type) && (Item[I].Flags & DIF_SHOWAMPERSAND)==0)
      if (IsKeyHighlighted(Item[I].Data,Key,Translate))
      {
        int DisableSelect=FALSE;
        if (I>0 && Item[I].Type==DI_TEXT && IsEdit(Item[I-1].Type) &&
            Item[I].Y1==Item[I-1].Y1 && Item[I].Y1!=Item[I+1].Y1)
        {
          I=ChangeFocus(I,-1,FALSE);
          DisableSelect=TRUE;
        }
        else
          if (Item[I].Type==DI_TEXT || Item[I].Type==DI_VTEXT ||
              Item[I].Type==DI_SINGLEBOX || Item[I].Type==DI_DOUBLEBOX)
          {
            I=ChangeFocus(I,1,FALSE);
            DisableSelect=TRUE;
          }
        Item[FocusPos].Focus=0;
        Item[I].Focus=1;
        if ((Item[I].Type==DI_CHECKBOX || Item[I].Type==DI_RADIOBUTTON) &&
            (!DisableSelect || (Item[I].Flags & DIF_MOVESELECT)))
        {
          ProcessKey(KEY_SPACE);
          return(TRUE);
        }
        if (Item[I].Type==DI_BUTTON)
        {
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
        ShowDialog();
        return(TRUE);
      }
  }
  return(FALSE);
}
