#ifndef EDIT_HPP_5A787FA0_4FFF_4A61_811F_F8BAEDEF241B
#define EDIT_HPP_5A787FA0_4FFF_4A61_811F_F8BAEDEF241B
#pragma once

/*
edit.hpp

Строка редактора
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

#include "scrobj.hpp"
#include "bitflags.hpp"
#include "RegExp.hpp"

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITLINE
{
	FEDITLINE_MARKINGBLOCK         = 0x00000100,
	FEDITLINE_DROPDOWNBOX          = 0x00000200,
	FEDITLINE_CLEARFLAG            = 0x00000400,
	FEDITLINE_PASSWORDMODE         = 0x00000800,
	FEDITLINE_EDITBEYONDEND        = 0x00001000,
	FEDITLINE_EDITORMODE           = 0x00002000,
	FEDITLINE_OVERTYPE             = 0x00004000,
	FEDITLINE_DELREMOVESBLOCKS     = 0x00008000,  // Del удаляет блоки (Global->Opt->EditorDelRemovesBlocks)
	FEDITLINE_PERSISTENTBLOCKS     = 0x00010000,  // Постоянные блоки (Global->Opt->EditorPersistentBlocks)
	FEDITLINE_SHOWWHITESPACE       = 0x00020000,
	FEDITLINE_SHOWLINEBREAK        = 0x00040000,
	FEDITLINE_READONLY             = 0x00080000,
	FEDITLINE_CURSORVISIBLE        = 0x00100000,
	// Если ни один из FEDITLINE_PARENT_ не указан (или указаны оба), то Edit
	// явно не в диалоге юзается.
	FEDITLINE_PARENT_SINGLELINE    = 0x00200000,  // обычная строка ввода в диалоге
	FEDITLINE_PARENT_MULTILINE     = 0x00400000,  // для будущего Memo-Edit (DI_EDITOR или DIF_MULTILINE)
	FEDITLINE_PARENT_EDITOR        = 0x00800000,  // "вверху" обычный редактор
};

struct ColorItem
{
	// Usually we have only 1-2 coloring plugins.
	// Keeping a copy of GUID in each of thousands of color items is a giant waste of memory,
	// so GUIDs are stored in a separate set and here is only a pointer.
	const GUID* Owner;
	// Usually we have only 5-10 unique colors.
	// Keeping a copy of FarColor in each of thousands of color items is a giant waste of memory,
	// so FarColors are stored in a separate set and here is only a pointer.
	const FarColor* Color;
	unsigned int Priority;
	int StartPos;
	int EndPos;
	// it's an uint64 in plugin API, but only 0x1 and 0x2 are used now, so save some memory here.
	unsigned int Flags;

	const GUID& GetOwner() const { return *Owner; }
	void SetOwner(const GUID& Value);

	const FarColor& GetColor() const { return *Color; }
	void SetColor(const FarColor& Value);

	bool operator <(const ColorItem& rhs) const
	{
		return Priority < rhs.Priority;
	}
};

enum SetCPFlags
{
	SETCP_NOERROR    = 0x00000000,
	SETCP_WC2MBERROR = 0x00000001,
	SETCP_MB2WCERROR = 0x00000002,
	SETCP_OTHERERROR = 0x10000000,
};

class Editor;

class Edit: public SimpleScreenObject
{
	struct ShowInfo
	{
		int LeftPos;
		int CurTabPos;
	};
public:
	NONCOPYABLE(Edit);
	TRIVIALLY_MOVABLE(Edit);

	typedef std::function<bool(const ColorItem&)> delete_color_condition;

	Edit(window_ptr Owner);
	virtual ~Edit() = default;

	void FastShow(const ShowInfo* Info=nullptr);
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void Changed(bool DelBlock=false){};
	void SetDelRemovesBlocks(bool Mode) {m_Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Mode);}
	int GetDelRemovesBlocks() const {return m_Flags.Check(FEDITLINE_DELREMOVESBLOCKS); }
	void SetPersistentBlocks(bool Mode) {m_Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Mode);}
	int GetPersistentBlocks() const {return m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS); }
	void SetShowWhiteSpace(int Mode) {m_Flags.Change(FEDITLINE_SHOWWHITESPACE, Mode!=0); m_Flags.Change(FEDITLINE_SHOWLINEBREAK, Mode == 1);}

	const string& GetString() const { return m_Str; }

	void SetHiString(const string& Str);

	void SetEOL(const wchar_t *EOL);
	void SetEOL(const string& EOL) { return SetEOL(EOL.data()); }

	const wchar_t *GetEOL() const;
	string GetSelString() const;
	int GetLength() const;

	void SetString(const string& Str) { SetString(Str.data(), Str.size()); }
	void SetString(const wchar_t *Str, size_t Length);

	void InsertString(const string& Str) { InsertString(Str.data(), Str.size()); }
	void InsertString(const wchar_t *Str, size_t Length);

	void AppendString(const string& Str) { AppendString(Str.data(), Str.size()); }
	void AppendString(const wchar_t *Str, size_t Length);

	void ClearString() { SetString(L"", 0); }

	int Search(const string& Str,const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm, MatchHash* hm, string& ReplaceStr,int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength);
	void SetClearFlag(bool Flag) {m_Flags.Change(FEDITLINE_CLEARFLAG,Flag);}
	int GetClearFlag() const {return m_Flags.Check(FEDITLINE_CLEARFLAG);}
	void SetCurPos(int NewPos) {m_CurPos=NewPos; SetPrevCurPos(NewPos);}
	void AdjustMarkBlock();
	void AdjustPersistentMark();
	int GetCurPos() const { return m_CurPos; }
	int GetTabCurPos() const;
	void SetTabCurPos(int NewPos);
	int GetLeftPos() const {return LeftPos;}
	void SetLeftPos(int NewPos) {LeftPos=NewPos;}
	void SetPasswordMode(bool Mode) {m_Flags.Change(FEDITLINE_PASSWORDMODE,Mode);}
	// Получение максимального значения строки для потребностей Dialod API
	virtual int GetMaxLength() const {return -1;}
	void SetOvertypeMode(bool Mode) {m_Flags.Change(FEDITLINE_OVERTYPE, Mode);}
	bool GetOvertypeMode() const {return m_Flags.Check(FEDITLINE_OVERTYPE);}
	int RealPosToTab(int Pos) const;
	int TabPosToReal(int Pos) const;
	void Select(int Start,int End);
	void RemoveSelection();
	void AddSelect(int Start,int End);
	void GetSelection(intptr_t &Start,intptr_t &End) const;
	bool IsSelection() const {return !(m_SelStart==-1 && !m_SelEnd); }
	void GetRealSelection(intptr_t &Start,intptr_t &End) const;
	void SetEditBeyondEnd(bool Mode) {m_Flags.Change(FEDITLINE_EDITBEYONDEND, Mode);}
	void SetEditorMode(bool Mode) {m_Flags.Change(FEDITLINE_EDITORMODE, Mode);}
	bool ReplaceTabs();
	void InsertTab();
	void AddColor(const ColorItem& col);
	void DeleteColor(const delete_color_condition& Condition);
	bool GetColor(ColorItem& col, size_t Item) const;
	void Xlat(bool All=false);
	void SetDialogParent(DWORD Sets);
	void SetCursorType(bool Visible, DWORD Size);
	void GetCursorType(bool& Visible, DWORD& Size) const;
	bool GetReadOnly() const {return m_Flags.Check(FEDITLINE_READONLY);}
	void SetReadOnly(bool NewReadOnly) {m_Flags.Change(FEDITLINE_READONLY,NewReadOnly);}
	void SetHorizontalPosition(int X1, int X2) {SetPosition(X1, m_Y2, X2, m_Y2);}

protected:
	virtual void RefreshStrByMask(int InitMode=FALSE) {}

	auto CallbackSuppressor() { return make_raii_wrapper(this, &Edit::SuppressCallback, &Edit::RevertCallback); }

	void DeleteBlock();

	static int CheckCharMask(wchar_t Chr);

private:
	virtual void SuppressCallback() {}
	virtual void RevertCallback() {}

	virtual void DisplayObject() override;
	virtual const FarColor& GetNormalColor() const;
	virtual const FarColor& GetSelectedColor() const;
	virtual const FarColor& GetUnchangedColor() const;
	virtual size_t GetTabSize() const;
	virtual EXPAND_TABS GetTabExpandMode() const;
	virtual void SetInputMask(const string& InputMask) {}
	virtual string GetInputMask() const {return {};}
	virtual const string& WordDiv() const;
	virtual int GetPrevCurPos() const { return 0; }
	virtual void SetPrevCurPos(int Pos) {}
	virtual int GetCursorSize() const;
	virtual void SetCursorSize(int Size) {}
	virtual int GetMacroSelectionStart() const;
	virtual void SetMacroSelectionStart(int Value);
	virtual int GetLineCursorPos() const;
	virtual void SetLineCursorPos(int Value);

	int InsertKey(int Key);
	int RecurseProcessKey(int Key);
	void ApplyColor(const FarColor& SelColor, int XPos, int FocusedLeftPos);
	int GetNextCursorPos(int Position,int Where) const;
	int KeyMatchedMask(int Key, const string& Mask) const;
	int ProcessCtrlQ();
	int ProcessInsPath(unsigned int Key,int PrevSelStart=-1,int PrevSelEnd=0);
	int RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos) const;
	void FixLeftPos(int TabCurPos=-1);
	void SetRightCoord(int Value) {SetPosition(m_X1, m_Y2, Value, m_Y2);}
	Editor* GetEditor(void)const;

protected:
	// BUGBUG: the whole purpose of this class is to avoid zillions of casts in existing code by returning size() as int
	// Remove it after fixing all signed/unsigned mess
	class edit_string: public string
	{
	public:
		edit_string() {}
		edit_string(const wchar_t* Data, size_t Size): string(Data, Size) {}

		int size() const { return static_cast<int>(string::size()); }
	};
	edit_string m_Str;

	// KEEP ALIGNED!
	int m_CurPos;
private:
	friend class DlgEdit;
	friend class Editor;
	friend class FileEditor;

	// KEEP ALIGNED!
	std::multiset<ColorItem> ColorList;
	int m_SelStart;
	int m_SelEnd;
	int LeftPos;
	unsigned char EndType;
};

#endif // EDIT_HPP_5A787FA0_4FFF_4A61_811F_F8BAEDEF241B
