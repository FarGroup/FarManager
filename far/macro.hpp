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

#include "array.hpp"
#include "TStack.hpp"
#include "DList.hpp"
#include "tvar.hpp"
class Panel;

enum MACRODISABLEONLOAD
{
	MDOL_ALL            = 0x80000000, // дисаблим все макросы при загрузке
	MDOL_AUTOSTART      = 0x00000001, // дисаблим автостартующие макросы
};

typedef unsigned __int64 MACROFLAGS_MFLAGS;
static const MACROFLAGS_MFLAGS
	MFLAGS_DISABLEOUTPUT           =0x0000000000000001, // подавить обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS     =0x0000000000000002, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTARTED      =0x0000000000000004, //! этот макрос уже запускался при старте ФАРа
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

	MFLAGS_POSTFROMPLUGIN          =0x0000000000200000, //! последовательность пришла от АПИ
	MFLAGS_NEEDSAVEMACRO           =0x0000000000400000, //! необходимо этот макрос запомнить
	MFLAGS_DISABLEMACRO            =0x0000000000800000, //! этот макрос отключен
	MFLAGS_CALLPLUGINENABLEMACRO   =0x0000000001000000; // разрешить макросы при вызове плагина функцией CallPlugin


// коды возврата для KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE
{
	MACROMODE_NOMACRO          =0,  // не в режиме макро
	MACROMODE_EXECUTING        =1,  // исполнение: без передачи плагину пимп
	MACROMODE_EXECUTING_COMMON =2,  // исполнение: с передачей плагину пимп
	MACROMODE_RECORDING        =3,  // запись: без передачи плагину пимп
	MACROMODE_RECORDING_COMMON =4,  // запись: с передачей плагину пимп
};
// области действия макросов (начало исполнения) -  НЕ БОЛЕЕ 0xFF областей!
enum MACROMODEAREA
{
	// see also plugin.hpp # FARMACROAREA
	MACRO_OTHER                =   0, // Режим копирования текста с экрана, вертикальные меню
	MACRO_SHELL                =   1, // Файловые панели
	MACRO_VIEWER               =   2, // Внутренняя программа просмотра
	MACRO_EDITOR               =   3, // Редактор
	MACRO_DIALOG               =   4, // Диалоги
	MACRO_SEARCH               =   5, // Быстрый поиск в панелях
	MACRO_DISKS                =   6, // Меню выбора дисков
	MACRO_MAINMENU             =   7, // Основное меню
	MACRO_MENU                 =   8, // Прочие меню
	MACRO_HELP                 =   9, // Система помощи
	MACRO_INFOPANEL            =  10, // Информационная панель
	MACRO_QVIEWPANEL           =  11, // Панель быстрого просмотра
	MACRO_TREEPANEL            =  12, // Панель дерева папок
	MACRO_FINDFOLDER           =  13, // Поиск папок
	MACRO_USERMENU             =  14, // Меню пользователя
	MACRO_SHELLAUTOCOMPLETION  =  15, // Список автодополнения в панелях в ком.строке
	MACRO_DIALOGAUTOCOMPLETION =  16, // Список автодополнения в диалоге

	MACRO_COMMON,                     // ВЕЗДЕ! - должен быть предпоследним, т.к. приоритет самый низший !!!
	MACRO_LAST,                       // Должен быть всегда последним! Используется в циклах

	MACRO_INVALID = -4                // FIXME: Должен быть меньше чем MACRO_FUNCS
};

struct MacroPanelSelect {
	__int64 Index;
	TVar    *Item;
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
};

enum INTMF_FLAGS{
	IMFF_UNLOCKSCREEN               =0x00000001,
	IMFF_DISABLEINTINPUT            =0x00000002,
};

class MacroRecord
{
	friend class KeyMacro;
	private:
		MACROMODEAREA m_area;
		MACROFLAGS_MFLAGS m_flags;
		int m_key;
		string m_name;
		string m_code;
		string m_description;
		GUID m_guid;
		void* m_id;
		FARMACROCALLBACK m_callback;
		void* m_handle;
	public:
		MacroRecord();
		MacroRecord(MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,int Key,string Name,string Code,string Description);
		MacroRecord& operator= (const MacroRecord& src);
	public:
		MACROMODEAREA Area(void) {return m_area;}
		MACROFLAGS_MFLAGS Flags(void) {return m_flags;}
		int Key() { return m_key; }
		const string& Code(void) {return m_code;}
		const string& Name(void) {return m_name;}
		const string& Description(void) {return m_description;}
		bool IsSave(void) {return (m_flags&MFLAGS_NEEDSAVEMACRO) != 0;}
		void SetSave(void) {m_flags|=MFLAGS_NEEDSAVEMACRO;}
		void ClearSave(void) {m_flags&=~MFLAGS_NEEDSAVEMACRO;}
};

class MacroState
{
	private:
		MacroState& operator= (const MacroState&);
	public:
		INPUT_RECORD cRec; // "описание реально нажатой клавиши"
		int Executing;
		DList<MacroRecord> m_MacroQueue;
		int KeyProcess;
		DWORD HistoryDisable;
		bool UseInternalClipboard;
	public:
		MacroState();
		MacroRecord* GetCurMacro() { return m_MacroQueue.Empty() ? nullptr : m_MacroQueue.First(); }
		void RemoveCurMacro() { if (!m_MacroQueue.Empty()) m_MacroQueue.Delete(m_MacroQueue.First()); }
};

class KeyMacro
{
	private:
		TArray<MacroRecord> m_Macros[MACRO_LAST];
		MACROMODEAREA m_Mode;
		MacroState* m_CurState;
		TStack<MacroState*> m_StateStack;
		MACRORECORDANDEXECUTETYPE m_Recording;
		string m_RecCode;
		string m_RecDescription;
		MACROMODEAREA m_RecMode;
		MACROMODEAREA StartMode; //FIXME
		class LockScreen* m_LockScr;
		string m_LastKey;
		string m_LastErrorStr;
		int m_LastErrorLine;
		int m_InternalInput;
	private:
		bool ReadKeyMacro(MACROMODEAREA Area);
		void WriteMacro(void);
		void* CallMacroPlugin(OpenMacroInfo* Info);
		int AssignMacroKey(DWORD& MacroKey,UINT64& Flags);
		static intptr_t WINAPI AssignMacroDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2);
		static intptr_t WINAPI ParamMacroDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2);
		int GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
		void InitInternalVars(bool InitedRAM=true);
		bool InitMacroExecution(void);
		bool UpdateLockScreen(bool recreate=false);
		MacroRecord* GetCurMacro() { return m_CurState->GetCurMacro(); }
		MacroRecord* GetTopMacro() { return m_StateStack.empty()?nullptr:(*m_StateStack.Peek())->GetCurMacro(); }
		void RemoveCurMacro() { m_CurState->RemoveCurMacro(); }
		void RestoreMacroChar(void);
	public:
		KeyMacro();
		~KeyMacro();
	public:
		int  IsRecording();
		int  IsExecuting();
		int  IsExecutingLastKey();
		int  IsDisableOutput();
		bool IsHistoryDisable(int TypeHistory);
		DWORD SetHistoryDisableMask(DWORD Mask);
		DWORD GetHistoryDisableMask();
		void SetMode(MACROMODEAREA Mode);
		MACROMODEAREA GetMode(void);
		bool LoadMacros(bool InitedRAM=true,bool LoadAll=true);
		void SaveMacros(void);
		// получить данные о макросе (возвращает статус)
		int GetCurRecord(MacroRecord* RBuf=nullptr,int *KeyPos=nullptr);
		int ProcessEvent(const struct FAR_INPUT_RECORD *Rec);
		int GetKey();
		int PeekKey();
		static MACROMODEAREA GetAreaCode(const wchar_t *AreaName);
		static int GetMacroKeyInfo(bool FromDB,MACROMODEAREA Mode,int Pos,string &strKeyName,string &strDescription);
		static void SetMacroConst(int ConstIndex, __int64 Value);
		// послать сигнал на прерывание макроса
		void SendDropProcess();
		bool CheckWaitKeyFunc();

		void PushState();
		void PopState();

		// Функция получения индекса нужного макроса в массиве
		int GetIndex(MACROMODEAREA* area, int Key, string& strKey, MACROMODEAREA CheckMode, bool UseCommon=true, bool StrictKeys=false);
		int GetIndex(int Key, string& strKey, MACROMODEAREA CheckMode, bool UseCommon=true, bool StrictKeys=false)
			{ MACROMODEAREA dummy; return GetIndex(&dummy,Key,strKey,CheckMode,UseCommon,StrictKeys); }
		void RunStartMacro();
		int AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback);
		int DelMacro(const GUID& PluginId,void* Id);
		// Поместить временное строковое представление макроса
		bool PostNewMacro(const wchar_t *PlainText,UINT64 Flags=0,DWORD AKey=0,bool onlyCheck=false);
		bool ParseMacroString(const wchar_t *Sequence,bool onlyCheck=false,bool skipFile=true);
		void GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc);
		intptr_t CallFar(intptr_t OpCode, FarMacroCall* Data);
};

const wchar_t *eStackAsString(int Pos=0);
inline bool IsMenuArea(int Area){return Area==MACRO_MAINMENU || Area==MACRO_MENU || Area==MACRO_DISKS || Area==MACRO_USERMENU || Area==MACRO_SHELLAUTOCOMPLETION || Area==MACRO_DIALOGAUTOCOMPLETION;}
