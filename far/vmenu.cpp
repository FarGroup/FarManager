/*
vmenu.cpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * ...
*/

/* Revision: 1.14 09.04.2001 $ */

/*
Modify:
  09.04.2001 SVS
    ! Избавимся от некоторых варнингов
  20.02.2001 SVS
    + Добавлена функция SetSelectPos() - переместить курсор с учетом
      Disabled & Separator
    ! Изменения в клавишнике и "мышнике" :-) с учетом введения SetSelectPos()
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
    ! Оптимизирован механизм отображения сепаратора - сначала формируем в
      памяти, потом выводим в виртуальный буфер
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  11.12.2000 tran
    + прокрутка мышью не должна врапить меню
  20.09.2000 SVS
    + Функция GetItemPtr - получить указатель на нужный Item.
  29.08.2000 tran 1.09
    - BUG с не записью \0 в конец строки в GetUserData
  01.08.2000 SVS
    + В ShowMenu добавлен параметр, сообщающий - вызвали ли функцию
      самостоятельно или из другой функции ;-)
    - Bug в конструкторе, если передали NULL для Title
    ! ListBoxControl -> VMFlags
    + функция удаления N пунктов меню
    + функция обработки меню (по умолчанию)
    + функция посылки сообщений меню
    ! Изменен вызов конструктора для указания функции-обработчика и родителя!
  28.07.2000 SVS
    + Добавлены цветовые атрибуты (в переменных) и функции, связанные с
      атрибутами:
      SetColors();
      GetColors();
  23.07.2000 SVS
    + Куча рамарок в исходниках :-)
    ! AlwaysScrollBar изменен на ListBoxControl
    ! Тень рисуется только для меню, для ListBoxControl она ненужна
  18.07.2000 SVS
    ! изменен вызов конструктора (пареметр isAlwaysScrollBar) с учетом
      необходимости scrollbar в DI_COMBOBOX (и в будущем - DI_LISTBOX)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  06.07.2000 tran
    + mouse support for menu scrollbar
  29.06.2000 SVS
    ! Показывать ScrollBar в меню если включена опция ShowMenuScrollbar
  28.06.2000 tran
    + вертикальный скролбар в меню при необходимости
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

/* $ 18.07.2000 SVS
   ! изменен вызов конструктора (isListBoxControl) с учетом необходимости
     scrollbar в DI_LISTBOX & DI_COMBOBOX
*/
VMenu::VMenu(char *Title,       // заголовок меню
             struct MenuData *Data, // пункты меню
             int ItemCount,     // количество пунктов меню
             int MaxHeight,     // максимальная высота
             DWORD Flags,       // нужен ScrollBar?
             FARWINDOWPROC Proc,    // обработчик
             Dialog *ParentDialog)  // родитель для ListBox
{
  int I;
  VMenu::VMFlags=Flags;
/* SVS $ */

  VMenu::ParentDialog=ParentDialog;
  UpdateRequired=TRUE;
  WrapMode=TRUE;
  CallCount=0;
  TopPos=0;
  ShowAmpersand=FALSE;
  SaveScr=NULL;

  if(!Proc) // функция должна быть всегда!!!
    Proc=(FARWINDOWPROC)VMenu::DefMenuProc;
  VMenuProc=Proc;

  if (Title!=NULL)
    strcpy(VMenu::Title,Title);
  else
    *VMenu::Title=0;

  *BottomTitle=0;
  VMenu::Item=NULL;
  VMenu::ItemCount=0;
  DrawBackground=TRUE;


  for (I=0; I < ItemCount; I++)
  {
    struct MenuItem NewItem;
    memset(&NewItem,0,sizeof(NewItem));
    if ((unsigned int)Data[I].Name < MAX_MSG)
      strcpy(NewItem.Name,MSG((unsigned int)Data[I].Name));
    else
      strcpy(NewItem.Name,Data[I].Name);
    NewItem.Selected=Data[I].Selected;
    NewItem.Checked=Data[I].Checked;
    NewItem.Separator=Data[I].Separator;
    AddItem(&NewItem);
  }

  VMenu::MaxHeight=MaxHeight;
  DialogStyle=0;
  BoxType=DOUBLE_BOX;
  /* $ 01.08.2000 SVS
   - Bug в конструкторе, если передали NULL для Title
  */
  MaxLength=strlen(VMenu::Title)+2;
  /* SVS $ */
  for (SelectPos=0,I=0;I<ItemCount;I++)
  {
    int Length=strlen(Item[I].Name);
    if (Length>MaxLength)
      MaxLength=Length;
    if (Item[I].Selected)
      SelectPos=I;
  }
  /* $ 28.07.2000 SVS
     Установим цвет по умолчанию
  */
  SetColors(NULL);
  /* SVS $*/
  if (!(VMenu::VMFlags&VMENU_LISTBOX) && CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    if (PrevMacroMode!=MACRO_MAINMENU)
      CtrlObject->Macro.SetMode(MACRO_OTHER);
  }
}


VMenu::~VMenu()
{
  if (!(VMenu::VMFlags&VMENU_LISTBOX) && CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
  Hide();
  DeleteItems();
}


void VMenu::DeleteItems()
{
  /* $ 13.07.2000 SVS
     ни кто не вызывал запрос памяти через new :-)
  */
  if(Item)
    free(Item);
  /* SVS $ */
  Item=NULL;
  ItemCount=0;
}


void VMenu::Hide()
{
  while (CallCount>0)
    Sleep(10);
  CallCount++;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  if(!(VMenu::VMFlags&VMENU_LISTBOX) && SaveScr)
  {
    delete SaveScr;
    SaveScr=NULL;
    ScreenObject::Hide();
  }

  UpdateRequired=TRUE;
  CallCount--;
}


void VMenu::Show()
{
  while (CallCount>0)
    Sleep(10);
  CallCount++;
  if(VMenu::VMFlags&VMENU_LISTBOX)
    BoxType=SHORT_DOUBLE_BOX;

  int AutoCenter=FALSE,AutoHeight=FALSE;
  if (X1==-1)
  {
    X1=(ScrX-MaxLength-4)/2;
    AutoCenter=TRUE;
  }
  if (X1<2)
    X1=2;
  if (X2<=0)
    X2=X1+MaxLength+4;
  if (!AutoCenter && X2>ScrX-4+2*(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
  {
    X1+=ScrX-4-X2;
    X2=ScrX-4;
    if (X1<2)
      X1=2;
  }
  if (X2>ScrX-2)
    X2=ScrX-2;
  if (Y1==-1)
  {
    if (MaxHeight!=0 && MaxHeight<ItemCount)
      Y1=(ScrY-MaxHeight-2)/2;
    else
      if ((Y1=(ScrY-ItemCount-2)/2)<0)
        Y1=0;
    AutoHeight=TRUE;
  }
  if (Y2<=0)
    if (MaxHeight!=0 && MaxHeight<ItemCount)
      Y2=Y1+MaxHeight+1;
    else
      Y2=Y1+ItemCount+1;
  if (Y2>ScrY)
    Y2=ScrY;
  if (AutoHeight && Y1<3 && Y2>ScrY-3)
  {
    Y1=2;
    Y2=ScrY-2;
  }
  if (X2>X1 && Y2>Y1)
  {
    if(!(VMenu::VMFlags&VMENU_LISTBOX))
      ScreenObject::Show();
    else
    {
      UpdateRequired=TRUE;
      DisplayObject();
    }
  }
  CallCount--;
}

/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::DisplayObject()
{
  if (!UpdateRequired)
    return;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  UpdateRequired=0;
  ExitCode=-1;

  if (!(VMenu::VMFlags&VMENU_LISTBOX) && SaveScr==NULL)
  {
    if (DrawBackground && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
      SaveScr=new SaveScreen(X1-2,Y1-1,X2+4,Y2+2);
    else
      SaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
  }
  if (DrawBackground)
  {
    /* $ 23.07.2000 SVS
       Тень для ListBox ненужна
    */
    if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
    {
      Box(X1,Y1,X2,Y2,VMenu::Colors[1],BoxType);
      if(!(VMenu::VMFlags&(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR)))
      {
        MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
        MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
      }
    }
    else
    {
      SetScreen(X1-2,Y1-1,X2+2,Y2+1,' ',VMenu::Colors[0]);
      if(!(VMenu::VMFlags&(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR)))
      {
        MakeShadow(X1,Y2+2,X2+3,Y2+2);
        MakeShadow(X2+3,Y1,X2+4,Y2+2);
      }
      if (BoxType!=NO_BOX)
        Box(X1,Y1,X2,Y2,VMenu::Colors[1],BoxType);
    }
    /* SVS $*/
  }
  if(!(VMenu::VMFlags&VMENU_LISTBOX))
  {
    if (*Title)
    {
      GotoXY(X1+(X2-X1-1-strlen(Title))/2,Y1);
      SetColor(VMenu::Colors[2]);
      mprintf(" %s ",Title);
    }
    if (*BottomTitle)
    {
      GotoXY(X1+(X2-X1-1-strlen(BottomTitle))/2,Y2);
      SetColor(VMenu::Colors[2]);
      mprintf(" %s ",BottomTitle);
    }
  }
  SetCursorType(0,10);
  ShowMenu(TRUE);
}
/* SVS $ */


/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::ShowMenu(int IsParent)
{
  char TmpStr[1024];
  unsigned char BoxChar[2],BoxChar2[2];
  int Y,I;
  if (ItemCount==0 || X2<=X1 || Y2<=Y1)
    return;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  BoxChar2[1]=BoxChar[1]=0;

  if(!IsParent && (VMenu::VMFlags&VMENU_LISTBOX))
  {
    BoxType=SHORT_SINGLE_BOX;
    SetScreen(X1,Y1,X2,Y2,' ',VMenu::Colors[0]);
    if (BoxType!=NO_BOX)
      Box(X1,Y1,X2,Y2,VMenu::Colors[1],BoxType);
  }

  switch(BoxType)
  {
    case NO_BOX:
      *BoxChar=' ';
      break;
    case SINGLE_BOX:
    case SHORT_SINGLE_BOX:
      *BoxChar=0x0B3;
      break;
    case DOUBLE_BOX:
    case SHORT_DOUBLE_BOX:
      *BoxChar=0x0BA;
      break;
  }
  if (SelectPos<ItemCount)
    Item[SelectPos].Selected=1;
  if (SelectPos>TopPos+Y2-Y1-2)
    TopPos=SelectPos-(Y2-Y1-2);
  if (SelectPos<TopPos)
    TopPos=SelectPos;
  for (Y=Y1+1,I=TopPos;Y<Y2;Y++,I++)
  {
    GotoXY(X1,Y);
    if (I<ItemCount)
      if (Item[I].Separator)
      {
        int SepWidth=X2-X1+1;
        char *Ptr=TmpStr+1;
        MakeSeparator(SepWidth,TmpStr,
          BoxType==NO_BOX?0:(BoxType==SINGLE_BOX||BoxType==SHORT_SINGLE_BOX?2:1));

        if (I>0 && I<ItemCount-1 && SepWidth>3)
          for (int J=0;Ptr[J+3]!=0;J++)
          {
            if (Item[I-1].Name[J]==0)
              break;
            if (Item[I-1].Name[J]==0x0B3)
            {
              int Correction=0;
              if (!ShowAmpersand && memchr(Item[I-1].Name,'&',J)!=NULL)
                Correction=1;
              if (strlen(Item[I+1].Name)>=J && Item[I+1].Name[J]==0x0B3)
                Ptr[J-Correction+2]=0x0C5;
              else
                Ptr[J-Correction+2]=0x0C1;
            }
          }
        Text(X1,Y,VMenu::Colors[1],TmpStr);
      }
      else
      {
        SetColor(VMenu::Colors[1]);
        Text((char*)BoxChar);
        GotoXY(X2,Y);
        Text((char*)BoxChar);
        if (Item[I].Selected)
          SetColor(VMenu::Colors[6]);
        else
          SetColor(VMenu::Colors[3]);
        GotoXY(X1+1,Y);
        char Check=' ';
        if (Item[I].Checked)
          if (Item[I].Checked==1)
            Check=0x0FB;
          else
            Check=Item[I].Checked;
        sprintf(TmpStr,"%c %.*s",Check,X2-X1-3,Item[I].Name);
        int Col;

        if (Item[I].Selected)
            Col=VMenu::Colors[7];
        else
            Col=VMenu::Colors[4];
        if (ShowAmpersand)
          Text(TmpStr);
        else
          HiText(TmpStr,Col);
        mprintf("%*s",X2-WhereX(),"");
      }
    else
    {
      SetColor(VMenu::Colors[1]);
      Text((char*)BoxChar);
      GotoXY(X2,Y);
      Text((char*)BoxChar);
      GotoXY(X1+1,Y);
      SetColor(VMenu::Colors[3]);
      mprintf("%*s",X2-X1-1,"");
    }
  }
  /* $ 28.06.2000 tran
       показываем скролбар если пунктов в меню больше чем
       его высота
     $ 29.06.2000 SVS
       Показывать ScrollBar в меню если включена опция Opt.ShowMenuScrollbar
     $ 18.07.2000 SVS
       + всегда покажет scrollbar для DI_LISTBOX & DI_COMBOBOX и опционально
         для вертикального меню
  */
  if (((VMenu::VMFlags&(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR)) ||
       Opt.ShowMenuScrollbar) &&
      (Y2-Y1-1)<ItemCount)
  {
    SetColor(VMenu::Colors[8]);
    ScrollBar(X2,Y1+1,Y2-Y1-1,SelectPos,ItemCount);
  }
  /* 18.07.2000 SVS $ */
  /* SVS $ */
  /* tran $ */
}
/* 28.07.2000 SVS $ */

int VMenu::AddItem(struct MenuItem *NewItem)
{
  while (CallCount>0)
    Sleep(10);
  CallCount++;
  struct MenuItem *NewPtr;
  int Length;
  if (ItemCount>=TopPos && ItemCount<TopPos+Y2-Y1)
    UpdateRequired=1;
  if ((ItemCount & 255)==0)
  {
    if ((NewPtr=(struct MenuItem *)realloc(Item,sizeof(struct MenuItem)*(ItemCount+256+1)))==NULL)
      return(0);
    Item=NewPtr;
  }
  Item[ItemCount]=*NewItem;
  Length=strlen(Item[ItemCount].Name);
  char *TabPtr;
  while ((TabPtr=strchr(Item[ItemCount].Name,'\t'))!=NULL)
    *TabPtr=' ';
  if (Length>MaxLength)
    MaxLength=Length;
  if (Item[ItemCount].Selected)
    SelectPos=ItemCount;
  CallCount--;
  return(ItemCount++);
}
/* $ 01.08.2000 SVS
   функция удаления N пунктов меню
*/
int VMenu::DeleteItem(int ID,int Count)
{
  if(ID > 0)
  {
    if(ID >= ItemCount)
      return ItemCount;
    if(ID+Count >= ItemCount)
      Count=ItemCount-ID; //???
  }
  else // Если ID < 0, то подразумеваем, что счет ведется с конца списка
  {
    if(ItemCount+ID < 0)
      return ItemCount;
    else
    {
      ID+=ItemCount;
      if(ID+Count >= ItemCount)
        Count=ItemCount-ID; //???
    }
  }
  if(Count <= 0)
    return ItemCount;

  while (CallCount>0)
    Sleep(10);
  CallCount++;


  if ((ID >= TopPos && ID < TopPos+Y2-Y1) ||
      (ID+Count >= TopPos && ID+Count < TopPos+Y2-Y1) //???
     )
    UpdateRequired=1;

  memmove(Item+ID,Item+ID+Count,sizeof(struct MenuItem)*Count); //???

  if (Item[ID].Selected)
  {
    if(ID == ItemCount)
      SelectPos--;
  }

  ItemCount-=Count;

  CallCount--;
  return(ItemCount);
}
/* SVS $ */


int VMenu::ProcessKey(int Key)
{
  int I;

  if (Key==KEY_NONE || Key==KEY_IDLE)
    return(FALSE);

  UpdateRequired=1;
  if (ItemCount==0)
    if (Key!=KEY_F1 && Key!=KEY_F10 && Key!=KEY_ESC)
    {
      ExitCode=-1;
      return(FALSE);
    }

  while (CallCount>0)
    Sleep(10);
  CallCount++;

  switch(Key)
  {
    case KEY_F1:
      if(VMenu::ParentDialog)
        VMenu::ParentDialog->ProcessKey(Key);
      else
        ShowHelp();
      break;
    case KEY_ENTER:
      if(VMenu::ParentDialog)
        VMenu::ParentDialog->ProcessKey(Key);
      else
      {
        EndLoop=TRUE;
        ExitCode=SelectPos;
      }
      break;
    case KEY_ESC:
    case KEY_F10:
      if(VMenu::ParentDialog)
        VMenu::ParentDialog->ProcessKey(Key);
      else
      {
        EndLoop=TRUE;
        ExitCode=-1;
      }
      break;
    case KEY_HOME:
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      SelectPos=SetSelectPos(0,1);
      ShowMenu(TRUE);
      break;
    case KEY_END:
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      SelectPos=SetSelectPos(ItemCount-1,-1);
      ShowMenu(TRUE);
      break;
    case KEY_PGUP:
      if((I=SelectPos-(Y2-Y1-1)) < 0)
        I=0;
      SelectPos=SetSelectPos(I,1);
      ShowMenu(TRUE);
      break;
    case KEY_PGDN:
      if((I=SelectPos+(Y2-Y1-1)) >= ItemCount)
        I=ItemCount-1;
      SelectPos=SetSelectPos(I,-1);
      ShowMenu(TRUE);
      break;
    case KEY_LEFT:
    case KEY_UP:
      SelectPos=SetSelectPos(SelectPos-1,-1);
      ShowMenu(TRUE);
      break;
    case KEY_RIGHT:
    case KEY_DOWN:
      SelectPos=SetSelectPos(SelectPos+1,1);
      ShowMenu(TRUE);
      break;
    case KEY_TAB:
    case KEY_SHIFTTAB:
      if(VMenu::ParentDialog)
      {
        VMenu::ParentDialog->ProcessKey(Key);
        break;
      }
    default:
      for (I=0;I<ItemCount;I++)
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,FALSE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu(TRUE);
          if(!VMenu::ParentDialog)
          {
            ExitCode=I;
            EndLoop=TRUE;
          }
          break;
        }
      if (!EndLoop)
        for (I=0;I<ItemCount;I++)
          if (Dialog::IsKeyHighlighted(Item[I].Name,Key,TRUE))
            {
              Item[SelectPos].Selected=0;
              Item[I].Selected=1;
              SelectPos=I;
              ShowMenu(TRUE);
              if(!VMenu::ParentDialog)
              {
                ExitCode=I;
                EndLoop=TRUE;
              }
              break;
            }
      CallCount--;
      return(FALSE);
  }
  CallCount--;
  return(TRUE);
}

// переместить курсор с учетом Disabled & Separator
int VMenu::SetSelectPos(int Pos,int Direct)
{
  int OrigPos=Pos, Pass=0;

  do{
    if (Pos<0)
    {
      if (WrapMode)
        Pos=ItemCount-1;
      else
        Pos=0;
    }

    if (Pos>=ItemCount)
    {
      if (WrapMode)
        Pos=0;
      else
        Pos=ItemCount-1;
    }

    if(!Item[Pos].Separator && !Item[Pos].Disabled)
      break;

    Pos+=Direct;

    if(Pass)
      return SelectPos;

    if(OrigPos == Pos) // круг пройден - ничего не найдено :-(
      Pass++;
  } while (1);

  Item[SelectPos].Selected=0;
  Item[Pos].Selected=1;
  return Pos;
}


int VMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int MsPos,MsX,MsY;
  int XX2;

  UpdateRequired=1;
  if (ItemCount==0)
  {
    ExitCode=-1;
    return(FALSE);
  }

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  /* $ 06.07.2000 tran
     + mouse support for menu scrollbar
  */

  int SbY1=Y1+1, SbY2=Y2-1;

  XX2=X2;
  if (Opt.ShowMenuScrollbar && (Y2-Y1-1)<ItemCount)
    XX2--;  // уменьшает площадь, в которой меню следит за мышью само

  if (Opt.ShowMenuScrollbar && MsX==X2 && (Y2-Y1-1)<ItemCount &&
      (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) )
  {
    if (MsY==SbY1)
    {
      while (IsMouseButtonPressed())
      {
        /* $ 11.12.2000 tran
           прокрутка мышью не должна врапить меню
        */
        if (SelectPos!=0)
            ProcessKey(KEY_UP);
        ShowMenu(TRUE);
        /* tran $ */
      }
      return(TRUE);
    }
    if (MsY==SbY2)
    {
      while (IsMouseButtonPressed())
      {
        /* $ 11.12.2000 tran
           прокрутка мышью не должна врапить меню
        */
        if (SelectPos!=ItemCount-1)
            ProcessKey(KEY_DOWN);
        /* tran $ */
        ShowMenu(TRUE);
      }
      return(TRUE);
    }
    if (MsY>SbY1 && MsY<SbY2)
    {
      int SbHeight=Y2-Y1-2;
      int Delta;
      MsPos=(ItemCount-1)*(MsY-Y1)/(SbHeight);
      if(MsPos >= ItemCount)
      {
        MsPos=ItemCount-1;
        Delta=-1;
      }
      if(MsPos < 0)
      {
        MsPos=0;
        Delta=1;
      }
      if(!Item[MsPos].Separator && !Item[MsPos].Disabled)
        SelectPos=SetSelectPos(MsPos,Delta); //??
      ShowMenu(TRUE);
      return(TRUE);
    }
  }
  /* tran 06.07.2000 $ */

  // dwButtonState & 3 - Left & Right button
  if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MsX>X1 && MsX<X2)
  {
    if (MsY==Y1)
    {
      while (MsY==Y1 && SelectPos>0 && IsMouseButtonPressed())
        ProcessKey(KEY_UP);
      return(TRUE);
    }
    if (MsY==Y2)
    {
      while (MsY==Y2 && SelectPos<ItemCount-1 && IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
  }

  while (CallCount>0)
    Sleep(10);

  /* $ 06.07.2000 tran
     + mouse support for menu scrollbar
     */
  if (MsX>X1 && MsX<XX2 && MsY>Y1 && MsY<Y2)
  /* tran 06.07.2000 $ */
  {
    MsPos=TopPos+MsY-Y1-1;
    if (MsPos<ItemCount && !Item[MsPos].Separator && !Item[MsPos].Disabled)
    {
      if (MouseX!=PrevMouseX || MouseY!=PrevMouseY || MouseEvent->dwEventFlags==0)
      {
        Item[SelectPos].Selected=0;
        Item[MsPos].Selected=1;
        SelectPos=MsPos;
        ShowMenu(TRUE);
      }
      if (MouseEvent->dwEventFlags==0 && (MouseEvent->dwButtonState & 3)==0)
        ProcessKey(KEY_ENTER);
    }
    return(TRUE);
  }
  else
    if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MouseEvent->dwEventFlags==0)
    {
      ProcessKey(KEY_ESC);
      return(TRUE);
    }

  return(FALSE);
}


void VMenu::SetBottomTitle(char *BottomTitle)
{
  int Length;
  UpdateRequired=1;
  strncpy(VMenu::BottomTitle,BottomTitle,sizeof(VMenu::BottomTitle));
  Length=strlen(BottomTitle)+2;
  if (Length > MaxLength)
    MaxLength=Length;
}


void VMenu::SetBoxType(int BoxType)
{
  VMenu::BoxType=BoxType;
}


void VMenu::SetFlags(unsigned int Flags)
{
  UpdateRequired=1;
  DrawBackground=!(Flags & MENU_DISABLEDRAWBACKGROUND);
  WrapMode=(Flags & MENU_WRAPMODE);
  ShowAmpersand=(Flags & MENU_SHOWAMPERSAND);
}


int VMenu::GetUserData(void *Data,int Size,int Position)
{
  if (ItemCount==0)
    return(0);
  while (CallCount>0)
    Sleep(10);
  CallCount++;
  int DataPos=(Position==-1) ? SelectPos : Position;
  if (DataPos>=ItemCount)
    DataPos=ItemCount-1;
  int DataSize=Item[DataPos].UserDataSize;
//  SysLog("VMenu::GetUserData: Size=%i, DataSize=%i",Size,DataSize);
  /* $ 29.08.2000 tran
     - BUG with no \0 setting */
  if (DataSize>0 && Size>0 && Data!=NULL)
  {
    char *Ptr=(Item[DataPos].Flags&1)?Item[DataPos].PtrData:Item[DataPos].UserData;
    memmove(Data,Ptr,Min(Size,DataSize));
    // вот тут кое-кто забыл в конец строки 0 записать....
    ((char*)Data)[Min(Size,DataSize)]=0;
  }
  /* tran 29.08.2000 $ */

  CallCount--;
  return(DataSize);
}


int VMenu::GetSelection(int Position)
{
  if (ItemCount==0)
    return(0);
  while (CallCount>0)
    Sleep(10);
  int Pos=(Position==-1) ? SelectPos : Position;
  if (Pos>=ItemCount)
    Pos=ItemCount-1;
  if (Item[Pos].Separator)
    return(0);
  return(Item[Pos].Checked);
}


void VMenu::SetSelection(int Selection,int Position)
{
  while (CallCount>0)
    Sleep(10);
  if (ItemCount==0)
    return;
  int Pos=(Position==-1) ? SelectPos : Position;
  if (Pos>=ItemCount)
    Pos=ItemCount-1;
  Item[Pos].Checked=Selection;
}

/* $ 20.09.2000 SVS
  + Функция GetItemPtr - получить указатель на нужный Item.
*/
#ifndef _MSC_VER
#pragma warn -par
#endif
BOOL VMenu::GetItemPtr(struct MenuItem *Item1,int Position)
{
  if (ItemCount==0)
    return FALSE;
  while (CallCount>0)
    Sleep(10);
  int Pos=(Position==-1) ? SelectPos : Position;
  if (Pos>=ItemCount)
    Pos=ItemCount-1;
  Item1=&Item[Pos];
  return TRUE;
}
#ifndef _MSC_VER
#pragma warn +par
#endif
/* SVS $*/

int VMenu::GetSelectPos()
{
  return(SelectPos);
}

void VMenu::AssignHighlights(int Reverse)
{
  char Used[256];
  memset(Used,0,sizeof(Used));
  int I;
  for (I=Reverse ? ItemCount-1:0;I>=0 && I<ItemCount;I+=Reverse ? -1:1)
  {
    char *Name=Item[I].Name;
    char *ChPtr=strchr(Name,'&');
    if (ChPtr!=NULL && !ShowAmpersand)
    {
      Used[LocalUpper(ChPtr[1])]=TRUE;
      Used[LocalLower(ChPtr[1])]=TRUE;
    }
  }
  for (I=Reverse ? ItemCount-1:0;I>=0 && I<ItemCount;I+=Reverse ? -1:1)
  {
    char *Name=Item[I].Name;
    char *ChPtr=strchr(Name,'&');
    if (ChPtr==NULL || ShowAmpersand)
      for (int J=0;Name[J]!=0;J++)
        if (Name[J]=='&' || !Used[Name[J]] && LocalIsalphanum(Name[J]))
        {
          Used[Name[J]]=TRUE;
          Used[LocalUpper(Name[J])]=TRUE;
          Used[LocalLower(Name[J])]=TRUE;
          memmove(Name+J+1,Name+J,strlen(Name+J)+1);
          Name[J]='&';
          break;
        }
  }
  ShowAmpersand=FALSE;
}

/* $ 28.07.2000 SVS

*/
void VMenu::SetColors(short *Colors)
{
  int I;
  if(Colors)
    for(I=0; I < sizeof(VMenu::Colors)/sizeof(VMenu::Colors[0]); ++I)
      VMenu::Colors[I]=Colors[I];
  else
  {
    VMenu::Colors[0]=DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUTEXT;
    VMenu::Colors[1]=DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUBOX;
    VMenu::Colors[2]=COL_MENUTITLE;
    VMenu::Colors[3]=DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUTEXT;
    VMenu::Colors[4]=DialogStyle ? COL_DIALOGMENUHIGHLIGHT:COL_MENUHIGHLIGHT;
    VMenu::Colors[5]=DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUBOX;
    VMenu::Colors[6]=DialogStyle ? COL_DIALOGMENUSELECTEDTEXT:COL_MENUSELECTEDTEXT;
    VMenu::Colors[7]=DialogStyle ? COL_DIALOGMENUSELECTEDHIGHLIGHT:COL_MENUSELECTEDHIGHLIGHT;
    VMenu::Colors[8]=DialogStyle ? COL_DIALOGMENUSCROLLBAR: COL_MENUSCROLLBAR;
  }
}

void VMenu::GetColors(short *Colors)
{
  int I;
  for(I=0; I < sizeof(VMenu::Colors)/sizeof(VMenu::Colors[0]); ++I)
  {
    Colors[I]=VMenu::Colors[I];
  }
}

/* SVS $*/

#ifndef _MSC_VER
#pragma warn -par
#endif
// функция обработки меню (по умолчанию)
long WINAPI VMenu::DefMenuProc(HANDLE hVMenu,int Msg,int Param1,long Param2)
{
  return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif

#ifndef _MSC_VER
#pragma warn -par
#endif
// функция посылки сообщений меню
long WINAPI VMenu::SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,long Param2)
{
  if(hVMenu)
    return ((VMenu*)hVMenu)->VMenuProc(hVMenu,Msg,Param1,Param2);
  return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif
