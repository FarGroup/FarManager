#ifndef __CONTROLOBJECT_HPP__
#define __CONTROLOBJECT_HPP__
/*
ctrlobj.hpp

���������� ���������� ���������, ������� ��������� ���������� � ����

*/

/* Revision: 1.08 21.11.2005 $ */

/*
Modify:
  21.11.2005 WARP
    + HistoryW
  12.05.2001 DJ
    ! FrameManager ������� �� CtrlObject
    ! ���������� ��������� �� CtrlObject �������� ����
  06.05.2001 DJ
    ! �������� #include
  05.05.2001 DJ
    + �������� NWZ
  29.04.2001 ��
    + ��������� NWZ �� ����������
  28.02.2001 IS
    ! CmdLine ������ ���������
  15.12.2000 SVS
    ! ����� ShowCopyright - public static & �������� Flags.
  15.07.2000 tran
    + new method Redraw()
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "macro.hpp"
#include "plugins.hpp"

class CommandLine;
class History;
class HistoryW;
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
    HistoryW *CmdHistory,*FolderHistory,*ViewHistory;

    KeyBar *MainKeyBar;
    MenuBar *TopMenuBar;
    HighlightFiles *HiFiles;
    GroupSort *GrpSort;
    FilePositionCache *ViewerPosCache,*EditorPosCache;
    KeyMacro Macro;
    PluginsSet Plugins;

    static void ShowCopyright(DWORD Flags=0);
};

extern ControlObject *CtrlObject;

#endif // __CONTROLOBJECT_HPP__
