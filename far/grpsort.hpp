#ifndef __GROUPSORT_HPP__
#define __GROUPSORT_HPP__
/*
grpsort.hpp

Группы сортировки

*/

/* Revision: 1.01 23.04.2001 $ */

/*
Modify:
  23.04.2001 SVS
    ! КХЕ! Новый вз<ляд на %PATHEXT% - то что редактируем и то, что
      юзаем - разные сущности.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class GroupSort
{
  private:
    struct GroupSortData *GroupData;
    int GroupCount;

  private:
    int EditGroupsMenu(int Pos);
    char *GetMask(int Idx);
    BOOL AddMask(struct GroupSortData *Dest,char *Mask,int Group);
    void DeleteMask(struct GroupSortData *CurGroupData);

  public:
    GroupSort();
    ~GroupSort();

  public:
    int GetGroup(char *Path);
    void EditGroups();
};

#endif	// __GROUPSORT_HPP__
