#ifndef HISTORY_HPP_B662E92D_BF1B_4B20_AD60_8959531FA6EE
#define HISTORY_HPP_B662E92D_BF1B_4B20_AD60_8959531FA6EE
#pragma once

/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)
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

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/function_ref.hpp"
#include "common/noncopyable.hpp"
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

class Dialog;
class HistoryConfig;
class VMenu2;
class Bool3Option;

enum history_type
{
	HISTORYTYPE_CMD,
	HISTORYTYPE_FOLDER,
	HISTORYTYPE_VIEW,
	HISTORYTYPE_DIALOG
};

enum history_record_type: int
{
	HR_DEFAULT,
	HR_VIEWER = HR_DEFAULT,
	HR_EDITOR,
	HR_EXTERNAL,
	HR_EXTERNAL_WAIT,
	HR_EDITOR_RO,
};

enum history_return_type
{
	HRT_CANCEL,
	HRT_ENTER,
	HRT_SHIFTENTER,
	HRT_CTRLENTER,
	HRT_F3, //internal
	HRT_F4, //internal
	HRT_CTRLSHIFTENTER,
	HRT_CTRLALTENTER,
};

class History: noncopyable
{
public:
	History(history_type TypeHistory, string_view HistoryName, const Bool3Option& State, bool KeepSelectedPos);

	void AddToHistory(string_view Str, history_record_type Type = HR_DEFAULT, const UUID* Uuid = nullptr, string_view File = {}, string_view Data = {});
	history_return_type Select(string_view Title, string_view HelpTopic, string& strStr, history_record_type& Type, UUID* Uuid = nullptr, string* File = nullptr, string* Data = nullptr);
	history_return_type Select(VMenu2& HistoryMenu, int Height, Dialog const* Dlg, string& strStr);
	string GetPrev();
	string GetNext();
	bool GetSimilar(string &strStr, int LastCmdPartLength, bool bAppend=false);
	void GetAllSimilar(string_view Str, function_ref<void(string_view Name, unsigned long long Id, bool IsLocked)> Callback) const;
	void ResetPosition() { m_CurrentItem = 0; }
	bool DeleteIfUnlocked(unsigned long long id);
	string LastItem();
	bool IsOnTop() const { return !m_CurrentItem; }

	static void CompactHistory();

	[[nodiscard]]
	auto suppressor() { return make_raii_wrapper<&History::suppress_add, &History::restore_add>(this); }

private:
	bool EqualType(history_record_type Type1, history_record_type Type2) const;
	history_return_type ProcessMenu(string& strStr, UUID* Uuid, string* File, string* Data, string_view Title, VMenu2& HistoryMenu, int Height, history_record_type& Type, const Dialog* Dlg);
	const std::unique_ptr<HistoryConfig>& HistoryCfgRef() const;

	void introduce_record(os::chrono::time_point Time) const;
	void forget_record(os::chrono::time_point Time) const;
	bool is_known_record(os::chrono::time_point Time) const;
	void refresh_known_records() const;

	void suppress_add();
	void restore_add();

	history_type m_TypeHistory;
	string m_HistoryName;
	Bool3Option const& m_State;
	std::atomic_size_t m_SuppressAdd{};
	bool m_KeepSelectedPos;
	unsigned long long m_CurrentItem{};
};

#endif // HISTORY_HPP_B662E92D_BF1B_4B20_AD60_8959531FA6EE
