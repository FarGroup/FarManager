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

// Self:
#include "clipboard.hpp"

// Internal:

// Platform:
#include "platform.clipboard.hpp"

// Common:
#include "common/singleton.hpp"

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
class system_clipboard final: public clipboard, public singleton<system_clipboard>
{
	IMPLEMENTS_SINGLETON;

public:
	bool Open() override
	{
		assert(!m_Opened);

		if (m_Opened)
			return false;

		m_Opened = os::clipboard::open();
		return m_Opened;
	}

	bool Close() noexcept override
	{
		// Closing already closed buffer is OK
		if (!m_Opened)
			return true;

		if (!os::clipboard::close())
			return false;

		m_Opened = false;
		return true;
	}

	bool Clear() override
	{
		assert(m_Opened);
		return os::clipboard::clear();
	}

	bool SetText(const string_view Str) override
	{
		assert(m_Opened);
		return os::clipboard::set_text(Str);
	}

	bool SetVText(const string_view Str) override
	{
		assert(m_Opened);
		return os::clipboard::set_vtext(Str);
	}

	bool SetHDROP(const string_view NamesData, const bool Move) override
	{
		assert(m_Opened);
		return os::clipboard::set_files(NamesData, Move);
	}

	bool GetText(string& Data) const override
	{
		assert(m_Opened);
		return os::clipboard::get_text(Data);
	}

	bool GetVText(string& Data) const override
	{
		assert(m_Opened);
		return os::clipboard::get_vtext(Data);
	}

private:
	system_clipboard() = default;

	~system_clipboard() override
	{
		system_clipboard::Close();
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
	~internal_clipboard() override
	{
		internal_clipboard::Close();
	}

	std::optional<string> m_Data;
	bool m_Vertical{};
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

		m_Data = os::clipboard::testing::capture();
	}

	~clipboard_guard()
	{
		const clipboard_accessor Clip(clipboard_mode::system);
		if (!Clip->Open())
			return;

		os::clipboard::testing::restore(m_Data);
	}

private:
	os::clipboard::testing::state* m_Data{};
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
