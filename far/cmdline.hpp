#ifndef __COMMANDLINE_HPP__
#define __COMMANDLINE_HPP__
/*
cmdline.hpp

Командная строка

*/

#include "scrobj.hpp"
#include "edit.hpp"
#include "farconst.hpp"
#include "UnicodeString.hpp"

enum {
  FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
};

class CommandLine:public ScreenObject
{
  private:
    Edit CmdStr;
    SaveScreen *BackgroundScreen;
    string strCurDir;
    string strLastCmdStr;
    int  LastCmdPartLength;

  private:
    virtual void DisplayObject();
    int CmdExecute(const wchar_t *CmdLine,int AlwaysWaitFinish,int SeparateWindow,int DirectRun);
    int ProcessOSCommands(const wchar_t *CmdLine,int SeparateWindow);
    void GetPrompt(string &strDestStr);
    BOOL SetLastCmdStr(const wchar_t *Ptr);

  public:
    CommandLine();
    virtual ~CommandLine();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual int VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

    int GetCurDir(string &strCurDir);
    void SetCurDir(const wchar_t *CurDir);

    void GetString (string &strStr);

    void SetString(const wchar_t *Str,BOOL Redraw=TRUE);
    int GetLength() {return(CmdStr.GetLength());};
    void ExecString(const wchar_t *Str,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE);

    void ShowViewEditHistory();

    void InsertString(const wchar_t *Str);

    void SetCurPos(int Pos);
    int GetCurPos();

    void SetPersistentBlocks(int Mode);

    void GetSelString(string &strStr);


    void Select(int,int);

    void GetSelection(int &Start,int &End);
    void SaveBackground(int X1,int Y1,int X2,int Y2);
    void SaveBackground();
    void ShowBackground();
    void CorrectRealScreenCoord();
    virtual void ResizeConsole();
    void LockUpdatePanel(int Mode) {Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);};
};

#endif  // __COMMANDLINE_HPP__
