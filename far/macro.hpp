#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.26 12.09.2003 $ */

/*
Modify:
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

/* $TODO:
    1. Удалить IndexMode[], Sort()
    2. Из MacroPROM сделать
       struct MacroRecord *MacroPROM[MACRO_LAST];
*/
class KeyMacro
{
  private:
    // тип записи - с вызовом диалога настроек или...
    // 0 - нет записи, 1 - простая запись, 2 - вызов диалога настроек
    int Recording;
    int Executing;
    int InternalInput;
    int IsRedrawEditor;

    int Mode;
    int ExecMacroPos;
    int ExecKeyPos;
    int StartMode;
    int StartMacroPos;

    // т.н. MacroPROM (Programmable Read-Only Memory) -
    // сюда "могут" писать только при чтении макросов (занесение нового),
    // а исполнять через MacroRAM
    int MacroPROMCount;
    struct MacroRecord *MacroPROM;

    // т.н. MacroRAM - текущее исполнение
    int MacroRAMCount;
    struct MacroRecord *MacroRAM;

    int IndexMode[MACRO_LAST][2];

    int RecBufferSize;
    DWORD *RecBuffer;


    class LockScreen *LockScr;

  private:
    int ReadMacros(int ReadMode, char *Buffer, int BufferSize);
    DWORD AssignMacroKey();
    int GetMacroSettings(int Key,DWORD &Flags);
    void InitVars(BOOL InitedRAM=TRUE);
    void InitVarsPROM();
    void ReleaseTempBuffer(BOOL All=FALSE); // удалить временный буфер

    DWORD SwitchFlags(DWORD& Flags,DWORD Value);
    static long WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    char *MkRegKeyName(int IdxMacro,char *RegKeyName);

    BOOL CheckEditSelected(DWORD CurFlags);
    BOOL CheckInsidePlugin(DWORD CurFlags);
    BOOL CheckPanel(int PanelMode,DWORD CurFlags);
    BOOL CheckCmdLine(int CmdLength,DWORD Flags);
    BOOL CheckFileFolder(Panel *ActivePanel,DWORD CurFlags);
    BOOL CheckAll(DWORD CurFlags);
    void Sort(void);
    BOOL IfCondition(DWORD Key,DWORD Flags,DWORD Code);
    DWORD KeyFromBuffer(struct MacroRecord *MR,int KeyPos);

  public:
    KeyMacro();
    ~KeyMacro();

  public:
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();

    int  IsRecording() {return(Recording);};
    int  IsExecuting() {return(Executing);};
    int  IsExecutingLastKey();
    int  IsDsableOutput() {return CheckCurMacroFlags(MFLAGS_DISABLEOUTPUT);};
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int  GetMode() {return(Mode);};

    void RunStartMacro();

    // Поместить временное строковое представление макроса
    int PostTempKeyMacro(char *KeyBuffer);
    // Поместить временный рекорд (бинарное представление)
    int PostTempKeyMacro(struct MacroRecord *MRec);

    int  LoadMacros(BOOL InitedRAM=TRUE);
    void SaveMacros(BOOL AllSaved=TRUE);

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST?Mode:MACRO_LAST][0];}
    // Функция получения индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    char *GetMacroPlainText(char *Dest);
    int   GetMacroPlainTextSize();

    void SetRedrawEditor(int Sets){IsRedrawEditor=Sets;}

    // получить данные о макросе (возвращает статус)
    int GetCurRecord(struct MacroRecord* RBuf=NULL,int *KeyPos=NULL);
    // проверить флаги текущего исполняемого макроса.
    BOOL CheckCurMacroFlags(DWORD Flags);

    static char* GetSubKey(int Mode);
    static int   GetSubKey(char *Mode);
    static char *MkTextSequence(DWORD *Buffer,int BufferSize);
    // из строкового представления макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,const char *BufPtr);
    void DropProcess();
};

#endif	// __KEYMACRO_HPP__
