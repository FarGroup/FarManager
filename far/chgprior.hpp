#ifndef __CHANGEPRIORITY_HPP__
#define __CHANGEPRIORITY_HPP__
/*
chgprior.hpp

class ChangePriority

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class ChangePriority
{
  private:
    int SavePriority;
  public:
    ChangePriority(int NewPriority);
    ~ChangePriority();
};

#endif	// __CHANGEPRIORITY_HPP__
