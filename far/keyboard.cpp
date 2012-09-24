/*
keyboard.cpp

Функции, имеющие отношение к клавитуре
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "keyboard.hpp"
#include "keys.hpp"
#include "farqueue.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "grabber.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"
#include "imports.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "strmix.hpp"
#include "synchro.hpp"
#include "constitle.hpp"
#include "window.hpp"
#include "console.hpp"
#include "palette.hpp"

/* start Глобальные переменные */

// "дополнительная" очередь кодов клавиш
FarQueue<DWORD> *KeyQueue=nullptr;

FarKeyboardState IntKeyState={};

/* end Глобальные переменные */

static SHORT KeyToVKey[WCHAR_MAX];
static WCHAR VKeyToASCII[0x200];

static unsigned int AltValue=0;
static int KeyCodeForALT_LastPressed=0;

static MOUSE_EVENT_RECORD lastMOUSE_EVENT_RECORD;
static int ShiftPressedLast=FALSE,AltPressedLast=FALSE,CtrlPressedLast=FALSE;
static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?

static int RightShiftPressedLast=FALSE,RightAltPressedLast=FALSE,RightCtrlPressedLast=FALSE;
static BOOL IsKeyRCASPressed=FALSE; // Right CtrlAltShift - нажато или нет?

static clock_t PressedLastTime,KeyPressedLastTime;
static int ShiftState=0;
static int LastShiftEnterPressed=FALSE;

/* ----------------------------------------------------------------- */
static struct TTable_KeyToVK
{
	int Key;
	int VK;
} Table_KeyToVK[]=
{
//   {KEY_PGUP,          VK_PRIOR},
//   {KEY_PGDN,          VK_NEXT},
//   {KEY_END,           VK_END},
//   {KEY_HOME,          VK_HOME},
//   {KEY_LEFT,          VK_LEFT},
//   {KEY_UP,            VK_UP},
//   {KEY_RIGHT,         VK_RIGHT},
//   {KEY_DOWN,          VK_DOWN},
//   {KEY_INS,           VK_INSERT},
//   {KEY_DEL,           VK_DELETE},
//   {KEY_LWIN,          VK_LWIN},
//   {KEY_RWIN,          VK_RWIN},
//   {KEY_APPS,          VK_APPS},
//   {KEY_MULTIPLY,      VK_MULTIPLY},
//   {KEY_ADD,           VK_ADD},
//   {KEY_SUBTRACT,      VK_SUBTRACT},
//   {KEY_DIVIDE,        VK_DIVIDE},
//   {KEY_F1,            VK_F1},
//   {KEY_F2,            VK_F2},
//   {KEY_F3,            VK_F3},
//   {KEY_F4,            VK_F4},
//   {KEY_F5,            VK_F5},
//   {KEY_F6,            VK_F6},
//   {KEY_F7,            VK_F7},
//   {KEY_F8,            VK_F8},
//   {KEY_F9,            VK_F9},
//   {KEY_F10,           VK_F10},
//   {KEY_F11,           VK_F11},
//   {KEY_F12,           VK_F12},
	{KEY_BREAK,         VK_CANCEL},
	{KEY_BS,            VK_BACK},
	{KEY_TAB,           VK_TAB},
	{KEY_ENTER,         VK_RETURN},
	{KEY_NUMENTER,      VK_RETURN}, //????
	{KEY_ESC,           VK_ESCAPE},
	{KEY_SPACE,         VK_SPACE},
	{KEY_NUMPAD5,       VK_CLEAR},
};


struct TFKey3
{
	DWORD Key;
	int   Len;
	const wchar_t *Name;
	const wchar_t *UName;
};

static TFKey3 FKeys1[]=
{
	{ KEY_RCTRLALTSHIFTRELEASE,24, L"RightCtrlAltShiftRelease", L"RIGHTCTRLALTSHIFTRELEASE"},
	{ KEY_RCTRLALTSHIFTPRESS,  22, L"RightCtrlAltShiftPress", L"RIGHTCTRLALTSHIFTPRESS"},
	{ KEY_CTRLALTSHIFTRELEASE, 19, L"CtrlAltShiftRelease", L"CTRLALTSHIFTRELEASE"},
	{ KEY_CTRLALTSHIFTPRESS,   17, L"CtrlAltShiftPress", L"CTRLALTSHIFTPRESS"},
	{ KEY_LAUNCH_MEDIA_SELECT, 17, L"LaunchMediaSelect", L"LAUNCHMEDIASELECT"},
	{ KEY_BROWSER_FAVORITES,   16, L"BrowserFavorites", L"BROWSERFAVORITES"},
	{ KEY_MEDIA_PREV_TRACK,    14, L"MediaPrevTrack", L"MEDIAPREVTRACK"},
	{ KEY_MEDIA_PLAY_PAUSE,    14, L"MediaPlayPause", L"MEDIAPLAYPAUSE"},
	{ KEY_MEDIA_NEXT_TRACK,    14, L"MediaNextTrack", L"MEDIANEXTTRACK"},
	{ KEY_BROWSER_REFRESH,     14, L"BrowserRefresh", L"BROWSERREFRESH"},
	{ KEY_BROWSER_FORWARD,     14, L"BrowserForward", L"BROWSERFORWARD"},
	//{ KEY_HP_COMMUNITIES,      13, L"HPCommunities", L"HPCOMMUNITIES"},
	{ KEY_BROWSER_SEARCH,      13, L"BrowserSearch", L"BROWSERSEARCH"},
	{ KEY_MSWHEEL_RIGHT,       12, L"MsWheelRight", L"MSWHEELRIGHT"},
#if 0
	{ KEY_MSM1DBLCLICK,        12, L"MsM1DblClick", L"MSM1DBLCLICK"},
	{ KEY_MSM2DBLCLICK,        12, L"MsM2DblClick", L"MSM2DBLCLICK"},
	{ KEY_MSM3DBLCLICK,        12, L"MsM3DblClick", L"MSM3DBLCLICK"},
	{ KEY_MSLDBLCLICK,         11, L"MsLDblClick", L"MSLDBLCLICK"},
	{ KEY_MSRDBLCLICK,         11, L"MsRDblClick", L"MSRDBLCLICK"},
#endif
	{ KEY_MSWHEEL_DOWN,        11, L"MsWheelDown", L"MSWHEELDOWN"},
	{ KEY_MSWHEEL_LEFT,        11, L"MsWheelLeft", L"MSWHEELLEFT"},
	//{ KEY_AC_BOOKMARKS,        11, L"ACBookmarks", L"ACBOOKMARKS"},
	{ KEY_BROWSER_STOP,        11, L"BrowserStop", L"BROWSERSTOP"},
	{ KEY_BROWSER_HOME,        11, L"BrowserHome", L"BROWSERHOME"},
	{ KEY_BROWSER_BACK,        11, L"BrowserBack", L"BROWSERBACK"},
	{ KEY_VOLUME_MUTE,         10, L"VolumeMute", L"VOLUMEMUTE"},
	{ KEY_VOLUME_DOWN,         10, L"VolumeDown", L"VOLUMEDOWN"},
	{ KEY_SCROLLLOCK,          10, L"ScrollLock", L"SCROLLLOCK"},
	{ KEY_LAUNCH_MAIL,         10, L"LaunchMail", L"LAUNCHMAIL"},
	{ KEY_LAUNCH_APP2,         10, L"LaunchApp2", L"LAUNCHAPP2"},
	{ KEY_LAUNCH_APP1,         10, L"LaunchApp1", L"LAUNCHAPP1"},
	//{ KEY_HP_INTERNET,         10, L"HPInternet", L"HPINTERNET"},
	//{ KEY_AC_FORWARD,           9, L"ACForward", L"ACFORWARD"},
	//{ KEY_AC_REFRESH,           9, L"ACRefresh", L"ACREFRESH"},
	{ KEY_MSWHEEL_UP,           9, L"MsWheelUp", L"MSWHEELUP"},
	{ KEY_MEDIA_STOP,           9, L"MediaStop", L"MEDIASTOP"},
	{ KEY_BACKSLASH,            9, L"BackSlash", L"BACKSLASH"},
	//{ KEY_HP_MEETING,           9, L"HPMeeting", L"HPMEETING"},
	{ KEY_MSM1CLICK,            9, L"MsM1Click", L"MSM1CLICK"},
	{ KEY_MSM2CLICK,            9, L"MsM2Click", L"MSM2CLICK"},
	{ KEY_MSM3CLICK,            9, L"MsM3Click", L"MSM3CLICK"},
	{ KEY_MSLCLICK,             8, L"MsLClick", L"MSLCLICK"},
	{ KEY_MSRCLICK,             8, L"MsRClick", L"MSRCLICK"},
	//{ KEY_HP_MARKET,            8, L"HPMarket", L"HPMARKET"},
	{ KEY_VOLUME_UP,            8, L"VolumeUp", L"VOLUMEUP"},
	{ KEY_SUBTRACT,             8, L"Subtract", L"SUBTRACT"},
	{ KEY_NUMENTER,             8, L"NumEnter", L"NUMENTER"},
	{ KEY_MULTIPLY,             8, L"Multiply", L"MULTIPLY"},
	{ KEY_CAPSLOCK,             8, L"CapsLock", L"CAPSLOCK"},
	{ KEY_PRNTSCRN,             8, L"PrntScrn", L"PRNTSCRN"},
	{ KEY_NUMLOCK,              7, L"NumLock", L"NUMLOCK"},
	{ KEY_DECIMAL,              7, L"Decimal", L"DECIMAL"},
	{ KEY_STANDBY,              7, L"Standby", L"STANDBY"},
	//{ KEY_HP_SEARCH,            8, L"HPSearch", L"HPSEARCH"},
	//{ KEY_HP_HOME,              6, L"HPHome", L"HPHOME"},
	//{ KEY_HP_MAIL,              6, L"HPMail", L"HPMAIL"},
	//{ KEY_HP_NEWS,              6, L"HPNews", L"HPNEWS"},
	//{ KEY_AC_BACK,              6, L"ACBack", L"ACBACK"},
	//{ KEY_AC_STOP,              6, L"ACStop", L"ACSTOP"},
	{ KEY_DIVIDE,               6, L"Divide", L"DIVIDE"},
	{ KEY_NUMDEL,               6, L"NumDel", L"NUMDEL"},
	{ KEY_SPACE,                5, L"Space", L"SPACE"},
	{ KEY_RIGHT,                5, L"Right", L"RIGHT"},
	{ KEY_PAUSE,                5, L"Pause", L"PAUSE"},
	{ KEY_ENTER,                5, L"Enter", L"ENTER"},
	{ KEY_CLEAR,                5, L"Clear", L"CLEAR"},
	{ KEY_BREAK,                5, L"Break", L"BREAK"},
	{ KEY_PGUP,                 4, L"PgUp", L"PGUP"},
	{ KEY_PGDN,                 4, L"PgDn", L"PGDN"},
	{ KEY_LEFT,                 4, L"Left", L"LEFT"},
	{ KEY_HOME,                 4, L"Home", L"HOME"},
	{ KEY_DOWN,                 4, L"Down", L"DOWN"},
	{ KEY_APPS,                 4, L"Apps", L"APPS"},
	{ KEY_RWIN,                 4 ,L"RWin", L"RWIN"},
	{ KEY_NUMPAD9,              4 ,L"Num9", L"NUM9"},
	{ KEY_NUMPAD8,              4 ,L"Num8", L"NUM8"},
	{ KEY_NUMPAD7,              4 ,L"Num7", L"NUM7"},
	{ KEY_NUMPAD6,              4 ,L"Num6", L"NUM6"},
	{ KEY_NUMPAD5,              4, L"Num5", L"NUM5"},
	{ KEY_NUMPAD4,              4 ,L"Num4", L"NUM4"},
	{ KEY_NUMPAD3,              4 ,L"Num3", L"NUM3"},
	{ KEY_NUMPAD2,              4 ,L"Num2", L"NUM2"},
	{ KEY_NUMPAD1,              4 ,L"Num1", L"NUM1"},
	{ KEY_NUMPAD0,              4 ,L"Num0", L"NUM0"},
	{ KEY_LWIN,                 4 ,L"LWin", L"LWIN"},
	{ KEY_TAB,                  3, L"Tab", L"TAB"},
	{ KEY_INS,                  3, L"Ins", L"INS"},
	{ KEY_F10,                  3, L"F10", L"F10"},
	{ KEY_F11,                  3, L"F11", L"F11"},
	{ KEY_F12,                  3, L"F12", L"F12"},
	{ KEY_F13,                  3, L"F13", L"F13"},
	{ KEY_F14,                  3, L"F14", L"F14"},
	{ KEY_F15,                  3, L"F15", L"F15"},
	{ KEY_F16,                  3, L"F16", L"F16"},
	{ KEY_F17,                  3, L"F17", L"F17"},
	{ KEY_F18,                  3, L"F18", L"F18"},
	{ KEY_F19,                  3, L"F19", L"F19"},
	{ KEY_F20,                  3, L"F20", L"F20"},
	{ KEY_F21,                  3, L"F21", L"F21"},
	{ KEY_F22,                  3, L"F22", L"F22"},
	{ KEY_F23,                  3, L"F23", L"F23"},
	{ KEY_F24,                  3, L"F24", L"F24"},
	{ KEY_ESC,                  3, L"Esc", L"ESC"},
	{ KEY_END,                  3, L"End", L"END"},
	{ KEY_DEL,                  3, L"Del", L"DEL"},
	{ KEY_ADD,                  3, L"Add", L"ADD"},
	{ KEY_UP,                   2, L"Up", L"UP"},
	{ KEY_F9,                   2, L"F9", L"F9"},
	{ KEY_F8,                   2, L"F8", L"F8"},
	{ KEY_F7,                   2, L"F7", L"F7"},
	{ KEY_F6,                   2, L"F6", L"F6"},
	{ KEY_F5,                   2, L"F5", L"F5"},
	{ KEY_F4,                   2, L"F4", L"F4"},
	{ KEY_F3,                   2, L"F3", L"F3"},
	{ KEY_F2,                   2, L"F2", L"F2"},
	{ KEY_F1,                   2, L"F1", L"F1"},
	{ KEY_BS,                   2, L"BS", L"BS"},
	{ KEY_BACKBRACKET,          1, L"]",  L"]"},
	{ KEY_QUOTE,                1, L"\"",  L"\""},
	{ KEY_BRACKET,              1, L"[",  L"["},
	{ KEY_COLON,                1, L":",  L":"},
	{ KEY_SEMICOLON,            1, L";",  L";"},
	{ KEY_SLASH,                1, L"/",  L"/"},
	{ KEY_DOT,                  1, L".",  L"."},
	{ KEY_COMMA,                1, L",",  L","},
};

static TFKey3 ModifKeyName[]=
{
	{ KEY_RCTRL  ,5 ,L"RCtrl", L"RCTRL"},
	{ KEY_SHIFT  ,5 ,L"Shift", L"SHIFT"},
	{ KEY_CTRL   ,4 ,L"Ctrl", L"CTRL"},
	{ KEY_RALT   ,4 ,L"RAlt", L"RALT"},
	{ KEY_ALT    ,3 ,L"Alt", L"ALT"},
	{ KEY_M_SPEC ,4 ,L"Spec", L"SPEC"},
	{ KEY_M_OEM  ,3 ,L"Oem", L"OEM"},
//  { KEY_LCTRL  ,5 ,L"LCtrl", L"LCTRL"},
//  { KEY_LALT   ,4 ,L"LAlt", L"LALT"},
//  { KEY_LSHIFT ,6 ,L"LShift", L"LSHIFT"},
//  { KEY_RSHIFT ,6 ,L"RShift", L"RSHIFT"},
};

#if defined(SYSLOG)
static TFKey3 SpecKeyName[]=
{
	{ KEY_CONSOLE_BUFFER_RESIZE,19, L"ConsoleBufferResize", L"CONSOLEBUFFERRESIZE"},
	{ KEY_OP_SELWORD           ,10, L"OP_SelWord", L"OP_SELWORD"},
	{ KEY_KILLFOCUS             ,9, L"KillFocus", L"KILLFOCUS"},
	{ KEY_GOTFOCUS              ,8, L"GotFocus", L"GOTFOCUS"},
	{ KEY_DRAGCOPY             , 8, L"DragCopy", L"DRAGCOPY"},
	{ KEY_DRAGMOVE             , 8, L"DragMove", L"DRAGMOVE"},
	{ KEY_OP_PLAINTEXT         , 7, L"OP_Text", L"OP_TEXT"},
	{ KEY_OP_XLAT              , 7, L"OP_Xlat", L"OP_XLAT"},
	{ KEY_NONE                 , 4, L"None", L"NONE"},
	{ KEY_IDLE                 , 4, L"Idle", L"IDLE"},
};
#endif

/* ----------------------------------------------------------------- */

static HKL Layout[10]={0};
static int LayoutNumber=0;

/*
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
	LayoutNumber=GetKeyboardLayoutList(ARRAYSIZE(Layout),Layout); // возвращает 0! в telnet

	if (!LayoutNumber)
	{
		HKEY hk=nullptr;

		if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Keyboard Layout\\Preload", 0, KEY_READ, &hk)==ERROR_SUCCESS)
		{
			DWORD dwType, dwIndex, dwDataSize, dwValueSize, dwKeyb;
			wchar_t SData[16], SValue[16];

			for (dwIndex=0; dwIndex < (int)ARRAYSIZE(Layout); dwIndex++)
			{
				dwValueSize=16;
				dwDataSize=16*sizeof(wchar_t);

				if (ERROR_SUCCESS==RegEnumValue(hk, dwIndex, SValue, &dwValueSize, nullptr, &dwType,(LPBYTE)SData, &dwDataSize))
				{
					if (dwType == REG_SZ && isdigit(SValue[0]) &&
					        (isdigit(SData[0]) || (SData[0] >= L'a' && SData[0] <= L'f') || (SData[0] >= L'A' && SData[0] <= L'F')))
					{
						wchar_t *endptr=nullptr;
						dwKeyb=wcstoul(SData, &endptr, 16); // SData=="00000419"

						if (dwKeyb)
						{
							if (dwKeyb <= 0xFFFF)
								dwKeyb |= (dwKeyb << 16);

							Layout[LayoutNumber++] = (HKL)((intptr_t)dwKeyb);
						}
					}
				}
				else
					break;
			}

			RegCloseKey(hk);
		}
	}

	ClearArray(KeyToVKey);
	ClearArray(VKeyToASCII);

	if (LayoutNumber && LayoutNumber < (int)ARRAYSIZE(Layout))
	{
		BYTE KeyState[0x100]={};
		WCHAR buf[1];

		//KeyToVKey - используется чтоб проверить если два символа это одна и таже кнопка на клаве
		//*********
		//Так как сделать полноценное мапирование между всеми раскладками не реально,
		//по причине того что во время проигрывания макросов нет такого понятия раскладка
		//то сделаем наилучшую попытку - смысл такой, делаем полное мапирование всех возможных
		//VKs и ShiftVKs в юникодные символы проходясь по всем раскладкам с одним но:
		//если разные VK мапятся в тот же юникод символ то мапирование будет только для первой
		//раскладки которая вернула этот символ
		//
		for (BYTE j=0; j<2; j++)
		{
			KeyState[VK_SHIFT]=j*0x80;

			for (int i=0; i<LayoutNumber; i++)
			{
				for (int VK=0; VK<256; VK++)
				{
					if (ToUnicodeEx(LOBYTE(VK),0,KeyState,buf,1,0,Layout[i]) > 0)
					{
						if (!KeyToVKey[buf[0]])
							KeyToVKey[buf[0]] = VK + j*0x100;
					}
				}
			}
		}

		//VKeyToASCII - используется вместе с KeyToVKey чтоб подменить нац. символ на US-ASCII
		//***********
		//Имея мапирование юникод -> VK строим обратное мапирование
		//VK -> символы с кодом меньше 0x80, т.е. только US-ASCII символы
		for (WCHAR i=1, x=0; i < 0x80; i++)
		{
			x = KeyToVKey[i];

			if (x && !VKeyToASCII[x])
				VKeyToASCII[x]=Upper(i);
		}
	}
}

//Сравнивает если Key и CompareKey это одна и та же клавиша в разных раскладках
bool KeyToKeyLayoutCompare(int Key, int CompareKey)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyToKeyLayoutCompare()"));
	_KEYMACRO(SysLog(L"Param: Key=%08X",Key));
	Key = KeyToVKey[Key&0xFFFF]&0xFF;
	CompareKey = KeyToVKey[CompareKey&0xFFFF]&0xFF;

	if (Key  && Key == CompareKey)
		return true;

	return false;
}

//Должно вернуть клавишный Eng эквивалент Key
int KeyToKeyLayout(int Key)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyToKeyLayout()"));
	_KEYMACRO(SysLog(L"Param: Key=%08X",Key));
	int VK = KeyToVKey[Key&0xFFFF];

	if (VK && VKeyToASCII[VK])
		return VKeyToASCII[VK];

	return Key;
}

/*
  State:
    -1 get state, 0 off, 1 on, 2 flip
*/
int SetFLockState(UINT vkKey, int State)
{
	UINT ExKey=(vkKey==VK_CAPITAL?0:KEYEVENTF_EXTENDEDKEY);

	switch (vkKey)
	{
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_SCROLL:
			break;
		default:
			return -1;
	}

	short oldState=GetKeyState(vkKey);

	if (State >= 0)
	{
		//if (State == 2 || (State==1 && !(keyState[vkKey] & 1)) || (!State && (keyState[vkKey] & 1)) )
		if (State == 2 || (State==1 && !oldState) || (!State && oldState))
		{
			keybd_event(vkKey, 0, ExKey, 0);
			keybd_event(vkKey, 0, ExKey | KEYEVENTF_KEYUP, 0);
		}
	}

	return (int)(WORD)oldState;
}

int InputRecordToKey(const INPUT_RECORD *r)
{
	if (r)
	{
		INPUT_RECORD Rec=*r; // НАДО!, т.к. внутри CalcKeyCode
		//   структура INPUT_RECORD модифицируется!

		return (int)ShieldCalcKeyCode(&Rec,FALSE,nullptr,true);
	}

	return KEY_NONE;
}


int KeyToInputRecord(int Key, INPUT_RECORD *Rec)
{
  int VirtKey, ControlState;
  return TranslateKeyToVK(Key, VirtKey, ControlState, Rec);
}

//BUGBUG - временная затычка
void ProcessKeyToInputRecord(int Key, unsigned int dwControlState, INPUT_RECORD *Rec)
{
	if (Rec)
	{
		Rec->EventType=KEY_EVENT;
		Rec->Event.KeyEvent.bKeyDown=1;
		Rec->Event.KeyEvent.wRepeatCount=1;
		Rec->Event.KeyEvent.wVirtualKeyCode=Key;
		Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_VSC);

		//BUGBUG
		Rec->Event.KeyEvent.uChar.UnicodeChar=MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_CHAR);

		//BUGBUG
 		Rec->Event.KeyEvent.dwControlKeyState=
 			(dwControlState&PKF_SHIFT?SHIFT_PRESSED:0)|
 			(dwControlState&PKF_ALT?LEFT_ALT_PRESSED:0)|
 			(dwControlState&PKF_RALT?RIGHT_ALT_PRESSED:0)|
 			(dwControlState&PKF_RCONTROL?RIGHT_CTRL_PRESSED:0)|
 			(dwControlState&PKF_CONTROL?LEFT_CTRL_PRESSED:0);
	}
}

void FarKeyToInputRecord(const FarKey& Key,INPUT_RECORD* Rec)
{
	if (Rec)
	{
		Rec->EventType=KEY_EVENT;
		Rec->Event.KeyEvent.bKeyDown=1;
		Rec->Event.KeyEvent.wRepeatCount=1;
		Rec->Event.KeyEvent.wVirtualKeyCode=Key.VirtualKeyCode;
		Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_VSC);

		//BUGBUG
		Rec->Event.KeyEvent.uChar.UnicodeChar=MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_CHAR);

 		Rec->Event.KeyEvent.dwControlKeyState=Key.ControlKeyState;
	}
}

DWORD IsMouseButtonPressed()
{
	INPUT_RECORD rec;

	if (PeekInputRecord(&rec))
	{
		GetInputRecord(&rec);
	}

	Sleep(1);
	return IntKeyState.MouseButtonState;
}

static DWORD KeyMsClick2ButtonState(DWORD Key,DWORD& Event)
{
	Event=0;
#if 0

	switch (Key)
	{
		case KEY_MSM1DBLCLICK:
		case KEY_MSM2DBLCLICK:
		case KEY_MSM3DBLCLICK:
		case KEY_MSLDBLCLICK:
		case KEY_MSRDBLCLICK:
			Event=MOUSE_MOVED;
	}

#endif

	switch (Key)
	{
		case KEY_MSLCLICK:
			return FROM_LEFT_1ST_BUTTON_PRESSED;
		case KEY_MSM1CLICK:
			return FROM_LEFT_2ND_BUTTON_PRESSED;
		case KEY_MSM2CLICK:
			return FROM_LEFT_3RD_BUTTON_PRESSED;
		case KEY_MSM3CLICK:
			return FROM_LEFT_4TH_BUTTON_PRESSED;
		case KEY_MSRCLICK:
			return RIGHTMOST_BUTTON_PRESSED;
	}

	return 0;
}

void ReloadEnvironment()
{
	struct addr
	{
		HKEY Key;
		string SubKey;
	}
	Addr[]=
	{
		{HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"},
		{HKEY_CURRENT_USER, L"Environment"},
		{HKEY_CURRENT_USER, L"Volatile Environment"}
	};

	string strName, strData;
	for(size_t i=0; i<ARRAYSIZE(Addr); i++)
	{
		DWORD Types[]={REG_SZ,REG_EXPAND_SZ}; // REG_SZ first
		for(size_t t=0; t<ARRAYSIZE(Types); t++) // two passes
		{
			DWORD Type;
			for(int j=0; EnumRegValueEx(Addr[i].Key, Addr[i].SubKey, j, strName, strData, nullptr, nullptr, &Type); j++)
			{
				if(Type==Types[t])
				{
					if(Type==REG_EXPAND_SZ)
					{
						apiExpandEnvironmentStrings(strData, strData);
					}
					if(Addr[i].Key==HKEY_CURRENT_USER)
					{
						// see http://support.microsoft.com/kb/100843 for details
						if(!StrCmpI(strName, L"path") || !StrCmpI(strName, L"libpath") || !StrCmpI(strName, L"os2libpath"))
						{
							string strMergedPath;
							apiGetEnvironmentVariable(strName, strMergedPath);
							if(strMergedPath.At(strMergedPath.GetLength()-1)!=L';')
							{
								strMergedPath+=L';';
							}
							strData=strMergedPath+strData;
						}
					}
					SetEnvironmentVariable(strName, strData);
				}
			}
		}
	}
}

static bool was_repeat = false;
static WORD last_pressed_keycode = (WORD)-1;

bool IsRepeatedKey()
{
	return was_repeat;
}

static DWORD __GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro,bool ProcessMouse,bool AllowSynchro);

DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro,bool ProcessMouse,bool AllowSynchro)
{
	DWORD Key = __GetInputRecord(rec,ExcludeMacro,ProcessMouse,AllowSynchro);

	if (Key)
	{
		if (CtrlObject)
		{
			ProcessConsoleInputInfo Info={sizeof(Info),PCIF_NONE,*rec};
			//Info.hPanel
			if (WaitInMainLoop)
				Info.Flags|=PCIF_FROMMAIN;
			switch (CtrlObject->Plugins->ProcessConsoleInput(&Info))
			{
				case 1:
					Key=KEY_NONE;
					KeyToInputRecord(Key, rec);
					break;
				case 2:
					*rec=Info.Rec;
					Key=CalcKeyCode(rec,FALSE);
					break;
			}
		}
	}
	return Key;
}

static DWORD __GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro,bool ProcessMouse,bool AllowSynchro)
{
	_KEYMACRO(CleverSysLog Clev(L"GetInputRecord()"));
	static int LastEventIdle=FALSE;
	size_t ReadCount;
	DWORD LoopCount=0,CalcKey;
	DWORD ReadKey=0;
	int NotMacros=FALSE;
	static int LastMsClickMacroKey=0;
	struct FAR_INPUT_RECORD irec={};

	if (AllowSynchro)
		PluginSynchroManager.Process();

	if (!ExcludeMacro && CtrlObject && CtrlObject->Cp())
	{
		//_KEYMACRO(CleverSysLog SL(L"GetInputRecord()"));
		CtrlObject->Macro.RunStartMacro();
		int MacroKey=CtrlObject->Macro.GetKey();

		if (MacroKey)
		{
			DWORD EventState,MsClickKey;

			if ((MsClickKey=KeyMsClick2ButtonState(MacroKey,EventState)) )
			{
				// Ахтунг! Для мышиной клавиши вернем значение MOUSE_EVENT, соответствующее _последнему_ событию мыши.
				rec->EventType=MOUSE_EVENT;
				rec->Event.MouseEvent=lastMOUSE_EVENT_RECORD;
				rec->Event.MouseEvent.dwButtonState=MsClickKey;
				rec->Event.MouseEvent.dwEventFlags=EventState;
				LastMsClickMacroKey=MacroKey;
				return MacroKey;
			}
			else
			{
				// если предыдущая клавиша мышиная - сбросим состояние панели Drag
				if (KeyMsClick2ButtonState(LastMsClickMacroKey,EventState))
				{
					LastMsClickMacroKey=0;
					Panel::EndDrag();
				}

				ScrBuf.Flush();
				int VirtKey,ControlState;
				TranslateKeyToVK(MacroKey,VirtKey,ControlState,rec);
				rec->EventType=((((unsigned int)MacroKey >= KEY_MACRO_BASE && (unsigned int)MacroKey <= KEY_MACRO_ENDBASE) || ((unsigned int)MacroKey>=KEY_OP_BASE && (unsigned int)MacroKey <=KEY_OP_ENDBASE)) || (MacroKey&(~0xFF000000)) >= KEY_END_FKEY)?0:FARMACRO_KEY_EVENT;

				if (!(MacroKey&KEY_SHIFT))
					IntKeyState.ShiftPressed=0;

				//_KEYMACRO(SysLog(L"MacroKey1 =%s",_FARKEY_ToName(MacroKey)));
				// ClearStruct(*rec);
				return(MacroKey);
			}
		}
	}

	if (KeyQueue && KeyQueue->Peek())
	{
		CalcKey=KeyQueue->Get();
		NotMacros=CalcKey&0x80000000?1:0;
		CalcKey&=~0x80000000;

		//???
		if (!ExcludeMacro && CtrlObject && CtrlObject->Macro.IsRecording() &&
			(CalcKey == (KEY_ALT|KEY_NUMPAD0) || CalcKey == (KEY_RALT|KEY_NUMPAD0) || CalcKey == (KEY_ALT|KEY_INS) || CalcKey == (KEY_RALT|KEY_INS)))
		{
			_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(CalcKey)));
			FrameManager->SetLastInputRecord(rec);
			irec.IntKey=CalcKey;
			irec.Rec=*rec;
			if (CtrlObject->Macro.ProcessEvent(&irec))
			{
				RunGraber();
				rec->EventType=0;
				CalcKey=KEY_NONE;
			}

			return(CalcKey);
		}

		if (!NotMacros)
		{
			_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(CalcKey)));
			FrameManager->SetLastInputRecord(rec);
			irec.IntKey=CalcKey;
			irec.Rec=*rec;
			if (!ExcludeMacro && CtrlObject && CtrlObject->Macro.ProcessEvent(&irec))
			{
				rec->EventType=0;
				CalcKey=KEY_NONE;
			}
		}

		return(CalcKey);
	}

	int EnableShowTime=Opt.Clock && (WaitInMainLoop || (CtrlObject &&
	                                 CtrlObject->Macro.GetMode()==MACRO_SEARCH));

	if (EnableShowTime)
		ShowTime(1);

	ScrBuf.Flush();

	if (!LastEventIdle)
		StartIdleTime=clock();

	LastEventIdle=FALSE;

	BOOL ZoomedState=IsZoomed(Console.GetWindow());
	BOOL IconicState=IsIconic(Console.GetWindow());

	bool FullscreenState=IsConsoleFullscreen();

	for (;;)
	{
		// "Реакция" на максимизацию/восстановление окна консоли
		if (ZoomedState!=IsZoomed(Console.GetWindow()) && IconicState==IsIconic(Console.GetWindow()))
		{
			ZoomedState=!ZoomedState;
			ChangeVideoMode(ZoomedState);
		}

		if (!(LoopCount & 15))
		{
			if(CtrlObject && CtrlObject->Plugins->GetPluginsCount())
			{
				SetFarConsoleMode();
			}

			bool CurrentFullscreenState=IsConsoleFullscreen();
			if(CurrentFullscreenState && !FullscreenState)
			{
				ChangeVideoMode(25,80);
			}
			FullscreenState=CurrentFullscreenState;

			Window.Check();

			if(Events.EnvironmentChangeEvent.Signaled())
			{
				ReloadEnvironment();
			}
		}

		Console.PeekInput(rec, 1, ReadCount);

		/* $ 26.04.2001 VVM
		   ! Убрал подмену колесика */
		if (ReadCount)
		{
			//check for flock
			if (rec->EventType==KEY_EVENT && !rec->Event.KeyEvent.wVirtualScanCode && (rec->Event.KeyEvent.wVirtualKeyCode==VK_NUMLOCK||rec->Event.KeyEvent.wVirtualKeyCode==VK_CAPITAL||rec->Event.KeyEvent.wVirtualKeyCode==VK_SCROLL))
			{
				INPUT_RECORD pinp;
				size_t nread;
				Console.ReadInput(&pinp, 1, nread);
				was_repeat = false;
				last_pressed_keycode = (WORD)-1;
				continue;
			}
			break;
		}

		ScrBuf.Flush();
		Sleep(10);

		static bool ExitInProcess = false;
		if (CloseFAR && !ExitInProcess)
		{
			ExitInProcess = true;
			FrameManager->ExitMainLoop(FALSE);
			return KEY_NONE;
		}

		if (!(LoopCount & 15))
		{
			clock_t CurTime=clock();

			if (EnableShowTime)
				ShowTime(0);

			if (WaitInMainLoop)
			{
				if (!(LoopCount & 63))
				{
					static int UpdateReenter=0;

					if (!UpdateReenter && CurTime-KeyPressedLastTime>300)
					{
						UpdateReenter=TRUE;
						CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						UpdateReenter=FALSE;
					}
				}
			}

			if (Opt.ScreenSaver && Opt.ScreenSaverTime>0 &&
			        CurTime-StartIdleTime>Opt.ScreenSaverTime*60000)
				if (!ScreenSaver(WaitInMainLoop))
					return(KEY_NONE);

			if (!WaitInMainLoop && LoopCount==64)
			{
				LastEventIdle=TRUE;
				ClearStruct(*rec);
				rec->EventType=KEY_EVENT;
				return(KEY_IDLE);
			}
		}

		if (!(LoopCount & 3))
		{
			if (PluginSynchroManager.Process())
			{
				ClearStruct(*rec);
				return KEY_NONE;
			}
		}

		LoopCount++;
	} // while (1)


	clock_t CurClock=clock();

	if (rec->EventType==KEY_EVENT)
	{
		static bool bForceAltGr = false;

		if (!rec->Event.KeyEvent.bKeyDown)
		{
			was_repeat = false;
			last_pressed_keycode = (WORD)-1;
		}
		else
		{
			was_repeat = (last_pressed_keycode == rec->Event.KeyEvent.wVirtualKeyCode);
			last_pressed_keycode = rec->Event.KeyEvent.wVirtualKeyCode;

			if (rec->Event.KeyEvent.wVirtualKeyCode == VK_MENU)
			{
				// Шаманство с AltGr (виртуальная клавиатура)
				bForceAltGr = (rec->Event.KeyEvent.wVirtualScanCode == 0)
					&& ((rec->Event.KeyEvent.dwControlKeyState & 0x1F) == 0x0A);
			}
		}

		if (bForceAltGr && (rec->Event.KeyEvent.dwControlKeyState & 0x1F) == 0x0A)
		{
			rec->Event.KeyEvent.dwControlKeyState &= ~LEFT_ALT_PRESSED;
			rec->Event.KeyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
		}

		DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;

		//_SVS(if(rec->EventType==KEY_EVENT)SysLog(L"[%d] if(rec->EventType==KEY_EVENT) >>> %s",__LINE__,_INPUT_RECORD_Dump(rec)));
		if (CtrlObject && CtrlObject->Macro.IsRecording())
		{
			static WORD PrevVKKeyCode=0; // NumLock+Cursor
			WORD PrevVKKeyCode2=PrevVKKeyCode;
			PrevVKKeyCode=rec->Event.KeyEvent.wVirtualKeyCode;

			/* 1.07.2001 KM
			  При отпускании Shift-Enter в диалоге назначения
			  вылазил Shift после отпускания клавиш.
			*/
			if ((PrevVKKeyCode2==VK_SHIFT && PrevVKKeyCode==VK_RETURN &&
			        rec->Event.KeyEvent.bKeyDown) ||
			        (PrevVKKeyCode2==VK_RETURN && PrevVKKeyCode==VK_SHIFT &&
			         !rec->Event.KeyEvent.bKeyDown))
			{
				if (PrevVKKeyCode2 != VK_SHIFT)
				{
					INPUT_RECORD pinp;
					size_t nread;
					// Удалим из очереди...
					Console.ReadInput(&pinp, 1, nread);
					return KEY_NONE;
				}
			}
		}

		IntKeyState.CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
		IntKeyState.AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
		IntKeyState.ShiftPressed=(CtrlState & SHIFT_PRESSED);
		IntKeyState.RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
		IntKeyState.RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
		IntKeyState.RightShiftPressed=(CtrlState & SHIFT_PRESSED); //???
		KeyPressedLastTime=CurClock;

		/* $ 24.08.2000 SVS
		   + Добавление на реакцию KEY_CTRLALTSHIFTRELEASE
		*/
		if (IsKeyCASPressed && (Opt.CASRule&1) && (!IntKeyState.CtrlPressed || !IntKeyState.AltPressed || !IntKeyState.ShiftPressed))
		{
			IsKeyCASPressed=FALSE;
			return KEY_CTRLALTSHIFTRELEASE;
		}

		if (IsKeyRCASPressed && (Opt.CASRule&2) && (!IntKeyState.RightCtrlPressed || !IntKeyState.RightAltPressed || !IntKeyState.ShiftPressed))
		{
			IsKeyRCASPressed=FALSE;
			return KEY_RCTRLALTSHIFTRELEASE;
		}
	}
	else
	{
		was_repeat = false;
		last_pressed_keycode = (WORD)-1;
	}

	if (rec->EventType==FOCUS_EVENT)
	{
		/* $ 28.04.2001 VVM
		  + Не только обработаем сами смену фокуса, но и передадим дальше */
		IntKeyState.ShiftPressed=RightShiftPressedLast=ShiftPressedLast=FALSE;
		IntKeyState.CtrlPressed=CtrlPressedLast=RightCtrlPressedLast=FALSE;
		IntKeyState.AltPressed=AltPressedLast=RightAltPressedLast=FALSE;
		IntKeyState.MouseButtonState=0;
		ShiftState=FALSE;
		PressedLastTime=0;
		Console.ReadInput(rec, 1, ReadCount);
		CalcKey=rec->Event.FocusEvent.bSetFocus?KEY_GOTFOCUS:KEY_KILLFOCUS;
		//ClearStruct(*rec);
		//rec->EventType=KEY_EVENT;
		//чтоб решить баг винды приводящий к появлению скролов и т.п. после потери фокуса
		if (CalcKey == KEY_GOTFOCUS)
			RestoreConsoleWindowRect();
		else
			SaveConsoleWindowRect();

		return CalcKey;
	}

	//_SVS(if(rec->EventType==KEY_EVENT)SysLog(L"[%d] if(rec->EventType==KEY_EVENT) >>> %s",__LINE__,_INPUT_RECORD_Dump(rec)));
	IntKeyState.ReturnAltValue=FALSE;
	CalcKey=CalcKeyCode(rec,TRUE,&NotMacros);

	//_SVS(SysLog(L"1) CalcKey=%s",_FARKEY_ToName(CalcKey)));
	if (IntKeyState.ReturnAltValue && !NotMacros)
	{
		_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(CalcKey)));
		FrameManager->SetLastInputRecord(rec);
		irec.IntKey=CalcKey;
		irec.Rec=*rec;
		if (CtrlObject && CtrlObject->Macro.ProcessEvent(&irec))
		{
			rec->EventType=0;
			CalcKey=KEY_NONE;
		}

		return(CalcKey);
	}

	Console.ReadInput(rec, 1, ReadCount);

	if (EnableShowTime)
		ShowTime(1);

	bool SizeChanged=false;
	if(Opt.WindowMode)
	{
		SMALL_RECT CurConRect;
		Console.GetWindowRect(CurConRect);
		if(CurConRect.Bottom-CurConRect.Top!=ScrY || CurConRect.Right-CurConRect.Left!=ScrX)
		{
			SizeChanged=true;
		}
	}

	/*& 17.05.2001 OT Изменился размер консоли, генерим клавишу*/
	if (rec->EventType==WINDOW_BUFFER_SIZE_EVENT || SizeChanged)
	{
		int PScrX=ScrX;
		int PScrY=ScrY;
		// _SVS(SysLog(1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT)"));
		Sleep(1);
		GetVideoMode(CurSize);
		bool NotIgnore=Opt.WindowMode && (rec->Event.WindowBufferSizeEvent.dwSize.X!=CurSize.X || rec->Event.WindowBufferSizeEvent.dwSize.Y!=CurSize.Y);
		if (PScrX+1 == CurSize.X && PScrY+1 == CurSize.Y && !NotIgnore)
		{
			return KEY_NONE;
		}
		else
		{
			PrevScrX=PScrX;
			PrevScrY=PScrY;
			// _SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
			Sleep(1);

			if (FrameManager)
			{
				ScrBuf.ResetShadow();
				// апдейтим панели (именно они сейчас!)
				LockScreen LckScr;

				if (GlobalSaveScrPtr)
					GlobalSaveScrPtr->Discard();

				FrameManager->ResizeAllFrame();
				FrameManager->GetCurrentFrame()->Show();
				// _SVS(SysLog(L"PreRedrawFunc = %p",PreRedrawFunc));
				PreRedrawItem preRedrawItem=PreRedraw.Peek();

				if (preRedrawItem.PreRedrawFunc)
				{
					preRedrawItem.PreRedrawFunc();
				}
			}

			return(KEY_CONSOLE_BUFFER_RESIZE);
		}
	}

	if (rec->EventType==KEY_EVENT)
	{
		DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;
		DWORD KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
		IntKeyState.CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
		IntKeyState.AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
		IntKeyState.RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
		IntKeyState.RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);

		// Для NumPad!
		if ((CalcKey&(KEY_CTRL|KEY_SHIFT|KEY_ALT|KEY_RCTRL|KEY_RALT)) == KEY_SHIFT &&
		        (CalcKey&KEY_MASKF) >= KEY_NUMPAD0 && (CalcKey&KEY_MASKF) <= KEY_NUMPAD9)
			IntKeyState.ShiftPressed=SHIFT_PRESSED;
		else
			IntKeyState.ShiftPressed=(CtrlState & SHIFT_PRESSED);

		if ((KeyCode==VK_F16 && ReadKey==VK_F16) || !KeyCode)
			return(KEY_NONE);

		if (!rec->Event.KeyEvent.bKeyDown &&
		        (KeyCode==VK_SHIFT || KeyCode==VK_CONTROL || KeyCode==VK_MENU) &&
		        CurClock-PressedLastTime<500)
		{
			int Key=-1;

			if (ShiftPressedLast && KeyCode==VK_SHIFT)
			{
				if (ShiftPressedLast)
				{
					Key=KEY_SHIFT;
					// _SVS(SysLog(L"ShiftPressedLast, Key=KEY_SHIFT"));
				}
				else if (RightShiftPressedLast)
				{
					Key=KEY_RSHIFT;
					// _SVS(SysLog(L"RightShiftPressedLast, Key=KEY_RSHIFT"));
				}
			}

			if (KeyCode==VK_CONTROL)
			{
				if (CtrlPressedLast)
				{
					Key=KEY_CTRL;
					// _SVS(SysLog(L"CtrlPressedLast, Key=KEY_CTRL"));
				}
				else if (RightCtrlPressedLast)
				{
					Key=KEY_RCTRL;
					// _SVS(SysLog(L"CtrlPressedLast, Key=KEY_RCTRL"));
				}
			}

			if (KeyCode==VK_MENU)
			{
				if (AltPressedLast)
				{
					Key=KEY_ALT;
					// _SVS(SysLog(L"AltPressedLast, Key=KEY_ALT"));
				}
				else if (RightAltPressedLast)
				{
					Key=KEY_RALT;
					// _SVS(SysLog(L"RightAltPressedLast, Key=KEY_RALT"));
				}
			}

			{
				_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(Key)));
				if(FrameManager)
				{
					FrameManager->SetLastInputRecord(rec);
				}
				irec.IntKey=Key;
				irec.Rec=*rec;
				if (Key!=-1 && !NotMacros && CtrlObject && CtrlObject->Macro.ProcessEvent(&irec))
				{
					rec->EventType=0;
					Key=KEY_NONE;
				}
			}

			if (Key!=-1)
				return(Key);
		}

		RightShiftPressedLast=FALSE;
		CtrlPressedLast=RightCtrlPressedLast=FALSE;
		AltPressedLast=RightAltPressedLast=FALSE;
		ShiftPressedLast=(KeyCode==VK_SHIFT && rec->Event.KeyEvent.bKeyDown) ||
		                 (KeyCode==VK_RETURN && IntKeyState.ShiftPressed && !rec->Event.KeyEvent.bKeyDown);

		if (!ShiftPressedLast)
			if (KeyCode==VK_CONTROL && rec->Event.KeyEvent.bKeyDown)
			{
				if (CtrlState & RIGHT_CTRL_PRESSED)
				{
					RightCtrlPressedLast=TRUE;
					// _SVS(SysLog(L"RightCtrlPressedLast=TRUE;"));
				}
				else
				{
					CtrlPressedLast=TRUE;
					// _SVS(SysLog(L"CtrlPressedLast=TRUE;"));
				}
			}

		if (!ShiftPressedLast && !CtrlPressedLast && !RightCtrlPressedLast)
		{
			if (KeyCode==VK_MENU && rec->Event.KeyEvent.bKeyDown)
			{
				if (CtrlState & RIGHT_ALT_PRESSED)
				{
					RightAltPressedLast=TRUE;
				}
				else
				{
					AltPressedLast=TRUE;
				}

				PressedLastTime=CurClock;
			}
		}
		else
			PressedLastTime=CurClock;

		if (KeyCode==VK_SHIFT || KeyCode==VK_MENU || KeyCode==VK_CONTROL || KeyCode==VK_NUMLOCK || KeyCode==VK_SCROLL || KeyCode==VK_CAPITAL)
		{
			if ((KeyCode==VK_NUMLOCK || KeyCode==VK_SCROLL || KeyCode==VK_CAPITAL) &&
			        (CtrlState&(LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED))
			   )
			{
				// TODO:
				;
			}
			else
			{
				/* $ 24.08.2000 SVS
				   + Добавление на реакцию KEY_CTRLALTSHIFTPRESS
				*/
				switch (KeyCode)
				{
					case VK_SHIFT:
					case VK_MENU:
					case VK_CONTROL:

						if (!IsKeyCASPressed && IntKeyState.CtrlPressed && IntKeyState.AltPressed && IntKeyState.ShiftPressed)
						{
							if (!IsKeyRCASPressed && IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
							{
								if (Opt.CASRule&2)
								{
									IsKeyRCASPressed=TRUE;
									return (KEY_RCTRLALTSHIFTPRESS);
								}
							}
							else if (Opt.CASRule&1 && !(IntKeyState.RightCtrlPressed || IntKeyState.RightAltPressed))
							{
								IsKeyCASPressed=TRUE;
								return (KEY_CTRLALTSHIFTPRESS);
							}
						}

						break;
					case VK_LSHIFT:
					case VK_LMENU:
					case VK_LCONTROL:

						if (!IsKeyRCASPressed && IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
						{
							if ((Opt.CASRule&2))
							{
								IsKeyRCASPressed=TRUE;
								return (KEY_RCTRLALTSHIFTPRESS);
							}
						}

						break;
				}

				return(KEY_NONE);
			}
		}

		Panel::EndDrag();
	}

	if (rec->EventType==MOUSE_EVENT)
	{
		lastMOUSE_EVENT_RECORD=rec->Event.MouseEvent;
		IntKeyState.PreMouseEventFlags=IntKeyState.MouseEventFlags;
		IntKeyState.MouseEventFlags=rec->Event.MouseEvent.dwEventFlags;
		DWORD CtrlState=rec->Event.MouseEvent.dwControlKeyState;
		KeyMacro::SetMacroConst(constMsCtrlState,(__int64)CtrlState);
		KeyMacro::SetMacroConst(constMsEventFlags,(__int64)IntKeyState.MouseEventFlags);

		IntKeyState.CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
		IntKeyState.AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
		IntKeyState.ShiftPressed=(CtrlState & SHIFT_PRESSED);
		IntKeyState.RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
		IntKeyState.RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
		IntKeyState.RightShiftPressed=(CtrlState & SHIFT_PRESSED);
		DWORD BtnState=rec->Event.MouseEvent.dwButtonState;
		KeyMacro::SetMacroConst(constMsButton,(__int64)rec->Event.MouseEvent.dwButtonState);

		if (IntKeyState.MouseEventFlags != MOUSE_MOVED)
		{
			// _SVS(SysLog(L"1. CtrlState=%X IntKeyState.PrevRButtonPressed=%d,RButtonPressed=%d",CtrlState,IntKeyState.PrevRButtonPressed,RButtonPressed));
			IntKeyState.PrevMouseButtonState=IntKeyState.MouseButtonState;
		}

		IntKeyState.MouseButtonState=BtnState;
		// _SVS(SysLog(L"2. BtnState=%X IntKeyState.PrevRButtonPressed=%d,RButtonPressed=%d",BtnState,IntKeyState.PrevRButtonPressed,RButtonPressed));
		IntKeyState.PrevMouseX=IntKeyState.MouseX;
		IntKeyState.PrevMouseY=IntKeyState.MouseY;
		IntKeyState.MouseX=rec->Event.MouseEvent.dwMousePosition.X;
		IntKeyState.MouseY=rec->Event.MouseEvent.dwMousePosition.Y;
		KeyMacro::SetMacroConst(constMsX,(__int64)IntKeyState.MouseX);
		KeyMacro::SetMacroConst(constMsY,(__int64)IntKeyState.MouseY);

		/* $ 26.04.2001 VVM
		   + Обработка колесика мышки. */
		if (IntKeyState.MouseEventFlags == MOUSE_WHEELED)
		{ // Обработаем колесо и заменим на спец.клавиши
			short zDelta = HIWORD(rec->Event.MouseEvent.dwButtonState);
			CalcKey = (zDelta>0)?KEY_MSWHEEL_UP:KEY_MSWHEEL_DOWN;
			/* $ 27.04.2001 SVS
			   Не были учтены шифтовые клавиши при прокрутке колеса, из-за чего
			   нельзя было использовать в макросах нечто вроде "ShiftMsWheelUp"
			*/
			CalcKey |= (CtrlState&SHIFT_PRESSED?KEY_SHIFT:0)|
			           (CtrlState&LEFT_CTRL_PRESSED?KEY_CTRL:0)|
			           (CtrlState&RIGHT_CTRL_PRESSED?KEY_RCTRL:0)|
			           (CtrlState&LEFT_ALT_PRESSED?KEY_ALT:0)|
			           (CtrlState&RIGHT_ALT_PRESSED?KEY_RALT:0);
		}

		// Обработка горизонтального колесика (NT>=6)
		if (IntKeyState.MouseEventFlags == MOUSE_HWHEELED)
		{
			short zDelta = HIWORD(rec->Event.MouseEvent.dwButtonState);
			CalcKey = (zDelta>0)?KEY_MSWHEEL_RIGHT:KEY_MSWHEEL_LEFT;
			CalcKey |= (CtrlState&SHIFT_PRESSED?KEY_SHIFT:0)|
			           (CtrlState&LEFT_CTRL_PRESSED?KEY_CTRL:0)|
			           (CtrlState&RIGHT_CTRL_PRESSED?KEY_RCTRL:0)|
			           (CtrlState&LEFT_ALT_PRESSED?KEY_ALT:0)|
			           (CtrlState&RIGHT_ALT_PRESSED?KEY_RALT:0);
		}

		if (rec->EventType==MOUSE_EVENT && (!ExcludeMacro||ProcessMouse) && CtrlObject && (ProcessMouse || !(CtrlObject->Macro.IsRecording() || CtrlObject->Macro.IsExecuting())))
		{
			if (IntKeyState.MouseEventFlags != MOUSE_MOVED)
			{
				DWORD MsCalcKey=0;
				#if 0

				if (rec->Event.MouseEvent.dwButtonState&RIGHTMOST_BUTTON_PRESSED)
					MsCalcKey=(IntKeyState.MouseEventFlags == DOUBLE_CLICK)?KEY_MSRDBLCLICK:KEY_MSRCLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
					MsCalcKey=(IntKeyState.MouseEventFlags == DOUBLE_CLICK)?KEY_MSLDBLCLICK:KEY_MSLCLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED)
					MsCalcKey=(IntKeyState.MouseEventFlags == DOUBLE_CLICK)?KEY_MSM1DBLCLICK:KEY_MSM1CLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_3RD_BUTTON_PRESSED)
					MsCalcKey=(IntKeyState.MouseEventFlags == DOUBLE_CLICK)?KEY_MSM2DBLCLICK:KEY_MSM2CLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_4TH_BUTTON_PRESSED)
					MsCalcKey=(IntKeyState.MouseEventFlags == DOUBLE_CLICK)?KEY_MSM3DBLCLICK:KEY_MSM3CLICK;

				#else

				if (rec->Event.MouseEvent.dwButtonState&RIGHTMOST_BUTTON_PRESSED)
					MsCalcKey=KEY_MSRCLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
					MsCalcKey=KEY_MSLCLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED)
					MsCalcKey=KEY_MSM1CLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_3RD_BUTTON_PRESSED)
					MsCalcKey=KEY_MSM2CLICK;
				else if (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_4TH_BUTTON_PRESSED)
					MsCalcKey=KEY_MSM3CLICK;

				#endif

				if (MsCalcKey)
				{
					MsCalcKey |= (CtrlState&SHIFT_PRESSED?KEY_SHIFT:0)|
					             (CtrlState&LEFT_CTRL_PRESSED?KEY_CTRL:0)|
					             (CtrlState&RIGHT_CTRL_PRESSED?KEY_RCTRL:0)|
					             (CtrlState&LEFT_ALT_PRESSED?KEY_ALT:0)|
					             (CtrlState&RIGHT_ALT_PRESSED?KEY_RALT:0);

					// для WaitKey()
					if (ProcessMouse)
						return MsCalcKey;
					else
					{
						_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(MsCalcKey)));
						FrameManager->SetLastInputRecord(rec);
						irec.IntKey=MsCalcKey;
						irec.Rec=*rec;
						if (CtrlObject->Macro.ProcessEvent(&irec))
						{
							ClearStruct(*rec);
							return KEY_NONE;
						}
					}
				}
			}
		}
	}

	int GrayKey=(CalcKey==KEY_ADD || CalcKey==KEY_SUBTRACT || CalcKey==KEY_MULTIPLY);

	if (ReadKey && !GrayKey)
		CalcKey=ReadKey;

	{
		_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(%s)",__LINE__,_FARKEY_ToName(CalcKey)));
		if(FrameManager)
		{
			FrameManager->SetLastInputRecord(rec);
		}
		irec.IntKey=CalcKey;
		irec.Rec=*rec;
		if (!NotMacros && CtrlObject && CtrlObject->Macro.ProcessEvent(&irec))
		{
			rec->EventType=0;
			CalcKey=KEY_NONE;
		}
	}

	return(CalcKey);
}

DWORD PeekInputRecord(INPUT_RECORD *rec,bool ExcludeMacro)
{
	size_t ReadCount;
	DWORD Key;
	ScrBuf.Flush();

	if (KeyQueue && (Key=KeyQueue->Peek()) )
	{
		int VirtKey,ControlState;
		ReadCount=TranslateKeyToVK(Key,VirtKey,ControlState,rec)?1:0;
	}
	else if ((!ExcludeMacro) && (Key=CtrlObject->Macro.PeekKey()) )
	{
		int VirtKey,ControlState;
		ReadCount=TranslateKeyToVK(Key,VirtKey,ControlState,rec)?1:0;
	}
	else
	{
		Console.PeekInput(rec, 1, ReadCount);
	}

	if (!ReadCount)
		return 0;

	return(CalcKeyCode(rec,TRUE)); // ShieldCalcKeyCode?
}

/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
     Если KeyWait = -1 - как и раньше
*/
DWORD WaitKey(DWORD KeyWait,DWORD delayMS,bool ExcludeMacro)
{
	bool Visible=false;
	DWORD Size=0;

	if (KeyWait == KEY_CTRLALTSHIFTRELEASE || KeyWait == KEY_RCTRLALTSHIFTRELEASE)
	{
		GetCursorType(Visible,Size);
		SetCursorType(0,10);
	}

	clock_t CheckTime=clock()+delayMS;
	DWORD Key;

	for (;;)
	{
		INPUT_RECORD rec;
		Key=KEY_NONE;

		if (PeekInputRecord(&rec,ExcludeMacro))
		{
			Key=GetInputRecord(&rec,ExcludeMacro,true);
		}

		if (KeyWait == (DWORD)-1)
		{
			if ((Key&(~KEY_CTRLMASK)) < KEY_END_FKEY || IsInternalKeyReal(Key&(~KEY_CTRLMASK)))
				break;
		}
		else if (Key == KeyWait)
			break;

		if (delayMS && clock() >= CheckTime)
		{
			Key=KEY_NONE;
			break;
		}

		Sleep(1);
	}

	if (KeyWait == KEY_CTRLALTSHIFTRELEASE || KeyWait == KEY_RCTRLALTSHIFTRELEASE)
		SetCursorType(Visible,Size);

	return Key;
}

int WriteInput(int Key,DWORD Flags)
{
	if (Flags&(SKEY_VK_KEYS|SKEY_IDLE))
	{
		INPUT_RECORD Rec;
		size_t WriteCount;

		if (Flags&SKEY_IDLE)
		{
			Rec.EventType=FOCUS_EVENT;
			Rec.Event.FocusEvent.bSetFocus=TRUE;
		}
		else
		{
			Rec.EventType=KEY_EVENT;
			Rec.Event.KeyEvent.bKeyDown=1;
			Rec.Event.KeyEvent.wRepeatCount=1;
			Rec.Event.KeyEvent.wVirtualKeyCode=Key;
			Rec.Event.KeyEvent.wVirtualScanCode=MapVirtualKey(Rec.Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_VSC);

			if (Key < 0x30 || Key > 0x5A) // 0-9:;<=>?@@ A..Z  //?????
				Key=0;

			Rec.Event.KeyEvent.uChar.UnicodeChar=Rec.Event.KeyEvent.uChar.AsciiChar=Key;
			Rec.Event.KeyEvent.dwControlKeyState=0;
		}

		return Console.WriteInput(&Rec, 1, WriteCount);
	}
	else if (KeyQueue)
	{
		return KeyQueue->Put(((DWORD)Key)|(Flags&SKEY_NOTMACROS?0x80000000:0));
	}
	else
		return 0;
}


int CheckForEscSilent()
{
	if(CloseFAR)
	{
		return TRUE;
	}

	INPUT_RECORD rec;
	BOOL Processed=TRUE;
	/* TODO: Здесь, в общем то - ХЗ, т.к.
	         по хорошему нужно проверять CtrlObject->Macro.PeekKey() на ESC или BREAK
	         Но к чему это приведет - пока не могу дать ответ !!!
	*/

	// если в "макросе"...
	if (CtrlObject->Macro.IsExecuting() != MACROMODE_NOMACRO && FrameManager->GetCurrentFrame())
	{
		if (CtrlObject->Macro.IsDsableOutput())
			Processed=FALSE;
	}

	if (Processed && PeekInputRecord(&rec))
	{
		int MMode=CtrlObject->Macro.GetMode();
		CtrlObject->Macro.SetMode(MACRO_LAST); // чтобы не срабатывали макросы :-)
		int Key=GetInputRecord(&rec,false,false,false);
		CtrlObject->Macro.SetMode(MMode);

		if (Key==KEY_ESC)
			return TRUE;
		if (Key==KEY_BREAK)
		{
			if (CtrlObject->Macro.IsExecuting() != MACROMODE_NOMACRO)
				CtrlObject->Macro.SendDropProcess();
			return TRUE;
		}
		else if (Key==KEY_ALTF9 || Key==KEY_RALTF9)
			FrameManager->ProcessKey(KEY_ALTF9);
	}

	if (!Processed && CtrlObject->Macro.IsExecuting() != MACROMODE_NOMACRO)
		ScrBuf.Flush();

	return FALSE;
}

int ConfirmAbortOp()
{
	return (Opt.Confirm.Esc && !CloseFAR)?AbortMessage():TRUE;
}

/* $ 09.02.2001 IS
     Подтверждение нажатия Esc
*/
int CheckForEsc()
{
	if (CheckForEscSilent())
		return(ConfirmAbortOp());
	else
		return FALSE;
}

/* $ 25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
*/
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
   Size=0 - по максимуму!
*/
static string &GetShiftKeyName(string &strName, DWORD Key,int& Len)
{
	if ((Key&KEY_RCTRL) == KEY_RCTRL)   strName += ModifKeyName[0].Name;
	else if (Key&KEY_CTRL)              strName += ModifKeyName[2].Name;

//  else if(Key&KEY_LCTRL)             strcat(Name,ModifKeyName[3].Name);

	if ((Key&KEY_RALT) == KEY_RALT)     strName += ModifKeyName[3].Name;
	else if (Key&KEY_ALT)               strName += ModifKeyName[4].Name;

//  else if(Key&KEY_LALT)    strcat(Name,ModifKeyName[6].Name);

	if (Key&KEY_SHIFT)                  strName += ModifKeyName[1].Name;

//  else if(Key&KEY_LSHIFT)  strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_RSHIFT)  strcat(Name,ModifKeyName[1].Name);
	if (Key&KEY_M_SPEC)                 strName += ModifKeyName[5].Name;
	else if (Key&KEY_M_OEM)             strName += ModifKeyName[6].Name;

	Len=(int)strName.GetLength();
	return strName;
}

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!

   Функция KeyNameToKey ждет строку по вот такой спецификации:

   1. Сочетания, определенные в структуре FKeys1[]
   2. Опциональные модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift) и 1 символ, например, AltD или CtrlC
   3. "Alt" (или RAlt) и 5 десятичных цифр (с ведущими нулями)
   4. "Spec" и 5 десятичных цифр (с ведущими нулями)
   5. "Oem" и 5 десятичных цифр (с ведущими нулями)
   6. только модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift)
*/
int KeyNameToKey(const wchar_t *Name)
{
	if (!Name || !*Name)
		return -1;

	DWORD Key=0;
    // _SVS(SysLog(L"KeyNameToKey('%s')",Name));

	// Это макроклавиша?
	if (Name[0] == L'$' && Name[1])
		return -1;// KeyNameMacroToKey(Name);

	if (Name[0] == L'%' && Name[1])
		return -1;

	if (Name[1] && wcspbrk(Name,L"()")) // если не один символ и встречаются '(' или ')', то это явно не клавиша!
		return -1;

//   if((Key=KeyNameMacroToKey(Name)) != (DWORD)-1)
//     return Key;
	int I, Pos;
	static string strTmpName;
	strTmpName = Name;
	strTmpName.Upper();
	int Len=(int)strTmpName.GetLength();

	// пройдемся по всем модификаторам
	for (Pos=I=0; I < int(ARRAYSIZE(ModifKeyName)); ++I)
	{
		if (wcsstr(strTmpName,ModifKeyName[I].UName) && !(Key&ModifKeyName[I].Key))
		{
			int CntReplace=ReplaceStrings(strTmpName,ModifKeyName[I].UName,L"",-1,true);
			Key|=ModifKeyName[I].Key;
			Pos+=ModifKeyName[I].Len*CntReplace;
		}
	}
    // _SVS(SysLog(L"[%d] Name=%s",__LINE__,Name));

	//Pos=strlen(TmpName);

	// если что-то осталось - преобразуем.
	if (Pos < Len)
	{
		// сначала - FKeys1 - Вариант (1)
		const wchar_t* Ptr=Name+Pos;
		int PtrLen = Len-Pos;

		for (I=(int)ARRAYSIZE(FKeys1)-1; I>=0; I--)
		{
			if (PtrLen == FKeys1[I].Len && !StrCmpI(Ptr,FKeys1[I].Name))
			{
				Key|=FKeys1[I].Key;
				Pos+=FKeys1[I].Len;
				break;
			}
		}

		if (I == -1) // F-клавиш нет?
		{
			/*
				здесь только 5 оставшихся вариантов:
				2) Опциональные модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift) и 1 символ, например, AltD или CtrlC
				3) "Alt" (или RAlt) и 5 десятичных цифр (с ведущими нулями)
				4) "Spec" и 5 десятичных цифр (с ведущими нулями)
				5) "Oem" и 5 десятичных цифр (с ведущими нулями)
				6) только модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift)
			*/

			if (Len == 1 || Pos == Len-1) // Вариант (2)
			{
				int Chr=Name[Pos];

				// если были модификаторы Alt/Ctrl, то преобразуем в "физичекую клавишу" (независимо от языка)
				if (Key&(KEY_ALT|KEY_RCTRL|KEY_CTRL|KEY_RALT))
				{
					if (Chr > 0x7F)
						Chr=KeyToKeyLayout(Chr);

					Chr=Upper(Chr);
				}

				Key|=Chr;

				if (Chr)
					Pos++;
			}
			else if (Key == KEY_ALT || Key == KEY_RALT || Key == KEY_M_SPEC || Key == KEY_M_OEM) // Варианты (3), (4) и (5)
			{
				wchar_t *endptr=nullptr;
				int K=(int)wcstol(Ptr, &endptr, 10);

				if (Ptr+5 == endptr)
				{
					if (Key == KEY_ALT || Key == KEY_RALT) // Вариант (3) - Alt-Num
						Key=(Key|K|KEY_ALTDIGIT)&(~(KEY_ALT|KEY_RALT));
					else if (Key == KEY_M_SPEC) // Вариант (4)
						Key=(Key|(K+KEY_VK_0xFF_BEGIN))&(~(KEY_M_SPEC|KEY_M_OEM));
					else if (Key == KEY_M_OEM) // Вариант (5)
						Key=(Key|(K+KEY_FKEY_BEGIN))&(~(KEY_M_SPEC|KEY_M_OEM));

					Pos=Len;
				}
			}
			// Вариант (6). Уже "собран".
		}
	}

	// _SVS(SysLog(L"Key=0x%08X (%c) => '%s'",Key,(Key?Key:' '),Name));
	return (!Key || Pos < Len)? -1: (int)Key;
}

#ifdef FAR_LUA
bool InputRecordToText(const INPUT_RECORD *Rec, string &strKeyText)
{
	return KeyToText(InputRecordToKey(Rec),strKeyText);
}
#endif

BOOL KeyToText(int Key0, string &strKeyText0)
{
	string strKeyText;
	DWORD Key=(DWORD)Key0, FKey=(DWORD)Key0&0xFFFFFF;
	//if(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE)
	//  return KeyMacroToText(Key0, strKeyText0);

	if (Key&KEY_ALTDIGIT)
	{
		strKeyText.Format(L"Alt%05d", Key&FKey);
	}
	else
	{
		int I, Len;
		GetShiftKeyName(strKeyText,Key,Len);

		for (I=0; I<int(ARRAYSIZE(FKeys1)); I++)
		{
			if (FKey==FKeys1[I].Key)
			{
				strKeyText += FKeys1[I].Name;
				break;
			}
		}

		if (I  == ARRAYSIZE(FKeys1))
		{
			FormatString strKeyTemp;
			if (FKey >= KEY_VK_0xFF_BEGIN && FKey <= KEY_VK_0xFF_END)
			{
				strKeyTemp << L"Spec" <<fmt::MinWidth(5) << FKey-KEY_VK_0xFF_BEGIN;
				strKeyText += strKeyTemp;
			}
			else if (FKey > KEY_LAUNCH_APP2 && FKey < KEY_CTRLALTSHIFTPRESS)
			{
				strKeyTemp << L"Oem" <<fmt::MinWidth(5) << FKey-KEY_FKEY_BEGIN;
				strKeyText += strKeyTemp;
			}
			else
			{
				#if defined(SYSLOG)
				// Этот кусок кода нужен только для того, что "спецклавиши" логировались нормально
				for (I=0; I<ARRAYSIZE(SpecKeyName); I++)
					if (FKey==SpecKeyName[I].Key)
					{
						strKeyText += SpecKeyName[I].Name;
						break;
					}

				if (I  == ARRAYSIZE(SpecKeyName))
				#endif

				{
					FKey=Upper((wchar_t)Key&0xFFFF);

					wchar_t KeyText[2]={};

					if (FKey >= L'A' && FKey <= L'Z')
					{
						if (Key&(KEY_RCTRL|KEY_CTRL|KEY_RALT|KEY_ALT)) // ??? а если есть другие модификаторы ???
							KeyText[0]=(wchar_t)FKey; // для клавиш с модификаторами подставляем "латиницу" в верхнем регистре
						else
							KeyText[0]=(wchar_t)(Key&0xFFFF);
					}
					else
						KeyText[0]=(wchar_t)Key&0xFFFF;

					strKeyText += KeyText;
				}
			}
		}

		if (strKeyText.IsEmpty())
		{
			strKeyText0.Clear();
			return FALSE;
		}
	}

	strKeyText0 = strKeyText;
	return TRUE;
}


int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *Rec)
{
	_KEYMACRO(CleverSysLog Clev(L"TranslateKeyToVK()"));
	_KEYMACRO(SysLog(L"Param: Key=%08X",Key));

 	WORD EventType=KEY_EVENT;

 	DWORD FKey  =Key&KEY_END_SKEY;
 	DWORD FShift=Key&KEY_CTRLMASK;

	VirtKey=0;

  	ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
  	             (FShift&KEY_ALT?PKF_ALT:0)|
 	             (FShift&KEY_RALT?PKF_RALT:0)|
 	             (FShift&KEY_RCTRL?PKF_RCONTROL:0)|
  	             (FShift&KEY_CTRL?PKF_CONTROL:0);

	bool KeyInTable=false;
	size_t i;
	for (i=0; i < ARRAYSIZE(Table_KeyToVK); i++)
	{
		if (FKey==(DWORD)Table_KeyToVK[i].Key)
		{
			VirtKey=Table_KeyToVK[i].VK;
			KeyInTable=true;
			break;
		}
	}

	if (!KeyInTable)
	{
 		// TODO: KEY_ALTDIGIT
 		if ((FKey>=L'0' && FKey<=L'9') || (FKey>=L'A' && FKey<=L'Z'))
 		{
			VirtKey=FKey;
			if ((FKey>=L'A' && FKey<=L'Z') && !(FShift&0xFF000000))
				FShift |= KEY_SHIFT;
		}
 		//else if (FKey > KEY_VK_0xFF_BEGIN && FKey < KEY_VK_0xFF_END)
 		//	VirtKey=FKey-KEY_FKEY_BEGIN;
		else if (FKey > KEY_FKEY_BEGIN && FKey < KEY_END_FKEY)
			VirtKey=FKey-KEY_FKEY_BEGIN;
		else if (FKey && FKey < WCHAR_MAX)
		{
			short Vk = VkKeyScan(static_cast<WCHAR>(FKey));
			if (Vk == -1)
			{
				for (i=0; i < ARRAYSIZE(Layout); ++i)
					if (Layout[i])
					{
						Vk = VkKeyScanEx(static_cast<WCHAR>(FKey),Layout[i]);
						if (Vk != -1)
							break;
					}
			}

			if (Vk == -1)
			{
				// Заполнить хотя бы .UnicodeChar = FKey
				VirtKey = -1;
			}
			else
			{
				if (IsCharUpper(FKey) && !(FShift&0xFF000000))
					FShift |= KEY_SHIFT;

				VirtKey = Vk&0xFF;
				if (HIBYTE(Vk)&&(HIBYTE(Vk)&6)!=6) //RAlt-E в немецкой раскладке это евро, а не CtrlRAltЕвро
				{
					FShift|=(HIBYTE(Vk)&1?KEY_SHIFT:0)|
							(HIBYTE(Vk)&2?KEY_CTRL:0)|
							(HIBYTE(Vk)&4?KEY_ALT:0);

			  		ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
  	        				 (FShift&KEY_ALT?PKF_ALT:0)|
		 					 (FShift&KEY_RALT?PKF_RALT:0)|
 	    					 (FShift&KEY_RCTRL?PKF_RCONTROL:0)|
  	            			 (FShift&KEY_CTRL?PKF_CONTROL:0);
				}
			}

		}
		else if (!FKey)
		{
			DWORD ExtKey[]={KEY_SHIFT,VK_SHIFT,KEY_CTRL,VK_CONTROL,KEY_ALT,VK_MENU,KEY_RSHIFT,VK_RSHIFT,KEY_RCTRL,VK_RCONTROL,KEY_RALT,VK_RMENU};
			for (i=0; i < ARRAYSIZE(ExtKey); i+=2)
				if(FShift == ExtKey[i])
				{
					VirtKey=ExtKey[i+1];
					break;
				}
		}
		else
 		{
  			VirtKey=FKey;
 			switch (FKey)
 			{
 				case KEY_NUMDEL:
 					VirtKey=VK_DELETE;
 					break;
 				case KEY_NUMENTER:
 					VirtKey=VK_RETURN;
 					break;

 				case KEY_NONE:
 				case KEY_IDLE:
 					EventType=MENU_EVENT;
 					break;

 				case KEY_DRAGCOPY:
 				case KEY_DRAGMOVE:
 					EventType=MENU_EVENT;
 					break;

 				case KEY_MSWHEEL_UP:
 				case KEY_MSWHEEL_DOWN:
 				case KEY_MSWHEEL_LEFT:
 				case KEY_MSWHEEL_RIGHT:
 				case KEY_MSLCLICK:
 				case KEY_MSRCLICK:
 				case KEY_MSM1CLICK:
 				case KEY_MSM2CLICK:
 				case KEY_MSM3CLICK:
 					EventType=MOUSE_EVENT;
 					break;
				case KEY_KILLFOCUS:
				case KEY_GOTFOCUS:
 					EventType=FOCUS_EVENT;
 					break;
				case KEY_CONSOLE_BUFFER_RESIZE:
 					EventType=WINDOW_BUFFER_SIZE_EVENT;
 					break;
				default:
 					EventType=MENU_EVENT;
 					break;
 			}
 		}
	}

	/* TODO:
		KEY_CTRLALTSHIFTPRESS
		KEY_CTRLALTSHIFTRELEASE
		KEY_RCTRLALTSHIFTPRESS
		KEY_RCTRLALTSHIFTRELEASE
	*/


	if (Rec)
	{
		Rec->EventType=EventType;

		switch (EventType)
		{
			case KEY_EVENT:
			{
				if (VirtKey)
				{
					Rec->Event.KeyEvent.bKeyDown=1;
					Rec->Event.KeyEvent.wRepeatCount=1;
					if (VirtKey != -1)
					{
						// При нажатии RCtrl и RAlt в консоль приходит VK_CONTROL и VK_MENU а не их правые аналоги
						Rec->Event.KeyEvent.wVirtualKeyCode = (VirtKey==VK_RCONTROL)?VK_CONTROL:(VirtKey==VK_RMENU)?VK_MENU:VirtKey;
						Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_VSC);
					}
					else
					{
						Rec->Event.KeyEvent.wVirtualKeyCode = 0;
						Rec->Event.KeyEvent.wVirtualScanCode = 0;
					}
					Rec->Event.KeyEvent.uChar.UnicodeChar=FKey > WCHAR_MAX?0:FKey;

					// здесь подход к Shift-клавишам другой, нежели для ControlState
					Rec->Event.KeyEvent.dwControlKeyState=
					    (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
					    (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
					    (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
					    (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
					    (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);

					DWORD ExtKey[]={KEY_PGUP,KEY_PGDN,KEY_END,KEY_HOME,KEY_LEFT,KEY_UP,KEY_RIGHT,KEY_DOWN,KEY_INS,KEY_DEL,KEY_NUMENTER};
					for (i=0; i < ARRAYSIZE(ExtKey); i++)
						if(FKey == ExtKey[i])
						{
							Rec->Event.KeyEvent.dwControlKeyState|=ENHANCED_KEY;
							break;
						}
				}
				break;
			}

			case MOUSE_EVENT:
			{
				DWORD ButtonState=0;
				DWORD EventFlags=0;

				switch (FKey)
				{
					case KEY_MSWHEEL_UP:
						ButtonState=MAKELONG(0,120);
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_DOWN:
						ButtonState=MAKELONG(0,(WORD)(short)-120);
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_RIGHT:
						ButtonState=MAKELONG(0,120);
						EventFlags|=MOUSE_HWHEELED;
						break;
					case KEY_MSWHEEL_LEFT:
						ButtonState=MAKELONG(0,(WORD)(short)-120);
						EventFlags|=MOUSE_HWHEELED;
						break;

					case KEY_MSLCLICK:
						ButtonState=FROM_LEFT_1ST_BUTTON_PRESSED;
						break;
					case KEY_MSRCLICK:
						ButtonState=RIGHTMOST_BUTTON_PRESSED;
						break;
					case KEY_MSM1CLICK:
						ButtonState=FROM_LEFT_2ND_BUTTON_PRESSED;
						break;
					case KEY_MSM2CLICK:
						ButtonState=FROM_LEFT_3RD_BUTTON_PRESSED;
						break;
					case KEY_MSM3CLICK:
						ButtonState=FROM_LEFT_4TH_BUTTON_PRESSED;
						break;
				}

				Rec->Event.MouseEvent.dwButtonState=ButtonState;
				Rec->Event.MouseEvent.dwEventFlags=EventFlags;
				Rec->Event.MouseEvent.dwControlKeyState=
					    (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
					    (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
					    (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
					    (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
					    (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);
				Rec->Event.MouseEvent.dwMousePosition.X=IntKeyState.MouseX;
				Rec->Event.MouseEvent.dwMousePosition.Y=IntKeyState.MouseY;
				break;
			}
			case WINDOW_BUFFER_SIZE_EVENT:
				GetVideoMode(Rec->Event.WindowBufferSizeEvent.dwSize);
				break;
			case MENU_EVENT:
				Rec->Event.MenuEvent.dwCommandId=0;
				break;
			case FOCUS_EVENT:
				Rec->Event.FocusEvent.bSetFocus = FKey == KEY_KILLFOCUS?FALSE:TRUE;
				break;
		}
	}

	_SVS(string strKeyText0;KeyToText(Key,strKeyText0));
	_SVS(SysLog(L"%s or %s ==> %s",_FARKEY_ToName(Key),_MCODE_ToName(Key),_INPUT_RECORD_Dump(Rec)));
	_SVS(SysLog(L"return VirtKey=%x",VirtKey));
	return VirtKey;
}


int IsNavKey(DWORD Key)
{
	static DWORD NavKeys[][2]=
	{
		{0,KEY_CTRLC},
		{0,KEY_RCTRLC},
		{0,KEY_INS},      {0,KEY_NUMPAD0},
		{0,KEY_CTRLINS},  {0,KEY_CTRLNUMPAD0},
		{0,KEY_RCTRLINS}, {0,KEY_RCTRLNUMPAD0},

		{1,KEY_LEFT},     {1,KEY_NUMPAD4},
		{1,KEY_RIGHT},    {1,KEY_NUMPAD6},
		{1,KEY_HOME},     {1,KEY_NUMPAD7},
		{1,KEY_END},      {1,KEY_NUMPAD1},
		{1,KEY_UP},       {1,KEY_NUMPAD8},
		{1,KEY_DOWN},     {1,KEY_NUMPAD2},
		{1,KEY_PGUP},     {1,KEY_NUMPAD9},
		{1,KEY_PGDN},     {1,KEY_NUMPAD3},
		//!!!!!!!!!!!
	};

	for (int I=0; I < int(ARRAYSIZE(NavKeys)); I++)
		if ((!NavKeys[I][0] && Key==NavKeys[I][1]) ||
		        (NavKeys[I][0] && (Key&0x00FFFFFF)==(NavKeys[I][1]&0x00FFFFFF)))
			return TRUE;

	return FALSE;
}

int IsShiftKey(DWORD Key)
{
	static DWORD ShiftKeys[]=
	{
		KEY_SHIFTLEFT,          KEY_SHIFTNUMPAD4,
		KEY_SHIFTRIGHT,         KEY_SHIFTNUMPAD6,
		KEY_SHIFTHOME,          KEY_SHIFTNUMPAD7,
		KEY_SHIFTEND,           KEY_SHIFTNUMPAD1,
		KEY_SHIFTUP,            KEY_SHIFTNUMPAD8,
		KEY_SHIFTDOWN,          KEY_SHIFTNUMPAD2,
		KEY_SHIFTPGUP,          KEY_SHIFTNUMPAD9,
		KEY_SHIFTPGDN,          KEY_SHIFTNUMPAD3,
		KEY_CTRLSHIFTHOME,      KEY_CTRLSHIFTNUMPAD7,
		KEY_RCTRLSHIFTHOME,     KEY_RCTRLSHIFTNUMPAD7,
		KEY_CTRLSHIFTPGUP,      KEY_CTRLSHIFTNUMPAD9,
		KEY_RCTRLSHIFTPGUP,     KEY_RCTRLSHIFTNUMPAD9,
		KEY_CTRLSHIFTEND,       KEY_CTRLSHIFTNUMPAD1,
		KEY_RCTRLSHIFTEND,      KEY_RCTRLSHIFTNUMPAD1,
		KEY_CTRLSHIFTPGDN,      KEY_CTRLSHIFTNUMPAD3,
		KEY_RCTRLSHIFTPGDN,     KEY_RCTRLSHIFTNUMPAD3,
		KEY_CTRLSHIFTLEFT,      KEY_CTRLSHIFTNUMPAD4,
		KEY_RCTRLSHIFTLEFT,     KEY_RCTRLSHIFTNUMPAD4,
		KEY_CTRLSHIFTRIGHT,     KEY_CTRLSHIFTNUMPAD6,
		KEY_RCTRLSHIFTRIGHT,    KEY_RCTRLSHIFTNUMPAD6,
		KEY_ALTSHIFTDOWN,       KEY_ALTSHIFTNUMPAD2,
		KEY_RALTSHIFTDOWN,      KEY_RALTSHIFTNUMPAD2,
		KEY_ALTSHIFTLEFT,       KEY_ALTSHIFTNUMPAD4,
		KEY_RALTSHIFTLEFT,      KEY_RALTSHIFTNUMPAD4,
		KEY_ALTSHIFTRIGHT,      KEY_ALTSHIFTNUMPAD6,
		KEY_RALTSHIFTRIGHT,     KEY_RALTSHIFTNUMPAD6,
		KEY_ALTSHIFTUP,         KEY_ALTSHIFTNUMPAD8,
		KEY_RALTSHIFTUP,        KEY_RALTSHIFTNUMPAD8,
		KEY_ALTSHIFTEND,        KEY_ALTSHIFTNUMPAD1,
		KEY_RALTSHIFTEND,       KEY_RALTSHIFTNUMPAD1,
		KEY_ALTSHIFTHOME,       KEY_ALTSHIFTNUMPAD7,
		KEY_RALTSHIFTHOME,      KEY_RALTSHIFTNUMPAD7,
		KEY_ALTSHIFTPGDN,       KEY_ALTSHIFTNUMPAD3,
		KEY_RALTSHIFTPGDN,      KEY_RALTSHIFTNUMPAD3,
		KEY_ALTSHIFTPGUP,       KEY_ALTSHIFTNUMPAD9,
		KEY_RALTSHIFTPGUP,      KEY_RALTSHIFTNUMPAD9,
		KEY_CTRLALTPGUP,        KEY_CTRLALTNUMPAD9,
		KEY_RCTRLRALTPGUP,      KEY_RCTRLRALTNUMPAD9,
		KEY_CTRLRALTPGUP,       KEY_CTRLRALTNUMPAD9,
		KEY_RCTRLALTPGUP,       KEY_RCTRLALTNUMPAD9,
		KEY_CTRLALTHOME,        KEY_CTRLALTNUMPAD7,
		KEY_RCTRLRALTHOME,      KEY_RCTRLRALTNUMPAD7,
		KEY_CTRLRALTHOME,       KEY_CTRLRALTNUMPAD7,
		KEY_RCTRLALTHOME,       KEY_RCTRLALTNUMPAD7,
		KEY_CTRLALTPGDN,        KEY_CTRLALTNUMPAD2,
		KEY_RCTRLRALTPGDN,      KEY_RCTRLRALTNUMPAD2,
		KEY_CTRLRALTPGDN,       KEY_CTRLRALTNUMPAD2,
		KEY_RCTRLALTPGDN,       KEY_RCTRLALTNUMPAD2,
		KEY_CTRLALTEND,         KEY_CTRLALTNUMPAD1,
		KEY_RCTRLRALTEND,       KEY_RCTRLRALTNUMPAD1,
		KEY_CTRLRALTEND,        KEY_CTRLRALTNUMPAD1,
		KEY_RCTRLALTEND,        KEY_RCTRLALTNUMPAD1,
		KEY_CTRLALTLEFT,        KEY_CTRLALTNUMPAD4,
		KEY_RCTRLRALTLEFT,      KEY_RCTRLRALTNUMPAD4,
		KEY_CTRLRALTLEFT,       KEY_CTRLRALTNUMPAD4,
		KEY_RCTRLALTLEFT,       KEY_RCTRLALTNUMPAD4,
		KEY_CTRLALTRIGHT,       KEY_CTRLALTNUMPAD6,
		KEY_RCTRLRALTRIGHT,     KEY_RCTRLRALTNUMPAD6,
		KEY_CTRLRALTRIGHT,      KEY_CTRLRALTNUMPAD6,
		KEY_RCTRLALTRIGHT,      KEY_RCTRLALTNUMPAD6,
		KEY_ALTUP,
		KEY_RALTUP,
		KEY_ALTLEFT,
		KEY_RALTLEFT,
		KEY_ALTDOWN,
		KEY_RALTDOWN,
		KEY_ALTRIGHT,
		KEY_RALTRIGHT,
		KEY_ALTHOME,
		KEY_RALTHOME,
		KEY_ALTEND,
		KEY_RALTEND,
		KEY_ALTPGUP,
		KEY_RALTPGUP,
		KEY_ALTPGDN,
		KEY_RALTPGDN,
		KEY_ALT,
		KEY_RALT,
		KEY_CTRL,
		KEY_RCTRL,
	};

	for (int I=0; I<int(ARRAYSIZE(ShiftKeys)); I++)
		if (Key==ShiftKeys[I])
			return TRUE;

	return FALSE;
}

DWORD ShieldCalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros,bool ProcessCtrlCode)
{
	FarKeyboardState _IntKeyState=IntKeyState; // нада! ибо CalcKeyCode "портит"... (Mantis#0001760)
	ClearStruct(IntKeyState);
	DWORD Ret=CalcKeyCode(rec,RealKey,NotMacros,ProcessCtrlCode);
	IntKeyState=_IntKeyState;
	return Ret;
}

// GetAsyncKeyState(VK_RSHIFT)
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros,bool ProcessCtrlCode)
{
	_SVS(CleverSysLog Clev(L"CalcKeyCode"));
	_SVS(SysLog(L"CalcKeyCode -> %s| RealKey=%d  *NotMacros=%d",_INPUT_RECORD_Dump(rec),RealKey,(NotMacros?*NotMacros:0)));
	UINT CtrlState=(rec->EventType==MOUSE_EVENT)?rec->Event.MouseEvent.dwControlKeyState:rec->Event.KeyEvent.dwControlKeyState;
	UINT ScanCode=rec->Event.KeyEvent.wVirtualScanCode;
	UINT KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
	WCHAR Char=rec->Event.KeyEvent.uChar.UnicodeChar;
	// _SVS(if(KeyCode == VK_DECIMAL || KeyCode == VK_DELETE) SysLog(L"CalcKeyCode -> CtrlState=%04X KeyCode=%s ScanCode=%08X AsciiChar=%02X IntKeyState.ShiftPressed=%d ShiftPressedLast=%d",CtrlState,_VK_KEY_ToName(KeyCode), ScanCode, Char.AsciiChar,IntKeyState.ShiftPressed,ShiftPressedLast));

	if (NotMacros)
		*NotMacros=CtrlState&0x80000000?TRUE:FALSE;

//  CtrlState&=~0x80000000;

	if (!(rec->EventType==KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT || rec->EventType == MOUSE_EVENT))
		return(KEY_NONE);

	if (!RealKey)
	{
		IntKeyState.CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
		IntKeyState.AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
		IntKeyState.ShiftPressed=(CtrlState & SHIFT_PRESSED);
		IntKeyState.RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
		IntKeyState.RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
		IntKeyState.RightShiftPressed=(CtrlState & SHIFT_PRESSED);
	}

	DWORD Modif=(IntKeyState.ShiftPressed?KEY_SHIFT:0)
		|(IntKeyState.CtrlPressed?(IntKeyState.RightCtrlPressed?KEY_RCTRL:KEY_CTRL):0)
		|(IntKeyState.AltPressed?(IntKeyState.RightAltPressed?KEY_RALT:KEY_ALT):0);
	DWORD ModifAlt=(IntKeyState.RightAltPressed?KEY_RALT:(IntKeyState.AltPressed?KEY_ALT:0));
	DWORD ModifCtrl=(IntKeyState.RightCtrlPressed?KEY_RCTRL:(IntKeyState.CtrlPressed?KEY_CTRL:0));

	if (rec->EventType==MOUSE_EVENT)
	{
		if (!(rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED || rec->Event.MouseEvent.dwEventFlags==MOUSE_HWHEELED || rec->Event.MouseEvent.dwEventFlags==0))
		{
			return(KEY_NONE);
		}

		if (rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED)
		{
			if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) > 0)
				return(Modif|KEY_MSWHEEL_UP);
			else if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) < 0)
				return(Modif|KEY_MSWHEEL_DOWN);
		}
		else if (rec->Event.MouseEvent.dwEventFlags==MOUSE_HWHEELED)
		{
			if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) > 0)
				return(Modif|KEY_MSWHEEL_RIGHT);
			else if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) < 0)
				return(Modif|KEY_MSWHEEL_LEFT);
		}
		else if (rec->Event.MouseEvent.dwEventFlags==0)
		{
			switch (rec->Event.MouseEvent.dwButtonState)
			{
			case FROM_LEFT_1ST_BUTTON_PRESSED:
				return(Modif|KEY_MSLCLICK);
			case RIGHTMOST_BUTTON_PRESSED:
				return(Modif|KEY_MSRCLICK);
			case FROM_LEFT_2ND_BUTTON_PRESSED:
				return(Modif|KEY_MSM1CLICK);
			case FROM_LEFT_3RD_BUTTON_PRESSED:
				return(Modif|KEY_MSM2CLICK);
			case FROM_LEFT_4TH_BUTTON_PRESSED:
				return(Modif|KEY_MSM3CLICK);
			}
		}

		return(KEY_NONE);
	}

	if (rec->Event.KeyEvent.wVirtualKeyCode >= 0xFF && RealKey)
	{
		//VK_?=0x00FF, Scan=0x0013 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0014 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0015 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001A uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001B uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001E uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001F uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0023 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		if (!rec->Event.KeyEvent.bKeyDown && (CtrlState&(ENHANCED_KEY|NUMLOCK_ON)))
			return Modif|(KEY_VK_0xFF_BEGIN+ScanCode);

		return KEY_IDLE;
	}

	static DWORD Time=0;

	if (!AltValue)
	{
		Time=GetTickCount();
	}

	if (!rec->Event.KeyEvent.bKeyDown)
	{
		KeyCodeForALT_LastPressed=0;

		if (KeyCode==VK_MENU && AltValue)
		{
			//FlushInputBuffer();//???
			INPUT_RECORD TempRec;
			size_t ReadCount;
			Console.ReadInput(&TempRec, 1, ReadCount);
			IntKeyState.ReturnAltValue=TRUE;
			//_SVS(SysLog(L"0 AltNumPad -> AltValue=0x%0X CtrlState=%X",AltValue,CtrlState));
			AltValue&=0xFFFF;
			/*
			О перетаскивании из проводника / вставке текста в консоль, на примере буквы 'ы':

			1. Нажимается Alt:
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_MENU,    UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED

			2. Через numpad-клавиши вводится код символа в OEM, если он туда мапится, или 63 ('?'), если не мапится:
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD2, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD2, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD3, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD3, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD5, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD5, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED

			3. Отжимается Alt, при этом в uChar.UnicodeChar лежит исходный символ:
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_MENU,    UnicodeChar=1099, dwControlKeyState=0

			Мораль сей басни такова: если rec->Event.KeyEvent.uChar.UnicodeChar не пуст - берём его, а не то, что во время удерживания Alt пришло.
			*/

			if (rec->Event.KeyEvent.uChar.UnicodeChar)
			{
				// BUGBUG: в Windows 7 Event.KeyEvent.uChar.UnicodeChar _всегда_ заполнен, но далеко не всегда тем, чем надо.
				// условно считаем, что если интервал между нажатиями не превышает 50 мс, то это сгенерированная при D&D или вставке комбинация,
				// иначе - ручной ввод.
				if (GetTickCount()-Time<50)
				{
					AltValue=rec->Event.KeyEvent.uChar.UnicodeChar;
				}
			}
			else
			{
				rec->Event.KeyEvent.uChar.UnicodeChar=static_cast<WCHAR>(AltValue);
			}

			// _SVS(SysLog(L"KeyCode==VK_MENU -> AltValue=%X (%c)",AltValue,AltValue));
			return(AltValue);
		}
		else
			return(KEY_NONE);
	}

	//прежде, чем убирать это шаманство, поставьте себе раскладку, в которой по ralt+символ можно вводить символы.
	//например немецкую:
	//ralt+m - мю
	//ralt+q - @
	//ralt+e - евро
	//ralt+] - ~
	//ralt+2 - квадрат
	//ralt+3 - куб
	//ralt+7 - {
	//ralt+8 - [
	//ralt+9 - ]
	//ralt+0 - }
	//ralt+- - "\"
	//или латышскую:
	//ralt+4 - евро
	//ralt+a/ralt+shift+a
	//ralt+c/ralt+shift+c
	//и т.д.
	if ((CtrlState & 9)==9)
	{
		if (Char>=' ')
			return Char;
		else if (RealKey && ScanCode && !Char && (KeyCode && KeyCode != VK_MENU))
			//Это шаманство для ввода всяческих букв с тильдами, акцентами и прочим.
			//Напимер на Шведской раскладке, "AltGr+VK_OEM_1" вообще не должно обрабатываться фаром, т.к. это DeadKey
			//Dn, 1, Vk="VK_CONTROL" [17/0x0011], Scan=0x001D uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000008 (Casac - ecns)
			//Dn, 1, Vk="VK_MENU" [18/0x0012], Scan=0x0038 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000109 (CasAc - Ecns)
			//Dn, 1, Vk="VK_OEM_1" [186/0x00BA], Scan=0x001B uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000009 (CasAc - ecns)
			//Up, 1, Vk="VK_CONTROL" [17/0x0011], Scan=0x001D uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000001 (casAc - ecns)
			//Up, 1, Vk="VK_MENU" [18/0x0012], Scan=0x0038 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000100 (casac - Ecns)
			//Up, 1, Vk="VK_OEM_1" [186/0x00BA], Scan=0x0000 uChar=[U='~' (0x007E): A='~' (0x7E)] Ctrl=0x00000000 (casac - ecns)
			//Up, 1, Vk="VK_OEM_1" [186/0x00BA], Scan=0x001B uChar=[U='и' (0x00A8): A='' (0xA8)] Ctrl=0x00000000 (casac - ecns)
			//Dn, 1, Vk="VK_A" [65/0x0041], Scan=0x001E uChar=[U='у' (0x00E3): A='' (0xE3)] Ctrl=0x00000000 (casac - ecns)
			//Up, 1, Vk="VK_A" [65/0x0041], Scan=0x001E uChar=[U='a' (0x0061): A='a' (0x61)] Ctrl=0x00000000 (casac - ecns)
			return KEY_NONE;
		else
			IntKeyState.CtrlPressed=0;
	}

	if (KeyCode==VK_MENU)
		AltValue=0;

	if (Char && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed)
	{
		if (KeyCode==VK_OEM_3 && !Opt.UseVk_oem_x)
			return(IntKeyState.ShiftPressed ? '~':'`');

		if (KeyCode==VK_OEM_7 && !Opt.UseVk_oem_x)
			return(IntKeyState.ShiftPressed ? '"':'\'');
	}

	if (Char<L' ' && (IntKeyState.CtrlPressed || IntKeyState.AltPressed))
	{
		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				Char=L',';
				break;
			case VK_OEM_PERIOD:
				Char=L'.';
				break;
			case VK_OEM_4:

				if (!Opt.UseVk_oem_x)
					Char=L'[';

				break;
			case VK_OEM_5:

				//Char.AsciiChar=ScanCode==0x29?0x15:'\\'; //???
				if (!Opt.UseVk_oem_x)
					Char=L'\\';

				break;
			case VK_OEM_6:

				if (!Opt.UseVk_oem_x)
					Char=L']';

				break;
			case VK_OEM_7:

				if (!Opt.UseVk_oem_x)
					Char=L'\"';

				break;
		}
	}

	/* $ 24.08.2000 SVS
	   "Персональные 100 грамм" :-)
	*/
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_SHIFT:
			case VK_MENU:
			case VK_CONTROL:
			{
				if (IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
				{
					if ((Opt.CASRule&2))
						return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);
				}
				else if (Opt.CASRule&1)
					return (IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE);
			}
		}
	}

	if (IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_RSHIFT:
			case VK_RMENU:
			case VK_RCONTROL:

				if (Opt.CASRule&2)
					return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);

				break;
		}
	}

	if (KeyCode>=VK_F1 && KeyCode<=VK_F24)
//    return(Modif+KEY_F1+((KeyCode-VK_F1)<<8));
		return(Modif+KEY_F1+((KeyCode-VK_F1)));

	int NotShift=!IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed;

	if (IntKeyState.AltPressed && !IntKeyState.CtrlPressed && !IntKeyState.ShiftPressed)
	{
		if (!AltValue)
		{
			if (KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0)
			{
				if (CtrlObject && CtrlObject->Macro.IsRecording())
				{
					_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessEvent(KEY_INS|KEY_ALT)",__LINE__));
					struct FAR_INPUT_RECORD irec={KEY_INS|KEY_ALT,*rec};
					CtrlObject->Macro.ProcessEvent(&irec);
				}

				// макрос проигрывается и мы "сейчас" в состоянии выполнения функции waitkey? (Mantis#0000968: waitkey() пропускает AltIns)
				if (CtrlObject->Macro.IsExecuting() && CtrlObject->Macro.CheckWaitKeyFunc())
					return KEY_INS|KEY_ALT;

				RunGraber();
				return(KEY_NONE);
			}
		}

		// _SVS(SysLog(L"1 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
		if (!(CtrlState & ENHANCED_KEY)
		        //(CtrlState&NUMLOCK_ON) && KeyCode >= VK_NUMPAD0 && KeyCode <= VK_NUMPAD9 ||
		        // !(CtrlState&NUMLOCK_ON) && KeyCode < VK_NUMPAD0
		   )
		{
			// _SVS(SysLog(L"2 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
			static unsigned int ScanCodes[]={82,79,80,81,75,76,77,71,72,73};

			for (int I=0; I<int(ARRAYSIZE(ScanCodes)); I++)
			{
				if (ScanCodes[I]==ScanCode)
				{
					if (RealKey && (unsigned int)KeyCodeForALT_LastPressed != KeyCode)
					{
						AltValue=AltValue*10+I;
						KeyCodeForALT_LastPressed=KeyCode;
					}

//          _SVS(SysLog(L"AltNumPad -> AltValue=0x%0X CtrlState=%X",AltValue,CtrlState));
					if (AltValue)
						return(KEY_NONE);
				}
			}
		}
	}

	/*
	NumLock=Off
	  Down
	    CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
	    CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	  Num2
	    CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	    CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00

	  Ctrl-8
	    CtrlState=0008 KeyCode=0026 ScanCode=00000048 AsciiChar=00
	  Ctrl-Shift-8               ^^!!!
	    CtrlState=0018 KeyCode=0026 ScanCode=00000048 AsciiChar=00

	------------------------------------------------------------------------
	NumLock=On

	  Down
	    CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
	    CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	  Num2
	    CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32
	    CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32

	  Ctrl-8
	    CtrlState=0028 KeyCode=0068 ScanCode=00000048 AsciiChar=00
	  Ctrl-Shift-8               ^^!!!
	    CtrlState=0028 KeyCode=0026 ScanCode=00000048 AsciiChar=00
	*/

	/* ------------------------------------------------------------- */
	switch (KeyCode)
	{
		case VK_INSERT:
		case VK_NUMPAD0:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_INS);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD0)
				return '0';

			return Modif|KEY_NUMPAD0;
		case VK_DOWN:
		case VK_NUMPAD2:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_DOWN);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD2)
				return '2';

			return Modif|KEY_NUMPAD2;
		case VK_LEFT:
		case VK_NUMPAD4:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_LEFT);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD4)
				return '4';

			return Modif|KEY_NUMPAD4;
		case VK_RIGHT:
		case VK_NUMPAD6:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_RIGHT);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD6)
				return '6';

			return Modif|KEY_NUMPAD6;
		case VK_UP:
		case VK_NUMPAD8:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_UP);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD8)
				return '8';

			return Modif|KEY_NUMPAD8;
		case VK_END:
		case VK_NUMPAD1:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_END);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD1)
				return '1';

			return Modif|KEY_NUMPAD1;
		case VK_HOME:
		case VK_NUMPAD7:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_HOME);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD7)
				return '7';

			return Modif|KEY_NUMPAD7;
		case VK_NEXT:
		case VK_NUMPAD3:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_PGDN);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD3)
				return '3';

			return Modif|KEY_NUMPAD3;
		case VK_PRIOR:
		case VK_NUMPAD9:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_PGUP);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD9)
				return '9';

			return Modif|KEY_NUMPAD9;
		case VK_CLEAR:
		case VK_NUMPAD5:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_NUMPAD5);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD5)
				return '5';

			return Modif|KEY_NUMPAD5;
		case VK_DELETE:
		case VK_DECIMAL:

			if (CtrlState&ENHANCED_KEY)
			{
				return (Modif|KEY_DEL);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_DECIMAL)
				return KEY_DECIMAL;

			return Modif|KEY_NUMDEL;
	}

	switch (KeyCode)
	{
		case VK_RETURN:
			if (IntKeyState.ShiftPressed && RealKey && !ShiftPressedLast && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !LastShiftEnterPressed)
				return (CtrlState&ENHANCED_KEY)?KEY_NUMENTER:KEY_ENTER;

			LastShiftEnterPressed=Modif&KEY_SHIFT?TRUE:FALSE;
			return Modif|((CtrlState&ENHANCED_KEY)?KEY_NUMENTER:KEY_ENTER);
		case VK_BROWSER_BACK:
			return Modif|KEY_BROWSER_BACK;
		case VK_BROWSER_FORWARD:
			return Modif|KEY_BROWSER_FORWARD;
		case VK_BROWSER_REFRESH:
			return Modif|KEY_BROWSER_REFRESH;
		case VK_BROWSER_STOP:
			return Modif|KEY_BROWSER_STOP;
		case VK_BROWSER_SEARCH:
			return Modif|KEY_BROWSER_SEARCH;
		case VK_BROWSER_FAVORITES:
			return Modif|KEY_BROWSER_FAVORITES;
		case VK_BROWSER_HOME:
			return Modif|KEY_BROWSER_HOME;
		case VK_VOLUME_MUTE:
			return Modif|KEY_VOLUME_MUTE;
		case VK_VOLUME_DOWN:
			return Modif|KEY_VOLUME_DOWN;
		case VK_VOLUME_UP:
			return Modif|KEY_VOLUME_UP;
		case VK_MEDIA_NEXT_TRACK:
			return Modif|KEY_MEDIA_NEXT_TRACK;
		case VK_MEDIA_PREV_TRACK:
			return Modif|KEY_MEDIA_PREV_TRACK;
		case VK_MEDIA_STOP:
			return Modif|KEY_MEDIA_STOP;
		case VK_MEDIA_PLAY_PAUSE:
			return Modif|KEY_MEDIA_PLAY_PAUSE;
		case VK_LAUNCH_MAIL:
			return Modif|KEY_LAUNCH_MAIL;
		case VK_LAUNCH_MEDIA_SELECT:
			return Modif|KEY_LAUNCH_MEDIA_SELECT;
		case VK_LAUNCH_APP1:
			return Modif|KEY_LAUNCH_APP1;
		case VK_LAUNCH_APP2:
			return Modif|KEY_LAUNCH_APP2;
		case VK_APPS:
			return(Modif|KEY_APPS);
		case VK_LWIN:
			return(Modif|KEY_LWIN);
		case VK_RWIN:
			return(Modif|KEY_RWIN);
		case VK_BACK:
			return(Modif|KEY_BS);
		case VK_SPACE:
			if (Char == L' ' || !Char)
				return(Modif|KEY_SPACE);
			return Char;
		case VK_TAB:
			return(Modif|KEY_TAB);
		case VK_ADD:
			return(Modif|KEY_ADD);
		case VK_SUBTRACT:
			return(Modif|KEY_SUBTRACT);
		case VK_ESCAPE:
			return(Modif|KEY_ESC);
	}

	switch (KeyCode)
	{
		case VK_CAPITAL:
			return(Modif|KEY_CAPSLOCK);
		case VK_NUMLOCK:
			return(Modif|KEY_NUMLOCK);
		case VK_SCROLL:
			return(Modif|KEY_SCROLLLOCK);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog(L"CtrlAltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='A' && KeyCode<='Z')
			return Modif|KeyCode;

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
			case VK_CANCEL:
				return(Modif|KEY_BREAK);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
			return(KEY_NONE);

		if (KeyCode)
			return(Modif|KeyCode);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog(L"CtrlAlt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='A' && KeyCode<='Z')
			return Modif|KeyCode;

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
				// KEY_EVENT_RECORD: Dn, 1, Vk="VK_CANCEL" [3/0x0003], Scan=0x0046 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x0000014A (CAsac - EcnS)
			case VK_PAUSE:
				return Modif|KEY_PAUSE;
			case VK_CANCEL:
				return Modif|KEY_BREAK;
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
			return(KEY_NONE);

		if (KeyCode)
			return Modif|KeyCode;
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_MENU && KeyCode!=VK_SHIFT) SysLog(L"AltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
		{
			string strKey;
			if (WaitInFastFind > 0 &&
			        CtrlObject->Macro.GetCurRecord(nullptr,nullptr) < MACROMODE_RECORDING &&
			        CtrlObject->Macro.GetIndex(KEY_ALTSHIFT0+KeyCode-'0',strKey,-1) == -1)
			{
				return Modif|Char;
			}
			else
			{
				return Modif|KeyCode;
			}
		}

		if (!WaitInMainLoop && KeyCode>='A' && KeyCode<='Z')
			return Modif|KeyCode;

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'_');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
			case VK_PAUSE:
				return(Modif|KEY_PAUSE);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
			return(KEY_NONE);

		if (KeyCode)
			return Modif|KeyCode;
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_SHIFT) SysLog(L"CtrlShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
			return(Modif|KeyCode);

		if (KeyCode>='A' && KeyCode<='Z')
			return(Modif|KeyCode);

		switch (KeyCode)
		{
			case VK_OEM_PERIOD:
				return(Modif|KEY_DOT);
			case VK_OEM_4:
				return(Modif|KEY_BRACKET);
			case VK_OEM_6:
				return(Modif|KEY_BACKBRACKET);
			case VK_OEM_2:
				return(Modif|KEY_SLASH);
			case VK_OEM_5:
				return(Modif|KEY_BACKSLASH);
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return(Modif+KEY_BACKSLASH);
			}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
			return(KEY_NONE);

		if (KeyCode)
			return Modif|KeyCode;
	}

	/* ------------------------------------------------------------- */
	if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
	{
		if (KeyCode>='0' && KeyCode<='9')
			return(KEY_RCTRL0+KeyCode-'0');
	}

	/* ------------------------------------------------------------- */
	if (!IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(KEY_DIVIDE);
			case VK_CANCEL:
				CtrlObject->Macro.SendDropProcess();
				return(KEY_BREAK);
			case VK_MULTIPLY:
				return(KEY_MULTIPLY);
			case VK_PAUSE:
				return(KEY_PAUSE);
			case VK_SLEEP:
				return KEY_STANDBY;
			case VK_SNAPSHOT:
				return KEY_PRNTSCRN;
		}
	}
	else if (KeyCode == VK_CANCEL && IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed)
	{
		CtrlObject->Macro.SendDropProcess();
		return(ModifCtrl|KEY_BREAK);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL) SysLog(L"Ctrl -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
			return ModifCtrl|KeyCode;

		if (KeyCode>='A' && KeyCode<='Z')
			return ModifCtrl|KeyCode;

		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				return(ModifCtrl|KEY_COMMA);
			case VK_OEM_PERIOD:
				return(ModifCtrl|KEY_DOT);
			case VK_OEM_2:
				return(ModifCtrl|KEY_SLASH);
			case VK_OEM_4:
				return(ModifCtrl|KEY_BRACKET);
			case VK_OEM_5:
				return(ModifCtrl|KEY_BACKSLASH);
			case VK_OEM_6:
				return(ModifCtrl|KEY_BACKBRACKET);
			case VK_OEM_7:
				return(ModifCtrl+'\''); // KEY_QUOTE
			case VK_MULTIPLY:
				return(ModifCtrl|KEY_MULTIPLY);
			case VK_DIVIDE:
				return(ModifCtrl|KEY_DIVIDE);
			case VK_PAUSE:
				if (CtrlState&ENHANCED_KEY)
					return ModifCtrl|KEY_NUMLOCK;
				CtrlObject->Macro.SendDropProcess();
				return KEY_BREAK;
			case VK_SLEEP:
				return ModifCtrl|KEY_STANDBY;
			case VK_SNAPSHOT:
				return ModifCtrl|KEY_PRNTSCRN;
			case VK_OEM_102: // <> \|
 				return ModifCtrl|KEY_BACKSLASH;
		}

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(ModifCtrl+'`');
				case VK_OEM_MINUS:
					return(ModifCtrl+'-');
				case VK_OEM_PLUS:
					return(ModifCtrl+'=');
				case VK_OEM_1:
					return(ModifCtrl+KEY_SEMICOLON);
			}

		if (KeyCode)
		{
			if (ProcessCtrlCode)
			{
				if (KeyCode == VK_CONTROL)
					return (IntKeyState.CtrlPressed && !IntKeyState.RightCtrlPressed)?KEY_CTRL:(IntKeyState.RightCtrlPressed?KEY_RCTRL:KEY_CTRL);
				else if (KeyCode == VK_RCONTROL)
					return KEY_RCTRL;
			}

			if (!RealKey && KeyCode==VK_CONTROL)
				return KEY_NONE;

			return ModifCtrl|KeyCode;
		}

		if (Char)
			return ModifCtrl|Char;
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.AltPressed)
	{

		_SVS(if (KeyCode!=VK_MENU) SysLog(L"Alt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(ModifAlt+'`');
				case VK_OEM_MINUS:
					return(ModifAlt+'-');
				case VK_OEM_PLUS:
					return(ModifAlt+'=');
				case VK_OEM_5:
					return(ModifAlt+KEY_BACKSLASH);
				case VK_OEM_6:
					return(ModifAlt+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(ModifAlt+KEY_BRACKET);
				case VK_OEM_7:
					return(ModifAlt+'\'');
				case VK_OEM_1:
					return(ModifAlt+KEY_SEMICOLON);
				case VK_OEM_2:
					return(ModifAlt+KEY_SLASH);
				case VK_OEM_102: // <> \|
 					return ModifAlt+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				return(ModifAlt|KEY_COMMA);
			case VK_OEM_PERIOD:
				return(ModifAlt|KEY_DOT);
			case VK_DIVIDE:
				return(ModifAlt|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(ModifAlt|KEY_MULTIPLY);
			case VK_PAUSE:
				return(ModifAlt+KEY_PAUSE);
			case VK_SLEEP:
				return ModifAlt|KEY_STANDBY;
			case VK_SNAPSHOT:
				return ModifAlt|KEY_PRNTSCRN;
		}

		if (Char)
		{
			if (!Opt.ShiftsKeyRules || WaitInFastFind > 0)
				return ModifAlt|Upper(Char);
			else if (WaitInMainLoop)
				return ModifAlt|Char;
		}

		if (ProcessCtrlCode)
		{
			if (KeyCode == VK_MENU)
				return (IntKeyState.AltPressed && !IntKeyState.RightAltPressed)?KEY_ALT:(IntKeyState.RightAltPressed?KEY_RALT:KEY_ALT);
			else if (KeyCode == VK_RMENU)
				return KEY_RALT;
		}

		if (!RealKey && KeyCode==VK_MENU)
			return KEY_NONE;

		if (KeyCode)
			return ModifAlt|KeyCode;
	}

	if (!IntKeyState.CtrlPressed && !IntKeyState.AltPressed && (KeyCode >= VK_OEM_1 && KeyCode <= VK_OEM_8) && !Char)
	{
		//Это шаманство для того, чтобы фар не реагировал на DeadKeys (могут быть нажаты с Shift-ом)
		//которые используются для ввода символов с диакритикой (тильды, шапки, и пр.)
		//Dn, Vk="VK_SHIFT"    [ 16/0x0010], Scan=0x002A uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x10
		//Dn, Vk="VK_OEM_PLUS" [187/0x00BB], Scan=0x000D uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x10
		//Up, Vk="VK_OEM_PLUS" [187/0x00BB], Scan=0x0000 uChar=[U=''  (0x02C7): A='?' (0xC7)] Ctrl=0x10
		//Up, Vk="VK_OEM_PLUS" [187/0x00BB], Scan=0x000D uChar=[U=''  (0x02C7): A='?' (0xC7)] Ctrl=0x10
		//Up, Vk="VK_SHIFT"    [ 16/0x0010], Scan=0x002A uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00
		//Dn, Vk="VK_C"        [ 67/0x0043], Scan=0x002E uChar=[U=''  (0x010D): A=' ' (0x0D)] Ctrl=0x00
		//Up, Vk="VK_C"        [ 67/0x0043], Scan=0x002E uChar=[U='c' (0x0063): A='c' (0x63)] Ctrl=0x00
		return KEY_NONE;
	}

	if (IntKeyState.ShiftPressed)
	{
		_SVS(if (KeyCode!=VK_SHIFT) SysLog(L"Shift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
		switch (KeyCode)
		{

			case VK_OEM_MINUS:
				return (Char>=' ')?Char:KEY_SHIFT|'-';
			case VK_OEM_PLUS:
				return (Char>=' ')?Char:KEY_SHIFT|'+';
			case VK_DIVIDE:
				return KEY_SHIFT|KEY_DIVIDE;
			case VK_MULTIPLY:
				return KEY_SHIFT|KEY_MULTIPLY;
			case VK_PAUSE:
				return KEY_SHIFT|KEY_PAUSE;
			case VK_SLEEP:
				return KEY_SHIFT|KEY_STANDBY;
			case VK_SNAPSHOT:
				return KEY_SHIFT|KEY_PRNTSCRN;
		}

		if (ProcessCtrlCode)
		{
			if (KeyCode == VK_SHIFT)
				return KEY_SHIFT;
			else if (KeyCode == VK_RSHIFT)
				return KEY_RSHIFT;
		}
	}

	if (Char && (ModifAlt || ModifCtrl))
		return Modif|Char;
	return Char?Char:KEY_NONE;
}
