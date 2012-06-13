#pragma once

/*  TStack.hpp
    Шаблон работы со стеком (LIFO)

    TStack<Object> Stack;
    // Object должен иметь конструктор по умолчанию и оператор
    // Object& operator=(const Object &)
*/
/*
Copyright © 2009 Far Group
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

template <class Object>
class TStack:NonCopyable
{
	private:
		struct OneItem
		{
			Object  Item;
			OneItem *Next;
		};

		struct OneItem *Top;

		DWORD Size;

	public:
		TStack():Top(nullptr), Size(0) {};
		~TStack() { Free(); }

	public:
		// вернуть количество элементов на стеке
		DWORD size() const { return Size; }

		// возвращает TRUE, если список пуст
		bool empty() const { return !Size; }

		// взять элемент со стека
		// при удаче вернется адрес Destination, иначе - nullptr
		Object *Pop(Object &Destination)
		{
			if (Top)
			{
				--Size;
				Destination=Top->Item;
				struct OneItem *Temp=Top->Next;
				delete Top;
				Top=Temp;
				return &Destination;
			}

			return nullptr;
		}

		// взять элемент со стека без изменения стека
		// при удаче вернется адрес Destination, иначе - nullptr
		Object *Peek(/*Object &Destination*/)
		{
			if (Top)
			{
				//Destination=Top->Item;
				//return &Destination;
				return &Top->Item;
			}

			return nullptr;
		}

		// положить элемент на стек
		// при удаче вернется адрес элемента на стеке, иначе - nullptr
		Object *Push(const Object &Source)
		{
			struct OneItem *Temp=new OneItem;

			if (Temp)
			{
				Temp->Next=Top;
				Temp->Item=Source;
				Top=Temp;
				++Size;
				return &Top->Item;
			}

			return nullptr;
		}

		// два верхних элемента в вершине стека обменять местами
		void Swap()
		{
			if (Top && Top->Next)
			{
				struct OneItem *Temp=Top->Next;
				Top->Next=Temp->Next;
				Temp->Next=Top;
				Top=Temp;
			}
		}

		// очистить стек
		void Free()
		{
			while (Top)
			{
				struct OneItem *Temp=Top->Next;
				delete Top;
				Top=Temp;
			}

			Size=0;
		}
};
