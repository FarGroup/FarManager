#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

/* Revision: 1.03 04.04.2001 $ */

/*
Modify:
  04.04.2001 SVS
    - Избавляемся от "залипания" :-)
    ! убран "мусор" - ненужные новые переменные CtrlShiftState
  07.08.2000 SVS
    + Изменение любого Label - функция Change(Group,...)
  02.08.2000 SVS
    + Дополнительные индикаторы
      CtrlShiftName, AltShiftName, CtrlAltName
    + К этим индикаторам - функции
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

/* $ 07.08.2000 SVS
   Группы меток
*/
enum {
  KBL_MAIN=0,
  KBL_SHIFT,
  KBL_CTRL,
  KBL_ALT,
  KBL_CTRLSHIFT,
  KBL_ALTSHIFT,
  KBL_CTRLALT
};
/* SVS $ */

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

    /* $ 07.08.2000 SVS
       Изменение любого Label
    */
    void Change(int Group,char *NewStr,int Pos);
    /* SVS $ */

    void RedrawIfChanged();
};

#endif	// __KEYBAR_HPP__
