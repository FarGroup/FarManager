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
#include "interf.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "console.hpp"
#include "plugins.hpp"
#include "notification.hpp"
#include "lang.hpp"
#include "datetime.hpp"
#include "string_utils.hpp"
#include "global.hpp"
#include "log.hpp"
#include "xlat.hpp"

// Platform:
#include "platform.hpp"
#include "platform.version.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* start Глобальные переменные */

FarKeyboardState IntKeyState{};

/* end Глобальные переменные */

static std::array<short, WCHAR_MAX + 1> KeyToVKey;
static std::array<wchar_t, 512> VKeyToASCII;

static unsigned int AltValue=0;
static unsigned int KeyCodeForALT_LastPressed=0;
static bool IsWindowFocused{};

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

static bool s_WakeupForClock{};
void wakeup_for_clock(bool Value)
{
	s_WakeupForClock = Value;
}

static bool s_WakeupForScreensaver{};
void wakeup_for_screensaver(bool Value)
{
	s_WakeupForScreensaver = Value;
}

/* ----------------------------------------------------------------- */
struct TFKey
{
	DWORD Key;
	lng LocalizedNameId;
	string_view Name;

	bool operator==(DWORD rhsKey) const {return Key == rhsKey;}
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
};

static auto& Layout()
{
	static auto s_Layout = os::get_keyboard_layout_list();
	return s_Layout;
}

/*
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
	KeyToVKey.fill(0);
	VKeyToASCII.fill(0);

	//KeyToVKey - используется чтоб проверить если два символа это одна и та же кнопка на клаве
	//*********
	//Так как сделать полноценное мапирование между всеми раскладками не реально,
	//по причине того что во время проигрывания макросов нет такого понятия раскладка
	//то сделаем наилучшую попытку - смысл такой, делаем полное мапирование всех возможных
	//VKs и ShiftVKs в юникодные символы проходясь по всем раскладкам с одним но:
	//если разные VK мапятся в тот же юникод символ то мапирование будет только для первой
	//раскладки которая вернула этот символ
	//

	BYTE KeyState[256]{};

	for (const auto j: std::views::iota(0, 2))
	{
		KeyState[VK_SHIFT] = j * 0x80;

		for (const auto& i: Layout())
		{
			for (const auto VK : std::views::iota(0, 256))
			{
				if (wchar_t Buffer[2]; os::to_unicode(VK, 0, KeyState, Buffer, 0, i) > 0)
				{
					const auto idx = Buffer[0];
					if (!KeyToVKey[idx])
						KeyToVKey[idx] = VK + j * 0x100;

					// VKeyToASCII - используется вместе с KeyToVKey чтоб подменить нац. символ на US-ASCII
					// Имея мапирование юникод -> VK строим обратное мапирование
					// VK -> символы с кодом меньше 0x80, т.е. только US-ASCII символы
					if (idx < 0x80 && !VKeyToASCII[VK + j * 0x100])
						VKeyToASCII[VK + j * 0x100] = upper(idx);
				}
			}
		}
	}

	// If the user has the 'X' UI language, but doesn't have the 'X' keyboard layout for whatever reason,
	// this would allow to map that language via user-defined XLat tables
	xlat_observe_tables([](wchar_t const Local, wchar_t const English)
	{
		if (!KeyToVKey[Local])
			KeyToVKey[Local] = KeyToVKey[English];
	});
}

//Сравнивает если Key и CompareKey это одна и та же клавиша в разных раскладках
bool KeyToKeyLayoutCompare(int Key, int CompareKey)
{
	Key = KeyToVKey[Key&0xFFFF]&0xFF;
	CompareKey = KeyToVKey[CompareKey&0xFFFF]&0xFF;

	return Key && Key == CompareKey;
}

//Должно вернуть клавишный Eng эквивалент Key
int KeyToKeyLayout(int Key)
{
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
			if (oldState & 0x8000) // key is down
			{
				keybd_event(vkKey, 0, ExKey | KEYEVENTF_KEYUP, 0);
				keybd_event(vkKey, 0, ExKey, 0);
			}
			else
			{
				keybd_event(vkKey, 0, ExKey, 0);
				keybd_event(vkKey, 0, ExKey | KEYEVENTF_KEYUP, 0);
			}
		}
	}

	return oldState;
}

static unsigned int ShieldCalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros = {});

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

		const auto Layout = console.GetKeyboardLayout();
		Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKeyEx(Rec->Event.KeyEvent.wVirtualKeyCode, MAPVK_VK_TO_VSC, Layout);
		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeyexw
		// an unshifted character value in the low order word of the return value
		Rec->Event.KeyEvent.uChar.UnicodeChar = extract_integer<wchar_t, 0>(MapVirtualKeyEx(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_CHAR, Layout));

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

bool IsMouseButtonEvent(DWORD const EventFlags)
{
	return EventFlags == 0 || EventFlags == DOUBLE_CLICK;
}

int get_wheel_threshold(int ConfigValue)
{
	return ConfigValue? ConfigValue : WHEEL_DELTA;
}

static int get_wheel_scroll(unsigned const Type, int const ConfigValue)
{
	if (ConfigValue)
		return ConfigValue;

	if (UINT Value; SystemParametersInfo(Type, 0, &Value, 0))
		return Value;

	return 1;
}

int get_wheel_scroll_lines(int const ConfigValue)
{
	return get_wheel_scroll(SPI_GETWHEELSCROLLLINES, ConfigValue);
}

int get_wheel_scroll_chars(int const ConfigValue)
{
	return get_wheel_scroll(SPI_GETWHEELSCROLLCHARS, ConfigValue);
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

class wake_event
{
public:
	static auto& ref()
	{
		if (!s_WakeEvent)
			s_WakeEvent = os::concurrency::event(os::event::type::automatic, os::event::state::nonsignaled);

		return s_WakeEvent;
	}

private:
	static inline os::concurrency::event s_WakeEvent;
};

void main_loop_process_messages()
{
	wake_event::ref().set();
}

DWORD GetInputRecordNoMacroArea(INPUT_RECORD *rec)
{
	const auto SavedArea = Global->CtrlObject->Macro.GetArea();
	SCOPE_EXIT{ Global->CtrlObject->Macro.SetArea(SavedArea); };

	Global->CtrlObject->Macro.SetArea(MACROAREA_LAST); // чтобы не срабатывали макросы :-)
	return GetInputRecord(rec, false, false);
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
			((MacroKey & ~0xFF000000) >= KEY_END_FKEY && !any_of(MacroKey & ~0xFF000000, KEY_NUMENTER, KEY_NUMDEL))?
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
	IsWindowFocused = Got;

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
	}

	return KEY_CONSOLE_BUFFER_RESIZE;
}

static const far_key_code WheelKeys[][2] =
{
	{ KEY_MSWHEEL_DOWN, KEY_MSWHEEL_UP },
	{ KEY_MSWHEEL_LEFT, KEY_MSWHEEL_RIGHT }
};

static bool ProcessMouseEvent(MOUSE_EVENT_RECORD& MouseEvent, bool ExcludeMacro, bool ProcessMouse, DWORD& CalcKey)
{
	lastMOUSE_EVENT_RECORD = MouseEvent;
	IntKeyState.PreMouseEventFlags = std::exchange(IntKeyState.MouseEventFlags, MouseEvent.dwEventFlags);
	const auto CtrlState = MouseEvent.dwControlKeyState;
	KeyMacro::SetMacroConst(constMsCtrlState, CtrlState);
	KeyMacro::SetMacroConst(constMsEventFlags, IntKeyState.MouseEventFlags);
	KeyMacro::SetMacroConst(constMsLastCtrlState, CtrlState);

	UpdateIntKeyState(CtrlState);

	KeyMacro::SetMacroConst(constMsButton, MouseEvent.dwButtonState);

	IntKeyState.PrevMouseButtonState = std::exchange(IntKeyState.MouseButtonState, MouseEvent.dwButtonState);
	IntKeyState.MousePrevPos = std::exchange(IntKeyState.MousePos, MouseEvent.dwMousePosition);
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
		// https://learn.microsoft.com/en-gb/windows/win32/inputdev/wm-mousewheel
		// The wheel rotation will be a multiple of WHEEL_DELTA, which is set at 120.
		// This is the threshold for action to be taken, and one such action
		// (for example, scrolling one increment) should occur for each delta.
		// The delta was set to 120 to allow Microsoft or other vendors to build
		// finer-resolution wheels (a freely-rotating wheel with no notches)
		// to send more messages per rotation, but with a smaller value in each message.
		// To use this feature, you can either add the incoming delta values
		// until WHEEL_DELTA is reached (so for a delta-rotation you get the same response),
		// or scroll partial lines in response to the more frequent messages.
		// You can also choose your scroll granularity and accumulate deltas until it is reached.
		static int StoredTicks = 0;
		const auto Ticks = static_cast<short>(extract_integer<WORD, 1>(MouseEvent.dwButtonState));

		// Discard stored ticks on scrolling direction change
		if ((Ticks > 0) == (StoredTicks > 0))
			StoredTicks += Ticks;
		else
			StoredTicks = Ticks;

		const auto Threshold = get_wheel_threshold(Global->Opt->MsWheelThreshold);

		if (std::abs(StoredTicks) < Threshold)
		{
			CalcKey = KEY_NONE;
			return true;
		}

		const auto& WheelKeysPair = WheelKeys[IntKeyState.MouseEventFlags == MOUSE_HWHEELED? 1 : 0];
		const auto Key = WheelKeysPair[StoredTicks > 0? 1 : 0];
		CalcKey = Key | GetModifiers();

		// Move accumulated ticks into the event, so that clients can inspect them via Manager::NumberOfWheelEvents() and act accordingly.
		const auto Remainder = StoredTicks % Threshold;
		MouseEvent.dwButtonState = make_integer<DWORD>(extract_integer<WORD, 0>(MouseEvent.dwButtonState), static_cast<WORD>(StoredTicks - Remainder));
		StoredTicks = Remainder;
		return false;
	}

	if ((!ExcludeMacro || ProcessMouse) && Global->CtrlObject && (ProcessMouse || !(Global->CtrlObject->Macro.IsRecording() || Global->CtrlObject->Macro.IsExecuting())))
	{
		if (!IntKeyState.MouseEventFlags)
		{
			// By clearing the previously pressed buttons we ensure that the newly pressed one will be reported
			const auto MsCalcKey = ButtonStateToKeyMsClick(MouseEvent.dwButtonState & ~IntKeyState.PrevMouseButtonState);
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

static unsigned int CalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros = {});

static DWORD GetInputRecordImpl(INPUT_RECORD *rec,bool ExcludeMacro,bool ProcessMouse)
{
	if (!os::handle::is_signaled(console.GetInputHandle()))
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

		const FAR_INPUT_RECORD irec{ CalcKey, *rec };
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

	Global->ScrBuf->Flush();

	auto FullscreenState = IsConsoleFullscreen();

	for (;;)
	{
		WindowState.update();

		if(Global->CtrlObject)
		{
			SetFarConsoleMode();
		}

		{
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

		static bool ExitInProcess = false;
		if (Global->CloseFAR && !ExitInProcess)
		{
			ExitInProcess = true;
			Global->WindowManager->ExitMainLoop(FALSE);
			return KEY_NONE;
		}

		if (!os::handle::is_signaled(console.GetInputHandle()) && message_manager::instance().dispatch())
		{
			*rec = {};
			return KEY_NONE;
		}

		static auto LastActivity = std::chrono::steady_clock::now();

		std::optional<std::chrono::milliseconds> Timeout;
		if (s_WakeupForClock || s_WakeupForScreensaver)
			Timeout = till_next_minute();

		const auto Status = os::handle::wait_any({ console.GetInputHandle(), wake_event::ref().native_handle() }, Timeout);

		if (!Status)
		{
			if (IsWindowFocused && Global->Opt->ScreenSaver &&
				Global->Opt->ScreenSaverTime > 0 &&
				std::chrono::steady_clock::now() - LastActivity >= std::chrono::minutes(Global->Opt->ScreenSaverTime))
			{
				ScreenSaver();
			}
		}
		else if (*Status == 0)
		{
			LastActivity = std::chrono::steady_clock::now();
		}
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

	if (rec->EventType == FOCUS_EVENT)
	{
		return ProcessFocusEvent(rec->Event.FocusEvent.bSetFocus != FALSE);
	}

	if (rec->EventType == WINDOW_BUFFER_SIZE_EVENT)
	{
		// Fake event, generated internally
		const auto IsInternalEvent = !rec->Event.WindowBufferSizeEvent.dwSize.X && !rec->Event.WindowBufferSizeEvent.dwSize.Y;

		static point LastBufferSize{};
		SCOPE_EXIT
		{
			if (!IsInternalEvent)
				LastBufferSize = rec->Event.WindowBufferSizeEvent.dwSize;
		};

		if (
			// Skip if the size isn't changed to filter out Windows 10 rubbish (see https://github.com/Microsoft/console/issues/281)
			IsInternalEvent || IsConsoleViewportSizeChanged()
		)
		{
			// Do not use rec->Event.WindowBufferSizeEvent.dwSize here - we need a 'virtual' size
			point Size;
			return console.GetSize(Size)? ProcessBufferSizeEvent(Size) : static_cast<DWORD>(KEY_CONSOLE_BUFFER_RESIZE);
		}

		if (LastBufferSize != rec->Event.WindowBufferSizeEvent.dwSize)
		{
			// Buffer size changed, but the window size stayed the same.
			// In window mode this means that the actual drawing area could now be dramatically different
			console.ResetViewportPosition();
			Global->ScrBuf->Invalidate();
			Global->ScrBuf->Flush();
		}
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

		if (std::ranges::any_of(Keys, [&CalcKey](const KeysData& A){ return CalcKey == A.FarKey && !PressedLast.Check(A.Modif); }))
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

DWORD GetInputRecord(INPUT_RECORD *rec, bool ExcludeMacro, bool ProcessMouse)
{
	*rec = {};

	DWORD Key = GetInputRecordImpl(rec, ExcludeMacro, ProcessMouse);

	if (Key && !in_closed_range(KEY_OP_BASE, Key, KEY_OP_ENDBASE))
	{
		if (Global->CtrlObject)
		{
			ProcessConsoleInputInfo Info{ sizeof(Info), PCIF_NONE, *rec };

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
DWORD WaitKey(DWORD KeyWait, std::optional<std::chrono::milliseconds> const Timeout, bool ExcludeMacro)
{
	// Don't wait for console input handle here.
	// People expect strange things from this function, e.g. working with "keys" sent by macros.
	// Yes, this means constant polling and high CPU load.
	// And this is why we can't have nice things.

	std::optional<time_check> TimeCheck;
	if (Timeout)
		TimeCheck.emplace(time_check::mode::delayed, *Timeout);

	for (;;)
	{
		INPUT_RECORD rec;
		const auto Key = PeekInputRecord(&rec, ExcludeMacro)?
			GetInputRecord(&rec, ExcludeMacro, true) :
			static_cast<DWORD>(KEY_NONE);

		if (KeyWait == static_cast<DWORD>(-1))
		{
			if ((Key & ~KEY_CTRLMASK) < KEY_END_FKEY || IsInternalKeyReal(Key & ~KEY_CTRLMASK))
				return Key;
		}
		else if (Key == KeyWait)
			return Key;

		if (TimeCheck && *TimeCheck)
		{
			return KEY_NONE;
		}

		os::chrono::sleep_for(1ms);
	}
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
		switch (GetInputRecordNoMacroArea(&rec))
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
		const auto ItemIterator = std::ranges::find_if(FKeys1, [&](TFKey const& i)
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
			else
			{
				const auto
					OemPrefix = L"Oem"sv,
					SpecPrefix = L"Spec"sv;

				if (const auto IsOem = Name.starts_with(OemPrefix); IsOem || Name.starts_with(SpecPrefix))
				{
					const auto Tail = Name.substr(IsOem? OemPrefix.size() : SpecPrefix.size());
					if (Tail.size() == 5 && std::ranges::all_of(Tail, std::iswdigit)) // Варианты (3) и (4)
					{
						Key |= (IsOem? KEY_FKEY_BEGIN : KEY_VK_0xFF_BEGIN) | from_string<unsigned>(Tail);
						Name = {};
					}
				}
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

	// Ugh, ranges are awesome.
	if (const auto FKeys1Iterator = std::ranges::find_if(FKeys1, [&](TFKey const& Key){ return Key == FKey; }); FKeys1Iterator != std::cend(FKeys1))
	{
		AddSeparator(strKeyText);
		append(strKeyText, ToText(*FKeys1Iterator));
		return strKeyText;
	}

	if (FKey >= KEY_VK_0xFF_BEGIN && FKey <= KEY_VK_0xFF_END)
	{
		AddSeparator(strKeyText);
		far::format_to(strKeyText, L"Spec{:0>5}"sv, FKey - KEY_VK_0xFF_BEGIN);
		return strKeyText;

	}

	if (FKey > KEY_VK_0xFF_END && FKey <= KEY_END_FKEY)
	{
		AddSeparator(strKeyText);
		far::format_to(strKeyText, L"Oem{:0>5}"sv, FKey - KEY_FKEY_BEGIN);
		return strKeyText;

	}

	FKey = upper(static_cast<wchar_t>(Key & 0xFFFF));

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
			const auto& Msg = msg(i.LocalizedNameId);
			return Msg.empty()? i.Name : Msg;
		},
		[](string& str)
		{
			if (!str.empty())
				str += L'+';
		}
	);
}

string KeysListToLocalizedText(std::span<unsigned int const> const Keys)
{
	return join(L" "sv, Keys | std::views::transform([](unsigned int const Key){ return KeyToLocalizedText(Key); }));
}

static int key_to_vk(unsigned int const Key)
{
	switch (Key)
	{
	case KEY_BREAK:       return VK_CANCEL;
	case KEY_BS:          return VK_BACK;
	case KEY_TAB:         return VK_TAB;
	case KEY_ENTER:       return VK_RETURN;
	case KEY_NUMENTER:    return VK_RETURN;
	case KEY_ESC:         return VK_ESCAPE;
	case KEY_SPACE:       return VK_SPACE;
	case KEY_NUMDEL:      return VK_DELETE;
	case KEY_NUMPAD5:     return VK_CLEAR;
	default:              return 0;
	}
}

int TranslateKeyToVK(int Key, INPUT_RECORD* Rec)
{
	WORD EventType=KEY_EVENT;

	DWORD FKey  =Key&KEY_END_SKEY;
	DWORD FShift=Key&KEY_CTRLMASK;

	auto VirtKey = key_to_vk(FKey);

	if (VirtKey == 0)
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
			short Vk = VkKeyScanEx(static_cast<wchar_t>(FKey), console.GetKeyboardLayout());
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

				VirtKey = extract_integer<BYTE, 0>(Vk);
				const auto CtrlState = extract_integer<BYTE, 1>(Vk);
				if (CtrlState && (CtrlState & 6) != 6) //RAlt-E в немецкой раскладке это евро, а не CtrlRAltЕвро
				{
					FShift |=
						(CtrlState & 1? KEY_SHIFT : NO_KEY) |
						(CtrlState & 2? KEY_CTRL  : NO_KEY) |
						(CtrlState & 4? KEY_ALT   : NO_KEY);
				}
			}

		}
		else if (!FKey)
		{
			static const std::pair<far_key_code, DWORD> ExtKeyMap[]
			{
				// the order is important, because "the key" is the last component of e.g. `CtrlAltShift`
				{KEY_SHIFT, VK_SHIFT},
				{KEY_RSHIFT, VK_RSHIFT},
				{KEY_ALT, VK_MENU},
				{KEY_RALT, VK_RMENU},
				{KEY_CTRL, VK_CONTROL},
				{KEY_RCTRL, VK_RCONTROL},
			};

			// In case of CtrlShift, CtrlAlt, AltShift, CtrlAltShift there is no unambiguous mapping.
			const auto ItemIterator = std::ranges::find_if(ExtKeyMap, [&](auto const& i){ return (i.first & FShift) != 0; });
			if (ItemIterator != std::cend(ExtKeyMap))
				VirtKey = ItemIterator->second;
		}
		else
		{
			VirtKey=FKey;
			switch (FKey)
			{
				case KEY_NONE:
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

			if (const auto ItemIterator = std::ranges::find(Keys, FKey, &KeysData::FarKey); ItemIterator != std::cend(Keys))
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
						Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKeyEx(Rec->Event.KeyEvent.wVirtualKeyCode, MAPVK_VK_TO_VSC, console.GetKeyboardLayout());
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

					static const DWORD ExtKey[]
					{
						KEY_PGUP,
						KEY_PGDN,
						KEY_END,
						KEY_HOME,
						KEY_LEFT,
						KEY_UP,
						KEY_RIGHT,
						KEY_DOWN,
						KEY_INS,
						KEY_DEL,
						KEY_NUMENTER,
						//todo Browser*, Launch*, Media*, ... (Standby, Spec*, Oem* ?)
						KEY_LWIN,
						KEY_RWIN,
						KEY_APPS,
						KEY_PRNTSCRN,
						KEY_BREAK,
						KEY_DIVIDE,
						KEY_NUMLOCK
					};

					if (contains(ExtKey, FKey) || VirtKey==VK_RCONTROL || VirtKey==VK_RMENU)
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
						ButtonState = make_integer<DWORD, 0, 120>();
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_DOWN:
						ButtonState = make_integer<DWORD, 0, static_cast<unsigned short>(-120)>();
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_RIGHT:
						ButtonState = make_integer<DWORD, 0, 120>();
						EventFlags|=MOUSE_HWHEELED;
						break;
					case KEY_MSWHEEL_LEFT:
						ButtonState = make_integer<DWORD, 0, static_cast<unsigned short>(-120)>();
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

	return VirtKey;
}

bool IsModifKey(DWORD const Key)
{
	return Key && (Key & (KEY_CTRL | KEY_ALT | KEY_SHIFT | KEY_RCTRL | KEY_RALT)) == Key;
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
	return Key < 0x10000 || in_closed_range(KEY_MULTIPLY, Key, KEY_DIVIDE);
}

static unsigned int ShieldCalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros)
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
static int GetMappedCharacter(int VKey, int const ScanCode)
{
	// VK_OEM_* are mapped to different physical keys on different national keyboards.
	// We map them to US keyboard characters via scan codes to ensure consistency.
	// The key names will likely be incorrect for, say, German keyboards,
	// but at least default actions and macros will work regardless of the input language.
	switch (VKey)
	{
	case VK_OEM_PERIOD:
	case VK_OEM_COMMA:
	case VK_OEM_MINUS:
	case VK_OEM_PLUS:
	case VK_OEM_1:
	case VK_OEM_2:
	case VK_OEM_3:
	case VK_OEM_4:
	case VK_OEM_5:
	case VK_OEM_6:
	case VK_OEM_7:
	case VK_OEM_8:
	case VK_OEM_102:
		switch (ScanCode)
		{
		case 12: return '-';
		case 13: return '=';
		case 26: return KEY_BRACKET;
		case 27: return KEY_BACKBRACKET;
		case 39: return KEY_SEMICOLON;
		case 40: return '\'';
		case 41: return '`';
		case 43: return KEY_BACKSLASH;
		case 51: return KEY_COMMA;
		case 52: return KEY_DOT;
		case 53: return KEY_SLASH;
		case 86: return KEY_BACKSLASH;
		default: return 0;
		}
	default: return 0;
	}
}

static int GetNumpadKey(const int KeyCode, const int CtrlState, const int Modif)
{
	static const struct numpad_mapping
	{
		int VCode;
		int FarCodeNumpad;
		int FarCodeEnhanced;
		int FarCodeForNumLock;
	}
	NumpadMapping[]
	{
		{ VK_NUMPAD0, KEY_NUMPAD0, KEY_INS,   '0' },
		{ VK_NUMPAD1, KEY_NUMPAD1, KEY_END,   '1' },
		{ VK_NUMPAD2, KEY_NUMPAD2, KEY_DOWN,  '2' },
		{ VK_NUMPAD3, KEY_NUMPAD3, KEY_PGDN,  '3' },
		{ VK_NUMPAD4, KEY_NUMPAD4, KEY_LEFT,  '4' },
		{ VK_NUMPAD5, KEY_NUMPAD5, KEY_CLEAR, '5' },
		{ VK_NUMPAD6, KEY_NUMPAD6, KEY_RIGHT, '6' },
		{ VK_NUMPAD7, KEY_NUMPAD7, KEY_HOME,  '7' },
		{ VK_NUMPAD8, KEY_NUMPAD8, KEY_UP,    '8' },
		{ VK_NUMPAD9, KEY_NUMPAD9, KEY_PGUP,  '9' },
		{ VK_DECIMAL, KEY_NUMDEL,  KEY_DEL,   KEY_DECIMAL },
	};

	const auto
		NumKey = 0x100;

	const auto GetMappingIndex = [&]
	{
		switch (KeyCode)
		{
		case VK_INSERT:   return 0;
		case VK_END:      return 1;
		case VK_DOWN:     return 2;
		case VK_NEXT:     return 3;
		case VK_LEFT:     return 4;
		case VK_CLEAR:    return 5;
		case VK_RIGHT:    return 6;
		case VK_HOME:     return 7;
		case VK_UP:       return 8;
		case VK_PRIOR:    return 9;
		case VK_DELETE:   return 10;

		case VK_NUMPAD0:  return NumKey + 0;
		case VK_NUMPAD1:  return NumKey + 1;
		case VK_NUMPAD2:  return NumKey + 2;
		case VK_NUMPAD3:  return NumKey + 3;
		case VK_NUMPAD4:  return NumKey + 4;
		case VK_NUMPAD5:  return NumKey + 5;
		case VK_NUMPAD6:  return NumKey + 6;
		case VK_NUMPAD7:  return NumKey + 7;
		case VK_NUMPAD8:  return NumKey + 8;
		case VK_NUMPAD9:  return NumKey + 9;
		case VK_DECIMAL:  return NumKey + 10;

		default:
			return -1;
		}
	};

	const auto MappingIndex = GetMappingIndex();
	if (MappingIndex == -1)
		return 0;

	const auto IsNumKey = (MappingIndex & NumKey) != 0;

	const auto& Mapping = NumpadMapping[MappingIndex & ~NumKey];

	if (CtrlState & ENHANCED_KEY)
	{
		return Modif | Mapping.FarCodeEnhanced;
	}

	if (IsNumKey && !Modif && KeyCode == Mapping.VCode)
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
	case DOUBLE_CLICK:
	{
		// By clearing the previously pressed buttons we ensure that the newly pressed one will be reported
		const auto MsKey = ButtonStateToKeyMsClick(MouseEvent.dwButtonState & ~IntKeyState.PrevMouseButtonState);
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
		const auto Key = WheelKeysPair[static_cast<short>(extract_integer<WORD, 1>(MouseEvent.dwButtonState)) > 0? 1 : 0];
		return Key;
	}

	default:
		break;
	}

	return 0;
}

static unsigned int CalcKeyCode(INPUT_RECORD* rec, bool RealKey, bool* NotMacros)
{
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

		return KEY_NONE;
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

	if (!Char && os::is_dead_key(rec->Event.KeyEvent, console.GetKeyboardLayout()))
		return KEY_NONE;

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
	if ((CtrlState & (LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED)) == (LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED) && Char >= L' ')
	{
		IntKeyState.LeftCtrlPressed = IntKeyState.RightCtrlPressed = false;
		return Char;
	}

	if (KeyCode==VK_MENU)
		AltValue=0;

	if (in_closed_range(static_cast<unsigned>(VK_F1), KeyCode, static_cast<unsigned>(VK_F24)))
		return Modif + KEY_F1 + (KeyCode - VK_F1);

	if (IntKeyState.OnlyAltPressed())
	{
		if (!(CtrlState & ENHANCED_KEY))
		{
			static unsigned int const ScanCodes[]{ 82, 79, 80, 81, 75, 76, 77, 71, 72, 73 };

			for (const auto i: std::views::iota(0uz, std::size(ScanCodes)))
			{
				if (ScanCodes[i] != ScanCode)
					continue;

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
			console.ResetViewportPosition();
			return KEY_NONE;
		}
		else
			return Result;
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

	if (const auto OemKey = GetMappedCharacter(KeyCode, ScanCode))
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

TEST_CASE("keyboard.TranslateKeyToVK")
{
	static const struct
	{
		far_key_code Key;
		unsigned ExpectedVK;
	}
	Tests[]
	{
		{ KEY_ESC,           VK_ESCAPE, },
		{ KEY_SHIFTSPACE,    VK_SPACE, },
		{ KEY_ALTF1,         VK_F1, },
		{ KEY_NUMENTER,      VK_RETURN, },
		{ KEY_SHIFTNUMENTER, VK_RETURN, },
		{ KEY_NUMDEL,        VK_DELETE, },
		{ KEY_CTRLNUMDEL,    VK_DELETE, },
	};

	for (const auto& i: Tests)
	{
		INPUT_RECORD Record;
		TranslateKeyToVK(i.Key, &Record);
		REQUIRE(Record.EventType == KEY_EVENT);
		REQUIRE(Record.Event.KeyEvent.bKeyDown);
		REQUIRE(Record.Event.KeyEvent.wVirtualKeyCode == i.ExpectedVK);
	}
}

#endif
