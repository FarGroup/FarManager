#ifndef __KEYBAR_HPP__
#define __KEYBAR_HPP__
/*
keybar.hpp

Keybar

*/

#include "farconst.hpp"
#include "scrobj.hpp"

enum
{
	KBL_MAIN=0,
	KBL_SHIFT,
	KBL_CTRL,
	KBL_ALT,
	KBL_CTRLSHIFT,
	KBL_ALTSHIFT,
	KBL_CTRLALT,
	KBL_CTRLALTSHIFT,

	KBL_GROUP_COUNT
};

const int KEY_COUNT = 12;

typedef char KeyBarTitle [16];
typedef KeyBarTitle KeyBarTitleGroup [KEY_COUNT];

class KeyBar: public ScreenObject
{
	private:
		ScreenObject *Owner;

		KeyBarTitleGroup KeyTitles [KBL_GROUP_COUNT];
		int KeyCounts [KBL_GROUP_COUNT];

		KeyBarTitleGroup RegKeyTitles [KBL_GROUP_COUNT];
		bool RegReaded;

		int AltState,CtrlState,ShiftState;
		int DisableMask;

		char Language[LANGUAGENAME_SIZE];
		char RegGroupName[32];

	private:
		virtual void DisplayObject();

	public:
		KeyBar();
		virtual  ~KeyBar() {};

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

		void SetOwner(ScreenObject *Owner);

		void ClearGroup(int Group);

		void ReadRegGroup(const char *RegGroup, const char *szLanguage);
		void SetRegGroup(int Group);
		void SetAllRegGroup(void);

		void SetGroup(int Group,const char **Key,int KeyCount);
		// Групповая установка идущих подряд строк LNG для указанной группы
		void SetAllGroup(int Group, int StartIndex, int Count);

		void Set(const char **Key,int KeyCount)             { SetGroup(KBL_MAIN, Key, KeyCount); }
		void SetShift(const char **Key,int KeyCount)        { SetGroup(KBL_SHIFT, Key, KeyCount); }
		void SetAlt(const char **Key,int KeyCount)          { SetGroup(KBL_ALT, Key, KeyCount); }
		void SetCtrl(const char **Key,int KeyCount)         { SetGroup(KBL_CTRL, Key, KeyCount); }
		void SetCtrlShift(const char **Key,int KeyCount)    { SetGroup(KBL_CTRLSHIFT, Key, KeyCount); }
		void SetAltShift(const char **Key,int KeyCount)     { SetGroup(KBL_ALTSHIFT, Key, KeyCount); }
		void SetCtrlAlt(const char **Key,int KeyCount)      { SetGroup(KBL_CTRLALT, Key, KeyCount); }
		void SetCtrlAltShift(const char **Key,int KeyCount) { SetGroup(KBL_CTRLALTSHIFT, Key, KeyCount); }

		void Change(const char *NewStr,int Pos)            { Change(KBL_MAIN, NewStr, Pos); }

		// Изменение любого Label
		void Change(int Group,const char *NewStr,int Pos);

		void SetDisableMask(int Mask);

		void RedrawIfChanged();
		virtual void ResizeConsole();
};

#endif  // __KEYBAR_HPP__
