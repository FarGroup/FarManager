#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class KeyBar:public ScreenObject
{
  private:
    void DisplayObject();
    BaseInput *Owner;
    char KeyName[12][10];
    char ShiftKeyName[12][10],AltKeyName[12][10],CtrlKeyName[12][10];
    int KeyCount,ShiftKeyCount,AltKeyCount,CtrlKeyCount;
    int AltState,CtrlState,ShiftState;
    int DisableMask;
  public:
    KeyBar();
    void SetOwner(BaseInput *Owner);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Set(char **Key,int KeyCount);
    void SetShift(char **Key,int KeyCount);
    void SetAlt(char **Key,int KeyCount);
    void SetCtrl(char **Key,int KeyCount);
    void SetDisableMask(int Mask);
    void Change(char *NewStr,int Pos);
    void RedrawIfChanged();
};


#endif	// __KEYBAR_HPP__