#ifndef __FARQUEUE_HPP__
#define __FARQUEUE_HPP__
/*
farqueue.hpp

������ ������ � ��������

*/

/* Revision: 1.00 24.01.2001 $ */

/*
Modify:
  24.01.2001 SVS
   + ����� ����.
*/

template <class Object>
class FarQueue
{
  private:
    Object        *Array;
    int            Size;
    int            CurrentSize;
    int            Front;
    int            Back;

  private:
    void increment(int& x);

  public:
    FarQueue(int SizeQueue=64);
   ~FarQueue();

  public:
    int Init(int SizeQueue);

    BOOL isEmpty() const;
    BOOL isFull() const;

    Object Peek() const;


    Object Get();
    int Put(const Object& x);
};

#endif // _FARQUEUE_H
