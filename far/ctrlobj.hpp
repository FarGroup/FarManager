#ifndef __CONTROLOBJECT_HPP__
#define __CONTROLOBJECT_HPP__
/*
ctrlobj.hpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши

*/

/* Revision: 1.01 15.07.2000 $ */

/*
Modify:
  15.07.2000 tran
    + new method Redraw()
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class ControlObject:public BaseInput
{
  private:
    Panel* ControlObject::CreatePanel(int Type);
    void DeletePanel(Panel *Deleted);
    void ShowCopyright();
    int EndLoop;
    int LastLeftType,LastRightType;
    int LeftStateBeforeHide,RightStateBeforeHide,HideState;
    Panel *LastLeftFilePanel,*LastRightFilePanel;
  public:
    ControlObject();
    ~ControlObject();
    void Init();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void EnterMainLoop();
    void ExitMainLoop(int Ask);
    Panel* GetAnotherPanel(Panel *Current);
    Panel* ChangePanelToFilled(Panel *Current,int NewType);
    Panel* ChangePanel(Panel *Current,int NewType,int CreateNew,int Force);
    void SetPanelPositions(int LeftFullScreen,int RightFullScreen);
    void SetScreenPositions();
    void RedrawKeyBar();
    /* $ 15.07.2000 tran
       here is :) */
    void Redraw();
    /* tran 15.07.2000 $ */

    Panel *LeftPanel,*RightPanel,*ActivePanel;

    Manager ModalManager;
    CommandLine CmdLine;
    History *CmdHistory,*FolderHistory,*ViewHistory;
    KeyBar MainKeyBar;
    MenuBar TopMenuBar;
    HighlightFiles HiFiles;
    GroupSort GrpSort;
    FilePositionCache ViewerPosCache,EditorPosCache;
    KeyMacro Macro;
    PluginsSet Plugins;
};


#endif // __CONTROLOBJECT_HPP__
