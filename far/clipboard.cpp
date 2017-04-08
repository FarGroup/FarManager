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

#include "headers.hpp"
#pragma hdrstop

#include "clipboard.hpp"
#include "console.hpp"
#include "encoding.hpp"

clipboard_mode default_clipboard_mode::m_Mode = clipboard_mode::system;

void default_clipboard_mode::set(clipboard_mode Mode)
{
	m_Mode = Mode;
}

clipboard_mode default_clipboard_mode::get()
{
	return m_Mode;
}

//-----------------------------------------------------------------------------
enum class Clipboard::clipboard_format
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
class system_clipboard: noncopyable, public Clipboard
{
public:
	static Clipboard& GetInstance()
	{
		static system_clipboard s_Clipboard;
		return s_Clipboard;
	}

	virtual ~system_clipboard() override
	{
		system_clipboard::Close();
	}

	virtual bool Open() override
	{
		assert(!m_Opened);

		if (m_Opened)
			return false;

		if (!OpenClipboard(Console().GetWindow()))
			return false;

		m_Opened = true;
		return true;
	}

	virtual bool Close() override
	{
		// Closing already closed buffer is OK
		if (!m_Opened)
			return true;

		if (!CloseClipboard())
			return false;

		m_Opened = false;
		return true;
	}

	virtual bool Clear() override
	{
		assert(m_Opened);

		return EmptyClipboard() != FALSE;
	}

private:
	system_clipboard() = default;

	virtual HANDLE GetData(unsigned uFormat) const override
	{
		assert(m_Opened);

		return GetClipboardData(uFormat);
	}

	virtual bool SetData(unsigned uFormat, os::memory::global::ptr&& hMem) override
	{
		assert(m_Opened);

		if (!SetClipboardData(uFormat, hMem.get()))
			return false;

		hMem.release();

		auto Locale = os::memory::global::copy<LCID>(LOCALE_USER_DEFAULT);
		if (!Locale)
			return false;

		if (!SetClipboardData(CF_LOCALE, Locale.get()))
			return false;

		Locale.release();
		return true;
	}

	virtual unsigned RegisterFormat(clipboard_format Format) const override
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
		auto& FormatData = FormatNames[static_cast<unsigned>(Format)];
		if (!FormatData.second)
		{
			FormatData.second = RegisterClipboardFormat(FormatData.first);
		}
		return FormatData.second;
	}

	virtual bool IsFormatAvailable(unsigned Format) const override
	{
		return IsClipboardFormatAvailable(Format) != FALSE;
	}
};

//-----------------------------------------------------------------------------
class internal_clipboard: noncopyable, public Clipboard
{
public:
	static Clipboard& GetInstance()
	{
		static internal_clipboard s_Clipboard;
		return s_Clipboard;
	}

	static auto CreateInstance()
	{
		return std::unique_ptr<Clipboard>(new internal_clipboard);
	}

	virtual ~internal_clipboard() override
	{
		internal_clipboard::Close();
	}

	virtual bool Open() override
	{
		assert(!m_Opened);

		if (m_Opened)
			return false;

		m_Opened = true;
		return true;
	}

	virtual bool Close() override
	{
		// Closing already closed buffer is OK
		m_Opened = false;
		return true;
	}

	virtual bool Clear() override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_InternalData.clear();
		return true;
	}

private:
	internal_clipboard() = default;

	virtual HANDLE GetData(unsigned uFormat) const override
	{
		assert(m_Opened);

		if (!m_Opened || !uFormat)
			return nullptr;

		const auto ItemIterator = m_InternalData.find(uFormat);
		return ItemIterator != m_InternalData.cend()? ItemIterator->second.get() : nullptr;
	}

	virtual bool SetData(unsigned uFormat, os::memory::global::ptr&& hMem) override
	{
		assert(m_Opened);

		if (!m_Opened)
			return false;

		m_InternalData[uFormat] = std::move(hMem);
		return true;
	}

	virtual unsigned RegisterFormat(clipboard_format Format) const override
	{
		enum { FarClipboardMagic = 0xFC };
		return static_cast<unsigned>(Format) + FarClipboardMagic;
	}

	virtual bool IsFormatAvailable(unsigned Format) const override
	{
		return Format && m_InternalData.count(Format);
	}

	std::unordered_map<unsigned, os::memory::global::ptr> m_InternalData;
};

//-----------------------------------------------------------------------------
static thread_local Clipboard* OverridenInternalClipboard;

void clipboard_restorer::operator()(Clipboard* Clip) const
{
	OverridenInternalClipboard = nullptr;
	delete Clip;
}

std::unique_ptr<Clipboard, clipboard_restorer> OverrideClipboard()
{
	auto ClipPtr = internal_clipboard::CreateInstance();
	OverridenInternalClipboard = ClipPtr.get();
	return std::unique_ptr<Clipboard, clipboard_restorer>(ClipPtr.release());
}

Clipboard& Clipboard::GetInstance(clipboard_mode Mode)
{
	return OverridenInternalClipboard? *OverridenInternalClipboard :
	       Mode == clipboard_mode::system? system_clipboard::GetInstance() : internal_clipboard::GetInstance();
}

bool Clipboard::SetText(const wchar_t *Data, size_t Size)
{
	if (!Clear())
		return false;

	if (!Data)
		return true;

	auto hData = os::memory::global::copy(Data, Size);
	if (!hData)
		return false;

	if (!SetData(CF_UNICODETEXT, std::move(hData)))
		return false;

	// 'Notepad++ binary text length'
	// return value is ignored - non-critical feature
	SetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length), os::memory::global::copy(static_cast<uint32_t>(Size)));

	return true;
}

bool Clipboard::SetVText(const wchar_t *Data, size_t Size)
{
	if (!SetText(Data, Size))
		return false;

	if (!Data)
		return true;

	const auto FarVerticalBlock = RegisterFormat(clipboard_format::vertical_block_unicode);

	if (!FarVerticalBlock)
		return false;

	if (!SetData(FarVerticalBlock, nullptr))
		return false;

	// 'Borland IDE Block Type'
	// return value is ignored - non-critical feature
	SetData(RegisterFormat(clipboard_format::borland_ide_dev_block), os::memory::global::copy('\x02'));

	// 'MSDEVColumnSelect'
	// return value is ignored - non-critical feature
	SetData(RegisterFormat(clipboard_format::ms_dev_column_select), nullptr);

	return true;
}

bool Clipboard::SetHDROP(const string& NamesData, bool bMoved)
{
	if (NamesData.empty())
		return false;

	const auto RawDataSize = (NamesData.size() + 1) * sizeof(wchar_t);
	auto hMemory = os::memory::global::alloc(GMEM_MOVEABLE, sizeof(DROPFILES) + RawDataSize);
	if (!hMemory)
		return false;

	const auto Drop = os::memory::global::lock<LPDROPFILES>(hMemory);
	if (!Drop)
		return false;

	Drop->pFiles = sizeof(DROPFILES);
	Drop->pt.x = 0;
	Drop->pt.y = 0;
	Drop->fNC = TRUE;
	Drop->fWide = TRUE;
	memcpy(Drop.get() + 1, NamesData.data(), RawDataSize);

	if (!Clear() || !SetData(CF_HDROP, std::move(hMemory)))
		return false;

	if (!bMoved)
		return true;

	auto hMemoryMove = os::memory::global::copy<DWORD>(DROPEFFECT_MOVE);
	if (!hMemoryMove)
		return false;

	return SetData(RegisterFormat(clipboard_format::preferred_drop_effect), std::move(hMemoryMove));
}

bool Clipboard::GetText(string& Data) const
{
	auto hClipData = GetData(CF_UNICODETEXT);
	if (!hClipData)
		return GetHDROPAsText(Data);

	const auto ClipAddr = os::memory::global::lock<const wchar_t*>(hClipData);
	if (!ClipAddr)
		return false;

	const auto& GetBinaryTextLength = [this]
	{
		auto hClipDataLen = GetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length));
		if (!hClipDataLen)
			return string::npos;

		const auto ClipLength = os::memory::global::lock<const uint32_t*>(hClipDataLen);
		return ClipLength? static_cast<size_t>(*ClipLength.get()) : string::npos;
	};

	const auto DataSize = GetBinaryTextLength();
	Data.assign(ClipAddr.get(), DataSize == string::npos? wcslen(ClipAddr.get()) : DataSize);
	return true;
}

bool Clipboard::GetHDROPAsText(string& data) const
{
	const auto hClipData = GetData(CF_HDROP);
	if (!hClipData)
		return false;

	const auto Files = os::memory::global::lock<const DROPFILES*>(hClipData);
	if (!Files)
		return false;

	const auto StartA=reinterpret_cast<const char*>(Files.get()) + Files->pFiles;

	if (Files->fWide)
	{
		const auto Start = reinterpret_cast<const wchar_t*>(StartA);
		for (const auto& i: enum_substrings(Start))
		{
			data.append(i.data(), i.size()).append(L"\r\n");
		}
	}
	else
	{
		for (const auto& i: enum_substrings(StartA))
		{
			append(data, encoding::ansi::get_chars(i.data(), i.size()), L"\r\n"s);
		}
	}

	return true;
}

bool Clipboard::GetVText(string& data) const
{
	const auto& IsBorlandVerticalBlock = [this]
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
		return GetText(data);
	}

	const auto hClipData = GetData(RegisterFormat(clipboard_format::vertical_block_oem));
	if (!hClipData)
		return false;

	const auto OemData = os::memory::global::lock<const char*>(hClipData);
	if (!OemData)
		return false;

	data = encoding::oem::get_chars(OemData.get());
	return true;
}

//-----------------------------------------------------------------------------
bool SetClipboardText(const wchar_t* Data, size_t Size)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->SetText(Data, Size);
}

bool SetClipboardVText(const wchar_t *Data, size_t Size)
{
	clipboard_accessor Clip;
	return Clip->Open() && Clip->SetVText(Data, Size);
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

