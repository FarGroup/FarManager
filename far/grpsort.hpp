#ifndef __GROUPSORT_HPP__
#define __GROUPSORT_HPP__
/*
grpsort.hpp

Группы сортировки

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
