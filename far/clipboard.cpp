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

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/enum_substrings.hpp"
#include "common/singleton.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

void default_clipboard_mode::set(clipboard_mode Mode) noexcept
{
	m_Mode = Mode;
}

clipboard_mode default_clipboard_mode::get() noexcept
{
	return m_Mode;
}

//-----------------------------------------------------------------------------
enum class clipboard::clipboard_format
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
class system_clipboard: public clipboard, public singleton<system_clipboard>
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

		for (size_t i = 0; i != Attempts; ++i)
		{
			if (OpenClipboard(console.GetWindow()))
			{
				m_Opened = true;
				return true;
			}

			os::chrono::sleep_for((i + 1) * 50ms);
		}

		return false;
	}

	bool Close() noexcept override
	{
		// Closing already closed buffer is OK
		if (!m_Opened)
			return true;

		if (!CloseClipboard())
			return false;

		m_Opened = false;
		return true;
	}

	bool Clear() override
	{
		assert(m_Opened);

		return EmptyClipboard() != FALSE;
	}

private:
	system_clipboard() = default;

	HANDLE GetData(unsigned uFormat) const override
	{
		assert(m_Opened);

		return GetClipboardData(uFormat);
	}

	bool SetData(unsigned const Format, os::memory::global::ptr&& Data) override
	{
		assert(m_Opened);

		if (!SetClipboardData(Format, Data.get()))
			return false;

		// Owned by the OS now
		(void)Data.release();
		return true;
	}

	unsigned RegisterFormat(clipboard_format Format) const override
	{
		static std::pair<const wchar_t*, unsigned> FormatNames[] =
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
		}
		return FormatId;
	}

	bool IsFormatAvailable(unsigned Format) const override
	{
		return IsClipboardFormatAvailable(Format) != FALSE;
	}
};

//-----------------------------------------------------------------------------
class internal_clipboard: public clipboard, public singleton<internal_clipboard>
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

		m_InternalData.clear();
		return true;
	}

private:
	internal_clipboard() = default;

	HANDLE GetData(unsigned uFormat) const override
	{
		assert(m_Opened);

		if (!m_Opened || !uFormat)
			return nullptr;

		const auto ItemIterator = m_InternalData.find(uFormat);
		return ItemIterator != m_InternalData.cend()? ItemIterator->second.get() : nullptr;
	}

	bool SetData(unsigned uFormat, os::memory::global::ptr&& hMem) override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_InternalData[uFormat] = std::move(hMem);
		return true;
	}

	unsigned RegisterFormat(clipboard_format Format) const override
	{
		enum { FarClipboardMagic = 0xFC };
		return static_cast<unsigned>(Format) + FarClipboardMagic;
	}

	bool IsFormatAvailable(unsigned Format) const override
	{
		return Format && m_InternalData.count(Format);
	}

	std::unordered_map<unsigned, os::memory::global::ptr> m_InternalData;
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

bool clipboard::SetText(const string_view Str)
{
	if (!Clear())
		return false;

	auto hData = os::memory::global::copy(Str);
	if (!hData)
		return false;

	if (!SetData(CF_UNICODETEXT, std::move(hData)))
		return false;

	// 'Notepad++ binary text length'
	// return value is ignored - non-critical feature
	SetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length), os::memory::global::copy(static_cast<uint32_t>(Str.size())));

	// return value is ignored - non-critical feature
	if (auto Locale = os::memory::global::copy(GetUserDefaultLCID()))
		SetData(CF_LOCALE, std::move(Locale));

	return true;
}

bool clipboard::SetVText(const string_view Str)
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
	SetData(RegisterFormat(clipboard_format::borland_ide_dev_block), os::memory::global::copy('\x02'));

	// 'MSDEVColumnSelect'
	// return value is ignored - non-critical feature
	SetData(RegisterFormat(clipboard_format::ms_dev_column_select), os::memory::global::copy(0));

	return true;
}

bool clipboard::SetHDROP(const string_view NamesData, const bool bMoved)
{
	if (NamesData.empty())
		return false;

	auto Memory = os::memory::global::alloc(GMEM_MOVEABLE, sizeof(DROPFILES) + (NamesData.size() + 1) * sizeof(wchar_t));
	if (!Memory)
		return false;

	const auto Drop = os::memory::global::lock<LPDROPFILES>(Memory);
	if (!Drop)
		return false;

	Drop->pFiles = sizeof(DROPFILES);
	Drop->pt.x = 0;
	Drop->pt.y = 0;
	Drop->fNC = TRUE;
	Drop->fWide = TRUE;
	*copy_string(NamesData, static_cast<wchar_t*>(static_cast<void*>(Drop.get() + 1))) = {};

	if (!Clear() || !SetData(CF_HDROP, std::move(Memory)))
		return false;

	if (!bMoved)
		return true;

	auto hMemoryMove = os::memory::global::copy<DWORD>(DROPEFFECT_MOVE);
	if (!hMemoryMove)
		return false;

	return SetData(RegisterFormat(clipboard_format::preferred_drop_effect), std::move(hMemoryMove));
}

bool clipboard::GetText(string& Data) const
{
	const auto hClipData = GetData(CF_UNICODETEXT);
	if (!hClipData)
		return GetHDROPAsText(Data);

	const auto ClipAddr = os::memory::global::lock<const wchar_t*>(hClipData);
	if (!ClipAddr)
		return false;

	const auto GetBinaryTextLength = [this]
	{
		const auto hClipDataLen = GetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length));
		if (!hClipDataLen)
			return string::npos;

		const auto ClipLength = os::memory::global::lock<const uint32_t*>(hClipDataLen);
		return ClipLength? static_cast<size_t>(*ClipLength) : string::npos;
	};

	const auto DataSize = GetBinaryTextLength();
	Data.assign(ClipAddr.get(), DataSize == string::npos? wcslen(ClipAddr.get()) : DataSize);
	return true;
}

bool clipboard::GetHDROPAsText(string& data) const
{
	const auto hClipData = GetData(CF_HDROP);
	if (!hClipData)
		return false;

	const auto Files = os::memory::global::lock<const DROPFILES*>(hClipData);
	if (!Files)
		return false;

	const auto StartA=reinterpret_cast<const char*>(Files.get()) + Files->pFiles;

	const auto Eol = eol::system.str();
	if (Files->fWide)
	{
		const auto Start = reinterpret_cast<const wchar_t*>(StartA);

		for (const auto& i: enum_substrings(Start))
		{
			append(data, i, Eol);
		}
	}
	else
	{
		for (const auto& i: enum_substrings(StartA))
		{
			append(data, encoding::ansi::get_chars(i), Eol);
		}
	}

	return true;
}

bool clipboard::GetVText(string& Data) const
{
	const auto IsBorlandVerticalBlock = [this]
	{
		const auto hClipData = GetData(RegisterFormat(clipboard_format::borland_ide_dev_block));
		if (!hClipData)
			return false;

		const auto ClipAddr = os::memory::global::lock<const char*>(hClipData);
		return ClipAddr && *ClipAddr == 0x02;
	};

	if (IsFormatAvailable(RegisterFormat(clipboard_format::vertical_block_unicode)) ||
	    IsFormatAvailable(RegisterFormat(clipboard_format::ms_dev_column_select)) ||
	    IsBorlandVerticalBlock())
	{
		return GetText(Data);
	}

	const auto hClipData = GetData(RegisterFormat(clipboard_format::vertical_block_oem));
	if (!hClipData)
		return false;

	const auto OemData = os::memory::global::lock<const char*>(hClipData);
	if (!OemData)
		return false;

	Data = encoding::oem::get_chars(OemData.get());
	return true;
}

//-----------------------------------------------------------------------------
bool SetClipboardText(const string_view Str)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->SetText(Str);
}

bool SetClipboardVText(const string_view Str)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->SetVText(Str);
}

bool GetClipboardText(string& data)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->GetText(data);
}

bool GetClipboardVText(string& data)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->GetVText(data);
}

bool ClearClipboard()
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->Clear();
}

bool ClearInternalClipboard()
{
	clipboard_accessor Clip(clipboard_mode::internal);
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
