/*
dialog.cpp

Класс диалога

*/

/* Revision: 1.06 25.07.2000 $ */

/*
Modify:
  25.07.2000 SVS
   + Новый параметр в конструкторе
  23.07.2000 SVS
   + Куча ремарок в исходниках :-)
   + Изменен вызов конструтора - добавка в виде функции обработки
   ! Строковые константы "SavedDialogHistory\\%s",
     "Locked%d" и "Line%d" сделаны поименованными.
   + Функция обработки диалога (по умолчанию) DefDlgProc() - забито место :-)
  19.07.2000 SVS
    ! "...В редакторе команд меню нажмите home shift+end del
      блок не удаляется..."
      DEL у итемов, имеющих DIF_EDITOR, работал без учета выделения...
  18.07.2000 SVS
    + Обработка элемента DI_COMBOBOX (пока все еще редактируемого)
    + Функция-обработчик выбора из списка - SelectFromComboBox
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  05.07.2000 SVS
    + добавлена проверка на флаг DIF_EDITEXPAND - расширение переменных
      среды в элементе диалога DI_EDIT
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

static char fmtLocked[]="Locked%d";
static char fmtLine[]  ="Line%d";
static char fmtSavedDialogHistory[]="SavedDialogHistory\\%s";

/* Public:
   Конструктор класса Dialog
*/
Dialog::Dialog(struct DialogItem *Item,int ItemCount,FARDIALOGPROC DlgProc,long Param)
{
  CreateObjects=FALSE;
  InitObjects=FALSE;
  DialogTooLong=FALSE;
  WarningStyle=0;

  if(!DlgProc)
    DlgProc=(FARDIALOGPROC)Dialog::DefDlgProc;
  Dialog::DlgProc=DlgProc;

  Dialog::Item=Item;
  Dialog::ItemCount=ItemCount;

  if (CtrlObject!=NULL)
  {
    // запомним пред. режим макро.
    PrevMacroMode=CtrlObject->Macro.GetMode();
    // макросить будет в диалогах :-)
    CtrlObject->Macro.SetMode(MACRO_DIALOG);
  }

  // запоминаем предыдущий заголовок консоли
  GetConsoleTitle(OldConsoleTitle,sizeof(OldConsoleTitle));
}


/* Public, Virtual:
   Деструктор класса Dialog
*/
Dialog::~Dialog()
{
  INPUT_RECORD rec;

  GetDialogObjectsData();
  DeleteDialogObjects();

  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);

  Hide();
  ScrBuf.Flush();

  PeekInputRecord(&rec);
  SetConsoleTitle(OldConsoleTitle);
}


/* Public, Virtual:
   Расчет значений координат окна диалога и вызов функции
   ScreenObject::Show() для вывода диалога на экран.
*/
void Dialog::Show()
{
  if (X1 == -1) // задано центрирование диалога по горизонтали?
  {             //   X2 при этом = ширине диалога.
    X1=(ScrX - X2 + 1)/2;

    if (X1 <= 0) // ширина диалога больше ширины экрана?
    {
      DialogTooLong=X2-1;
      X1=0;
      X2=ScrX;
    }
    else
      X2+=X1-1;
  }

  if (Y1 == -1) // задано центрирование диалога по вертикали?
  {             //   Y2 при этом = высоте диалога.
    Y1=(ScrY - Y2 + 1)/2;

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
  // вызывает DisplayObject()
  ScreenObject::Show();
}


/* Private, Virtual:
   Инициализация объектов и вывод диалога на экран.
*/
void Dialog::DisplayObject()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  Shadow();              // "наводим" тень

  if (!InitObjects)      // самодостаточный вариант, когда
  {                      //  элементы инициализируются при первом вызове.
    InitDialogObjects();
    InitObjects=TRUE;    // все объекты проинициализированы!
  }

  ShowDialog();          // "нарисуем" диалог.
}


/* Public:
   Инициализация элементов диалога.
*/
void Dialog::InitDialogObjects()
{
  int I, J, TitleSet;
  int Length,StartX;
  struct DialogItem *CurItem;

  // предварительный цикл по поводу кнопок и заголовка консоли
  for(I=0, TitleSet=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];

    // для кнопок не имеющи стиля "Показывает заголовок кнопки без скобок"
    //  добавим энти самые скобки
    if (CurItem->Type==DI_BUTTON &&
        (CurItem->Flags & DIF_NOBRACKETS)==0 &&
        *CurItem->Data != '[')
    {
      char BracketedTitle[200];
      sprintf(BracketedTitle,"[ %s ]",CurItem->Data);
      strcpy(CurItem->Data,BracketedTitle);
    }

    // по первому попавшемуся "тексту" установим заголовок консоли!
    if (!TitleSet &&             // при условии, что еще не устанавливали
         (CurItem->Type==DI_TEXT ||
          CurItem->Type==DI_DOUBLEBOX ||
          CurItem->Type==DI_SINGLEBOX))
      for (J=0;CurItem->Data[J]!=0;J++)
        if (LocalIsalpha(CurItem->Data[J]))
        {
          SetFarTitle(CurItem->Data+J);
          TitleSet=TRUE;
          break;
        }
  }

  // а теперь все сначала и по полной программе...
  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];

    // Последовательно объявленные элементы с флагом DIF_CENTERGROUP
    // и одинаковой вертикальной позицией будут отцентрированы в диалоге.
    // Их координаты X не важны. Удобно использовать для центрирования
    // групп кнопок.
    if ((CurItem->Flags & DIF_CENTERGROUP) &&
        (I==0 ||
        (Item[I-1].Flags & DIF_CENTERGROUP)==0 ||
        Item[I-1].Y1!=CurItem->Y1))
    {
      Length=0;

      for (J=I; J < ItemCount &&
                (Item[J].Flags & DIF_CENTERGROUP) &&
                Item[J].Y1==Item[I].Y1; J++)
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

      for (J=I; J < ItemCount &&
                (Item[J].Flags & DIF_CENTERGROUP) &&
                Item[J].Y1==Item[I].Y1; J++)
      {
        Item[J].X1=StartX;
        StartX+=HiStrlen(Item[J].Data);

        if (Item[J].Type==DI_BUTTON && *Item[J].Data!=' ')
          StartX+=2;
      }
    }

    // "редакторы" - разговор особый...
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
        // если DI_FIXEDIT, то курсор сразу ставится на замену...
        //   ай-ай - было недокументированно :-)
        DialogEdit->SetMaxLength(CurItem->X2-CurItem->X1+1);
        DialogEdit->SetOvertypeMode(TRUE);
      }
      else
        // "мини-редактор"
        // Последовательно определенные поля ввода (edit controls),
        // имеющие этот флаг группируются в редактор с возможностью
        // вставки и удаления строк
        if (!(CurItem->Flags & DIF_EDITOR))
        {
          DialogEdit->SetEditBeyondEnd(FALSE);
          DialogEdit->SetClearFlag(1);
        }
      /* $ 18.03.2000 SVS
         Если это ComBoBox и данные не установлены, то берем из списка
         при условии, что хоть один из пунктов имеет Selected != 0
      */
      if (CurItem->Type==DI_COMBOBOX && CurItem->Data[0] == 0)
      {
        struct FarListItem *ListItems=(struct FarListItem *)CurItem->Selected;

        for (J=0; ListItems[J].Text[0]; J++)
        {
          if(ListItems[J].Selected)
          {
            // берем только первый пункт для области редактирования
            strcpy(CurItem->Data, ListItems[J].Text);
            break;
          }
        }
      }
      /* SVS $ */
      DialogEdit->SetString(CurItem->Data);

      if (CurItem->Type==DI_FIXEDIT)
        DialogEdit->SetCurPos(0);

      DialogEdit->FastShow();
    }
  }
  // все объекты созданы
  CreateObjects=TRUE;
}


/* Private:
   Получение данных и удаление "редакторов"
*/
void Dialog::DeleteDialogObjects()
{
  int I;
  for (I=0; I < ItemCount; I++)
    if (IsEdit(Item[I].Type))
    {
      ((Edit *)(Item[I].ObjPtr))->GetString(Item[I].Data,sizeof(Item[I].Data));
      /*$ 05.07.2000 SVS $
          Проверка - этот элемент предполагает расширение переменных среды?
      */
      if(Item[I].Flags&DIF_EDITEXPAND)
         ExpandEnvironmentStr(Item[I].Data, Item[I].Data,sizeof(Item[I].Data));
      /* SVS */
      delete (Edit *)(Item[I].ObjPtr);
    }
}

/* Public:
   Сохраняет значение из полей редактирования.
   При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
  int I;
  for (I=0; I < ItemCount; I++)
    if (IsEdit(Item[I].Type))
    {
      ((Edit *)(Item[I].ObjPtr))->GetString(Item[I].Data,sizeof(Item[I].Data));
      /*$ 05.07.2000 SVS $
      Проверка - этот элемент предполагает расширение переменных среды?
      т.к. функция GetDialogObjectsData() может вызываться самостоятельно
      Но надо проверить!*/
      if(Item[I].Flags&DIF_EDITEXPAND)
         ExpandEnvironmentStr(Item[I].Data, Item[I].Data,sizeof(Item[I].Data));
      /* end */
      if (ExitCode>=0 && (Item[I].Flags & DIF_HISTORY) && Item[I].Selected && Opt.DialogsEditHistory)
        AddToEditHistory(Item[I].Data,(char *)Item[I].Selected);
    }
}

/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog()
{
  struct DialogItem *CurItem;
  int X,Y;
  int I;

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  SetScreen(X1,Y1,X2,Y2,' ',WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

  for (I=0; I < ItemCount; I++)
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
          int XB;

          sprintf(Title," %s ",CurItem->Data);
          XB=X1+CurItem->X1+(CurItem->X2-CurItem->X1+1-HiStrlen(Title))/2;

          if (CurItem->Flags & DIF_LEFTTEXT && X1+CurItem->X1+1 < XB)
            XB=X1+CurItem->X1+1;

          SetColor(WarningStyle ? COL_WARNDIALOGBOXTITLE:COL_DIALOGBOXTITLE);
          GotoXY(XB,Y1+CurItem->Y1);
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

      /* $ 18.07.2000 SVS
         + обработка элемента DI_COMBOBOX
      */
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
      case DI_COMBOBOX:
      {
        Edit *EditPtr=(Edit *)(CurItem->ObjPtr);
        if (CurItem->Focus)
        {
          SetCursorType(1,-1);
          EditPtr->Show();
        }
        else
          EditPtr->FastShow();

        if (CurItem->Selected &&
             ((CurItem->Flags & DIF_HISTORY) &&
              Opt.DialogsEditHistory
              || CurItem->Type == DI_COMBOBOX))
        {
          int EditX1,EditY1,EditX2,EditY2;

          EditPtr->GetPosition(EditX1,EditY1,EditX2,EditY2);
          //Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
          Text(EditX2+1,EditY1,COL_DIALOGTEXT,"");
        }
        // для DI_COMBOBOX здесь должна быть обработка флага DIF_DROPDOWNLIST
        // это когда Edit будет иметь не просто ReadOnly, а ...
        break;
        /* SVS $ */
      }

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
    } // end switch(...
  } // end for (I=...
}

/* Public, Virtual:
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
      /* $ 18.07.2000 SVS
         + обработка DI_COMBOBOX - выбор из списка!
      */
      else if(Type == DI_COMBOBOX && Item[FocusPos].Selected)
        SelectFromComboBox((Edit *)(Item[FocusPos].ObjPtr),(struct FarListItem *)Item[FocusPos].Selected);
      /* SVS $ */
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
              /* $ 19.07.2000 SVS
                 ! "...В редакторе команд меню нажмите home shift+end del
                   блок не удаляется..."
                   DEL у итемов, имеющих DIF_EDITOR, работал без учета
                   выделения...
              */
              if (FocusPos<ItemCount+1 && (Item[FocusPos+1].Flags & DIF_EDITOR))
              {
                char Str[1024];
                Edit *edt=(Edit *)Item[FocusPos].ObjPtr;
                int CurPos=edt->GetCurPos();
                int Length=edt->GetLength();
                int SelStart, SelEnd;

                edt->GetSelection(SelStart, SelEnd);
                edt->GetString(Str,sizeof(Str));
                int LengthStr=strlen(Str);
                if(SelStart > -1)
                {
                  memmove(&Str[SelStart],&Str[SelEnd],Length-SelEnd+1);
                  edt->SetString(Str);
                  edt->SetCurPos(SelStart);
                  ShowDialog();
                  return(TRUE);
                }
                else if (CurPos>=Length)
                {
                  Edit *edt_1=(Edit *)Item[FocusPos+1].ObjPtr;
                  edt_1->GetString(Str+LengthStr,sizeof(Str)-LengthStr);
                  edt_1->SetString(Str);
                  ProcessKey(KEY_CTRLY);
                  edt->SetCurPos(CurPos);
                  ShowDialog();
                  return(TRUE);
                }
              }
              break;
              /* SVS $*/
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


/* Public, Virtual:
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
            /* $ 18.07.2000 SVS
               + Проверка на тип элемента DI_COMBOBOX
            */
            if (MsX==EditX2+1 && MsY==EditY1 && Item[I].Selected &&
                ((Item[I].Flags & DIF_HISTORY) && Opt.DialogsEditHistory
                 || Item[I].Type == DI_COMBOBOX))
            /* SVS $ */
            {
              Item[FocusPos].Focus=0;
              Item[I].Focus=1;
              ProcessKey(KEY_CTRLDOWN);
              return(TRUE);
            }
          }
        }
        if (Item[I].Type==DI_BUTTON &&
            MsY==Y1+Item[I].Y1 &&
            MsX < X1+Item[I].X1+HiStrlen(Item[I].Data))
        {
          Item[FocusPos].Focus=0;
          Item[I].Focus=1;
          ShowDialog();
          while (IsMouseButtonPressed())
            ;
          if (MouseX <  X1 ||
              MouseX >  X1+Item[I].X1+HiStrlen(Item[I].Data)+4 ||
              MouseY != Y1+Item[I].Y1)
          {
            Item[FocusPos].Focus=1;
            Item[I].Focus=0;
            ShowDialog();
            return(TRUE);
          }
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }

        if ((Item[I].Type == DI_CHECKBOX ||
             Item[I].Type == DI_RADIOBUTTON) &&
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


/* Private:
   Изменяет фокус ввода (воздействие клавишами
     KEY_TAB, KEY_SHIFTTAB, KEY_UP, KEY_DOWN,
   а так же Alt-HotKey)
*/
int Dialog::ChangeFocus(int FocusPos,int Step,int SkipGroup)
{
  int Type;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент - LostFocus() - теряет фокус ввода.
  // Если функция диалога вернула FALSE, то это значит, что
  //   передавать фокус "не хотим" и вываливаемся отсель...
  // ...
  //
  while (1)
  {
    FocusPos+=Step;
    if (FocusPos>=ItemCount)
      FocusPos=0;
    if (FocusPos<0)
      FocusPos=ItemCount-1;

    Type=Item[FocusPos].Type;

    if (Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type))
      break;
    if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[FocusPos].Selected))
      break;
  }
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент GotFocus() - получил фокус ввода.
  // Игнорируем возвращаемое функцией диалога значение
  // ...
  //
  return(FocusPos);
}

/* Public, Static:
   преобразует данные об элементах диалога во внутреннее
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
    Item[I].ObjPtr=NULL;
  }
}

/* Private:
   Проверяет тип элемента диалога на предмет строки ввода
   (DI_EDIT, DI_FIXEDIT, DI_PSWEDIT) и в случае успеха возвращает TRUE
*/
/* $ 18.07.2000 SVS
   ! элемент DI_COMBOBOX относится к категории строковых редакторов...
*/
int Dialog::IsEdit(int Type)
{
  return(Type==DI_EDIT ||
         Type==DI_FIXEDIT ||
         Type==DI_PSWEDIT ||
         Type == DI_COMBOBOX);
}
/* SVS $ */


/* Private:
   Заполняем выпадающий список из истории
*/
void Dialog::SelectFromEditHistory(Edit *EditLine,char *HistoryName)
{
  char RegKey[80],KeyValue[80],Str[512];
  int I,Dest;
  int Checked;

  sprintf(RegKey,fmtSavedDialogHistory,HistoryName);
  {
    // создание пустого вертикального меню
    VMenu HistoryMenu("",NULL,0,8);

    struct MenuItem HistoryItem;
    int EditX1,EditY1,EditX2,EditY2;
    int ItemsCount;

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
    ItemsCount=0;
    for (I=0; I < 16; I++)
    {
      sprintf(KeyValue,fmtLine,I);
      GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
      if (*Str==0)
        continue;

      sprintf(KeyValue,fmtLocked,I);

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
        int Locked;
        for (I=0,Dest=0;I<16;I++)
        {
          sprintf(KeyValue,fmtLine,I);
          GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
          DeleteRegValue(RegKey,KeyValue);
          sprintf(KeyValue,fmtLocked,I);
          GetRegKey(RegKey,KeyValue,Locked,0);
          DeleteRegValue(RegKey,KeyValue);

          // залоченные пункты истории не удаляются
          if (Locked)
          {
            sprintf(KeyValue,fmtLine,Dest);
            SetRegKey(RegKey,KeyValue,Str);
            sprintf(KeyValue,fmtLocked,Dest);
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
        sprintf(KeyValue,fmtLocked,HistoryMenu.GetSelectPos());
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


/* Private:
   Работа с историей - добавление и reorder списка
*/
void Dialog::AddToEditHistory(char *AddStr,char *HistoryName)
{
  int LastLine=15,FirstLine=16, I, Locked;

  if (*AddStr==0)
    return;

  char RegKey[80],SrcKeyValue[80],DestKeyValue[80],Str[512];
  sprintf(RegKey,fmtSavedDialogHistory,HistoryName);

  for (I=0; I < 16; I++)
  {
    sprintf(SrcKeyValue,fmtLocked,I);
    GetRegKey(RegKey,SrcKeyValue,Locked,0);
    if (!Locked)
    {
      FirstLine=I;
      break;
    }
  }

  for (I=0; I < 16; I++)
  {
    sprintf(SrcKeyValue,fmtLine,I);
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
      sprintf(SrcKeyValue,fmtLocked,Src);
      GetRegKey(RegKey,SrcKeyValue,Locked,0);
      if (Locked)
        continue;
      for (int Dest=Src+1;Dest<=LastLine;Dest++)
      {
        sprintf(DestKeyValue,fmtLocked,Dest);
        GetRegKey(RegKey,DestKeyValue,Locked,0);
        if (!Locked)
        {
          sprintf(SrcKeyValue,fmtLine,Src);
          GetRegKey(RegKey,SrcKeyValue,Str,"",sizeof(Str));
          sprintf(DestKeyValue,fmtLine,Dest);
          SetRegKey(RegKey,DestKeyValue,Str);
          break;
        }
      }
    }
    char FirstLineKeyValue[20];
    sprintf(FirstLineKeyValue,fmtLine,FirstLine);
    SetRegKey(RegKey,FirstLineKeyValue,AddStr);
  }
}


/* Public, Static:

*/
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


/* Private:

*/
int Dialog::ProcessHighlighting(int Key,int FocusPos,int Translate)
{
  int I;
  for (I=0;I<ItemCount;I++)
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

/* Private:
   Заполняем выпадающий список для ComboBox
*/
/*
   $ 18.07.2000 SVS
   Функция-обработчик выбора из списка и установки...
*/
void Dialog::SelectFromComboBox(
         Edit *EditLine,                   // строка редактирования
         struct FarListItem *ListItems)    // список строк
{
  char Str[512];
  struct MenuItem ComboBoxItem;
  int EditX1,EditY1,EditX2,EditY2;
  int I,Dest;
  {
    // создание пустого вертикального меню
    //  с обязательным показом ScrollBar
    VMenu ComboBoxMenu("",NULL,0,8,TRUE);

    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
    if (EditX2>ScrX)
      EditX2=ScrX;
    ComboBoxItem.Checked=ComboBoxItem.Separator=0;
    ComboBoxMenu.SetFlags(MENU_SHOWAMPERSAND);
    ComboBoxMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
    ComboBoxMenu.SetBoxType(SHORT_SINGLE_BOX);

    // заполнение пунктов меню
    int ItemsCount=0;
    /* Последний пункт списка - ограничититель - в нем Tetx[0]
       должен быть равен '\0'
    */
    for (I=0;ListItems[I].Text[0];I++)
    {
      // int Checked;
      ComboBoxItem.Selected=ListItems[I].Selected;
      strcpy(ComboBoxItem.Name,ListItems[I].Text);
      strcpy(ComboBoxItem.UserData,ListItems[I].Text);
      ComboBoxItem.UserDataSize=strlen(ListItems[I].Text);
      ComboBoxMenu.AddItem(&ComboBoxItem);
      ItemsCount++;
    }
    if (ItemsCount==0)
      return;

    ComboBoxMenu.Show();
    while (!ComboBoxMenu.Done())
    {
      int Key=ComboBoxMenu.ReadInput();
      // здесь можно добавить что-то свое, например,
      //  обработку multiselect ComboBox
      ComboBoxMenu.ProcessInput();
    }

    int ExitCode=ComboBoxMenu.GetExitCode();
    if (ExitCode<0)
      return;
    ComboBoxMenu.GetUserData(Str,sizeof(Str),ExitCode);
  }
  EditLine->SetString(Str);
  EditLine->SetLeftPos(0);
  Redraw();
}
/* SVS $ */

/* $ 23.07.2000 SVS
   функция обработки диалога (по умолчанию)
*/
#pragma argused
long WINAPI Dialog::DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case WM_INITDIALOG:
      return TRUE;
  }
  return FALSE;
}
/* SVS $ */
