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

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng : int;
class RegExp;
struct error_state_ex;

/*
  Функция GetSearchReplaceString выводит диалог поиска или замены, принимает
  от пользователя данные и в случае успешного выполнения диалога возвращает
  TRUE.
  Параметры:
    IsReplaceMode
      true  - если хотим заменять
      false - если хотим искать

    Title
      Заголовок диалога.
      Если пустая строка, то применяется MEditReplaceTitle или MEditSearchTitle в зависимости от параметра IsReplaceMode

    SearchStr
      Строка поиска.
      Результат отработки диалога заносится в нее же.

    ReplaceStr,
      Строка замены.
      Результат отработки диалога заносится в нее же.
      Для случая, если IsReplaceMode=FALSE может быть равна nullptr

    TextHistoryName
      Имя истории строки поиска.
      Если пустая строка, то принимается значение "SearchText"

    ReplaceHistoryName
      Имя истории строки замены.
      Если пустая строка, то принимается значение "ReplaceText"

    Case
      Ссылка на переменную, указывающую на значение опции "Case sensitive"

    WholeWords
      Ссылка на переменную, указывающую на значение опции "Whole words"

    Reverse
      Ссылка на переменную, указывающую на значение опции "Reverse search"

    SelectFound
      Ссылка на переменную, указывающую на значение опции "Select found"

    Regexp
      Ссылка на переменную, указывающую на значение опции "Regular expressions"

    Regexp
      Ссылка на переменную, указывающую на значение опции "Preserve style"

    HelpTopic
      Имя темы помощи.
      Если пустая строка - тема помощи не назначается.

  Возвращаемое значение:
  0 - пользователь отказался от диалога (Esc)
    1  - пользователь подтвердил свои намерения
    2 - выбран поиск всех вхождений

*/
int GetSearchReplaceString(
	bool IsReplaceMode,
	string_view Title,
	string_view SubTitle,
	string& SearchStr,
	string& ReplaceStr,
	string_view TextHistoryName,
	string_view ReplaceHistoryName,
	bool* pCase,
	bool* pWholeWords,
	bool* pReverse,
	bool* pRegexp,
	bool* pPreserveStyle,
	string_view HelpTopic = {},
	bool HideAll=false,
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

class operation_cancelled: public std::exception
{
};

[[noreturn]]
inline void cancel_operation()
{
	throw operation_cancelled{};
}

void ReCompileErrorMessage(const RegExp& re, string_view str);
void ReMatchErrorMessage(const RegExp& re);

struct goto_coord
{
	long long value;
	bool exist;
	bool percent;
	int relative;
};

bool GoToRowCol(goto_coord& Row, goto_coord& Col, bool& Hex, string_view HelpTopic);

bool RetryAbort(std::vector<string>&& Messages);

#endif // STDDLG_HPP_D7E3481D_D478_4F57_8C20_7E0A21FAE788
