#ifndef __SCREENOBJECT_HPP__
#define __SCREENOBJECT_HPP__
/*
scrobj.hpp

Parent class для всех screen objects

*/

/* Revision: 1.07 19.11.2001 $ */

/*
Modify:
  19.11.2001 OT
    Исправление поведения режима фуллскриновых панелей. 115 и 116 баги
  11.07.2001 OT
    Перенос ShadowSaveScr из приватной области в protected
   23.06.2001
    ! Убран член под названиес Type, который нигде не используется...
   14.06.2001
    + Новый метод SetScreenPosition() - без аргументов. Будет использоваться объектами,
      которым требуется выставить свои размеры, не прямям, а косвенным образом,
      зависяшим от состояния других объектов.
  21.05.2001 OT
    + Реакция на изменение размера консоли
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
    int EnableRestoreScreen;
  protected:
    int Visible;
    SaveScreen *ShadowSaveScr;
    int X1,Y1,X2,Y2;
    int ObjWidth,ObjHeight;
    int SetPositionDone;
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
    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
    virtual void SetScreenPosition();
    virtual void ResizeConsole(){};
    void GetPosition(int& X1,int& Y1,int& X2,int& Y2);
    int IsVisible() { return(Visible); };
    void SetVisible(int Visible) {ScreenObject::Visible=Visible;};
    void SetRestoreScreenMode(int Mode) {EnableRestoreScreen=Mode;};
    void Shadow();
};

#endif  // __SCREENOBJECT_HPP__
