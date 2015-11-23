#pragma once

/*
codepage_selection.hpp
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

// Тип выбранной таблицы символов
enum CPSelectType
{
	CPST_FAVORITE = 1, // Избранная таблица символов
	CPST_FIND = 2  // Таблица символов участвующая в поиске по всем таблицам символов
};

enum
{
	StandardCPCount = 2 /* OEM, ANSI */ + 2 /* UTF-16 LE, UTF-16 BE */ + 1 /* UTF-8 */
};

class Dialog;
struct DialogBuilderListItem2;
class VMenu2;
ENUM(CodePagesCallbackCallSource);

class codepages: noncopyable
{
public:
	~codepages();

	bool IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize = size_t(-1)) const;
	bool SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool ViewOnly = false, bool bShowAutoDetect = false);
	UINT FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly = false);
	void FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly = false);
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
	inline size_t GetMenuItemCodePage(size_t Position = -1);
	inline size_t GetListItemCodePage(size_t Position);
	inline bool IsPositionStandard(UINT position);
	inline bool IsPositionFavorite(UINT position);
	inline bool IsPositionNormal(UINT position);
	string FormatCodePageString(uintptr_t CodePage, const string& CodePageName, bool IsCodePageNameCustom) const;
	void AddCodePage(const string& codePageName, uintptr_t codePage, size_t position, bool enabled, bool checked, bool IsCodePageNameCustom);
	void AddStandardCodePage(const wchar_t *codePageName, uintptr_t codePage, int position = -1, bool enabled = true);
	void AddSeparator(LPCWSTR Label = nullptr, size_t position = -1);
	size_t size() const;
	size_t GetCodePageInsertPosition(uintptr_t codePage, size_t start, size_t length);
	void AddCodePages(DWORD codePages);
	void SetFavorite(bool State);
	void FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly = false, bool bShowAutoDetect = false);
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
};

codepages& Codepages();

class F8CP
{
public:
	F8CP(bool viewer = false);

	uintptr_t NextCP(uintptr_t cp) const;
	const string& NextCPname(uintptr_t cp) const;

private:
	string m_AcpName, m_OemName, m_UtfName;
	mutable string m_Number;
	std::vector<UINT> m_F8CpOrder;
};
