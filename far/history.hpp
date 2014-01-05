#pragma once

/*
history.hpp

Èñòîðèÿ (Alt-F8, Alt-F11, Alt-F12)
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

#include "config.hpp"
#include "vmenu2.hpp"

class Dialog;
class VMenu2;

enum history_type
{
	HISTORYTYPE_CMD,
	HISTORYTYPE_FOLDER,
	HISTORYTYPE_VIEW,
	HISTORYTYPE_DIALOG
};

ENUM(history_record_type)
{
	HR_DEFAULT,
	HR_VIEWER = HR_DEFAULT,
	HR_EDITOR,
	HR_EXTERNAL,
	HR_EXTERNAL_WAIT,
	HR_EDITOR_RO,
};

ENUM(history_return_type)
{
	HRT_CANCEL,
	HRT_ENTER,
	HRT_SHIFTETNER,
	HRT_CTRLENTER,
	HRT_F3, //internal
	HRT_F4, //internal
	HRT_CTRLSHIFTENTER,
	HRT_CTRLALTENTER,
};

class History
{
public:
	History(history_type TypeHistory, const string& HistoryName, const BoolOption& EnableSave);
	~History();

	void AddToHistory(const string& Str, history_record_type Type = HR_DEFAULT, const GUID* Guid=nullptr, const wchar_t *File=nullptr, const wchar_t *Data=nullptr, bool SaveForbid=false);
	history_return_type Select(const wchar_t *Title, const wchar_t *HelpTopic, string &strStr, history_record_type &Type, GUID* Guid=nullptr, string *File=nullptr, string *Data=nullptr);
	history_return_type Select(VMenu2 &HistoryMenu, int Height, Dialog *Dlg, string &strStr);
	void GetPrev(string &strStr);
	void GetNext(string &strStr);
	bool GetSimilar(string &strStr, int LastCmdPartLength, bool bAppend=false);
	bool GetAllSimilar(VMenu2 &HistoryMenu,const string& Str);
	void SetAddMode(bool EnableAdd, int RemoveDups, bool KeepSelectedPos);
	void ResetPosition() { CurrentItem = 0; }
	bool DeleteIfUnlocked(unsigned __int64 id);
	bool ReadLastItem(const string& HistoryName, string &strStr);

	static void CompactHistory();

private:
	bool EqualType(history_record_type Type1, history_record_type Type2) const;
	history_return_type ProcessMenu(string &strStr, GUID* Guid, string *File, string *Data, const wchar_t *Title, VMenu2 &HistoryMenu, int Height, history_record_type& Type, Dialog *Dlg);
	const std::unique_ptr<HistoryConfig>& HistoryCfgRef() const;

	history_type TypeHistory;
	string strHistoryName;
	const BoolOption& EnableSave;
	bool EnableAdd;
	bool KeepSelectedPos;
	int RemoveDups;
	unsigned __int64 CurrentItem;
};
