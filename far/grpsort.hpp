#ifndef __GROUPSORT_HPP__
#define __GROUPSORT_HPP__
/*
grpsort.hpp

Группы сортировки

*/

/* Revision: 1.02 15.03.2006 $ */

class GroupSort
{
  private:
    struct GroupSortData **GroupData;
    int GroupCount;

  private:
    int EditGroupsMenu(int Pos);
    const wchar_t *GetMask(int Idx);
    BOOL AddMask(struct GroupSortData *Dest,const wchar_t *Mask,int Group);
    void DeleteMask(struct GroupSortData *CurGroupData);

  public:
    GroupSort();
    ~GroupSort();

  public:
    int GetGroup(const wchar_t *Path);
    void EditGroups();
};

#endif  // __GROUPSORT_HPP__
