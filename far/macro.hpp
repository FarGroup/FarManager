#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

Макросы

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class KeyMacro
{
  private:
    void ReadMacros(int ReadMode);
    void KeyToText(int Key,char *KeyName);
    int GetMacroSettings(int &DisableOutput,int &RunAfterStart,
                         int &EmptyCommandLine,int &NotEmptyCommandLine);

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
  public:
    KeyMacro();
    ~KeyMacro();
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();
    int IsRecording() {return(Recording);};
    int IsExecuting() {return(Executing);};
    void SaveMacros();
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int GetMode() {return(Mode);};
    void RunStartMacro();
};

#endif	// __KEYMACRO_HPP__
