#ifndef __DLGEDIT_HPP__
#define __DLGEDIT_HPP__
/*
dlgedit.hpp

Одиночная строка редактирования для диалога
(как наследник класса Edit)

*/

/* Revision: 1.00 15.05.2002 $ */

/*
Modify:
  15.05.2002 SVS
    ! Типа создан. Сюда нужно перетащить из  edit.hpp все вещи,
      касаемые масок и.. все что относится только к диалогам
      Это пока только шаблон, заготовка для будущего перехода
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
