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

static char fmtUpperGroup[]="UpperGroup%d";
static char fmtLowerGroup[]="LowerGroup%d";
static char SortGroupsKeyName[]="SortGroups";

GroupSort::GroupSort()
{
  char GroupName[80],GroupMask[FILEFILTER_MASK_SIZE];
  static char *GroupFMT[2]={fmtUpperGroup,fmtLowerGroup};
  int  GroupDelta[2]={0,DEFAULT_SORT_GROUP+1};

  GroupData.Free();

  for (int J=0; J < 2; ++J)
  {
    for (int I=0;;I++)
    {
      sprintf(GroupName,GroupFMT[J],I);

      if (!GetRegKey(SortGroupsKeyName,GroupName,GroupMask,"",sizeof(GroupMask)))
        break;

      FileFilterParams *NewGroup = GroupData.addItem();

      if(NewGroup)
      {
        //Дефолтные значения выбраны так чтоб как можно правильней загрузить
        //настройки старых версий фара.

        char RegKey[80];

        sprintf(RegKey,"%s\\%s",SortGroupsKeyName,GroupName);

        NewGroup->SetMask((DWORD)GetRegKey(RegKey,"UseMask",1),
                           GroupMask);

        FILETIME DateAfter, DateBefore;
        GetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
        GetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
        NewGroup->SetDate((DWORD)GetRegKey(RegKey,"UseDate",0),
                          (DWORD)GetRegKey(RegKey,"DateType",0),
                          DateAfter,
                          DateBefore);

        NewGroup->SetSize((DWORD)GetRegKey(RegKey,"UseSize",0),
                          (DWORD)GetRegKey(RegKey,"SizeType",0),
                          GetRegKey64(RegKey,"SizeAbove",_i64(-1)),
                          GetRegKey64(RegKey,"SizeBelow",_i64(-1)));

        NewGroup->SetAttr((DWORD)GetRegKey(RegKey,"UseAttr",1),
                          (DWORD)GetRegKey(RegKey,"AttrSet",0),
                          (DWORD)GetRegKey(RegKey,"AttrClear",FILE_ATTRIBUTE_DIRECTORY));

        NewGroup->SetSortGroup(I+GroupDelta[J]);
      }
      else
        break;
    }
  }
}

GroupSort::~GroupSort()
{
  GroupData.Free();
}

int GroupSort::GetGroup(WIN32_FIND_DATA *fd)
{
  for (int i=0; i<GroupData.getCount(); i++)
  {
    FileFilterParams *CurGroupData=GroupData.getItem(i);
    if(CurGroupData->FileInFilter(fd))
       return(CurGroupData->GetSortGroup());
  }
  return DEFAULT_SORT_GROUP;
}

int GroupSort::GetGroup(FileListItem *fli)
{
  for (int i=0; i<GroupData.getCount(); i++)
  {
    FileFilterParams *CurGroupData=GroupData.getItem(i);
    if(CurGroupData->FileInFilter(fli))
       return(CurGroupData->GetSortGroup());
  }
  return DEFAULT_SORT_GROUP;
}

void GroupSort::EditGroups()
{
  int Pos=0, StartGroupCount=GroupData.getCount();

  while (Pos!=-1)
    Pos=EditGroupsMenu(Pos);

  for (int UpperGroup=0, LowerGroup=0, i=0; i< Max((int)GroupData.getCount(),StartGroupCount); i++)
  {
    char GroupName[80];
    char RegKey[80];
    if (i<GroupData.getCount())
    {
      if (GroupData.getItem(i)->GetSortGroup()<DEFAULT_SORT_GROUP)
      {
        sprintf(GroupName,fmtUpperGroup,UpperGroup);
        GroupData.getItem(i)->SetSortGroup(UpperGroup++);
      }
      else
      {
        sprintf(GroupName,fmtLowerGroup,LowerGroup);
        GroupData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP+1+LowerGroup++);
      }

      sprintf(RegKey,"%s\\%s",SortGroupsKeyName,GroupName);

      FileFilterParams *CurGroup = GroupData.getItem(i);

      const char *Mask;
      SetRegKey(RegKey,"UseMask",CurGroup->GetMask(&Mask));
      SetRegKey(SortGroupsKeyName,GroupName,Mask);


      DWORD DateType;
      FILETIME DateAfter, DateBefore;
      SetRegKey(RegKey,"UseDate",CurGroup->GetDate(&DateType, &DateAfter, &DateBefore));
      SetRegKey(RegKey,"DateType",DateType);
      SetRegKey(RegKey,"DateAfter",(BYTE *)&DateAfter,sizeof(DateAfter));
      SetRegKey(RegKey,"DateBefore",(BYTE *)&DateBefore,sizeof(DateBefore));


      DWORD SizeType;
      __int64 SizeAbove, SizeBelow;
      SetRegKey(RegKey,"UseSize",CurGroup->GetSize(&SizeType, &SizeAbove, &SizeBelow));
      SetRegKey(RegKey,"SizeType",SizeType);
      SetRegKey64(RegKey,"SizeAbove",SizeAbove);
      SetRegKey64(RegKey,"SizeBelow",SizeBelow);


      DWORD AttrSet, AttrClear;
      SetRegKey(RegKey,"UseAttr",CurGroup->GetAttr(&AttrSet, &AttrClear));
      SetRegKey(RegKey,"AttrSet",AttrSet);
      SetRegKey(RegKey,"AttrClear",AttrClear);
    }
    else
    {
      sprintf(GroupName,fmtUpperGroup,UpperGroup++);
      sprintf(RegKey,"%s\\%s",SortGroupsKeyName,GroupName);
      DeleteRegValue(SortGroupsKeyName,GroupName);
      DeleteKeyTree(RegKey);
      sprintf(GroupName,fmtLowerGroup,LowerGroup++);
      sprintf(RegKey,"%s\\%s",SortGroupsKeyName,GroupName);
      DeleteRegValue(SortGroupsKeyName,GroupName);
      DeleteKeyTree(RegKey);
    }
  }

  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->LeftPanel->Redraw();
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
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
  for (I=0;GroupPos<GroupData.getCount();I++)
  {
    if (GroupData.getItem(GroupPos)->GetSortGroup()>DEFAULT_SORT_GROUP)
      break;
    const char *Mask;
    GroupData.getItem(GroupPos)->GetMask(&Mask);
    xstrncpy(ListItem.Name,Mask,sizeof(ListItem.Name)-1);
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
  for (;GroupPos<GroupData.getCount();I++)
  {
    const char *Mask;
    GroupData.getItem(GroupPos)->GetMask(&Mask);
    xstrncpy(ListItem2.Name,Mask,sizeof(ListItem2.Name)-1);
    ListItem2.SetSelect(I == Pos);
    GroupList.AddItem(&ListItem2);
    GroupPos++;
  }

  *ListItem.Name=0;
  ListItem.SetSelect(GroupData.getCount()+2 == Pos);
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
          if (ListPos<GroupData.getCount())
          {
            char GroupName[72]="\"";
            const char *Mask;
            GroupData.getItem(ListPos)->GetMask(&Mask);
            xstrncpy(GroupName+1,Mask,sizeof(GroupName)-4);
            strcat(GroupName,"\"");
            if (Message(MSG_WARNING,2,MSG(MSortGroupsTitle),
                        MSG(MSortGroupsAskDel),GroupName,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;
            GroupData.deleteItem(ListPos);
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            FileFilterParams *NewGroup = GroupData.insertItem(ListPos);

            if (NewGroup)
            {
              //AY: Раз создаём новый фильтр то думаю будет логично если он будет только для файлов
              NewGroup->SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);

              if(FileFilterConfig(NewGroup))
              {
                NewGroup->SetSortGroup(UpperGroup ? 0 : DEFAULT_SORT_GROUP+1);
                return(SelPos);
              }
              else
                GroupData.deleteItem(ListPos);
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupData.getCount())
          {
            FileFilterParams *NewGroup = GroupData.getItem(ListPos);
            if(FileFilterConfig(NewGroup))
            {
              NewGroup->SetSortGroup(UpperGroup ? 0 : DEFAULT_SORT_GROUP+1);
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
