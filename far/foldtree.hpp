#ifndef __FOLDERTREE_HPP__
#define __FOLDERTREE_HPP__
/*
foldtree.hpp

Поиск каталога по Alt-F10

*/

/* Revision: 1.04 21.11.2005 $ */

/*
Modify:
  21.11.2005 WARP
    ! char LastName -> string strLastName
  01.08.2005 WARP
    ! see 00035.Unicode.txt
  24.10.2001 SVS
    + дополнительный параметр у FolderTree - "ЭТО НЕ ПАНЕЛЬ!"
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "modal.hpp"
#include "farconst.hpp"

class TreeList;
class Edit;

class FolderTree:public Modal
{
  private:
    TreeList *Tree;
    Edit *FindEdit;

    string strNewFolder;
    string strLastName;

  private:
    void DrawEdit();

  public:
    FolderTree(string &strResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2,int IsPanel=TRUE);

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
};


#endif  // __FOLDERTREE_HPP__
