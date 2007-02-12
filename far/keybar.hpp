#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

#include "scrobj.hpp"

enum {
  KBL_MAIN=0,
  KBL_SHIFT,
  KBL_CTRL,
  KBL_ALT,
  KBL_CTRLSHIFT,
  KBL_ALTSHIFT,
  KBL_CTRLALT,

  KBL_GROUP_COUNT
};

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
    virtual void DisplayObject();

  public:
    KeyBar();
    virtual  ~KeyBar() {};
    void SetOwner(ScreenObject *Owner);
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    void SetGroup(int Group,const char **Key,int KeyCount);
    void ClearGroup(int Group);

    void Set(const char **Key,int KeyCount)            { SetGroup (KBL_MAIN, Key, KeyCount); }
    void SetShift(const char **Key,int KeyCount)       { SetGroup (KBL_SHIFT, Key, KeyCount); }
    void SetAlt(const char **Key,int KeyCount)         { SetGroup (KBL_ALT, Key, KeyCount); }
    void SetCtrl(const char **Key,int KeyCount)        { SetGroup (KBL_CTRL, Key, KeyCount); }
    void SetCtrlShift(const char **Key,int KeyCount)   { SetGroup (KBL_CTRLSHIFT, Key, KeyCount); }
    void SetAltShift(const char **Key,int KeyCount)    { SetGroup (KBL_ALTSHIFT, Key, KeyCount); }
    void SetCtrlAlt(const char **Key,int KeyCount)     { SetGroup (KBL_CTRLALT, Key, KeyCount); }

    void SetDisableMask(int Mask);
    void Change(const char *NewStr,int Pos)            { Change (KBL_MAIN, NewStr, Pos); }

    /* $ 07.08.2000 SVS
       Изменение любого Label
    */
    void Change(int Group,const char *NewStr,int Pos);
    /* SVS $ */

    /* $ 30.04.2001 DJ
       Групповая установка идущих подряд строк LNG для указанной группы
    */
    void SetAllGroup (int Group, int StartIndex, int Count);
    /* DJ $ */

    void RedrawIfChanged();
    virtual void ResizeConsole();
};

/* DJ $ */

#endif  // __KEYBAR_HPP__
