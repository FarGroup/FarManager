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
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
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

static std::vector<FileFilterParams*> *FilterData, *TempFilterData;

FileFilterParams *FoldersFilter;

static bool bMenuOpen = false;

static bool Changed = false;

FileFilter::FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType):
	m_HostPanel(HostPanel),
	m_FilterType(FilterType)
{
	UpdateCurrentTime();
}

FileFilter::~FileFilter()
{
}

Panel *FileFilter::GetHostPanel()
{
	if (!m_HostPanel || m_HostPanel == (Panel *)PANEL_ACTIVE)
	{
		return Global->CtrlObject->Cp()->ActivePanel;
	}
	else if (m_HostPanel == (Panel *)PANEL_PASSIVE)
	{
		return Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel);
	}

	return m_HostPanel;
}

bool FileFilter::FilterEdit()
{
	if (bMenuOpen)
		return false;

	Changed = true;
	bMenuOpen = true;
	MenuItemEx ListItem;
	int ExitCode;
	bool bNeedUpdate=false;
	VMenu2 FilterList(MSG(MFilterTitle),nullptr,0,ScrY-6);
	FilterList.SetHelp(L"FiltersMenu");
	FilterList.SetPosition(-1,-1,0,0);
	FilterList.SetBottomTitle(MSG(MFilterBottom));
	FilterList.SetFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);

	bool first = true;
	std::for_each(CONST_RANGE(*FilterData, i)
	{
		ListItem.Clear();
		MenuString(ListItem.strName, i);

		if (first)
		{
			first = false;
			ListItem.Flags|=LIF_SELECTED;
		}
		int Check = GetCheck(i);

		if (Check)
			ListItem.SetCheck(Check);

		FilterList.AddItem(&ListItem);
	});

	if (m_FilterType != FFT_CUSTOM)
	{
		std::list<std::pair<string, int>> Extensions;
		{
			enumFileFilterFlagsType FFFT = GetFFFT();

			FOR_CONST_RANGE(*TempFilterData, i)
			{
				//AY: Будем показывать только те выбранные авто фильтры
				//(для которых нету файлов на панели) которые выбраны в области данного меню
				if (!(*i)->GetFlags(FFFT))
					continue;

				const wchar_t *FMask;
				(*i)->GetMask(&FMask);
				string strMask = FMask;
				Unquote(strMask);

				if (!ParseAndAddMasks(Extensions, strMask, 0, GetCheck(*i)))
					break;
			}
		}
		ListItem.Clear();
		ListItem.Flags|=LIF_SEPARATOR;
		FilterList.AddItem(&ListItem);
		ListItem.Clear();
		FoldersFilter->SetTitle(MSG(MFolderFileType));
		MenuString(ListItem.strName,FoldersFilter,false,L'0');
		int Check = GetCheck(FoldersFilter);

		if (Check)
			ListItem.SetCheck(Check);

		FilterList.AddItem(&ListItem);

		if (GetHostPanel()->GetMode()==NORMAL_PANEL)
		{
			string strFileName;
			FAR_FIND_DATA fdata;
			ScanTree ScTree(FALSE,FALSE);
			ScTree.SetFindPath(GetHostPanel()->GetCurDir(), L"*");

			while (ScTree.GetNextName(&fdata,strFileName))
				if (!ParseAndAddMasks(Extensions, fdata.strFileName, fdata.dwFileAttributes, 0))
					break;
		}
		else
		{
			string strFileName;
			DWORD FileAttr;

			for (int i=0; GetHostPanel()->GetFileName(strFileName,i,FileAttr); i++)
				if (!ParseAndAddMasks(Extensions, strFileName, FileAttr, 0))
					break;
		}

		Extensions.sort();
		ListItem.Clear();

		wchar_t h = L'1';
		for (auto i = Extensions.begin(); i != Extensions.end(); ++i, (h == L'9'? h = L'A' : (h == L'Z' || h? h++ : h=0)))
		{
			MenuString(ListItem.strName, nullptr, false, h, true, i->first, MSG(MPanelFileType));
			size_t Length = i->first.GetLength() + 1;
			ListItem.SetCheck(i->second);
			FilterList.SetUserData(i->first.CPtr(), Length * sizeof(wchar_t), FilterList.AddItem(&ListItem));
		}
	}

	ExitCode=FilterList.RunEx([&](int Msg, void *param)->int
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
				int SelPos=FilterList.GetSelectPos();

				if (SelPos<0)
					break;

				int Check=FilterList.GetCheck(SelPos);
				int NewCheck;

				if (Key==KEY_BS)
					NewCheck = 0;
				else if (Key==KEY_SPACE)
					NewCheck = Check ? 0 : L'+';
				else
					NewCheck = (Check == Key) ? 0 : Key;

				FilterList.SetCheck(NewCheck,SelPos);
				FilterList.SetSelectPos(SelPos,1);
				FilterList.Key(KEY_DOWN);
				bNeedUpdate=true;
				return 1;
			}
			case KEY_SHIFTBS:
			{
				for (int I=0; I < FilterList.GetItemCount(); I++)
				{
					FilterList.SetCheck(FALSE, I);
				}

				break;
			}
			case KEY_F4:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size())
				{
					if (FileFilterConfig(FilterData->at(SelPos)))
					{
						ListItem.Clear();
						MenuString(ListItem.strName,FilterData->at(SelPos));
						int Check = GetCheck(FilterData->at(SelPos));

						if (Check)
							ListItem.SetCheck(Check);

						FilterList.DeleteItem(SelPos);
						FilterList.AddItem(&ListItem,SelPos);
						FilterList.SetSelectPos(SelPos,1);
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData->size())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_NUMPAD0:
			case KEY_INS:
			case KEY_F5:
			{
				int pos=FilterList.GetSelectPos();
				if (pos<0)
					break;
				size_t SelPos=pos;
				size_t SelPos2=pos+1;

				SelPos = std::min(FilterData->size(), SelPos);

				auto NewFilter = new FileFilterParams;
				FilterData->insert(FilterData->begin()+SelPos, NewFilter);

				if (Key==KEY_F5)
				{
					if (SelPos2 < FilterData->size())
					{
						*NewFilter = *FilterData->at(SelPos2);
						NewFilter->SetTitle(L"");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 == FilterData->size()+2)
					{
						*NewFilter = *FoldersFilter;
						NewFilter->SetTitle(L"");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 > FilterData->size()+2)
					{
						NewFilter->SetMask(1,static_cast<const wchar_t*>(FilterList.GetUserData(nullptr, 0, static_cast<int>(SelPos2-1))));
						//Авто фильтры они только для файлов, папки не должны к ним подходить
						NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					}
					else
					{
						delete NewFilter;
						FilterData->erase(FilterData->begin()+SelPos);
						break;
					}
				}
				else
				{
					//AY: Раз создаём новый фильтр то думаю будет логично если он будет только для файлов
					NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
				}

				if (FileFilterConfig(NewFilter))
				{
					ListItem.Clear();
					MenuString(ListItem.strName,NewFilter);
					FilterList.AddItem(&ListItem,static_cast<int>(SelPos));
					FilterList.SetSelectPos(static_cast<int>(SelPos),1);
					bNeedUpdate=true;
				}
				else
				{
					delete NewFilter;
					FilterData->erase(FilterData->begin()+SelPos);
				}
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size())
				{
					string strQuotedTitle=FilterData->at(SelPos)->GetTitle();
					InsertQuote(strQuotedTitle);

					if (!Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
					            strQuotedTitle,MSG(MDelete),MSG(MCancel)))
					{
						auto i = FilterData->begin()+SelPos;
						delete *i;
						FilterData->erase(i);
						FilterList.DeleteItem(SelPos);
						FilterList.SetSelectPos(SelPos,1);
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData->size())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_CTRLUP:
			case KEY_RCTRLUP:
			case KEY_CTRLDOWN:
			case KEY_RCTRLDOWN:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size() && !((Key==KEY_CTRLUP || Key==KEY_RCTRLUP) && !SelPos) &&
					!((Key==KEY_CTRLDOWN || Key==KEY_RCTRLDOWN) && SelPos==(int)(FilterData->size()-1)))
				{
					int NewPos = SelPos + ((Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN) ? 1 : -1);
					MenuItemEx CurItem  = *FilterList.GetItemPtr(SelPos);
					MenuItemEx NextItem = *FilterList.GetItemPtr(NewPos);
					auto i1 = FilterData->begin() + NewPos, i2 = FilterData->begin() + SelPos;
					std::swap(*i1, *i2);

					if (NewPos<SelPos)
					{
						FilterList.DeleteItem(NewPos,2);
						FilterList.AddItem(&CurItem,NewPos);
						FilterList.AddItem(&NextItem,SelPos);
					}
					else
					{
						FilterList.DeleteItem(SelPos,2);
						FilterList.AddItem(&NextItem,SelPos);
						FilterList.AddItem(&CurItem,NewPos);
					}

					FilterList.SetSelectPos(NewPos,1);
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
		ProcessSelection(&FilterList);

	if (Global->Opt->AutoSaveSetup)
		SaveFilters();

	if (ExitCode!=-1 || bNeedUpdate)
	{
		if (m_FilterType == FFT_PANEL)
		{
			GetHostPanel()->Update(UPDATE_KEEP_SELECTION);
			GetHostPanel()->Redraw();
		}
	}

	bMenuOpen = false;
	return (ExitCode!=-1);
}

enumFileFilterFlagsType FileFilter::GetFFFT()
{
	if (m_FilterType == FFT_PANEL)
	{
		if (GetHostPanel() == Global->CtrlObject->Cp()->RightPanel)
		{
			return FFFT_RIGHTPANEL;
		}
		else
		{
			return FFFT_LEFTPANEL;
		}
	}
	else if (m_FilterType == FFT_COPY)
	{
		return FFFT_COPY;
	}
	else if (m_FilterType == FFT_FINDFILE)
	{
		return FFFT_FINDFILE;
	}
	else if (m_FilterType == FFT_SELECT)
	{
		return FFFT_SELECT;
	}
	return FFFT_CUSTOM;
}

int FileFilter::GetCheck(FileFilterParams *FFP)
{
	DWORD Flags = FFP->GetFlags(GetFFFT());

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

void FileFilter::ProcessSelection(VMenu2 *FilterList)
{
	enumFileFilterFlagsType FFFT = GetFFFT();
	FileFilterParams *CurFilterData;

	for (int i=0,j=0; i < FilterList->GetItemCount(); i++)
	{
		int Check=FilterList->GetCheck(i);
		CurFilterData=nullptr;

		if (i < (int)FilterData->size())
		{
			CurFilterData = FilterData->at(i);
		}
		else if (i == (int)(FilterData->size() + 1))
		{
			CurFilterData = FoldersFilter;
		}
		else if (i > (int)(FilterData->size() + 1))
		{
			const wchar_t *FMask=nullptr;
			string Mask(static_cast<const wchar_t*>(FilterList->GetUserData(nullptr, 0, i)));
			string strMask1(Mask);
			//AY: Так как в меню мы показываем только те выбранные авто фильтры
			//которые выбраны в области данного меню и TempFilterData вполне
			//может содержать маску которую тока что выбрали в этом меню но
			//она уже была выбрана в другом и так как TempFilterData
			//и авто фильтры в меню отсортированы по алфавиту то немного
			//поколдуем чтоб не было дубликатов в памяти.
			Unquote(strMask1);

			while (j < static_cast<int>(TempFilterData->size()))
			{
				CurFilterData = TempFilterData->at(j);
				string strMask2;
				CurFilterData->GetMask(&FMask);
				strMask2 = FMask;
				Unquote(strMask2);

				if (StrCmpI(strMask1,strMask2)<1)
					break;

				j++;
			}

			if (CurFilterData)
			{
				if (!StrCmpI(Mask,FMask))
				{
					if (!Check)
					{
						bool bCheckedNowhere = true;

						for (int n=FFFT_FIRST; n < FFFT_COUNT; n++)
						{
							if (n != FFFT && CurFilterData->GetFlags((enumFileFilterFlagsType)n))
							{
								bCheckedNowhere = false;
								break;
							}
						}

						if (bCheckedNowhere)
						{
							auto i = TempFilterData->begin()+j;
							delete *i;
							TempFilterData->erase(i);
							continue;
						}
					}
					else
					{
						j++;
					}
				}
				else
					CurFilterData=nullptr;
			}

			if (Check && !CurFilterData)
			{
				auto NewFilter = new FileFilterParams;
				TempFilterData->insert(TempFilterData->begin() + j, NewFilter);

					NewFilter->SetMask(1,Mask);
					//Авто фильтры они только для файлов, папки не должны к ним подходить
					NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					j++;
					CurFilterData = NewFilter;
			}
		}

		if (!CurFilterData)
			continue;

		CurFilterData->SetFlags(FFFT, FFF_NONE);

		if (Check==L'+')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE);
		else if (Check==L'-')
			CurFilterData->SetFlags(FFFT, FFF_EXCLUDE);
		else if (Check==L'I')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE|FFF_STRONG);
		else if (Check==L'X')
			CurFilterData->SetFlags(FFFT, FFF_EXCLUDE|FFF_STRONG);
	}
}

void FileFilter::UpdateCurrentTime()
{
	SYSTEMTIME cst;
	FILETIME cft;
	GetSystemTime(&cst);
	SystemTimeToFileTime(&cst, &cft);
	ULARGE_INTEGER current = {cft.dwLowDateTime, cft.dwHighDateTime};
	CurrentTime = current.QuadPart;
}

bool FileFilter::FileInFilter(const FileListItem& fli,enumFileInFilterType *foundType)
{
	FAR_FIND_DATA fde;
	fde.dwFileAttributes=fli.FileAttr;
	fde.ftCreationTime=fli.CreationTime;
	fde.ftLastAccessTime=fli.AccessTime;
	fde.ftLastWriteTime=fli.WriteTime;
	fde.ftChangeTime=fli.ChangeTime;
	fde.nFileSize=fli.FileSize;
	fde.nAllocationSize=fli.AllocationSize;
	fde.strFileName=fli.strName;
	fde.strAlternateFileName=fli.strShortName;
	return FileInFilter(fde, foundType, &fli.strName);
}

bool FileFilter::FileInFilter(const FAR_FIND_DATA& fde,enumFileInFilterType *foundType, const string* FullName)
{
	enumFileFilterFlagsType FFFT = GetFFFT();
	bool bFound=false;
	bool bAnyIncludeFound=false;
	bool bAnyFolderIncludeFound=false;
	bool bInc=false;
	bool bFolder=(fde.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
	FileFilterParams *CurFilterData;
	DWORD Flags;

	for (size_t i=0; i<FilterData->size(); i++)
	{
		CurFilterData = FilterData->at(i);
		Flags = CurFilterData->GetFlags(FFFT);

		if (Flags)
		{
			if (bFound && !(Flags&FFF_STRONG))
				continue;

			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				DWORD AttrClear;

				if (CurFilterData->GetAttr(nullptr,&AttrClear))
					bAnyFolderIncludeFound = bAnyFolderIncludeFound || !(AttrClear&FILE_ATTRIBUTE_DIRECTORY);
			}

			if (CurFilterData->FileInFilter(fde, CurrentTime, FullName))
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
	for (size_t i=0; i<TempFilterData->size(); i++)
	{
		CurFilterData = TempFilterData->at(i);
		Flags = CurFilterData->GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

			if (bFolder) //авто-фильтры никогда не могут быть для папок
				continue;

			if (CurFilterData->FileInFilter(fde, CurrentTime, FullName))
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
	//будет сделать ей include если небыло дугих include фильтров на папки.
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
	FAR_FIND_DATA fde;
	PluginPanelItemToFindDataEx(&fd,&fde);
	return FileInFilter(fde, foundType, &fde.strFileName);
}

bool FileFilter::IsEnabledOnPanel()
{
	if (m_FilterType != FFT_PANEL)
		return false;

	enumFileFilterFlagsType FFFT = GetFFFT();

	for (size_t i=0; i<FilterData->size(); i++)
	{
		if (FilterData->at(i)->GetFlags(FFFT))
			return true;
	}

	if (FoldersFilter->GetFlags(FFFT))
		return true;

	for (size_t i=0; i<TempFilterData->size(); i++)
	{
		if (TempFilterData->at(i)->GetFlags(FFFT))
			return true;
	}

	return false;
}

void FileFilter::InitFilter()
{
	if(!FilterData)
		FilterData = new std::vector<FileFilterParams*>;
	if(!TempFilterData)
		TempFilterData = new std::vector<FileFilterParams*>;

	string strKeyName;
	string strTitle, strMask, strSizeBelow, strSizeAbove;

	HierarchicalConfig *cfg = Global->Db->CreateFiltersConfig();

	unsigned __int64 root = cfg->GetKeyID(0, L"Filters");

	{
		static FileFilterParams _FoldersFilter;
		FoldersFilter = &_FoldersFilter;
		FoldersFilter->SetMask(0,L"");
		FoldersFilter->SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);

		if (!root)
		{
			delete cfg;
			return;
		}

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(root,L"FoldersFilterFFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			FoldersFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	while (1)
	{
		strKeyName = FormatString() << "Filter" << FilterData->size();

		unsigned __int64 key = cfg->GetKeyID(root, strKeyName);

		if (!key || !cfg->GetValue(key,L"Title",strTitle))
			break;

		auto NewFilter = new FileFilterParams;
		FilterData->push_back(NewFilter);

		//Дефолтные значения выбраны так чтоб как можно правильней загрузить
		//настройки старых версий фара.
		NewFilter->SetTitle(strTitle);

		strMask.Clear();
		cfg->GetValue(key,L"Mask",strMask);
		unsigned __int64 UseMask = 1;
		cfg->GetValue(key,L"UseMask",&UseMask);
		NewFilter->SetMask(UseMask!=0, strMask);

		FILETIME DateAfter = {}, DateBefore = {};
		cfg->GetValue(key,L"DateAfter", &DateAfter, sizeof(DateAfter));
		cfg->GetValue(key,L"DateBefore", &DateBefore, sizeof(DateBefore));

		unsigned __int64 UseDate = 0;
		cfg->GetValue(key,L"UseDate",&UseDate);
		unsigned __int64 DateType = 0;
		cfg->GetValue(key,L"DateType",&DateType);
		unsigned __int64 RelativeDate = 0;
		cfg->GetValue(key,L"RelativeDate",&RelativeDate);
		NewFilter->SetDate(UseDate!=0, (DWORD)DateType, DateAfter, DateBefore, RelativeDate!=0);

		strSizeAbove.Clear();
		cfg->GetValue(key,L"SizeAboveS",strSizeAbove);
		strSizeBelow.Clear();
		cfg->GetValue(key,L"SizeBelowS",strSizeBelow);
		unsigned __int64 UseSize = 0;
		cfg->GetValue(key,L"UseSize",&UseSize);
		NewFilter->SetSize(UseSize!=0, strSizeAbove, strSizeBelow);

		unsigned __int64 UseHardLinks = 0;
		cfg->GetValue(key,L"UseHardLinks",&UseHardLinks);
		unsigned __int64 HardLinksAbove;
		cfg->GetValue(key,L"HardLinksAbove",&HardLinksAbove);
		unsigned __int64 HardLinksBelow;
		cfg->GetValue(key,L"HardLinksAbove",&HardLinksBelow);
		NewFilter->SetHardLinks(UseHardLinks!=0,HardLinksAbove,HardLinksBelow);

		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key,L"UseAttr",&UseAttr);
		unsigned __int64 AttrSet = 0;
		cfg->GetValue(key,L"AttrSet", &AttrSet);
		unsigned __int64 AttrClear = FILE_ATTRIBUTE_DIRECTORY;
		cfg->GetValue(key,L"AttrClear",&AttrClear);
		NewFilter->SetAttr(UseAttr!=0, (DWORD)AttrSet, (DWORD)AttrClear);

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	while (1)
	{
		strKeyName = FormatString() << L"PanelMask" << TempFilterData->size();

		unsigned __int64 key = cfg->GetKeyID(root, strKeyName);

		if (!key || !cfg->GetValue(key,L"Mask",strMask))
			break;

		auto NewFilter = new FileFilterParams;
		TempFilterData->push_back(NewFilter);

		NewFilter->SetMask(1,strMask);
		//Авто фильтры они только для файлов, папки не должны к ним подходить
		NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	delete cfg;
}


void FileFilter::CloseFilter()
{
	if(FilterData)
	{
		DeleteValues(*FilterData);
		delete FilterData;
		FilterData = nullptr;
	}

	if(TempFilterData)
	{
		DeleteValues(*TempFilterData);
		delete TempFilterData;
		TempFilterData = nullptr;
	}
}

void FileFilter::SaveFilters()
{
	if (!Changed)
		return;

	Changed = false;

	string strKeyName;
	FileFilterParams *CurFilterData;
	HierarchicalConfig *cfg = Global->Db->CreateFiltersConfig();

	unsigned __int64 root = cfg->GetKeyID(0, L"Filters");
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(0, L"Filters");

	if (!root)
	{
		delete cfg;
		return;
	}

	for (size_t i=0; i<FilterData->size(); i++)
	{
		strKeyName = FormatString() << L"Filter" << i;
		unsigned __int64 key = cfg->CreateKey(root, strKeyName);
		if (!key)
			break;
		CurFilterData = FilterData->at(i);

		cfg->SetValue(key,L"Title",CurFilterData->GetTitle());
		const wchar_t *Mask;
		cfg->SetValue(key,L"UseMask",CurFilterData->GetMask(&Mask)?1:0);
		cfg->SetValue(key,L"Mask",Mask);
		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		bool bRelative;
		cfg->SetValue(key,L"UseDate",CurFilterData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
		cfg->SetValue(key,L"DateType",DateType);
		cfg->SetValue(key,L"DateAfter", &DateAfter, sizeof(DateAfter));
		cfg->SetValue(key,L"DateBefore", &DateBefore, sizeof(DateBefore));
		cfg->SetValue(key,L"RelativeDate",bRelative?1:0);
		const wchar_t *SizeAbove, *SizeBelow;
		cfg->SetValue(key,L"UseSize",CurFilterData->GetSize(&SizeAbove, &SizeBelow)?1:0);
		cfg->SetValue(key,L"SizeAboveS",SizeAbove);
		cfg->SetValue(key,L"SizeBelowS",SizeBelow);
		DWORD HardLinksAbove,HardLinksBelow;
		cfg->SetValue(key,L"UseHardLinks",CurFilterData->GetHardLinks(&HardLinksAbove,&HardLinksBelow)?1:0);
		cfg->SetValue(key,L"HardLinksAboveS",SizeAbove);
		cfg->SetValue(key,L"HardLinksBelowS",SizeBelow);
		DWORD AttrSet, AttrClear;
		cfg->SetValue(key,L"UseAttr",CurFilterData->GetAttr(&AttrSet, &AttrClear)?1:0);
		cfg->SetValue(key,L"AttrSet",AttrSet);
		cfg->SetValue(key,L"AttrClear",AttrClear);
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData->GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(key,L"FFlags", Flags, sizeof(Flags));
	}

	for (size_t i=0; i<TempFilterData->size(); i++)
	{
		strKeyName = FormatString() << L"PanelMask" << i;
		unsigned __int64 key = cfg->CreateKey(root, strKeyName);
		if (!key)
			break;
		CurFilterData = TempFilterData->at(i);

		const wchar_t *Mask;
		CurFilterData->GetMask(&Mask);
		cfg->SetValue(key,L"Mask",Mask);
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData->GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(key,L"FFlags", Flags, sizeof(Flags));
	}

	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = FoldersFilter->GetFlags((enumFileFilterFlagsType)i);

		cfg->SetValue(root,L"FoldersFilterFFlags", Flags, sizeof(Flags));
	}

	delete cfg;
}

void FileFilter::SwapPanelFlags(FileFilterParams *CurFilterData)
{
	DWORD LPFlags = CurFilterData->GetFlags(FFFT_LEFTPANEL);
	DWORD RPFlags = CurFilterData->GetFlags(FFFT_RIGHTPANEL);
	CurFilterData->SetFlags(FFFT_LEFTPANEL,  RPFlags);
	CurFilterData->SetFlags(FFFT_RIGHTPANEL, LPFlags);
}

void FileFilter::SwapFilter()
{
	Changed = true;

	for (size_t i=0; i<FilterData->size(); i++)
		SwapPanelFlags(FilterData->at(i));

	SwapPanelFlags(FoldersFilter);

	for (size_t i=0; i<TempFilterData->size(); i++)
		SwapPanelFlags(TempFilterData->at(i));
}

int FileFilter::ParseAndAddMasks(std::list<std::pair<string, int>>& Extensions, const wchar_t *FileName, DWORD FileAttr, int Check)
{
	if (!StrCmp(FileName,L".") || TestParentFolderName(FileName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		return -1;

	const wchar_t *DotPtr=wcsrchr(FileName,L'.');
	string strMask;

	// Если маска содержит разделитель (',' или ';'), то возьмем ее в кавычки
	if (!DotPtr)
		strMask = L"*.";
	else if (wcspbrk(DotPtr,L",;"))
		strMask = FormatString() << L"\"*" << DotPtr << '"';
	else
		strMask = FormatString() << L'*' << DotPtr;

	if (std::find_if(Extensions.begin(), Extensions.end(), [&strMask](const VALUE_TYPE(Extensions)& i)
	{
		return !StrCmpI(strMask, i.first);
	}) != Extensions.end())
		return -1;

	Extensions.push_back(VALUE_TYPE(Extensions)(strMask, Check));
	return 1;
}
