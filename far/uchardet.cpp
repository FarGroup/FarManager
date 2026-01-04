/*
uchardet.cpp

UCD wrapper

*/
/*
Copyright © 2011 Far Group
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
#include "uchardet.hpp"

// Internal:
#include "components.hpp"
#include "config.hpp"
#include "encoding.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/preprocessor.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace uchardet
{
WARNING_PUSH(2)

WARNING_DISABLE_MSC(4305) // '=': truncation from 'double' to 'float'
WARNING_DISABLE_MSC(4701) // '=': potentially uninitialized local variable 'name' used
WARNING_DISABLE_MSC(5219) // implicit conversion from 'type1' to 'type2', possible loss of data
WARNING_DISABLE_MSC(5262) // implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases

WARNING_DISABLE_GCC("-Wcast-qual")
WARNING_DISABLE_GCC("-Wdouble-promotion")
WARNING_DISABLE_GCC("-Wlogical-op")
WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
WARNING_DISABLE_GCC("-Wmaybe-uninitialized")
WARNING_DISABLE_GCC("-Wold-style-cast")
WARNING_DISABLE_GCC("-Wsuggest-override")
WARNING_DISABLE_GCC("-Wzero-as-null-pointer-constant")

WARNING_DISABLE_CLANG("-Weverything")
WARNING_DISABLE_CLANG("-Wold-style-cast")

#include "thirdparty/uchardet/nscore.h"
#include "thirdparty/uchardet/nsUniversalDetector.h"

#include "thirdparty/uchardet/CharDistribution.cpp"
#include "thirdparty/uchardet/JpCntx.cpp"
#include "thirdparty/uchardet/nsBig5Prober.cpp"
#include "thirdparty/uchardet/nsCharSetProber.cpp"
#include "thirdparty/uchardet/nsCJKDetector.cpp"
#include "thirdparty/uchardet/nsEscCharsetProber.cpp"
#include "thirdparty/uchardet/nsEscSM.cpp"
#include "thirdparty/uchardet/nsEUCJPProber.cpp"
#include "thirdparty/uchardet/nsEUCKRProber.cpp"
#include "thirdparty/uchardet/nsEUCTWProber.cpp"
#include "thirdparty/uchardet/nsGB2312Prober.cpp"
#include "thirdparty/uchardet/nsHebrewProber.cpp"
#include "thirdparty/uchardet/nsJohabProber.cpp"
#include "thirdparty/uchardet/nsLanguageDetector.cpp"
//#include "thirdparty/uchardet/nsLatin1Prober.cpp"
#include "thirdparty/uchardet/nsMBCSGroupProber.cpp"
#include "thirdparty/uchardet/nsMBCSSM.cpp"
#include "thirdparty/uchardet/nsSBCharSetProber.cpp"
#include "thirdparty/uchardet/nsSBCSGroupProber.cpp"
#include "thirdparty/uchardet/nsSJISProber.cpp"
#include "thirdparty/uchardet/nsUniversalDetector.cpp"
//#include "thirdparty/uchardet/nsUTF8Prober.cpp"

#define UCHARDET_LANGUAGE Arabic
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Belarusian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Bulgarian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Catalan
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Croatian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Czech
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Danish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE English
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Esperanto
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Estonian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Finnish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE French
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Georgian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE German
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Greek
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Hebrew
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Hindi
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Hungarian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Irish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Italian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Latvian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Lithuanian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Macedonian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Maltese
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Norwegian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Polish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Portuguese
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Romanian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Russian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Serbian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Slovak
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Slovene
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Spanish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Swedish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Thai
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Turkish
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Ukrainian
#include "uchardet_model.hpp"
#define UCHARDET_LANGUAGE Vietnamese
#include "uchardet_model.hpp"

WARNING_POP()
}

namespace
{
	SCOPED_ACTION(components::component)([]
	{
		return components::info{ L"uchardet"sv, L"0.0.8"sv }; // BUGBUG, update manually when needed
	});
}

static const auto& CpMap()
{
	static const std::unordered_map<std::string_view, uintptr_t> Map
	{
		{ "CP737"sv,             737 },

		{ "IBM852"sv,            852 },
		{ "IBM855"sv,            855 },
		{ "IBM862"sv,            862 },
		{ "IBM865"sv,            865 },
		{ "IBM866"sv,            866 },

		{ "WINDOWS-1250"sv,      1250 },
		{ "WINDOWS-1251"sv,      1251 },
		{ "WINDOWS-1252"sv,      1252 },
		{ "WINDOWS-1253"sv,      1253 },
		{ "WINDOWS-1255"sv,      1255 },
		{ "WINDOWS-1256"sv,      1256 },
		{ "WINDOWS-1257"sv,      1257 },
		{ "WINDOWS-1258"sv,      1258 },

		{ "ISO-8859-1"sv,        28591 },
		{ "ISO-8859-2"sv,        28592 },
		{ "ISO-8859-3"sv,        28593 },
		{ "ISO-8859-4"sv,        28594 },
		{ "ISO-8859-5"sv,        28595 },
		{ "ISO-8859-6"sv,        28596 },
		{ "ISO-8859-7"sv,        28597 },
		{ "ISO-8859-8"sv,        28598 }, // or 38598?
		{ "ISO-8859-9"sv,        28599 },
		{ "ISO-8859-10"sv,       28600 },
		{ "ISO-8859-11"sv,       28601 },
		{ "ISO-8859-13"sv,       28603 },
		{ "ISO-8859-15"sv,       28605 },
		{ "ISO-8859-16"sv,       28606 },

		{ "ISO-2022-CN"sv,       50227 }, // or 50229?
		{ "ISO-2022-JP"sv,       50220 }, // or 50221 / 50222?
		{ "ISO-2022-KR"sv,       50225 },

		{ "EUC-TW"sv,            20000 },
		{ "EUC-JP"sv,            20932 },
		{ "EUC-KR"sv,            51949 },

		{ "TIS-620"sv,           874 },
		{ "SHIFT_JIS"sv,         932 },
		{ "UHC",                 949 },
		{ "BIG5"sv,              950 },
		{ "Johab",               1361 },
		{ "KOI8-R"sv,            20866 },
		{ "VISCII"sv,            28591 }, // No direct mapping, the closest one
		{ "HZ-GB-2312"sv,        52936 },
		{ "GB18030"sv,           54936 },

		{ "MAC-CYRILLIC"sv,      10007 },
		{ "MAC-CENTRALEUROPE"sv, 10029 },

		{ "ASCII"sv,             20127 },

		{ "UTF-8"sv,             CP_UTF8 },
	};

	return Map;
}

class UniversalDetector final: public uchardet::nsUniversalDetector
{
public:
	explicit UniversalDetector(function_ref<bool(uintptr_t)> const IsCodepageAcceptable):
		nsUniversalDetector(Global->Opt->NoAutoDetectCJK ? NS_FILTER_NON_CJK : NS_FILTER_ALL),
		m_IsCodepageAcceptable(IsCodepageAcceptable)
	{
	}

	bool GetCodePage(uintptr_t& Codepage) const
	{
		if (m_Codepage == -1)
			return false;

		Codepage = m_Codepage;

		LOGDEBUG(L"UCD: codepage {} ({})"sv,
			m_Codepage,
			format_result(m_Name, m_Language, m_Confidence)
		);

		return true;
	}

private:
	void Report(const char* const Encoding, const char* const Language, float const Confidence) override
	{
		if (Confidence < m_Confidence)
			return;

		const auto Iterator = CpMap().find(Encoding);
		if (Iterator == CpMap().end())
		{
			LOGWARNING(L"UCD: unexpected result [{}]"sv, format_result(Encoding, Language, Confidence));
			return;
		}

		if (!m_IsCodepageAcceptable(Iterator->second))
			return;

		m_Codepage = Iterator->second;
		m_Name = Encoding;
		m_Language = Language;
		m_Confidence = Confidence;
	}

	static string format_result(const char* const Encoding, const char* const Language, float const Confidence)
	{
		return far::format(L"{}, {}{}{:.0f}%"sv,
			encoding::ascii::get_chars(Encoding),
			Language? encoding::ascii::get_chars(Language) : L""sv,
			Language? L", "sv : L""sv,
			Confidence * 10000 / 100
		);
	}

	function_ref<bool(uintptr_t)> m_IsCodepageAcceptable;
	int m_Codepage{-1};
	const char* m_Name{};
	const char* m_Language{};
	float m_Confidence{};
};

bool GetCpUsingUniversalDetector(std::string_view const Str, uintptr_t& Codepage, function_ref<bool(uintptr_t)> const IsCodepageAcceptable)
{
	UniversalDetector Detector(IsCodepageAcceptable);
	Detector.HandleData(Str.data(), static_cast<uint32_t>(Str.size()));
	Detector.DataEnd();
	return Detector.GetCodePage(Codepage);
}
