#ifndef __FILEPANELS_HPP__
#define __FILEPANELS_HPP__
/*
filepanels.hpp

файловые панели

*/

/* Revision: 1.00 09.01.2001 $ */

/*
Modify:
  01.01.2001 tran
      created
*/

class FilePanels:public Modal
{
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

    void   SetupKeyBar();

    virtual void Show();

    void Redraw();

    Panel *LeftPanel,
          *RightPanel,
          *ActivePanel;

    CommandLine *CmdLine;
    KeyBar      MainKeyBar;
    MenuBar     TopMenuBar;

    int LastLeftType,
        LastRightType;
    int LeftStateBeforeHide,
        RightStateBeforeHide,
        HideState;


    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);


    void SetScreenPositions();

    void Update();

    virtual int GetTypeAndName(char *Type,char *Name);

    virtual char *GetTypeName(){return "[FilePanels]";};
    virtual void OnChangeFocus(int focus);

    void RedrawKeyBar(); // virtual
};

#endif // __FILEPANELS_HPP__
