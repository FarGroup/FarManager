/*
grpsort.cpp

Группы сортировки

*/

/* Revision: 1.09 29.04.2001 $ */

/*
Modify:
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  23.04.2001 SVS
    ! КХЕ! Новый взбляд на %PATHEXT% - то что редактируем и то, что
      юзаем - разные сущности.
  08.04.2001 SVS
    ! Группы сортировки не поддерживают переменные среды. В морг!
      Ставим скорость во главу угла.
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
  char GroupName[80],GroupStr[GROUPSORT_MASK_SIZE], *Ptr, *OPtr;
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
      struct GroupSortData NewGroup={0}; // обязательно проинициализируем
      if(AddMask(&NewGroup,GroupStr,I+GroupDelta[J]))
      {
        struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
        if (NewGroupData==NULL)
        {
          DeleteMask(&NewGroup);
          break;
        }
        GroupData=NewGroupData;
        memcpy(GroupData+GroupCount,&NewGroup,sizeof(struct GroupSortData));
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
      DeleteMask(GroupData+I);
    free(GroupData);
  }
}

BOOL GroupSort::AddMask(struct GroupSortData *Dest,char *Mask,int Group)
{
  char *Ptr, *OPtr;
  /* Обработка %PATHEXT% */
  // память под оригинал - OriginalMasks
  if((OPtr=(char *)realloc(Dest->OriginalMasks,strlen(Mask)+1)) == NULL)
    return FALSE;
  strcpy(OPtr,Mask); // сохраняем оригинал.
  // проверим
  if((Ptr=strchr(Mask,'%')) != NULL && !strnicmp(Ptr,"%PATHEXT%",9))
  {
    int IQ1=(*(Ptr+9) == ',')?10:9;
    // Если встречается %pathext%, то допишем в конец...
    memmove(Ptr,Ptr+IQ1,strlen(Ptr+IQ1)+1);
    Add_PATHEXT(Mask); // добавляем то, чего нету.
    // в GroupStr находится рабочая маска
  }
  else
  {
    free(OPtr); // "съэкономи пару байт"
    OPtr=NULL;
  }
  // память под рабочую маску
  if((Ptr=(char *)realloc(Dest->Masks,strlen(Mask)+1)) == NULL)
  {
    if(OPtr) free(OPtr);
    return FALSE;
  }

  strcpy((Dest->Masks=Ptr),Mask);
  Dest->OriginalMasks=OPtr;
  Dest->Group=Group;
  return TRUE;
}

void GroupSort::DeleteMask(struct GroupSortData *CurGroupData)
{
  if(CurGroupData->Masks)
  {
    free(CurGroupData->Masks);
    CurGroupData->Masks=NULL;
  }
  if(CurGroupData->OriginalMasks)
  {
    free(CurGroupData->OriginalMasks);
    CurGroupData->OriginalMasks=NULL;
  }
}

char *GroupSort::GetMask(int Idx)
{
  struct GroupSortData *GData=GroupData+Idx;
  return ((GData->OriginalMasks)?GData->OriginalMasks:GData->Masks);
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
        sprintf(GroupName,fmtUpperGroup,UpperGroup);
        GroupData[I].Group=UpperGroup++;
      }
      else
      {
        sprintf(GroupName,fmtLowerGroup,LowerGroup);
        GroupData[I].Group=DEFAULT_SORT_GROUP+1+LowerGroup++;
      }
      SetRegKey(SortGroupsKeyName,GroupName,GetMask(I));
    }
    else
    {
      sprintf(GroupName,fmtUpperGroup,UpperGroup++);
      DeleteRegValue(SortGroupsKeyName,GroupName);
      sprintf(GroupName,fmtLowerGroup,LowerGroup++);
      DeleteRegValue(SortGroupsKeyName,GroupName);
    }
  }
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->LeftPanel->Redraw();
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
  struct GroupSortData NewGroup;
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
    strncpy(ListItem.Name,GetMask(GroupPos),sizeof(ListItem.Name));
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
    strncpy(ListItem2.Name,GetMask(GroupPos),sizeof(ListItem2.Name));
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
            strncpy(GroupName+1,GetMask(ListPos),sizeof(GroupName)-4);
            strcat(GroupName,"\"");
            if (Message(MSG_WARNING,2,MSG(MSortGroupsTitle),
                        MSG(MSortGroupsAskDel),GroupName,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;
            DeleteMask(GroupData+ListPos);
            for (int I=ListPos+1;I<GroupCount;I++)
              GroupData[I-1]=GroupData[I];
            GroupCount--;
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            NewMasks[0]=0;
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks","",NewMasks,sizeof(NewMasks),HelpSortGroups))
            {
              memset(&NewGroup,0,sizeof(struct GroupSortData));
              if(AddMask(&NewGroup,NewMasks,UpperGroup ? 0:DEFAULT_SORT_GROUP+1))
              {
                struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
                if (NewGroupData==NULL)
                {
                  DeleteMask(&NewGroup);
                  break;
                }
                GroupData=NewGroupData;
                GroupCount++;
                for (int I=GroupCount-1;I>ListPos;I--)
                  GroupData[I]=GroupData[I-1];
                memcpy(GroupData+ListPos,&NewGroup,sizeof(struct GroupSortData));
                return(SelPos);
              }
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupCount)
          {
            memcpy(&NewGroup,GroupData+ListPos,sizeof(struct GroupSortData));
            strcpy(NewMasks,GetMask(ListPos));
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks",NewMasks,NewMasks,sizeof(NewMasks),HelpSortGroups))
            {
              if(AddMask(&NewGroup,NewMasks,UpperGroup ? 0:DEFAULT_SORT_GROUP+1))
              {
                memcpy(GroupData+ListPos,&NewGroup,sizeof(struct GroupSortData));
                return(SelPos);
              }
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
