#ifndef __GROUPSORT_HPP__
#define __GROUPSORT_HPP__
/*
grpsort.hpp

Группы сортировки

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class GroupSort
{
  private:
    int EditGroupsMenu(int Pos);
    struct GroupSortData *GroupData;
    int GroupCount;
  public:
    GroupSort();
    ~GroupSort();
    int GetGroup(char *Path);
    void EditGroups();
};

#endif	// __GROUPSORT_HPP__
