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

void BaseFormat::Reset()
{
	_Width=0;
	_Precision=static_cast<size_t>(-1);
	_FillChar=L' ';
	_Align=fmt::A_RIGHT;
}

void BaseFormat::Put(LPCWSTR Data,size_t Length)
{
	if (_Precision==static_cast<size_t>(-1))
	{
		_Precision=Length;
	}

	string OutStr(Data,Min(_Precision,Length));

	if (_Align==fmt::A_RIGHT)
	{
		while (OutStr.GetLength()<_Width)
		{
			OutStr.Insert(0,_FillChar);
		}
	}
	else
	{
		while (OutStr.GetLength()<_Width)
		{
			OutStr.Append(_FillChar);
		}
	}

	Commit(OutStr);
	Reset();
}

BaseFormat& BaseFormat::operator<<(WCHAR Value)
{
	Put(&Value,1);
	return *this;
}

BaseFormat& BaseFormat::operator<<(INT64 Value)
{
	WCHAR Buffer[32];
	_i64tow(Value,Buffer,10);
	Put(Buffer,StrLength(Buffer));
	return *this;
}

BaseFormat& BaseFormat::operator<<(UINT64 Value)
{
	WCHAR Buffer[32];
	_ui64tow(Value,Buffer,10);
	Put(Buffer,StrLength(Buffer));
	return *this;
}

BaseFormat& BaseFormat::operator<<(LPCWSTR Data)
{
	Data=NullToEmpty(Data);
	Put(Data,StrLength(Data));
	return *this;
}

BaseFormat& BaseFormat::operator<<(string& String)
{
	Put(String,String.GetLength());
	return *this;
}

BaseFormat& BaseFormat::operator<<(const fmt::Width& Manipulator)
{
	SetWidth(Manipulator.GetValue());
	return *this;
}

BaseFormat& BaseFormat::operator<<(const fmt::Precision& Manipulator)
{
	SetPrecision(Manipulator.GetValue());
	return *this;
}


BaseFormat& BaseFormat::operator<<(const fmt::FillChar& Manipulator)
{
	SetFillChar(Manipulator.GetValue());
	return *this;
}

BaseFormat& BaseFormat::operator<<(const fmt::LeftAlign& Manipulator)
{
	SetAlign(fmt::A_LEFT);
	return *this;
}

BaseFormat& BaseFormat::operator<<(const fmt::RightAlign& Manipulator)
{
	SetAlign(fmt::A_RIGHT);
	return *this;
}

void FormatScreen::Commit(const string& Data)
{
	Text(Data);
}

