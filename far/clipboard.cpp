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
#include "strmix.hpp"

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
ENUM(FAR_CLIPBOARD_FORMAT)
{
	FCF_VERTICALBLOCK_OEM,
	FCF_VERTICALBLOCK_UNICODE,
	FCF_CFSTR_PREFERREDDROPEFFECT,
	FCF_MSDEVCOLUMNSELECT,
	FCF_BORLANDIDEVBLOCK,
	FCF_NOTEPADPLUSPLUS_BINARYTEXTLENGTH,

	FCF_COUNT
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
		Close();
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
	system_clipboard() {}

	virtual HANDLE GetData(UINT uFormat) const override
	{
		assert(m_Opened);

		return GetClipboardData(uFormat);
	}

	virtual bool SetData(UINT uFormat, os::memory::global::ptr&& hMem) override
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

	virtual UINT RegisterFormat(FAR_CLIPBOARD_FORMAT Format) const override
	{
		static const wchar_t* FormatNames[] =
		{
			L"FAR_VerticalBlock",
			L"FAR_VerticalBlock_Unicode",
			CFSTR_PREFERREDDROPEFFECT,
			L"MSDEVColumnSelect",
			L"Borland IDE Block Type",
			L"Notepad++ binary text length",
		};

		static_assert(ARRAYSIZE(FormatNames) == FCF_COUNT, "Wrong size of FormatNames");
		assert(Format < FCF_COUNT);
		return RegisterClipboardFormat(FormatNames[Format]);
	}

	virtual bool IsFormatAvailable(UINT Format) const override
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
		Close();
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
	internal_clipboard() {}

	virtual HANDLE GetData(UINT uFormat) const override
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

	virtual bool SetData(UINT uFormat, os::memory::global::ptr&& hMem) override
	{
		assert(m_Opened);

		if (m_Opened)
		{
			m_InternalData[uFormat] = std::move(hMem);
			return true;
		}
		return false;
	}

	virtual UINT RegisterFormat(FAR_CLIPBOARD_FORMAT Format) const override
	{
		return Format + 0xFC; // magic number stands for "Far Clipboard"
	}

	virtual bool IsFormatAvailable(UINT Format) const override
	{
		return Format && m_InternalData.count(Format);
	}

	std::unordered_map<UINT, os::memory::global::ptr> m_InternalData;
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
				SetData(RegisterFormat(FCF_NOTEPADPLUSPLUS_BINARYTEXTLENGTH), os::memory::global::copy(binary_length));
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
		const auto FarVerticalBlock = RegisterFormat(FCF_VERTICALBLOCK_UNICODE);

		if (!FarVerticalBlock)
			return false;

		Result = Result && SetData(FarVerticalBlock, nullptr);

		// 'Borland IDE Block Type'
		char Cx02 = '\x02';
		SetData(RegisterFormat(FCF_BORLANDIDEVBLOCK), os::memory::global::copy(Cx02));
		// 'MSDEVColumnSelect'
		SetData(RegisterFormat(FCF_MSDEVCOLUMNSELECT), nullptr);
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
							if(SetData(RegisterFormat(FCF_CFSTR_PREFERREDDROPEFFECT), std::move(hMemoryMove)))
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

bool Clipboard::GetText(string& data) const
{
	bool Result = false;

	if (auto hClipData = GetData(CF_UNICODETEXT))
	{
		if (const auto ClipAddr = os::memory::global::lock<const wchar_t*>(hClipData))
		{
			Result = true;
			data = ClipAddr.get();
			size_t len = string::npos;
			if (auto hClipDataLen = GetData(RegisterFormat(FCF_NOTEPADPLUSPLUS_BINARYTEXTLENGTH)))
			{
				if (const auto ClipLength = os::memory::global::lock<const uint32_t*>(hClipDataLen))
					len = static_cast<size_t>(ClipLength.get()[0]);
			}
			if (len == string::npos)
				data = ClipAddr.get();
			else
				data.assign(ClipAddr.get(), len);
		}
	}
	else
	{
		Result = GetHDROPAsText(data);
	}

	return Result;
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
				FOR(const auto& i, os::enum_strings(Start))
				{
					data.append(i.data(), i.size()).append(L"\r\n");
				}
			}
			else
			{
				FOR(const auto& i, (os::enum_strings_t<const char*, const char*>(StartA)))
				{
					data.append(wide(std::string(i.data(), i.size()), CP_ACP)).append(L"\r\n");
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

	bool ColumnSelect = IsFormatAvailable(RegisterFormat(FCF_VERTICALBLOCK_UNICODE));

	if (!ColumnSelect)
	{
		ColumnSelect = IsFormatAvailable(RegisterFormat(FCF_MSDEVCOLUMNSELECT));
	}

	if (!ColumnSelect)
	{
		if (const auto hClipData = GetData(RegisterFormat(FCF_BORLANDIDEVBLOCK)))
			if (const auto ClipAddr = os::memory::global::lock<const char*>(hClipData))
				ColumnSelect = (*ClipAddr & 0x02) != 0;
	}

	if (ColumnSelect)
	{
		Result = GetText(data);
	}
	else
	{
		const auto Far1xVerticalBlock = RegisterFormat(FCF_VERTICALBLOCK_OEM);
		if (const auto hClipData = GetData(Far1xVerticalBlock))
		{
			if (const auto OemData = os::memory::global::lock<const char*>(hClipData))
			{
				data = wide(OemData.get());
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

