#ifndef __CONTROLOBJECT_HPP__
#define __CONTROLOBJECT_HPP__
/*
ctrlobj.hpp

”правление остальными объектами, раздача сообщений клавиатуры и мыши

*/

#include "macro.hpp"
#include "plugins.hpp"

class CommandLine;
class History;
class KeyBar;
class MenuBar;
class HighlightFiles;
class GroupSort;
class FilePositionCache;
class FilePanels;

class ControlObject
{
  private:
    FilePanels *FPanels;

  public:
    ControlObject();
    ~ControlObject();

  public:
    void Init();
//    int ProcessKey(int Key);
//    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
//    Panel* GetAnotherPanel(Panel *Current);
//    Panel* ChangePanelToFilled(Panel *Current,int NewType);
//    Panel* ChangePanel(Panel *Current,int NewType,int CreateNew,int Force);
//    void SetPanelPositions(int LeftFullScreen,int RightFullScreen);
//    void SetScreenPositions();
//    void RedrawKeyBar();
    /* $ 15.07.2000 tran
       here is :) */
//    void Redraw();
    /* tran 15.07.2000 $ */

  public:
    FilePanels *Cp();

    void CreateFilePanels();

    CommandLine *CmdLine;
    History *CmdHistory,*FolderHistory,*ViewHistory;
    KeyBar *MainKeyBar;
    MenuBar *TopMenuBar;
    HighlightFiles *HiFiles;
    FilePositionCache *ViewerPosCache,*EditorPosCache;
    KeyMacro Macro;
    PluginsSet Plugins;

    static void ShowCopyright(DWORD Flags=0);
};

extern ControlObject *CtrlObject;

#endif // __CONTROLOBJECT_HPP__
