#pragma once

/*
array.hpp

Шаблон работы с массивом

 TArray<Object> Array;
 // Object должен иметь конструктор по умолчанию и следующие операторы
 //  bool operator==(const Object &) const
 //  bool operator<(const Object &) const
 //  const Object& operator=(const Object &)

 TPointerArray<Object> Array;
 Object должен иметь конструктор по умолчанию.
 Класс для тупой но прозрачной работы с массивом понтеров на класс Object
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#ifdef __GNUC__
typedef int __cdecl(*TARRAYCMPFUNC)(const void *el1,const void *el2);
#else
typedef int (*TARRAYCMPFUNC)(const void *el1,const void *el2);
#endif

template <class Object>
class TArray
{
	private:
		unsigned int internalCount, Count, Delta;
		Object **items;

	private:
		static int __cdecl CmpItems(const Object **el1, const Object **el2);
		bool deleteItem(unsigned int index);

	public:
		TArray(unsigned int Delta=8);
		~TArray() { Free(); }
		TArray<Object>(const TArray<Object> &rhs);
		TArray& operator=(const TArray<Object> &rhs);

	public:
		void Free();
		void setDelta(unsigned int newDelta);
		bool setSize(unsigned int newSize);
		Object *setItem(unsigned int index, const Object &newItem);
		Object *getItem(unsigned int index);
		int getIndex(const Object &item, int start=-1);
		void Sort(TARRAYCMPFUNC user_cmp_func=NULL); // отсортировать массив
		bool Pack(); // упаковать массив - вместо нескольких одинаковых элементов,
		// идущих подряд, оставить только один. Возвращает, false,
		// если изменений массива не производилось.
		// Вызов Pack() после Sort(NULL) приведет к устранению
		// дубликатов

	public: // inline
		unsigned int getSize() const { return Count; }
		Object *addItem(const Object &newItem) { return setItem(Count,newItem); }
};

template <class Object>
TArray<Object>::TArray(unsigned int delta):
		internalCount(0), Count(0), items(NULL)
{
	setDelta(delta);
}

template <class Object>
void TArray<Object>::Free()
{
	if (items)
	{
		for (unsigned i=0; i<Count; ++i)
			delete items[i];

		xf_free(items);
		items=NULL;
	}

	Count=internalCount=0;
}

template <class Object>
Object *TArray<Object>::setItem(unsigned int index, const Object &newItem)
{
	bool set=true;

	if (index<Count)
		deleteItem(index);
	else
		set=setSize(index+(index==Count));

	if (set)
	{
		items[index]=new Object;

		if (items[index])
			*items[index]=newItem;

		return items[index];
	}

	return NULL;
}

template <class Object>
Object *TArray<Object>::getItem(unsigned int index)
{
	return (index<Count)?items[index]:NULL;
}

template <class Object>
void TArray<Object>::Sort(TARRAYCMPFUNC user_cmp_func)
{
	if (Count)
	{
		if (!user_cmp_func)
			user_cmp_func=reinterpret_cast<TARRAYCMPFUNC>(CmpItems);

		far_qsort(reinterpret_cast<char*>(items),Count,sizeof(Object*),user_cmp_func);
	}
}

template <class Object>
bool TArray<Object>::Pack()
{
	bool was_changed=false;

	for (unsigned int index=1; index<Count; ++index)
	{
		if (*items[index-1]==*items[index])
		{
			deleteItem(index);
			was_changed=true;
			--Count;

			if (index<Count)
			{
				memmove(&items[index], &items[index+1], sizeof(Object*)*(Count-index));
				--index;
			}
		}
	}

	return was_changed;
}

template <class Object>
bool TArray<Object>::deleteItem(unsigned int index)
{
	if (index<Count)
	{
		delete items[index];
		items[index]=NULL;
		return true;
	}

	return false;
}

template <class Object>
bool TArray<Object>::setSize(unsigned int newSize)
{
	bool rc=false;

	if (newSize < Count)              // уменьшение размера
	{
		for (unsigned int i=newSize; i<Count; ++i)
		{
			delete items[i];
			items[i]=NULL;
		}

		Count=newSize;
		rc=true;
	}
	else if (newSize < internalCount) // увеличение, но в рамках имеющегося
	{
		for (unsigned int i=Count; i<newSize; ++i)
			items[i]=NULL;

		Count=newSize;
		rc=true;
	}
	else                              // увеличение размера
	{
		unsigned int Remainder=newSize%Delta;
		unsigned int newCount=Remainder?(newSize+Delta)-Remainder:
		                      newSize?newSize:Delta;
		Object **newItems=static_cast<Object**>(xf_malloc(newCount*sizeof(Object*)));

		if (newItems)
		{
			if (items)
			{
				memcpy(newItems,items,Count*sizeof(Object*));
				xf_free(items);
			}

			items=newItems;
			internalCount=newCount;

			for (unsigned int i=Count; i<newSize; ++i)
				items[i]=NULL;

			Count=newSize;
			rc=true;
		}
	}

	return rc;
}

template <class Object>
int __cdecl TArray<Object>::CmpItems(const Object **el1, const Object **el2)
{
	if (el1==el2)
		return 0;
	else if (**el1==**el2)
		return 0;
	else if (**el1<**el2)
		return -1;
	else
		return 1;
}

template <class Object>
TArray<Object>::TArray(const TArray<Object> &rhs):
		items(NULL), Count(0), internalCount(0)
{
	*this=rhs;
}

template <class Object>
TArray<Object>& TArray<Object>::operator=(const TArray<Object> &rhs)
{
	if (this == &rhs)
		return *this;

	setDelta(rhs.Delta);

	if (setSize(rhs.Count))
	{
		for (unsigned i=0; i<Count; ++i)
		{
			if (rhs.items[i])
			{
				if (!items[i])
					items[i]=new Object;

				if (items[i])
					*items[i]=*rhs.items[i];
				else
				{
					Free();
					break;
				}
			}
			else
			{
				delete items[i];
				items[i]=NULL;
			}
		}
	}

	return *this;
}

template <class Object>
void TArray<Object>::setDelta(unsigned int newDelta)
{
	if (newDelta<4)
		newDelta=4;

	Delta=newDelta;
}

template <class Object>
int TArray<Object>::getIndex(const Object &item, int start)
{
	int rc=-1;

	if (start==-1)
		start=0;

	for (unsigned int i=start; i<Count; ++i)
	{
		if (items[i] && item==*items[i])
		{
			rc=i;
			break;
		}
	}

	return rc;
}

template <class Object>
class TPointerArray
{
	private:
		unsigned int internalCount, Count, Delta;
		Object **items;

	private:
		bool setSize(unsigned int newSize)
		{
			bool rc=false;

			if (newSize < Count)              // уменьшение размера
			{
				Count=newSize;
				rc=true;
			}
			else if (newSize < internalCount) // увеличение, но в рамках имеющегося
			{
				for (unsigned int i=Count; i<newSize; i++)
					items[i]=NULL;

				Count=newSize;
				rc=true;
			}
			else                              // увеличение размера
			{
				unsigned int Remainder=newSize%Delta;
				unsigned int newCount=Remainder?(newSize+Delta)-Remainder:(newSize?newSize:Delta);
				Object **newItems=static_cast<Object**>(xf_realloc(items,newCount*sizeof(Object*)));

				if (newItems)
				{
					items=newItems;
					internalCount=newCount;

					for (unsigned int i=Count; i<newSize; i++)
						items[i]=NULL;

					Count=newSize;
					rc=true;
				}
			}

			return rc;
		}

	public:
		TPointerArray(unsigned int delta=1) { items=NULL; Count=internalCount=0; setDelta(delta); }
		~TPointerArray() { Free(); }

		void Free()
		{
			if (items)
			{
				for (unsigned int i=0; i<Count; ++i)
					delete items[i];

				xf_free(items);
				items=NULL;
			}

			Count=internalCount=0;
		}

		void setDelta(unsigned int newDelta) { if (newDelta<1) newDelta=1; Delta=newDelta; }

		Object *getItem(unsigned int index) { return (index<Count)?items[index]:NULL; }

		Object *lastItem() { return Count?items[Count-1]:NULL; }

		Object *addItem() { return insertItem(Count); }

		Object *insertItem(unsigned int index)
		{
			if (index>Count)
				return NULL;

			Object *newItem = new Object;

			if (newItem && setSize(Count+1))
			{
				for (unsigned int i=Count-1; i>index; i--)
					items[i]=items[i-1];

				items[index]= newItem;
				return items[index];
			}

			if (newItem)
				delete newItem;

			return NULL;
		}

		bool deleteItem(unsigned int index)
		{
			if (index<Count)
			{
				delete items[index];

				for (unsigned int i=index+1; i<Count; i++)
					items[i-1]=items[i];

				setSize(Count-1);
				return true;
			}

			return false;
		}

		bool swapItems(unsigned int index1, unsigned int index2)
		{
			if (index1<Count && index2<Count)
			{
				Object *temp = items[index1];
				items[index1] = items[index2];
				items[index2] = temp;
				return true;
			}

			return false;
		}

		unsigned int getCount() const { return Count; }
};
