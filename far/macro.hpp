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
#include "noncopyable.hpp"

struct GetMacroData;

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

typedef unsigned __int64 MACROFLAGS_MFLAGS;
static const MACROFLAGS_MFLAGS
	// public flags, read from/saved to config
	MFLAGS_PUBLIC_MASK             =0x00000000FFFFFFFF,
	MFLAGS_ENABLEOUTPUT            =0x0000000000000001, // не подавлять обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS     =0x0000000000000002, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTART        =0x0000000000000008, // этот макрос запускается при старте ФАРа
	MFLAGS_EMPTYCOMMANDLINE        =0x0000000000000010, // запускать, если командная линия пуста
	MFLAGS_NOTEMPTYCOMMANDLINE     =0x0000000000000020, // запускать, если командная линия не пуста
	MFLAGS_EDITSELECTION           =0x0000000000000040, // запускать, если есть выделение в редакторе
	MFLAGS_EDITNOSELECTION         =0x0000000000000080, // запускать, если есть нет выделения в редакторе
	MFLAGS_SELECTION               =0x0000000000000100, // активная:  запускать, если есть выделение
	MFLAGS_PSELECTION              =0x0000000000000200, // пассивная: запускать, если есть выделение
	MFLAGS_NOSELECTION             =0x0000000000000400, // активная:  запускать, если есть нет выделения
	MFLAGS_PNOSELECTION            =0x0000000000000800, // пассивная: запускать, если есть нет выделения
	MFLAGS_NOFILEPANELS            =0x0000000000001000, // активная:  запускать, если это плагиновая панель
	MFLAGS_PNOFILEPANELS           =0x0000000000002000, // пассивная: запускать, если это плагиновая панель
	MFLAGS_NOPLUGINPANELS          =0x0000000000004000, // активная:  запускать, если это файловая панель
	MFLAGS_PNOPLUGINPANELS         =0x0000000000008000, // пассивная: запускать, если это файловая панель
	MFLAGS_NOFOLDERS               =0x0000000000010000, // активная:  запускать, если текущий объект "файл"
	MFLAGS_PNOFOLDERS              =0x0000000000020000, // пассивная: запускать, если текущий объект "файл"
	MFLAGS_NOFILES                 =0x0000000000040000, // активная:  запускать, если текущий объект "папка"
	MFLAGS_PNOFILES                =0x0000000000080000, // пассивная: запускать, если текущий объект "папка"

	// private flags, for runtime purposes only
	MFLAGS_PRIVATE_MASK            =0xFFFFFFFF00000000,
	MFLAGS_POSTFROMPLUGIN          =0x0000000100000000; // последовательность пришла от АПИ


// коды возврата для KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE
{
	MACROMODE_NOMACRO          =0,  // не в режиме макро
	MACROMODE_EXECUTING        =1,  // исполнение: без передачи плагину пимп
	MACROMODE_EXECUTING_COMMON =2,  // исполнение: с передачей плагину пимп
	MACROMODE_RECORDING        =3,  // запись: без передачи плагину пимп
	MACROMODE_RECORDING_COMMON =4,  // запись: с передачей плагину пимп
};

struct MacroPanelSelect
{
	__int64 Index;
	TVar    *Item;
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
};

class MacroRecord:NonCopyable
{
public:
	MacroRecord():
		m_flags(),
		m_key(-1),
		m_macroId(),
		m_macrovalue(),
		m_handle(0)
	{
	}

	MacroRecord(MACROFLAGS_MFLAGS Flags, int MacroId, int Key, const wchar_t* Code):
		m_flags(Flags),
		m_key(Key),
		m_code(Code),
		m_macroId(MacroId),
		m_macrovalue(),
		m_handle(0)
	{
	}

	MacroRecord(MacroRecord&& rhs):
		m_flags(),
		m_key(-1),
		m_macroId(),
		m_macrovalue(),
		m_handle(0)
	{
		*this = std::move(rhs);
	}

	MOVE_OPERATOR_BY_SWAP(MacroRecord);

	void swap(MacroRecord& rhs)
	{
		std::swap(m_flags, rhs.m_flags);
		std::swap(m_key, rhs.m_key);
		m_code.swap(rhs.m_code);
		std::swap(m_macroId, rhs.m_macroId);
		std::swap(m_macrovalue, rhs.m_macrovalue);
		std::swap(m_handle, rhs.m_handle);
	}

	MACROFLAGS_MFLAGS Flags() const {return m_flags;}
	int Key() const { return m_key; }
	const string& Code() const {return m_code;}

	intptr_t GetHandle() const { return m_handle; }
	void SetHandle(intptr_t handle) { m_handle = handle; }
	void SetBooleanValue(int val) { m_macrovalue.Type=FMVT_BOOLEAN; m_macrovalue.Boolean=val; }
	FarMacroValue* GetValue() { return &m_macrovalue; }

private:
	MACROFLAGS_MFLAGS m_flags;     // Флаги макропоследовательности
	int m_key;                     // Назначенная клавиша
	string m_code;                 // оригинальный "текст" макроса
	int m_macroId;                 // Идентификатор загруженного макроса в плагине LuaMacro; 0 для макроса, запускаемого посредством MSSC_POST.
	FarMacroValue m_macrovalue;    // Значение, хранимое исполняющимся макросом
	intptr_t m_handle;             // Хэндл исполняющегося макроса

	friend class KeyMacro;
};

STD_SWAP_SPEC(MacroRecord);

class MacroState:NonCopyable
{
public:
	MacroState():
		IntKey(0),
		Executing(),
		KeyProcess(),
		HistoryDisable(),
		UseInternalClipboard()
	{
	}

	MacroState(MacroState&& rhs):
		IntKey(0),
		Executing(),
		KeyProcess(),
		HistoryDisable(),
		UseInternalClipboard()
	{
		*this = std::move(rhs);
	}

	MOVE_OPERATOR_BY_SWAP(MacroState);

	void swap(MacroState& rhs)
	{
		std::swap(IntKey, rhs.IntKey);
		std::swap(Executing, rhs.Executing);
		m_MacroQueue.swap(rhs.m_MacroQueue);
		std::swap(KeyProcess, rhs.KeyProcess);
		std::swap(HistoryDisable, rhs.HistoryDisable);
		std::swap(UseInternalClipboard, rhs.UseInternalClipboard);
	}

	MacroRecord* GetCurMacro() { return m_MacroQueue.empty()? nullptr : &m_MacroQueue.front(); }
	const MacroRecord* GetCurMacro() const { return m_MacroQueue.empty()? nullptr : &m_MacroQueue.front(); }
	void RemoveCurMacro() { m_MacroQueue.pop_front(); }

public:
	DWORD IntKey; // "описание реально нажатой клавиши"
	int Executing;
	std::list<MacroRecord> m_MacroQueue;
	int KeyProcess;
	DWORD HistoryDisable;
	bool UseInternalClipboard;
};

STD_SWAP_SPEC(MacroState);

class Dialog;

class KeyMacro:NonCopyable
{
public:
	KeyMacro();
	~KeyMacro();

	int  IsRecording() const { return m_Recording; }
	int  IsExecuting() const;
	int  IsDisableOutput() const;
	bool IsHistoryDisable(int TypeHistory) const;
	DWORD SetHistoryDisableMask(DWORD Mask);
	DWORD GetHistoryDisableMask() const;
	void SetMode(FARMACROAREA Mode) { m_Mode=Mode; }
	FARMACROAREA GetMode() const { return m_Mode; }
	bool Load(bool InitedRAM=true,bool LoadAll=true);
	bool Save(bool always);
	// получить данные о макросе (возвращает статус)
	int GetCurRecord() const;
	int ProcessEvent(const FAR_INPUT_RECORD *Rec);
	int GetKey();
	int PeekKey();
	bool GetMacroKeyInfo(const string& strMode,int Pos,string &strKeyName,string &strDescription);
	static void SetMacroConst(int ConstIndex, __int64 Value);
	// послать сигнал на прерывание макроса
	void SendDropProcess();
	bool CheckWaitKeyFunc();

	bool MacroExists(int Key, FARMACROAREA CheckMode, bool UseCommon);
	void RunStartMacro();
	int AddMacro(const wchar_t *PlainText,const wchar_t *Description, FARMACROAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback);
	int DelMacro(const GUID& PluginId,void* Id);
	bool PostNewMacro(const wchar_t* PlainText,UINT64 Flags=0,DWORD AKey=0) { return PostNewMacro(0,PlainText,Flags,AKey); }
	bool ParseMacroString(const wchar_t* Sequence,bool onlyCheck,bool skipFile);
	bool ExecuteString(MacroExecuteString *Data);
	void GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc);
	intptr_t CallFar(intptr_t OpCode, FarMacroCall* Data);
	void CallPluginSynchro(MacroPluginReturn *Params, FarMacroCall **Target, int *Boolean);
	const wchar_t *eStackAsString() { return varTextDate; }
	void SuspendMacros(bool Suspend) { Suspend ? ++m_InternalInput : --m_InternalInput; }

private:
	bool CallMacroPlugin(OpenMacroPluginInfo* Info);
	int AssignMacroKey(DWORD& MacroKey,UINT64& Flags);
	intptr_t AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t ParamMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	int GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
	void InitInternalVars(bool InitedRAM=true);
	MacroRecord* CheckCurMacro();
	MacroRecord* GetCurMacro() { return m_CurState.GetCurMacro(); }
	const MacroRecord* GetCurMacro() const { return m_CurState.GetCurMacro(); }
	MacroRecord* GetTopMacro() { return m_StateStack.empty()? nullptr: m_StateStack.top().GetCurMacro(); }
	void RemoveCurMacro() { m_CurState.RemoveCurMacro(); }
	void RestoreMacroChar();
	bool PostNewMacro(int macroId,const wchar_t* PlainText,UINT64 Flags,DWORD AKey);
	void PushState(bool withClip);
	void PopState(bool withClip);
	bool LM_GetMacro(GetMacroData* Data, FARMACROAREA Mode, const string& TextKey, bool UseCommon, bool CheckOnly);
	void LM_ProcessMacro(FARMACROAREA Mode, const string& TextKey, const string& Code, MACROFLAGS_MFLAGS Flags, const string& Description, const GUID* Guid=nullptr, FARMACROCALLBACK Callback=nullptr, void* CallbackId=nullptr);
	void CallPlugin(MacroPluginReturn *mpr, FarMacroValue *fmv, bool CallPluginRules);

	FARMACROAREA m_Mode;
	MacroState m_CurState;
	std::stack<MacroState> m_StateStack;
	MACRORECORDANDEXECUTETYPE m_Recording;
	string m_RecCode;
	string m_RecDescription;
	FARMACROAREA m_RecMode;
	FARMACROAREA StartMode; //FIXME
	string m_LastErrorStr;
	int m_LastErrorLine;
	int m_InternalInput;
	int m_MacroPluginIsRunning;
	int m_DisableNested;
	int m_WaitKey;
	const wchar_t* varTextDate;

};

inline bool IsMenuArea(int Area){return Area==MACROAREA_MAINMENU || Area==MACROAREA_MENU || Area==MACROAREA_DISKS || Area==MACROAREA_USERMENU || Area==MACROAREA_SHELLAUTOCOMPLETION || Area==MACROAREA_DIALOGAUTOCOMPLETION;}
