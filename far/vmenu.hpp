﻿#ifndef VMENU_HPP_DF9F4258_12AF_4721_9D5F_BE29A59649C2
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

// Internal:
#include "modal.hpp"
#include "bitflags.hpp"
#include "farcolor.hpp"

// Platform:

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

// Цветовые атрибуты - индексы в массиве цветов
enum vmenu_colors
{
	VMenuColorBody                = 0,     // подложка
	VMenuColorBox                 = 1,     // рамка
	VMenuColorTitle               = 2,     // заголовок - верхний и нижний
	VMenuColorText                = 3,     // Текст пункта
	VMenuColorHighlight           = 4,     // HotKey
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
	VMENU_NONE                 = 0,
	VMENU_ALWAYSSCROLLBAR      = 8_bit,  // всегда показывать скроллбар
	VMENU_LISTBOX              = 9_bit,  // Это список в диалоге
	VMENU_SHOWNOBOX            = 10_bit, // показать без рамки
	VMENU_AUTOHIGHLIGHT        = 11_bit, // автоматически выбирать симолы подсветки
	VMENU_REVERSEHIGHLIGHT     = 12_bit, // ... только с конца
	VMENU_UPDATEREQUIRED       = 13_bit, // лист необходимо обновить (перерисовать)
	VMENU_DISABLEDRAWBACKGROUND= 14_bit, // подложку не рисовать
	VMENU_WRAPMODE             = 15_bit, // зацикленный список (при перемещении)
	VMENU_SHOWAMPERSAND        = 16_bit, // символ '&' показывать AS IS
	VMENU_WARNDIALOG           = 17_bit, //
	VMENU_LISTHASFOCUS         = 21_bit, // меню является списком в диалоге и имеет фокус
	VMENU_COMBOBOX             = 22_bit, // меню является комбобоксом и обрабатывается менеджером по-особому.
	VMENU_MOUSEDOWN            = 23_bit, //
	VMENU_CHANGECONSOLETITLE   = 24_bit, //
	VMENU_MOUSEREACTION        = 25_bit, // реагировать на движение мыши? (перемещать позицию при перемещении курсора мыши?)
	VMENU_DISABLED             = 26_bit, //
	VMENU_NOMERGEBORDER        = 27_bit, //
	VMENU_REFILTERREQUIRED     = 28_bit, // перед отрисовкой необходимо обновить фильтр
	VMENU_LISTSINGLEBOX        = 29_bit, // список, всегда с одинарной рамкой
	VMENU_COMBOBOXEVENTKEY     = 30_bit, // посылать события клавиатуры в диалоговую проц. для открытого комбобокса
	VMENU_COMBOBOXEVENTMOUSE   = 31_bit, // посылать события мыши в диалоговую проц. для открытого комбобокса
};

class Dialog;
class SaveScreen;

struct menu_item
{
	string Name;
	LISTITEMFLAGS Flags{};
	DWORD AccelKey{};

	menu_item() = default;

	explicit menu_item(string_view const Text):
		Name(Text)
	{
	}

	menu_item(string_view const Text, LISTITEMFLAGS const Flags, DWORD const AccelKey = 0):
		Name(Text),
		Flags(Flags),
		AccelKey(AccelKey)
	{
	}

	unsigned long long SetCheck()
	{
		Flags &= ~0xFFFF;
		Flags |= LIF_CHECKED;
		return Flags;
	}

	unsigned long long SetCustomCheck(wchar_t Char)
	{
		Flags &= ~0xFFFF;
		Flags |= LIF_CHECKED | Char;
		return Flags;
	}

	unsigned long long ClearCheck()
	{
		Flags &= ~0xFFFF;
		Flags &= ~LIF_CHECKED;
		return Flags;
	}

	LISTITEMFLAGS SetSelect(bool Value) { if (Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
	LISTITEMFLAGS SetDisable(bool Value) { if (Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
	LISTITEMFLAGS SetGrayed(bool Value) { if (Value) Flags|=LIF_GRAYED; else Flags&=~LIF_GRAYED; return Flags;}

};

struct MenuItemEx: menu_item
{
	NONCOPYABLE(MenuItemEx);
	MOVABLE(MenuItemEx);

	MenuItemEx() = default;
	using menu_item::menu_item;

	std::any ComplexUserData;
	intptr_t SimpleUserData{};

	size_t ShowPos{};
	wchar_t AutoHotkey{};
	size_t AutoHotkeyPos{};
	short Len[2]{};                  // размеры 2-х частей
	short Idx2{};                    // начало 2-й части
	std::list<std::pair<int, int>> Annotations;
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
	static vmenu_ptr create(string Title, span<menu_item const> Data, int MaxHeight = 0, DWORD Flags = 0, dialog_ptr ParentDialog = nullptr);

	VMenu(private_tag, string Title, int MaxHeight, dialog_ptr ParentDialog);
	~VMenu() override;

	void Show() override;
	void Hide() override;
	string GetTitle() const override;
	FARMACROAREA GetMacroArea() const override;
	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return CheckFlags(VMENU_COMBOBOX) ? windowtype_combobox : windowtype_menu; }
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void ResizeConsole() override;
	void SetDeleting() override;
	void ShowConsoleTitle() override;
	void OnClose() override;

	void FastShow() { ShowMenu(); }
	void ResetCursor();
	void SetTitle(string_view Title);
	void SetBottomTitle(string_view BottomTitle);
	string &GetBottomTitle(string &strDest) const;
	void SetDialogStyle(bool Style) { ChangeFlags(VMENU_WARNDIALOG, Style); SetColors(nullptr); }
	void SetUpdateRequired(bool SetUpdate) { ChangeFlags(VMENU_UPDATEREQUIRED, SetUpdate); }
	void SetBoxType(int BoxType);
	void SetMenuFlags(DWORD Flags) { VMFlags.Set(Flags); }
	void ClearFlags(DWORD Flags) { VMFlags.Clear(Flags); }
	bool CheckFlags(DWORD Flags) const { return VMFlags.Check(Flags); }
	DWORD GetFlags() const { return VMFlags.Flags(); }
	DWORD ChangeFlags(DWORD Flags, bool Status) { return VMFlags.Change(Flags, Status); }
	void AssignHighlights(bool Reverse = false);
	void SetColors(const FarDialogItemColors *ColorsIn = nullptr);
	void GetColors(FarDialogItemColors *ColorsOut);
	void SetOneColor(int Index, PaletteColors Color);
	bool ProcessFilterKey(int Key);
	void clear();
	int DeleteItem(int ID, int Count = 1);
	int AddItem(MenuItemEx&& NewItem, int PosAdd = std::numeric_limits<int>::max());
	int AddItem(const FarList *List);
	int AddItem(const wchar_t *NewStrItem);
	int InsertItem(const FarListInsert *NewItem);
	bool UpdateItem(const FarListUpdate *NewItem);
	int FindItem(const FarListFind *FItem);
	int FindItem(int StartIndex, string_view Pattern, unsigned long long Flags = 0);
	void RestoreFilteredItems();
	void FilterStringUpdated();
	void FilterUpdateHeight(bool bShrink = false);
	void SetFilterEnabled(bool bEnabled) { bFilterEnabled = bEnabled; }
	void SetFilterLocked(bool bLocked) { bFilterEnabled = bLocked; }
	bool AddToFilter(string_view Str);
	size_t size() const { return Items.size(); }
	bool empty() const { return Items.empty(); }
	// SelectPos == -1 & non-empty Items - everything is filtered
	bool HasVisible() const { return SelectPos > -1 && !Items.empty(); }
	int GetShowItemCount() const { return static_cast<int>(Items.size() - ItemHiddenCount); }
	int GetVisualPos(int Pos);
	int VisualPosToReal(int VPos);

	intptr_t GetSimpleUserData(int Position = -1) const;

	std::any* GetComplexUserData(int Position = -1);
	template<class T>
	T* GetComplexUserDataPtr(int Position = -1)
	{
		return std::any_cast<T>(GetComplexUserData(Position));
	}
	void SetComplexUserData(const std::any& Data, int Position = -1);

	int GetSelectPos() const { return SelectPos; }
	int GetLastSelectPosResult() const { return SelectPosResult; }
	int GetSelectPos(FarListPos *ListPos) const;
	int SetSelectPos(const FarListPos *ListPos, int Direct = 0);
	int SetSelectPos(int Pos, int Direct, bool stop_on_edge = false);
	int GetCheck(int Position = -1);
	void SetCheck(int Position = -1);
	void SetCustomCheck(wchar_t Char, int Position = -1);
	void ClearCheck(int Position = -1);
	bool UpdateRequired() const;
	void UpdateItemFlags(int Pos, unsigned long long NewFlags);
	MenuItemEx& at(size_t n);
	MenuItemEx& current() { return at(-1); }
	bool Pack();
	bool GetVMenuInfo(FarListInfo* Info) const;
	void SetMaxHeight(int NewMaxHeight);
	size_t GetVDialogItemID() const { return DialogItemID; }
	void SetVDialogItemID(size_t NewDialogItemID) { DialogItemID = NewDialogItemID; }
	void SetId(const UUID& Id);
	const UUID& Id() const;
	bool IsComboBox() const { return GetDialog() && CheckFlags(VMENU_COMBOBOX); }
	dialog_ptr GetDialog() const {return ParentDialog.lock();}

	void SortItems(bool Reverse = false, int Offset = 0);

	template<class predicate>
	void SortItems(predicate Pred, bool Reverse = false, int Offset = 0)
	{
		SortItemParam Param { Reverse, Offset };
		std::sort(ALL_RANGE(Items), [&](const auto& a, const auto& b) { return Pred(a, b, Param); });

		// скорректируем SelectPos
		UpdateSelectPos();

		SetMenuFlags(VMENU_UPDATEREQUIRED);
	}

	static FarListItem *MenuItem2FarList(const MenuItemEx *MItem, FarListItem *FItem);
	static std::vector<string> AddHotkeys(span<menu_item> MenuItems);

	size_t MaxItemLength() const;

private:
	void init(span<menu_item const> Data, DWORD Flags);

	void DisplayObject() override;

	void ShowMenu(bool IsParent = false);
	void DrawTitles() const;
	int GetItemPosition(int Position) const;
	bool CheckKeyHiOrAcc(DWORD Key, int Type, bool Translate, bool ChangePos, int& NewPos);
	int CheckHighlights(wchar_t CheckSymbol,int StartPos=0);
	wchar_t GetHighlights(const MenuItemEx *Item) const;
	bool ShiftItemShowPos(int Pos,int Direct);
	void UpdateMaxLengthFromTitles();
	void UpdateMaxLength(size_t Length);
	bool ShouldSendKeyToFilter(unsigned Key) const;
	//корректировка текущей позиции и флагов SELECTED
	void UpdateSelectPos();
	void EnableFilter(bool Enable);
	size_t GetServiceAreaSize();

	size_t Text(string_view Str) const;
	size_t Text(wchar_t Char) const;

	string strTitle;
	string strBottomTitle;
	int SelectPos{-1};
	int SelectPosResult{-1};
	int TopPos{};
	int MaxHeight;
	bool WasAutoHeight{};
	size_t m_MaxItemLength{};
	int m_BoxType;
	window_ptr CurrentWindow;
	bool PrevCursorVisible{};
	size_t PrevCursorSize{};
	// переменная, отвечающая за отображение scrollbar в DI_LISTBOX & DI_COMBOBOX
	BitFlags VMFlags;
	// Для LisBox - родитель в виде диалога
	std::weak_ptr<Dialog> ParentDialog;
	size_t DialogItemID{};
	bool bFilterEnabled{};
	bool bFilterLocked{};
	string strFilter;
	std::vector<MenuItemEx> Items;
	intptr_t ItemHiddenCount{};
	intptr_t ItemSubMenusCount{};
	FarColor Colors[VMENU_COLOR_COUNT]{};
	size_t MaxLineWidth{};
	bool bRightBtnPressed{};
	UUID MenuId;
};

#endif // VMENU_HPP_DF9F4258_12AF_4721_9D5F_BE29A59649C2
