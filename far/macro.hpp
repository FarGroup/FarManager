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

extern void SZLOG (const char *fmt, ...);

// FIXME: for SciTE only.
#if SCITE == 1
#define FAR_LUA
#endif

#ifdef FAR_LUA
#include "array.hpp"
#include "TStack.hpp"
#include "DList.hpp"
class Panel;

enum MACRODISABLEONLOAD
{
	MDOL_ALL            = 0x80000000, // дисаблим все макросы при загрузке
	MDOL_AUTOSTART      = 0x00000001, // дисаблим автостартующие макросы
};

typedef unsigned __int64 MACROFLAGS_MFLAGS;
static const MACROFLAGS_MFLAGS
	MFLAGS_DISABLEOUTPUT           =0x0000000000000001, //! подавить обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS     =0x0000000000000002, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTARTED      =0x0000000000000004, //! этот макрос уже запускался при старте ФАРа
	MFLAGS_RUNAFTERFARSTART        =0x0000000000000008, //! этот макрос запускается при старте ФАРа

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
	MFLAGS_CALLPLUGINENABLEMACRO   =0x0000000001000000; //! разрешить макросы при вызове плагина функцией CallPlugin


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
	MACRO_LAST                        // Должен быть всегда последним! Используется в циклах
};

/*
struct MacroRecord
{
	MACROMODEAREA Area;
	MACROFLAGS_MFLAGS  Flags;        // Флаги макропоследовательности
	wchar_t *Name;                   // имя записи, может совпадать с именем клавиши
	int    Key;                      // Назначенная клавиша
	wchar_t  *Src;                   // оригинальный "текст" макроса
	wchar_t  *Description;           // описание макроса
	GUID Guid;                       // Гуид владельца макроса
	void* Id;                        // параметр калбака
	FARMACROCALLBACK Callback;       // каллбак для плагинов
};
*/
class MacroRecord
{
	friend class KeyMacro;
	private:
		MACROMODEAREA m_area;
		MACROFLAGS_MFLAGS m_flags;
		string m_name;
		string m_code;
		string m_description;
		GUID m_guid;
		void* m_id;
		FARMACROCALLBACK m_callback;
	public:
		MacroRecord();
		MacroRecord(MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,string Name,string Code,string Description);
		MacroRecord& operator= (const MacroRecord& src);
	public:
		MACROMODEAREA Area(void) {return m_area;}
		MACROFLAGS_MFLAGS Flags(void) {return m_flags;}
		const string& Code(void) {return m_code;}
		const string& Name(void) {return m_name;}
		const string& Description(void) {return m_description;}
		bool IsSave(void) {return m_flags&MFLAGS_NEEDSAVEMACRO;}
		void SetSave(void) {m_flags|=MFLAGS_NEEDSAVEMACRO;}
		void ClearSave(void) {m_flags&=~MFLAGS_NEEDSAVEMACRO;}
};

class RunState
{
	friend class KeyMacro;
	private:
		void* m_handle;
		MACROFLAGS_MFLAGS m_flags;
	public:
		RunState() : m_handle(nullptr),m_flags(0) {}
		RunState(void* h,MACROFLAGS_MFLAGS f=0) : m_handle(h),m_flags(f) {}
		RunState(const RunState& r) : m_handle(r.m_handle),m_flags(r.m_flags) {}
		RunState& operator=(const RunState& r) { m_handle=r.m_handle; m_flags=r.m_flags; return *this; }
		operator bool() { return m_handle != nullptr; }
};

class KeyMacro
{
	private:
		TArray<MacroRecord> m_Macros[MACRO_LAST];
		DList<MacroRecord> m_MacroQueue;
		MACROMODEAREA m_Mode;
		TStack<RunState> m_State;
		RunState m_RunState;
		MACRORECORDANDEXECUTETYPE m_Recording;
		string m_RecCode;
		string m_RecDescription;
		MACROMODEAREA m_RecMode;
		MACROMODEAREA StartMode; //FIXME
		class LockScreen* m_LockScr;
		string m_LastKey;
	private:
		bool ReadMacro(MACROMODEAREA Area);
		void WriteMacro(void);
		void* CallPlugin(unsigned Type,void* Data);
		int AssignMacroKey(DWORD& MacroKey,UINT64& Flags);
		static intptr_t WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
		static intptr_t WINAPI ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
		int GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
		bool InitMacroExecution(MacroRecord* macro);
		bool UpdateLockScreen(bool recreate=false);

		BOOL CheckEditSelected(UINT64 CurFlags);
		BOOL CheckInsidePlugin(UINT64 CurFlags);
		BOOL CheckPanel(int PanelMode,UINT64 CurFlags, BOOL IsPassivePanel);
		BOOL CheckCmdLine(int CmdLength,UINT64 Flags);
		BOOL CheckFileFolder(Panel *ActivePanel,UINT64 CurFlags, BOOL IsPassivePanel);
		BOOL CheckAll(UINT64 CurFlags);

	public:
		KeyMacro();
		~KeyMacro();
	public:
		int  IsRecording();
		int  IsExecuting();
		int  IsExecutingLastKey();
		int  IsDsableOutput();
		bool IsHistoryDisable(int TypeHistory);
		void SetMode(int Mode); //FIXME: int->MACROMODEAREA
		MACROMODEAREA GetMode(void);
		bool LoadMacros(bool InitedRAM=true,bool LoadAll=true);
		void SaveMacros(void);
		// получить данные о макросе (возвращает статус)
		int GetCurRecord(struct MacroRecord* RBuf=nullptr,int *KeyPos=nullptr);
		int ProcessEvent(const struct FAR_INPUT_RECORD *Rec);
		int GetKey();
		int PeekKey();
		static int   GetAreaCode(const wchar_t *AreaName);
		static int   GetMacroKeyInfo(bool FromDB,int Mode,int Pos,string &strKeyName,string &strDescription);
		// послать сигнал на прерывание макроса
		void SendDropProcess();
		bool CheckWaitKeyFunc();
		// Функция получения индекса нужного макроса в массиве
		int GetIndex(int* area, int Key, string& strKey, int CheckMode, bool UseCommon=true, bool StrictKeys=false);
		int GetIndex(int Key, string& strKey, int CheckMode, bool UseCommon=true, bool StrictKeys=false)
			{ int dummy; return GetIndex(&dummy,Key,strKey,CheckMode,UseCommon,StrictKeys); }
		void RunStartMacro();
		int AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback);
		int DelMacro(const GUID& PluginId,void* Id);
		// Поместить временное строковое представление макроса
		int PostNewMacro(const wchar_t *PlainText,UINT64 Flags=0,DWORD AKey=0,bool onlyCheck=false);
		bool ParseMacroString(const wchar_t *Sequence,bool onlyCheck=false);
		int CallFar(int OpCode, FarMacroCall* Data);
};

const wchar_t *eStackAsString(int Pos=0);
inline bool IsMenuArea(int Area){return Area==MACRO_MAINMENU || Area==MACRO_MENU || Area==MACRO_DISKS || Area==MACRO_USERMENU || Area==MACRO_SHELLAUTOCOMPLETION || Area==MACRO_DIALOGAUTOCOMPLETION;}
#else
#include "TStack.hpp"
#include "macrocompiler.hpp"
#include "tvar.hpp"
#include "macroopcode.hpp"

enum MACRODISABLEONLOAD
{
	MDOL_ALL            = 0x80000000, // дисаблим все макросы при загрузке
	MDOL_AUTOSTART      = 0x00000001, // дисаблим автостартующие макросы
};

// области действия макросов (начало исполнения) -  НЕ БОЛЕЕ 0xFF областей!
enum MACROMODEAREA
{
	MACRO_FUNCS                =  -3,
	MACRO_CONSTS               =  -2,
	MACRO_VARS                 =  -1,

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
	MACRO_LAST                        // Должен быть всегда последним! Используется в циклах
};

typedef unsigned __int64 MACROFLAGS_MFLAGS;
static const MACROFLAGS_MFLAGS
	MFLAGS_MODEMASK                =0x00000000000000FF, // маска для выделения области действия (области начала исполнения) макроса

	MFLAGS_DISABLEOUTPUT           =0x0000000000000100, // подавить обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS     =0x0000000000000200, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTARTED      =0x0000000000000400, // этот макрос уже запускался при старте ФАРа
	MFLAGS_RUNAFTERFARSTART        =0x0000000000000800, // этот макрос запускается при старте ФАРа

	MFLAGS_EMPTYCOMMANDLINE        =0x0000000000001000, // запускать, если командная линия пуста
	MFLAGS_NOTEMPTYCOMMANDLINE     =0x0000000000002000, // запускать, если командная линия не пуста
	MFLAGS_EDITSELECTION           =0x0000000000004000, // запускать, если есть выделение в редакторе
	MFLAGS_EDITNOSELECTION         =0x0000000000008000, // запускать, если есть нет выделения в редакторе

	MFLAGS_SELECTION               =0x0000000000010000, // активная:  запускать, если есть выделение
	MFLAGS_PSELECTION              =0x0000000000020000, // пассивная: запускать, если есть выделение
	MFLAGS_NOSELECTION             =0x0000000000040000, // активная:  запускать, если есть нет выделения
	MFLAGS_PNOSELECTION            =0x0000000000080000, // пассивная: запускать, если есть нет выделения
	MFLAGS_NOFILEPANELS            =0x0000000000100000, // активная:  запускать, если это плагиновая панель
	MFLAGS_PNOFILEPANELS           =0x0000000000200000, // пассивная: запускать, если это плагиновая панель
	MFLAGS_NOPLUGINPANELS          =0x0000000000400000, // активная:  запускать, если это файловая панель
	MFLAGS_PNOPLUGINPANELS         =0x0000000000800000, // пассивная: запускать, если это файловая панель
	MFLAGS_NOFOLDERS               =0x0000000001000000, // активная:  запускать, если текущий объект "файл"
	MFLAGS_PNOFOLDERS              =0x0000000002000000, // пассивная: запускать, если текущий объект "файл"
	MFLAGS_NOFILES                 =0x0000000004000000, // активная:  запускать, если текущий объект "папка"
	MFLAGS_PNOFILES                =0x0000000008000000, // пассивная: запускать, если текущий объект "папка"

	MFLAGS_POSTFROMPLUGIN          =0x0000000020000000, // последовательность пришла от АПИ
	MFLAGS_NEEDSAVEMACRO           =0x0000000040000000, // необходимо этот макрос запомнить
	MFLAGS_DISABLEMACRO            =0x0000000080000000, // этот макрос отключен
	MFLAGS_CALLPLUGINENABLEMACRO   =0x0000000100000000; // разрешить макросы при вызове плагина функцией CallPlugin


// коды возврата для KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE
{
	MACROMODE_NOMACRO          =0,  // не в режиме макро
	MACROMODE_EXECUTING        =1,  // исполнение: без передачи плагину пимп
	MACROMODE_EXECUTING_COMMON =2,  // исполнение: с передачей плагину пимп
	MACROMODE_RECORDING        =3,  // запись: без передачи плагину пимп
	MACROMODE_RECORDING_COMMON =4,  // запись: с передачей плагину пимп
};

class Panel;

struct TMacroFunction;
typedef bool (*INTMACROFUNC)(const TMacroFunction*);

enum INTMF_FLAGS{
	IMFF_UNLOCKSCREEN               =0x00000001,
	IMFF_DISABLEINTINPUT            =0x00000002,
};

struct TMacroFunction
{
	const wchar_t *Name;             // имя функции
	const wchar_t *fnGUID;           // GUID обработчика функции

	//wchar_t  *Src;                   // оригинальный "текст" макроса
	//wchar_t  *Description;           // описание макроса

	const wchar_t *Syntax;           // Синтаксис функции

	INTMACROFUNC Func;               // функция
	DWORD *Buffer;                   // компилированная последовательность (OpCode) макроса
	int    BufferSize;               // Размер буфера компилированной последовательности
	DWORD IntFlags;                  // флаги из INTMF_FLAGS (в основном отвечающие "как вызывать функцию")
	TMacroOpCode Code;               // байткод функции
};

struct MacroRecord
{
	MACROFLAGS_MFLAGS  Flags;        // Флаги макропоследовательности
	wchar_t *Name;                   // имя записи, может совпадать с именем клавиши
	int    Key;                      // Назначенная клавиша
	int    BufferSize;               // Размер буфера компилированной последовательности
	DWORD *Buffer;                   // компилированная последовательность (OpCode) макроса
	wchar_t  *Src;                   // оригинальный "текст" макроса
	wchar_t  *Description;           // описание макроса
	GUID Guid;                       // Гуид владельца макроса
	void* Id;                        // параметр калбака
	FARMACROCALLBACK Callback;       // каллбак для плагинов
};

#define STACKLEVEL      32

struct MacroState
{
	INPUT_RECORD cRec; // "описание реально нажатой клавиши"
	int Executing;
	struct MacroRecord *MacroWORK; // т.н. текущее исполнение
	TVarTable *locVarTable;
	int KeyProcess;
	int MacroPC;
	int ExecLIBPos;
	int MacroWORKCount;
	DWORD HistoryDisable;
	bool UseInternalClipboard;
	bool AllocVarTable;

	void Init(TVarTable *tbl);
};


struct MacroPanelSelect {
	__int64 Index;
	TVar    *Item;
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
};

/* $TODO:
    1. Удалить IndexMode[], Sort()
    2. Из MacroLIB сделать
       struct MacroRecord *MacroLIB[MACRO_LAST];
*/
class KeyMacro
{
	private:
		DWORD MacroVersion;

		static DWORD LastOpCodeUF; // последний не занятый OpCode для UserFunction (относительно KEY_MACRO_U_BASE)
		// для функций
		static size_t CMacroFunction;
		static size_t AllocatedFuncCount;
		static TMacroFunction *AMacroFunction;

		// тип записи - с вызовом диалога настроек или...
		// 0 - нет записи, 1 - простая запись, 2 - вызов диалога настроек
		int Recording;
		int InternalInput;
		int IsRedrawEditor;

		int Mode;
		int StartMode;

		struct MacroState Work;
		struct MacroState PCStack[STACKLEVEL];
		int CurPCStack;

		bool StopMacro;

		// сюда "могут" писать только при чтении макросов (занесение нового),
		// а исполнять через MacroWORK
		int MacroLIBCount;
		struct MacroRecord *MacroLIB;

		int IndexMode[MACRO_LAST][2];

		int RecBufferSize;
		DWORD *RecBuffer;
		wchar_t *RecSrc;
		wchar_t *RecDescription;

		class LockScreen *LockScr;

	private:
		void DestroyMacroLib();
		int ReadVarsConst(int ReadMode, string &strBuffer);
		int ReadMacroFunction(int ReadMode, string &strBuffer);
		int WriteVarsConst(int WriteMode);
		int ReadMacros(int ReadMode, string &strBuffer);
		void SavePluginFunctionToDB(const TMacroFunction *MF);
		void SaveMacroRecordToDB(const MacroRecord *MR);
		void ReadVarsConsts();
		void ReadPluginFunctions();
		int ReadKeyMacro(int Area);

		void WriteVarsConsts();
		void WritePluginFunctions();
		void WriteMacroRecords();
		int AssignMacroKey(DWORD &MacroKey, UINT64 &Flags);
		int GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
		void InitInternalVars(BOOL InitedRAM=TRUE);
		void InitInternalLIBVars();
		void ReleaseWORKBuffer(BOOL All=FALSE); // удалить временный буфер

		UINT64 SwitchFlags(UINT64& Flags,UINT64 Value);

		BOOL CheckEditSelected(UINT64 CurFlags);
		BOOL CheckInsidePlugin(UINT64 CurFlags);
		BOOL CheckPanel(int PanelMode,UINT64 CurFlags, BOOL IsPassivePanel);
		BOOL CheckCmdLine(int CmdLength,UINT64 Flags);
		BOOL CheckFileFolder(Panel *ActivePanel,UINT64 CurFlags, BOOL IsPassivePanel);
		BOOL CheckAll(int CheckMode,UINT64 CurFlags);
		void Sort();
		TVar FARPseudoVariable(UINT64 Flags,DWORD Code,DWORD& Err);
		DWORD GetOpCode(struct MacroRecord *MR,int PC);
		DWORD SetOpCode(struct MacroRecord *MR,int PC,DWORD OpCode);

	private:
		static intptr_t WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
		static intptr_t WINAPI ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);

	public:
		KeyMacro();
		~KeyMacro();

	public:
		int ProcessEvent(const struct FAR_INPUT_RECORD *Rec);
		int GetKey();
		int PeekKey();
		bool IsOpCode(DWORD p);
		bool CheckWaitKeyFunc();

		int PushState(bool CopyLocalVars=FALSE);
		int PopState();
		int GetLevelState() {return CurPCStack;};

		int  IsRecording() {return(Recording);};
		int  IsExecuting() {return(Work.Executing);};
		int  IsExecutingLastKey();
		int  IsDsableOutput() {return CheckCurMacroFlags(MFLAGS_DISABLEOUTPUT);};
		void SetMode(int Mode) {KeyMacro::Mode=Mode;};
		int  GetMode() {return(Mode);};

		void DropProcess();

		// послать сигнал на прерывание макроса
		void SendDropProcess();

		void RunStartMacro();

		// Поместить временное строковое представление макроса
		int PostNewMacro(const wchar_t *PlainText,UINT64 Flags=0,DWORD AKey=0,bool onlyCheck=false);
		// Поместить временный рекорд (бинарное представление)
		int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0,bool IsPluginSend=false);

		bool LoadVarFromDB(const wchar_t *Name, TVar &Value);
		bool SaveVarToDB(const wchar_t *Name, TVar Value);

		int  LoadMacros(BOOL InitedRAM=TRUE,BOOL LoadAll=TRUE);
		void SaveMacros();

		int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST-1?Mode:MACRO_LAST-1][0];}
		// Функция получения индекса нужного макроса в массиве
		int GetIndex(int Key, string& strKey, int CheckMode, bool UseCommon=true, bool StrictKeys=false);
		#if 0
		// получение размера, занимаемого указанным макросом
		int GetRecordSize(int Key, int Mode);
		#endif

		bool GetPlainText(string& Dest);
		int  GetPlainTextSize();

		//void SetRedrawEditor(int Sets) {IsRedrawEditor=Sets;}

		void RestartAutoMacro(int Mode);

		// получить данные о макросе (возвращает статус)
		int GetCurRecord(struct MacroRecord* RBuf=nullptr,int *KeyPos=nullptr);
		// проверить флаги текущего исполняемого макроса.
		BOOL CheckCurMacroFlags(DWORD Flags);

		bool IsHistoryDisable(int TypeHistory);
		DWORD SetHistoryDisableMask(DWORD Mask);
		DWORD GetHistoryDisableMask();

		static const wchar_t* GetAreaName(int AreaCode);
		static int   GetAreaCode(const wchar_t *AreaName);
		static int   GetMacroKeyInfo(bool FromDB,int Mode,int Pos,string &strKeyName,string &strDescription);
		static wchar_t *MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src=nullptr);
		// из строкового представления макроса сделать MacroRecord
		int ParseMacroString(struct MacroRecord *CurMacro,const wchar_t *BufPtr,bool onlyCheck=false);
		BOOL GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc);
		BOOL GetMacroParseError(string *Err1, string *Err2, string *Err3, string *Err4);

		static void SetMacroConst(const wchar_t *ConstName, const TVar& Value);
		static DWORD GetNewOpCode();

		static size_t GetCountMacroFunction();
		static const TMacroFunction *GetMacroFunction(size_t Index);
		static void RegisterMacroIntFunction();
		static TMacroFunction *RegisterMacroFunction(const TMacroFunction *tmfunc);
		static bool UnregMacroFunction(size_t Index);

		int AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback);
		int DelMacro(const GUID& PluginId,void* Id);
		void DelMacro(size_t Index);

		int GetCurrentCallPluginMode();

		static TVarTable *GetLocalVarTable();
};

BOOL KeyMacroToText(int Key,string &strKeyText0);
int KeyNameMacroToKey(const wchar_t *Name);
void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const wchar_t *name);
const wchar_t *eStackAsString(int Pos=0);

inline bool IsMenuArea(int Area){return Area==MACRO_MAINMENU || Area==MACRO_MENU || Area==MACRO_DISKS || Area==MACRO_USERMENU || Area==MACRO_SHELLAUTOCOMPLETION || Area==MACRO_DIALOGAUTOCOMPLETION;}


const wchar_t* GetAreaName(DWORD AreaValue);
DWORD GetAreaValue(const wchar_t* AreaName);

const wchar_t* GetFlagName(DWORD FlagValue);
DWORD GetFlagValue(const wchar_t* FlagName);

class TVMStack: public TStack<TVar>
{
	private:
		const TVar Error;

	public:
		TVMStack() {}
		~TVMStack() {}

	public:
		const TVar &Pop();
		TVar &Pop(TVar &dest);
		void Swap();
		const TVar &Peek();
};

extern TVarTable glbVarTable;
extern TVarTable glbConstTable;
extern TVMStack VMStack;
#endif
