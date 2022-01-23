﻿/*
history.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "history.hpp"

// Internal:
#include "lang.hpp"
#include "keys.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "ctrlobj.hpp"
#include "configdb.hpp"
#include "datetime.hpp"
#include "uuids.far.hpp"
#include "uuids.far.dialogs.hpp"
#include "scrbuf.hpp"
#include "plugins.hpp"
#include "string_utils.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "global.hpp"
#include "FarDlgBuilder.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	struct [[nodiscard]] time_point_hash
	{
		[[nodiscard]]
		size_t operator()(os::chrono::time_point const Time) const
		{
			return make_hash(Time.time_since_epoch().count());
		}
	};

	using known_records = std::unordered_set<os::chrono::time_point, time_point_hash>;

	class [[nodiscard]] history_white_list_t
	{
	public:
		bool empty() const
		{
			return m_KnownRecords.empty();
		}

		bool check(os::chrono::time_point const Time) const
		{
			return Time < m_StartTime || contains(m_KnownRecords, Time);
		}

		void add(os::chrono::time_point const Time)
		{
			m_KnownRecords.emplace(Time);
		}

		void remove(os::chrono::time_point const Time)
		{
			m_KnownRecords.erase(Time);
		}

		void assign(known_records&& Records)
		{
			m_KnownRecords = std::move(Records);
		}

	private:
		known_records m_KnownRecords;
		os::chrono::time_point m_StartTime{ os::chrono::nt_clock::now() };
	};

	static auto& history_white_list()
	{
		static history_white_list_t Instance;
		return Instance;
	}
}

History::History(history_type TypeHistory, string_view const HistoryName, const Bool3Option& State, bool const KeepSelectedPos):
	m_TypeHistory(TypeHistory),
	m_HistoryName(HistoryName),
	m_State(State),
	m_KeepSelectedPos(KeepSelectedPos)
{
	// Initialise with the current time
	history_white_list();
}

void History::CompactHistory()
{
	const auto& HistoryConfig = ConfigProvider().HistoryCfg();
	SCOPED_ACTION(auto)(HistoryConfig->ScopedTransaction());

	HistoryConfig->DeleteOldUnlocked(HISTORYTYPE_CMD, {}, Global->Opt->HistoryLifetime, Global->Opt->HistoryCount);
	HistoryConfig->DeleteOldUnlocked(HISTORYTYPE_FOLDER, {}, Global->Opt->FoldersHistoryLifetime, Global->Opt->FoldersHistoryCount);
	HistoryConfig->DeleteOldUnlocked(HISTORYTYPE_VIEW, {}, Global->Opt->ViewHistoryLifetime, Global->Opt->ViewHistoryCount);

	for(const auto& i: HistoryConfig->LargeHistoriesEnumerator(HISTORYTYPE_DIALOG, Global->Opt->DialogsHistoryCount))
	{
		HistoryConfig->DeleteOldUnlocked(HISTORYTYPE_DIALOG, i, Global->Opt->DialogsHistoryLifetime, Global->Opt->DialogsHistoryCount);
	}
}

void History::AddToHistory(string_view const Str, history_record_type const Type, const UUID* const Uuid, string_view const File, string_view const Data)
{
	if (m_SuppressAdd)
		return;

	if (Global->CtrlObject->Macro.IsExecuting() && Global->CtrlObject->Macro.IsHistoryDisabled(m_TypeHistory))
		return;

	if (m_TypeHistory!=HISTORYTYPE_DIALOG && (m_TypeHistory!=HISTORYTYPE_FOLDER || !Uuid || *Uuid == FarUuid) && Str.empty())
		return;

	const auto Time = os::chrono::nt_clock::now();
	bool Lock = false;
	const auto strUuid = Uuid? uuid::str(*Uuid) : L""s;

	unsigned long long DeleteId = 0;

	const bool ignore_data = m_TypeHistory == HISTORYTYPE_CMD;

	for (const auto& i: HistoryCfgRef()->Enumerator(m_TypeHistory, m_HistoryName, Str, true))
	{
		if (!EqualType(Type, i.Type))
			continue;

		if (!equal_icase(strUuid, i.Uuid))
			continue;

		if (!equal_icase(File, i.File))
			continue;

		if (!ignore_data && !equal_icase(Data, i.Data))
			continue;

		Lock = Lock || i.Lock;
		DeleteId = i.Id;

		forget_record(i.Time);

		break;
	}

	HistoryCfgRef()->DeleteAndAddAsync(DeleteId, m_TypeHistory, m_HistoryName, Str, Type, Lock, Time, strUuid, File, Data);  //Async - should never be used in a transaction

	introduce_record(Time);

	ResetPosition();
}

string History::LastItem()
{
	const auto CurrentItem = m_CurrentItem;
	ResetPosition();
	SCOPE_EXIT{ m_CurrentItem = CurrentItem; };

	return GetPrev();
}

history_return_type History::Select(string_view const Title, string_view const HelpTopic, string &strStr, history_record_type &Type, UUID* Uuid, string *File, string *Data)
{
	const auto Height = ScrY - 8;
	const auto HistoryMenu = VMenu2::create(string(Title), {}, Height);
	HistoryMenu->SetMenuFlags(VMENU_WRAPMODE);
	HistoryMenu->SetHelp(HelpTopic);

	HistoryMenu->SetPosition({ -1, -1, 0, 0 });

	if (m_TypeHistory == HISTORYTYPE_CMD || m_TypeHistory == HISTORYTYPE_FOLDER || m_TypeHistory == HISTORYTYPE_VIEW)
		HistoryMenu->SetId(m_TypeHistory == HISTORYTYPE_CMD?HistoryCmdId:(m_TypeHistory == HISTORYTYPE_FOLDER?HistoryFolderId:HistoryEditViewId));

	const auto ret = ProcessMenu(strStr, Uuid, File, Data, Title, *HistoryMenu, Height, Type, nullptr);
	Global->ScrBuf->Flush();
	return ret;
}

history_return_type History::Select(VMenu2& HistoryMenu, int Height, Dialog const* const Dlg, string& strStr)
{
	history_record_type Type;
	return ProcessMenu(strStr, {}, {}, {}, {}, HistoryMenu, Height, Type, Dlg);
}

history_return_type History::ProcessMenu(string& strStr, UUID* const Uuid, string* const File, string* const Data, string_view const Title, VMenu2& HistoryMenu, int const Height, history_record_type& Type, const Dialog* const Dlg)
{
	struct
	{
		unsigned long long id = 0;
		string name, uuid, file, data;
		history_record_type type = HR_DEFAULT;
	}
	SelectedRecord;

	FarListPos Pos{ sizeof(Pos) };
	int MenuExitCode=-1;
	history_return_type RetCode = HRT_ENTER;
	bool Done=false;
	bool SetUpMenuPos=false;

	if (m_TypeHistory == HISTORYTYPE_DIALOG && !HistoryCfgRef()->Count(m_TypeHistory,m_HistoryName))
		return HRT_CANCEL;

	while (!Done)
	{
		bool IsUpdate=false;
		HistoryMenu.clear();
		{
			bool bSelected=false;
			int LastDay=0, LastMonth = 0, LastYear = 0;

			const auto GetTitle = [](auto RecordType)
			{
				switch (RecordType)
				{
				case HR_VIEWER:
					return msg(lng::MHistoryView);
				case HR_EDITOR:
				case HR_EDITOR_RO:
					return msg(lng::MHistoryEdit);
				case HR_EXTERNAL:
				case HR_EXTERNAL_WAIT:
					return msg(lng::MHistoryExt);
				}

				return L""s;
			};

			for (auto& i: HistoryCfgRef()->Enumerator(m_TypeHistory, m_HistoryName, {}, m_TypeHistory == HISTORYTYPE_DIALOG))
			{
				if (!is_known_record(i.Time))
					continue;

				string strRecord;

				if (m_TypeHistory == HISTORYTYPE_VIEW)
					strRecord = concat(GetTitle(i.Type), L':', i.Type == HR_EDITOR_RO? L'-' : L' ');

				else if (m_TypeHistory == HISTORYTYPE_FOLDER)
				{
					if(const auto UuidOpt = uuid::try_parse(i.Uuid); UuidOpt && *UuidOpt != FarUuid)
					{
						const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(*UuidOpt);
						strRecord = (pPlugin ? pPlugin->Title() : L'{' + i.Uuid + L'}') + L':';
						if(!i.File.empty())
							strRecord += i.File + L':';
					}
				}

				SYSTEMTIME SavedTime;
				utc_to_local(i.Time, SavedTime);
				if(LastDay != SavedTime.wDay || LastMonth != SavedTime.wMonth || LastYear != SavedTime.wYear)
				{
					LastDay = SavedTime.wDay;
					LastMonth = SavedTime.wMonth;
					LastYear = SavedTime.wYear;
					MenuItemEx Separator;
					Separator.Flags = LIF_SEPARATOR;
					string Time;
					std::tie(Separator.Name, Time) = ConvertDate(i.Time, 8, 1);
					HistoryMenu.AddItem(Separator);
				}
				strRecord += i.Name;

				if (m_TypeHistory != HISTORYTYPE_DIALOG)
					inplace::escape_ampersands(strRecord);

				MenuItemEx MenuItem(strRecord);
				i.Lock? MenuItem.SetCheck() : MenuItem.ClearCheck();
				MenuItem.ComplexUserData = i.Id;

				if (!SetUpMenuPos && m_CurrentItem == i.Id)
				{
					MenuItem.SetSelect(true);
					bSelected=true;
				}

				HistoryMenu.AddItem(MenuItem);
			}

			if (!SetUpMenuPos && !bSelected && m_TypeHistory!=HISTORYTYPE_DIALOG && !HistoryMenu.empty())
			{
				FarListPos p{ sizeof(p), static_cast<intptr_t>(HistoryMenu.size() - 1) };
				HistoryMenu.SetSelectPos(&p);
			}
		}

		if (m_TypeHistory == HISTORYTYPE_DIALOG)
		{
			if (!HistoryMenu.size())
				return HRT_CANCEL;

			HistoryMenu.SetPosition(Dlg->CalcComboBoxPos(nullptr, HistoryMenu.size()));
		}
		else
			HistoryMenu.SetPosition({ -1, -1, 0, 0 });

		if (SetUpMenuPos)
		{
			Pos.SelectPos = Pos.SelectPos < static_cast<intptr_t>(HistoryMenu.size()) ? Pos.SelectPos : static_cast<intptr_t>(HistoryMenu.size() - 1);
			Pos.TopPos = std::min(Pos.TopPos, static_cast<intptr_t>(HistoryMenu.size() - Height));
			HistoryMenu.SetSelectPos(&Pos);
			SetUpMenuPos=false;
		}

		HistoryMenu.Redraw();

		MenuExitCode=HistoryMenu.Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			if (m_TypeHistory == HISTORYTYPE_DIALOG && Key==KEY_TAB) // Tab в списке хистори диалогов - аналог Enter
			{
				HistoryMenu.Close();
				return 1;
			}

			HistoryMenu.GetSelectPos(&Pos);
			const auto CurrentRecordPtr = HistoryMenu.GetComplexUserDataPtr<unsigned long long>(Pos.SelectPos);
			const auto CurrentRecord = CurrentRecordPtr? *CurrentRecordPtr : 0;
			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_CTRLR: // обновить с удалением недоступных
				case KEY_RCTRLR:
				{
					if (m_TypeHistory == HISTORYTYPE_FOLDER || m_TypeHistory == HISTORYTYPE_VIEW)
					{
						bool ModifiedHistory=false;

						SCOPED_ACTION(auto) = HistoryCfgRef()->ScopedTransaction();

						for (const auto& i: HistoryCfgRef()->Enumerator(m_TypeHistory, m_HistoryName))
						{
							if (i.Lock) // залоченные не трогаем
								continue;

							if (!is_known_record(i.Time))
								continue;

							// убить запись из истории
							bool kill=false;
							if(const auto UuidOpt = uuid::try_parse(i.Uuid); UuidOpt && *UuidOpt != FarUuid)
							{
								if (!Global->CtrlObject->Plugins->FindPlugin(*UuidOpt) || (!i.File.empty() && !os::fs::exists(i.File)))
									kill = true;
							}
							else if (!os::fs::exists(i.Name))
								kill=true;

							if(kill)
							{
								HistoryCfgRef()->Delete(i.Id);
								forget_record(i.Time);
								ModifiedHistory=true;
							}
						}

						if (ModifiedHistory) // избавляемся от лишних телодвижений
						{
							IsUpdate=true;
							HistoryMenu.Close(Pos.SelectPos);
						}

						ResetPosition();
					}

					break;
				}
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_RCTRLSHIFTNUMENTER:
				case KEY_CTRLNUMENTER:
				case KEY_RCTRLNUMENTER:
				case KEY_SHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_RCTRLSHIFTENTER:
				case KEY_CTRLENTER:
				case KEY_RCTRLENTER:
				case KEY_SHIFTENTER:
				case KEY_CTRLALTENTER:
				case KEY_RCTRLRALTENTER:
				case KEY_CTRLRALTENTER:
				case KEY_RCTRLALTENTER:
				case KEY_CTRLALTNUMENTER:
				case KEY_RCTRLRALTNUMENTER:
				case KEY_CTRLRALTNUMENTER:
				case KEY_RCTRLALTNUMENTER:
				{
					if (m_TypeHistory == HISTORYTYPE_DIALOG)
						break;

					HistoryMenu.Close(Pos.SelectPos);
					Done=true;
					RetCode =
						flags::check_any(Key, KEY_CTRL | KEY_RCTRL) && flags::check_any(Key, KEY_ALT | KEY_RALT)?
							HRT_CTRLALTENTER :
							flags::check_any(Key, KEY_CTRL | KEY_RCTRL) && flags::check_any(Key, KEY_SHIFT)?
								HRT_CTRLSHIFTENTER :
								flags::check_any(Key, KEY_SHIFT)?
									HRT_SHIFTETNER :
									HRT_CTRLENTER;
					break;
				}
				case KEY_F3:
				case KEY_F4:
				case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
					switch (m_TypeHistory)
					{
					case HISTORYTYPE_VIEW:
						HistoryMenu.Close(Pos.SelectPos);
						Done = true;
						RetCode = (Key == KEY_F4? HRT_F4 : HRT_F3);
						break;

					case HISTORYTYPE_CMD:
						if (Key == KEY_F3 && CurrentRecord)
						{
							string HistoryData;

							if (HistoryCfgRef()->Get(CurrentRecord, {}, {}, {}, {}, {}, &HistoryData))
							{
								DialogBuilder Builder(lng::MHistoryInfoTitle);
								Builder.AddText(lng::MHistoryInfoFolder);
								Builder.AddConstEditField(HistoryData, std::max(20, std::min(static_cast<int>(HistoryData.size()), ScrX - 5 * 2)));
								Builder.AddOK();
								Builder.ShowDialog();
							}
						}
						break;

					default:
						break;
					}
					break;

				// $ 09.04.2001 SVS - Фича - копирование из истории строки в Clipboard
				case KEY_CTRLC:
				case KEY_RCTRLC:
				case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
				case KEY_RCTRLINS: case KEY_RCTRLNUMPAD0:
				{
					if (CurrentRecord)
					{
						string Name;
						if (HistoryCfgRef()->Get(CurrentRecord, &Name, {}, {}, {}, {}, {}))
							SetClipboardText(Name);
					}

					break;
				}
				// Lock/Unlock
				case KEY_INS:
				case KEY_NUMPAD0:
				{
					if (CurrentRecord)
					{
						HistoryCfgRef()->FlipLock(CurrentRecord);

						HistoryMenu.FlipCheck(Pos.SelectPos);
					}

					break;
				}
				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDEL:
				{
					if (CurrentRecord && !HistoryCfgRef()->IsLocked(CurrentRecord))
					{
						HistoryCfgRef()->Delete(CurrentRecord);
						ResetPosition();

						if (os::chrono::time_point Time; HistoryCfgRef()->Get(CurrentRecord, {}, {}, &Time, {}, {}, {}))
							forget_record(Time);

						HistoryMenu.DeleteItem(Pos.SelectPos);
					}

					break;
				}
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (!HistoryMenu.empty() && (!Global->Opt->Confirm.HistoryClear ||
						Message(MSG_WARNING,
							msg((m_TypeHistory==HISTORYTYPE_CMD || m_TypeHistory==HISTORYTYPE_DIALOG? lng::MHistoryTitle: (m_TypeHistory==HISTORYTYPE_FOLDER? lng::MFolderHistoryTitle : lng::MViewHistoryTitle))),
							{
								msg(lng::MHistoryClear)
							},
							{ lng::MClear, lng::MCancel }) == message_result::first_button))
					{
						HistoryCfgRef()->DeleteAllUnlocked(m_TypeHistory,m_HistoryName);

						refresh_known_records();

						ResetPosition();
						HistoryMenu.Close(Pos.SelectPos);
						IsUpdate=true;
					}

					break;
				}

				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		HistoryMenu.ClearDone();
		if (IsUpdate)
			continue;

		Done=true;

		if (MenuExitCode >= 0)
		{
			SelectedRecord.id = *HistoryMenu.GetComplexUserDataPtr<unsigned long long>(MenuExitCode);

			if (!SelectedRecord.id)
				return HRT_CANCEL;

			if (!HistoryCfgRef()->Get(SelectedRecord.id, &SelectedRecord.name, &SelectedRecord.type, {}, &SelectedRecord.uuid, &SelectedRecord.file, &SelectedRecord.data))
				return HRT_CANCEL;

			if (SelectedRecord.type != HR_EXTERNAL && SelectedRecord.type != HR_EXTERNAL_WAIT
				&& RetCode != HRT_CTRLENTER && ((m_TypeHistory == HISTORYTYPE_FOLDER && SelectedRecord.uuid.empty()) || m_TypeHistory == HISTORYTYPE_VIEW) && !os::fs::exists(SelectedRecord.name))
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
				const auto ErrorState = last_error();

				if (SelectedRecord.type == HR_EDITOR && m_TypeHistory == HISTORYTYPE_VIEW) // Edit? тогда спросим и если надо создадим
				{
					if (Message(MSG_WARNING, ErrorState,
						Title,
						{
							SelectedRecord.name,
							msg(lng::MViewHistoryIsCreate)
						},
						{ lng::MHYes, lng::MHNo }) == message_result::first_button)
						break;
				}
				else
				{
					Message(MSG_WARNING, ErrorState,
						Title,
						{
							SelectedRecord.name
						},
						{ lng::MOk });
				}

				Done=false;
				SetUpMenuPos=true;
				continue;
			}
		}
	}

	if (MenuExitCode < 0 || !SelectedRecord.id)
		return HRT_CANCEL;

	if (m_KeepSelectedPos)
	{
		m_CurrentItem = SelectedRecord.id;
	}

	strStr = SelectedRecord.name;

	if(Uuid)
	{
		if (const auto UuidOpt = uuid::try_parse(SelectedRecord.uuid))
			*Uuid = *UuidOpt;
		else
			*Uuid = FarUuid;
	}

	if(File)
		*File = SelectedRecord.file;

	if(Data)
		*Data = SelectedRecord.data;

	switch(RetCode)
	{
	case HRT_CANCEL:
		break;

	case HRT_ENTER:
	case HRT_SHIFTETNER:
	case HRT_CTRLENTER:
	case HRT_CTRLSHIFTENTER:
	case HRT_CTRLALTENTER:
		Type = SelectedRecord.type;
		break;

	case HRT_F3:
		Type = HR_VIEWER;
		RetCode = HRT_ENTER;
		break;

	case HRT_F4:
		Type = HR_EDITOR;
		if (SelectedRecord.type == HR_EDITOR_RO)
			Type = HR_EDITOR_RO;
		RetCode = HRT_ENTER;
		break;
	}

	return RetCode;
}

string History::GetPrev()
{
	string Result;
	os::chrono::time_point Time;

	for (;;)
	{
		const auto NewItem = HistoryCfgRef()->GetPrev(m_TypeHistory, m_HistoryName, m_CurrentItem, Result, Time);
		if (NewItem == m_CurrentItem)
			break;

		m_CurrentItem = NewItem;

		if (is_known_record(Time))
			break;
	}

	return Result;
}


string History::GetNext()
{
	string Result;
	os::chrono::time_point Time;

	for (;;)
	{
		const auto NewItem = HistoryCfgRef()->GetNext(m_TypeHistory, m_HistoryName, m_CurrentItem, Result, Time);
		if (NewItem == m_CurrentItem)
			break;

		m_CurrentItem = NewItem;

		if (is_known_record(Time))
			break;
	}

	return Result;
}


bool History::GetSimilar(string &strStr, int LastCmdPartLength, bool bAppend)
{
	auto Length=static_cast<int>(strStr.size());

	if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
		Length=LastCmdPartLength;

	if (LastCmdPartLength==-1)
	{
		ResetPosition();
	}

	string strName;
	os::chrono::time_point Time;

	auto CurrentItem = m_CurrentItem;
	auto FirstPreviousItem = CurrentItem;

	for (;;)
	{
		const auto NewItem = HistoryCfgRef()->CyclicGetPrev(m_TypeHistory, m_HistoryName, CurrentItem, strName, Time);
		// Empty history
		if (NewItem == CurrentItem)
			break;

		// Full circle
		if (NewItem == FirstPreviousItem)
			break;

		if (CurrentItem == m_CurrentItem)
			FirstPreviousItem = NewItem;

		CurrentItem = NewItem;

		// Empty item
		if (!NewItem)
			continue;

		if (!is_known_record(Time) || !starts_with_icase(strName, string_view(strStr).substr(0, Length)) || strStr == strName)
			continue;

		if (bAppend)
			strStr.append(strName, Length, string::npos); // gcc 7.3-8.1 bug: npos required. TODO: Remove after we move to 8.2 or later
		else
			strStr = strName;

		m_CurrentItem = CurrentItem;
		return true;
	}

	return false;
}

void History::GetAllSimilar(string_view const Str, function_ref<void(string_view Name, unsigned long long Id, bool IsLocked)> const Callback) const
{
	for (const auto& i: HistoryCfgRef()->Enumerator(m_TypeHistory, m_HistoryName, {}, true))
	{
		if (is_known_record(i.Time) && starts_with_icase(i.Name, Str))
		{
			Callback(i.Name, i.Id, i.Lock);
		}
	}
}

bool History::DeleteIfUnlocked(unsigned long long id)
{
	if (HistoryCfgRef()->IsLocked(id))
		return false;

	HistoryCfgRef()->Delete(id);

	if (os::chrono::time_point Time; HistoryCfgRef()->Get(id, {}, {}, &Time, {}, {}, {}))
		forget_record(Time);

	ResetPosition();
	return true;
}

bool History::EqualType(history_record_type Type1, history_record_type Type2) const
{
	return Type1 == Type2 || (m_TypeHistory == HISTORYTYPE_VIEW && ((Type1 == HR_EDITOR_RO && Type2 == HR_EDITOR) || (Type1 == HR_EDITOR && Type2 == HR_EDITOR_RO)));
}

const std::unique_ptr<HistoryConfig>& History::HistoryCfgRef() const
{
	return m_State == BSTATE_UNCHECKED? ConfigProvider().HistoryCfgMem() : ConfigProvider().HistoryCfg();
}

void History::introduce_record(os::chrono::time_point Time) const
{
	if (m_State == BSTATE_3STATE)
		history_white_list().add(Time);
}

void History::forget_record(os::chrono::time_point Time) const
{
	if (m_State == BSTATE_3STATE)
		history_white_list().remove(Time);
}

bool History::is_known_record(os::chrono::time_point const Time) const
{
	return m_State != BSTATE_3STATE || history_white_list().check(Time);
}

void History::refresh_known_records() const
{
	if (m_State != BSTATE_3STATE)
		return;

	if (history_white_list().empty())
		return;

	known_records NewKnownRecords;
	for (const auto& i: HistoryCfgRef()->Enumerator(m_TypeHistory, m_HistoryName))
	{
		if (is_known_record(i.Time))
		{
			NewKnownRecords.emplace(i.Time);
		}
	}

	history_white_list().assign(std::move(NewKnownRecords));
}

void History::suppress_add()
{
	++m_SuppressAdd;
}

void History::restore_add()
{
	--m_SuppressAdd;
}
