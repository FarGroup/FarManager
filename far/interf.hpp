#ifndef INTERF_HPP_A91E1A99_C78E_41EC_B0F8_5C35A6C99116
#define INTERF_HPP_A91E1A99_C78E_41EC_B0F8_5C35A6C99116
#pragma once

/*
interf.hpp

Консольные функции ввода-вывода
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

#include "farcolor.hpp"

#include "common/2d/matrix.hpp"
#include "common/2d/rectangle.hpp"
#include "common/singleton.hpp"

struct FAR_CHAR_INFO;
struct FarColor;
enum class lng : int;
extern WCHAR BoxSymbols[];
extern COORD InitSize, CurSize;
extern SHORT ScrX,ScrY;
extern SHORT PrevScrX,PrevScrY;
extern DWORD InitialConsoleMode;

// типы рамок
enum
{
	NO_BOX,
	SINGLE_BOX,
	SHORT_SINGLE_BOX,
	DOUBLE_BOX,
	SHORT_DOUBLE_BOX
};

enum BOX_DEF_SYMBOLS
{
	BS_X_B0,          // 0xB0
	BS_X_B1,          // 0xB1
	BS_X_B2,          // 0xB2
	BS_V1,            // 0xB3
	BS_R_H1V1,        // 0xB4
	BS_R_H2V1,        // 0xB5
	BS_R_H1V2,        // 0xB6
	BS_RT_H1V2,       // 0xB7
	BS_RT_H2V1,       // 0xB8
	BS_R_H2V2,        // 0xB9
	BS_V2,            // 0xBA
	BS_RT_H2V2,       // 0xBB
	BS_RB_H2V2,       // 0xBC
	BS_RB_H1V2,       // 0xBD
	BS_RB_H2V1,       // 0xBE
	BS_RT_H1V1,       // 0xBF
	BS_LB_H1V1,       // 0xС0
	BS_B_H1V1,        // 0xС1
	BS_T_H1V1,        // 0xС2
	BS_L_H1V1,        // 0xС3
	BS_H1,            // 0xС4
	BS_C_H1V1,        // 0xС5
	BS_L_H2V1,        // 0xС6
	BS_L_H1V2,        // 0xС7
	BS_LB_H2V2,       // 0xС8
	BS_LT_H2V2,       // 0xС9
	BS_B_H2V2,        // 0xСA
	BS_T_H2V2,        // 0xСB
	BS_L_H2V2,        // 0xСC
	BS_H2,            // 0xСD
	BS_C_H2V2,        // 0xСE
	BS_B_H2V1,        // 0xСF
	BS_B_H1V2,        // 0xD0
	BS_T_H2V1,        // 0xD1
	BS_T_H1V2,        // 0xD2
	BS_LB_H1V2,       // 0xD3
	BS_LB_H2V1,       // 0xD4
	BS_LT_H2V1,       // 0xD5
	BS_LT_H1V2,       // 0xD6
	BS_C_H1V2,        // 0xD7
	BS_C_H2V1,        // 0xD8
	BS_RB_H1V1,       // 0xD9
	BS_LT_H1V1,       // 0xDA
	BS_X_DB,          // 0xDB
	BS_X_DC,          // 0xDC
	BS_X_DD,          // 0xDD
	BS_X_DE,          // 0xDE
	BS_X_DF,          // 0xDF
	BS_SPACE,         // 0x20

	BS_COUNT
};

void ShowTime();
void ShowTimeInBackground();

void InitConsole();
void CloseConsole();
void SetFarConsoleMode(bool SetsActiveBuffer = false);
void ChangeConsoleMode(HANDLE ConsoleHandle, DWORD Mode);
void FlushInputBuffer();
void SetVideoMode();
void ChangeVideoMode(bool Maximize);
void ChangeVideoMode(int NumLines,int NumColumns);
void UpdateScreenSize();
void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx=-1, int Sy=-1);
void SaveConsoleWindowRect();
void RestoreConsoleWindowRect();

void GotoXY(int X,int Y);
int WhereX();
int WhereY();
void MoveCursor(point Point);
point GetCursorPos();
void SetCursorType(bool Visible, DWORD Size);
void SetInitialCursorType();
void GetCursorType(bool& Visible, DWORD& Size);
void MoveRealCursor(int X,int Y);
void GetRealCursorPos(SHORT& X,SHORT& Y);
void ScrollScreen(int Count);

void Text(point Where, const FarColor& Color, string_view Str);

void Text(string_view Str);
inline void Text(wchar_t const c) { return Text({ &c, 1 }); }

void Text(lng MsgId);

void VText(string_view Str);

void HiText(const string& Str,const FarColor& HiColor,int isVertText=0);
void PutText(rectangle Where, const FAR_CHAR_INFO* Src);
void GetText(rectangle Where, matrix<FAR_CHAR_INFO>& Dest);

void BoxText(string_view Str, bool IsVert = false);
inline void BoxText(wchar_t const Chr) { return BoxText({ &Chr, 1 }, false); }

void SetScreen(rectangle Where, wchar_t Ch,const FarColor& Color);
void MakeShadow(rectangle Where);
void ChangeBlockColor(rectangle Where, const FarColor& Color);
void SetColor(int Color);
void SetColor(PaletteColors Color);
void SetColor(const FarColor& Color);
void SetRealColor(const FarColor& Color);
void ClearScreen(const FarColor& Color);
const FarColor& GetColor();

void Box(rectangle Where, const FarColor& Color,int Type);
bool ScrollBarRequired(UINT Length, unsigned long long ItemsCount);
bool ScrollBarEx(UINT X1, UINT Y1, UINT Length, unsigned long long TopItem, unsigned long long ItemsCount);
bool ScrollBarEx3(UINT X1, UINT Y1, UINT Length, unsigned long long Start, unsigned long long End, unsigned long long Size);

enum class line_type
{
	h1,
	h2,
	h1_to_none,
	h2_to_none,
	h1_to_v1,
	h1_to_v2,
	h2_to_v1,
	h2_to_v2,
	h_user,

	v1,
	v2,
	v1_to_none,
	v2_to_none,
	v1_to_h1,
	v1_to_h2,
	v2_to_h1,
	v2_to_h2,
	v_user,

	count
};

string MakeLine(int Length, line_type Type = line_type::h1_to_v2, string_view UserLine = {});
void DrawLine(int Length, line_type Type, string_view UserLine = {});

string make_progressbar(size_t Size, size_t Percent, bool ShowPercent, bool PropagateToTasbkar);

void fix_coordinates(rectangle& Where);

size_t HiStrlen(const string& Str);
int HiFindRealPos(const string& Str, int Pos, bool ShowAmp);
int HiFindNextVisualPos(const string& Str, int Pos, int Direct);
string HiText2Str(const string& Str);
void RemoveHighlights(string& Str);

bool IsConsoleFullscreen();
bool IsConsoleSizeChanged();

void SaveNonMaximisedBufferSize(const COORD& Size);
COORD GetNonMaximisedBufferSize();

void AdjustConsoleScreenBufferSize(bool TransitionFromFullScreen);

class consoleicons: public singleton<consoleicons>
{
	IMPLEMENTS_SINGLETON;

public:
	void setFarIcons();
	void restorePreviousIcons();

private:
	consoleicons() = default;

	struct icon
	{
		bool IsBig;
		HICON Icon;
		HICON PreviousIcon;
		bool Changed;
	};

	icon m_Large{true};
	icon m_Small{false};

	bool m_Loaded{};
};

#endif // INTERF_HPP_A91E1A99_C78E_41EC_B0F8_5C35A6C99116
