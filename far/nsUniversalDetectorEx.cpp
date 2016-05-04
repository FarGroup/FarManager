/*
nsUniversalDetectorEx.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "components.hpp"

namespace ucd
{
WARNING_PUSH(1)

WARNING_DISABLE_GCC("-Wcast-qual")
WARNING_DISABLE_GCC("-Wuseless-cast")
WARNING_DISABLE_GCC("-Wzero-as-null-pointer-constant")
WARNING_DISABLE_GCC("-Wnon-virtual-dtor")


#include "thirdparty/ucd/nsCore.h"
#include "thirdparty/ucd/nsError.h"
#include "thirdparty/ucd/nsUniversalDetector.h"
#include "thirdparty/ucd/CharDistribution.cpp"
#include "thirdparty/ucd/JpCntx.cpp"
#include "thirdparty/ucd/LangBulgarianModel.cpp"
#include "thirdparty/ucd/LangCyrillicModel.cpp"
#include "thirdparty/ucd/LangGreekModel.cpp"
#include "thirdparty/ucd/LangHebrewModel.cpp"
#include "thirdparty/ucd/LangHungarianModel.cpp"
#include "thirdparty/ucd/LangThaiModel.cpp"
#include "thirdparty/ucd/nsBig5Prober.cpp"
#include "thirdparty/ucd/nsCharSetProber.cpp"
#include "thirdparty/ucd/nsEscCharsetProber.cpp"
#include "thirdparty/ucd/nsEscSM.cpp"
#include "thirdparty/ucd/nsEUCJPProber.cpp"
#include "thirdparty/ucd/nsEUCKRProber.cpp"
#include "thirdparty/ucd/nsEUCTWProber.cpp"
#include "thirdparty/ucd/nsGB2312Prober.cpp"
#include "thirdparty/ucd/nsHebrewProber.cpp"
#include "thirdparty/ucd/nsLatin1Prober.cpp"
#include "thirdparty/ucd/nsMBCSGroupProber.cpp"
#include "thirdparty/ucd/nsMBCSSM.cpp"
#include "thirdparty/ucd/nsSBCharSetProber.cpp"
#include "thirdparty/ucd/nsSBCSGroupProber.cpp"
#include "thirdparty/ucd/nsSJISProber.cpp"
#include "thirdparty/ucd/nsUniversalDetector.cpp"
#include "thirdparty/ucd/nsUTF8Prober.cpp"

WARNING_POP()
};

static string getInfo() { return L"Mozilla Universal Charset Detector"; } // BUGBUG, version unknown
SCOPED_ACTION(components::component)(getInfo);

static const auto& CpMap()
{
	static std::unordered_map<std::string, uintptr_t> sCpMap = 
	{
		{ "UTF16-LE", CP_UNICODE },
		{ "UTF16-BE", CP_REVERSEBOM },
		{ "UTF-8", CP_UTF8 },
		{ "windows-1250", 1250 },
		{ "windows-1251", 1251 },
		{ "windows-1252", 1252 },
		{ "windows-1253", 1253 },
		{ "windows-1255", 1255 },
		{ "IBM855", 855 },
		{ "IBM866", 866 },
		{ "KOI8-R", 20866 },
		{ "x-mac-hebrew", 10005 },
		{ "x-mac-cyrillic", /*10007*/ 1251 }, //Оно слишком похоже на 1251 и детектор, бывает, путает
		{ "ISO-8859-2", 28592 },
		{ "ISO-8859-5", 28595 },
		{ "ISO-8859-7", 28597 },
		{ "ISO-8859-8", 28598 },
		{ "ISO-8859-8-I", 38598 },

		/*
		and the rest:

		"Shift_JIS"
		"gb18030"
		"x-euc-tw"
		"EUC-KR"
		"EUC-JP"
		"Big5"
		"X-ISO-10646-UCS-4-3412" - UCS-4, unusual octet order BOM (3412)
		"X-ISO-10646-UCS-4-2143" - UCS-4, unusual octet order BOM (2143)
		"UTF-32BE"
		"UTF-32LE"
		"ISO-2022-CN"
		"ISO-2022-JP"
		"ISO-2022-KR"
		"TIS-620"
		*/
	};
	return sCpMap;
}

class nsUniversalDetectorEx : public ucd::nsUniversalDetector
{
public:
	nsUniversalDetectorEx(): nsUniversalDetector(NS_FILTER_NON_CJK), m_codepage(-1) {}
	int getCodePage() const { return m_codepage; }

private:
	virtual void Report(const char* aCharset) override
	{
		const auto i = CpMap().find(aCharset);
		m_codepage = i != CpMap().end()? i->second : -1;
	}

	int m_codepage;
};

uintptr_t GetCpUsingUniversalDetector(const void* data, size_t size)
{
	nsUniversalDetectorEx ns;
	ns.HandleData(static_cast<const char*>(data), static_cast<uint32_t>(size));
	ns.DataEnd();
	return ns.getCodePage();
}
