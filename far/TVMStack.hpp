#ifndef __TVMSTACK_HPP__
#define __TVMSTACK_HPP__

/*  VMStack.hpp
    Стек (LIFO) для виртуальной машины макросов
*/

#include "tvar.hpp"

class TVMStack
{
  private:
    unsigned int Total;
    struct OneItem
    {
      TVar  Item;
      OneItem *Next;

      OneItem(TVar NewVar,OneItem *NextItem) : Item(NewVar), Next(NextItem) {}
    };

    struct OneItem *Top, *current;

  public:
    static TVar errorStack;

  public:
    TVMStack():Total(0), Top(NULL) {};
    ~TVMStack() { Free(); }

  public:
    // вернуть количество элементов на стеке
    unsigned int Size() const { return Total; }

    // взять элемент со стека
    TVar Pop();

    // взять элемент со стека без изменения стека
    TVar Peek();

    // положить элемент на стек
    TVar Push(const TVar &Source);

    // очистить стек
    void Free();

    bool isEmpty() const {return Total==0;};

  private:
    TVMStack& operator=(const TVMStack& rhs); /* чтобы не генерировалось */
    TVMStack(const TVMStack& rhs);            /* по умолчанию            */
};

#endif // __TVMSTACK_HPP__
