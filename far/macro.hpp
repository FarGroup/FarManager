#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "syntax.hpp"
#include "tvar.hpp"

enum MACRODISABLEONLOAD{
  MDOL_ALL            = 0x80000000, // дисаблим все макросы при загрузке
  MDOL_AUTOSTART      = 0x00000001, // дисаблим автостартующие макросы
};

// области действия макросов (начало исполнения) -  НЕ БОЛЕЕ 0xFF областей!
enum MACROMODEAREA {
  MACRO_FUNC         =  -3,
  MACRO_CONSTS       =  -2,
  MACRO_VARS         =  -1,

  MACRO_OTHER        =   0, // Режим копирования текста с экрана, вертикальные меню
  MACRO_SHELL        =   1, // Файловые панели
  MACRO_VIEWER       =   2, // Внутренняя программа просмотра
  MACRO_EDITOR       =   3, // Редактор
  MACRO_DIALOG       =   4, // Диалоги
  MACRO_SEARCH       =   5, // Быстрый поиск в панелях
  MACRO_DISKS        =   6, // Меню выбора дисков
  MACRO_MAINMENU     =   7, // Основное меню
  MACRO_MENU         =   8, // Прочие меню
  MACRO_HELP         =   9, // Система помощи
  MACRO_INFOPANEL    =  10, // Информационная панель
  MACRO_QVIEWPANEL   =  11, // Панель быстрого просмотра
  MACRO_TREEPANEL    =  12, // Панель дерева папок
  MACRO_FINDFOLDER   =  13, // Поиск папок
  MACRO_USERMENU     =  14, // Меню пользователя

  MACRO_COMMON,             // ВЕЗДЕ! - должен быть предпоследним, т.к. приоритет самый низший !!!
  MACRO_LAST                // Должен быть всегда последним! Используется в циклах
};

enum MACROFLAGS_MFLAGS{
  MFLAGS_MODEMASK            =0x000000FF, // маска для выделения области действия (области начала исполнения) макроса

  MFLAGS_DISABLEOUTPUT       =0x00000100, // подавить обновление экрана во время выполнения макроса
  MFLAGS_NOSENDKEYSTOPLUGINS =0x00000200, // НЕ передавать клавиши во время записи/воспроизведения макроса
  MFLAGS_RUNAFTERFARSTARTED  =0x00000400, // этот макрос уже запускался при старте ФАРа
  MFLAGS_RUNAFTERFARSTART    =0x00000800, // этот макрос запускается при старте ФАРа

  MFLAGS_EMPTYCOMMANDLINE    =0x00001000, // запускать, если командная линия пуста
  MFLAGS_NOTEMPTYCOMMANDLINE =0x00002000, // запускать, если командная линия не пуста

  MFLAGS_SELECTION           =0x00004000, // активная:  запускать, если есть выделение
  MFLAGS_NOSELECTION         =0x00008000, // активная:  запускать, если есть нет выделения
  MFLAGS_PSELECTION          =0x00010000, // пассивная: запускать, если есть выделение
  MFLAGS_PNOSELECTION        =0x00020000, // пассивная: запускать, если есть нет выделения
  MFLAGS_EDITSELECTION       =0x00040000, // запускать, если есть выделение в редакторе
  MFLAGS_EDITNOSELECTION     =0x00080000, // запускать, если есть нет выделения в редакторе
  MFLAGS_NOFILEPANELS        =0x00100000, // активная:  запускать, если это плагиновая панель
  MFLAGS_NOPLUGINPANELS      =0x00200000, // активная:  запускать, если это файловая панель
  MFLAGS_PNOFILEPANELS       =0x00400000, // пассивная: запускать, если это плагиновая панель
  MFLAGS_PNOPLUGINPANELS     =0x00800000, // пассивная: запускать, если это файловая панель
  MFLAGS_NOFOLDERS           =0x01000000, // активная:  запускать, если текущий объект "файл"
  MFLAGS_PNOFOLDERS          =0x02000000, // пассивная: запускать, если текущий объект "файл"
  MFLAGS_PNOFILES            =0x04000000, // пассивная: запускать, если текущий объект "папка"
  MFLAGS_NOFILES             =0x08000000, // активная:  запускать, если текущий объект "папка"

  MFLAGS_REG_MULTI_SZ        =0x10000000, // REG_MULTI_SZ?
  MFLAGS_REUSEMACRO          =0x20000000, // повторное использование макросов (вызов макроса из макроса)
  MFLAGS_NEEDSAVEMACRO       =0x40000000, // необходимо этот макрос запомнить
  MFLAGS_DISABLEMACRO        =0x80000000, // этот макрос отключен
};


// коды возврата для KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE{
  MACROMODE_NOMACRO          =0,  // не в режиме макро
  MACROMODE_EXECUTING        =1,  // исполнение: без передачи плагину пимп
  MACROMODE_EXECUTING_COMMON =2,  // исполнение: с передачей плагину пимп
  MACROMODE_RECORDING        =3,  // запись: без передачи плагину пимп
  MACROMODE_RECORDING_COMMON =4,  // запись: с передачей плагину пимп
};

class Panel;

struct MacroRecord
{
  DWORD  Flags;         // Флаги макропоследовательности
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
  int UsedInternalClipboard;
  struct MacroRecord *MacroWORK; // т.н. текущее исполнение

  bool AllocVarTable;
  TVarTable *locVarTable;

  void Init(TVarTable *tbl);
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

    DWORD SwitchFlags(DWORD& Flags,DWORD Value);
    string &MkRegKeyName(int IdxMacro,string &strRegKeyName);

    BOOL CheckEditSelected(DWORD CurFlags);
    BOOL CheckInsidePlugin(DWORD CurFlags);
    BOOL CheckPanel(int PanelMode,DWORD CurFlags, BOOL IsPassivePanel);
    BOOL CheckCmdLine(int CmdLength,DWORD Flags);
    BOOL CheckFileFolder(Panel *ActivePanel,DWORD CurFlags, BOOL IsPassivePanel);
    BOOL CheckAll(int CheckMode,DWORD CurFlags);
		void Sort();
    TVar FARPseudoVariable(DWORD Flags,DWORD Code,DWORD& Err);
    DWORD GetOpCode(struct MacroRecord *MR,int PC);
    DWORD SetOpCode(struct MacroRecord *MR,int PC,DWORD OpCode);

  private:
    static LONG_PTR WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
    static LONG_PTR WINAPI ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

  public:
    KeyMacro();
    ~KeyMacro();

  public:
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();
    bool IsOpCode(DWORD p);

    int PushState(bool CopyLocalVars=FALSE);
    int PopState();
    int GetLevelState(){return CurPCStack;};

    int  IsRecording() {return(Recording);};
    int  IsExecuting() {return(Work.Executing);};
    int  IsExecutingLastKey();
    int  IsDsableOutput() {return CheckCurMacroFlags(MFLAGS_DISABLEOUTPUT);};
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int  GetMode() {return(Mode);};

    void DropProcess();

    void RunStartMacro();

    // Поместить временное строковое представление макроса
    int PostNewMacro(const wchar_t *PlainText,DWORD Flags=0,DWORD AKey=0,BOOL onlyCheck=FALSE);
    // Поместить временный рекорд (бинарное представление)
    int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0,BOOL IsPluginSend=FALSE);

    int  LoadMacros(BOOL InitedRAM=TRUE);
    void SaveMacros(BOOL AllSaved=TRUE);

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST-1?Mode:MACRO_LAST-1][0];}
    // Функция получения индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode, bool UseCommon=true);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    bool GetPlainText(string& Dest);
    int  GetPlainTextSize();

    void SetRedrawEditor(int Sets){IsRedrawEditor=Sets;}

    void RestartAutoMacro(int Mode);

    // получить данные о макросе (возвращает статус)
    int GetCurRecord(struct MacroRecord* RBuf=NULL,int *KeyPos=NULL);
    // проверить флаги текущего исполняемого макроса.
    BOOL CheckCurMacroFlags(DWORD Flags);

    static const wchar_t* GetSubKey(int Mode);
    static int   GetSubKey(const wchar_t *Mode);
    static int   GetMacroKeyInfo(bool FromReg,int Mode,int Pos,string &strKeyName,string &strDescription);
    static wchar_t *MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src=NULL);
    // из строкового представления макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,const wchar_t *BufPtr,BOOL onlyCheck=FALSE);
    BOOL GetMacroParseError(string *ErrMsg1,string *ErrMsg2,string *ErrMsg3);

    static void SetMacroConst(const wchar_t *ConstName, const TVar Value);
};

BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0);
int WINAPI KeyNameMacroToKey(const wchar_t *Name);
void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const wchar_t *name);
const wchar_t *eStackAsString(int Pos=0);

#endif // __KEYMACRO_HPP__
