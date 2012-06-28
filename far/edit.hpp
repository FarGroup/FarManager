#pragma once

/*
edit.hpp

Реализация одиночной строки редактирования
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
#include "plugin.hpp"

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
	FEDITLINE_DELREMOVESBLOCKS     = 0x00008000,  // Del удаляет блоки (Opt.EditorDelRemovesBlocks)
	FEDITLINE_PERSISTENTBLOCKS     = 0x00010000,  // Постоянные блоки (Opt.EditorPersistentBlocks)
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
	GUID Owner;
	unsigned Priority;
	int SubPriority;
	int StartPos;
	int EndPos;
	FarColor Color;
	unsigned __int64 Flags;
};

enum SetCPFlags
{
	SETCP_NOERROR    = 0x00000000,
	SETCP_WC2MBERROR = 0x00000001,
	SETCP_MB2WCERROR = 0x00000002,
	SETCP_OTHERERROR = 0x10000000,
};

class Edit:public ScreenObject
{
		friend class DlgEdit;
		friend class Editor;
		friend class FileEditor;

	protected:
		wchar_t *Str;
		int StrSize;
		int CurPos;
		int LeftPos;

		virtual void DisableCallback(){};
		virtual void RevertCallback(){};

	private:
		Edit  *m_next;
		Edit  *m_prev;
		int MaxLength;
		wchar_t *Mask;
		ColorItem *ColorList;
		int    ColorCount;
		int    MaxColorCount;
		bool   ColorListNeedSort;
		bool   ColorListNeedFree;

		FarColor Color;
		FarColor SelColor;

		int    PrevCurPos;       // 12.08.2000 KM - предыдущее положение курсора

		int    TabSize;          // 14.02.2001 IS - Размер табуляции - по умолчанию равен Opt.TabSize;

		int    TabExpandMode;

		int    MSelStart;
		int    SelStart;
		int    SelEnd;

		int    EndType;

		int    CursorSize;
		int    CursorPos;
		const class StringOption* strWordDiv;

		UINT m_codepage; //BUGBUG

	protected:
		void   DeleteBlock();

	private:
		virtual void   DisplayObject();
		virtual void SetUnchangedColor(){};
		int    InsertKey(int Key);
		int    RecurseProcessKey(int Key);
		void   ApplyColor();
		int    GetNextCursorPos(int Position,int Where);
		void   RefreshStrByMask(int InitMode=FALSE);
		int KeyMatchedMask(int Key);

		int ProcessCtrlQ();
		int ProcessInsDate(const wchar_t *Str);
		int ProcessInsPlainText(const wchar_t *Str);

		int CheckCharMask(wchar_t Chr);
		int ProcessInsPath(int Key,int PrevSelStart=-1,int PrevSelEnd=0);

		int RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos);

		const wchar_t* WordDiv();
		void FixLeftPos(int TabCurPos=-1);
	public:
		Edit(ScreenObject *pOwner = nullptr, bool bAllocateData = true);
		virtual ~Edit();

	public:

		DWORD SetCodePage(UINT codepage, bool check_only, char * &buff, int &bsize); //BUGBUG
		UINT GetCodePage();  //BUGBUG

		virtual void  FastShow();
		virtual int   ProcessKey(int Key);
		virtual int   ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
#ifdef FAR_LUA
#else
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
#endif

		//   ! Функция установки текущих Color,SelColor и ColorUnChanged!
		void  SetObjectColor(PaletteColors Color,PaletteColors SelColor = COL_COMMANDLINESELECTED);
		void  SetObjectColor(const FarColor& Color,const FarColor& SelColor);
		void  GetObjectColor(FarColor& Color, FarColor& SelColor) {Color = this->Color; SelColor = this->SelColor;}

		void SetTabSize(int NewSize) { TabSize=NewSize; }
		int  GetTabSize() {return TabSize; }

		void SetDelRemovesBlocks(int Mode) {Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Mode);}
		int  GetDelRemovesBlocks() {return Flags.Check(FEDITLINE_DELREMOVESBLOCKS); }

		void SetPersistentBlocks(int Mode) {Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Mode);}
		int  GetPersistentBlocks() {return Flags.Check(FEDITLINE_PERSISTENTBLOCKS); }

		void SetShowWhiteSpace(int Mode) {Flags.Change(FEDITLINE_SHOWWHITESPACE, Mode!=0); Flags.Change(FEDITLINE_SHOWLINEBREAK, Mode == 1);}

		void  GetString(wchar_t *Str, int MaxSize);
		void  GetString(string &strStr);

		const wchar_t* GetStringAddr();

		void  SetHiString(const wchar_t *Str);
		void  SetString(const wchar_t *Str,int Length=-1);

		void  SetBinaryString(const wchar_t *Str,int Length);

		void  GetBinaryString(const wchar_t **Str, const wchar_t **EOL,int &Length);

		void  SetEOL(const wchar_t *EOL);
		const wchar_t *GetEOL();

		int   GetSelString(wchar_t *Str,int MaxSize);
		int   GetSelString(string &strStr);

		int   GetLength();

		void AppendString(const wchar_t *Str);
		void  InsertString(const wchar_t *Str);
		void  InsertBinaryString(const wchar_t *Str,int Length);

		int   Search(const string& Str,string& ReplaceStr,int Position,int Case,int WholeWords,int Reverse,int Regexp, int *SearchLength);

		void  SetClearFlag(int Flag) {Flags.Change(FEDITLINE_CLEARFLAG,Flag);}
		int   GetClearFlag() {return Flags.Check(FEDITLINE_CLEARFLAG);}
		void  SetCurPos(int NewPos) {CurPos=NewPos; PrevCurPos=NewPos;}
		int   GetCurPos() {return(CurPos);}
		int   GetTabCurPos();
		void  SetTabCurPos(int NewPos);
		int   GetLeftPos() {return(LeftPos);}
		void  SetLeftPos(int NewPos) {LeftPos=NewPos;}
		void  SetPasswordMode(int Mode) {Flags.Change(FEDITLINE_PASSWORDMODE,Mode);};
		void  SetMaxLength(int Length) {MaxLength=Length;};

		// Получение максимального значения строки для потребностей Dialod API
		int   GetMaxLength() {return MaxLength;};

		void  SetInputMask(const wchar_t *InputMask);
		const wchar_t* GetInputMask() {return Mask;}

		void  SetOvertypeMode(int Mode) {Flags.Change(FEDITLINE_OVERTYPE,Mode);};
		int   GetOvertypeMode() {return Flags.Check(FEDITLINE_OVERTYPE);};

		void  SetConvertTabs(int Mode) { TabExpandMode = Mode;};
		int   GetConvertTabs() {return TabExpandMode;};

		int   RealPosToTab(int Pos);
		int   TabPosToReal(int Pos);
		void  Select(int Start,int End);
		void  AddSelect(int Start,int End);
		void  GetSelection(int &Start,int &End);
		BOOL  IsSelection() {return  SelStart==-1 && !SelEnd?FALSE:TRUE; };
		void  GetRealSelection(int &Start,int &End);
		void  SetEditBeyondEnd(int Mode) {Flags.Change(FEDITLINE_EDITBEYONDEND,Mode);};
		void  SetEditorMode(int Mode) {Flags.Change(FEDITLINE_EDITORMODE,Mode);};
		bool  ReplaceTabs();

		void  InsertTab();

		void  AddColor(ColorItem *col,bool skipsort=false);
		void  SortColorUnlocked();
		int   DeleteColor(int ColorPos,const GUID& Owner,bool skipfree=false);
		int   GetColor(ColorItem *col,int Item);

		void Xlat(bool All=false);

		void SetDialogParent(DWORD Sets);
		void SetCursorType(bool Visible, DWORD Size);
		void GetCursorType(bool& Visible, DWORD& Size);
		int  GetReadOnly() {return Flags.Check(FEDITLINE_READONLY);}
		void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITLINE_READONLY,NewReadOnly);}
		void SetWordDiv(const StringOption& WordDiv) {strWordDiv=&WordDiv;}
		virtual void Changed(bool DelBlock=false){};
};



class History;
class VMenu;

// Надстройка над Edit.
// Одиночная строка ввода для диалогов и комстроки (не для редактора)

class EditControl:public Edit
{
	friend class DlgEdit;

	bool Selection;
	int SelectionStart;
	int MacroAreaAC;

	History* pHistory;
	FarList* pList;
	void SetMenuPos(VMenu& menu);
	int AutoCompleteProc(bool Manual,bool DelBlock,int& BackKey, int Area);

	FarColor ColorUnChanged;   // 28.07.2000 SVS - для диалога

	BitFlags ECFlags;

	bool ACState;

	bool CallbackSaveState;

	virtual void DisableCallback()
	{
		CallbackSaveState = m_Callback.Active;
		m_Callback.Active = false;
	}
	virtual void RevertCallback()
	{
		m_Callback.Active = CallbackSaveState;
	};

public:
	typedef void (*EDITCHANGEFUNC)(void* aParam);
	struct Callback
	{
		bool Active;
		EDITCHANGEFUNC m_Callback;
		void* m_Param;
	};
	Callback m_Callback;

	enum ECFLAGS
	{
		EC_ENABLEAUTOCOMPLETE  = 0x1,
		EC_COMPLETE_FILESYSTEM = 0x2,
		EC_COMPLETE_PATH       = 0x4,
		EC_COMPLETE_HISTORY    = 0x8,
	};

	EditControl(ScreenObject *pOwner=nullptr,Callback* aCallback=nullptr,bool bAllocateData=true,History* iHistory=0,FarList* iList=0,DWORD iFlags=0);
	virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
	virtual void Show();
	virtual void Changed(bool DelBlock=false);
	virtual void SetUnchangedColor();

	void AutoComplete(bool Manual,bool DelBlock);
	void SetAutocomplete(bool State) {State? ECFlags.Set(EC_ENABLEAUTOCOMPLETE) : ECFlags.Clear(EC_ENABLEAUTOCOMPLETE);}
	bool GetAutocomplete() {return ECFlags.Check(EC_ENABLEAUTOCOMPLETE) != 0;}
	void SetMacroAreaAC(int Area){MacroAreaAC=Area;}
	void SetCallbackState(bool Enable){m_Callback.Active=Enable;}

	void  SetObjectColor(PaletteColors Color,PaletteColors SelColor = COL_COMMANDLINESELECTED,PaletteColors ColorUnChanged=COL_DIALOGEDITUNCHANGED);
	void  SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged);
	void  GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged);
	int  GetDropDownBox() {return Flags.Check(FEDITLINE_DROPDOWNBOX);}
	void SetDropDownBox(int NewDropDownBox) {Flags.Change(FEDITLINE_DROPDOWNBOX,NewDropDownBox);}
};
