#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.04 26.12.2000 $ */

/*
Modify:
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

struct MacroRecord
{
  DWORD Flags;
  int   Key;
  int   BufferSize;
  int  *Buffer;
};

class KeyMacro
{
  private:
    /* $ 10.09.2000 SVS
      ! Функция ReadMacros имеет дополнительные аргументы
    */
    struct TKeyNames{
      char Name[32];
      int  Code;
    };

    class LockScreen *LockScr;

    struct MacroRecord *Macros;
    int MacrosNumber;
    int Recording;
    int *RecBuffer;
    int RecBufferSize;
    int Executing;
    int ExecMacroPos;
    int ExecKeyPos;
    int InternalInput;
    int Mode;
    int StartMode;
    int StartMacroPos;

    struct MacroRecord *TempMacro; // временный буфер для 1 макро
    int TempMacroType;             // тип этого буфера

  private:
    int  ReadMacros(int ReadMode,struct TKeyNames *KeyNames,
                    int CountKeyNames, char *Buffer, int BufferSize);
    int GetMacroSettings(int &DisableOutput,int &RunAfterStart,
                         int &EmptyCommandLine,int &NotEmptyCommandLine,
                         int &FilePanels,int &PluginPanels);
    void InitVars();
    void ReleaseTempBuffer(); // удалить временный буфер

    // из строкового представления макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,char *BufPtr);
    DWORD SwitchFlags(DWORD& Flags,DWORD Value);

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

    // "играть" строковое представление макроса
    int PlayKeyMacro(char *KeyBuffer);
    // "играть" рекорд (бинарное представление)
    int PlayKeyMacro(struct MacroRecord *MRec);

    int  LoadMacros();
    void SaveMacros();

    static char* GetSubKey(int Mode);
    static int   GetSubKey(char *Mode);
};

#endif	// __KEYMACRO_HPP__
