#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Frame.

*/

/* Revision: 1.35 12.07.2001 $ */

/*
Modify:
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
#define DMODE_OLDSTYLE      0x80000000 // Диалог в старом (до 1.70) стиле

// Флаги для функции ConvertItem
#define CVTITEM_TOPLUGIN    0
#define CVTITEM_FROMPLUGIN  1

/* $ 01.08.2000 SVS
  У структур DialogI* изменены:
  union {
    unsigned int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
  } Addons;

*/
// for class Dialog
/*
Описывает один элемент диалога - внутренне представление.
Для плагинов это FarDialogItem (за исключением ObjPtr)
*/
/* $ 12.08.2000 KM
   Дополнительное поле, содержащее маску ввода
*/
/* $ 08.12.2000 SVS
   Data "объединен" с новой структурой
*/
struct DialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  union {
    unsigned int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  unsigned char DefaultButton;
  union {
    char Data[512];
    int  ListPos;
    struct {
      DWORD PtrFlags;
      int   PtrLength;
      void *PtrData;
      char  PtrTail[1];
    } Ptr;
  };
  void *ObjPtr;
  VMenu *ListPtr;
};
/* SVS $ */

/*
Описывает один элемент диалога - для сокращения объемов
Структура аналогичена структуре InitDialogItem (см. "Far PlugRinG
Russian Help Encyclopedia of Developer")
*/
struct DialogData
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  union {
    unsigned int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  unsigned char DefaultButton;
  char *Data;
};
/* SVS $*/
/* KM $*/

class Edit;

class Dialog: public Frame
{
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
    DWORD DialogMode;       // Флаги текущего режима диалога
    /* SVS $ */
    /* $ 11.08.2000 SVS
      + Данные, специфические для конкретного экземпляра диалога
    */
    long DataDialog;            // первоначально здесь параметр,
                                //   переданный в конструктор
    /* SVS $ */
    struct DialogItem *Item;    // массив элементов диалога
    int ItemCount;              // количество элементов диалога

    char OldConsoleTitle[512];  // предыдущий заголовок консоли
    int DialogTooLong;          //
    int PrevMacroMode;          // предыдущий режим макро

    FARWINDOWPROC DlgProc;      // функция обработки диалога

    /* $ 31.07.2000 tran
       переменные для перемещения диалога */
    int  OldX1,OldX2,OldY1,OldY2;
    /* tran 31.07.2000 $ */

    /* $ 17.05.2001 DJ */
    char *HelpTopic;
    int  EndLoop;
    /* DJ $ */

    /* $ 19.05.2001 DJ
       если true, Dialog освобождает список Item в деструкторе
    */
    int OwnsItems;
    /* DJ $ */
    /* $ 23.06.2001 KM
       + Содержит статус комбобокса и хистори:
         TRUE - открыт, FALSE - закрыт.
    */
    volatile int DropDownOpened;
    /* KM $ */

  private:
    /* $ 18.08.2000 SVS
      + SetDialogMode - Управление флагами текущего режима диалога
    */
    void SkipDialogMode(DWORD Flags){ DialogMode&=~Flags; }
    /* SVS $ */
    /* $ 23.08.2000 SVS
       + Проверка флага
    */
    int CheckDialogMode(DWORD Flags){ return(DialogMode&Flags); }
    /* SVS $ */

    void DisplayObject();
    void DeleteDialogObjects();
    /* $ 22.08.2000 SVS
      ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
    */
    void ShowDialog(int ID=-1);
    /* SVS $ */

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
    void SelectFromEditHistory(Edit *EditLine,char *HistoryName,char *Str,int MaxLen);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    int SelectFromComboBox(Edit *EditLine,VMenu *List,char *Str,int MaxLen);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: Поиск входжение подстроки в истории
    */
    int FindInEditForAC(int TypeFind,void *HistoryName,char *FindStr,int MaxLen);
    /* SVS $ */
    int AddToEditHistory(char *AddStr,char *HistoryName);
    int ProcessHighlighting(int Key,int FocusPos,int Translate);

    /* $ 08.09.2000 SVS
      Функция SelectOnEntry - выделение строки редактирования
      Обработка флага DIF_SELECTONENTRY
    */
    void SelectOnEntry(int Pos);
    /* SVS $ */

    void CheckDialogCoord(void);
    BOOL GetItemRect(int I,RECT& Rect);

    /* $ 19.05.2001 DJ
       возвращает заголовок диалога (текст первого текста или фрейма)
    */
    char *GetDialogTitle();
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

  public:
    Dialog(struct DialogItem *Item,int ItemCount,FARWINDOWPROC DlgProc=NULL,long Param=NULL);
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
    /* $ 24.08.2000 SVS
       InitDialogObjects имеет параметр - для выборочной реинициализации
       элементов
    */
    int  InitDialogObjects(int ID=-1);
    /* 24.08.2000 SVS $ */
    /* SVS $ */
    void GetDialogObjectsData();

    void SetDialogMode(DWORD Flags){ DialogMode|=Flags; }

    /* $ 28.07.2000 SVS
       + Функция ConvertItem - преобразования из внутреннего представления
        в FarDialogItem и обратно
    */
    static void ConvertItem(int FromPlugin,struct FarDialogItem *Item,struct DialogItem *Data,
                           int Count,BOOL InternalCall=FALSE);
    /* SVS $ */
    static void DataToItem(struct DialogData *Data,struct DialogItem *Item,
                           int Count);
    static int IsKeyHighlighted(char *Str,int Key,int Translate,int AmpPos=-1);
    /* $ 23.07.2000 SVS
       функция обработки диалога (по умолчанию)
    */
    static long WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* SVS $ */
    /* $ 28.07.2000 SVS
       функция посылки сообщений диалогу
    */
    static long WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* SVS $ */

    /* $ 31.07.2000 tran
       метод для перемещения диалога */
    void AdjustEditPos(int dx,int dy);
    /* tran 31.07.2000 $ */

    /* $ 09.08.2000 KM
       Добавление функции, которая позволяет проверить
       находится ли диалог в режиме перемещения.
    */
    int IsMoving() {return CheckDialogMode(DMODE_DRAGGED);}
    /* KM $ */
    /* $ 10.08.2000 SVS
       можно ли двигать диалог :-)
    */
    void SetModeMoving(int IsMoving) {
      if(IsMoving)
        SetDialogMode(DMODE_ISCANMOVE);
      else
        SkipDialogMode(DMODE_ISCANMOVE);
    };
    int  GetModeMoving(void) {return CheckDialogMode(DMODE_ISCANMOVE);};
    /* SVS $ */
    /* $ 11.08.2000 SVS
       Работа с доп. данными экземпляра диалога
    */
    void SetDialogData(long NewDataDialog);
    long GetDialogData(void) {return DataDialog;};
    /* SVS $ */

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
    int Done() const
      { return EndLoop; }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog();
    /* DJ $ */

    /* $ 19.05.2001 DJ */
    void SetOwnsItems (int AOwnsItems) { AOwnsItems = OwnsItems; }
    virtual int GetTypeAndName(char *Type,char *Name);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    /* DJ $ */

    /* $ 20.05.2001 DJ */
    virtual void OnChangeFocus (int Focus);
    /* DJ $ */
    int GetMacroMode();

/* $ Введена для нужд CtrlAltShift OT */
    int FastHide();
    void ResizeConsole();
    void OnDestroy();

    bool Resized;


};

#endif // __DIALOG_HPP__
