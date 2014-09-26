/*
format.cpp

Форматирование строк
*/
/*
Copyright © 2009 Far Group
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

#include "format.hpp"
#include "interf.hpp"
#include "language.hpp"

BaseFormat::BaseFormat()
{
	Reset();
}

BaseFormat& BaseFormat::SetMaxWidth(size_t Precision)
{
	m_MaxWidth = Precision;
	return *this;
}

BaseFormat& BaseFormat::SetMinWidth(size_t Width)
{
	m_MinWidth = Width;
	return *this;
}

BaseFormat& BaseFormat::SetExactWidth(size_t SetExactWidth)
{
	m_MinWidth = SetExactWidth;
	m_MaxWidth = SetExactWidth;
	return *this;
}

BaseFormat& BaseFormat::SetAlign(fmt::AlignType Align)
{
	m_Align = Align;
	return *this;
}
BaseFormat& BaseFormat::SetFillChar(wchar_t Char)
{
	m_FillChar = Char;
	return *this;
}

BaseFormat& BaseFormat::SetRadix(int Radix)
{
	m_Radix=Radix;
	return *this;
}

BaseFormat& BaseFormat::Put(LPCWSTR Data, size_t Length)
{
	if (m_MaxWidth == fmt::MaxWidth::GetDefault())
	{
		m_MaxWidth = Length;
	}

	string OutStr(Data, std::min(m_MaxWidth, Length));

	if (OutStr.size() < m_MinWidth)
	{
		if (m_Align == fmt::A_RIGHT)
		{
			OutStr.insert(0, m_MinWidth - OutStr.size(), m_FillChar);
		}
		else
		{
			OutStr.append(m_MinWidth - OutStr.size(), m_FillChar);
		}
	}

	Commit(OutStr);
	Reset();
	return *this;
}

BaseFormat& BaseFormat::operator<<(INT64 Value)
{
	return ToString(Value);
}

BaseFormat& BaseFormat::operator<<(UINT64 Value)
{
	return ToString(Value);
}

BaseFormat& BaseFormat::operator<<(short Value)
{
	return operator<<(static_cast<INT64>(Value));
}

BaseFormat& BaseFormat::operator<<(USHORT Value)
{
	return operator<<(static_cast<UINT64>(Value));
}

BaseFormat& BaseFormat::operator<<(int Value)
{
	return operator<<(static_cast<INT64>(Value));
}

BaseFormat& BaseFormat::operator<<(UINT Value)
{
	return operator<<(static_cast<UINT64>(Value));
}

BaseFormat& BaseFormat::operator<<(long Value)
{
	return operator<<(static_cast<INT64>(Value));
}

BaseFormat& BaseFormat::operator<<(ULONG Value)
{
	return operator<<(static_cast<UINT64>(Value));
}

BaseFormat& BaseFormat::operator<<(wchar_t Value)
{
	return Put(&Value,1);
}

BaseFormat& BaseFormat::operator<<(LPCWSTR Data)
{
	Data = NullToEmpty(Data);
	return Put(Data, StrLength(Data));
}

BaseFormat& BaseFormat::operator<<(const string& String)
{
	return Put(String.data(), String.size());
}

BaseFormat& BaseFormat::operator<<(const fmt::MinWidth& Manipulator)
{
	return SetMinWidth(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::ExactWidth& Manipulator)
{
	return SetExactWidth(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::MaxWidth& Manipulator)
{
	return SetMaxWidth(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::FillChar& Manipulator)
{
	return SetFillChar(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::Radix& Manipulator)
{
	return SetRadix(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::Align& Manipulator)
{
	return SetAlign(Manipulator.GetValue());
}

BaseFormat& BaseFormat::operator<<(const fmt::LeftAlign& Manipulator)
{
	return SetAlign(fmt::A_LEFT);
}

BaseFormat& BaseFormat::operator<<(const fmt::RightAlign& Manipulator)
{
	return SetAlign(fmt::A_RIGHT);
}

BaseFormat& BaseFormat::operator<<(const fmt::Flush& Manipulator)
{
	return Flush();
}

void BaseFormat::Reset()
{
	m_MinWidth = fmt::MinWidth::GetDefault();
	m_MaxWidth = fmt::MaxWidth::GetDefault();
	m_FillChar = fmt::FillChar::GetDefault();
	m_Align = fmt::Align::GetDefault();
	m_Radix = fmt::Radix::GetDefault();
}

template<class T>
BaseFormat& BaseFormat::ToString(T Value)
{
	static_assert(std::is_integral<T>::value, "Value type is not integral");
	wchar_t Buffer[65];
	std::is_signed<T>::value? _i64tow(Value, Buffer, m_Radix) : _ui64tow(Value, Buffer, m_Radix);
	if (m_Radix > 10)
	{
		UpperBuf(Buffer, ARRAYSIZE(Buffer));
	}
	return Put(Buffer, StrLength(Buffer));
}




void FormatString::Commit(const string& Data)
{
	append(Data);
}

void FormatScreen::Commit(const string& Data)
{
	Text(Data);
}

LangString::LangString(enum LNGID MessageId):
	Iteration(0)
{
	assign(MSG(MessageId));
}

LangString::LangString(const string& str):
	Iteration(0)
{
	assign(str);
}

LangString::LangString(string&& str):
Iteration(0)
{
	assign(std::move(str));
}

void LangString::Commit(const string& Data)
{
	ReplaceStrings(*this, L"%" + std::to_wstring(++Iteration), Data);
}