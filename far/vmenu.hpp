#ifndef VMENU_HPP_DF9F4258_12AF_4721_9D5F_BE29A59649C2
#define VMENU_HPP_DF9F4258_12AF_4721_9D5F_BE29A59649C2
#pragma once

/*
vmenu.hpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "bitflags.hpp"
#include "farcolor.hpp"

// Цветовые атрибуты - индексы в массиве цветов
enum vmenu_colors
{
	VMenuColorBody                = 0,     // подложка
	VMenuColorBox                 = 1,     // рамка
	VMenuColorTitle               = 2,     // заголовок - верхний и нижний
	VMenuColorText                = 3,     // Текст пункта
	VMenuColorHilite              = 4,     // HotKey
	VMenuColorSeparator           = 5,     // separator
	VMenuColorSelected            = 6,     // Выбранный
	VMenuColorHSelect             = 7,     // Выбранный - HotKey
	VMenuColorScrollBar           = 8,     // ScrollBar
	VMenuColorDisabled            = 9,     // Disabled
	VMenuColorArrows              =10,     // '<' & '>' обычные
	VMenuColorArrowsSelect        =11,     // '<' & '>' выбранные
	VMenuColorArrowsDisabled      =12,     // '<' & '>' Disabled
	VMenuColorGrayed              =13,     // "серый"
	VMenuColorSelGrayed           =14,     // выбранный "серый"

	VMENU_COLOR_COUNT,                     // всегда последняя - размерность массива
};

enum VMENU_FLAGS
{
	VMENU_NONE                 =0x00000000,
	VMENU_ALWAYSSCROLLBAR      =0x00000100, // всегда показывать скроллбар
	VMENU_LISTBOX              =0x00000200, // Это список в диалоге
	VMENU_SHOWNOBOX            =0x00000400, // показать без рамки
	VMENU_AUTOHIGHLIGHT        =0x00000800, // автоматически выбирать симолы подсветки
	VMENU_REVERSEHIGHLIGHT     =0x00001000, // ... только с конца
	VMENU_UPDATEREQUIRED       =0x00002000, // лист необходимо обновить (перерисовать)
	VMENU_DISABLEDRAWBACKGROUND=0x00004000, // подложку не рисовать
	VMENU_WRAPMODE             =0x00008000, // зацикленный список (при перемещении)
	VMENU_SHOWAMPERSAND        =0x00010000, // символ '&' показывать AS IS
	VMENU_WARNDIALOG           =0x00020000, //
	VMENU_LISTHASFOCUS         =0x00200000, // меню является списком в диалоге и имеет фокус
	VMENU_COMBOBOX             =0x00400000, // меню является комбобоксом и обрабатывается менеджером по-особому.
	VMENU_MOUSEDOWN            =0x00800000, //
	VMENU_CHANGECONSOLETITLE   =0x01000000, //
	VMENU_MOUSEREACTION        =0x02000000, // реагировать на движение мыши? (перемещать позицию при перемещении курсора мыши?)
	VMENU_DISABLED             =0x04000000, //
	VMENU_NOMERGEBORDER        =0x08000000, //
	VMENU_REFILTERREQUIRED     =0x10000000, // перед отрисовкой необходимо обновить фильтр
	VMENU_LISTSINGLEBOX        =0x20000000, // список, всегда с одинарной рамкой
	VMENU_COMBOBOXEVENTKEY     =0x40000000, // посылать события клавиатуры в диалоговую проц. для открытого комбобокса
	VMENU_COMBOBOXEVENTMOUSE   =0x80000000, // посылать события мыши в диалоговую проц. для открытого комбобокса
};

class Dialog;
class SaveScreen;


struct MenuItemEx
{
	NONCOPYABLE(MenuItemEx);
	MOVABLE(MenuItemEx);

	MenuItemEx(const string& Text = {}):
		strName(Text),
		Flags(),
		ShowPos(),
		AccelKey(),
		AmpPos(),
		Len(),
		Idx2()
	{
	}

	string strName;
	unsigned long long  Flags;                  // Флаги пункта
	any UserData;
	int   ShowPos;
	DWORD  AccelKey;
	short AmpPos;                  // Позиция автоназначенной подсветки
	short Len[2];                  // размеры 2-х частей
	short Idx2;                    // начало 2-й части
	std::list<std::pair<int, int>> Annotations;

	unsigned long long SetCheck(int Value)
	{
		if (Value)
		{
			Flags|=LIF_CHECKED;
			Flags &= ~0xFFFF;

			if (Value!=1) Flags|=Value&0xFFFF;
		}
		else
		{
			Flags&=~(0xFFFF|LIF_CHECKED);
		}

		return Flags;
	}

	unsigned long long SetSelect(int Value) { if (Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
	unsigned long long SetDisable(int Value) { if (Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
};


struct MenuDataEx
{
	const wchar_t *Name;

	LISTITEMFLAGS Flags;
	DWORD AccelKey;

	DWORD SetCheck(int Value)
	{
		if (Value)
		{
			Flags &= ~0xFFFF;
			Flags|=((Value&0xFFFF)|LIF_CHECKED);
		}
		else
			Flags&=~(0xFFFF|LIF_CHECKED);

		return Flags;
	}

	DWORD SetSelect(int Value) { if (Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
	DWORD SetDisable(int Value) { if (Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
	DWORD SetGrayed(int Value) { if (Value) Flags|=LIF_GRAYED; else Flags&=~LIF_GRAYED; return Flags;}
};

struct SortItemParam
{
	bool Reverse;
	int Offset;
};

class window;

class VMenu: public SimpleModal
{
	struct private_tag {};
public:
	static vmenu_ptr create(const string& Title, const MenuDataEx *Data, int ItemCount, int MaxHeight = 0, DWORD Flags = 0, dialog_ptr ParentDialog = nullptr);

	VMenu(private_tag, const string& Title, int MaxHeight, dialog_ptr ParentDialog);
	virtual ~VMenu() override;

	virtual void Show() override;
	virtual void Hide() override;
	virtual string GetTitle() const override;
	virtual FARMACROAREA GetMacroArea() const override;
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return CheckFlags(VMENU_COMBOBOX) ? windowtype_combobox : windowtype_menu; }
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	virtual void ResizeConsole() override;
	virtual void SetDeleting(void) override;
	virtual void ShowConsoleTitle() override;

	void FastShow() { ShowMenu(); }
	void ResetCursor();
	void SetTitle(const string& Title);
	void SetBottomTitle(const wchar_t *BottomTitle);
	string &GetBottomTitle(string &strDest) const;
	void SetDialogStyle(bool Style) { ChangeFlags(VMENU_WARNDIALOG, Style); SetColors(nullptr); }
	void SetUpdateRequired(bool SetUpdate) { ChangeFlags(VMENU_UPDATEREQUIRED, SetUpdate); }
	void SetBoxType(int BoxType);
	void SetMenuFlags(DWORD Flags) { VMFlags.Set(Flags); }
	void ClearFlags(DWORD Flags) { VMFlags.Clear(Flags); }
	bool CheckFlags(DWORD Flags) const { return VMFlags.Check(Flags); }
	DWORD GetFlags() const { return VMFlags.Flags(); }
	DWORD ChangeFlags(DWORD Flags, bool Status) { return VMFlags.Change(Flags, Status); }
	void AssignHighlights(int Reverse);
	void SetColors(const FarDialogItemColors *ColorsIn = nullptr);
	void GetColors(FarDialogItemColors *ColorsOut);
	void SetOneColor(int Index, PaletteColors Color);
	int ProcessFilterKey(int Key);
	void clear();
	int DeleteItem(int ID, int Count = 1);
	int AddItem(MenuItemEx&& NewItem, int PosAdd = 0x7FFFFFFF);
	int AddItem(const FarList *NewItem);
	int AddItem(const wchar_t *NewStrItem);
	int InsertItem(const FarListInsert *NewItem);
	int UpdateItem(const FarListUpdate *NewItem);
	int FindItem(const FarListFind *FindItem);
	int FindItem(int StartIndex, const string& Pattern, unsigned long long Flags = 0);
	void RestoreFilteredItems();
	void FilterStringUpdated();
	void FilterUpdateHeight(bool bShrink = false);
	void SetFilterEnabled(bool bEnabled) { bFilterEnabled = bEnabled; }
	void SetFilterLocked(bool bLocked) { bFilterEnabled = bLocked; }
	bool AddToFilter(const wchar_t *str);
	void SetFilterString(const wchar_t *str);
	size_t size() const { return Items.size(); }
	bool empty() const { return Items.empty(); }
	int GetShowItemCount() const { return static_cast<int>(Items.size() - ItemHiddenCount); }
	int GetVisualPos(int Pos);
	int VisualPosToReal(int VPos);
	template<class T>
	T* GetUserDataPtr(int Position = -1)
	{
		return any_cast<T>(GetUserData(Position));
	}
	void SetUserData(const any& Data, int Position = -1);
	int GetSelectPos() const { return SelectPos; }
	int GetLastSelectPosResult() const { return SelectPosResult; }
	int GetSelectPos(FarListPos *ListPos) const;
	int SetSelectPos(const FarListPos *ListPos, int Direct = 0);
	int SetSelectPos(int Pos, int Direct, bool stop_on_edge = false);
	int GetCheck(int Position = -1);
	void SetCheck(int Check, int Position = -1);
	bool UpdateRequired() const;
	void UpdateItemFlags(int Pos, unsigned long long NewFlags);
	MenuItemEx& at(size_t n);
	MenuItemEx& current() { return at(-1); }
	void SortItems(bool Reverse = false, int Offset = 0);
	bool Pack();
	bool GetVMenuInfo(FarListInfo* Info) const;
	void SetMaxHeight(int NewMaxHeight);
	size_t GetVDialogItemID() const { return DialogItemID; }
	void SetVDialogItemID(size_t NewDialogItemID) { DialogItemID = NewDialogItemID; }
	void SetId(const GUID& Id);
	const GUID& Id() const;
	bool IsComboBox() const { return GetDialog() && CheckFlags(VMENU_COMBOBOX); }
	dialog_ptr GetDialog() const {return ParentDialog.lock();}

	template<class predicate>
	void SortItems(predicate Pred, bool Reverse = false, int Offset = 0)
	{
		SortItemParam Param { Reverse, Offset };
		std::sort(ALL_RANGE(Items), [&](const auto& a, const auto& b) { return Pred(a, b, Param); });

		// скорректируем SelectPos
		UpdateSelectPos();

		SetMenuFlags(VMENU_UPDATEREQUIRED);
	}

	static FarListItem *MenuItem2FarList(const MenuItemEx *ListItem, FarListItem *Item);
	static std::vector<string> AddHotkeys(const range<MenuDataEx*>& MenuItems);

private:
	void init(const MenuDataEx *Data, int ItemsCount, DWORD Flags);

	virtual void DisplayObject() override;

	void ShowMenu(bool IsParent = false);
	void DrawTitles() const;
	int GetItemPosition(int Position) const;
	bool CheckKeyHiOrAcc(DWORD Key,int Type,int Translate,bool ChangePos,int& NewPos);
	int CheckHighlights(wchar_t Chr,int StartPos=0);
	wchar_t GetHighlights(const MenuItemEx *_item) const;
	bool ShiftItemShowPos(int Pos,int Direct);
	bool ItemCanHaveFocus(unsigned long long Flags) const;
	bool ItemCanBeEntered(unsigned long long Flags) const;
	bool ItemIsVisible(unsigned long long Flags) const;
	void UpdateMaxLengthFromTitles();
	void UpdateMaxLength(size_t Length);
	void UpdateInternalCounters(unsigned long long OldFlags, unsigned long long NewFlags);
	bool ShouldSendKeyToFilter(int Key) const;
	//корректировка текущей позиции и флагов SELECTED
	void UpdateSelectPos();
	any* GetUserData(int Position = -1);

	string strTitle;
	string strBottomTitle;
	int SelectPos;
	int SelectPosResult;
	int TopPos;
	int MaxHeight;
	bool WasAutoHeight;
	size_t m_MaxLength;
	int m_BoxType;
	window_ptr CurrentWindow;
	bool PrevCursorVisible;
	DWORD PrevCursorSize;
	// переменная, отвечающая за отображение scrollbar в DI_LISTBOX & DI_COMBOBOX
	BitFlags VMFlags;
	BitFlags VMOldFlags;
	// Для LisBox - родитель в виде диалога
	std::weak_ptr<Dialog> ParentDialog;
	size_t DialogItemID;
	bool bFilterEnabled;
	bool bFilterLocked;
	string strFilter;
	std::vector<MenuItemEx> Items;
	intptr_t ItemHiddenCount;
	intptr_t ItemSubMenusCount;
	FarColor Colors[VMENU_COLOR_COUNT];
	size_t MaxLineWidth;
	bool bRightBtnPressed;
	GUID MenuId;
};

#endif // VMENU_HPP_DF9F4258_12AF_4721_9D5F_BE29A59649C2
