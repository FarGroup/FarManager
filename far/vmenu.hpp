#ifndef __VMENU_HPP__
#define __VMENU_HPP__
/*
vmenu.hpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * ...

*/

/* Revision: 1.21 29.06.2001 $ */

/*
Modify:
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

#define VMENU_COLOR_COUNT  16

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
#define VMENU_LISTBOX	            0x00000200
#define VMENU_SHOWNOBOX             0x00000400
#define VMENU_AUTOHIGHLIGHT	        0x00000800
#define VMENU_REVERSIHLIGHT	        0x00001000
#define VMENU_UPDATEREQUIRED        0x00002000
#define VMENU_DISABLEDRAWBACKGROUND 0x00004000
#define VMENU_WRAPMODE              0x00008000
#define VMENU_SHOWAMPERSAND         0x00010000
#define VMENU_WARNDIALOG            0x00020000

class Dialog;
class SaveScreen;


struct MenuItem
{
  DWORD  Flags;                  // Флаги пункта
  char   Name[130];              // Текст пункта
  short  AmpPos;                 // Позиция автоназначенной подсветки
  int    UserDataSize;           // Размер пользовательских данных
  union {                        // Пользовательские данные:
    char  *UserData;             // - указатель!
    char   Str4[4];              // - strlen(строка)+1 <= 4
  };

  DWORD SetCheck(int Value){ if(Value) {Flags|=LIF_CHECKED; if(Value!=1) Flags|=Value&0xFFFF;} else Flags&=~(0xFFFF|LIF_CHECKED); return Flags;}
  DWORD SetSelect(int Value){ if(Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
  DWORD SetDisable(int Value){ if(Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
};

struct MenuData
{
  char *Name;
  DWORD Flags;

  DWORD SetCheck(int Value){ if(Value) Flags|=((Value&0xFFFF)|LIF_CHECKED); else Flags&=~(0xFFFF|LIF_CHECKED); return Flags;}
  DWORD SetSelect(int Value){ if(Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
  DWORD SetDisable(int Value){ if(Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
};

class VMenu: public Modal
{
  private:
    SaveScreen *SaveScr;
    char Title[100];
    char BottomTitle[100];
    int SelectPos,TopPos;
    int MaxHeight;
    int MaxLength;
    int BoxType;
    int CallCount;
    int PrevMacroMode;
    /* $ 18.07.2000 SVS
       + переменная, отвечающая за отображение scrollbar в
         DI_LISTBOX & DI_COMBOBOX
    */
    DWORD VMFlags;
    /* SVS $ */

    /* $ 01.08.2000 SVS
       + Для LisBox - родитель в виде диалога
       + Обработчик меню!
    */
    Dialog *ParentDialog;
    FARWINDOWPROC VMenuProc;      // функция обработки меню
    /* SVS $ */

  protected:
    struct MenuItem *Item;
    int ItemCount;
    /* $ 28.07.2000 SVS
       Цветовые атрибуты
    */
    short Colors[VMENU_COLOR_COUNT];
    /* SVS */

  private:
    void DisplayObject();
    void ShowMenu(int IsParent=0);
    int  GetPosition(int Position);

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
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();

    void SetTitle(const char *Title);
    char *GetTitle(char *Dest,int Size);
    void SetBottomTitle(const char *BottomTitle);
    char *GetBottomTitle(char *Dest,int Size);
    void SetDialogStyle(int Style) {ChangeFlags(VMENU_WARNDIALOG,Style);SetColors(NULL);}
    void SetUpdateRequired(int SetUpdate) {ChangeFlags(VMENU_UPDATEREQUIRED,SetUpdate);}
    void SetBoxType(int BoxType);

    void SetFlags(DWORD Flags){ VMFlags|=Flags; }
    void SkipFlags(DWORD Flags){ VMFlags&=~Flags; }
    int  CheckFlags(DWORD Flags){ return(VMFlags&Flags); }
    DWORD ChangeFlags(DWORD Flags,BOOL Status);

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

    int  AddItem(struct MenuItem *NewItem,int PosAdd=-1);
    int  AddItem(struct FarList *NewItem);
    int  AddItem(char *NewStrItem);

    int  InsertItem(struct FarListInsert *NewItem);
    int  UpdateItem(struct FarList *NewItem);
    int  FindItem(struct FarListFind *FindItem);
    int  FindItem(int StartIndex,char *Pattern,DWORD Flags=0);

    int  GetItemCount() {return(ItemCount);};

    void *GetUserData(void *Data,int Size,int Position=-1);
    int  GetUserDataSize(int Position=-1);
    int  SetUserData(void *Data,int Size=0,int Position=-1);

    int  GetSelectPos() {return SelectPos;}
    int  SetSelectPos(int Pos,int Direct);
    int  GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);

    /* $ 20.09.2000 SVS
      + Функция GetItemPtr - получить указатель на нужный Item.
    */
    struct MenuItem *GetItemPtr(int Position=-1);
    /* SVS $*/

    void SortItems(int Direction=0);
    BOOL GetVMenuInfo(struct FarListInfo* Info);

    static struct MenuItem *FarList2MenuItem(struct FarListItem *Items,struct MenuItem *ListItem);
    static struct FarListItem *MenuItem2FarList(struct MenuItem *ListItem,struct FarListItem *Items);

    /* $ 01.08.2000 SVS
       функция обработки меню (по умолчанию)
    */
    static long WINAPI DefMenuProc(HANDLE hVMenu,int Msg,int Param1,long Param2);
    // функция посылки сообщений меню
    static long WINAPI SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,long Param2);
    /* SVS $ */

    Frame *FrameFromLaunched;
};

#endif	// __VMENU_HPP__
