#ifndef __COMMANDLINE_HPP__
#define __COMMANDLINE_HPP__
/*
cmdline.hpp

Командная строка

*/

/* Revision: 1.05 13.08.2001 $ */

/*
Modify:
  13.08.2001 SKV
    + GetSelString, Select
  17.05.2001 OT
    - Отрисовка при изменении размеров консоли - ResizeConsole().
  11.05.2001 OT
    ! Новые методы для отрисовки Background
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
    SaveScreen *BackgroundScreen;
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
    /* $ 10.05.2001 DJ */
    void ShowViewEditHistory();
    /* DJ $ */
    void InsertString(char *Str);
    void SetCurPos(int Pos);
    int GetCurPos();
    /* $ 11.05.2001 OT */

    /*$ 13.08.2001 SKV*/
    void GetSelString(char*,int);
    void Select(int,int);
    /* SKV$*/
    void SaveBackground(int X1,int Y1,int X2,int Y2);
    void SaveBackground();
    void ShowBackground();
    /* OT $ */
    void ResizeConsole();
};

#endif  // __COMMANDLINE_HPP__
