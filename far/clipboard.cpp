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

default_clipboard_mode::mode default_clipboard_mode::m_Mode = default_clipboard_mode::system;

void default_clipboard_mode::set(default_clipboard_mode::mode Mode)
{
	m_Mode = Mode;
}

default_clipboard_mode::mode default_clipboard_mode::get()
{
	return m_Mode;
}

//-----------------------------------------------------------------------------
enum class Clipboard::clipboard_format: unsigned
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

	virtual ~system_clipboard()
	{
		system_clipboard::Close();
	}

	virtual bool Open() override
	{
		assert(!m_Opened);

		if (!m_Opened)
		{
			if (OpenClipboard(Console().GetWindow()))
			{
				m_Opened = true;
				return true;
			}
		}
		return false;
	}

	virtual bool Close() override
	{
		// Closing already closed buffer is OK
		if (m_Opened)
		{
			if (!CloseClipboard())
				return false;
			m_Opened = false;
		}
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

		if (SetClipboardData(uFormat, hMem.get()))
		{
			hMem.release();

			if (auto Locale = os::memory::global::copy<LCID>(LOCALE_USER_DEFAULT))
			{
				if (SetClipboardData(CF_LOCALE, Locale.get()))
				{
					Locale.release();
				}
				return true;
			}
		}
		return false;
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

		static_assert(std::size(FormatNames) == static_cast<unsigned>(clipboard_format::count), "Wrong size of FormatNames");
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

	virtual ~internal_clipboard()
	{
		internal_clipboard::Close();
	}

	virtual bool Open() override
	{
		assert(!m_Opened);

		if (!m_Opened)
		{
			m_Opened = true;
			return true;
		}
		return false;
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

		if (m_Opened)
		{
			m_InternalData.clear();
			return true;
		}
		return false;
	}

private:
	internal_clipboard() = default;

	virtual HANDLE GetData(unsigned uFormat) const override
	{
		assert(m_Opened);

		if (m_Opened && uFormat)
		{
			const auto ItemIterator = m_InternalData.find(uFormat);
			if (ItemIterator != m_InternalData.cend())
				return ItemIterator->second.get();
		}
		return nullptr;
	}

	virtual bool SetData(unsigned uFormat, os::memory::global::ptr&& hMem) override
	{
		assert(m_Opened);

		if (m_Opened)
		{
			m_InternalData[uFormat] = std::move(hMem);
			return true;
		}
		return false;
	}

	virtual unsigned RegisterFormat(clipboard_format Format) const override
	{
		return static_cast<unsigned>(Format) + 0xFC; // magic number stands for "Far Clipboard"
	}

	virtual bool IsFormatAvailable(unsigned Format) const override
	{
		return Format && m_InternalData.count(Format);
	}

	std::unordered_map<unsigned, os::memory::global::ptr> m_InternalData;
};

//-----------------------------------------------------------------------------
Clipboard& Clipboard::GetInstance(default_clipboard_mode::mode Mode)
{
	return Mode == default_clipboard_mode::system? system_clipboard::GetInstance() : internal_clipboard::GetInstance();
}

bool Clipboard::SetText(const wchar_t *Data, size_t Size)
{
	auto Result = Clear();
	if (Data)
	{
		if (auto hData = os::memory::global::copy(Data, Size))
		{
			Result = SetData(CF_UNICODETEXT, std::move(hData));
			if (Result)
			{
				// 'Notepad++ binary text length'
				auto binary_length = static_cast<uint32_t>(Size);
				SetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length), os::memory::global::copy(binary_length));
			}
		}
		else
		{
			Result = false;
		}
	}
	return Result;
}

bool Clipboard::SetVText(const wchar_t *Data, size_t Size)
{
	auto Result = SetText(Data, Size);

	if (Result && Data)
	{
		const auto FarVerticalBlock = RegisterFormat(clipboard_format::vertical_block_unicode);

		if (!FarVerticalBlock)
			return false;

		Result = Result && SetData(FarVerticalBlock, nullptr);

		// 'Borland IDE Block Type'
		char Cx02 = '\x02';
		SetData(RegisterFormat(clipboard_format::borland_ide_dev_block), os::memory::global::copy(Cx02));
		// 'MSDEVColumnSelect'
		SetData(RegisterFormat(clipboard_format::ms_dev_column_select), nullptr);
	}
	return Result;
}

bool Clipboard::SetHDROP(const string& NamesData, bool bMoved)
{
	bool Result=false;
	if (!NamesData.empty())
	{
		const auto RawDataSize = (NamesData.size() + 1) * sizeof(wchar_t);
		if (auto hMemory = os::memory::global::alloc(GMEM_MOVEABLE, sizeof(DROPFILES) + RawDataSize))
		{
			if (const auto Drop = os::memory::global::lock<LPDROPFILES>(hMemory))
			{
				Drop->pFiles=sizeof(DROPFILES);
				Drop->pt.x=0;
				Drop->pt.y=0;
				Drop->fNC = TRUE;
				Drop->fWide = TRUE;
				memcpy(Drop.get() + 1, NamesData.data(), RawDataSize);
				Clear();
				if(SetData(CF_HDROP, std::move(hMemory)))
				{
					if(bMoved)
					{
						if (auto hMemoryMove = os::memory::global::copy<DWORD>(DROPEFFECT_MOVE))
						{
							if(SetData(RegisterFormat(clipboard_format::preferred_drop_effect), std::move(hMemoryMove)))
							{
								Result = true;
							}
						}
					}
					else
						Result = true;
				}
			}
		}
	}
	return Result;
}

bool Clipboard::GetText(string& Data) const
{
	if (auto hClipData = GetData(CF_UNICODETEXT))
	{
		if (const auto ClipAddr = os::memory::global::lock<const wchar_t*>(hClipData))
		{
			size_t DataSize = string::npos;
			if (auto hClipDataLen = GetData(RegisterFormat(clipboard_format::notepad_plusplus_binary_text_length)))
			{
				if (const auto ClipLength = os::memory::global::lock<const uint32_t*>(hClipDataLen))
				{
					DataSize = static_cast<size_t>(*ClipLength.get());
				}
			}
			Data.assign(ClipAddr.get(), DataSize == string::npos? wcslen(ClipAddr.get()) : DataSize);
			return true;
		}
	}
	else
	{
		return GetHDROPAsText(Data);
	}

	return false;
}

bool Clipboard::GetHDROPAsText(string& data) const
{
	bool Result = false;

	if (const auto hClipData = GetData(CF_HDROP))
	{
		if (const auto Files = os::memory::global::lock<const DROPFILES*>(hClipData))
		{
			const auto StartA=reinterpret_cast<const char*>(Files.get())+Files->pFiles;
			const auto Start = reinterpret_cast<const wchar_t*>(StartA);
			if(Files->fWide)
			{
				for (const auto& i: enum_substrings(Start))
				{
					data.append(i.data(), i.size()).append(L"\r\n");
				}
			}
			else
			{
				for (const auto& i: enum_substrings(StartA))
				{
					data.append(encoding::ansi::get_chars(i)).append(L"\r\n");
				}
			}
			Result = true;
		}
	}
	return Result;
}

bool Clipboard::GetVText(string& data) const
{
	bool Result = false;

	bool ColumnSelect = IsFormatAvailable(RegisterFormat(clipboard_format::vertical_block_unicode));

	if (!ColumnSelect)
	{
		ColumnSelect = IsFormatAvailable(RegisterFormat(clipboard_format::ms_dev_column_select));
	}

	if (!ColumnSelect)
	{
		if (const auto hClipData = GetData(RegisterFormat(clipboard_format::borland_ide_dev_block)))
			if (const auto ClipAddr = os::memory::global::lock<const char*>(hClipData))
				ColumnSelect = *ClipAddr == 0x02;
	}

	if (ColumnSelect)
	{
		Result = GetText(data);
	}
	else
	{
		const auto Far1xVerticalBlock = RegisterFormat(clipboard_format::vertical_block_oem);
		if (const auto hClipData = GetData(Far1xVerticalBlock))
		{
			if (const auto OemData = os::memory::global::lock<const char*>(hClipData))
			{
				data = encoding::oem::get_chars(OemData.get());
				Result = true;
			}
		}
	}

	return Result;
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
	clipboard_accessor Clip(default_clipboard_mode::internal);
	return Clip->Open() && Clip->Clear();
}

bool CopyData(const clipboard_accessor& From, clipboard_accessor& To)
{
	string Data;
	if (From->GetVText(Data))
	{
		return To->SetVText(Data);
	}
	else if (From->GetText(Data))
	{
		return To->SetText(Data);
	}
	return false;
}

