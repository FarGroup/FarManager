#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Frame.

*/

/* Revision: 1.82 07.10.2005 $ */

/*
Modify:
  07.10.2005 SVS
    + Для MACRO (для "короткого" доступа к элементам класса):
        GetAllItem()
        GetAllItemCount()
        GetDlgFocusPos()
  24.04.2005 AY
    ! GCC
  22.03.2005 SVS
    + DMODE_BEGINLOOP
  29.01.2005 WARP
    ! Небольшой cleanup (см. 01920.vmenu_dialog_cleanup.txt)
  27.12.2004 WARP
    ! Сделал критические секции при практически всех вызовах Dialog & VMenu
  25.12.2004 WARP
    ! центрирование диалогов (подробнее см. 01894.DialogCenter.txt)
  08.12.2004 WARP
    ! Патч для поиска #1. Подробнее 01864.FindFile.txt
  06.07.2004 SVS
    + CheckHighlights для Macro II
  28.02.2004 SVS
    ! DLGEDITLILE_ -> DLGEDITLINE_
    + DLGITEMINTERNALFLAGS
  19.02.2004 SVS
    ! Флаг DMODE_MOUSELIST ненужен, т.к. это была ошибка: управление реакцией
      должно быть на уровне элемента, а не всего диалога
  18.12.2003 SVS
    + DMOUSEBUTTON_??? - на какие кнопки реагировать
  23.10.2003 SVS
    ! Dialog::GetDialogTitle возвращает const
    + CRITICAL_SECTION CSection
  30.05.2003 SVS
    + DMODE_CLICKOUTSIDE - было нажатие мыши вне диалога?
  19.05.2003 SVS
    + Добавка к DialogItem - SelStart, SelEnd. Отвечают за сохранение
      параметров выделения в строках редактирования
  25.02.2003 SVS
    - BugZ#811 - Зависание при невозможности загрузить плуг
      Последствия разборок с BugZ#806. Добавим флаг DMODE_MSGINTERNAL, который
      будет выставляться в Message().
      По хорошему здесь нужна очередь! Именно для нее и введена структура
      FarDialogMessage. Сейчас она пока функциональной нагрузки не несет...
      (введена, что не забыть сделать как можно быстрее, иначе таких баг-репортов
      будет море).
  17.10.2002 SVS
    + Добавим в Do_ProcessNextCtrl() параметр про принудительную
      прорисовку 2 элементов (старого и нового)
  30.09.2002 SVS
    ! SelectFromComboBox имеет первый параметр типа struct DialogItem
  23.09.2002 SVS
    + DialogItem имеет IFlags - для внутренних нужд.
  20.09.2002 SVS
    + флаги DMODE_NODRAWSHADOW и DMODE_NODRAWPANEL
  10.09.2002 SVS
    ! DialogItem приведем к FarDialogItem
    + некоторое количество приватных функций (куски повторяющегося код
      вынесены в отдельные функции)
  17.08.2002 SVS
    ! Обработка CtrlDown вынесена из обработчика клавиатуры в отдельную
      функцию ProcessOpenComboBox, т.к. вызов ProcessKey(KEY_CTRLDOWN)
      из мышиного обработчика приводит к побочным отрицательным эффектам
  18.05.2002 SVS
    ! OwnsItems -> DMODE_OWNSITEMS
  15.05.2002 SVS
    ! Вместо Edit юзаем новый класс DlgEdit
  06.05.2002 SVS
    + InitDialog() - инициализация диалога. Инициализируется в Process()
      или самостоятельно, но до вызова Show()
    ! InitDialogObjects() переехала в privat
  29.04.2002 SVS
    + ProcessRadioButton
  26.04.2002 SVS
    - BugZ#484 - Addons\Macros\Space.reg (про заголовки консоли)
  22.04.2002 KM
    ! Удалена функция OnDestroy - потеряла актуальность.
  08.04.2002 SVS
   + CtlColorDlgItem()
  11.03.2002 SVS
   ! Немного выравнивания... с переносом полей.
     На общей функциональности не отобразится, т.к. структура DialogItem
     приватная и в чистом виде не юзается.
  13.02.2002 SVS
   ! Первый параметр у IsKeyHighlighted() - const
  11.02.2002 SVS
   ! OriginalListItems к терапевту... - юзаем DlgProc
  05.02.2002 SVS
   + DialogItem.OriginalListItems
  21.01.2002 SVS
   ! Изменены данные для UserControl
  08.01.2002 SVS
   + SetListMouseReaction()
  21.12.2001 SVS
   ! unsigned char -> short для координат в структурах DialogItem и DialogData
   + LenStrItem() - выдает на гора размер с учетом флага DIF_SHOWAMPERSAND
  09.12.2001 DJ
   + ProcessLastHistory() - обработка DIF_USELASTHISTORY
  04.12.2001 SVS
   + ProcessCenterGroup() - пересчет координат элементов с флагом DIF_CENTERGROUP
  21.11.2001 SVS
   + Автоматизация (часть I)
   ! Изменения в структуре DialogItem: +DialogItem.ID, +DialogItem.AutoCount
     DialogItem.ID - пока зарезервированное поле, но потом это будет основным
     идентификатором контрола!
  12.11.2001 SVS
   ! SelectFromEditHistory() возвращает значение.
   ! SelectOnEntry() имеет доп.параметр - выделять или не выделять.
  12.11.2001 OT
   - VC выдает ошибку...
  08.11.2001 SVS
   ! Добавка в виде BitFlags - управление флагами текущего режима диалога
  06.11.2001 SVS
   ! Заводим доп.параметр struct DialogItem *CurItem у функции
     SelectFromEditHistory()
  01.11.2001 SVS
   ! MakeDialogItems перехала в dialog.hpp из farconst.hpp
  15.08.2001 SVS
   + DMODE_MOUSEEVENT
  23.07.2001 OT
   -  Исправление отрисовки меню в новом MA в макросах
  22.07.2001 SVS
   ! Пересмотрены параметры функции SelectFromComboBox() - ведь и так
     передаем указатель аж на цельный класс строки, так что...
  21.07.2001 KM
   ! Объявление FindFiles другом диалога для доступа к члену Item.
  12.07.2001 OT
   - Исправление ситуации (после 816) F11->F4->Esc-> :(
  11.07.2001 OT
   ! Перенос CtrlAltShift в Manager
  09.07.2001 OT
   - Исправление MacroMode для диалогов
  23.06.2001 KM
   + Функции программного открытия/закрытия комбобокса и хистори
     и получения статуса открытости/закрытости комбобокса и хистори.
   + Переменная DropDownOpened для хранения статуса комбобокса и хистори.
  04.06.2001 SVS
   ! HISTORY_COUNT -> farconst.hpp
   ! AddToEditHistory() - параметр про размер теперь нафиг ненужен.
  30.05.2001 KM
   + SetItemRect - функция для изменения размеров и/или положения
     итема в диалоге.
  23.05.2001 SVS
   - Проблемы с горячими клавишами в меню. Добавлен 4-й параметр в
     IsKeyHighlighted - позиция амперсанта (по умолчанию = -1)
  19.05.2001 DJ
   + OwnsItems
  17.05.2001 DJ
   ! Dialog унаследован от Frame
   + CloseDialog()
  14.05.2001 SVS
   ! DMODE_SMALLDILAOG -> DMODE_SMALLDIALOG
  12.05.2001 SVS
   ! Изменился второй параметр у SelectFromComboBox();
   ! Функция SelectFromComboBox() теперь возвращает код возврата.
   + DialogItem.ListPtr - для DI_COMBOBOX
  10.05.2001 SVS
   + FDLG_SMALLDILAOG - не рисовать "платформу"
   ! SetWarningStyle удалена
   ! Функция SetDialogMode перенесена в public секцию.
  06.05.2001 DJ
   + перетрях #include
  28.04.2001 SVS
   + GetItemRect() - получить координаты итема.
  12.04.2001 SVS
   ! функция AddToEditHistory теперь возвращает результат операции
     добавления строки в историю
  12.04.2001 SVS
   + CheckDialogCoord() - проверка и корректировка координат диалога
  23.03.2001 SVS
   ! У функции ConvertItem() новый параметр InternalCall - сейчас
     используется только для DN_EDITCHANGE
  13.02.2001 SVS
   + Дополнительный параметр у FindInEditForAC, SelectFromEditHistory,
     AddToEditHistory и SelectFromComboBox - MaxLen - максимальный размер
     строки назначения.
  24.09.2000 SVS
   + DMODE_ALTDRAGGED - при движении диалога по Alt-стрелка
  08.09.2000 SVS
   + Стиль диалога DMODE_OLDSTYLE - диалог в старом стиле.
   + Функция SelectOnEntry - выделение строки редактирования
     (Обработка флага DIF_SELECTONENTRY)
  30.08.2000 SVS
   + Режим диалога DMODE_SHOW - Диалог виден?
   + Метод Hide()
  29.08.2000 SVS
   ! При подмене темы помощи из диаловой процедуры...
     короче, нужно вновь формировать контент!
  24.08.2000 SVS
   + InitDialogObjects() имеет параметр - для выборочной реинициализации
     элементов
  23.08.2000 SVS
   ! изменения для DataDialog.
   + Переменная класса FocusPos - всегда известно какой элемент в фокусе
   ! Переменные IsCanMove, InitObjects, CreateObjects, WarningStyle, Dragged
     удалены -> битовые флаги в DialogMode
   ! Массив LV удален за ненадобностью.
   + CheckDialogMode - функция проверки флага DialogMode
  22.08.2000 SVS
   ! С моим английским вообще ни как :-((
     IsMovedDialog -> IsCanMove
     SetModeMoving -> SetMoveEnable
     GetModeMoving -> GetMoveEnable
   ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
  18.08.2000 SVS
   + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
   + DialogMode - Флаги текущего режима диалога
  11.08.2000 SVS
   + Данные, специфические для конкретного экземпляра диалога
   + Для того, чтобы послать DMSG_CLOSE нужно переопределить Process
  10.08.2000 SVS
   + переменная IsMovedDialog - можно ли двигать диалог :-)
   + функция установки IsMovedDialog
  09.08.2000 KM 1.09
   + Добавление функции проверки на режим перемещения диалога.
     Кстати новер редакции действительно 1.09 - один был пропущен.
  01.08.2000 SVS
   - переменная класса lastKey удалена за ненадобностью :-)
  31.07.2000 tran & SVS
   + переменная класса Dragged - флаг перемещения
     а также OldX*, OldY*,
     метод - AdjustEditPos(int dx,int dy) - подравнивает координаты Edit"ов
   + Сохранение того, что под индикатором перемещения диалога
  28.07.2000 SVS
   + Переменная класса InitParam - хранит параметр, переданный
     в диалог.
   ! Теперь InitDialogObjects возвращает ID элемента с фокусом ввода
   + Функция ChangeFocus2
     Изменяет фокус ввода между двумя элементами.
   ! Переметр Edit *EditLine в функции FindInEditForAC нафиг ненужен!
   ! FindInEditHistory -> FindInEditForAC
     Поиск как в истории, так и в ComboBox`е (чтобы не пладить кода)
   ! SelectFromComboBox имеет дополнительный параметр с тем, чтобы
     позиционировать item в меню со списком в соответсвии со строкой ввода
   + Функция IsFocused, определяющая - "Может ли элемент диалога
     иметь фокус ввода"
   ! IsEdit стал Static-методом!
   + функция посылки сообщений диалогу SendDlgMessage
   + Функция ConvertItem - преобразования из внутреннего представления
     в FarDialogItem и обратно
  26.07.2000 SVS
   + FindInEditHistory: Поиск входжение подстроки в истории
  25.07.2000 SVS
   + Private: lastKey - для AutoComplit последняя клавиша
   + Дополнительный параметр в SelectFromEditHistory для выделения
     нужной позиции в истории (если она соответствует строке ввода)
  25.07.2000 SVS
   ! новый параметр в конструкторе
  23.07.2000 SVS
   + Куча ремарок в исходниках :-)
   + Функция обработки диалога (по умолчанию) - забито место :-)
   ! Изменен вызов конструктора
  18.07.2000 SVS
    + функция SelectFromComboBox для выбора из DI_COMBOBOX
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "frame.hpp"
#include "plugin.hpp"
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"

// Флаги текущего режима диалога
#define DMODE_INITOBJECTS   0x00000001 // элементы инициализарованы?
#define DMODE_CREATEOBJECTS 0x00000002 // объекты (Edit,...) созданы?
#define DMODE_WARNINGSTYLE  0x00000004 // Warning Dialog Style?
#define DMODE_DRAGGED       0x00000008 // диалог двигается?
#define DMODE_ISCANMOVE     0x00000010 // можно ли двигать диалог?
#define DMODE_ALTDRAGGED    0x00000020 // диалог двигается по Alt-Стрелка?
#define DMODE_SMALLDIALOG   0x00000040 // "короткий диалог"
#define DMODE_DRAWING       0x00001000 // диалог рисуется?
#define DMODE_KEY           0x00002000 // Идет посылка клавиш?
#define DMODE_SHOW          0x00004000 // Диалог виден?
#define DMODE_MOUSEEVENT    0x00008000 // Нужно посылать MouseMove в обработчик?
#define DMODE_RESIZED       0x00010000 //
#define DMODE_ENDLOOP       0x00020000 // Конец цикла обработки диалога?
#define DMODE_BEGINLOOP     0x00040000 // Начало цикла обработки диалога?
#define DMODE_OWNSITEMS     0x00080000 // если TRUE, Dialog освобождает список Item в деструкторе
#define DMODE_NODRAWSHADOW  0x00100000 // не рисовать тень?
#define DMODE_NODRAWPANEL   0x00200000 // не рисовать подложку?
#define DMODE_CLICKOUTSIDE  0x20000000 // было нажатие мыши вне диалога?
#define DMODE_MSGINTERNAL   0x40000000 // Внутренняя Message?
#define DMODE_OLDSTYLE      0x80000000 // Диалог в старом (до 1.70) стиле

#define DIMODE_REDRAW       0x00000001 // требуется принудительная прорисовка итема?

// Флаги для функции ConvertItem
#define CVTITEM_TOPLUGIN    0
#define CVTITEM_FROMPLUGIN  1

#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

enum DLGEDITLINEFLAGS {
  DLGEDITLINE_CLEARSELONKILLFOCUS = 0x00000001, // управляет выделением блока при потере фокуса ввода
  DLGEDITLINE_SELALLGOTFOCUS      = 0x00000002, // управляет выделением блока при получении фокуса ввода
  DLGEDITLINE_NOTSELONGOTFOCUS    = 0x00000004, // не восстанавливать выделение строки редактирования при получении фокуса ввода
  DLGEDITLINE_NEWSELONGOTFOCUS    = 0x00000008, // управляет процессом выделения блока при получении фокуса
  DLGEDITLINE_GOTOEOLGOTFOCUS     = 0x00000010, // при получении фокуса ввода переместить курсор в конец строки
  DLGEDITLINE_PERSISTBLOCK        = 0x00000020, // постоянные блоки в строках ввода
  DLGEDITLINE_AUTOCOMPLETE        = 0x00000040, // автозавершение в строках ввода
  DLGEDITLINE_AUTOCOMPLETECTRLEND = 0x00000040, // при автозавершение подтверждать комбинацией Ctrl-End
  DLGEDITLINE_HISTORY             = 0x00000100, // история в строках ввода диалогов
};


enum DLGITEMINTERNALFLAGS {
  DLGIIF_LISTREACTIONFOCUS        = 0x00000001, // MouseReaction для фокусного элемента
  DLGIIF_LISTREACTIONNOFOCUS      = 0x00000002, // MouseReaction для не фокусного элемента
  DLGIIF_EDITPATH                 = 0x00000004, // здесь Ctrl-End в строке редактирования будет выдавать на гора автодополнение существующих путей в дополнении к выбору из истории
};


#define MakeDialogItems(Data,Item) \
  struct DialogItem Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItem(Data,Item,sizeof(Data)/sizeof(Data[0]));


// Структура, описывающая автоматизацию для DIF_AUTOMATION
// на первом этапе - примитивная - выставление флагов у элементов для CheckBox
struct DialogItemAutomation{
  WORD ID;                    // Для этого элемента...
  DWORD Flags[3][2];          // ...выставить вот эти флаги
                              // [0] - Unchecked, [1] - Checked, [2] - 3Checked
                              // [][0] - Set, [][1] - Skip
};

// Данные для DI_USERCONTROL
class DlgUserControl{
  public:
    COORD CursorPos;
    int   CursorVisible,CursorSize;

  public:
    DlgUserControl(){CursorSize=CursorPos.X=CursorPos.Y=-1;CursorVisible=0;}
   ~DlgUserControl(){};
};

/*
Описывает один элемент диалога - внутренне представление.
Для плагинов это FarDialogItem (за исключением ObjPtr)
*/
struct DialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;
  union {
    char Data[512];
    struct {
      DWORD PtrFlags;
      int   PtrLength;
      void *PtrData;
      char  PtrTail[1];
    } Ptr;
  };

  WORD ID;
  BitFlags IFlags;
  int AutoCount;   // Автоматизация
  struct DialogItemAutomation* AutoPtr;
  DWORD_PTR UserData; // ассоциированные данные

  // прочее
  void *ObjPtr;
  VMenu *ListPtr;
  DlgUserControl *UCData;

  int SelStart;
  int SelEnd;
};

/*
Описывает один элемент диалога - для сокращения объемов
Структура аналогичена структуре InitDialogItem (см. "Far PlugRinG
Russian Help Encyclopedia of Developer")
*/
struct DialogData
{
  WORD  Type;
  short X1,Y1,X2,Y2;
  BYTE  Focus;
  union {
    unsigned int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  BYTE  DefaultButton;
  char *Data;
};

struct FarDialogMessage{
  HANDLE hDlg;
  int    Msg;
  int    Param1;
  LONG_PTR   Param2;
};

class DlgEdit;
class ConsoleTitle;

class Dialog: public Frame
{
  /* $ 21.07.2001 KM
    ! Объявление FindFiles другом диалога для доступа к члену Item.
  */
  friend class FindFiles;
  /* KM $ */
  private:
    /* $ 29.08.2000 SVS
       + Номер плагина, для формирования HelpTopic
    */
    int PluginNumber;
    /* SVS $ */
    /* $ 23.08.2000 SVS
       + Переменная класса FocusPos
    */
    int FocusPos;               // всегда известно какой элемент в фокусе
    /* SVS $ */
    int PrevFocusPos;           // всегда известно какой элемент был в фокусе
    /* $ 18.08.2000 SVS
      + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
      + DialogMode - Флаги текущего режима диалога
    */
    int IsEnableRedraw;         // Разрешена перерисовка диалога? ( 0 - разрешена)
    BitFlags DialogMode;        // Флаги текущего режима диалога
    /* SVS $ */
    /* $ 11.08.2000 SVS
      + Данные, специфические для конкретного экземпляра диалога
    */
    long DataDialog;            // первоначально здесь параметр,
                                //   переданный в конструктор
    /* SVS $ */
    struct DialogItem *Item;    // массив элементов диалога
    int ItemCount;              // количество элементов диалога

    ConsoleTitle *OldTitle;     // предыдущий заголовок
    int DialogTooLong;          //
    int PrevMacroMode;          // предыдущий режим макро

    FARWINDOWPROC DlgProc;      // функция обработки диалога

    /* $ 31.07.2000 tran
       переменные для перемещения диалога */
    int  OldX1,OldX2,OldY1,OldY2;
    /* tran 31.07.2000 $ */

    /* $ 17.05.2001 DJ */
    char *HelpTopic;
    /* DJ $ */
    /* $ 23.06.2001 KM
       + Содержит статус комбобокса и хистори:
         TRUE - открыт, FALSE - закрыт.
    */
    volatile int DropDownOpened;
    /* KM $ */
    CriticalSection CS;

    int RealWidth, RealHeight;

  private:
    void DisplayObject();
    void DeleteDialogObjects();
    int  LenStrItem(int ID,char *Str=NULL);
    /* $ 22.08.2000 SVS
      ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
        ID=-1 - отрисовать весь диалог
    */
    void ShowDialog(int ID=-1);
    /* SVS $ */

    DWORD CtlColorDlgItem(int ItemPos,int Type,int Focus,DWORD Flags);
    /* $ 28.07.2000 SVS
       + Изменяет фокус ввода между двумя элементами.
         Вынесен отдельно для того, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
    */
    int ChangeFocus2(int KillFocusPos,int SetFocusPos);
    /* SVS $ */
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    static int IsEdit(int Type);
    /* $ 28.07.2000 SVS
       Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
    */
    static int IsFocused(int Type);
    /* SVS $ */
    /* $ 26.07.2000 SVS
      + Дополнительный параметр в SelectFromEditHistory для выделения
       нужной позиции в истории (если она соответствует строке ввода)
    */
    BOOL SelectFromEditHistory(struct DialogItem *CurItem,DlgEdit *EditLine,char *HistoryName,char *Str,int MaxLen);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    int SelectFromComboBox(struct DialogItem *CurItem,DlgEdit*EditLine,VMenu *List,int MaxLen);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: Поиск входжение подстроки в истории
    */
    int FindInEditForAC(int TypeFind,void *HistoryName,char *FindStr,int MaxLen);
    /* SVS $ */
    int AddToEditHistory(char *AddStr,char *HistoryName);

    /* $ 09.12.2001 DJ
       обработка DIF_USELASTHISTORY
    */
    void ProcessLastHistory (struct DialogItem *CurItem, int MsgIndex);
    /* DJ $ */

    int ProcessHighlighting(int Key,int FocusPos,int Translate);
    BOOL CheckHighlights(BYTE Chr);

    /* $ 08.09.2000 SVS
      Функция SelectOnEntry - выделение строки редактирования
      Обработка флага DIF_SELECTONENTRY
    */
    void SelectOnEntry(int Pos,BOOL Selected);
    /* SVS $ */

    void CheckDialogCoord(void);
    BOOL GetItemRect(int I,RECT& Rect);

    /* $ 19.05.2001 DJ
       возвращает заголовок диалога (текст первого текста или фрейма)
    */
    const char *GetDialogTitle();
    /* DJ $ */

    /* $ 30.05.2000 KM
       Меняет координаты или размер итема диалога.
    */
    BOOL SetItemRect(int ID,SMALL_RECT *Rect);
    /* KM $ */

    /* $ 23.06.2001 KM
       + Функции программного открытия/закрытия комбобокса и хистори
         и получения статуса открытости/закрытости комбобокса и хистори.
    */
    volatile void SetDropDownOpened(int Status){ DropDownOpened=Status; }
    volatile int GetDropDownOpened(){ return DropDownOpened; }
    /* KM $ */

    void ProcessCenterGroup(void);
    int ProcessRadioButton(int);

    /* $ 24.08.2000 SVS
       InitDialogObjects имеет параметр - для выборочной реинициализации
       элементов
    */
    int  InitDialogObjects(int ID=-1);
    /* 24.08.2000 SVS $ */

    int ProcessOpenComboBox(int Type,struct DialogItem *CurItem,int CurFocusPos);
    int ProcessMoveDialog(DWORD Key);

    int Do_ProcessTab(int Next);
    int Do_ProcessNextCtrl(int Next,BOOL IsRedraw=TRUE);
    int Do_ProcessFirstCtrl();
    int Do_ProcessSpace();

    LONG_PTR CallDlgProc (int nMsg, int nParam1, LONG_PTR nParam2);

  public:
    Dialog(struct DialogItem *Item,int ItemCount,FARWINDOWPROC DlgProc=NULL,LONG_PTR Param=0);
    ~Dialog();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Show();
    /* $ 30.08.2000 SVS
       Надобно перехватить Hide()
    */
    void Hide();
    /* SVS $ */
    void FastShow() {ShowDialog();}
    /* $ 28.07.2000 SVS
       Теперь InitDialogObjects возвращает ID элемента
       с фокусом ввода
    */
    /* SVS $ */
    void GetDialogObjectsData();

    void SetDialogMode(DWORD Flags){ DialogMode.Set(Flags); }

    /* $ 28.07.2000 SVS
       + Функция ConvertItem - преобразования из внутреннего представления
        в FarDialogItem и обратно
    */
    static void ConvertItem(int FromPlugin,struct FarDialogItem *Item,struct DialogItem *Data,
                           int Count,BOOL InternalCall=FALSE);
    /* SVS $ */
    static void DataToItem(struct DialogData *Data,struct DialogItem *Item,
                           int Count);
    static int IsKeyHighlighted(const char *Str,int Key,int Translate,int AmpPos=-1);

    /* $ 31.07.2000 tran
       метод для перемещения диалога */
    void AdjustEditPos(int dx,int dy);
    /* tran 31.07.2000 $ */

    /* $ 09.08.2000 KM
       Добавление функции, которая позволяет проверить
       находится ли диалог в режиме перемещения.
    */
    int IsMoving() {return DialogMode.Check(DMODE_DRAGGED);}
    /* KM $ */
    /* $ 10.08.2000 SVS
       можно ли двигать диалог :-)
    */
    void SetModeMoving(int IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
    int  GetModeMoving(void) {return DialogMode.Check(DMODE_ISCANMOVE);}
    /* SVS $ */
    /* $ 11.08.2000 SVS
       Работа с доп. данными экземпляра диалога
    */
    void SetDialogData(long NewDataDialog);
    long GetDialogData(void) {return DataDialog;};
    /* SVS $ */

    void InitDialog(void);
    /* $ 11.08.2000 SVS
       Для того, чтобы послать DMSG_CLOSE нужно переопределить Process
    */
    void Process();
    /* SVS $ */
    /* $ 29.08.2000 SVS
       + Установить номер плагина, для формирования HelpTopic
    */
    void SetPluginNumber(int NewPluginNumber){PluginNumber=NewPluginNumber;}
    /* SVS $ */

    /* $ 17.05.2001 DJ */
    void SetHelp(const char *Topic);
    void ShowHelp();
    int Done() { return DialogMode.Check(DMODE_ENDLOOP); }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog();
    /* DJ $ */

    /* $ 19.05.2001 DJ */
    // void SetOwnsItems (int AOwnsItems) { AOwnsItems = OwnsItems; } !!!!!!! :-)
    void SetOwnsItems (int AOwnsItems) { DialogMode.Change(DMODE_OWNSITEMS,AOwnsItems); }

    virtual int GetTypeAndName(char *Type,char *Name);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    virtual const char *GetTypeName() {return "[Dialog]";};
    /* DJ $ */

    int GetMacroMode();

    /* $ Введена для нужд CtrlAltShift OT */
    int FastHide();
    void ResizeConsole();
//    void OnDestroy();

    // For MACRO
    const struct DialogItem *GetAllItem(){return Item;};
    int GetAllItemCount(){return ItemCount;};              // количество элементов диалога
    int GetDlgFocusPos(){return FocusPos;};


    int SetAutomation(WORD IDParent,WORD id,
                        DWORD UncheckedSet,DWORD UncheckedSkip,
                        DWORD CheckedSet,DWORD CheckedSkip,
                        DWORD Checked3Set=0,DWORD Checked3Skip=0);

    /* $ 23.07.2000 SVS: функция обработки диалога (по умолчанию) */
    static LONG_PTR WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
    /* $ 28.07.2000 SVS: функция посылки сообщений диалогу */
    static LONG_PTR WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
};

#endif // __DIALOG_HPP__
