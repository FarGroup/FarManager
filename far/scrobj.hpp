#ifndef __SCREENOBJECT_HPP__
#define __SCREENOBJECT_HPP__
/*
scrobj.hpp

Parent class для всех screen objects

*/

/* Revision: 1.02 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  15.07.2000 tran
    + add new dirty method - Hide0(), jys set Visible to False
      used in FileViewer, for keybar hiding
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class SaveScreen;

class ScreenObject
{
  private:
    virtual void DisplayObject() {};
    SaveScreen *ShadowSaveScr;
    int Visible;
    int Type;
    int EnableRestoreScreen;
    int SetPositionDone;
  protected:
    int X1,Y1,X2,Y2;
    int ObjWidth,ObjHeight;
  public:
    SaveScreen *SaveScr;
    ScreenObject();
    virtual ~ScreenObject();
    virtual int ProcessKey(int Key) { return(0); };
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) { return(0); };
    virtual void Hide();
    /* $ 15.07.2000 tran
       dirty hack :( */
    virtual void Hide0();
    /* tran 15.07.2000 $ */
    virtual void Show();
    virtual void ShowConsoleTitle() {};
    void SavePrevScreen();
    void Redraw();
    void SetPosition(int X1,int Y1,int X2,int Y2);
    void GetPosition(int& X1,int& Y1,int& X2,int& Y2);
    int IsVisible() { return(Visible); };
    void SetVisible(int Visible) {ScreenObject::Visible=Visible;};
    void SetRestoreScreenMode(int Mode) {EnableRestoreScreen=Mode;};
    void Shadow();
};

#endif  // __SCREENOBJECT_HPP__
