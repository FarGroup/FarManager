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
static BitFlags FolderFlags; //флаги для разрешения фильтрации папок

FileFilter::FileFilter(Panel *HostPanel, enumFileFilterType FilterType)
{
  m_HostPanel=HostPanel;
  m_FilterType=FilterType;
}

FileFilter::~FileFilter()
{
}

bool FileFilter::FilterEdit()
{
  struct MenuItemEx ListItem;
  int ExitCode;
  bool bNeedUpdate=false;
  VMenu FilterList(L"",NULL,0,ScrY-6);

  {
    DWORD Inc,Exc;
    GetIncludeExcludeFlags(Inc,Exc);
    if (FolderFlags.Check(Inc))
      FilterList.SetTitle(UMSG(MFilterTitle_IncFolders));
    else if (FolderFlags.Check(Exc))
      FilterList.SetTitle(UMSG(MFilterTitle_ExcFolders));
    else
      FilterList.SetTitle(UMSG(MFilterTitle_FilterFolders));
  }

  FilterList.SetHelp(L"FiltersMenu");
  FilterList.SetPosition(-1,-1,0,0);
  FilterList.SetBottomTitle(UMSG(MFilterBottom));
  FilterList.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);

  for (unsigned int i=0; i<FilterData.getCount(); i++)
  {
    ListItem.Clear();
    MenuString(ListItem.strName,FilterData.getItem(i));

    if(i == 0)
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
    DWORD Inc,Exc;
    GetIncludeExcludeFlags(Inc,Exc);
    for (unsigned int i=0; i<TempFilterData.getCount(); i++)
    {
      //AY: Будем показывать только те выбранные авто фильтры
      //(для которых нету файлов на панели) которые выбраны в области данного меню
      if (!TempFilterData.getItem(i)->Flags.Check(Inc|Exc))
        continue;
      const wchar_t *FMask;
      TempFilterData.getItem(i)->GetMask(&FMask);
      string strMask = FMask;
      Unquote(strMask);
      if(!ParseAndAddMasks(&ExtPtr,strMask,0,ExtCount,GetCheck(TempFilterData.getItem(i))))
        break;
    }
  }

  ListItem.Clear();
  ListItem.Flags|=LIF_SEPARATOR;
  FilterList.AddItem(&ListItem);

  ListItem.Clear();
  FoldersFilter.SetTitle(UMSG(MFolderFileType));
  MenuString(ListItem.strName,&FoldersFilter);
  int Check = GetCheck(&FoldersFilter);
  if (Check)
    ListItem.SetCheck(Check);
  FilterList.AddItem(&ListItem);

  if (m_HostPanel->GetMode()==NORMAL_PANEL)
  {
    string strCurDir, strFileName;
    FAR_FIND_DATA_EX fdata;

    m_HostPanel->GetCurDir(strCurDir);

    ScanTree ScTree(FALSE,FALSE);
    ScTree.SetFindPath(strCurDir,L"*.*");
    while (ScTree.GetNextName(&fdata,strFileName))
      if(!ParseAndAddMasks(&ExtPtr,fdata.strFileName,fdata.dwFileAttributes,ExtCount,0))
        break;
  }
  else
  {
    string strFileName;
    DWORD FileAttr;
    for (int i=0; m_HostPanel->GetFileName(strFileName,i,FileAttr); i++)
      if(!ParseAndAddMasks(&ExtPtr,strFileName,FileAttr,ExtCount,0))
        break;
  }

  far_qsort((void *)ExtPtr,ExtCount,NM*sizeof(wchar_t),ExtSort);

  ListItem.Clear();

  for (int i=0; i<ExtCount; i++)
  {
    wchar_t *CurExtPtr=ExtPtr+i*NM;
    MenuString(ListItem.strName,NULL,false,true,CurExtPtr,UMSG(MPanelFileType));
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

    switch(Key)
    {
      case KEY_SPACE:
      case L'+':
      case L'-':
      case KEY_BS:
      {
        int SelPos=FilterList.GetSelectPos();

        if (SelPos==(int)FilterData.getCount())
          break;

        int Check=FilterList.GetSelection(SelPos),NewCheck;
        if (Key==L'-')
          NewCheck=(Check==L'-') ? 0:L'-';
        else if (Key==L'+')
          NewCheck=(Check==L'+') ? 0:L'+';
        else if (Key==KEY_BS)
          NewCheck=0;
        else
          NewCheck=Check ? 0:L'+';

        FilterList.SetSelection(NewCheck,SelPos);
        FilterList.SetSelectPos(SelPos,1);
        FilterList.SetUpdateRequired(TRUE);
        FilterList.FastShow();
        FilterList.ProcessKey(KEY_DOWN);
        break;
      }

      case KEY_SHIFTSUBTRACT:
      case KEY_SHIFTBS:
      {
        for (int I=0; I < FilterList.GetItemCount(); I++)
        {
          FilterList.SetSelection(FALSE, I);
        }
        FilterList.SetUpdateRequired(TRUE);
        FilterList.FastShow();
        if (Key!=KEY_SHIFTSUBTRACT)
          break;
      }
      case KEY_CTRLF:
      {
        DWORD Inc,Exc;
        GetIncludeExcludeFlags(Inc,Exc);
        if (Key==KEY_CTRLF)
        {
          if (m_FilterType == FFT_SELECT)
            FolderFlags.Swap(Exc);
          else
            FolderFlags.Swap(Inc);
        }
        else
        {
          if (m_FilterType == FFT_SELECT)
            FolderFlags.Set(Exc);
          else
            FolderFlags.Set(Inc);
        }
        if (FolderFlags.Check(Inc))
          FilterList.SetTitle(UMSG(MFilterTitle_IncFolders));
        else if (FolderFlags.Check(Exc))
          FilterList.SetTitle(UMSG(MFilterTitle_ExcFolders));
        else
          FilterList.SetTitle(UMSG(MFilterTitle_FilterFolders));
        FilterList.SetUpdateRequired(TRUE);
        FilterList.SetPosition(-1,-1,0,0);
        FilterList.Show();
        bNeedUpdate=true;
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
          Message(MSG_WARNING,1,UMSG(MFilterTitle),UMSG(MCanEditCustomFilterOnly),UMSG(MOk));
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
            NewFilter->Flags.ClearAll();
          }
          else if (SelPos2 == (int)(FilterData.getCount()+2))
          {
            *NewFilter = FoldersFilter;

            NewFilter->SetTitle(L"");
            NewFilter->Flags.ClearAll();
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
          string strQuotedTitle;
          strQuotedTitle.Format(L"\"%s\"",FilterData.getItem(SelPos)->GetTitle());
          if (Message(0,2,UMSG(MFilterTitle),UMSG(MAskDeleteFilter),
                       (const wchar_t *)strQuotedTitle,UMSG(MDelete),UMSG(MCancel))==0)
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
          Message(MSG_WARNING,1,UMSG(MFilterTitle),UMSG(MCanDeleteCustomFilterOnly),UMSG(MOk));
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
          MenuItemEx CurItem, NextItem;
          CurItem = *FilterList.GetItemPtr(SelPos);
          NextItem = *FilterList.GetItemPtr(NewPos);

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
      }
    }
  }

  ExitCode=FilterList.Modal::GetExitCode();

  if (ExitCode!=-1)
    ProcessSelection(&FilterList);

  if(Opt.AutoSaveSetup)
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

void FileFilter::GetIncludeExcludeFlags(DWORD &Inc, DWORD &Exc)
{
  if (m_FilterType == FFT_PANEL)
  {
    if (m_HostPanel==CtrlObject->Cp()->RightPanel)
    {
      Inc = FFF_RPANELINCLUDE;
      Exc = FFF_RPANELEXCLUDE;
    }
    else
    {
      Inc = FFF_LPANELINCLUDE;
      Exc = FFF_LPANELEXCLUDE;
    }
  }
  else if (m_FilterType == FFT_COPY)
  {
    Inc = FFF_COPYINCLUDE;
    Exc = FFF_COPYEXCLUDE;
  }
  else if (m_FilterType == FFT_FINDFILE)
  {
    Inc = FFF_FINDFILEINCLUDE;
    Exc = FFF_FINDFILEEXCLUDE;
  }
  else
  {
    Inc = FFF_SELECTINCLUDE;
    Exc = FFF_SELECTEXCLUDE;
  }
}

int FileFilter::GetCheck(FileFilterParams *FFP)
{
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  if (FFP->Flags.Check(Inc))
    return L'+';
  else if (FFP->Flags.Check(Exc))
    return L'-';

  return 0;
}

void FileFilter::ProcessSelection(VMenu *FilterList)
{
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  FileFilterParams *CurFilterData;
  for (unsigned int i=0,j=0; i < (unsigned)FilterList->GetItemCount(); i++)
  {
    int Check=FilterList->GetSelection(i);

    CurFilterData=NULL;

    if (i < FilterData.getCount())
    {
      CurFilterData = FilterData.getItem(i);
    }
    else if (i == (FilterData.getCount() + 2))
    {
      CurFilterData = &FoldersFilter;
    }
    else if (i > (FilterData.getCount() + 2))
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
          if (!Check && !CurFilterData->Flags.Check(~(Inc|Exc)))
          {
            TempFilterData.deleteItem(j);
            continue;
          }
          else
            j++;
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

    CurFilterData->Flags.Clear(Inc|Exc);
    if (Check==L'+')
      CurFilterData->Flags.Set(Inc);
    else if (Check==L'-')
      CurFilterData->Flags.Set(Exc);
  }
}

bool FileFilter::FileInFilter(FileListItem *fli)
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

  return FileInFilter(&fd);
}

bool FileFilter::FileInFilter(const FAR_FIND_DATA_EX *fde, bool IsExcludeDir)
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

  return FileInFilter(&fd, IsExcludeDir);
}

bool FileFilter::FileInFilter(const FAR_FIND_DATA *fd, bool IsExcludeDir)
{
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  if (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
  {
    if (FolderFlags.Check(Inc))
      return !IsExcludeDir;
    if (FolderFlags.Check(Exc))
      return false;
  }

  bool flag=false;
  bool bInc=false;
  bool bExc=false;
  FileFilterParams *CurFilterData;

  for (unsigned int i=0; i<FilterData.getCount(); i++)
  {
    CurFilterData = FilterData.getItem(i);

    if (CurFilterData->Flags.Check(Inc|Exc))
    {
      flag = flag || CurFilterData->Flags.Check(Inc);
      if (CurFilterData->FileInFilter(fd))
        CurFilterData->Flags.Check(Inc)?bInc=true:bExc=true;
    }
  }

  if (FoldersFilter.Flags.Check(Inc|Exc))
  {
    flag = flag || FoldersFilter.Flags.Check(Inc);
    if (FoldersFilter.FileInFilter(fd))
      FoldersFilter.Flags.Check(Inc)?bInc=true:bExc=true;
  }

  for (unsigned int i=0; i<TempFilterData.getCount(); i++)
  {
    CurFilterData = TempFilterData.getItem(i);

    if (CurFilterData->Flags.Check(Inc|Exc))
    {
      flag = flag || CurFilterData->Flags.Check(Inc);
      if (CurFilterData->FileInFilter(fd))
        CurFilterData->Flags.Check(Inc)?bInc=true:bExc=true;
    }
  }

  if(IsExcludeDir) return bExc;

  if (bExc) return false;
  if (bInc) return true;
  return !flag;
}

bool FileFilter::IsEnabledOnPanel()
{
  if (m_FilterType != FFT_PANEL)
    return false;

  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  for (unsigned int i=0; i<FilterData.getCount(); i++)
  {
    if (FilterData.getItem(i)->Flags.Check(Inc|Exc))
      return true;
  }

  if (FoldersFilter.Flags.Check(Inc|Exc))
    return true;

  for (unsigned int i=0; i<TempFilterData.getCount(); i++)
  {
    if (TempFilterData.getItem(i)->Flags.Check(Inc|Exc))
      return true;
  }

  return false;
}

void FileFilter::InitFilter()
{
  FilterData.Free();
  TempFilterData.Free();

  string strRegKey;
  string strTitle, strMask;

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
      NewFilter->SetMask((DWORD)GetRegKey(strRegKey,L"UseMask",1),
                         strMask);

      FILETIME DateAfter, DateBefore;
      GetRegKey(strRegKey,L"DateAfter",(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
      GetRegKey(strRegKey,L"DateBefore",(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
      NewFilter->SetDate((DWORD)GetRegKey(strRegKey,L"UseDate",0),
                         (DWORD)GetRegKey(strRegKey,L"DateType",0),
                         DateAfter,
                         DateBefore);

      NewFilter->SetSize((DWORD)GetRegKey(strRegKey,L"UseSize",0),
                         (DWORD)GetRegKey(strRegKey,L"SizeType",0),
                         GetRegKey64(strRegKey,L"SizeAbove",(unsigned __int64)_i64(-1)),
                         GetRegKey64(strRegKey,L"SizeBelow",(unsigned __int64)_i64(-1)));

      NewFilter->SetAttr((DWORD)GetRegKey(strRegKey,L"UseAttr",1),
                         (DWORD)GetRegKey(strRegKey,L"AttrSet",0),
                         (DWORD)GetRegKey(strRegKey,L"AttrClear",FILE_ATTRIBUTE_DIRECTORY));

      NewFilter->Flags.Set((DWORD)GetRegKey(strRegKey,L"Flags",0));
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

      NewFilter->Flags.Set((DWORD)GetRegKey(strRegKey,L"Flags",0));
    }
    else
      break;
  }

  FoldersFilter.SetMask(0,L"");
  FoldersFilter.SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);
  FoldersFilter.Flags.Set((DWORD)GetRegKey(L"Filters",L"FoldersFilterFlags",0));

  FolderFlags.Set((DWORD)GetRegKey(L"Filters",L"FolderFlags",FFF_RPANELINCLUDE|FFF_LPANELINCLUDE|FFF_FINDFILEINCLUDE|FFF_COPYINCLUDE|FFF_SELECTEXCLUDE));
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
		SetRegKey(strRegKey,L"UseMask",CurFilterData->GetMask(&Mask));
		SetRegKey(strRegKey,L"Mask",Mask);


		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		SetRegKey(strRegKey,L"UseDate",CurFilterData->GetDate(&DateType, &DateAfter, &DateBefore));
		SetRegKey(strRegKey,L"DateType",DateType);
		SetRegKey(strRegKey,L"DateAfter",(BYTE *)&DateAfter,sizeof(DateAfter));
		SetRegKey(strRegKey,L"DateBefore",(BYTE *)&DateBefore,sizeof(DateBefore));


		DWORD SizeType;
		__int64 SizeAbove, SizeBelow;
		SetRegKey(strRegKey,L"UseSize",CurFilterData->GetSize(&SizeType, &SizeAbove, &SizeBelow));
		SetRegKey(strRegKey,L"SizeType",SizeType);
		SetRegKey64(strRegKey,L"SizeAbove",SizeAbove);
		SetRegKey64(strRegKey,L"SizeBelow",SizeBelow);


		DWORD AttrSet, AttrClear;
		SetRegKey(strRegKey,L"UseAttr",CurFilterData->GetAttr(&AttrSet, &AttrClear));
		SetRegKey(strRegKey,L"AttrSet",AttrSet);
		SetRegKey(strRegKey,L"AttrClear",AttrClear);

		SetRegKey(strRegKey,L"Flags",CurFilterData->Flags.Flags);
	}

	for (unsigned int i=0; i<TempFilterData.getCount(); i++)
	{
		strRegKey.Format(L"Filters\\PanelMask%d",i);

		CurFilterData = TempFilterData.getItem(i);

		const wchar_t *Mask;
		CurFilterData->GetMask(&Mask);
		SetRegKey(strRegKey,L"Mask",Mask);

		SetRegKey(strRegKey,L"Flags",CurFilterData->Flags.Flags);
	}

	SetRegKey(L"Filters",L"FoldersFilterFlags",FoldersFilter.Flags.Flags);
	SetRegKey(L"Filters",L"FolderFlags",FolderFlags.Flags);
}

void FileFilter::SwapPanelFlags(FileFilterParams *CurFilterData)
{
  DWORD flags=0;

  if (CurFilterData->Flags.Check(FFF_LPANELINCLUDE))
  {
    flags|=FFF_RPANELINCLUDE;
  }
  if (CurFilterData->Flags.Check(FFF_RPANELINCLUDE))
  {
    flags|=FFF_LPANELINCLUDE;
  }
  if (CurFilterData->Flags.Check(FFF_LPANELEXCLUDE))
  {
    flags|=FFF_RPANELEXCLUDE;
  }
  if (CurFilterData->Flags.Check(FFF_RPANELEXCLUDE))
  {
    flags|=FFF_LPANELEXCLUDE;
  }

  CurFilterData->Flags.Clear(FFF_RPANELEXCLUDE|FFF_LPANELEXCLUDE|FFF_RPANELINCLUDE|FFF_LPANELINCLUDE);
  CurFilterData->Flags.Set(flags);
}

void FileFilter::SwapFilter()
{
  for (unsigned int i=0; i<FilterData.getCount(); i++)
    SwapPanelFlags(FilterData.getItem(i));

  SwapPanelFlags(&FoldersFilter);

  for (unsigned int i=0; i<TempFilterData.getCount(); i++)
    SwapPanelFlags(TempFilterData.getItem(i));

  DWORD flags=0;
  if (FolderFlags.Check(FFF_LPANELINCLUDE))
  {
    flags|=FFF_RPANELINCLUDE;
  }
  if (FolderFlags.Check(FFF_RPANELINCLUDE))
  {
    flags|=FFF_LPANELINCLUDE;
  }
  FolderFlags.Clear(FFF_RPANELINCLUDE|FFF_LPANELINCLUDE);
  FolderFlags.Set(flags);
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
  else if(wcspbrk(DotPtr,L",;"))
    strMask.Format(L"\"*%s\"",DotPtr);
  else
    strMask.Format(L"*%s",DotPtr);

  // сначала поиск...
  unsigned int Cnt=ExtCount;
  if(lfind((const void *)(const wchar_t *)strMask,(void *)*ExtPtr,&Cnt,NM*sizeof(wchar_t),ExtSort))
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
