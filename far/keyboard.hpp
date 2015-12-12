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

enum
{
	SKEY_VK_KEYS           = 0x40000000,
	SKEY_IDLE              = 0x80000000,
	SKEY_NOTMACROS         = 0x00000001,
};

enum
{
	MOUSE_ANY_BUTTON_PRESSED =
		FROM_LEFT_1ST_BUTTON_PRESSED|
		FROM_LEFT_2ND_BUTTON_PRESSED|
		FROM_LEFT_3RD_BUTTON_PRESSED|
		FROM_LEFT_4TH_BUTTON_PRESSED|
		RIGHTMOST_BUTTON_PRESSED,
};

std::deque<DWORD>& KeyQueue();

struct FarKeyboardState {
	int AltPressed;
	int CtrlPressed;
	int ShiftPressed;
	int RightAltPressed;
	int RightCtrlPressed;
	int RightShiftPressed;
	DWORD MouseButtonState;
	DWORD PrevMouseButtonState;
	int PrevLButtonPressed;
	int PrevRButtonPressed;
	int PrevMButtonPressed;
	SHORT PrevMouseX;
	SHORT PrevMouseY;
	SHORT MouseX;
	SHORT MouseY;
	int PreMouseEventFlags;
	int MouseEventFlags;
	int ReturnAltValue;   // только что был ввод Alt-Цифира?
};

extern FarKeyboardState IntKeyState;

void InitKeysArray();
bool KeyToKeyLayoutCompare(int Key, int CompareKey);
int KeyToKeyLayout(int Key);

// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
DWORD IsMouseButtonPressed();
int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *rec=nullptr);
int KeyNameToKey(const string& Name);
bool InputRecordToText(const INPUT_RECORD *Rec, string &strKeyText);
bool KeyToText(int Key, string &strKeyText);
bool KeyToLocalizedText(int Key, string &strKeyText);
unsigned int InputRecordToKey(const INPUT_RECORD *Rec);
bool KeyToInputRecord(int Key, INPUT_RECORD *Rec);
void ProcessKeyToInputRecord(int Key, unsigned int dwControlState, INPUT_RECORD *Rec);
void FarKeyToInputRecord(const FarKey& Key,INPUT_RECORD* Rec);
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false,bool ProcessMouse=false,bool AllowSynchro=true);
DWORD GetInputRecordNoMacroArea(INPUT_RECORD *rec,bool AllowSynchro=true);
DWORD PeekInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=true);
bool IsRepeatedKey();
unsigned int ShieldCalcKeyCode(const INPUT_RECORD* rec, int RealKey, int *NotMacros = nullptr);
unsigned int CalcKeyCode(const INPUT_RECORD* rec, int RealKey, int *NotMacros = nullptr);
DWORD WaitKey(DWORD KeyWait=(DWORD)-1,DWORD delayMS=0,bool ExcludeMacro=true);
int SetFLockState(UINT vkKey, int State);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
bool CheckForEsc();
bool CheckForEscSilent();
bool ConfirmAbortOp();

#endif // KEYBOARD_HPP_63436F7A_609D_4E3B_8EF8_178B9829AB46
