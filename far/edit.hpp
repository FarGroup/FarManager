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
#include "colors.hpp"
#include "bitflags.hpp"
#include "macro.hpp"
#include "mix.hpp"

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
	FEDITLINE_CMP_CHANGED          = 0x01000000,
};

struct ColorItem
{
	// Usually we have only 1-2 coloring plugins.
	// Keeping a copy of GUID in each of thousands of color items is a giant waste of memory,
	// so GUID's are stored in separate set and here is only a pointer.
	const void* Owner;
	// Usually we have only 5-10 unique colors.
	// Keeping a copy of FarColor in each of thousands of color items is a giant waste of memory,
	// so FarColor's are stored in separate set and here is only a pointer.
	const void* Color;
	unsigned int Priority;
	int StartPos;
	int EndPos;
	// it's an uint64 in plugin API, but only 0x1 and 0x2 are used now, so save some memory here.
	unsigned int Flags;

	const GUID& GetOwner() const;
	void SetOwner(const GUID& Value);

	const FarColor& GetColor() const;
	void SetColor(const FarColor& Value);

	bool operator <(const ColorItem& rhs) const
	{
		return Priority < rhs.Priority;
	}
};

class sets
{
public:
	std::unordered_set<FarColor, color_hash> ColorSet;
	std::unordered_set<GUID, uuid_hash, uuid_equal> GuidSet;
};

enum SetCPFlags
{
	SETCP_NOERROR    = 0x00000000,
	SETCP_WC2MBERROR = 0x00000001,
	SETCP_MB2WCERROR = 0x00000002,
	SETCP_OTHERERROR = 0x10000000,
};

class RegExp;

class Edit:public ScreenObject
{
	enum EDITCOLORLISTFLAGS
	{
		ECLF_NEEDSORT = 0x1,
		ECLF_NEEDFREE = 0x2,
	};
public:
	Edit(ScreenObject *pOwner = nullptr, bool bAllocateData = true);
	Edit(Edit&& rhs);
	virtual ~Edit();

	MOVE_OPERATOR_BY_SWAP(Edit);

	void swap(Edit& rhs) noexcept
	{
		ScreenObject::swap(rhs);
		std::swap(Str, rhs.Str);
		std::swap(StrSize, rhs.StrSize);
		std::swap(CurPos, rhs.CurPos);
		std::swap(ColorList, rhs.ColorList);
		std::swap(ColorCount, rhs.ColorCount);
		std::swap(MaxColorCount, rhs.MaxColorCount);
		std::swap(SelStart, rhs.SelStart);
		std::swap(SelEnd, rhs.SelEnd);
		std::swap(LeftPos, rhs.LeftPos);
		std::swap(ColorListFlags, rhs.ColorListFlags);
		std::swap(EndType, rhs.EndType);
	}

	virtual void FastShow();
	virtual int ProcessKey(int Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void Changed(bool DelBlock=false){};
	void SetDelRemovesBlocks(bool Mode) {Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Mode);}
	int GetDelRemovesBlocks() const {return Flags.Check(FEDITLINE_DELREMOVESBLOCKS); }
	void SetPersistentBlocks(bool Mode) {Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Mode);}
	int GetPersistentBlocks() const {return Flags.Check(FEDITLINE_PERSISTENTBLOCKS); }
	void SetShowWhiteSpace(int Mode) {Flags.Change(FEDITLINE_SHOWWHITESPACE, Mode!=0); Flags.Change(FEDITLINE_SHOWLINEBREAK, Mode == 1);}
	void GetString(wchar_t *Str, int MaxSize) const;
	void GetString(string &strStr) const;
	const wchar_t* GetStringAddr() const;
	void SetHiString(const string& Str);
	void SetString(const wchar_t *Str,int Length=-1);
	void SetBinaryString(const wchar_t *Str,int Length);
	void GetBinaryString(const wchar_t **Str, const wchar_t **EOL,intptr_t &Length);
	void SetEOL(const wchar_t *EOL);
	const wchar_t *GetEOL() const;
	int GetSelString(wchar_t *Str,int MaxSize);
	int GetSelString(string &strStr, size_t MaxSize = string::npos) const;
	int GetLength() const;
	void AppendString(const wchar_t *Str);
	void InsertString(const string& Str);
	void InsertBinaryString(const wchar_t *Str,int Length);
	int Search(const string& Str,const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm,string& ReplaceStr,int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength);
	void SetClearFlag(bool Flag) {Flags.Change(FEDITLINE_CLEARFLAG,Flag);}
	int GetClearFlag() const {return Flags.Check(FEDITLINE_CLEARFLAG);}
	void SetCurPos(int NewPos) {CurPos=NewPos; SetPrevCurPos(NewPos);}
	int GetCurPos() const { return CurPos; }
	int GetTabCurPos() const;
	void SetTabCurPos(int NewPos);
	int GetLeftPos() const {return LeftPos;}
	void SetLeftPos(int NewPos) {LeftPos=NewPos;}
	void SetPasswordMode(bool Mode) {Flags.Change(FEDITLINE_PASSWORDMODE,Mode);}
	// Получение максимального значения строки для потребностей Dialod API
	virtual int GetMaxLength() const {return -1;}
	void SetOvertypeMode(bool Mode) {Flags.Change(FEDITLINE_OVERTYPE, Mode);}
	bool GetOvertypeMode() const {return Flags.Check(FEDITLINE_OVERTYPE);}
	int RealPosToTab(int Pos) const;
	int TabPosToReal(int Pos) const;
	void Select(int Start,int End);
	void AddSelect(int Start,int End);
	void GetSelection(intptr_t &Start,intptr_t &End) const;
	bool IsSelection() const {return !(SelStart==-1 && !SelEnd); }
	void GetRealSelection(intptr_t &Start,intptr_t &End) const;
	void SetEditBeyondEnd(bool Mode) {Flags.Change(FEDITLINE_EDITBEYONDEND, Mode);}
	void SetEditorMode(bool Mode) {Flags.Change(FEDITLINE_EDITORMODE, Mode);}
	bool ReplaceTabs();
	void InsertTab();
	void AddColor(ColorItem *col,bool skipsort=false);
	void SortColorUnlocked();
	int DeleteColor(int ColorPos,const GUID& Owner,bool skipfree=false);
	int GetColor(ColorItem *col,int Item) const;
	void Xlat(bool All=false);
	void SetDialogParent(DWORD Sets);
	void SetCursorType(bool Visible, DWORD Size);
	void GetCursorType(bool& Visible, DWORD& Size) const;
	bool GetReadOnly() const {return Flags.Check(FEDITLINE_READONLY);}
	void SetReadOnly(bool NewReadOnly) {Flags.Change(FEDITLINE_READONLY,NewReadOnly);}

protected:
	virtual void DisableCallback() {}
	virtual void RevertCallback() {}
	virtual void RefreshStrByMask(int InitMode=FALSE) {}

	void DeleteBlock();

	static int CheckCharMask(wchar_t Chr);

private:
	virtual void DisplayObject() override;
	virtual const FarColor& GetNormalColor() const;
	virtual const FarColor& GetSelectedColor() const;
	virtual const FarColor& GetUnchangedColor() const;
	virtual const int GetTabSize() const;
	virtual const EXPAND_TABS GetTabExpandMode() const;
	virtual const void SetInputMask(const string& InputMask) {}
	virtual const string GetInputMask() const {return string();}
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
	void ApplyColor(const FarColor& SelColor);
	int GetNextCursorPos(int Position,int Where) const;
	int KeyMatchedMask(int Key, const string& Mask) const;
	int ProcessCtrlQ();
	int ProcessInsDate(const wchar_t *Str);
	int ProcessInsPlainText(const wchar_t *Str);
	int ProcessInsPath(int Key,int PrevSelStart=-1,int PrevSelEnd=0);
	int RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos) const;
	void FixLeftPos(int TabCurPos=-1);
	void SetRightCoord(int Value) {SetPosition(X1, Y2, Value, Y2);}

protected:
	// KEEP ALIGNED!
	wchar_t *Str;
	int StrSize;
	int CurPos;
private:
	// KEEP ALIGNED!
	ColorItem *ColorList;
	int ColorCount;
	int MaxColorCount;
	int SelStart;
	int SelEnd;
	int LeftPos;
	TBitFlags<unsigned char> ColorListFlags;
	unsigned char EndType;

	friend class DlgEdit;
	friend class Editor;
	friend class FileEditor;
};
