/*
char_width.cpp

Fullwidth support
*/
/*
Copyright © 2021 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "char_width.hpp"

// Internal:
#include "console.hpp"
#include "locale.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/optional_bitset.hpp"

// External:

#ifdef ENABLE_TESTS
#include "testing.hpp"
#endif

//----------------------------------------------------------------------------

namespace
{
	auto s_FullWidthEnabled = false;

	enum class codepoint_width: signed char
	{
		ambiguous = -1,
		narrow    = 1,
		wide      = 2,
	};

	[[nodiscard]]
	auto quick_width(char_width::codepoint const Codepoint) noexcept
	{
		return in_closed_range(U'\U00000020', Codepoint, U'\U0000007E')?
			codepoint_width::narrow :
			codepoint_width::ambiguous;
	}

	[[nodiscard]]
	auto to_bits(codepoint_width const Width)
	{
		switch (Width)
		{
		case codepoint_width::ambiguous: return optional_bitset_state::unknown;
		case codepoint_width::wide:      return optional_bitset_state::yes;
		case codepoint_width::narrow:    return optional_bitset_state::no;
		default: std::unreachable();
		}
	}

	[[nodiscard]]
	auto from_bits(optional_bitset_state const State)
	{
		switch (State)
		{
		case optional_bitset_state::unknown: return codepoint_width::ambiguous;
		case optional_bitset_state::yes:     return codepoint_width::wide;
		case optional_bitset_state::no:      return codepoint_width::narrow;
		default: std::unreachable();
		}
	}

	[[nodiscard]]
	auto is_bmp(char_width::codepoint const Codepoint)
	{
		return Codepoint <= std::numeric_limits<char16_t>::max();
	}

	[[nodiscard]]
	auto device_width(char_width::codepoint const Codepoint, bool const ClearCacheOnly = false)
	{
		static optional_bitset<std::numeric_limits<char16_t>::max() + 1> FastCache;
		static std::unordered_map<char_width::codepoint, codepoint_width> SlowCache;

		if (ClearCacheOnly)
		{
			FastCache.reset();
			SlowCache.clear();
			console.ClearWideCache();
			return codepoint_width::ambiguous;
		}

		const auto IsBMP = is_bmp(Codepoint);

		if (IsBMP)
		{
			if (const auto Width = from_bits(FastCache.check(Codepoint)); Width != codepoint_width::ambiguous)
				return Width;
		}
		else
		{
			if (const auto Iterator = SlowCache.find(Codepoint); Iterator != SlowCache.cend())
				return Iterator->second;
		}

		const auto Result = static_cast<codepoint_width>(console.GetWidthPreciseExpensive(Codepoint));

		if (IsBMP)
			FastCache.assign(Codepoint, to_bits(Result));
		else
			SlowCache[Codepoint] = Result;

		return Result;
	}

	[[nodiscard]]
	auto is_fullwidth_supported()
	{
#ifdef ENABLE_TESTS
		if (is_test_run())
			return true;
#endif

		// FW works in VT (Win10+) and in classic grid mode with CJK locales (LVB)
		return console.IsVtSupported() || locale.is_cjk();
	}

	[[nodiscard]]
	auto get_width(char_width::codepoint const RawCodepoint)
	{
		const auto Codepoint = RawCodepoint > std::numeric_limits<char16_t>::max()?
			RawCodepoint :
			static_cast<char_width::codepoint>(ReplaceControlCharacter(static_cast<wchar_t>(RawCodepoint)));

		if (const auto Width = quick_width(Codepoint); Width != codepoint_width::ambiguous)
		{
			// We *assume* that ASCII characters are always narrow, but... trust no one.
			assert(Width == device_width(Codepoint));
			return Width;
		}

		if (const auto Width = device_width(Codepoint); Width != codepoint_width::ambiguous)
			return Width;

		return codepoint_width::narrow;
	}

	[[nodiscard]]
	bool is_legacy_rendering()
	{
#ifdef ENABLE_TESTS
		if (is_test_run())
			return false;
#endif

		return !console.IsVtActive();
	}
}

namespace char_width
{
	[[nodiscard]]
	size_t get(codepoint const Codepoint)
	{
		if (!is_bmp(Codepoint) && is_legacy_rendering())
			return 2; // Classic grid mode, nothing we can do :(

		if (!s_FullWidthEnabled || !is_fullwidth_supported())
			return 1;

		return static_cast<size_t>(get_width(Codepoint));
	}

	[[nodiscard]]
	bool is_wide(codepoint const Codepoint)
	{
		return get(Codepoint) > 1;
	}

	void enable(bool const Value)
	{
		s_FullWidthEnabled = Value;
	}

	[[nodiscard]]
	bool is_enabled()
	{
		return s_FullWidthEnabled;
	}

	void invalidate()
	{
		if (is_enabled())
			(void)device_width(0, true);
	}

	[[nodiscard]]
	bool is_half_width_surrogate_broken()
	{
		// As of 23 Jun 2022 conhost and WT render half-width surrogates as half-width,
		// but advance the cursor position as if they were full-width.
		// We can work around it by moving the cursor backwards each time.
		// They might fix it eventually, so it's better to detect it dynamically.

		// Mathematical Bold Fraktur Small A, U+1D586, half-width
		static const auto Result = console.GetWidthPreciseExpensive(U'𝖆');
		return Result > 1;
	}

	bool is_grapheme_clusters_on()
	{
		static const auto Result = []
		{
			if (const auto IsOn = console.is_grapheme_clusters_on(); IsOn)
			{
				LOGDEBUG(L"Grapheme clusters (VT): {}"sv, *IsOn);
				return *IsOn;
			}

			const auto IsOn = console.GetWidthPreciseExpensive(L"à"sv) == 1;
			LOGDEBUG(L"Grapheme clusters (heuristics): {}"sv, IsOn);
			return IsOn;
		}();

		return Result;
	}
}
