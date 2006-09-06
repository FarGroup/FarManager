#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

/* Revision: 1.08 11.12.2005 $ */

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

typedef wchar_t KeyBarTitle [10];
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

    void SetGroup(int Group,wchar_t **Key,int KeyCount);
    void ClearGroup(int Group);

    void Set(wchar_t **Key,int KeyCount)            { SetGroup (KBL_MAIN, Key, KeyCount); }
    void SetShift(wchar_t **Key,int KeyCount)       { SetGroup (KBL_SHIFT, Key, KeyCount); }
    void SetAlt(wchar_t **Key,int KeyCount)         { SetGroup (KBL_ALT, Key, KeyCount); }
    void SetCtrl(wchar_t **Key,int KeyCount)        { SetGroup (KBL_CTRL, Key, KeyCount); }
    /* $ 02.08.2000 SVS
       Дополнительные индикаторы
    */
    void SetCtrlShift(wchar_t **Key,int KeyCount)   { SetGroup (KBL_CTRLSHIFT, Key, KeyCount); }
    void SetAltShift(wchar_t **Key,int KeyCount)    { SetGroup (KBL_ALTSHIFT, Key, KeyCount); }
    void SetCtrlAlt(wchar_t **Key,int KeyCount)     { SetGroup (KBL_CTRLALT, Key, KeyCount); }
    /* SVS $*/

    void SetDisableMask(int Mask);
    void Change(wchar_t *NewStr,int Pos)            { Change (KBL_MAIN, NewStr, Pos); }

    /* $ 07.08.2000 SVS
       Изменение любого Label
    */
    void Change(int Group,wchar_t *NewStr,int Pos);
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

#endif  // __KEYBAR_HPP__
