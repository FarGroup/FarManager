/*
grpsort.cpp

Группы сортировки

*/

/* Revision: 1.01 13.07.2000 $ */

/*
Modify:
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

GroupSort::GroupSort()
{
  int I;
  GroupData=NULL;
  GroupCount=0;
  for (I=0;;I++)
  {
    char GroupName[80],GroupStr[sizeof(GroupData->Masks)];
    sprintf(GroupName,"UpperGroup%d",I);
    GetRegKey("SortGroups",GroupName,GroupStr,"",sizeof(GroupStr));
    if (*GroupStr==0)
      break;
    struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
    if (NewGroupData==NULL)
      break;
    GroupData=NewGroupData;
    strcpy(GroupData[GroupCount].Masks,GroupStr);
    GroupData[GroupCount].Group=I;
    GroupCount++;
  }
  for (I=0;;I++)
  {
    char GroupName[80],GroupStr[sizeof(GroupData->Masks)];
    sprintf(GroupName,"LowerGroup%d",I);
    GetRegKey("SortGroups",GroupName,GroupStr,"",sizeof(GroupStr));
    if (*GroupStr==0)
      break;
    struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
    if (NewGroupData==NULL)
      break;
    GroupData=NewGroupData;
    strcpy(GroupData[GroupCount].Masks,GroupStr);
    GroupData[GroupCount].Group=I+DEFAULT_SORT_GROUP+1;
    GroupCount++;
  }
}


GroupSort::~GroupSort()
{
  /* $ 13.07.2000 SVS
     ни кто не вызывал запрос памяти через new :-)
  */
  free(GroupData);
  /* SVS $ */
}


int GroupSort::GetGroup(char *Path)
{
  for (int I=0;I<GroupCount;I++)
  {
    struct GroupSortData *CurGroupData=&GroupData[I];
    char ArgName[NM],*NamePtr=CurGroupData->Masks;
    while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
      if (CmpName(ArgName,Path))
        return(CurGroupData->Group);
  }
  return(DEFAULT_SORT_GROUP);
}


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
        sprintf(GroupName,"UpperGroup%d",UpperGroup);
        GroupData[I].Group=UpperGroup++;
      }
      else
      {
        sprintf(GroupName,"LowerGroup%d",LowerGroup);
        GroupData[I].Group=DEFAULT_SORT_GROUP+1+LowerGroup++;
      }
      SetRegKey("SortGroups",GroupName,GroupData[I].Masks);
    }
    else
    {
      sprintf(GroupName,"UpperGroup%d",UpperGroup++);
      DeleteRegValue("SortGroups",GroupName);
      sprintf(GroupName,"LowerGroup%d",LowerGroup++);
      DeleteRegValue("SortGroups",GroupName);
    }
  }
  CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->LeftPanel->Redraw();
  CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
  struct MenuItem ListItem;
  ListItem.Checked=ListItem.Separator=*ListItem.UserData=ListItem.UserDataSize=0;

  VMenu GroupList(MSG(MSortGroupsTitle),NULL,0,ScrY-4);
  GroupList.SetFlags(MENU_WRAPMODE|MENU_SHOWAMPERSAND);
  GroupList.SetHelp("SortGroups");
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

  for (;GroupPos<GroupCount;I++)
  {
    struct MenuItem ListItem;
    ListItem.Checked=ListItem.Separator=*ListItem.UserData=ListItem.UserDataSize=0;

    strncpy(ListItem.Name,GroupData[GroupPos].Masks,sizeof(ListItem.Name));
    ListItem.Selected=(I == Pos);
    GroupList.AddItem(&ListItem);
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
            char GroupName[1024];
            sprintf(GroupName,"\"%s\"",GroupData[ListPos].Masks);
            if (Message(MSG_WARNING,2,MSG(MSortGroupsTitle),
                        MSG(MSortGroupsAskDel),GroupName,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;

            for (int I=ListPos+1;I<GroupCount;I++)
              GroupData[I-1]=GroupData[I];
            GroupCount--;
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            char NewMasks[1024];
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks","",NewMasks,sizeof(NewMasks)))
            {
              struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
              if (NewGroupData==NULL)
                break;
              GroupData=NewGroupData;
              GroupCount++;
              for (int I=GroupCount-1;I>ListPos;I--)
                GroupData[I]=GroupData[I-1];
              strncpy(GroupData[ListPos].Masks,NewMasks,sizeof(GroupData[ListPos].Masks));
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupCount)
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks",GroupData[ListPos].Masks,GroupData[ListPos].Masks,sizeof(GroupData[0].Masks)))
            {
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
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

