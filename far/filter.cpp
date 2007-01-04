/*
filter.cpp

Фильтр (Ctrl-I)

*/

#include "headers.hpp"
#pragma hdrstop

#include "CFileMask.hpp"
#include "filter.hpp"
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

static int _cdecl ExtSort(const void *el1,const void *el2);

struct FilterDataRecord
{
  string strTitle;
  wchar_t *Masks;
  int LeftPanelInclude;
  int LeftPanelExclude;
  int RightPanelInclude;
  int RightPanelExclude;
};

static FilterDataRecord **FilterData=NULL;
static int FilterDataCount=0;

PanelFilter::PanelFilter(Panel *HostPanel)
{
  IncludeMaskIsOK=ExcludeMaskIsOK=false;
  IncludeMaskStr=ExcludeMaskStr=NULL;
  PanelFilter::HostPanel=HostPanel;
  AddMasks(NULL,0);
  FilterDataRecord *CurFilterData=NULL;
  for (int I=0; I < FilterDataCount; I++)
  {
    CurFilterData = FilterData[I];

    if (HostPanel==CtrlObject->Cp()->LeftPanel)
    {
      if (CurFilterData->LeftPanelInclude)
        AddMasks(CurFilterData->Masks,FALSE);
      else
        if (CurFilterData->LeftPanelExclude)
          AddMasks(CurFilterData->Masks,TRUE);
    }
    else
    {
      if (CurFilterData->RightPanelInclude)
        AddMasks(CurFilterData->Masks,FALSE);
      else
        if (CurFilterData->RightPanelExclude)
          AddMasks(CurFilterData->Masks,TRUE);
    }
  }
}


PanelFilter::~PanelFilter()
{
  if(IncludeMaskStr) xf_free(IncludeMaskStr);
  if(ExcludeMaskStr) xf_free(ExcludeMaskStr);
}


void PanelFilter::FilterEdit()
{
  /* $ 13.10.2000 tran
     NeedUpdate - если фильтр менялся по f4, ins, del
     панель перерисуется при выходе по esc
  */
  int FirstCall=TRUE,Pos=0,NeedUpdate=0;
  while (Pos!=-1)
  {
    Pos=ShowFilterMenu(Pos,FirstCall,&NeedUpdate);
    FirstCall=FALSE;
  }
}


int PanelFilter::ShowFilterMenu(int Pos,int FirstCall,int *NeedUpdate)
{
  string strTitle, strMasks;
  struct MenuItemEx ListItem;
  int ExitCode;
  {
    int I;
    VMenu FilterList(UMSG(MFilterTitle),NULL,0,TRUE,ScrY-6);

    FilterList.SetHelp(L"Filter");
    FilterList.SetPosition(-1,-1,0,0);
    FilterList.SetBottomTitle(UMSG(MFilterBottom));
    /* $ 16.06.2001 KM
       ! Добавление WRAPMODE в меню.
    */
    FilterList.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
    /* KM $ */

    if (Pos>FilterDataCount)
      Pos=0;

    for (I=0;I<FilterDataCount;I++)
    {
      ListItem.Clear ();

      ListItem.strName.Format (L"%-30.30s %c %-30.30s",(const wchar_t*)FilterData[I]->strTitle,VerticalLine,FilterData[I]->Masks);

      if(I == Pos)
        ListItem.Flags|=LIF_SELECTED;

      if (HostPanel==CtrlObject->Cp()->LeftPanel)
      {
        if (FilterData[I]->LeftPanelInclude)
          ListItem.SetCheck(L'+');
        else if (FilterData[I]->LeftPanelExclude)
          ListItem.SetCheck(L'-');
      }
      else
      {
        if (FilterData[I]->RightPanelInclude)
          ListItem.SetCheck(L'+');
        else if (FilterData[I]->RightPanelExclude)
          ListItem.SetCheck(L'-');
      }

      FilterList.SetUserData(FilterData[I]->Masks,0,
            FilterList.AddItemW(&ListItem));
    }

    ListItem.Clear ();

    if (FilterDataCount==0)
    {
      /* $ 11.10.2001 VVM
        ! Если пользовательских фильтров нет, то надпись во всю ширину меню */
      ListItem.strName.Format (L"%-60.60s",UMSG(MNoCustomFilters));
      /* VVM $ */
      if(!Pos)
        ListItem.Flags|=LIF_SELECTED;
      FilterList.AddItemW(&ListItem);
    }

    string strFileName;
    wchar_t *ExtPtr=NULL;
    FAR_FIND_DATA_EX fdata;
    int FileAttr,ExtCount=0;

    if (HostPanel->GetMode()==NORMAL_PANEL)
    {
      string strCurDir;
      HostPanel->GetCurDirW(strCurDir);

      ScanTree ScTree(FALSE,FALSE);
      ScTree.SetFindPathW(strCurDir,L"*.*");
      while (ScTree.GetNextNameW(&fdata, strFileName))
        if(!ParseAndAddMasks(&ExtPtr,fdata.strFileName,fdata.dwFileAttributes,ExtCount))
          break;
    }
    else
    {
      for (I=0;HostPanel->GetFileNameW(strFileName,I,FileAttr);I++)
        if(!ParseAndAddMasks(&ExtPtr,strFileName,FileAttr,ExtCount))
          break;
    }

    far_qsort((void *)ExtPtr,ExtCount,NM,ExtSort);

    ListItem.Clear ();

    if (ExtCount>0)
    {
      ListItem.Flags|=LIF_SEPARATOR;
      FilterList.AddItemW(&ListItem);
    }

    ListItem.Flags&=~LIF_SEPARATOR;

    for (I=0;I<ExtCount;I++)
    {
      wchar_t *CurExtPtr=ExtPtr+I*NM;
      ListItem.strName.Format (L"%-30.30s %c %-30.30s", UMSG(MPanelFileType),VerticalLine,CurExtPtr);
      ListItem.SetCheck(CurExtPtr[wcslen(CurExtPtr)+1]);
      ListItem.Flags&=~LIF_SELECTED;
      FilterList.SetUserData(CurExtPtr,0,FilterList.AddItemW(&ListItem));
    }
    /* $ 13.07.2000 SVS
       ни кто не вызывал запрос памяти через new :-)
    */
    xf_free(ExtPtr);
    /* SVS $ */

    if (!FirstCall)
      ProcessSelection(&FilterList);

    FilterList.Show();

    int SelPos;

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
          SelPos=FilterList.GetSelectPos();
          if (SelPos<FilterDataCount)
          {
            strTitle = FilterData[SelPos]->strTitle;
            strMasks = FilterData[SelPos]->Masks;

            if (EditRecord(strTitle,strMasks))
            {
              wchar_t *Ptr;
              if((Ptr=(wchar_t *)xf_realloc(FilterData[SelPos]->Masks,(strMasks.GetLength()+1)*sizeof(wchar_t))) != NULL)
              {
                FilterData[SelPos]->strTitle = strTitle;
                FilterData[SelPos]->Masks=Ptr;
                wcscpy(FilterData[SelPos]->Masks, strMasks);
                SaveFilters();
                *NeedUpdate=1;
                return(SelPos);
              }
              break;
            }
          }
          else
          {
            MessageW(MSG_WARNING,1,UMSG(MFilterTitle),UMSG(MCanEditCustomFilterOnly),UMSG(MOk));
            continue;
          }
          break;
        }

        case KEY_INS:
        {
          SelPos=FilterList.GetSelectPos();
          wchar_t *Ptr;

          if (SelPos>FilterDataCount)
            SelPos=FilterDataCount;

          strTitle=strMasks=L"";
          if (EditRecord(strTitle,strMasks))
          {
            FilterDataRecord **NewFilterData;

            if((Ptr=(wchar_t *)xf_malloc((strMasks.GetLength()+1)*sizeof (wchar_t))) == NULL)
              break;

            if ((NewFilterData=(FilterDataRecord **)xf_realloc(FilterData,sizeof(*FilterData)*FilterDataCount))==NULL)
            {
              xf_free(Ptr);
              break;
            }

            FilterData=NewFilterData;

            for (int I=FilterDataCount-1;I>=SelPos;I--)
              FilterData[I+1]=FilterData[I];

            FilterDataCount++;

            FilterData[SelPos] = new FilterDataRecord;

            FilterData[SelPos]->strTitle = strTitle;
            FilterData[SelPos]->Masks=Ptr;
            wcscpy(FilterData[SelPos]->Masks, strMasks);
            FilterData[SelPos]->LeftPanelInclude=0;
            FilterData[SelPos]->LeftPanelExclude=0;
            FilterData[SelPos]->RightPanelInclude=0;
            FilterData[SelPos]->RightPanelExclude=0;

            SaveFilters();
            SaveSelection();
            *NeedUpdate=1;
            return(SelPos);
          }
          break;
        }

        case KEY_DEL:
        {
          SelPos=FilterList.GetSelectPos();
          if (SelPos<FilterDataCount)
          {
            string strQuotedTitle;
            strQuotedTitle.Format (L"\"%s\"", (const wchar_t*)FilterData[SelPos]->strTitle);
            if (MessageW(0,2,UMSG(MFilterTitle),UMSG(MAskDeleteFilter),
                        strQuotedTitle,UMSG(MDelete),UMSG(MCancel))==0)
            {
              if(FilterData[SelPos]->Masks)
                xf_free(FilterData[SelPos]->Masks);

              delete FilterData[SelPos]; //!!!

              for (int I=SelPos+1;I<FilterDataCount;I++)
                FilterData[I-1]=FilterData[I];

              FilterDataCount--;
              SaveFilters();
              SaveSelection();

              if (SelPos>=FilterDataCount && SelPos>0)
                SelPos--;

              *NeedUpdate=1;
              return(SelPos);
            }
          }
          else
          {
            MessageW(MSG_WARNING,1,UMSG(MFilterTitle),UMSG(MCanDeleteCustomFilterOnly),UMSG(MOk));
            continue;
          }
        }

        default:
        {
          FilterList.ProcessInput();
          break;
        }
      }
    }

    ExitCode=FilterList.Modal::GetExitCode();
    if (ExitCode!=-1 || *NeedUpdate)
      ProcessSelection(&FilterList);
  }

  if (ExitCode!=-1 || *NeedUpdate)
  {
    HostPanel->Update(UPDATE_KEEP_SELECTION);
    HostPanel->Redraw();
  }

  return(-1);
}


void PanelFilter::ProcessSelection(VMenu *FilterList)
{
  string strMasks;

  AddMasks(NULL,0);
  FilterDataRecord *CurFilterData = NULL;
  for (int I=0; I < FilterList->GetItemCount(); I++)
  {
    CurFilterData = FilterData[I];

    int Check=FilterList->GetSelection(I);

    if ( Check && FilterList->GetUserDataSize (I) )
    {
        int nSize = FilterList->GetUserDataSize (I);

        wchar_t *Masks = strMasks.GetBuffer (nSize);

        FilterList->GetUserData(Masks, nSize+1, I);

        strMasks.ReleaseBuffer ();

        AddMasks(strMasks,Check==L'-');
    }

    if (I<FilterDataCount)
    {
      if (HostPanel==CtrlObject->Cp()->LeftPanel)
        CurFilterData->LeftPanelInclude=CurFilterData->LeftPanelExclude=0;

      if (HostPanel==CtrlObject->Cp()->RightPanel)
        CurFilterData->RightPanelInclude=CurFilterData->RightPanelExclude=0;

      if (Check=='+')
      {
        if (HostPanel==CtrlObject->Cp()->LeftPanel)
          CurFilterData->LeftPanelInclude=TRUE;
        else
          CurFilterData->RightPanelInclude=TRUE;
      }

      if (Check=='-')
      {
        if (HostPanel==CtrlObject->Cp()->LeftPanel)
          CurFilterData->LeftPanelExclude=TRUE;
        else
          CurFilterData->RightPanelExclude=TRUE;
      }
    }
  }
}


/* $ 01.07.2001 IS
   Используем нормальные классы для работы с масками файлов
*/
void PanelFilter::AddMasks(const wchar_t *Masks,int Exclude)
{
  if (Masks==NULL)
  {
    // Освободим память
    IncludeMask.Free();
    ExcludeMask.Free();

    if(IncludeMaskStr)
      xf_free(IncludeMaskStr);

    if(ExcludeMaskStr)
      xf_free(ExcludeMaskStr);

    IncludeMaskStr=ExcludeMaskStr=NULL;
    IncludeMaskIsOK=ExcludeMaskIsOK=FALSE;

    return;
  }

  int AddSize=wcslen(Masks)+4, OldSize;

  if(Exclude)
  {
    ExcludeMask.Free();

    OldSize=ExcludeMaskStr?wcslen(ExcludeMaskStr):0;
    wchar_t *NewExcludeMaskStr=(wchar_t *)xf_realloc(ExcludeMaskStr,(AddSize+OldSize)*sizeof(wchar_t));
    if (NewExcludeMaskStr==NULL)
       return;

    ExcludeMaskStr=NewExcludeMaskStr;
    if(OldSize)
    {
      if(ExcludeMaskStr[OldSize-1]!=L',')
      {
        ExcludeMaskStr[OldSize]=L',';
        ExcludeMaskStr[OldSize+1]=0;
      }
    }
    else
      *ExcludeMaskStr=0;

    wcscat(ExcludeMaskStr, Masks);
    ExcludeMaskIsOK=ExcludeMask.Set(ExcludeMaskStr, FMF_SILENT);
  }
  else
  {
    IncludeMask.Free();

    OldSize=IncludeMaskStr?wcslen(IncludeMaskStr):0;
    wchar_t *NewIncludeMaskStr=(wchar_t *)xf_realloc(IncludeMaskStr,(AddSize+OldSize)*sizeof(wchar_t));

    if (NewIncludeMaskStr==NULL)
       return;

    IncludeMaskStr=NewIncludeMaskStr;
    if(OldSize)
    {
      if (IncludeMaskStr[OldSize-1]!=L',')
      {
        IncludeMaskStr[OldSize]=L',';
        IncludeMaskStr[OldSize+1]=0;
      }
    }
    else
      *IncludeMaskStr=0;

    wcscat(IncludeMaskStr, Masks);
    IncludeMaskIsOK=IncludeMask.Set(IncludeMaskStr, FMF_SILENT);
  }
}

int PanelFilter::CheckNameW(const wchar_t *Name)
{
  if (ExcludeMaskIsOK && ExcludeMask.Compare(Name))
    return FALSE;
  else
    return IncludeMaskIsOK?IncludeMask.Compare(Name):TRUE;
}

/* IS $ */

bool PanelFilter::IsEnabled()
{
  return IncludeMaskIsOK || ExcludeMaskIsOK;
}

void PanelFilter::InitFilter()
{
  string strRegKey;

  FilterData=NULL;
  FilterDataCount=0;

  while (1)
  {
    string strFilterTitle, strFilterMask;
    wchar_t *Ptr;

    strRegKey.Format (L"Filters\\Filter%d", FilterDataCount);

    GetRegKeyW(strRegKey, L"Title", strFilterTitle, L"");

    if (!GetRegKeyW(strRegKey,L"Mask",strFilterMask,L""))
      break;

    if((Ptr=(wchar_t *)xf_malloc((strFilterMask.GetLength()+1)*sizeof (wchar_t))) == NULL)
      break;

    FilterDataRecord **NewFilterData;
    if ((NewFilterData=(FilterDataRecord **)xf_realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
    {
      xf_free(Ptr);
      break;
    }
    FilterData=NewFilterData;

    FilterData[FilterDataCount] = new FilterDataRecord;

    FilterData[FilterDataCount]->strTitle = strFilterTitle;
    FilterData[FilterDataCount]->Masks=Ptr;
    wcscpy (FilterData[FilterDataCount]->Masks, strFilterMask);
    GetRegKeyW(strRegKey,L"LeftInclude",FilterData[FilterDataCount]->LeftPanelInclude,0);
    GetRegKeyW(strRegKey,L"LeftExclude",FilterData[FilterDataCount]->LeftPanelExclude,0);
    GetRegKeyW(strRegKey,L"RightInclude",FilterData[FilterDataCount]->RightPanelInclude,0);
    GetRegKeyW(strRegKey,L"RightExclude",FilterData[FilterDataCount]->RightPanelExclude,0);
    FilterDataCount++;
  }
}


void PanelFilter::CloseFilter()
{
  if(FilterData)
  {
    for(int I=0; I < FilterDataCount; ++I)
    {
      if(FilterData[I]->Masks)
        xf_free(FilterData[I]->Masks);

      delete FilterData[I];
    }

    xf_free(FilterData);
  }
}


void PanelFilter::SaveSelection()
{
  string strRegKey;

  for (int I=0;I <FilterDataCount;I++)
  {
    strRegKey.Format (L"Filters\\Filter%d", I);
    SetRegKeyW(strRegKey,L"LeftInclude",FilterData[I]->LeftPanelInclude);
    SetRegKeyW(strRegKey,L"LeftExclude",FilterData[I]->LeftPanelExclude);
    SetRegKeyW(strRegKey,L"RightInclude",FilterData[I]->RightPanelInclude);
    SetRegKeyW(strRegKey,L"RightExclude",FilterData[I]->RightPanelExclude);
  }
}


void PanelFilter::SaveFilters()
{
  FilterDataRecord *CurFilterData=NULL;
  string strRegKey;

  for (int I=0; I < FilterDataCount+5; I++)
  {
    CurFilterData = FilterData[I];

    strRegKey.Format (L"Filters\\Filter%d", I);

    if (I>=FilterDataCount)
      DeleteRegKeyW(strRegKey);
    else
    {
      SetRegKeyW(strRegKey,L"Title",CurFilterData->strTitle);
      SetRegKeyW(strRegKey,L"Mask",CurFilterData->Masks);
    }
  }
}


int PanelFilter::EditRecord(string &strTitle, string &strMasks)
{
  const wchar_t *HistoryName=L"Masks";
  static struct DialogDataEx EditDlgData[]=
  {
    /* 0 */ DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(const wchar_t *)MFilterTitle,
    /* 1 */ DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MEnterFilterTitle,
    /* 2 */ DI_EDIT,5,3,70,3,1,0,0,0,L"",
    /* 3 */ DI_TEXT,5,4,0,0,0,0,0,0,(const wchar_t *)MFilterMasks,
    /* 4 */ DI_EDIT,5,5,70,5,0,(DWORD_PTR)HistoryName,DIF_HISTORY,0,L"",
    /* 5 */ DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 6 */ DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    /* 7 */ DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
   };

  MakeDialogItemsEx(EditDlgData,EditDlg);

  EditDlg[2].strData = strTitle;
  EditDlg[4].strData = strMasks;
  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp(L"Filter");
    Dlg.SetPosition(-1,-1,76,10);
    /* $ 01.07.2001 IS
       теперь введенные маски проверяются на корректность
    */
    CFileMaskW CheckMask;
    for(;;)
    {
      Dlg.ClearDone();
      Dlg.Process();
      /* $ 18.09.2002 DJ
         сообщение, если не введена маска
      */

      strMasks = EditDlg[4].strData;

      if (Dlg.GetExitCode()!=6)
        return(FALSE);

      if ( strMasks.IsEmpty() )
      {
        MessageW (MSG_DOWN|MSG_WARNING,1,UMSG(MWarning),UMSG(MAssocNeedMask), UMSG(MOk));
        continue;
      }

      /* $ 16.03.2002 IS
         В Фильтрах тоже можно использовать маски-иключения.
      */
      if(CheckMask.Set(strMasks, 0))
      /* IS $ */
        break;
      /* DJ $ */
    }
    /* IS $ */
  }

  strTitle = EditDlg[2].strData;
  return(TRUE);
}


void PanelFilter::SwapFilter()
{
  FilterDataRecord *CurFilterData=NULL;
  for (int I=0; I < FilterDataCount; I++)
  {
    CurFilterData = FilterData[I];
    int Swap=CurFilterData->LeftPanelInclude;
    CurFilterData->LeftPanelInclude=CurFilterData->RightPanelInclude;
    CurFilterData->RightPanelInclude=Swap;

    Swap=CurFilterData->LeftPanelExclude;
    CurFilterData->LeftPanelExclude=CurFilterData->RightPanelExclude;
    CurFilterData->RightPanelExclude=Swap;
  }
}

int PanelFilter::ParseAndAddMasks(wchar_t **ExtPtr,const wchar_t *FileName,DWORD FileAttr,int& ExtCount)
{
  if (!wcscmp(FileName,L".") || TestParentFolderNameW(FileName) || (FileAttr & FA_DIREC))
    return -1;

  const wchar_t *DotPtr=wcsrchr(FileName,L'.');

  string strMask;

  /* $ 01.07.2001 IS
     Если маска содержит разделитель (',' или ';'), то возьмем ее в
     кавычки
  */
  if (DotPtr==NULL)
    strMask = L"*.";
  else

  if (wcspbrk(DotPtr,L",;"))
    strMask.Format (L"\"*%s\"",DotPtr);
  else
    strMask.Format (L"*%s",DotPtr);
  /* IS $ */

  // сначала поиск...
  unsigned int Cnt=ExtCount;
  if(lfind((const void *)(const wchar_t*)strMask,(void *)*ExtPtr,&Cnt,NM,ExtSort))
    return -1;

  // ... а потом уже выделение памяти!
  wchar_t *NewPtr;
  if ((NewPtr=(wchar_t *)xf_realloc(*ExtPtr,(NM*(ExtCount+1))*sizeof(wchar_t))) == NULL)
    return 0;
  *ExtPtr=NewPtr;

  NewPtr=*ExtPtr+ExtCount*NM;
  wcsncpy(NewPtr,strMask,NM-2);

  NewPtr=NewPtr+wcslen(NewPtr)+1;
//!!! Если нужно "решить" BugZ#413, то раскомментировать ЭТОТ кусок!!!
// Для ускорения здесь неплохо бы иметь нечто вроде ExcludeMask.CheckExt(Mask),
// т.к. нам нафиг ненать здесь знать имя файла!!!
/*
  if (ExcludeMaskIsOK && ExcludeMask.Compare(FileName))
    *NewPtr='-';
  else if(IncludeMaskIsOK && IncludeMask.Compare(FileName))
    *NewPtr='+';
  else
*/
//!!! Если нужно "решить" BugZ#413, то раскомментировать ЭТОТ кусок!!!
    *NewPtr=0;

  ExtCount++;

  return 1;
}

int _cdecl ExtSort(const void *el1,const void *el2)
{
  return LocalStricmpW((const wchar_t *)el1,(const wchar_t *)el2);
}
