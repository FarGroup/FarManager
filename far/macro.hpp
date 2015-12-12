#ifndef MACRO_HPP_BA3167E8_1846_4B24_88A4_CF59CA90169F
#define MACRO_HPP_BA3167E8_1846_4B24_88A4_CF59CA90169F
#pragma once

/*
macro.hpp

Макросы
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

#include "tvar.hpp"

// Macro Const
enum {
	constMsX          = 0,
	constMsY          = 1,
	constMsButton     = 2,
	constMsCtrlState  = 3,
	constMsEventFlags = 4,
	constMsLastCtrlState = 5,
	constMsLAST       = 6,
};

enum MACRODISABLEONLOAD
{
	MDOL_ALL            = 0x80000000, // дисаблим все макросы при загрузке
	MDOL_AUTOSTART      = 0x00000001, // дисаблим автостартующие макросы
};

struct MacroPanelSelect
{
	__int64 Index;
	TVar    *Item;
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
};

class Dialog;

class KeyMacro: noncopyable
{
public:
	KeyMacro();

	static bool AddMacro(const GUID& PluginId,const MacroAddMacro* Data);
	static bool DelMacro(const GUID& PluginId,void* Id);
	static bool ExecuteString(MacroExecuteString *Data);
	static bool GetMacroKeyInfo(const string& strArea,int Pos,string &strKeyName,string &strDescription);
	static int  IsDisableOutput();
	static int  IsExecuting();
	static bool IsHistoryDisable(int TypeHistory);
	static bool MacroExists(int Key, FARMACROAREA Area, bool UseCommon);
	static void RunStartMacro();
	static bool SaveMacros(bool always);
	static void SendDropProcess();
	static void SetMacroConst(int ConstIndex, __int64 Value);
	static bool PostNewMacro(const wchar_t* Sequence, FARKEYMACROFLAGS Flags, DWORD AKey = 0);

	intptr_t CallFar(intptr_t OpCode, FarMacroCall* Data);
	bool CheckWaitKeyFunc() const;
	int  GetState() const;
	int  GetKey();
	void GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc) const;
	FARMACROAREA GetArea() const { return m_Area; }
	const wchar_t* GetStringToPrint() const { return m_StringToPrint; }
	int  IsRecording() const { return m_Recording; }
	bool LoadMacros(bool FromFar, bool InitedRAM=true, const FarMacroLoad *Data=nullptr);
	bool ParseMacroString(const wchar_t* Sequence,FARKEYMACROFLAGS Flags,bool skipFile);
	int  PeekKey() const;
	int  ProcessEvent(const FAR_INPUT_RECORD *Rec);
	void SetArea(FARMACROAREA Area) { m_Area=Area; }
	void SuspendMacros(bool Suspend) { Suspend ? ++m_InternalInput : --m_InternalInput; }

private:
	intptr_t AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	int  AssignMacroKey(DWORD& MacroKey,UINT64& Flags);
	int  GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
	intptr_t ParamMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	void RestoreMacroChar() const;

	FARMACROAREA m_Area;
	FARMACROAREA m_StartMode;
	FARMACROSTATE m_Recording;
	string m_RecCode;
	string m_RecDescription;
	string m_LastErrorStr;
	int m_LastErrorLine;
	int m_InternalInput;
	int m_WaitKey;
	const wchar_t* m_StringToPrint;
};

inline bool IsMenuArea(int Area){return Area==MACROAREA_MAINMENU || Area==MACROAREA_MENU || Area==MACROAREA_DISKS || Area==MACROAREA_USERMENU || Area==MACROAREA_SHELLAUTOCOMPLETION || Area==MACROAREA_DIALOGAUTOCOMPLETION;}

#endif // MACRO_HPP_BA3167E8_1846_4B24_88A4_CF59CA90169F
