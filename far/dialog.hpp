#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Frame.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
//#define DMODE_OWNSITEMS     0x00080000 // если TRUE, Dialog освобождает список Item в деструкторе
#define DMODE_NODRAWSHADOW  0x00100000 // не рисовать тень?
#define DMODE_NODRAWPANEL   0x00200000 // не рисовать подложку?
#define DMODE_CLICKOUTSIDE  0x20000000 // было нажатие мыши вне диалога?
#define DMODE_MSGINTERNAL   0x40000000 // Внутренняя Message?
#define DMODE_OLDSTYLE      0x80000000 // Диалог в старом (до 1.70) стиле

#define DIMODE_REDRAW       0x00000001 // требуется принудительная прорисовка итема?

// Флаги для функции ConvertItem
enum CVTITEMFLAGS {
	CVTITEM_TOPLUGIN        = 0,
	CVTITEM_FROMPLUGIN      = 1,
	CVTITEM_TOPLUGINSHORT   = 2,
	CVTITEM_FROMPLUGINSHORT = 3
};

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
  DLGIIF_COMBOBOXNOREDRAWEDIT     = 0x00000008, // не прорисовывать строку редактирования при изменениях в комбо
  DLGIIF_COMBOBOXEVENTKEY         = 0x00000010, // посылать события клавиатуры в диалоговую проц. для открытого комбобокса
  DLGIIF_COMBOBOXEVENTMOUSE       = 0x00000020, // посылать события мыши в диалоговую проц. для открытого комбобокса
  DLGIIF_EDITCHANGEPROCESSED      = 0x00000040, // элемент обрабатывает событие DN_EDITCHANGE
};


#define MakeDialogItemsEx(Data,Item) \
	DialogItemEx Item[countof(Data)]; \
	Dialog::DataToItemEx(Data,Item,countof(Data));

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
  	DWORD_PTR Reserved;
    int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
		FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;

  string strData;
  size_t nMaxLength;

  WORD ID;
  BitFlags IFlags;
  unsigned AutoCount;   // Автоматизация
	DialogItemAutomation* AutoPtr;
  DWORD_PTR UserData; // ассоциированные данные

  // прочее
  void *ObjPtr;
  VMenu *ListPtr;
  DlgUserControl *UCData;

  int SelStart;
  int SelEnd;

  void Clear()
  {
    Type=0;
    X1=0;
    Y1=0;
    X2=0;
    Y2=0;
    Focus=0;
    History=NULL;
    Flags=0;
    DefaultButton=0;

    strData=L"";
    nMaxLength=0;

    ID=0;
    IFlags.ClearAll();
    AutoCount=0;
    AutoPtr=NULL;
    UserData=0;

    ObjPtr=NULL;
    ListPtr=NULL;
    UCData=NULL;

    SelStart=0;
    SelEnd=0;
  }
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
    DWORD_PTR Reserved;
    unsigned int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
		FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  BYTE  DefaultButton;

  const wchar_t *Data;
};


struct FarDialogMessage{
  HANDLE   hDlg;
  int      Msg;
  int      Param1;
  LONG_PTR Param2;
};

class DlgEdit;
class ConsoleTitle;

typedef LONG_PTR (WINAPI *SENDDLGMESSAGE) (HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

class Dialog: public Frame
{
  friend class FindFiles;

  private:
    bool bInitOK;               // диалог был успешно инициализирован
    INT_PTR PluginNumber;       // Номер плагина, для формирования HelpTopic
    unsigned FocusPos;               // всегда известно какой элемент в фокусе
    unsigned PrevFocusPos;           // всегда известно какой элемент был в фокусе
    int IsEnableRedraw;         // Разрешена перерисовка диалога? ( 0 - разрешена)
    BitFlags DialogMode;        // Флаги текущего режима диалога

    LONG_PTR DataDialog;        // Данные, специфические для конкретного экземпляра диалога (первоначально здесь параметр, переданный в конструктор)

		DialogItemEx **Item; // массив элементов диалога
		DialogItemEx *pSaveItemEx; // пользовательский массив элементов диалога

    unsigned ItemCount;         // количество элементов диалога

    ConsoleTitle *OldTitle;     // предыдущий заголовок
    int PrevMacroMode;          // предыдущий режим макро

    FARWINDOWPROC RealDlgProc;      // функция обработки диалога

    // переменные для перемещения диалога
    int OldX1,OldX2,OldY1,OldY2;

    wchar_t *HelpTopic;

    volatile int DropDownOpened;// Содержит статус комбобокса и хистори: TRUE - открыт, FALSE - закрыт.

    CriticalSection CS;

    int RealWidth, RealHeight;

  private:
    void Init(FARWINDOWPROC DlgProc,LONG_PTR InitParam);
    virtual void DisplayObject();
    void DeleteDialogObjects();
    int  LenStrItem(int ID, const wchar_t *lpwszStr = NULL);

    void ShowDialog(unsigned ID=(unsigned)-1);  //    ID=-1 - отрисовать весь диалог

    LONG_PTR CtlColorDlgItem(int ItemPos,int Type,int Focus,DWORD Flags);
    /* $ 28.07.2000 SVS
       + Изменяет фокус ввода между двумя элементами.
         Вынесен отдельно для того, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
    */
    unsigned ChangeFocus2(unsigned KillFocusPos,unsigned SetFocusPos);

    unsigned ChangeFocus(unsigned FocusPos,int Step,int SkipGroup);
		BOOL SelectFromEditHistory(DialogItemEx *CurItem,DlgEdit *EditLine,const wchar_t *HistoryName,string &strStr);
		int SelectFromComboBox(DialogItemEx *CurItem,DlgEdit*EditLine,VMenu *List);
    int FindInEditForAC(int TypeFind, const wchar_t *HistoryName, string &strFindStr);
    int AddToEditHistory(const wchar_t *AddStr,const wchar_t *HistoryName);

    void ProcessLastHistory (DialogItemEx *CurItem, int MsgIndex); // обработка DIF_USELASTHISTORY

    int ProcessHighlighting(int Key,unsigned FocusPos,int Translate);
    BOOL CheckHighlights(WORD Chr);

    void SelectOnEntry(unsigned Pos,BOOL Selected);

    void CheckDialogCoord(void);
    BOOL GetItemRect(unsigned I,RECT& Rect);

    // возвращает заголовок диалога (текст первого текста или фрейма)
    const wchar_t *GetDialogTitle();

    BOOL SetItemRect(unsigned ID,SMALL_RECT *Rect);

    /* $ 23.06.2001 KM
       + Функции программного открытия/закрытия комбобокса и хистори
         и получения статуса открытости/закрытости комбобокса и хистори.
    */
    volatile void SetDropDownOpened(int Status){ DropDownOpened=Status; }
    volatile int GetDropDownOpened(){ return DropDownOpened; }

    void ProcessCenterGroup(void);
    unsigned ProcessRadioButton(unsigned);

    unsigned InitDialogObjects(unsigned ID=(unsigned)-1);

    int ProcessOpenComboBox(int Type,DialogItemEx *CurItem,unsigned CurFocusPos);
    int ProcessMoveDialog(DWORD Key);

    int Do_ProcessTab(int Next);
    int Do_ProcessNextCtrl(int Next,BOOL IsRedraw=TRUE);
    int Do_ProcessFirstCtrl();
    int Do_ProcessSpace();
		void SetComboBoxPos();

    LONG_PTR CallDlgProc (int nMsg, int nParam1, LONG_PTR nParam2);

  public:
    Dialog(DialogItemEx *SrcItem, unsigned SrcItemCount,
           FARWINDOWPROC DlgProc=NULL,LONG_PTR InitParam=0);
    Dialog(FarDialogItem *SrcItem, unsigned SrcItemCount,
           FARWINDOWPROC DlgProc=NULL,LONG_PTR InitParam=0);
    bool InitOK() {return bInitOK;}
    virtual ~Dialog();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
    virtual void Show();
    virtual void Hide();
    void FastShow() {ShowDialog();}

    void GetDialogObjectsData();

    void SetDialogMode(DWORD Flags){ DialogMode.Set(Flags); }

    // преобразования из внутреннего представления в FarDialogItem и обратно
    static bool ConvertItemEx (CVTITEMFLAGS FromPlugin,FarDialogItem *Item,
                               DialogItemEx *Data, unsigned Count);
    // преобразования из внутреннего представления в FarDialogItem в пользовательский буффер
	static size_t ConvertItemEx2(FarDialogItem *Item,DialogItemEx *Data);

    static void DataToItemEx(DialogDataEx *Data,DialogItemEx *Item,
                           int Count);

    static int IsKeyHighlighted(const wchar_t *Str,int Key,int Translate,int AmpPos=-1);

    // метод для перемещения диалога
    void AdjustEditPos(int dx,int dy);

    int IsMoving() {return DialogMode.Check(DMODE_DRAGGED);}
    void SetModeMoving(int IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
    int  GetModeMoving(void) {return DialogMode.Check(DMODE_ISCANMOVE);}
    void SetDialogData(LONG_PTR NewDataDialog);
    LONG_PTR GetDialogData(void) {return DataDialog;};

    void InitDialog(void);
    void Process();
    void SetPluginNumber(INT_PTR NewPluginNumber){PluginNumber=NewPluginNumber;}

    void SetHelp(const wchar_t *Topic);
    void ShowHelp();
    int Done() { return DialogMode.Check(DMODE_ENDLOOP); }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog(bool deleteFrame=true);

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    virtual const wchar_t *GetTypeName() {return L"[Dialog]";};

    virtual int GetMacroMode();

    /* $ Введена для нужд CtrlAltShift OT */
    virtual int FastHide();
    virtual void ResizeConsole();
//    virtual void OnDestroy();

    // For MACRO
		const DialogItemEx **GetAllItem(){return (const DialogItemEx**)Item;};
    unsigned GetAllItemCount(){return ItemCount;};              // количество элементов диалога
    unsigned GetDlgFocusPos(){return FocusPos;};


    int SetAutomation(WORD IDParent,WORD id,
                        DWORD UncheckedSet,DWORD UncheckedSkip,
                        DWORD CheckedSet,DWORD CheckedSkip,
                        DWORD Checked3Set=0,DWORD Checked3Skip=0);

    /* $ 23.07.2000 SVS: функция обработки диалога (по умолчанию) */
    static LONG_PTR WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
    /* $ 28.07.2000 SVS: функция посылки сообщений диалогу */
    static LONG_PTR WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

    LONG_PTR WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

    virtual void SetPosition(int X1,int Y1,int X2,int Y2);

    BOOL IsInited(void);
    BOOL IsEditChanged(unsigned ID);
};

#endif // __DIALOG_HPP__
