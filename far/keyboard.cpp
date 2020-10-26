/*
keyboard.cpp

Функции, имеющие отношение к клавиатуре
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "keyboard.hpp"

// Internal:
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "console.hpp"
#include "plugins.hpp"
#include "notification.hpp"
#include "lang.hpp"
#include "datetime.hpp"
#include "string_utils.hpp"
#include "global.hpp"

// Platform:
#include "platform.reg.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* start Глобальные переменные */

FarKeyboardState IntKeyState={};

/* end Глобальные переменные */

static std::array<short, WCHAR_MAX + 1> KeyToVKey;
static std::array<wchar_t, 512> VKeyToASCII;

static unsigned int AltValue=0;
static unsigned int KeyCodeForALT_LastPressed=0;

static MOUSE_EVENT_RECORD lastMOUSE_EVENT_RECORD;

enum MODIF_PRESSED_LAST
{
	MODIF_SHIFT = 0_bit,
	MODIF_ALT   = 1_bit,
	MODIF_RALT  = 2_bit,
	MODIF_CTRL  = 3_bit,
	MODIF_RCTRL = 4_bit,
};
static TBitFlags<size_t> PressedLast;

static std::chrono::steady_clock::time_point KeyPressedLastTime;

static struct window_state
{
	struct state
	{
		bool m_IsMaximized;
		bool m_IsMinimized;

		bool is_restored() const
		{
			return !m_IsMaximized && !m_IsMinimized;
		}
	}
	m_State, m_PrevState;

	void update()
	{
		m_PrevState = std::exchange(m_State, { IsZoomed(console.GetWindow()) != FALSE, IsIconic(console.GetWindow()) != FALSE });

		if ((m_PrevState.m_IsMaximized && m_State.is_restored()) || (m_PrevState.is_restored() && m_State.m_IsMaximized))
		{
			ChangeVideoMode(m_State.m_IsMaximized);
		}
	}

	bool is_restored() const
	{
		return m_State.is_restored();
	}
}
WindowState;

/* ----------------------------------------------------------------- */
struct TFKey
{
	DWORD Key;
	lng LocalizedNameId;
	string_view Name;

	bool operator ==(DWORD rhsKey) const {return Key == rhsKey;}
};

static const TFKey FKeys1[]
{
	{ KEY_COMMA,                    lng::MKeyComma,                  L","sv,                      },
	{ KEY_DOT,                      lng::MKeyDot,                    L"."sv,                      },
	{ KEY_SLASH,                    lng::MKeySlash,                  L"/"sv,                      },
	{ KEY_SEMICOLON,                lng::MKeySemicolon,              L";"sv,                      },
	{ KEY_COLON,                    lng::MKeyColon,                  L":"sv,                      },
	{ KEY_BRACKET,                  lng::MKeyBracket,                L"["sv,                      },
	{ KEY_QUOTE,                    lng::MKeyQuote,                  L"\""sv,                     },
	{ KEY_BACKBRACKET,              lng::MKeyBackbracket,            L"]"sv,                      },
	{ KEY_BS,                       lng::MKeyBs,                     L"BS"sv,                     },
	{ KEY_F1,                       lng::MKeyF1,                     L"F1"sv,                     },
	{ KEY_F2,                       lng::MKeyF2,                     L"F2"sv,                     },
	{ KEY_F3,                       lng::MKeyF3,                     L"F3"sv,                     },
	{ KEY_F4,                       lng::MKeyF4,                     L"F4"sv,                     },
	{ KEY_F5,                       lng::MKeyF5,                     L"F5"sv,                     },
	{ KEY_F6,                       lng::MKeyF6,                     L"F6"sv,                     },
	{ KEY_F7,                       lng::MKeyF7,                     L"F7"sv,                     },
	{ KEY_F8,                       lng::MKeyF8,                     L"F8"sv,                     },
	{ KEY_F9,                       lng::MKeyF9,                     L"F9"sv,                     },
	{ KEY_UP,                       lng::MKeyUp,                     L"Up"sv,                     },
	{ KEY_ADD,                      lng::MKeyAdd,                    L"Add"sv,                    },
	{ KEY_DEL,                      lng::MKeyDel,                    L"Del"sv,                    },
	{ KEY_END,                      lng::MKeyEnd,                    L"End"sv,                    },
	{ KEY_ESC,                      lng::MKeyEsc,                    L"Esc"sv,                    },
	{ KEY_F10,                      lng::MKeyF10,                    L"F10"sv,                    },
	{ KEY_F11,                      lng::MKeyF11,                    L"F11"sv,                    },
	{ KEY_F12,                      lng::MKeyF12,                    L"F12"sv,                    },
	{ KEY_F13,                      lng::MKeyF13,                    L"F13"sv,                    },
	{ KEY_F14,                      lng::MKeyF14,                    L"F14"sv,                    },
	{ KEY_F15,                      lng::MKeyF15,                    L"F15"sv,                    },
	{ KEY_F16,                      lng::MKeyF16,                    L"F16"sv,                    },
	{ KEY_F17,                      lng::MKeyF17,                    L"F17"sv,                    },
	{ KEY_F18,                      lng::MKeyF18,                    L"F18"sv,                    },
	{ KEY_F19,                      lng::MKeyF19,                    L"F19"sv,                    },
	{ KEY_F20,                      lng::MKeyF20,                    L"F20"sv,                    },
	{ KEY_F21,                      lng::MKeyF21,                    L"F21"sv,                    },
	{ KEY_F22,                      lng::MKeyF22,                    L"F22"sv,                    },
	{ KEY_F23,                      lng::MKeyF23,                    L"F23"sv,                    },
	{ KEY_F24,                      lng::MKeyF24,                    L"F24"sv,                    },
	{ KEY_INS,                      lng::MKeyIns,                    L"Ins"sv,                    },
	{ KEY_TAB,                      lng::MKeyTab,                    L"Tab"sv,                    },
	{ KEY_LWIN,                     lng::MKeyLwin,                   L"LWin"sv,                   },
	{ KEY_NUMPAD0,                  lng::MKeyNumpad0,                L"Num0"sv,                   },
	{ KEY_NUMPAD1,                  lng::MKeyNumpad1,                L"Num1"sv,                   },
	{ KEY_NUMPAD2,                  lng::MKeyNumpad2,                L"Num2"sv,                   },
	{ KEY_NUMPAD3,                  lng::MKeyNumpad3,                L"Num3"sv,                   },
	{ KEY_NUMPAD4,                  lng::MKeyNumpad4,                L"Num4"sv,                   },
	{ KEY_CLEAR,                    lng::MKeyClear,                  L"Clear"sv,                  }, // KEY_CLEAR == KEY_NUMPAD5. We use "Clear", KeyNameToKey supports both
	{ KEY_NUMPAD5,                  lng::MKeyNumpad5,                L"Num5"sv,                   },
	{ KEY_NUMPAD6,                  lng::MKeyNumpad6,                L"Num6"sv,                   },
	{ KEY_NUMPAD7,                  lng::MKeyNumpad7,                L"Num7"sv,                   },
	{ KEY_NUMPAD8,                  lng::MKeyNumpad8,                L"Num8"sv,                   },
	{ KEY_NUMPAD9,                  lng::MKeyNumpad9,                L"Num9"sv,                   },
	{ KEY_RWIN,                     lng::MKeyRwin,                   L"RWin"sv,                   },
	{ KEY_APPS,                     lng::MKeyApps,                   L"Apps"sv,                   },
	{ KEY_DOWN,                     lng::MKeyDown,                   L"Down"sv,                   },
	{ KEY_HOME,                     lng::MKeyHome,                   L"Home"sv,                   },
	{ KEY_LEFT,                     lng::MKeyLeft,                   L"Left"sv,                   },
	{ KEY_PGDN,                     lng::MKeyPgdn,                   L"PgDn"sv,                   },
	{ KEY_PGUP,                     lng::MKeyPgup,                   L"PgUp"sv,                   },
	{ KEY_BREAK,                    lng::MKeyBreak,                  L"Break"sv,                  },
	{ KEY_ENTER,                    lng::MKeyEnter,                  L"Enter"sv,                  },
	{ KEY_PAUSE,                    lng::MKeyPause,                  L"Pause"sv,                  },
	{ KEY_RIGHT,                    lng::MKeyRight,                  L"Right"sv,                  },
	{ KEY_SPACE,                    lng::MKeySpace,                  L"Space"sv,                  },
	{ KEY_NUMDEL,                   lng::MKeyNumdel,                 L"NumDel"sv,                 },
	{ KEY_DIVIDE,                   lng::MKeyDivide,                 L"Divide"sv,                 },
	{ KEY_STANDBY,                  lng::MKeyStandby,                L"Standby"sv,                },
	{ KEY_DECIMAL,                  lng::MKeyDecimal,                L"Decimal"sv,                },
	{ KEY_NUMLOCK,                  lng::MKeyNumlock,                L"NumLock"sv,                },
	{ KEY_PRNTSCRN,                 lng::MKeyPrntscrn,               L"PrntScrn"sv,               },
	{ KEY_CAPSLOCK,                 lng::MKeyCapslock,               L"CapsLock"sv,               },
	{ KEY_MULTIPLY,                 lng::MKeyMultiply,               L"Multiply"sv,               },
	{ KEY_NUMENTER,                 lng::MKeyNumenter,               L"NumEnter"sv,               },
	{ KEY_SUBTRACT,                 lng::MKeySubtract,               L"Subtract"sv,               },
	{ KEY_VOLUME_UP,                lng::MKeyVolumeUp,               L"VolumeUp"sv,               },
	{ KEY_MSRCLICK,                 lng::MKeyMsrclick,               L"MsRClick"sv,               },
	{ KEY_MSLCLICK,                 lng::MKeyMslclick,               L"MsLClick"sv,               },
	{ KEY_MSM1CLICK,                lng::MKeyMsm1click,              L"MsM1Click"sv,              },
	{ KEY_MSM2CLICK,                lng::MKeyMsm2click,              L"MsM2Click"sv,              },
	{ KEY_MSM3CLICK,                lng::MKeyMsm3click,              L"MsM3Click"sv,              },
	{ KEY_BACKSLASH,                lng::MKeyBackslash,              L"BackSlash"sv,              },
	{ KEY_MEDIA_STOP,               lng::MKeyMediaStop,              L"MediaStop"sv,              },
	{ KEY_MSWHEEL_UP,               lng::MKeyMswheelUp,              L"MsWheelUp"sv,              },
	{ KEY_LAUNCH_APP1,              lng::MKeyLaunchApp1,             L"LaunchApp1"sv,             },
	{ KEY_LAUNCH_APP2,              lng::MKeyLaunchApp2,             L"LaunchApp2"sv,             },
	{ KEY_LAUNCH_MAIL,              lng::MKeyLaunchMail,             L"LaunchMail"sv,             },
	{ KEY_SCROLLLOCK,               lng::MKeyScrolllock,             L"ScrollLock"sv,             },
	{ KEY_VOLUME_DOWN,              lng::MKeyVolumeDown,             L"VolumeDown"sv,             },
	{ KEY_VOLUME_MUTE,              lng::MKeyVolumeMute,             L"VolumeMute"sv,             },
	{ KEY_BROWSER_BACK,             lng::MKeyBrowserBack,            L"BrowserBack"sv,            },
	{ KEY_BROWSER_HOME,             lng::MKeyBrowserHome,            L"BrowserHome"sv,            },
	{ KEY_BROWSER_STOP,             lng::MKeyBrowserStop,            L"BrowserStop"sv,            },
	{ KEY_MSWHEEL_LEFT,             lng::MKeyMswheelLeft,            L"MsWheelLeft"sv,            },
	{ KEY_MSWHEEL_DOWN,             lng::MKeyMswheelDown,            L"MsWheelDown"sv,            },
	{ KEY_MSWHEEL_RIGHT,            lng::MKeyMswheelRight,           L"MsWheelRight"sv,           },
	{ KEY_BROWSER_SEARCH,           lng::MKeyBrowserSearch,          L"BrowserSearch"sv,          },
	{ KEY_BROWSER_FORWARD,          lng::MKeyBrowserForward,         L"BrowserForward"sv,         },
	{ KEY_BROWSER_REFRESH,          lng::MKeyBrowserRefresh,         L"BrowserRefresh"sv,         },
	{ KEY_MEDIA_NEXT_TRACK,         lng::MKeyMediaNextTrack,         L"MediaNextTrack"sv,         },
	{ KEY_MEDIA_PLAY_PAUSE,         lng::MKeyMediaPlayPause,         L"MediaPlayPause"sv,         },
	{ KEY_MEDIA_PREV_TRACK,         lng::MKeyMediaPrevTrack,         L"MediaPrevTrack"sv,         },
	{ KEY_BROWSER_FAVORITES,        lng::MKeyBrowserFavorites,       L"BrowserFavorites"sv,       },
	{ KEY_LAUNCH_MEDIA_SELECT,      lng::MKeyLaunchMediaSelect,      L"LaunchMediaSelect"sv,      },
};

static const TFKey ModifKeyName[]
{
	// The order matters - this is how we expect them in key names like CtrlAltWhatever
	// Don't change.
	{ KEY_CTRL,     lng::MKeyCtrl,   L"Ctrl"sv, },
	{ KEY_RCTRL,    lng::MKeyRCtrl,  L"RCtrl"sv, },
	{ KEY_ALT,      lng::MKeyAlt,    L"Alt"sv, },
	{ KEY_RALT,     lng::MKeyRAlt,   L"RAlt"sv, },
	{ KEY_SHIFT,    lng::MKeyShift,  L"Shift"sv, },
	{ KEY_M_SPEC,   lng(-1),         L"Spec"sv, },
	{ KEY_M_OEM,    lng(-1),         L"Oem"sv, },
};

#if defined(SYSLOG)
static const TFKey SpecKeyName[]
{
	{ KEY_IDLE,                       lng(-1), L"Idle"sv,                     },
	{ KEY_NONE,                       lng(-1), L"None"sv,                     },
	{ KEY_OP_XLAT,                    lng(-1), L"OP_Xlat"sv,                  },
	{ KEY_OP_PLAINTEXT,               lng(-1), L"OP_Text"sv,                  },
	{ KEY_DRAGMOVE,                   lng(-1), L"DragMove"sv,                 },
	{ KEY_DRAGCOPY,                   lng(-1), L"DragCopy"sv,                 },
	{ KEY_GOTFOCUS,                   lng(-1), L"GotFocus"sv,                 },
	{ KEY_KILLFOCUS,                  lng(-1), L"KillFocus"sv,                },
	{ KEY_OP_SELWORD,                 lng(-1), L"OP_SelWord"sv,               },
	{ KEY_CONSOLE_BUFFER_RESIZE,      lng(-1), L"ConsoleBufferResize"sv,      },
};
#endif

/* ----------------------------------------------------------------- */

static auto& Layout()
{
	static std::vector<HKL> s_Layout;
	return s_Layout;
}

/*
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
	if (const auto LayoutNumber = GetKeyboardLayoutList(0, nullptr))
	{
		Layout().resize(LayoutNumber);
		Layout().resize(GetKeyboardLayoutList(LayoutNumber, Layout().data())); // if less than expected
	}
	else // GetKeyboardLayoutList can return 0 in telnet mode
	{
		Layout().reserve(10);
		for (const auto& i: os::reg::enum_value(os::reg::key::current_user, L"Keyboard Layout\\Preload"sv))
		{
			if (i.type() == REG_SZ && std::iswdigit(i.name().front()))
			{
				const auto Value = i.get_string();
				if (!Value.empty() && std::iswxdigit(Value.front()))
				{
					uintptr_t KbLayout;
					if (from_string(Value, KbLayout, nullptr, 16) && KbLayout)
					{
						if (KbLayout <= 0xffff)
							KbLayout |= KbLayout << 16;
						Layout().emplace_back(reinterpret_cast<HKL>(KbLayout));
					}
					else
					{
						// TODO: log
					}
				}
			}
		}
	}

	KeyToVKey.fill(0);
	VKeyToASCII.fill(0);

	if (!Layout().empty())
	{
		BYTE KeyState[0x100]={};
		//KeyToVKey - используется чтоб проверить если два символа это одна и та же кнопка на клаве
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

			for (const auto& i: Layout())
			{
				for (int VK = 0; VK != 256; ++VK)
				{
					wchar_t idx;
					if (ToUnicodeEx(VK, 0, KeyState, &idx, 1, 0, i) > 0)
					{
						if (!KeyToVKey[idx])
							KeyToVKey[idx] = VK + j * 0x100;
					}
				}
			}
		}

		//VKeyToASCII - используется вместе с KeyToVKey чтоб подменить нац. символ на US-ASCII
		//***********
		//Имея мапирование юникод -> VK строим обратное мапирование
		//VK -> символы с кодом меньше 0x80, т.е. только US-ASCII символы
		for (wchar_t i = 1; i < 0x80; i++)
		{
			const auto x = KeyToVKey[i];

			if (x && !VKeyToASCII[x])
				VKeyToASCII[x]=upper(i);
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

	return Key && Key == CompareKey;
}

//Должно вернуть клавишный Eng эквивалент Key
int KeyToKeyLayout(int Key)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyToKeyLayout()"));
	_KEYMACRO(SysLog(L"Param: Key=%08X",Key));
	const auto VK = KeyToVKey[Key&0xFFFF];

	if (VK && VKeyToASCII[VK])
		return VKeyToASCII[VK];

	return Key;
}

/*
  State:
    -1 get state, 0 off, 1 on, 2 flip
*/
int SetFLockState(unsigned const vkKey, int const State)
{
	const auto ExKey = (vkKey == VK_CAPITAL? 0 : KEYEVENTF_EXTENDEDKEY);

	switch (vkKey)
	{
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_SCROLL:
			break;
		default:
			return -1;
	}

	const auto oldState = GetKeyState(vkKey);

	if (State >= 0)
	{
		if (State == 2 || (State==1 && !oldState) || (!State && oldState))
		{
			keybd_event(vkKey, 0, ExKey, 0);
			keybd_event(vkKey, 0, ExKey | KEYEVENTF_KEYUP, 0);
		}
	}

	return oldState;
}

unsigned int InputRecordToKey(const INPUT_RECORD* Rec)
{
	if (Rec)
	{
		auto RecCopy = *Rec; // НАДО!, т.к. внутри CalcKeyCode
		//   структура INPUT_RECORD модифицируется!

		return ShieldCalcKeyCode(&RecCopy, false);
	}

	return KEY_NONE;
}


bool KeyToInputRecord(int Key, INPUT_RECORD *Rec)
{
	return TranslateKeyToVK(Key, Rec) != 0;
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

	return IntKeyState.MouseButtonState;
}

static std::chrono::milliseconds keyboard_delay()
{
	DWORD RepeatDelay;
	if (!SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &RepeatDelay, 0))
		RepeatDelay = 1;

	// 0...3: 250...1000 ms
	return 250ms * (RepeatDelay + 1);
}

static std::chrono::steady_clock::duration keyboard_rate()
{
	DWORD RepeatSpeed;
	if (!SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &RepeatSpeed, 0))
		RepeatSpeed = 15;

	// 0...31: approximately 2.5...30 repetitions per second
	const auto RepetitionsPerSecond = 2.5 + (30.0 - 2.5) * RepeatSpeed / 31;
	return std::chrono::duration_cast<std::chrono::steady_clock::duration>(1s / RepetitionsPerSecond);
}

class keyboard_repeat_emulation::implementation
{
public:
	void reset() const
	{
		m_DelayCheck.reset();
		m_RepeatCheck.reset();
		m_Repeating = {};
	}

	bool signaled() const
	{
		if (!m_RepeatCheck)
			return false;

		if (m_Repeating && !m_DelayCheck.is_time())
			return false;

		m_Repeating = true;

		return true;
	}

private:
	time_check mutable
		m_DelayCheck{ time_check::mode::delayed, keyboard_delay() },
		m_RepeatCheck{ time_check::mode::immediate, keyboard_rate() };

	bool mutable m_Repeating{};
};

keyboard_repeat_emulation::keyboard_repeat_emulation() :
	m_Impl(std::make_unique<implementation>())
{
}

keyboard_repeat_emulation::~keyboard_repeat_emulation() = default;

void keyboard_repeat_emulation::reset() const
{
	return m_Impl->reset();
}

bool keyboard_repeat_emulation::signaled() const
{
	return m_Impl->signaled();
}

bool while_mouse_button_pressed(function_ref<bool(DWORD)> const Action)
{
	keyboard_repeat_emulation const Emulation;

	while (const auto Button = IsMouseButtonPressed())
	{
		if (!Emulation.signaled())
		{
			std::this_thread::yield();
			continue;
		}

		if (!Action(Button))
			return false;
	}

	return true;
}

static auto ButtonStateToKeyMsClick(DWORD ButtonState)
{
	switch (ButtonState)
	{
	case FROM_LEFT_1ST_BUTTON_PRESSED:
		return KEY_MSLCLICK;
	case RIGHTMOST_BUTTON_PRESSED:
		return KEY_MSRCLICK;
	case FROM_LEFT_2ND_BUTTON_PRESSED:
		return KEY_MSM1CLICK;
	case FROM_LEFT_3RD_BUTTON_PRESSED:
		return KEY_MSM2CLICK;
	case FROM_LEFT_4TH_BUTTON_PRESSED:
		return KEY_MSM3CLICK;
	default:
		return KEY_NONE;
	}
}

static auto KeyMsClickToButtonState(DWORD Key)
{
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
	default:
		return 0;
	}
}

static auto was_repeat = false;
static auto last_pressed_keycode = static_cast<WORD>(-1);

bool IsRepeatedKey()
{
	return was_repeat;
}

// "дополнительная" очередь кодов клавиш
static auto& KeyQueue()
{
	static std::deque<DWORD> s_KeyQueue;
	return s_KeyQueue;
}

void ClearKeyQueue()
{
	KeyQueue().clear();
}

DWORD GetInputRecordNoMacroArea(INPUT_RECORD *rec,bool AllowSynchro)
{
	const auto SavedArea = Global->CtrlObject->Macro.GetArea();
	SCOPE_EXIT{ Global->CtrlObject->Macro.SetArea(SavedArea); };

	Global->CtrlObject->Macro.SetArea(MACROAREA_LAST); // чтобы не срабатывали макросы :-)
	return GetInputRecord(rec, false, false, AllowSynchro);
}

static bool ProcessMacros(INPUT_RECORD* rec, DWORD& Result)
{
	if (!Global->CtrlObject || !Global->CtrlObject->Cp())
		return false;

	Global->CtrlObject->Macro.RunStartMacro();

	if (const auto MacroKey = Global->CtrlObject->Macro.GetKey())
	{
		static int LastMsClickMacroKey = 0;
		if (const auto MsClickKey = KeyMsClickToButtonState(MacroKey))
		{
			// Ахтунг! Для мышиной клавиши вернем значение MOUSE_EVENT, соответствующее _последнему_ событию мыши.
			rec->EventType = MOUSE_EVENT;
			rec->Event.MouseEvent = lastMOUSE_EVENT_RECORD;
			rec->Event.MouseEvent.dwButtonState = MsClickKey;
			rec->Event.MouseEvent.dwEventFlags = 0;
			LastMsClickMacroKey = MacroKey;
			Result = MacroKey;
			return true;
		}

		// если предыдущая клавиша мышиная - сбросим состояние панели Drag
		if (KeyMsClickToButtonState(LastMsClickMacroKey))
		{
			LastMsClickMacroKey = 0;
			Panel::EndDrag();
		}

		Global->ScrBuf->Flush();

		if (!TranslateKeyToVK(MacroKey, rec))
			return false;

		rec->EventType =
			in_closed_range(KEY_MACRO_BASE, static_cast<far_key_code>(MacroKey), KEY_MACRO_ENDBASE) ||
			in_closed_range(KEY_OP_BASE, static_cast<far_key_code>(MacroKey), KEY_OP_ENDBASE) ||
			(MacroKey&~0xFF000000) >= KEY_END_FKEY?
			0 : KEY_EVENT;

		if (!(MacroKey&KEY_SHIFT))
			IntKeyState.LeftShiftPressed = IntKeyState.RightShiftPressed = false;

		Result = MacroKey;
		return true;
	}


	// BUGBUG should it be here?
	if (Global->WindowManager->HaveAnyMessage())
	{
		Global->WindowManager->PluginCommit();
	}
	return false;
}

static void DropConsoleInputEvent()
{
	INPUT_RECORD rec;
	console.ReadOneInput(rec);
}

static void UpdateIntKeyState(DWORD CtrlState)
{
	IntKeyState.LeftCtrlPressed = (CtrlState & LEFT_CTRL_PRESSED) != 0;
	IntKeyState.LeftAltPressed = (CtrlState & LEFT_ALT_PRESSED) != 0;
	IntKeyState.LeftShiftPressed = (CtrlState & SHIFT_PRESSED) != 0;
	IntKeyState.RightCtrlPressed = (CtrlState & RIGHT_CTRL_PRESSED) != 0;
	IntKeyState.RightAltPressed = (CtrlState & RIGHT_ALT_PRESSED) != 0;
	IntKeyState.RightShiftPressed = (CtrlState & SHIFT_PRESSED) != 0; // ???
}

static DWORD ProcessFocusEvent(bool Got)
{
	/* $ 28.04.2001 VVM
	+ Не только обработаем сами смену фокуса, но и передадим дальше */
	PressedLast.ClearAll();

	UpdateIntKeyState(0);

	IntKeyState.MouseButtonState = 0;

	const auto CalcKey = Got? KEY_GOTFOCUS : KEY_KILLFOCUS;

	if (!IsWindows10OrGreater())
	{
		//чтоб решить баг винды приводящий к появлению скролов и т.п. после потери фокуса
		CalcKey == KEY_GOTFOCUS? RestoreConsoleWindowRect() : SaveConsoleWindowRect();
	}
	return CalcKey;
}

static DWORD ProcessBufferSizeEvent(point const Size)
{
	if (WindowState.is_restored())
	{
		SaveNonMaximisedBufferSize(Size);
	}

	const auto PScrX = ScrX;
	const auto PScrY = ScrY;

	UpdateScreenSize();

	PrevScrX = PScrX;
	PrevScrY = PScrY;

	AdjustConsoleScreenBufferSize();
	console.ResetViewportPosition();

	if (Global->WindowManager)
	{
		// апдейтим панели (именно они сейчас!)
		SCOPED_ACTION(LockScreen);

		if (Global->GlobalSaveScrPtr)
			Global->GlobalSaveScrPtr->Discard();

		Global->WindowManager->ResizeAllWindows();
		Global->WindowManager->GetCurrentWindow()->Show();
		// _SVS(SysLog(L"PreRedrawFunc = %p",PreRedrawFunc));
		TPreRedrawFunc::instance()([](const PreRedrawItem& Item)
		{
			Item();
		});
	}

	return KEY_CONSOLE_BUFFER_RESIZE;
}

static const far_key_code WheelKeys[][2] =
{
	{ KEY_MSWHEEL_DOWN, KEY_MSWHEEL_UP },
	{ KEY_MSWHEEL_LEFT, KEY_MSWHEEL_RIGHT }
};

static bool ProcessMouseEvent(const MOUSE_EVENT_RECORD& MouseEvent, bool ExcludeMacro, bool ProcessMouse, DWORD& CalcKey)
{
	lastMOUSE_EVENT_RECORD = MouseEvent;
	IntKeyState.PreMouseEventFlags = std::exchange(IntKeyState.MouseEventFlags, MouseEvent.dwEventFlags);
	const auto CtrlState = MouseEvent.dwControlKeyState;
	KeyMacro::SetMacroConst(constMsCtrlState, CtrlState);
	KeyMacro::SetMacroConst(constMsEventFlags, IntKeyState.MouseEventFlags);
	KeyMacro::SetMacroConst(constMsLastCtrlState, CtrlState);

	UpdateIntKeyState(CtrlState);

	const auto BtnState = MouseEvent.dwButtonState;
	KeyMacro::SetMacroConst(constMsButton, MouseEvent.dwButtonState);

	if (IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		IntKeyState.PrevMouseButtonState = IntKeyState.MouseButtonState;
	}

	IntKeyState.MouseButtonState = BtnState;
	IntKeyState.MousePrevPos = IntKeyState.MousePos;
	IntKeyState.MousePos = MouseEvent.dwMousePosition;
	KeyMacro::SetMacroConst(constMsX, IntKeyState.MousePos.x);
	KeyMacro::SetMacroConst(constMsY, IntKeyState.MousePos.y);

	/* $ 26.04.2001 VVM
	+ Обработка колесика мышки. */

	const auto GetModifiers = [CtrlState]
	{
		return
			(CtrlState & SHIFT_PRESSED? KEY_SHIFT : NO_KEY) |
			(CtrlState & LEFT_CTRL_PRESSED? KEY_CTRL : NO_KEY) |
			(CtrlState & RIGHT_CTRL_PRESSED? KEY_RCTRL : NO_KEY) |
			(CtrlState & LEFT_ALT_PRESSED? KEY_ALT : NO_KEY) |
			(CtrlState & RIGHT_ALT_PRESSED? KEY_RALT : NO_KEY);
	};

	if (IntKeyState.MouseEventFlags == MOUSE_WHEELED || IntKeyState.MouseEventFlags == MOUSE_HWHEELED)
	{
		const auto& WheelKeysPair = WheelKeys[IntKeyState.MouseEventFlags == MOUSE_HWHEELED? 1 : 0];
		const auto Key = WheelKeysPair[static_cast<short>(HIWORD(MouseEvent.dwButtonState)) > 0? 1 : 0];
		CalcKey = Key | GetModifiers();
		return false;
	}

	if ((!ExcludeMacro || ProcessMouse) && Global->CtrlObject && (ProcessMouse || !(Global->CtrlObject->Macro.IsRecording() || Global->CtrlObject->Macro.IsExecuting())))
	{
		if (IntKeyState.MouseEventFlags != MOUSE_MOVED)
		{
			const auto MsCalcKey = ButtonStateToKeyMsClick(MouseEvent.dwButtonState);
			if (MsCalcKey != KEY_NONE)
			{
				CalcKey = MsCalcKey | GetModifiers();

				// для WaitKey()
				if (ProcessMouse)
				{
					return true;
				}
			}
		}
	}
	return false;
}

static DWORD GetInputRecordImpl(INPUT_RECORD *rec,bool ExcludeMacro,bool ProcessMouse,bool AllowSynchro)
{
	_KEYMACRO(CleverSysLog Clev(L"GetInputRecord()"));

	if (AllowSynchro)
		message_manager::instance().dispatch();

	DWORD CalcKey;

	if (!ExcludeMacro)
	{
		if (ProcessMacros(rec, CalcKey))
		{
			return CalcKey;
		}
	}

	auto NotMacros = false;

	const auto ProcessMacroEvent = [&]
	{
		if (NotMacros || ExcludeMacro)
			return CalcKey;

		_KEYMACRO(SysLog(L"[%d] CALL Global->CtrlObject->Macro.ProcessEvent(%s)", __LINE__, _FARKEY_ToName(CalcKey)));

		FAR_INPUT_RECORD irec = { CalcKey, *rec };
		if (!Global->CtrlObject || !Global->CtrlObject->Macro.ProcessEvent(&irec))
			return CalcKey;

		rec->EventType = 0;
		return static_cast<DWORD>(KEY_NONE);
	};

	if (!KeyQueue().empty())
	{
		CalcKey=KeyQueue().front();
		KeyQueue().pop_front();
		NotMacros = (CalcKey & 0x80000000) != 0;
		CalcKey &= ~0x80000000;
		return ProcessMacroEvent();
	}

	const auto EnableShowTime = Global->Opt->Clock && (Global->IsPanelsActive() || (Global->CtrlObject && Global->CtrlObject->Macro.GetArea() == MACROAREA_SEARCH));

	if (EnableShowTime)
		ShowTime();

	Global->ScrBuf->Flush();

	static auto LastEventIdle = false;

	if (!LastEventIdle)
		Global->StartIdleTime = std::chrono::steady_clock::now();

	LastEventIdle = false;

	auto FullscreenState = IsConsoleFullscreen();

	DWORD LoopCount = 0;
	for (;;)
	{
		WindowState.update();

		if (!(LoopCount & 15))
		{
			if(Global->CtrlObject)
			{
				SetFarConsoleMode();
			}

			const auto CurrentFullscreenState = IsConsoleFullscreen();
			if(CurrentFullscreenState && !FullscreenState)
			{
				ChangeVideoMode(25,80);
			}
			FullscreenState=CurrentFullscreenState;
		}

		if (console.PeekOneInput(*rec))
		{
			//check for flock
			if (rec->EventType==KEY_EVENT && !rec->Event.KeyEvent.wVirtualScanCode && (rec->Event.KeyEvent.wVirtualKeyCode==VK_NUMLOCK||rec->Event.KeyEvent.wVirtualKeyCode==VK_CAPITAL||rec->Event.KeyEvent.wVirtualKeyCode==VK_SCROLL))
			{
				DropConsoleInputEvent();
				was_repeat = false;
				last_pressed_keycode = static_cast<WORD>(-1);
				continue;
			}
			break;
		}

		Global->ScrBuf->Flush();
		os::chrono::sleep_for(10ms);

		static bool ExitInProcess = false;
		if (Global->CloseFAR && !ExitInProcess)
		{
			ExitInProcess = true;
			Global->WindowManager->ExitMainLoop(FALSE);
			return KEY_NONE;
		}

		if (!(LoopCount & 15))
		{
			const auto CurTime = std::chrono::steady_clock::now();

			if (EnableShowTime)
				ShowTime();

			if (Global->IsPanelsActive())
			{
				if (!(LoopCount & 63))
				{
					static bool UpdateReenter = false;

					if (!UpdateReenter && CurTime - KeyPressedLastTime > 300ms)
					{
						if (Global->WindowManager->IsPanelsActive())
						{
							UpdateReenter = true;
							Global->CtrlObject->Cp()->LeftPanel()->UpdateIfChanged(true);
							Global->CtrlObject->Cp()->RightPanel()->UpdateIfChanged(true);
							UpdateReenter = false;
						}
					}
				}
			}

			if (Global->Opt->ScreenSaver &&
				Global->Opt->ScreenSaverTime > 0 &&
				CurTime - Global->StartIdleTime > std::chrono::minutes(Global->Opt->ScreenSaverTime))
			{
				ScreenSaver();
			}

			if (!Global->IsPanelsActive() && LoopCount==64)
			{
				LastEventIdle = true;
				*rec = {};
				rec->EventType=KEY_EVENT;
				return KEY_IDLE;
			}
		}

		if (!(LoopCount & 3))
		{
			if (message_manager::instance().dispatch())
			{
				*rec = {};
				return KEY_NONE;
			}
		}

		LoopCount++;
	}


	const auto CurTime = std::chrono::steady_clock::now();

	if (rec->EventType==KEY_EVENT)
	{
		static bool bForceAltGr = false;

		if (!rec->Event.KeyEvent.bKeyDown)
		{
			was_repeat = false;
			last_pressed_keycode = static_cast<WORD>(-1);
		}
		else
		{
			was_repeat = (last_pressed_keycode == rec->Event.KeyEvent.wVirtualKeyCode);
			last_pressed_keycode = rec->Event.KeyEvent.wVirtualKeyCode;

			if (rec->Event.KeyEvent.wVirtualKeyCode == VK_MENU)
			{
				// Шаманство с AltGr (виртуальная клавиатура)
				bForceAltGr = (rec->Event.KeyEvent.wVirtualScanCode == 0)
					&& ((rec->Event.KeyEvent.dwControlKeyState & 0x1F) == (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED));
			}
		}

		if (bForceAltGr && (rec->Event.KeyEvent.dwControlKeyState & 0x1F) == (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED))
		{
			rec->Event.KeyEvent.dwControlKeyState &= ~LEFT_ALT_PRESSED;
			rec->Event.KeyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
		}

		const auto CtrlState = rec->Event.KeyEvent.dwControlKeyState;

		if (Global->CtrlObject && Global->CtrlObject->Macro.IsRecording())
		{
			static WORD PrevVKKeyCode=0; // NumLock+Cursor
			const auto PrevVKKeyCode2 = PrevVKKeyCode;
			PrevVKKeyCode=rec->Event.KeyEvent.wVirtualKeyCode;

			// Для Shift-Enter в диалоге назначения вылазил Shift после отпускания клавиш.
			//
			if (PrevVKKeyCode2==VK_RETURN && PrevVKKeyCode==VK_SHIFT  && !rec->Event.KeyEvent.bKeyDown)
			{
				DropConsoleInputEvent();
				return KEY_NONE;
			}
		}

		UpdateIntKeyState(CtrlState);

		KeyPressedLastTime = CurTime;
	}
	else
	{
		was_repeat = false;
		last_pressed_keycode = static_cast<WORD>(-1);
	}

	IntKeyState.ReturnAltValue = false;
	CalcKey=CalcKeyCode(rec, true, &NotMacros);

	if (IntKeyState.ReturnAltValue)
	{
		return ProcessMacroEvent();
	}

	console.ReadOneInput(*rec);

	if (EnableShowTime)
		ShowTime();

	if (rec->EventType == FOCUS_EVENT)
	{
		return ProcessFocusEvent(rec->Event.FocusEvent.bSetFocus != FALSE);
	}

	if (
		// Legitimate event, but skip if the size isn't changed to filter out Windows 10 rubbish (see https://github.com/Microsoft/console/issues/281)
		(rec->EventType == WINDOW_BUFFER_SIZE_EVENT && rec->Event.WindowBufferSizeEvent.dwSize.X && rec->Event.WindowBufferSizeEvent.dwSize.Y && IsConsoleSizeChanged()) ||
		// Fake event, generated internally
		(rec->EventType == WINDOW_BUFFER_SIZE_EVENT && !rec->Event.WindowBufferSizeEvent.dwSize.X && !rec->Event.WindowBufferSizeEvent.dwSize.Y) ||
		// Size changed
		IsConsoleSizeChanged()
	)
	{
		// Do not use rec->Event.WindowBufferSizeEvent.dwSize here - we need a 'virtual' size
		point Size;
		return console.GetSize(Size)? ProcessBufferSizeEvent(Size) : static_cast<DWORD>(KEY_CONSOLE_BUFFER_RESIZE);
	}

	if (rec->EventType==KEY_EVENT)
	{
		const auto CtrlState = rec->Event.KeyEvent.dwControlKeyState;
		const auto KeyCode = rec->Event.KeyEvent.wVirtualKeyCode;

		UpdateIntKeyState(CtrlState);

		KeyMacro::SetMacroConst(constMsLastCtrlState,CtrlState);

		// Для NumPad!
		IntKeyState.LeftShiftPressed = IntKeyState.RightShiftPressed =
			((CalcKey & (KEY_CTRL | KEY_SHIFT | KEY_ALT | KEY_RCTRL | KEY_RALT)) == KEY_SHIFT && (CalcKey & KEY_MASKF) >= KEY_NUMPAD0 && (CalcKey & KEY_MASKF) <= KEY_NUMPAD9)?
			true :
			(CtrlState & SHIFT_PRESSED) != 0;

		struct KeysData
		{
			size_t FarKey;
			size_t VkKey;
			size_t Modif;
			bool Enhanced;
		}
		const Keys[]
		{
			{ KEY_SHIFT,     VK_SHIFT,       MODIF_SHIFT,      false,    },
			{ KEY_ALT,       VK_MENU,        MODIF_ALT,        false,    },
			{ KEY_RALT,      VK_MENU,        MODIF_RALT,       true,     },
			{ KEY_CTRL,      VK_CONTROL,     MODIF_CTRL,       false,    },
			{ KEY_RCTRL,     VK_CONTROL,     MODIF_RCTRL,      true,     },
		};

		if (std::any_of(ALL_CONST_RANGE(Keys), [&CalcKey](const KeysData& A){ return CalcKey == A.FarKey && !PressedLast.Check(A.Modif); }))
			CalcKey = KEY_NONE;

		const size_t AllModif = KEY_CTRL | KEY_ALT | KEY_SHIFT | KEY_RCTRL | KEY_RALT;
		if ((CalcKey&AllModif) && !(CalcKey&~AllModif) && !PressedLast.Check(MODIF_SHIFT | MODIF_ALT | MODIF_RALT | MODIF_CTRL | MODIF_RCTRL)) CalcKey=KEY_NONE;
		PressedLast.ClearAll();

		if (rec->Event.KeyEvent.bKeyDown)
		{
			for (const auto& A: Keys)
			{
				if (KeyCode == A.VkKey && (!A.Enhanced || CtrlState & ENHANCED_KEY))
					PressedLast.Set(A.Modif);
			}
		}

		Panel::EndDrag();
	}

	if (rec->EventType==MOUSE_EVENT)
	{
		if (ProcessMouseEvent(rec->Event.MouseEvent, ExcludeMacro, ProcessMouse, CalcKey))
			return CalcKey;
	}

	return ProcessMacroEvent();
}

DWORD GetInputRecord(INPUT_RECORD *rec, bool ExcludeMacro, bool ProcessMouse, bool AllowSynchro)
{
	*rec = {};

	DWORD Key = GetInputRecordImpl(rec, ExcludeMacro, ProcessMouse, AllowSynchro);

	if (Key)
	{
		if (Global->CtrlObject)
		{
			ProcessConsoleInputInfo Info = { sizeof(Info), PCIF_NONE, *rec };

			switch (Global->CtrlObject->Plugins->ProcessConsoleInput(&Info))
			{
			case 1:
				Key = KEY_NONE;
				KeyToInputRecord(Key, rec);
				break;
			case 2:
				*rec = Info.Rec;
				Key = CalcKeyCode(rec, false);
				break;
			}
		}
	}
	return Key;
}

DWORD PeekInputRecord(INPUT_RECORD *rec,bool ExcludeMacro)
{
	*rec = {};

	DWORD Key;
	Global->ScrBuf->Flush();

	if (!KeyQueue().empty() && (Key = KeyQueue().front()) != 0)
	{
		if (!TranslateKeyToVK(Key, rec))
			return 0;
	}
	else if (!ExcludeMacro && (Key=Global->CtrlObject->Macro.PeekKey()) != 0)
	{
		if (!TranslateKeyToVK(Key, rec))
			return 0;
	}
	else
	{
		if (!console.PeekOneInput(*rec))
			return 0;
	}

	return CalcKeyCode(rec, true); // ShieldCalcKeyCode?
}

/* $ 24.08.2000 SVS
 + Параметр у функции WaitKey - возможность ожидать конкретную клавишу
     Если KeyWait = -1 - как и раньше
*/
DWORD WaitKey(DWORD KeyWait,DWORD delayMS,bool ExcludeMacro)
{
	const time_check TimeCheck(time_check::mode::delayed, std::chrono::milliseconds(delayMS));
	DWORD Key;

	for (;;)
	{
		INPUT_RECORD rec;
		Key=KEY_NONE;

		if (PeekInputRecord(&rec,ExcludeMacro))
		{
			Key=GetInputRecord(&rec,ExcludeMacro,true);
		}

		if (KeyWait == static_cast<DWORD>(-1))
		{
			if ((Key&~KEY_CTRLMASK) < KEY_END_FKEY || IsInternalKeyReal(Key&~KEY_CTRLMASK))
				break;
		}
		else if (Key == KeyWait)
			break;

		if (TimeCheck)
		{
			Key=KEY_NONE;
			break;
		}

		os::chrono::sleep_for(1ms);
	}

	return Key;
}

bool WriteInput(int Key)
{
	if (KeyQueue().size() > 1024)
		return false;

	KeyQueue().emplace_back(Key);
	return true;
}

bool CheckForEscSilent()
{
	if(Global->CloseFAR)
	{
		return true;
	}

	INPUT_RECORD rec;
	bool Processed = true;
	/* TODO: Здесь, в общем то - ХЗ, т.к.
	         по хорошему нужно проверять Global->CtrlObject->Macro.PeekKey() на ESC или BREAK
	         Но к чему это приведет - пока не могу дать ответ !!!
	*/

	// если в "макросе"...
	if (Global->CtrlObject->Macro.IsExecuting() && Global->WindowManager->GetCurrentWindow())
	{
		if (Global->CtrlObject->Macro.IsOutputDisabled())
			Processed = false;
	}

	if (Processed && PeekInputRecord(&rec))
	{
		switch (GetInputRecordNoMacroArea(&rec, false))
		{
		case KEY_ESC:
		case KEY_BREAK:
			return true;

		case KEY_ALTF9:
		case KEY_RALTF9:
			Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF9));
			break;

		default:
			break;
		}
	}

	if (!Processed && Global->CtrlObject->Macro.IsExecuting())
		Global->ScrBuf->Flush();

	return false;
}

bool ConfirmAbortOp()
{
	return (Global->Opt->Confirm.Esc && !Global->CloseFAR)? AbortMessage() : true;
}

/* $ 09.02.2001 IS
     Подтверждение нажатия Esc
*/
bool CheckForEsc()
{
	return CheckForEscSilent()? ConfirmAbortOp() : false;
}

using tfkey_to_text = string_view(const TFKey&);
using add_separator = void(string&);

static string GetShiftKeyName(DWORD Key, tfkey_to_text ToText, add_separator AddSeparator)
{
	string Result;
	for (const auto& i: ModifKeyName)
	{
		if (Key & i.Key)
		{
			AddSeparator(Result);
			append(Result, ToText(i));
		}
	}
	return Result;
}


/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается 0
   Может и криво, но правильно и коротко!

   Функция KeyNameToKey ждет строку по вот такой спецификации:

   1. Сочетания, определенные в структуре FKeys1[]
   2. Опциональные модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift) и 1 символ, например, AltD или CtrlC
   3. "Spec" и 5 десятичных цифр (с ведущими нулями)
   4. "Oem" и 5 десятичных цифр (с ведущими нулями)
   5. только модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift)
*/
int KeyNameToKey(string_view Name)
{
	if (Name.empty())
		return 0;

	DWORD Key=0;

	if (Name.size() > 1) // если не один символ
	{
		if (Name[0] == L'%')
		return 0;

		if (Name.find_first_of(L"()"sv) != string::npos) // встречаются '(' или ')', то это явно не клавиша!
			return 0;
	}

	// пройдемся по всем модификаторам
	for (;;)
	{
		bool Found = false;
		for (const auto&i : ModifKeyName)
		{
			if (!(Key & i.Key))
			{
				if (starts_with_icase(Name, i.Name))
				{
					Name.remove_prefix(i.Name.size());
					Key |= i.Key;
					Found = true;
				}
			}
		}
		if (!Found)
			break;
	}

	// если что-то осталось - преобразуем.
	if (!Name.empty())
	{
		// сначала - FKeys1 - Вариант (1)
		const auto ItemIterator = std::find_if(CONST_RANGE(FKeys1, i)
		{
			return equal_icase(Name, i.Name);
		});

		if (ItemIterator != std::cend(FKeys1))
		{
			Key |= ItemIterator->Key;
			Name.remove_prefix(ItemIterator->Name.size());
		}
		else // F-клавиш нет?
		{
			/*
				здесь только 5 оставшихся вариантов:
				2) Опциональные модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift) и 1 символ, например, AltD или CtrlC
				3) "Spec" и 5 десятичных цифр (с ведущими нулями)
				4) "Oem" и 5 десятичных цифр (с ведущими нулями)
				5) только модификаторы (Alt/RAlt/Ctrl/RCtrl/Shift)
			*/

			if (Name.size() == 1) // Вариант (2)
			{
				int Chr = Name.front();

				// если были модификаторы Alt/Ctrl, то преобразуем в "физическую клавишу" (независимо от языка)
				if (Key&(KEY_ALT|KEY_RCTRL|KEY_CTRL|KEY_RALT))
				{
					if (Chr > 0x7F)
						Chr=KeyToKeyLayout(Chr);

					Chr=upper(Chr);
				}

				Key|=Chr;

				if (Chr)
					Name.remove_prefix(1);
			}
			else if (
				Name.size() == 5 && any_of(Key, KEY_M_SPEC, KEY_M_OEM) &&
				std::all_of(ALL_CONST_RANGE(Name), std::iswdigit)
			) // Варианты (3) и (4)
			{
				const auto K = from_string<unsigned>(Name);

				if (Key == KEY_M_SPEC) // Вариант (3)
					Key = (Key & ~KEY_M_SPEC) | (K + KEY_VK_0xFF_BEGIN);
				else // Вариант (4)
					Key = (Key & ~KEY_M_OEM) | (K + KEY_FKEY_BEGIN);

				Name = {};
			}
			// Вариант (5). Уже "собран".
		}
	}

	return (!Key || !Name.empty())? 0: static_cast<int>(Key);
}

string InputRecordToText(const INPUT_RECORD *Rec)
{
	return KeyToText(InputRecordToKey(Rec));
}

static string KeyToTextImpl(unsigned int const Key0, tfkey_to_text ToText, add_separator AddSeparator)
{
	if (Key0 == static_cast<unsigned int>(-1))
		return {};

	const auto Key = static_cast<DWORD>(Key0);
	auto FKey = static_cast<DWORD>(Key0) & 0xFFFFFF;

	auto strKeyText = GetShiftKeyName(Key, ToText, AddSeparator);

	const auto FKeys1Iterator = std::find(ALL_CONST_RANGE(FKeys1), FKey);
	if (FKeys1Iterator != std::cend(FKeys1))
	{
		AddSeparator(strKeyText);
		append(strKeyText, ToText(*FKeys1Iterator));
	}
	else
	{
		if (FKey >= KEY_VK_0xFF_BEGIN && FKey <= KEY_VK_0xFF_END)
		{
			AddSeparator(strKeyText);
			format_to(strKeyText, FSTR(L"Spec{0:0>5}"), FKey - KEY_VK_0xFF_BEGIN);
		}
		else if (FKey > KEY_VK_0xFF_END && FKey <= KEY_END_FKEY)
		{
			AddSeparator(strKeyText);
			format_to(strKeyText, FSTR(L"Oem{0:0>5}"), FKey - KEY_FKEY_BEGIN);
		}
		else
		{
#if defined(SYSLOG)
			// Этот кусок кода нужен только для того, что "спецклавиши" логировались нормально
			const auto SpecKeyIterator = std::find(ALL_CONST_RANGE(SpecKeyName), FKey);
			if (SpecKeyIterator != std::cend(SpecKeyName))
			{
				AddSeparator(strKeyText);
				append(strKeyText, ToText(*SpecKeyIterator));
			}
			else
#endif
			{
				FKey=upper(static_cast<wchar_t>(Key & 0xFFFF));

				wchar_t KeyText;

				if (FKey >= L'A' && FKey <= L'Z')
				{
					if (Key&(KEY_RCTRL|KEY_CTRL|KEY_RALT|KEY_ALT)) // ??? а если есть другие модификаторы ???
						KeyText = static_cast<wchar_t>(FKey); // для клавиш с модификаторами подставляем "латиницу" в верхнем регистре
					else
						KeyText = static_cast<wchar_t>(Key & 0xFFFF);
				}
				else
					KeyText = static_cast<wchar_t>(Key & 0xFFFF);

				if (KeyText)
				{
					AddSeparator(strKeyText);
					strKeyText += KeyText;
				}
			}
		}
	}

	return strKeyText;
}

string KeyToText(unsigned int const Key)
{
	return KeyToTextImpl(Key,
		[](const TFKey& i) { return i.Name; },
		[](string&) {}
	);
}

string KeyToLocalizedText(unsigned int const Key)
{
	return KeyToTextImpl(Key,
		[](const TFKey& i)
		{
			if (i.LocalizedNameId != lng(-1))
			{
				const auto& Msg = msg(i.LocalizedNameId);
				if (!Msg.empty())
					return string_view(Msg);
			}
			return i.Name;
		},
		[](string& str)
		{
			if (!str.empty())
				str += L'+';
		}
	);
}

string KeysListToLocalizedText(span<unsigned int const> const Keys)
{
	return join(select(Keys, [](unsigned int const Key) { return KeyToLocalizedText(Key); }), L" "sv);
}

int TranslateKeyToVK(int Key, INPUT_RECORD* Rec)
{
	_KEYMACRO(CleverSysLog Clev(L"TranslateKeyToVK()"));
	_KEYMACRO(SysLog(L"Param: Key=%08X",Key));

	WORD EventType=KEY_EVENT;

	DWORD FKey  =Key&KEY_END_SKEY;
	DWORD FShift=Key&KEY_CTRLMASK;

	int VirtKey=0;

	bool KeyInTable = false;
	{
		static const std::pair<unsigned int, int> Table_KeyToVK[] =
		{
			{ KEY_BREAK, VK_CANCEL },
			{ KEY_BS, VK_BACK },
			{ KEY_TAB, VK_TAB },
			{ KEY_ENTER, VK_RETURN },
			{ KEY_NUMENTER, VK_RETURN }, //????
			{ KEY_ESC, VK_ESCAPE },
			{ KEY_SPACE, VK_SPACE },
			{ KEY_NUMPAD5, VK_CLEAR },
		};

		const auto ItemIterator = std::find_if(CONST_RANGE(Table_KeyToVK, i) { return static_cast<DWORD>(i.first) == FKey; });
		if (ItemIterator != std::cend(Table_KeyToVK))
		{
			VirtKey = ItemIterator->second;
			KeyInTable = true;
		}
	}

	if (!KeyInTable)
	{
		if ((FKey>=L'0' && FKey<=L'9') || (FKey>=L'A' && FKey<=L'Z'))
		{
			VirtKey=FKey;
			if (FKey >= L'A' && !(FShift & 0xFF000000))
				FShift |= KEY_SHIFT;
		}
		else if (FKey > KEY_FKEY_BEGIN && FKey < KEY_END_FKEY)
			VirtKey=FKey-KEY_FKEY_BEGIN;
		else if (FKey && FKey < WCHAR_MAX)
		{
			short Vk = VkKeyScan(static_cast<wchar_t>(FKey));
			if (Vk == -1)
			{
				for (const auto& i: Layout())
				{
					if ((Vk = VkKeyScanEx(static_cast<wchar_t>(FKey), i)) != -1)
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
					FShift|=(HIBYTE(Vk)&1?KEY_SHIFT:NO_KEY)|
					        (HIBYTE(Vk)&2?KEY_CTRL:NO_KEY)|
					        (HIBYTE(Vk)&4?KEY_ALT:NO_KEY);
				}
			}

		}
		else if (!FKey)
		{
			static const std::pair<far_key_code, DWORD> ExtKeyMap[]=
			{
				{KEY_SHIFT, VK_SHIFT},
				{KEY_CTRL, VK_CONTROL},
				{KEY_ALT, VK_MENU},
				{KEY_RSHIFT, VK_RSHIFT},
				{KEY_RCTRL, VK_RCONTROL},
				{KEY_RALT, VK_RMENU},
			};

			// In case of CtrlShift, CtrlAlt, AltShift, CtrlAltShift there is no unambiguous mapping.
			const auto ItemIterator = std::find_if(CONST_RANGE(ExtKeyMap, i) { return (i.first & FShift) != 0; });
			if (ItemIterator != std::cend(ExtKeyMap))
				VirtKey = ItemIterator->second;
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
		if (FShift&KEY_SHIFT)
		{
			struct KeysData
			{
				DWORD FarKey;
				wchar_t Char;
			}
			const Keys[]
			{
				{'0',')'},{'1','!'},{'2','@'},{'3','#'},{'4','$'},
				{'5','%'},{'6','^'},{'7','&'},{'8','*'},{'9','('},
				{'`','~'},{'-','_'},{'=','+'},{'\\','|'},{'[','{'},
				{']','}'},{';',':'},{'\'','"'},{',','<'},{'.','>'},
				{'/','?'}
			};

			const auto ItemIterator = std::find_if(CONST_RANGE(Keys, Item)
			{
				return Item.FarKey == FKey;
			});

			if (ItemIterator != std::cend(Keys))
			{
				FKey = ItemIterator->Char;
			}
		}
	}

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
					    (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0)|
					    (FKey==KEY_DECIMAL?NUMLOCK_ON:0);

					static const DWORD ExtKey[] = {KEY_PGUP, KEY_PGDN, KEY_END, KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_INS, KEY_DEL, KEY_NUMENTER};
					if (contains(ExtKey, FKey))
						Rec->Event.KeyEvent.dwControlKeyState|=ENHANCED_KEY;
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
						ButtonState = MAKELONG(0, static_cast<WORD>(static_cast<short>(-120)));
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_RIGHT:
						ButtonState=MAKELONG(0,120);
						EventFlags|=MOUSE_HWHEELED;
						break;
					case KEY_MSWHEEL_LEFT:
						ButtonState = MAKELONG(0, static_cast<WORD>(static_cast<short>(-120)));
						EventFlags|=MOUSE_HWHEELED;
						break;

					case KEY_MSLCLICK:
					case KEY_MSRCLICK:
					case KEY_MSM1CLICK:
					case KEY_MSM2CLICK:
					case KEY_MSM3CLICK:
						ButtonState = KeyMsClickToButtonState(FKey);
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
				Rec->Event.MouseEvent.dwMousePosition.X = IntKeyState.MousePos.x;
				Rec->Event.MouseEvent.dwMousePosition.Y = IntKeyState.MousePos.y;
				break;
			}
			case WINDOW_BUFFER_SIZE_EVENT:
				UpdateScreenSize();
				break;
			case MENU_EVENT:
				Rec->Event.MenuEvent.dwCommandId=0;
				break;
			case FOCUS_EVENT:
				Rec->Event.FocusEvent.bSetFocus = FKey != KEY_KILLFOCUS;
				break;
		}
	}

	_SVS(SysLog(L"%s or %s ==> %s",_FARKEY_ToName(Key),_MCODE_ToName(Key),_INPUT_RECORD_Dump(Rec)));
	_SVS(SysLog(L"return VirtKey=%x",VirtKey));
	return VirtKey;
}


int IsNavKey(DWORD Key)
{
	static const std::pair<DWORD, DWORD> NavKeysMap[] =
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

	return std::any_of(CONST_RANGE(NavKeysMap, i)
	{
		return (!i.first && Key==i.second) || (i.first && (Key&0x00FFFFFF) == (i.second&0x00FFFFFF));
	});
}

int IsShiftKey(DWORD Key)
{
	static const DWORD ShiftKeys[] =
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
	};

	return IsModifKey(Key) || contains(ShiftKeys, Key);
}

bool IsModifKey(DWORD const Key)
{
	return any_of(Key, KEY_CTRL | KEY_ALT | KEY_SHIFT | KEY_RCTRL | KEY_RALT);
}

bool IsInternalKeyReal(unsigned int Key)
{
	return any_of(Key,
		KEY_NUMDEL,
		KEY_NUMENTER,
		KEY_MSWHEEL_UP, KEY_MSWHEEL_DOWN,
		KEY_MSWHEEL_LEFT, KEY_MSWHEEL_RIGHT,
		KEY_MSLCLICK, KEY_MSRCLICK, KEY_MSM1CLICK, KEY_MSM2CLICK, KEY_MSM3CLICK);
}

bool IsCharKey(unsigned int Key)
{
	return Key < 0x1000 || in_closed_range(KEY_MULTIPLY, Key, KEY_DIVIDE);
}

unsigned int ShieldCalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros)
{
	const auto SavedIntKeyState = IntKeyState; // нада! ибо CalcKeyCode "портит"... (Mantis#0001760)
	IntKeyState = {};
	const auto Ret = CalcKeyCode(rec, RealKey, NotMacros);
	IntKeyState = SavedIntKeyState;
	return Ret;
}

static int GetDirectlyMappedKey(int VKey)
{
	switch (VKey)
	{
	case VK_BROWSER_BACK: return KEY_BROWSER_BACK;
	case VK_BROWSER_FORWARD: return KEY_BROWSER_FORWARD;
	case VK_BROWSER_REFRESH: return KEY_BROWSER_REFRESH;
	case VK_BROWSER_STOP: return KEY_BROWSER_STOP;
	case VK_BROWSER_SEARCH: return KEY_BROWSER_SEARCH;
	case VK_BROWSER_FAVORITES: return KEY_BROWSER_FAVORITES;
	case VK_BROWSER_HOME: return KEY_BROWSER_HOME;
	case VK_VOLUME_MUTE: return KEY_VOLUME_MUTE;
	case VK_VOLUME_DOWN: return KEY_VOLUME_DOWN;
	case VK_VOLUME_UP: return KEY_VOLUME_UP;
	case VK_MEDIA_NEXT_TRACK: return KEY_MEDIA_NEXT_TRACK;
	case VK_MEDIA_PREV_TRACK: return KEY_MEDIA_PREV_TRACK;
	case VK_MEDIA_STOP: return KEY_MEDIA_STOP;
	case VK_MEDIA_PLAY_PAUSE: return KEY_MEDIA_PLAY_PAUSE;
	case VK_LAUNCH_MAIL: return KEY_LAUNCH_MAIL;
	case VK_LAUNCH_MEDIA_SELECT: return KEY_LAUNCH_MEDIA_SELECT;
	case VK_LAUNCH_APP1: return KEY_LAUNCH_APP1;
	case VK_LAUNCH_APP2: return KEY_LAUNCH_APP2;
	case VK_APPS: return KEY_APPS;
	case VK_LWIN: return KEY_LWIN;
	case VK_RWIN: return KEY_RWIN;
	case VK_BACK: return KEY_BS;
	case VK_TAB: return KEY_TAB;
	case VK_ADD: return KEY_ADD;
	case VK_SUBTRACT: return KEY_SUBTRACT;
	case VK_ESCAPE: return KEY_ESC;
	case VK_CAPITAL: return KEY_CAPSLOCK;
	case VK_NUMLOCK: return KEY_NUMLOCK;
	case VK_SCROLL: return KEY_SCROLLLOCK;
	case VK_DIVIDE: return KEY_DIVIDE;
	case VK_CANCEL: return KEY_BREAK;
	case VK_MULTIPLY: return KEY_MULTIPLY;
	case VK_SLEEP: return KEY_STANDBY;
	case VK_SNAPSHOT: return KEY_PRNTSCRN;
	default: return 0;
	}
}

// These VK_* map to different characters if Shift (and only Shift) is pressed
static int GetMappedCharacter(int VKey)
{
	switch (VKey)
	{
	case VK_OEM_PERIOD: return KEY_DOT;
	case VK_OEM_COMMA: return KEY_COMMA;
	case VK_OEM_MINUS: return '-';
	case VK_OEM_PLUS: return '=';

	// BUGBUG hard-coded for the US standard keyboard
	case VK_OEM_1: return KEY_SEMICOLON;
	case VK_OEM_2: return KEY_SLASH;
	case VK_OEM_3: return '`';
	case VK_OEM_4: return KEY_BRACKET;
	case VK_OEM_5: return KEY_BACKSLASH;
	case VK_OEM_6: return KEY_BACKBRACKET;
	case VK_OEM_7: return '\'';

	// BUGBUG does not exist in the US standard keyboard,
	// but does exist in some others (e.g. the UK)
	// '`' might not always be accurate, but it's better than nothing
	case VK_OEM_8: return '`';

	case VK_OEM_102: return KEY_BACKSLASH; // <> \|

	default: return 0;
	}
}

static int GetNumpadKey(const int KeyCode, const int CtrlState, const int Modif)
{
	static const struct numpad_mapping
	{
		int VCode;
		int FarCodeNumpad;
		int FarCodeEnhached;
		int FarCodeForNumLock;
	}
	NumpadMapping[] =
	{
		{ VK_NUMPAD0, KEY_NUMPAD0, KEY_INS, '0' },
		{ VK_NUMPAD1, KEY_NUMPAD1, KEY_END, '1' },
		{ VK_NUMPAD2, KEY_NUMPAD2, KEY_DOWN, '2' },
		{ VK_NUMPAD3, KEY_NUMPAD3, KEY_PGDN, '3' },
		{ VK_NUMPAD4, KEY_NUMPAD4, KEY_LEFT, '4' },
		{ VK_NUMPAD5, KEY_NUMPAD5, KEY_CLEAR, '5' },
		{ VK_NUMPAD6, KEY_NUMPAD6, KEY_RIGHT, '6' },
		{ VK_NUMPAD7, KEY_NUMPAD7, KEY_HOME, '7' },
		{ VK_NUMPAD8, KEY_NUMPAD8, KEY_UP, '8' },
		{ VK_NUMPAD9, KEY_NUMPAD9, KEY_PGUP, '9' },
		{ VK_DECIMAL, KEY_NUMDEL, KEY_DEL, KEY_DECIMAL },
	};

	const auto GetMappingIndex = [KeyCode]
	{
		switch (KeyCode)
		{
		case VK_INSERT: case VK_NUMPAD0: return 0;
		case VK_END:    case VK_NUMPAD1: return 1;
		case VK_DOWN:   case VK_NUMPAD2: return 2;
		case VK_NEXT:   case VK_NUMPAD3: return 3;
		case VK_LEFT:   case VK_NUMPAD4: return 4;
		case VK_CLEAR:  case VK_NUMPAD5: return 5;
		case VK_RIGHT:  case VK_NUMPAD6: return 6;
		case VK_HOME:   case VK_NUMPAD7: return 7;
		case VK_UP:     case VK_NUMPAD8: return 8;
		case VK_PRIOR:  case VK_NUMPAD9: return 9;
		case VK_DELETE: case VK_DECIMAL: return 10;
		default:
			return -1;
		}
	};

	const auto MappingIndex = GetMappingIndex();
	if (MappingIndex == -1)
		return 0;

	const auto& Mapping = NumpadMapping[MappingIndex];

	if (CtrlState & ENHANCED_KEY)
	{
		return Modif | Mapping.FarCodeEnhached;
	}

	if ((CtrlState & NUMLOCK_ON) && !Modif && KeyCode == Mapping.VCode)
	{
		return Mapping.FarCodeForNumLock;
	}

	return Modif | Mapping.FarCodeNumpad;
}

static int GetMouseKey(const MOUSE_EVENT_RECORD& MouseEvent)
{
	switch (MouseEvent.dwEventFlags)
	{
	case 0:
	{
		const auto MsKey = ButtonStateToKeyMsClick(MouseEvent.dwButtonState);
		if (MsKey != KEY_NONE)
		{
			return MsKey;
		}
	}
	break;

	case MOUSE_WHEELED:
	case MOUSE_HWHEELED:
	{
		const auto& WheelKeysPair = WheelKeys[MouseEvent.dwEventFlags == MOUSE_HWHEELED? 1 : 0];
		const auto Key = WheelKeysPair[static_cast<short>(HIWORD(MouseEvent.dwButtonState)) > 0? 1 : 0];
		return Key;
	}

	default:
		break;
	}

	return 0;
}

unsigned int CalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros)
{
	_SVS(CleverSysLog Clev(L"CalcKeyCode"));
	_SVS(SysLog(L"CalcKeyCode -> %s| RealKey=%d  *NotMacros=%d",_INPUT_RECORD_Dump(rec),RealKey,(NotMacros?*NotMacros:0)));
	const auto CtrlState = rec->EventType==MOUSE_EVENT? rec->Event.MouseEvent.dwControlKeyState : rec->Event.KeyEvent.dwControlKeyState;
	const auto ScanCode = rec->Event.KeyEvent.wVirtualScanCode;
	const auto KeyCode = rec->Event.KeyEvent.wVirtualKeyCode;
	const auto Char = rec->Event.KeyEvent.uChar.UnicodeChar;

	if (NotMacros)
		*NotMacros = (CtrlState&0x80000000) != 0;

	if (!(rec->EventType==KEY_EVENT || rec->EventType == MOUSE_EVENT))
		return KEY_NONE;

	if (!RealKey)
	{
		UpdateIntKeyState(CtrlState);
	}

	const auto ModifCtrl = IntKeyState.RightCtrlPressed? KEY_RCTRL : IntKeyState.CtrlPressed()? KEY_CTRL : NO_KEY;
	const auto ModifAlt = IntKeyState.RightAltPressed? KEY_RALT: IntKeyState.AltPressed()? KEY_ALT : NO_KEY;
	const auto ModifShift = IntKeyState.RightShiftPressed? KEY_SHIFT : IntKeyState.ShiftPressed() ? KEY_SHIFT : NO_KEY;
	const auto Modif = ModifCtrl | ModifAlt | ModifShift;

	if (rec->EventType==MOUSE_EVENT)
	{
		if (const auto MouseKey = GetMouseKey(rec->Event.MouseEvent))
		{
			return Modif | MouseKey;
		}

		return KEY_NONE;
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

	static const time_check TimeCheck(time_check::mode::delayed, 50ms);

	if (!AltValue)
	{
		TimeCheck.reset();
	}

	if (!rec->Event.KeyEvent.bKeyDown)
	{
		KeyCodeForALT_LastPressed=0;

		switch (KeyCode)
		{
			case VK_MENU:
				if (AltValue)
				{
					if (RealKey)
					{
						DropConsoleInputEvent();
					}
					IntKeyState.ReturnAltValue = true;
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

					// Starting from Windows 7 Event.KeyEvent.uChar.UnicodeChar is always populated, but not always properly.
					// We can't really recognise Drag&Drop (where it's correct) and Alt+Numpad (where it isn't - see https://github.com/microsoft/terminal/issues/3323 for details).
					// Let's assume that if the interval between the events is less than 50 ms - it's probably D&D and the manual input otherwise.

					// Windows 10 "new console" uses a different method for D&D & paste:

					// bKeyDown=TRUE,   wRepeatCount=1, wVirtualKeyCode=NULL, UnicodeChar=1099,    dwControlKeyState=0
					// bKeyDown=FALSE,  wRepeatCount=1, wVirtualKeyCode=NULL, UnicodeChar=1099,    dwControlKeyState=0

					// wVirtualKeyCode might or might not be NULL depending on your keyboard layout *facepalm*
					// This means that it no longer conflicts with Alt-Numpad and we don't need this hack (but still need for the classic console)

					if (!::console.IsVtSupported() && rec->Event.KeyEvent.uChar.UnicodeChar && !TimeCheck)
					{
						AltValue=rec->Event.KeyEvent.uChar.UnicodeChar;
					}

					// Reconstruct the broken UnicodeChar. See https://github.com/microsoft/terminal/issues/3323 for details.
					rec->Event.KeyEvent.uChar.UnicodeChar = AltValue;
					return AltValue;
				}
				return Modif|((CtrlState&ENHANCED_KEY)?KEY_RALT:KEY_ALT);

			case VK_CONTROL:
				return Modif|((CtrlState&ENHANCED_KEY)?KEY_RCTRL:KEY_CTRL);

			case VK_SHIFT:
				return Modif|KEY_SHIFT;
		}
		return KEY_NONE;
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
	if ((CtrlState & (LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED)) == (LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED))
	{
		if (Char >= L' ')
		{
			return Char;
		}

		if (RealKey && ScanCode && !Char && KeyCode && KeyCode != VK_MENU)
		{
			//Это шаманство для ввода всяческих букв с тильдами, акцентами и прочим.
			//Например на Шведской раскладке, "AltGr+VK_OEM_1" вообще не должно обрабатываться фаром, т.к. это DeadKey
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
		}

		IntKeyState.LeftCtrlPressed = IntKeyState.RightCtrlPressed = false;
	}

	if (KeyCode==VK_MENU)
		AltValue=0;

	if (in_closed_range(unsigned(VK_F1), KeyCode, unsigned(VK_F24)))
		return Modif + KEY_F1 + (KeyCode - VK_F1);

	if (IntKeyState.OnlyAltPressed())
	{
		if (!(CtrlState & ENHANCED_KEY))
		{
			static unsigned int const ScanCodes[]{ 82, 79, 80, 81, 75, 76, 77, 71, 72, 73 };

			for (size_t i = 0; i != std::size(ScanCodes); ++i)
			{
				if (ScanCodes[i]==ScanCode)
				{
					if (RealKey && KeyCodeForALT_LastPressed != KeyCode)
					{
						AltValue = AltValue * 10 + static_cast<int>(i);
						KeyCodeForALT_LastPressed=KeyCode;
					}

					if (AltValue)
						return KEY_NONE;
				}
			}
		}
	}

	switch (KeyCode)
	{
	case VK_RETURN:
		return Modif | ((CtrlState & ENHANCED_KEY)? KEY_NUMENTER : KEY_ENTER);

	case VK_PAUSE:
		return Modif | ((CtrlState & ENHANCED_KEY)? KEY_NUMLOCK : KEY_PAUSE);

	case VK_SPACE:
		if (Char == L' ' || !Char)
			return Modif | KEY_SPACE;
		return Char;
	}

	if (const auto NumpadKey = GetNumpadKey(KeyCode, CtrlState, Modif))
	{
		// Modif is added from within GetNumpadKey conditionally
		return NumpadKey;
	}

	if(const auto MappedKey = GetDirectlyMappedKey(KeyCode))
	{
		const auto Result = Modif | MappedKey;

		if (Result == KEY_ESC && console.IsViewportShifted())
		{
			console.ResetPosition();
			return KEY_NONE;
		}
		else
			return Result;
	}

	if (!IntKeyState.CtrlPressed() && !IntKeyState.AltPressed() && (KeyCode >= VK_OEM_1 && KeyCode <= VK_OEM_8) && !Char)
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

	if (!IntKeyState.CtrlPressed() && !IntKeyState.AltPressed())
	{
		// Shift or none - characters only
		if (!Char || KeyCode == VK_SHIFT)
			return KEY_NONE;
		return Char;
	}

	if (in_closed_range(L'0',  KeyCode, L'9') || in_closed_range(L'A', KeyCode, L'Z'))
		return Modif | KeyCode;

	if (const auto OemKey = GetMappedCharacter(KeyCode))
	{
		return Modif + OemKey;
	}

	return Char? Modif | Char : KEY_NONE;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("keyboard.KeyNames")
{
	static_assert(KEY_CLEAR == KEY_NUMPAD5);

	static const struct
	{
		int Key;
		string_view Str;
		string_view Str2;
	}
	Tests[]
	{
		{ 0,                          {} },
		{ 0,                          L"BANANA"sv },
		{ L'C',                       L"C"sv },
		{ KEY_ALT,                    L"Alt"sv },
		{ KEY_RALT,                   L"RAlt"sv },
		{ KEY_CTRL,                   L"CtRl"sv },
		{ KEY_RCTRL,                  L"RCtrL"sv },
		{ KEY_SLASH,                  L"/"sv },
		{ KEY_SHIFT,                  L"SHIFT"sv },
		{ KEY_CTRLC,                  L"CtrlC"sv },
		{ KEY_CLEAR,                  L"Clear"sv, L"Num5"sv },
		{ KEY_SHIFTF12,               L"ShiftF12"sv },
		{ KEY_CTRLALTENTER|KEY_SHIFT, L"CtrlAltShiftEnter"sv },
		{ KEY_VK_0xFF_BEGIN,          L"Spec00000"sv },
		{ KEY_VK_0xFF_END,            L"Spec00255"sv },
		{ KEY_VK_0xFF_END + 1,        L"Oem00512"sv },
		{ KEY_END_FKEY,               L"Oem65535"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Key == KeyNameToKey(i.Str));

		if (!i.Str2.empty())
			REQUIRE(i.Key == KeyNameToKey(i.Str2));

		const auto Str = KeyToText(i.Key);
		if (i.Key)
			REQUIRE(equal_icase(i.Str, Str));
		else
			REQUIRE(Str.empty());
	}
}
#endif
