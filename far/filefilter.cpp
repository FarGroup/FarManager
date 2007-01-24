/*
filefilter.cpp

Файловый фильтр

*/
#include "headers.hpp"
#pragma hdrstop

#include "CFileMask.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "scantree.hpp"
#include "filefilter.hpp"

FileFilterParams::FileFilterParams()
{
  *m_Title=0;
  SetMask(0,"");
  SetSize(0,FSIZE_INBYTES,_i64(-1),_i64(-1));
  memset(&FDate,0,sizeof(FDate));
  memset(&FAttr,0,sizeof(FAttr));
  Flags.ClearAll();
}

void FileFilterParams::SetTitle(const char *Title)
{
  xstrncpy(m_Title,Title,sizeof(m_Title)-1);
}

void FileFilterParams::SetMask(DWORD Used, const char *Mask)
{
  FMask.Used = Used;
  xstrncpy(FMask.Mask,Mask,sizeof(FMask.Mask)-1);
  // Проверка на валидность текущих настроек фильтра
  if ((*FMask.Mask==0) || (!FMask.FilterMask.Set(FMask.Mask,FMF_SILENT)))
  {
    xstrncpy(FMask.Mask,"*.*",sizeof(FMask.Mask)-1);
    FMask.FilterMask.Set(FMask.Mask,FMF_SILENT);
  }
}

void FileFilterParams::SetDate(DWORD Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore)
{
  FDate.Used=Used;
  FDate.DateType=(FDateType)DateType;
  if (DateType>FDATE_OPENED || DateType<FDATE_MODIFIED)
  {
    FDate.DateType=FDATE_MODIFIED;
  }
  FDate.DateAfter=DateAfter;
  FDate.DateBefore=DateBefore;
}

void FileFilterParams::SetSize(DWORD Used, DWORD SizeType, __int64 SizeAbove, __int64 SizeBelow)
{
  FSize.Used=Used;
  FSize.SizeType=(FSizeType)SizeType;
  if (SizeType>=FSIZE_IN_LAST || SizeType<FSIZE_INBYTES)
  {
    FSize.SizeType=FSIZE_INBYTES;
  }
  FSize.SizeAbove=SizeAbove;
  FSize.SizeBelow=SizeBelow;
  FSize.SizeAboveReal=SizeAbove;
  FSize.SizeBelowReal=SizeBelow;
  switch (FSize.SizeType)
  {
    case FSIZE_INBYTES:
      // Размер введён в байтах, значит ничего не меняем.
      break;
    case FSIZE_INKBYTES:
      // Размер введён в килобайтах, переведём его в байты.
      // !!! Проверки на превышение максимального значения не делаются !!!
      FSize.SizeAboveReal>>=10;
      FSize.SizeBelowReal>>=10;
      break;
    case FSIZE_INMBYTES:
      // Задел // Размер введён в мегабайтах, переведём его в байты.
      // !!! Проверки на превышение максимального значения не делаются !!!
      FSize.SizeAboveReal>>=20;
      FSize.SizeBelowReal>>=20;
      break;
    case FSIZE_INGBYTES:
      // Задел // Размер введён в гигабайтах, переведём его в байты.
      // !!! Проверки на превышение максимального значения не делаются !!!
      FSize.SizeAboveReal>>=30;
      FSize.SizeBelowReal>>=30;
      break;
  }
}

void FileFilterParams::SetAttr(DWORD Used, DWORD AttrSet, DWORD AttrClear)
{
  FAttr.Used=Used;
  FAttr.AttrSet=AttrSet;
  FAttr.AttrClear=AttrClear;
}

const char *FileFilterParams::GetTitle()
{
  return m_Title;
}

DWORD FileFilterParams::GetMask(const char **Mask)
{
  if (Mask)
    *Mask=FMask.Mask;
  return FMask.Used;
}

DWORD FileFilterParams::GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore)
{
  if (DateType)
    *DateType=FDate.DateType;
  if (DateAfter)
    *DateAfter=FDate.DateAfter;
  if (DateBefore)
    *DateBefore=FDate.DateBefore;
  return FDate.Used;
}

DWORD FileFilterParams::GetSize(DWORD *SizeType, __int64 *SizeAbove, __int64 *SizeBelow)
{
  if (SizeType)
    *SizeType=FSize.SizeType;
  if (SizeAbove)
    *SizeAbove=FSize.SizeAbove;
  if (SizeBelow)
    *SizeBelow=FSize.SizeBelow;
  return FSize.Used;
}

DWORD FileFilterParams::GetAttr(DWORD *AttrSet, DWORD *AttrClear)
{
  if (AttrSet)
    *AttrSet=FAttr.AttrSet;
  if (AttrClear)
    *AttrClear=FAttr.AttrClear;
  return FAttr.Used;
}

bool FileFilterParams::FileInFilter(WIN32_FIND_DATA *fd)
{
  // Пустое значение?
  if (fd==NULL)
    return false;

  // Режим проверки маски файла включен?
  if (FMask.Used)
  {
    // Файл не попадает под маску введённую в фильтре?
    if (!FMask.FilterMask.Compare(fd->cFileName))
      // Не пропускаем этот файл
      return false;
  }

  // Режим проверки размера файла включен?
  if (FSize.Used)
  {
    // Преобразуем размер из двух DWORD в беззнаковый __int64
    unsigned __int64 fsize=(unsigned __int64)fd->nFileSizeLow|((unsigned __int64)fd->nFileSizeHigh<<32);

    if (FSize.SizeAbove != _i64(-1))
    {
      if (fsize < FSize.SizeAboveReal)      // Размер файла меньше минимального разрешённого по фильтру?
        return false;                       // Не пропускаем этот файл
    }

    if (FSize.SizeBelow != _i64(-1))
    {

      if (fsize > FSize.SizeBelowReal)      // Размер файла больше максимального разрешённого по фильтру?
        return false;                       // Не пропускаем этот файл
    }
  }

  // Режим проверки времени файла включен?
  if (FDate.Used)
  {
    // Преобразуем FILETIME в беззнаковый __int64
    unsigned __int64 &after=(unsigned __int64 &)FDate.DateAfter;
    unsigned __int64 &before=(unsigned __int64 &)FDate.DateBefore;

    if (after!=_ui64(0) || before!=_ui64(0))
    {
      unsigned __int64 ftime=_ui64(0);

      switch (FDate.DateType)
      {
        case FDATE_MODIFIED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastWriteTime;
          break;
        case FDATE_CREATED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftCreationTime;
          break;
        case FDATE_OPENED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastAccessTime;
          break;
      }

      // Есть введённая пользователем начальная дата?
      if (after!=_ui64(0))
        // Дата файла меньше начальной даты по фильтру?
        if (ftime<after)
          // Не пропускаем этот файл
          return false;

      // Есть введённая пользователем конечная дата?
      if (before!=_ui64(0))
        // Дата файла больше конечной даты по фильтру?
        if (ftime>before)
          return false;
    }
  }

  // Режим проверки атрибутов файла включен?
  if (FAttr.Used)
  {
    // Проверка попадания файла по установленным атрибутам
    if ((fd->dwFileAttributes & FAttr.AttrSet) != FAttr.AttrSet)
      return false;

    // Проверка попадания файла по отсутствующим атрибутам
    if (fd->dwFileAttributes & FAttr.AttrClear)
      return FALSE;
  }

  // Да! Файл выдержал все испытания и будет допущен к использованию
  // в вызвавшей эту функцию операции.
  return true;
}

static int _cdecl ExtSort(const void *el1,const void *el2);

static FileFilterParams **FilterData=NULL, **TempFilterData=NULL;
static int FilterDataCount=0, TempFilterDataCount=0;
static FileFilterParams FoldersFilter;

FileFilter::FileFilter(Panel *HostPanel, enumFileFilterType FilterType)
{
  m_HostPanel=HostPanel;
  m_FilterType=FilterType;
}

FileFilter::~FileFilter()
{
}

void FileFilter::FilterEdit()
{
  const unsigned char VerticalLine=0x0B3;
  struct MenuItem ListItem;
  int ExitCode;
  bool bNeedUpdate=false;
  VMenu FilterList(MSG(MFilterTitle),NULL,0,ScrY-6);

  FilterList.SetHelp("Filter");
  FilterList.SetPosition(-1,-1,0,0);
  FilterList.SetBottomTitle(MSG(MFilterBottom));
  FilterList.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);

  for (int i=0; i<FilterDataCount; i++)
  {
    memset(&ListItem,0,sizeof(ListItem));
    const char *Mask;
    DWORD MaskUsed=FilterData[i]->GetMask(&Mask);
    sprintf(ListItem.Name,"%-30.30s %c %-30.30s",FilterData[i]->GetTitle(),VerticalLine,MaskUsed?Mask:"");

    if(i == 0)
      ListItem.Flags|=LIF_SELECTED;

    int Check = GetCheck(FilterData[i]);
    if (Check)
      ListItem.SetCheck(Check);

    FilterList.AddItem(&ListItem);
  }

  if (FilterDataCount==0)
  {
    memset(&ListItem,0,sizeof(ListItem));
    /* 11.10.2001 VVM
        ! Если пользовательских фильтров нет, то надпись во всю ширину меню */
    sprintf(ListItem.Name,"%-60.60s",MSG(MNoCustomFilters));
    ListItem.Flags|=LIF_SELECTED;
    FilterList.AddItem(&ListItem);
  }

  char FileName[NM],*ExtPtr=NULL;
  WIN32_FIND_DATA fdata;
  int FileAttr,ExtCount=0;

  for (int i=0; i<TempFilterDataCount; i++)
  {
    const char *Mask;
    TempFilterData[i]->GetMask(&Mask);
    if(!ParseAndAddMasks(&ExtPtr,Mask,0,ExtCount,GetCheck(TempFilterData[i])))
      break;

  }

  memset(&ListItem,0,sizeof(ListItem));
  ListItem.Flags|=LIF_SEPARATOR;
  FilterList.AddItem(&ListItem);

  memset(&ListItem,0,sizeof(ListItem));
  sprintf(ListItem.Name,"%-30.30s %c %-30.30s",MSG(MFolderFileType),VerticalLine,"");
  int Check = GetCheck(&FoldersFilter);
  if (Check)
    ListItem.SetCheck(Check);
  FilterList.AddItem(&ListItem);

  if (m_HostPanel->GetMode()==NORMAL_PANEL)
  {
    char CurDir[NM];
    m_HostPanel->GetCurDir(CurDir);

    ScanTree ScTree(FALSE,FALSE);
    ScTree.SetFindPath(CurDir,"*.*");
    while (ScTree.GetNextName(&fdata,FileName, sizeof (FileName)-1))
      if(!ParseAndAddMasks(&ExtPtr,fdata.cFileName,fdata.dwFileAttributes,ExtCount,0))
        break;
  }
  else
  {
    for (int i=0; m_HostPanel->GetFileName(FileName,i,FileAttr); i++)
      if(!ParseAndAddMasks(&ExtPtr,FileName,FileAttr,ExtCount,0))
        break;
  }

  far_qsort((void *)ExtPtr,ExtCount,NM,ExtSort);

  memset(&ListItem,0,sizeof(ListItem));

  for (int i=0; i<ExtCount; i++)
  {
    char *CurExtPtr=ExtPtr+i*NM;
    sprintf(ListItem.Name,"%-30.30s %c %-30.30s",MSG(MPanelFileType),VerticalLine,CurExtPtr);
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

    switch(Key)
    {
      case KEY_SPACE:
      case '+':
      case '-':
      {
        if (!FilterList.GetSelectPos() && !FilterDataCount)
          break;

        int Check=FilterList.GetSelection(),NewCheck;
        if (Key=='-')
          NewCheck=(Check=='-') ? 0:'-';
        else if (Key=='+')
          NewCheck=(Check=='+') ? 0:'+';
        else
          NewCheck=Check ? 0:'+';

        FilterList.SetSelection(NewCheck);
        FilterList.SetUpdateRequired(TRUE);
        FilterList.FastShow();
        FilterList.ProcessKey(KEY_DOWN);
        break;
      }

      /* $ 11.09.2003 VVM
          + SHIFT-<GREY MINUS> сбрасывает все пометки фильтра */
      case KEY_SHIFTSUBTRACT:
      {
        for (int I=0; I < FilterList.GetItemCount(); I++)
        {
          FilterList.SetSelection(FALSE, I);
        }
        FilterList.SetUpdateRequired(TRUE);
        FilterList.FastShow();
        break;
      }
      /* VVM $ */

      case KEY_F4:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterDataCount)
        {
          if (FileFilterConfig(FilterData[SelPos]))
          {
            memset(&ListItem,0,sizeof(ListItem));
            const char *Mask;
            DWORD MaskUsed=FilterData[SelPos]->GetMask(&Mask);
            sprintf(ListItem.Name,"%-30.30s %c %-30.30s",FilterData[SelPos]->GetTitle(),VerticalLine,MaskUsed?Mask:"");
            int Check = GetCheck(FilterData[SelPos]);
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
        else
        {
          Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
          continue;
        }
        break;
      }

      case KEY_INS:
      {
        int SelPos=FilterList.GetSelectPos();

        if (SelPos>FilterDataCount)
          SelPos=FilterDataCount;

        FileFilterParams *NewFilter = new FileFilterParams;

        if (NewFilter && FileFilterConfig(NewFilter))
        {

          FileFilterParams **NewFilterData;
          if ((NewFilterData=(FileFilterParams **)xf_realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
          {
            delete NewFilter;
            break;
          }

          FilterData=NewFilterData;

          for (int I=FilterDataCount-1;I>=SelPos;I--)
            FilterData[I+1]=FilterData[I];

          FilterDataCount++;
          FilterData[SelPos]=NewFilter;

          if (FilterDataCount == 1)
            FilterList.DeleteItem(0);

          memset(&ListItem,0,sizeof(ListItem));
          const char *Mask;
          DWORD MaskUsed=FilterData[SelPos]->GetMask(&Mask);
          sprintf(ListItem.Name,"%-30.30s %c %-30.30s",FilterData[SelPos]->GetTitle(),VerticalLine,MaskUsed?Mask:"");

          FilterList.AddItem(&ListItem,SelPos);

          FilterList.AdjustSelectPos();
          FilterList.SetSelectPos(SelPos,1);
          FilterList.SetPosition(-1,-1,-1,-1);
          FilterList.Show();
          bNeedUpdate=true;
        }
        else if (NewFilter)
          delete NewFilter;
        break;
      }

      case KEY_DEL:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterDataCount)
        {
          char QuotedTitle[512];
          sprintf(QuotedTitle,"\"%.*s\"",sizeof(QuotedTitle)-1,FilterData[SelPos]->GetTitle());
          if (Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
                      QuotedTitle,MSG(MDelete),MSG(MCancel))==0)
          {
            delete FilterData[SelPos];

            for (int I=SelPos+1;I<FilterDataCount;I++)
              FilterData[I-1]=FilterData[I];

            FilterDataCount--;

            FilterList.DeleteItem(SelPos);

            if (FilterDataCount==0)
            {
              memset(&ListItem,0,sizeof(ListItem));
              /* 11.10.2001 VVM
                  ! Если пользовательских фильтров нет, то надпись во всю ширину меню */
              sprintf(ListItem.Name,"%-60.60s",MSG(MNoCustomFilters));
              FilterList.AddItem(&ListItem,0);
            }

            if (SelPos>=FilterDataCount && SelPos>0)
              SelPos--;

            FilterList.AdjustSelectPos();
            FilterList.SetSelectPos(SelPos,1);
            FilterList.SetPosition(-1,-1,-1,-1);
            FilterList.Show();
            bNeedUpdate=true;
          }
        }
        else
        {
          Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
          continue;
        }
        break;
      }

      case KEY_CTRLUP:
      case KEY_CTRLDOWN:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterDataCount && !(Key==KEY_CTRLUP && SelPos==0) && !(Key==KEY_CTRLDOWN && SelPos==FilterDataCount-1))
        {
          int NewPos = SelPos + (Key == KEY_CTRLDOWN ? 1 : -1);
          MenuItem CurItem, NextItem;
          memcpy(&CurItem,FilterList.GetItemPtr(SelPos),sizeof(CurItem));
          memcpy(&NextItem,FilterList.GetItemPtr(NewPos),sizeof(NextItem));

          FileFilterParams *Temp;
          Temp = FilterData[NewPos];
          FilterData[NewPos] = FilterData[SelPos];
          FilterData[SelPos] = Temp;

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

  if (ExitCode!=-1 || bNeedUpdate)
  {
    m_HostPanel->Update(UPDATE_KEEP_SELECTION);
    m_HostPanel->Redraw();
  }
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
  else
  {
    Inc = FFF_FINDFILEINCLUDE;
    Exc = FFF_FINDFILEEXCLUDE;
  }
}

int FileFilter::GetCheck(FileFilterParams *FFP)
{
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  if (FFP->Flags.Check(Inc))
    return '+';
  else if (FFP->Flags.Check(Exc))
    return '-';

  return 0;
}

void FileFilter::ProcessSelection(VMenu *FilterList)
{
  if(TempFilterData)
  {
    for (int i=0; i < TempFilterDataCount; i++)
      delete TempFilterData[i];

    xf_free(TempFilterData);
    TempFilterData=NULL;
    TempFilterDataCount=0;
  }

  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  FileFilterParams *CurFilterData;
  for (int i=0,j=0; i < FilterList->GetItemCount(); i++)
  {
    int Check=FilterList->GetSelection(i);

    CurFilterData=NULL;

    if (i < FilterDataCount)
    {
      CurFilterData = FilterData[i];
    }
    else if (i == (FilterDataCount + (FilterDataCount > 0 ? 0 : 1) + 1))
    {
      CurFilterData = &FoldersFilter;
    }
    else if (i > (FilterDataCount + (FilterDataCount > 0 ? 0 : 1) + 1))
    {
      char Mask[NM];
      FilterList->GetUserData(Mask,sizeof(Mask),i);

      if (j < TempFilterDataCount)
      {
         CurFilterData = TempFilterData[j];

         const char *FMask;
         CurFilterData->GetMask(&FMask);
         if (!strcmp(Mask,FMask))
         {
           if (!Check && !CurFilterData->Flags.Check(~(Inc|Exc)))
           {
              delete TempFilterData[j];

              for (int i=j+1; i<TempFilterDataCount; i++)
                TempFilterData[i-1]=FilterData[i];

              TempFilterDataCount--;
           }
           else
             j++;
         }
         else
           CurFilterData=NULL;

      }
      if (Check && !CurFilterData)
      {
        FileFilterParams *NewFilter = new FileFilterParams;

        if (NewFilter)
        {
          FileFilterParams **NewFilterData;
          if ((NewFilterData=(FileFilterParams **)xf_realloc(TempFilterData,sizeof(*TempFilterData)*(TempFilterDataCount+1)))==NULL)
          {
            delete NewFilter;
            break;
          }

          NewFilter->SetMask(1,Mask);

          TempFilterData=NewFilterData;

          for (int i=TempFilterDataCount-1; i>=j; i--)
            TempFilterData[i+1]=TempFilterData[i];

          TempFilterData[j++]=NewFilter;
          TempFilterDataCount++;
          CurFilterData = NewFilter;
        }
        else
          continue;
      }
    }

    if (!CurFilterData)
      continue;

    CurFilterData->Flags.Clear(Inc|Exc);
    if (Check=='+')
      CurFilterData->Flags.Set(Inc);
    else if (Check=='-')
      CurFilterData->Flags.Set(Exc);
  }
}

bool FileFilter::FileInFilter(WIN32_FIND_DATA *fd)
{
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  bool flag=false;
  FileFilterParams *CurFilterData;

  for (int i=0; i<FilterDataCount; i++)
  {
    CurFilterData = FilterData[i];

    if (CurFilterData->Flags.Check(Inc|Exc))
    {
      flag = flag || CurFilterData->Flags.Check(Inc);
      if (CurFilterData->FileInFilter(fd))
        return CurFilterData->Flags.Check(Inc)?true:false;
    }
  }

  for (int i=0; i<TempFilterDataCount; i++)
  {
    CurFilterData = TempFilterData[i];

    if (CurFilterData->Flags.Check(Inc|Exc))
    {
      flag = flag || CurFilterData->Flags.Check(Inc);
      if (CurFilterData->FileInFilter(fd))
        return CurFilterData->Flags.Check(Inc)?true:false;
    }
  }

  if (FoldersFilter.Flags.Check(Inc|Exc))
  {
    flag = flag || FoldersFilter.Flags.Check(Inc);
    if (FoldersFilter.FileInFilter(fd))
      return FoldersFilter.Flags.Check(Inc)?true:false;
  }

  return !flag;
}

bool FileFilter::IsEnabledOnPanel()
{
  if (m_FilterType != FFT_PANEL)
    return false;

  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  for (int i=0; i<FilterDataCount; i++)
  {
    if (FilterData[i]->Flags.Check(Inc|Exc))
      return true;
  }

  for (int i=0; i<TempFilterDataCount; i++)
  {
    if (TempFilterData[i]->Flags.Check(Inc|Exc))
      return true;
  }

  if (FoldersFilter.Flags.Check(Inc|Exc))
    return true;

  return false;
}

void FileFilter::InitFilter()
{
  FilterData=NULL;
  FilterDataCount=0;

  TempFilterData=NULL;
  TempFilterDataCount=0;

  char RegKey[80], *PtrRegKey;

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  while (1)
  {
    itoa(FilterDataCount,PtrRegKey,10);

    char Title[512];
    if (!GetRegKey(RegKey,"Title",Title,"",sizeof(Title)))
      break;

    FileFilterParams *NewFilter = new FileFilterParams;

    if (NewFilter)
    {
      FileFilterParams **NewFilterData;
      if ((NewFilterData=(FileFilterParams **)xf_realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
      {
        delete NewFilter;
        break;
      }

      FilterData=NewFilterData;

      NewFilter->SetTitle(Title);

      char Mask[512];
      GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask));
      NewFilter->SetMask((DWORD)GetRegKey(RegKey,"UseMask",0),
                         Mask);

      FILETIME DateAfter, DateBefore;
      GetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
      GetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
      NewFilter->SetDate((DWORD)GetRegKey(RegKey,"UseDate",0),
                         (DWORD)GetRegKey(RegKey,"DateType",0),
                         DateAfter,
                         DateBefore);

      NewFilter->SetSize((DWORD)GetRegKey(RegKey,"UseSize",0),
                         (DWORD)GetRegKey(RegKey,"SizeType",0),
                         GetRegKey64(RegKey,"SizeAbove",_i64(-1)),
                         GetRegKey64(RegKey,"SizeBelow",_i64(-1)));

      NewFilter->SetAttr((DWORD)GetRegKey(RegKey,"UseAttr",0),
                         (DWORD)GetRegKey(RegKey,"AttrSet",0),
                         (DWORD)GetRegKey(RegKey,"AttrClear",0));

      NewFilter->Flags.Set((DWORD)GetRegKey(RegKey,"Flags",0));

      FilterData[FilterDataCount++]=NewFilter;
    }
    else
      break;
  }

  strcpy(RegKey,"Filters\\PanelMask");
  PtrRegKey=RegKey+strlen(RegKey);

  while (1)
  {
    itoa(TempFilterDataCount,PtrRegKey,10);

    char Mask[512];
    if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
      break;

    FileFilterParams *NewFilter = new FileFilterParams;

    if (NewFilter)
    {
      FileFilterParams **NewFilterData;
      if ((NewFilterData=(FileFilterParams **)xf_realloc(TempFilterData,sizeof(*TempFilterData)*(TempFilterDataCount+1)))==NULL)
      {
        delete NewFilter;
        break;
      }

      TempFilterData=NewFilterData;

      NewFilter->SetMask(1, Mask);

      NewFilter->Flags.Set((DWORD)GetRegKey(RegKey,"Flags",0));

      TempFilterData[TempFilterDataCount++]=NewFilter;
    }
    else
      break;
  }

  FoldersFilter.SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);
  FoldersFilter.Flags.Set((DWORD)GetRegKey("Filters","FoldersFilterFlags",0));
}


void FileFilter::CloseFilter()
{
  if(FilterData)
  {
    for (int i=0; i < FilterDataCount; i++)
      delete FilterData[i];

    xf_free(FilterData);
    FilterData=NULL;
    FilterDataCount=0;
  }

  if(TempFilterData)
  {
    for (int i=0; i < TempFilterDataCount; i++)
      delete TempFilterData[i];

    xf_free(TempFilterData);
    TempFilterData=NULL;
    TempFilterDataCount=0;
  }
}

void FileFilter::SaveFilters()
{
  char RegKey[80], *PtrRegKey;

  DeleteKeyTree("Filters");

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  FileFilterParams *CurFilterData;

  for (int i=0; i<FilterDataCount; i++)
  {
    CurFilterData = FilterData[i];
    itoa(i,PtrRegKey,10);

    SetRegKey(RegKey,"Title",CurFilterData->GetTitle());

    const char *Mask;
    SetRegKey(RegKey,"UseMask",CurFilterData->GetMask(&Mask));
    SetRegKey(RegKey,"Mask",Mask);


    DWORD DateType;
    FILETIME DateAfter, DateBefore;
    SetRegKey(RegKey,"UseDate",CurFilterData->GetDate(&DateType, &DateAfter, &DateBefore));
    SetRegKey(RegKey,"DateType",DateType);
    SetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,sizeof(DateAfter));
    SetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,sizeof(DateBefore));


    DWORD SizeType;
    __int64 SizeAbove, SizeBelow;
    SetRegKey(RegKey,"UseSize",CurFilterData->GetSize(&SizeType, &SizeAbove, &SizeBelow));
    SetRegKey(RegKey,"SizeType",SizeType);
    SetRegKey64(RegKey,"SizeAbove",SizeAbove);
    SetRegKey64(RegKey,"SizeBelow",SizeBelow);


    DWORD AttrSet, AttrClear;
    SetRegKey(RegKey,"UseAttr",CurFilterData->GetAttr(&AttrSet, &AttrClear));
    SetRegKey(RegKey,"AttrSet",AttrSet);
    SetRegKey(RegKey,"AttrClear",AttrClear);

    SetRegKey(RegKey,"Flags",CurFilterData->Flags.Flags);
  }

  strcpy(RegKey,"Filters\\PanelMask");
  PtrRegKey=RegKey+strlen(RegKey);

  for (int i=0; i<TempFilterDataCount; i++)
  {
    CurFilterData = TempFilterData[i];
    itoa(i,PtrRegKey,10);

    const char *Mask;
    CurFilterData->GetMask(&Mask);
    SetRegKey(RegKey,"Mask",Mask);

    SetRegKey(RegKey,"Flags",CurFilterData->Flags.Flags);
  }

  SetRegKey("Filters","FoldersFilterFlags",FoldersFilter.Flags.Flags);
}

void FileFilter::SwapFilter()
{
  FileFilterParams *Temp[] = {&FoldersFilter};

  for (int j=0; j<3; j++)
  {
    FileFilterParams **Data;
    int count;

    if (j==0)
    {
      Data=FilterData;
      count=FilterDataCount;
    }
    else if (j==1)
    {
      Data=TempFilterData;
      count=TempFilterDataCount;
    }
    else
    {
      Data=Temp;
      count=1;
    }

    for (int I=0; I < count; I++)
    {
      FileFilterParams *CurFilterData=Data[I];

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
  }
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
  else if(strpbrk(DotPtr,",;"))
    sprintf(Mask,"\"*%s\"",DotPtr);
  else
    sprintf(Mask,"*%s",DotPtr);
  /* IS $ */

  // сначала поиск...
  unsigned int Cnt=ExtCount;
  if(lfind((const void *)Mask,(void *)*ExtPtr,&Cnt,NM,ExtSort))
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


void GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator)
{
  char Temp[32], Digit[16],*PtrDigit;
  int I;

  xstrncpy(Temp,Src,sizeof(Temp)-1);
  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const char *Ptr=Temp;
  while((Ptr=GetCommaWord(Ptr,Digit,Separator)) != NULL)
  {
    PtrDigit=Digit;
    while (*PtrDigit && !isdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=atoi(PtrDigit);
    ++I;
  }
}

void StrToDateTime(const char *CDate,const char *CTime,FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator)
{
  unsigned DateN[3],TimeN[3];
  SYSTEMTIME st;
  FILETIME lft;

  // Преобразуем введённые пользователем дату и время
  GetFileDateAndTime(CDate,DateN,DateSeparator);
  GetFileDateAndTime(CTime,TimeN,TimeSeparator);
  if(DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1)
  {
    // Пользователь оставил дату пустой, значит обнулим дату и время.
    memset(&ft,0,sizeof(ft));
    return;
  }

  memset(&st,0,sizeof(st));

  // "Оформим"
  switch(DateFormat)
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
  st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
  st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

  if (st.wYear<100)
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;

  // преобразование в "удобоваримый" формат
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,&ft);
  return;
}

enum enumFileFilterConfig {
    ID_FF_TITLE,

    ID_FF_NAME,
    ID_FF_NAMEEDIT,

    ID_FF_SEPARATOR1,

    ID_FF_MATCHMASK,
    ID_FF_MASKEDIT,

    ID_FF_SEPARATOR2,

    ID_FF_MATCHSIZE,
    ID_FF_SIZEDIVIDER,
    ID_FF_SIZEFROM,
    ID_FF_SIZEFROMEDIT,
    ID_FF_SIZETO,
    ID_FF_SIZETOEDIT,

    ID_FF_MATCHDATE,
    ID_FF_DATETYPE,
    ID_FF_DATEAFTER,
    ID_FF_DATEAFTEREDIT,
    ID_FF_TIMEAFTEREDIT,
    ID_FF_DATEBEFORE,
    ID_FF_DATEBEFOREEDIT,
    ID_FF_TIMEBEFOREEDIT,
    ID_FF_CURRENT,
    ID_FF_BLANK,

    ID_FF_SEPARATOR3,
    ID_FF_VSEPARATOR1,

    ID_FF_MATCHATTRIBUTES,
    ID_FF_READONLY,
    ID_FF_ARCHIVE,
    ID_FF_HIDDEN,
    ID_FF_SYSTEM,
    ID_FF_DIRECTORY,
    ID_FF_COMPRESSED,
    ID_FF_ENCRYPTED,
    ID_FF_NOTINDEXED,
    ID_FF_SPARSE,
    ID_FF_TEMP,
    ID_FF_REPARSEPOINT,

    ID_FF_SEPARATOR4,

    ID_FF_OK,
    ID_FF_RESET,
    ID_FF_CANCEL
};

LONG_PTR WINAPI FileFilterConfigDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_BTNCLICK:
    {
      if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK) //Current и Blank
      {
        FILETIME ft;
        char Date[16],Time[16];

        if (Param1==ID_FF_CURRENT)
        {
          GetSystemTimeAsFileTime(&ft);
          ConvertDate(ft,Date,Time,8,FALSE,FALSE,TRUE);
        }
        else
          *Date=*Time=0;

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(LONG_PTR)Date);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)Time);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(LONG_PTR)Date);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)Time);

        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,ID_FF_DATEAFTEREDIT,0);
        COORD r;
        r.X=r.Y=0;
        Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,ID_FF_DATEAFTEREDIT,(LONG_PTR)&r);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      else if (Param1==ID_FF_RESET) // Reset
      {
        // очистка диалога
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,(LONG_PTR)"*.*");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,(LONG_PTR)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,(LONG_PTR)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(LONG_PTR)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(LONG_PTR)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)"");

        /* 14.06.2004 KM
           Заменим BSTATE_UNCHECKED на BSTATE_3STATE, в данном
           случае это будет логичнее, т.с. дефолтное значение
        */
        for(int I=ID_FF_READONLY; I <= ID_FF_REPARSEPOINT; ++I)
          Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);

        // 6, 13 - позиции в списке
        struct FarListPos LPos={0,0};
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_SIZEDIVIDER,(LONG_PTR)&LPos);
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,(LONG_PTR)&LPos);

        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,BSTATE_UNCHECKED);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      break;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool FileFilterConfig(FileFilterParams *FF)
{
/*
    00000000001111111111222222222233333333334444444444555555555566666666667777777777
    01234567890123456789012345678901234567890123456789012345678901234567890123456789
00
01     +-------------------------- Фильтр операций --------------------------+
02     | Название фильтра:                                                   |
03     | "какое-то название                                                "|
04     +-------------------------- Параметры файла --------------------------+
05     | [ ] Совпадение с маской (масками)                                   |
06     |   *.*                                                              |
07     +------------------------------+--------------------------------------+
08     | [x] Размер в                 ¦ [x] Дата/время                       |
09     |              "мегабайтах   "¦               "модификация         "|
10     |   Больше или равен: "      " ¦   Начиная с:  "  .  .   " "  :  :  " |
11     |   Меньше или равен: "      " ¦   Заканчивая: "  .  .   " "  :  :  " |
12     |                              ¦                [ Текущая ] [ Сброс ] |
13     +------------------------------+--------------------------------------+
14     | [ ] Атрибуты                                                        |
15     |   [?] Только для чтения  [?] Каталог           [?] Разреженный      |
16     |   [?] Архивный           [?] Сжатый            [?] Временный        |
17     |   [?] Скрытый            [?] Зашифрованный     [?] Символ. связь    |
18     |   [?] Системный          [?] Неиндексируемый                        |
19     +---------------------------------------------------------------------+
20     |                  [ Ок ]  [ Очистить ]  [ Отмена ]                   |
21     +---------------------------------------------------------------------+
22
23
24
25
*/

  // Временная маска.
  CFileMask FileMask;
  int I;
  const char VerticalLine[] = {0x0C2,0x0B3,0x0B3,0x0B3,0x0B3,0x0B3,0x0C1,0};
  // Маска для ввода размеров файла
  const char DigitMask[] = "99999999999999999999";
  // История для маски файлов
  const char FilterMasksHistoryName[] = "FilterMasks";
  // Маски для диалога настройки
  char DateMask[16],DateStrAfter[16],DateStrBefore[16];
  char TimeMask[16],TimeStrAfter[16],TimeStrBefore[16];

  // Определение параметров даты и времени в системе.
  int DateSeparator=GetDateSeparator();
  int TimeSeparator=GetTimeSeparator();
  int DateFormat=GetDateFormat();

  switch(DateFormat)
  {
    case 0:
      // Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
      sprintf(DateMask,"99%c99%c9999",DateSeparator,DateSeparator);
      break;
    case 1:
      // Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
      sprintf(DateMask,"99%c99%c9999",DateSeparator,DateSeparator);
      break;
    default:
      // Маска даты для формата YYYY.MM.DD
      sprintf(DateMask,"9999%c99%c99",DateSeparator,DateSeparator);
      break;
  }
  // Маска времени
  sprintf(TimeMask,"99%c99%c99",TimeSeparator,TimeSeparator);

  struct DialogData FilterDlgData[]=
  {
  /* 00 */DI_DOUBLEBOX,3,1,73,21,0,0,DIF_SHOWAMPERSAND,0,(char *)MFileFilterTitle,

  /* 01 */DI_TEXT,5,2,0,2,1,0,0,0,(char *)MFileFilterName,
  /* 02 */DI_EDIT,5,3,71,3,0,0,0,0,"",

  /* 03 */DI_TEXT,0,4,0,4,0,0,DIF_SEPARATOR,0,"",

  /* 04 */DI_CHECKBOX,5,5,0,5,0,0,DIF_AUTOMATION,0,(char *)MFileFilterMatchMask,
  /* 05 */DI_EDIT,7,6,71,6,0,(DWORD_PTR)FilterMasksHistoryName,DIF_HISTORY,0,"",

  /* 06 */DI_TEXT,0,7,0,7,0,0,DIF_SEPARATOR,0,"",

  /* 07 */DI_CHECKBOX,5,8,0,8,0,0,DIF_AUTOMATION,0,(char *)MFileFilterSize,
  /* 08 */DI_COMBOBOX,18,9,32,9,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
  /* 09 */DI_TEXT,7,10,23,10,0,0,0,0,(char *)MFileFilterSizeFrom,
  /* 10 */DI_FIXEDIT,18,10,32,10,0,(DWORD_PTR)DigitMask,DIF_MASKEDIT,0,"",
  /* 11 */DI_TEXT,7,11,23,11,0,0,0,0,(char *)MFileFilterSizeTo,
  /* 12 */DI_FIXEDIT,18,11,32,11,0,(DWORD_PTR)DigitMask,DIF_MASKEDIT,0,"",

  /* 14 */DI_CHECKBOX,36,8,0,8,0,0,DIF_AUTOMATION,0,(char *)MFileFilterDate,
  /* 15 */DI_COMBOBOX,50,9,71,9,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
  /* 16 */DI_TEXT,38,10,48,10,0,0,0,0,(char *)MFileFilterAfter,
  /* 17 */DI_FIXEDIT,50,10,59,10,0,(DWORD_PTR)DateMask,DIF_MASKEDIT,0,"",
  /* 18 */DI_FIXEDIT,64,10,71,10,0,(DWORD_PTR)TimeMask,DIF_MASKEDIT,0,"",
  /* 19 */DI_TEXT,38,11,48,11,0,0,0,0,(char *)MFileFilterBefore,
  /* 20 */DI_FIXEDIT,50,11,59,11,0,(DWORD_PTR)DateMask,DIF_MASKEDIT,0,"",
  /* 21 */DI_FIXEDIT,64,11,71,11,0,(DWORD_PTR)TimeMask,DIF_MASKEDIT,0,"",
  /* 22 */DI_BUTTON,0,12,0,12,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterCurrent,
  /* 23 */DI_BUTTON,0,12,71,12,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterBlank,

  /* 24 */DI_TEXT,0,13,0,13,0,0,DIF_SEPARATOR,0,"",

  /* 13 */DI_VTEXT,34,7,34,13,0,0,0,0,(char *)VerticalLine,

  /* 25 */DI_CHECKBOX, 5,14,0,14,0,0,DIF_AUTOMATION,0,(char *)MFileFilterAttr,
  /* 26 */DI_CHECKBOX, 7,15,0,15,0,0,DIF_3STATE,0,(char *)MFileFilterAttrR,
  /* 27 */DI_CHECKBOX, 7,16,0,16,0,0,DIF_3STATE,0,(char *)MFileFilterAttrA,
  /* 28 */DI_CHECKBOX, 7,17,0,17,0,0,DIF_3STATE,0,(char *)MFileFilterAttrH,
  /* 29 */DI_CHECKBOX, 7,18,0,18,0,0,DIF_3STATE,0,(char *)MFileFilterAttrS,
  /* 30 */DI_CHECKBOX,29,15,0,15,0,0,DIF_3STATE,0,(char *)MFileFilterAttrD,
  /* 31 */DI_CHECKBOX,29,16,0,16,0,0,DIF_3STATE,0,(char *)MFileFilterAttrC,
  /* 32 */DI_CHECKBOX,29,17,0,17,0,0,DIF_3STATE,0,(char *)MFileFilterAttrE,
  /* 33 */DI_CHECKBOX,29,18,0,18,0,0,DIF_3STATE,0,(char *)MFileFilterAttrNI,
  /* 34 */DI_CHECKBOX,51,15,0,15,0,0,DIF_3STATE,0,(char *)MFileFilterAttrSparse,
  /* 35 */DI_CHECKBOX,51,16,0,16,0,0,DIF_3STATE,0,(char *)MFileFilterAttrT,
  /* 36 */DI_CHECKBOX,51,17,0,17,0,0,DIF_3STATE,0,(char *)MFileFilterAttrReparse,

  /* 37 */DI_TEXT,0,19,0,19,0,0,DIF_SEPARATOR,0,"",

  /* 38 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,1,(char *)MFileFilterOk,
  /* 39 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MFileFilterReset,
  /* 40 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(char *)MFileFilterCancel,
  };

  MakeDialogItems(FilterDlgData,FilterDlg);

  //FilterDlg[ID_FF_SIZEDIVIDER].X1=FilterDlg[ID_FF_MATCHSIZE].X1+strlen(FilterDlg[ID_FF_MATCHSIZE].Data)-(strchr(FilterDlg[ID_FF_MATCHSIZE].Data,'&')?1:0)+5;
  //FilterDlg[ID_FF_SIZEDIVIDER].X2+=FilterDlg[ID_FF_SIZEDIVIDER].X1;
  //FilterDlg[ID_FF_DATETYPE].X1=FilterDlg[ID_FF_MATCHDATE].X1+strlen(FilterDlg[ID_FF_MATCHDATE].Data)-(strchr(FilterDlg[ID_FF_MATCHDATE].Data,'&')?1:0)+5;
  //FilterDlg[ID_FF_DATETYPE].X2+=FilterDlg[ID_FF_DATETYPE].X1;

  FilterDlg[ID_FF_BLANK].X1=FilterDlg[ID_FF_BLANK].X2-strlen(FilterDlg[ID_FF_BLANK].Data)+(strchr(FilterDlg[ID_FF_BLANK].Data,'&')?1:0)-3;
  FilterDlg[ID_FF_CURRENT].X2=FilterDlg[ID_FF_BLANK].X1-2;
  FilterDlg[ID_FF_CURRENT].X1=FilterDlg[ID_FF_CURRENT].X2-strlen(FilterDlg[ID_FF_CURRENT].Data)+(strchr(FilterDlg[ID_FF_CURRENT].Data,'&')?1:0)-3;

  xstrncpy(FilterDlg[ID_FF_NAMEEDIT].Data,FF->GetTitle(),sizeof(FilterDlg[ID_FF_NAMEEDIT].Data));

  const char *Mask;
  FilterDlg[ID_FF_MATCHMASK].Selected=FF->GetMask(&Mask);
  xstrncpy(FilterDlg[ID_FF_MASKEDIT].Data,Mask,sizeof(FilterDlg[3].Data));
  if (!FilterDlg[ID_FF_MATCHMASK].Selected)
    FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

  // Лист для комбобокса: байты - килобайты
  FarList SizeList;
  FarListItem TableItemSize[FSIZE_IN_LAST];
  // Настройка списка множителей для типа размера
  SizeList.Items=TableItemSize;
  SizeList.ItemsNumber=FSIZE_IN_LAST;

  memset(TableItemSize,0,sizeof(FarListItem)*FSIZE_IN_LAST);
  for(int i=0; i < FSIZE_IN_LAST; ++i)
    xstrncpy(TableItemSize[i].Text,MSG(MFileFilterSizeInBytes+i),sizeof(TableItemSize[i].Text)-1);

  DWORD SizeType;
  __int64 SizeAbove, SizeBelow;
  FilterDlg[ID_FF_MATCHSIZE].Selected=FF->GetSize(&SizeType,&SizeAbove,&SizeBelow);
  FilterDlg[ID_FF_SIZEDIVIDER].ListItems=&SizeList;

  xstrncpy(FilterDlg[ID_FF_SIZEDIVIDER].Data,TableItemSize[SizeType].Text,sizeof(FilterDlg[ID_FF_SIZEDIVIDER].Data));

  if (SizeAbove != _i64(-1))
    _ui64toa(SizeAbove,FilterDlg[ID_FF_SIZEFROMEDIT].Data,10);

  if (SizeBelow != _i64(-1))
    _ui64toa(SizeBelow,FilterDlg[ID_FF_SIZETOEDIT].Data,10);

  if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
    for(I=ID_FF_SIZEDIVIDER; I <= ID_FF_SIZETOEDIT; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  // Лист для комбобокса времени файла
  FarList DateList;
  FarListItem TableItemDate[DATE_COUNT];
  // Настройка списка типов дат файла
  DateList.Items=TableItemDate;
  DateList.ItemsNumber=DATE_COUNT;

  memset(TableItemDate,0,sizeof(FarListItem)*DATE_COUNT);
  xstrncpy(TableItemDate[0].Text,MSG(MFileFilterModified),sizeof(TableItemDate[0].Text)-1);
  xstrncpy(TableItemDate[1].Text,MSG(MFileFilterCreated),sizeof(TableItemDate[1].Text)-1);
  xstrncpy(TableItemDate[2].Text,MSG(MFileFilterOpened),sizeof(TableItemDate[2].Text)-1);

  DWORD DateType;
  FILETIME DateAfter, DateBefore;
  FilterDlg[ID_FF_MATCHDATE].Selected=FF->GetDate(&DateType,&DateAfter,&DateBefore);
  FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;

  xstrncpy(FilterDlg[ID_FF_DATETYPE].Data,TableItemDate[DateType].Text,sizeof(FilterDlg[ID_FF_DATETYPE].Data));

  ConvertDate(DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,8,FALSE,FALSE,TRUE);
  ConvertDate(DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,8,FALSE,FALSE,TRUE);

  if (!FilterDlg[ID_FF_MATCHDATE].Selected)
    for(I=ID_FF_DATETYPE; I <= ID_FF_BLANK; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  DWORD AttrSet, AttrClear;
  FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF->GetAttr(&AttrSet,&AttrClear);
  FilterDlg[ID_FF_READONLY].Selected=(AttrSet & FILE_ATTRIBUTE_READONLY?1:AttrClear & FILE_ATTRIBUTE_READONLY?0:2);
  FilterDlg[ID_FF_ARCHIVE].Selected=(AttrSet & FILE_ATTRIBUTE_ARCHIVE?1:AttrClear & FILE_ATTRIBUTE_ARCHIVE?0:2);
  FilterDlg[ID_FF_HIDDEN].Selected=(AttrSet & FILE_ATTRIBUTE_HIDDEN?1:AttrClear & FILE_ATTRIBUTE_HIDDEN?0:2);
  FilterDlg[ID_FF_SYSTEM].Selected=(AttrSet & FILE_ATTRIBUTE_SYSTEM?1:AttrClear & FILE_ATTRIBUTE_SYSTEM?0:2);
  FilterDlg[ID_FF_COMPRESSED].Selected=(AttrSet & FILE_ATTRIBUTE_COMPRESSED?1:AttrClear & FILE_ATTRIBUTE_COMPRESSED?0:2);
  FilterDlg[ID_FF_ENCRYPTED].Selected=(AttrSet & FILE_ATTRIBUTE_ENCRYPTED?1:AttrClear & FILE_ATTRIBUTE_ENCRYPTED?0:2);
  FilterDlg[ID_FF_DIRECTORY].Selected=(AttrSet & FILE_ATTRIBUTE_DIRECTORY?1:AttrClear & FILE_ATTRIBUTE_DIRECTORY?0:2);
  FilterDlg[ID_FF_NOTINDEXED].Selected=(AttrSet & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?1:AttrClear & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?0:2);
  FilterDlg[ID_FF_SPARSE].Selected=(AttrSet & FILE_ATTRIBUTE_SPARSE_FILE?1:AttrClear & FILE_ATTRIBUTE_SPARSE_FILE?0:2);
  FilterDlg[ID_FF_TEMP].Selected=(AttrSet & FILE_ATTRIBUTE_TEMPORARY?1:AttrClear & FILE_ATTRIBUTE_TEMPORARY?0:2);
  FilterDlg[ID_FF_REPARSEPOINT].Selected=(AttrSet & FILE_ATTRIBUTE_REPARSE_POINT?1:AttrClear & FILE_ATTRIBUTE_REPARSE_POINT?0:2);

  if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
  {
    for(I=ID_FF_READONLY; I <= ID_FF_REPARSEPOINT; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;
  }

  Dialog Dlg(FilterDlg,sizeof(FilterDlg)/sizeof(FilterDlg[0]),FileFilterConfigDlgProc);

  Dlg.SetHelp("OpFilter");
  Dlg.SetPosition(-1,-1,77,23);

  Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEDIVIDER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETO,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_NOTINDEXED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SPARSE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_TEMP,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_REPARSEPOINT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,0,0,DIF_DISABLE);

  for (;;)
  {
    Dlg.ClearDone();
    Dlg.Process();
    int ExitCode=Dlg.GetExitCode();

    if (ExitCode==ID_FF_OK) // Ok
    {
      // Если введённая пользователем маска не корректна, тогда вернёмся в диалог
      if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set(FilterDlg[ID_FF_MASKEDIT].Data,0))
        continue;

      FF->SetTitle(FilterDlg[ID_FF_NAMEEDIT].Data);

      FF->SetMask(FilterDlg[ID_FF_MATCHMASK].Selected,
                  FilterDlg[ID_FF_MASKEDIT].Data);

      if(!*RemoveExternalSpaces(FilterDlg[ID_FF_SIZEFROMEDIT].Data))
        SizeAbove=_i64(-1);
      else
        SizeAbove=_atoi64(FilterDlg[ID_FF_SIZEFROMEDIT].Data);

      if(!*RemoveExternalSpaces(FilterDlg[ID_FF_SIZETOEDIT].Data))
        SizeBelow=_i64(-1);
      else
        SizeBelow=_atoi64(FilterDlg[ID_FF_SIZETOEDIT].Data);

      FF->SetSize(FilterDlg[ID_FF_MATCHSIZE].Selected,
                  FilterDlg[ID_FF_SIZEDIVIDER].ListPos,
                  SizeAbove,
                  SizeBelow);

      StrToDateTime(FilterDlg[ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,DateAfter,DateFormat,DateSeparator,TimeSeparator);
      StrToDateTime(FilterDlg[ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,DateBefore,DateFormat,DateSeparator,TimeSeparator);

      FF->SetDate(FilterDlg[ID_FF_MATCHDATE].Selected,
                  FilterDlg[ID_FF_DATETYPE].ListPos,
                  DateAfter,
                  DateBefore);

      AttrSet=0;
      AttrClear=0;

      AttrSet|=(FilterDlg[ID_FF_READONLY].Selected==1?FILE_ATTRIBUTE_READONLY:0);
      AttrSet|=(FilterDlg[ID_FF_ARCHIVE].Selected==1?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrSet|=(FilterDlg[ID_FF_HIDDEN].Selected==1?FILE_ATTRIBUTE_HIDDEN:0);
      AttrSet|=(FilterDlg[ID_FF_SYSTEM].Selected==1?FILE_ATTRIBUTE_SYSTEM:0);
      AttrSet|=(FilterDlg[ID_FF_COMPRESSED].Selected==1?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrSet|=(FilterDlg[ID_FF_ENCRYPTED].Selected==1?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrSet|=(FilterDlg[ID_FF_DIRECTORY].Selected==1?FILE_ATTRIBUTE_DIRECTORY:0);
      AttrSet|=(FilterDlg[ID_FF_NOTINDEXED].Selected==1?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
      AttrSet|=(FilterDlg[ID_FF_SPARSE].Selected==1?FILE_ATTRIBUTE_SPARSE_FILE:0);
      AttrSet|=(FilterDlg[ID_FF_TEMP].Selected==1?FILE_ATTRIBUTE_TEMPORARY:0);
      AttrSet|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==1?FILE_ATTRIBUTE_REPARSE_POINT:0);
      AttrClear|=(FilterDlg[ID_FF_READONLY].Selected==0?FILE_ATTRIBUTE_READONLY:0);
      AttrClear|=(FilterDlg[ID_FF_ARCHIVE].Selected==0?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrClear|=(FilterDlg[ID_FF_HIDDEN].Selected==0?FILE_ATTRIBUTE_HIDDEN:0);
      AttrClear|=(FilterDlg[ID_FF_SYSTEM].Selected==0?FILE_ATTRIBUTE_SYSTEM:0);
      AttrClear|=(FilterDlg[ID_FF_COMPRESSED].Selected==0?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrClear|=(FilterDlg[ID_FF_ENCRYPTED].Selected==0?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrClear|=(FilterDlg[ID_FF_DIRECTORY].Selected==0?FILE_ATTRIBUTE_DIRECTORY:0);
      AttrClear|=(FilterDlg[ID_FF_NOTINDEXED].Selected==0?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
      AttrClear|=(FilterDlg[ID_FF_SPARSE].Selected==0?FILE_ATTRIBUTE_SPARSE_FILE:0);
      AttrClear|=(FilterDlg[ID_FF_TEMP].Selected==0?FILE_ATTRIBUTE_TEMPORARY:0);
      AttrClear|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==0?FILE_ATTRIBUTE_REPARSE_POINT:0);

      FF->SetAttr(FilterDlg[ID_FF_MATCHATTRIBUTES].Selected,
                  AttrSet,
                  AttrClear);

      return true;
    }
    else
      break;
  }

  return false;
}
