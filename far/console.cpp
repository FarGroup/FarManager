/*
console.cpp

Console functions
*/
/*
Copyright © 2010 Far Group
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
#include "console.hpp"

// Internal:
#include "imports.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "strmix.hpp"
#include "exception.hpp"
#include "palette.hpp"
#include "encoding.hpp"
#include "char_width.hpp"
#include "log.hpp"

// Platform:
#include "platform.version.hpp"

// Common:
#include "common.hpp"
#include "common/2d/algorithm.hpp"
#include "common/algorithm.hpp"
#include "common/enum_substrings.hpp"
#include "common/from_string.hpp"
#include "common/io.hpp"
#include "common/scope_exit.hpp"
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#define ESC L"\u001b"
#define CSI ESC L"["
#define ST ESC L"\\"
#define OSC(Command) ESC L"]" Command ST ""sv
#define ANSISYSSC CSI L"s"
#define ANSISYSRC CSI L"u"

static bool sWindowMode;
static bool sEnableVirtualTerminal;

constexpr auto bad_char_replacement = L' ';

wchar_t ReplaceControlCharacter(wchar_t const Char)
{
	switch (Char)
	{
	// C0
	case L'\u0000': return L' '; // space
	case L'\u0001': return L'☺'; // white smiling face
	case L'\u0002': return L'☻'; // black smiling face
	case L'\u0003': return L'♥'; // black heart suit
	case L'\u0004': return L'♦'; // black diamond suit
	case L'\u0005': return L'♣'; // black club suit
	case L'\u0006': return L'♠'; // black spade suit
	case L'\u0007': return L'•'; // bullet
	case L'\u0008': return L'◘'; // inverse bullet
	case L'\u0009': return L'○'; // white circle
	case L'\u000A': return L'◙'; // inverse white circle
	case L'\u000B': return L'♂'; // male sign
	case L'\u000C': return L'♀'; // female sign
	case L'\u000D': return L'♪'; // eighth note
	case L'\u000E': return L'♫'; // beamed eighth notes
	case L'\u000F': return L'☼'; // white sun with rays
	case L'\u0010': return L'►'; // black right - pointing pointer
	case L'\u0011': return L'◄'; // black left - pointing pointer
	case L'\u0012': return L'↕'; // up down arrow
	case L'\u0013': return L'‼'; // double exclamation mark
	case L'\u0014': return L'¶'; // pilcrow sign
	case L'\u0015': return L'§'; // section sign
	case L'\u0016': return L'▬'; // black rectangle
	case L'\u0017': return L'↨'; // up down arrow with base
	case L'\u0018': return L'↑'; // upwards arrow
	case L'\u0019': return L'↓'; // downwards arrow
	case L'\u001A': return L'→'; // rightwards arrow
	case L'\u001B': return L'←'; // leftwards arrow
	case L'\u001C': return L'∟'; // right angle
	case L'\u001D': return L'↔'; // left right arrow
	case L'\u001E': return L'▲'; // black up - pointing triangle
	case L'\u001F': return L'▼'; // black down - pointing triangle
	case L'\u007F': return L'⌂'; // house

	// C1
	// These are considered control characters too now.
	// Unlike C0, it is unclear what glyphs to use, so just replace with FFFD for now.
	case L'\u0080':
	case L'\u0081':
	case L'\u0082':
	case L'\u0083':
	case L'\u0084':
	case L'\u0085':
	case L'\u0086':
	case L'\u0087':
	case L'\u0088':
	case L'\u0089':
	case L'\u008A':
	case L'\u008B':
	case L'\u008C':
	case L'\u008D':
	case L'\u008E':
	case L'\u008F':
	case L'\u0090':
	case L'\u0091':
	case L'\u0092':
	case L'\u0093':
	case L'\u0094':
	case L'\u0095':
	case L'\u0096':
	case L'\u0097':
	case L'\u0098':
	case L'\u0099':
	case L'\u009A':
	case L'\u009B':
	case L'\u009C':
	case L'\u009D':
	case L'\u009E':
	case L'\u009F': return encoding::replace_char;

	default:   return Char;
	}
}

static bool sanitise_dbsc_pair(FAR_CHAR_INFO& First, FAR_CHAR_INFO& Second)
{
	const auto
		IsFirst = flags::check_one(First.Attributes.Flags, COMMON_LVB_LEADING_BYTE),
		IsSecond = flags::check_one(Second.Attributes.Flags, COMMON_LVB_TRAILING_BYTE);

	if (!IsFirst && !IsSecond)
	{
		// Not DBSC, awesome
		return false;
	}

	flags::clear(First.Attributes.Flags, COMMON_LVB_LEADING_BYTE);
	flags::clear(Second.Attributes.Flags, COMMON_LVB_TRAILING_BYTE);

	if (IsFirst && IsSecond && First == Second)
	{
		// Valid DBSC, awesome
		flags::set(First.Attributes.Flags, COMMON_LVB_LEADING_BYTE);
		flags::set(Second.Attributes.Flags, COMMON_LVB_TRAILING_BYTE);

		return false;
	}

	if (IsFirst)
		First.Char = bad_char_replacement;

	if (IsSecond)
		Second.Char = bad_char_replacement;

	return true;
}

static bool sanitise_surrogate_pair(FAR_CHAR_INFO& First, FAR_CHAR_INFO& Second)
{
	const auto
		IsFirst = encoding::utf16::is_high_surrogate(First.Char),
		IsSecond = encoding::utf16::is_low_surrogate(Second.Char);

	if (!IsFirst && !IsSecond)
	{
		// Not surrogate, awesome
		return false;
	}

	if (IsFirst && IsSecond && First.Attributes == Second.Attributes)
	{
		// Valid surrogate, awesome
		return false;
	}

	if (IsFirst)
		First.Char = bad_char_replacement;

	if (IsSecond)
		Second.Char = bad_char_replacement;

	return true;
}

bool sanitise_pair(FAR_CHAR_INFO& First, FAR_CHAR_INFO& Second)
{
	return sanitise_dbsc_pair(First, Second) || sanitise_surrogate_pair(First, Second);
}

bool get_console_screen_buffer_info(HANDLE ConsoleOutput, CONSOLE_SCREEN_BUFFER_INFO* ConsoleScreenBufferInfo)
{
	if (!GetConsoleScreenBufferInfo(ConsoleOutput, ConsoleScreenBufferInfo))
	{
		LOGERROR(L"GetConsoleScreenBufferInfo(): {}"sv, os::last_error());
		return false;
	}

	const auto& Window = ConsoleScreenBufferInfo->srWindow;

	// Mantis#3919: Windows 10 is a PITA
	if (Window.Left > Window.Right || Window.Top > Window.Bottom)
	{
		LOGERROR(L"Console window state is broken, trying to repair"sv);

		auto NewWindow = Window;

		if (Window.Left > Window.Right)
		{
			NewWindow.Left = 0;
			NewWindow.Right = std::min(ConsoleScreenBufferInfo->dwMaximumWindowSize.X - 1, ConsoleScreenBufferInfo->dwSize.X - 1);
		}

		// https://forum.farmanager.com/viewtopic.php?p=170779#p170779
		if (Window.Top > Window.Bottom)
		{
			NewWindow.Bottom = ConsoleScreenBufferInfo->dwSize.Y - 1;
			NewWindow.Top = std::clamp(NewWindow.Top, short{}, NewWindow.Bottom);
		}

		if (!SetConsoleWindowInfo(ConsoleOutput, true, &NewWindow))
		{
			LOGERROR(L"SetConsoleWindowInfo(): {}"sv, os::last_error());
			return false;
		}

		if (!GetConsoleScreenBufferInfo(ConsoleOutput, ConsoleScreenBufferInfo))
		{
			LOGERROR(L"GetConsoleScreenBufferInfo(): {}"sv, os::last_error());
			return false;
		}
	}

	return true;
}

static COORD make_coord(point const& Point)
{
	return
	{
		static_cast<short>(Point.x),
		static_cast<short>(Point.y)
	};
}

static SMALL_RECT make_rect(rectangle const& Rectangle)
{
	return
	{
		static_cast<short>(Rectangle.left),
		static_cast<short>(Rectangle.top),
		static_cast<short>(Rectangle.right),
		static_cast<short>(Rectangle.bottom)
	};
}

static short GetDelta(CONSOLE_SCREEN_BUFFER_INFO const& csbi)
{
	return csbi.dwSize.Y - (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

namespace console_detail
{
	// пишем/читаем порциями по 32 K, иначе проблемы.
	const unsigned int MAXSIZE = 32768;

	class external_console
	{
	public:
		NONCOPYABLE(external_console);
		external_console() = default;

		struct ModuleImports
		{
		private:
			os::rtdl::module m_Module{ L"extendedconsole.dll"sv };

		public:
#define DECLARE_IMPORT_FUNCTION(name, ...) os::rtdl::function_pointer<__VA_ARGS__> p ## name{ m_Module, #name }

			DECLARE_IMPORT_FUNCTION(ReadOutput,           BOOL WINAPI(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion));
			DECLARE_IMPORT_FUNCTION(WriteOutput,          BOOL WINAPI(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion));
			DECLARE_IMPORT_FUNCTION(Commit,               BOOL WINAPI());
			DECLARE_IMPORT_FUNCTION(GetTextAttributes,    BOOL WINAPI(FarColor* Attributes));
			DECLARE_IMPORT_FUNCTION(SetTextAttributes,    BOOL WINAPI(const FarColor* Attributes));
			DECLARE_IMPORT_FUNCTION(ClearExtraRegions,    BOOL WINAPI(const FarColor* Color, int Mode));

#undef DECLARE_IMPORT_FUNCTION
		}
		Imports;
	};

	enum
	{
		BufferSize = 8192
	};

	static bool is_redirected(int const HandleType)
	{
		DWORD Mode;
		return !GetConsoleMode(GetStdHandle(HandleType), &Mode);
	}

	class consolebuf final: public std::wstreambuf
	{
	public:
		NONCOPYABLE(consolebuf);

		explicit(false) consolebuf(int const Type):
			m_Type(Type),
			m_Redirected(is_redirected(Type)),
			m_InBuffer(BufferSize, {}),
			m_OutBuffer(BufferSize, {})
		{
			setg(m_InBuffer.data(), m_InBuffer.data() + m_InBuffer.size(), m_InBuffer.data() + m_InBuffer.size());
			setp(m_OutBuffer.data(), m_OutBuffer.data() + m_OutBuffer.size());
		}

		void color(const FarColor& Color)
		{
			m_Colour = Color;
		}

protected:
		int_type underflow() override
		{
			const auto Size = read(m_InBuffer);
			if (!Size)
				return traits_type::eof();

			setg(m_InBuffer.data(), m_InBuffer.data(), m_InBuffer.data() + Size);
			return m_InBuffer[0];
		}

		int_type overflow(int_type Ch) override
		{
			write({ pbase(), static_cast<size_t>(pptr() - pbase()) });

			setp(m_OutBuffer.data(), m_OutBuffer.data() + m_OutBuffer.size());

			if (traits_type::eq_int_type(Ch, traits_type::eof()))
			{
				flush();
			}
			else
			{
				sputc(Ch);
			}

			return 0;
		}

		int sync() override
		{
			overflow(traits_type::eof());
			return 0;
		}

	private:
		size_t read(std::span<wchar_t> const Str) const
		{
			if (m_Redirected)
			{
				DWORD BytesRead;
				if (!ReadFile(GetStdHandle(m_Type), Str.data(), static_cast<DWORD>(Str.size() * sizeof(wchar_t)), &BytesRead, {}))
					throw far_fatal_exception(L"File read error"sv);

				return BytesRead / sizeof(wchar_t);
			}

			size_t Size;
			if (!::console.Read(Str, Size))
				throw far_fatal_exception(L"Console read error"sv);

			return Size;
		}

		void write(string_view const Str) const
		{
			if (Str.empty())
				return;

			if (m_Redirected)
			{
				const auto write = [&](void const* Data, size_t const Size)
				{
					DWORD BytesWritten;
					if (!WriteFile(GetStdHandle(m_Type), Data, static_cast<DWORD>(Size), &BytesWritten, {}))
						throw far_fatal_exception(L"File write error"sv);
				};

				if constexpr ([[maybe_unused]] constexpr auto UseUtf8Output = true)
				{
					const auto Utf8Str = encoding::utf8::get_bytes(Str);
					write(Utf8Str.data(), Utf8Str.size());
				}
				else
				{
					write(Str.data(), Str.size() * sizeof(wchar_t));
				}

				return;
			}

			FarColor CurrentColor{};
			const auto ChangeColour = m_Colour && ::console.GetTextAttributes(CurrentColor);

			if (ChangeColour)
			{
				CurrentColor = colors::unresolve_defaults(CurrentColor);
				::console.SetTextAttributes(colors::merge(CurrentColor, *m_Colour));
			}

			SCOPE_EXIT{ if (ChangeColour) ::console.SetTextAttributes(CurrentColor); };

			if (!::console.Write(Str))
				throw far_fatal_exception(L"Console write error"sv);
		}

		void flush() const
		{
			if (m_Redirected)
			{
				FlushFileBuffers(GetStdHandle(m_Type));
				return;
			}

			::console.Commit();
		}

		int m_Type;
		bool m_Redirected;
		string m_InBuffer, m_OutBuffer;
		std::optional<FarColor> m_Colour;
	};

	class stream_buffer_overrider
	{
	public:
		NONCOPYABLE(stream_buffer_overrider);

		stream_buffer_overrider(std::wios& Stream, int const HandleType, std::optional<FarColor> const Color = {}):
			m_Buf(HandleType),
			m_Override(Stream, m_Buf)
		{
			if (Color)
				m_Buf.color(*Color);
		}

	private:
		consolebuf m_Buf;
		io::wstreambuf_override m_Override;
	};

	class console_detail::console::stream_buffers_overrider
	{
	public:
		NONCOPYABLE(stream_buffers_overrider);

		stream_buffers_overrider():
			m_ErrorColor(fg_color(F_LIGHTRED)),
			m_In(std::wcin, STD_INPUT_HANDLE),
			m_Out(std::wcout, STD_OUTPUT_HANDLE),
			m_Err(std::wcerr, STD_ERROR_HANDLE, m_ErrorColor),
			m_Log(std::wclog, STD_ERROR_HANDLE, m_ErrorColor)
		{
		}

	private:
		static FarColor fg_color(int const NtColor)
		{
			auto Color = colors::NtColorToFarColor(NtColor);
			Color.SetBgDefault();
			return Color;
		}

		FarColor m_ErrorColor;
		stream_buffer_overrider m_In, m_Out, m_Err, m_Log;
	};

	static nifty_counter::buffer<external_console> Storage;
	static auto& ExternalConsole = reinterpret_cast<external_console&>(Storage);

	class hide_cursor
	{
	public:
		NONCOPYABLE(hide_cursor);

		hide_cursor():
			m_Restore(::console.GetCursorInfo(m_CursorInfo) && ::console.SetCursorInfo({ m_CursorInfo.dwSize }))
		{
		}

		~hide_cursor()
		{
			if (m_Restore)
				(void)::console.SetCursorInfo(m_CursorInfo);
		}

		CONSOLE_CURSOR_INFO m_CursorInfo{};
		bool m_Restore{};
	};

	class scoped_vt_output
	{
	public:
		NONCOPYABLE(scoped_vt_output);

		scoped_vt_output():
			m_ConsoleMode(::console.UpdateMode(::console.GetOutputHandle(), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING, 0))
		{
		}

		~scoped_vt_output()
		{
			if (m_ConsoleMode)
				::console.SetMode(::console.GetOutputHandle(), *m_ConsoleMode);
		}

		explicit operator bool() const
		{
			return m_ConsoleMode.has_value();
		}

	private:
		std::optional<DWORD> m_ConsoleMode;
	};

	class scoped_vt_input
	{
	public:
		NONCOPYABLE(scoped_vt_input);

		scoped_vt_input():
			m_ConsoleMode(::console.UpdateMode(::console.GetInputHandle(), ENABLE_VIRTUAL_TERMINAL_INPUT, ENABLE_LINE_INPUT))
		{
		}

		~scoped_vt_input()
		{
			if (m_ConsoleMode)
				::console.SetMode(::console.GetInputHandle(), *m_ConsoleMode);
		}

		explicit operator bool() const
		{
			return m_ConsoleMode.has_value();
		}

	private:
		std::optional<DWORD> m_ConsoleMode;
	};

	static string query_vt(string_view const Command)
	{
		// A VT query works as follows:
		// - We cast an unpronounceable spell into the output stream.
		// - If the terminal recognizes the spell, it conjures
		//   an equally unpronounceable answer into the input stream.
		//   Notably, it takes its time at that and answers asynchronously.
		// - If the terminal does not recognize the spell, it does not
		//   burden itself with explanations and stays silent.

		// A classic, timeless design, a pinnacle of 70's or whatever.

		// The only problem with it is that there is no way to tell what will happen.
		// You conjure a dodgy incantation, which may or may not be supported, and pray.
		// Maybe the response comes immediately.
		// Maybe later.
		// Maybe never.

		// 🤦

		// To make sure that we do not deadlock ourselves here we prepend & append
		// this dummy DA command that always works (since it was in the initial WT release),
		// so that the response is always non-empty and we know exactly where it starts and ends.
		// In other words:
		// - <attributes>[the response we are actually after]<attributes>: yay.
		// - <attributes><attributes>: nay, the request is unsupported.

		// Ah, and since it is input stream, the user can type any rubbish into it at the same time.
		// Fortunately, it seems that user input is queued before and/or after the responses,
		// but does not interlace with them.

		// We also need to enable VT input, otherwise it will only work in a real console.
		// Are you not entertained?

		scoped_vt_input const VtInput;
		if (!VtInput)
			throw far_exception(L"scoped_vt_input"sv);

		const auto Dummy = CSI L"0c"sv;

		if (!::console.Write(concat(Dummy, Command, Dummy)))
			throw far_exception(L"WriteConsole"sv);

		string Response;

		std::optional<size_t>
			FirstTokenPrefixPos,
			FirstTokenSuffixPos,
			SecondTokenPrefixPos,
			SecondTokenSuffixPos;

		const auto
			TokenPrefix = CSI "?"sv,
			TokenSuffix = L"c"sv;

		while (!SecondTokenSuffixPos)
		{
			wchar_t ResponseBuffer[8192];
			size_t ResponseSize;

			if (!::console.Read(ResponseBuffer, ResponseSize))
				throw far_exception(L"ReadConsole"sv);

			Response.append(ResponseBuffer, ResponseSize);

			if (!FirstTokenPrefixPos)
				if (const auto Pos = Response.find(TokenPrefix); Pos != Response.npos)
					FirstTokenPrefixPos = Pos;

			if (FirstTokenPrefixPos && !FirstTokenSuffixPos)
				if (const auto Pos = Response.find(TokenSuffix, *FirstTokenPrefixPos + TokenPrefix.size()); Pos != Response.npos)
					FirstTokenSuffixPos = Pos;

			if (FirstTokenSuffixPos && !SecondTokenPrefixPos)
				if (const auto Pos = Response.find(TokenPrefix, *FirstTokenSuffixPos + TokenSuffix.size()); Pos != Response.npos)
					SecondTokenPrefixPos = Pos;

			if (SecondTokenPrefixPos && !SecondTokenSuffixPos)
				if (const auto Pos = Response.find(TokenSuffix, *SecondTokenPrefixPos + TokenPrefix.size()); Pos != Response.npos)
					SecondTokenSuffixPos = Pos;
		}

		Response.resize(*SecondTokenPrefixPos);
		Response.erase(0, *FirstTokenSuffixPos + TokenSuffix.size());

		return Response;
	}

	console::console():
		m_OriginalInputHandle(GetStdHandle(STD_INPUT_HANDLE)),
		m_StreamBuffersOverrider(std::make_unique<stream_buffers_overrider>())
	{
		placement::construct(ExternalConsole);
	}

	console::~console()
	{
		placement::destruct(ExternalConsole);
	}

	bool console::Allocate() const
	{
		if (!AllocConsole())
		{
			LOGERROR(L"AllocConsole(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::Free() const
	{
		if (!FreeConsole())
		{
			LOGERROR(L"FreeConsole(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	HANDLE console::GetInputHandle() const
	{
		return GetStdHandle(STD_INPUT_HANDLE);
	}

	HANDLE console::GetOutputHandle() const
	{
		return GetStdHandle(STD_OUTPUT_HANDLE);
	}

	HANDLE console::GetErrorHandle() const
	{
		return GetStdHandle(STD_ERROR_HANDLE);
	}

	HANDLE console::GetOriginalInputHandle() const
	{
		return m_OriginalInputHandle;
	}

	static bool is_pseudo_console(HWND const Window)
	{
		wchar_t ClassName[MAX_PATH];
		const auto Size = GetClassName(Window, ClassName, static_cast<int>(std::size(ClassName)));
		return string_view(ClassName, Size) == L"PseudoConsoleWindow"sv;
	}

	HWND console::GetWindow() const
	{
		const auto Window = GetConsoleWindow();

		if (is_pseudo_console(Window))
		{
			if (const auto Owner = ::GetWindow(Window, GW_OWNER))
				return Owner;
		}

		return Window;
	}

	bool console::GetSize(point& Size) const
	{
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		if (sWindowMode)
		{
			const auto& Window = ConsoleScreenBufferInfo.srWindow;

			Size =
			{
				Window.Right - Window.Left + 1,
				Window.Bottom - Window.Top + 1
			};
		}
		else
		{
			Size = ConsoleScreenBufferInfo.dwSize;
		}

		return true;
	}

	bool console::SetSize(point const Size) const
	{
		if (!sWindowMode)
			return SetScreenBufferSize(Size);

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		csbi.srWindow.Left = 0;
		csbi.srWindow.Right = Size.x - 1;
		csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
		csbi.srWindow.Top = csbi.srWindow.Bottom - (Size.y - 1);
		point WindowCoord{ csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1 };
		if (WindowCoord.x > csbi.dwSize.X || WindowCoord.y > csbi.dwSize.Y)
		{
			WindowCoord.x = std::max(WindowCoord.x, static_cast<int>(csbi.dwSize.X));
			WindowCoord.y = std::max(WindowCoord.y, static_cast<int>(csbi.dwSize.Y));
			if (!SetScreenBufferSize(WindowCoord))
				return false;

			if (WindowCoord.x > csbi.dwSize.X)
			{
				// windows sometimes uses existing colors to init right region of screen buffer
				ClearExtraRegions(colors::default_color(), CR_RIGHT);
			}
		}

		return SetWindowRect(csbi.srWindow);
	}

	bool console::SetScreenBufferSize(point const Size) const
	{
		const auto Out = GetOutputHandle();

		// This abominable workaround is for another Windows 10 bug, see https://github.com/microsoft/terminal/issues/2366
		// TODO: check the OS version once they fix it
		if (IsVtSupported())
		{
			CONSOLE_SCREEN_BUFFER_INFO Info;
			if (get_console_screen_buffer_info(Out, &Info))
			{
				// Make sure the cursor is within the new buffer
				if (!(Info.dwCursorPosition.X < Size.x && Info.dwCursorPosition.Y < Size.y))
				{
					if (!SetConsoleCursorPosition(
						Out,
						{
							std::min(Info.dwCursorPosition.X, static_cast<SHORT>(Size.x - 1)),
							std::min(Info.dwCursorPosition.Y, static_cast<SHORT>(Size.y - 1))
						}
					))
					{
						LOGERROR(L"SetConsoleCursorPosition(): {}"sv, os::last_error());
						return false;
					}
				}

				// Make sure the window is within the new buffer:
				rectangle Rect(Info.srWindow);
				if (Size.x < Rect.right + 1 || Size.y < Rect.bottom + 1)
				{
					const auto Width = Rect.width(), Height = Rect.height();
					Rect.bottom = std::min(Rect.bottom, Size.y - 1);
					Rect.right = std::min(Rect.right, Size.x - 1);
					Rect.top = std::max(0, Rect.bottom - Height);
					Rect.left = std::max(0, Rect.right - Width);

					if (!SetWindowRect(Rect))
						return false;
				}
			}
		}

		if (!SetConsoleScreenBufferSize(Out, make_coord(Size)))
		{
			LOGERROR(L"SetConsoleScreenBufferSize(): {}"sv, os::last_error());
			return false;
		}

		// After changing the buffer size the window size is not always correct
		if (IsVtSupported())
		{
			CONSOLE_SCREEN_BUFFER_INFO Info;
			if (get_console_screen_buffer_info(Out, &Info))
			{
				SetWindowRect(Info.srWindow);
			}
		}

		return true;
	}

	bool console::GetWindowRect(rectangle& ConsoleWindow) const
	{
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		ConsoleWindow = ConsoleScreenBufferInfo.srWindow;
		return true;
	}

	bool console::SetWindowRect(rectangle const& ConsoleWindow) const
	{
		const auto Rect = make_rect(ConsoleWindow);
		if (!SetConsoleWindowInfo(GetOutputHandle(), true, &Rect))
		{
			LOGERROR(L"SetConsoleWindowInfo(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::GetWorkingRect(rectangle& WorkingRect) const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		WorkingRect.bottom = csbi.dwSize.Y - 1;
		WorkingRect.left = 0;
		WorkingRect.right = WorkingRect.left + ScrX;
		WorkingRect.top = WorkingRect.bottom - ScrY;
		return true;
	}

	string console::GetPhysicalTitle() const
	{
		// Don't use GetConsoleTitle here, it's buggy.
		string Title;
		if (!os::GetWindowText(GetWindow(), Title))
		{
			LOGERROR(L"GetWindowText(): {}"sv, os::last_error());
		}

		return Title;
	}

	string console::GetTitle() const
	{
		return m_Title;
	}

	bool console::SetTitle(string_view const Title) const
	{
		m_Title = Title;
		if (!SetConsoleTitle(m_Title.c_str()))
		{
			LOGERROR(L"SetConsoleTitle(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	static HKL get_keyboard_layout_imm()
	{
		const auto ImeWnd = ImmGetDefaultIMEWnd(::console.GetWindow());
		if (!ImeWnd)
			return {};

		const auto ThreadId = GetWindowThreadProcessId(ImeWnd, {});
		if (!ThreadId)
		{
			LOGWARNING(L"GetWindowThreadProcessId(): {}"sv, os::last_error());
			return {};
		}

		return GetKeyboardLayout(ThreadId);
	}

	HKL console::GetKeyboardLayout() const
	{
		if (const auto Hkl = get_keyboard_layout_imm())
			return Hkl;

		wchar_t Buffer[KL_NAMELENGTH];
		if (!imports.GetConsoleKeyboardLayoutNameW(Buffer))
		{
			LOGWARNING(L"GetConsoleKeyboardLayoutNameW(): {}"sv, os::last_error());
			return {};
		}

		return os::make_hkl(Buffer);
	}

	uintptr_t console::GetInputCodepage() const
	{
		return GetConsoleCP();
	}

	bool console::SetInputCodepage(uintptr_t Codepage) const
	{
		if (!SetConsoleCP(Codepage))
		{
			LOGERROR(L"SetConsoleCP(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	uintptr_t console::GetOutputCodepage() const
	{
		return GetConsoleOutputCP();
	}

	bool console::SetOutputCodepage(uintptr_t Codepage) const
	{
		if (!SetConsoleOutputCP(Codepage))
		{
			LOGERROR(L"SetConsoleOutputCP(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const
	{
		if (!SetConsoleCtrlHandler(HandlerRoutine, Add))
		{
			LOGERROR(L"SetConsoleCtrlHandler(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::GetMode(HANDLE ConsoleHandle, DWORD& Mode) const
	{
		if (!GetConsoleMode(ConsoleHandle, &Mode))
		{
			LOGERROR(L"GetConsoleMode(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::SetMode(HANDLE ConsoleHandle, DWORD Mode) const
	{
		if (!SetConsoleMode(ConsoleHandle, Mode))
		{
			LOGERROR(L"SetConsoleMode(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	std::optional<DWORD> console::UpdateMode(HANDLE const ConsoleHandle, DWORD const ToSet, DWORD const ToClear) const
	{
		DWORD CurrentMode;

		if (!GetMode(ConsoleHandle, CurrentMode))
			return {};

		if (const auto NewMode = (CurrentMode | ToSet) & ~ToClear; NewMode != CurrentMode && !SetMode(ConsoleHandle, NewMode))
			return {};

		return CurrentMode;
	}

	bool console::IsVtSupported() const
	{
		static const auto Result = [&]
		{
			// https://devblogs.microsoft.com/commandline/24-bit-color-in-the-windows-console/
			if (!os::version::is_win10_build_or_later(14931))
				return false;

			const auto Handle = GetOutputHandle();

			DWORD CurrentMode;
			if (!GetMode(Handle, CurrentMode))
				return false;

			if (CurrentMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
				return true;

			if (!SetMode(Handle, CurrentMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
				return false;

			SetMode(Handle, CurrentMode);
			return true;
		}();

		return Result;
	}

	static bool layout_has_altgr(HKL const Layout)
	{
		static std::unordered_map<HKL, bool> LayoutState;
		const auto [Iterator, Inserted] = LayoutState.emplace(Layout, false);
		if (!Inserted)
			return Iterator->second;

		BYTE KeyState[256]{};
		KeyState[VK_CONTROL] = 0b10000000;
		KeyState[VK_MENU]    = 0b10000000;

		for (const auto VK: std::views::iota(0, 256))
		{
			if (VK == VK_PACKET)
				continue;

			if (wchar_t Buffer[2]; os::to_unicode(VK, 0, KeyState, Buffer, 0, Layout) > 0)
			{
				return Iterator->second = true;
			}
		}

		return false;
	}

	static void undo_altgr_if_redundant(KEY_EVENT_RECORD& KeyEvent)
	{
		const auto AltGr = LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED;

		if ((KeyEvent.dwControlKeyState & AltGr) != AltGr)
			return; // It's not AltGr

		if (KeyEvent.uChar.UnicodeChar)
			return; // It produces a character

		const auto Layout = ::console.GetKeyboardLayout();

		if (os::is_dead_key(KeyEvent, Layout))
			return; // It produces a dead key

		if (!layout_has_altgr(Layout))
			return; // It's not AltGr

		// It's AltGr that produces nothing. We can safely patch it to normal RAlt
		KeyEvent.dwControlKeyState &= ~LEFT_CTRL_PRESSED;

		BYTE KeyState[256]{};
		KeyState[VK_SHIFT] = KeyEvent.dwControlKeyState & SHIFT_PRESSED? 0b10000000 : 0;
		KeyState[VK_CAPITAL] = KeyEvent.dwControlKeyState & CAPSLOCK_ON? 0b00000001 : 0;

		if (wchar_t Buffer[2]; os::to_unicode(KeyEvent.wVirtualKeyCode, KeyEvent.wVirtualScanCode, KeyState, Buffer, 0, Layout) > 0)
			KeyEvent.uChar.UnicodeChar = Buffer[0];
	}

	static void postprocess_key_event(KEY_EVENT_RECORD& KeyEvent)
	{
		undo_altgr_if_redundant(KeyEvent);
	}

	static bool get_current_console_font(HANDLE OutputHandle, CONSOLE_FONT_INFO& FontInfo)
	{
		if (!GetCurrentConsoleFont(OutputHandle, FALSE, &FontInfo))
		{
			LOGERROR(L"GetCurrentConsoleFont(): {}"sv, os::last_error());
			return false;
		}

		// in XP FontInfo.dwFontSize contains something else than the size in pixels.
		FontInfo.dwFontSize = GetConsoleFontSize(OutputHandle, FontInfo.nFont);

		return FontInfo.dwFontSize.X && FontInfo.dwFontSize.Y;
	}

	// Workaround for a bug in the classic console: mouse position is screen-based
	static void fix_wheel_coordinates(MOUSE_EVENT_RECORD& Record)
	{
		if (!(Record.dwEventFlags & (MOUSE_WHEELED | MOUSE_HWHEELED)))
			return;

		// 'New' console doesn't need this
		if (::console.IsVtSupported())
			return;

		// Note: using the current mouse position rather than what's in the wheel event
		POINT CursorPos;
		if (!GetCursorPos(&CursorPos))
		{
			LOGWARNING(L"GetCursorPos(): {}"sv, os::last_error());
			return;
		}

		const auto WindowHandle = ::console.GetWindow();

		auto RelativePos = CursorPos;
		if (!ScreenToClient(WindowHandle, &RelativePos))
		{
			LOGWARNING(L"ScreenToClient(): {}"sv, os::last_error());
			return;
		}

		const auto OutputHandle = ::console.GetOutputHandle();

		CONSOLE_FONT_INFO FontInfo;
		if (!get_current_console_font(OutputHandle, FontInfo))
			return;

		CONSOLE_SCREEN_BUFFER_INFO Csbi;
		if (!get_console_screen_buffer_info(OutputHandle, &Csbi))
			return;

		const auto Set = [&](auto Coord, auto Point, auto SmallRect)
		{
			Record.dwMousePosition.*Coord = std::clamp(Csbi.srWindow.*SmallRect + static_cast<int>(RelativePos.*Point) / FontInfo.dwFontSize.*Coord, 0, Csbi.dwSize.*Coord - 1);
		};

		Set(&COORD::X, &POINT::x, &SMALL_RECT::Left);
		Set(&COORD::Y, &POINT::y, &SMALL_RECT::Top);
	}

	static void postprocess_mouse_event(MOUSE_EVENT_RECORD& MouseEvent)
	{
		if (!sWindowMode)
			return;

		fix_wheel_coordinates(MouseEvent);

		MouseEvent.dwMousePosition.Y = std::max(0, MouseEvent.dwMousePosition.Y - ::console.GetDelta());

		if (point Size; ::console.GetSize(Size))
			MouseEvent.dwMousePosition.X = std::min(MouseEvent.dwMousePosition.X, static_cast<short>(Size.x - 1));
	}

	static void postprocess_event(INPUT_RECORD& Record)
	{
		switch (Record.EventType)
		{
		case KEY_EVENT:
			postprocess_key_event(Record.Event.KeyEvent);
			break;

		case MOUSE_EVENT:
			postprocess_mouse_event(Record.Event.MouseEvent);
			break;

		default:
			break;
		}
	}

	std::optional<KEY_EVENT_RECORD> console::queued() const
	{
		if (!m_QueuedKeys.wRepeatCount)
			return {};

		auto Result = m_QueuedKeys;
		Result.wRepeatCount = 1;
		return Result;
	}

	bool console::PeekOneInput(INPUT_RECORD& Record) const
	{
		// See below
		if (const auto Key = queued())
		{
			Record.EventType = KEY_EVENT;
			Record.Event.KeyEvent = *Key;
			return true;
		}

		DWORD NumberOfEvents = 0;
		if (!PeekConsoleInput(GetInputHandle(), &Record, 1, &NumberOfEvents))
		{
			LOGERROR(L"PeekConsoleInput(): {}"sv, os::last_error());
			return false;
		}

		if (!NumberOfEvents)
			return false;

		postprocess_event(Record);

		return true;
	}

	bool console::ReadOneInput(INPUT_RECORD& Record) const
	{
		// See below
		if (const auto Key = queued())
		{
			Record.EventType = KEY_EVENT;
			Record.Event.KeyEvent = *Key;
			--m_QueuedKeys.wRepeatCount;
			return true;
		}

		DWORD NumberOfEvents = 0;
		if (!ReadConsoleInput(GetInputHandle(), &Record, 1, &NumberOfEvents))
		{
			LOGERROR(L"ReadConsoleInput(): {}"sv, os::last_error());
			return false;
		}

		if (!NumberOfEvents)
			return false;

		postprocess_event(Record);

		// https://learn.microsoft.com/en-us/windows/console/key-event-record-str
		// wRepeatCount
		// The repeat count, which indicates that a key is being held down.
		// For example, when a key is held down, you might get five events
		// with this member equal to 1, one event with this member equal to 5,
		// or multiple events with this member greater than or equal to 1.

		// We do not burden the rest of the code with these shenanigans
		// always yield key events with the repeat count equal to 1
		// and maintain an internal "queue" to yield the rest during the next calls.
		if (Record.EventType == KEY_EVENT && Record.Event.KeyEvent.wRepeatCount > 1)
		{
			m_QueuedKeys = Record.Event.KeyEvent;
			Record.Event.KeyEvent.wRepeatCount = 1;
			--m_QueuedKeys.wRepeatCount;
		}

		return true;
	}

	bool console::WriteInput(std::span<INPUT_RECORD> const Buffer, size_t& NumberOfEventsWritten) const
	{
		if (sWindowMode)
		{
			const auto Delta = GetDelta();

			for (auto& i: Buffer)
			{
				if (i.EventType == MOUSE_EVENT)
				{
					i.Event.MouseEvent.dwMousePosition.Y += Delta;
				}
			}
		}
		DWORD dwNumberOfEventsWritten = 0;
		SCOPE_EXIT{ NumberOfEventsWritten = dwNumberOfEventsWritten; };

		if (!WriteConsoleInput(GetInputHandle(), Buffer.data(), static_cast<DWORD>(Buffer.size()), &dwNumberOfEventsWritten))
		{
			LOGERROR(L"WriteConsoleInput(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	static bool ReadOutputImpl(CHAR_INFO* const Buffer, point const BufferSize, rectangle& ReadRegion)
	{
		auto Rect = make_rect(ReadRegion);
		SCOPE_EXIT{ ReadRegion = Rect; };

		if (!ReadConsoleOutput(::console.GetOutputHandle(), Buffer, make_coord(BufferSize), {}, &Rect))
		{
			LOGERROR(L"ReadConsoleOutput(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, point const BufferCoord, rectangle const& ReadRegionRelative) const
	{
		if (ExternalConsole.Imports.pReadOutput)
		{
			const COORD BufferSize{ static_cast<short>(Buffer.width()), static_cast<short>(Buffer.height()) };
			auto ReadRegion = make_rect(ReadRegionRelative);
			return ExternalConsole.Imports.pReadOutput(Buffer.data(), BufferSize, make_coord(BufferCoord), &ReadRegion) != FALSE;
		}

		const int Delta = sWindowMode? GetDelta() : 0;
		auto ReadRegion = ReadRegionRelative;
		ReadRegion.top += Delta;
		ReadRegion.bottom += Delta;

		const rectangle SubRect
		{
			BufferCoord.x,
			BufferCoord.y,
			BufferCoord.x + ReadRegion.width() - 1,
			BufferCoord.y + ReadRegion.height() - 1
		};

		std::vector<CHAR_INFO> ConsoleBuffer(SubRect.width() * SubRect.height());

		point const BufferSize{ SubRect.width(), SubRect.height() };

		if (BufferSize.x * BufferSize.y * sizeof(CHAR_INFO) > MAXSIZE)
		{
			const auto HeightStep = std::max(MAXSIZE / (BufferSize.x * sizeof(CHAR_INFO)), 1uz);

			const size_t Height = ReadRegion.bottom - ReadRegion.top + 1;

			for (size_t i = 0; i < Height; i += HeightStep)
			{
				auto PartialReadRegion = ReadRegion;
				PartialReadRegion.top += static_cast<int>(i);
				PartialReadRegion.bottom = std::min(ReadRegion.bottom, static_cast<int>(PartialReadRegion.top + HeightStep - 1));
				point const PartialBufferSize{ BufferSize.x, PartialReadRegion.height() };
				if (!ReadOutputImpl(ConsoleBuffer.data() + i * PartialBufferSize.x, PartialBufferSize, PartialReadRegion))
					return false;
			}
		}
		else
		{
			auto ReadRegionCopy = ReadRegion;
			if (!ReadOutputImpl(ConsoleBuffer.data(), BufferSize, ReadRegionCopy))
				return false;
		}

		auto ConsoleBufferIterator = ConsoleBuffer.cbegin();

		const auto replace_replacement_if_needed = [IsDefaultReplacementWide = char_width::is_wide(encoding::replace_char)](CHAR_INFO const& Cell)
		{
			// In some cases the host replaces various non-BMP codepoints with FFFD,
			// but always treats it as a narrow character, even when it's not (e.g. in MS Gothic).
			// Reading those FFFDs and writing them back breaks the layout spectacularly.
			// Replacing them with '?' is the easiest way to fix it.
			return IsDefaultReplacementWide && Cell.Char.UnicodeChar == encoding::replace_char && !(Cell.Attributes & COMMON_LVB_SBCSDBCS)? L'?' : Cell.Char.UnicodeChar;
		};

		for_submatrix(Buffer, SubRect, [&](FAR_CHAR_INFO& i)
		{
			const auto& Cell = *ConsoleBufferIterator++;
			i = { replace_replacement_if_needed(Cell), {}, {}, colors::unresolve_defaults(colors::NtColorToFarColor(Cell.Attributes)) };
		});

		return true;
	}

	static constexpr uint8_t vt_color_index(uint8_t const Index)
	{
		if (Index > colors::index::nt_last)
			return Index;

		// NT is RGB, VT is BGR
		constexpr uint8_t Table[]
		{
			//BGR     RGB
			0b000, // 000
			0b100, // 001
			0b010, // 010
			0b110, // 011
			0b001, // 100
			0b101, // 101
			0b011, // 110
			0b111, // 111
		};

		return (Index & 0b1000) | Table[Index & 0b111];
	}

	static constexpr struct
	{
		FARCOLORFLAGS Flags;
		string_view Normal, Intense, ExtendedColour, Default, Separator, ExtraSeparator;
		bool PreferBasicIndex;
	}
	ColorsMapping[]
	{
		// Initially Windows supported only RGB format ";2;R;G;B", ":2::R:G:B" was added much later.
		// Underline only supports the latter, which seems to be more standard/preferable, but we cannot use it exclusively:
		// as of Nov 2023 the host that comes with the OS only supports the former.
		{ FCF_FG_INDEX, L"3"sv, L"9"sv,  L"38"sv, L"39"sv, L";"sv, L""sv,  true  },
		{ FCF_BG_INDEX, L"4"sv, L"10"sv, L"48"sv, L"49"sv, L";"sv, L""sv,  true  },
		{ 0,            L""sv,  L""sv,   L"58"sv, L"59"sv, L":"sv, L":"sv, false },
	};

	enum class colors_mapping_type
	{
		foreground,
		background,
		underline,
	};

	static constexpr struct
	{
		FARCOLORFLAGS Style;
		string_view On, Off;
	}
	StyleMapping[]
	{
		{ FCF_FG_BOLD,         L"1"sv,     L"22"sv },
		{ FCF_FG_ITALIC,       L"3"sv,     L"23"sv },
		{ FCF_FG_OVERLINE,     L"53"sv,    L"55"sv },
		{ FCF_FG_STRIKEOUT,    L"9"sv,     L"29"sv },
		{ FCF_FG_FAINT,        L"2"sv,     L"22"sv },
		{ FCF_FG_BLINK,        L"5"sv,     L"25"sv },
		{ FCF_FG_INVERSE,      L"7"sv,     L"27"sv },
		{ FCF_FG_INVISIBLE,    L"8"sv,     L"28"sv },
	};

	static constexpr string_view UnderlineStyleMapping[]
	{
		L"24"sv,  // UNDERLINE_NONE
		L"4"sv,   // UNDERLINE_SINGLE
		L"21"sv,  // UNDERLINE_DOUBLE
		L"4:3"sv, // UNDERLINE_CURLY
		L"4:4"sv, // UNDERLINE_DOT
		L"4:5"sv, // UNDERLINE_DASH
	};

	static void make_vt_color(colors::single_color const Color, colors_mapping_type const MappingType, string& Str)
	{
		const auto& Mapping = ColorsMapping[std::to_underlying(MappingType)];

		if (Color.IsIndex)
		{
			if (colors::is_default(Color.Value))
				append(Str, Mapping.Default);
			else if (const auto Index = vt_color_index(colors::index_value(Color.Value)); Index < colors::index::nt_size && Mapping.PreferBasicIndex)
				append(Str, Color.Value & C_INTENSE? Mapping.Intense : Mapping.Normal, static_cast<wchar_t>(L'0' + (Index & 0b111)));
			else
				far::format_to(Str, L"{1}{0}5{0}{2}"sv, Mapping.Separator, Mapping.ExtendedColour, Index);
		}
		else
		{
			const auto RGBA = colors::to_rgba(Color.Value);
			far::format_to(Str, L"{2}{0}2{0}{1}{3}{0}{4}{0}{5}"sv, Mapping.Separator, Mapping.ExtraSeparator, Mapping.ExtendedColour, RGBA.r, RGBA.g, RGBA.b);
		}
	}

	static void make_vt_style(FARCOLORFLAGS const Style, string& Str, FARCOLORFLAGS const LastStyle)
	{
		for (const auto& i: StyleMapping)
		{
			const auto Was = (LastStyle & i.Style) != 0;
			const auto Is  = (Style & i.Style) != 0;

			if (Was == Is)
				continue;

			append(Str, Is > Was? i.On : i.Off, L';');
		}

		// We should only enter this function if the style has changed and it should add or remove at least something,
		// so no need to check before pop:
		Str.pop_back();
	}

	static void make_vt_attributes(const FarColor& Color, string& Str, FarColor const& LastColor)
	{
		using colors::single_color;
		const auto StyleMaskWithoutUnderline = FCF_STYLEMASK & ~FCF_FG_UNDERLINE_MASK;

		struct expanded_state
		{
			single_color ForegroundColor, BackgroundColor;
			FARCOLORFLAGS Style;
			UNDERLINE_STYLE UnderlineStyle;
			single_color UnderlineColor;

			bool operator==(expanded_state const&) const = default;

			explicit expanded_state(FarColor const& Color):
				ForegroundColor(single_color::foreground(Color)),
				BackgroundColor(single_color::background(Color)),
				Style(Color.Flags& StyleMaskWithoutUnderline),
				UnderlineStyle(Color.GetUnderline()),
				UnderlineColor(single_color::underline(Color))
			{
				if (Color.Flags & COMMON_LVB_GRID_HORIZONTAL)
					Style |= FCF_FG_OVERLINE;

				if (Color.Flags & COMMON_LVB_REVERSE_VIDEO)
					Style |= FCF_FG_INVERSE;

				if (Color.Flags & COMMON_LVB_UNDERSCORE && UnderlineStyle == UNDERLINE_STYLE::UNDERLINE_NONE)
					UnderlineStyle = UNDERLINE_STYLE::UNDERLINE_SINGLE;

				if (
					// If there's no underline, no point in emitting its color
					UnderlineStyle == UNDERLINE_NONE ||
					// UnderlineColor repurposed a previously reserved field,
					// which means that it will likely be set to 0 ("transparent black")
					// when coming from external sources like config or plugins.
					// We don't want to treat that case as black for obvious reasons.
					colors::is_transparent(UnderlineColor.Value) ||
					// No point in emitting the color if it's the same as foreground
					UnderlineColor == ForegroundColor
				)
					UnderlineColor = single_color::default_color();
			}
		}
		const
		Current(Color), Last(LastColor);

		if (Current == Last)
			return;

		Str += CSI ""sv;

		auto ModeAdded = false;

		if (Current.ForegroundColor != Last.ForegroundColor)
		{
			make_vt_color(Current.ForegroundColor, colors_mapping_type::foreground, Str);
			ModeAdded = true;
		}

		if (Current.BackgroundColor != Last.BackgroundColor)
		{
			if (ModeAdded)
				Str += L';';

			make_vt_color(Current.BackgroundColor, colors_mapping_type::background, Str);
			ModeAdded = true;
		}

		if (Current.Style != Last.Style)
		{
			if (ModeAdded)
				Str += L';';

			make_vt_style(Current.Style, Str, Last.Style);
			ModeAdded = true;
		}

		if (Current.UnderlineStyle != Last.UnderlineStyle)
		{
			if (ModeAdded)
				Str += L';';

			Str += UnderlineStyleMapping[Current.UnderlineStyle];
			ModeAdded = true;
		}

		if (Current.UnderlineColor != Last.UnderlineColor)
		{
			if (ModeAdded)
				Str += L';';

			make_vt_color(Current.UnderlineColor, colors_mapping_type::underline, Str);
			ModeAdded = true;
		}

		assert(ModeAdded);

		Str += L'm';
	}

	static bool is_same_color(FarColor const& a, FarColor const& b)
	{
		// FCF_RAWATTR_MASK contains non-VT stuff we don't care about.
		// FCF_INHERIT_STYLE only affects logical composition.
		constexpr auto IgnoredFlags = FCF_RAWATTR_MASK | FCF_INHERIT_STYLE;

		return
			(a.Flags & ~IgnoredFlags) == (b.Flags & ~IgnoredFlags) &&
			a.ForegroundColor == b.ForegroundColor &&
			a.BackgroundColor == b.BackgroundColor &&
			a.UnderlineColor == b.UnderlineColor;
			// Reserved contains non-BMP codepoints and is of no interest here.
	}

	static void make_vt_sequence(std::span<FAR_CHAR_INFO> Input, string& Str, FarColor& LastColor)
	{
		const auto CharWidthEnabled = char_width::is_enabled();

		std::optional<wchar_t> LeadingChar;

		for (const auto& [Cell, n]: enumerate(Input))
		{
			if (CharWidthEnabled)
			{
				if (n != Input.size() - 1)
				{
					sanitise_pair(Cell, Input[n + 1]);
				}

				if (Cell.Attributes.Flags & COMMON_LVB_TRAILING_BYTE)
				{
					if (!LeadingChar)
					{
						Cell.Char = bad_char_replacement;
						flags::clear(Cell.Attributes.Flags, COMMON_LVB_TRAILING_BYTE);
					}
					else if (Cell.Char == *LeadingChar)
					{
						if (Cell.Char == encoding::replace_char && !char_width::is_wide(encoding::replace_char))
						{
							// As of 13 Jul 2022 ReadConsoleOutputW doesn't work with surrogate pairs (see microsoft/terminal#10810)
							// It returns two FFFDs instead with leading and trailing flags.
							// We can't just drop the trailing one here because FFFD isn't always wide and the layout might get broken.
							Cell.Char = L' ';
						}
						else
						{
							LeadingChar.reset();
							continue;
						}
					}
				}
				else if (!n && encoding::utf16::is_low_surrogate(Cell.Char))
				{
					Cell.Char = bad_char_replacement;
				}

				LeadingChar.reset();

				if (Cell.Attributes.Flags & COMMON_LVB_LEADING_BYTE)
				{
					if (n == Input.size() - 1)
					{
						flags::clear(Cell.Attributes.Flags, COMMON_LVB_LEADING_BYTE);
						Cell.Char = bad_char_replacement;
					}
					else
					{
						LeadingChar = Cell.Char;
					}
				}
				else if (
					n == Input.size() - 1 &&
					(
						encoding::utf16::is_high_surrogate(Cell.Char) ||
						// FFFD can be wide too
						(Cell.Char == encoding::replace_char && Cell.Reserved1 <= std::numeric_limits<wchar_t>::max() && char_width::is_wide(encoding::replace_char))
					)
				)
				{
					Cell.Char = bad_char_replacement;
				}
			}

			if (!is_same_color(Cell.Attributes, LastColor))
			{
				make_vt_attributes(Cell.Attributes, Str, LastColor);
				LastColor = Cell.Attributes;
			}

			if (CharWidthEnabled && Cell.Char == encoding::replace_char && Cell.Reserved1 > std::numeric_limits<wchar_t>::max())
			{
				const auto Pair = encoding::utf16::to_surrogate(Cell.Reserved1);
				append(Str, Pair.first, Pair.second);

				if (char_width::is_half_width_surrogate_broken())
					append(Str, CSI L"1D"sv); // Yuck
			}
			else
			{
				Str += ReplaceControlCharacter(Cell.Char);
			}
		}
	}

	class console::implementation
	{
		class foreign_blocks_list
		{
		public:
			void queue(FAR_CHAR_INFO const& Cell, point const& Point, rectangle const WorkingArea)
			{
				const auto IsForeign = check(Cell);
				if (IsForeign)
				{
					if (!m_ForeignBlock)
						m_ForeignBlock.emplace(WorkingArea.left + Point.x, WorkingArea.top + Point.y, WorkingArea.left + Point.x, WorkingArea.top + Point.y);
					else
						++m_ForeignBlock->right;
				}

				if (m_ForeignBlock && (!IsForeign || Point.x == WorkingArea.width() - 1 || Point.y == WorkingArea.height() - 1))
					queue();
			}

			void unstash() const
			{
				for (const auto& Block : m_ForeignBlocks)
					::console.unstash_output(Block);
			}

		private:
			static bool check(FAR_CHAR_INFO const& Cell)
			{
				return
					Cell.Attributes.Flags & FCF_FOREIGN &&
					colors::is_transparent(Cell.Attributes.ForegroundColor) &&
					colors::is_transparent(Cell.Attributes.BackgroundColor);
			}

			void queue()
			{
				for (auto& Block: m_ForeignBlocks)
				{
					if (
						Block.left == m_ForeignBlock->left &&
						Block.right == m_ForeignBlock->right &&
						Block.bottom == m_ForeignBlock->top - 1
						)
					{
						Block.bottom = m_ForeignBlock->bottom;
						m_ForeignBlock.reset();
						return;
					}
				}

				m_ForeignBlocks.emplace_back(*m_ForeignBlock);
				m_ForeignBlock.reset();
			}

			std::vector<rectangle> m_ForeignBlocks;
			std::optional<rectangle> m_ForeignBlock;
		};

	public:
		static bool WriteOutputVT(matrix<FAR_CHAR_INFO>& Buffer, point const BufferCoord, rectangle const& WriteRegion)
		{
			const rectangle SubRect
			{
				BufferCoord.x,
				BufferCoord.y,
				BufferCoord.x + WriteRegion.width() - 1,
				BufferCoord.y + WriteRegion.height() - 1
			};

			const auto Out = ::console.GetOutputHandle();

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (!get_console_screen_buffer_info(Out, &csbi))
				return false;

			point SavedCursorPosition;
			if (!::console.GetCursorRealPosition(SavedCursorPosition))
				return false;

			// Ideally this should be filtered out earlier
			if (WriteRegion.left > csbi.dwSize.X - 1 || WriteRegion.top > csbi.dwSize.Y - 1)
				return false;

			SCOPED_ACTION(hide_cursor);

			// Move the viewport down
			if (!::console.SetCursorRealPosition({0, csbi.dwSize.Y - 1}))
				return false;

			// Set cursor position within the viewport
			if (!::console.SetCursorRealPosition({WriteRegion.left, WriteRegion.top}))
				return false;

			SCOPE_EXIT
			{
				// Move the viewport down
				::console.SetCursorRealPosition({ 0, csbi.dwSize.Y - 1 });
				// Restore cursor position within the viewport
				::console.SetCursorRealPosition(SavedCursorPosition);
				// Restore buffer relative position
				if (csbi.srWindow.Left || csbi.srWindow.Bottom != csbi.dwSize.Y - 1)
					::console.SetWindowRect(csbi.srWindow);
			};

			string Str;

			// The idea is to reduce the number of reallocations,
			// but only if it's not a single / double cell update
			if (const auto Area = SubRect.width() * SubRect.height(); Area > 4)
				Str.reserve(std::max(1024, Area * 2));

			auto LastColor = colors::default_color();

			point ViewportSize;
			{
				rectangle WindowRect;
				if (!::console.GetWindowRect(WindowRect))
					return false;

				ViewportSize = { WindowRect.width(), WindowRect.height() };
			}

			// If SubRect is too tall (e.g. when we flushing the old content of console resize), the rest will be dropped.
			// VT is a bloody joke.
			for (int SubrectOffset = 0; SubrectOffset < SubRect.height(); SubrectOffset += ViewportSize.y)
			{
				if (SubrectOffset)
				{
					// Move the viewport one "page" down
					if (!::console.SetCursorRealPosition({0, std::min(csbi.dwSize.Y - 1, WriteRegion.top + SubrectOffset + ViewportSize.y - 1)}))
						return false;
					// Set cursor position within the viewport
					if (!::console.SetCursorRealPosition({ WriteRegion.left, WriteRegion.top + SubrectOffset }))
						return false;
				}

				// Don't do CUP here: the viewport origin is too unstable to rely on it, especially since we touch it just above.
				// Saving, restoring and moving down seems to be more reliable.
				// Words cannot describe how much I despise VT.

				// Save cursor position
				Str = ANSISYSSC L""sv;

				foreign_blocks_list ForeignBlocksList;

				for (const auto i: std::views::iota(SubRect.top + SubrectOffset, std::min(SubRect.top + SubrectOffset + ViewportSize.y, SubRect.bottom + 1)))
				{
					if (i != SubRect.top + SubrectOffset)
					{
						Str +=
							ANSISYSRC // Restore cursor position
							CSI L"1B" // Move cursor down
							ANSISYSSC // Save again

							// conhost used to preserve colors after ANSISYSRC, but it is not the case anymore (see terminal#14612)
							// Explicitly reset them here for consistency across implementations.
							CSI L"m"sv;

						LastColor = colors::default_color();
					}

					const auto BlockRow = Buffer[i].subspan(SubRect.left, SubRect.width());
					make_vt_sequence(BlockRow, Str, LastColor);

					if (SubRect.right == ScrX && i != ScrY)
					{
						// Explicitly ending rows with \n should (hopefully) give a hint to the host
						// that we're writing something structured and not just a stream,
						// so it's better to leave the text alone when resizing the buffer.
						// Surprisingly, it also fixes terminal#15153.
						Str += L'\n';
					}

					for (const auto& Cell: BlockRow)
					{
						ForeignBlocksList.queue(Cell, { static_cast<int>(&Cell - BlockRow.data()), i - (SubRect.top + SubrectOffset) }, SubRect);
					}
				}

				if (!::console.Write(Str))
					return false;

				ForeignBlocksList.unstash();

				Str.clear();
			}

			return ::console.Write(CSI L"m"sv);
		}

		class cursor_suppressor: public hide_cursor
		{
		public:
			NONCOPYABLE(cursor_suppressor);

			cursor_suppressor():
				m_Restore(::console.GetCursorRealPosition(m_Position) && ::console.SetCursorPosition({}))
			{
			}

			~cursor_suppressor()
			{
				if (m_Restore)
					(void)::console.SetCursorRealPosition(m_Position);
			}

		private:
			point m_Position;
			bool m_Restore{};
		};

		static bool WriteOutputNTImpl(CHAR_INFO const* const Buffer, point const BufferSize, rectangle const& WriteRegion)
		{
			// https://github.com/microsoft/terminal/issues/10456
			// It looks like only a specific range of Windows 10 versions is affected.
			static const auto IsCursorPositionWorkaroundNeeded = os::version::is_win10_build_or_later(19041) && !os::version::is_win10_build_or_later(21277);

			std::optional<cursor_suppressor> CursorSuppressor;
			if (IsCursorPositionWorkaroundNeeded && char_width::is_enabled())
				CursorSuppressor.emplace();

			auto SysWriteRegion = make_rect(WriteRegion);
			if (!WriteConsoleOutput(::console.GetOutputHandle(), Buffer, make_coord(BufferSize), {}, &SysWriteRegion))
			{
				LOGERROR(L"WriteConsoleOutput(): {}"sv, os::last_error());
				return false;
			}

			return true;
		}

		static bool WriteOutputNT(matrix<FAR_CHAR_INFO>& Buffer, point const BufferCoord, rectangle const& WriteRegion)
		{
			const rectangle SubRect
			{
				BufferCoord.x,
				BufferCoord.y,
				BufferCoord.x + WriteRegion.width() - 1,
				BufferCoord.y + WriteRegion.height() - 1
			};

			std::vector<CHAR_INFO> ConsoleBuffer;
			ConsoleBuffer.reserve(SubRect.width() * SubRect.height());

			foreign_blocks_list ForeignBlocksList;

			if (char_width::is_enabled())
			{
				for_submatrix(Buffer, SubRect, [&](FAR_CHAR_INFO& Cell, point const Point)
				{
					if (!Point.x)
					{
						if (Cell.Attributes.Flags & COMMON_LVB_TRAILING_BYTE)
						{
							flags::clear(Cell.Attributes.Flags, COMMON_LVB_TRAILING_BYTE);
							Cell.Char = bad_char_replacement;
						}
						else if (encoding::utf16::is_low_surrogate(Cell.Char))
						{
							Cell.Char = bad_char_replacement;
						}
					}

					if (Point.x != SubRect.width() - 1)
					{
						sanitise_pair(Cell, Buffer[SubRect.top + Point.y][SubRect.left + Point.x + 1]);
					}
					else
					{
						if (Cell.Attributes.Flags & COMMON_LVB_LEADING_BYTE)
						{
							flags::clear(Cell.Attributes.Flags, COMMON_LVB_LEADING_BYTE);
							Cell.Char = bad_char_replacement;
						}
						else if (encoding::utf16::is_high_surrogate(Cell.Char))
						{
							Cell.Char = bad_char_replacement;
						}
					}

					ConsoleBuffer.emplace_back(CHAR_INFO{ { ReplaceControlCharacter(Cell.Char) }, colors::FarColorToConsoleColor(Cell.Attributes) });
					ForeignBlocksList.queue(Cell, Point, SubRect);
				});
			}
			else
			{
				for_submatrix(Buffer, SubRect, [&](const FAR_CHAR_INFO& Cell, point const Point)
				{
					ConsoleBuffer.emplace_back(CHAR_INFO{ { ReplaceControlCharacter(Cell.Char) }, colors::FarColorToConsoleColor(Cell.Attributes) });
					ForeignBlocksList.queue(Cell, Point, SubRect);
				});
			}

			point const BufferSize{ SubRect.width(), SubRect.height() };

			if (BufferSize.x * BufferSize.y * sizeof(CHAR_INFO) > MAXSIZE)
			{
				const auto HeightStep = std::max(MAXSIZE / (BufferSize.x * sizeof(CHAR_INFO)), 1uz);

				for (size_t i = 0, Height = WriteRegion.height(); i < Height; i += HeightStep)
				{
					rectangle const PartialWriteRegion
					{
						WriteRegion.left,
						WriteRegion.top + static_cast<int>(i),
						WriteRegion.right,
						std::min(WriteRegion.bottom, static_cast<int>(WriteRegion.top + static_cast<int>(i) + HeightStep - 1))
					};

					point const PartialBufferSize
					{
						BufferSize.x,
						PartialWriteRegion.height()
					};

					if (!WriteOutputNTImpl(ConsoleBuffer.data() + i * PartialBufferSize.x, PartialBufferSize, PartialWriteRegion))
						return false;
				}
			}
			else
			{
				if (!WriteOutputNTImpl(ConsoleBuffer.data(), BufferSize, WriteRegion))
					return false;
			}

			ForeignBlocksList.unstash();

			return true;
		}

		static bool SetTextAttributesVT(const FarColor& Attributes)
		{
			// For fallback
			SetTextAttributesNT(Attributes);

			string Str;
			make_vt_attributes(Attributes, Str, {});
			return ::console.Write(Str);
		}

		static bool SetTextAttributesNT(const FarColor& Attributes)
		{
			if (!SetConsoleTextAttribute(::console.GetOutputHandle(), colors::FarColorToConsoleColor(Attributes)))
			{
				LOGERROR(L"SetConsoleTextAttribute(): {}"sv, os::last_error());
				return false;
			}

			return true;
		}

		static bool GetPaletteVT(std::array<COLORREF, 256>& Palette)
		{
			try
			{
				LOGDEBUG(L"Reading VT palette - here be dragons"sv);

				const auto
					OSCPrefix = ESC L"]4"sv,
					OSCSuffix = ST L""sv;

				string Request;
				Request.reserve(OSCPrefix.size() + L";255;?"sv.size() * Palette.size() - 100 - 10 + OSCSuffix.size());

				// A single OSC for the whole thing.
				// Querying the palette was introduced after the terse syntax, so it's fine.
				Request = OSCPrefix;

				for (const auto i: std::views::iota(0uz, Palette.size()))
					far::format_to(Request, L";{};?"sv, vt_color_index(static_cast<uint8_t>(i)));

				Request += OSCSuffix;

				const auto ResponseData = query_vt(Request);
				if (ResponseData.empty())
				{
					LOGWARNING(L"OSC 4 query is not supported"sv, Request);
					return false;
				}

				const auto give_up = [&]
				{
					throw far_exception(far::format(L"Incorrect response: {}"sv, ResponseData), false);
				};

				string_view Response = ResponseData;
				if (!Response.ends_with(L'\\'))
					give_up();

				Response.remove_suffix(1);

				const auto
					Prefix = ESC "]"sv,
					Suffix = ESC ""sv,
					RGBPrefix = L"rgb:"sv;

				size_t ColorsSet = 0;

				for (auto PaletteToken: enum_tokens(Response, L"\\"sv))
				{
					if (!PaletteToken.starts_with(Prefix) || !PaletteToken.ends_with(Suffix))
						give_up();

					PaletteToken.remove_prefix(Prefix.size());
					PaletteToken.remove_suffix(Suffix.size());

					enum_tokens const Subtokens(PaletteToken, L";"sv);

					auto SubIterator = Subtokens.cbegin();
					if (SubIterator == Subtokens.cend())
						give_up();

					if (*SubIterator++ != L"4"sv)
						give_up();

					const auto VtIndex = from_string<unsigned>(*SubIterator++);
					if (VtIndex >= Palette.size())
						give_up();

					auto& PaletteColor = Palette[vt_color_index(VtIndex)];

					auto ColorStr = *SubIterator;
					if (!ColorStr.starts_with(RGBPrefix))
						give_up();

					ColorStr.remove_prefix(RGBPrefix.size());

					if (ColorStr.size() != L"0000"sv.size() * 3 + 2)
						give_up();

					const auto color = [&](size_t const Offset)
					{
						const auto Value = from_string<unsigned>(ColorStr.substr(Offset * L"0000/"sv.size(), 4), {}, 16);
						if (Value > 0xffff)
							give_up();

						return Value / 0x0101;
					};

					PaletteColor = RGB(color(0), color(1), color(2));
					++ColorsSet;
				}

				if (ColorsSet != Palette.size())
					give_up();

				LOGDEBUG(L"VT palette read successfuly"sv);
				return true;
			}
			catch (far_exception const& e)
			{
				LOGERROR(L"{}"sv, e);
				return false;
			}
		}

		static bool GetPaletteNT(std::array<COLORREF, 256>& Palette)
		{
			if (!imports.GetConsoleScreenBufferInfoEx)
				return false;

			CONSOLE_SCREEN_BUFFER_INFOEX csbi{ sizeof(csbi) };
			if (!imports.GetConsoleScreenBufferInfoEx(::console.GetOutputHandle(), &csbi))
			{
				LOGERROR(L"GetConsoleScreenBufferInfoEx(): {}"sv, os::last_error());
				return false;
			}

			std::ranges::copy(csbi.ColorTable, Palette.begin());

			return true;
		}

		static bool SetPaletteVT(std::array<COLORREF, 256> const& Palette)
		{
			string Str;
			Str.reserve(OSC(L"4;255;rgb:ff/ff/ff").size() * Palette.size() - 100 - 10);

			for (const auto& [Color, i] : enumerate(Palette))
			{
				const auto RGBA = colors::to_rgba(Color);
				// A separate OSC for every color: unfortunately the terse syntax was only added in 2020
				far::format_to(Str, OSC(L"4;{};rgb:{:02x}/{:02x}/{:02x}"), vt_color_index(i), RGBA.r, RGBA.g, RGBA.b);
			}

			return ::console.Write(Str);
		}

		static bool SetPaletteNT(std::array<COLORREF, 256> const& Palette)
		{
			if (!imports.GetConsoleScreenBufferInfoEx)
				return false;

			const auto Output = ::console.GetOutputHandle();

			CONSOLE_SCREEN_BUFFER_INFOEX csbi{ sizeof(csbi) };
			if (!imports.GetConsoleScreenBufferInfoEx(Output, &csbi))
			{
				LOGERROR(L"GetConsoleScreenBufferInfoEx(): {}"sv, os::last_error());
				return false;
			}

			std::span const NtPalette(Palette.data(), colors::index::nt_size);

			if (std::ranges::equal(NtPalette, csbi.ColorTable))
				return true;

			std::ranges::copy(NtPalette, std::begin(csbi.ColorTable));

			if (!imports.SetConsoleScreenBufferInfoEx(Output, &csbi))
			{
				LOGERROR(L"SetConsoleScreenBufferInfoEx(): {}"sv, os::last_error());
				return false;
			}

			// Get + Set screws up the window size 🤦
			if (!SetConsoleWindowInfo(Output, true, &csbi.srWindow))
				LOGWARNING(L"SetConsoleWindowInfo(): {}"sv, os::last_error());

			return true;
		}

	};

	bool console::WriteOutput(matrix<FAR_CHAR_INFO>& Buffer, point BufferCoord, const rectangle& WriteRegionRelative) const
	{
		if (IsVtActive())
		{
			const int Delta = sWindowMode? GetDelta() : 0;
			auto WriteRegion = WriteRegionRelative;
			WriteRegion.top += Delta;
			WriteRegion.bottom += Delta;
			return implementation::WriteOutputVT(Buffer, BufferCoord, WriteRegion);
		}

		if (ExternalConsole.Imports.pWriteOutput)
		{
			const COORD BufferSize{ static_cast<short>(Buffer.width()), static_cast<short>(Buffer.height()) };
			auto WriteRegion = make_rect(WriteRegionRelative);
			return ExternalConsole.Imports.pWriteOutput(Buffer.data(), BufferSize, make_coord(BufferCoord), &WriteRegion) != FALSE;
		}

		const int Delta = sWindowMode? GetDelta() : 0;
		auto WriteRegion = WriteRegionRelative;
		WriteRegion.top += Delta;
		WriteRegion.bottom += Delta;

		return implementation::WriteOutputNT(Buffer, BufferCoord, WriteRegion);
	}

	bool console::WriteOutputGather(matrix<FAR_CHAR_INFO>& Buffer, std::span<rectangle const> WriteRegions) const
	{
		// TODO: VT can handle this in one go
		for (const auto& i: WriteRegions)
		{
			if (!WriteOutput(Buffer, { i.left, i.top }, i))
				return false;
		}

		return true;
	}

	bool console::Read(std::span<wchar_t> const Buffer, size_t& Size) const
	{
		DWORD NumberOfCharsRead;
		if (!ReadConsole(GetInputHandle(), Buffer.data(), static_cast<DWORD>(Buffer.size()), &NumberOfCharsRead, {}))
		{
			LOGERROR(L"ReadConsole(): {}"sv, os::last_error());
			return false;
		}

		Size = NumberOfCharsRead;
		return true;
	}

	bool console::Write(const string_view Str) const
	{
		DWORD NumberOfCharsWritten;
		if (!WriteConsole(GetOutputHandle(), Str.data(), static_cast<DWORD>(Str.size()), &NumberOfCharsWritten, {}))
		{
			LOGERROR(L"WriteConsole(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::Commit() const
	{
		if (ExternalConsole.Imports.pCommit)
			return ExternalConsole.Imports.pCommit() != FALSE;

		// reserved
		return true;
	}

	bool console::GetTextAttributes(FarColor& Attributes) const
	{
		if (ExternalConsole.Imports.pGetTextAttributes)
			return ExternalConsole.Imports.pGetTextAttributes(&Attributes) != FALSE;

		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		Attributes = colors::NtColorToFarColor(ConsoleScreenBufferInfo.wAttributes);
		return true;
	}

	bool console::SetTextAttributes(const FarColor& Attributes) const
	{
		if (ExternalConsole.Imports.pSetTextAttributes)
			return ExternalConsole.Imports.pSetTextAttributes(&Attributes) != FALSE;

		return (IsVtActive()? implementation::SetTextAttributesVT : implementation::SetTextAttributesNT)(Attributes);
	}

	bool console::GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
	{
		if (!GetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo))
		{
			LOGERROR(L"GetConsoleCursorInfo(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
	{
		if (!SetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo))
		{
			LOGERROR(L"SetConsoleCursorInfo(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::GetCursorPosition(point& Position) const
	{
		if (!GetCursorRealPosition(Position))
			return false;

		if (sWindowMode)
			Position.y -= GetDelta();

		return true;
	}

	bool console::SetCursorPosition(point Position) const
	{
		if (sWindowMode)
		{
			point Size{};
			GetSize(Size);
			Position.x = std::min(Position.x, Size.x - 1);
			Position.y = std::max(0, Position.y);
			Position.y += GetDelta();
		}
		return SetCursorRealPosition(Position);
	}

	bool console::FlushInputBuffer() const
	{
		if (m_QueuedKeys.wRepeatCount)
			m_QueuedKeys = {};

		return FlushConsoleInputBuffer(GetInputHandle()) != FALSE;
	}

	bool console::GetNumberOfInputEvents(size_t& NumberOfEvents) const
	{
		if (DWORD dwNumberOfEvents = 0; GetNumberOfConsoleInputEvents(GetInputHandle(), &dwNumberOfEvents))
		{
			NumberOfEvents = m_QueuedKeys.wRepeatCount + dwNumberOfEvents;
			return true;
		}

		if (!m_QueuedKeys.wRepeatCount)
			return false;

		NumberOfEvents = m_QueuedKeys.wRepeatCount;
		return true;
	}

	bool console::GetAlias(string_view const Name, string& Value, string_view const ExeName) const
	{
		SCOPED_ACTION(os::last_error_guard);

		null_terminated const C_Name(Name), C_ExeName(ExeName);

		return os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](std::span<wchar_t> Buffer)
		{
			// This API design is mental:
			// - If everything is ok, it return the string size, including the terminating \0
			// - If the buffer size is too small, it returns the input buffer size and sets last error to ERROR_INSUFFICIENT_BUFFER
			// - It can also return 0 in case of other errors
			// This means that if the string size is exactly (BufferSize - 1), the only way to understand whether it succeeded or not
			// is to look at the error code. And it doesn't reset it on success, so have to do it ourselves to be sure. *facepalm*
			SetLastError(ERROR_SUCCESS);
			const auto BufferSizeInBytes = Buffer.size() * sizeof(wchar_t);
			const size_t ReturnedSizeInBytes = GetConsoleAlias(
				const_cast<wchar_t*>(C_Name.c_str()),
				Buffer.data(),
				static_cast<DWORD>(BufferSizeInBytes),
				const_cast<wchar_t*>(C_ExeName.c_str())
			);

			if (!ReturnedSizeInBytes || (ReturnedSizeInBytes == BufferSizeInBytes && GetLastError() == ERROR_INSUFFICIENT_BUFFER))
				return 0uz;

			return ReturnedSizeInBytes / sizeof(wchar_t) - 1;
		});
	}

	struct console::console_aliases::data
	{
		// We only use it to bulk copy the aliases from one console to another,
		// so no need to care about case insensitivity and fancy lookup.
		std::vector<std::pair<string, std::vector<std::pair<string, string>>>> Aliases;
	};

	console::console_aliases::console_aliases() = default;
	console::console_aliases::~console_aliases() = default;

	console::console_aliases console::GetAllAliases() const
	{
		const auto ExeLength = GetConsoleAliasExesLength();
		if (!ExeLength)
			return {};

		std::vector<wchar_t> ExeBuffer(ExeLength / sizeof(wchar_t) + 1); // +1 for double \0
		if (!GetConsoleAliasExes(ExeBuffer.data(), ExeLength))
		{
			LOGERROR(L"GetConsoleAliasExes(): {}"sv, os::last_error());
			return {};
		}

		auto Aliases = std::make_unique<console_aliases::data>();

		std::vector<wchar_t> AliasesBuffer;
		for (const auto& ExeToken: enum_substrings(ExeBuffer))
		{
			// It's ok, ExeToken is guaranteed to be null-terminated
			const auto ExeNamePtr = const_cast<wchar_t*>(ExeToken.data());
			const auto AliasesLength = GetConsoleAliasesLength(ExeNamePtr);
			AliasesBuffer.resize(AliasesLength / sizeof(wchar_t) + 1); // +1 for double \0
			if (!GetConsoleAliases(AliasesBuffer.data(), AliasesLength, ExeNamePtr))
			{
				LOGERROR(L"GetConsoleAliases(): {}"sv, os::last_error());
				continue;
			}

			std::pair<string, std::vector<std::pair<string, string>>> ExeData;
			ExeData.first = ExeNamePtr;
			for (const auto& AliasToken: enum_substrings(AliasesBuffer))
			{
				ExeData.second.emplace_back(split(AliasToken));
			}

			Aliases->Aliases.emplace_back(std::move(ExeData));
		}

		console_aliases Result;
		Result.m_Data = std::move(Aliases);

		return Result;
	}

	void console::SetAllAliases(console_aliases&& Aliases) const
	{
		if (!Aliases.m_Data)
			return;

		for (const auto& [ExeName, ExeAliases]: Aliases.m_Data->Aliases)
		{
			for (const auto& [Alias, Value]: ExeAliases)
			{
				AddConsoleAlias(
					const_cast<wchar_t*>(Alias.c_str()),
					const_cast<wchar_t*>(Value.c_str()),
					const_cast<wchar_t*>(ExeName.c_str())
				);
			}
		}
	}

	bool console::GetDisplayMode(DWORD& Mode) const
	{
		if (!GetConsoleDisplayMode(&Mode))
		{
			LOGERROR(L"GetConsoleDisplayMode(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	static bool validate_console_size(point const Size)
	{
		// https://github.com/microsoft/terminal/issues/10337

		// As of 15 Jul 2024 GetLargestConsoleWindowSize is broken in WT.
		// It takes the current screen size in pixels and divides it by an inadequate font size, e.g. 1x16 or 1x1.

		// It is unlikely that it is ever gonna be fixed, so we do a few very basic checks here to filter out obvious rubbish.

		if (Size.x <= 0 || Size.y <= 0)
			return false;

		// A typical screen ratio these days is roughly 2:1.
		// A typical font cell is about 1:2, so the expected screen ratio in cells
		// is around 4 for the landscape and around 1 for the portrait, give or take.
		// Anything twice larger than that is likely rubbish.
		if (Size.x >= 8 * Size.y || Size.y >= 2 * Size.x)
			return false;

		// The API works with SHORTs, anything larger than that makes no sense.
		if (Size.x >= std::numeric_limits<SHORT>::max() || Size.y >= std::numeric_limits<SHORT>::max())
			return false;

		// If we got here, it is either legit or they used some fallback 1x1 font and the proportions are not screwed enough to fail the checks above.
		if (const auto Monitor = MonitorFromWindow(::console.GetWindow(), MONITOR_DEFAULTTONEAREST))
		{
			if (MONITORINFO Info{ sizeof(Info) }; GetMonitorInfo(Monitor, &Info))
			{
				// The smallest selectable in the UI font is 5x2. Anything smaller than that is likely rubbish and unreadable anyway.
				if (const auto AssumedFontHeight = (Info.rcWork.bottom - Info.rcWork.top) / Size.y; AssumedFontHeight < 5)
					return false;

				if (const auto AssumedFontWidth = (Info.rcWork.right - Info.rcWork.left) / Size.x; AssumedFontWidth < 2)
					return false;
			}
		}

		return true;
	}

	point console::GetLargestWindowSize(HANDLE const ConsoleOutput) const
	{
		point Result = GetLargestConsoleWindowSize(ConsoleOutput);

		if (!validate_console_size(Result))
		{
			LOGERROR(L"GetLargestConsoleWindowSize(): the reported size {{{}, {}}} makes no sense. Talk to your terminal or OS vendor."sv, Result.x, Result.y);
			return {};
		}

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (get_console_screen_buffer_info(ConsoleOutput, &csbi) && csbi.dwSize.Y > Result.y)
		{
			CONSOLE_FONT_INFO FontInfo;
			if (get_current_console_font(ConsoleOutput, FontInfo))
			{
				Result.x -= std::lround(GetSystemMetrics(SM_CXVSCROLL) * 1.0 / FontInfo.dwFontSize.X);
			}
		}
		return Result;
	}

	bool console::SetActiveScreenBuffer(HANDLE ConsoleOutput)
	{
		if (!SetConsoleActiveScreenBuffer(ConsoleOutput))
		{
			LOGERROR(L"SetConsoleActiveScreenBuffer(): {}"sv, os::last_error());
			return false;
		}

		m_ActiveConsoleScreenBuffer = ConsoleOutput;
		return true;
	}

	HANDLE console::GetActiveScreenBuffer() const
	{
		return m_ActiveConsoleScreenBuffer;
	}

	bool console::ClearExtraRegions(const FarColor& Color, int Mode) const
	{
		if (ExternalConsole.Imports.pClearExtraRegions)
			return ExternalConsole.Imports.pClearExtraRegions(&Color, Mode) != FALSE;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		DWORD CharsWritten;
		const auto ConColor = colors::FarColorToConsoleColor(Color);

		if (Mode&CR_TOP)
		{
			const DWORD TopSize = csbi.dwSize.X * csbi.srWindow.Top;
			const COORD TopCoord{};
			FillConsoleOutputCharacter(GetOutputHandle(), L' ', TopSize, TopCoord, &CharsWritten);
			FillConsoleOutputAttribute(GetOutputHandle(), ConColor, TopSize, TopCoord, &CharsWritten);
		}

		if (Mode&CR_RIGHT)
		{
			const DWORD RightSize = csbi.dwSize.X - csbi.srWindow.Right;
			COORD RightCoord{ csbi.srWindow.Right, ::GetDelta(csbi) };
			for (; RightCoord.Y < csbi.dwSize.Y; RightCoord.Y++)
			{
				FillConsoleOutputCharacter(GetOutputHandle(), L' ', RightSize, RightCoord, &CharsWritten);
				FillConsoleOutputAttribute(GetOutputHandle(), ConColor, RightSize, RightCoord, &CharsWritten);
			}
		}
		return true;
	}

	bool console::Clear(const FarColor& Color) const
	{
		ClearExtraRegions(Color, CR_BOTH);

		point ViewportSize;
		if (!GetSize(ViewportSize))
			return false;

		const auto ConColor = colors::FarColorToConsoleColor(Color);
		const DWORD Size = ViewportSize.x * ViewportSize.y;

		COORD const Coord{ 0, GetDelta() };
		DWORD CharsWritten;
		FillConsoleOutputCharacter(GetOutputHandle(), L' ', Size, Coord, &CharsWritten);
		FillConsoleOutputAttribute(GetOutputHandle(), ConColor, Size, Coord, &CharsWritten);

		return true;
	}

	bool console::ScrollWindow(int Lines, int Columns) const
	{
		bool process = false;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		if ((Lines < 0 && csbi.srWindow.Top) || (Lines > 0 && csbi.srWindow.Bottom != csbi.dwSize.Y - 1))
		{
			csbi.srWindow.Top += Lines;
			csbi.srWindow.Bottom += Lines;

			if (csbi.srWindow.Top < 0)
			{
				csbi.srWindow.Bottom -= csbi.srWindow.Top;
				csbi.srWindow.Top = 0;
			}

			if (csbi.srWindow.Bottom >= csbi.dwSize.Y)
			{
				csbi.srWindow.Top -= (csbi.srWindow.Bottom - (csbi.dwSize.Y - 1));
				csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
			}
			process = true;
		}

		if ((Columns < 0 && csbi.srWindow.Left) || (Columns > 0 && csbi.srWindow.Right != csbi.dwSize.X - 1))
		{
			csbi.srWindow.Left += Columns;
			csbi.srWindow.Right += Columns;

			if (csbi.srWindow.Left < 0)
			{
				csbi.srWindow.Right -= csbi.srWindow.Left;
				csbi.srWindow.Left = 0;
			}

			if (csbi.srWindow.Right >= csbi.dwSize.X)
			{
				csbi.srWindow.Left -= (csbi.srWindow.Right - (csbi.dwSize.X - 1));
				csbi.srWindow.Right = csbi.dwSize.X - 1;
			}
			process = true;
		}

		return process && SetWindowRect(csbi.srWindow);
	}

	bool console::ScrollWindowToBegin() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		if (!csbi.srWindow.Top)
			return false;

		csbi.srWindow.Bottom -= csbi.srWindow.Top;
		csbi.srWindow.Top = 0;
		return SetWindowRect(csbi.srWindow);
	}

	bool console::ScrollWindowToEnd() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))

		if (csbi.srWindow.Bottom == csbi.dwSize.Y - 1)
			return false;

		csbi.srWindow.Top += csbi.dwSize.Y - 1 - csbi.srWindow.Bottom;
		csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
		return SetWindowRect(csbi.srWindow);
	}

	bool console::IsFullscreenSupported() const
	{
#ifdef _WIN64
		return false;
#else
		if (!imports.GetConsoleScreenBufferInfoEx)
			return true;

		CONSOLE_SCREEN_BUFFER_INFOEX csbiex{ sizeof(csbiex) };
		if (!imports.GetConsoleScreenBufferInfoEx(GetOutputHandle(), &csbiex))
		{
			LOGWARNING(L"GetConsoleScreenBufferInfoEx(): {}"sv, os::last_error());
			return true;
		}

		return csbiex.bFullscreenSupported != FALSE;
#endif
	}

	bool console::ResetViewportPosition() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		rectangle const Window = csbi.srWindow;
		point SavedCursorPosition;
		const auto RestoreCursorPosition = GetCursorRealPosition(SavedCursorPosition) && SavedCursorPosition.y > csbi.dwSize.Y - Window.height() && SavedCursorPosition.x < Window.width();

		SCOPED_ACTION(hide_cursor);

		// Move the viewport down
		if (!SetCursorRealPosition({ 0, csbi.dwSize.Y - 1 }))
			return false;

		if (RestoreCursorPosition)
			(void)SetCursorRealPosition(SavedCursorPosition);

		return true;
	}

	bool console::IsVtEnabled() const
	{
		DWORD Mode;
		return GetMode(GetOutputHandle(), Mode) && Mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	}

	short console::GetDelta() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return 0;

		return ::GetDelta(csbi);
	}

	bool console::input_queue_inspector::search(function_ref<bool(INPUT_RECORD const&)> Predicate)
	{
		const auto NumberOfEvents = []
		{
			size_t Result;
			return ::console.GetNumberOfInputEvents(Result)? Result : 0;
		}();

		if (m_Buffer.size() < NumberOfEvents)
		{
			m_Buffer.clear();
			resize_exp(m_Buffer, NumberOfEvents);
		}

		if (!os::handle::is_signaled(::console.GetInputHandle(), 100ms))
			return false;

		DWORD EventsRead = 0;
		if (!PeekConsoleInput(::console.GetInputHandle(), m_Buffer.data(), static_cast<DWORD>(m_Buffer.size()), &EventsRead))
		{
			LOGERROR(L"PeekConsoleInput(): {}"sv, os::last_error());
			return false;
		}

		return std::ranges::any_of(m_Buffer | std::views::take(EventsRead), Predicate);
	}

	bool console::ScrollScreenBuffer(rectangle const& ScrollRectangle, point DestinationOrigin, const FAR_CHAR_INFO& Fill) const
	{
		const CHAR_INFO SysFill{ { Fill.Char }, colors::FarColorToConsoleColor(Fill.Attributes) };
		const auto SysScrollRect = make_rect(ScrollRectangle);
		return ScrollConsoleScreenBuffer(GetOutputHandle(), &SysScrollRect, {}, make_coord(DestinationOrigin), &SysFill) != FALSE;
	}

	bool console::ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		const auto Scroll = [&](rectangle const& Rect)
		{
			return ScrollScreenBuffer(
				Rect,
				{
					Rect.left,
					static_cast<int>(Rect.top - NumLines)
				},
				Fill);
		};


		rectangle const TopRectangle
		{
			0,
			0,
			csbi.dwSize.X - 1,
			csbi.dwSize.Y - 1 - (ScrY + 1)
		};

		if (TopRectangle.bottom >= TopRectangle.top && !Scroll(TopRectangle))
			return false;

		rectangle const RightRectangle
		{
			ScrX + 1,
			TopRectangle.bottom + 1,
			csbi.dwSize.X - 1,
			csbi.dwSize.Y - 1
		};

		if (RightRectangle.right >= RightRectangle.left && !Scroll(RightRectangle))
			return false;

		return true;
	}

	bool console::IsViewportVisible() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		const auto Height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		const auto Width = csbi.srWindow.Right - csbi.srWindow.Left + 1;

		return csbi.srWindow.Bottom >= csbi.dwSize.Y - Height && csbi.srWindow.Left < Width;
	}

	bool console::IsViewportShifted() const
	{
		if (!sWindowMode)
			return false;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		return csbi.srWindow.Left || csbi.srWindow.Bottom + 1 != csbi.dwSize.Y;
	}

	bool console::IsPositionVisible(point const Position) const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &csbi))
			return false;

		if (!in_closed_range(csbi.srWindow.Left, Position.x, csbi.srWindow.Right))
			return false;

		const auto RealY = Position.y + (sWindowMode? ::GetDelta(csbi) : 0);
		return in_closed_range(csbi.srWindow.Top, RealY, csbi.srWindow.Bottom);
	}

	bool console::IsScrollbackPresent() const
	{
		return GetDelta() != 0;
	}

	bool console::IsVtActive() const
	{
		return sEnableVirtualTerminal && IsVtEnabled();
	}

	bool console::ExternalRendererLoaded() const
	{
		return ExternalConsole.Imports.pWriteOutput.operator bool();
	}

	size_t console::GetWidthPreciseExpensive(char32_t const Codepoint)
	{
		// It ain't stupid if it works

		const auto initialize = [this]
		{
			m_WidthTestScreen.reset(CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, {}, {}, CONSOLE_TEXTMODE_BUFFER, {}));

			const auto TestScreenX = 20, TestScreenY = 1;
			const SMALL_RECT WindowInfo{ 0, 0, TestScreenX - 1, TestScreenY - 1 };
			if (!SetConsoleWindowInfo(m_WidthTestScreen.native_handle(), true, &WindowInfo))
			{
				LOGWARNING(L"SetConsoleWindowInfo(): {}"sv, os::last_error());
			}

			if (!SetConsoleScreenBufferSize(m_WidthTestScreen.native_handle(), { TestScreenX, TestScreenY }))
			{
				LOGWARNING(L"SetConsoleScreenBufferSize(): {}"sv, os::last_error());
			}
		};

		if (!m_WidthTestScreen)
		{
			initialize();
		}

		while (!SetConsoleCursorPosition(m_WidthTestScreen.native_handle(), {}))
		{
			LOGWARNING(L"SetConsoleCursorPosition(): {}"sv, os::last_error());

			if (GetLastError() != ERROR_INVALID_HANDLE)
				return 1;

			LOGINFO(L"Reinitializing"sv);
			initialize();
			return 1;
		}

		DWORD Written;
		const auto Pair = encoding::utf16::to_surrogate(Codepoint);
		const std::array Chars{ Pair.first, Pair.second };
		if (!WriteConsole(m_WidthTestScreen.native_handle(), Chars.data(), Pair.second? 2 : 1, &Written, {}))
		{
			LOGWARNING(L"WriteConsole(): {}"sv, os::last_error());
			return 1;
		}

		CONSOLE_SCREEN_BUFFER_INFO Info;
		if (!get_console_screen_buffer_info(m_WidthTestScreen.native_handle(), &Info))
			return 1;

		return Info.dwCursorPosition.X;
	}

	void console::ClearWideCache()
	{
		m_WidthTestScreen = {};
	}

	bool console::GetPalette(std::array<COLORREF, 256>& Palette) const
	{
		// Happy path
		const auto VtEnabled = IsVtEnabled();
		if (VtEnabled && implementation::GetPaletteVT(Palette))
			return true;

		// Legacy console
		if (VtEnabled || !IsVtSupported())
			return implementation::GetPaletteNT(Palette);

		// If VT is not enabled, we enable it temporarily and use VT method if we can:
		if ([[maybe_unused]] scoped_vt_output const VtOutput{})
			return implementation::GetPaletteVT(Palette);

		// Otherwise fallback to NT
		return implementation::GetPaletteNT(Palette);
	}

	bool console::SetPalette(std::array<COLORREF, 256> const& Palette) const
	{
		// Happy path
		const auto VtEnabled = IsVtEnabled();
		if (VtEnabled && implementation::SetPaletteVT(Palette))
			return true;

		// Legacy console
		if (VtEnabled || !IsVtSupported())
			return implementation::SetPaletteNT(Palette);

		// These methods are currently not synchronized in WT 🤦
		// As of 8 Oct 2022 NT method doesn't affect the display, only the array returned in CSBI.
		// VT does and updates the CSBI too.

		// If VT is not enabled, we enable it temporarily and use VT method if we can:
		if ([[maybe_unused]] scoped_vt_output const VtOutput{})
			return implementation::SetPaletteVT(Palette);

		// Otherwise fallback to NT
		return implementation::SetPaletteNT(Palette);
	}

	void console::EnableWindowMode(bool const Value)
	{
		sWindowMode = Value;
	}

	void console::EnableVirtualTerminal(bool const Value)
	{
		sEnableVirtualTerminal = Value;
	}

	static wchar_t state_to_vt(TBPFLAG const State)
	{
		switch (State)
		{
		case TBPF_NOPROGRESS:    return L'0';
		case TBPF_INDETERMINATE: return L'3';
		case TBPF_NORMAL:        return L'1';
		case TBPF_ERROR:         return L'2';
		case TBPF_PAUSED:        return L'4';
		default:
			std::unreachable();
		}
	}

	void console::set_progress_state(TBPFLAG const State) const
	{
		send_vt_command(far::format(OSC(L"9;4;{}"), state_to_vt(State)));
	}

	void console::set_progress_value(TBPFLAG const State, size_t const Percent) const
	{
		// 🤦
		send_vt_command(far::format(OSC(L"9;4;{};{}"), state_to_vt(State), Percent));
	}

// I'd prefer a more obscure number, but looks like only 1-6 are supported
#define SERVICE_PAGE_NUMBER "3"

	void console::stash_output() const
	{
		send_vt_command(CSI L";;;;1;;;" SERVICE_PAGE_NUMBER "$v"sv);
	}

	void console::unstash_output(rectangle const Coordinates) const
	{
		send_vt_command(far::format(
			CSI L"{};{};{};{};" SERVICE_PAGE_NUMBER ";{};{};1$v"sv,
			1 + Coordinates.top,
			1 + Coordinates.left,
			1 + Coordinates.bottom,
			1 + Coordinates.right,
			1 + Coordinates.top,
			1 + Coordinates.left
		));
	}

#undef SERVICE_PAGE_NUMBER

	void console::start_prompt() const
	{
		send_vt_command(OSC("133;D"));
		send_vt_command(OSC("133;A"));
	}

	void console::start_command() const
	{
		send_vt_command(OSC("133;B"));
	}

	void console::start_output() const
	{
		send_vt_command(OSC("133;C"));
	}

	void console::command_finished() const
	{
		send_vt_command(OSC("133;D"));
	}

	void console::command_finished(int const ExitCode) const
	{
		send_vt_command(far::format(OSC("133;D;{}"), ExitCode));
	}

	void console::command_not_found(string_view const Command) const
	{
		send_vt_command(far::format(OSC("9001;CmdNotFound;{}"), Command));
	}

	bool console::GetCursorRealPosition(point& Position) const
	{
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!get_console_screen_buffer_info(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		Position = ConsoleScreenBufferInfo.dwCursorPosition;
		return true;
	}

	bool console::SetCursorRealPosition(point const Position) const
	{
		if (!SetConsoleCursorPosition(GetOutputHandle(), make_coord(Position)))
		{
			LOGERROR(L"SetConsoleCursorPosition(): {}"sv, os::last_error());
			return false;
		}

		return true;
	}

	bool console::send_vt_command(string_view Command) const
	{
		// Happy path
		if (::console.IsVtEnabled())
			return Write(Command);

		// Legacy console
		if (!IsVtSupported())
			return false;

		// If VT is not enabled, we enable it temporarily
		if ([[maybe_unused]] scoped_vt_output const VtOutput{})
			return Write(Command);

		return false;
	}
}

NIFTY_DEFINE(console_detail::console, console);

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("console.vt_color")
{
	const auto I = FCF_INDEXMASK;

	static const struct
	{
		FarColor Color;
		string_view Fg, Bg;
	}
	Tests[]
	{
		{ { I, { 0x0      }, { 0x0      } }, L"30"sv,               L"40"sv,               },
		{ { I, { 0x1      }, { 0x1      } }, L"34"sv,               L"44"sv,               },
		{ { I, { 0x7      }, { 0x7      } }, L"37"sv,               L"47"sv,               },
		{ { I, { 0x8      }, { 0x8      } }, L"90"sv,               L"100"sv,              },
		{ { I, { 0x9      }, { 0x9      } }, L"94"sv,               L"104"sv,              },
		{ { I, { 0xF      }, { 0xF      } }, L"97"sv,               L"107"sv,              },
		{ { I, { 0x10     }, { 0x10     } }, L"38;5;16"sv,          L"48;5;16"sv,          },
		{ { I, { 0xC0     }, { 0xC0     } }, L"38;5;192"sv,         L"48;5;192"sv,         },
		{ { I, { 0xFF     }, { 0xFF     } }, L"38;5;255"sv,         L"48;5;255"sv,         },
		{ { 0, { 0x000000 }, { 0x000000 } }, L"38;2;0;0;0"sv,       L"48;2;0;0;0"sv,       },
		{ { 0, { 0x123456 }, { 0x654321 } }, L"38;2;86;52;18"sv,    L"48;2;33;67;101"sv,   },
		{ { 0, { 0x00D5FF }, { 0xBB5B00 } }, L"38;2;255;213;0"sv,   L"48;2;0;91;187"sv,    },
		{ { 0, { 0xABCDEF }, { 0xFEDCBA } }, L"38;2;239;205;171"sv, L"48;2;186;220;254"sv, },
		{ { 0, { 0xFFFFFF }, { 0xFFFFFF } }, L"38;2;255;255;255"sv, L"48;2;255;255;255"sv, },
	};

	for (const auto& i: Tests)
	{
		string Str[2];
		console_detail::make_vt_color(colors::single_color::foreground(i.Color), console_detail::colors_mapping_type::foreground, Str[0]);
		console_detail::make_vt_color(colors::single_color::background(i.Color), console_detail::colors_mapping_type::background, Str[1]);
		REQUIRE(Str[0] == i.Fg);
		REQUIRE(Str[1] == i.Bg);
	}
}

TEST_CASE("console.vt_sequence")
{
	FAR_CHAR_INFO const def{ L' ', {}, {}, colors::default_color() };

	const auto check = [](std::span<FAR_CHAR_INFO> const Buffer, string_view const Expected)
	{
		string Actual;
		auto LastColor = colors::default_color();
		console_detail::make_vt_sequence(Buffer, Actual, LastColor);
		REQUIRE(Expected == Actual);
	};

#define SGR(modes) CSI #modes "m"
#define VTSTR(str) L"" str ""sv

	{
		FAR_CHAR_INFO Buffer[]{ def };
		check(Buffer, L" "sv);
	}

	{
		FAR_CHAR_INFO Buffer[]{ def, def, def, def };
		Buffer[1].Attributes.BackgroundColor = colors::opaque(C_MAGENTA);
		Buffer[2].Attributes.ForegroundColor = colors::opaque(C_GREEN);
		Buffer[3].Attributes.Flags |= FCF_FG_BOLD;
		check(Buffer, VTSTR(
			" "
			SGR(45) " "
			SGR(32;49) " "
			SGR(39;1) " "
		));
	}

	{
		FAR_CHAR_INFO Buffer[]{ def, def, def };
		Buffer[0].Attributes.Flags |= FCF_FG_BOLD;
		Buffer[0].Attributes.BackgroundColor = colors::opaque(C_BLUE);
		Buffer[0].Attributes.ForegroundColor = colors::opaque(C_LIGHTGREEN);

		Buffer[1] = Buffer[0];

		Buffer[2] = Buffer[1];
		flags::clear(Buffer[2].Attributes.Flags, FCF_FG_BOLD);

		check(Buffer, VTSTR(
			SGR(92;44;1) "  "
			SGR(22) " "
		));
	}

	{
		FAR_CHAR_INFO Buffer[]{ def, def, def, def, def };
		Buffer[0].Attributes.BackgroundColor = colors::opaque(C_BLUE);
		Buffer[0].Attributes.ForegroundColor = colors::opaque(C_YELLOW);

		Buffer[1] = Buffer[0];
		Buffer[1].Attributes.SetUnderline(UNDERLINE_CURLY);
		Buffer[1].Attributes.UnderlineColor = Buffer[1].Attributes.ForegroundColor;
		Buffer[1].Attributes.SetUnderlineIndex(Buffer[1].Attributes.IsFgIndex());

		Buffer[2] = Buffer[1];
		Buffer[2].Attributes.UnderlineColor = colors::opaque(C_RED);

		Buffer[3] = Buffer[1];

		Buffer[4] = Buffer[3];
		Buffer[4].Attributes.ForegroundColor = colors::opaque(C_MAGENTA);
		Buffer[4].Attributes.UnderlineColor = colors::opaque(C_MAGENTA);


		check(Buffer, VTSTR(
			SGR(93;44) " "
			SGR(4:3) " "
			SGR(58:5:1) " "
			SGR(59) " "
			SGR(35) " "
		));
	}

	{
		FAR_CHAR_INFO Buffer[]{ def, def, def, def, def, def };

		Buffer[0].Attributes.BackgroundColor = colors::opaque(C_BLUE);
		Buffer[0].Attributes.ForegroundColor = colors::opaque(C_LIGHTGREEN);
		Buffer[0].Attributes.SetUnderline(UNDERLINE_DOUBLE);

		Buffer[1] = Buffer[0];
		Buffer[1].Attributes.SetUnderline(UNDERLINE_CURLY);
		Buffer[1].Attributes.UnderlineColor = colors::opaque(C_YELLOW);

		Buffer[2] = Buffer[1];
		Buffer[2].Attributes.SetUnderline(UNDERLINE_DOT);

		Buffer[3] = Buffer[2];
		Buffer[3].Attributes.SetUnderline(UNDERLINE_DASH);
		Buffer[3].Attributes.UnderlineColor = colors::opaque(0x112233);
		Buffer[3].Attributes.SetUnderlineIndex(false);

		Buffer[4] = Buffer[3];
		Buffer[4].Attributes.SetUnderline(UNDERLINE_NONE);
		Buffer[4].Attributes.UnderlineColor = colors::opaque(0xAABBCC);

		Buffer[5] = Buffer[4];
		Buffer[5].Attributes.SetUnderline(UNDERLINE_NONE);
		Buffer[5].Attributes.UnderlineColor = colors::opaque(0xFF06B5);


		check(Buffer, VTSTR(
			SGR(92;44;21) " "
			SGR(4:3;58:5:11)  " "
			SGR(4:4) " "
			SGR(4:5;58:2::51:34:17) " "
			SGR(24;59)
			"  "
		));
	}

#undef VTSTR
#undef SGR
}

#endif
