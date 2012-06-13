#pragma once

/*
DList.hpp
Шаблон работы с двусвязным списком.
Type должен иметь конструктор по умолчанию, если используются методы
Push, Unshift, InsertBefore или InsertAfter с item должен так же
существовать оператор копирования:
      Type& operator=(const Type &)
*/
/*
Copyright © 2009 lort
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


class CDList
{
		size_t count;
	protected:
		struct Node
		{
			Node *next;
			Node *prev;
		};
		CDList();
		virtual ~CDList(){};
		void *CInsertBefore(void *b, void *item);
		void *CInsertAfter(void *a, void *item);
		void CMoveBefore(void *b, void *item);
		void CMoveAfter(void *a, void *item);
		void *CDelete(void *item);
		void CSwap(CDList &l);
		Node root;

		virtual Node *AllocNode(void *key)=0;
		virtual void DeleteNode(Node *node)=0;

	public:
		void Clear();
		size_t Count()const {return count;};
		bool Empty()const {return !count;};
};

template <typename Type>
class DList : public CDList
{
		struct TNode : Node
		{
			Type type;
			TNode(Type *t) {if (t) type=*t;}
		};
		Node *AllocNode(void *key) {return new TNode((Type*)key);}
		void DeleteNode(Node *node) {delete(TNode*)node;}

		Type *Node2Type(Node *node) {return node!=&root ? (Type*)((BYTE*)node+sizeof(Node)) : nullptr;}
		Node *Type2Node(const Type *item) {return item ? (Node*)((BYTE*)item-sizeof(Node)) : &root;}

	public:
		virtual ~DList() {Clear();};

		//создать новый элемент и поместить его в конец сприска
		//возвращает указатель на созданный элемент
		Type *Push(const Type *item=nullptr) {return (Type*)CInsertBefore(nullptr, (void*)item);}

		//создать новый элемент и поместить его в начало сприска
		//возвращает указатель на созданный элемент
		Type *Unshift(const Type *item=nullptr) {return (Type*)CInsertAfter(nullptr, (void*)item);}

		//создать новый элемент и поместить его в списке перед before
		//если before==nullptr элемент помещается в конец списка
		//возвращает указатель на созданный элемент
		Type *InsertBefore(const Type *before, const Type *item=nullptr) {return (Type*)CInsertBefore((void*)before, (void*)item);}

		//создать новый элемент и поместить его в списке после after
		//если after==nullptr элемент помещается в начало списка
		//возвращает указатель на созданный элемент
		Type *InsertAfter(const Type *after, const Type *item=nullptr) {return (Type*)CInsertAfter((void*)after, (void*)item);}

		void MoveBefore(const Type *before, const Type *item) {CMoveBefore((void*)before, (void*)item);}
		void MoveAfter(const Type *after, const Type *item) {CMoveAfter((void*)after, (void*)item);}

		//удалить элемент item из списка, возвращается указатель на предыдущий элемент,
		//если удалялся первый элемент возвращается nullptr
		Type *Delete(Type *item) {return (Type*)CDelete(item);}

		//возвращает первый элемент списка или nullptr если список пустой
		Type *First() {return Node2Type(root.next);}

		//возвращает последний элемент списка или nullptr если список пустой
		Type *Last() {return Node2Type(root.prev);}

		//возвращает элемент следующий за item или nullptr если item последний элемент.
		//Next(nullptr) возвращает первый элемент
		Type *Next(const Type *item) {return Node2Type(Type2Node(item)->next);}

		//возвращает элемент идущий в списке перед item или nullptr если item первый элемент.
		//Prev(nullptr) возвращает последний элемент
		Type *Prev(const Type *item) {return Node2Type(Type2Node(item)->prev);}

		//меняет местами содержимое списков
		void Swap(DList<Type> &l) {CSwap(l);}

		bool Contains(const Type& item)
		{
			for(const Type* i = First(); i; i = Next(i))
			{
				if(*i == item)
				{
					return true;
				}
			};
			return false;
		}
};
