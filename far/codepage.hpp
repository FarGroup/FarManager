#pragma once

/*
codepage.hpp

Работа с кодовыми страницами
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "configdb.hpp"
#include "windowsfwd.hpp"

namespace unicode
{
	size_t to(uintptr_t Codepage, const wchar_t* Data, size_t Size, char* Buffer, size_t BufferSize, bool* UsedDefaultChar = nullptr);
	std::string to(uintptr_t Codepage, const wchar_t* Data, size_t Size, bool* UsedDefaultChar = nullptr);

	size_t from(uintptr_t Codepage, const char* Data, size_t Size, wchar_t* Buffer, size_t BufferSize);
	string from(uintptr_t Codepage, const char* Data, size_t Size);
}

// Тип выбранной таблицы символов
enum CPSelectType
{
	CPST_FAVORITE = 1, // Избранная таблица символов
	CPST_FIND     = 2  // Таблица символов участвующая в поиске по всем таблицам символов
};

enum
{
	StandardCPCount = 2 /* OEM, ANSI */ + 2 /* UTF-16 LE, UTF-16 BE */ + 1 /* UTF-8 */
};

inline bool IsUnicodeCodePage(uintptr_t cp) { return cp == CP_UNICODE || cp == CP_REVERSEBOM; }
inline bool IsStandardCodePage(uintptr_t cp) { return IsUnicodeCodePage(cp) || cp == CP_UTF8 || cp == GetOEMCP() || cp == GetACP(); }
inline bool IsUnicodeOrUtfCodePage(uintptr_t cp) { return IsUnicodeCodePage(cp) || cp==CP_UTF8 || cp==CP_UTF7; }

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072.aspx
inline bool IsNoFlagsCodepage(uintptr_t cp) { return (cp >= 50220 && cp <= 50222) || cp == 50225 || cp == 50227 || cp == 50229 || (cp >= 57002 && cp <= 57011) || cp == CP_UTF7 || cp == CP_SYMBOL; }

class Dialog;
struct DialogBuilderListItem2;
class VMenu2;
ENUM(CodePagesCallbackCallSource);

class codepages: noncopyable
{
public:
	~codepages();

	std::pair<UINT, string> GetCodePageInfo(UINT cp) const;
	bool IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize = size_t(-1)) const;
	bool SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool ViewOnly=false, bool bShowAutoDetect=false);
	UINT FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly=false);
	void FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly=false);
	string& FormatCodePageName(uintptr_t CodePage, string& CodePageName) const;

	static long long GetFavorite(uintptr_t cp);
	static void SetFavorite(uintptr_t cp, long long value);
	static void DeleteFavorite(uintptr_t cp);
	static GeneralConfig::int_values_enumerator GetFavoritesEnumerator();

private:
	friend codepages& Codepages();
	friend class system_codepages_enumerator;

	codepages();

	string& FormatCodePageName(uintptr_t CodePage, string& CodePageName, bool &IsCodePageNameCustom) const;
	inline uintptr_t GetMenuItemCodePage(int Position=-1);
	inline uintptr_t GetListItemCodePage(int Position=-1);
	inline bool IsPositionStandard(UINT position);
	inline bool IsPositionFavorite(UINT position);
	inline bool IsPositionNormal(UINT position);
	string FormatCodePageString(uintptr_t CodePage, const string& CodePageName, bool IsCodePageNameCustom) const;
	void AddCodePage(const string& codePageName, uintptr_t codePage, int position, bool enabled, bool checked, bool IsCodePageNameCustom);
	void AddStandardCodePage(const wchar_t *codePageName, uintptr_t codePage, int position=-1, bool enabled=true);
	void AddSeparator(LPCWSTR Label=nullptr,int position=-1);
	int GetItemsCount();
	int GetCodePageInsertPosition(uintptr_t codePage, int start, int length);
	void AddCodePages(DWORD codePages);
	void SetFavorite(bool State);
	void FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly=false, bool bShowAutoDetect=false);
	intptr_t EditDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void EditCodePageName();

	Dialog* dialog;
	UINT control;
	std::vector<DialogBuilderListItem2> *DialogBuilderList;
	vmenu2_ptr CodePagesMenu;
	uintptr_t currentCodePage;
	int favoriteCodePages, normalCodePages;
	bool selectedCodePages;
	CodePagesCallbackCallSource CallbackCallSource;

	class codepages_data
	{
	public:
		typedef std::unordered_map<UINT, std::pair<UINT, string>> cp_map;
		const cp_map& get() const;

	private:
		friend class system_codepages_enumerator;

		mutable cp_map installed_cp;
	}
	data;
};

codepages& Codepages();

//#############################################################################

class MultibyteCodepageDecoder
{
public:
	UINT current_cp;
	int  current_mb;

	bool SetCP(UINT cp);

	int GetChar(const char* buff, size_t cb, wchar_t& wchar) const;

	MultibyteCodepageDecoder() : current_cp(0), current_mb(0) {}

private:
	std::vector<BYTE> len_mask; //[256]
	std::vector<wchar_t> m1;    //[256]
	std::vector<wchar_t> m2;  //[65536]
};

//#############################################################################

namespace Utf
{
	const wchar_t REPLACE_CHAR  = L'\xFFFD'; // Replacement
	const wchar_t BOM_CHAR      = L'\xFEFF'; // Zero Length Space
	const wchar_t CONTINUE_CHAR = L'\x203A'; // Single Right-Pointing Angle Quotation Mark


	struct Errs
	{
		int first_src;
		int first_out;
		int count;
		bool small_buff;
	};

	int ToWideChar(uintptr_t cp, const char *src, size_t len, wchar_t* out, size_t wlen, Errs *errs);
}

namespace Utf7 {
	int ToWideChar(const char *src, size_t len, wchar_t* out, size_t wlen, Utf::Errs *errs);
}

namespace Utf8 {
	int ToWideChar(const char *s, size_t nc, wchar_t *w1, wchar_t *w2, size_t wlen, int &tail);
	int ToWideChar(const char *src, size_t len, wchar_t* out, size_t wlen, Utf::Errs *errs);
	size_t ToMultiByte(const wchar_t *src, size_t len, char *dst);
}

//#############################################################################

class F8CP
{
public:
	F8CP(bool viewer=false);

	uintptr_t NextCP(uintptr_t cp) const;
	const string& NextCPname(uintptr_t cp) const;

private:
	string m_AcpName, m_OemName, m_UtfName;
	mutable string m_Number;
	std::vector<UINT> m_F8CpOrder;
};

//#############################################################################
