#ifndef __FILEPANELS_HPP__
#define __FILEPANELS_HPP__
/*
filepanels.hpp

файловые панели

*/

/* Revision: 1.06 21.05.2001 $ */

/*
Modify:
  21.05.2001 OT
    + Реакция на изменение размеров консоли virtual void ResizeConsole();
  15.05.2001 OT
    ! NWZ -> NFZ
  11.05.2001 OT
    ! Отрисовка Background
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  01.01.2001 tran
      created
*/

#include "frame.hpp"
#include "keybar.hpp"
#include "menubar.hpp"

class Panel;
class CommandLine;

class FilePanels:public Frame
{
  private:
    void DisplayObject();
    int Focus;
    typedef class Frame inherited;


  public:

    FilePanels();
    ~FilePanels();

    void Init();

    Panel *LastLeftFilePanel,
          *LastRightFilePanel;

    Panel* CreatePanel(int Type);
    void   DeletePanel(Panel *Deleted);
    Panel* GetAnotherPanel(Panel *Current);
    Panel* ChangePanelToFilled(Panel *Current,int NewType);
    Panel* ChangePanel(Panel *Current,int NewType,int CreateNew,int Force);
    void   SetPanelPositions(int LeftFullScreen,int RightFullScreen);
    void   SetPanelPositions();

    void   SetupKeyBar();

    virtual void Show();

    void Redraw();

    Panel *LeftPanel,
          *RightPanel,
          *ActivePanel;

    KeyBar      MainKeyBar;
    MenuBar     TopMenuBar;

    int LastLeftType,
        LastRightType;
    int LeftStateBeforeHide,
        RightStateBeforeHide;


    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);


    void SetScreenPositions();

    void Update();

    virtual int GetTypeAndName(char *Type,char *Name);
    virtual int GetType() { return MODALTYPE_PANELS; }

    virtual char *GetTypeName(){return "[FilePanels]";};
    virtual void OnChangeFocus(int focus);

    void RedrawKeyBar(); // virtual
    virtual void ShowConsoleTitle();
//OT
    virtual void ResizeConsole();
};

#endif // __FILEPANELS_HPP__
