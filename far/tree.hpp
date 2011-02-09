#pragma once

/*
tree.hpp

avl дерево

*/
/*
Copyright © 2011 Far Group
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

enum AVL_SKEW
{
	AVL_NONE,
	AVL_LEFT,
	AVL_RIGHT
};

enum AVL_RES
{
	AVL_ERROR=0,
	AVL_OK,
	AVL_BALANCE
};

template <class D> class Tree;

template <class D> class Node
{
	public:
		Node<D> *left, *right;
		D *data;
		AVL_SKEW skew;
		virtual void init(void);
	public:
		Node() { init(); }
		Node(D *value) { init(); data=value; }
		virtual ~Node() { delete data; }
		const D *get_data(void) const;
	friend class Tree<D>;
};

template <class D> void Node<D>::init(void)
{
	data=nullptr;
	left=right=nullptr;
	skew=AVL_NONE;
}

template <class D> const D *Node<D>::get_data(void) const
{
	return data;
}

template <class D> class Tree
{
	private:
		int node_count;
		void recurse_delete(Node<D> *node)
		{
			if(node->left) recurse_delete(node->left);
			if(node->right) recurse_delete(node->right);
			delete node;
		}
		AVL_RES internal_insert(Node<D> **node,D *data,D **result_data);
		AVL_RES internal_remove(Node<D> **node,D *data);
		AVL_RES leftgrown(Node<D> **node);
		AVL_RES rightgrown(Node<D> **node);
		void rotleft(Node<D> **node);
		void rotright(Node<D> **node);
		AVL_RES leftshrunk(Node<D> **node);
		AVL_RES rightshrunk(Node<D> **node);
		int findhighest(Node<D> *target,Node<D> **node,AVL_RES *res);
		int findlowest(Node<D> *target,Node<D> **node,AVL_RES *res);
		D *internal_query(Node<D> *node,D *data);
	protected:
		Node<D> *root;
	public:
		Tree(): node_count(0),root(nullptr) {}
		virtual ~Tree() {}
		int count(void) { return node_count; }
		virtual void clear(void);
		virtual long compare(Node<D> *first,D *second)=0;
		D *insert(D *data);
		void remove(D *data);
		D *query(D *data);
		Node<D> *get_top(void) { return root; }
};

template <class D> void Tree<D>::clear(void)
{
	if(root) recurse_delete(root);
	root=nullptr;
	node_count=0;
}

template <class D> D *Tree<D>::insert(D *data)
{
	D *result=nullptr;
	internal_insert(&root,data,&result);
	return result;
}

template <class D> void Tree<D>::remove(D *data)
{
	internal_remove(&root,data);
}

template <class D> D *Tree<D>::query(D *data)
{
	return internal_query(root,data);
}

template <class D> AVL_RES Tree<D>::internal_insert(Node<D> **node,D *data,D **result_data)
{
	AVL_RES result;

	if(!(*node))
	{
		if(!(*node=new Node<D>(data)))
		{
			return AVL_ERROR;
		}
		*result_data=(*node)->data;
		node_count++;
		return AVL_BALANCE;
	}
	long diff=compare(*node,data);
	if(diff<0)
	{
		if((result=internal_insert(&(*node)->left,data,result_data))==AVL_BALANCE)
		{
			return leftgrown(node);
		}
		return result;
	}
	if(diff>0)
	{
		if((result=internal_insert(&(*node)->right,data,result_data))==AVL_BALANCE)
		{
			return rightgrown(node);
		}
		return result;
	}
	*result_data=(*node)->data;
	delete data;
	return AVL_ERROR;
}

template <class D> AVL_RES Tree<D>::internal_remove(Node<D> **node,D *data)
{
	AVL_RES result=AVL_BALANCE;
	if(!(*node)) return AVL_ERROR;
	long diff=compare(*node,data);
	if(diff<0)
	{
		if((result=internal_remove(&(*node)->left,data))==AVL_BALANCE)
		{
			return leftshrunk(node);
		}
		return result;
	}
	if(diff>0)
	{
		if((result=internal_remove(&(*node)->right,data))==AVL_BALANCE)
		{
			return rightshrunk(node);
		}
		return result;
	}
	node_count--;
	if((*node)->left)
	{
		if(findhighest(*node,&((*node)->left),&result))
		{
			if(result==AVL_BALANCE)
			{
				result=leftshrunk(node);
			}
			return result;
		}
	}
	if((*node)->right)
	{
		if(findlowest(*node,&((*node)->right),&result))
		{
			if(result==AVL_BALANCE)
			{
				result=rightshrunk(node);
			}
			return result;
		}
	}
	delete *node;
	*node=nullptr;
	return AVL_BALANCE;
}

template <class D> void Tree<D>::rotleft(Node<D> **node)
{
	Node<D> *tmp=*node;
	*node=(*node)->right;
	tmp->right=(*node)->left;
	(*node)->left=tmp;
}

template <class D> void Tree<D>::rotright(Node<D> **node)
{
	Node<D> *tmp=*node;
	*node=(*node)->left;
	tmp->left=(*node)->right;
	(*node)->right=tmp;
}

template <class D> AVL_RES Tree<D>::leftgrown(Node<D> **node)
{
	switch((*node)->skew)
	{
		case AVL_LEFT:
			if((*node)->left->skew==AVL_LEFT)
			{
				(*node)->skew=(*node)->left->skew=AVL_NONE;
				rotright(node);
			}
			else
			{
				switch((*node)->left->right->skew)
				{
					case AVL_LEFT:
						(*node)->skew=AVL_RIGHT;
						(*node)->left->skew=AVL_NONE;
						break;
					case AVL_RIGHT:
						(*node)->skew=AVL_NONE;
						(*node)->left->skew=AVL_LEFT;
						break;

					default:
						(*node)->skew=AVL_NONE;
						(*node)->left->skew=AVL_NONE;
				}
				(*node)->left->right->skew=AVL_NONE;
				rotleft(&(*node)->left);
				rotright(node);
			}
			return AVL_OK;

		case AVL_RIGHT:
			(*node)->skew=AVL_NONE;
			return AVL_OK;

		default:
			(*node)->skew=AVL_LEFT;
			return AVL_BALANCE;
	}
}

template <class D> AVL_RES Tree<D>::rightgrown(Node<D> **node)
{
	switch((*node)->skew)
	{
		case AVL_LEFT:
			(*node)->skew=AVL_NONE;
			return AVL_OK;
		case AVL_RIGHT:
			if((*node)->right->skew==AVL_RIGHT)
			{
				(*node)->skew=(*node)->right->skew=AVL_NONE;
				rotleft(node);
			}
			else
			{
				switch((*node)->right->left->skew)
				{
					case AVL_RIGHT:
						(*node)->skew=AVL_LEFT;
						(*node)->right->skew=AVL_NONE;
						break;
					case AVL_LEFT:
						(*node)->skew=AVL_NONE;
						(*node)->right->skew=AVL_RIGHT;
						break;
					default:
						(*node)->skew=AVL_NONE;
						(*node)->right->skew=AVL_NONE;
				}
				(*node)->right->left->skew=AVL_NONE;
				rotright(&(*node)->right);
				rotleft(node);
			}
			return AVL_OK;
		default:
			(*node)->skew=AVL_RIGHT;
			return AVL_BALANCE;
	}
}

template <class D> AVL_RES Tree<D>::leftshrunk(Node<D> **node)
{
	switch((*node)->skew)
	{
		case AVL_LEFT:
			(*node)->skew=AVL_NONE;
			return AVL_BALANCE;

		case AVL_RIGHT:
			if((*node)->right->skew==AVL_RIGHT)
			{
				(*node)->skew=(*node)->right->skew=AVL_NONE;
				rotleft(node);
				return AVL_BALANCE;
			}
			else if((*node)->right->skew==AVL_NONE)
			{
				(*node)->skew=AVL_RIGHT;
				(*node)->right->skew=AVL_LEFT;
				rotleft(node);
				return AVL_OK;
			}
			else
			{
				switch((*node)->right->left->skew)
				{
					case AVL_LEFT:
						(*node)->skew=AVL_NONE;
						(*node)->right->skew=AVL_RIGHT;
						break;
					case AVL_RIGHT:
						(*node)->skew=AVL_LEFT;
						(*node)->right->skew=AVL_NONE;
						break;
					default:
						(*node)->skew=AVL_NONE;
						(*node)->right->skew=AVL_NONE;
				}
				(*node)->right->left->skew=AVL_NONE;
				rotright(&(*node)->right);
				rotleft(node);
				return AVL_BALANCE;
			}
		default:
			(*node)->skew=AVL_RIGHT;
			return AVL_OK;
	}
}

template <class D> AVL_RES Tree<D>::rightshrunk(Node<D> **node)
{
	switch((*node)->skew)
	{
		case AVL_RIGHT:
			(*node)->skew=AVL_NONE;
			return AVL_BALANCE;
		case AVL_LEFT:
			if((*node)->left->skew==AVL_LEFT)
			{
				(*node)->skew=(*node)->left->skew=AVL_NONE;
				rotright(node);
				return AVL_BALANCE;
			}
			else if((*node)->left->skew==AVL_NONE)
			{
				(*node)->skew=AVL_LEFT;
				(*node)->left->skew=AVL_RIGHT;
				rotright(node);
				return AVL_OK;
			}
			else
			{
				switch((*node)->left->right->skew)
				{
					case AVL_LEFT:
						(*node)->skew=AVL_RIGHT;
						(*node)->left->skew=AVL_NONE;
						break;
					case AVL_RIGHT:
						(*node)->skew=AVL_NONE;
						(*node)->left->skew=AVL_LEFT;
						break;
					default:
						(*node)->skew=AVL_NONE;
						(*node)->left->skew=AVL_NONE;
				}
				(*node)->left->right->skew=AVL_NONE;
				rotleft(&(*node)->left);
				rotright(node);
				return AVL_BALANCE;
			}
		default:
			(*node)->skew=AVL_LEFT;
			return AVL_OK;
	}
}

template <class D> int Tree<D>::findhighest(Node<D> *target,Node<D> **node,AVL_RES *res)
{
	Node<D> *tmp; D *tmp_data;
	*res=AVL_BALANCE;
	if(!(*node)) return 0;
	if((*node)->right)
	{
		if(!findhighest(target,&(*node)->right,res)) return 0;
		if(*res==AVL_BALANCE)
		{
			*res=rightshrunk(node);
		}
		return 1;
	}
	tmp_data=target->data;
	target->data=(*node)->data;
	(*node)->data=tmp_data;
	tmp=*node;
	*node=(*node)->left;
	delete tmp;
	return 1;
}

template <class D> int Tree<D>::findlowest(Node<D> *target,Node<D> **node,AVL_RES *res)
{
	Node<D> *tmp; D *tmp_data;
	*res=AVL_BALANCE;
	if(!(*node)) return 0;
	if((*node)->left)
	{
		if(!findlowest(target,&(*node)->left,res)) return 0;
		if(*res==AVL_BALANCE)
		{
			*res=leftshrunk(node);
		}
		return 1;
	}
	tmp_data=target->data;
	target->data=(*node)->data;
	(*node)->data=tmp_data;
	tmp=*node;
	*node=(*node)->right;
	delete tmp;
	return 1;
}

template <class D> D *Tree<D>::internal_query(Node<D> *node,D *data)
{
	if(!node) return nullptr;
	long diff=compare(node,data);
	if(diff<0) return internal_query(node->left,data);
	if(diff>0) return internal_query(node->right,data);
	return node->data;
}
