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

class History;
class VMenu2;

class EditControl:public Edit
{
	struct Callback;
public:
	EditControl(ScreenObject *pOwner, Callback* aCallback=nullptr,bool bAllocateData=true,History* iHistory=0,FarList* iList=0,DWORD iFlags=0);
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void Show() override;
	virtual void Changed(bool DelBlock=false) override;

	void AutoComplete(bool Manual,bool DelBlock);
	void SetAutocomplete(bool State) {State? ECFlags.Set(EC_ENABLEAUTOCOMPLETE) : ECFlags.Clear(EC_ENABLEAUTOCOMPLETE);}
	bool GetAutocomplete() {return ECFlags.Check(EC_ENABLEAUTOCOMPLETE) != 0;}
	void SetMacroAreaAC(FARMACROAREA Area){MacroAreaAC=Area;}
	void SetCallbackState(bool Enable){m_Callback.Active=Enable;}
	void SetObjectColor(PaletteColors Color = COL_DIALOGEDIT, PaletteColors SelColor = COL_DIALOGEDITSELECTED, PaletteColors ColorUnChanged=COL_DIALOGEDITUNCHANGED);
	void SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged);
	void GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged);
	int GetDropDownBox() {return Flags.Check(FEDITLINE_DROPDOWNBOX);}
	void SetDropDownBox(bool NewDropDownBox) {Flags.Change(FEDITLINE_DROPDOWNBOX,NewDropDownBox);}
	virtual int GetMaxLength() const override {return MaxLength;}
	void SetMaxLength(int Length) {MaxLength=Length;}

	enum ECFLAGS
	{
		EC_ENABLEAUTOCOMPLETE  = 0x1,
		EC_COMPLETE_FILESYSTEM = 0x2,
		EC_COMPLETE_PATH       = 0x4,
		EC_COMPLETE_HISTORY    = 0x8,
	};

protected:
	virtual void RefreshStrByMask(int InitMode=FALSE);

private:
	virtual const FarColor& GetNormalColor() const override;
	virtual const FarColor& GetSelectedColor() const override;
	virtual const FarColor& GetUnchangedColor() const override;
	virtual const int GetTabSize() const override;
	virtual const EXPAND_TABS GetTabExpandMode() const override;
	virtual const string GetInputMask() const override {return Mask;}
	virtual const void SetInputMask(const string& InputMask) override;
	virtual const string& WordDiv() const override;
	virtual int GetPrevCurPos() const override { return PrevCurPos; }
	virtual void SetPrevCurPos(int Pos) override { PrevCurPos = Pos; }
	virtual int GetCursorSize() override { return CursorSize; }
	virtual void SetCursorSize(int Size) override { CursorSize = Size; }
	virtual int GetMacroSelectionStart() const override {return MacroSelectionStart;}
	virtual void SetMacroSelectionStart(int Value) override {MacroSelectionStart = Value;}
	virtual int GetLineCursorPos() const override {return CursorPos;}
	virtual void SetLineCursorPos(int Value) override {CursorPos = Value;}
	virtual void DisableCallback() override
	{
		CallbackSaveState = m_Callback.Active;
		m_Callback.Active = false;
	}
	virtual void RevertCallback() override
	{
		m_Callback.Active = CallbackSaveState;
	};

	void SetMenuPos(VMenu2& menu);
	int AutoCompleteProc(bool Manual,bool DelBlock,int& BackKey, FARMACROAREA Area);

	struct Callback
	{
		bool Active;
		void (*m_Callback)(void* aParam);
		void* m_Param;
	};

	string Mask;
	History* pHistory;
	FarList* pList;

	FarColor Color;
	FarColor SelColor;
	FarColor ColorUnChanged;

	int MaxLength;
	int CursorSize;
	int CursorPos;
	int PrevCurPos; //Для определения направления передвижения курсора при наличии маски
	int MacroSelectionStart;
	int SelectionStart;
	FARMACROAREA MacroAreaAC;
	BitFlags ECFlags;
	Callback m_Callback;
	bool Selection;
	bool MenuUp;
	bool ACState;
	bool CallbackSaveState;

	friend class DlgEdit;
};
