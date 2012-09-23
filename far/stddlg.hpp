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

/*
  Функция GetSearchReplaceString выводит диалог поиска или замены, принимает
  от пользователя данные и в случае успешного выполнения диалога возвращает
  TRUE.
  Параметры:
    IsReplaceMode
      true  - если хотим заменять
      false - если хотим искать

    SearchStr
      Строка поиска.
      Результат отработки диалога заносится в нее же.

    ReplaceStr,
      Строка замены.
      Результат отработки диалога заносится в нее же.
      Для случая, если IsReplaceMode=FALSE может быть равна nullptr

    TextHistoryName
      Имя истории строки поиска.
      Если установлено в nullptr, то по умолчанию
      принимается значение "SearchText"
      Если установлено в пустую строку, то история вестись не будет

    ReplaceHistoryName
      Имя истории строки замены.
      Если установлено в nullptr, то по умолчанию
      принимается значение "ReplaceText"
      Если установлено в пустую строку, то история вестись не будет

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

    HelpTopic
      Имя темы помощи.
      Если nullptr или пустая строка - тема помощи не назначается.

  Возвращаемое значение:
  0 - пользователь отказался от диалога (Esc)
    1  - пользователь подтвердил свои намерения
    2 - выбран поиск всех вхождений

*/
int GetSearchReplaceString(
    bool IsReplaceMode,
    string& SearchStr,
    string& ReplaceStr,
    const wchar_t *TextHistoryName,
    const wchar_t *ReplaceHistoryName,
    bool& Case,
    bool& WholeWords,
    bool& Reverse,
    bool& Regexp,
    const wchar_t *HelpTopic=nullptr);

int GetString(
    const wchar_t *Title,
    const wchar_t *SubTitle,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    string &strDestText,
    const wchar_t *HelpTopic = nullptr,
    DWORD Flags = 0,
    int *CheckBoxValue = nullptr,
    const wchar_t *CheckBoxText = nullptr,
    class Plugin* PluginNumber = nullptr,
    const GUID* Id = nullptr
);

// для диалога GetNameAndPassword()
enum FlagsNameAndPassword
{
	GNP_USELAST      = 0x00000001UL, // использовать последние введенные данные
};

int GetNameAndPassword(const wchar_t *Title,string &strUserName, string &strPassword, const wchar_t *HelpTopic,DWORD Flags);

int OperationFailed(const string& Object, LNGID Title, const wchar_t* Description, bool AllowSkip = true);
