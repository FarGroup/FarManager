#pragma once

/*
keyboard.hpp

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

#include "farqueue.hpp"

enum
{
	SKEY_VK_KEYS           = 0x40000000,
	SKEY_IDLE              = 0x80000000,
	SKEY_NOTMACROS         = 0x00000001,
};

#define MOUSE_ANY_BUTTON_PRESSED (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED|FROM_LEFT_2ND_BUTTON_PRESSED|FROM_LEFT_3RD_BUTTON_PRESSED|FROM_LEFT_4TH_BUTTON_PRESSED)

extern FarQueue<DWORD> *KeyQueue;

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
int KeyNameToKey(const wchar_t *Name);
bool InputRecordToText(const INPUT_RECORD *Rec, string &strKeyText);
BOOL KeyToText(int Key, string &strKeyText);
int InputRecordToKey(const INPUT_RECORD *Rec);
int KeyToInputRecord(int Key, INPUT_RECORD *Rec);
void ProcessKeyToInputRecord(int Key, unsigned int dwControlState, INPUT_RECORD *Rec);
void FarKeyToInputRecord(const FarKey& Key,INPUT_RECORD* Rec);
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false,bool ProcessMouse=false,bool AllowSynchro=true);
DWORD PeekInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=true);
bool IsRepeatedKey();
DWORD ShieldCalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=nullptr, bool ProcessCtrlCode=false);
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=nullptr, bool ProcessCtrlCode=false);
DWORD WaitKey(DWORD KeyWait=(DWORD)-1,DWORD delayMS=0,bool ExcludeMacro=true);
int SetFLockState(UINT vkKey, int State);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();
int CheckForEscSilent();
int ConfirmAbortOp();
