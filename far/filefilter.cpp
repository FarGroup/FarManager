/*
filefilter.cpp

Файловый фильтр
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
#include "filefilter.hpp"

// Internal:
#include "filefilterparams.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "mix.hpp"
#include "configdb.hpp"
#include "keyboard.hpp"
#include "uuids.far.dialogs.hpp"
#include "lang.hpp"
#include "string_sort.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/bytes_view.hpp"
#include "common/scope_exit.hpp"
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#define STR_INIT(x) x = WSTRVIEW(x)

namespace names
{
	static const string_view
		STR_INIT(Filters),
		STR_INIT(FoldersFilter),
		STR_INIT(Filter),
		STR_INIT(Flags),
		STR_INIT(Title),
		STR_INIT(UseAttr),
		STR_INIT(AttrSet),
		STR_INIT(AttrClear),
		STR_INIT(UseMask),
		STR_INIT(Mask),
		STR_INIT(UseDate),
		STR_INIT(DateType),
		STR_INIT(DateTimeAfter),
		STR_INIT(DateTimeBefore),
		STR_INIT(DateRelative),
		STR_INIT(UseSize),
		STR_INIT(SizeAboveS),
		STR_INIT(SizeBelowS),
		STR_INIT(UseHardLinks),
		STR_INIT(HardLinksAbove),
		STR_INIT(HardLinksBelow);
}

// Old format
// TODO 2020 Q4: remove
namespace legacy_names
{
	static const string_view
		STR_INIT(FFlags),
		STR_INIT(FoldersFilterFFlags),
		STR_INIT(IgnoreMask),
		STR_INIT(DateAfter),
		STR_INIT(DateBefore),
		STR_INIT(RelativeDate),
		STR_INIT(IncludeAttributes),
		STR_INIT(ExcludeAttributes);

}

#undef STR_INIT

static const string_view FilterFlagNames[]
{
	L"LeftPanel"sv,
	L"RightPanel"sv,
	L"FindFile"sv,
	L"Copy"sv,
	L"Select"sv,
	L"Custom"sv,
};

static_assert(std::size(FilterFlagNames) == FFFT_COUNT);

static auto& FilterData()
{
	static std::vector<FileFilterParams> sFilterData;
	return sFilterData;
}

static auto& TempFilterData()
{
	static std::unordered_map<string, FileFilterParams, hash_icase_t, equal_icase_t> sTempFilterData;
	return sTempFilterData;
}

static FileFilterParams *FoldersFilter;

static bool Changed = false;

FileFilter::FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType):
	m_HostPanel(HostPanel),
	m_FilterType(FilterType)
{
	UpdateCurrentTime();
}

static void ParseAndAddMasks(std::map<string, int, string_sort::less_t>& Extensions, string_view const FileName, os::fs::attributes const FileAttr, int const Check)
{
	if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) || IsParentDirectory(FileName))
		return;

	const auto Ext = name_ext(FileName).second;
	Extensions.emplace(Ext.empty()? L"*."s : concat(L'*', Ext), Check);
}

void FileFilter::FilterEdit()
{
	static bool bMenuOpen = false;
	if (bMenuOpen)
		return;
	bMenuOpen = true;
	SCOPE_EXIT{ bMenuOpen = false; };

	bool NeedUpdate = false;

	const auto FilterList = VMenu2::create(msg(lng::MFilterTitle), {}, ScrY - 6);
	FilterList->SetHelp(L"FiltersMenu"sv);
	FilterList->SetPosition({ -1, -1, 0, 0 });
	FilterList->SetBottomTitle(KeysToLocalizedText(L'+', L'-', KEY_SPACE, L'I', L'X', KEY_BS, KEY_SHIFTBS, KEY_INS, KEY_DEL, KEY_F4, KEY_F5, KEY_CTRLUP, KEY_CTRLDOWN));
	FilterList->SetMenuFlags(VMENU_WRAPMODE);
	FilterList->SetId(FiltersMenuId);

	for (auto& i: FilterData())
	{
		MenuItemEx ListItem(MenuString(&i));
		if (const auto Check = GetCheck(i))
			ListItem.SetCustomCheck(Check);
		FilterList->AddItem(ListItem);
	}

	if (m_FilterType != FFT_CUSTOM)
	{
		std::map<string, int, string_sort::less_t> Extensions;

		{
			const auto FFFT = GetFFFT();

			for (const auto& [Key, CurFilterData]: TempFilterData())
			{
				//AY: Будем показывать только те выбранные авто фильтры
				//(для которых нету файлов на панели) которые выбраны в области данного меню
				if (CurFilterData.GetFlags(FFFT))
				{
					ParseAndAddMasks(Extensions, unquote(CurFilterData.GetMask()), 0, GetCheck(CurFilterData));
				}
			}
		}

		{
			MenuItemEx ListItem;
			ListItem.Flags = LIF_SEPARATOR;
			FilterList->AddItem(ListItem);
		}

		{
			FoldersFilter->SetTitle(msg(lng::MFolderFileType));
			MenuItemEx ListItem(MenuString(FoldersFilter,false,L'0'));
			const auto Check = GetCheck(*FoldersFilter);

			if (Check)
				ListItem.SetCustomCheck(Check);

			FilterList->AddItem(ListItem);
		}

		{
			string strFileName;
			os::fs::attributes FileAttr;

			for (int i = 0; m_HostPanel->GetFileName(strFileName, i, FileAttr); i++)
				ParseAndAddMasks(Extensions, strFileName, FileAttr, 0);

			if (const auto* FilteredExtensions = m_HostPanel->GetFilteredExtensions())
			{
				for (const auto& i: *FilteredExtensions)
				{
					ParseAndAddMasks(Extensions, i, 0, 0);
				}
			}
		}

		wchar_t h = L'1';
		for (const auto& [Ext, Mark]: Extensions)
		{
			MenuItemEx ListItem(MenuString(nullptr, false, h, true, Ext, msg(lng::MPanelFileType)));
			Mark? ListItem.SetCustomCheck(Mark) : ListItem.ClearCheck();
			ListItem.ComplexUserData = Ext;
			FilterList->AddItem(ListItem);

			h == L'9' ? h = L'A' : ((h == L'Z' || !h)? h = 0 : ++h);
		}
	}

	FilterList->RunEx([&](int Msg, void *param)
	{
		if (Msg==DN_LISTHOTKEY)
			return 1;
		if (Msg!=DN_INPUT)
			return 0;

		auto Key = InputRecordToKey(static_cast<INPUT_RECORD*>(param));

		if (Key==KEY_ADD)
			Key=L'+';
		else if (Key==KEY_SUBTRACT)
			Key=L'-';
		else if (Key==L'i')
			Key=L'I';
		else if (Key==L'x')
			Key=L'X';

		int KeyProcessed = 1;

		switch (Key)
		{
			case L'+':
			case L'-':
			case L'I':
			case L'X':
			case KEY_SPACE:
			case KEY_BS:
			{
				const auto SelPos = FilterList->GetSelectPos();

				if (SelPos<0)
					break;

				const auto Check = FilterList->GetCheck(SelPos);
				int NewCheck;

				if (Key==KEY_BS)
					NewCheck = 0;
				else if (Key==KEY_SPACE)
					NewCheck = Check ? 0 : L'+';
				else
					NewCheck = (Check == Key) ? 0 : Key;

				NewCheck? FilterList->SetCustomCheck(NewCheck, SelPos) : FilterList->ClearCheck();
				FilterList->SetSelectPos(SelPos,1);
				FilterList->Key(KEY_DOWN);
				NeedUpdate = true;
				return 1;
			}
			case KEY_SHIFTBS:
			{
				for (size_t i = 0, size = FilterList->size(); i != size; ++i)
				{
					FilterList->ClearCheck(static_cast<int>(i));
				}
				NeedUpdate = true;
				break;
			}
			case KEY_F4:
			{
				const auto SelPos = FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (static_cast<size_t>(SelPos) < FilterData().size())
				{
					if (FileFilterConfig(&FilterData()[SelPos]))
					{
						MenuItemEx ListItem(MenuString(&FilterData()[SelPos]));

						if (const auto Check = FilterList->GetCheck(SelPos))
							ListItem.SetCustomCheck(Check);

						FilterList->DeleteItem(SelPos);
						FilterList->AddItem(ListItem,SelPos);
						FilterList->SetSelectPos(SelPos,1);
						NeedUpdate = true;
					}
				}
				else
				{
					Message(MSG_WARNING,
						msg(lng::MFilterTitle),
						{
							msg(lng::MCanEditCustomFilterOnly)
						},
						{ lng::MOk });
				}

				break;
			}
			case KEY_NUMPAD0:
			case KEY_INS:
			case KEY_F5:
			{
				int pos=FilterList->GetSelectPos();
				if (pos<0)
				{
					if (Key==KEY_F5)
						break;
					pos=0;
				}

				FileFilterParams NewFilter;

				if (Key==KEY_F5)
				{
					const size_t ListPos = pos;
					if (ListPos < FilterData().size())
					{
						NewFilter = FilterData()[ListPos].Clone();
						NewFilter.SetTitle({});
						NewFilter.ClearAllFlags();
					}
					else if (ListPos == FilterData().size() + 1)
					{
						NewFilter = FoldersFilter->Clone();
						NewFilter.SetTitle({});
						NewFilter.ClearAllFlags();
					}
					else if (ListPos > FilterData().size() + 1)
					{
						NewFilter.SetMask(true, *FilterList->GetComplexUserDataPtr<string>(ListPos));
						//Авто фильтры они только для файлов, папки не должны к ним подходить
						NewFilter.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);
					}
					else
					{
						break;
					}
				}
				else
				{
					//AY: Раз создаём новый фильтр то думаю будет логично если он будет только для файлов
					NewFilter.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);
				}

				if (FileFilterConfig(&NewFilter))
				{
					const auto NewPos = std::min(FilterData().size(), static_cast<size_t>(pos));
					const auto FilterIterator = FilterData().emplace(FilterData().begin() + NewPos, std::move(NewFilter));
					FilterList->AddItem(MenuItemEx(MenuString(&*FilterIterator)), static_cast<int>(NewPos));
					FilterList->SetSelectPos(static_cast<int>(NewPos),1);
					NeedUpdate = true;
				}
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				const auto SelPos = FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (static_cast<size_t>(SelPos) < FilterData().size())
				{
					if (Message(0,
						msg(lng::MFilterTitle),
						{
							msg(lng::MAskDeleteFilter),
							quote_unconditional(FilterData()[SelPos].GetTitle())
						},
						{ lng::MDelete, lng::MCancel }) == Message::first_button)
					{
						FilterData().erase(FilterData().begin() + SelPos);
						FilterList->DeleteItem(SelPos);
						FilterList->SetSelectPos(SelPos,1);
						NeedUpdate = true;
					}
				}
				else
				{
					Message(MSG_WARNING,
						msg(lng::MFilterTitle),
						{
							msg(lng::MCanDeleteCustomFilterOnly)
						},
						{ lng::MOk });
				}

				break;
			}
			case KEY_CTRLUP:
			case KEY_RCTRLUP:
			case KEY_CTRLDOWN:
			case KEY_RCTRLDOWN:
			{
				const auto SelPos = FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (static_cast<size_t>(SelPos) < FilterData().size() && !(any_of(Key, KEY_CTRLUP, KEY_RCTRLUP) && !SelPos) &&
					!(any_of(Key, KEY_CTRLDOWN, KEY_RCTRLDOWN) && static_cast<size_t>(SelPos) == FilterData().size() - 1))
				{
					const auto NewPos = SelPos + (any_of(Key, KEY_CTRLDOWN, KEY_RCTRLDOWN)? 1 : -1);
					using std::swap;
					swap(FilterList->at(SelPos), FilterList->at(NewPos));
					swap(FilterData()[NewPos], FilterData()[SelPos]);
					FilterList->SetSelectPos(NewPos,1);
					NeedUpdate = true;
				}

				break;
			}

			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	if (!NeedUpdate)
		return;

	Changed = true;

	ProcessSelection(FilterList.get());

	if (Global->Opt->AutoSaveSetup)
		Save(false);

	if (m_FilterType == FFT_PANEL)
	{
		m_HostPanel->Update(UPDATE_KEEP_SELECTION);
		m_HostPanel->Refresh();
	}
}

enumFileFilterFlagsType FileFilter::GetFFFT() const
{
	switch (m_FilterType)
	{
	case FFT_PANEL: return m_HostPanel == Global->CtrlObject->Cp()->RightPanel().get()? FFFT_RIGHTPANEL : FFFT_LEFTPANEL;
	case FFT_COPY: return FFFT_COPY;
	case FFT_FINDFILE: return FFFT_FINDFILE;
	case FFT_SELECT: return FFFT_SELECT;
	case FFT_CUSTOM: return FFFT_CUSTOM;
	default:
		assert(false);
		return FFFT_CUSTOM;
	}
}

wchar_t FileFilter::GetCheck(const FileFilterParams& FFP) const
{
	const auto Flags = FFP.GetFlags(GetFFFT());

	if (Flags & FFF_INCLUDE)
		return Flags & FFF_STRONG? L'I' : L'+';

	if (Flags & FFF_EXCLUDE)
		return Flags & FFF_STRONG? L'X' : L'-';

	return 0;
}

void FileFilter::ProcessSelection(VMenu2 *FilterList) const
{
	const auto FFFT = GetFFFT();

	for (size_t i = 0, size = FilterList->size(); i != size; ++i)
	{
		const auto Check = FilterList->GetCheck(static_cast<int>(i));
		FileFilterParams* CurFilterData = nullptr;

		if (i < FilterData().size())
		{
			CurFilterData = &FilterData()[i];
		}
		else if (i == FilterData().size() + 1)
		{
			CurFilterData = FoldersFilter;
		}
		else if (i > FilterData().size() + 1)
		{
			const auto& Mask = *FilterList->GetComplexUserDataPtr<string>(i);
			if (const auto Iterator = TempFilterData().find(Mask); Iterator != TempFilterData().cend())
			{
				if (!Check)
				{
					bool bCheckedNowhere = true;

					for (int n = 0; n != FFFT_COUNT; ++n)
					{
						if (n != FFFT && Iterator->second.GetFlags(static_cast<enumFileFilterFlagsType>(n)))
						{
							bCheckedNowhere = false;
							break;
						}
					}

					if (bCheckedNowhere)
					{
						TempFilterData().erase(Iterator);
						continue;
					}
				}

				CurFilterData = &Iterator->second;
			}
			else if (Check)
			{
				FileFilterParams NewFilter;
				NewFilter.SetMask(true, Mask.find_first_of(L",;"sv, L"*."sv.size()) == string::npos? Mask : quote(Mask));
				//Авто фильтры они только для файлов, папки не должны к ним подходить
				NewFilter.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);
				auto NewIterator = TempFilterData().emplace(Mask, std::move(NewFilter)).first;
				CurFilterData = &NewIterator->second;
			}
		}

		if (!CurFilterData)
			continue;

		const auto KeyToFlags = [](int Key) -> DWORD
		{
			switch (Key)
			{
			case L'+': return FFF_INCLUDE;
			case L'-': return FFF_EXCLUDE;
			case L'I': return FFF_INCLUDE | FFF_STRONG;
			case L'X': return FFF_EXCLUDE | FFF_STRONG;
			default:
				return 0;
			}
		};

		CurFilterData->SetFlags(FFFT, KeyToFlags(Check));
	}
}

void FileFilter::UpdateCurrentTime()
{
	CurrentTime = os::chrono::nt_clock::now();
}

bool FileFilter::FileInFilter(const FileListItem* fli, filter_status* FilterStatus)
{
	return FileInFilter(*fli, FilterStatus, fli->FileName);
}

bool FileFilter::FileInFilter(const os::fs::find_data& fde, filter_status* const FilterStatus, string_view const FullName)
{
	const auto FFFT = GetFFFT();
	const auto bFolder = (fde.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

	auto IsFound = false;
	auto IsAnyIncludeFound = false;
	auto IsAnyFolderIncludeFound = false;
	auto IsIncluded = false;

	const auto ProcessFilters = [&]
	{
		for (const auto& CurFilterData : FilterData())
		{
			const auto Flags = CurFilterData.GetFlags(FFFT);

			if (!Flags || (IsFound && !(Flags & FFF_STRONG)))
				continue;

			if (Flags & FFF_INCLUDE)
			{
				IsAnyIncludeFound = true;

				os::fs::attributes AttrClear;
				if (CurFilterData.GetAttr(nullptr, &AttrClear))
					IsAnyFolderIncludeFound = IsAnyFolderIncludeFound || !(AttrClear & FILE_ATTRIBUTE_DIRECTORY);
			}

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				IsFound = true;
				IsIncluded = (Flags & FFF_INCLUDE) != 0;

				if (Flags & FFF_STRONG)
					return false;
			}
		}

		return true;
	};

	const auto ProcessFoldersAutoFilter = [&]
	{
		if (FFFT == FFFT_CUSTOM)
			return true;

		const auto Flags = FoldersFilter->GetFlags(FFFT);

		if (Flags && (!IsFound || (Flags&FFF_STRONG)))
		{
			if (Flags&FFF_INCLUDE)
			{
				IsAnyIncludeFound = true;
				IsAnyFolderIncludeFound = true;
			}

			if (bFolder && FoldersFilter->FileInFilter(fde, CurrentTime, FullName))
			{
				IsFound = true;
				IsIncluded = (Flags & FFF_INCLUDE) != 0;

				if (Flags&FFF_STRONG)
					return false;
			}
		}
		return true;
	};

	const auto ProcessAutoFilters = [&]
	{
		for (const auto& [Key, CurFilterData]: TempFilterData())
		{
			const auto Flags = CurFilterData.GetFlags(FFFT);

			if (!Flags || (IsFound && !(Flags&FFF_STRONG)))
				continue;

			IsAnyIncludeFound = IsAnyIncludeFound || (Flags&FFF_INCLUDE);

			// AutoFilters are not relevant for folders.
			// However, we cannot skip the whole loop as we still need to check if any "include" exists (see above)
			if (bFolder)
				continue;

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				IsFound = true;

				IsIncluded = (Flags & FFF_INCLUDE) != 0;

				if (Flags&FFF_STRONG)
					return false;
			}
		}
		return true;
	};

	if (ProcessFilters() && ProcessFoldersAutoFilter() && ProcessAutoFilters())
	{
		//Если папка и она не попала ни под какой exclude фильтр то самое логичное
		//будет сделать ей include если не было дугих include фильтров на папки.
		//А вот Select логичней всего работать чисто по заданному фильтру.
		if (!IsFound && bFolder && !IsAnyFolderIncludeFound && m_FilterType != FFT_SELECT)
		{
			if (FilterStatus)
				*FilterStatus = filter_status::in_include; //???

			return true;
		}
	}

	if (FilterStatus)
		*FilterStatus = !IsFound? filter_status::not_in_filter : IsIncluded? filter_status::in_include : filter_status::in_exclude;

	if (IsFound)
		return IsIncluded;

	//Если элемент не попал ни под один фильтр то он будет включен
	//только если не было ни одного Include фильтра (т.е. были только фильтры исключения).
	return !IsAnyIncludeFound;
}

bool FileFilter::FileInFilter(const PluginPanelItem& fd, filter_status* FilterStatus)
{
	os::fs::find_data fde;
	PluginPanelItemToFindDataEx(fd, fde);
	return FileInFilter(fde, FilterStatus, fde.FileName);
}

bool FileFilter::IsEnabledOnPanel()
{
	if (m_FilterType != FFT_PANEL)
		return false;

	const auto FFFT = GetFFFT();

	if (std::any_of(CONST_RANGE(FilterData(), i) { return i.GetFlags(FFFT); }))
		return true;

	if (FoldersFilter->GetFlags(FFFT))
		return true;

	return std::any_of(CONST_RANGE(TempFilterData(), i) { return i.second.GetFlags(FFFT); });
}

FileFilterParams FileFilter::LoadFilter(/*const*/ HierarchicalConfig& cfg, unsigned long long KeyId)
{
	FileFilterParams Item;

	const HierarchicalConfig::key Key(KeyId);

	Item.SetTitle(cfg.GetValue<string>(Key, names::Title));

	unsigned long long UseMask = 1;
	if (!cfg.GetValue(Key, names::UseMask, UseMask))
	{
		// Old format
		// TODO 2020 Q4: remove
		unsigned long long IgnoreMask = 0;
		if (cfg.GetValue(Key, legacy_names::IgnoreMask, IgnoreMask))
		{
			UseMask = !IgnoreMask;
			cfg.SetValue(Key, names::UseMask, UseMask);
			cfg.DeleteValue(Key, legacy_names::IgnoreMask);
		}
	}

	Item.SetMask(UseMask != 0, cfg.GetValue<string>(Key, names::Mask));

	unsigned long long DateAfter = 0;
	if (!cfg.GetValue(Key, names::DateTimeAfter, DateAfter))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (bytes Blob; cfg.GetValue(Key, legacy_names::DateAfter, Blob) && deserialise(Blob, DateAfter))
		{
			cfg.SetValue(Key, names::DateTimeAfter, DateAfter);
			cfg.DeleteValue(Key, legacy_names::DateAfter);
		}
	}

	unsigned long long DateBefore = 0;
	if (!cfg.GetValue(Key, names::DateTimeBefore, DateBefore))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (bytes Blob; cfg.GetValue(Key, legacy_names::DateBefore, Blob) && deserialise(Blob, DateBefore))
		{
			cfg.SetValue(Key, names::DateTimeBefore, DateBefore);
			cfg.DeleteValue(Key, legacy_names::DateBefore);
		}
	}

	const auto UseDate = cfg.GetValue<bool>(Key, names::UseDate);
	const auto DateType = cfg.GetValue<unsigned long long>(Key, names::DateType);

	unsigned long long DateRelative = 0;
	if (!cfg.GetValue(Key, names::DateRelative, DateRelative))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (cfg.GetValue(Key, legacy_names::RelativeDate, DateRelative))
		{
			cfg.SetValue(Key, names::DateRelative, DateRelative);
			cfg.DeleteValue(Key, legacy_names::RelativeDate);
		}
	}

	Item.SetDate(UseDate, static_cast<enumFDateType>(DateType), DateRelative?
		filter_dates(os::chrono::hectonanoseconds(DateAfter), os::chrono::hectonanoseconds(DateBefore)) :
		filter_dates(os::chrono::nt_clock::from_hectonanoseconds(DateAfter), os::chrono::nt_clock::from_hectonanoseconds(DateBefore)));

	const auto UseSize = cfg.GetValue<bool>(Key, names::UseSize);

	const auto SizeAbove = cfg.GetValue<string>(Key, names::SizeAboveS);
	const auto SizeBelow = cfg.GetValue<string>(Key, names::SizeBelowS);
	Item.SetSize(UseSize, SizeAbove, SizeBelow);

	const auto UseHardLinks = cfg.GetValue<bool>(Key, names::UseHardLinks);
	const auto HardLinksAbove = cfg.GetValue<unsigned long long>(Key, names::HardLinksAbove);
	const auto HardLinksBelow = cfg.GetValue<unsigned long long>(Key, names::HardLinksBelow);
	Item.SetHardLinks(UseHardLinks, HardLinksAbove, HardLinksBelow);

	unsigned long long AttrSet = 0;
	if (!cfg.GetValue(Key, names::AttrSet, AttrSet))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (cfg.GetValue(Key, legacy_names::IncludeAttributes, AttrSet))
		{
			cfg.SetValue(Key, names::AttrSet, AttrSet);
			cfg.DeleteValue(Key, legacy_names::IncludeAttributes);
		}
	}

	unsigned long long AttrClear = 0;
	if (!cfg.GetValue(Key, names::AttrClear, AttrClear))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (cfg.GetValue(Key, legacy_names::ExcludeAttributes, AttrClear))
		{
			cfg.SetValue(Key, names::AttrClear, AttrClear);
			cfg.DeleteValue(Key, legacy_names::ExcludeAttributes);
		}
	}

	unsigned long long UseAttr = 0;
	if (!cfg.GetValue(Key, names::UseAttr, UseAttr))
	{
		// Old format
		// TODO 2020 Q4: remove
		if (AttrSet || AttrClear)
		{
			UseAttr = true;
			cfg.SetValue(Key, names::UseAttr, UseAttr);
		}
	}

	Item.SetAttr(UseAttr != 0, static_cast<os::fs::attributes>(AttrSet), static_cast<os::fs::attributes>(AttrClear));

	return Item;
}

static void SaveFlags(const HierarchicalConfigUniquePtr& Cfg, HierarchicalConfig::key const FilterKey, const FileFilterParams& Item)
{
	const auto FlagsKey = Cfg->CreateKey(FilterKey, names::Flags);

	for (size_t i = 0; i != std::size(FilterFlagNames); ++i)
	{
		Cfg->SetValue(FlagsKey, FilterFlagNames[i], Item.GetFlags(static_cast<enumFileFilterFlagsType>(i)));
	}
}

static bool LoadFlags(const HierarchicalConfigUniquePtr& Cfg, HierarchicalConfig::key const FilterKey, FileFilterParams& Item)
{
	const auto FlagsKey = Cfg->FindByName(FilterKey, names::Flags);
	if (!FlagsKey)
		return false;

	auto AnyLoaded = false;

	for (size_t i = 0; i != std::size(FilterFlagNames); ++i)
	{
		unsigned long long Value;
		if (Cfg->GetValue(FlagsKey, FilterFlagNames[i], Value))
		{
			AnyLoaded = true;
			Item.SetFlags(static_cast<enumFileFilterFlagsType>(i), Value);
		}
	}

	return AnyLoaded;
}

// Old format
// TODO 2020 Q4: remove
static bool LoadLegacyFlags(const HierarchicalConfigUniquePtr& Cfg, HierarchicalConfig::key const Key, string_view const Name, FileFilterParams& Item)
{
	const auto LegacyCount = FFFT_CUSTOM + 1;
	static_assert(FFFT_COUNT >= LegacyCount);

	DWORD LegacyFlags[LegacyCount]{};
	if (bytes Blob; !Cfg->GetValue(Key, Name, Blob) || !deserialise(Blob, LegacyFlags))
		return false;

	for (int i = 0; i != LegacyCount; ++i)
		Item.SetFlags(static_cast<enumFileFilterFlagsType>(i), LegacyFlags[i]);

	return true;
}

void FileFilter::InitFilter()
{
	static FileFilterParams _FoldersFilter;
	FoldersFilter = &_FoldersFilter;
	FoldersFilter->SetMask(false, L"*"sv);
	FoldersFilter->SetAttr(true, FILE_ATTRIBUTE_DIRECTORY, 0);

	const auto cfg = ConfigProvider().CreateFiltersConfig();
	const auto root = cfg->FindByName(cfg->root_key, names::Filters);

	if (!root)
		return;

	if (const auto FoldersFilterKey = cfg->FindByName(root, names::FoldersFilter))
	{
		LoadFlags(cfg, FoldersFilterKey, *FoldersFilter);
	}
	else
	{
		// Old format
		// TODO 2020 Q4: remove
		if (LoadLegacyFlags(cfg, root, legacy_names::FoldersFilterFFlags, *FoldersFilter))
		{
			SaveFlags(cfg, cfg->CreateKey(root, names::FoldersFilter), *FoldersFilter);
			cfg->DeleteValue(root, legacy_names::FoldersFilterFFlags);
		}
	}

	for (const auto& Key: cfg->KeysEnumerator(root, names::Filter))
	{
		auto NewItem = LoadFilter(*cfg, Key.get());

		if (!LoadFlags(cfg, Key, NewItem))
		{
			// Old format
			// TODO 2020 Q4: remove
			if (LoadLegacyFlags(cfg, Key, legacy_names::FFlags, NewItem))
			{
				SaveFlags(cfg, Key, NewItem);
				cfg->DeleteValue(Key, legacy_names::FFlags);
			}
		}

		FilterData().emplace_back(std::move(NewItem));
	}

	for (const auto& Key: cfg->KeysEnumerator(root, L"PanelMask"sv))
	{
		FileFilterParams NewItem;

		auto Mask = cfg->GetValue<string>(Key, names::Mask);
		NewItem.SetMask(true, Mask);

		//Авто фильтры они только для файлов, папки не должны к ним подходить
		NewItem.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);

		if (!LoadFlags(cfg, Key, NewItem))
		{
			// Old format
			// TODO 2020 Q4: remove
			if (LoadLegacyFlags(cfg, Key, legacy_names::FFlags, NewItem))
			{
				SaveFlags(cfg, Key, NewItem);
				cfg->DeleteValue(Key, legacy_names::FFlags);
			}
		}

		TempFilterData().emplace(unquote(std::move(Mask)), std::move(NewItem));
	}
}


void FileFilter::CloseFilter()
{
	FilterData().clear();
	TempFilterData().clear();
}

void FileFilter::SaveFilter(HierarchicalConfig& cfg, unsigned long long KeyId, const FileFilterParams& Item)
{
	HierarchicalConfig::key Key(KeyId);

	cfg.SetValue(Key, names::Title, Item.GetTitle());
	cfg.SetValue(Key, names::UseMask, Item.IsMaskUsed());
	cfg.SetValue(Key, names::Mask, Item.GetMask());

	DWORD DateType;
	filter_dates Dates;
	cfg.SetValue(Key, names::UseDate, Item.GetDate(&DateType, &Dates));
	cfg.SetValue(Key, names::DateType, DateType);

	Dates.visit(overload
	{
		[&](os::chrono::duration After, os::chrono::duration Before)
		{
			cfg.SetValue(Key, names::DateTimeAfter, os::chrono::nt_clock::to_hectonanoseconds(After));
			cfg.SetValue(Key, names::DateTimeBefore, os::chrono::nt_clock::to_hectonanoseconds(Before));
			cfg.SetValue(Key, names::DateRelative, true);
		},
		[&](os::chrono::time_point After, os::chrono::time_point Before)
		{
			cfg.SetValue(Key, names::DateTimeAfter, os::chrono::nt_clock::to_hectonanoseconds(After));
			cfg.SetValue(Key, names::DateTimeBefore, os::chrono::nt_clock::to_hectonanoseconds(Before));
			cfg.SetValue(Key, names::DateRelative, false);
		}
	});

	cfg.SetValue(Key, names::UseSize, Item.IsSizeUsed());
	cfg.SetValue(Key, names::SizeAboveS, Item.GetSizeAbove());
	cfg.SetValue(Key, names::SizeBelowS, Item.GetSizeBelow());

	DWORD HardLinksAbove,HardLinksBelow;
	cfg.SetValue(Key, names::UseHardLinks, Item.GetHardLinks(&HardLinksAbove,&HardLinksBelow)? 1 : 0);
	cfg.SetValue(Key, names::HardLinksAbove, HardLinksAbove);
	cfg.SetValue(Key, names::HardLinksBelow, HardLinksBelow);

	os::fs::attributes AttrSet, AttrClear;
	cfg.SetValue(Key, names::UseAttr, Item.GetAttr(&AttrSet, &AttrClear));
	cfg.SetValue(Key, names::AttrSet, AttrSet);
	cfg.SetValue(Key, names::AttrClear, AttrClear);
}

void FileFilter::Save(bool always)
{
	if (!always && !Changed)
		return;

	const auto cfg = ConfigProvider().CreateFiltersConfig();

	SCOPED_ACTION(auto)(cfg->ScopedTransaction());

	if (const auto Key = cfg->FindByName(cfg->root_key, names::Filters))
		cfg->DeleteKeyTree(Key);

	const auto root = cfg->CreateKey(cfg->root_key, names::Filters);

	for (size_t i=0; i<FilterData().size(); ++i)
	{
		const auto Key = cfg->CreateKey(root, names::Filter + str(i));
		const auto& CurFilterData = FilterData()[i];

		SaveFilter(*cfg, Key.get(), CurFilterData);
		SaveFlags(cfg, Key, CurFilterData);
	}

	for (const auto& [CurFilterData, i]: enumerate(TempFilterData()))
	{
		const auto Key = cfg->CreateKey(root, L"PanelMask"sv + str(i));
		cfg->SetValue(Key, names::Mask, CurFilterData.second.GetMask());
		SaveFlags(cfg, Key, CurFilterData.second);
	}

	SaveFlags(cfg, cfg->CreateKey(root, names::FoldersFilter), *FoldersFilter);

	Changed = false;
}

static void SwapPanelFlags(FileFilterParams& CurFilterData)
{
	const auto LPFlags = CurFilterData.GetFlags(FFFT_LEFTPANEL);
	const auto RPFlags = CurFilterData.GetFlags(FFFT_RIGHTPANEL);
	CurFilterData.SetFlags(FFFT_LEFTPANEL,  RPFlags);
	CurFilterData.SetFlags(FFFT_RIGHTPANEL, LPFlags);
}

void FileFilter::SwapFilter()
{
	Changed = true;

	for (auto& i: FilterData())
		SwapPanelFlags(i);

	SwapPanelFlags(*FoldersFilter);

	for (auto& i: TempFilterData())
		SwapPanelFlags(i.second);
}
