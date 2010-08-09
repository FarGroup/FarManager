/*
filefilter.cpp

Файловый фильтр

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "scantree.hpp"
#include "filefilter.hpp"
#include "array.hpp"
#include "filelist.hpp"

static int _cdecl ExtSort(const void *el1,const void *el2);

static TPointerArray<FileFilterParams> FilterData, TempFilterData;
static FileFilterParams FoldersFilter;

FileFilter::FileFilter(Panel *HostPanel, enumFileFilterType FilterType)
{
	m_HostPanel=HostPanel;
	m_FilterType=FilterType;
	UpdateCurrentTime();
}

FileFilter::~FileFilter()
{
}

bool FileFilter::FilterEdit()
{
	struct MenuItem ListItem;
	int ExitCode;
	bool bNeedUpdate=false;
	VMenu FilterList(MSG(MFilterTitle),NULL,0,ScrY-6);
	FilterList.SetHelp("FiltersMenu");
	FilterList.SetPosition(-1,-1,0,0);
	FilterList.SetBottomTitle(MSG(MFilterBottom));
	FilterList.SetFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		memset(&ListItem,0,sizeof(ListItem));
		MenuString(ListItem.Name,FilterData.getItem(i));

		if (i == 0)
			ListItem.Flags|=LIF_SELECTED;

		int Check = GetCheck(FilterData.getItem(i));

		if (Check)
			ListItem.SetCheck(Check);

		FilterList.AddItem(&ListItem);
	}

	memset(&ListItem,0,sizeof(ListItem));

	if (FilterData.getCount()==0)
		ListItem.Flags|=LIF_SELECTED;

	FilterList.AddItem(&ListItem);
	char *ExtPtr=NULL;
	int ExtCount=0;
	{
		enumFileFilterFlagsType FFFT = GetFFFT();

		for (unsigned int i=0; i<TempFilterData.getCount(); i++)
		{
			//AY: Будем показывать только те выбранные авто фильтры
			//(для которых нету файлов на панели) которые выбраны в области данного меню
			if (!TempFilterData.getItem(i)->GetFlags(FFFT))
				continue;

			const char *FMask;
			TempFilterData.getItem(i)->GetMask(&FMask);
			char Mask[FILEFILTER_MASK_SIZE];
			xstrncpy(Mask,FMask,sizeof(Mask)-1);
			Unquote(Mask);

			if (!ParseAndAddMasks(&ExtPtr,Mask,0,ExtCount,GetCheck(TempFilterData.getItem(i))))
				break;
		}
	}
	memset(&ListItem,0,sizeof(ListItem));
	ListItem.Flags|=LIF_SEPARATOR;
	FilterList.AddItem(&ListItem);
	memset(&ListItem,0,sizeof(ListItem));
	FoldersFilter.SetTitle(MSG(MFolderFileType));
	MenuString(ListItem.Name,&FoldersFilter,false,'0');
	int Check = GetCheck(&FoldersFilter);

	if (Check)
		ListItem.SetCheck(Check);

	FilterList.AddItem(&ListItem);

	if (m_HostPanel->GetMode()==NORMAL_PANEL)
	{
		char CurDir[NM];
		char FileName[NM];
		WIN32_FIND_DATA fdata;
		m_HostPanel->GetCurDir(CurDir);
		ScanTree ScTree(FALSE,FALSE);
		ScTree.SetFindPath(CurDir,"*.*");

		while (ScTree.GetNextName(&fdata,FileName, sizeof(FileName)-1))
			if (!ParseAndAddMasks(&ExtPtr,fdata.cFileName,fdata.dwFileAttributes,ExtCount,0))
				break;
	}
	else
	{
		char FileName[NM];
		int FileAttr;

		for (int i=0; m_HostPanel->GetFileName(FileName,i,FileAttr); i++)
			if (!ParseAndAddMasks(&ExtPtr,FileName,FileAttr,ExtCount,0))
				break;
	}

	far_qsort((void *)ExtPtr,ExtCount,NM,ExtSort);
	memset(&ListItem,0,sizeof(ListItem));

	for (int i=0, h='1'; i<ExtCount; i++, (h=='9'?h='A':(h=='Z'||h==0?h=0:h++)))
	{
		char *CurExtPtr=ExtPtr+i*NM;
		MenuString(ListItem.Name,NULL,false,h,true,CurExtPtr,MSG(MPanelFileType));
		ListItem.SetCheck(CurExtPtr[strlen(CurExtPtr)+1]);
		FilterList.SetUserData(CurExtPtr,0,FilterList.AddItem(&ListItem));
	}

	xf_free(ExtPtr);
	FilterList.Show();

	while (!FilterList.Done())
	{
		int Key=FilterList.ReadInput();

		if (Key==KEY_ADD)
			Key='+';
		else if (Key==KEY_SUBTRACT)
			Key='-';
		else if (Key=='i')
			Key='I';
		else if (Key=='x')
			Key='X';

		switch (Key)
		{
			case '+':
			case '-':
			case 'I':
			case 'X':
			case KEY_SPACE:
			case KEY_BS:
			{
				int SelPos=FilterList.GetSelectPos();

				if (SelPos==FilterData.getCount())
					break;

				int Check=FilterList.GetSelection(SelPos);
				int NewCheck;

				if (Key==KEY_BS)
					NewCheck = 0;
				else if (Key==KEY_SPACE)
					NewCheck = Check ? 0 : '+';
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
						memset(&ListItem,0,sizeof(ListItem));
						MenuString(ListItem.Name,FilterData.getItem(SelPos));
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
						NewFilter->SetTitle("");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 == (FilterData.getCount()+2))
					{
						*NewFilter = FoldersFilter;
						NewFilter->SetTitle("");
						NewFilter->ClearAllFlags();
					}
					else if (SelPos2 > (int)(FilterData.getCount()+2))
					{
						char Mask[NM];
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
					memset(&ListItem,0,sizeof(ListItem));
					MenuString(ListItem.Name,NewFilter);
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
					char QuotedTitle[512+2];
					sprintf(QuotedTitle,"\"%.*s\"",sizeof(QuotedTitle)-1-2,FilterData.getItem(SelPos)->GetTitle());

					if (Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
					            QuotedTitle,MSG(MDelete),MSG(MCancel))==0)
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

				if (SelPos<(int)FilterData.getCount() && !(Key==KEY_CTRLUP && SelPos==0) && !(Key==KEY_CTRLDOWN && SelPos==FilterData.getCount()-1))
				{
					int NewPos = SelPos + (Key == KEY_CTRLDOWN ? 1 : -1);
					MenuItem CurItem, NextItem;
					memcpy(&CurItem,FilterList.GetItemPtr(SelPos),sizeof(CurItem));
					memcpy(&NextItem,FilterList.GetItemPtr(NewPos),sizeof(NextItem));
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
				if (Key!=KEY_NUMENTER && Key!=KEY_ENTER && Key!=KEY_ESC && Key!=KEY_F10 && ((Key>0x20 && Key<0xFF) || Key&(KEY_ALT|KEY_RALT)))
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
			m_HostPanel->Update(UPDATE_KEEP_SELECTION);
			m_HostPanel->Redraw();
		}
	}

	return (ExitCode!=-1);
}

enumFileFilterFlagsType FileFilter::GetFFFT()
{
	if (m_FilterType == FFT_PANEL)
	{
		if (m_HostPanel==CtrlObject->Cp()->RightPanel)
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
			return 'I';

		return '+';
	}
	else if (Flags&FFF_EXCLUDE)
	{
		if (Flags&FFF_STRONG)
			return 'X';

		return '-';
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
		else if (i == (FilterData.getCount() + 2))
		{
			CurFilterData = &FoldersFilter;
		}
		else if (i > (int)(FilterData.getCount() + 2))
		{
			const char *FMask;
			char Mask[NM], Mask1[NM];
			FilterList->GetUserData(Mask,sizeof(Mask),i);
			//AY: Так как в меню мы показываем только те выбранные авто фильтры
			//которые выбраны в области данного меню и TempFilterData вполне
			//может содержать маску которую тока что выбрали в этом меню но
			//она уже была выбрана в другом и так как TempFilterData
			//и авто фильтры в меню отсортированы по алфавиту то немного
			//поколдуем чтоб не было дубликатов в памяти.
			xstrncpy(Mask1,Mask,sizeof(Mask1)-1);
			Unquote(Mask1);

			while ((CurFilterData=TempFilterData.getItem(j))!=NULL)
			{
				char Mask2[FILEFILTER_MASK_SIZE];
				CurFilterData->GetMask(&FMask);
				xstrncpy(Mask2,FMask,sizeof(Mask2)-1);
				Unquote(Mask2);

				if (LocalStricmp(Mask1,Mask2)<1)
					break;

				j++;
			}

			if (CurFilterData)
			{
				if (!LocalStricmp(Mask,FMask))
				{
					if (!Check)
					{
						bool bCheckedNowhere = true;

						for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
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

		if (Check=='+')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE);
		else if (Check=='-')
			CurFilterData->SetFlags(FFFT, FFF_EXCLUDE);
		else if (Check=='I')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE|FFF_STRONG);
		else if (Check=='X')
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

bool FileFilter::FileInFilter(FileListItem *fli,enumFileInFilterType *foundType)
{
	WIN32_FIND_DATA fd;
	fd.dwFileAttributes=fli->FileAttr;
	fd.ftCreationTime=fli->CreationTime;
	fd.ftLastAccessTime=fli->AccessTime;
	fd.ftLastWriteTime=fli->WriteTime;
	fd.nFileSizeHigh=fli->UnpSizeHigh;
	fd.nFileSizeLow=fli->UnpSize;
	fd.dwReserved0=fli->PackSizeHigh;
	fd.dwReserved1=fli->PackSize;
	xstrncpy(fd.cFileName,fli->Name,sizeof(fd.cFileName)-1);
	xstrncpy(fd.cAlternateFileName,fli->ShortName,sizeof(fd.cAlternateFileName)-1);
	return FileInFilter(&fd,foundType);
}

bool FileFilter::FileInFilter(WIN32_FIND_DATA *fd,enumFileInFilterType *foundType)
{
	enumFileFilterFlagsType FFFT = GetFFFT();
	bool bFound=false;
	bool bAnyIncludeFound=false;
	bool bAnyFolderIncludeFound=false;
	bool bInc=false;
	bool bFolder=(fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
	FileFilterParams *CurFilterData;
	DWORD Flags;
	unsigned int i;

	for (i=0; i<FilterData.getCount(); i++)
	{
		CurFilterData = FilterData.getItem(i);
		Flags = CurFilterData->GetFlags(FFFT);

		if (Flags) // enumFileFilterFlags
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

	//авто-фильтр папки
	{
		Flags = FoldersFilter.GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				bAnyFolderIncludeFound = true;
			}

			if (bFolder && FoldersFilter.FileInFilter(fd, CurrentTime))
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
	for (i=0; i<TempFilterData.getCount(); i++)
	{
		CurFilterData = TempFilterData.getItem(i);
		Flags = CurFilterData->GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

			if (bFolder) //авто-фильтры никогда не могут быть для папок
				continue;

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
	char RegKey[80], *PtrRegKey;
	strcpy(RegKey,"Filters\\Filter");
	PtrRegKey=RegKey+strlen(RegKey);

	while (1)
	{
		itoa(FilterData.getCount(),PtrRegKey,10);
		char Title[512];

		if (!GetRegKey(RegKey,"Title",Title,"",sizeof(Title)))
			break;

		FileFilterParams *NewFilter = FilterData.addItem();

		if (NewFilter)
		{
			//Дефолтные значения выбраны так чтоб как можно правильней загрузить
			//настройки старых версий фара.
			NewFilter->SetTitle(Title);
			char Mask[FILEFILTER_MASK_SIZE];
			GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask));
			NewFilter->SetMask(GetRegKey(RegKey,"UseMask",1)!=0,
			                   Mask);
			FILETIME DateAfter, DateBefore;
			GetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
			GetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
			NewFilter->SetDate(GetRegKey(RegKey,"UseDate",0)!=0,
			                   (DWORD)GetRegKey(RegKey,"DateType",0),
			                   DateAfter,
			                   DateBefore,
			                   GetRegKey(RegKey,"RelativeDate",0)!=0);
			char SizeAbove[FILEFILTER_SIZE_SIZE];
			char SizeBelow[FILEFILTER_SIZE_SIZE];
			GetRegKey(RegKey,"SizeAboveS",SizeAbove,"",sizeof(SizeAbove));
			GetRegKey(RegKey,"SizeBelowS",SizeBelow,"",sizeof(SizeBelow));
			NewFilter->SetSize(GetRegKey(RegKey,"UseSize",0)!=0,
			                   SizeAbove,
			                   SizeBelow);
			NewFilter->SetAttr(GetRegKey(RegKey,"UseAttr",1)!=0,
			                   (DWORD)GetRegKey(RegKey,"AttrSet",0),
			                   (DWORD)GetRegKey(RegKey,"AttrClear",FILE_ATTRIBUTE_DIRECTORY));
			DWORD Flags[FFFT_COUNT];
			GetRegKey(RegKey,"FFlags",(BYTE *)Flags,NULL,sizeof(Flags));

			for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
				NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
		}
		else
			break;
	}

	strcpy(RegKey,"Filters\\PanelMask");
	PtrRegKey=RegKey+strlen(RegKey);

	while (1)
	{
		itoa(TempFilterData.getCount(),PtrRegKey,10);
		char Mask[FILEFILTER_MASK_SIZE];

		if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
			break;

		FileFilterParams *NewFilter = TempFilterData.addItem();

		if (NewFilter)
		{
			NewFilter->SetMask(1,Mask);
			//Авто фильтры они только для файлов, папки не должны к ним подходить
			NewFilter->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
			DWORD Flags[FFFT_COUNT];
			GetRegKey(RegKey,"FFlags",(BYTE *)Flags,NULL,sizeof(Flags));

			for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
				NewFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
		}
		else
			break;
	}

	{
		FoldersFilter.SetMask(0,"");
		FoldersFilter.SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);
		DWORD Flags[FFFT_COUNT];
		GetRegKey("Filters","FoldersFilterFFlags",(BYTE *)Flags,NULL,sizeof(Flags));

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
	char RegKey[80], *PtrRegKey;
	DeleteKeyTree("Filters");
	strcpy(RegKey,"Filters\\Filter");
	PtrRegKey=RegKey+strlen(RegKey);
	FileFilterParams *CurFilterData;

	for (unsigned int i=0; i<FilterData.getCount(); i++)
	{
		CurFilterData = FilterData.getItem(i);
		itoa(i,PtrRegKey,10);
		SetRegKey(RegKey,"Title",CurFilterData->GetTitle());
		const char *Mask;
		SetRegKey(RegKey,"UseMask",CurFilterData->GetMask(&Mask)?1:0);
		SetRegKey(RegKey,"Mask",Mask);
		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		bool bRelative;
		SetRegKey(RegKey,"UseDate",CurFilterData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
		SetRegKey(RegKey,"DateType",DateType);
		SetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,sizeof(DateAfter));
		SetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,sizeof(DateBefore));
		SetRegKey(RegKey,"RelativeDate",bRelative?1:0);
		const char *SizeAbove, *SizeBelow;
		SetRegKey(RegKey,"UseSize",CurFilterData->GetSize(&SizeAbove, &SizeBelow)?1:0);
		SetRegKey(RegKey,"SizeAboveS",SizeAbove);
		SetRegKey(RegKey,"SizeBelowS",SizeBelow);
		DWORD AttrSet, AttrClear;
		SetRegKey(RegKey,"UseAttr",CurFilterData->GetAttr(&AttrSet, &AttrClear)?1:0);
		SetRegKey(RegKey,"AttrSet",AttrSet);
		SetRegKey(RegKey,"AttrClear",AttrClear);
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = CurFilterData->GetFlags((enumFileFilterFlagsType)i);

		SetRegKey(RegKey,"FFlags",(BYTE *)Flags,sizeof(Flags));
	}

	strcpy(RegKey,"Filters\\PanelMask");
	PtrRegKey=RegKey+strlen(RegKey);

	for (unsigned int i=0; i<TempFilterData.getCount(); i++)
	{
		CurFilterData = TempFilterData.getItem(i);
		itoa(i,PtrRegKey,10);
		const char *Mask;
		CurFilterData->GetMask(&Mask);
		SetRegKey(RegKey,"Mask",Mask);
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = CurFilterData->GetFlags((enumFileFilterFlagsType)i);

		SetRegKey(RegKey,"FFlags",(BYTE *)Flags,sizeof(Flags));
	}

	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = FoldersFilter.GetFlags((enumFileFilterFlagsType)i);

		SetRegKey("Filters","FoldersFilterFFlags",(BYTE *)Flags,sizeof(Flags));
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

int FileFilter::ParseAndAddMasks(char **ExtPtr,const char *FileName,DWORD FileAttr,int& ExtCount,int Check)
{
	if (!strcmp(FileName,".") || TestParentFolderName(FileName) || (FileAttr & FA_DIREC))
		return -1;

	const char *DotPtr=strrchr(FileName,'.');
	char Mask[NM];

	/* $ 01.07.2001 IS
	   Если маска содержит разделитель (',' или ';'), то возьмем ее в
	   кавычки
	*/
	if (DotPtr==NULL)
		strcpy(Mask,"*.");
	else if (strpbrk(DotPtr,",;"))
		sprintf(Mask,"\"*%s\"",DotPtr);
	else
		sprintf(Mask,"*%s",DotPtr);

	/* IS $ */
	// сначала поиск...
	unsigned int Cnt=ExtCount;

	if (lfind((const void *)Mask,(void *)*ExtPtr,&Cnt,NM,ExtSort))
		return -1;

	// ... а потом уже выделение памяти!
	char *NewPtr;

	if ((NewPtr=(char *)xf_realloc(*ExtPtr,NM*(ExtCount+1))) == NULL)
		return 0;

	*ExtPtr=NewPtr;
	NewPtr=*ExtPtr+ExtCount*NM;
	xstrncpy(NewPtr,Mask,NM-2);
	NewPtr=NewPtr+strlen(NewPtr)+1;
	*NewPtr=Check;
	ExtCount++;
	return 1;
}

int _cdecl ExtSort(const void *el1,const void *el2)
{
	return LocalStricmp((char *)el1,(char *)el2);
}
