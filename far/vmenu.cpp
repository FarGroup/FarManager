/*
vmenu.cpp

Обычное вертикальное меню

*/

/* Revision: 1.01 28.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  28.06.2000 tran
    + вертикальный скролбар в меню при необходимости
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
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


VMenu::VMenu(char *Title,struct MenuData *Data,int ItemCount,int MaxHeight)
{
  int I;
  UpdateRequired=TRUE;
  WrapMode=TRUE;
  CallCount=0;
  TopPos=0;
  ShowAmpersand=FALSE;
  SaveScr=NULL;
  if (Title!=NULL)
    strcpy(VMenu::Title,Title);
  else
    *VMenu::Title=0;
  *BottomTitle=0;
  VMenu::Item=NULL;
  VMenu::ItemCount=0;
  DrawBackground=TRUE;

  for (I=0;I<ItemCount;I++)
  {
    struct MenuItem NewItem;
    if ((unsigned int)Data[I].Name < MAX_MSG)
      strcpy(NewItem.Name,MSG((unsigned int)Data[I].Name));
    else
      strcpy(NewItem.Name,Data[I].Name);
    NewItem.Selected=Data[I].Selected;
    NewItem.Checked=Data[I].Checked;
    NewItem.Separator=Data[I].Separator;
    *NewItem.UserData=NewItem.UserDataSize=0;
    AddItem(&NewItem);
  }

  VMenu::MaxHeight=MaxHeight;
  DialogStyle=0;
  BoxType=DOUBLE_BOX;
  MaxLength=strlen(Title)+2;
  for (SelectPos=0,I=0;I<ItemCount;I++)
  {
    int Length=strlen(Item[I].Name);
    if (Length>MaxLength)
      MaxLength=Length;
    if (Item[I].Selected)
      SelectPos=I;
  }
  if (CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    if (PrevMacroMode!=MACRO_MAINMENU)
      CtrlObject->Macro.SetMode(MACRO_OTHER);
  }
}


VMenu::~VMenu()
{
  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
  Hide();
  DeleteItems();
}


void VMenu::DeleteItems()
{
  delete Item;
  Item=NULL;
  ItemCount=0;
}


void VMenu::Hide()
{
  while (CallCount>0)
    Sleep(10);
  CallCount++;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  delete SaveScr;
  SaveScr=NULL;
  ScreenObject::Hide();
  UpdateRequired=TRUE;
  CallCount--;
}


void VMenu::Show()
{
  while (CallCount>0)
    Sleep(10);
  CallCount++;
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
    ScreenObject::Show();
  CallCount--;
}


void VMenu::DisplayObject()
{
  if (!UpdateRequired)
    return;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  UpdateRequired=0;
  ExitCode=-1;
  if (SaveScr==NULL)
    if (DrawBackground && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
      SaveScr=new SaveScreen(X1-2,Y1-1,X2+4,Y2+2);
    else
      SaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
  if (DrawBackground)
  {
    if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
    {
      Box(X1,Y1,X2,Y2,COL_MENUBOX,BoxType);
      MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
      MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
    }
    else
    {
      SetScreen(X1-2,Y1-1,X2+2,Y2+1,' ',DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUTEXT);
      MakeShadow(X1,Y2+2,X2+3,Y2+2);
      MakeShadow(X2+3,Y1,X2+4,Y2+2);
      if (BoxType!=NO_BOX)
        Box(X1,Y1,X2,Y2,COL_MENUBOX,BoxType);
    }
  }
  if (*Title)
  {
    GotoXY(X1+(X2-X1-1-strlen(Title))/2,Y1);
    SetColor(COL_MENUTITLE);
    mprintf(" %s ",Title);
  }
  if (*BottomTitle)
  {
    GotoXY(X1+(X2-X1-1-strlen(BottomTitle))/2,Y2);
    SetColor(COL_MENUTITLE);
    mprintf(" %s ",BottomTitle);
  }
  SetCursorType(0,10);
  ShowMenu();
}


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


void VMenu::ShowMenu()
{
  char TmpStr[256],*BoxChar;
  int Y,I;
  if (ItemCount==0 || X2<=X1 || Y2<=Y1)
    return;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  switch(BoxType)
  {
    case NO_BOX:
      BoxChar=" ";
      break;
    case SINGLE_BOX:
    case SHORT_SINGLE_BOX:
      BoxChar="│";
      break;
    case DOUBLE_BOX:
    case SHORT_DOUBLE_BOX:
      BoxChar="║";
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
        SetColor(COL_MENUBOX);
        memset(&TmpStr[1],'─',X2-X1-1);
        switch(BoxType)
        {
          case NO_BOX:
            Text(" ");
            GotoXY(X2,Y);
            Text(" ");
            break;
          case SINGLE_BOX:
          case SHORT_SINGLE_BOX:
            Text("├");
            GotoXY(X2,Y);
            Text("┤");
            break;
          case DOUBLE_BOX:
          case SHORT_DOUBLE_BOX:
            ShowSeparator(X2-X1+1);
            break;
        }
        memset(TmpStr,'─',X2-X1-1);
        TmpStr[X2-X1-1]=0;
        if (I>0 && I<ItemCount-1 && X2-X1-1>3)
          for (int J=0;TmpStr[J+3]!=0;J++)
          {
            if (Item[I-1].Name[J]==0)
              break;
            if (Item[I-1].Name[J]=='│')
            {
              int Correction=0;
              if (!ShowAmpersand && memchr(Item[I-1].Name,'&',J)!=NULL)
                Correction=1;
              if (strlen(Item[I+1].Name)>=J && Item[I+1].Name[J]=='│')
                TmpStr[J-Correction+2]='┼';
              else
                TmpStr[J-Correction+2]='┴';
            }
          }
        GotoXY(X1+1,Y);
        Text(TmpStr);
      }
      else
      {
        SetColor(DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUBOX);
        Text(BoxChar);
        GotoXY(X2,Y);
        Text(BoxChar);
        if (Item[I].Selected)
          SetColor(DialogStyle ? COL_DIALOGMENUSELECTEDTEXT:COL_MENUSELECTEDTEXT);
        else
          SetColor(DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUTEXT);
        GotoXY(X1+1,Y);
        char Check=' ';
        if (Item[I].Checked)
          if (Item[I].Checked==1)
            Check='√';
          else
            Check=Item[I].Checked;
        sprintf(TmpStr,"%c %.*s",Check,X2-X1-3,Item[I].Name);
        int Col;
        if (DialogStyle)
          if (Item[I].Selected)
            Col=COL_DIALOGMENUSELECTEDHIGHLIGHT;
          else
            Col=COL_DIALOGMENUHIGHLIGHT;
        else
          if (Item[I].Selected)
            Col=COL_MENUSELECTEDHIGHLIGHT;
          else
            Col=COL_MENUHIGHLIGHT;
        if (ShowAmpersand)
          Text(TmpStr);
        else
          HiText(TmpStr,Col);
        mprintf("%*s",X2-WhereX(),"");
      }
    else
    {
      SetColor(DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUBOX);
      Text(BoxChar);
      GotoXY(X2,Y);
      Text(BoxChar);
      GotoXY(X1+1,Y);
      SetColor(DialogStyle ? COL_DIALOGMENUTEXT:COL_MENUTEXT);
      mprintf("%*s",X2-X1-1,"");
    }
  }
  /* $ 28.06.2000 tran
     показываем скролбар если пунктов в меню больше чем
     его высота */
  if ((Y2-Y1-1)<ItemCount )
  {
    SetColor(COL_PANELSCROLLBAR);
    ScrollBar(X2,Y1+1,Y2-Y1-1,SelectPos,ItemCount);
  }
  /* tran $ */
}


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
      ShowHelp();
      break;
    case KEY_ENTER:
      EndLoop=TRUE;
      ExitCode=SelectPos;
      break;
    case KEY_ESC:
    case KEY_F10:
      EndLoop=TRUE;
      ExitCode=-1;
      break;
    case KEY_HOME:
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      Item[SelectPos].Selected=0;
      Item[0].Selected=1;
      SelectPos=0;
      ShowMenu();
      break;
    case KEY_END:
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      Item[SelectPos].Selected=0;
      Item[ItemCount-1].Selected=1;
      SelectPos=ItemCount-1;
      ShowMenu();
      break;
    case KEY_PGUP:
      Item[SelectPos].Selected=0;
      SelectPos-=Y2-Y1-1;
      if (SelectPos<0)
        SelectPos=0;
      if (Item[SelectPos].Separator && SelectPos>0)
        SelectPos--;
      Item[SelectPos].Selected=1;
      ShowMenu();
      break;
    case KEY_PGDN:
      Item[SelectPos].Selected=0;
      SelectPos+=Y2-Y1-1;
      if (SelectPos>=ItemCount)
        SelectPos=ItemCount-1;
      if (Item[SelectPos].Separator && SelectPos<ItemCount-1)
        SelectPos++;
      Item[SelectPos].Selected=1;
      ShowMenu();
      break;
    case KEY_LEFT:
    case KEY_UP:
      Item[SelectPos].Selected=0;
      do {
        if (--SelectPos<0)
          if (WrapMode)
            SelectPos=ItemCount-1;
          else
          {
            SelectPos=0;
            break;
          }
      } while (Item[SelectPos].Separator);
      Item[SelectPos].Selected=1;
      ShowMenu();
      break;
    case KEY_RIGHT:
    case KEY_DOWN:
      Item[SelectPos].Selected=0;
      do {
        if (++SelectPos==ItemCount)
          if (WrapMode)
            SelectPos=0;
          else
          {
            SelectPos=ItemCount-1;
            break;
          }
      } while (Item[SelectPos].Separator);
      Item[SelectPos].Selected=1;
      ShowMenu();
      break;
    default:
      for (I=0;I<ItemCount;I++)
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,FALSE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ExitCode=I;
          EndLoop=TRUE;
          break;
        }
      if (!EndLoop)
        for (I=0;I<ItemCount;I++)
          if (Dialog::IsKeyHighlighted(Item[I].Name,Key,TRUE))
            {
              Item[SelectPos].Selected=0;
              Item[I].Selected=1;
              SelectPos=I;
              ShowMenu();
              ExitCode=I;
              EndLoop=TRUE;
              break;
            }
      CallCount--;
      return(FALSE);
  }
  CallCount--;
  return(TRUE);
}


int VMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int MsPos,MsX,MsY;

  UpdateRequired=1;
  if (ItemCount==0)
  {
    ExitCode=-1;
    return(FALSE);
  }

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;

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

  if (MsX>X1 && MsX<X2 && MsY>Y1 && MsY<Y2)
  {
    MsPos=TopPos+MsY-Y1-1;
    if (MsPos<ItemCount && !Item[MsPos].Separator)
    {
      if (MouseX!=PrevMouseX || MouseY!=PrevMouseY || MouseEvent->dwEventFlags==0)
      {
        Item[SelectPos].Selected=0;
        Item[MsPos].Selected=1;
        SelectPos=MsPos;
        ShowMenu();
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
  if (DataSize>0 && Size>0 && Data!=NULL)
    memcpy(Data,Item[DataPos].UserData,Min(Size,DataSize));
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


int VMenu::GetSelectPos()
{
  return(SelectPos);
}


void VMenu::AssignHighlights(int Reverse)
{
  char Used[256];
  memset(Used,0,sizeof(Used));
  for (int I=Reverse ? ItemCount-1:0;I>=0 && I<ItemCount;I+=Reverse ? -1:1)
  {
    char *Name=Item[I].Name;
    char *ChPtr=strchr(Name,'&');
    if (ChPtr!=NULL && !ShowAmpersand)
    {
      Used[LocalUpper(ChPtr[1])]=TRUE;
      Used[LocalLower(ChPtr[1])]=TRUE;
    }
  }
  for (int I=Reverse ? ItemCount-1:0;I>=0 && I<ItemCount;I+=Reverse ? -1:1)
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

