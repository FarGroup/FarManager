/*
dialog.cpp

Класс диалога

*/

/* Revision: 1.48 20.10.2000 $ */

/*
Modify:
  20.10.2000 SVS
    + DM_GETFOCUS - получить ID элемента имеющего фокус ввода
  16.10.2000 tran 1.47
   + для EDIT полей выставляется ограничение в 511 символов
  27.09.2000 SVS
   ! Alt-Up/Down/Left/Right - убрал (чтобы в будущем не пересекались
     с MultiEdit)
   ! Ctrl-Alt-Shift - реагируем, если надо.
  24.09.2000 SVS
   + Движение диалога - Alt-стрелки
   + вызов функции Xlat
  22.09.2000 SVS
   ! Уточнение AutoComplete при постоянных блоках.
  20.09.2000 SVS
   ! Enter в строках ввода (кроме DIF_EDITOR) завершает диалог.
  18.09.2000 SVS
   + DIF_READONLY - флаг для строк редактирования
      (пока! для строк редактирования).
  18.09.2000 SVS
   ! Уточнения для SelectOnEntry
   ! Маска не должна быть пустой (строка из пробелов не учитывается)!
  14.09.2000 SVS
   + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX
      выставляется флаг MENU_SHOWAMPERSAND. Этот флаг подавляет такое
      поведение
  12.09.2000 SVS
   ! Задаем поведение для кнопки с DefaultButton=1:
     Такая кнопка независимо от стиля диалога инициирует сообщение DM_CLOSE.
   ! Исправляем ситуацию с BackSpace в DIF_EDITOR
   ! Решаем проблему, если Del нажали в позиции большей, чем длина
     строки (DIF_EDITOR)
  11.09.2000 SVS
   + Ctrl-U в строках ввода снимает пометку блока
  09.09.2000 SVS
   + DIF_NOFOCUS - элемент не получает фокуса ввода (клавиатурой)
   + Стиль диалога DMODE_OLDSTYLE - диалог в старом стиле.
   ! вывалить из диалога, ткнув вне диалога, сможем только в старом стиле.
   + Функция SelectOnEntry - выделение строки редактирования
     (Обработка флага DIF_SELECTONENTRY)
  08.09.2000 SVS
   - Если коротко, то DM_SETFOCUS вроде как и работал :-)
   ! Уточнение для DN_MOUSECLICK
  31.08.2000 SVS
   + DM_ENABLE (не полностью готов :-)
   - Бага с вызовом файлов помощи.
  30.08.2000 SVS
   + Метод Hide()
   + Режим диалога DMODE_SHOW - Диалог виден?
   ! уточнения для IsEnableRedraw
   + DM_MOVEDIALOG - перемещение диалога.
   ! Изменение цветов для ComboBox(DowpDownList)
  29.08.2000 SVS
   ! При подмене темы помощи из диаловой процедуры...
     короче, нужно вновь формировать контент!
  29.08.2000 SVS
   - Первый официальный альфа-баг - функция ProcessHighlighting
     MY> Работа с диалогами стала ГЛЮЧНАЯ. Я имею в виду горячие клавиши.
     MY> Входим в настройку чего угодно, жмем Alt-нужную букву и
     MY> наблюдаем разнообразные глюки.
  28.08.2000 SVS
   - баг рук кривых (или не внимательности!) :-)
  25.08.2000 SVS
   + DM_GETDLGRECT - получить координаты диалогового окна
   ! Уточнение для DN_MOUSECLICK
  25.08.2000 SVS
   ! Уточнения, относительно фокуса ввода - ох уж эти сказочки, блин.
  24.08.2000 SVS
   + InitDialogObjects имеет параметр - для выборочной реинициализации
     элементов
  24.08.2000 SVS
   ! Охрененная дыра!!!! (дальше только матом)... это про ComboBox
     Этого лучше не знать...
   + CtrlAltShift - спрятать/показать диалог...
   + Элемент DI_USERCONTROL - отрисовкой занимается плагин.
  24.08.2000 SVS
   ! Критическая ошибка - с фокусов не порядки...
     перестарался с ChangeFocus()
  23.08.2000 SVS
   ! Уточнения категорий DMSG_* -> DM_ (месаг) & DN_ (нотифи)
   + DM_KEY        - послать/получить клавишу(ы)
   + DM_GETDLGDATA - взять данные диалога.
   + DM_SETDLGDATA - установить данные диалога.
   + DM_SHOWDIALOG - показать/спрятать диалог
   + Переменная класса FocusPos - всегда известно какой элемент в фокусе
   ! Переменные IsCanMove, InitObjects, CreateObjects, WarningStyle, Dragged
     удалены -> битовые флаги в DialogMode
   + множество мелких уточнений ;-)
  22.08.2000 SVS
   ! С моим английским вообще ни как :-((
     IsMovedDialog -> IsCanMove
   ! DMSG_PAINT -> DMSG_DRAWDIALOG
   ! DMSG_DRAWITEM -> DMSG_DRAWDLGITEM
   ! DMSG_CHANGELIST -> DMSG_LISTCHANGE
   ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
  21.08.2000 SVS 1.23
   ! Описание сообщений убрано - смотрите в хелпе :-)))
   ! Самыми последними в этом файле должны быть DefDlgProc и SendDlgMessage
     (мне так удобно :-)
   + Перед отрисовкой DI_LISTBOX спросим об изменении цветовых атрибутов
   ! DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
   - глюк в AutoComplete при включенных постоянных блоках
   ! DMSG_CHANGEITEM -> DMSG_EDITCHANGE
   + DMSG_BTNCLICK
  12.08.2000 KM 1.20
   + Добавление работы DI_FIXEDIT с флагом DIF_MASKEDIT для установки
     маски в строку ввода.
  18.08.2000 SVS
   + DialogMode - Флаги текущего режима диалога
   + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
   + Сообщения: DMSG_ENABLEREDRAW, DMSG_MOUSECLICK
   + DI_BUTTON тоже теперь может иметь DIF_SETCOLOR
   + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
   - Если на выход процедура ответила "НЕТ", то диалог зацикливался, т.к.
     не был сброшен флаг выхода.
  15.08.2000 SVS
   ! Для DropDownList цвета обрабатываем по иному.
   + Сделаем так, чтобы ткнув мышкой в DropDownList список раскрывался сам.
  11.08.2000 SVS
   + Данные, специфические для конкретного экземпляра диалога
   + Для того, чтобы послать DMSG_CLOSE нужно переопределить Process
   ! Уточнение для DMSG_CLOSE
  11.08.2000 KM 1.18
   ! Убран дублирующий код, исправляющий некорректное перемещение диалога
     мышкой. Оказывается мы с Андреем сделали патчик в один день :)
   ! Чуть-чуть переделано таскание мышкой диалога. Было: после первого нажатия
     мышкой на диалоге отображение режима перемещения не включалось до тех пор
     пока не начиналось само движение диалога, после чего и происходило
     отображение начала этого режима, что есть в корне идеологически неверно :)
   ! Артефакт: если после первого запуска фара, не пермещая мышь
     нажать левую кнопку, чтобы переместить диалог (диалог находится под мышью),
     то из-за того, что к этому моменту PrevMouseX и PrevMouseY ещё не определены,
     также неопределённым получался прыжок диалога.
  10.08.2000 SVS
   + переменная IsMovedDialog - можно ли двигать диалог :-)
   + функция установки IsMovedDialog
  09.08.2000 tran 1.16
   - убраны "салазки"
  09.08.2000 KM
   ! При включении режима перемещения диалога добавлено
     отключение мигающего курсора. Так косметика.
   ! Поправлено перемещение диалога мышкой, Андрей-то не захотел
     исправлять :). Теперь перемещение стало корректным, без прокатывания
     диалога по экрану.
   ! При выходе за край экрана диалогом, тень, по возможности, продолжает
     рисоваться.

     Номер редакции остался прежним - здесь было
     забегание вперёд.
  07.08.2000 SVS
   + В Функции выравнивания кооржинат про ListBox забыли!
  04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  03.08.2000 tran
   + мышиный перетаск диалога - хватание за пустое место
     внимание - ограничение невыхода за границы экрана посталено не зря
     иначе ползут глюки...
     изменена индикация - по углам красные палки
     с MOVE в угла могут быть глюки в изображении.
     строки про MOVE закоментарены "//# "
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

//////////////////////////////////////////////////////////////////////////
/* Public:
   Конструктор класса Dialog
*/
Dialog::Dialog(struct DialogItem *Item,int ItemCount,
               FARWINDOWPROC DlgProc,long InitParam)
{
  /* $ 29.08.2000 SVS
    Номер плагина, вызвавшего диалог (-1 = Main)
  */
  PluginNumber=-1;
  /* SVS $ */
  /* $ 23.08.2000 SVS
    Режимы диалога.
  */
  DialogMode=0;
  /* SVS $ */
  /* $ 11.08.2000 SVS
    + Данные, специфические для конкретного экземпляра диалога
  */
  Dialog::DataDialog=InitParam;
  /* SVS $ */
  DialogTooLong=0;
  /* $ 10.08.2000 SVS
     Изначально диалоги можно таскать
  */
  SetDialogMode(DMODE_ISCANMOVE);
  /* SVS $ */
  /* $ 18.08.2000 SVS
  */
  /*
    + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
      =0 - разрешена, другие значение - не перерисовывать
        Когда посылается сообщение DMSG_ENABLEREDRAW, то этот флаг
        при Param1=TRUE увеличивается, при Param1 = FALSE - уменьшается
  */
  IsEnableRedraw=0;

  FocusPos=-1;
  PrevFocusPos=-1;

  if(!DlgProc) // функция должна быть всегда!!!
  {
    DlgProc=(FARWINDOWPROC)Dialog::DefDlgProc;
    // знать диалог в старом стиле - учтем этот факт!
    SetDialogMode(DMODE_OLDSTYLE);
  }
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


//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
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

/* $ 30.08.2000 SVS
  Цель перехвата данной функции - управление видимостью...
*/
void Dialog::Hide()
{
  ScreenObject::Hide();
  SkipDialogMode(DMODE_SHOW);
}
/* SVS $*/

//////////////////////////////////////////////////////////////////////////
/* Private, Virtual:
   Инициализация объектов и вывод диалога на экран.
*/
void Dialog::DisplayObject()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  Shadow();              // "наводим" тень

  if (!CheckDialogMode(DMODE_INITOBJECTS))      // самодостаточный вариант, когда
  {                      //  элементы инициализируются при первом вызове.
    /* $ 28.07.2000 SVS
       Укажем процедуре, что у нас все Ок!
    */
    if(DlgProc((HANDLE)this,DN_INITDIALOG,InitDialogObjects(),DataDialog))
    {
      // еще разок, т.к. данные могли быть изменены
      InitDialogObjects();
    }
    // все объекты проинициализированы!
    SetDialogMode(DMODE_INITOBJECTS);
  }

  ShowDialog();          // "нарисуем" диалог.
}


//////////////////////////////////////////////////////////////////////////
/* Public:
   Инициализация элементов диалога.
*/
/* $ 28.07.2000 SVS
   Теперь InitDialogObjects возвращает ID элемента
   с фокусом ввода
*/
/* $ 24.08.2000 SVS
  InitDialogObjects имеет параметр - для выборочной реинициализации
  элементов. ID = -1 - касаемо всех объектов
*/
int Dialog::InitDialogObjects(int ID)
{
  int I, J, TitleSet;
  int Length,StartX;
  int Type;
  struct DialogItem *CurItem;
  int InitItemCount;

  if(ID+1 > ItemCount)
    return -1;

  if(ID == -1) // инициализируем все?
  {
    ID=0;
    InitItemCount=ItemCount;
  }
  else
  {
    InitItemCount=ID+1;
  }


  // предварительный цикл по поводу кнопок и заголовка консоли
  for(I=ID, TitleSet=0; I < InitItemCount; I++)
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
     if(FocusPos == -1 &&
        IsFocused(CurItem->Type) &&
        CurItem->Focus &&
        !(CurItem->Flags&(DIF_DISABLE|DIF_NOFOCUS)))
       FocusPos=I; // запомним первый фокусный элемент
     CurItem->Focus=0; // сбросим для всех, чтобы не оказалось,
                       //   что фокусов - как у дурочка фантиков
  }

  // Опять про фокус ввода - теперь, если "чудо" забыло выставить
  // хотя бы один, то ставим на первый подходящий
  if(FocusPos == -1)
  {
    for (I=0; I < ItemCount; I++) // по всем!!!!
    {
      CurItem=&Item[I];
      if(IsFocused(CurItem->Type) && !(CurItem->Flags&(DIF_DISABLE|DIF_NOFOCUS)))
      {
        FocusPos=I;
        break;
      }
    }
  }
  if(FocusPos == -1) // ну ни хрена себе - нет ни одного
  {                  //   элемента с возможностью фокуса
     FocusPos=0;     // убится, блин
  }

  // ну вот и добрались до!
  Item[FocusPos].Focus=1;

  // если будет редактор, то обязательно будет выделен.
  SelectOnEntry(FocusPos);

  // а теперь все сначала и по полной программе...
  for (I=ID; I < InitItemCount; I++)
  {
    CurItem=&Item[I];
    Type=CurItem->Type;

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
      if (!CheckDialogMode(DMODE_CREATEOBJECTS))
        CurItem->ObjPtr=new VMenu(NULL,NULL,0,CurItem->Y2-CurItem->Y1+1,
                               VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,NULL/*,this*/);

      VMenu *ListBox=(VMenu *)CurItem->ObjPtr;

      if(ListBox)
      {
        // удалим все итемы
        ListBox->DeleteItems();

        struct MenuItem ListItem;
        /* $ 13.09.2000 SVS
           + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX &
             DI_COMBOBOX выставляется флаг MENU_SHOWAMPERSAND. Этот флаг
             подавляет такое поведение
        */
        if(!(CurItem->Flags&DIF_LISTNOAMPERSAND))
          ListBox->SetFlags(MENU_SHOWAMPERSAND);
        /* SVS $*/
        ListBox->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                             X1+CurItem->X2,Y1+CurItem->Y2);
        ListBox->SetBoxType(SHORT_SINGLE_BOX);

        struct FarList *List=CurItem->ListItems;
        if(List && List->Items)
        {
          struct FarListItem *Items=List->Items;
          for (J=0; J < List->ItemsNumber; J++)
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
      if (!CheckDialogMode(DMODE_CREATEOBJECTS))
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
      /* $ 18.09.2000 SVS
         ReadOnly!
      */
      if (CurItem->Flags & DIF_READONLY)
      {
         DialogEdit->ReadOnly=1;
      }
      /* SVS $ */
      /* $ 15.10.2000 tran
        строка редакторирование должна иметь максимум в 511 символов */
      DialogEdit->SetMaxLength(511);
      /* tran $ */
      DialogEdit->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                              X1+CurItem->X2,Y1+CurItem->Y2);
      DialogEdit->SetObjectColor(
         FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGEDIT:COL_DIALOGEDIT),
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
        /* $ 21.08.2000 SVS
           DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
        */
        if(CurItem->Flags&DIF_HISTORY)
          CurItem->Flags&=~DIF_MASKEDIT;
        /* SVS $ */
        // если DI_FIXEDIT, то курсор сразу ставится на замену...
        //   ай-ай - было недокументированно :-)
        DialogEdit->SetMaxLength(CurItem->X2-CurItem->X1+1);
        DialogEdit->SetOvertypeMode(TRUE);
        /* $ 12.08.2000 KM
           Если тип строки ввода DI_FIXEDIT и установлен флаг DIF_MASKEDIT
           и непустой параметр CurItem->Mask, то вызываем новую функцию
           для установки маски в объект Edit.
        */
        /* $ 18.09.2000 SVS
          Маска не должна быть пустой (строка из пробелов не учитывается)!
        */
        if ((CurItem->Flags & DIF_MASKEDIT) && CurItem->Mask)
        {
          char *Ptr=CurItem->Mask;
          while(*Ptr && *Ptr == ' ') ++Ptr;
          if(*Ptr)
            DialogEdit->SetInputMask(CurItem->Mask);
          else
          {
            CurItem->Mask=NULL;
            CurItem->Flags&=~DIF_MASKEDIT;
          }
        }
        /* SVS $ */
        /* KM $ */
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
        struct FarListItem *ListItems=CurItem->ListItems->Items;
        int Length=CurItem->ListItems->ItemsNumber;

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

  // все объекты созданы!
  SetDialogMode(DMODE_CREATEOBJECTS);
  return I;
}
/* 24.08.2000 SVS $ */


//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
/* $ 22.08.2000 SVS
  ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
*/
/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog(int ID)
{
  struct DialogItem *CurItem;
  int X,Y;
  int I,DrawItemCount;
  unsigned long Attr;

  /* $ 18.08.2000 SVS
     Если не разрешена отрисовка, то вываливаем.
  */
  if(IsEnableRedraw ||                 // разрешена прорисовка ?
     (ID+1 > ItemCount) ||             // а номер в рамках дозволенного?
     CheckDialogMode(DMODE_DRAWING))   // диалог рисуется?
    return;
  /* SVS $ */

  SetDialogMode(DMODE_DRAWING);  // диалог рисуется!!!

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  if(ID == -1) // рисуем все?
  {
    /* $ 28.07.2000 SVS
       Перед прорисовкой диалога посылаем сообщение в обработчик
    */
      DlgProc((HANDLE)this,DN_DRAWDIALOG,0,0);
    /* SVS $ */

    /* $ 28.07.2000 SVS
       перед прорисовкой подложки окна диалога...
    */
    Attr=DlgProc((HANDLE)this,DN_CTLCOLORDIALOG,0,CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
    SetScreen(X1,Y1,X2,Y2,' ',Attr);
    /* SVS $ */
  }

  if(ID == -1) // рисуем все?
  {
    ID=0;
    DrawItemCount=ItemCount;
  }
  else
  {
    DrawItemCount=ID+1;
  }

  for (I=ID; I < DrawItemCount; I++)
  {
    CurItem=&Item[I];

    /* $ 28.07.2000 SVS
       Перед прорисовкой каждого элемента посылаем сообщение
       посредством функции SendDlgMessage - в ней делается все!
    */
    Dialog::SendDlgMessage((HANDLE)this,DN_DRAWDLGITEM,I,0);
    /* SVS $ */
    /* $ 28.07.2000 SVS
       перед прорисовкой каждого элемента диалога выясним атритубы отрисовки
    */
    switch(CurItem->Type)
    {
/* ***************************************************************** */
      case DI_USERCONTROL:
        if(CurItem->VBuf)
        {
          PutText(X1+CurItem->X1,Y1+CurItem->Y1,X1+CurItem->X2,Y1+CurItem->Y2,CurItem->VBuf);
        }
        break; //уже наприсовали :-)))

/* ***************************************************************** */
      case DI_SINGLEBOX:
      case DI_DOUBLEBOX:
      {
        Attr=MAKELONG(
          MAKEWORD(FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGBOXTITLE:COL_DIALOGBOXTITLE), // Title LOBYTE
                 FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)),// HiText HIBYTE
          MAKEWORD(FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGBOX:COL_DIALOGBOX), // Box LOBYTE
                 0)                                               // HIBYTE
        );
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

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
      }

/* ***************************************************************** */
      case DI_TEXT:
      {
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
            Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGBOX:COL_DIALOGBOX;
          else
            Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT;

        Attr=MAKELONG(
           MAKEWORD(FarColorToReal(Attr),
                   FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)), // HIBYTE HiText
             0);
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);
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
      }

/* ***************************************************************** */
      case DI_VTEXT:
      {
        if (CurItem->Flags & DIF_BOXCOLOR)
          Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGBOX:COL_DIALOGBOX;
        else
          if (CurItem->Flags & DIF_SETCOLOR)
            Attr=(CurItem->Flags & DIF_COLORMASK);
          else
            Attr=(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,FarColorToReal(Attr));
        SetColor(Attr&0xFF);
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);
        VText(CurItem->Data);
        break;
      }

/* ***************************************************************** */
      /* $ 18.07.2000 SVS
         + обработка элемента DI_COMBOBOX
      */
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
      case DI_COMBOBOX:
      {
        Edit *EditPtr=(Edit *)(CurItem->ObjPtr);

        /* $ 15.08.2000 SVS
           ! Для DropDownList цвета обрабатываем по иному
        */
        /* $ 30.08.2000 SVS
           ! "Цвета, видите ли, ему не понравились" :-)
        */
        Attr=EditPtr->GetObjectColor();
        if(CurItem->Type == DI_COMBOBOX && (CurItem->Flags & DIF_DROPDOWNLIST))
        {
          DWORD AAA=Attr&0xFF;
          Attr=MAKEWORD(FarColorToReal(AAA),
                 FarColorToReal((!CurItem->Focus)?COL_DIALOGEDIT:COL_DIALOGEDITSELECTED));
          Attr=MAKELONG(Attr, // EditLine (Lo=Color, Hi=Selected)
            MAKEWORD(FarColorToReal(AAA), // EditLine - UnChanched Color
            FarColorToReal(COL_DIALOGTEXT) // HistoryLetter
           ));
        }
        else
        {
          Attr=MAKEWORD(FarColorToReal(Attr&0xFF),FarColorToReal(COL_DIALOGEDITSELECTED));
          Attr=MAKELONG(Attr, // EditLine (Lo=Color, Hi=Selected)
             MAKEWORD(FarColorToReal(EditPtr->GetObjectColorUnChanged()), // EditLine - UnChanched Color
             FarColorToReal(COL_DIALOGTEXT) // HistoryLetter
             ));
        }
        /* SVS $ */
        /* SVS $ */
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

        EditPtr->SetObjectColor(Attr&0xFF,HIBYTE(LOWORD(Attr)),LOBYTE(HIWORD(Attr)));

        if (CurItem->Focus)
        {
          /* $ 09.08.2000 KM
             Отключение мигающего курсора при перемещении диалога
          */
          if (!CheckDialogMode(DMODE_DRAGGED))
            SetCursorType(1,-1);
          EditPtr->Show();
          /* KM $ */
          SelectOnEntry(I);
        }
        else
          EditPtr->FastShow();

        /* $ 09.08.2000 KM
           Отключение мигающего курсора при перемещении диалога
        */
        if (CheckDialogMode(DMODE_DRAGGED))
          SetCursorType(FALSE,0);
        /* KM $ */

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

/* ***************************************************************** */
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      case DI_LISTBOX:
      {
        VMenu *ListBox=(VMenu *)(CurItem->ObjPtr);
        if(ListBox)
        {
          /* $ 21.08.2000 SVS
             Перед отрисовкой спросим об изменении цветовых атрибутов
          */
          short Colors[9];
          ListBox->GetColors(Colors);
          if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
                          sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
            ListBox->SetColors(Colors);
          /* SVS $ */
          if (CurItem->Focus)
            ListBox->Show();
          else
            ListBox->FastShow();
        }
        break;
      }
      /* 01.08.2000 SVS $ */

/* ***************************************************************** */
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
      {
        if (CurItem->Flags & DIF_SETCOLOR)
          Attr=(CurItem->Flags & DIF_COLORMASK);
        else
          Attr=(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

        Attr=MAKEWORD(FarColorToReal(Attr),
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGHIGHLIGHTTEXT:COL_DIALOGHIGHLIGHTTEXT)); // HiText
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

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
          /* $ 09.08.2000 KM
             Отключение мигающего курсора при перемещении диалога
          */
          if (!CheckDialogMode(DMODE_DRAGGED))
            SetCursorType(1,-1);
          MoveCursor(X1+CurItem->X1+1,Y1+CurItem->Y1);
          /* KM $ */
        }

        break;
      }

/* ***************************************************************** */
      case DI_BUTTON:
      {
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);

        /* $ 18.08.2000 SVS
           + DI_BUTTON тоже теперь может иметь DIF_SETCOLOR
        */
        if (CurItem->Focus)
        {
          SetCursorType(0,10);
          Attr=MAKEWORD(
             (CurItem->Flags & DIF_SETCOLOR)?(CurItem->Flags & DIF_COLORMASK):
               FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGSELECTEDBUTTON:COL_DIALOGSELECTEDBUTTON), // TEXT
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON:COL_DIALOGHIGHLIGHTSELECTEDBUTTON)); // HiText
        }
        else
        {
          Attr=MAKEWORD(
             (CurItem->Flags & DIF_SETCOLOR)?(CurItem->Flags & DIF_COLORMASK):
               FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGBUTTON:COL_DIALOGBUTTON), // TEXT
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGHIGHLIGHTBUTTON:COL_DIALOGHIGHLIGHTBUTTON)); // HiText
        }
        /* SVS $ */
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);
        SetColor(Attr&0xFF);
        HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));
        break;
      }

/* ***************************************************************** */
    } // end switch(...
    /* 28.07.2000 SVS $ */
  } // end for (I=...

  /* $ 31.07.2000 SVS
     Включим индикатор перемещения...
  */
  if ( CheckDialogMode(DMODE_DRAGGED) ) // если диалог таскается
  {
    /* $ 03.08.2000 tran
       вывод текста в углу может приводить к ошибкам изображения
       1) когда диалог перемещается в угол
       2) когда диалог перемещается из угла
       сделал вывод красных палочек по углам */
    //Text(0,0,0xCE,"Move");
    Text(X1,Y1,0xCE,"\\");
    Text(X1,Y2,0xCE,"/");
    Text(X2,Y1,0xCE,"/");
    Text(X2,Y2,0xCE,"\\");
  }
  /* SVS $ */


  SkipDialogMode(DMODE_DRAWING);  // конец отрисовки диалога!!!
  SetDialogMode(DMODE_SHOW); // диалог на экране!
}
/* SVS 22.08.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от клавиатуры.
   Перекрывает BaseInput::ProcessKey.
*/
int Dialog::ProcessKey(int Key)
{
  int I,J;
  char Str[1024];
  char *PtrStr;
  Edit *CurEditLine;

  /* $ 31.07.2000 tran
     + перемещение диалога по экрану */
  if (CheckDialogMode(DMODE_DRAGGED)) // если диалог таскается
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
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
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
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
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
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
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
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_ENTER:
        case KEY_CTRLF5:
            SkipDialogMode(DMODE_DRAGGED); // закончим движение!
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_ESC:
            Hide();
            AdjustEditPos(OldX1-X1,OldY1-Y1);
            X1=OldX1;
            X2=OldX2;
            Y1=OldY1;
            Y2=OldY2;
            SkipDialogMode(DMODE_DRAGGED);
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
    }
    if(CheckDialogMode(DMODE_ALTDRAGGED))
    {
      SkipDialogMode(DMODE_DRAGGED|DMODE_ALTDRAGGED);
      Show();
    }
    return (TRUE);
  }
  /* $ 10.08.2000 SVS
     Двигаем, если разрешено! (IsCanMove)
  */
  if (Key == KEY_CTRLF5 && CheckDialogMode(DMODE_ISCANMOVE))
  /* SVS 10.08.2000 $*/
  {
    // включаем флаг и запоминаем координаты
    SetDialogMode(DMODE_DRAGGED);
    OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
    //# GetText(0,0,3,0,LV);
    Show();
    return (TRUE);
  }
  /* tran 31.07.2000 $ */

  if (Key==KEY_NONE || Key==KEY_IDLE)
  {
    /* $ 28.07.2000 SVS
       Передадим этот факт в обработчик :-)
    */
    DlgProc((HANDLE)this,DN_ENTERIDLE,0,0);
    /* SVS $ */
    return(FALSE);
  }

  // "ХАчу глянуть на то, что под диалогом..."
  if(Key == KEY_CTRLALTSHIFTPRESS && CheckDialogMode(DMODE_SHOW))
  {
    if(Opt.AllCtrlAltShiftRule & CASR_DIALOG)
    {
      Hide();
      WaitKey(KEY_CTRLALTSHIFTRELEASE);
      Show();
    }
    return(TRUE);
  }

  int Type=Item[FocusPos].Type;

  if(!CheckDialogMode(DMODE_KEY))
    DlgProc((HANDLE)this,DM_KEY,FocusPos,Key);

  // небольшая оптимизация
  if(Type==DI_CHECKBOX)
  {
    if((Key == KEY_ADD      && !Item[FocusPos].Selected) ||
       (Key == KEY_SUBTRACT &&  Item[FocusPos].Selected))
    Key=KEY_SPACE;
  }
  else if(Key == KEY_ADD)
    Key='+';
  else if(Key == KEY_SUBTRACT)
    Key='-';

  switch(Key)
  {
    case KEY_F1:
      /* $ 28.07.2000 SVS
         Перед выводом диалога посылаем сообщение в обработчик
         и если вернули что надо, то выводим подсказку
      */
      PtrStr=(char*)DlgProc((HANDLE)this,DN_HELP,FocusPos,(long)&HelpTopic[0]);
      if(PtrStr && *PtrStr)
      {
        /* $ 31.08.2000 SVS
           - Бага с вызовом файлов помощи.
        */
        if(PluginNumber != -1)
        {
          /* $ 29.08.2000 SVS
             ! При подмене темы помощи из диаловой процедуры...
               короче, нужно вновь формировать контент!
          */
          if (*PtrStr==':')       // Main Topic?
            strcpy(Str,PtrStr+1);
          else if (*PtrStr=='#')  // уже сформировано?
            strcpy(Str,PtrStr);
          else                    // надо формировать...
          {
            strcpy(&Str[512],CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
            *PointToName(&Str[512])=0;
            sprintf(Str,"#%s#%s",&Str[512],PtrStr);
          }
          /* SVS $ */
        }
        else
          strcpy(Str,PtrStr);

        SetHelp(Str);
        /* SVS $ */
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
      if(!CheckDialogMode(DMODE_OLDSTYLE))
      {
        EndLoop=FALSE; // только если есть
        return TRUE; // делать больше не чего
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
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,I-1,0);
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,I,0);
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
      else if (Type==DI_BUTTON)
      {
        /* $ 21.08.2000 SVS
           Нет срабатывания, если давим на кнопку
        */
        Item[FocusPos].Selected=1;
        // сообщение - "Кнокна кликнута"
        Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,0);
        /* $ 18.08.2000 SVS
           + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
        */
        if((Item[FocusPos].Flags&DIF_BTNNOCLOSE))
        {
//          ShowDialog(); //???
          return(TRUE);
        }
        /* SVS $ */
        /* SVS 21.08.2000 $ */
        /* $ 12.09.2000 SVS
          ! Задаем поведение для кнопки с DefaultButton=1
        */
        if(CheckDialogMode(DMODE_OLDSTYLE) || Item[FocusPos].DefaultButton)
        {
          ExitCode=FocusPos;
          EndLoop=TRUE;
        }
        /* SVS $ */
      }
      else if(IsEdit(Type) || CheckDialogMode(DMODE_OLDSTYLE))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            if (!IsEdit(Item[I].Type))
              Item[I].Selected=1;
            ExitCode=I;
          }

        EndLoop=TRUE;
        if (ExitCode==-1)
          ExitCode=FocusPos;
      }
      return(TRUE);

    case KEY_ESC:
    case KEY_BREAK:
    case KEY_F10:
      EndLoop=TRUE;
      ExitCode=(Key==KEY_BREAK) ? -2:-1;
      return(TRUE);
/*
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
*/
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
        if(!Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,Item[FocusPos].Selected))
          Item[FocusPos].Selected =! Item[FocusPos].Selected;
        /* SVS $ */
        ShowDialog();
        return(TRUE);
      }
      if (Type==DI_RADIOBUTTON)
      {
        int PrevRB;
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
          {
            PrevRB=I;
          }
          ++I;
          /* SVS $ */
        } while (I<ItemCount && Item[I].Type==DI_RADIOBUTTON &&
                 (Item[I].Flags & DIF_GROUP)==0);

        Item[FocusPos].Selected=1;
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
          посредством функции SendDlgMessage - в ней делается все!
        */
        if(!Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,PrevRB))
        {
           // вернем назад, если пользователь не захотел...
           Item[PrevRB].Selected=1;
           Item[FocusPos].Selected=0;
        }
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
        Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
        /* SVS $ */
        return(TRUE);
      }
      return(TRUE);

    case KEY_HOME:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

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
          //Dialog::SendDlgMessage((HANDLE)this,DN_CHANGEITEM,FocusPos,0);
          //Dialog::SendDlgMessage((HANDLE)this,DN_CHANGEITEM,I,0);
          /* SVS $ */
          ShowDialog();
          return(TRUE);
        }
      return(TRUE);

    case KEY_LEFT:
    case KEY_RIGHT:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

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
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

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
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
    case KEY_PGDN:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

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
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

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
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */

      /* $ 21.08.2000 SVS
         Autocomplete при постоянных блоках и немного оптимизации ;-)
      */
      if (IsEdit(Type))
      {
        Edit *edt=(Edit *)Item[FocusPos].ObjPtr;
        int SelStart, SelEnd;

        /* $ 11.09.2000 SVS
           Ctrl-U в строках ввода снимает пометку блока
        */
        if(Key == KEY_CTRLU)
        {
          edt->SetClearFlag(0);
          edt->Select(-1,0);
          edt->Show();
          return TRUE;
        }
        /* SVS $ */

        if (Item[FocusPos].Flags & DIF_EDITOR)
          switch(Key)
          {
            /* $ 12.09.2000 SVS
              Исправляем ситуацию с BackSpace в DIF_EDITOR
            */
            case KEY_BS:
            {
              int CurPos=edt->GetCurPos();
              if(!edt->GetCurPos())
              {
                if(FocusPos > 0 &&
                   (Item[FocusPos-1].Flags&DIF_EDITOR))
                {
                  Edit *edt_1=(Edit *)Item[FocusPos-1].ObjPtr;
                  edt_1->GetString(Str,sizeof(Str));
                  CurPos=strlen(Str);
                  edt->GetString(Str+CurPos,sizeof(Str)-CurPos);
                  edt_1->SetString(Str);
                  for (I=FocusPos+1;I<ItemCount;I++)
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
                   ProcessKey(KEY_UP);
                   edt_1->SetCurPos(CurPos);
                }
              }
              else
              {
                edt->ProcessKey(Key);
              }
              Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
              ShowDialog();
              return(TRUE);
            }
            /* SVS $ */

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
              Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
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
                  Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
                  /* SVS $ */
                  ShowDialog();
                  return(TRUE);
                }
                else if (CurPos>=Length)
                {
                  Edit *edt_1=(Edit *)Item[FocusPos+1].ObjPtr;
                  /* $ 12.09.2000 SVS
                     Решаем проблему, если Del нажали в позиции
                     большей, чем длина строки
                  */
                  if (CurPos > Length)
                  {
                    LengthStr=CurPos;
                    memset(Str+Length,' ',CurPos-Length);
                  }
                  /* SVS $*/
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

        /* $ 24.09.2000 SVS
           Вызов функции Xlat
        */
        if(Opt.XLatDialogKey && Key == Opt.XLatDialogKey)
        {
          edt->Xlat();
          return TRUE;
        }
        /* SVS $ */

        if (edt->ProcessKey(Key))
        {
          /* $ 26.07.2000 SVS
             AutoComplite: Если установлен DIF_HISTORY
                 и разрешено автозавершение!.
          */
          if(Opt.AutoComplete && Key < 256 && Key != KEY_BS && Key != KEY_DEL &&
             ((Item[FocusPos].Flags & DIF_HISTORY) || Type == DI_COMBOBOX)
            )
          {
            /* $ 01.08.2000 SVS
               Небольшой глючек с AutoComplete
            */
            int CurPos=edt->GetCurPos();
            /* SVS $*/
            //text to search for
            edt->GetString(Str,sizeof(Str));
            edt->GetSelection(SelStart,SelEnd);
            if(SelStart <= 0)
              SelStart=sizeof(Str);
            else
              SelStart++;

            // а вот при постоянных блоках остатка строки нам ненать
            //  даже если блок помечен в серёдке... :-)
            if(Opt.EditorPersistentBlocks)
            {
              // ненать по возможности :-)
              if(CurPos <= SelEnd)
              {
                Str[CurPos]=0;
                edt->Select(CurPos,sizeof(Str)); //select the appropriate text
                edt->DeleteBlock();
                edt->FastShow();
              }
            }
            SelEnd=strlen(Str);
            //find the string in the list
            if (FindInEditForAC(Type == DI_COMBOBOX,
                         (void *)Item[FocusPos].Selected,Str))
            {
              edt->SetString(Str);
              edt->Select(SelEnd,sizeof(Str)); //select the appropriate text
              /* $ 01.08.2000 SVS
                 Небольшой глючек с AutoComplete
              */
              edt->SetCurPos(CurPos); // SelEnd
              /* SVS $*/
              /* $ 28.07.2000 SVS
                При изменении состояния каждого элемента посылаем сообщение
                посредством функции SendDlgMessage - в ней делается все!
              */
              Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
              /* SVS $ */
              Redraw();
            }
          }
          /* SVS $ */
          return(TRUE);
        }
        /* SVS 21.08.2000 $ */
      }

      if (ProcessHighlighting(Key,FocusPos,FALSE))
        return(TRUE);

      return(ProcessHighlighting(Key,FocusPos,TRUE));
  }
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от "мыши".
   Перекрывает BaseInput::ProcessMouse.
*/
/* $ 18.08.2000 SVS
   + DN_MOUSECLICK
*/
int Dialog::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int I, J;
  int MsX,MsY;
  int Type;

  if (MouseEvent->dwButtonState==0)
    return(FALSE);
  if(!CheckDialogMode(DMODE_SHOW))
    return FALSE;

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  if (MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2)
  {
    DlgProc((HANDLE)this,DN_MOUSECLICK,-1,(long)MouseEvent);
    /* $ 09.09.2000 SVS
       Учтем DMODE_OLDSTYLE - вывалить из диалога, ткнув вне диалога
       сможем только в старом стиле.
    */
    if(CheckDialogMode(DMODE_OLDSTYLE))
    {
      if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        ProcessKey(KEY_ESC);
      else
        ProcessKey(KEY_ENTER);
    }
    else
      MessageBeep(MB_ICONHAND);
    /* SVS $ */
    return(TRUE);
  }

  if (MouseEvent->dwEventFlags==0)
  {
    /* $ 21.08.2000 SVS
       DN_MOUSECLICK - первично.
    */
    for (I=0;I<ItemCount;I++)
    {
      int IX1=Item[I].X1+X1,
          IY1=Item[I].Y1+Y1,
          IX2=Item[I].X2+X1,
          IY2=Item[I].Y2+Y1;
      BOOL Send_DN=TRUE;

      if(MsX >= IX1 && MsY >= IY1 && MsY <= IY2 && MsX <= IX2)
      {
        switch(Item[I].Type)
        {
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
            if(((MsX == IX1 || MsX == IX2) && MsY >= IY1 && MsY <= IY2) || // vert
               ((MsY == IY1 || MsY == IY2) && MsX >= IX1 && MsX <= IX2) )   // hor
                break;
            Send_DN=FALSE;
            break;
          case DI_USERCONTROL:
            // для user-типа подготовим координаты мыши
            MouseEvent->dwMousePosition.X-=IX1;
            MouseEvent->dwMousePosition.Y-=IY1;
            break;
        }
        if(Send_DN)
          if((J=DlgProc((HANDLE)this,DN_MOUSECLICK,I,(long)MouseEvent)))
            return TRUE;

        if(Item[I].Type == DI_USERCONTROL)
        {
           ChangeFocus2(FocusPos,I);
           ShowDialog();
           return(TRUE);
        }
      }
    }
    /* SVS $ */

    if((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
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
            /* $ 15.08.2000 SVS
               + Сделаем так, чтобы ткнув мышкой в DropDownList
                 список раскрывался сам.
               Есть некоторая глюкавость - когда список раскрыт и мы
               мышой переваливаем на другой элемент, то список закрывается
               но перехода реального на указанный элемент диалога не происходит
            */
            int EditX1,EditY1,EditX2,EditY2;
            Edit *EditLine=(Edit *)(Item[I].ObjPtr);
            EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);

            if(MsY==EditY1 && Type == DI_COMBOBOX &&
               (Item[I].Flags & DIF_DROPDOWNLIST) &&
               MsX >= EditX1 && MsX <= EditX2+1)
            {
              EditLine->SetClearFlag(0);
              ChangeFocus2(FocusPos,I);
              ShowDialog();
              ProcessKey(KEY_CTRLDOWN);
              return(TRUE);
            }
            /* SVS $ */

            if (EditLine->ProcessMouse(MouseEvent))
            {
              EditLine->SetClearFlag(0);
              ChangeFocus2(FocusPos,I);
              ShowDialog();
              return(TRUE);
            }
            else
            {
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
      } // for (I=0;I<ItemCount;I++)
      // ДЛЯ MOUSE-Перемещалки:
      //   Сюда попадаем в том случае, если мышь не попала на активные элементы
      //
      /* $ 10.08.2000 SVS
         Двигаем, если разрешено! (IsCanMove)
      */
      if (CheckDialogMode(DMODE_ISCANMOVE))
      {
        /* $ 03.08.2000 tran
           ну раз попадаем - то будем перемещать */
        //SetDialogMode(DMODE_DRAGGED);
        OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
        MsX=MouseX;
        MsY=MouseY;
        /* $ 11.08.2000 KM
           Переменная, индицирующая первое попадание в процедуру
           перемещения диалога.
        */
        static int First=TRUE;
        /* KM $ */
        while (1)
        {
            int mb=IsMouseButtonPressed();
            /* $ 09.08.2000 tran
               - долой "салазки" :) */
            /* $ 11.08.2000 KM
               Если первый раз пришло сообщение о нажатой кнопке мыши,
               то естественно мы проходим в процедуру перемещения, чтобы
               корректно отобразить включение этого режима сразу же, а
               не после начала движения.
            */
            if ( mb==1 && MouseX==MsX && MouseY==MsY && !First )
                continue;
            MsX=MouseX;
            MsY=MouseY;
            /* KM $ */
            /* tran 09.08.2000 $ */

            int mx,my;
//            SysLog("MouseMove:(), MouseX=%i, MousePrevX=%i,MouseY=%i, MousePrevY=%i",MouseX,PrevMouseX,MouseY,PrevMouseY);
            if ( mb==1 ) // left key, still dragging
            {
                Hide();
                /* 11.08.2000 KM
                   Артефакт: если после первого запуска фара, не пермещая мышь
                   нажать левую кнопку, чтобы переместить диалог, то из-за
                   того, что к этому моменту PrevMouseX и PrevMouseY ещё не определены,
                   также неопределённым получался прыжок диалога. Поэтому по первому
                   входу в режим перемещения инициализируем переменные 0.
                */
                if (First)
                  mx=my=0;
                else
                {
                  mx=MouseX-PrevMouseX;
                  my=MouseY-PrevMouseY;
                }
                /* KM $ */
                if ( X1+mx>=0 && X2+mx<=ScrX )
                {
                    X1+=mx;
                    X2+=mx;
                    AdjustEditPos(mx,0);
                }
                if ( Y1+my>=0 && Y2+my<=ScrY )
                {
                    Y1+=my;
                    Y2+=my;
                    AdjustEditPos(0,my);
                }
                Show();
            }
            else if (mb==2) // right key, abort
            {
                Hide();
                AdjustEditPos(OldX1-X1,OldY1-Y1);
                X1=OldX1;
                X2=OldX2;
                Y1=OldY1;
                Y2=OldY2;
                /* $ 11.08.2000 KM
                   При выходе из режима перемещения подготовим
                   переменную для последующего корректного входа.
                */
                First=TRUE;
                /* KM $ */
                SkipDialogMode(DMODE_DRAGGED);
                Show();
                break;
            }
            else  // release key, drop dialog
            {
                /* $ 11.08.2000 KM
                   При выходе из режима перемещения подготовим
                   переменную для последующего корректного входа.
                */
                First=TRUE;
                /* KM $ */
                SkipDialogMode(DMODE_DRAGGED);
                Show();
                break;
            }
            /* $ 11.08.2000 KM
               Всё хватит, первый раз мы уже заходили...
            */
            First=FALSE;
            /* KM $ */
        }// while (1)
        /* tran 03.08.2000 $ */
      }
      /* SVS 10.08.2000 $*/
    }
  }
  return(FALSE);
}
/* SVS 18.08.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* Private:
   Изменяет фокус ввода (воздействие клавишами
     KEY_TAB, KEY_SHIFTTAB, KEY_UP, KEY_DOWN,
   а так же Alt-HotKey)
*/
/* $ 28.07.2000 SVS
   Довесок для сообщений DN_KILLFOCUS & DN_SETFOCUS
*/
/* $ 24.08.2000 SVS
   Добавка для DI_USERCONTROL
*/
int Dialog::ChangeFocus(int FocusPos,int Step,int SkipGroup)
{
  int Type,OrigFocusPos=FocusPos;
//  int FucusPosNeed=-1;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент - LostFocus() - теряет фокус ввода.
//  if(CheckDialogMode(DMODE_INITOBJECTS))
//    FucusPosNeed=DlgProc((HANDLE)this,DN_KILLFOCUS,FocusPos,0);
//  if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
//    FocusPos=FucusPosNeed;
//  else
  {
    while (1)
    {
      FocusPos+=Step;
      if (FocusPos>=ItemCount)
        FocusPos=0;
      if (FocusPos<0)
        FocusPos=ItemCount-1;

      Type=Item[FocusPos].Type;

      if(!(Item[FocusPos].Flags&DIF_NOFOCUS))
      {
        if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
          break;
        if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[FocusPos].Selected))
          break;
      }
      // убираем зацикливание с последующим подвисанием :-)
      if(OrigFocusPos == FocusPos)
        break;
    }
  }

//  Dialog::FocusPos=FocusPos;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент GotFocus() - получил фокус ввода.
  // Игнорируем возвращаемое функцией диалога значение
//  if(CheckDialogMode(DMODE_INITOBJECTS))
//    DlgProc((HANDLE)this,DN_GOTFOCUS,FocusPos,0);
  return(FocusPos);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DN_KILLFOCUS & DM_SETFOCUS
*/
int Dialog::ChangeFocus2(int KillFocusPos,int SetFocusPos)
{
  int FucusPosNeed=-1;
  if(!(Item[SetFocusPos].Flags&DIF_NOFOCUS))
  {
    if(CheckDialogMode(DMODE_INITOBJECTS))
      FucusPosNeed=DlgProc((HANDLE)this,DN_KILLFOCUS,KillFocusPos,0);

    if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
      SetFocusPos=FucusPosNeed;

    if(Item[SetFocusPos].Flags&DIF_NOFOCUS)
       SetFocusPos=KillFocusPos;

    Item[KillFocusPos].Focus=0;
    Item[SetFocusPos].Focus=1;

    Dialog::PrevFocusPos=Dialog::FocusPos;
    Dialog::FocusPos=SetFocusPos;
    if(CheckDialogMode(DMODE_INITOBJECTS))
      DlgProc((HANDLE)this,DN_GOTFOCUS,SetFocusPos,0);
  }
  else
    SetFocusPos=KillFocusPos;

  return(SetFocusPos);
}
/* SVS $ */

/* $ 08.09.2000 SVS
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(int Pos)
{
  if(IsEdit(Item[Pos].Type) &&
     (Item[Pos].Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
    )
  {
    Edit *edt=(Edit *)Item[Pos].ObjPtr;
    edt->Select(0,edt->GetLength());
  }
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
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
  if(FromPlugin == CVTITEM_TOPLUGIN)
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

//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
/* $ 24.08.2000 SVS
   Добавка для DI_USERCONTROL
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
         Type==DI_LISTBOX ||
         Type==DI_USERCONTROL);
}
/* 24.08.2000 SVS $ */
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
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
    struct FarListItem *ListItems=((struct FarList *)HistoryName)->Items;
    int Count=((struct FarList *)HistoryName)->ItemsNumber;

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


//////////////////////////////////////////////////////////////////////////
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
    if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
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


//////////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////////
/* Public, Static:
   Проверка на HotKey
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


//////////////////////////////////////////////////////////////////////////
/* Private:
   Если жмакнули Alt-???
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
        if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
          break; // сказали не продолжать обработку...
        /* SVS $ */

        //???? здесь проверить по поводу DI_COMBOBOX!!!
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
        /* $ 29.08.2000 SVS
           - Первый официальный альфа-баг - функция ProcessHighlighting
           MY> Работа с диалогами стала ГЛЮЧНАЯ. Я имею в виду горячие клавиши.
           MY> Входим в настройку чего угодно, жмем Alt-нужную букву и
           MY> наблюдаем разнообразные глюки.

           А ларчик просто открывался :-)))
        */
        ChangeFocus2(FocusPos,I);
        /* SVS $ */
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
        // при ComboBox`е - "вываливаем" последний //????
        if (Item[I].Type==DI_COMBOBOX)
        {
          ProcessKey(KEY_CTRLDOWN);
          return(TRUE);
        }
        ShowDialog();
        return(TRUE);
      }
  }
  return(FALSE);
}


//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список для ComboBox
*/
/*
   $ 18.07.2000 SVS
   Функция-обработчик выбора из списка и установки...
*/
void Dialog::SelectFromComboBox(
         Edit *EditLine,                   // строка редактирования
         struct FarList *List,    // список строк
         char *IStr)
{
  char Str[512];
  struct MenuItem ComboBoxItem;
  struct FarListItem *ListItems=List->Items;
  int EditX1,EditY1,EditX2,EditY2;
  int I,Dest;

  // создание пустого вертикального меню
  //  с обязательным показом ScrollBar
  VMenu ComboBoxMenu("",NULL,0,8,VMENU_ALWAYSSCROLLBAR,NULL/*,this*/);

  EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
  if (EditX2-EditX1<20)
    EditX2=EditX1+20;
  if (EditX2>ScrX)
    EditX2=ScrX;
#if 0
  if(!(Item[FocusPos].Flags&DIF_LISTNOAMPERSAND))
#endif
    ComboBoxMenu.SetFlags(MENU_SHOWAMPERSAND);
  ComboBoxMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
  ComboBoxMenu.SetBoxType(SHORT_SINGLE_BOX);

  // заполнение пунктов меню
  /* Последний пункт списка - ограничититель - в нем Tetx[0]
     должен быть равен '\0'
  */
  for (Dest=I=0;I < List->ItemsNumber;I++)
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
  if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
                  sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
    ComboBoxMenu.SetColors(Colors);
  /* SVS $ */

  ComboBoxMenu.Show();
//  Dest=ComboBoxMenu.GetSelectPos();
  while (!ComboBoxMenu.Done())
  {
    int Key=ComboBoxMenu.ReadInput();
    // здесь можно добавить что-то свое, например,
//    I=ComboBoxMenu.GetSelectPos();
//    if(I != Dest)
//    {
//      Dest=I;
//      DlgProc((HANDLE)this,DN_LISTCHANGE,,(long)List);
//    }
    //  обработку multiselect ComboBox
    ComboBoxMenu.ProcessInput();
  }

  int ExitCode=ComboBoxMenu.GetExitCode();
  if (ExitCode<0)
    return;
  /* Запомним текущее состояние */
  for (I=0; I < List->ItemsNumber; I++)
    ListItems[I].Flags&=~LIF_SELECTED;
  ListItems[ExitCode].Flags|=LIF_SELECTED;
  ComboBoxMenu.GetUserData(Str,sizeof(Str),ExitCode);

  EditLine->SetString(Str);
  EditLine->SetLeftPos(0);
  Redraw();
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 31.07.2000 tran
   + функция подравнивания координат edit классов */
/* $ 07.08.2000 SVS
   + а про ListBox забыли?*/
void Dialog::AdjustEditPos(int dx, int dy)
{
  struct DialogItem *CurItem;
  int I;
  int x1,x2,y1,y2;

  ScreenObject *DialogEdit;
  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];
    if (IsEdit(CurItem->Type) || CurItem->Type == DI_LISTBOX)
    {
       DialogEdit=(ScreenObject *)CurItem->ObjPtr;
       DialogEdit->GetPosition(x1,y1,x2,y2);
       x1+=dx;
       x2+=dx;
       y1+=dy;
       y2+=dy;
       DialogEdit->SetPosition(x1,y1,x2,y2);
    }
  }
}
/* SVS $ */
/* tran 31.07.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* $ 11.08.2000 SVS
   Работа с доп. данными экземпляра диалога
   Пока простое копирование (присвоение)
*/
void Dialog::SetDialogData(long NewDataDialog)
{
  DataDialog=NewDataDialog;
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////
/* $ 11.08.2000 SVS
   + Для того, чтобы послать DM_CLOSE нужно переопределить Process
*/
void Dialog::Process()
{
  do{
    Modal::Process();
    if(DlgProc((HANDLE)this,DM_CLOSE,ExitCode,0) || ExitCode == -2)
      break;
    /* $ 18.08.2000 SVS
       Вах-вах, а сбросить-то флаг забыли 8-=(((
    */
    ClearDone();
    /* SVS $ */
  }while(1);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
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
    case DN_INITDIALOG:
      return FALSE; // изменений не было!

    case DM_CLOSE:
      return TRUE;  // согласен с закрытием

    case DN_KILLFOCUS:
      return -1;    // "Согласен с потерей фокуса"

    case DN_GOTFOCUS:
      return 0;     // always 0

    case DN_HELP:
      return Param2; // что передали, то и...

    case DN_DRAWDIALOG:
      return 0;

    case DN_CTLCOLORDIALOG:
      return Param2;

    case DN_CTLCOLORDLGITEM:
      return Param2;

    case DN_CTLCOLORDLGLIST:
      return FALSE;

    case DN_ENTERIDLE:
      return 0;     // always 0
  }

  // предварительно проверим...
  if(Param1 >= Dlg->ItemCount)
    return 0;

  CurItem=&Dlg->Item[Param1];
  Type=CurItem->Type;

  Ptr=CurItem->Data;

  switch(Msg)
  {
    case DN_MOUSECLICK:
      return 0;

    case DN_DRAWDLGITEM:
      return 0;

    case DN_HOTKEY:
      return TRUE;

    case DN_EDITCHANGE:
      return TRUE;

    case DN_BTNCLICK:
      return TRUE;

    case DN_LISTCHANGE:
      return TRUE;

    /* $ 23.08.2000 SVS
       + получить клавишу(ы)
    */
    case DM_KEY:
      return FALSE;
    /* SVS $ */
  }

  return 0;
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
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

//  CurItem=&Dlg->Item[Param1];
  CurItem=Dlg->Item+Param1;
  Type=CurItem->Type;
  Ptr=CurItem->Data;

  switch(Msg)
  {
    case DM_SETTEXT:
      if(Param2) // если здесь NULL, то это еще один способ получить размер
      {
        switch(Type)
        {
          case DI_USERCONTROL:
          case DI_TEXT:
          case DI_VTEXT:
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
            strncpy(Ptr,(char *)Param2,512);
            if(Dlg->CheckDialogMode(DMODE_SHOW))
            {
              Dlg->ShowDialog(Param1);
              ScrBuf.Flush();
            }
            return strlen((char *)Param2)+1;

          case DI_BUTTON:
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
        Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        if(Dlg->CheckDialogMode(DMODE_SHOW)) // достаточно ли этого????!!!!
        {
          Dlg->ShowDialog(Param1);
          ScrBuf.Flush();
        }
        return strlen((char *)Param2)+1;
      }
      return 0;

    case DM_SETTEXTLENGTH:
      if(IsEdit(Type) && !(CurItem->Flags & DIF_DROPDOWNLIST))
      {
        Param1=((Edit *)(CurItem->ObjPtr))->GetMaxLength();
        ((Edit *)(CurItem->ObjPtr))->SetMaxLength(Param2);
        Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        return Param1;
      }
      return 0;

    case DN_LISTCHANGE:
      return Dlg->DlgProc(hDlg,Msg,Param1,(long)&CurItem->ListItems);

    case DN_EDITCHANGE:
      // преобразуем данные для!
      Dialog::ConvertItem(CVTITEM_TOPLUGIN,&PluginDialogItem,CurItem,1);
      I=Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem);
      Dialog::ConvertItem(CVTITEM_FROMPLUGIN,&PluginDialogItem,CurItem,1);
      return I;

    case DN_BTNCLICK:
      return Dlg->DlgProc(hDlg,Msg,Param1,Param2);

    case DN_DRAWDLGITEM:
      // преобразуем данные для!
      Dialog::ConvertItem(CVTITEM_TOPLUGIN,&PluginDialogItem,CurItem,1);
      Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem);
      Dialog::ConvertItem(CVTITEM_FROMPLUGIN,&PluginDialogItem,CurItem,1);
      return 0;

    case DM_SETREDRAW:
      if(Dlg->CheckDialogMode(DMODE_INITOBJECTS))
        Dlg->Show();
      return 0;

    /* $ 08.09.2000 SVS
      - Если коротко, то DM_SETFOCUS вроде как и работал :-)
    */
    case DM_SETFOCUS:
//      if(!Dialog::IsFocused(Dlg->Item[Param1].Type))
//        return FALSE;
      if(Dlg->ChangeFocus2(Dlg->FocusPos,Param1) == Param1)
      {
        Dlg->ShowDialog();
        return TRUE;
      }
      return FALSE;
    /* SVS $ */

    /* $ 20.10.2000 SVS
      Получить ID фокуса
    */
    case DM_GETFOCUS:
      return Dlg->FocusPos;
    /* SVS $ */

    case DM_GETTEXT:
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

          case DI_USERCONTROL:
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
      // следовательно сразу должен идти "case DM_GETTEXTLENGTH"!!!

    case DM_GETTEXTLENGTH:
      Len=strlen(Ptr)+1;
      switch(Type)
      {
        case DI_BUTTON:
          if (!(CurItem->Flags & DIF_NOBRACKETS))
            Len-=4;
          break;

        case DI_USERCONTROL:
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

    case DM_GETDLGITEM:
      if(Param2)
      {
        Dialog::ConvertItem(CVTITEM_TOPLUGIN,(struct FarDialogItem *)Param2,CurItem,1);
/*
        if(IsEdit(Type))
        {
          ((Edit *)(CurItem->ObjPtr))->GetString(Str,sizeof(Str));
          strcpy((char *)Param2,Str);
        }
        else
          strcpy(((struct FarDialogItem *)Param2)->Data,CurItem->Data);
*/
        return TRUE;
      }
      return FALSE;

    case DM_SETDLGITEM:
      if(Param2)
      {
        Dialog::ConvertItem(CVTITEM_FROMPLUGIN,(struct FarDialogItem *)Param2,CurItem,1);
        CurItem->Type=Type;
        // еще разок, т.к. данные могли быть изменены
        Dlg->InitDialogObjects(Param1);
        if(Dlg->CheckDialogMode(DMODE_SHOW))
        {
          Dlg->ShowDialog(Param1);
          ScrBuf.Flush();
        }
        return TRUE;
      }
      return FALSE;


    /* $ 18.08.2000 SVS
       + Разрешение/запрещение отрисовки диалога
    */
    case DM_ENABLEREDRAW:
      if(Param1)
        Dlg->IsEnableRedraw++;
      else
        Dlg->IsEnableRedraw--;

      if(!Dlg->IsEnableRedraw)
        if(Dlg->CheckDialogMode(DMODE_INITOBJECTS))
          Dlg->Show();
      return 0;
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + показать/спрятать диалог.
    */
    case DM_SHOWDIALOG:
//      if(!Dlg->IsEnableRedraw)
      {
        if(Param1)
        {
          if(!Dlg->IsVisible())
            Dlg->Show();
        }
        else
        {
          if(Dlg->IsVisible())
            Dlg->Hide();
        }
      }
      return 0;
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + установить/взять данные диалога.
    */
    case DM_SETDLGDATA:
    {
      long PrewDataDialog=Dlg->DataDialog;
      Dlg->DataDialog=Param2;
      return PrewDataDialog;
    }

    case DM_GETDLGDATA:
    {
      return Dlg->DataDialog;
    }
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + послать клавишу(ы)
    */
    case DM_KEY:
    {
      int *KeyArray=(int*)Param2;
      Dlg->SetDialogMode(DMODE_KEY);
      for(I=0; I < Param1; ++I)
        Dlg->ProcessKey(KeyArray[I]);
      Dlg->SkipDialogMode(DMODE_KEY);
      return 0;
    }
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + принудительно закрыть диалог
    */
    case DM_CLOSE:
      if(Param1 == -1)
        Dlg->ExitCode=Dlg->FocusPos;
      else
        Dlg->ExitCode=Param1;
      Dlg->EndLoop=TRUE;
      return TRUE;  // согласен с закрытием
    /* SVS $ */

    /* $ 25.08.2000 SVS
        + получить координаты диалогового окна
    */
    case DM_GETDLGRECT:
    {
      if(Param2)
      {
        int x1,y1,x2,y2;
        Dlg->GetPosition(x1,y1,x2,y2);
        ((SMALL_RECT*)Param2)->Left=x1;
        ((SMALL_RECT*)Param2)->Top=y1;
        ((SMALL_RECT*)Param2)->Right=x2;
        ((SMALL_RECT*)Param2)->Bottom=y2;
        return TRUE;
      }
      return FALSE;
    }
    /* SVS $ */

    /* $ 30.08.2000 SVS
        + программное перемещение диалога
    */
    case DM_MOVEDIALOG:
    {
      int W1,H1;

      W1=Dlg->X2-Dlg->X1;
      H1=Dlg->Y2-Dlg->Y1;
      // сохранили
      Dlg->OldX1=Dlg->X1;
      Dlg->OldY1=Dlg->Y1;
      Dlg->OldX2=Dlg->X2;
      Dlg->OldY2=Dlg->Y2;
      // переместили
      if(Param1)   // абсолютно?
      {
        Dlg->X1=((COORD*)Param2)->X;
        Dlg->Y1=((COORD*)Param2)->Y;
      }
      else         // значит относительно
      {
        Dlg->X1+=((COORD*)Param2)->X;
        Dlg->Y1+=((COORD*)Param2)->Y;
      }

      // проверили и скорректировали
      if(Dlg->X1 < 0)         Dlg->X1=0;
      if(Dlg->Y1 < 0)         Dlg->Y1=0;
      if(Dlg->X1+W1 >= ScrX)  Dlg->X1=ScrX-W1; //?
      if(Dlg->Y1+H1 >= ScrY)  Dlg->Y1=ScrY-H1; //?
      Dlg->X2=Dlg->X1+W1;
      Dlg->Y2=Dlg->Y1+H1;
      ((COORD*)Param2)->X=Dlg->X1;
      ((COORD*)Param2)->Y=Dlg->Y1;

      I=Dlg->IsVisible();
      if(I) Dlg->Hide();
      // приняли.
      Dlg->AdjustEditPos(Dlg->X1-Dlg->OldX1,Dlg->Y1-Dlg->OldY1);
      if(I) Dlg->Show(); // только если диалог был виден

      return Param2;
    }
    /* SVS $ */

    /* $ 31.08.2000 SVS
        + переключение/получение состояния Enable/Disable элемента
    */
    case DM_ENABLE:
    {
      DWORD PrevFlags=CurItem->Flags;
      if(Param2 != -1)
      {
         if(Param2)
           CurItem->Flags&=~DIF_DISABLE;
         else
           CurItem->Flags|=DIF_DISABLE;
      }
      return (PrevFlags&DIF_DISABLE)?FALSE:TRUE;
    }
    /* SVS $ */
  }

  // Все, что сами не отрабатываем - посылаем на обработку обработчику.
  return Dlg->DlgProc(hDlg,Msg,Param1,Param2);
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////

