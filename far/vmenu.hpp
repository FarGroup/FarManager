#ifndef __VMENU_HPP__
#define __VMENU_HPP__
/*
vmenu.hpp

ќбычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
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

#include "modal.hpp"
#include "plugin.hpp"
#include "manager.hpp"
#include "frame.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"


// ÷ветовые атрибуты - индексы в массиве цветов
enum{
  VMenuColorBody                = 0,     // подложка
  VMenuColorBox                 = 1,     // рамка
  VMenuColorTitle               = 2,     // заголовок - верхний и нижний
  VMenuColorText                = 3,     // “екст пункта
  VMenuColorHilite              = 4,     // HotKey
  VMenuColorSeparator           = 5,     // separator
  VMenuColorSelected            = 6,     // ¬ыбранный
  VMenuColorHSelect             = 7,     // ¬ыбранный - HotKey
  VMenuColorScrollBar           = 8,     // ScrollBar
  VMenuColorDisabled            = 9,     // Disabled
  VMenuColorArrows              =10,     // '<' & '>' обычные
  VMenuColorArrowsSelect        =11,     // '<' & '>' выбранные
  VMenuColorArrowsDisabled      =12,     // '<' & '>' Disabled
  VMenuColorGrayed              =13,     // "серый"
  VMenuColorSelGrayed           =14,     // выбранный "серый"

  VMENU_COLOR_COUNT,                     // всегда последн€€ - размерность массива
};

#define VMENU_ALWAYSSCROLLBAR       0x00000100  // всегда показывать скроллбар
#define VMENU_LISTBOX               0x00000200  // Ёто список в диалоге
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
#define VMENU_LISTHASFOCUS          0x00200000  // меню €вл€етс€ списком в диалоге и имеет фокус
#define VMENU_COMBOBOX              0x00400000  // меню €вл€етс€ комбобоксом и обрабатываетс€ менеджером по-особому.
#define VMENU_MOUSEDOWN             0x00800000  //
#define VMENU_CHANGECONSOLETITLE    0x01000000  //
#define VMENU_SELECTPOSNONE         0x02000000  //
#define VMENU_MOUSEREACTION         0x04000000  // реагировать на движение мыши? (перемещать позицию при перемещении курсора мыши?)
#define VMENU_DISABLED              0x08000000  //

class Dialog;
class SaveScreen;


struct MenuItemEx
{
  DWORD  Flags;                  // ‘лаги пункта

  string strName;

  DWORD  AccelKey;
  int    UserDataSize;           // –азмер пользовательских данных
  union {                        // ѕользовательские данные:
    char  *UserData;             // - указатель!
    char   Str4[4];              // - strlen(строка)+1 <= 4
  };

  short AmpPos;                  // ѕозици€ автоназначенной подсветки
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

  //UserData не копируетс€.
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

    // переменна€, отвечающа€ за отображение scrollbar в DI_LISTBOX & DI_COMBOBOX
    BitFlags VMFlags;
    BitFlags VMOldFlags;

    Dialog *ParentDialog;         // ƒл€ LisBox - родитель в виде диалога
    int DialogItemID;
    FARWINDOWPROC VMenuProc;      // функци€ обработки меню

    CRITICAL_SECTION CSection;
    ConsoleTitle *OldTitle;     // предыдущий заголовок

    CriticalSection CS;
		bool *Used;

  protected:

    MenuItemEx **Item;

    int ItemCount;
    int ItemHiddenCount;

    int LastAddedItem;

    BYTE Colors[VMENU_COLOR_COUNT];

  public:
    Frame *FrameFromLaunched;

  private:
    virtual void DisplayObject();
    void ShowMenu(int IsParent=0);
    void DrawTitles();
    int  GetItemPosition(int Position);
    static int _SetUserData(MenuItemEx *PItem,const void *Data,int Size);
    static void* _GetUserData(MenuItemEx *PItem,void *Data,int Size);
    BOOL CheckKeyHiOrAcc(DWORD Key,int Type,int Translate);
    BOOL CheckHighlights(WORD Chr);
    wchar_t GetHighlights(const struct MenuItemEx *_item);
    BOOL ShiftItemShowPos(int Pos,int Direct);
    int RecalcItemHiddenCount();

  public:

    VMenu(const wchar_t *Title,
          MenuDataEx *Data,
          int ItemCount,
          int MaxHeight=0,
          DWORD Flags=0,
          FARWINDOWPROC Proc=NULL,
          Dialog *ParentDialog=NULL);


    virtual ~VMenu();

  public:
    void FastShow() {ShowMenu();}
    virtual void Show();
    virtual void Hide();

    void SetTitle(const wchar_t *Title);
    virtual string &GetTitle(string &strDest,int SubLen=-1,int TruncSize=0);
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
    void SetOneColor (int Index, short Color);

    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		BOOL UpdateRequired();

    void DeleteItems();
    int  DeleteItem(int ID,int Count=1);

    int  AddItem(const MenuItemEx *NewItem,int PosAdd=0x7FFFFFFF);
    int  AddItem(const FarList *NewItem);
    int  AddItem(const wchar_t *NewStrItem);

    int  InsertItem(const FarListInsert *NewItem);
    int  UpdateItem(const FarListUpdate *NewItem);
    int  FindItem(const FarListFind *FindItem);
    int  FindItem(int StartIndex,const wchar_t *Pattern,DWORD Flags=0);

    int  GetItemCount() {return(ItemCount);};
    int  GetShowItemCount() {return(ItemCount-ItemHiddenCount);};
    int  GetVisualPos(int Pos);

    void *GetUserData(void *Data,int Size,int Position=-1);
    int  GetUserDataSize(int Position=-1);
    int  SetUserData(void *Data,int Size=0,int Position=-1);

    int  GetSelectPos() {return VMFlags.Check(VMENU_SELECTPOSNONE)?-1:SelectPos;}
    int  GetSelectPos(struct FarListPos *ListPos);
    int  SetSelectPos(int Pos,int Direct);
    int  SetSelectPos(struct FarListPos *ListPos);
    int  GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);
    //   функци€, провер€юща€ корректность текущей позиции и флагов SELECTED
    void AdjustSelectPos();

    virtual void Process();
    virtual void ResizeConsole();

    struct MenuItemEx *GetItemPtr(int Position=-1);

    void SortItems(int Direction=0,int Offset=0,BOOL SortForDataDWORD=FALSE);
    BOOL GetVMenuInfo(struct FarListInfo* Info);

    virtual const wchar_t *GetTypeName() {return L"[VMenu]";};
    virtual int GetTypeAndName(string &strType, string &strName);

    virtual int GetType() { return CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU; }

    void SetMaxHeight(int NewMaxHeight);

    int GetVDialogItemID() const {return DialogItemID;};
    void SetVDialogItemID(int NewDialogItemID) {DialogItemID=NewDialogItemID;};

  public:
    static MenuItemEx *FarList2MenuItem(const FarListItem *Item,MenuItemEx *ListItem);
    static FarListItem *MenuItem2FarList(const MenuItemEx *ListItem,FarListItem *Item);

    static LONG_PTR WINAPI DefMenuProc(HANDLE hVMenu,int Msg,int Param1,LONG_PTR Param2);
    static LONG_PTR WINAPI SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,LONG_PTR Param2);
};


#endif  // __VMENU_HPP__
