#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

ћакросы

*/

#include "farconst.hpp"
#include "syntax.hpp"
#include "UnicodeString.hpp"

class Panel;

struct MacroRecord
{
  DWORD  Flags;         // ‘лаги макропоследовательности
  int    Key;           // Ќазначенна€ клавиша
  int    BufferSize;    // –азмер буфера компилированной последовательности
  DWORD *Buffer;        // компилированна€ последовательность (OpCode) макроса
  wchar_t  *Src;           // оригинальный "текст" макроса
  DWORD  Reserved[3];   // зарезервировано
};

#define STACKLEVEL      32

struct MacroState
{
  int KeyProcess;
  int Executing;
  int MacroPC;
  int ExecLIBPos;
  int MacroWORKCount;
  struct MacroRecord *MacroWORK; // т.н. текущее исполнение
};

/* $TODO:
    1. ”далить IndexMode[], Sort()
    2. »з MacroLIB сделать
       struct MacroRecord *MacroLIB[MACRO_LAST];
*/
class KeyMacro
{
  private:
    DWORD MacroVersion;
    // тип записи - с вызовом диалога настроек или...
    // 0 - нет записи, 1 - проста€ запись, 2 - вызов диалога настроек
    int Recording;
    int InternalInput;
    int IsRedrawEditor;

    int Mode;
    int StartMode;

    struct MacroState Work;
    struct MacroState PCStack[STACKLEVEL];
    int CurPCStack;

    // сюда "могут" писать только при чтении макросов (занесение нового),
    // а исполн€ть через MacroWORK
    int MacroLIBCount;
    struct MacroRecord *MacroLIB;

    int IndexMode[MACRO_LAST][2];

    int RecBufferSize;
    DWORD *RecBuffer;

    class LockScreen *LockScr;

  private:
    int ReadVarsConst(int ReadMode, string &strBuffer);
    int WriteVarsConst(int ReadMode);
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
    void Sort(void);
    TVar FARPseudoVariable(DWORD Flags,DWORD Code);
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

    // ѕоместить временное строковое представление макроса
    int PostNewMacro(const wchar_t *PlainText,DWORD Flags=0);
    // ѕоместить временный рекорд (бинарное представление)
    int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0);

    int  LoadMacros(BOOL InitedRAM=TRUE);
    void SaveMacros(BOOL AllSaved=TRUE);

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST-1?Mode:MACRO_LAST-1][0];}
    // ‘ункци€ получени€ индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    wchar_t *GetPlainText(wchar_t *Dest);
    int   GetPlainTextSize();

    void SetRedrawEditor(int Sets){IsRedrawEditor=Sets;}

    void RestartAutoMacro(int Mode);

    // получить данные о макросе (возвращает статус)
    int GetCurRecord(struct MacroRecord* RBuf=NULL,int *KeyPos=NULL);
    // проверить флаги текущего исполн€емого макроса.
    BOOL CheckCurMacroFlags(DWORD Flags);

    static const wchar_t* GetSubKey(int Mode);
    static int   GetSubKey(const wchar_t *Mode);
    static int   GetMacroKeyInfo(int Mode,int Pos,const wchar_t *KeyName,string &strDescription);
    static wchar_t *MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src=NULL);
    // из строкового представлени€ макроса сделать MacroRecord
    int ParseMacroString(struct MacroRecord *CurMacro,const wchar_t *BufPtr);
    void DropProcess();
};

#endif // __KEYMACRO_HPP__
