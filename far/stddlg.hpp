#pragma once

/*
stddlg.hpp

Куча разных стандартных диалогов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
      TRUE  - если хотим заменять
      FALSE - если хотим искать

    SearchStr
      Указатель на строку поиска.
      Результат отработки диалога заносится в нее же.

    ReplaceStr,
      Указатель на строку замены.
      Результат отработки диалога заносится в нее же.
      Для случая, если IsReplaceMode=FALSE может быть равна NULL

    TextHistoryName
      Имя истории строки поиска.
      Если установлено в NULL, то по умолчанию
      принимается значение "SearchText"
      Если установлено в пустую строку, то история вестись не будет

    ReplaceHistoryName
      Имя истории строки замены.
      Если установлено в NULL, то по умолчанию
      принимается значение "ReplaceText"
      Если установлено в пустую строку, то история вестись не будет

    *Case
      Указатель на переменную, указывающую на значение опции "Case sensitive"
      Если = NULL, то принимается значение 0 (игнорировать регистр)

    *WholeWords
      Указатель на переменную, указывающую на значение опции "Whole words"
      Если = NULL, то принимается значение 0 (в том числе в подстроке)

    *Reverse
      Указатель на переменную, указывающую на значение опции "Reverse search"
      Если = NULL, то принимается значение 0 (прямой поиск)

    *SelectFound
      Указатель на переменную, указывающую на значение опции "Select found"
      Если = NULL, то принимается значение 0 (не выделять найденное)

    *Regexp
      Указатель на переменную, указывающую на значение опции "Regular expressions"
      Если = NULL, то принимается значение 0 (не регэксп)

    *HelpTopic
      Имя темы помощи.
      Если NULL или пустая строка - тема помощи не назначается.

  Возвращаемое значение:
    TRUE  - пользователь подтвердил свои намериния
    FALSE - пользователь отказался от диалога (Esc)
*/
int WINAPI GetSearchReplaceString(
    int IsReplaceMode,
    string *pSearchStr,
    string *pReplaceStr,
    const wchar_t *TextHistoryName,
    const wchar_t *ReplaceHistoryName,
    int *Case,
    int *WholeWords,
    int *Reverse,
    int *SelectFound,
    int *Regexp,
    const wchar_t *HelpTopic=NULL);

int __stdcall GetString(
    const wchar_t *Title,
    const wchar_t *SubTitle,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    string &strDestText,
    const wchar_t *HelpTopic = NULL,
    DWORD Flags = 0,
    int *CheckBoxValue = NULL,
    const wchar_t *CheckBoxText = NULL
);

// для диалога GetNameAndPassword()
enum FlagsNameAndPassword
{
	GNP_USELAST      = 0x00000001UL, // использовать последние введенные данные
};

int WINAPI GetNameAndPassword(const wchar_t *Title,string &strUserName, string &strPassword, const wchar_t *HelpTopic,DWORD Flags);
