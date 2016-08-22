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

#include "edit.hpp"
#include "farcolor.hpp"

class History;
class VMenu2;

class EditControl:public Edit
{
	struct Callback;
	typedef std::function<int(const Manager::Key& Key)> parent_processkey_t;
public:
	EditControl(window_ptr Owner, SimpleScreenObject* Parent, parent_processkey_t&& ParentProcessKey = nullptr, Callback* aCallback = nullptr, History* iHistory = nullptr, FarList* iList = nullptr, DWORD iFlags = 0);
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void Show() override;
	virtual void Changed(bool DelBlock=false) override;

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
	virtual int GetMaxLength() const override {return MaxLength;}
	void SetMaxLength(int Length) {MaxLength=Length;}
	void SetClearFlag(bool Flag) { m_Flags.Change(FEDITLINE_CLEARFLAG, Flag); }
	int GetClearFlag() const { return m_Flags.Check(FEDITLINE_CLEARFLAG); }

	enum ECFLAGS
	{
		EC_ENABLEAUTOCOMPLETE                   = bit(0),
		EC_COMPLETE_FILESYSTEM                  = bit(1),
		EC_COMPLETE_PATH                        = bit(2),
		EC_COMPLETE_HISTORY                     = bit(3),
		EC_COMPLETE_ENVIRONMENT                 = bit(4),
	};

protected:
	virtual void RefreshStrByMask(int InitMode=FALSE) override;

private:
	friend class DlgEdit;

	virtual const FarColor& GetNormalColor() const override;
	virtual const FarColor& GetSelectedColor() const override;
	virtual const FarColor& GetUnchangedColor() const override;
	virtual size_t GetTabSize() const override;
	virtual EXPAND_TABS GetTabExpandMode() const override;
	virtual string GetInputMask() const override {return m_Mask;}
	virtual void SetInputMask(const string& InputMask) override;
	virtual const string& WordDiv() const override;
	virtual int GetPrevCurPos() const override { return PrevCurPos; }
	virtual void SetPrevCurPos(int Pos) override { PrevCurPos = Pos; }
	virtual int GetCursorSize() const override { return CursorSize; }
	virtual void SetCursorSize(int Size) override { CursorSize = Size; }
	virtual int GetMacroSelectionStart() const override {return MacroSelectionStart;}
	virtual void SetMacroSelectionStart(int Value) override {MacroSelectionStart = Value;}
	virtual int GetLineCursorPos() const override {return CursorPos;}
	virtual void SetLineCursorPos(int Value) override {CursorPos = Value;}
	virtual void SuppressCallback() override { ++m_CallbackSuppressionsCount; }
	virtual void RevertCallback() override { --m_CallbackSuppressionsCount; }

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
