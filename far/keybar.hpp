#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

/* Revision: 1.07 14.06.2001 $ */

/*
Modify:
  14.06.2001 OT
    ! "Бунт" ;-)
  06.05.2001 DJ
    ! перетрях #include
  30.04.2001 DJ
    ! Все нафиг переделано :-) Убран весь дублирующийся код. Публичный API
      сохранен.
  28.04.2001 VVM
    + ProcessKey() функция.
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

#include "scrobj.hpp"

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
  KBL_CTRLALT,

  /* $ 30.04.2001 DJ */
  KBL_GROUP_COUNT
  /* DJ $ */
};
/* SVS $ */

const int KEY_COUNT = 12;

/* $ 30.04.2001 DJ
   добавлен typedef; структура класса переделана, дабы избавиться от
   дублирования кода и данных
*/

typedef char KeyBarTitle [10];
typedef KeyBarTitle KeyBarTitleGroup [KEY_COUNT];

class KeyBar: public ScreenObject
{
  private:
    ScreenObject *Owner;

    KeyBarTitleGroup KeyTitles [KBL_GROUP_COUNT];
    int KeyCounts [KBL_GROUP_COUNT];

    int AltState,CtrlState,ShiftState;
    int DisableMask;

  private:
    void DisplayObject();

  public:
    KeyBar();
    void SetOwner(ScreenObject *Owner);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    void SetGroup(int Group,char **Key,int KeyCount);
    void ClearGroup(int Group);

    void Set(char **Key,int KeyCount)            { SetGroup (KBL_MAIN, Key, KeyCount); }
    void SetShift(char **Key,int KeyCount)       { SetGroup (KBL_SHIFT, Key, KeyCount); }
    void SetAlt(char **Key,int KeyCount)         { SetGroup (KBL_ALT, Key, KeyCount); }
    void SetCtrl(char **Key,int KeyCount)        { SetGroup (KBL_CTRL, Key, KeyCount); }
    /* $ 02.08.2000 SVS
       Дополнительные индикаторы
    */
    void SetCtrlShift(char **Key,int KeyCount)   { SetGroup (KBL_CTRLSHIFT, Key, KeyCount); }
    void SetAltShift(char **Key,int KeyCount)    { SetGroup (KBL_ALTSHIFT, Key, KeyCount); }
    void SetCtrlAlt(char **Key,int KeyCount)     { SetGroup (KBL_CTRLALT, Key, KeyCount); }
    /* SVS $*/

    void SetDisableMask(int Mask);
    void Change(char *NewStr,int Pos)            { Change (KBL_MAIN, NewStr, Pos); }

    /* $ 07.08.2000 SVS
       Изменение любого Label
    */
    void Change(int Group,char *NewStr,int Pos);
    /* SVS $ */

    /* $ 30.04.2001 DJ
       Групповая установка идущих подряд строк LNG для указанной группы
    */
    void SetAllGroup (int Group, int StartIndex, int Count);
    /* DJ $ */

    void RedrawIfChanged();
    void ResizeConsole();
};

/* DJ $ */

#endif	// __KEYBAR_HPP__
