#ifndef CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
#define CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
#pragma once

/*
clipboard.hpp

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

// Internal:
#include "platform.memory.hpp"

// Platform:

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

enum class clipboard_mode
{
	system,
	internal
};

class default_clipboard_mode
{
public:
	static void set(clipboard_mode Mode) noexcept;
	static clipboard_mode get() noexcept;

private:
	static inline clipboard_mode m_Mode = clipboard_mode::system;
};

class clipboard
{
public:
	static clipboard& GetInstance(clipboard_mode Mode);
	virtual ~clipboard() = default;

	virtual bool Open() = 0;
	virtual bool Close() noexcept = 0;
	virtual bool Clear() = 0;

	bool SetText(string_view Str);
	bool SetVText(string_view Str);
	bool SetHDROP(string_view NamesData, bool bMoved);

	bool GetText(string& Data) const;
	bool GetVText(string& Data) const;

protected:
	enum class clipboard_format;

	bool m_Opened {};

private:
	virtual bool SetData(unsigned uFormat, os::memory::global::ptr&& hMem) = 0;
	virtual HANDLE GetData(unsigned uFormat) const = 0;
	virtual unsigned RegisterFormat(clipboard_format Format) const = 0;
	virtual bool IsFormatAvailable(unsigned Format) const = 0;

	bool GetHDROPAsText(string& data) const;
};

class clipboard_accessor:noncopyable
{
public:
	explicit clipboard_accessor(clipboard_mode Mode = default_clipboard_mode::get()): m_Clipboard(clipboard::GetInstance(Mode)) {}
	~clipboard_accessor() { m_Clipboard.Close(); }
	auto operator->() const noexcept { return &m_Clipboard; }

private:
	clipboard& m_Clipboard;
};


bool SetClipboardText(string_view Str);
bool SetClipboardVText(string_view Str);

bool GetClipboardText(string& data);
bool GetClipboardVText(string& data);

bool ClearClipboard();

bool ClearInternalClipboard();

bool CopyData(const clipboard_accessor& From, const clipboard_accessor& To);

struct clipboard_restorer
{
	void operator()(const clipboard* Clip) const noexcept;
};

std::unique_ptr<clipboard, clipboard_restorer> OverrideClipboard();

#endif // CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
