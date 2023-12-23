#ifndef KEYBOARD_HPP_63436F7A_609D_4E3B_8EF8_178B9829AB46
#define KEYBOARD_HPP_63436F7A_609D_4E3B_8EF8_178B9829AB46
#pragma once

/*
keyboard.hpp

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

// Internal:

// Platform:

// Common:
#include "common/2d/point.hpp"
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

struct FarKey;

enum
{
	MOUSE_ANY_BUTTON_PRESSED =
		FROM_LEFT_1ST_BUTTON_PRESSED|
		FROM_LEFT_2ND_BUTTON_PRESSED|
		FROM_LEFT_3RD_BUTTON_PRESSED|
		FROM_LEFT_4TH_BUTTON_PRESSED|
		RIGHTMOST_BUTTON_PRESSED,
};

void ClearKeyQueue();
void main_loop_process_messages();

struct FarKeyboardState
{
	bool LeftAltPressed;
	bool LeftCtrlPressed;
	bool LeftShiftPressed;
	bool RightAltPressed;
	bool RightCtrlPressed;
	bool RightShiftPressed;
	DWORD MouseButtonState;
	DWORD PrevMouseButtonState;
	bool PrevLButtonPressed;
	bool PrevRButtonPressed;
	bool PrevMButtonPressed;
	point MousePrevPos;
	point MousePos;
	int PreMouseEventFlags;
	int MouseEventFlags;
	bool ReturnAltValue;   // только что был ввод Alt-Цифира?

	bool AltPressed() const { return LeftAltPressed || RightAltPressed; }
	bool CtrlPressed() const { return LeftCtrlPressed || RightCtrlPressed; }
	bool ShiftPressed() const { return LeftShiftPressed || RightShiftPressed; }

	bool OnlyAltPressed() const { return !CtrlPressed() && AltPressed() &&  !ShiftPressed(); }
	bool OnlyCtrlPressed() const { return CtrlPressed() && !AltPressed() && !ShiftPressed(); }
	bool OnlyShiftPressed() const { return !CtrlPressed() && !AltPressed() && ShiftPressed(); }

	bool OnlyAltShiftPressed() const { return !CtrlPressed() && AltPressed() && ShiftPressed(); }
	bool OnlyCtrlShiftPressed() const { return CtrlPressed() && !AltPressed() && ShiftPressed(); }
	bool OnlyCtrlAltPressed() const { return CtrlPressed() && AltPressed() && !ShiftPressed(); }
	bool OnlyCtrlAltShiftPressed() const { return CtrlPressed() && AltPressed() && ShiftPressed(); }

	bool NonePressed() const { return !CtrlPressed() && !AltPressed() && !ShiftPressed(); }
};

extern FarKeyboardState IntKeyState;

void InitKeysArray();
bool KeyToKeyLayoutCompare(int Key, int CompareKey);
int KeyToKeyLayout(int Key);

class keyboard_repeat_emulation
{
public:
	keyboard_repeat_emulation();
	~keyboard_repeat_emulation();

	void reset() const;
	bool signaled() const;

private:
	class implementation;
	std::unique_ptr<implementation> m_Impl;
};

// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
DWORD IsMouseButtonPressed();
bool while_mouse_button_pressed(function_ref<bool(DWORD)> Action);
bool IsMouseButtonEvent(DWORD EventFlags);
int get_wheel_threshold(int ConfigValue);
int get_wheel_scroll_lines(int ConfigValue);
int get_wheel_scroll_chars(int ConfigValue);
int TranslateKeyToVK(int Key, INPUT_RECORD* Rec = nullptr);
int KeyNameToKey(string_view Name);
string InputRecordToText(const INPUT_RECORD *Rec);
string KeyToText(unsigned int Key);
string KeyToLocalizedText(unsigned int Key);
string KeysListToLocalizedText(std::span<unsigned int const> Keys);
string KeysToLocalizedText(auto const... Keys)
{
	return KeysListToLocalizedText({{ Keys... }});
}
unsigned int InputRecordToKey(const INPUT_RECORD *Rec);
bool KeyToInputRecord(int Key, INPUT_RECORD *Rec);
void FarKeyToInputRecord(const FarKey& Key,INPUT_RECORD* Rec);
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false,bool ProcessMouse=false);
DWORD GetInputRecordNoMacroArea(INPUT_RECORD *rec);
DWORD PeekInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=true);
bool IsRepeatedKey();
DWORD WaitKey(DWORD KeyWait = static_cast<DWORD>(-1), std::optional<std::chrono::milliseconds> Timeout = {}, bool ExcludeMacro = true);
int SetFLockState(unsigned vkKey, int State);
bool WriteInput(int Key);
bool IsModifKey(DWORD Key);
bool IsInternalKeyReal(unsigned int Key);
bool IsCharKey(unsigned int Key);
bool CheckForEscSilent();

void wakeup_for_clock(bool Value);
void wakeup_for_screensaver(bool Value);

#endif // KEYBOARD_HPP_63436F7A_609D_4E3B_8EF8_178B9829AB46
