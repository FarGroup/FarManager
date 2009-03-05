#ifndef __KEYMACRO_HPP__
#define __KEYMACRO_HPP__
/*
macro.hpp

ћакросы
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
  int UsedInternalClipboard;
  struct MacroRecord *MacroWORK; // т.н. текущее исполнение

  bool AllocVarTable;
  TVarTable *locVarTable;

  void Init(TVarTable *tbl);
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
    void Sort(void);
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

    // ѕоместить временное строковое представление макроса
    int PostNewMacro(const wchar_t *PlainText,DWORD Flags=0,DWORD AKey=0,BOOL onlyCheck=FALSE);
    // ѕоместить временный рекорд (бинарное представление)
    int PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag=0,BOOL IsPluginSend=FALSE);

    int  LoadMacros(BOOL InitedRAM=TRUE);
    void SaveMacros(BOOL AllSaved=TRUE);

    int GetStartIndex(int Mode) {return IndexMode[Mode<MACRO_LAST-1?Mode:MACRO_LAST-1][0];}
    // ‘ункци€ получени€ индекса нужного макроса в массиве
    int GetIndex(int Key, int Mode, bool UseCommon=true);
    // получение размера, занимаемого указанным макросом
    int GetRecordSize(int Key, int Mode);

    bool GetPlainText(string& Dest);
    int  GetPlainTextSize();

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
    BOOL GetMacroParseError(string *ErrMsg1,string *ErrMsg2,string *ErrMsg3);

    static void SetMacroConst(const wchar_t *ConstName, const TVar Value);
};

#endif // __KEYMACRO_HPP__
