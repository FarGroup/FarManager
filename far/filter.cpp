/*
filter.cpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


static int _cdecl ExtSort(const void *el1,const void *el2);

static struct FilterDataRecord *FilterData;
static int FilterDataCount;

PanelFilter::PanelFilter(Panel *HostPanel)
{
  PanelFilter::HostPanel=HostPanel;
  FilterMask=NULL;
  FilterMaskCount=0;
  ExcludeFilterMask=NULL;
  ExcludeFilterMaskCount=0;
  AddMasks(NULL,0);
  for (int I=0;I<FilterDataCount;I++)
    if (HostPanel==CtrlObject->LeftPanel)
    {
      if (FilterData[I].LeftPanelInclude)
        AddMasks(FilterData[I].Masks,FALSE);
      else
        if (FilterData[I].LeftPanelExclude)
          AddMasks(FilterData[I].Masks,TRUE);
    }
    else
      if (FilterData[I].RightPanelInclude)
        AddMasks(FilterData[I].Masks,FALSE);
      else
        if (FilterData[I].RightPanelExclude)
          AddMasks(FilterData[I].Masks,TRUE);
}


PanelFilter::~PanelFilter()
{
  delete FilterMask;
  delete ExcludeFilterMask;
}


void PanelFilter::FilterEdit()
{
  int FirstCall=TRUE,Pos=0;
  while (Pos!=-1)
  {
    Pos=ShowFilterMenu(Pos,FirstCall);
    FirstCall=FALSE;
  }
}


int PanelFilter::ShowFilterMenu(int Pos,int FirstCall)
{
  int ExitCode;
  {
    int I;
    VMenu FilterList(MSG(MFilterTitle),NULL,0,ScrY-6);
    struct MenuItem ListItem;
    ListItem.Checked=ListItem.Separator=*ListItem.UserData=ListItem.UserDataSize=0;

    FilterList.SetHelp("Filter");
    FilterList.SetPosition(-1,-1,0,0);
    FilterList.SetBottomTitle(MSG(MFilterBottom));
    FilterList.SetFlags(MENU_SHOWAMPERSAND);

    if (Pos>FilterDataCount)
      Pos=0;

    for (I=0;I<FilterDataCount;I++)
    {
      sprintf(ListItem.Name,"%-30.30s │ %-30.30s",&FilterData[I].Title,&FilterData[I].Masks);
      ListItem.Selected=(I == Pos);
      strcpy(ListItem.UserData,FilterData[I].Masks);
      ListItem.UserDataSize=strlen(FilterData[I].Masks)+1;
      ListItem.Checked=0;
      if (HostPanel==CtrlObject->LeftPanel)
      {
        if (FilterData[I].LeftPanelInclude)
          ListItem.Checked='+';
        else
          if (FilterData[I].LeftPanelExclude)
            ListItem.Checked='-';
      }
      else
        if (FilterData[I].RightPanelInclude)
          ListItem.Checked='+';
        else
          if (FilterData[I].RightPanelExclude)
            ListItem.Checked='-';
      FilterList.AddItem(&ListItem);
    }

    memset(&ListItem,0,sizeof(ListItem));
    if (FilterDataCount==0)
    {
      sprintf(ListItem.Name,"%-30.30s │",MSG(MNoCustomFilters));
      ListItem.Selected=(Pos==0);
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

      while (ScTree.GetNextName(&fdata,FileName))
      {
        if (strcmp(fdata.cFileName,".")==0 || (fdata.cFileName,"..")==0 ||
            (fdata.dwFileAttributes & FA_DIREC))
          continue;
        char *NewPtr=(char *)realloc(ExtPtr,NM*(ExtCount+1));
        if (NewPtr==NULL)
          break;
        ExtPtr=NewPtr;
        char *DotPtr=strrchr(fdata.cFileName,'.');
        char Mask[NM];
        if (DotPtr==NULL)
          strcpy(Mask,"*.");
        else
          sprintf(Mask,"*%s",DotPtr);
        strcpy(ExtPtr+ExtCount*NM,Mask);
        ExtCount++;
      }
    }
    else
      for (I=0;HostPanel->GetFileName(FileName,I,FileAttr);I++)
      {
        if (strcmp(FileName,".")==0 || (FileName,"..")==0 ||
            (FileAttr & FA_DIREC))
          continue;
        char *NewPtr=(char *)realloc(ExtPtr,NM*(ExtCount+1));
        if (NewPtr==NULL)
          break;
        ExtPtr=NewPtr;
        char *DotPtr=strrchr(FileName,'.');
        char Mask[NM];
        if (DotPtr==NULL)
          strcpy(Mask,"*.");
        else
          sprintf(Mask,"*%s",DotPtr);
        strcpy(ExtPtr+ExtCount*NM,Mask);
        ExtCount++;
      }


    qsort((void *)ExtPtr,ExtCount,NM,ExtSort);

    if (ExtCount>0)
    {
      memset(&ListItem,0,sizeof(ListItem));
      ListItem.Separator=1;
      FilterList.AddItem(&ListItem);
    }

    ListItem.Separator=0;
    for (I=0;I<ExtCount;I++)
    {
      char *CurExtPtr=ExtPtr+I*NM;
      if (I>0 && LocalStricmp(CurExtPtr-NM,CurExtPtr)==0)
        continue;
      sprintf(ListItem.Name,"%-30.30s │ %-30.30s",MSG(MPanelFileType),CurExtPtr);
      ListItem.Selected=0;
      strcpy(ListItem.UserData,CurExtPtr);
      ListItem.UserDataSize=strlen(CurExtPtr)+1;
      FilterList.AddItem(&ListItem);
    }
    delete ExtPtr;

    FilterList.Show();

    if (!FirstCall)
      ProcessSelection(FilterList);

    while (!FilterList.Done())
    {
      int Key=FilterList.ReadInput();
      if (Key==KEY_ADD)
        Key='+';
      else
        if (Key==KEY_SUBTRACT)
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
            else
              if (Key=='+')
                NewCheck=(Check=='+') ? 0:'+';
              else
                NewCheck=Check ? 0:'+';
            FilterList.SetSelection(NewCheck);
            FilterList.SetUpdateRequired(TRUE);
            FilterList.FastShow();
            FilterList.ProcessKey(KEY_DOWN);
          }
          break;
        case KEY_F4:
          {
            int SelPos=FilterList.GetSelectPos();
            if (SelPos<FilterDataCount)
            {
              char Title[512],Masks[512];
              strcpy(Title,FilterData[SelPos].Title);
              strcpy(Masks,FilterData[SelPos].Masks);
              if (EditRecord(Title,Masks))
              {
                strcpy(FilterData[SelPos].Title,Title);
                strcpy(FilterData[SelPos].Masks,Masks);
                SaveFilters();
                return(SelPos);
              }
            }
            else
            {
              Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
              continue;
            }
          }
          break;
        case KEY_INS:
          {
            int SelPos=FilterList.GetSelectPos();
            if (SelPos>FilterDataCount)
              SelPos=FilterDataCount;
            char Title[512],Masks[512];
            *Title=*Masks=0;
            if (EditRecord(Title,Masks))
            {
              struct FilterDataRecord *NewFilterData;
              if ((NewFilterData=(struct FilterDataRecord *)realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
                break;
              FilterData=NewFilterData;
              for (int I=FilterDataCount-1;I>=SelPos;I--)
                FilterData[I+1]=FilterData[I];
              FilterDataCount++;
              strncpy(FilterData[SelPos].Title,Title,sizeof(FilterData[0].Title));
              strncpy(FilterData[SelPos].Masks,Masks,sizeof(FilterData[0].Masks));
              FilterData[SelPos].LeftPanelInclude=0;
              FilterData[SelPos].LeftPanelExclude=0;
              FilterData[SelPos].RightPanelInclude=0;
              FilterData[SelPos].RightPanelExclude=0;
              SaveFilters();
              SaveSelection();
              return(SelPos);
            }
          }
          break;
        case KEY_DEL:
          {
            int SelPos=FilterList.GetSelectPos();
            if (SelPos<FilterDataCount)
            {
              char QuotedTitle[512];
              sprintf(QuotedTitle,"\"%.*s\"",sizeof(QuotedTitle)-1,FilterData[SelPos].Title);
              if (Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
                          QuotedTitle,MSG(MDelete),MSG(MCancel))==0)
              {
                for (int I=SelPos+1;I<FilterDataCount;I++)
                  FilterData[I-1]=FilterData[I];
                FilterDataCount--;
                SaveFilters();
                SaveSelection();
                if (SelPos>=FilterDataCount && SelPos>0)
                  SelPos--;
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
          FilterList.ProcessInput();
          break;
      }
    }
    ExitCode=FilterList.GetExitCode();
    if (ExitCode!=-1)
      ProcessSelection(FilterList);
  }
  if (ExitCode!=-1)
  {
    HostPanel->Update(UPDATE_KEEP_SELECTION);
    HostPanel->Redraw();
  }
  return(-1);
}


void PanelFilter::ProcessSelection(VMenu &FilterList)
{
  AddMasks(NULL,0);
  for (int I=0;I<FilterList.GetItemCount();I++)
  {
    char Masks[NM];
    int Check=FilterList.GetSelection(I);
    if (Check && FilterList.GetUserData(Masks,sizeof(Masks),I))
      AddMasks(Masks,Check=='-');
    if (I<FilterDataCount)
    {
      if (HostPanel==CtrlObject->LeftPanel)
        FilterData[I].LeftPanelInclude=FilterData[I].LeftPanelExclude=0;
      if (HostPanel==CtrlObject->RightPanel)
        FilterData[I].RightPanelInclude=FilterData[I].RightPanelExclude=0;
      if (Check=='+')
        if (HostPanel==CtrlObject->LeftPanel)
          FilterData[I].LeftPanelInclude=TRUE;
        else
          FilterData[I].RightPanelInclude=TRUE;
      if (Check=='-')
        if (HostPanel==CtrlObject->LeftPanel)
          FilterData[I].LeftPanelExclude=TRUE;
        else
          FilterData[I].RightPanelExclude=TRUE;
    }
  }
}


void PanelFilter::AddMasks(char *Masks,int Exclude)
{
  if (Masks==NULL)
  {
    delete FilterMask;
    FilterMask=NULL;
    FilterMaskCount=0;
    delete ExcludeFilterMask;
    ExcludeFilterMask=NULL;
    ExcludeFilterMaskCount=0;
    return;
  }
  char ArgName[NM],*NamePtr=Masks;
  while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
  {
    RemoveTrailingSpaces(ArgName);
    if (Exclude)
    {
      char *NewFilterMask=(char *)realloc(ExcludeFilterMask,(ExcludeFilterMaskCount+1)*NM);
      if (NewFilterMask==NULL)
        break;
      ExcludeFilterMask=NewFilterMask;
      strcpy(ExcludeFilterMask+ExcludeFilterMaskCount*NM,ArgName);
      ExcludeFilterMaskCount++;
    }
    else
    {
      char *NewFilterMask=(char *)realloc(FilterMask,(FilterMaskCount+1)*NM);
      if (NewFilterMask==NULL)
        break;
      FilterMask=NewFilterMask;
      strcpy(FilterMask+FilterMaskCount*NM,ArgName);
      FilterMaskCount++;
    }
  }
}


int PanelFilter::CheckName(char *Name)
{
  if (ExcludeFilterMaskCount>0)
    for (int I=0;I<ExcludeFilterMaskCount;I++)
      if (CmpName(ExcludeFilterMask+NM*I,Name))
        return(FALSE);
  if (FilterMaskCount==0)
    return(TRUE);
  for (int I=0;I<FilterMaskCount;I++)
    if (CmpName(FilterMask+NM*I,Name))
      return(TRUE);
  return(FALSE);
}


bool PanelFilter::IsEnabled()
{
  return(ExcludeFilterMaskCount>0 || FilterMaskCount>0);
}


int _cdecl ExtSort(const void *el1,const void *el2)
{
  return(LocalStricmp((char *)el1,(char *)el2));
}


void PanelFilter::InitFilter()
{
  FilterData=NULL;
  FilterDataCount=0;
  while (1)
  {
    char FilterTitle[200],FilterMask[200];
    char RegKey[80];
    sprintf(RegKey,"Filters\\Filter%d",FilterDataCount);
    GetRegKey(RegKey,"Title",FilterTitle,"",sizeof(FilterTitle));
    if (!GetRegKey(RegKey,"Mask",FilterMask,"",sizeof(FilterMask)))
      break;

    struct FilterDataRecord *NewFilterData;
    if ((NewFilterData=(struct FilterDataRecord *)realloc(FilterData,sizeof(*FilterData)*(FilterDataCount+1)))==NULL)
      break;
    FilterData=NewFilterData;
    strcpy(FilterData[FilterDataCount].Title,FilterTitle);
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
  delete FilterData;
}


void PanelFilter::SaveSelection()
{
  for (int I=0;I<FilterDataCount;I++)
  {
    char RegKey[80];
    sprintf(RegKey,"Filters\\Filter%d",I);
    SetRegKey(RegKey,"LeftInclude",FilterData[I].LeftPanelInclude);
    SetRegKey(RegKey,"LeftExclude",FilterData[I].LeftPanelExclude);
    SetRegKey(RegKey,"RightInclude",FilterData[I].RightPanelInclude);
    SetRegKey(RegKey,"RightExclude",FilterData[I].RightPanelExclude);
  }
}


void PanelFilter::SaveFilters()
{
  for (int I=0;I<FilterDataCount+5;I++)
  {
    char RegKey[80];
    sprintf(RegKey,"Filters\\Filter%d",I);

    if (I>=FilterDataCount)
      DeleteRegKey(RegKey);
    else
    {
      SetRegKey(RegKey,"Title",FilterData[I].Title);
      SetRegKey(RegKey,"Mask",FilterData[I].Masks);
    }
  }
}


int PanelFilter::EditRecord(char *Title,char *Masks)
{
  const char *HistoryName="Masks";
  static struct DialogData EditDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MFilterTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEnterFilterTitle,
    DI_EDIT,5,3,70,3,1,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MFilterMasks,
    DI_EDIT,5,5,70,5,0,(DWORD)HistoryName,DIF_HISTORY,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(EditDlgData,EditDlg);

  strcpy(EditDlg[2].Data,Title);
  strcpy(EditDlg[4].Data,Masks);
  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp("Filter");
    Dlg.SetPosition(-1,-1,76,10);
    Dlg.Process();
    if (Dlg.GetExitCode()!=6 || *EditDlg[4].Data==0)
      return(FALSE);
  }
  strcpy(Title,EditDlg[2].Data);
  strcpy(Masks,EditDlg[4].Data);
  return(TRUE);
}


void PanelFilter::SwapFilter()
{
  for (int I=0;I<FilterDataCount;I++)
  {
    int Swap=FilterData[I].LeftPanelInclude;
    FilterData[I].LeftPanelInclude=FilterData[I].RightPanelInclude;
    FilterData[I].RightPanelInclude=Swap;

    Swap=FilterData[I].LeftPanelExclude;
    FilterData[I].LeftPanelExclude=FilterData[I].RightPanelExclude;
    FilterData[I].RightPanelExclude=Swap;
  }
}

