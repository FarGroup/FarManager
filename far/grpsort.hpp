#ifndef __GROUPSORT_HPP__
#define __GROUPSORT_HPP__
/*
grpsort.hpp

Группы сортировки

*/

#include "filefilterparams.hpp"
#include "array.hpp"

class GroupSort
{
  private:
    TPointerArray<FileFilterParams> GroupData;

  private:
    int EditGroupsMenu(int Pos);

  public:
    GroupSort();
    ~GroupSort();

  public:
    int GetGroup(WIN32_FIND_DATA *fd);
    int GetGroup(FileListItem *fli);
    void EditGroups();
};

#endif  // __GROUPSORT_HPP__
