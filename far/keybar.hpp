#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

#include "scrobj.hpp"
#include "unicodestring.hpp"

//   Группы меток
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

typedef wchar_t KeyBarTitle [16];
typedef KeyBarTitle KeyBarTitleGroup [KEY_COUNT];

class KeyBar: public ScreenObject
{
  private:
    ScreenObject *Owner;

    KeyBarTitleGroup KeyTitles [KBL_GROUP_COUNT];
    int KeyCounts [KBL_GROUP_COUNT];

    int AltState,CtrlState,ShiftState;
    int DisableMask;

    KeyBarTitleGroup RegKeyTitles [KBL_GROUP_COUNT];
    bool RegReaded;

    string strLanguage;
    string strRegGroupName;

  private:
    virtual void DisplayObject();

  public:
    KeyBar();
    virtual  ~KeyBar() {}

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    void SetOwner(ScreenObject *Owner);

    void ReadRegGroup(const wchar_t *RegGroup, string &strLanguage);
    void SetRegGroup(int Group);
    void SetAllRegGroup(void);

    void SetGroup(int Group,const wchar_t * const *Key,int KeyCount);
    // Групповая установка идущих подряд строк LNG для указанной группы
    void SetAllGroup (int Group, int StartIndex, int Count);

    void ClearGroup(int Group);

    void Set(const wchar_t * const *Key,int KeyCount)            { SetGroup (KBL_MAIN, Key, KeyCount); }
    void SetShift(const wchar_t * const *Key,int KeyCount)       { SetGroup (KBL_SHIFT, Key, KeyCount); }
    void SetAlt(const wchar_t * const *Key,int KeyCount)         { SetGroup (KBL_ALT, Key, KeyCount); }
    void SetCtrl(const wchar_t * const *Key,int KeyCount)        { SetGroup (KBL_CTRL, Key, KeyCount); }
    void SetCtrlShift(const wchar_t * const *Key,int KeyCount)   { SetGroup (KBL_CTRLSHIFT, Key, KeyCount); }
    void SetAltShift(const wchar_t * const *Key,int KeyCount)    { SetGroup (KBL_ALTSHIFT, Key, KeyCount); }
    void SetCtrlAlt(const wchar_t **Key,int KeyCount)            { SetGroup (KBL_CTRLALT, Key, KeyCount); }

    void SetDisableMask(int Mask);
    void Change(const wchar_t *NewStr,int Pos)                   { Change (KBL_MAIN, NewStr, Pos); }

    // Изменение любого Label
    void Change(int Group,const wchar_t *NewStr,int Pos);

    void RedrawIfChanged();
    virtual void ResizeConsole();
};

#endif  // __KEYBAR_HPP__
