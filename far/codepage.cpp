/*
codepage.cpp
*/
/*
Copyright © 2024 Far Group
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
#include "codepage.hpp"

// Internal:
#include "exception_handler.hpp"
#include "log.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/from_string.hpp"

// External:

//----------------------------------------------------------------------------

uintptr_t encoding::codepage::detail::utf8::id()
{
	return CP_UTF8;
}

uintptr_t encoding::codepage::detail::ansi::id()
{
	return GetACP();
}

uintptr_t encoding::codepage::detail::oem::id()
{
	return GetOEMCP();
}

uintptr_t encoding::codepage::normalise(uintptr_t const Codepage)
{
	switch (Codepage)
	{
	case CP_OEMCP: return oem();
	case CP_ACP:   return ansi();
	default:       return Codepage;
	}
}

static string_view extract_codepage_name(string_view const Str)
{
	// Windows: "XXXX (Name)", Wine: "Name"

	const auto OpenBracketPos = Str.find(L'(');
	if (OpenBracketPos == Str.npos)
		return Str;

	const auto Name = Str.substr(OpenBracketPos + 1);

	const auto CloseBracketPos = Name.rfind(L')');
	if (CloseBracketPos == Str.npos)
		return Str;

	return Name.substr(0, CloseBracketPos);
}

static std::optional<cp_info> get_codepage_info(unsigned const Codepage, wchar_t const* const CodepageStr)
{
	if (CPINFOEX Info; GetCPInfoEx(Codepage, 0, &Info))
	{
		return
		{{
			string(extract_codepage_name(Info.CodePageName)),
			static_cast<unsigned char>(Info.MaxCharSize)
		}};
	}

	if (const auto LastError = os::last_error(); LastError.Win32Error)
		LOGDEBUG(L"GetCPInfoEx({}): {}"sv, Codepage, LastError);

	if (CPINFO Info; GetCPInfo(Codepage, &Info))
	{
		return
		{{
			CodepageStr,
			static_cast<unsigned char>(Info.MaxCharSize)
		}};
	}

	if (const auto LastError = os::last_error(); LastError.Win32Error)
		LOGWARNING(L"GetCPInfo({}): {}"sv, Codepage, LastError);

	return {};
}

class installed_codepages
{
public:
	explicit installed_codepages(cp_map& InstalledCp):
		m_InstalledCp(&InstalledCp)
	{
		Context = this;

		if (!EnumSystemCodePages(callback, CP_INSTALLED))
			LOGWARNING(L"EnumSystemCodePages(): {}"sv, os::last_error());

		Context = {};

		rethrow_if(m_ExceptionPtr);
	}

private:
	static inline thread_local installed_codepages* Context;

	static BOOL WINAPI callback(wchar_t* const cpNum)
	{
		return Context->enum_cp_callback(cpNum);
	}

	BOOL enum_cp_callback(wchar_t const* CpStr)
	{
		return cpp_try(
		[&]
		{
			const auto Codepage = from_string<unsigned>(CpStr);

			if (const auto Info = get_codepage_info(Codepage, CpStr); Info && Info->MaxCharSize)
				m_InstalledCp->try_emplace(Codepage, *Info);

			return TRUE;
		},
		save_exception_and_return<FALSE>(m_ExceptionPtr)
		);
	}

	cp_map* m_InstalledCp;
	std::exception_ptr m_ExceptionPtr;
};

static auto get_installed_codepages()
{
	cp_map InstalledCodepages;
	SCOPED_ACTION(installed_codepages)(InstalledCodepages);
	return InstalledCodepages;
}

const cp_map& InstalledCodepages()
{
	static const auto s_InstalledCodepages = get_installed_codepages();
	return s_InstalledCodepages;
}

cp_info const* GetCodePageInfo(uintptr_t cp)
{
	// Standard unicode CPs (1200, 1201, 65001) are NOT in the list.
	const auto& InstalledCp = InstalledCodepages();

	if (const auto found = InstalledCp.find(static_cast<unsigned>(cp)); found != InstalledCp.cend())
		return &found->second;

	return {};
}

bool IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize)
{
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	const auto Info = GetCodePageInfo(CodePage);
	return Info && Info->MaxCharSize <= MaxCharSize;
}

bool IsVirtualCodePage(uintptr_t cp)
{
	return cp == CP_DEFAULT || cp == CP_REDETECT || cp == CP_ALL;
}

bool IsUtf16CodePage(uintptr_t cp)
{
	return cp == CP_UTF16LE || cp == CP_UTF16BE;
}

bool IsStandardCodePage(uintptr_t cp)
{
	return IsUtfCodePage(cp) || cp == encoding::codepage::ansi() || cp == encoding::codepage::oem();
}

bool IsUtfCodePage(uintptr_t cp)
{
	return cp == CP_UTF8 || IsUtf16CodePage(cp) || cp == CP_UTF7;
}

string ShortReadableCodepageName(uintptr_t cp)
{
	switch (cp)
	{
	case CP_UTF7:    return L"UTF-7"s;
	case CP_UTF8:    return L"UTF-8"s;
	case CP_UTF16LE: return L"U16LE"s;
	case CP_UTF16BE: return L"U16BE"s;
	default: return
		cp == encoding::codepage::ansi()? L"ANSI"s :
		cp == encoding::codepage::oem()?  L"OEM"s :
		str(cp);
	}
}


#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("encoding.extract_codepage_name")
{
	static const struct
	{
		string_view Str, Name;
	}
	Tests[]
	{
		{ {},                        {} },
		{ L"banana"sv,               L"banana"sv },
		{ L"69 (ANSI - Klingon)"sv,  L"ANSI - Klingon"sv },
		{ L"(((deeper)))"sv,         L"((deeper))"sv },
		{ L"(no"sv,                  L"(no"sv },
		{ L")(oh no"sv,              L")(oh no"sv },
		{ L")(oh yes)("sv,           L"oh yes"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(extract_codepage_name(i.Str) == i.Name);
	}
}
#endif
