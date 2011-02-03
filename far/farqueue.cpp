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
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "farqueue.hpp"

template <class Object>
FarQueue<Object>::FarQueue(int SizeQueue):
	Array(nullptr)
{
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
	return !CurrentSize;
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
	return Size ;
}

template <class Object>
Object FarQueue<Object>::Peek() const
{
	if (isEmpty())
		return 0;

	return Array[Front];
}

template <class Object>
Object FarQueue<Object>::Get()
{
	if (isEmpty())
		return 0;

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
