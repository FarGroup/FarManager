#ifndef __COMMANDLINE_HPP__
#define __COMMANDLINE_HPP__
/*
cmdline.hpp

Командная строка

*/

/* Revision: 1.20 25.05.2006 $ */

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
    void DisplayObject();
    int CmdExecute(const wchar_t *CmdLine,int AlwaysWaitFinish,int SeparateWindow,int DirectRun);
    int ProcessOSCommands(const wchar_t *CmdLine,int SeparateWindow);
    void GetPrompt(string &strDestStr);
    BOOL SetLastCmdStr(const wchar_t *Ptr);

  public:
    CommandLine();
    ~CommandLine();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    int GetCurDirW(string &strCurDir);
    void SetCurDirW(const wchar_t *CurDir);

    void GetStringW (string &strStr);

    void SetStringW(const wchar_t *Str,BOOL Redraw=TRUE);
    int GetLength() {return(CmdStr.GetLength());};
    void ExecString(const wchar_t *Str,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE);

    void ShowViewEditHistory();

    void InsertStringW(const wchar_t *Str);

    void SetCurPos(int Pos);
    int GetCurPos();

    void SetPersistentBlocks(int Mode);

    void GetSelStringW (string &strStr);


    void Select(int,int);

    void GetSelection(int &Start,int &End);
    void SaveBackground(int X1,int Y1,int X2,int Y2);
    void SaveBackground();
    void ShowBackground();
    void CorrectRealScreenCoord();
    void ResizeConsole();
    void LockUpdatePanel(int Mode) {Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);};
};

#endif  // __COMMANDLINE_HPP__
