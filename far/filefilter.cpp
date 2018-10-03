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

#include "filefilter.hpp"

#include "filefilterparams.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "scantree.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "mix.hpp"
#include "configdb.hpp"
#include "keyboard.hpp"
#include "DlgGuid.hpp"
#include "lang.hpp"
#include "string_sort.hpp"
#include "global.hpp"

#include "platform.fs.hpp"

#include "common/bytes_view.hpp"
#include "common/scope_exit.hpp"

#include "format.hpp"

static const struct
{
	const string_view

	Filters,
	Filter,
	FFlags,
	FoldersFilterFFlags,

	Title,
	UseAttr, AttrSet, AttrClear,
	UseMask, Mask,
	UseDate, DateType, DateTimeAfter, DateTimeBefore, DateRelative,
	UseSize, SizeAbove, SizeBelow,
	UseHardLinks, HardLinksAbove, HardLinksBelow;
}
Strings =
{
	L"Filters"sv,
	L"Filter"sv,
	L"FFlags"sv,
	L"FoldersFilterFFlags"sv,

	L"Title"sv,
	L"UseAttr"sv, L"AttrSet"sv, L"AttrClear"sv,
	L"UseMask"sv, L"Mask"sv,
	L"UseDate"sv, L"DateType"sv, L"DateTimeAfter"sv, L"DateTimeBefore"sv, L"DateRelative"sv,
	L"UseSize"sv, L"SizeAboveS"sv, L"SizeBelowS"sv,
	L"UseHardLinks"sv, L"HardLinksAbove"sv, L"HardLinksBelow"sv,
};

static auto& FilterData()
{
	static std::vector<FileFilterParams> sFilterData;
	return sFilterData;
}

static auto& TempFilterData()
{
	static std::vector<FileFilterParams> sTempFilterData;
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
	FilterList->SetBottomTitle(msg(lng::MFilterBottom));
	FilterList->SetMenuFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);
	FilterList->SetId(FiltersMenuId);

	std::for_each(RANGE(FilterData(), i)
	{
		MenuItemEx ListItem(MenuString(&i));
		if (const auto Check = GetCheck(i))
			ListItem.SetCustomCheck(Check);
		FilterList->AddItem(ListItem);
	});

	if (m_FilterType != FFT_CUSTOM)
	{
		using extension_list = std::list<std::pair<string, int>>;
		extension_list Extensions;

		{
			const auto FFFT = GetFFFT();

			for (const auto& i: TempFilterData())
			{
				//AY: Будем показывать только те выбранные авто фильтры
				//(для которых нету файлов на панели) которые выбраны в области данного меню
				if (i.GetFlags(FFFT))
				{
					if (!ParseAndAddMasks(Extensions, unquote(i.GetMask()), 0, GetCheck(i)))
						break;
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

		if (m_HostPanel->GetMode() == panel_mode::NORMAL_PANEL)
		{
			string strFileName;
			os::fs::find_data fdata;
			ScanTree ScTree(false, false);
			ScTree.SetFindPath(m_HostPanel->GetCurDir(), L"*"sv);

			while (ScTree.GetNextName(fdata,strFileName))
				if (!ParseAndAddMasks(Extensions, fdata.FileName, fdata.Attributes, 0))
					break;
		}
		else
		{
			string strFileName;
			DWORD FileAttr;

			for (int i = 0; m_HostPanel->GetFileName(strFileName, i, FileAttr); i++)
				if (!ParseAndAddMasks(Extensions, strFileName, FileAttr, 0))
					break;
		}

		Extensions.sort([](const extension_list::value_type& a, const extension_list::value_type& b)
		{
			return string_sort::less(a.first, b.first);
		});

		wchar_t h = L'1';
		for (const auto& i: Extensions)
		{
			MenuItemEx ListItem(MenuString(nullptr, false, h, true, i.first, msg(lng::MPanelFileType)));
			i.second? ListItem.SetCustomCheck(i.second) : ListItem.ClearCheck();
			ListItem.ComplexUserData = i.first;
			FilterList->AddItem(ListItem);

			h == L'9' ? h = L'A' : ((h == L'Z' || !h)? h = 0 : ++h);
		}
	}

	const auto ExitCode=FilterList->RunEx([&](int Msg, void *param)
	{
		if (Msg==DN_LISTHOTKEY)
			return 1;
		if (Msg!=DN_INPUT)
			return 0;

		int Key=InputRecordToKey(static_cast<INPUT_RECORD*>(param));

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
				int SelPos=FilterList->GetSelectPos();

				if (SelPos<0)
					break;

				int Check=FilterList->GetCheck(SelPos);
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

				break;
			}
			case KEY_F4:
			{
				int SelPos=FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData().size())
				{
					if (FileFilterConfig(&FilterData()[SelPos]))
					{
						MenuItemEx ListItem(MenuString(&FilterData()[SelPos]));

						if (const auto Check = GetCheck(FilterData()[SelPos]))
							ListItem.SetCustomCheck(Check);

						FilterList->DeleteItem(SelPos);
						FilterList->AddItem(ListItem,SelPos);
						FilterList->SetSelectPos(SelPos,1);
						NeedUpdate = true;
					}
				}
				else if (SelPos>(int)FilterData().size())
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
					size_t ListPos = pos;
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
				int SelPos=FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData().size())
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
				else if (SelPos>(int)FilterData().size())
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
				int SelPos=FilterList->GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData().size() && !((Key == KEY_CTRLUP || Key == KEY_RCTRLUP) && !SelPos) &&
					!((Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN) && SelPos == (int)(FilterData().size() - 1)))
				{
					int NewPos = SelPos + ((Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN) ? 1 : -1);
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

	for (size_t i = 0, j = 0, size = FilterList->size(); i != size; ++i)
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
			//AY: Так как в меню мы показываем только те выбранные авто фильтры
			//которые выбраны в области данного меню и TempFilterData вполне
			//может содержать маску которую тока что выбрали в этом меню но
			//она уже была выбрана в другом и так как TempFilterData
			//и авто фильтры в меню отсортированы по алфавиту то немного
			//поколдуем чтоб не было дубликатов в памяти.
			const auto strMask1 = unquote(Mask);

			while (j < TempFilterData().size())
			{
				CurFilterData = &TempFilterData()[j];
				const auto strMask2 = unquote(CurFilterData->GetMask());

				if (!string_sort::less(strMask2, strMask1))
					break;

				j++;
			}

			if (CurFilterData)
			{
				if (equal_icase(Mask, CurFilterData->GetMask()))
				{
					if (!Check)
					{
						bool bCheckedNowhere = true;

						for (int n = FFFT_FIRST; n < FFFT_COUNT; n++)
						{
							if (n != FFFT && CurFilterData->GetFlags((enumFileFilterFlagsType)n))
							{
								bCheckedNowhere = false;
								break;
							}
						}

						if (bCheckedNowhere)
						{
							TempFilterData().erase(TempFilterData().begin() + j);
							continue;
						}
					}
					else
					{
						j++;
					}
				}
				else
					CurFilterData = nullptr;
			}

			if (Check && !CurFilterData)
			{
				FileFilterParams NewFilter;
				NewFilter.SetMask(true, Mask);
				//Авто фильтры они только для файлов, папки не должны к ним подходить
				NewFilter.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);
				CurFilterData = &*TempFilterData().emplace(TempFilterData().begin() + j, std::move(NewFilter));
				j++;
			}
		}

		if (!CurFilterData)
			continue;

		const auto& KeyToFlags = [](int Key) -> DWORD
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
	return FileInFilter(*fli, FilterStatus, &fli->FileName);
}

bool FileFilter::FileInFilter(const os::fs::find_data& fde, filter_status* FilterStatus, const string* FullName)
{
	const auto FFFT = GetFFFT();
	const auto bFolder = (fde.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

	auto IsFound = false;
	auto IsAnyIncludeFound = false;
	auto IsAnyFolderIncludeFound = false;
	auto IsIncluded = false;

	const auto& ProcessFilters = [&]
	{
		for (const auto& CurFilterData : FilterData())
		{
			const auto Flags = CurFilterData.GetFlags(FFFT);

			if (!Flags || (IsFound && !(Flags & FFF_STRONG)))
				continue;

			if (Flags & FFF_INCLUDE)
			{
				IsAnyIncludeFound = true;

				DWORD AttrClear;
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

	const auto& ProcessFoldersAutoFilter = [&]
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

	const auto& ProcessAutoFilters = [&]
	{
		for (const auto& CurFilterData : TempFilterData())
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
	return FileInFilter(fde, FilterStatus, &fde.FileName);
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

	return std::any_of(CONST_RANGE(TempFilterData(), i) { return i.GetFlags(FFFT); });
}

void FileFilter::LoadFilter(const HierarchicalConfig* cfg, unsigned long long KeyId, FileFilterParams& Item)
{
	HierarchicalConfig::key Key(KeyId);

	string Title;
	cfg->GetValue(Key, Strings.Title, Title);
	Item.SetTitle(Title);

	unsigned long long UseMask = 1;
	cfg->GetValue(Key, Strings.UseMask, UseMask);

	string Mask;
	cfg->GetValue(Key, Strings.Mask, Mask);
	Item.SetMask(UseMask != 0, Mask);

	unsigned long long DateAfter = 0;
	cfg->GetValue(Key, Strings.DateTimeAfter, DateAfter);

	unsigned long long DateBefore = 0;
	cfg->GetValue(Key, Strings.DateTimeBefore, DateBefore);

	unsigned long long UseDate = 0;
	cfg->GetValue(Key, Strings.UseDate, UseDate);

	unsigned long long DateType = 0;
	cfg->GetValue(Key, Strings.DateType, DateType);

	unsigned long long DateRelative = 0;
	cfg->GetValue(Key, Strings.DateRelative, DateRelative);

	Item.SetDate(UseDate != 0, static_cast<enumFDateType>(DateType), DateRelative?
		filter_dates(os::chrono::duration(DateAfter), os::chrono::duration(DateBefore)) :
		filter_dates(os::chrono::time_point(os::chrono::duration(DateAfter)), os::chrono::time_point(os::chrono::duration(DateBefore))));

	string SizeAbove;
	cfg->GetValue(Key, Strings.SizeAbove, SizeAbove);

	string SizeBelow;
	cfg->GetValue(Key, Strings.SizeBelow, SizeBelow);

	unsigned long long UseSize = 0;
	cfg->GetValue(Key, Strings.UseSize, UseSize);
	Item.SetSize(UseSize != 0, SizeAbove, SizeBelow);

	unsigned long long UseHardLinks = 0;
	cfg->GetValue(Key, Strings.UseHardLinks, UseHardLinks);

	unsigned long long HardLinksAbove = 0;
	cfg->GetValue(Key, Strings.HardLinksAbove, HardLinksAbove);

	unsigned long long HardLinksBelow = 0;
	cfg->GetValue(Key, Strings.HardLinksBelow, HardLinksBelow);

	Item.SetHardLinks(UseHardLinks != 0, HardLinksAbove, HardLinksBelow);

	unsigned long long AttrSet = 0;
	cfg->GetValue(Key, Strings.AttrSet, AttrSet);

	unsigned long long AttrClear = 0;
	cfg->GetValue(Key, Strings.AttrClear, AttrClear);

	unsigned long long UseAttr = 0;
	cfg->GetValue(Key, Strings.UseAttr, UseAttr);

	Item.SetAttr(UseAttr != 0, static_cast<DWORD>(AttrSet), static_cast<DWORD>(AttrClear));
}

void FileFilter::InitFilter()
{
	static FileFilterParams _FoldersFilter;
	FoldersFilter = &_FoldersFilter;
	FoldersFilter->SetMask(false, L"*"sv);
	FoldersFilter->SetAttr(true, FILE_ATTRIBUTE_DIRECTORY, 0);

	const auto cfg = ConfigProvider().CreateFiltersConfig();
	const auto root = cfg->FindByName(cfg->root_key(), Strings.Filters);

	if (!root)
		return;

	const auto& LoadFlags = [&cfg](const auto& Key, const auto& Name, auto& Item)
	{
		DWORD Flags[FFFT_COUNT]{};
		cfg->GetValue(Key, Name, bytes::reference(Flags));

		for (DWORD i = FFFT_FIRST; i < FFFT_COUNT; i++)
			Item.SetFlags(static_cast<enumFileFilterFlagsType>(i), Flags[i]);
	};

	LoadFlags(root, Strings.FoldersFilterFFlags, *FoldersFilter);

	for (;;)
	{
		const auto key = cfg->FindByName(root, Strings.Filter + str(FilterData().size()));
		if (!key)
			break;

		FileFilterParams NewItem;
		LoadFilter(cfg.get(), key.get(), NewItem);
		LoadFlags(key, Strings.FFlags, NewItem);
		FilterData().emplace_back(std::move(NewItem));
	}

	for (;;)
	{
		const auto key = cfg->FindByName(root, L"PanelMask"sv + str(TempFilterData().size()));
		if (!key)
			break;

		FileFilterParams NewItem;

		string Mask;
		cfg->GetValue(key, Strings.Mask, Mask);
		NewItem.SetMask(true, Mask);

		//Авто фильтры они только для файлов, папки не должны к ним подходить
		NewItem.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);

		LoadFlags(key, Strings.FFlags, NewItem);

		TempFilterData().emplace_back(std::move(NewItem));
	}
}


void FileFilter::CloseFilter()
{
	FilterData().clear();
	TempFilterData().clear();
}

void FileFilter::SaveFilter(HierarchicalConfig *cfg, unsigned long long KeyId, const FileFilterParams& Item)
{
	HierarchicalConfig::key Key(KeyId);

	cfg->SetValue(Key, Strings.Title, Item.GetTitle());
	cfg->SetValue(Key, Strings.UseMask, Item.IsMaskUsed());
	cfg->SetValue(Key, Strings.Mask, Item.GetMask());

	DWORD DateType;
	filter_dates Dates;
	cfg->SetValue(Key, Strings.UseDate, Item.GetDate(&DateType, &Dates));
	cfg->SetValue(Key, Strings.DateType, DateType);

	Dates.visit(overload
	(
		[&](os::chrono::duration After, os::chrono::duration Before)
		{
			cfg->SetValue(Key, Strings.DateTimeAfter, After.count());
			cfg->SetValue(Key, Strings.DateTimeBefore, Before.count());
			cfg->SetValue(Key, Strings.DateRelative, true);
		},
		[&](os::chrono::time_point After, os::chrono::time_point Before)
		{
			cfg->SetValue(Key, Strings.DateTimeAfter, After.time_since_epoch().count());
			cfg->SetValue(Key, Strings.DateTimeBefore, Before.time_since_epoch().count());
			cfg->SetValue(Key, Strings.DateRelative, false);
		}
	));

	cfg->SetValue(Key, Strings.UseSize, Item.IsSizeUsed());
	cfg->SetValue(Key, Strings.SizeAbove, Item.GetSizeAbove());
	cfg->SetValue(Key, Strings.SizeBelow, Item.GetSizeBelow());

	DWORD HardLinksAbove,HardLinksBelow;
	cfg->SetValue(Key, Strings.UseHardLinks, Item.GetHardLinks(&HardLinksAbove,&HardLinksBelow)? 1 : 0);
	cfg->SetValue(Key, Strings.HardLinksAbove, HardLinksAbove);
	cfg->SetValue(Key, Strings.HardLinksBelow, HardLinksBelow);

	DWORD AttrSet, AttrClear;
	cfg->SetValue(Key, Strings.UseAttr, Item.GetAttr(&AttrSet, &AttrClear));
	cfg->SetValue(Key, Strings.AttrSet, AttrSet);
	cfg->SetValue(Key, Strings.AttrClear, AttrClear);
}

void FileFilter::Save(bool always)
{
	if (!always && !Changed)
		return;

	const auto cfg = ConfigProvider().CreateFiltersConfig();

	auto root = cfg->FindByName(cfg->root_key(), Strings.Filters);
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(cfg->root_key(), Strings.Filters);
	if (!root)
		return;

	const auto& SaveFlags = [&cfg](const auto& Key, const auto& Name, const auto& Item)
	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD j = FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = Item.GetFlags(static_cast<enumFileFilterFlagsType>(j));

		cfg->SetValue(Key, Name, bytes_view(Flags));
	};

	for (size_t i=0; i<FilterData().size(); ++i)
	{
		const auto Key = cfg->CreateKey(root, Strings.Filter + str(i));
		if (!Key)
			break;

		const auto& CurFilterData = FilterData()[i];

		SaveFilter(cfg.get(), Key.get(), CurFilterData);
		SaveFlags(Key, Strings.FFlags, CurFilterData);
	}

	for (size_t i=0; i<TempFilterData().size(); ++i)
	{
		const auto Key = cfg->CreateKey(root, L"PanelMask"sv + str(i));
		if (!Key)
			break;

		const auto& CurFilterData = TempFilterData()[i];

		cfg->SetValue(Key, Strings.Mask, CurFilterData.GetMask());
		SaveFlags(Key, Strings.FFlags, CurFilterData);
	}

	SaveFlags(root, Strings.FoldersFilterFFlags, *FoldersFilter);

	Changed = false;
}

void FileFilter::SwapPanelFlags(FileFilterParams& CurFilterData)
{
	const auto LPFlags = CurFilterData.GetFlags(FFFT_LEFTPANEL);
	const auto RPFlags = CurFilterData.GetFlags(FFFT_RIGHTPANEL);
	CurFilterData.SetFlags(FFFT_LEFTPANEL,  RPFlags);
	CurFilterData.SetFlags(FFFT_RIGHTPANEL, LPFlags);
}

void FileFilter::SwapFilter()
{
	Changed = true;
	std::for_each(ALL_RANGE(FilterData()), SwapPanelFlags);
	SwapPanelFlags(*FoldersFilter);
	std::for_each(ALL_RANGE(TempFilterData()), SwapPanelFlags);
}

int FileFilter::ParseAndAddMasks(std::list<std::pair<string, int>>& Extensions, const string& FileName, DWORD FileAttr, int Check)
{
	if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) || IsParentDirectory(FileName))
		return -1;

	const auto DotPos = FileName.rfind(L'.');
	string strMask;

	if (DotPos == string::npos)
	{
		strMask = L"*."s;
	}
	else
	{
		const auto Ext = string_view(FileName).substr(DotPos);
		// Если маска содержит разделитель (',' или ';'), то возьмем ее в кавычки
		strMask = FileName.find_first_of(L",;", DotPos) != string::npos? concat(L"\"*"sv, Ext, L'"') : concat(L'*', Ext);
	}

	if (std::any_of(CONST_RANGE(Extensions, i) { return equal_icase(i.first, strMask); }))
		return -1;

	Extensions.emplace_back(strMask, Check);
	return 1;
}
