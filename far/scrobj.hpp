#ifndef __SCREENOBJECT_HPP__
#define __SCREENOBJECT_HPP__
/*
scrobj.hpp

Parent class для всех screen objects

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class ScreenObject:public BaseInput
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
    virtual void Hide();
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

#endif	// __SCREENOBJECT_HPP__
