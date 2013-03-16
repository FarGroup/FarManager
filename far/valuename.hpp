#pragma once

/*
valuename.hpp

*/
/*
Copyright © 2012 Far Group
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

/*
container: some std container (array, vector, etc) of name_value_pair
*/
template<class container>
typename container::value_type::pair_name_type GetNameOfValue(typename container::value_type::pair_value_type Value, container& From)
{
	auto Item = std::find_if(CONST_RANGE(From, i)
	{
		return i.Value == Value;
	});
	return Item == From.cend()? typename container::value_type::pair_name_type() : Item->Name;
}

template<class container>
typename container::value_type::pair_value_type GetValueOfVame(typename container::value_type::pair_name_type Name, container& From)
{
	auto Item = std::find_if(CONST_RANGE(From, i)
	{
		return !StrCmpI(i.Name, Name);
	});
	return Item == From.cend()? typename container::value_type::pair_value_type() : Item->Value;
}

template<class container>
const string FlagsToString(typename container::value_type::pair_value_type Flags, const container& From, wchar_t Separator = L' ')
{
	string strFlags;
	std::for_each(CONST_RANGE(From, i)
	{
		if (Flags & i.Value)
		{
			strFlags.Append(i.Name).Append(Separator);
		}
	});

	if(!strFlags.IsEmpty())
	{
		strFlags.SetLength(strFlags.GetLength() - 1);
	}

	return strFlags;
}

template<class container>
typename container::value_type::pair_value_type StringToFlags(const string& strFlags, const container& From, const wchar_t* Separators = L"|;, ")
{
	auto Flags = typename container::value_type::pair_value_type();
	if(!strFlags.IsEmpty())
	{
		auto FlagList(StringToList(strFlags, STLF_UNIQUE, Separators));
		std::for_each(CONST_RANGE(FlagList, i)
		{
			Flags |= GetValueOfVame(i, From);
		});
	}
	return Flags;
}
