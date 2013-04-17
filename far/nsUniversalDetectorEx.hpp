#pragma once

/*
nsUniversalDetectorEx.hpp

UCD
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

namespace ucd
{
#include "UCD/nscore.h"
#include "UCD/nsUniversalDetector.h"
}

class nsUniversalDetectorEx : public ucd::nsUniversalDetector
{
	public:

		nsUniversalDetectorEx() : nsUniversalDetector(NS_FILTER_NON_CJK)
		{
			m_codepage = -1;
		}

		int getCodePage()
		{
			return m_codepage;
		}

	protected:

		virtual void Report(const char* aCharset) override
		{
			if (!strcmp(aCharset, "windows-1250"))
				m_codepage = 1250;
			else if (!strcmp(aCharset, "windows-1251"))
				m_codepage = 1251;
			else if (!strcmp(aCharset, "windows-1252"))
				m_codepage = 1252;
			else if (!strcmp(aCharset, "windows-1253"))
				m_codepage = 1253;
			else if (!strcmp(aCharset, "windows-1255"))
				m_codepage = 1255;
			else if (!strcmp(aCharset, "UTF16-LE"))
				m_codepage = CP_UNICODE;
			else if (!strcmp(aCharset, "UTF16-BE"))
				m_codepage = CP_REVERSEBOM;
			else if (!strcmp(aCharset, "UTF-8"))
				m_codepage = CP_UTF8;
			else if (!strcmp(aCharset, "IBM855"))
				m_codepage = 855;
			else if (!strcmp(aCharset, "IBM866"))
				m_codepage = 866;
			else if (!strcmp(aCharset, "KOI8-R"))
				m_codepage = 20866;
			else if (!strcmp(aCharset, "x-mac-hebrew"))
				m_codepage = 10005;
			else if (!strcmp(aCharset, "x-mac-cyrillic"))
				m_codepage = 1251; /*10007*/ //Оно слишком похоже на 1251 и детектор бывает путает.
			else if (!strcmp(aCharset, "ISO-8859-2"))
				m_codepage = 28592;
			else if (!strcmp(aCharset, "ISO-8859-5"))
				m_codepage = 28595;
			else if (!strcmp(aCharset, "ISO-8859-7"))
				m_codepage = 28597;
			else if (!strcmp(aCharset, "ISO-8859-8"))
				m_codepage = 28598;
			else if (!strcmp(aCharset, "ISO-8859-8-I"))
				m_codepage = 38598;

			/*
			and the rest:
			"Shift_JIS"
			"gb18030"
			"x-euc-tw"
			"EUC-KR"
			"EUC-JP"
			"Big5"
			"X-ISO-10646-UCS-4-3412" - UCS-4, unusual octet order BOM (3412)
			"UTF-32BE"
			"X-ISO-10646-UCS-4-2143" - UCS-4, unusual octet order BOM (2143)
			"UTF-32LE"
			ISO-2022-CN
			ISO-2022-JP
			ISO-2022-KR
			"TIS-620"
			*/
		}

	private:

		int m_codepage;
};
