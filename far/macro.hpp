#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.31 28.10.2003 $ */

/*
Modify:
  28.10.2003 SVS
    ! Executing -> MacroState.Executing
  15.10.2003 SVS
    + GetMacroKeyInfo - информация об очередной макроклавише.
    + Сохранение/восстановление макроокружения.
  04.10.2003 SVS
    ! Куча переделок - все описание см. 01715.Macro.txt
  26.09.2003 SVS
    ! Переименование
      GetMacroPlainText        -> GetPlainText
      GetMacroPlainTextSize    -> GetPlainTextSize
  22.09.2003 SVS
    + KeyMacro::KeyToBuffer() - поместить код в буфер
    ! KeyMacro::IfCondition() - возвращает int
  12.09.2003 SVS
    + Добавлена функция KeyMacro::GetMacroPlainTextSize() - размер plain-text
  15.07.2003 SVS
    + KeyMacro::CheckInsidePlugin() - "мы внутри плагина?"
    + KeyMacro::DropProcess() - прервать текущий исполняемый макрос.
  02.05.2003 SVS
    - BugZ#790 - Редактирование макроса самим собой прерывает его исполнение?
    + IsExecutingLastKey() - введем проверку на... "это последняя клавиша макрокоманды?"
  19.08.2002 SVS
    + KeyMacro::KeyFromBuffer() - юзать ее для получения нужной клавиши из
      буфера
  02.06.2002 SVS
    ! Внедрение const
    ! ParseMacroString стала public
  12.04.2002 SVS
    ! Уберем #if/#endif - выбор на уровне MAK-файла (технологический патч)
    ! SaveMacros - один параметр
  03.03.2002 SVS
    + TempMacroNumber - количество структур во временной очереди
  10.12.2001 SVS
    + IsDsableOutput() - проверка на "отображаемость"
  14.09.2001 SVS
    - BugZ#9 - окончание
  07.09.2001 SVS
    + CheckCurMacroFlags() - проверка флагов текущего _ИСПОЛНЯЕМОГО_ макроса.
  15.08.2001 SVS
    ! косметика - для собственных нужд (по поводу macro2.?pp)
  09.08.2001 SVS
    + IfCondition() - вернет TRUE/FALSE в зависимости от условия
  22.06.2001 SVS
    + GetMacroPlainText()
  20.06.2001 SVS
    ! Названия функций приведены к более конкретному их назначению:
      PlayKeyMacro -> PostTempKeyMacro
    ! TempMacroType удален за ненадобностью, т.к. для Temp-макросов все равно
      память динамически перераспределяется.
  23.05.2001 SVS
    ! IndexMode - двумерный массив: первый индекс - начало, второй - количество.
  23.05.2001 SVS
    + Sort()
    + IndexMode - массив начала макросов в Macros
  16.05.2001 SVS
    + GetCurRecord() - для дампа
  06.05.2001 DJ
    ! перетрях #include
  25.04.2001 SVS
    ! Код проверки флагов для старта макросов вынесен в функции Check* -
      слишком много повторяющегося кода :-(
  08.03.2001 SVS
    + Функция MkTextSequence - формирование строкового представления Sequence
  22.01.2001 SVS
    + Функция MkRegKeyName - формирование имени ключа в реестре.
  17.01.2001 SVS
    + функции получения индекса макроса и размера под макропоследовательность:
       int GetIndex(int Key, int Mode);
       int GetRecordSize(int Key, int Mode);
  04.01.2001 SVS
    ! изменен ReadMacros и GetMacroSettings
    + функция AssignMacroKey
    ! удалена структура struct TKeyNames
  26.12.2000 SVS
    + SwitchFlags()
  23.12.2000 SVS
    + int ParseMacroString(struct MacroRecord *CurMacro,char *BufPtr)
    + int PlayKeyMacro(struct MacroRecord *MRec)
    + int PlayKeyMacro(char *KeyBuffer)
  21.12.2000 SVS
    ! структура MacroRecord перенесена из struct.hpp и "сжата"
    ! Функция KeyToText удалена за ненадобностью
  10.09.2000 SVS
    ! Функция ReadMacros имеет дополнительные аргументы
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/
#include "farconst.hpp"

class Panel;

struct MacroRecord
{
  DWORD Flags;
  int   Key;
  int   BufferSize;
  DWORD *Buffer;
};

#define STACKLEVEL	16

struct MacroState
{
  int Executing;
  int MacroPC;
  int ExecLIBPos;
  int MacroWORKCount;
  struct MacroRecord *MacroWORK; // т.н. текущее исполнение
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

    class LockScreen *LockScr;

  private:
    int ReadMacros(int ReadMode, char *Buffer, int BufferSize);
    DWORD AssignMacroKey();
    int GetMacroSettings(int Key,DWORD &Flags);
    void InitInternalVars(BOOL InitedRAM=TRUE);
    void InitInternalLIBVars();
    void ReleaseWORKBuffer(BOOL All=FALSE); // удалить временный буфер

    DWORD SwitchFlags(DWORD& Flags,DWORD Value);
    char *MkRegKeyName(int IdxMacro,char *RegKeyName);

    BOOL CheckEditSelected(DWORD CurFlags);
    BOOL CheckInsidePlugin(DWORD CurFlags);
    BOOL CheckPanel(int PanelMode,DWORD CurFlags, BOOL IsPassivePanel);
    BOOL CheckCmdLine(int CmdLength,DWORD Flags);
    BOOL CheckFileFolder(Panel *ActivePanel,DWORD CurFlags, BOOL IsPassivePanel);
    BOOL CheckAll(int CheckMode,DWORD CurFlags);
    void Sort(void);
    int  IfCondition(DWORD Key,DWORD Flags,DWORD Code);
    DWORD GetOpCode(struct MacroRecord *MR,int PC);
    DWORD SetOpCode(struct MacroRecord *MR,int PC,DWORD OpCode);

  private:
    static long WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long WINAPI ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

  public:
    KeyMacro();
    ~KeyMacro();

  public:
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();

    int PushState();
    int PopState();
    int GetLevelState(){return CurPCStack;};

    int  IsRecording() {return(Recording);};
    int  IsExecuting() {return(Work.Executing);};
    int  IsExecutingLastKey();
    int  IsDsableOutput() {return CheckCurMacroFlags(MFLAGS_DISABLEOUTPUT);};
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int  GetMode() {return(Mode);};

    void RunStartMacro();

    // Поместить временное строковое представление макроса
    int PostNewMacro(char *PlainText,DWORD Flags=0);
    // Поместить временный рекорд (бинарное представление)
    int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0);

    int  LoadMacros(BOOL InitedRAM=TRUE);
    void SaveMacros(BOOL AllSaved=TRUE);

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST?Mode:MACRO_LAST][0];}
    // Функция получения индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    char *GetPlainText(char *Dest);
    int   GetPlainTextSize();

    void SetRedrawEditor(int Sets){IsRedrawEditor=Sets;}

    // получить данные о макросе (возвращает статус)
    int GetCurRecord(struct MacroRecord* RBuf=NULL,int *KeyPos=NULL);
    // проверить флаги текущего исполняемого макроса.
    BOOL CheckCurMacroFlags(DWORD Flags);

    static char* GetSubKey(int Mode);
    static int   GetSubKey(char *Mode);
    static int   GetMacroKeyInfo(int Mode,int Pos,char *KeyName,char *Description,int DescriptionSize);
    static char *MkTextSequence(DWORD *Buffer,int BufferSize);
    // из строкового представления макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,const char *BufPtr);
    void DropProcess();
};

#endif	// __KEYMACRO_HPP__
