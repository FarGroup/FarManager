/*
filefilter.cpp

Файловый фильтр
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

#include "headers.hpp"
#pragma hdrstop

#include "filefilter.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "scantree.hpp"
#include "array.hpp"
#include "filelist.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"

static int _cdecl ExtSort(const void *el1,const void *el2);

static TPointerArray<FileFilterParams> FilterData, TempFilterData;
static FileFilterParams FoldersFilter;

static bool bMenuOpen = false;

FileFilter::FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType)
{
	m_HostPanel=HostPanel;
	m_FilterType=FilterType;
	UpdateCurrentTime();
}

FileFilter::~FileFilter()
{
}

Panel *FileFilter::GetHostPanel()
{
	if (m_HostPanel == (Panel *)PANEL_ACTIVE)
	{
		return CtrlObject->Cp()->ActivePanel;
	}
	else if (m_HostPanel == (Panel *)PANEL_PASSIVE)
	{
		return CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
	}

	return m_HostPanel;
}

bool FileFilter::FilterEdit()
{
	if (bMenuOpen)
		return false;

	bMenuOpen = true;
	MenuItemEx ListItem;
	int ExitCode;
	bool bNeedUpdate=false;
	VMenu FilterList(MSG(MFilterTitle),NULL,0,ScrY-6);
	FilterList.SetHelp(L"FiltersMenu");
	FilterList.SetPosition(-1,-1,0,0);
	FilterList.SetBottomTitle(MSG(MFilterBottom));
	FilterList.SetFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		ListItem.Clear();
		MenuString(ListItem.strName,FilterData.getItem(i));

		if (i == 0)
			ListItem.Flags|=LIF_SELECTED;

		int Check = GetCheck(FilterData.getItem(i));

		if (Check)
			ListItem.SetCheck(Check);

		FilterList.AddItem(&ListItem);
	}

	ListItem.Clear();

	if (FilterData.getCount()==0)
		ListItem.Flags|=LIF_SELECTED;

	FilterList.AddItem(&ListItem);
	wchar_t *ExtPtr=NULL;
	int ExtCount=0;
	{
		enumFileFilterFlagsType FFFT = GetFFFT();

		for (unsigned int i=0; i<TempFilterData.getCount(); i++)
		{
			//AY: Будем показывать только те выбранные авто фильтры
			//(для которых нету файлов на панели) которые выбраны в области данного меню
			if (!TempFilterData.getItem(i)->GetFlags(FFFT))
				continue;

			const wchar_t *FMask;
			TempFilterData.getItem(i)->GetMask(&FMask);
			string strMask = FMask;
			Unquote(strMask);

			if (!ParseAndAddMasks(&ExtPtr,strMask,0,ExtCount,GetCheck(TempFilterData.getItem(i))))
				break;
		}
	}
	ListItem.Clear();
	ListItem.Flags|=LIF_SEPARATOR;
	FilterList.AddItem(&ListItem);
	ListItem.Clear();
	FoldersFilter.SetTitle(MSG(MFolderFileType));
	MenuString(ListItem.strName,&FoldersFilter,false,L'0');
	int Check = GetCheck(&FoldersFilter);

	if (Check)
		ListItem.SetCheck(Check);

	FilterList.AddItem(&ListItem);

	if (GetHostPanel()->GetMode()==NORMAL_PANEL)
	{
		string strCurDir, strFileName;
		FAR_FIND_DATA_EX fdata;
		GetHostPanel()->GetCurDir(strCurDir);
		ScanTree ScTree(FALSE,FALSE);
		ScTree.SetFindPath(strCurDir,L"*");

		while (ScTree.GetNextName(&fdata,strFileName))
			if (!ParseAndAddMasks(&ExtPtr,fdata.strFileName,fdata.dwFileAttributes,ExtCount,0))
				break;
	}
	else
	{
		string strFileName;
		DWORD FileAttr;

		for (int i=0; GetHostPanel()->GetFileName(strFileName,i,FileAttr); i++)
			if (!ParseAndAddMasks(&ExtPtr,strFileName,FileAttr,ExtCount,0))
				break;
	}

	far_qsort((void *)ExtPtr,ExtCount,NM*sizeof(wchar_t),ExtSort);
	ListItem.Clear();

	for (int i=0, h=L'1'; i<ExtCount; i++, (h==L'9'?h=L'A':(h==L'Z'||h==0?h=0:h++)))
	{
		wchar_t *CurExtPtr=ExtPtr+i*NM;
		MenuString(ListItem.strName,NULL,false,h,true,CurExtPtr,MSG(MPanelFileType));
		ListItem.SetCheck(CurExtPtr[StrLength(CurExtPtr)+1]);
		FilterList.SetUserData(CurExtPtr,0,FilterList.AddItem(&ListItem));
	}

	xf_free(ExtPtr);
	FilterList.Show();

	while (!FilterList.Done())
	{
		int Key=FilterList.ReadInput();

		if (Key==KEY_ADD)
			Key=L'+';
		else if (Key==KEY_SUBTRACT)
			Key=L'-';
		else if (Key==L'i')
			Key=L'I';
		else if (Key==L'x')
			Key=L'X';

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

				if (SelPos==(int)FilterData.getCount())
					break;

				int Check=FilterList.GetSelection(SelPos);
				int NewCheck;

				if (Key==KEY_BS)
					NewCheck = 0;
				else if (Key==KEY_SPACE)
					NewCheck = Check ? 0 : L'+';
				else
					NewCheck = (Check == Key) ? 0 : Key;

				FilterList.SetSelection(NewCheck,SelPos);
				FilterList.SetSelectPos(SelPos,1);
				FilterList.SetUpdateRequired(TRUE);
				FilterList.FastShow();
				FilterList.ProcessKey(KEY_DOWN);
				break;
			}
			case KEY_SHIFTBS:
			{
				for (int I=0; I < FilterList.GetItemCount(); I++)
				{
					FilterList.SetSelection(FALSE, I);
				}

				FilterList.SetUpdateRequired(TRUE);
				FilterList.FastShow();
				break;
			}
			case KEY_F4:
			{
				int SelPos=FilterList.GetSelectPos();

				if (SelPos<(int)FilterData.getCount())
				{
					if (FileFilterConfig(FilterData.getItem(SelPos)))
					{
						ListItem.Clear();
						MenuString(ListItem.strName,FilterData.getItem(SelPos));
						int Check = GetCheck(FilterData.getItem(SelPos));

						if (Check)
							ListItem.SetCheck(Check);

						FilterList.DeleteItem(SelPos);
						FilterList.AddItem(&ListItem,SelPos);
						FilterList.AdjustSelectPos();
						FilterList.SetSelectPos(SelPos,1);
						FilterList.SetUpdateRequired(TRUE);
						FilterList.FastShow();
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData.getCount())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_NUMPAD0:
			case KEY_INS:
			case KEY_F5:
			{
				int SelPos=FilterList.GetSelectPos();
				int SelPos2=SelPos+1;

				if (SelPos>(int)FilterData.getCount())
					SelPos=FilterData.getCount();

				FileFilterParams *NewFilter = FilterData.insertItem(SelPos);

				if (!NewFilter)
					break;

				if (Key==KEY_F5)
				{
					if (SelPos2 < (int)FilterData.getCount())
					{
						*NewFilter = *FilterData.getItem(SelPos2);
						NewFilter->SetTitle(L"");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 == (int)(FilterData.getCount()+2))
					{
						*NewFilter = FoldersFilter;
						NewFilter->SetTitle(L"");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 > (int)(FilterData.getCount()+2))
					{
						wchar_t Mask[NM];
						FilterList.GetUserData(Mask,sizeof(Mask),SelPos2-1);
						NewFilter->SetMask(1,Mask);
						//Авто фильтры они только для файлов, папки не должны к ним подходить
						NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					}
					else
					{
						FilterData.deleteItem(SelPos);
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
					FilterList.AddItem(&ListItem,SelPos);
					FilterList.AdjustSelectPos();
					FilterList.SetSelectPos(SelPos,1);
					FilterList.SetPosition(-1,-1,0,0);
					FilterList.Show();
					bNeedUpdate=true;
				}
				else
					FilterData.deleteItem(SelPos);

				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				int SelPos=FilterList.GetSelectPos();

				if (SelPos<(int)FilterData.getCount())
				{
					string strQuotedTitle=FilterData.getItem(SelPos)->GetTitle();
					InsertQuote(strQuotedTitle);

					if (Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
					            strQuotedTitle,MSG(MDelete),MSG(MCancel))==0)
					{
						FilterData.deleteItem(SelPos);
						FilterList.DeleteItem(SelPos);
						FilterList.AdjustSelectPos();
						FilterList.SetSelectPos(SelPos,1);
						FilterList.SetPosition(-1,-1,0,0);
						FilterList.Show();
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData.getCount())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_CTRLUP:
			case KEY_CTRLDOWN:
			{
				int SelPos=FilterList.GetSelectPos();

				if (SelPos<(int)FilterData.getCount() && !(Key==KEY_CTRLUP && SelPos==0) && !(Key==KEY_CTRLDOWN && SelPos==(int)(FilterData.getCount()-1)))
				{
					int NewPos = SelPos + (Key == KEY_CTRLDOWN ? 1 : -1);
					MenuItemEx CurItem  = *FilterList.GetItemPtr(SelPos);
					MenuItemEx NextItem = *FilterList.GetItemPtr(NewPos);
					FilterData.swapItems(NewPos,SelPos);

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

					FilterList.AdjustSelectPos();
					FilterList.SetSelectPos(NewPos,1);
					FilterList.SetUpdateRequired(TRUE);
					FilterList.FastShow();
					bNeedUpdate=true;
				}

				break;
			}
			default:
			{
				FilterList.ProcessInput();

				//заставляем хоткеи позиционировать курсор на пункте но не закрывать меню
				if (Key!=KEY_NUMENTER && Key!=KEY_ENTER && Key!=KEY_ESC && Key!=KEY_F10 && (IsAlphaNum(Key) || Key&(KEY_ALT|KEY_RALT)))
					FilterList.ClearDone();
			}
		}
	}

	ExitCode=FilterList.Modal::GetExitCode();

	if (ExitCode!=-1)
		ProcessSelection(&FilterList);

	if (Opt.AutoSaveSetup)
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
		if (GetHostPanel() == CtrlObject->Cp()->RightPanel)
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

//  else if (m_FilterType == FFT_SELECT)
//  {
	return FFFT_SELECT;
//  }
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

void FileFilter::ProcessSelection(VMenu *FilterList)
{
	enumFileFilterFlagsType FFFT = GetFFFT();
	FileFilterParams *CurFilterData;

	for (int i=0,j=0; i < FilterList->GetItemCount(); i++)
	{
		int Check=FilterList->GetSelection(i);
		CurFilterData=NULL;

		if (i < (int)FilterData.getCount())
		{
			CurFilterData = FilterData.getItem(i);
		}
		else if (i == (int)(FilterData.getCount() + 2))
		{
			CurFilterData = &FoldersFilter;
		}
		else if (i > (int)(FilterData.getCount() + 2))
		{
			const wchar_t *FMask;
			wchar_t Mask[NM];
			string strMask1;
			FilterList->GetUserData(Mask,sizeof(Mask),i);
			//AY: Так как в меню мы показываем только те выбранные авто фильтры
			//которые выбраны в области данного меню и TempFilterData вполне
			//может содержать маску которую тока что выбрали в этом меню но
			//она уже была выбрана в другом и так как TempFilterData
			//и авто фильтры в меню отсортированы по алфавиту то немного
			//поколдуем чтоб не было дубликатов в памяти.
			strMask1 = Mask;
			Unquote(strMask1);

			while ((CurFilterData=TempFilterData.getItem(j))!=NULL)
			{
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

						for (int i=FFFT_FIRST; i < FFFT_COUNT; i++)
						{
							if (i != FFFT && CurFilterData->GetFlags((enumFileFilterFlagsType)i))
							{
								bCheckedNowhere = false;
								break;
							}
						}

						if (bCheckedNowhere)
						{
							TempFilterData.deleteItem(j);
							continue;
						}
					}
					else
					{
						j++;
					}
				}
				else
					CurFilterData=NULL;
			}

			if (Check && !CurFilterData)
			{
				FileFilterParams *NewFilter = TempFilterData.insertItem(j);

				if (NewFilter)
				{
					NewFilter->SetMask(1,Mask);
					//Авто фильтры они только для файлов, папки не должны к ним подходить
					NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					j++;
					CurFilterData = NewFilter;
				}
				else
					continue;
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
	ULARGE_INTEGER current;
	current.u.LowPart  = cft.dwLowDateTime;
	current.u.HighPart = cft.dwHighDateTime;
	CurrentTime = current.QuadPart;
}

bool FileFilter::FileInFilter(const FileListItem *fli,enumFileInFilterType *foundType)
{
	FAR_FIND_DATA fd;
	fd.dwFileAttributes=fli->FileAttr;
	fd.ftCreationTime=fli->CreationTime;
	fd.ftLastAccessTime=fli->AccessTime;
	fd.ftLastWriteTime=fli->WriteTime;
	fd.nFileSize=fli->UnpSize;
	fd.nPackSize=fli->PackSize;
	fd.lpwszFileName=(wchar_t *)(const wchar_t *)fli->strName;
	fd.lpwszAlternateFileName=(wchar_t *)(const wchar_t *)fli->strShortName;
	return FileInFilter(&fd,foundType);
}

bool FileFilter::FileInFilter(const FAR_FIND_DATA_EX *fde,enumFileInFilterType *foundType)
{
	FAR_FIND_DATA fd;
	fd.dwFileAttributes=fde->dwFileAttributes;
	fd.ftCreationTime=fde->ftCreationTime;
	fd.ftLastAccessTime=fde->ftLastAccessTime;
	fd.ftLastWriteTime=fde->ftLastWriteTime;
	fd.nFileSize=fde->nFileSize;
	fd.nPackSize=fde->nPackSize;
	fd.lpwszFileName=(wchar_t *)(const wchar_t *)fde->strFileName;
	fd.lpwszAlternateFileName=(wchar_t *)(const wchar_t *)fde->strAlternateFileName;
	return FileInFilter(&fd,foundType);
}

bool FileFilter::FileInFilter(const FAR_FIND_DATA *fd,enumFileInFilterType *foundType)
{
	enumFileFilterFlagsType FFFT = GetFFFT();
	bool bFound=false;
	bool bAnyIncludeFound=false;
	bool bAnyFolderIncludeFound=false;
	bool bInc=false;
	bool bFolder=(fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
	FileFilterParams *CurFilterData;
	DWORD Flags;

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		CurFilterData = FilterData.getItem(i);
		Flags = CurFilterData->GetFlags(FFFT);

		if (Flags)
		{
			if (bFound && !(Flags&FFF_STRONG))
				continue;

			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				DWORD AttrClear;

				if (CurFilterData->GetAttr(NULL,&AttrClear))
					bAnyFolderIncludeFound = bAnyFolderIncludeFound || !(AttrClear&FILE_ATTRIBUTE_DIRECTORY);
			}

			if (CurFilterData->FileInFilter(fd, CurrentTime))
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

	if (bFolder)
	{
		Flags = FoldersFilter.GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

			if (FoldersFilter.FileInFilter(fd, CurrentTime))
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
	else //авто-фильтры никогда не могут быть для папок
	{
		for (unsigned int i=0; i<TempFilterData.getCount(); i++)
		{
			CurFilterData = TempFilterData.getItem(i);
			Flags = CurFilterData->GetFlags(FFFT);

			if (Flags)
			{
				if (bFound && !(Flags&FFF_STRONG))
					continue;

				bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

				if (CurFilterData->FileInFilter(fd, CurrentTime))
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

bool FileFilter::IsEnabledOnPanel()
{
	if (m_FilterType != FFT_PANEL)
		return false;

	enumFileFilterFlagsType FFFT = GetFFFT();

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		if (FilterData.getItem(i)->GetFlags(FFFT))
			return true;
	}

	if (FoldersFilter.GetFlags(FFFT))
		return true;

	for (unsigned int i=0; i<TempFilterData.getCount(); i++)
	{
		if (TempFilterData.getItem(i)->GetFlags(FFFT))
			return true;
	}

	return false;
}

void FileFilter::InitFilter()
{
	FilterData.Free();
	TempFilterData.Free();
	string strRegKey;
	string strTitle, strMask, strSizeBelow, strSizeAbove;

	while (1)
	{
		strRegKey.Format(L"Filters\\Filter%d",FilterData.getCount());

		if (!GetRegKey(strRegKey,L"Title",strTitle,L""))
			break;

		FileFilterParams *NewFilter = FilterData.addItem();

		if (NewFilter)
		{
			//Дефолтные значения выбраны так чтоб как можно правильней загрузить
			//настройки старых версий фара.
			NewFilter->SetTitle(strTitle);
			GetRegKey(strRegKey,L"Mask",strMask,L"");
			NewFilter->SetMask(GetRegKey(strRegKey,L"UseMask",1)!=0,
			                   strMask);
			FILETIME DateAfter, DateBefore;
			GetRegKey(strRegKey,L"DateAfter",(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
			GetRegKey(strRegKey,L"DateBefore",(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
			NewFilter->SetDate(GetRegKey(strRegKey,L"UseDate",0)!=0,
			                   (DWORD)GetRegKey(strRegKey,L"DateType",0),
			                   DateAfter,
			                   DateBefore,
			                   GetRegKey(strRegKey,L"RelativeDate",0)!=0);
			GetRegKey(strRegKey,L"SizeAboveS",strSizeAbove,L"");
			GetRegKey(strRegKey,L"SizeBelowS",strSizeBelow,L"");
			NewFilter->SetSize(GetRegKey(strRegKey,L"UseSize",0)!=0,
			                   strSizeAbove,
			                   strSizeBelow);
			NewFilter->SetAttr(GetRegKey(strRegKey,L"UseAttr",1)!=0,
			                   (DWORD)GetRegKey(strRegKey,L"AttrSet",0),
			                   (DWORD)GetRegKey(strRegKey,L"AttrClear",FILE_ATTRIBUTE_DIRECTORY));
			DWORD Flags[FFFT_COUNT];
			GetRegKey(strRegKey,L"FFlags",(BYTE *)Flags,NULL,sizeof(Flags));

			for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
				NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
		}
		else
			break;
	}

	while (1)
	{
		strRegKey.Format(L"Filters\\PanelMask%d",TempFilterData.getCount());

		if (!GetRegKey(strRegKey,L"Mask",strMask,L""))
			break;

		FileFilterParams *NewFilter = TempFilterData.addItem();

		if (NewFilter)
		{
			NewFilter->SetMask(1,strMask);
			//Авто фильтры они только для файлов, папки не должны к ним подходить
			NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
			DWORD Flags[FFFT_COUNT];
			GetRegKey(strRegKey,L"FFlags",(BYTE *)Flags,NULL,sizeof(Flags));

			for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
				NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
		}
		else
			break;
	}

	{
		FoldersFilter.SetMask(0,L"");
		FoldersFilter.SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);
		DWORD Flags[FFFT_COUNT];
		GetRegKey(L"Filters",L"FoldersFilterFFlags",(BYTE *)Flags,NULL,sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			FoldersFilter.SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}
}


void FileFilter::CloseFilter()
{
	FilterData.Free();
	TempFilterData.Free();
}

void FileFilter::SaveFilters()
{
	string strRegKey;
	DeleteKeyTree(L"Filters");
	FileFilterParams *CurFilterData;

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		strRegKey.Format(L"Filters\\Filter%d",i);
		CurFilterData = FilterData.getItem(i);
		SetRegKey(strRegKey,L"Title",CurFilterData->GetTitle());
		const wchar_t *Mask;
		SetRegKey(strRegKey,L"UseMask",CurFilterData->GetMask(&Mask)?1:0);
		SetRegKey(strRegKey,L"Mask",Mask);
		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		bool bRelative;
		SetRegKey(strRegKey,L"UseDate",CurFilterData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
		SetRegKey(strRegKey,L"DateType",DateType);
		SetRegKey(strRegKey,L"DateAfter",(BYTE *)&DateAfter,sizeof(DateAfter));
		SetRegKey(strRegKey,L"DateBefore",(BYTE *)&DateBefore,sizeof(DateBefore));
		SetRegKey(strRegKey,L"RelativeDate",bRelative?1:0);
		const wchar_t *SizeAbove, *SizeBelow;
		SetRegKey(strRegKey,L"UseSize",CurFilterData->GetSize(&SizeAbove, &SizeBelow)?1:0);
		SetRegKey(strRegKey,L"SizeAboveS",SizeAbove);
		SetRegKey(strRegKey,L"SizeBelowS",SizeBelow);
		DWORD AttrSet, AttrClear;
		SetRegKey(strRegKey,L"UseAttr",CurFilterData->GetAttr(&AttrSet, &AttrClear)?1:0);
		SetRegKey(strRegKey,L"AttrSet",AttrSet);
		SetRegKey(strRegKey,L"AttrClear",AttrClear);
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = CurFilterData->GetFlags((enumFileFilterFlagsType)i);

		SetRegKey(strRegKey,L"FFlags",(BYTE *)Flags,sizeof(Flags));
	}

	for (unsigned int i=0; i<TempFilterData.getCount(); i++)
	{
		strRegKey.Format(L"Filters\\PanelMask%d",i);
		CurFilterData = TempFilterData.getItem(i);
		const wchar_t *Mask;
		CurFilterData->GetMask(&Mask);
		SetRegKey(strRegKey,L"Mask",Mask);
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = CurFilterData->GetFlags((enumFileFilterFlagsType)i);

		SetRegKey(strRegKey,L"FFlags",(BYTE *)Flags,sizeof(Flags));
	}

	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = FoldersFilter.GetFlags((enumFileFilterFlagsType)i);

		SetRegKey(L"Filters",L"FoldersFilterFFlags",(BYTE *)Flags,sizeof(Flags));
	}
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
	for (unsigned int i=0; i<FilterData.getCount(); i++)
		SwapPanelFlags(FilterData.getItem(i));

	SwapPanelFlags(&FoldersFilter);

	for (unsigned int i=0; i<TempFilterData.getCount(); i++)
		SwapPanelFlags(TempFilterData.getItem(i));
}

int FileFilter::ParseAndAddMasks(wchar_t **ExtPtr,const wchar_t *FileName,DWORD FileAttr,int& ExtCount,int Check)
{
	if (!StrCmp(FileName,L".") || TestParentFolderName(FileName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		return -1;

	const wchar_t *DotPtr=wcsrchr(FileName,L'.');
	string strMask;

	// Если маска содержит разделитель (',' или ';'), то возьмем ее в кавычки
	if (DotPtr==NULL)
		strMask = L"*.";
	else if (wcspbrk(DotPtr,L",;"))
		strMask.Format(L"\"*%s\"",DotPtr);
	else
		strMask.Format(L"*%s",DotPtr);

	// сначала поиск...
	unsigned int Cnt=ExtCount;

	if (_lfind((const void *)(const wchar_t *)strMask,(void *)*ExtPtr,&Cnt,NM*sizeof(wchar_t),ExtSort))
		return -1;

	// ... а потом уже выделение памяти!
	wchar_t *NewPtr;

	if ((NewPtr=(wchar_t *)xf_realloc(*ExtPtr,NM*(ExtCount+1)*sizeof(wchar_t))) == NULL)
		return 0;

	*ExtPtr=NewPtr;
	NewPtr=*ExtPtr+ExtCount*NM;
	xwcsncpy(NewPtr,strMask,NM-2);
	NewPtr=NewPtr+StrLength(NewPtr)+1;
	*NewPtr=Check;
	ExtCount++;
	return 1;
}

int _cdecl ExtSort(const void *el1,const void *el2)
{
	return StrCmpI((const wchar_t *)el1,(const wchar_t *)el2);
}
