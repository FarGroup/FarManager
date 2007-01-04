#ifndef __DLGEDIT_HPP__
#define __DLGEDIT_HPP__
/*
dlgedit.hpp

Одиночная строка редактирования для диалога
(как наследник класса Edit)

*/

#include "edit.hpp"

class DlgEdit:public Edit
{
  friend class Dialog;

  private: // приватные данные

  public:  // публичные данные

  private: // приватные методы

  public:
    DlgEdit();
    ~DlgEdit();

  public: // публичные методы
};

#endif  // __DLGEDIT_HPP__
