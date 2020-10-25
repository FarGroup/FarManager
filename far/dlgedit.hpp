#ifndef DLGEDIT_HPP_976E81C0_DB62_4FC2_8FFD_73529F28E044
#define DLGEDIT_HPP_976E81C0_DB62_4FC2_8FFD_73529F28E044
#pragma once

/*
dlgedit.hpp

Одиночная строка редактирования для диалога (как наследник класса Edit)
Мультиредактор
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

/*
  Сюда нужно перетащить из edit.hpp и editor.hpp все вещи,
  касаемые масок и.. все что относится только к диалогам
  Это пока только шаблон, заготовка для будущего перехода
*/

// Internal:
#include "scrobj.hpp"
#include "farcolor.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

struct FarColor;
class History;
class EditControl;

enum DLGEDITTYPE
{
	DLGEDIT_MULTILINE,
	DLGEDIT_SINGLELINE,
};

class Dialog;
class Editor;

class DlgEdit: public SimpleScreenObject
{
public:
	// for CtrlEnd
	string strLastStr;
	int LastPartLength;

	BitFlags& Flags() const;

	DlgEdit(window_ptr Owner,size_t Index,DLGEDITTYPE Type);
	~DlgEdit() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;

	void Show() override;
	void SetPosition(rectangle Where) override;
	rectangle GetPosition() const override;

	void Hide() override;
	void ShowConsoleTitle() override;
	void SetScreenPosition() override;
	void ResizeConsole() override;
	long long VMProcess(int OpCode, void *vParam = nullptr, long long iParam = 0) override;

	void  SetDialogParent(DWORD Sets);
	void  SetDropDownBox(bool NewDropDownBox);
	void  SetPasswordMode(bool Mode);

	int   GetMaxLength() const;
	void  SetMaxLength(int Length);
	int   GetLength() const;
	int   GetStrSize(int Row = -1) const;

	void  SetInputMask(string_view InputMask);
	string GetInputMask() const;

	void  SetOvertypeMode(bool Mode);
	bool  GetOvertypeMode() const;

	void  SetEditBeyondEnd(bool Mode);

	void  SetClearFlag(bool Flag);
	bool  GetClearFlag() const;

	void  Changed();
	void  SetString(string_view Str);
	void  InsertString(string_view Str);
	void  SetHiString(string_view Str);
	const string& GetString(int Row = -1) const;            // Row==-1 - current line

	void  SetCurPos(int NewCol, int NewRow=-1); // Row==-1 - current line
	int   GetCurPos() const;
	int   GetCurRow() const;

	int   GetTabCurPos() const;
	void  SetTabCurPos(int NewPos);

	void  SetPersistentBlocks(bool Mode);
	int   GetPersistentBlocks() const;
	void  SetDelRemovesBlocks(bool NewMode);
	int   GetDelRemovesBlocks() const;

	void  SetObjectColor(PaletteColors Color,PaletteColors SelColor=COL_COMMANDLINESELECTED,PaletteColors ColorUnChanged=COL_DIALOGEDITUNCHANGED);
	void  SetObjectColor(const FarColor& Color,const FarColor& SelColor,const FarColor& ColorUnChanged);
	void  GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged) const;

	void  FastShow();
	int   GetLeftPos() const;
	void  SetLeftPos(int NewPos,int Row=-1); // Row==-1 - current line

	void  DeleteBlock();

	void  Select(int Start,int End);           // TODO: не учтено для multiline!
	void  RemoveSelection();                   // TODO: не учтено для multiline!
	void  GetSelection(intptr_t &Start, intptr_t &End) const;   // TODO: не учтено для multiline!

	void Xlat(bool All=false);

	void SetCursorType(bool Visible, size_t Size);
	void GetCursorType(bool& Visible, size_t& Size) const;

	bool GetReadOnly() const;
	void SetReadOnly(bool NewReadOnly);

	void SetCallbackState(bool Enable);
	void AutoComplete(bool Manual, bool DelBlock);

	bool HistoryGetSimilar(string &strStr, int LastCmdPartLength, bool bAppend=false) const;

	const std::unique_ptr<History>& GetHistory() const { return iHistory; }
	void SetHistory(string_view Name);

private:
	friend class SetAutocomplete;

	void DisplayObject() override;
	static void EditChange(void* aParam);
	void DoEditChange() const;
	Dialog* GetDialog() const;

	size_t m_Index;
	DLGEDITTYPE Type;
	std::unique_ptr<History> iHistory;
	std::unique_ptr<EditControl> lineEdit;
#if defined(PROJECT_DI_MEMOEDIT)
	Editor *multiEdit;
#endif
};

#endif // DLGEDIT_HPP_976E81C0_DB62_4FC2_8FFD_73529F28E044
