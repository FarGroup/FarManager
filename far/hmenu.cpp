HMenu::HMenu(struct HMenuData *Item,int ItemCount)
{
  SubMenu=NULL;
  HMenu::Item=Item;
  HMenu::ItemCount=ItemCount;
}


void HMenu::DisplayObject()
{
  SetScreen(X1,Y1,X2,Y2,' ',COL_HMENUTEXT);
  SetCursorType(0,10);
  ShowMenu();
}


void HMenu::ShowMenu()
{
  char TmpStr[256];
  int I;
  GotoXY(X1+2,Y1);
  for (I=0;I<ItemCount;I++)
  {
    ItemX[I]=WhereX();
    if (Item[I].Selected)
      SetColor(COL_HMENUSELECTEDTEXT);
    else
      SetColor(COL_HMENUTEXT);
    sprintf(TmpStr,"  %s  ",Item[I].Name);
    HiText(TmpStr,Item[I].Selected ? COL_HMENUSELECTEDHIGHLIGHT:COL_HMENUHIGHLIGHT);
  }
  ItemX[ItemCount]=WhereX();
}


int HMenu::ProcessKey(int Key)
{
  int I;

  for (SelectPos=0,I=0;I<ItemCount;I++)
    if (Item[I].Selected)
    {
      SelectPos=I;
      break;
    }

  switch(Key)
  {
    case KEY_NONE:
    case KEY_IDLE:
      return(FALSE);
    case KEY_F1:
      ShowHelp();
      return(TRUE);
    case KEY_ENTER:
    case KEY_DOWN:
      if (Item[SelectPos].SubMenu)
      {
        ProcessSubMenu(Item[SelectPos].SubMenu,Item[SelectPos].SubMenuSize,
                       Item[SelectPos].SubMenuHelp,ItemX[SelectPos],
                       Y1+1,VExitCode);
        if (VExitCode!=-1)
        {
          EndLoop=TRUE;
          ExitCode=SelectPos;
        }
        return(TRUE);
      }
      return(FALSE);
    case KEY_ESC:
    case KEY_F10:
      EndLoop=TRUE;
      ExitCode=-1;
      return(FALSE);
    case KEY_HOME:
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      Item[SelectPos].Selected=0;
      Item[0].Selected=1;
      SelectPos=0;
      ShowMenu();
      return(TRUE);
    case KEY_END:
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      Item[SelectPos].Selected=0;
      Item[ItemCount-1].Selected=1;
      SelectPos=ItemCount-1;
      ShowMenu();
      return(TRUE);
    case KEY_LEFT:
      Item[SelectPos].Selected=0;
      if (--SelectPos<0)
        SelectPos=ItemCount-1;
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    case KEY_RIGHT:
      Item[SelectPos].Selected=0;
      if (++SelectPos==ItemCount)
        SelectPos=0;
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    default:
      for (I=0;I<ItemCount;I++)
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,FALSE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      for (I=0;I<ItemCount;I++)
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,TRUE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      return(FALSE);
  }
}


int HMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int I;
  int MsX,MsY;

  for (SelectPos=0,I=0;I<ItemCount;I++)
    if (Item[I].Selected)
    {
      SelectPos=I;
      break;
    }

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  if (MsY==Y1 && MsX>=X1 && MsX<=X2)
  {
    for (I=0;I<ItemCount;I++)
      if (MsX>=ItemX[I] && MsX<ItemX[I+1])
      {
        if (SubMenu!=NULL && SelectPos==I)
          return(FALSE);
        Item[SelectPos].Selected=0;
        Item[I].Selected=1;
        SelectPos=I;
        ShowMenu();
        ProcessKey(KEY_ENTER);
      }
  }
  else
    if ((MouseEvent->dwButtonState & 3)==0 && MouseEvent->dwEventFlags==0)
      ProcessKey(KEY_ESC);
  return(TRUE);
}


void HMenu::GetExitCode(int &ExitCode,int &VExitCode)
{
  ExitCode=HMenu::ExitCode;
  VExitCode=HMenu::VExitCode;
}


void HMenu::ProcessSubMenu(struct MenuData *Data,int DataCount,
                           char *SubMenuHelp,int X,int Y,int &Position)
{
  if (SubMenu!=NULL)
    delete SubMenu;
  Position=-1;
  SubMenu=new VMenu("",Data,DataCount);
  SubMenu->SetBoxType(SHORT_DOUBLE_BOX);
  SubMenu->SetFlags(MENU_WRAPMODE);
  SubMenu->SetHelp(SubMenuHelp);
  SubMenu->SetPosition(X,Y,0,0);
  SubMenu->Show();
  while (!SubMenu->Done())
  {
    INPUT_RECORD rec;
    int Key;
    Key=GetInputRecord(&rec);
    if (rec.EventType==MOUSE_EVENT)
    {
      if (rec.Event.MouseEvent.dwMousePosition.Y==Y1)
        if (ProcessMouse(&rec.Event.MouseEvent))
        {
          delete SubMenu;
          SubMenu=NULL;
          return;
        }
      SubMenu->ProcessMouse(&rec.Event.MouseEvent);
    }
    else
    {
      if (Key==KEY_LEFT || Key==KEY_RIGHT)
      {
        delete SubMenu;
        SubMenu=NULL;
        ProcessKey(Key);
        ProcessKey(KEY_ENTER);
        return;
      }
      SubMenu->ProcessKey(Key);
    }
  }

  Position=SubMenu->GetExitCode();
  delete SubMenu;
  SubMenu=NULL;
}

