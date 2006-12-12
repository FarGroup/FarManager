#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Frame.

*/

#include "frame.hpp"
#include "plugin.hpp"
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"
#include "UnicodeString.hpp"

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


#define MakeDialogItemsEx(Data,Item) \
  struct DialogItemEx Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItemEx(Data,Item,sizeof(Data)/sizeof(Data[0]));

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
struct DialogItemEx
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;

  string strData;
  int nMaxLength;

  WORD ID;
  BitFlags IFlags;
  int AutoCount;   // Автоматизация
  struct DialogItemAutomation* AutoPtr;
  DWORD UserData; // ассоциированные данные

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

struct DialogDataEx
{
  WORD  Type;
  short X1,Y1,X2,Y2;
  BYTE  Focus;
  union {
    unsigned int Selected;
    const wchar_t *History;
    char *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  BYTE  DefaultButton;

  const wchar_t *Data;
};


struct FarDialogMessage{
  HANDLE hDlg;
  int    Msg;
  int    Param1;
  long   Param2;
};

class DlgEdit;
class ConsoleTitle;

typedef long (__stdcall *SENDDLGMESSAGE) (HANDLE hDlg,int Msg,int Param1,long Param2);

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
    struct DialogItemEx **Item;    // массив элементов диалога
    DialogItemEx *pSaveItemEx;

    int ItemCount;              // количество элементов диалога

    ConsoleTitle *OldTitle;     // предыдущий заголовок
    int DialogTooLong;          //
    int PrevMacroMode;          // предыдущий режим макро

    FARWINDOWPROC DlgProc;      // функция обработки диалога
    SENDDLGMESSAGE pfnSendDlgMessage;

    /* $ 31.07.2000 tran
       переменные для перемещения диалога */
    int  OldX1,OldX2,OldY1,OldY2;
    /* tran 31.07.2000 $ */

    /* $ 17.05.2001 DJ */
    wchar_t *HelpTopic;
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
    int  LenStrItem(int ID, const wchar_t *lpwszStr = NULL);
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
    BOOL SelectFromEditHistory(struct DialogItemEx *CurItem,DlgEdit *EditLine,const wchar_t *HistoryName,string &strStr,int MaxLen);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    int SelectFromComboBox(struct DialogItemEx *CurItem,DlgEdit*EditLine,VMenu *List,int MaxLen);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: Поиск входжение подстроки в истории
    */
    int FindInEditForAC(int TypeFind, const wchar_t *HistoryName, string &strFindStr);
    /* SVS $ */
    int AddToEditHistory(const wchar_t *AddStr,const wchar_t *HistoryName);

    /* $ 09.12.2001 DJ
       обработка DIF_USELASTHISTORY
    */
    void ProcessLastHistory (struct DialogItemEx *CurItem, int MsgIndex);
    /* DJ $ */

    int ProcessHighlighting(int Key,int FocusPos,int Translate);
    BOOL CheckHighlights(WORD Chr);

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
    const wchar_t *GetDialogTitle();
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

    int ProcessOpenComboBox(int Type,struct DialogItemEx *CurItem,int CurFocusPos);
    int ProcessMoveDialog(DWORD Key);

    int Do_ProcessTab(int Next);
    int Do_ProcessNextCtrl(int Next,BOOL IsRedraw=TRUE);
    int Do_ProcessFirstCtrl();
    int Do_ProcessSpace();

    int CallDlgProc (int nMsg, int nParam1, int nParam2);

  public:
    Dialog(struct DialogItemEx *Item,int ItemCount,FARWINDOWPROC DlgProc=NULL,long Param=0);
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
    static void ConvertItemEx (int FromPlugin,struct FarDialogItem *Item,struct DialogItemEx *Data,
                           int Count,BOOL InternalCall=FALSE);

    /* SVS $ */
    static void DataToItemEx(struct DialogDataEx *Data,struct DialogItemEx *Item,
                           int Count);

    static int IsKeyHighlighted(const wchar_t *Str,int Key,int Translate,int AmpPos=-1);

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
    void SetHelp(const wchar_t *Topic);
    void ShowHelp();
    int Done() { return DialogMode.Check(DMODE_ENDLOOP); }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog();
    /* DJ $ */

    /* $ 19.05.2001 DJ */
    // void SetOwnsItems (int AOwnsItems) { AOwnsItems = OwnsItems; } !!!!!!! :-)
    void SetOwnsItems (int AOwnsItems) { DialogMode.Change(DMODE_OWNSITEMS,AOwnsItems); }

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    virtual const wchar_t *GetTypeName() {return L"[Dialog]";};
    /* DJ $ */

    int GetMacroMode();

    /* $ Введена для нужд CtrlAltShift OT */
    int FastHide();
    void ResizeConsole();
//    void OnDestroy();

    // For MACRO
    const struct DialogItemEx **GetAllItem(){return (const DialogItemEx**)Item;};
    int GetAllItemCount(){return ItemCount;};              // количество элементов диалога
    int GetDlgFocusPos(){return FocusPos;};


    int SetAutomation(WORD IDParent,WORD id,
                        DWORD UncheckedSet,DWORD UncheckedSkip,
                        DWORD CheckedSet,DWORD CheckedSkip,
                        DWORD Checked3Set=0,DWORD Checked3Skip=0);

    /* $ 23.07.2000 SVS: функция обработки диалога (по умолчанию) */
    static long WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* $ 28.07.2000 SVS: функция посылки сообщений диалогу */
    static long __stdcall SendDlgMessage (HANDLE hDlg,int Msg,int Param1,long Param2)
    {
        Dialog *D = (Dialog*)hDlg;
        return D->pfnSendDlgMessage (hDlg, Msg, Param1, Param2);
    }

    static long __stdcall SendDlgMessageAnsi(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long __stdcall SendDlgMessageUnicode(HANDLE hDlg,int Msg,int Param1,long Param2);

    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
};

#endif // __DIALOG_HPP__
