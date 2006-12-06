#ifndef __FOLDERTREE_HPP__
#define __FOLDERTREE_HPP__
/*
foldtree.hpp

Поиск каталога по Alt-F10

*/

#include "frame.hpp"
#include "farconst.hpp"

class TreeList;
class Edit;
class SaveScreen;

class FolderTree:public Frame
{
  private:
    TreeList *Tree;
    Edit *FindEdit;
    SaveScreen *TopScreen;      // область сохранения под хелпом

    int ModalMode;
    int IsFullScreen;
    int IsStandalone;
    int  PrevMacroMode;        // предыдущий режим макроса

    string strNewFolder;
    string strLastName;

  private:
    void DrawEdit();
    virtual void DisplayObject();
    void SetCoords();

  public:
    FolderTree(string &strResultFolder,int ModalMode,int IsStandalone=TRUE,int IsFullScreen=TRUE);
    ~FolderTree();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    void OnChangeFocus(int focus); // вызывается при смене фокуса
    void ResizeConsole();
    /* $ Введена для нужд CtrlAltShift OT */
    int  FastHide();
    void Hide();

    virtual const wchar_t *GetTypeName() {return L"[FolderTree]";}
    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_FINDFOLDER; }

};


#endif  // __FOLDERTREE_HPP__
