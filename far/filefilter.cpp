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

void FileFilter::FilterEdit()
{
  struct MenuItem ListItem;
  int ExitCode;
  bool bNeedUpdate=false;
  VMenu FilterList("",NULL,0,ScrY-6);

  {
    DWORD Inc,Exc;
    GetIncludeExcludeFlags(Inc,Exc);
    if (FolderFlags.Check(Inc))
      FilterList.SetTitle(MSG(MFilterTitle1));
    else
      FilterList.SetTitle(MSG(MFilterTitle2));
  }

  FilterList.SetHelp("Filter");
  FilterList.SetPosition(-1,-1,0,0);
  FilterList.SetBottomTitle(MSG(MFilterBottom));
  FilterList.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);

  for (int i=0; i<FilterData.getCount(); i++)
  {
    memset(&ListItem,0,sizeof(ListItem));
    MenuString(ListItem.Name,FilterData.getItem(i));

    if(i == 0)
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

  for (int i=0; i<TempFilterData.getCount(); i++)
  {
    const char *FMask;
    TempFilterData.getItem(i)->GetMask(&FMask);
    char Mask[FILEFILTER_MASK_SIZE];
    xstrncpy(Mask,FMask,sizeof(Mask)-1);
    Unquote(Mask);
    if(!ParseAndAddMasks(&ExtPtr,Mask,0,ExtCount,GetCheck(TempFilterData.getItem(i))))
      break;
  }

  memset(&ListItem,0,sizeof(ListItem));
  ListItem.Flags|=LIF_SEPARATOR;
  FilterList.AddItem(&ListItem);

  memset(&ListItem,0,sizeof(ListItem));
  FoldersFilter.SetTitle(MSG(MFolderFileType));
  MenuString(ListItem.Name,&FoldersFilter);
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
    while (ScTree.GetNextName(&fdata,FileName, sizeof (FileName)-1))
      if(!ParseAndAddMasks(&ExtPtr,fdata.cFileName,fdata.dwFileAttributes,ExtCount,0))
        break;
  }
  else
  {
    char FileName[NM];
    int FileAttr;
    for (int i=0; m_HostPanel->GetFileName(FileName,i,FileAttr); i++)
      if(!ParseAndAddMasks(&ExtPtr,FileName,FileAttr,ExtCount,0))
        break;
  }

  far_qsort((void *)ExtPtr,ExtCount,NM,ExtSort);

  memset(&ListItem,0,sizeof(ListItem));

  for (int i=0; i<ExtCount; i++)
  {
    char *CurExtPtr=ExtPtr+i*NM;
    MenuString(ListItem.Name,NULL,false,true,CurExtPtr,MSG(MPanelFileType));
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
      //case KEY_CTRLF:
      case KEY_BS:
      {
        //if (Key!=KEY_CTRLF && !FilterList.GetSelectPos() && !FilterData.getCount())
          //break;

        int SelPos=FilterList.GetSelectPos();

        //if (Key==KEY_CTRLF) //Работает как Space но для Folders
          //SelPos=(FilterData.getCount() + 2);

        if (SelPos==FilterData.getCount())
          break;

        int Check=FilterList.GetSelection(SelPos),NewCheck;
        if (Key=='-')
          NewCheck=(Check=='-') ? 0:'-';
        else if (Key=='+')
          NewCheck=(Check=='+') ? 0:'+';
        else if (Key==KEY_BS)
          NewCheck=0;
        else
          NewCheck=Check ? 0:'+';

        FilterList.SetSelection(NewCheck,SelPos);
        FilterList.SetSelectPos(SelPos,1);
        FilterList.SetUpdateRequired(TRUE);
        FilterList.FastShow();
        if (Key!=KEY_CTRLF)
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
          FolderFlags.Swap(Inc);
        else
          FolderFlags.Set(Inc);
        if (FolderFlags.Check(Inc))
          FilterList.SetTitle(MSG(MFilterTitle1));
        else
          FilterList.SetTitle(MSG(MFilterTitle2));
        FilterList.SetUpdateRequired(TRUE);
        FilterList.SetPosition(-1,-1,0,0);
        FilterList.Show();
        bNeedUpdate=true;
        break;
      }

      case KEY_F4:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterData.getCount())
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
        else if (SelPos>FilterData.getCount())
        {
          Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
        }
        break;
      }

      case KEY_INS:
      case KEY_F5:
      {
        int SelPos=FilterList.GetSelectPos();
        int SelPos2=SelPos+1;

        if (SelPos>FilterData.getCount())
          SelPos=FilterData.getCount();

        FileFilterParams *NewFilter = FilterData.insertItem(SelPos);
        if (!NewFilter)
          break;

        if (Key==KEY_F5)
        {
          if (SelPos2 < FilterData.getCount())
          {
            *NewFilter = *FilterData.getItem(SelPos2);

            NewFilter->SetTitle("");
            NewFilter->Flags.ClearAll();
          }
          else if (SelPos2 == (FilterData.getCount()+2))
          {
            *NewFilter = FoldersFilter;

            NewFilter->SetTitle("");
            NewFilter->Flags.ClearAll();
          }
          else if (SelPos2 > (FilterData.getCount()+2))
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

      case KEY_DEL:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterData.getCount())
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
        else if (SelPos>FilterData.getCount())
        {
          Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
        }
        break;
      }

      case KEY_CTRLUP:
      case KEY_CTRLDOWN:
      {
        int SelPos=FilterList.GetSelectPos();
        if (SelPos<FilterData.getCount() && !(Key==KEY_CTRLUP && SelPos==0) && !(Key==KEY_CTRLDOWN && SelPos==FilterData.getCount()-1))
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
      }
    }
  }

  ExitCode=FilterList.Modal::GetExitCode();

  if (ExitCode!=-1)
    ProcessSelection(&FilterList);

  if (ExitCode!=-1 || bNeedUpdate)
  {
    SaveFilters(false);
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
  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  FileFilterParams *CurFilterData;
  for (int i=0,j=0; i < FilterList->GetItemCount(); i++)
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
      char Mask[NM];
      FilterList->GetUserData(Mask,sizeof(Mask),i);

      if (j < TempFilterData.getCount())
      {
         CurFilterData = TempFilterData.getItem(j);

         const char *FMask;
         CurFilterData->GetMask(&FMask);
         if (!strcmp(Mask,FMask))
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

  if (FolderFlags.Check(Inc) && (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    return true;

  bool flag=false;
  FileFilterParams *CurFilterData;

  for (int i=0; i<FilterData.getCount(); i++)
  {
    CurFilterData = FilterData.getItem(i);

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

  for (int i=0; i<TempFilterData.getCount(); i++)
  {
    CurFilterData = TempFilterData.getItem(i);

    if (CurFilterData->Flags.Check(Inc|Exc))
    {
      flag = flag || CurFilterData->Flags.Check(Inc);
      if (CurFilterData->FileInFilter(fd))
        return CurFilterData->Flags.Check(Inc)?true:false;
    }
  }

  return !flag;
}

bool FileFilter::IsEnabledOnPanel()
{
  if (m_FilterType != FFT_PANEL)
    return false;

  DWORD Inc,Exc;
  GetIncludeExcludeFlags(Inc,Exc);

  for (int i=0; i<FilterData.getCount(); i++)
  {
    if (FilterData.getItem(i)->Flags.Check(Inc|Exc))
      return true;
  }

  if (FoldersFilter.Flags.Check(Inc|Exc))
    return true;

  for (int i=0; i<TempFilterData.getCount(); i++)
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
      NewFilter->SetMask((DWORD)GetRegKey(RegKey,"UseMask",1),
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

      NewFilter->SetAttr((DWORD)GetRegKey(RegKey,"UseAttr",1),
                         (DWORD)GetRegKey(RegKey,"AttrSet",0),
                         (DWORD)GetRegKey(RegKey,"AttrClear",FILE_ATTRIBUTE_DIRECTORY));

      NewFilter->Flags.Set((DWORD)GetRegKey(RegKey,"Flags",0));
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

      NewFilter->Flags.Set((DWORD)GetRegKey(RegKey,"Flags",0));
    }
    else
      break;
  }

  FoldersFilter.SetMask(0,"");
  FoldersFilter.SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);
  FoldersFilter.Flags.Set((DWORD)GetRegKey("Filters","FoldersFilterFlags",0));

  FolderFlags.Set((DWORD)GetRegKey("Filters","FolderFlags",FFF_RPANELINCLUDE|FFF_LPANELINCLUDE|FFF_FINDFILEINCLUDE|FFF_COPYINCLUDE));
}


void FileFilter::CloseFilter()
{
  FilterData.Free();
  TempFilterData.Free();
}

void FileFilter::SaveFilters(bool SaveAll)
{
  char RegKey[80], *PtrRegKey;

  DeleteKeyTree("Filters");

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  FileFilterParams *CurFilterData;

  for (int i=0; i<FilterData.getCount(); i++)
  {
    CurFilterData = FilterData.getItem(i);
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

    SetRegKey(RegKey,"Flags",SaveAll ? CurFilterData->Flags.Flags : 0);
  }

  if (SaveAll)
  {
    strcpy(RegKey,"Filters\\PanelMask");
    PtrRegKey=RegKey+strlen(RegKey);

    for (int i=0; i<TempFilterData.getCount(); i++)
    {
      CurFilterData = TempFilterData.getItem(i);
      itoa(i,PtrRegKey,10);

      const char *Mask;
      CurFilterData->GetMask(&Mask);
      SetRegKey(RegKey,"Mask",Mask);

      SetRegKey(RegKey,"Flags",CurFilterData->Flags.Flags);
    }

    SetRegKey("Filters","FoldersFilterFlags",FoldersFilter.Flags.Flags);

    SetRegKey("Filters","FolderFlags",FolderFlags.Flags);
  }
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
  for (int i=0; i<FilterData.getCount(); i++)
    SwapPanelFlags(FilterData.getItem(i));

  SwapPanelFlags(&FoldersFilter);

  for (int i=0; i<TempFilterData.getCount(); i++)
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
