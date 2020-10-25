#ifndef EDITCONTROL_HPP_ECD19E42_9258_4A76_99C9_67FF54F11289
#define EDITCONTROL_HPP_ECD19E42_9258_4A76_99C9_67FF54F11289
#pragma once

/*
editcontrol.hpp

Надстройка над Edit.
Одиночная строка ввода для диалогов и комстроки (не для редактора)
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
#include "edit.hpp"
#include "farcolor.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class History;
class VMenu2;

class EditControl:public Edit
{
	struct Callback;
	using parent_processkey_t = std::function<int(const Manager::Key& Key)>;
public:
	EditControl(window_ptr Owner, SimpleScreenObject* Parent, parent_processkey_t&& ParentProcessKey = nullptr, Callback* aCallback = nullptr, History* iHistory = nullptr, FarList* iList = nullptr, DWORD iFlags = 0);

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	void Show() override;
	void Changed(bool DelBlock=false) override;
	int GetMaxLength() const override {return MaxLength;}

	void AutoComplete(bool Manual,bool DelBlock);
	void SetAutocomplete(bool State) {State? ECFlags.Set(EC_ENABLEAUTOCOMPLETE) : ECFlags.Clear(EC_ENABLEAUTOCOMPLETE);}
	bool GetAutocomplete() const {return ECFlags.Check(EC_ENABLEAUTOCOMPLETE) != 0;}
	void SetMacroAreaAC(FARMACROAREA Area){MacroAreaAC=Area;}
	void SetCallbackState(bool Enable){m_Callback.Active=Enable;}
	void SetObjectColor(PaletteColors Color = COL_DIALOGEDIT, PaletteColors SelColor = COL_DIALOGEDITSELECTED, PaletteColors ColorUnChanged=COL_DIALOGEDITUNCHANGED);
	void SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged);
	void GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged) const;
	int GetDropDownBox() const {return m_Flags.Check(FEDITLINE_DROPDOWNBOX);}
	void SetDropDownBox(bool NewDropDownBox) {m_Flags.Change(FEDITLINE_DROPDOWNBOX,NewDropDownBox);}
	void SetMaxLength(int Length) {MaxLength=Length;}
	void SetClearFlag(bool Flag) { m_Flags.Change(FEDITLINE_CLEARFLAG, Flag); }
	bool GetClearFlag() const { return m_Flags.Check(FEDITLINE_CLEARFLAG); }

	enum ECFLAGS
	{
		EC_ENABLEAUTOCOMPLETE                   = 0_bit,
		EC_COMPLETE_FILESYSTEM                  = 1_bit,
		EC_COMPLETE_PATH                        = 2_bit,
		EC_COMPLETE_HISTORY                     = 3_bit,
		EC_COMPLETE_ENVIRONMENT                 = 4_bit,
	};

protected:
	void RefreshStrByMask(int InitMode=FALSE) override;

private:
	friend class DlgEdit;

	const FarColor& GetNormalColor() const override;
	const FarColor& GetSelectedColor() const override;
	const FarColor& GetUnchangedColor() const override;
	size_t GetTabSize() const override;
	EXPAND_TABS GetTabExpandMode() const override;
	string GetInputMask() const override {return m_Mask;}
	void SetInputMask(string_view InputMask) override;
	const string& WordDiv() const override;
	int GetPrevCurPos() const override { return PrevCurPos; }
	void SetPrevCurPos(int Pos) override { PrevCurPos = Pos; }
	int GetCursorSize() const override { return CursorSize; }
	void SetCursorSize(size_t Size) override { CursorSize = static_cast<int>(Size); }
	int GetMacroSelectionStart() const override {return MacroSelectionStart;}
	void SetMacroSelectionStart(int Value) override {MacroSelectionStart = Value;}
	int GetLineCursorPos() const override {return CursorPos;}
	void SetLineCursorPos(int Value) override {CursorPos = Value;}
	void SuppressCallback() override { ++m_CallbackSuppressionsCount; }
	void RevertCallback() override { --m_CallbackSuppressionsCount; }

	void SetMenuPos(VMenu2& menu);
	int AutoCompleteProc(bool Manual,bool DelBlock,Manager::Key& BackKey, FARMACROAREA Area);

	struct Callback
	{
		bool Active;
		void (*m_Callback)(void* aParam);
		void* m_Param;
	};

	string m_Mask;
	History* pHistory;
	FarList* pList;

	FarColor m_Color;
	FarColor m_SelectedColor;
	FarColor m_UnchangedColor;
	parent_processkey_t m_ParentProcessKey;

	int MaxLength;
	int CursorSize;
	int CursorPos;
	int PrevCurPos; //Для определения направления передвижения курсора при наличии маски
	int MacroSelectionStart;
	int SelectionStart;
	FARMACROAREA MacroAreaAC;
	BitFlags ECFlags;
	Callback m_Callback;
	size_t m_CallbackSuppressionsCount;
	bool Selection;
	bool MenuUp;
	bool ACState;
};

#endif // EDITCONTROL_HPP_ECD19E42_9258_4A76_99C9_67FF54F11289
