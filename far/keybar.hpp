#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

/* Revision: 1.01 02.08.2000 $ */

/*
Modify:
  02.08.2000 SVS
    + Дополнительные индикаторы
      CtrlShiftName, AltShiftName, CtrlAltName
    + К этим индикаторам - функции
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class KeyBar:public ScreenObject
{
  private:
    BaseInput *Owner;
    char KeyName[12][10];
    char ShiftKeyName[12][10],AltKeyName[12][10],CtrlKeyName[12][10];
    /* $ 02.08.2000 SVS
       Дополнительные индикаторы
    */
    char CtrlShiftKeyName[12][10],AltShiftKeyName[12][10],CtrlAltKeyName[12][10];
    int CtrlShiftKeyCount,AltShiftKeyCount,CtrlAltKeyCount;
    int AltShiftState,CtrlAltState,CtrlShiftState;
    /* SVS $*/
    int KeyCount,ShiftKeyCount,AltKeyCount,CtrlKeyCount;
    int AltState,CtrlState,ShiftState;
    int DisableMask;

  private:
    void DisplayObject();

  public:
    KeyBar();
    void SetOwner(BaseInput *Owner);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Set(char **Key,int KeyCount);
    void SetShift(char **Key,int KeyCount);
    void SetAlt(char **Key,int KeyCount);
    void SetCtrl(char **Key,int KeyCount);
    /* $ 02.08.2000 SVS
       Дополнительные индикаторы
    */
    void SetCtrlShift(char **Key,int KeyCount);
    void SetAltShift(char **Key,int KeyCount);
    void SetCtrlAlt(char **Key,int KeyCount);
    /* SVS $*/
    void SetDisableMask(int Mask);
    void Change(char *NewStr,int Pos);
    void RedrawIfChanged();
};


#endif	// __KEYBAR_HPP__