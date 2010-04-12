#ifndef _TSTACK_HPP_
#define _TSTACK_HPP_

/*  TStack.hpp
    Шаблон работы со стеком (LIFO)

    TStack<Object> Stack;
    // Object должен иметь конструктор по умолчанию и оператор
    // const Object& operator=(const Object &)
*/

template <class Object>
class TStack
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
		TStack():Size(0), Top(NULL) {};
		~TStack() { Free(); }

	public:
		// вернуть количество элементов на стеке
		DWORD size() const { return Size; }

		// возвращает TRUE, если список пуст
		bool empty() const { return !Size; }

		// взять элемент со стека
		// при удаче вернется адрес Destination, иначе - NULL
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

			return NULL;
		}

		// взять элемент со стека без изменения стека
		// при удаче вернется адрес Destination, иначе - NULL
		Object *Peek(/*Object &Destination*/)
		{
			if (Top)
			{
				//Destination=Top->Item;
				//return &Destination;
				return &Top->Item;
			}

			return NULL;
		}

		// положить элемент на стек
		// при удаче вернется адрес элемента на стеке, иначе - NULL
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

			return NULL;
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

	private:
		TStack& operator=(const TStack& rhs); /* чтобы не генерировалось */
		TStack(const TStack& rhs);            /* по умолчанию            */
};

#endif // _TSTACK_HPP_
