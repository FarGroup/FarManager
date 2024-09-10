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

// Internal:
#include "farcolor.hpp"

// Platform:

// Common:
#include "common/2d/matrix.hpp"
#include "common/2d/rectangle.hpp"
#include "common/function_ref.hpp"
#include "common/singleton.hpp"

// External:

//----------------------------------------------------------------------------

struct FAR_CHAR_INFO;
struct FarColor;
enum class lng : int;
extern wchar_t BoxSymbols[];
extern point InitSize, CurSize;
extern int ScrX, ScrY;
extern int PrevScrX,PrevScrY;

struct console_mode
{
	DWORD Input{};
	DWORD Output{};
	DWORD Error{};
};

extern std::optional<console_mode> InitialConsoleMode;

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
	BS_X_B0,          // ░
	BS_X_B1,          // ▒
	BS_X_B2,          // ▓
	BS_V1,            // │
	BS_R_H1V1,        // ┤
	BS_R_H2V1,        // ╡
	BS_R_H1V2,        // ╢
	BS_RT_H1V2,       // ╖
	BS_RT_H2V1,       // ╕
	BS_R_H2V2,        // ╣
	BS_V2,            // ║
	BS_RT_H2V2,       // ╗
	BS_RB_H2V2,       // ╝
	BS_RB_H1V2,       // ╜
	BS_RB_H2V1,       // ╛
	BS_RT_H1V1,       // ┐
	BS_LB_H1V1,       // └
	BS_B_H1V1,        // ┴
	BS_T_H1V1,        // ┬
	BS_L_H1V1,        // ├
	BS_H1,            // ─
	BS_C_H1V1,        // ┼
	BS_L_H2V1,        // ╞
	BS_L_H1V2,        // ╟
	BS_LB_H2V2,       // ╚
	BS_LT_H2V2,       // ╔
	BS_B_H2V2,        // ╩
	BS_T_H2V2,        // ╦
	BS_L_H2V2,        // ╠
	BS_H2,            // ═
	BS_C_H2V2,        // ╬
	BS_B_H2V1,        // ╧
	BS_B_H1V2,        // ╨
	BS_T_H2V1,        // ╤
	BS_T_H1V2,        // ╥
	BS_LB_H1V2,       // ╙
	BS_LB_H2V1,       // ╘
	BS_LT_H2V1,       // ╒
	BS_LT_H1V2,       // ╓
	BS_C_H1V2,        // ╫
	BS_C_H2V1,        // ╪
	BS_RB_H1V1,       // ┘
	BS_LT_H1V1,       // ┌
	BS_X_DB,          // █
	BS_X_DC,          // ▄
	BS_X_DD,          // ▌
	BS_X_DE,          // ▐
	BS_X_DF,          // ▀
	BS_SPACE,         // ' '

	BS_COUNT
};

void ShowTime();

void InitConsole();
void CloseConsole();
void SetFarConsoleMode(bool SetsActiveBuffer = false);
bool ChangeConsoleMode(HANDLE ConsoleHandle, DWORD Mode);
void FlushInputBuffer();
void SetVideoMode();
void ChangeVideoMode(bool Maximize);
void ChangeVideoMode(int NumLines,int NumColumns);
void UpdateScreenSize();
void GenerateWINDOW_BUFFER_SIZE_EVENT();
void SaveConsoleWindowRect();
void RestoreConsoleWindowRect();

void GotoXY(int X,int Y);
int WhereX();
int WhereY();
void MoveCursor(point Point);
point GetCursorPos();
void SetCursorType(bool Visible, size_t Size);
void HideCursor();
void ShowCursor();
void SetInitialCursorType();
void GetCursorType(bool& Visible, size_t& Size);
void MoveRealCursor(int X,int Y);
size_t NumberOfEmptyLines(size_t Desired);

struct position_parser_state
{
	size_t StringIndex{};
	size_t VisualIndex{};

	function_ref<void(size_t, size_t)> signal{nullptr};
};

size_t string_pos_to_visual_pos(string_view Str, size_t StringPos, size_t TabSize, position_parser_state* SavedState = {});
size_t visual_pos_to_string_pos(string_view Str, size_t VisualPos, size_t TabSize, position_parser_state* SavedState = {});

size_t visual_string_length(string_view Str);

bool is_valid_surrogate_pair(string_view Str);
bool is_valid_surrogate_pair(wchar_t First, wchar_t Second);

void Text(point Where, const FarColor& Color, string_view Str);

size_t Text(string_view Str, size_t MaxWidth);
size_t Text(string_view Str);

size_t Text(wchar_t Char, size_t MaxWidth);
size_t Text(wchar_t Char);

size_t Text(lng MsgId, size_t MaxWidth);
size_t Text(lng MsgId);

size_t VText(string_view Str, size_t MaxWidth);
size_t VText(string_view Str);

size_t HiText(string_view Str, const FarColor& Color, size_t MaxWidth);
size_t HiText(string_view Str, const FarColor& Color);

size_t HiVText(string_view Str, const FarColor& Color, size_t MaxWidth);
size_t HiVText(string_view Str, const FarColor& Color);

void PutText(rectangle Where, const FAR_CHAR_INFO* Src);
void GetText(rectangle Where, matrix<FAR_CHAR_INFO>& Dest);

void SetScreen(rectangle Where, wchar_t Ch,const FarColor& Color);
void MakeShadow(rectangle Where);
void DropShadow(rectangle Where);
void SetColor(int Color);
void SetColor(PaletteColors Color);
void SetColor(const FarColor& Color);
void SetRealColor(const FarColor& Color);
const FarColor& GetColor();

void Box(rectangle Where, const FarColor& Color,int Type);
bool ScrollBarRequired(size_t Length, unsigned long long ItemsCount);
bool ScrollBar(size_t X1, size_t Y1, size_t Length, unsigned long long TopItem, unsigned long long ItemsCount);
bool ScrollBarEx(size_t X1, size_t Y1, size_t Length, unsigned long long Start, unsigned long long End, unsigned long long Size);

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

size_t HiStrlen(string_view Str);
size_t HiFindRealPos(string_view Str, size_t Pos);
string HiText2Str(string_view Str, size_t* HotkeyVisualPos = {});
bool HiTextHotkey(string_view Str, wchar_t& Hotkey, size_t* HotkeyVisualPos = {});

namespace inplace
{
	void remove_highlight(string& Str);
	void escape_ampersands(string& Str);
}

string remove_highlight(string Str);
string remove_highlight(string_view Str);
string escape_ampersands(string Str);
string escape_ampersands(string_view Str);

bool IsConsoleFullscreen();
bool IsConsoleViewportSizeChanged();

void SaveNonMaximisedBufferSize(point const& Size);
point GetNonMaximisedBufferSize();

void AdjustConsoleScreenBufferSize();

void SetPalette();

class consoleicons: public singleton<consoleicons>
{
	IMPLEMENTS_SINGLETON;

public:
	void update_icon();
	void set_icon(int IconId);
	void restore_icon();

	size_t size() const;

private:
	consoleicons() = default;
	~consoleicons() = default;

	struct icon
	{
		bool IsBig;
		std::optional<HICON> InitialIcon;
	};

	icon m_Large{true};
	icon m_Small{false};
};

size_t ConsoleChoice(string_view Message, string_view Choices, size_t Default, function_ref<void()> MessagePrinter);
bool ConsoleYesNo(string_view Message, bool Default, function_ref<void()> MessagePrinter);

#endif // INTERF_HPP_A91E1A99_C78E_41EC_B0F8_5C35A6C99116
