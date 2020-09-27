#ifndef CODEPAGE_SELECTION_HPP_AD209CF7_F280_4E6D_83A7_F0601E4EBB71
#define CODEPAGE_SELECTION_HPP_AD209CF7_F280_4E6D_83A7_F0601E4EBB71
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

// Internal:
#include "configdb.hpp"
#include "windowsfwd.hpp"

// Platform:

// Common:
#include "common/singleton.hpp"
#include "common/view/select.hpp"

// External:

//----------------------------------------------------------------------------

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
class DialogBuilderListItem;
class VMenu2;
enum CodePagesCallbackCallSource: int;
struct cp_info;

class codepages: public singleton<codepages>
{
	IMPLEMENTS_SINGLETON;

public:
	NONCOPYABLE(codepages);
	~codepages();

	bool SelectCodePage(uintptr_t& CodePage, bool ViewOnly, bool bShowAutoDetect);
	size_t FillCodePagesList(Dialog* Dlg, size_t controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool allowChecked, bool bViewOnly);
	void FillCodePagesList(std::vector<DialogBuilderListItem> &List, bool allowAuto, bool allowAll, bool allowDefault, bool allowChecked, bool bViewOnly);

	static bool IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize = size_t(-1));
	static std::optional<cp_info> GetInfo(uintptr_t CodePage);
	static long long GetFavorite(uintptr_t cp);
	static void SetFavorite(uintptr_t cp, long long value);
	static void DeleteFavorite(uintptr_t cp);
	static auto GetFavoritesEnumerator()
	{
		return select(ConfigProvider().GeneralCfg()->ValuesEnumerator<long long>(FavoriteCodePagesKey()),
			[t = std::pair<unsigned long, long long>{}](const auto& i) mutable -> auto&
		{
			// All this magic is to keep reference semantics to make analysers happy.
			t = { std::stoul(i.first), i.second };
			return t;
		});
	}

private:
	friend class system_codepages_enumerator;

	codepages();

	static bool GetCodePageCustomName(uintptr_t CodePage, string& CodePageName);
	size_t GetMenuItemCodePage(size_t Position = -1) const;
	size_t GetListItemCodePage(size_t Position) const;
	bool IsPositionStandard(size_t position) const;
	bool IsPositionFavorite(size_t position) const;
	bool IsPositionNormal(size_t position) const;
	string FormatCodePageString(uintptr_t CodePage, string_view CodePageName, bool IsCodePageNameCustom) const;
	void AddCodePage(string_view codePageName, uintptr_t codePage, size_t position, bool enabled, bool checked, bool IsCodePageNameCustom) const;
	void AddStandardCodePage(string_view codePageName, uintptr_t codePage, int position = -1, bool enabled = true) const;
	void AddSeparator(const string& Label, size_t position = -1) const;
	size_t size() const;
	size_t GetCodePageInsertPosition(uintptr_t codePage, size_t start, size_t length);
	void AddCodePages(DWORD codePages);
	void SetFavorite(bool State);
	void FillCodePagesVMenu(bool bViewOnly, bool bShowAutoDetect);
	intptr_t EditDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void EditCodePageName();

	static string_view FavoriteCodePagesKey();

	Dialog* dialog{};
	size_t control{};
	std::vector<DialogBuilderListItem> *DialogBuilderList{};
	vmenu2_ptr CodePagesMenu{};
	uintptr_t currentCodePage{};
	int favoriteCodePages{}, normalCodePages{};
	bool selectedCodePages{};
	CodePagesCallbackCallSource CallbackCallSource;
};

class F8CP
{
public:
	explicit F8CP(bool viewer = false);

	uintptr_t NextCP(uintptr_t cp) const;
	string NextCPname(uintptr_t cp) const;

private:
	string m_AcpName, m_OemName, m_UtfName;
	std::vector<uintptr_t> m_F8CpOrder;
};

#endif // CODEPAGE_SELECTION_HPP_AD209CF7_F280_4E6D_83A7_F0601E4EBB71
