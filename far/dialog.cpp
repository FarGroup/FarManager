/*
dialog.cpp

Класс диалога

*/

/* Revision: 1.12 01.08.2000 $ */

/*
Modify:
  01.08.2000 SVS
   ! History теперь ВСЕГДА имеет ScrollBar, т.к. этот элемент ближе
     к ComboBox`у, нежели к меню.
   - переменная класса lastKey удалена за ненадобностью :-)
   + Обычный ListBox
   - Небольшой глючек с AutoComplete
   ! В History должно заносится значение (для DIF_EXPAND...) перед
     расширением среды!
   + Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
     то подстанавливаем первое значение из History
   - Отключена возможность для DI_PSWEDIT иметь History...
     ...Что бы небыло повадно... и для повыщения защиты, т.с.
  31.07.2000 tran & SVS
   + перемещение диалога по экрану клавишами. Ctrl-F5 включает режим
     перемещения. Индикация перемещения - "Move" в левом верхнем углу
  28.07.2000 SVS
   ! Переметр Edit *EditLine в функции FindInEditForAC нафиг ненужен!
   - Небольшой баг с автозавершением:
       ...Есть в хистори на F7 - "templates". Жму F7, нажимаю shift+t=T и ...
       получаю маленькую t. В итоге большую добился только набиранием её
       после маленькой и стиранием оной...
   - Если плагин не выставил ни одного эелемента с фокусом,
     то придется самому об этом позаботиться, и выставить
     фокус на первом вразумительном элементе ;-)
   + AutoComplite: Для DI_COMBOBOX.
   ! SelectFromComboBox имеет дополнительный параметр с тем, чтобы
     позиционировать item в меню со списком в соответсвии со строкой ввода
   ! FindInEditHistory -> FindInEditForAC
     Поиск как в истории, так и в ComboBox`е (чтобы не пладить кода)
   + Функция IsFocused, определяющая - "Может ли элемент диалога
     иметь фокус ввода"
   + Функция ConvertItem - преобразования из внутреннего представления
     в FarDialogItem и обратно
   + Некоторое количество сообщений:
        DMSG_INITDIALOG, DMSG_ENTERIDLE, DMSG_HELP, DMSG_PAINT,
        DMSG_SETREDRAW, DMSG_DRAWITEM, DMSG_GETDLGITEM, DMSG_KILLFOCUS,
        DMSG_GOTFOCUS, DMSG_SETFOCUS, DMSG_GETTEXTLENGTH, DMSG_GETTEXT,
        DMSG_CTLCOLORDIALOG, DMSG_CTLCOLORDLGITEM, DMSG_CTLCOLORDLGLIST,
        DMSG_SETTEXTLENGTH, DMSG_SETTEXT, DMSG_CHANGEITEM, DMSG_HOTKEY,
        DMSG_CLOSE,
  26.07.2000 SVS
   + Ну наконец-то - долгожданный нередактируемый ComboBox
  26.07.2000 SVS
   + AutoComplite: Для DIF_HISTORY.
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
Dialog::Dialog(struct DialogItem *Item,int ItemCount,
               FARWINDOWPROC DlgProc,long InitParam)
{
  CreateObjects=FALSE;
  InitObjects=FALSE;
  DialogTooLong=FALSE;
  WarningStyle=0;

  if(!DlgProc) // функция должна быть всегда!!!
    DlgProc=(FARWINDOWPROC)Dialog::DefDlgProc;
  Dialog::DlgProc=DlgProc;

  Dialog::InitParam=InitParam;
  Dialog::Item=Item;
  Dialog::ItemCount=ItemCount;

  Dragged=0;

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
    /* $ 28.07.2000 SVS
       Укажем процедуре, что у нас все Ок!
    */
    DlgProc((HANDLE)this,DMSG_INITDIALOG,InitDialogObjects(),InitParam);
    // еще разок, т.к. данные могли быть изменены
    InitDialogObjects();
    /* SVS $ */
    InitObjects=TRUE;    // все объекты проинициализированы!
  }

  ShowDialog();          // "нарисуем" диалог.
}


/* Public:
   Инициализация элементов диалога.
*/
/* $ 28.07.2000 SVS
   Теперь InitDialogObjects возвращает ID элемента
   с фокусом ввода
*/
int Dialog::InitDialogObjects()
{
  int I, J, TitleSet;
  int Length,StartX;
  int FocusSet, Type;
  struct DialogItem *CurItem;

  // предварительный цикл по поводу кнопок и заголовка консоли
  for(I=0, FocusSet=0, TitleSet=0; I < ItemCount; I++)
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

     // предварительный поик фокуса
     if(IsFocused(CurItem->Type) && CurItem->Focus)
       FocusSet++;
  }

  // а теперь все сначала и по полной программе...
  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];
    Type=CurItem->Type;

    // Если фокус не установлен - найдем!
    if(!FocusSet && IsFocused(Type))
    {
      FocusSet++;
      CurItem->Focus=1;
    }
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
    /* $ 01.08.2000 SVS
       Обычный ListBox
    */
    if (Type==DI_LISTBOX)
    {
      if (!CreateObjects)
        CurItem->ObjPtr=new VMenu(NULL,NULL,0,CurItem->Y2-CurItem->Y1+1,
                               VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,NULL);

      VMenu *ListBox=(VMenu *)CurItem->ObjPtr;

      if(ListBox)
      {
        // удалим все итемы
        ListBox->DeleteItems();

        struct MenuItem ListItem;
        ListBox->SetFlags(MENU_SHOWAMPERSAND);
        ListBox->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                             X1+CurItem->X2,Y1+CurItem->Y2);
        ListBox->SetBoxType(SHORT_SINGLE_BOX);

        struct FarListItems *List=CurItem->ListItems;
        if(List && List->Items)
        {
          struct FarListItem *Items=List->Items;
          for (J=0; J < List->CountItems; J++)
          {
            ListItem.Separator=Items[J].Flags&LIF_SEPARATOR;
            ListItem.Selected=Items[J].Flags&LIF_SELECTED;
            ListItem.Checked=Items[J].Flags&LIF_CHECKED;
            strcpy(ListItem.Name,Items[J].Text);
            strcpy(ListItem.UserData,Items[J].Text);
            ListItem.UserDataSize=strlen(Items[J].Text);

            ListBox->AddItem(&ListItem);
          }
        }
      }
    }
    /* SVS $*/
    // "редакторы" - разговор особый...
    if (IsEdit(Type))
    {
      if (!CreateObjects)
        CurItem->ObjPtr=new Edit;

      Edit *DialogEdit=(Edit *)CurItem->ObjPtr;
      /* $ 26.07.2000 SVS
         Ну наконец-то - долгожданный нередактируемый ComboBox
      */
      if (CurItem->Flags & DIF_DROPDOWNLIST)
      {
         DialogEdit->DropDownBox=1;
      }
      /* SVS $ */
      DialogEdit->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                              X1+CurItem->X2,Y1+CurItem->Y2);
      DialogEdit->SetObjectColor(
         FarColorToReal(WarningStyle ? COL_WARNDIALOGEDIT:COL_DIALOGEDIT),
         FarColorToReal(COL_DIALOGEDITSELECTED));
      if (CurItem->Type==DI_PSWEDIT)
      {
        DialogEdit->SetPasswordMode(TRUE);
        /* $ 01.08.2000 SVS
          ...Что бы небыло повадно... и для повыщения защиты, т.с.
        */
        CurItem->Flags&=~DIF_HISTORY;
        /* SVS $ */
      }

      if (Type==DI_FIXEDIT)
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

      /* $ 01.08.2000 SVS
         Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
         то подстанавливаем первое значение из History
      */
      if((CurItem->Flags&(DIF_HISTORY|DIF_USELASTHISTORY)) == (DIF_HISTORY|DIF_USELASTHISTORY) &&
         !CurItem->Data[0])
      {
        char RegKey[80],KeyValue[80];
        sprintf(RegKey,fmtSavedDialogHistory,(char*)CurItem->History);
        GetRegKey(RegKey,"Line0",CurItem->Data,"",sizeof(CurItem->Data));
      }
      /* SVS $ */

      /* $ 18.03.2000 SVS
         Если это ComBoBox и данные не установлены, то берем из списка
         при условии, что хоть один из пунктов имеет Selected != 0
      */
      if (Type==DI_COMBOBOX && CurItem->Data[0] == 0 && CurItem->ListItems)
      {
        struct FarListItem *ListItems=
                   ((struct FarListItems *)CurItem->ListItems)->Items;
        int Length=((struct FarListItems *)CurItem->ListItems)->CountItems;

        for (J=0; J < Length; J++)
        {
          if(ListItems[J].Flags & LIF_SELECTED)
          {
            // берем только первый пункт для области редактирования
            strcpy(CurItem->Data, ListItems[J].Text);
            break;
          }
        }
      }
      /* SVS $ */
      DialogEdit->SetString(CurItem->Data);

      if (Type==DI_FIXEDIT)
        DialogEdit->SetCurPos(0);

      DialogEdit->FastShow();
    }
  }

  /* $ 28.07.2000 SVS
     - Если плагин не выставил ни одного эелемента с фокусом,
       то придется самому об этом позаботиться, и выставить
       фокус на первом вразумительном элементе ;-)
  */
  for (I=0;I<ItemCount;I++)
    if (Item[I].Focus)
      break;
  if(I == ItemCount)
    I=ChangeFocus(0,1,0);
  /* SVS $ */

  // все объекты созданы
  CreateObjects=TRUE;
  return I;
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
    else if(Item[I].Type == DI_LISTBOX && Item[I].ObjPtr)
      delete (VMenu *)(Item[I].ObjPtr);
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
      if (ExitCode>=0 && (Item[I].Flags & DIF_HISTORY) && Item[I].History && Opt.DialogsEditHistory)
        AddToEditHistory(Item[I].Data,Item[I].History);
      /* $ 01.08.2000 SVS
         ! В History должно заносится значение (для DIF_EXPAND...) перед
          расширением среды!
      */
      /*$ 05.07.2000 SVS $
      Проверка - этот элемент предполагает расширение переменных среды?
      т.к. функция GetDialogObjectsData() может вызываться самостоятельно
      Но надо проверить!*/
      if(Item[I].Flags&DIF_EDITEXPAND)
         ExpandEnvironmentStr(Item[I].Data, Item[I].Data,sizeof(Item[I].Data));
      /* SVS $ */
      /* 01.08.2000 SVS $ */
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
  unsigned long Attr;

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  /* $ 28.07.2000 SVS
     Перед прорисовкой диалога посылаем сообщение в обработчик
  */
  DlgProc((HANDLE)this,DMSG_PAINT,0,0);
  /* SVS $ */

  /* $ 28.07.2000 SVS
     перед прорисовкой подложки окна диалога...
  */
  Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDIALOG,0,WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
  SetScreen(X1,Y1,X2,Y2,' ',Attr);
  /* SVS $ */

  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];

    /* $ 28.07.2000 SVS
       Перед прорисовкой каждого элемента посылаем сообщение
       посредством функции SendDlgMessage - в ней делается все!
    */
    Dialog::SendDlgMessage((HANDLE)this,DMSG_DRAWITEM,I,0);
    /* SVS $ */
    /* $ 28.07.2000 SVS
       перед прорисовкой каждого элемента диалога выясним атритубы отрисовки
    */
    switch(CurItem->Type)
    {
      case DI_SINGLEBOX:
      case DI_DOUBLEBOX:
        Attr=MAKELONG(
          MAKEWORD(FarColorToReal(WarningStyle ? COL_WARNDIALOGBOXTITLE:COL_DIALOGBOXTITLE), // Title LOBYTE
                 FarColorToReal(WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)),// HiText HIBYTE
          MAKEWORD(FarColorToReal(WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX), // Box LOBYTE
                 0)                                               // HIBYTE
        );
        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,Attr);

        Box(X1+CurItem->X1,Y1+CurItem->Y1,X1+CurItem->X2,Y1+CurItem->Y2,
            LOBYTE(HIWORD(Attr)),
            (CurItem->Type==DI_SINGLEBOX) ? SINGLE_BOX:DOUBLE_BOX);

        if (*CurItem->Data)
        {
          char Title[200];
          int XB;

          sprintf(Title," %s ",CurItem->Data);
          XB=X1+CurItem->X1+(CurItem->X2-CurItem->X1+1-HiStrlen(Title))/2;

          if (CurItem->Flags & DIF_LEFTTEXT && X1+CurItem->X1+1 < XB)
            XB=X1+CurItem->X1+1;

          SetColor(Attr&0xFF);
          GotoXY(XB,Y1+CurItem->Y1);
          HiText(Title,HIBYTE(LOWORD(Attr)));
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
          Attr=CurItem->Flags & DIF_COLORMASK;
        else
          if (CurItem->Flags & DIF_BOXCOLOR)
            Attr=WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX;
          else
            Attr=WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT;

        Attr=MAKELONG(
           MAKEWORD(FarColorToReal(Attr),
                   FarColorToReal(WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)), // HIBYTE HiText
             0);
        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,Attr);
        SetColor(Attr&0xFF);

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
          HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));

        break;

      case DI_VTEXT:
        if (CurItem->Flags & DIF_BOXCOLOR)
          Attr=WarningStyle ? COL_WARNDIALOGBOX:COL_DIALOGBOX;
        else
          if (CurItem->Flags & DIF_SETCOLOR)
            Attr=(CurItem->Flags & DIF_COLORMASK);
          else
            Attr=(WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,FarColorToReal(Attr));
        SetColor(Attr&0xFF);
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

        Attr=EditPtr->GetObjectColor();
        Attr=MAKEWORD(FarColorToReal(Attr&0xFF),FarColorToReal(HIWORD(Attr)));
        Attr=MAKELONG(Attr, // EditLine (Lo=Color, Hi=Selected)
           MAKEWORD(FarColorToReal(EditPtr->GetObjectColorUnChanged()), // EditLine - UnChanched Color
           FarColorToReal(COL_DIALOGTEXT) // HistoryLetter
           ));
        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,Attr);

        EditPtr->SetObjectColor(Attr&0xFF,HIBYTE(LOWORD(Attr)),LOBYTE(HIWORD(Attr)));

        if (CurItem->Focus)
        {
          SetCursorType(1,-1);
          EditPtr->Show();
        }
        else
          EditPtr->FastShow();

        if (CurItem->History &&
             ((CurItem->Flags & DIF_HISTORY) &&
              Opt.DialogsEditHistory
              || CurItem->Type == DI_COMBOBOX))
        {
          int EditX1,EditY1,EditX2,EditY2;

          EditPtr->GetPosition(EditX1,EditY1,EditX2,EditY2);
          //Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
          Text(EditX2+1,EditY1,HIBYTE(HIWORD(Attr)),"");
        }
        break;
        /* SVS $ */
      }

      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      case DI_LISTBOX:
      {
        VMenu *ListBox=(VMenu *)(CurItem->ObjPtr);
        if(ListBox)
        {
          if (CurItem->Focus)
            ListBox->Show();
          else
            ListBox->FastShow();
        }
        break;
      }
      /* 01.08.2000 SVS $ */

      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
        if (CurItem->Flags & DIF_SETCOLOR)
          Attr=(CurItem->Flags & DIF_COLORMASK);
        else
          Attr=(WarningStyle ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

        Attr=MAKEWORD(FarColorToReal(Attr),
             FarColorToReal(WarningStyle ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)); // HiText
        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,Attr);

        SetColor(Attr&0xFF);

        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);

        if (CurItem->Type==DI_CHECKBOX)
          mprintf("[%c] ",CurItem->Selected ? 'x':' ');
        else
          if (CurItem->Flags & DIF_MOVESELECT)
            mprintf(" %c ",CurItem->Selected ? '\07':' ');
          else
            mprintf("(%c) ",CurItem->Selected ? '\07':' ');

        HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));

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
          Attr=MAKEWORD(
             FarColorToReal(WarningStyle ? COL_WARNDIALOGSELECTEDBUTTON:COL_DIALOGSELECTEDBUTTON), // TEXT
             FarColorToReal(WarningStyle ? COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON:COL_DIALOGHIGHLIGHTSELECTEDBUTTON)); // HiText
        }
        else
        {
          Attr=MAKEWORD(
             FarColorToReal(WarningStyle ? COL_WARNDIALOGBUTTON:COL_DIALOGBUTTON), // TEXT
             FarColorToReal(WarningStyle ? COL_WARNDIALOGHIGHLIGHTBUTTON:COL_DIALOGHIGHLIGHTBUTTON)); // HiText
        }
        Attr=DlgProc((HANDLE)this,DMSG_CTLCOLORDLGITEM,I,Attr);
        SetColor(Attr&0xFF);
        HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));
        break;

    } // end switch(...
    /* 28.07.2000 SVS $ */
  } // end for (I=...

  /* $ 31.07.2000 SVS
     Включим индикатор перемещения...
  */
  if ( Dragged ) // если диалог таскается
  {
    Text(0,0,0xCE,"Move");
  }
  /* SVS $ */
}

/* Public, Virtual:
   Обработка данных от клавиатуры.
   Перекрывает BaseInput::ProcessKey.
*/
int Dialog::ProcessKey(int Key)
{
  int FocusPos=0,I,J;
  char Str[1024];
  char *PtrStr;
  Edit *CurEditLine;

  /* $ 31.07.2000 tran
     + перемещение диалога по экрану */
  if ( Dragged ) // если диалог таскается
  {
    int rr=1;
    switch (Key)
    {
        case KEY_CTRLLEFT:
            rr=10;
        case KEY_LEFT:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X1>0 )
                {
                    X1--;
                    X2--;
                    AdjustEditPos(-1,0);
                }
            Show();
            break;
        case KEY_CTRLRIGHT:
            rr=10;
        case KEY_RIGHT:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X2<ScrX )
                {
                    X1++;
                    X2++;
                    AdjustEditPos(1,0);
                }
            Show();
            break;
        case KEY_PGUP:
        case KEY_CTRLUP:
            rr=5;
        case KEY_UP:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y1>0 )
                {
                    Y1--;
                    Y2--;
                    AdjustEditPos(0,-1);
                }
            Show();
            break;
        case KEY_PGDN:
        case KEY_CTRLDOWN:
            rr=5;
        case KEY_DOWN:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y2<ScrY )
                {
                    Y1++;
                    Y2++;
                    AdjustEditPos(0,1);
                }
            Show();
            break;
        case KEY_ENTER:
        case KEY_CTRLF5:
            Dragged=0;
            Show();
            PutText(0,0,3,0,LV);
            break;
        case KEY_ESC:
            Hide();
            AdjustEditPos(OldX1-X1,OldY1-Y1);
            X1=OldX1;
            X2=OldX2;
            Y1=OldY1;
            Y2=OldY2;
            Show();
            PutText(0,0,3,0,LV);
            Dragged=0;
            break;
    }
    return (TRUE);
  }

  if (Key == KEY_CTRLF5)
  {
    // включаем флаг и запоминаем координаты
    Dragged=1;
    OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
    GetText(0,0,3,0,LV);
    Show();
    return (TRUE);
  }
  /* tran 31.07.2000 $ */

  if (Key==KEY_NONE || Key==KEY_IDLE)
  {
    /* $ 28.07.2000 SVS
       Передадим этот факт в обработчик :-)
    */
    DlgProc((HANDLE)this,DMSG_ENTERIDLE,0,0);
    /* SVS $ */
    return(FALSE);
  }

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
      /* $ 28.07.2000 SVS
         Перед выводом диалога посылаем сообщение в обработчик
         и если вернули что надо, то выводим подсказку
      */
      PtrStr=(char*)DlgProc((HANDLE)this,DMSG_HELP,FocusPos,(long)&HelpTopic[0]);
      if(PtrStr && *PtrStr)
      {
        SetHelp(PtrStr);
        ShowHelp();
      }
      /* SVS $ */
      return(TRUE);

    case KEY_TAB:
    case KEY_SHIFTTAB:
// Здесь с фокусом ОООЧЕНЬ ТУМАННО!!!
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
      ChangeFocus2(FocusPos,I);
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
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,I-1,0);
          Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,I,0);
          /* SVS $ */
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
        Item[FocusPos].Selected =! Item[FocusPos].Selected;
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
           посредством функции SendDlgMessage - в ней делается все!
        */
        Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
        /* SVS $ */
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
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          J=Item[I].Selected;
          Item[I].Selected=0;
          if(J)
            Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,I,0);
          ++I;
          /* SVS $ */
        } while (I<ItemCount && Item[I].Type==DI_RADIOBUTTON && (Item[I].Flags & DIF_GROUP)==0);
        Item[FocusPos].Selected=1;
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
          посредством функции SendDlgMessage - в ней делается все!
        */
        Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
        /* SVS $ */
        ShowDialog();
        return(TRUE);
      }
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
          посредством функции SendDlgMessage - в ней делается все!
        */
        Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
        /* SVS $ */
        return(TRUE);
      }
      return(TRUE);

    case KEY_HOME:
      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }

      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */

      for (I=0;I<ItemCount;I++)
        if (IsFocused(Item[I].Type))
        {
          ChangeFocus2(FocusPos,I);
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
          Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,I,0);
          /* SVS $ */
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
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */
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
            ChangeFocus2(FocusPos,MinPos);
            if (Item[MinPos].Flags & DIF_MOVESELECT)
              ProcessKey(KEY_SPACE);
            else
              ShowDialog();
            return(TRUE);
          }
      }

    case KEY_UP:
    case KEY_DOWN:
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */
      {
        int PrevPos=0;
        if (Item[FocusPos].Flags & DIF_EDITOR)
          PrevPos=((Edit *)(Item[FocusPos].ObjPtr))->GetCurPos();
        I=ChangeFocus(FocusPos,(Key==KEY_LEFT || Key==KEY_UP) ? -1:1,FALSE);
        Item[FocusPos].Focus=0;
        Item[I].Focus=1;
        ChangeFocus2(FocusPos,I);
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
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */
      else if (!(Item[FocusPos].Flags & DIF_EDITOR))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            ChangeFocus2(FocusPos,I);
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
      CurEditLine=((Edit *)(Item[FocusPos].ObjPtr));
      if (IsEdit(Type) &&
           (Item[FocusPos].Flags & DIF_HISTORY) &&
           Opt.DialogsEditHistory &&
           Item[FocusPos].History)
      /* $ 26.07.2000 SVS
         Передаем то, что в строке ввода в функцию выбора из истории
         для выделения нужного пункта в истории.
      */
      {
        CurEditLine->GetString(Str,sizeof(Str));
        SelectFromEditHistory(CurEditLine,Item[FocusPos].History,Str);
      }
      /* SVS $ */
      /* $ 18.07.2000 SVS
         + обработка DI_COMBOBOX - выбор из списка!
      */
      else if(Type == DI_COMBOBOX && Item[FocusPos].ListItems)
      {
        CurEditLine->GetString(Str,sizeof(Str));
        SelectFromComboBox(CurEditLine,
                      Item[FocusPos].ListItems,Str);
      }
      /* SVS $ */
      return(TRUE);

    default:
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */
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
                    ((Edit *)(Item[I].ObjPtr))->GetString(Str,sizeof(Str));
                    ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
                  }
                  ((Edit *)(Item[I].ObjPtr))->SetString("");
                }
                else
                  break;
              /* $ 28.07.2000 SVS
                При изменении состояния каждого элемента посылаем сообщение
                посредством функции SendDlgMessage - в ней делается все!
              */
              Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
              /* SVS $ */
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
                  /* $ 28.07.2000 SVS
                    При изменении состояния каждого элемента посылаем сообщение
                    посредством функции SendDlgMessage - в ней делается все!
                  */
                  Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
                  /* SVS $ */
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
        {
          /* $ 26.07.2000 SVS
             AutoComplite: Если установлен DIF_HISTORY
                 и разрешено автозавершение!.
          */
          if(Opt.AutoComplete && Key < 256 && Key != KEY_BS && Key != KEY_DEL &&
             ((Item[FocusPos].Flags & DIF_HISTORY) || Type == DI_COMBOBOX)
            )
          {
            Edit *cb=((Edit *)(Item[FocusPos].ObjPtr));
            int SelStart, SelEnd, IX;

            /* $ 01.08.2000 SVS
               Небольшой глючек с AutoComplete
            */
            int CurPos=cb->GetCurPos();
            /* SVS $*/
            //text to search for
            cb->GetString(Str,sizeof(Str));
            cb->GetSelection(SelStart,SelEnd);
            if(SelStart <= 0)
              SelStart=sizeof(Str);
            else
              SelStart++;

            SelEnd=strlen(Str);
            //find the string in the list
            if (FindInEditForAC(Type == DI_COMBOBOX,(void *)Item[FocusPos].Selected,Str))
            {
              cb->SetString(Str);
              cb->Select(SelEnd,sizeof(Str)); //select the appropriate text
              /* $ 01.08.2000 SVS
                 Небольшой глючек с AutoComplete
              */
              cb->SetCurPos(CurPos); // SelEnd
              /* SVS $*/
              /* $ 28.07.2000 SVS
                При изменении состояния каждого элемента посылаем сообщение
                посредством функции SendDlgMessage - в ней делается все!
              */
              Dialog::SendDlgMessage((HANDLE)this,DMSG_CHANGEITEM,FocusPos,0);
              /* SVS $ */
              Redraw();
            }
          }
          /* SVS $ */
          return(TRUE);
        }
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
  int Type;

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
    {
      Type=Item[I].Type;
      if (MsX>=X1+Item[I].X1)
      {
        /* $ 01.08.2000 SVS
           Обычный ListBox
        */
        if(Type == DI_LISTBOX    &&
            MsY >= Y1+Item[I].Y1 &&
            MsY <= Y1+Item[I].Y2 &&
            MsX <= X1+Item[I].X2)
        {
          if(FocusPos != I)
            ChangeFocus2(FocusPos,I);
          ShowDialog();
          ((VMenu *)(Item[I].ObjPtr))->ProcessMouse(MouseEvent);
          return(TRUE);
        }
        /* SVS $ */

        if (IsEdit(Type))
        {
          Edit *EditLine=(Edit *)(Item[I].ObjPtr);
          if (EditLine->ProcessMouse(MouseEvent))
          {
            EditLine->SetClearFlag(0);
            ChangeFocus2(FocusPos,I);
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
            if (MsX==EditX2+1 && MsY==EditY1 && Item[I].History &&
                ((Item[I].Flags & DIF_HISTORY) && Opt.DialogsEditHistory
                 || Type == DI_COMBOBOX))
            /* SVS $ */
            {
              ChangeFocus2(FocusPos,I);
              ProcessKey(KEY_CTRLDOWN);
              return(TRUE);
            }
          }
        }
        if (Type==DI_BUTTON &&
            MsY==Y1+Item[I].Y1 &&
            MsX < X1+Item[I].X1+HiStrlen(Item[I].Data))
        {
          ChangeFocus2(FocusPos,I);
          ShowDialog();
          while (IsMouseButtonPressed())
            ;
          if (MouseX <  X1 ||
              MouseX >  X1+Item[I].X1+HiStrlen(Item[I].Data)+4 ||
              MouseY != Y1+Item[I].Y1)
          {
            ChangeFocus2(FocusPos,I);
            ShowDialog();
            return(TRUE);
          }
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }

        if ((Type == DI_CHECKBOX ||
             Type == DI_RADIOBUTTON) &&
            MsY==Y1+Item[I].Y1 &&
            MsX < (X1+Item[I].X1+HiStrlen(Item[I].Data)+4-((Item[I].Flags & DIF_MOVESELECT)!=0)))
        {
          ChangeFocus2(FocusPos,I);
          ProcessKey(KEY_SPACE);
          return(TRUE);
        }
      }
    // ДЛЯ MOUSE-Перемещалки:
    //   Сюда попадаем в том случае, если мышь не попала на активные элементы
    //
    }
  }
  return(FALSE);
}


/* Private:
   Изменяет фокус ввода (воздействие клавишами
     KEY_TAB, KEY_SHIFTTAB, KEY_UP, KEY_DOWN,
   а так же Alt-HotKey)
*/
/* $ 28.07.2000 SVS
   Довесок для сообщений DMSG_KILLFOCUS & DMSG_SETFOCUS
*/
int Dialog::ChangeFocus(int FocusPos,int Step,int SkipGroup)
{
  int Type,OrigFocusPos=FocusPos;
  int FucusPosNeed=-1;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент - LostFocus() - теряет фокус ввода.
  if(InitObjects)
    FucusPosNeed=DlgProc((HANDLE)this,DMSG_KILLFOCUS,FocusPos,0);
  if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
    FocusPos=FucusPosNeed;
  else
  {
    while (1)
    {
      FocusPos+=Step;
      if (FocusPos>=ItemCount)
        FocusPos=0;
      if (FocusPos<0)
        FocusPos=ItemCount-1;

      Type=Item[FocusPos].Type;

      if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type))
        break;
      if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[FocusPos].Selected))
        break;

      // убираем зацикливание с последующим подвисанием :-)
      if(OrigFocusPos == FocusPos)
        break;
    }
  }

  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент GotFocus() - получил фокус ввода.
  // Игнорируем возвращаемое функцией диалога значение
  if(InitObjects)
    DlgProc((HANDLE)this,DMSG_GOTFOCUS,FocusPos,0);
  return(FocusPos);
}

/* $ 28.07.2000 SVS
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
*/
int Dialog::ChangeFocus2(int KillFocusPos,int SetFocusPos)
{
  int FucusPosNeed=-1;
  if(InitObjects)
    FucusPosNeed=DlgProc((HANDLE)this,DMSG_KILLFOCUS,KillFocusPos,0);
  if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
    SetFocusPos=FucusPosNeed;

  Item[KillFocusPos].Focus=0;
  Item[SetFocusPos].Focus=1;

  if(InitObjects)
    DlgProc((HANDLE)this,DMSG_GOTFOCUS,SetFocusPos,0);
  return(SetFocusPos);
}
/* SVS $ */
/* $ 28.07.2000 SVS
   Public, Static:
   + функция ConvertItem - обратное преобразование элементов диалога из
   внутреннего представления во внешние
*/
void Dialog::ConvertItem(int FromPlugin,
                         struct FarDialogItem *Item,struct DialogItem *Data,
                         int Count)
{
  int I;
  if(!FromPlugin)
    for (I=0; I < Count; I++)
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
      strcpy(Item[I].Data,Data[I].Data);
    }
  else
    for (I=0; I < Count; I++)
    {
      Data[I].Type=Item[I].Type;
      Data[I].X1=Item[I].X1;
      Data[I].Y1=Item[I].Y1;
      Data[I].X2=Item[I].X2;
      Data[I].Y2=Item[I].Y2;
      Data[I].Focus=Item[I].Focus;
      Data[I].Selected=Item[I].Selected;
      Data[I].Flags=Item[I].Flags;
      Data[I].DefaultButton=Item[I].DefaultButton;
      strcpy(Data[I].Data,Item[I].Data);
    }
}
/* SVS $ */

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

/* $ 28.07.2000 SVS
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
int Dialog::IsFocused(int Type)
{
  return(Type==DI_EDIT ||
         Type==DI_FIXEDIT ||
         Type==DI_PSWEDIT ||
         Type==DI_COMBOBOX ||
         Type==DI_BUTTON ||
         Type==DI_CHECKBOX ||
         Type==DI_RADIOBUTTON ||
         Type==DI_LISTBOX);
}
/* SVS $ */

/* $ 26.07.2000 SVS
   AutoComplite: Поиск входжение подстроки в истории
*/
/* $ 28.07.2000 SVS
   ! Переметр Edit *EditLine нафиг ненужен!
*/
int Dialog::FindInEditForAC(int TypeFind,void *HistoryName,char *FindStr)
{
  char Str[1024];
  int I, Count;

  if(!TypeFind)
  {
    char RegKey[80],KeyValue[80];
    sprintf(RegKey,fmtSavedDialogHistory,(char*)HistoryName);
    // просмотр пунктов истории
    for (I=0; I < 16; I++)
    {
      sprintf(KeyValue,fmtLine,I);
      GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
      if (!LocalStrnicmp(Str,FindStr,strlen(FindStr)))
        break;
    }
    if (I == 16)
      return FALSE;
    /* $ 28.07.2000 SVS
       Введенные буковки не затрагиваем, а дополняем недостающее.
    */
    strcat(FindStr,&Str[strlen(FindStr)]);
    /* SVS $ */
  }
  else
  {
    struct FarListItem *ListItems=((struct FarListItems *)HistoryName)->Items;
    int Count=((struct FarListItems *)HistoryName)->CountItems;

    for (I=0; I < Count ;I++)
    {
      if (!LocalStrnicmp(ListItems[I].Text,FindStr,strlen(FindStr)))
        break;
    }
    if (I  == Count)
      return FALSE;
    strcat(FindStr,&ListItems[I].Text[strlen(FindStr)]);
  }
  return TRUE;
}
/*  SVS $ */

/* Private:
   Заполняем выпадающий список из истории
*/
/* $ 26.07.2000 SVS
  + Дополнительный параметр в SelectFromEditHistory для выделения
   нужной позиции в истории (если она соответствует строке ввода)
*/
void Dialog::SelectFromEditHistory(Edit *EditLine,char *HistoryName,char *IStr)
/* SVS $ */
{
  char RegKey[80],KeyValue[80],Str[512];
  int I,Dest;
  int Checked;

  sprintf(RegKey,fmtSavedDialogHistory,HistoryName);
  {
    // создание пустого вертикального меню
    VMenu HistoryMenu("",NULL,0,8,VMENU_ALWAYSSCROLLBAR);

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
    for (Dest=I=0; I < 16; I++)
    {
      sprintf(KeyValue,fmtLine,I);
      GetRegKey(RegKey,KeyValue,Str,"",sizeof(Str));
      if (*Str==0)
        continue;

      sprintf(KeyValue,fmtLocked,I);

      GetRegKey(RegKey,KeyValue,(int)Checked,0);
      HistoryItem.Checked=Checked;
      /* $ 26.07.2000 SVS
         Выставим Selected при полном совпадении строки ввода и истории
      */
      if((HistoryItem.Selected=(!Dest && !strcmp(IStr,Str))?TRUE:FALSE) == TRUE)
         Dest++;
      /* SVS $ */
      strncpy(HistoryItem.Name,Str,sizeof(HistoryItem.Name)-1);
      HistoryItem.Name[sizeof(HistoryItem.Name)-1]=0;
      strncpy(HistoryItem.UserData,Str,sizeof(HistoryItem.UserData));
      HistoryItem.UserDataSize=strlen(Str)+1;
      HistoryMenu.AddItem(&HistoryItem);
      ItemsCount++;
    }
    if (ItemsCount==0)
      return;

    /* $ 28.07.2000 SVS
       Перед отрисовкой спросим об изменении цветовых атрибутов
    */
    short Colors[9];
    HistoryMenu.GetColors(Colors);
    if(DlgProc((HANDLE)this,DMSG_CTLCOLORDLGLIST,
                    sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
      HistoryMenu.SetColors(Colors);
    /* SVS $ */
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
        SelectFromEditHistory(EditLine,HistoryName,IStr);
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
        /* $ 28.07.2000 SVS
           Сообщим о случивщемся факте процедуре обработки диалога
        */
        if(!DlgProc((HANDLE)this,DMSG_HOTKEY,I,Key))
          break; // сказали не продолжать обработку...
        /* SVS $ */

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
         struct FarListItems *List,    // список строк
         char *IStr)
{
  char Str[512];
  struct MenuItem ComboBoxItem;
  struct FarListItem *ListItems=List->Items;
  int EditX1,EditY1,EditX2,EditY2;
  int I,Dest;
  {
    // создание пустого вертикального меню
    //  с обязательным показом ScrollBar
    VMenu ComboBoxMenu("",NULL,0,8,VMENU_ALWAYSSCROLLBAR);

    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
    if (EditX2>ScrX)
      EditX2=ScrX;
    ComboBoxMenu.SetFlags(MENU_SHOWAMPERSAND);
    ComboBoxMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
    ComboBoxMenu.SetBoxType(SHORT_SINGLE_BOX);

    // заполнение пунктов меню
    /* Последний пункт списка - ограничититель - в нем Tetx[0]
       должен быть равен '\0'
    */
    for (Dest=I=0;I < List->CountItems;I++)
    {
      /* $ 28.07.2000 SVS
         Выставим Selected при полном совпадении строки ввода и списка
      */
      if(IStr && *IStr)
      {
        if((ComboBoxItem.Selected=(!Dest && !strcmp(IStr,ListItems[I].Text))?TRUE:FALSE) == TRUE)
           Dest++;
      }
      else
         ComboBoxItem.Selected=ListItems[I].Flags&LIF_SELECTED;

      ComboBoxItem.Separator=ListItems[I].Flags&LIF_SEPARATOR;
      ComboBoxItem.Checked=ListItems[I].Flags&LIF_CHECKED;
      /* 01.08.2000 SVS $ */
      /* SVS $ */
      strcpy(ComboBoxItem.Name,ListItems[I].Text);
      strcpy(ComboBoxItem.UserData,ListItems[I].Text);
      ComboBoxItem.UserDataSize=strlen(ListItems[I].Text);
      ComboBoxMenu.AddItem(&ComboBoxItem);
    }

    /* $ 28.07.2000 SVS
       Перед отрисовкой спросим об изменении цветовых атрибутов
    */
    short Colors[9];
    ComboBoxMenu.GetColors(Colors);
    if(DlgProc((HANDLE)this,DMSG_CTLCOLORDLGLIST,
                    sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
      ComboBoxMenu.SetColors(Colors);
    /* SVS $ */

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
    /* Запомним текущее состояние */
    for (I=0;ListItems[I].Text[0];I++)
      ListItems[I].Flags&=~LIF_SELECTED;
    ListItems[ComboBoxMenu.GetSelectPos()].Flags|=LIF_SELECTED;
  }
  EditLine->SetString(Str);
  EditLine->SetLeftPos(0);
  Redraw();
}
/* SVS $ */

/* $ 28.07.2000 SVS
   функция обработки диалога (по умолчанию)
   Вот именно эта функция и является последним рубежом обработки диалога.
   Т.е. здесь должна быть ВСЯ обработка ВСЕХ сообщений!!!
*/
long WINAPI Dialog::DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  struct DialogItem *CurItem;
  char *Ptr, Str[1024];
  int Len, Type, I;

  if(!Dlg)
    return 0;

  switch(Msg)
  {
    /* Сообщение DMSG_INITDIALOG посылается процедуре обработки диалога
       после того, как были проинициализированы все управляющие элементы
       диалога, но до того, как они стали видимы. В ответ на данное сообщение
       процедура обработки диалога инициализирует каждый элемент в некоторое
       корректное начально состояние. Например, она может заполнить блок
       списка элементами, которые потом просмотрит пользователь...
       Param1 = ID элемента, который получит фокус ввода по умолчанию.
       Param2 = Специфические данные, переденные функции DialogEx.
       Return = TRUE -  если процедура обработки диалога сама установила
                        фокус ввода на конкретный элемент.
                FALSE - будет установлен фокус ввода на элемент, указанный
                        в Param1
    */
    case DMSG_INITDIALOG:
      return TRUE;

    case DMSG_CLOSE:
      return TRUE;

    /* Сообщение DMSG_KILLFOCUS передается процедуре обработки диалога
       непосредственно перед потерей клавиатурного фокуса элементом
       диалога.
       Param1 = ID элемента, теряющего фокус ввода.
       Param2 = 0
       Return = -1 - "Согласен с потерей фокуса".
               >=0 - номер ЖЕЛАЕМОГО элемента, которому хотим передать фокус.
    */
    case DMSG_KILLFOCUS:
      return -1;

    /* Сообщение DMSG_GOTFOCUS посылается процедуре обработки диалога
       после того, как элемент диалога получил клавиатурный фокус ввода.
       Это есть нотификационное сообщение!
       Param1 = ID элемента получившего фокус ввода.
       Param2 = 0
       Return = 0
    */
    case DMSG_GOTFOCUS:
      return 0;

    /* Сообщение DMSG_HELP передается в обработчик диалога перед выводом
       темы помощи. Это сообщение позволяет управлять показом темы помощи
       на уровне отдельного элемента диалога.
       Param1 = ID элемента диалога, имеющий фокус ввода (текущий элемент)
       Param2 = Адрес строки темы подсказки, связанной с данным диалогом,
                который предполагается показать.
       Return = Адрес строки темы подсказки, связанной с данным диалогом,
                который будет выведен.
                Если вернули NULL, то тема помощи выводиться не будет.
    */
    case DMSG_HELP:
      return Param2;

    /* Сообщение DMSG_PAINT посылается перед прорисовкой всего диалога.
       Param1 = 0
       Param2 = 0
       Return = 0
    */
    case DMSG_PAINT:
      return 0;

    /* Сообщение DMSG_CTLCOLORDIALOG посылается в функцию обработки диалога
       перед прорисовкой подложки окна диалога.
       Сообщение приходит сразу после DMSG_PAINT.
       Param1 = 0
       Param2 = Атрибуты (цвет_фона+цвет_текста), с использованием которых
                обработчик диалога хочет отрисовать подложку диалога.
       Return = Атрибуты (цвет_фона+цвет_текста), с использованием которых
                обработчик диалога должен отрисовать подложку диалога.
    */
    case DMSG_CTLCOLORDIALOG:
      return Param2;

    /* Сообщение DMSG_CTLCOLORDLGITEM посылается процедуре обработки диалога
       перед отрисовкой конкретного элемента диалога. В ответ на это сообщение
       процедура обработки диалога может установить свои атрибуты (цвет текста
       и фона) для заданного элемента.
       Param1 = ID эелемента, который будет отрисован.
       Param2 = Атрибуты (цвет_фона+цвет_текста), с использованием которых
                обработчик диалога хочет отрисовать элемент.
       Return = Атрибуты (цвет_фона+цвет_текста), с использованием которых
                будет отрисован элемент.
    */
    case DMSG_CTLCOLORDLGITEM:
      return Param2;

    /* Сообщение DMSG_CTLCOLORDLGLIST посылается процедуре обработки диалога
       перед отрисовкой списка (DI_COMBOBOX, DI_LISTBOX, DIF_HISTORY). В ответ
       на это сообщение процедура обработки диалога может установить свои
       атрибуты (цвет текста и фона) для заданного элемента.
       Param1 = Количество элементов передаваемого массива атрибутов.
       Param2 = Указатель на массив атрибутов (цвет_фона+цвет_текста), с
                использованием которых обработчик диалога хочет отрисовать
                список:
                ListColorBody=0,      // подложка
                ListColorBox=1,       // рамка
                ListColorTitle=2,     // заголовок - верхний и нижний
                ListColorText=3,      // Текст пункта
                ListColorHilite=4,    // HotKey
                ListColorSeparator=5, // separator
                ListColorSelected=6,  // Выбранный
                ListColorHSelect=7,   // Выбранный - HotKey
                ListColorScrollBar=8  // ScrollBar
       Return = TRUE - если атрибуты изменены
                FALSE - оставить все как есть.
    */
    case DMSG_CTLCOLORDLGLIST:
      return FALSE;

    /* Сообщение DMSG_ENTERIDLE посылается в процедуру обработки диалога,
       который входит в холостое состояние. Диалоговое окно входит в
       состояние ожидания, когда нет никаких сообщений.
       Param1 = 0
       Param2 = 0
       Return = 0
    */
    case DMSG_ENTERIDLE:
      return 0;
  }

  // предварительно проверим...
  if(Param1 >= Dlg->ItemCount)
    return 0;

  CurItem=&Dlg->Item[Param1];
  Type=CurItem->Type;

  Ptr=CurItem->Data;

  switch(Msg)
  {
    /* Сообщение DMSG_DRAWITEM посылается обработчику диалога перед отрисовкой
       элемента диалога.
       Param1 = ID элемента диалога, который будет отрисован.
       Param2 = Указатель на структуру FarDialogItem, описывающую элемент для
                отрисовки.
       Return = 0
    */
    case DMSG_DRAWITEM:
      return 0;

    /* Сообщение DMSG_HOTKEY посылается в обработчик диалога, когда
       пользователь нажал Alt-буква.
       Param1 = ID элемента диалога, на который падет выбор.
       Param2 = Внутренний код клавиши.
       Return = TRUE - согласен - продолжаем обработку
                FALSE - не продолжать процесс далее.
    */
    case DMSG_HOTKEY:
      return TRUE;

    /* Сообщение DMSG_CHANGEITEM оповещает обработчик об изменении состояния
       элемента диалога - ввели символ в окне редактирования, переключили
       CheckBox (RadioButton),...
       Param1 = ID элемента диалога.
       Param2 = Указатель на структуру FarDialogItem, описывающую элемент для
                отрисовки.
       Return = TRUE - разрешить изменение (и соответственно отрисовку того,
                что пользователь ввёл)
                FALSE - запретить изменение.
    */
    case DMSG_CHANGEITEM:
      return TRUE;


    /* Сообщение LMSG_CHANGELIST оповещает обработчик об изменении состояния
       элемента списка (DI_COMBOBOX, DI_LISTBOX, DIF_HISTORY).
       Param1 = ID элемента (DI_COMBOBOX, DI_LISTBOX, DIF_HISTORY).
       Param2 = Указатель на структуру FarListItems, описывающую
                измененный элемент.
       Return = TRUE - разрешить изменение (и соответственно отрисовку того,
                что пользователь ввёл)
                FALSE - запретить изменение.
       Здесь это сообшение формирует запрос к функции диалога!
    */
    case DMSG_CHANGELIST:
      return TRUE;
  }

  return 0;
}
/* SVS $ */

/* $ 28.07.2000 SVS
   Посылка сообщения диалогу
   Некоторые сообщения эта функция обрабатывает сама, не передавая управление
   обработчику диалога.
*/
long WINAPI Dialog::SendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  struct DialogItem *CurItem;
  char *Ptr, Str[1024];
  int Len, Type, I;
  struct FarDialogItem PluginDialogItem;

  if(!Dlg)
    return 0;
  // предварительно проверим...
  if(Param1 >= Dlg->ItemCount)
    return 0;

  CurItem=&Dlg->Item[Param1];
  Type=CurItem->Type;
  Ptr=CurItem->Data;

  switch(Msg)
  {
    /* Сообщение DMSG_SETTEXT посылается стандартному обработчику диалога для
       установки строки ввода или заголовка элементов DI_CHECKBOX, DI_TEXT,...
       в новое значение.
       Param1 = ID требуемого элемента диалога.
       Param2 = Адрес строки, содержащей текст. Значение этого параметра равное
                NULL игнорируется.
       Return = Размер установленных данных +1 для символа "конец строки" ('\0')
    */
    case DMSG_SETTEXT:
      if(Param2) // если здесь NULL, то это еще один способ получить размер
      {
        switch(Type)
        {
          case DI_BUTTON:
          case DI_TEXT:
          case DI_VTEXT:
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
          case DI_CHECKBOX:
          case DI_RADIOBUTTON:
            strcpy(Ptr,(char *)Param2);
            break;

          case DI_COMBOBOX:
          case DI_EDIT:
          case DI_PSWEDIT:
          case DI_FIXEDIT:
            ((Edit *)(CurItem->ObjPtr))->SetString((char *)Param2);
            ((Edit *)(CurItem->ObjPtr))->Select(-1,-1); // снимаем выделение
            break;

          case DI_LISTBOX: // пока не трогаем - не реализован
            return 0;

          default:  // подразумеваем, что остались
            return 0;
        }
        Dlg->InitDialogObjects(); // переинициализируем элементы диалога
        return strlen((char *)Param2)+1;
      }
      return 0;

    /* Сообщение DMSG_SETTEXTLENGTH посылается стандартному обработчику
       диалога для установки максимального размера редактируемой строки
       строки...
       Param1 = ID элемента диалога (воздействует только на DI_COMBOBOX -
                без флага DIF_DROPDOWNLIST, DI_EDIT, DI_PSWEDIT, DI_FIXEDIT)
       Param2 = Требуемый размер
       Return = Предыдущее значение размера редактируемой строки
                или 0 в случае ошибки.
    */
    case DMSG_SETTEXTLENGTH:
      if(IsEdit(Type) && !(CurItem->Flags & DIF_DROPDOWNLIST))
      {
        Param1=((Edit *)(CurItem->ObjPtr))->GetMaxLength();
        ((Edit *)(CurItem->ObjPtr))->SetMaxLength(Param2);
        Dlg->InitDialogObjects(); // переинициализируем элементы диалога
        return Param1;
      }
      return 0;

    /* Сообщение LMSG_CHANGELIST оповещает обработчик об изменении состояния
       элемента списка (DI_COMBOBOX, DI_LISTBOX, DIF_HISTORY).
       Param1 = ID элемента (DI_COMBOBOX, DI_LISTBOX, DIF_HISTORY).
       Param2 = Указатель на структуру FarListItems, описывающую
                измененный элемент.
       Return = TRUE - разрешить изменение (и соответственно отрисовку того,
                что пользователь ввёл)
                FALSE - запретить изменение.
       Здесь это сообшение формирует запрос к функции диалога!
    */
    case DMSG_CHANGELIST:
      return Dlg->DlgProc(hDlg,Msg,Param1,(long)&CurItem->ListItems);

    /* Сообщение DMSG_CHANGEITEM оповещает обработчик об изменении состояния
       элемента диалога - ввели символ в окне редактирования, переключили
       CheckBox (RadioButton),...
       Param1 = ID элемента диалога.
       Param2 = Указатель на структуру FarDialogItem, описывающую измененный
                элемент.
       Return = TRUE - разрешить изменение (и соответственно отрисовку того,
                что пользователь ввёл)
                FALSE - запретить изменение.
       Здесь это сообшение формирует запрос к функции диалога!
    */
    case DMSG_CHANGEITEM:
      // преобразуем данные для!
      Dialog::ConvertItem(0,&PluginDialogItem,CurItem,1);
      I=Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem);
      Dialog::ConvertItem(1,&PluginDialogItem,CurItem,1);
      return I;

    /* Сообщение DMSG_DRAWITEM посылается обработчику диалога перед отрисовкой
       элемента диалога.
       Param1 = ID элемента диалога, который будет отрисован.
       Param2 = Указатель на структуру FarDialogItem, описывающую элемент для
                отрисовки.
       Return = 0
       Здесь это сообшение формирует запрос к функции диалога!
    */
    case DMSG_DRAWITEM:
      // преобразуем данные для!
      Dialog::ConvertItem(0,&PluginDialogItem,CurItem,1);
      Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem);
      Dialog::ConvertItem(1,&PluginDialogItem,CurItem,1);
      return 0;

    /* Плагин посылает сообщение DMSG_SETREDRAW стандартному обработчику
       диалога для перерисовки все диалогово окна.
       Param1 = 0
       Param2 = 0
       Return = 0
    */
    case DMSG_SETREDRAW:
      if(Dlg->InitObjects)
        Dlg->Show();
      return 0;

    /* Сообщение DMSG_SETFOCUS устанавливает клавиатурный фокус на заданный
       элемент диалога.
       Param1 = ID элемента, который должен получить фокус ввода.
       Param2 = 0
       Return = FALSE - это элемент не может иметь фокус ввода
                TRUE  - фокус ввода успешно установлен (с оговоркой!)
    */
    case DMSG_SETFOCUS:
      if(!Dialog::IsFocused(Type))
        return FALSE;
      Dlg->ChangeFocus(Param1,1,0);
      return TRUE;

    /* Сообщение DMSG_GETTEXT позволяет получить содержимое строк ввода или
       заголовков элементов DI_CHECKBOX, DI_TEXT,...
       Param1 = ID требуемого элемента
       Param2 = Укзатель на строку приемник.
                Если Param2 = NULL - это еще один способ получить размер
                данных (см. DMSG_GETTEXTLENGTH)
       Return = Размер данных +1 для символа "конец строки" ('\0')
    */
    case DMSG_GETTEXT:
      if(Param2) // если здесь NULL, то это еще один способ получить размер
      {
        switch(Type)
        {
          case DI_BUTTON:
            Len=strlen(Ptr);
            if (!(CurItem->Flags & DIF_NOBRACKETS))
            {
              Ptr+=2;
              Len-=4;
            }
            memmove((char *)Param2,Ptr,Len);
            ((char *)Param2)[Len]=0;
            break;

          case DI_TEXT:
          case DI_VTEXT:
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
          case DI_CHECKBOX:
          case DI_RADIOBUTTON:
            strcpy((char *)Param2,Ptr);
            break;

          case DI_COMBOBOX:
          case DI_EDIT:
          case DI_PSWEDIT:
          case DI_FIXEDIT:
            ((Edit *)(CurItem->ObjPtr))->GetString(Str,sizeof(Str));
            strcpy((char *)Param2,Str);
            break;

          case DI_LISTBOX: // пока не трогаем - не реализован
            *(char *)Param2='\0';
            break;

          default:  // подразумеваем, что остались
            *(char *)Param2='\0';
            break;
        }
        return strlen((char *)Param2)+1;
      }
      // здесь умышленно не ставим return, т.к. хотим получить размер
      // следовательно сразу должен идти "case DMSG_GETTEXTLENGTH"!!!

    /* Сообщение DMSG_GETTEXTLENGTH посылается стандартному обработчику диалога для получения размера строки...
       Param1 = ID требуемого элемента
       Param2 = 0
       Return = Размер данных (с учетом конечного нуля)
    */
    case DMSG_GETTEXTLENGTH:
      Len=strlen(Ptr)+1;
      switch(Type)
      {
        case DI_BUTTON:
          if (!(CurItem->Flags & DIF_NOBRACKETS))
            Len-=4;
          break;

        case DI_TEXT:
        case DI_VTEXT:
        case DI_SINGLEBOX:
        case DI_DOUBLEBOX:
        case DI_CHECKBOX:
        case DI_RADIOBUTTON:
          break;

        case DI_COMBOBOX:
        case DI_EDIT:
        case DI_PSWEDIT:
        case DI_FIXEDIT:
          Len=((Edit *)(CurItem->ObjPtr))->GetLength();

        case DI_LISTBOX: // пока не трогаем - не реализован
          Len=0;
          break;

        default:
          Len=0;
          break;
      }
      return Len;

    /* Сообщение DMSG_GETDLGITEM посылается стандартному обработчику
       диалога для получения полной информации о заданном элементе.
       Param1 = ID элемента диалога
       Param2 = Указатель на структуру FarDialogItem
       Return = TRUE - данные скопированы!
    */
    case DMSG_GETDLGITEM:
      if(Param2)
      {
        Dialog::ConvertItem(0,(struct FarDialogItem *)Param2,CurItem,1);
        if(IsEdit(Type))
        {
          ((Edit *)(CurItem->ObjPtr))->GetString(Str,sizeof(Str));
          strcpy((char *)Param2,Str);
        }
        else
          strcpy(((struct FarDialogItem *)Param2)->Data,CurItem->Data);
        return TRUE;
      }
      return FALSE;

    /* Сообщение DMSG_SETDLGITEM посылается стандартному обработчику диалога
       для изменения информации о заданном элементе.
       Param1 = ID элемента диалога
       Param2 = Указатель на структуру FarDialogItem
       Return = TRUE - данные обработаны!
    */
    case DMSG_SETDLGITEM:
      if(Param2)
      {
        Dialog::ConvertItem(TRUE,(struct FarDialogItem *)Param2,CurItem,1);
        CurItem->Type=Type;
        // еще разок, т.к. данные могли быть изменены
        Dlg->InitDialogObjects();
        return TRUE;
      }
      return FALSE;
  }

  // Все, что сами не отрабатываем - посылаем на обработку обработчику.
  return Dlg->DlgProc(hDlg,Msg,Param1,Param2);
}
/* SVS $ */


/* $ 31.07.2000 tran
   + функция подравнивания координат edit классов */
void Dialog::AdjustEditPos(int dx, int dy)
{
  struct DialogItem *CurItem;
  int I;
  int x1,x2,y1,y2;

  Edit *DialogEdit;
  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];
    if (IsEdit(CurItem->Type))
    {
       DialogEdit=(Edit *)CurItem->ObjPtr;
       DialogEdit->GetPosition(x1,y1,x2,y2);
       x1+=dx;
       x2+=dx;
       y1+=dy;
       y2+=dy;
       DialogEdit->SetPosition(x1,y1,x2,y2);
    }
  }
}
/* tran 31.07.2000 $ */
