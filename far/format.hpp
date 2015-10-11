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
	template<typename T, T Default> class ManipulatorTemplate
	{
	public:
		ManipulatorTemplate(T Value=DefaultValue):Value(Value) {}
		T GetValue() const { return Value; }
		static T GetDefault() { return DefaultValue; }
	private:
		const T Value;
		static const T DefaultValue = Default;
	};

	typedef ManipulatorTemplate<size_t, 0> MinWidth;
	typedef ManipulatorTemplate<size_t, static_cast<size_t>(-1)> MaxWidth;
	typedef ManipulatorTemplate<size_t, 1 /*any*/> ExactWidth;
	typedef ManipulatorTemplate<wchar_t, L' '> FillChar;
	typedef ManipulatorTemplate<int, 10> Radix;

	enum AlignType
	{
		A_LEFT,
		A_RIGHT,
	};
	typedef ManipulatorTemplate<AlignType, A_RIGHT> Align;

	template<AlignType T>class SimpleAlign {};
	typedef SimpleAlign<A_LEFT> LeftAlign;
	typedef SimpleAlign<A_RIGHT> RightAlign;

	class Flush {};
};

class BaseFormat
{
protected:
	BaseFormat();
	virtual ~BaseFormat() {}

	virtual BaseFormat& Flush() { return *this; }

	// attributes
	BaseFormat& SetMaxWidth(size_t Precision);
	BaseFormat& SetMinWidth(size_t Width);
	BaseFormat& SetExactWidth(size_t ExactWidth);
	BaseFormat& SetAlign(fmt::AlignType Align);
	BaseFormat& SetFillChar(wchar_t Char);
	BaseFormat& SetRadix(int Radix);

	BaseFormat& Put(LPCWSTR Data, size_t Length);

	// data
	BaseFormat& operator<<(INT64 Value);
	BaseFormat& operator<<(UINT64 Value);
	BaseFormat& operator<<(short Value);
	BaseFormat& operator<<(USHORT Value);
	BaseFormat& operator<<(int Value);
	BaseFormat& operator<<(UINT Value);
	BaseFormat& operator<<(long Value);
	BaseFormat& operator<<(ULONG Value);
	BaseFormat& operator<<(wchar_t Value);
	BaseFormat& operator<<(LPCWSTR Data);
	BaseFormat& operator<<(const string& String);

	// manipulators
	BaseFormat& operator<<(const fmt::MinWidth& Manipulator);
	BaseFormat& operator<<(const fmt::MaxWidth& Manipulator);
	BaseFormat& operator<<(const fmt::ExactWidth& Manipulator);
	BaseFormat& operator<<(const fmt::FillChar& Manipulator);
	BaseFormat& operator<<(const fmt::Radix& Manipulator);
	BaseFormat& operator<<(const fmt::Align& Manipulator);
	BaseFormat& operator<<(const fmt::LeftAlign& Manipulator);
	BaseFormat& operator<<(const fmt::RightAlign& Manipulator);
	BaseFormat& operator<<(const fmt::Flush& Manipulator);

	virtual void Commit(const string& Data)=0;

private:
	template<class T>
	BaseFormat& ToString(T Value);
	void Reset();

	size_t m_MinWidth;
	size_t m_MaxWidth;
	wchar_t m_FillChar;
	fmt::AlignType m_Align;
	int m_Radix;
};

class FormatString:public BaseFormat, public string
{
public:
	template<class T>
	FormatString& operator<<(const T& param) {return static_cast<FormatString&>(BaseFormat::operator<<(param));}

private:
	virtual void Commit(const string& Data) override;
};

class FormatScreen: noncopyable, public BaseFormat
{
public:
	template<class T>
	FormatScreen& operator<<(const T& param) {return static_cast<FormatScreen&>(BaseFormat::operator<<(param));}

private:
	virtual void Commit(const string& Data) override;
};

ENUM(LNGID);

namespace detail {
// TODO: rename
// TODO: %n -> {n}
class LangString:public BaseFormat, public string
{
public:
	LangString():Iteration(0) {};
	LangString(LNGID MessageId);
	LangString(const string& str);
	LangString(string&& str);
	template<class T>
	LangString& operator<<(const T& param) {return static_cast<LangString&>(BaseFormat::operator<<(param));}

private:
	virtual void Commit(const string& Data) override;
	size_t Iteration;
};
}

namespace detail
{
	template<class C, class Arg>
	void string_format_impl(C& Container, Arg&& arg)
	{
		Container << arg;
	}
}

#if defined _MSC_VER && _MSC_VER < 1800
#define STRING_FORMAT_IMPL_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
namespace detail \
{ \
	template<class C, VTE_TYPENAME(first), TYPENAME_LIST> \
	void string_format_impl(C& Container, VTE_REF_ARG(first), REF_ARG_LIST) \
	{ \
		Container << VTE_FWD_ARG(first); \
		string_format_impl(Container, FWD_ARG_LIST); \
	} \
}

#define STRING_FORMAT_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
template<class T, TYPENAME_LIST> \
string string_format(const T& Format, REF_ARG_LIST) \
{ \
	detail::LangString Container(Format); \
	detail::string_format_impl(Container, FWD_ARG_LIST); \
	return Container; \
} \

#include "common/variadic_emulation_helpers_begin.hpp"
VTE_GENERATE(STRING_FORMAT_IMPL_VTE)
VTE_GENERATE(STRING_FORMAT_VTE)
#include "common/variadic_emulation_helpers_end.hpp"

#undef STRING_FORMAT_VTE
#undef STRING_FORMAT_IMPL_VTE
#else
namespace detail
{
	template<class C, class Arg1, class... Args>
	void string_format_impl(C& Container, Arg1&& arg1, Args&&... args)
	{
		Container << arg1;
		string_format_impl(Container, std::forward<Args>(args)...);
	}
}

template<class T, class Arg1, class... Args>
string string_format(const T& Format, Arg1&& arg1, Args&&... args)
{
	detail::LangString Container(Format);
	detail::string_format_impl(Container, Format, std::forward<Args>(args)...);
	// slicing is ok
	return Container;
}
#endif
