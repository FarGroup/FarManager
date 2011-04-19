#pragma once

/*
format.hpp

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
namespace fmt
{
	class Width
	{
			size_t Value;
		public:
			Width(size_t Value=0) {this->Value=Value;}
			size_t GetValue()const {return Value;}
	};

	class Precision
	{
			size_t Value;
		public:
			Precision(size_t Value=static_cast<size_t>(-1)) {this->Value=Value;}
			size_t GetValue()const {return Value;}
	};

	class FillChar
	{
			WCHAR Value;
		public:
			FillChar(WCHAR Value=L' ') {this->Value=Value;}
			WCHAR GetValue()const {return Value;}
	};

	class Radix
	{
			int Value;
		public:
			Radix(int Value=10) {this->Value=Value;}
			int GetValue()const {return Value;}
	};

	enum AlignType
	{
		A_LEFT,
		A_RIGHT,
	};

	template<AlignType T>class Align {};

	typedef Align<A_LEFT> LeftAlign;
	typedef Align<A_RIGHT> RightAlign;
};

class BaseFormat
{
		size_t _Width;
		size_t _Precision;
		WCHAR _FillChar;
		fmt::AlignType _Align;
		int _Radix;

		void Reset();
		void Put(LPCWSTR Data,size_t Length);
		BaseFormat& ToString(INT64 Value, bool Signed);

	protected:
		virtual void Commit(const string& Data)=0;

	public:
		BaseFormat() {Reset();}
		virtual ~BaseFormat() {}

		// attributes
		void SetPrecision(size_t Precision=static_cast<size_t>(-1)) {_Precision=Precision;}
		void SetWidth(size_t Width=0) {_Width=Width;}
		void SetAlign(fmt::AlignType Align=fmt::A_RIGHT) {_Align=Align;}
		void SetFillChar(WCHAR Char=L' ') {_FillChar=Char;}
		void SetRadix(int Radix=10) {_Radix=Radix;}

		// data
		BaseFormat& operator<<(INT64 Value);
		BaseFormat& operator<<(UINT64 Value);

		BaseFormat& operator<<(short Value) {return operator<<(static_cast<INT64>(Value));}
		BaseFormat& operator<<(USHORT Value) {return operator<<(static_cast<UINT64>(Value));}

		BaseFormat& operator<<(int Value) {return operator<<(static_cast<INT64>(Value));}
		BaseFormat& operator<<(UINT Value) {return operator<<(static_cast<UINT64>(Value));}

		BaseFormat& operator<<(long Value) {return operator<<(static_cast<INT64>(Value));}
		BaseFormat& operator<<(ULONG Value) {return operator<<(static_cast<UINT64>(Value));}

		BaseFormat& operator<<(WCHAR Value);
		BaseFormat& operator<<(LPCWSTR Data);
		BaseFormat& operator<<(const string& String);

		// manipulators
		BaseFormat& operator<<(const fmt::Width& Manipulator);
		BaseFormat& operator<<(const fmt::Precision& Manipulator);
		BaseFormat& operator<<(const fmt::LeftAlign& Manipulator);
		BaseFormat& operator<<(const fmt::RightAlign& Manipulator);
		BaseFormat& operator<<(const fmt::FillChar& Manipulator);
		BaseFormat& operator<<(const fmt::Radix& Manipulator);
};

class FormatString:public BaseFormat, public string
{
	virtual void Commit(const string& Data);
};

class FormatScreen:public BaseFormat
{
	virtual void Commit(const string& Data);
};
