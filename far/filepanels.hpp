#ifndef __FILEPANELS_HPP__
#define __FILEPANELS_HPP__
/*
filepanels.hpp

файловые панели

*/

/* Revision: 1.14 03.06.2004 $ */

/*
Modify:
  03.06.2004 SVS
    ! Часть кода вынесена в отдельные функции:
       a) Установить фокус на противоположную панель (Tab)
          FilePanels::SetAnhoterPanelFocus()
       b) поменять панели местами (Ctrl-U)
          FilePanels::SwapPanels()
       c) сменить режим панели
          FilePanels::ChangePanelViewMode()
  08.04.2002 IS
    ! Немного const
  16.01.2002 OT
    Испраление поведения макросов в инфо-, квик- и три-панелей
  28.12.2001 DJ
    + единый метод GoToFile()
  02.11.2001 SVS
    ! возвращаемое значение у GetTypeName() - модификатор const
  18.07.2001 OT
    ! VFMenu
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  14.06.2001 OT
    ! "Бунт" ;-)
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
    typedef class Frame inherited;

  public:
    Panel *LastLeftFilePanel,
          *LastRightFilePanel;
    Panel *LeftPanel,
          *RightPanel,
          *ActivePanel;

    KeyBar      MainKeyBar;
    MenuBar     TopMenuBar;

    int LastLeftType,
        LastRightType;
    int LeftStateBeforeHide,
        RightStateBeforeHide;

  public:
    FilePanels();
    ~FilePanels();

  public:
    void Init();

    Panel* CreatePanel(int Type);
    void   DeletePanel(Panel *Deleted);
    Panel* GetAnotherPanel(Panel *Current);
    Panel* ChangePanelToFilled(Panel *Current,int NewType);
    Panel* ChangePanel(Panel *Current,int NewType,int CreateNew,int Force);
    void   SetPanelPositions(int LeftFullScreen,int RightFullScreen);
//    void   SetPanelPositions();

    void   SetupKeyBar();

    virtual void Show();

    void Redraw();

    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    int SetAnhoterPanelFocus(void);
    int SwapPanels(void);
    int ChangePanelViewMode(Panel *Current,int Mode,BOOL RefreshFrame);

    void SetScreenPosition();

    void Update();

    virtual int GetTypeAndName(char *Type,char *Name);
    virtual int GetType() { return MODALTYPE_PANELS; }
    virtual const char *GetTypeName(){return "[FilePanels]";};

    virtual void OnChangeFocus(int focus);

    void RedrawKeyBar(); // virtual
    virtual void ShowConsoleTitle();
    void ResizeConsole();
/* $ Введена для нужд CtrlAltShift OT */
    int FastHide();
    void Refresh();
    /* $ 28.12.2001 DJ
       единый метод для обработки Ctrl-F10 из вьюера и редактора
    */
    void GoToFile (const char *FileName);
    /* DJ $ */

    /* $ 16.01.2002 OT
       переопределенный виртуальный метод от Frame
    */
  int GetMacroMode();
    /* OT $ */
};

#endif // __FILEPANELS_HPP__
