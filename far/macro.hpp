#if defined(DMACRO2)
#include "macro2.hpp"
#else
#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.17 07.09.2001 $ */

/*
Modify:
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
    2. Из Macros сделать
       struct MacroRecord *Macros[MACRO_LAST];
*/
class KeyMacro
{
  private:
    /* $ 10.09.2000 SVS
      ! Функция ReadMacros имеет дополнительные аргументы
    */
    class LockScreen *LockScr;

    struct MacroRecord *Macros;
    int MacrosNumber;

    // тип записи - с вызовом диалога настроек или...
    // 0 - нет записи, 1 - простая запись, 2 - вызов диалога настроек
    int Recording;

    DWORD *RecBuffer;
    int IndexMode[MACRO_LAST][2];
    int RecBufferSize;
    int Executing;
    int ExecMacroPos;
    int ExecKeyPos;
    int InternalInput;
    int Mode;
    int StartMode;
    int StartMacroPos;

    struct MacroRecord *TempMacro; // временный буфер для 1 макро

  private:
    int ReadMacros(int ReadMode, char *Buffer, int BufferSize);
    DWORD AssignMacroKey();
    int GetMacroSettings(int Key,DWORD &Flags);
    void InitVars();
    void ReleaseTempBuffer(); // удалить временный буфер

    // из строкового представления макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,char *BufPtr);
    DWORD SwitchFlags(DWORD& Flags,DWORD Value);
    static long WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    char *MkRegKeyName(int IdxMacro,char *RegKeyName);

    BOOL CheckEditSelected(DWORD CurFlags);
    BOOL CheckPanel(int PanelMode,DWORD CurFlags);
    BOOL CheckCmdLine(int CmdLength,DWORD Flags);
    BOOL CheckFileFolder(Panel *ActivePanel,DWORD CurFlags);
    BOOL CheckAll(DWORD CurFlags);
    void Sort(void);
    BOOL IfCondition(DWORD Key,DWORD Flags,DWORD Code);

  public:
    KeyMacro();
    ~KeyMacro();

  public:
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();

    int  IsRecording() {return(Recording);};
    int  IsExecuting() {return(Executing);};
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int  GetMode() {return(Mode);};

    void RunStartMacro();

    // Поместить временное строковое представление макроса
    int PostTempKeyMacro(char *KeyBuffer);
    // Поместить временный рекорд (бинарное представление)
    int PostTempKeyMacro(struct MacroRecord *MRec);

    int  LoadMacros();
    void SaveMacros();

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST?Mode:MACRO_LAST][0];}
    // Функция получения индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    char *GetMacroPlainText(char *Dest);

    // получить данные о макросе (возвращает статус)
    int GetCurRecord(struct MacroRecord* RBuf=NULL,int *KeyPos=NULL);
    // проверить флаги текущего исполняемого макроса.
    BOOL CheckCurMacroFlags(DWORD Flags);

    static char* GetSubKey(int Mode);
    static int   GetSubKey(char *Mode);
    static char *MkTextSequence(DWORD *Buffer,int BufferSize);
};

#endif	// __KEYMACRO_HPP__
#endif // defined(DMACRO2)
