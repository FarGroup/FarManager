/*
DList.cpp

двусвязный список
*/
/*
Copyright © 2009 lort
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

#include "headers.hpp"
#pragma hdrstop

#include "DList.hpp"

CDList::CDList():
	count(0)
{
	root.next=&root;
	root.prev=&root;
}

void CDList::Clear()
{
	Node *f=root.next;

	while (f!=&root)
	{
		Node *f1=f;
		f=f->next;
		DeleteNode(f1);
	}

	count=0;
	root.next=&root;
	root.prev=&root;
}
void CDList::CSwap(CDList &l)
{
	Node *pr=root.prev;
	root.next->prev=&l.root;
	pr->next=&l.root;
	pr=l.root.prev;
	l.root.next->prev=&root;
	pr->next=&root;
	size_t tcount=count;
	Node troot=root;
	count=l.count;
	root=l.root;
	l.count=tcount;
	l.root=troot;
}
void *CDList::CInsertBefore(void *b, void *item)
{
	Node *Before=b ? (Node*)((BYTE*)b-sizeof(Node)) : &root;
	Node *node=AllocNode(item);
	node->prev=Before->prev;
	node->next=Before;
	Before->prev->next=node;
	Before->prev=node;
	++count;
	return ((BYTE*)node)+sizeof(Node);
}
void *CDList::CInsertAfter(void *a, void *item)
{
	Node *After=a ? (Node*)((BYTE*)a-sizeof(Node)) : &root;
	return CInsertBefore((BYTE*)After->next+sizeof(Node), item);
}
void CDList::CMoveBefore(void *b, void *item)
{
	if (!item) return;
	if (item == b) return;
	Node *Before=b ? (Node*)((BYTE*)b-sizeof(Node)) : &root;
	Node *node=(Node*)((BYTE*)item-sizeof(Node));
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev=Before->prev;
	node->next=Before;
	Before->prev->next=node;
	Before->prev=node;
}
void CDList::CMoveAfter(void *a, void *item)
{
	if (item == a) return;
	Node *After=a ? (Node*)((BYTE*)a-sizeof(Node)) : &root;
	CMoveBefore((BYTE*)After->next+sizeof(Node), item);
}
void *CDList::CDelete(void *item)
{
	Node *node=(Node*)((BYTE*)item-sizeof(Node));
	Node *pr=node->prev;
	Node *nx=node->next;
	pr->next=nx;
	nx->prev=pr;
	--count;
	DeleteNode(node);
	return pr==&root ? nullptr : ((BYTE*)pr)+sizeof(Node);
}
