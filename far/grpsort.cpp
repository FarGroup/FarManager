/*
grpsort.cpp

Группы сортировки

*/

/* Revision: 1.16 27.07.2001 $ */

/*
Modify:
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  18.07.2001 OT
    ! VFMenu
  06.07.2001 IS
    + Теперь в группах сортировки можно использовать маски исключения, маски
      файлов можно брать в кавычки, маски файлов проверяются на корректность,
      можно использовать точку с запятой в качестве разделителя масок.
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  07.05.2001 DJ
    ! оптимизация
  06.05.2001 DJ
    ! перетрях #include
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
  CFileMask *FMasks;
  char *OriginalMasks;
  int Group;
  int reserved; // для выравнивания на 16 :-)
};
/* IS $ */

#define GROUPSORT_MASK_SIZE      2048

static char fmtUpperGroup[]="UpperGroup%d";
static char fmtLowerGroup[]="LowerGroup%d";
static char SortGroupsKeyName[]="SortGroups";

GroupSort::GroupSort()
{
  int I, J;
  char GroupName[80],GroupStr[GROUPSORT_MASK_SIZE];
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

/* $ 01.05.2001 DJ
   оптимизированный формат хранения в Masks
*/
/* $ 06.07.2001 IS
   + вместо "рабочей" маски используем соответствующий класс
   ! не использую Add_PATHEXT
*/
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
    int IQ1=(*(Ptr+9) == ',')?10:9, offsetPtr=Ptr-Mask;
    // Если встречается %pathext%, то допишем в конец...
    memmove(Ptr,Ptr+IQ1,strlen(Ptr+IQ1)+1);

    char Tmp1[GROUPSORT_MASK_SIZE], *pSeparator;
    strcpy(Tmp1, Mask);
    pSeparator=strchr(Tmp1, EXCLUDEMASKSEPARATOR);
    if(pSeparator)
    {
      Ptr=Tmp1+offsetPtr;
      if(Ptr>pSeparator) // PATHEXT находится в масках исключения
        Add_PATHEXT(Mask); // добавляем то, чего нету.
      else
      {
        char Tmp2[GROUPSORT_MASK_SIZE];
        strcpy(Tmp2, pSeparator+1);
        *pSeparator=0;
        Add_PATHEXT(Tmp1);
        sprintf(Mask, "%s|%s", Tmp1, Tmp2);
      }
    }
    else
      Add_PATHEXT(Mask); // добавляем то, чего нету.
  }
  // память под рабочую маску
  if((Dest->FMasks=new CFileMask) == NULL)
  {
    free(OPtr);
    return FALSE;
  }

  if(!Dest->FMasks->Set(Mask, FMF_SILENT)) // проверим корректность маски
  {
    delete Dest->FMasks;
    Dest->FMasks=NULL;
    free(OPtr);
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
    free(CurGroupData->OriginalMasks);
    CurGroupData->OriginalMasks=NULL;
  }
}
/* IS $ */

/* $ 06.07.2001 IS "рабочей" маски теперь у нас нет */
char *GroupSort::GetMask(int Idx)
{
  return (GroupData+Idx)->OriginalMasks;
}
/* IS $ */

/* $ 01.05.2001 DJ
   оптимизированный формат хранения в Masks
*/
/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
int GroupSort::GetGroup(char *Path)
{
  for (int I=0;I<GroupCount;I++)
  {
    struct GroupSortData *CurGroupData=&GroupData[I];
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
  char NewMasks[GROUPSORT_MASK_SIZE];
  char *HelpSortGroups="SortGroups";
  struct MenuItem ListItem;
  struct MenuItem ListItem2;
  memset(&ListItem,0,sizeof(ListItem));

  VMenu GroupList(MSG(MSortGroupsTitle),NULL,0,ScrY-4);
  GroupList.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
  GroupList.SetHelp(HelpSortGroups);
  GroupList.SetPosition(-1,-1,0,0);
  GroupList.SetBottomTitle(MSG(MSortGroupsBottom));

  int GroupPos=0,I;
  for (I=0;GroupPos<GroupCount;I++)
  {
    if (GroupData[GroupPos].Group>DEFAULT_SORT_GROUP)
      break;
    strncpy(ListItem.Name,GetMask(GroupPos),sizeof(ListItem.Name)-1);
    ListItem.SetSelect(I == Pos);
    GroupList.AddItem(&ListItem);
    GroupPos++;
  }
  int UpperGroupCount=GroupPos;

  *ListItem.Name=0;
  ListItem.SetSelect(I == Pos);
  GroupList.AddItem(&ListItem);
  ListItem.Flags&=~LIF_SELECTED;
  ListItem.Flags|=LIF_SEPARATOR;
  GroupList.AddItem(&ListItem);
  ListItem.Flags&=~LIF_SEPARATOR;
  I+=2;

  memset(&ListItem2,0,sizeof(ListItem2));
  for (;GroupPos<GroupCount;I++)
  {
    strncpy(ListItem2.Name,GetMask(GroupPos),sizeof(ListItem2.Name)-1);
    ListItem2.SetSelect(I == Pos);
    GroupList.AddItem(&ListItem2);
    GroupPos++;
  }

  *ListItem.Name=0;
  ListItem.SetSelect(GroupCount+2 == Pos);
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
            /* $ 06.07.2001 IS
               проверяем маску на корректность
            */
            *NewMasks=0;
            CFileMask FMasks;
            int ExitCode;
            for(;;)
            {
               ExitCode=GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),
                         "Masks","",NewMasks,sizeof(NewMasks),HelpSortGroups);
               if(!ExitCode)
                  break;
               if(FMasks.Set(NewMasks, 0))
                  break;
            }
            if (ExitCode)
            /* IS $ */
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
            /* $ 06.07.2001 IS
               проверяем маску на корректность
            */
            CFileMask FMasks;
            int ExitCode;
            for(;;)
            {
               ExitCode=GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),
                                  "Masks",NewMasks,NewMasks,sizeof(NewMasks),
                                  HelpSortGroups);
               if(!ExitCode)
                  break;
               if(FMasks.Set(NewMasks, 0))
                  break;
            }
            if (ExitCode)
            /* IS $ */
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
