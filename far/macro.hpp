#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.02 21.12.2000 $ */

/*
Modify:
  21.12.2000 SVS
    ! структура MacroRecord перенесена из struct.hpp и "сжата"
    ! Функция KeyToText удалена за ненадобностью
  10.09.2000 SVS
    ! Функция ReadMacros имеет дополнительные аргументы
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

// for class KeyMacro
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

  private:
    void ReadMacros(int ReadMode,struct TKeyNames *KeyNames,
                    int CountKeyNames, char *Buffer, int BufferSize);
    int GetMacroSettings(int &DisableOutput,int &RunAfterStart,
                         int &EmptyCommandLine,int &NotEmptyCommandLine,
                         int &FilePanels,int &PluginPanels);

  public:
    KeyMacro();
    ~KeyMacro();

  public:
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();
    int IsRecording() {return(Recording);};
    int IsExecuting() {return(Executing);};
    void SaveMacros();
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int GetMode() {return(Mode);};
    void RunStartMacro();
    static char* GetSubKey(int Mode);
    static int GetSubKey(char *Mode);
};

#endif	// __KEYMACRO_HPP__
