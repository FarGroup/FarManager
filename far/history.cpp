/*
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

#include "headers.hpp"
#pragma hdrstop

#include "history.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "ctrlobj.hpp"
#include "configdb.hpp"
#include "datetime.hpp"
#include "FarGuid.hpp"
#include "DlgGuid.hpp"
#include "scrbuf.hpp"
#include "plugins.hpp"

History::History(history_type TypeHistory, const string& HistoryName, const BoolOption& EnableSave):
	m_TypeHistory(TypeHistory),
	m_HistoryName(HistoryName),
	m_EnableSave(EnableSave),
	m_EnableAdd(true),
	m_KeepSelectedPos(false),
	m_RemoveDups(1),
	m_CurrentItem(0)
{
}

History::~History()
{
}

void History::CompactHistory()
{
	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());

	ConfigProvider().HistoryCfg()->DeleteOldUnlocked(HISTORYTYPE_CMD, L"", Global->Opt->HistoryLifetime, Global->Opt->HistoryCount);
	ConfigProvider().HistoryCfg()->DeleteOldUnlocked(HISTORYTYPE_FOLDER, L"", Global->Opt->FoldersHistoryLifetime, Global->Opt->FoldersHistoryCount);
	ConfigProvider().HistoryCfg()->DeleteOldUnlocked(HISTORYTYPE_VIEW, L"", Global->Opt->ViewHistoryLifetime, Global->Opt->ViewHistoryCount);

	DWORD index=0;
	string strName;
	while (ConfigProvider().HistoryCfg()->EnumLargeHistories(index++, Global->Opt->DialogsHistoryCount, HISTORYTYPE_DIALOG, strName))
	{
		ConfigProvider().HistoryCfg()->DeleteOldUnlocked(HISTORYTYPE_DIALOG, strName, Global->Opt->DialogsHistoryLifetime, Global->Opt->DialogsHistoryCount);
	}
}

/*
   SaveForbid - принудительно запретить запись добавляемой строки.
                Используется на панели плагина
*/
void History::AddToHistory(const string& Str, history_record_type Type, const GUID* Guid, const wchar_t *File, const wchar_t *Data, bool SaveForbid)
{
	if (!m_EnableAdd || SaveForbid)
		return;

	if (Global->CtrlObject->Macro.IsExecuting() && Global->CtrlObject->Macro.IsHistoryDisable((int)m_TypeHistory))
		return;

	if (m_TypeHistory!=HISTORYTYPE_DIALOG && (m_TypeHistory!=HISTORYTYPE_FOLDER || !Guid || *Guid == FarGuid) && Str.empty())
		return;

	bool Lock = false;
	string strName(Str),strGuid,strFile(NullToEmpty(File)),strData(NullToEmpty(Data));
	if(Guid) strGuid=GuidToStr(*Guid);

	unsigned long long DeleteId = 0;

	const bool ignore_data = m_TypeHistory == HISTORYTYPE_CMD;

	if (m_RemoveDups) // удалять дубликаты?
	{
		DWORD index=0;
		string strHName,strHGuid,strHFile,strHData;
		history_record_type HType;
		bool HLock;
		unsigned long long id;
		unsigned long long Time;
		while (HistoryCfgRef()->Enum(index++,m_TypeHistory,m_HistoryName,&id,strHName,&HType,&HLock,&Time,strHGuid,strHFile,strHData))
		{
			if (EqualType(Type,HType))
			{
				using CompareFunction = int (*)(const string&, const string&);
				CompareFunction CaseSensitive = StrCmp, CaseInsensitive = StrCmpI;
				CompareFunction CmpFunction = (m_RemoveDups == 2 ? CaseInsensitive : CaseSensitive);

				if (!CmpFunction(strName, strHName) &&
					!CmpFunction(strGuid, strHGuid) &&
					!CmpFunction(strFile, strHFile) &&
					(ignore_data || !CmpFunction(strData, strHData)))
				{
					Lock = Lock || HLock;
					DeleteId = id;
					break;
				}
			}
		}
	}

	HistoryCfgRef()->DeleteAndAddAsync(DeleteId, m_TypeHistory, m_HistoryName, strName, Type, Lock, strGuid, strFile, strData);  //Async - should never be used in a transaction

	ResetPosition();
}

bool History::ReadLastItem(const string& HistoryName, string &strStr) const
{
	strStr.clear();
	return HistoryCfgRef()->GetNewest(HISTORYTYPE_DIALOG, HistoryName, strStr);
}

history_return_type History::Select(const wchar_t *Title, const wchar_t *HelpTopic, string &strStr, history_record_type &Type, GUID* Guid, string *File, string *Data)
{
	int Height=ScrY-8;
	const auto HistoryMenu = VMenu2::create(Title, nullptr, 0, Height);
	HistoryMenu->SetMenuFlags(VMENU_WRAPMODE);

	if (HelpTopic)
		HistoryMenu->SetHelp(HelpTopic);

	HistoryMenu->SetPosition(-1,-1,0,0);

	if (m_TypeHistory == HISTORYTYPE_CMD || m_TypeHistory == HISTORYTYPE_FOLDER || m_TypeHistory == HISTORYTYPE_VIEW)
		HistoryMenu->SetId(m_TypeHistory == HISTORYTYPE_CMD?HistoryCmdId:(m_TypeHistory == HISTORYTYPE_FOLDER?HistoryFolderId:HistoryEditViewId));

	const auto ret = ProcessMenu(strStr, Guid, File, Data, Title, *HistoryMenu, Height, Type, nullptr);
	Global->ScrBuf->Flush();
	return ret;
}

history_return_type History::Select(VMenu2 &HistoryMenu, int Height, Dialog *Dlg, string &strStr)
{
	history_record_type Type;
	return ProcessMenu(strStr,nullptr ,nullptr ,nullptr , nullptr, HistoryMenu, Height, Type, Dlg);
}

history_return_type History::ProcessMenu(string &strStr, GUID* Guid, string *pstrFile, string *pstrData, const wchar_t *Title, VMenu2 &HistoryMenu, int Height, history_record_type &Type, Dialog *Dlg)
{
	unsigned long long SelectedRecord = 0;
	string strSelectedRecordName,strSelectedRecordGuid,strSelectedRecordFile,strSelectedRecordData;
	history_record_type SelectedRecordType = HR_DEFAULT;
	FarListPos Pos={sizeof(FarListPos)};
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
			DWORD index=0;
			string strHName,strHGuid,strHFile,strHData;
			history_record_type HType;
			bool HLock;
			unsigned long long id;
			unsigned long long Time;
			int LastDay=0, LastMonth = 0, LastYear = 0;

			const auto& GetTitle = [](auto Type)
			{
				switch (Type)
				{
				case HR_VIEWER:
					return MSG(lng::MHistoryView);
				case HR_EDITOR:
				case HR_EDITOR_RO:
					return MSG(lng::MHistoryEdit);
				case HR_EXTERNAL:
				case HR_EXTERNAL_WAIT:
					return MSG(lng::MHistoryExt);
				}

				return L"";
			};

			while (HistoryCfgRef()->Enum(index++,m_TypeHistory,m_HistoryName,&id,strHName,&HType,&HLock,&Time,strHGuid,strHFile,strHData,m_TypeHistory==HISTORYTYPE_DIALOG))
			{
				string strRecord;

				if (m_TypeHistory == HISTORYTYPE_VIEW)
					strRecord = GetTitle(HType) + string(L":") + (HType == HR_EDITOR_RO ? L"-" : L" ");

				else if (m_TypeHistory == HISTORYTYPE_FOLDER)
				{
					GUID HGuid;
					if(StrToGuid(strHGuid,HGuid) &&  HGuid != FarGuid)
					{
						Plugin *pPlugin = Global->CtrlObject->Plugins->FindPlugin(HGuid);
						strRecord = (pPlugin ? pPlugin->GetTitle() : L"{" + strHGuid + L"}") + L":";
						if(!strHFile.empty())
							strRecord += strHFile + L":";
					}
				}
				const auto FTTime = UI64ToFileTime(Time);
				SYSTEMTIME SavedTime;
				Utc2Local(FTTime, SavedTime);
				if(LastDay != SavedTime.wDay || LastMonth != SavedTime.wMonth || LastYear != SavedTime.wYear)
				{
					LastDay = SavedTime.wDay;
					LastMonth = SavedTime.wMonth;
					LastYear = SavedTime.wYear;
					MenuItemEx Separator;
					Separator.Flags = LIF_SEPARATOR;
					string strTime;
					ConvertDate(FTTime, Separator.strName, strTime, 5, FALSE, FALSE, TRUE);
					HistoryMenu.AddItem(Separator);
				}
				strRecord += strHName;

				if (m_TypeHistory != HISTORYTYPE_DIALOG)
					ReplaceStrings(strRecord, L"&", L"&&");

				MenuItemEx MenuItem(strRecord);
				MenuItem.SetCheck(HLock?1:0);
				MenuItem.UserData = id;

				if (!SetUpMenuPos && m_CurrentItem==id)
				{
					MenuItem.SetSelect(TRUE);
					bSelected=true;
				}

				HistoryMenu.AddItem(MenuItem);
			}

			if (!SetUpMenuPos && !bSelected && m_TypeHistory!=HISTORYTYPE_DIALOG)
			{
				FarListPos p={sizeof(FarListPos)};
				p.SelectPos = HistoryMenu.size() - 1;
				p.TopPos = 0;
				HistoryMenu.SetSelectPos(&p);
			}
		}

		if (m_TypeHistory == HISTORYTYPE_DIALOG)
		{
			int X1,Y1,X2,Y2;
			Dlg->CalcComboBoxPos(nullptr, HistoryMenu.size(), X1, Y1, X2, Y2);
			HistoryMenu.SetPosition(X1, Y1, X2, Y2);
		}
		else
			HistoryMenu.SetPosition(-1,-1,0,0);

		if (SetUpMenuPos)
		{
			Pos.SelectPos = Pos.SelectPos < static_cast<intptr_t>(HistoryMenu.size()) ? Pos.SelectPos : static_cast<intptr_t>(HistoryMenu.size() - 1);
			Pos.TopPos = std::min(Pos.TopPos, static_cast<intptr_t>(HistoryMenu.size() - Height));
			HistoryMenu.SetSelectPos(&Pos);
			SetUpMenuPos=false;
		}

		if (m_TypeHistory == HISTORYTYPE_DIALOG && !HistoryMenu.size())
			return HRT_CANCEL;

		MenuExitCode=HistoryMenu.Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			if (m_TypeHistory == HISTORYTYPE_DIALOG && Key==KEY_TAB) // Tab в списке хистори диалогов - аналог Enter
			{
				HistoryMenu.Close();
				return 1;
			}

			HistoryMenu.GetSelectPos(&Pos);
			const auto CurrentRecordPtr = HistoryMenu.GetUserDataPtr<unsigned long long>(Pos.SelectPos);
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

						DWORD index=0;
						string strHName,strHGuid,strHFile,strHData;
						history_record_type HType;
						bool HLock;
						unsigned long long id;
						unsigned long long Time;
						while (HistoryCfgRef()->Enum(index++,m_TypeHistory,m_HistoryName,&id,strHName,&HType,&HLock,&Time,strHGuid,strHFile,strHData))
						{
							if (HLock) // залоченные не трогаем
								continue;

							// убить запись из истории
							bool kill=false;
							GUID HGuid;
							if(StrToGuid(strHGuid,HGuid) && HGuid != FarGuid)
							{
								if (!Global->CtrlObject->Plugins->FindPlugin(HGuid))
									kill=true;
								else if (!strHFile.empty() && !os::fs::exists(strHFile))
									kill=true;
							}
							else if (!os::fs::exists(strHName))
								kill=true;

							if(kill)
							{
								HistoryCfgRef()->Delete(id);
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
					RetCode = (Key==KEY_CTRLALTENTER||Key==KEY_RCTRLRALTENTER||Key==KEY_CTRLRALTENTER||Key==KEY_RCTRLALTENTER||
							Key==KEY_CTRLALTNUMENTER||Key==KEY_RCTRLRALTNUMENTER||Key==KEY_CTRLRALTNUMENTER||Key==KEY_RCTRLALTNUMENTER)? HRT_CTRLALTENTER
							:((Key==KEY_CTRLSHIFTENTER||Key==KEY_RCTRLSHIFTENTER||Key==KEY_CTRLSHIFTNUMENTER||Key==KEY_RCTRLSHIFTNUMENTER)? HRT_CTRLSHIFTENTER
							:((Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER)? HRT_SHIFTETNER
							:HRT_CTRLENTER));
					break;
				}
				case KEY_F3:
				case KEY_F4:
				case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
				{
					if (m_TypeHistory != HISTORYTYPE_VIEW)
						break;

					HistoryMenu.Close(Pos.SelectPos);
					Done=true;
					RetCode=(Key==KEY_F4? HRT_F4 : HRT_F3);
					break;
				}
				// $ 09.04.2001 SVS - Фича - копирование из истории строки в Clipboard
				case KEY_CTRLC:
				case KEY_RCTRLC:
				case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
				case KEY_RCTRLINS: case KEY_RCTRLNUMPAD0:
				{
					if (CurrentRecord)
					{
						string strName;
						if (HistoryCfgRef()->Get(CurrentRecord, strName))
							SetClipboardText(strName);
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
						ResetPosition();
						HistoryMenu.Close(Pos.SelectPos);
						IsUpdate=true;
						SetUpMenuPos=true;
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
						HistoryMenu.Close(Pos.SelectPos);
						IsUpdate=true;
						SetUpMenuPos=true;
					}

					break;
				}
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (!HistoryMenu.empty() &&
					        (!Global->Opt->Confirm.HistoryClear ||
					         (Global->Opt->Confirm.HistoryClear &&
					          Message(MSG_WARNING,2,
					                  MSG((m_TypeHistory==HISTORYTYPE_CMD || m_TypeHistory==HISTORYTYPE_DIALOG? lng::MHistoryTitle:
					                       (m_TypeHistory==HISTORYTYPE_FOLDER? lng::MFolderHistoryTitle : lng::MViewHistoryTitle))),
					                  MSG(lng::MHistoryClear),
					                  MSG(lng::MClear),MSG(lng::MCancel)) == Message::first_button)))
					{
						HistoryCfgRef()->DeleteAllUnlocked(m_TypeHistory,m_HistoryName);

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

		if (IsUpdate)
			continue;

		Done=true;

		if (MenuExitCode >= 0)
		{
			SelectedRecord = *HistoryMenu.GetUserDataPtr<unsigned long long>(MenuExitCode);

			if (!SelectedRecord)
				return HRT_CANCEL;

			if (!HistoryCfgRef()->Get(SelectedRecord, strSelectedRecordName, SelectedRecordType, strSelectedRecordGuid, strSelectedRecordFile, strSelectedRecordData))
				return HRT_CANCEL;

			if (SelectedRecordType != HR_EXTERNAL && SelectedRecordType != HR_EXTERNAL_WAIT
				&& RetCode != HRT_CTRLENTER && ((m_TypeHistory == HISTORYTYPE_FOLDER && strSelectedRecordGuid.empty()) || m_TypeHistory == HISTORYTYPE_VIEW) && !os::fs::exists(strSelectedRecordName))
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
				Global->CatchError();

				if (SelectedRecordType == HR_EDITOR && m_TypeHistory == HISTORYTYPE_VIEW) // Edit? тогда спросим и если надо создадим
				{
					if (Message(MSG_WARNING|MSG_ERRORTYPE,2,Title,strSelectedRecordName.data(),MSG(lng::MViewHistoryIsCreate),MSG(lng::MHYes),MSG(lng::MHNo)) == Message::first_button)
						break;
				}
				else
				{
					Message(MSG_WARNING|MSG_ERRORTYPE,1,Title,strSelectedRecordName.data(),MSG(lng::MOk));
				}

				Done=false;
				SetUpMenuPos=true;
				continue;
			}
		}
	}

	if (MenuExitCode < 0 || !SelectedRecord)
		return HRT_CANCEL;

	if (m_KeepSelectedPos)
	{
		m_CurrentItem = SelectedRecord;
	}

	strStr = strSelectedRecordName;
	if(Guid)
	{
		if(!StrToGuid(strSelectedRecordGuid,*Guid)) *Guid = FarGuid;
	}
	if(pstrFile) *pstrFile = strSelectedRecordFile;
	if(pstrData) *pstrData = strSelectedRecordData;

	switch(RetCode)
	{
	case HRT_CANCEL:
		break;

	case HRT_ENTER:
	case HRT_SHIFTETNER:
	case HRT_CTRLENTER:
	case HRT_CTRLSHIFTENTER:
	case HRT_CTRLALTENTER:
		Type = SelectedRecordType;
		break;

	case HRT_F3:
		Type = HR_VIEWER;
		RetCode = HRT_ENTER;
		break;

	case HRT_F4:
		Type = HR_EDITOR;
		if (SelectedRecordType == HR_EDITOR_RO)
			Type = HR_EDITOR_RO;
		RetCode = HRT_ENTER;
		break;
	}
	return RetCode;
}

void History::GetPrev(string &strStr)
{
	m_CurrentItem = HistoryCfgRef()->GetPrev(m_TypeHistory, m_HistoryName, m_CurrentItem, strStr);
}


void History::GetNext(string &strStr)
{
	m_CurrentItem = HistoryCfgRef()->GetNext(m_TypeHistory, m_HistoryName, m_CurrentItem, strStr);
}


bool History::GetSimilar(string &strStr, int LastCmdPartLength, bool bAppend)
{
	int Length=(int)strStr.size();

	if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
		Length=LastCmdPartLength;

	if (LastCmdPartLength==-1)
	{
		ResetPosition();
	}

	int i=0;
	string strName;
	unsigned long long HistoryItem=HistoryCfgRef()->CyclicGetPrev(m_TypeHistory, m_HistoryName, m_CurrentItem, strName);
	while (HistoryItem != m_CurrentItem)
	{
		if (!HistoryItem)
		{
			if (++i > 1) //infinite loop
				break;
			HistoryItem = HistoryCfgRef()->CyclicGetPrev(m_TypeHistory, m_HistoryName, HistoryItem, strName);
			continue;
		}

		if (!StrCmpNI(strStr.data(),strName.data(),Length) && strStr != strName)
		{
			if (bAppend)
				strStr.append(strName, Length, string::npos);
			else
				strStr = strName;

			m_CurrentItem = HistoryItem;
			return true;
		}

		HistoryItem = HistoryCfgRef()->CyclicGetPrev(m_TypeHistory, m_HistoryName, HistoryItem, strName);
	}

	return false;
}

std::vector<std::tuple<string, unsigned long long, bool>> History::GetAllSimilar(const string& Str) const
{
	FN_RETURN_TYPE(History::GetAllSimilar) Result;
	const auto Length = Str.size();
	DWORD index=0;
	string strHName,strHGuid,strHFile,strHData;
	history_record_type HType;
	bool HLock;
	unsigned long long id;
	unsigned long long Time;
	while (HistoryCfgRef()->Enum(index++,m_TypeHistory,m_HistoryName,&id,strHName,&HType,&HLock,&Time,strHGuid,strHFile,strHData,true))
	{
		if (!StrCmpNI(Str.data(),strHName.data(),Length))
		{
			Result.emplace_back(strHName, id, HLock);
		}
	}
	return Result;
}

bool History::DeleteIfUnlocked(unsigned long long id)
{
	bool b = false;
	if (id && !HistoryCfgRef()->IsLocked(id))
	{
		if (HistoryCfgRef()->Delete(id))
		{
			b = true;
			ResetPosition();
		}
	}
	return b;
}

void History::SetAddMode(bool EnableAdd, int RemoveDups, bool KeepSelectedPos)
{
	m_EnableAdd=EnableAdd;
	m_RemoveDups=RemoveDups;
	m_KeepSelectedPos=KeepSelectedPos;
}

bool History::EqualType(history_record_type Type1, history_record_type Type2) const
{
	return Type1 == Type2 || (m_TypeHistory == HISTORYTYPE_VIEW && ((Type1 == HR_EDITOR_RO && Type2 == HR_EDITOR) || (Type1 == HR_EDITOR && Type2 == HR_EDITOR_RO)));
}

const std::unique_ptr<HistoryConfig>& History::HistoryCfgRef() const
{
	return m_EnableSave? ConfigProvider().HistoryCfg() : ConfigProvider().HistoryCfgMem();
}
