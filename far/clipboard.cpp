/*
clipboard.cpp

Работа с буфером обмена.
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
#include "clipboard.hpp"

// Internal:
#include "console.hpp"
#include "encoding.hpp"
#include "eol.hpp"
#include "exception.hpp"
#include "log.hpp"

// Platform:
#include "platform.chrono.hpp"
#include "platform.process.hpp"

// Common:
#include "common/enum_substrings.hpp"
#include "common/singleton.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

static bool s_AnsiToUnicodeConversionWorkaround = false;

void default_clipboard_mode::set(clipboard_mode Mode) noexcept
{
	m_Mode = Mode;
}

clipboard_mode default_clipboard_mode::get() noexcept
{
	return m_Mode;
}

//-----------------------------------------------------------------------------
enum class clipboard_format
{
	vertical_block_oem,
	vertical_block_unicode,
	preferred_drop_effect,
	ms_dev_column_select,
	borland_ide_dev_block,
	notepad_plusplus_binary_text_length,

	count
};

//-----------------------------------------------------------------------------
class system_clipboard final: public clipboard, public singleton<system_clipboard>
{
	IMPLEMENTS_SINGLETON;

public:
	~system_clipboard() override
	{
		system_clipboard::Close();
	}

	bool Open() override
	{
		assert(!m_Opened);

		if (m_Opened)
			return false;

		// Clipboard is a shared resource
		const size_t Attempts = 5;

		for (const auto& i: irange(Attempts))
		{
			// TODO: this is bad, we should use a real window handle
			if (OpenClipboard(console.GetWindow()))
			{
				m_Opened = true;
				return true;
			}

			const auto Error = last_error();

			if (Error.Win32Error == ERROR_ACCESS_DENIED)
			{
				if (const auto Window = GetOpenClipboardWindow())
				{
					DWORD Pid;
					if (const auto ThreadId = GetWindowThreadProcessId(Window, &Pid))
					{
						LOGWARNING(L"Clipboard is locked by {} (PID {}, TID {})"sv, os::process::get_process_name(Pid), Pid, ThreadId);
					}
				}
			}

			LOGDEBUG(L"OpenClipboard(): {}"sv, Error);

			os::chrono::sleep_for((i + 1) * 50ms);
		}

		LOGERROR(L"OpenClipboard(): {}"sv, last_error());
		return false;
	}

	bool Close() noexcept override
	{
		// Closing already closed buffer is OK
		if (!m_Opened)
			return true;

		if (!CloseClipboard())
		{
			LOGERROR(L"CloseClipboard(): {}"sv, last_error());
			return false;
		}

		m_Opened = false;
		return true;
	}

	bool Clear() override
	{
		assert(m_Opened);

		if (!EmptyClipboard())
		{
			LOGERROR(L"EmptyClipboard(): {}"sv, last_error());
			return false;
		}

		return true;
	}

	bool SetText(const string_view Str) override
	{
		if (!Clear())
			return false;

		auto hData = os::memory::global::copy(Str);
		if (!hData)
		{
			LOGERROR(L"global::copy(): {}"sv, last_error());
			return false;
		}

		if (!SetData(CF_UNICODETEXT, std::move(hData)))
			return false;

		// 'Notepad++ binary text length'
		// return value is ignored - non-critical feature
		if (const auto Format = RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length))
		{
			if (auto Size = os::memory::global::copy(static_cast<uint32_t>(Str.size())))
				SetData(Format, std::move(Size));
			else
				LOGWARNING(L"global::copy(): {}"sv, last_error());
		}

		// return value is ignored - non-critical feature
		if (auto Locale = os::memory::global::copy(GetUserDefaultLCID()))
			SetData(CF_LOCALE, std::move(Locale));
		else
			LOGWARNING(L"global::copy(): {}"sv, last_error());

		return true;
	}

	bool SetVText(const string_view Str) override
	{
		if (!SetText(Str))
			return false;

		const auto FarVerticalBlock = RegisterFormat(clipboard_format::vertical_block_unicode);
		if (!FarVerticalBlock)
			return false;

		if (!SetData(FarVerticalBlock, os::memory::global::copy(0)))
			return false;

		// 'Borland IDE Block Type'
		// return value is ignored - non-critical feature
		if (const auto Format = RegisterFormat(clipboard_format::borland_ide_dev_block))
			SetData(Format, os::memory::global::copy('\2'));

		// 'MSDEVColumnSelect'
		// return value is ignored - non-critical feature
		if (const auto Format = RegisterFormat(clipboard_format::ms_dev_column_select))
			SetData(Format, os::memory::global::copy(0));

		return true;
	}

	bool SetHDROP(const string_view NamesData, const bool Move) override
	{
		if (NamesData.empty())
			return false;

		auto Memory = os::memory::global::alloc(GMEM_MOVEABLE, sizeof(DROPFILES) + (NamesData.size() + 1) * sizeof(wchar_t));
		if (!Memory)
		{
			LOGERROR(L"global::alloc(): {}"sv, last_error());
			return false;
		}

		const auto Drop = os::memory::global::lock<LPDROPFILES>(Memory);
		if (!Drop)
		{
			LOGERROR(L"global::lock(): {}"sv, last_error());
			return false;
		}

		Drop->pFiles = static_cast<DWORD>(aligned_sizeof<DROPFILES, sizeof(wchar_t)>);
		Drop->pt.x = 0;
		Drop->pt.y = 0;
		Drop->fNC = TRUE;
		Drop->fWide = TRUE;
		const auto NamesPtr = edit_as<wchar_t*>(Drop.get(), Drop->pFiles);
		assert(is_aligned(*NamesPtr));
		*copy_string(NamesData, NamesPtr) = {};

		if (!Clear() || !SetData(CF_HDROP, std::move(Memory)))
			return false;

		auto DropEffect = os::memory::global::copy<DWORD>(Move? DROPEFFECT_MOVE : DROPEFFECT_COPY);
		if (!DropEffect)
		{
			LOGERROR(L"global::copy(): {}"sv, last_error());
			return false;
		}

		const auto Format = RegisterFormat(clipboard_format::preferred_drop_effect);
		if (!Format)
			return false;

		return SetData(Format, std::move(DropEffect));
	}

	template<typename T>
	struct clip_ptr: public os::memory::global::lock_t<T const*>
	{
		size_t size{};
	};

	template<typename T>
	static clip_ptr<T> get_as(unsigned const Format)
	{
		if (!IsFormatAvailable(Format))
			return {};

		const auto DataHandle = GetClipboardData(Format);
		if (!DataHandle)
		{
			LOGWARNING(L"GetClipboardData(): {}"sv, last_error());
			return {};
		}

		auto DataPtr = os::memory::global::lock<T const*>(DataHandle);
		if (!DataPtr)
		{
			LOGWARNING(L"global::lock(): {}"sv, last_error());
			return {};
		}

		const auto DataSize = GlobalSize(DataHandle);

		if (DataSize < sizeof(T))
		{
			LOGWARNING(L"Insufficient data"sv);
			return {};
		}

		return { std::move(DataPtr), DataSize };
	}

	static unsigned find_first_supported_text_format()
	{
		for (auto Format = EnumClipboardFormats(0); Format; Format = EnumClipboardFormats(Format))
		{
			if (any_of(as_signed(Format), CF_UNICODETEXT, CF_TEXT))
				return Format;
		}

		return 0;
	}

	static bool get_ansi_text(std::string& Data)
	{
		const auto FirstFormat = find_first_supported_text_format();
		/*
		If CF_UNICODETEXT comes first, then it's definitely not generated. Our job here is done.
		If CF_TEXT comes first, it means nothing. See the comment below.
		*/
		if (FirstFormat != CF_TEXT)
			return false;

		const auto ClipData = get_as<char>(CF_TEXT);
		if (!ClipData)
			return false;

		const std::string_view DataView(ClipData.get(), ClipData.size / sizeof(*ClipData));
		if (DataView.empty())
			return false;

		const auto DataSize = static_cast<size_t>(std::find(ALL_CONST_RANGE(DataView), '\0') - DataView.cbegin());

		Data = DataView.substr(0, DataSize);

		return true;
	}

	static LCID get_locale()
	{
		const auto ClipData = get_as<LCID>(CF_LOCALE);
		if (!ClipData)
			return 0;

		return *ClipData;
	}

	static unsigned get_locale_codepage(LCID const Locale)
	{
		unsigned Acp;
		const int SizeInChars = sizeof(Acp) / sizeof(wchar_t);

		if (GetLocaleInfo(
			Locale,
				LOCALE_IDEFAULTANSICODEPAGE |
				LOCALE_RETURN_NUMBER,
			reinterpret_cast<wchar_t*>(&Acp),
			SizeInChars
		) != SizeInChars)
		{
			LOGWARNING(L"GetLocaleInfo(LOCALE_IDEFAULTANSICODEPAGE): {}"sv, last_error());
			return 0;
		}

		return Acp;
	}

	static void try_to_fix_incorrect_ansi_to_unicode_conversion(string& Data)
	{
		/*
		https://learn.microsoft.com/en-us/windows/win32/dataxchg/standard-clipboard-formats#CF_LOCALE
		When you close the clipboard, if it contains CF_TEXT data but no CF_LOCALE data,
		the system automatically sets the CF_LOCALE format to the current input language.
		The system uses the code page associated with CF_LOCALE to implicitly convert from CF_TEXT to CF_UNICODETEXT.

		If you are pasting information from the clipboard, retrieve the first clipboard format that you can handle.
		That will be the most descriptive clipboard format that you can handle.
		The system provides automatic type conversions for certain clipboard formats.
		In the case of such a format, this function enumerates the specified format,
		then enumerates the formats to which it can be converted.


		In other words, if the user copies a localized text from a non-Unicode application
		when their input language is not the same as the language of the aforementioned text,
		CF_UNICODETEXT will be garbled.

		Surprisingly, sometimes it is not a bug, but a feature.
		E.g. when the ACP is incompatible with the text, the user can switch to a compatible input language
		to guide the conversion to CF_UNICODETEXT and get sensible results... if the user knows about this feature in the first place.
		If they don't, it's rather "WTF why the text is broken?".
		And most users don't know and don't expect this street magic at all.

		So it's a double-edged sword and there is no win-win scenario.
		~15 years of observation show that users are generally expect the pasted text to be the same as copied, but...
		there is no way to even properly choose between CF_TEXT and CF_UNICODETEXT.

		The MSDN verse above claims that clipboard formats are ordered by their descriptiveness, but that's wishful thinking:
		yes, if an app, say, only adds CF_UNICODETEXT, Windows generates the rest and it works, but nothing in this Universe can stop an app from
		being annoyingly smart and adding multiple formats itself in any unholy order.
		And guess what, they do exactly that. At least these:
		- Visual Studio
		- Windows Terminal
		- WordPad
		- The whole "modern Windows UI"
		and who knows how many more add CF_TEXT manually *before* CF_UNICODETEXT. 🤦
		*/

		/*
		The situation is annoying enough to have a workaround,
		but probably isn't widespread enough to have it enabled by default:
		extra conversions below aren't free and pure ANSI apps are too rare these days
		to make everyone pay the price.
		 */
		if (!s_AnsiToUnicodeConversionWorkaround)
			return;

		const auto ClipboardLocale = get_locale();
		if (!ClipboardLocale)
			return;

		const auto ClipboardLocaleCodepage = get_locale_codepage(ClipboardLocale);
		if (!ClipboardLocaleCodepage)
			return;

		std::string AnsiData;
		if (!get_ansi_text(AnsiData) || AnsiData.empty())
			return;

		/*
		As explained above, we cannot trust the order of formats if CF_TEXT comes before CF_UNICODETEXT,
		so we check that CF_TEXT, converted to Unicode using the code page of the declared CF_LOCALE,
		is the same as CF_UNICODETEXT:
		- If it is, the conversion is lossless, either because it was CF_TEXT in the first place
		  and CF_UNICODETEXT was generated from it, or because it's pure ASCII.
		  Either way, it means that we won't make it worse and can proceed.
		- If it is not, it must be one of them smartasses who put it there manually in a wrong order and our job here is done.

		We use starts_with instead of == here because our encoding method, unlike the OS, tries to yield as many
		Unicode characters as possible, even if they're invalid. For these purposes such conversion is still lossless.
		*/
		if (const auto UnicodeData = encoding::get_chars(ClipboardLocaleCodepage, AnsiData); !starts_with(UnicodeData, Data))
			return;

		// Here it comes
		encoding::diagnostics Diagnostics;
		auto RecodedData = encoding::ansi::get_chars(AnsiData, &Diagnostics);

		if (Diagnostics.ErrorPosition || Diagnostics.IncompleteBytes)
			return;

		if (RecodedData == Data)
			return;

		LOGINFO(L"Potentially incorrect CF_UNICODETEXT detected, using CF_TEXT instead"sv);

		Data = std::move(RecodedData);
	}

	bool GetText(string& Data) const override
	{
		const auto ClipData = get_as<wchar_t>(CF_UNICODETEXT);
		if (!ClipData)
			return GetHDROPAsText(Data);

		const string_view DataView(ClipData.get(), ClipData.size / sizeof(*ClipData));

		const auto GetBinaryTextLength = []() -> std::optional<size_t>
		{
			const auto SizeFormat = RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length);
			if (!SizeFormat)
				return {};

			const auto ClipData = get_as<uint32_t>(SizeFormat);
			if (!ClipData)
				return {};

			return *ClipData;
		};

		const auto GetTextLength = [&]
		{
			if (const auto Length = GetBinaryTextLength())
				return *Length;

			return static_cast<size_t>(std::find(ALL_CONST_RANGE(DataView), L'\0') - DataView.cbegin());
		};

		Data = DataView.substr(0, GetTextLength());

		try_to_fix_incorrect_ansi_to_unicode_conversion(Data);

		return true;
	}

	bool GetVText(string& Data) const override
	{
		const auto IsBorlandVerticalBlock = []
		{
			const auto BlockFormat = RegisterFormat(clipboard_format::borland_ide_dev_block);
			if (!BlockFormat)
				return false;

			const auto ClipData = get_as<char>(BlockFormat);
			if (!ClipData)
				return false;

			return *ClipData == '\2';
		};

		if (IsFormatAvailable(RegisterFormat(clipboard_format::vertical_block_unicode)) ||
			IsFormatAvailable(RegisterFormat(clipboard_format::ms_dev_column_select)) ||
			IsBorlandVerticalBlock())
		{
			return GetText(Data);
		}

		const auto OemDataFormat = RegisterFormat(clipboard_format::vertical_block_oem);
		if (!OemDataFormat)
			return false;

		const auto ClipData = get_as<char>(OemDataFormat);
		if (!ClipData)
			return false;

		const std::string_view OemDataView(ClipData.get(), ClipData.size / sizeof(*ClipData));

		const auto OemDataSize = static_cast<size_t>(std::find(ALL_CONST_RANGE(OemDataView), '\0') - OemDataView.cbegin());
		encoding::oem::get_chars(OemDataView.substr(0, OemDataSize), Data);
		return true;
	}

private:
	system_clipboard() = default;

	template<typename char_type>
	static bool copy_strings(string& To, const DROPFILES* Drop, size_t Size)
	{
		const auto Names = std::basic_string_view(view_as<const char_type*>(Drop, Drop->pFiles), (Size - Drop->pFiles) / sizeof(char_type));
		if (Names.empty())
			return false;

		const auto Eol = eol::system.str();
		string Buffer;

		for (const auto& i: enum_substrings(Names))
		{
			if constexpr (std::is_same_v<char_type, wchar_t>)
			{
				append(To, i, Eol);
			}
			else
			{
				Buffer.clear();
				encoding::ansi::get_chars(i, Buffer);
				append(To, Buffer, Eol);
			}
		}

		return true;
	}

	bool GetHDROPAsText(string& data) const
	{
		const auto ClipData = get_as<DROPFILES>(CF_HDROP);
		if (!ClipData)
			return false;

		const auto Copy = ClipData->fWide? copy_strings<wchar_t> : copy_strings<char>;

		return Copy(data, ClipData.get(), ClipData.size);
	}

	bool SetData(unsigned const Format, os::memory::global::ptr&& Data) const
	{
		assert(m_Opened);

		if (!SetClipboardData(Format, Data.get()))
		{
			LOGWARNING(L"SetClipboardData(): {}"sv, last_error());
			return false;
		}

		// Owned by the OS now
		(void)Data.release();

		return true;
	}

	static unsigned RegisterFormat(clipboard_format Format)
	{
		static std::pair<const wchar_t*, unsigned> FormatNames[]
		{
			{ L"FAR_VerticalBlock", 0 },
			{ L"FAR_VerticalBlock_Unicode", 0 },
			{ CFSTR_PREFERREDDROPEFFECT, 0 },
			{ L"MSDEVColumnSelect", 0 },
			{ L"Borland IDE Block Type", 0 },
			{ L"Notepad++ binary text length", 0 },
		};

		static_assert(std::size(FormatNames) == static_cast<size_t>(clipboard_format::count));
		assert(Format < clipboard_format::count);
		auto& [FormatName, FormatId] = FormatNames[static_cast<unsigned>(Format)];
		if (!FormatId)
		{
			FormatId = RegisterClipboardFormat(FormatName);
			if (!FormatId)
			{
				LOGWARNING(L"RegisterClipboardFormat(): {}"sv, last_error());
			}
		}
		return FormatId;
	}

	static bool IsFormatAvailable(unsigned Format)
	{
		return Format && IsClipboardFormatAvailable(Format);
	}
};

//-----------------------------------------------------------------------------
class internal_clipboard final: public clipboard, public singleton<internal_clipboard>
{
	IMPLEMENTS_SINGLETON;

public:
	static auto CreateInstance()
	{
		return std::unique_ptr<clipboard>(new internal_clipboard);
	}

	~internal_clipboard() override
	{
		internal_clipboard::Close();
	}

	bool Open() override
	{
		assert(!m_Opened);

		if (m_Opened)
			return false;

		m_Opened = true;
		return true;
	}

	bool Close() noexcept override
	{
		// Closing already closed buffer is OK
		m_Opened = false;
		return true;
	}

	bool Clear() override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_Data.reset();
		return true;
	}

	bool SetText(string_view Str) override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_Data = Str;
		m_Vertical = false;

		return true;
	}

	bool SetVText(string_view Str) override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_Data = Str;
		m_Vertical = true;

		return true;
	}

	bool SetHDROP(string_view NamesData, bool Moved) override
	{
		return false;
	}

	bool GetText(string& Data) const override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		if (!m_Data)
			return false;

		Data = *m_Data;
		return true;
	}

	bool GetVText(string& Data) const override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		if (!m_Data)
			return false;

		if (!m_Vertical)
			return false;

		Data = *m_Data;
		return true;
	}

private:
	internal_clipboard() = default;

	std::optional<string> m_Data;
	bool m_Vertical;
};

//-----------------------------------------------------------------------------
static thread_local clipboard* OverridenInternalClipboard;

void clipboard_restorer::operator()(const clipboard* Clip) const noexcept
{
	OverridenInternalClipboard = nullptr;
	delete Clip;
}

std::unique_ptr<clipboard, clipboard_restorer> OverrideClipboard()
{
	auto ClipPtr = internal_clipboard::CreateInstance();
	OverridenInternalClipboard = ClipPtr.get();
	return std::unique_ptr<clipboard, clipboard_restorer>(ClipPtr.release());
}

clipboard& clipboard::GetInstance(clipboard_mode Mode)
{
	if (OverridenInternalClipboard)
		return *OverridenInternalClipboard;

	if (Mode == clipboard_mode::system)
		return system_clipboard::instance();

	return internal_clipboard::instance();
}

void clipboard::enable_ansi_to_unicode_conversion_workaround(bool Enable)
{
	s_AnsiToUnicodeConversionWorkaround = Enable;
}

//-----------------------------------------------------------------------------
bool SetClipboardText(const string_view Str)
{
	const clipboard_accessor Clip;
	return Clip->Open() && Clip->SetText(Str);
}

bool SetClipboardVText(const string_view Str)
{
	const clipboard_accessor Clip;
	return Clip->Open() && Clip->SetVText(Str);
}

bool GetClipboardText(string& data)
{
	const clipboard_accessor Clip;
	return Clip->Open() && Clip->GetText(data);
}

bool GetClipboardVText(string& data)
{
	const clipboard_accessor Clip;
	return Clip->Open() && Clip->GetVText(data);
}

bool ClearClipboard()
{
	const clipboard_accessor Clip;
	return Clip->Open() && Clip->Clear();
}

bool ClearInternalClipboard()
{
	const clipboard_accessor Clip(clipboard_mode::internal);
	return Clip->Open() && Clip->Clear();
}

bool CopyData(const clipboard_accessor& From, const clipboard_accessor& To)
{
	string Data;
	if (From->GetVText(Data))
	{
		return To->SetVText(Data);
	}

	if (From->GetText(Data))
	{
		return To->SetText(Data);
	}

	return false;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

class clipboard_guard
{
public:
	NONCOPYABLE(clipboard_guard);

	clipboard_guard()
	{
		const clipboard_accessor Clip(clipboard_mode::system);
		if (!Clip->Open())
			return;

		m_Data.reserve(CountClipboardFormats());

		for (auto i = EnumClipboardFormats(0); i; i = EnumClipboardFormats(i))
		{
			if (i == CF_BITMAP || i == CF_ENHMETAFILE)
				continue;

			m_Data.emplace_back(i, os::memory::global::copy(GetClipboardData(i)));
		}
	}

	~clipboard_guard()
	{
		if (m_Data.empty())
			return;

		const clipboard_accessor Clip(clipboard_mode::system);
		if (!Clip->Open())
			return;

		for (auto& [Format, Data]: m_Data)
		{
			SetClipboardData(Format, Data.release());
		}
	}

private:
	std::vector<std::pair<unsigned, os::memory::global::ptr>> m_Data;
};

TEST_CASE("clipboard.stream")
{
	SCOPED_ACTION(clipboard_guard);

	const auto Baseline = L"\0 Comfortably Numb \0"sv;
	string Str;

	const auto Mode = default_clipboard_mode::get();

	const std::array Types
	{
		std::pair{&SetClipboardText, &GetClipboardText},
		std::pair{&SetClipboardVText, &GetClipboardVText},
	};

	for (const auto i: { clipboard_mode::system, clipboard_mode::internal })
	{
		default_clipboard_mode::set(i);

		for (const auto& [Set, Get]: Types)
		{
			REQUIRE(Set(Baseline));
			REQUIRE(Get(Str));
			REQUIRE(Str == Baseline);

			REQUIRE(ClearClipboard());
			REQUIRE(!Get(Str));
		}
	}

	default_clipboard_mode::set(Mode);
}

TEST_CASE("clipboard.accessors")
{
	SCOPED_ACTION(clipboard_guard);

	const auto Baseline = L"\0 Hey Macarena \0"sv;
	string Str;

	const clipboard_accessor
		ClipSystem(clipboard_mode::system),
		ClipInternal(clipboard_mode::internal);

	REQUIRE(ClipSystem->Open());
	REQUIRE(ClipSystem->SetText(Baseline));
	REQUIRE(ClipInternal->Open());
	REQUIRE(CopyData(ClipSystem, ClipInternal));
	REQUIRE(ClipSystem->Clear());
	REQUIRE(ClipSystem->Close());
	REQUIRE(ClipInternal->GetText(Str));
	REQUIRE(ClipInternal->Clear());
	REQUIRE(ClipInternal->Close());
	REQUIRE(Str == Baseline);
}
#endif
