#ifndef __COMMANDLINE_HPP__
#define __COMMANDLINE_HPP__
/*
cmdline.hpp

Командная строка

*/

/* Revision: 1.02 10.05.2001 $ */

/*
Modify:
  10.05.2001 DJ
    * ShowViewEditHistory()
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "scrobj.hpp"
#include "edit.hpp"
#include "farconst.hpp"

class CommandLine:public ScreenObject
{
  private:
    void DisplayObject();
    int CmdExecute(char *CmdLine,int AlwaysWaitFinish,int SeparateWindow,
                   int DirectRun);
    int ProcessOSCommands(char *CmdLine);
    void GetPrompt(char *DestStr);
    Edit CmdStr;
    char CurDir[NM];
    char LastCmdStr[256];
    int LastCmdPartLength;
  public:
    CommandLine();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void GetCurDir(char *CurDir);
    void SetCurDir(char *CurDir);
    void GetString(char *Str,int MaxSize);
    void SetString(char *Str);
    int GetLength() {return(CmdStr.GetLength());};
    void ExecString(char *Str,int AlwaysWaitFinish,int SeparateWindow=FALSE,
                    int DirectRun=FALSE);
    void InsertString(char *Str);
    void SetCurPos(int Pos);
    int GetCurPos();
    /* $ 10.05.2001 DJ */
    void ShowViewEditHistory();
    /* DJ $ */
};

#endif	// __COMMANDLINE_HPP__
