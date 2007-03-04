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

#include "modal.hpp"
#include "plugin.hpp"
#include "manager.hpp"
#include "frame.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"
#include "UnicodeString.hpp"

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

#define VMENU_ALWAYSSCROLLBAR       0x00000100  // всегда показывать скроллбар
#define VMENU_LISTBOX               0x00000200  // Это список в диалоге
#define VMENU_SHOWNOBOX             0x00000400  // показать без рамки
#define VMENU_AUTOHIGHLIGHT         0x00000800  // автоматически выбирать симолы подсветки
#define VMENU_REVERSEHIGHLIGHT      0x00001000  // ... только с конца
#define VMENU_UPDATEREQUIRED        0x00002000  // лист необходимо обновить (перерисовать)
#define VMENU_DISABLEDRAWBACKGROUND 0x00004000  // подложку не рисовать
#define VMENU_WRAPMODE              0x00008000  // зацикленный список (при перемещении)
#define VMENU_SHOWAMPERSAND         0x00010000  // символ '&' показывать AS IS
#define VMENU_WARNDIALOG            0x00020000  //
#define VMENU_NOTCENTER             0x00040000  // не цитровать
#define VMENU_LEFTMOST              0x00080000  // "крайний слева" - нарисовать на 5 позиций вправо от центра (X1 => (ScrX+1)/2+5)
#define VMENU_NOTCHANGE             0x00100000  //
#define VMENU_LISTHASFOCUS          0x00200000  // меню является списком в диалоге и имеет фокус
#define VMENU_COMBOBOX              0x00400000  // меню является комбобоксом и обрабатывается менеджером по-особому.
#define VMENU_MOUSEDOWN             0x00800000  //
#define VMENU_CHANGECONSOLETITLE    0x01000000  //
#define VMENU_SELECTPOSNONE         0x02000000  //
#define VMENU_DISABLED              0x80000000  //

class Dialog;
class SaveScreen;


struct MenuItemEx
{
  DWORD  Flags;                  // Флаги пункта

  string strName;

  DWORD  AccelKey;
  int    UserDataSize;           // Размер пользовательских данных
  union {                        // Пользовательские данные:
    char  *UserData;             // - указатель!
    char   Str4[4];              // - strlen(строка)+1 <= 4
  };

  short AmpPos;                  // Позиция автоназначенной подсветки
  short Len[2];                  // размеры 2-х частей
  short Idx2;                    // начало 2-й части

  int   ShowPos;

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

  DWORD SetSelect(int Value){ if(Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
  DWORD SetDisable(int Value){ if(Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}

  void Clear ()
  {
    Flags = 0;
    strName = L"";
    AccelKey = 0;
    UserDataSize = 0;
    UserData = NULL;
    AmpPos = 0;
    Len[0] = 0;
    Len[1] = 0;
    Idx2 = 0;
    ShowPos = 0;
  }

  //UserData не копируется.
  const MenuItemEx& operator=(const MenuItemEx &srcMenu)
  {
    Flags = srcMenu.Flags;
    strName = srcMenu.strName;
    AccelKey = srcMenu.AccelKey;
    UserDataSize = 0;
    UserData = NULL;
    AmpPos = srcMenu.AmpPos;
    Len[0] = srcMenu.Len[0];
    Len[1] = srcMenu.Len[1];
    Idx2 = srcMenu.Idx2;
    ShowPos = srcMenu.ShowPos;
    return *this;
  }
};

struct MenuDataEx
{
  const wchar_t *Name;
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


class ConsoleTitle;

class VMenu: public Modal
{
#ifdef _MSC_VER
#pragma warning(disable:4250)
#endif //_MSC_VER
  private:
    string strTitle;
    string strBottomTitle;

    int SelectPos;
    int TopPos;
    int MaxHeight;
    int MaxLength;
    int BoxType;
    int PrevCursorVisible;
    int PrevCursorSize;
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
    int DialogItemID;
    FARWINDOWPROC VMenuProc;      // функция обработки меню
    /* SVS $ */

    short RLen[2];                // реальные размеры 2-х половин
    CRITICAL_SECTION CSection;
    ConsoleTitle *OldTitle;     // предыдущий заголовок

    CriticalSection CS;

  protected:

    MenuItemEx **Item;

    int ItemCount;

    int LastAddedItem;

    BYTE Colors[VMENU_COLOR_COUNT];

  public:
    Frame *FrameFromLaunched;

  private:
    void DisplayObject();
    void ShowMenu(int IsParent=0);
    void DrawTitles();
    int  GetPosition(int Position);
    static int _SetUserData(MenuItemEx *PItem,const void *Data,int Size);
    static void* _GetUserData(MenuItemEx *PItem,void *Data,int Size);
    BOOL CheckKeyHiOrAcc(DWORD Key,int Type,int Translate);
    BOOL CheckHighlights(WORD Chr);

  public:

    VMenu(const wchar_t *Title,
          MenuDataEx *Data,
          int ItemCount,
          bool bUnicode,/*FAKE, to make ctors different*/
          int MaxHeight=0,
          DWORD Flags=0,
          FARWINDOWPROC Proc=NULL,
          Dialog *ParentDialog=NULL);


    ~VMenu();

  public:
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();

    void SetTitle(const wchar_t *Title);
    string &GetTitle(string &strDest);
    const wchar_t *GetPtrTitle() { return (const wchar_t*)strTitle; }


    void SetBottomTitle(const wchar_t *BottomTitle);
    string &GetBottomTitle(string &strDest);
    void SetDialogStyle(int Style) {VMFlags.Change(VMENU_WARNDIALOG,Style);SetColors(NULL);}
    void SetUpdateRequired(int SetUpdate) {VMFlags.Change(VMENU_UPDATEREQUIRED,SetUpdate);}
    void SetBoxType(int BoxType);

    void SetFlags(DWORD Flags){ VMFlags.Set(Flags); }
    void ClearFlags(DWORD Flags){ VMFlags.Clear(Flags); }
    int  CheckFlags(DWORD Flags){ return VMFlags.Check(Flags); }
    DWORD ChangeFlags(DWORD Flags,BOOL Status) {return VMFlags.Change(Flags,Status);}

    void AssignHighlights(int Reverse);

    void SetColors(struct FarListColors *Colors=NULL);
    void GetColors(struct FarListColors *Colors);

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

    int  AddItemW(const MenuItemEx *NewItem,int PosAdd=0x7FFFFFFF);
    int  AddItem(const FarList *NewItem);
    int  AddItem(const wchar_t *NewStrItem);

    int  InsertItem(const FarListInsert *NewItem);
    int  UpdateItem(const FarListUpdate *NewItem);
    int  FindItem(const FarListFind *FindItem);
    int  FindItem(int StartIndex,const wchar_t *Pattern,DWORD Flags=0);

    int  GetItemCount() {return(ItemCount);};

    void *GetUserData(void *Data,int Size,int Position=-1);
    int  GetUserDataSize(int Position=-1);
    int  SetUserData(void *Data,int Size=0,int Position=-1);

    int  GetSelectPos() {return VMFlags.Check(VMENU_SELECTPOSNONE)?-1:SelectPos;}
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
    struct MenuItemEx *GetItemPtr(int Position=-1);
    /* SVS $*/

    void SortItems(int Direction=0,int Offset=0,BOOL SortForDataDWORD=FALSE);
    BOOL GetVMenuInfo(struct FarListInfo* Info);

    void SetExitCode(int Code) {Modal::SetExitCode(Code);}

    virtual const wchar_t *GetTypeName() {return L"[VMenu]";};
    virtual int GetTypeAndName(string &strType, string &strName);
    /* $ 28.04.2002 KM
        Меню может быть типа MODALTYPE_VMENU и MODALTYPE_COMBOBOX
    */
    virtual int GetType() { return CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU; }
    /* KM $ */
    void SetMaxHeight(int NewMaxHeight);

    int GetVDialogItemID() const {return DialogItemID;};
    void SetVDialogItemID(int NewDialogItemID) {DialogItemID=NewDialogItemID;};

  public:
    static MenuItemEx *FarList2MenuItem(const FarListItem *Item,MenuItemEx *ListItem);
    static FarListItem *MenuItem2FarList(const MenuItemEx *ListItem,FarListItem *Item);
    /* $ 01.08.2000 SVS
       функция обработки меню (по умолчанию)
    */
    static long WINAPI DefMenuProc(HANDLE hVMenu,int Msg,int Param1,long Param2);
    // функция посылки сообщений меню
    static long WINAPI SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,long Param2);
    /* SVS $ */

};


#endif  // __VMENU_HPP__
