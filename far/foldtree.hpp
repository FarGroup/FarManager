#ifndef __FOLDERTREE_HPP__
#define __FOLDERTREE_HPP__
/*
foldtree.hpp

Поиск каталога по Alt-F10

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class FolderTree:public Modal
{
  private:
    void DrawEdit();
    TreeList *Tree;
    Edit *FindEdit;
    char NewFolder[NM];
    char LastName[NM];
  public:
    FolderTree(char *ResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
};


#endif	// __FOLDERTREE_HPP__
