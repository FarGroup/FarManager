/*
farqueue.cpp

Шаблон работы с очередью
Использование:
     FarQueue<int> *q;
     q=new FarQueue<int>(10);

     for( int j = 0; j < 5; j++ )
     {
       for( int i = 0; i < 5; i++ )
         q->Put( i );

       while( !q->isEmpty( ) )
         cout << q->Get( ) << endl;
     }
*/

#include "headers.hpp"
#pragma hdrstop

#include "farqueue.hpp"

template <class Object>
FarQueue<Object>::FarQueue(int SizeQueue)
{
	Array=NULL;
	Init(SizeQueue);
}

template <class Object>
FarQueue<Object>::~FarQueue()
{
	if (Array)
		delete[] Array;
}


template <class Object>
BOOL FarQueue<Object>::isEmpty() const
{
	return CurrentSize == 0;
}

template <class Object>
BOOL FarQueue<Object>::isFull() const
{
	return CurrentSize == Size;
}

template <class Object>
int FarQueue<Object>::Init(int SizeQueue)
{
	if (Array)
		delete[] Array;

	Array=new Object[Size=SizeQueue];

	if (!Array)
		Size=0;

	CurrentSize=0;
	Front=0;
	Back=-1;
	return Size != 0;
}

template <class Object>
Object FarQueue<Object>::Peek() const
{
	if (isEmpty())
		return (Object)NULL;

	return Array[Front];
}

template <class Object>
Object FarQueue<Object>::Get()
{
	if (isEmpty())
		return (Object)NULL;

	CurrentSize--;
	Object FrontItem = Array[Front];
	increment(Front);
	return FrontItem;
}

template <class Object>
int FarQueue<Object>::Put(const Object& x)
{
	if (isFull())
		return FALSE;

	increment(Back);
	Array[Back] = x;
	CurrentSize++;
	return TRUE;
}

template <class Object>
void FarQueue<Object>::increment(int& x)
{
	if (++x == Size)
		x=0;
}

//#ifdef _MSC_VER
template class FarQueue<DWORD>;
//#endif

#if 0
#include <iostream.h>
void main()
{
	FarQueue<int> *q;
	q=new FarQueue<int>(1);

	for (int j = 0; j < 5; j++)
	{
		for (int i = 0; i < 5; i++)
			q->Put(i);

		while (!q->isEmpty())
			cout << q->Get() << endl;
	}

	delete q;
}
#endif
