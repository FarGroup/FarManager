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
	MACRO_AUTOCOMPLETION       =  15, // Список автодополнения

	MACRO_COMMON,                     // ВЕЗДЕ! - должен быть предпоследним, т.к. приоритет самый низший !!!
	MACRO_LAST                        // Должен быть всегда последним! Используется в циклах
};

enum MACROFLAGS_MFLAGS
{
	MFLAGS_MODEMASK            =0x000000FF, // маска для выделения области действия (области начала исполнения) макроса

	MFLAGS_DISABLEOUTPUT       =0x00000100, // подавить обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS =0x00000200, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTARTED  =0x00000400, // этот макрос уже запускался при старте ФАРа
	MFLAGS_RUNAFTERFARSTART    =0x00000800, // этот макрос запускается при старте ФАРа

	MFLAGS_EMPTYCOMMANDLINE    =0x00001000, // запускать, если командная линия пуста
	MFLAGS_NOTEMPTYCOMMANDLINE =0x00002000, // запускать, если командная линия не пуста
	MFLAGS_EDITSELECTION       =0x00004000, // запускать, если есть выделение в редакторе
	MFLAGS_EDITNOSELECTION     =0x00008000, // запускать, если есть нет выделения в редакторе

	MFLAGS_SELECTION           =0x00010000, // активная:  запускать, если есть выделение
	MFLAGS_PSELECTION          =0x00020000, // пассивная: запускать, если есть выделение
	MFLAGS_NOSELECTION         =0x00040000, // активная:  запускать, если есть нет выделения
	MFLAGS_PNOSELECTION        =0x00080000, // пассивная: запускать, если есть нет выделения
	MFLAGS_NOFILEPANELS        =0x00100000, // активная:  запускать, если это плагиновая панель
	MFLAGS_PNOFILEPANELS       =0x00200000, // пассивная: запускать, если это плагиновая панель
	MFLAGS_NOPLUGINPANELS      =0x00400000, // активная:  запускать, если это файловая панель
	MFLAGS_PNOPLUGINPANELS     =0x00800000, // пассивная: запускать, если это файловая панель
	MFLAGS_NOFOLDERS           =0x01000000, // активная:  запускать, если текущий объект "файл"
	MFLAGS_PNOFOLDERS          =0x02000000, // пассивная: запускать, если текущий объект "файл"
	MFLAGS_NOFILES             =0x04000000, // активная:  запускать, если текущий объект "папка"
	MFLAGS_PNOFILES            =0x08000000, // пассивная: запускать, если текущий объект "папка"

	MFLAGS_REG_MULTI_SZ        =0x10000000, // текст макроса многострочный (REG_MULTI_SZ)
	MFLAGS_POSTFROMPLUGIN      =0x20000000, // последовательность пришла от АПИ
	MFLAGS_NEEDSAVEMACRO       =0x40000000, // необходимо этот макрос запомнить
	MFLAGS_DISABLEMACRO        =0x80000000, // этот макрос отключен
};


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
	int nParam;                      // количество параметров
	int oParam;                      // необязательные параметры
	TMacroOpCode Code;               // байткод функции
	const wchar_t *fnGUID;           // GUID обработчика функции

	int    BufferSize;               // Размер буфера компилированной последовательности
	DWORD *Buffer;                   // компилированная последовательность (OpCode) макроса
	//wchar_t  *Src;                   // оригинальный "текст" макроса
	//wchar_t  *Description;           // описание макроса

	const wchar_t *Syntax;           // Синтаксис функции

	DWORD IntFlags;                  // флаги из INTMF_FLAGS (в основном отвечающие "как вызывать функцию")
	INTMACROFUNC Func;               // функция
};

struct MacroRecord
{
	UINT64  Flags;         // Флаги макропоследовательности
	int    Key;           // Назначенная клавиша
	int    BufferSize;    // Размер буфера компилированной последовательности
	DWORD *Buffer;        // компилированная последовательность (OpCode) макроса
	wchar_t  *Src;           // оригинальный "текст" макроса
	wchar_t  *Description;   // описание макроса
	DWORD  Reserved[2];   // зарезервировано
};

#define STACKLEVEL      32

struct MacroState
{
	int KeyProcess;
	int Executing;
	int MacroPC;
	int ExecLIBPos;
	int MacroWORKCount;
	DWORD HistroyEnable;
	bool UseInternalClipboard;
	struct MacroRecord *MacroWORK; // т.н. текущее исполнение
	INPUT_RECORD cRec; // "описание реально нажатой клавиши"

	bool AllocVarTable;
	TVarTable *locVarTable;

	void Init(TVarTable *tbl);
};


struct MacroPanelSelect {
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
	__int64 Index;
	TVar    *Item;
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

		class LockScreen *LockScr;

	private:
		int ReadVarsConst(int ReadMode, string &strBuffer);
		int ReadMacroFunction(int ReadMode, string &strBuffer);
		int WriteVarsConst(int WriteMode);
		int ReadMacros(int ReadMode, string &strBuffer);
		DWORD AssignMacroKey();
		int GetMacroSettings(int Key,DWORD &Flags);
		void InitInternalVars(BOOL InitedRAM=TRUE);
		void InitInternalLIBVars();
		void ReleaseWORKBuffer(BOOL All=FALSE); // удалить временный буфер

		UINT64 SwitchFlags(UINT64& Flags,UINT64 Value);
		string &MkRegKeyName(int IdxMacro,string &strRegKeyName);

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
		static INT_PTR WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
		static INT_PTR WINAPI ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);

	public:
		KeyMacro();
		~KeyMacro();

	public:
		int ProcessKey(int Key);
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
		int PostNewMacro(const wchar_t *PlainText,UINT64 Flags=0,DWORD AKey=0,BOOL onlyCheck=FALSE);
		// Поместить временный рекорд (бинарное представление)
		int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0,BOOL IsPluginSend=FALSE);

		int  LoadMacros(BOOL InitedRAM=TRUE,BOOL LoadAll=TRUE);
		void SaveMacros(BOOL AllSaved=TRUE);

		int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST-1?Mode:MACRO_LAST-1][0];}
		// Функция получения индекса нужного макроса в массиве
		int GetIndex(int Key, int Mode, bool UseCommon=true);
		// получение размера, занимаемого указанным макросом
		int GetRecordSize(int Key, int Mode);

		bool GetPlainText(string& Dest);
		int  GetPlainTextSize();

		void SetRedrawEditor(int Sets) {IsRedrawEditor=Sets;}

		void RestartAutoMacro(int Mode);

		// получить данные о макросе (возвращает статус)
		int GetCurRecord(struct MacroRecord* RBuf=nullptr,int *KeyPos=nullptr);
		// проверить флаги текущего исполняемого макроса.
		BOOL CheckCurMacroFlags(DWORD Flags);

		bool IsHistroyEnable(int TypeHistory);

		static const wchar_t* GetSubKey(int Mode);
		static int   GetSubKey(const wchar_t *Mode);
		static int   GetMacroKeyInfo(bool FromReg,int Mode,int Pos,string &strKeyName,string &strDescription);
		static wchar_t *MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src=nullptr);
		// из строкового представления макроса сделать MacroRecord
		int ParseMacroString(struct MacroRecord *CurMacro,const wchar_t *BufPtr,BOOL onlyCheck=FALSE);
		BOOL GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc);
		BOOL GetMacroParseError(string *Err1, string *Err2, string *Err3, string *Err4);

		static void SetMacroConst(const wchar_t *ConstName, const TVar& Value);
		static DWORD GetNewOpCode();

		static size_t GetCountMacroFunction();
		static const TMacroFunction *GetMacroFunction(size_t Index);
		static void RegisterMacroIntFunction();
		static TMacroFunction *RegisterMacroFunction(const TMacroFunction *tmfunc);
		static bool UnregMacroFunction(size_t Index);
};

BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0);
int WINAPI KeyNameMacroToKey(const wchar_t *Name);
void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const wchar_t *name);
const wchar_t *eStackAsString(int Pos=0);

inline bool IsMenuArea(int Area){return Area==MACRO_MAINMENU || Area==MACRO_MENU || Area==MACRO_DISKS || Area==MACRO_USERMENU || Area==MACRO_AUTOCOMPLETION;}
