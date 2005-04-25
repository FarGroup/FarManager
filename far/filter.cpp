/*
filter.cpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.30 25.04.2005 $ */

/*
Modify:
  24.04.2005 AY
    ! GCC
  11.09.2004 VVM
    + Нажатие SHIFT-<GREY MINUS> сбрасывает все пометки фильтра.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  18.09.2002 DJ
    ! Исправлено падение после закрытия диалога Find files, если
      в нем была введена маска, не заканчивающаяся на звездочку.
    ! Теперь выдается сообщение, если в диалоге редактирования
      фильтра (Ctrl-I) не введена маска.
  03.04.2002 SVS
    - BugZ#413 - Не отображается текущий фильтр  - закомментировано!!!!!!!!
    + ParseAndAddMasks() - выявлять и если надо добавлять очередную маску
      общий код вынесен в отдельную функцию с некоторой оптимизацией по
      размеру
    ! немного оптимизации кода...
  26.03.2002 DJ
    ! ScanTree::GetNextName() принимает размер буфера для имени файла
  16.03.2002 IS
    ! В Фильтрах тоже можно использовать маски-иключения.
  13.02.2002 SVS
    ! Уборка варнингов
  25.12.2001 SVS
    ! немного оптимизации (если VC сам умеет это делать, то
      борманду нужно помочь)
  11.10.2001 VVM
    ! Если пользовательских фильтров нет, то надпись во всю ширину меню
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  18.07.2001 OT
    ! VFMenu
  02.07.2001 IS
    ! IncludeMask(ExcludeMask).Set(NULL) -> IncludeMask(ExcludeMask).Free()
  01.07.2001 IS
    + Применяем специальный класс для работы с масками файлов (CFileMask)
    ! Внедрение const
  25.06.2001 IS
    ! Внедрение const
  16.06.2001 KM
    ! Добавление WRAPMODE в меню.
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  13.02.2001 SVS
    - Ошибка, синтаксическая :-( "ListItem.Flags==0;"
  12.02.2001 SVS
    - Баги после #440-го
  11.02.2001 SVS
    ! Введение DIF_VAREDIT позволило расширить размер под маски
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  13.10.2000 tran
    - при изменении custum фильтра и при отказе от меню панель не менялась
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
  char Title[128];
  char *Masks;
  int LeftPanelInclude;
  int LeftPanelExclude;
  int RightPanelInclude;
  int RightPanelExclude;
};

static struct FilterDataRecord *FilterData=NULL;
static int FilterDataCount=0;

static unsigned char VerticalLine=0x0B3;

PanelFilter::PanelFilter(Panel *HostPanel)
{
  IncludeMaskIsOK=ExcludeMaskIsOK=false;
  IncludeMaskStr=ExcludeMaskStr=NULL;
  PanelFilter::HostPanel=HostPanel;
  AddMasks(NULL,0);
  struct FilterDataRecord *CurFilterData=FilterData;
  for (int I=0; I < FilterDataCount; I++, CurFilterData++)
  {
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
  char Title[512],Masks[PANELFILTER_MASK_SIZE];
  struct MenuItem ListItem;
  int ExitCode;
  {
    int I;
    VMenu FilterList(MSG(MFilterTitle),NULL,0,ScrY-6);

    FilterList.SetHelp("Filter");
    FilterList.SetPosition(-1,-1,0,0);
    FilterList.SetBottomTitle(MSG(MFilterBottom));
    /* $ 16.06.2001 KM
       ! Добавление WRAPMODE в меню.
    */
    FilterList.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
    /* KM $ */

    if (Pos>FilterDataCount)
      Pos=0;

    for (I=0;I<FilterDataCount;I++)
    {
      memset(&ListItem,0,sizeof(ListItem));
      sprintf(ListItem.Name,"%-30.30s %c %-30.30s",FilterData[I].Title,VerticalLine,FilterData[I].Masks);

      if(I == Pos)
        ListItem.Flags|=LIF_SELECTED;

      if (HostPanel==CtrlObject->Cp()->LeftPanel)
      {
        if (FilterData[I].LeftPanelInclude)
          ListItem.SetCheck('+');
        else if (FilterData[I].LeftPanelExclude)
          ListItem.SetCheck('-');
      }
      else
      {
        if (FilterData[I].RightPanelInclude)
          ListItem.SetCheck('+');
        else if (FilterData[I].RightPanelExclude)
          ListItem.SetCheck('-');
      }

      FilterList.SetUserData(FilterData[I].Masks,0,
            FilterList.AddItem(&ListItem));
    }

    memset(&ListItem,0,sizeof(ListItem));
    if (FilterDataCount==0)
    {
      /* $ 11.10.2001 VVM
        ! Если пользовательских фильтров нет, то надпись во всю ширину меню */
      sprintf(ListItem.Name,"%-60.60s",MSG(MNoCustomFilters));
      /* VVM $ */
      if(!Pos)
        ListItem.Flags|=LIF_SELECTED;
      FilterList.AddItem(&ListItem);
    }

    char FileName[NM],*ExtPtr=NULL;
    WIN32_FIND_DATA fdata;
    int FileAttr,ExtCount=0;

    if (HostPanel->GetMode()==NORMAL_PANEL)
    {
      char CurDir[NM];
      HostPanel->GetCurDir(CurDir);

      ScanTree ScTree(FALSE,FALSE);
      ScTree.SetFindPath(CurDir,"*.*");
      while (ScTree.GetNextName(&fdata,FileName, sizeof (FileName)-1))
        if(!ParseAndAddMasks(&ExtPtr,fdata.cFileName,fdata.dwFileAttributes,ExtCount))
          break;
    }
    else
    {
      for (I=0;HostPanel->GetFileName(FileName,I,FileAttr);I++)
        if(!ParseAndAddMasks(&ExtPtr,FileName,FileAttr,ExtCount))
          break;
    }

    far_qsort((void *)ExtPtr,ExtCount,NM,ExtSort);

    memset(&ListItem,0,sizeof(ListItem));
    if (ExtCount>0)
    {
      ListItem.Flags|=LIF_SEPARATOR;
      FilterList.AddItem(&ListItem);
    }

    ListItem.Flags&=~LIF_SEPARATOR;

    for (I=0;I<ExtCount;I++)
    {
      char *CurExtPtr=ExtPtr+I*NM;
      sprintf(ListItem.Name,"%-30.30s %c %-30.30s",MSG(MPanelFileType),VerticalLine,CurExtPtr);
      ListItem.SetCheck(CurExtPtr[strlen(CurExtPtr)+1]);
      ListItem.Flags&=~LIF_SELECTED;
      FilterList.SetUserData(CurExtPtr,0,FilterList.AddItem(&ListItem));
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
            strcpy(Title,FilterData[SelPos].Title);
            strcpy(Masks,FilterData[SelPos].Masks);

            if (EditRecord(Title,Masks))
            {
              char *Ptr;
              if((Ptr=(char *)xf_realloc(FilterData[SelPos].Masks,strlen(Masks)+1)) != NULL)
              {
                strcpy(FilterData[SelPos].Title,Title);
                FilterData[SelPos].Masks=Ptr;
                strcpy(FilterData[SelPos].Masks,Masks);
                SaveFilters();
                *NeedUpdate=1;
                return(SelPos);
              }
              break;
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
          SelPos=FilterList.GetSelectPos();
          char *Ptr;

          if (SelPos>FilterDataCount)
            SelPos=FilterDataCount;

          *Title=*Masks=0;
          if (EditRecord(Title,Masks))
          {
            struct FilterDataRecord *NewFilterData;

            if((Ptr=(char *)xf_malloc(strlen(Masks)+1)) == NULL)
              break;

            if ((NewFilterData=(struct FilterDataRecord *)xf_realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
            {
              xf_free(Ptr);
              break;
            }

            FilterData=NewFilterData;

            for (int I=FilterDataCount-1;I>=SelPos;I--)
              FilterData[I+1]=FilterData[I];

            FilterDataCount++;
            memset(FilterData+SelPos,0,sizeof(struct FilterDataRecord));
            xstrncpy(FilterData[SelPos].Title,Title,sizeof(FilterData[0].Title)-1);
            FilterData[SelPos].Masks=Ptr;
            strcpy(FilterData[SelPos].Masks,Masks);
            FilterData[SelPos].LeftPanelInclude=0;
            FilterData[SelPos].LeftPanelExclude=0;
            FilterData[SelPos].RightPanelInclude=0;
            FilterData[SelPos].RightPanelExclude=0;

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
            char QuotedTitle[512];
            sprintf(QuotedTitle,"\"%.*s\"",sizeof(QuotedTitle)-1,FilterData[SelPos].Title);
            if (Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
                        QuotedTitle,MSG(MDelete),MSG(MCancel))==0)
            {
              if(FilterData[SelPos].Masks)
                xf_free(FilterData[SelPos].Masks);

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
            Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
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
  char Masks[PANELFILTER_MASK_SIZE];

  AddMasks(NULL,0);
  struct FilterDataRecord *CurFilterData=FilterData;
  for (int I=0; I < FilterList->GetItemCount(); I++, CurFilterData++)
  {
    int Check=FilterList->GetSelection(I);

    if (Check && FilterList->GetUserData(Masks,sizeof(Masks),I))
      AddMasks(Masks,Check=='-');

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
void PanelFilter::AddMasks(const char *Masks,int Exclude)
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

  int AddSize=strlen(Masks)+4, OldSize;

  if(Exclude)
  {
    ExcludeMask.Free();

    OldSize=ExcludeMaskStr?strlen(ExcludeMaskStr):0;
    char *NewExcludeMaskStr=(char *)xf_realloc(ExcludeMaskStr,AddSize+OldSize);
    if (NewExcludeMaskStr==NULL)
       return;

    ExcludeMaskStr=NewExcludeMaskStr;
    if(OldSize)
    {
      if(ExcludeMaskStr[OldSize-1]!=',')
      {
        ExcludeMaskStr[OldSize]=',';
        ExcludeMaskStr[OldSize+1]=0;
      }
    }
    else
      *ExcludeMaskStr=0;

    strcat(ExcludeMaskStr, Masks);
    ExcludeMaskIsOK=ExcludeMask.Set(ExcludeMaskStr, FMF_SILENT);
  }
  else
  {
    IncludeMask.Free();

    OldSize=IncludeMaskStr?strlen(IncludeMaskStr):0;
    char *NewIncludeMaskStr=(char *)xf_realloc(IncludeMaskStr,AddSize+OldSize);

    if (NewIncludeMaskStr==NULL)
       return;

    IncludeMaskStr=NewIncludeMaskStr;
    if(OldSize)
    {
      if (IncludeMaskStr[OldSize-1]!=',')
      {
        IncludeMaskStr[OldSize]=',';
        IncludeMaskStr[OldSize+1]=0;
      }
    }
    else
      *IncludeMaskStr=0;

    strcat(IncludeMaskStr, Masks);
    IncludeMaskIsOK=IncludeMask.Set(IncludeMaskStr, FMF_SILENT);
  }
}
/* IS $ */

/* $ 01.07.2001 IS
   Используем нормальные классы для работы с масками файлов
*/
int PanelFilter::CheckName(const char *Name)
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
  char RegKey[80], *PtrRegKey;

  FilterData=NULL;
  FilterDataCount=0;

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  while (1)
  {
    char FilterTitle[200],FilterMask[PANELFILTER_MASK_SIZE], *Ptr;

    itoa(FilterDataCount,PtrRegKey,10);
    GetRegKey(RegKey,"Title",FilterTitle,"",sizeof(FilterTitle));

    if (!GetRegKey(RegKey,"Mask",FilterMask,"",sizeof(FilterMask)))
      break;

    if((Ptr=(char *)xf_malloc(strlen(FilterMask)+1)) == NULL)
      break;

    struct FilterDataRecord *NewFilterData;
    if ((NewFilterData=(struct FilterDataRecord *)xf_realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
    {
      xf_free(Ptr);
      break;
    }
    FilterData=NewFilterData;

    strcpy(FilterData[FilterDataCount].Title,FilterTitle);
    FilterData[FilterDataCount].Masks=Ptr;
    strcpy(FilterData[FilterDataCount].Masks,FilterMask);
    GetRegKey(RegKey,"LeftInclude",FilterData[FilterDataCount].LeftPanelInclude,0);
    GetRegKey(RegKey,"LeftExclude",FilterData[FilterDataCount].LeftPanelExclude,0);
    GetRegKey(RegKey,"RightInclude",FilterData[FilterDataCount].RightPanelInclude,0);
    GetRegKey(RegKey,"RightExclude",FilterData[FilterDataCount].RightPanelExclude,0);
    FilterDataCount++;
  }
}


void PanelFilter::CloseFilter()
{
  if(FilterData)
  {
    for(int I=0; I < FilterDataCount; ++I)
      if(FilterData[I].Masks)
        xf_free(FilterData[I].Masks);

    xf_free(FilterData);
  }
}


void PanelFilter::SaveSelection()
{
  char RegKey[80], *PtrRegKey;

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  for (int I=0;I<FilterDataCount;I++)
  {
    itoa(I,PtrRegKey,10);
    SetRegKey(RegKey,"LeftInclude",FilterData[I].LeftPanelInclude);
    SetRegKey(RegKey,"LeftExclude",FilterData[I].LeftPanelExclude);
    SetRegKey(RegKey,"RightInclude",FilterData[I].RightPanelInclude);
    SetRegKey(RegKey,"RightExclude",FilterData[I].RightPanelExclude);
  }
}


void PanelFilter::SaveFilters()
{
  struct FilterDataRecord *CurFilterData=FilterData;
  char RegKey[80], *PtrRegKey;

  strcpy(RegKey,"Filters\\Filter");
  PtrRegKey=RegKey+strlen(RegKey);

  for (int I=0; I < FilterDataCount+5; I++, CurFilterData++)
  {
    itoa(I,PtrRegKey,10);

    if (I>=FilterDataCount)
      DeleteRegKey(RegKey);
    else
    {
      SetRegKey(RegKey,"Title",CurFilterData->Title);
      SetRegKey(RegKey,"Mask",CurFilterData->Masks);
    }
  }
}


int PanelFilter::EditRecord(char *Title,char *Masks)
{
  const char *HistoryName="Masks";
  static struct DialogData EditDlgData[]=
  {
    /* 0 */ DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MFilterTitle,
    /* 1 */ DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEnterFilterTitle,
    /* 2 */ DI_EDIT,5,3,70,3,1,0,0,0,"",
    /* 3 */ DI_TEXT,5,4,0,0,0,0,0,0,(char *)MFilterMasks,
    /* 4 */ DI_EDIT,5,5,70,5,0,(DWORD)HistoryName,DIF_VAREDIT|DIF_HISTORY,0,"",
    /* 5 */ DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /* 6 */ DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    /* 7 */ DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
   };

  MakeDialogItems(EditDlgData,EditDlg);

  strcpy(EditDlg[2].Data,Title);
  EditDlg[4].Ptr.PtrData=Masks;
  EditDlg[4].Ptr.PtrLength=PANELFILTER_MASK_SIZE;
  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp("Filter");
    Dlg.SetPosition(-1,-1,76,10);
    /* $ 01.07.2001 IS
       теперь введенные маски проверяются на корректность
    */
    CFileMask CheckMask;
    for(;;)
    {
      Dlg.ClearDone();
      Dlg.Process();
      /* $ 18.09.2002 DJ
         сообщение, если не введена маска
      */
      const char *FilterMask = static_cast<const char *> (EditDlg[4].Ptr.PtrData);
      if (Dlg.GetExitCode()!=6)
        return(FALSE);
      if (FilterMask[0]==0)
      {
        Message (MSG_DOWN|MSG_WARNING,1,MSG(MWarning),MSG(MAssocNeedMask), MSG(MOk));
        continue;
      }

      /* $ 16.03.2002 IS
         В Фильтрах тоже можно использовать маски-иключения.
      */
      if(CheckMask.Set(FilterMask, 0))
      /* IS $ */
        break;
      /* DJ $ */
    }
    /* IS $ */
  }
  strcpy(Title,EditDlg[2].Data);
  return(TRUE);
}


void PanelFilter::SwapFilter()
{
  struct FilterDataRecord *CurFilterData=FilterData;
  for (int I=0; I < FilterDataCount; I++, CurFilterData++)
  {
    int Swap=CurFilterData->LeftPanelInclude;
    CurFilterData->LeftPanelInclude=CurFilterData->RightPanelInclude;
    CurFilterData->RightPanelInclude=Swap;

    Swap=CurFilterData->LeftPanelExclude;
    CurFilterData->LeftPanelExclude=CurFilterData->RightPanelExclude;
    CurFilterData->RightPanelExclude=Swap;
  }
}

int PanelFilter::ParseAndAddMasks(char **ExtPtr,const char *FileName,DWORD FileAttr,int& ExtCount)
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
  size_t Cnt=ExtCount;
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
  return LocalStricmp((char *)el1,(char *)el2);
}
