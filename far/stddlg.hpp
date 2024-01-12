#ifndef STDDLG_HPP_D7E3481D_D478_4F57_8C20_7E0A21FAE788
#define STDDLG_HPP_D7E3481D_D478_4F57_8C20_7E0A21FAE788
#pragma once

/*
stddlg.hpp

Куча разных стандартных диалогов
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
#include "windowsfwd.hpp"
#include "dialog.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/function_ref.hpp"
#include "common/preprocessor.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng : int;
class regex_exception;
struct error_state_ex;

struct SearchReplaceDlgProps
{
	bool ReplaceMode{};
	bool ShowButtonsPrevNext{};
	bool ShowButtonAll{};
};

struct SearchReplaceDlgParams
{
	string SearchStr;
	std::optional<bytes> SearchBytes;
	std::optional<string> ReplaceStr;
	std::optional<bool> Hex;
	std::optional<bool> CaseSensitive;
	std::optional<bool> WholeWords;
	std::optional<bool> Regex;
	std::optional<bool> Fuzzy;
	std::optional<bool> PreserveStyle;

	enum class SharedGroup { view_edit, find_file, help, count };

	static const SearchReplaceDlgParams& GetShared(SharedGroup Group);
	void SaveToShared(SharedGroup Group) const;

	// Uses Hex.value_or(false)
	void SetSearchPattern(string_view TextString, string_view HexString, uintptr_t CodePage);

	auto IsSearchPatternEmpty() const noexcept
	{
		return Hex.value_or(false) ? SearchBytes.value().empty() : SearchStr.empty();
	}
};


enum class SearchReplaceDlgResult
{
	Cancel,
	Ok,
	Prev,
	Next,
	All,
};

/*
  Shows Search / Replace dialog, collects user's input, and returns the action selected by the user.

  Parameters:
    Props
      Define various aspects of dialog UX

    Params
      InOut parameter. Specifies which options to show in the dialog and provides initial values.
      If the dialog was closed by one of the action buttons, contains the values selected by the user.
      If the dialog was dismissed, the values stay unchanged.

    TextHistoryName
      Имя истории строки поиска.

    ReplaceHistoryName
      Имя истории строки замены.

    HelpTopic
      Имя темы помощи.
      Если пустая строка - тема помощи не назначается.

  Returns the action selected by the user.
*/
SearchReplaceDlgResult GetSearchReplaceString(
	SearchReplaceDlgProps Props,
	SearchReplaceDlgParams& Params,
	string_view TextHistoryName,
	string_view ReplaceHistoryName,
	uintptr_t CodePage,
	string_view HelpTopic = {},
	const UUID* Id = nullptr,
	function_ref<string(bool)> Picker = nullptr
);

bool GetString(
	string_view Title,
	string_view Prompt,
	string_view HistoryName,
	string_view SrcText,
	string &strDestText,
	string_view HelpTopic = {},
	DWORD Flags = 0,
	int* CheckBoxValue = {},
	string_view CheckBoxText = {},
	class Plugin* PluginNumber = {},
	const UUID* Id = {}
);

// для диалога GetNameAndPassword()
enum FlagsNameAndPassword
{
	GNP_USELAST      = 0_bit, // использовать последние введенные данные
};

bool GetNameAndPassword(string_view Title, string& strUserName, string& strPassword, string_view HelpTopic, DWORD Flags);

enum class operation
{
	retry,
	skip,
	skip_all,
	cancel
};

operation OperationFailed(const error_state_ex& ErrorState, string_view Object, lng Title, string Description, bool AllowSkip = true, bool AllowSkipAll = true);

class operation_cancelled final: public std::exception
{
};

[[noreturn]]
inline void cancel_operation()
{
	throw operation_cancelled{};
}

// true: success
// false: skip
// operation_cancelled exception: cancelled
bool retryable_ui_operation(function_ref<bool()> Action, string_view Name, lng ErrorDescription, bool& SkipErrors);

void ReCompileErrorMessage(regex_exception const& e, string_view str);

struct goto_coord
{
	long long value;
	bool exist;
	bool percent;
	int relative;
};

bool GoToRowCol(goto_coord& Row, goto_coord& Col, bool& Hex, string_view HelpTopic);

bool ConfirmAbort();
bool CheckForEscAndConfirmAbort();
bool RetryAbort(std::vector<string>&& Messages);

void regex_playground();

class progress_impl
{
protected:
	NONCOPYABLE(progress_impl);

	progress_impl() = default;
	~progress_impl();

	void init(std::span<DialogItemEx> Items, rectangle Position, const UUID* Id = nullptr);

	dialog_ptr m_Dialog;
};

class single_progress: progress_impl
{
public:
	single_progress(string_view Title, string_view Msg, size_t Percent);

	void update(string_view Msg) const;
	void update(size_t Percent) const;
};

class dirinfo_progress: progress_impl
{
public:
	explicit dirinfo_progress(string_view Title);

	void set_name(string_view Msg) const;
	void set_count(unsigned long long Count) const;
	void set_size(unsigned long long Size) const;
};

#endif // STDDLG_HPP_D7E3481D_D478_4F57_8C20_7E0A21FAE788
