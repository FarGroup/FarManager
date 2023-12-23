/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/

#include "plugin.hpp"

#include "TmpLng.hpp"
#include "TmpCfg.hpp"
#include "TmpClass.hpp"
#include "TmpPanel.hpp"
#include "guid.hpp"

#include <algorithm.hpp>
#include <enum_tokens.hpp>
#include <scope_exit.hpp>
#include <smart_ptr.hpp>
#include <string_utils.hpp>
#include <view/zip.hpp>

constexpr auto REMOVE_FLAG = 1;

static bool IsLinksDisplayed(const string_view ColumnTypes)
{
	return contains(enum_tokens(ColumnTypes, L","), L"LN"sv);
}

static bool IsOwnersDisplayed(const string_view ColumnTypes)
{
	return contains(enum_tokens(ColumnTypes, L","), L"O"sv);
}

static const wchar_t* NullToEmpty(const wchar_t* Str)
{
	return Str? Str : L"";
}

TmpPanel::TmpPanel(const string_view HostFile):
	m_HostFile(HostFile)
{
	IfOptCommonPanel();
}

int TmpPanel::GetFindData(PluginPanelItem*& pPanelItem, size_t& pItemsNumber, const OPERATION_MODES OpMode)
{
	IfOptCommonPanel();
	string ColumnTypes(MAX_PATH, 0);
	for (;;)
	{
		const size_t Size = PsInfo.PanelControl(this, FCTL_GETCOLUMNTYPES, ColumnTypes.size(), ColumnTypes.data());
		if (!Size)
		{
			ColumnTypes.clear();
			break;
		}

		const auto CurrentSize = ColumnTypes.size();
		ColumnTypes.resize(Size - 1);
		if (Size - 1 <= CurrentSize)
			break;
	}
	UpdateItems(IsOwnersDisplayed(ColumnTypes), IsLinksDisplayed(ColumnTypes));
	pPanelItem = m_Panel->Items.data();
	pItemsNumber = m_Panel->Items.size();
	return true;
}

void TmpPanel::GetOpenPanelInfo(OpenPanelInfo& Info)
{
	Info.StructSize = sizeof(Info);
	Info.Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY;

	if (!Opt.SafeModePanel)
		Info.Flags |= OPIF_REALNAMES;

	Info.HostFile = {};

	if (!m_HostFile.empty())
	{
		if (const size_t Size = PsInfo.PanelControl(this, FCTL_GETCURRENTPANELITEM, 0, {}))
		{
			const block_ptr<PluginPanelItem> ppi(Size);
			FarGetPluginPanelItem gpi{ sizeof(gpi), Size, ppi.data() };
			PsInfo.PanelControl(this, FCTL_GETCURRENTPANELITEM, 0, &gpi);

			if (ppi->FileName == L".."sv)
				Info.HostFile = m_HostFile.c_str();
		}
	}

	Info.CurDir = L"";
	Info.Format = GetMsg(MTempPanel);

	m_Title = concat(Opt.SafeModePanel? L"(R) "sv : L""sv, GetMsg(MTempPanel));
	if (Opt.CommonPanel)
		append(m_Title, L" ["sv, std::to_wstring(SharedData->CurrentCommonPanel), L"]"sv);

	Info.PanelTitle = m_Title.c_str();

	static PanelMode PanelModesArray[10];
	const auto StartModeIndex = 4;
	auto& StartMode = PanelModesArray[StartModeIndex];

	StartMode.Flags = PMFLAGS_CASECONVERSION;
	if (Opt.FullScreenPanel)
		StartMode.Flags |= PMFLAGS_FULLSCREEN;
	StartMode.ColumnTypes = Opt.ColumnTypes.c_str();
	StartMode.ColumnWidths = Opt.ColumnWidths.c_str();
	StartMode.StatusColumnTypes = Opt.StatusColumnTypes.c_str();
	StartMode.StatusColumnWidths = Opt.StatusColumnWidths.c_str();
	Info.PanelModesArray = PanelModesArray;
	Info.PanelModesNumber = std::size(PanelModesArray);
	Info.StartPanelMode = L'0' + StartModeIndex;

	static WORD FKeys[]
	{
		VK_F7, 0, MF7,
		VK_F2, SHIFT_PRESSED | LEFT_ALT_PRESSED, MAltShiftF2,
		VK_F3, SHIFT_PRESSED | LEFT_ALT_PRESSED, MAltShiftF3,
		VK_F12, SHIFT_PRESSED | LEFT_ALT_PRESSED, MAltShiftF12,
	};

	static KeyBarLabel kbl[std::size(FKeys) / 3];
	static KeyBarTitles kbt{ std::size(kbl), kbl };

	for (size_t j = 0, i = 0; i < std::size(FKeys); i += 3, ++j)
	{
		kbl[j].Key.VirtualKeyCode = FKeys[i];
		kbl[j].Key.ControlKeyState = FKeys[i + 1];

		if (FKeys[i + 2])
		{
			kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i + 2]);
			if (!Opt.CommonPanel && kbl[j].Key.VirtualKeyCode == VK_F12 && kbl[j].Key.ControlKeyState == (SHIFT_PRESSED | LEFT_ALT_PRESSED))
				kbl[j].Text = kbl[j].LongText = L"";
		}
		else
		{
			kbl[j].Text = kbl[j].LongText = L"";
		}
	}

	Info.KeyBar = &kbt;
}


int TmpPanel::SetDirectory(const wchar_t* Dir, const OPERATION_MODES OpMode)
{
	if ((OpMode & OPM_FIND)/* || lstrcmp(Dir,L"\\")==0*/)
		return(FALSE);

	if (lstrcmp(Dir, L"\\") == 0)
		PsInfo.PanelControl(this, FCTL_CLOSEPANEL, 0, {});
	else
		PsInfo.PanelControl(this, FCTL_CLOSEPANEL, 0, const_cast<wchar_t*>(Dir));
	return(TRUE);
}


bool TmpPanel::PutFiles(const std::span<const PluginPanelItem> Files, const wchar_t* const SrcPath, const OPERATION_MODES)
{
	m_UpdateNotNeeded = false;
	const auto Screen = BeginPutFiles();

	for (const auto& i: Files)
	{
		if (!PutOneFile(SrcPath, i))
		{
			CommitPutFiles(Screen, false);
			return false;
		}
	}

	CommitPutFiles(Screen, true);
	return true;
}

HANDLE TmpPanel::BeginPutFiles()
{
	IfOptCommonPanel();
	Opt.SelectedCopyContents = Opt.CopyContents;
	const auto Screen = PsInfo.SaveScreen(0, 0, -1, -1);
	const wchar_t* MsgItems[]
	{
		GetMsg(MTempPanel),
		GetMsg(MTempSendFiles),
	};
	PsInfo.Message(&MainGuid, {}, 0, {}, MsgItems, std::size(MsgItems), 0);
	return Screen;
}

bool TmpPanel::PutDirectoryContents(const wchar_t* Path)
{
	if (Opt.SelectedCopyContents == BSTATE_3STATE)
	{
		const wchar_t* MsgItems[]{ GetMsg(MWarning), GetMsg(MCopyContensMsg) };
		Opt.SelectedCopyContents = !PsInfo.Message(&MainGuid, {}, FMSG_MB_YESNO, L"Config", MsgItems, std::size(MsgItems), 0);
	}

	if (!Opt.SelectedCopyContents)
		return true;

	PluginPanelItem* DirItems;
	size_t DirItemsNumber;

	if (!PsInfo.GetDirList(Path, &DirItems, &DirItemsNumber))
		return false;

	SCOPE_EXIT{ PsInfo.FreeDirList(DirItems, DirItemsNumber); };

	for (const auto& i: std::span(DirItems, DirItemsNumber))
	{
		auto& NewItem = m_Panel->Items.emplace_back(i);
		NewItem.FileName = m_Panel->StringData.emplace_back(NewItem.FileName).c_str();
		NewItem.Owner = m_Panel->OwnerData.emplace_back(NullToEmpty(NewItem.Owner)).c_str();
		NewItem.UserData.Data = reinterpret_cast<void*>(m_Panel->Items.size() - 1);
	}

	return true;
}

bool TmpPanel::PutOneFile(const string& SrcPath, const PluginPanelItem& PanelItem)
{
	auto& NewItem = m_Panel->Items.emplace_back(PanelItem);
	auto& NewStr = m_Panel->StringData.emplace_back(NewItem.FileName);
	if (!SrcPath.empty() && !contains(PanelItem.FileName, L'\\'))
		NewStr = concat(SrcPath, SrcPath.back() == L'\\'? L""sv : L"\\"sv, PanelItem.FileName);

	NewItem.FileName = NewStr.c_str();
	NewItem.AlternateFileName = {};
	NewItem.Owner = m_Panel->OwnerData.emplace_back(NullToEmpty(NewItem.Owner)).c_str();
	NewItem.UserData.Data = reinterpret_cast<void*>(m_Panel->Items.size() - 1);

	if (NewItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return PutDirectoryContents(NewItem.FileName);

	return true;
}

bool TmpPanel::PutOneFile(const string& FilePath)
{
	auto& NewItem = m_Panel->Items.emplace_back();
	auto& NewStr = m_Panel->StringData.emplace_back(FilePath);
	NewItem.FileName = NewStr.c_str();
	NewItem.Owner = m_Panel->OwnerData.emplace_back(NullToEmpty(NewItem.Owner)).c_str();
	NewItem.UserData.Data = reinterpret_cast<void*>(m_Panel->Items.size() - 1);

	if (GetFileInfoAndValidate(FilePath, NewItem, NewStr, Opt.AnyInPanel) && NewItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return PutDirectoryContents(NewItem.FileName);

	return true;
}

void TmpPanel::CommitPutFiles(const HANDLE RestoreScreen, const bool Success)
{
	if (Success)
		RemoveDups();

	PsInfo.RestoreScreen(RestoreScreen);
}


int TmpPanel::SetFindList(const std::span<const PluginPanelItem> Files)
{
	const auto Screen = BeginPutFiles();
	FindSearchResultsPanel();
	clear();

	for (const auto& i: Files)
	{
		auto& NewItem = m_Panel->Items.emplace_back(i);
		NewItem.FileName = m_Panel->StringData.emplace_back(NewItem.FileName).c_str();
		NewItem.Owner = m_Panel->OwnerData.emplace_back(NullToEmpty(NewItem.Owner)).c_str();
		NewItem.UserData.Data = reinterpret_cast<void*>(m_Panel->Items.size() - 1);
	}

	CommitPutFiles(Screen, true);
	m_UpdateNotNeeded = true;
	return true;
}


void TmpPanel::FindSearchResultsPanel()
{
	if (!Opt.CommonPanel)
		return;

	if (Opt.NewPanelForSearchResults)
	{
		std::optional<size_t> SearchResultsPanel;

		for (size_t i = 0; i != std::size(SharedData->CommonPanels); i++)
		{
			if (SharedData->CommonPanels[i].Items.empty())
			{
				SearchResultsPanel = i;
				break;
			}
		}

		if (!SearchResultsPanel)
		{
			// all panels are full - use least recently used panel
			SearchResultsPanel = Opt.LastSearchResultsPanel++;

			if (Opt.LastSearchResultsPanel >= std::size(SharedData->CommonPanels))
				Opt.LastSearchResultsPanel = 0;
		}

		SharedData->CurrentCommonPanel = *SearchResultsPanel;
	}

	IfOptCommonPanel();
}

void TmpPanel::RemoveDups()
{
	if (m_Panel->Items.size() < 2)
		return;

	std::vector<PluginPanelItem*> ItemPtrs;
	ItemPtrs.reserve(m_Panel->Items.size());
	std::transform(ALL_RANGE(m_Panel->Items), std::back_inserter(ItemPtrs), [](PluginPanelItem& Item) { return &Item; });
	std::sort(ALL_RANGE(ItemPtrs), [](const PluginPanelItem* a, const PluginPanelItem* b)
	{
		return string_view(a->FileName) < string_view(b->FileName);
	});

	for (auto i = ItemPtrs.begin(), end = ItemPtrs.end() - 1; i != end; ++i)
	{
		if (string_view{(*i)->FileName} == string_view{ (*(i + 1))->FileName })
			(*(i + 1))->Flags |= REMOVE_FLAG;
	}

	RemoveEmptyItems();
}

void TmpPanel::RemoveEmptyItems()
{
	size_t EmptyCount{};
	auto StrIterator = m_Panel->StringData.begin();
	auto OwnerIterator = m_Panel->OwnerData.begin();

	for (auto& i: m_Panel->Items)
	{
		if (i.Flags & REMOVE_FLAG)
		{
			++EmptyCount;
			StrIterator = m_Panel->StringData.erase(StrIterator);
			OwnerIterator = m_Panel->OwnerData.erase(OwnerIterator);
			continue;
		}

		if (EmptyCount)
		{
			i.UserData.Data = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(i.UserData.Data) - EmptyCount);
			*(&i - EmptyCount) = i;
		}

		++StrIterator;
		++OwnerIterator;
	}

	m_Panel->Items.resize(m_Panel->Items.size() - EmptyCount);
}

static bool same_name(const WIN32_FIND_DATA& wfd, const PluginPanelItem& ffd)
{
	return !lstrcmp(wfd.cFileName, FSF.PointToName(ffd.FileName));
}

void TmpPanel::UpdateItems(const bool ShowOwners, const bool ShowLinks)
{
	if (m_UpdateNotNeeded || m_Panel->Items.empty())
	{
		m_UpdateNotNeeded = false;
		return;
	}

	const auto Screen = PsInfo.SaveScreen(0, 0, -1, -1);
	const wchar_t* MsgItems[]{ GetMsg(MTempPanel), GetMsg(MTempUpdate) };
	PsInfo.Message(&MainGuid, {}, 0, {}, MsgItems, std::size(MsgItems), 0);
	m_LastOwnersRead = ShowOwners;
	m_LastLinksRead = ShowLinks;

	auto NameDataIterator = m_Panel->StringData.begin();
	for (auto CurItem = m_Panel->Items.begin(), end = m_Panel->Items.end(); CurItem != end; ++CurItem, ++NameDataIterator)
	{
		const string_view FullName(CurItem->FileName);
		const auto SlashPos = FullName.rfind(L'\\');
		const auto Dir = FullName.substr(0, SlashPos == FullName.npos? 0 : SlashPos + 1);
		size_t SameFolderItems = 1;

		/* $ 23.12.2001 DJ
		   если FullName - это каталог, то FindFirstFile (FullName+"*.*")
		   этот каталог не найдет. Поэтому для каталогов оптимизацию с
		   SameFolderItems пропускаем.
		*/
		if (!Dir.empty() && Dir.size() < FullName.size())
		{
			for (auto Next = CurItem + 1; Next != end; ++Next)
			{
				const string_view NextName = Next->FileName;
				if (NextName.starts_with(Dir) && !contains(NextName.substr(Dir.size()), L'\\'))
					SameFolderItems++;
				else
					break;
			}
		}

		// SameFolderItems - оптимизация для случая, когда в панели лежат
		// несколько файлов из одного и того же каталога. При этом
		// FindFirstFile() делается один раз на каталог, а не отдельно для
		// каждого файла.
		if (SameFolderItems > 2)
		{
			WIN32_FIND_DATA FindData;
			const auto FindFile = Dir + L"*"sv;
			const auto NtPath = FormNtPath(FindFile);

			for (auto& j: std::ranges::subrange(CurItem, CurItem + SameFolderItems))
				j.Flags |= REMOVE_FLAG;

			if (const auto FindHandle = FindFirstFile(NtPath.c_str(), &FindData); FindHandle != INVALID_HANDLE_VALUE)
			{
				SCOPE_EXIT{ FindClose(FindHandle); };

				do
				{
					for (auto& j: std::ranges::subrange(CurItem, CurItem + SameFolderItems))
					{
						if ((j.Flags & REMOVE_FLAG) && same_name(FindData, j))
						{
							j.Flags &= ~REMOVE_FLAG;
							WFD2FFD(FindData, j, {});
							break;
						}
					}
				}
				while (FindNextFile(FindHandle, &FindData));
			}

			CurItem += SameFolderItems - 1;
			std::advance(NameDataIterator, SameFolderItems - 1);
		}
		else
		{
			if (!GetFileInfoAndValidate(FullName, *CurItem, *NameDataIterator, Opt.AnyInPanel))
				CurItem->Flags |= REMOVE_FLAG;
		}
	}

	RemoveEmptyItems();

	if (ShowOwners || ShowLinks)
	{
		for (const auto& [CurItem, OwnerData]: zip(m_Panel->Items, m_Panel->OwnerData))
		{
			if (ShowOwners)
			{
				OwnerData.resize(80);
				for (;;)
				{
					const auto Size = FSF.GetFileOwner({}, CurItem.FileName, OwnerData.data(), OwnerData.size());
					if (!Size)
					{
						OwnerData.clear();
						break;
					}

					const auto CurrentSize = OwnerData.size();
					OwnerData.resize(Size - 1);
					if (Size - 1 <= CurrentSize)
						break;
				}
				CurItem.Owner = OwnerData.c_str();
			}

			if (ShowLinks)
				CurItem.NumberOfLinks = FSF.GetNumberOfLinks(CurItem.FileName);
		}
	}

	PsInfo.RestoreScreen(Screen);
}

int TmpPanel::ProcessEvent(intptr_t Event, void*)
{
	if (Event == FE_CHANGEVIEWMODE)
	{
		IfOptCommonPanel();
		const size_t Size = PsInfo.PanelControl(this, FCTL_GETCOLUMNTYPES, 0, {});
		string ColumnTypes(Size, 0);
		PsInfo.PanelControl(this, FCTL_GETCOLUMNTYPES, static_cast<int>(Size), ColumnTypes.data());
		const auto UpdateOwners = IsOwnersDisplayed(ColumnTypes) && !m_LastOwnersRead;
		const auto UpdateLinks = IsLinksDisplayed(ColumnTypes) && !m_LastLinksRead;

		if (UpdateOwners || UpdateLinks)
		{
			UpdateItems(UpdateOwners, UpdateLinks);
			PsInfo.PanelControl(this, FCTL_UPDATEPANEL, TRUE, {});
			PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
		}
	}

	return false;
}


string TmpPanel::IsCurrentFileCorrect()
{
	PanelInfo PInfo{ sizeof(PanelInfo) };
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);
	const size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
	if (!Size)
		return {};
	const block_ptr<PluginPanelItem> ppi(Size);
	FarGetPluginPanelItem gpi{ sizeof(gpi), Size, ppi.data() };
	PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);

	if (lstrcmp(ppi->FileName, L".."))
	{
		PluginPanelItem TempFindData;
		string NameData;
		if (!GetFileInfoAndValidate(ppi->FileName, TempFindData, NameData, false))
			return {};
	}

	return ppi->FileName;
}

bool TmpPanel::ProcessKey(const INPUT_RECORD* Rec)
{
	if (Rec->EventType != KEY_EVENT)
		return false;

	const auto Key = Rec->Event.KeyEvent.wVirtualKeyCode;
	const auto ControlState = Rec->Event.KeyEvent.dwControlKeyState;
	bool isAltShift = ControlState == (SHIFT_PRESSED | LEFT_ALT_PRESSED) || ControlState == (SHIFT_PRESSED | RIGHT_ALT_PRESSED);
	bool isCtrl = ControlState == LEFT_CTRL_PRESSED || ControlState == RIGHT_CTRL_PRESSED;

	if (isAltShift && Key == VK_F3)
	{
		if (const auto CurFileName = IsCurrentFileCorrect(); !CurFileName.empty())
		{
			PanelInfo PInfo{ sizeof(PanelInfo) };
			PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

			if (CurFileName != L".."sv)
			{
				const size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
				const block_ptr<PluginPanelItem> ppi(Size);

				FarGetPluginPanelItem gpi{ sizeof(gpi), Size, ppi.data() };
				PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);

				if (const auto Attributes = static_cast<DWORD>(ppi->FileAttributes); Attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					FarPanelDirectory dirInfo{ sizeof(dirInfo), CurFileName.c_str() };
					PsInfo.PanelControl(PANEL_PASSIVE, FCTL_SETPANELDIRECTORY, 0, &dirInfo);
				}
				else
				{
					GoToFile(CurFileName, true);
				}

				PsInfo.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, {});
				return true;
			}
		}
	}

	if (!isCtrl && Key >= VK_F3 && Key <= VK_F8 && Key != VK_F7)
	{
		if (IsCurrentFileCorrect().empty())
			return true;
	}

	if (ControlState == 0 && Key == VK_RETURN && Opt.AnyInPanel)
	{
		if (const auto CurFileName = IsCurrentFileCorrect(); CurFileName.empty())
		{
			PsInfo.PanelControl(this, FCTL_SETCMDLINE, 0, const_cast<wchar_t*>(CurFileName.c_str()));
			return true;
		}
	}

	if (Opt.SafeModePanel && isCtrl && Key == VK_PRIOR)
	{
		if (const auto CurFileName = IsCurrentFileCorrect(); !CurFileName.empty())
		{
			if (CurFileName == L".."sv)
				SetDirectory(L".", 0);
			else
				GoToFile(CurFileName, false);

			return true;
		}
	}

	if (ControlState == 0 && Key == VK_F7)
	{
		ProcessRemoveKey();
		return true;
	}
	else if (isAltShift && Key == VK_F2)
	{
		ProcessSaveListKey();
		return true;
	}
	else
	{
		if (Opt.CommonPanel && isAltShift)
		{
			if (Key == VK_F12)
			{
				ProcessPanelSwitchMenu();
				return true;
			}
			else if (Key >= L'0' && Key <= L'9')
			{
				SwitchToPanel(Key - L'0');
				return true;
			}
		}
	}

	return false;
}

void TmpPanel::ProcessRemoveKey()
{
	IfOptCommonPanel();
	PanelInfo PInfo{ sizeof(PanelInfo) };
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

	for (size_t i = 0; i != PInfo.SelectedItemsNumber; ++i)
	{
		const size_t Size = PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, {});
		const block_ptr<PluginPanelItem> ppi(Size);
		FarGetPluginPanelItem gpi{ sizeof(gpi), Size, ppi.data() };
		PsInfo.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, &gpi);
		m_Panel->Items[reinterpret_cast<size_t>(ppi->UserData.Data)].Flags |= REMOVE_FLAG;
	}

	RemoveEmptyItems();
	PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 0, {});
	PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
	PsInfo.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &PInfo);

	if (PInfo.PanelType == PTYPE_QVIEWPANEL)
	{
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, {});
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, {});
	}
}

void TmpPanel::ProcessSaveListKey()
{
	IfOptCommonPanel();

	if (m_Panel->Items.empty())
		return;

	// default path: opposite panel directory\panel<index>.<mask extension>
	const size_t Size = PsInfo.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIRECTORY, 0, {});
	const block_ptr<FarPanelDirectory> Dir(Size);
	Dir->StructSize = sizeof(*Dir);
	PsInfo.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIRECTORY, Size, Dir.data());
	string ListPath = Dir->Name;

	if (ListPath.back() != L'\\')
		ListPath.push_back(L'\\');

	ListPath += L"panel"sv;

	if (Opt.CommonPanel)
		ListPath += std::to_wstring(SharedData->CurrentCommonPanel);

	const auto MaskPart = string_view(Opt.Mask).substr(0, Opt.Mask.find(L','));
	if (const auto [Name, Ext] = split(MaskPart, L'.'); !Ext.empty() && Ext.find_first_of(L"*?") == Ext.npos)
		append(ListPath, L'.', Ext);

	string Buffer(NT_MAX_PATH, 0);
	if (PsInfo.InputBox(&MainGuid, &SaveListDialogGuid, GetMsg(MTempPanel), GetMsg(MListFilePath), L"TmpPanel.SaveList", ListPath.c_str(), Buffer.data(), Buffer.size(), {}, FIB_BUTTONS))
	{
		ListPath = Buffer.c_str(); // REQUIRED, we don't know the size
		SaveListFile(ListPath);
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, {});
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, {});
	}
}

void TmpPanel::SaveListFile(const string& Path)
{
	IfOptCommonPanel();

	if (m_Panel->Items.empty())
		return;

	const auto NtPath = FormNtPath(GetFullPath(Path));

	const auto File = CreateFile(NtPath.c_str(), GENERIC_WRITE, 0, {}, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, {});
	if (File == INVALID_HANDLE_VALUE)
	{
		const wchar_t* Items[]{ GetMsg(MError) };
		PsInfo.Message(&MainGuid, {}, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK, {}, Items, std::size(Items), 0);
		return;
	}
	SCOPE_EXIT{ CloseHandle(File); };

	DWORD BytesWritten;

	if (Opt.ListUTF8)
	{
		static const unsigned char bomhi = SIGN_UTF8_HI;
		static const unsigned short bomlo = SIGN_UTF8_LO;
		WriteFile(File, &bomlo, sizeof(bomlo), &BytesWritten, {});
		WriteFile(File, &bomhi, sizeof(bomhi), &BytesWritten, {});
	}
	else
	{
		static const unsigned short bom = SIGN_UNICODE;
		WriteFile(File, &bom, sizeof(bom), &BytesWritten, {});
	}

	std::string Dest;

	for (const auto& i: m_Panel->Items)
	{
		const string_view FName = i.FileName;

		if (!Opt.ListUTF8)
		{
			const auto CRLF = L"\r\n"sv;
			WriteFile(File, FName.data(), static_cast<DWORD>(FName.size() * sizeof(wchar_t)), &BytesWritten, {});
			WriteFile(File, CRLF.data(), static_cast<DWORD>(CRLF.size() * sizeof(wchar_t)), &BytesWritten, {});
		}
		else
		{
			const auto CRLF = "\r\n";
			const auto Required = WideCharToMultiByte(CP_UTF8, 0, FName.data(), static_cast<int>(FName.size()), {}, 0, {}, {});
			Dest.resize(Required);
			Dest.resize(WideCharToMultiByte(CP_UTF8, 0, FName.data(), static_cast<int>(FName.size()), Dest.data(), static_cast<int>(Dest.size()), {}, {}));
			WriteFile(File, Dest.data(), static_cast<DWORD>(Dest.size()), &BytesWritten, {});
			WriteFile(File, CRLF, 2 * sizeof(char), &BytesWritten, {});
		}
	}
}

void TmpPanel::SwitchToPanel(const size_t NewPanelIndex)
{
	if (NewPanelIndex == SharedData->CurrentCommonPanel)
		return;

	SharedData->CurrentCommonPanel = NewPanelIndex;
	PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 0, {});
	PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
}

void TmpPanel::ProcessPanelSwitchMenu()
{
	FarMenuItem fmi[ARRAYSIZE(SharedData->CommonPanels)]{};
	const string_view Txt = GetMsg(MSwitchMenuTxt);
	string StrData[ARRAYSIZE(SharedData->CommonPanels)];

	for (unsigned int i = 0; i != std::size(SharedData->CommonPanels); ++i)
	{
		auto& Str = StrData[i];
		const auto Size = std::to_wstring(SharedData->CommonPanels[i].Items.size());

		wchar_t Hotkey[5]{ L'&', 0, L'.', L' ', 0 };

		if (i < 10)
			Hotkey[1] = L'0' + i;
		else if (i < 36)
			Hotkey[1] = L'A' - 10 + i;
		else
			Hotkey[0] = Hotkey[1] = Hotkey[2] = L' ';

		Str = concat(Hotkey, Txt, L' ', Size);
		fmi[i].Text = StrData[i].c_str();
	}

	fmi[SharedData->CurrentCommonPanel].Flags |= MIF_SELECTED;

	const auto ExitCode = PsInfo.Menu(
		&MainGuid,
		{},
		-1,
		-1,
		0,
		FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE,
		GetMsg(MSwitchMenuTitle),
		{},
		{},
		{},
		{},
		fmi,
		std::size(fmi));

	if (ExitCode < 0)
		return;

	SwitchToPanel(ExitCode);
}

void TmpPanel::IfOptCommonPanel()
{
	m_Panel = Opt.CommonPanel? &SharedData->CommonPanels[SharedData->CurrentCommonPanel] : &m_LocalPanel;
}

void TmpPanel::clear()
{
	m_Panel->clear();
}
