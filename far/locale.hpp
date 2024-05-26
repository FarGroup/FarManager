#ifndef LOCALE_HPP_C358EF79_F894_425E_B689_C4F4131DBE76
#define LOCALE_HPP_C358EF79_F894_425E_B689_C4F4131DBE76
#pragma once

/*
locale.hpp

*/
/*
Copyright © 2014 Far Group
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

// Platform:

// Common:
#include "common/nifty_counter.hpp"

// External:

//----------------------------------------------------------------------------

struct locale_names
{
	struct name
	{
		string Full;
		string Short;
	};

	name Months[12];
	name Weekdays[7];
};

enum class date_type
{
	mdy,
	dmy,
	ymd
};

namespace detail
{
	class locale
	{
	public:
		explicit locale(bool Invariant = false);

		bool is_invariant() const;
		bool is_cjk() const;
		date_type date_format() const;
		unsigned digits_grouping() const;
		wchar_t date_separator() const;
		wchar_t time_separator() const;
		wchar_t decimal_separator() const;
		wchar_t thousand_separator() const;
		const locale_names& LocalNames() const;
		const locale_names& EnglishNames() const;
		const locale_names& Names(bool Local) const;
		void invalidate();

	private:
		void refresh() const;

		bool m_IsInvariant;
		mutable bool m_IsCJK{};
		mutable date_type m_DateFormat{ date_type::mdy };
		mutable unsigned m_DigitsGrouping{};
		mutable wchar_t m_DateSeparator{};
		mutable wchar_t m_TimeSeparator{};
		mutable wchar_t m_DecimalSeparator{};
		mutable wchar_t m_ThousandSeparator{};
		mutable locale_names m_LocalNames;
		mutable locale_names m_EnglishNames;

		mutable bool m_Valid{};
	};
}

NIFTY_DECLARE(detail::locale, locale);

detail::locale const& invariant_locale();

#endif // LOCALE_HPP_C358EF79_F894_425E_B689_C4F4131DBE76
