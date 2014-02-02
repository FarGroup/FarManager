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

#include "scrobj.hpp"
#include "bitflags.hpp"
#include "editcontrol.hpp"
#include "editor.hpp"

enum DLGEDITTYPE
{
	DLGEDIT_MULTILINE,
	DLGEDIT_SINGLELINE,
};

class Dialog;

class DlgEdit: public ScreenObject
{
public:
	// for CtrlEnd
	string strLastStr;
	int LastPartLength;

	BitFlags& Flags();

	DlgEdit(Dialog* pOwner,size_t Index,DLGEDITTYPE Type);
	virtual ~DlgEdit();

	virtual int  ProcessKey(int Key) override;
	virtual int  ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;

	virtual void Show() override;
	virtual void SetPosition(int X1,int Y1,int X2,int Y2) override;
	virtual void GetPosition(int& X1,int& Y1,int& X2,int& Y2) const override;

	virtual void Hide() override;
	virtual void Hide0() override;
	virtual void ShowConsoleTitle() override;
	virtual void SetScreenPosition() override;
	virtual void ResizeConsole() override;
	virtual __int64  VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;

	void  SetDialogParent(DWORD Sets);
	void  SetDropDownBox(bool NewDropDownBox);
	void  SetPasswordMode(bool Mode);

	int   GetMaxLength() const;
	void  SetMaxLength(int Length);
	int   GetLength() const;
	int   GetStrSize(int Row = -1) const;

	void  SetInputMask(const string& InputMask);
	const wchar_t* GetInputMask() const;

	void  SetOvertypeMode(bool Mode);
	bool  GetOvertypeMode() const;

	void  SetEditBeyondEnd(bool Mode);

	void  SetClearFlag(bool Flag);
	int   GetClearFlag() const;

	void  Changed();
	void  SetString(const string& Str, bool disable_autocomplete = false, int pos = -1);
	void  InsertString(const string& Str);
	void  SetHiString(const string& Str);
	void  GetString(wchar_t *Str, int MaxSize, int Row = -1) const; // Row==-1 - current line
	void  GetString(string &strStr, int Row = -1) const;            // Row==-1 - current line
	const wchar_t* GetStringAddr() const;

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
	void  GetSelection(intptr_t &Start, intptr_t &End) const;   // TODO: не учтено для multiline!

	void Xlat(bool All=false);

	void SetCursorType(bool Visible, DWORD Size);
	void GetCursorType(bool& Visible, DWORD& Size) const;

	bool  GetReadOnly() const;
	void SetReadOnly(bool NewReadOnly);

	void SetCallbackState(bool Enable){lineEdit->SetCallbackState(Enable);}
	void AutoComplete(bool Manual,bool DelBlock){return lineEdit->AutoComplete(Manual,DelBlock);}

	bool HistoryGetSimilar(string &strStr, int LastCmdPartLength, bool bAppend=false);

	History* GetHistory() const {return iHistory;}
	void SetHistory(const string& Name);

private:
	Dialog* m_Dialog;
	size_t m_Index;
	DLGEDITTYPE Type;
	History* iHistory;

	EditControl   *lineEdit;
#if defined(PROJECT_DI_MEMOEDIT)
	Editor *multiEdit;
#endif

	virtual void DisplayObject() override;
	static void EditChange(void* aParam);
	void DoEditChange();
	friend class SetAutocomplete;
};
