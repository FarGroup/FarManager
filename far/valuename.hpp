#pragma once

/*
valuename.hpp

*/
/*
Copyright © 2013 Far Group
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

template<class value, class name>
struct value_name_pair
{
	value Value;
	name Name;

	typedef value pair_value_type;
	typedef name pair_name_type;
};


// container: some std container (array, vector, etc) or array of name_value_pair

template<class container, class value>
auto GetNameOfValue(const value& Value, const container& From) -> decltype(std::begin(From)->Name)
{
	static_assert(std::is_same<value, decltype(std::begin(From)->Value)>::value, "Wrong type of 'Value' parameter");
	auto ItemIterator = std::find_if(CONST_RANGE(From, i)
	{
		return i.Value == Value;
	});

	// VC10 workaround. TODO: remove EmptyName and use normal decltype after migrating to compiler with full C++11 support.
	auto EmptyName = typename DECLTYPE(ItemIterator->Name)();
	return ItemIterator == std::cend(From)? EmptyName : ItemIterator->Name;
}

template<class container, class name>
auto GetValueOfName(const name& Name, const container& From) -> decltype(std::begin(From)->Value)
{
	static_assert(std::is_same<name, decltype(std::begin(From)->Name)>::value, "Wrong type of 'Name' parameter");
	auto ItemIterator = std::find_if(CONST_RANGE(From, i)
	{
		return !StrCmpI(i.Name, Name);
	});
	// VC10 workaround. TODO: remove EmptyValue and use normal decltype after migrating to compiler with full C++11 support.
	auto EmptyValue = typename DECLTYPE(ItemIterator->Value)();
	return ItemIterator == std::cend(From)? EmptyValue : ItemIterator->Value;
}

template<class container, class value>
const string FlagsToString(const value& Flags, const container& From, wchar_t Separator = L' ')
{
	static_assert(std::is_same<value, decltype(std::begin(From)->Value)>::value, "Wrong type of 'Flags' parameter");
	string strFlags;
	std::for_each(CONST_RANGE(From, i)
	{
		if (Flags & i.Value)
		{
			strFlags.append(i.Name).append(1, Separator);
		}
	});

	if(!strFlags.empty())
	{
		strFlags.pop_back();
	}

	return strFlags;
}

template<class container>
auto StringToFlags(const string& strFlags, const container& From, const wchar_t* Separators = L"|;, ") -> decltype(std::begin(From)->Value)
{
	// VC10 workaround. TODO: use normal decltype after migrating to compiler with full C++11 support.
	auto Flags = typename DECLTYPE(std::begin(From)->Value)();
	if(!strFlags.empty())
	{
		auto FlagList(StringToList(strFlags, STLF_UNIQUE, Separators));
		std::for_each(CONST_RANGE(FlagList, i)
		{
			Flags |= GetValueOfName(i.data(), From);
		});
	}
	return Flags;
}
