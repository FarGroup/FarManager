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

#include "headers.hpp"
#pragma hdrstop

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
#include "datetime.hpp"
#include "colormix.hpp"

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

FileFilterParams *FoldersFilter;

static bool bMenuOpen = false;

static bool Changed = false;

FileFilter::FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType):
	m_HostPanel(HostPanel),
	m_FilterType(FilterType)
{
	UpdateCurrentTime();
}

bool FileFilter::FilterEdit()
{
	if (bMenuOpen)
		return false;

	Changed = true;
	bMenuOpen = true;
	int ExitCode;
	bool bNeedUpdate=false;
	const auto FilterList = VMenu2::create(msg(lng::MFilterTitle), nullptr, 0, ScrY - 6);
	FilterList->SetHelp(L"FiltersMenu");
	FilterList->SetPosition(-1,-1,0,0);
	FilterList->SetBottomTitle(msg(lng::MFilterBottom));
	FilterList->SetMenuFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);
	FilterList->SetId(FiltersMenuId);

	std::for_each(RANGE(FilterData(), i)
	{
		MenuItemEx ListItem(MenuString(&i));
		ListItem.SetCheck(GetCheck(i));
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
			int Check = GetCheck(*FoldersFilter);

			if (Check)
				ListItem.SetCheck(Check);

			FilterList->AddItem(ListItem);
		}

		if (m_HostPanel->GetMode() == panel_mode::NORMAL_PANEL)
		{
			string strFileName;
			os::fs::find_data fdata;
			ScanTree ScTree(false, false);
			ScTree.SetFindPath(m_HostPanel->GetCurDir(), L"*");

			while (ScTree.GetNextName(fdata,strFileName))
				if (!ParseAndAddMasks(Extensions, fdata.strFileName, fdata.dwFileAttributes, 0))
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
			return StrCmpI(a.first, b.first) < 0;
		});

		wchar_t h = L'1';
		for (const auto& i: Extensions)
		{
			MenuItemEx ListItem(MenuString(nullptr, false, h, true, i.first.data(), msg(lng::MPanelFileType).data()));
			ListItem.SetCheck(i.second);
			ListItem.UserData = i.first;
			FilterList->AddItem(ListItem);

			h == L'9' ? h = L'A' : ((h == L'Z' || !h)? h = 0 : ++h);
		}
	}

	ExitCode=FilterList->RunEx([&](int Msg, void *param)
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

				FilterList->SetCheck(NewCheck,SelPos);
				FilterList->SetSelectPos(SelPos,1);
				FilterList->Key(KEY_DOWN);
				bNeedUpdate=true;
				return 1;
			}
			case KEY_SHIFTBS:
			{
				for (size_t i = 0, size = FilterList->size(); i != size; ++i)
				{
					FilterList->SetCheck(FALSE, static_cast<int>(i));
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
						int Check = GetCheck(FilterData()[SelPos]);

						if (Check)
							ListItem.SetCheck(Check);

						FilterList->DeleteItem(SelPos);
						FilterList->AddItem(ListItem,SelPos);
						FilterList->SetSelectPos(SelPos,1);
						bNeedUpdate=true;
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
				size_t SelPos=pos;
				size_t SelPos2=pos+1;

				SelPos = std::min(FilterData().size(), SelPos);

				FileFilterParams NewFilter;

				if (Key==KEY_F5)
				{
					if (SelPos2 < FilterData().size())
					{
						NewFilter = FilterData()[SelPos2].Clone();
						NewFilter.SetTitle(L"");
						NewFilter.ClearAllFlags();
					}
					else if (SelPos2 == FilterData().size() + 2)
					{
						NewFilter = FoldersFilter->Clone();
						NewFilter.SetTitle(L"");
						NewFilter.ClearAllFlags();
					}
					else if (SelPos2 > FilterData().size() + 2)
					{
						NewFilter.SetMask(true, *FilterList->GetUserDataPtr<string>(SelPos2-1));
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
					const auto FilterIterator = FilterData().emplace(FilterData().begin() + SelPos, std::move(NewFilter));
					MenuItemEx ListItem(MenuString(&*FilterIterator));
					FilterList->AddItem(ListItem,static_cast<int>(SelPos));
					FilterList->SetSelectPos(static_cast<int>(SelPos),1);
					bNeedUpdate=true;
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
						bNeedUpdate=true;
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
					bNeedUpdate=true;
				}

				break;
			}

			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});


	if (ExitCode!=-1)
		ProcessSelection(FilterList.get());

	if (Global->Opt->AutoSaveSetup)
		Save(false);

	if (ExitCode!=-1 || bNeedUpdate)
	{
		if (m_FilterType == FFT_PANEL)
		{
			m_HostPanel->Update(UPDATE_KEEP_SELECTION);
			m_HostPanel->Refresh();
		}
	}

	bMenuOpen = false;
	return ExitCode != -1;
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

int FileFilter::GetCheck(const FileFilterParams& FFP) const
{
	DWORD Flags = FFP.GetFlags(GetFFFT());

	if (Flags&FFF_INCLUDE)
	{
		if (Flags&FFF_STRONG)
			return L'I';

		return L'+';
	}
	else if (Flags&FFF_EXCLUDE)
	{
		if (Flags&FFF_STRONG)
			return L'X';

		return L'-';
	}

	return 0;
}

void FileFilter::ProcessSelection(VMenu2 *FilterList) const
{
	const auto FFFT = GetFFFT();

	for (size_t i = 0, j = 0, size = FilterList->size(); i != size; ++i)
	{
		int Check = FilterList->GetCheck(static_cast<int>(i));
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
			const auto& Mask = *FilterList->GetUserDataPtr<string>(i);
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

				if (StrCmpI(strMask1, strMask2) < 1)
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
	CurrentTime = GetCurrentUTCTimeInUI64();
}

bool FileFilter::FileInFilter(const FileListItem* fli,enumFileInFilterType *foundType)
{
	os::fs::find_data fde;
	fde.dwFileAttributes=fli->FileAttr;
	fde.ftCreationTime=fli->CreationTime;
	fde.ftLastAccessTime=fli->AccessTime;
	fde.ftLastWriteTime=fli->WriteTime;
	fde.ftChangeTime=fli->ChangeTime;
	fde.nFileSize=fli->FileSize;
	fde.nAllocationSize=fli->AllocationSize;
	fde.strFileName=fli->strName;
	fde.strAlternateFileName=fli->strShortName;
	return FileInFilter(fde, foundType, &fli->strName);
}

bool FileFilter::FileInFilter(const os::fs::find_data& fde,enumFileInFilterType *foundType, const string* FullName)
{
	const auto FFFT = GetFFFT();
	bool bFound=false;
	bool bAnyIncludeFound=false;
	bool bAnyFolderIncludeFound=false;
	bool bInc=false;
	bool bFolder=(fde.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
	DWORD Flags;

	for (const auto& CurFilterData: FilterData())
	{
		Flags = CurFilterData.GetFlags(FFFT);

		if (Flags)
		{
			if (bFound && !(Flags&FFF_STRONG))
				continue;

			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				DWORD AttrClear;

				if (CurFilterData.GetAttr(nullptr,&AttrClear))
					bAnyFolderIncludeFound = bAnyFolderIncludeFound || !(AttrClear&FILE_ATTRIBUTE_DIRECTORY);
			}

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//авто-фильтр папки
	if (FFFT != FFFT_CUSTOM)
	{
		Flags = FoldersFilter->GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				bAnyFolderIncludeFound = true;
			}

			if (bFolder && FoldersFilter->FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//авто-фильтры
	for (const auto& CurFilterData: TempFilterData())
	{
		Flags = CurFilterData.GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

			if (bFolder) //авто-фильтры никогда не могут быть для папок
				continue;

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//Если папка и она не попала ни под какой exclude фильтр то самое логичное
	//будет сделать ей include если не было дугих include фильтров на папки.
	//А вот Select логичней всего работать чисто по заданному фильтру.
	if (!bFound && bFolder && !bAnyFolderIncludeFound && m_FilterType!=FFT_SELECT)
	{
		if (foundType)
			*foundType=FIFT_INCLUDE; //???

		return true;
	}

final:

	if (foundType)
		*foundType=!bFound?FIFT_NOTINTFILTER:(bInc?FIFT_INCLUDE:FIFT_EXCLUDE);

	if (bFound) return bInc;

	//Если элемент не попал ни под один фильтр то он будет включен
	//только если не было ни одного Include фильтра (т.е. были только фильтры исключения).
	return !bAnyIncludeFound;
}

bool FileFilter::FileInFilter(const PluginPanelItem& fd,enumFileInFilterType *foundType)
{
	os::fs::find_data fde;
	PluginPanelItemToFindDataEx(fd, fde);
	return FileInFilter(fde, foundType, &fde.strFileName);
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

void FileFilter::InitFilter()
{
	string strTitle, strMask, strSizeBelow, strSizeAbove;

	const auto cfg = ConfigProvider().CreateFiltersConfig();
	const auto root = cfg->FindByName(cfg->root_key(), L"Filters");

	{
		static FileFilterParams _FoldersFilter;
		FoldersFilter = &_FoldersFilter;
		FoldersFilter->SetMask(false, L"*");
		FoldersFilter->SetAttr(true, FILE_ATTRIBUTE_DIRECTORY, 0);

		if (!root)
		{
			return;
		}

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(root, L"FoldersFilterFFlags", bytes(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			FoldersFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	for (;;)
	{
		const auto key = cfg->FindByName(root, L"Filter" + str(FilterData().size()));

		if (!key || !cfg->GetValue(key,L"Title",strTitle))
			break;

		FileFilterParams NewItem;

		//Дефолтные значения выбраны так чтоб как можно правильней загрузить
		//настройки старых версий фара.
		NewItem.SetTitle(strTitle);

		strMask.clear();
		cfg->GetValue(key,L"Mask",strMask);
		unsigned long long UseMask = 1;
		cfg->GetValue(key, L"UseMask", UseMask);
		NewItem.SetMask(UseMask != 0, strMask);

		FILETIME DateAfter = {}, DateBefore = {};
		cfg->GetValue(key,L"DateAfter", bytes(DateAfter));
		cfg->GetValue(key,L"DateBefore", bytes(DateBefore));

		unsigned long long UseDate = 0;
		cfg->GetValue(key, L"UseDate", UseDate);
		unsigned long long DateType = 0;
		cfg->GetValue(key, L"DateType", DateType);
		unsigned long long RelativeDate = 0;
		cfg->GetValue(key, L"RelativeDate", RelativeDate);
		NewItem.SetDate(UseDate != 0, static_cast<enumFDateType>(DateType), DateAfter, DateBefore, RelativeDate != 0);

		strSizeAbove.clear();
		cfg->GetValue(key,L"SizeAboveS",strSizeAbove);
		strSizeBelow.clear();
		cfg->GetValue(key,L"SizeBelowS",strSizeBelow);
		unsigned long long UseSize = 0;
		cfg->GetValue(key, L"UseSize", UseSize);
		NewItem.SetSize(UseSize != 0, strSizeAbove, strSizeBelow);

		unsigned long long UseHardLinks = 0;
		cfg->GetValue(key, L"UseHardLinks", UseHardLinks);
		unsigned long long HardLinksAbove = 0;
		cfg->GetValue(key, L"HardLinksAbove", HardLinksAbove);
		unsigned long long HardLinksBelow = 0;
		cfg->GetValue(key, L"HardLinksAbove", HardLinksBelow);
		NewItem.SetHardLinks(UseHardLinks != 0, HardLinksAbove, HardLinksBelow);

		unsigned long long UseAttr = 1;
		cfg->GetValue(key, L"UseAttr", UseAttr);
		unsigned long long AttrSet = 0;
		cfg->GetValue(key, L"AttrSet", AttrSet);
		unsigned long long AttrClear = FILE_ATTRIBUTE_DIRECTORY;
		cfg->GetValue(key, L"AttrClear", AttrClear);
		NewItem.SetAttr(UseAttr != 0, (DWORD)AttrSet, (DWORD)AttrClear);

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", bytes(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			NewItem.SetFlags((enumFileFilterFlagsType)i, Flags[i]);

		FilterData().emplace_back(std::move(NewItem));
	}

	for (;;)
	{
		const auto key = cfg->FindByName(root, L"PanelMask" + str(TempFilterData().size()));

		if (!key || !cfg->GetValue(key,L"Mask",strMask))
			break;

		FileFilterParams NewItem;

		NewItem.SetMask(true, strMask);
		//Авто фильтры они только для файлов, папки не должны к ним подходить
		NewItem.SetAttr(true, 0, FILE_ATTRIBUTE_DIRECTORY);
		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", bytes(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			NewItem.SetFlags((enumFileFilterFlagsType)i, Flags[i]);

		TempFilterData().emplace_back(std::move(NewItem));
	}
}


void FileFilter::CloseFilter()
{
	FilterData().clear();
	TempFilterData().clear();
}

void FileFilter::Save(bool always)
{
	if (!always && !Changed)
		return;

	Changed = false;

	const auto cfg = ConfigProvider().CreateFiltersConfig();

	auto root = cfg->FindByName(cfg->root_key(), L"Filters");
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(cfg->root_key(), L"Filters");
	if (!root)
	{
		return;
	}

	for (size_t i=0; i<FilterData().size(); ++i)
	{
		const auto Key = cfg->CreateKey(root, L"Filter" + str(i));
		if (!Key)
			break;
		const auto& CurFilterData = FilterData()[i];

		cfg->SetValue(Key, L"Title",CurFilterData.GetTitle());
		cfg->SetValue(Key, L"UseMask", CurFilterData.IsMaskUsed());
		cfg->SetValue(Key, L"Mask", CurFilterData.GetMask());
		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		bool bRelative;
		cfg->SetValue(Key, L"UseDate",CurFilterData.GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)? 1 : 0);
		cfg->SetValue(Key, L"DateType", DateType);
		cfg->SetValue(Key, L"DateAfter", bytes_view(DateAfter));
		cfg->SetValue(Key, L"DateBefore", bytes_view(DateBefore));
		cfg->SetValue(Key, L"RelativeDate", bRelative?1:0);
		cfg->SetValue(Key, L"UseSize", CurFilterData.IsSizeUsed());
		cfg->SetValue(Key, L"SizeAboveS", CurFilterData.GetSizeAbove());
		cfg->SetValue(Key, L"SizeBelowS", CurFilterData.GetSizeBelow());
		DWORD HardLinksAbove,HardLinksBelow;
		cfg->SetValue(Key, L"UseHardLinks", CurFilterData.GetHardLinks(&HardLinksAbove,&HardLinksBelow)? 1 : 0);
		cfg->SetValue(Key, L"HardLinksAboveS", HardLinksAbove);
		cfg->SetValue(Key, L"HardLinksBelowS", HardLinksBelow);
		DWORD AttrSet, AttrClear;
		cfg->SetValue(Key, L"UseAttr", CurFilterData.GetAttr(&AttrSet, &AttrClear)? 1 : 0);
		cfg->SetValue(Key, L"AttrSet", AttrSet);
		cfg->SetValue(Key, L"AttrClear", AttrClear);
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData.GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(Key, L"FFlags", bytes_view(Flags));
	}

	for (size_t i=0; i<TempFilterData().size(); ++i)
	{
		const auto Key = cfg->CreateKey(root, L"PanelMask" + str(i));
		if (!Key)
			break;
		const auto& CurFilterData = TempFilterData()[i];

		cfg->SetValue(Key, L"Mask", CurFilterData.GetMask());
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData.GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(Key, L"FFlags", bytes_view(Flags));
	}

	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = FoldersFilter->GetFlags((enumFileFilterFlagsType)i);

		cfg->SetValue(root,L"FoldersFilterFFlags", bytes_view(Flags));
	}
}

void FileFilter::SwapPanelFlags(FileFilterParams& CurFilterData)
{
	DWORD LPFlags = CurFilterData.GetFlags(FFFT_LEFTPANEL);
	DWORD RPFlags = CurFilterData.GetFlags(FFFT_RIGHTPANEL);
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
	if (FileName == L"." || TestParentFolderName(FileName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		return -1;

	const auto DotPos = FileName.rfind(L'.');
	string strMask;

	if (DotPos == string::npos)
	{
		strMask = L"*.";
	}
	else
	{
		const auto Ext = make_string_view(FileName, DotPos);
		// Если маска содержит разделитель (',' или ';'), то возьмем ее в кавычки
		strMask = FileName.find_first_of(L",;", DotPos) != string::npos? concat(L"\"*"_sv, Ext, L'"') : concat(L'*', Ext);
	}

	if (std::any_of(CONST_RANGE(Extensions, i) { return equal_icase(i.first, strMask); }))
		return -1;

	Extensions.emplace_back(strMask, Check);
	return 1;
}
