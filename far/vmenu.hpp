#ifndef __VMENU_HPP__
#define __VMENU_HPP__
/*
vmenu.hpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...

*/

/* Revision: 1.41 31.05.2002 $ */

/*
Modify:
  31.05.2002 SVS
    + SetMaxHeight()
  18.05.2002 SVS
    ! MouseDown -> VMENU_MOUSEDOWN
  28.04.2002 KM
    + VMENU_COMBOBOX
    + Меню может быть типа MODALTYPE_VMENU и MODALTYPE_COMBOBOX
  13.04.2002 KM
    - ??? Я не понял зачем в классе свой член SaveScr,
      если в ScreenObj он уже есть. Используя SaveScr из
      ScreenObj удалось избавиться от неперерисовок при
      ресайзинге консоли.
  21.02.2002 DJ
    ! корректная отрисовка списков, имеющих рамку, но не имеющих фокуса
    + функция корректировки текущей позиции в меню
  13.02.2002 SVS
    + Один интересный повторяющийся кусок вынесен в CheckKeyHighlighted()
    + MenuItem.NamePtr
  11.02.2002 SVS
    + Член AccelKey в MenuData и MenuItem
    + BitFlags
    ! у функции UpdateItem() параметр должен быть типа FarListUpdate
  01.12.2001 DJ
    - корректный MenuItem::SetCheck()
  02.12.2001 KM
    + VMOldFlags
  30.11.2001 DJ
    - значение VMENU_COLOR_COUNT приведено в соответствие с действительностью
  06.11.2001 SVS
    ! VMENU_REVERSIHLIGHT -> VMENU_REVERSEHIGHLIGHT
  01.11.2001 SVS
    + немного про "типы" - GetType*()
  13.10.2001 VVM
    ! Теперь меню не реагирует на отпускание клавиши мышки, если клавиша была нажата не в меню.
  10.10.2001 IS
    ! внедрение const
  24.08.2001 VVM
    + void SetExitCode(int Code) - вызывает функцию от предка Modal::
  31.07.2001 KM
    + Проверим, а не выполняется ли какая функа у нас...
      GetCallCount().
  26.07.2001 OT
    Исправления отрисовки ShiftF10-F1-AltF9
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  18.07.2001 OT
    ! Новый класс VFMenu. Добавлены константы, позоляющие ресайзить меню
  30.06.2001 KM
    + SetSelectPos(struct FarListPos *)
  + GetSelectPos(struct FarListPos *)
  29.06.2001 SVS
    + Новый параметр у FindItem - флаги
  25.06.2001 IS
    ! Внедрение const
  14.06.2001 SVS
    ! число -> VMENU_COLOR_COUNT
  10.06.2001 SVS
    + FindItem с двумя параметрами.
  04.06.2001 SVS
    ! Уточнение структуры MenuItem
  03.06.2001 KM
    + Функции SetTitle, GetTitle, GetBottomTitle.
  03.06.2001 SVS
    ! переделка MenuItem
    + GetPosition() - возвращает реальную позицию итема.
    + GetUserDataSize() - получить размер данных
    + SetUserData() - присовокупить данные к пункту меню
    ! GetUserData() - возвращает указатель на сами данные
  30.05.2001 OT
    - Проблемы с отрисовкой VMenu. В новом члене Frame *FrameFromLaunched
      запоминается тот фрейм, откуда это меню запускалось.
      Чтобы потом он не перерисовавался, когда его не просят :)
  25.05.2001 DJ
   + SetOneColor()
  21.05.2001 SVS
   ! VMENU_DRAWBACKGROUND -> VMENU_DISABLEDRAWBACKGROUND
   ! MENU_* выкинуты
   ! DialogStyle -> VMENU_WARNDIALOG
   ! struct MenuData
     Поля Selected, Checked и Separator преобразованы в DWORD Flags
   ! struct MenuItem
     Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
  18.05.2001 SVS
   ! UpdateRequired -> VMENU_UPDATEREQUIRED
   ! DrawBackground -> VMENU_DRAWBACKGROUND
   ! WrapMode -> VMENU_WRAPMODE
   ! ShowAmpersand -> VMENU_SHOWAMPERSAND
   + Функции InsertItem(), FindItem(), UpdateRequired(), GetVMenuInfo()
  17.05.2001 SVS
   + UpdateItem()
   + FarList2MenuItem()
   + MenuItem2FarList()
   + MenuItem.AmpPos - позиция автоназначенной подсветки - для того, чтобы
     вернуть владельцу листа те данные, которые он передавал!
  12.05.2001 SVS
   + AddItem(char *NewStrItem);
  07.05.2001 SVS
   + AddItem, отличается тем, что параметр типа struct FarList - для
     сокращения кода в диалогах :-)
   + SortItems() - опять же - для диалогов
   * Изменен тип возвращаемого значения для GetItemPtr() и убран первый
     параметр функции - Item
  06.05.2001 DJ
   ! перетрях #include
  20.01.2001 SVS
   + SetSelectPos() - установить позицию курсора!
  20.09.2000 SVS
   + Функция GetItemPtr - получить указатель на нужный Item.
  01.08.2000 SVS
   + В ShowMenu добавлен параметр, сообщающий - вызвали ли функцию
     самостоятельно или из другой функции ;-)
   ! Вызов конструктора
   ! ListBoxControl -> VMFlags
   + Флаги для параметра Flags в конструкторе
   + функция обработки меню (по умолчанию)
   + функция посылки сообщений меню
   + функция удаления N пунктов меню
   ! Изменен вызов конструктора для указания функции-обработчика!
  28.07.2000 SVS
   + Добавлены цветовые атрибуты (в переменных) и функции, связанные с
     атрибутами:
  22.07.2000 SVS
   !  AlwaysScrollBar изменен на ListBoxControl
  18.07.2000 SVS
    + Добавлена переменная класса AlwaysScrollBar, предназначенная
      для отображения (всегда, по мере надобности!) в элементах
      DI_LISTBOX & DI_COMBOBOX
    ! В связи с этим изменен вызов конструктора класса.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "modal.hpp"
#include "plugin.hpp"
#include "manager.hpp"
#include "frame.hpp"
#include "bitflags.hpp"

/* $ 30.11.2001 DJ
   значение приведено в соответствие с действительностью
*/

#define VMENU_COLOR_COUNT  10

/* DJ $ */

/* $ 28.07.2000 SVS
   Цветовые атрибуты - индексы в массиве цветов
*/
enum{
  VMenuColorBody=0,      // подложка
  VMenuColorBox=1,       // рамка
  VMenuColorTitle=2,     // заголовок - верхний и нижний
  VMenuColorText=3,      // Текст пункта
  VMenuColorHilite=4,    // HotKey
  VMenuColorSeparator=5, // separator
  VMenuColorSelected=6,  // Выбранный
  VMenuColorHSelect=7,   // Выбранный - HotKey
  VMenuColorScrollBar=8, // ScrollBar
  VMenuColorDisabled=9,  // Disabled
};
/* SVS */

#define VMENU_ALWAYSSCROLLBAR       0x00000100
#define VMENU_LISTBOX               0x00000200
#define VMENU_SHOWNOBOX             0x00000400
#define VMENU_AUTOHIGHLIGHT         0x00000800
#define VMENU_REVERSEHIGHLIGHT      0x00001000
#define VMENU_UPDATEREQUIRED        0x00002000
#define VMENU_DISABLEDRAWBACKGROUND 0x00004000
#define VMENU_WRAPMODE              0x00008000
#define VMENU_SHOWAMPERSAND         0x00010000
#define VMENU_WARNDIALOG            0x00020000
#define VMENU_NOTCENTER             0x00040000
#define VMENU_LEFTMOST              0x00080000
#define VMENU_NOTCHANGE             0x00100000
/* $ 21.02.2002 DJ
   меню является списком в диалоге и имеет фокус
*/
#define VMENU_LISTHASFOCUS          0x00200000
/* DJ $ */
/* $ 28.04.2002 KM
    меню является комбобоксом и обрабатывается
    менеджером по-особому.
*/
#define VMENU_COMBOBOX              0x00400000
/* KM $ */
#define VMENU_MOUSEDOWN             0x00800000

class Dialog;
class SaveScreen;

struct MenuItem
{
  DWORD  Flags;                  // Флаги пункта
  union {
    char  Name[128];              // Текст пункта
    char *NamePtr;
  };
  DWORD  AccelKey;
  int    UserDataSize;           // Размер пользовательских данных
  union {                        // Пользовательские данные:
    char  *UserData;             // - указатель!
    char   Str4[4];              // - strlen(строка)+1 <= 4
  };

  short AmpPos;                  // Позиция автоназначенной подсветки
  short Len[2];		             // размеры 2-х частей
  short Idx2;		             // начало 2-й части

  /* $ 01.12.2001 DJ
     исправим баг, заодно нормально отформатируем код
  */
  DWORD SetCheck(int Value)
  {
    if(Value)
    {
      Flags|=LIF_CHECKED;
      Flags &= ~0xFFFF;
      if(Value!=1) Flags|=Value&0xFFFF;
    }
    else
      Flags&=~(0xFFFF|LIF_CHECKED);
    return Flags;
  }
  /* DJ $ */

  DWORD SetSelect(int Value){ if(Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
  DWORD SetDisable(int Value){ if(Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
  char  operator[](int Pos) const;
                     // здесь сыграем на том, что у нас union ;-)
  char* PtrName();
};

struct MenuData
{
  char *Name;
  DWORD Flags;
  DWORD AccelKey;

  /* $ 01.12.2001 DJ
     исправим баг, заодно нормально отформатируем код
  */
  DWORD SetCheck(int Value)
  {
    if(Value)
    {
      Flags &= ~0xFFFF;
      Flags|=((Value&0xFFFF)|LIF_CHECKED);
    }
    else
      Flags&=~(0xFFFF|LIF_CHECKED);
    return Flags;
  }
  /* DJ $ */

  DWORD SetSelect(int Value){ if(Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
  DWORD SetDisable(int Value){ if(Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
};

class VMenu: virtual public Modal, virtual public Frame
{
#ifdef _MSC_VER
#pragma warning(disable:4250)
#endif _MSC_VER
  private:
    char Title[100];
    char BottomTitle[100];
    int SelectPos;
    int TopPos;
    int MaxHeight;
    int MaxLength;
    int BoxType;
    int CallCount;
    int PrevMacroMode;
    /* $ 18.07.2000 SVS
       + переменная, отвечающая за отображение scrollbar в
         DI_LISTBOX & DI_COMBOBOX
    */
    BitFlags VMFlags;
    BitFlags VMOldFlags;
    /* SVS $ */

    /* $ 01.08.2000 SVS
       + Для LisBox - родитель в виде диалога
       + Обработчик меню!
    */
    Dialog *ParentDialog;
    FARWINDOWPROC VMenuProc;      // функция обработки меню
    /* SVS $ */

    short RLen[2];	              // реальные размеры 2-х половин

  protected:
    /* $ 13.04.2002 KM
      - ??? Я не понял зачем здесь свой член SaveScr,
        если в ScreenObj уже есть этот член.
    */
//    SaveScreen *SaveScr;
    /* KM $ */
    struct MenuItem *Item;
    int ItemCount;
    /* $ 28.07.2000 SVS
       Цветовые атрибуты
    */
    short Colors[VMENU_COLOR_COUNT];
    /* SVS */

  public:
    Frame *FrameFromLaunched;

  private:
    void DisplayObject();
    void ShowMenu(int IsParent=0);
    int  GetPosition(int Position);
    static int _SetUserData(struct MenuItem *PItem,const void *Data,int Size);
    static void* _GetUserData(struct MenuItem *PItem,void *Data,int Size);
    BOOL CheckKeyHiOrAcc(DWORD Key,int Type,int Translate);

  public:
    /* $ 18.07.2000 SVS
       ! изменен вызов конструктора с учетом необходимости scrollbar в
         DI_LISTBOX & DI_COMBOBOX
         По умолчанию - зависит от настроек показа scrollbar в меню,
         т.е. не требуется. Для данных элементов (DI_LISTBOX & DI_COMBOBOX)
         параметр isListBoxControl должен быть равен TRUE.
    */
    /* $ 01.08.2000 SVS
       Изменен вызов конструктора для указания функции-обработчика и родителя!
    */
    VMenu(const char *Title,
          struct MenuData *Data,int ItemCount,
          int MaxHeight=0,
          DWORD Flags=0,
          FARWINDOWPROC Proc=NULL,
          Dialog *ParentDialog=NULL);
    /* 01.08.2000 SVS $ */
    /* SVS $ */
    ~VMenu();

  public:
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();

    void SetTitle(const char *Title);
    char *GetTitle(char *Dest,int Size);
    void SetBottomTitle(const char *BottomTitle);
    char *GetBottomTitle(char *Dest,int Size);
    void SetDialogStyle(int Style) {VMFlags.Change(VMENU_WARNDIALOG,Style);SetColors(NULL);}
    void SetUpdateRequired(int SetUpdate) {VMFlags.Change(VMENU_UPDATEREQUIRED,SetUpdate);}
    void SetBoxType(int BoxType);

    void SetFlags(DWORD Flags){ VMFlags.Set(Flags); }
    void SkipFlags(DWORD Flags){ VMFlags.Skip(Flags); }
    int  CheckFlags(DWORD Flags){ return VMFlags.Check(Flags); }
    DWORD ChangeFlags(DWORD Flags,BOOL Status) {return VMFlags.Change(Flags,Status);}

    void AssignHighlights(int Reverse);
    void SetColors(short *Colors=NULL);
    void GetColors(short *Colors);

    /* $ 25.05.2001 DJ */
    void SetOneColor (int Index, short Color);
    /* DJ $ */

    int  ProcessKey(int Key);
    int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    BOOL UpdateRequired(void);

    void DeleteItems();
    /* $ 01.08.2000 SVS
       функция удаления N пунктов меню
    */
    int  DeleteItem(int ID,int Count=1);
    /* SVS $ */

    int  AddItem(const struct MenuItem *NewItem,int PosAdd=0x7FFFFFFF);
    int  AddItem(const struct FarList *NewItem);
    int  AddItem(const char *NewStrItem);

    int  InsertItem(const struct FarListInsert *NewItem);
    int  UpdateItem(const struct FarListUpdate *NewItem);
    int  FindItem(const struct FarListFind *FindItem);
    int  FindItem(int StartIndex,const char *Pattern,DWORD Flags=0);

    int  GetItemCount() {return(ItemCount);};
    /* $ 31.07.2001 KM
      + Проверим, а не выполняется ли какая функа у нас...
    */
    int  GetCallCount() { return CallCount; };
    /* KM $ */

    void *GetUserData(void *Data,int Size,int Position=-1);
    int  GetUserDataSize(int Position=-1);
    int  SetUserData(void *Data,int Size=0,int Position=-1);

    int  GetSelectPos() {return SelectPos;}
    int  GetSelectPos(struct FarListPos *ListPos);
    int  SetSelectPos(int Pos,int Direct);
    int  SetSelectPos(struct FarListPos *ListPos);
    int  GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);
    /* $ 21.02.2002 DJ
       функция, проверяющая корректность текущей позиции и флагов SELECTED
    */
    void AdjustSelectPos();
    /* DJ $ */

    void Process();
    void ResizeConsole();

    /* $ 20.09.2000 SVS
      + Функция GetItemPtr - получить указатель на нужный Item.
    */
    struct MenuItem *GetItemPtr(int Position=-1);
    /* SVS $*/

    void SortItems(int Direction=0);
    BOOL GetVMenuInfo(struct FarListInfo* Info);

    static struct MenuItem *FarList2MenuItem(const struct FarListItem *Item,struct MenuItem *ListItem);
    static struct FarListItem *MenuItem2FarList(const struct MenuItem *ListItem,struct FarListItem *Item);

    /* $ 01.08.2000 SVS
       функция обработки меню (по умолчанию)
    */
    static long WINAPI DefMenuProc(HANDLE hVMenu,int Msg,int Param1,long Param2);
    // функция посылки сообщений меню
    static long WINAPI SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,long Param2);
    /* SVS $ */

    void SetExitCode(int Code) {Modal::SetExitCode(Code);}

    virtual const char *GetTypeName() {return "[VMenu]";};
    virtual int GetTypeAndName(char *Type,char *Name);
    /* $ 28.04.2002 KM
        Меню может быть типа MODALTYPE_VMENU и MODALTYPE_COMBOBOX
    */
    virtual int GetType() { return CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU; }
    /* KM $ */
    void SetMaxHeight(int NewMaxHeight);
};


#endif  // __VMENU_HPP__
