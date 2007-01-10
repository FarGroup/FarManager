/*
grpsort.cpp

Группы сортировки

*/

#include "headers.hpp"
#pragma hdrstop

#include "grpsort.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "ctrlobj.hpp"
#include "CFileMask.hpp"

/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
struct GroupSortData
{
  CFileMaskW *FMasks;
  wchar_t *OriginalMasks;
  int Group;
  int reserved; // для выравнивания на 16 :-)

  void Clear ()
  {
    OriginalMasks = NULL;
    Group = 0;
  }
};
/* IS $ */

#define GROUPSORT_MASK_SIZE      2048

static const wchar_t *fmtUpperGroup=L"UpperGroup%d";
static const wchar_t *fmtLowerGroup=L"LowerGroup%d";
static const wchar_t *SortGroupsKeyName=L"SortGroups";

GroupSort::GroupSort()
{
  int I, J;
  string strGroupName, strGroupStr;
  static const wchar_t *GroupFMT[2]={fmtUpperGroup,fmtLowerGroup};
  int  GroupDelta[2]={0,DEFAULT_SORT_GROUP+1};

  GroupData=NULL;
  GroupCount=0;
  for (J=0; J < 2; ++J)
    for (I=0;;I++)
    {
      strGroupName.Format (GroupFMT[J],I);
      GetRegKeyW(SortGroupsKeyName,strGroupName,strGroupStr,L"");

      if ( strGroupStr.IsEmpty())
        break;

      GroupSortData *pNewGroup = new GroupSortData;
      pNewGroup->Clear ();

      if(AddMask(pNewGroup,strGroupStr,I+GroupDelta[J]))
      {
        GroupSortData **NewGroupData=(struct GroupSortData **)xf_realloc(GroupData, sizeof(*GroupData)*(GroupCount+1));
        if (NewGroupData==NULL)
        {
          DeleteMask(pNewGroup);
          delete pNewGroup;
          break;
        }
        GroupData=NewGroupData;
        GroupData[GroupCount] = pNewGroup;
        GroupCount++;
      }
      else
        break;
    }
}


GroupSort::~GroupSort()
{
  if(GroupData)
  {
    for(int I=0; I < GroupCount; ++I)
    {
      DeleteMask(GroupData[I]);
      delete GroupData[I];
    }

    xf_free(GroupData);
  }
}

/* $ 01.05.2001 DJ
   оптимизированный формат хранения в Masks
*/
/* $ 06.07.2001 IS
   + вместо "рабочей" маски используем соответствующий класс
   ! не использую Add_PATHEXT
*/
BOOL GroupSort::AddMask(GroupSortData *Dest,const wchar_t *Mask,int Group)
{
  wchar_t *Ptr, *OPtr;
  /* Обработка %PATHEXT% */
  // память под оригинал - OriginalMasks
  if((OPtr=(wchar_t *)xf_realloc(Dest->OriginalMasks,(wcslen(Mask)+1)*sizeof (wchar_t))) == NULL)
    return FALSE;

  wcscpy(OPtr,Mask); // сохраняем оригинал.

  string strMask = Mask;
  // проверим

  wchar_t *MaskCopy = strMask.GetBuffer ();

  if((Ptr=wcschr(MaskCopy, L'%')) != NULL && !LocalStrnicmpW(Ptr,L"%PATHEXT%",9))
  {
    int IQ1=(*(Ptr+9) == L',')?10:9, offsetPtr=Ptr-MaskCopy;
    // Если встречается %pathext%, то допишем в конец...
    memmove(Ptr,Ptr+IQ1,(wcslen(Ptr+IQ1)+1)*sizeof (wchar_t));

    strMask.ReleaseBuffer ();

    string strTmp1;

    wchar_t *pSeparator;
    strTmp1 = strMask;

    wchar_t *Tmp1 = strTmp1.GetBuffer ();

    pSeparator = wcschr(Tmp1, EXCLUDEMASKSEPARATOR);

    if(pSeparator)
    {
      Ptr=Tmp1+offsetPtr;
      if(Ptr>pSeparator) // PATHEXT находится в масках исключения
      {
        strTmp1.ReleaseBuffer ();
        Add_PATHEXT(strMask); // добавляем то, чего нету.
      }
      else
      {
        string strTmp2;
        strTmp2 = pSeparator+1;
        *pSeparator=0;

        strTmp1.ReleaseBuffer ();

        Add_PATHEXT(strTmp1);

        strMask.Format (L"%s|%s", (const wchar_t*)strTmp1, (const wchar_t*)strTmp2);
      }
    }
    else
    {
      strTmp1.ReleaseBuffer ();
      Add_PATHEXT(strMask); // добавляем то, чего нету.
    }
  }

  strMask.ReleaseBuffer ();

  // память под рабочую маску
  if((Dest->FMasks=new CFileMaskW) == NULL)
  {
    xf_free(OPtr);
    return FALSE;
  }

  if(!Dest->FMasks->Set(strMask, FMF_SILENT)) // проверим корректность маски
  {
    delete Dest->FMasks;
    Dest->FMasks=NULL;
    xf_free(OPtr);
    return FALSE;
  }

  Dest->OriginalMasks=OPtr;
  Dest->Group=Group;
  return TRUE;
}
/* IS $ */
/* DJ $ */

/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
void GroupSort::DeleteMask(struct GroupSortData *CurGroupData)
{
  if(CurGroupData->FMasks)
  {
    delete CurGroupData->FMasks;
    CurGroupData->FMasks=NULL;
  }
  if(CurGroupData->OriginalMasks)
  {
    xf_free(CurGroupData->OriginalMasks);
    CurGroupData->OriginalMasks=NULL;
  }
}
/* IS $ */

/* $ 06.07.2001 IS "рабочей" маски теперь у нас нет */
const wchar_t *GroupSort::GetMask(int Idx)
{
  return GroupData[Idx]->OriginalMasks;
}
/* IS $ */

/* $ 01.05.2001 DJ
   оптимизированный формат хранения в Masks
*/
/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
int GroupSort::GetGroup(const wchar_t *Path)
{
  for (int I=0;I<GroupCount;I++)
  {
    GroupSortData *CurGroupData=GroupData[I];
    if(CurGroupData->FMasks->Compare(Path))
       return(CurGroupData->Group);
  }
  return(DEFAULT_SORT_GROUP);
}
/* IS $ */
/* DJ $ */

void GroupSort::EditGroups()
{
  int UpperGroup,LowerGroup,I,Pos=0,StartGroupCount=GroupCount;

  while (Pos!=-1)
    Pos=EditGroupsMenu(Pos);

  for (UpperGroup=LowerGroup=I=0;I<Max(GroupCount,StartGroupCount);I++)
  {
    string strGroupName;
    if (I<GroupCount)
    {
      if (GroupData[I]->Group<DEFAULT_SORT_GROUP)
      {
        strGroupName.Format (fmtUpperGroup,UpperGroup);
        GroupData[I]->Group=UpperGroup++;
      }
      else
      {
        strGroupName.Format (fmtLowerGroup,LowerGroup);
        GroupData[I]->Group=DEFAULT_SORT_GROUP+1+LowerGroup++;
      }
      SetRegKeyW(SortGroupsKeyName,strGroupName,GetMask(I));
    }
    else
    {
      strGroupName.Format (fmtUpperGroup,UpperGroup++);
      DeleteRegValueW(SortGroupsKeyName,strGroupName);
      strGroupName.Format (fmtLowerGroup,LowerGroup++);
      DeleteRegValueW(SortGroupsKeyName,strGroupName);
    }
  }
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->LeftPanel->Redraw();
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
  GroupSortData *pNewGroup;
  string strNewMasks;
  wchar_t *HelpSortGroups=L"SortGroups";
  struct MenuItemEx ListItem;
  struct MenuItemEx ListItem2;

  VMenu GroupList(UMSG(MSortGroupsTitle),NULL,0,TRUE,ScrY-4);
  GroupList.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
  GroupList.SetHelp(HelpSortGroups);
  GroupList.SetPosition(-1,-1,0,0);
  GroupList.SetBottomTitle(UMSG(MSortGroupsBottom));

  int GroupPos=0,I;
  for (I=0;GroupPos<GroupCount;I++)
  {
    if (GroupData[GroupPos]->Group>DEFAULT_SORT_GROUP)
      break;
    ListItem.Clear ();
    ListItem.strName = GetMask(GroupPos);
    ListItem.SetSelect(I == Pos);
    GroupList.AddItemW(&ListItem);
    GroupPos++;
  }
  int UpperGroupCount=GroupPos;

  ListItem.Clear ();
  ListItem.strName=L"";
  ListItem.SetSelect(I == Pos);
  GroupList.AddItemW(&ListItem);
  ListItem.Flags&=~LIF_SELECTED;
  ListItem.Flags|=LIF_SEPARATOR;
  GroupList.AddItemW(&ListItem);
  ListItem.Flags&=~LIF_SEPARATOR;
  I+=2;

  for (;GroupPos<GroupCount;I++)
  {
    ListItem2.Clear ();
    ListItem2.strName = GetMask(GroupPos);
    ListItem2.SetSelect(I == Pos);
    GroupList.AddItemW(&ListItem2);
    GroupPos++;
  }

  ListItem2.Clear ();
  ListItem.strName=L"";
  ListItem.SetSelect(GroupCount+2 == Pos);
  GroupList.AddItemW(&ListItem);

  GroupList.Show();

  while (1)
  {
    while (!GroupList.Done())
    {
      int SelPos=GroupList.GetSelectPos();
      int ListPos=SelPos;
      int Key=GroupList.ReadInput();
      int UpperGroup=(SelPos<UpperGroupCount+1);
      if (ListPos>=UpperGroupCount)
      {
        if (ListPos==UpperGroupCount+1 || ListPos==UpperGroupCount &&
            Key!=KEY_INS && Key!=KEY_ESC)
        {
          GroupList.ProcessInput();
          GroupList.ClearDone();
          continue;
        }
        if (ListPos>UpperGroupCount+1)
          ListPos-=2;
        else
          ListPos=UpperGroupCount;
      }
      switch(Key)
      {
        case KEY_DEL:
          if (ListPos<GroupCount)
          {
            string strGroupName;

            strGroupName = L"\"";
            strGroupName += GetMask(ListPos);
            strGroupName += L"\"";

            if (MessageW(MSG_WARNING,2,UMSG(MSortGroupsTitle),
                        UMSG(MSortGroupsAskDel),strGroupName,
                        UMSG(MDelete),UMSG(MCancel))!=0)
              break;
            DeleteMask(GroupData[ListPos]);

            delete GroupData[ListPos];

            for (int I=ListPos+1;I<GroupCount;I++)
              GroupData[I-1]=GroupData[I];
            GroupCount--;
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            /* $ 06.07.2001 IS
               проверяем маску на корректность
            */
            string strNewMasks;
            CFileMaskW FMasks;
            int ExitCode;
            for(;;)
            {
               ExitCode=GetStringW(UMSG(MSortGroupsTitle),UMSG(MSortGroupsEnter),
                         L"Masks",L"",strNewMasks,NM,HelpSortGroups,FIB_BUTTONS);
               if(!ExitCode)
                  break;
               if(FMasks.Set(strNewMasks, 0))
                  break;
            }
            if (ExitCode)
            /* IS $ */
            {
              pNewGroup = new GroupSortData;
              pNewGroup->Clear ();
              if(AddMask(pNewGroup,strNewMasks,UpperGroup ? 0:DEFAULT_SORT_GROUP+1))
              {
                GroupSortData **NewGroupData=(struct GroupSortData **)xf_realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
                if (NewGroupData==NULL)
                {
                  DeleteMask(pNewGroup);
                  delete pNewGroup;
                  break;
                }
                GroupData=NewGroupData;
                GroupCount++;
                for (int I=GroupCount-1;I>ListPos;I--)
                  GroupData[I]=GroupData[I-1];
                GroupData[ListPos] = pNewGroup;
                return(SelPos);
              }
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupCount)
          {
            pNewGroup = GroupData[ListPos];
            strNewMasks = GetMask(ListPos);
            /* $ 06.07.2001 IS
               проверяем маску на корректность
            */
            CFileMaskW FMasks;
            int ExitCode;
            for(;;)
            {
               ExitCode=GetStringW(UMSG(MSortGroupsTitle),UMSG(MSortGroupsEnter),
                                  L"Masks",strNewMasks,strNewMasks,NM,
                                  HelpSortGroups,FIB_BUTTONS);
               if(!ExitCode)
                  break;
               if(FMasks.Set(strNewMasks, 0))
                  break;
            }
            if (ExitCode)
            /* IS $ */
            {
              if(AddMask(pNewGroup,strNewMasks,UpperGroup ? 0:DEFAULT_SORT_GROUP+1))
                return(SelPos);
            }
          }
          break;
        default:
          GroupList.ProcessInput();
          break;
      }
    }
    if (GroupList.Modal::GetExitCode()!=-1)
    {
      GroupList.ClearDone();
      GroupList.WriteInput(KEY_F4);
      continue;
    }
    break;
  }
  return(-1);
}
