/*
grpsort.cpp

Группы сортировки

*/

/* Revision: 1.06 06.04.2001 $ */

/*
Modify:
  06.04.2001 SVS
    + В группах сортировки можно задавать переменные окружения, с учетом
      переменной PATHEXT
  19.03.2001 SVS
    - Бага: Ptr был неинициализирован для случая если длины хватало.
    + Немного оптимизации кода.
  12.02.2001 SVS
    ! Не был назначен Хелп для ввода маски группы сортировки
    - устранение утечки памяти (после 440-го)
  11.02.2001 SVS
    ! Введение DIF_VAREDIT позволило расширить размер под маски
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static char fmtUpperGroup[]="UpperGroup%d";
static char fmtLowerGroup[]="LowerGroup%d";
static char SortGroupsKeyName[]="SortGroups";

GroupSort::GroupSort()
{
  int I, J;
  char GroupName[80],GroupStr[GROUPSORT_MASK_SIZE], *Ptr;
  static char *GroupFMT[2]={fmtUpperGroup,fmtLowerGroup};
  int  GroupDelta[2]={0,DEFAULT_SORT_GROUP+1};

  GroupData=NULL;
  GroupCount=0;
  for (J=0; J < 2; ++J)
    for (I=0;;I++)
    {
      sprintf(GroupName,GroupFMT[J],I);
      GetRegKey(SortGroupsKeyName,GroupName,GroupStr,"",sizeof(GroupStr));
      if (*GroupStr==0)
        break;
      if((Ptr=(char *)malloc(strlen(GroupStr)+1)) == NULL)
        break;
      struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
      if (NewGroupData==NULL)
      {
        free(Ptr);
        break;
      }
      GroupData=NewGroupData;
      GroupData[GroupCount].Masks=Ptr;
      strcpy(GroupData[GroupCount].Masks,GroupStr);
      GroupData[GroupCount].Group=I+GroupDelta[J];
      GroupCount++;
    }
}


GroupSort::~GroupSort()
{
  if(GroupData)
  {
    for(int I=0; I < GroupCount; ++I)
      if(GroupData[I].Masks)
        free(GroupData[I].Masks);
    free(GroupData);
  }
}


/* $ 06.04.2001 SVS
   В группах сортировки можно задавать переменные окружения, с учетом
   переменной PATHEXT
*/
int GroupSort::GetGroup(char *Path)
{
  char ExpandedStr[8192];
  for (int I=0;I<GroupCount;I++)
  {
    struct GroupSortData *CurGroupData=&GroupData[I];
    char ArgName[NM],*NamePtr;
    int Copied=ExpandPATHEXT(CurGroupData->Masks,ExpandedStr,sizeof(ExpandedStr));
    if ((Copied==0) || (Copied > sizeof(ExpandedStr)))
      strcpy(ExpandedStr, CurGroupData->Masks);
    NamePtr = ExpandedStr;
    while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
      if (CmpName(ArgName,Path))
        return(CurGroupData->Group);
  }
  return(DEFAULT_SORT_GROUP);
}
/* SVS $ */


void GroupSort::EditGroups()
{
  int UpperGroup,LowerGroup,I,Pos=0,StartGroupCount=GroupCount;

  while (Pos!=-1)
    Pos=EditGroupsMenu(Pos);

  for (UpperGroup=LowerGroup=I=0;I<Max(GroupCount,StartGroupCount);I++)
  {
    char GroupName[100];
    if (I<GroupCount)
    {
      if (GroupData[I].Group<DEFAULT_SORT_GROUP)
      {
        sprintf(GroupName,fmtUpperGroup,UpperGroup);
        GroupData[I].Group=UpperGroup++;
      }
      else
      {
        sprintf(GroupName,fmtLowerGroup,LowerGroup);
        GroupData[I].Group=DEFAULT_SORT_GROUP+1+LowerGroup++;
      }
      SetRegKey(SortGroupsKeyName,GroupName,GroupData[I].Masks);
    }
    else
    {
      sprintf(GroupName,fmtUpperGroup,UpperGroup++);
      DeleteRegValue(SortGroupsKeyName,GroupName);
      sprintf(GroupName,fmtLowerGroup,LowerGroup++);
      DeleteRegValue(SortGroupsKeyName,GroupName);
    }
  }
  CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->LeftPanel->Redraw();
  CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
  char NewMasks[GROUPSORT_MASK_SIZE], *Ptr;
  char *HelpSortGroups="SortGroups";
  struct MenuItem ListItem;
  struct MenuItem ListItem2;
  memset(&ListItem,0,sizeof(ListItem));

  VMenu GroupList(MSG(MSortGroupsTitle),NULL,0,ScrY-4);
  GroupList.SetFlags(MENU_WRAPMODE|MENU_SHOWAMPERSAND);
  GroupList.SetHelp(HelpSortGroups);
  GroupList.SetPosition(-1,-1,0,0);
  GroupList.SetBottomTitle(MSG(MSortGroupsBottom));

  int GroupPos=0,I;
  for (I=0;GroupPos<GroupCount;I++)
  {
    if (GroupData[GroupPos].Group>DEFAULT_SORT_GROUP)
      break;
    strncpy(ListItem.Name,GroupData[GroupPos].Masks,sizeof(ListItem.Name));
    ListItem.Selected=(I == Pos);
    GroupList.AddItem(&ListItem);
    GroupPos++;
  }
  int UpperGroupCount=GroupPos;

  *ListItem.Name=0;
  ListItem.Selected=(I == Pos);
  GroupList.AddItem(&ListItem);
  ListItem.Selected=0;
  ListItem.Separator=TRUE;
  GroupList.AddItem(&ListItem);
  ListItem.Separator=0;
  I+=2;

  memset(&ListItem2,0,sizeof(ListItem2));
  for (;GroupPos<GroupCount;I++)
  {
    strncpy(ListItem2.Name,GroupData[GroupPos].Masks,sizeof(ListItem2.Name));
    ListItem2.Selected=(I == Pos);
    GroupList.AddItem(&ListItem2);
    GroupPos++;
  }

  *ListItem.Name=0;
  ListItem.Selected=(GroupCount+2 == Pos);
  GroupList.AddItem(&ListItem);

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
            char GroupName[72]="\"";
            strncpy(GroupName+1,GroupData[ListPos].Masks,sizeof(GroupName)-4);
            strcat(GroupName,"\"");
            if (Message(MSG_WARNING,2,MSG(MSortGroupsTitle),
                        MSG(MSortGroupsAskDel),GroupName,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;

            free(GroupData[ListPos].Masks);

            for (int I=ListPos+1;I<GroupCount;I++)
              GroupData[I-1]=GroupData[I];
            GroupCount--;
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks","",NewMasks,sizeof(NewMasks),HelpSortGroups))
            {
              if((Ptr=(char *)malloc(strlen(NewMasks)+1)) == NULL)
                break;
              struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
              if (NewGroupData==NULL)
              {
                free(Ptr);
                break;
              }
              GroupData=NewGroupData;
              GroupCount++;
              for (int I=GroupCount-1;I>ListPos;I--)
                GroupData[I]=GroupData[I-1];
              GroupData[ListPos].Masks=Ptr;
              strcpy(GroupData[ListPos].Masks,NewMasks);
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupCount)
          {
            strcpy(NewMasks,GroupData[ListPos].Masks);
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks",NewMasks,NewMasks,sizeof(NewMasks),HelpSortGroups))
            {
              /* $ 19.03.2001 SVS
                 - Ptr был неинициализирован для случая если длины хватало.
              */
              if(strlen(GroupData[ListPos].Masks) < strlen(NewMasks))
              {
                Ptr=(char *)realloc(GroupData[ListPos].Masks,strlen(NewMasks)+1);
                if(Ptr == NULL)
                  break;
                GroupData[ListPos].Masks=Ptr;
              }
              /* SVS $ */
              strcpy(GroupData[ListPos].Masks,NewMasks);
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
            }
          }
          break;
        default:
          GroupList.ProcessInput();
          break;
      }
    }
    if (GroupList.GetExitCode()!=-1)
    {
      GroupList.ClearDone();
      GroupList.WriteInput(KEY_F4);
      continue;
    }
    break;
  }
  return(-1);
}
