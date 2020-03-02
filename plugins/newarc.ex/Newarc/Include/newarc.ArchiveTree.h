#pragma once
#include "newarc.h"


class ArchiveTree;
typedef ArchiveTree ArchiveTreeNode;

class StringCompareCase 
{
public:

	bool operator() (string strStr1, string strStr2) const
	{
		return _tcscmp(strStr1, strStr2) < 0;
	}
};

class StringCompareNoCase 
{
public:

	bool operator() (string strStr1, string strStr2) const
	{
		return FSF.LStricmp(strStr1, strStr2) < 0;
	}
};


typedef std::multimap<string, ArchiveTreeNode*, StringCompareCase> ArchiveTreeNodes;
typedef std::multimap<string, ArchiveTreeNode*, StringCompareCase>::iterator ArchiveTreeNodesIterator;



class ArchiveTree {
public:

	ArchiveTreeNodes children;

	bool bDummy;

	ArchiveTree* parent;

	ArchiveItem* item;
	string strFileName;

	int level;

public:


	ArchiveTree(int level)
	{
		this->level = level;
		item = nullptr;
	}

	ArchiveTree()
	{
		parent = nullptr;
		strFileName = _T("");

		this->level = 0;

		item = nullptr;

	}


	~ArchiveTree()
	{
		for (ArchiveTreeNodesIterator itr = children.begin(); itr != children.end(); ++itr)
			delete itr->second;
	}

	// /aaa/bbb/
	// /aaa/bbb.txt
	// /aaa/
	// /aaa.txt

	bool AddItem(const TCHAR* lpName, ArchiveItem* pItem)
	{
		TCHAR* lpNameCopy = StrDuplicate(lpName);

		ArchiveTreeNode* pPath = this;

		//пиздец, извините, срочно убрать
		while ( (lpNameCopy[StrLength(lpNameCopy)-1] == _T('/')) || //бля, индусский код
				(lpNameCopy[StrLength(lpNameCopy)-1] == _T('\\')) )
				lpNameCopy[StrLength(lpNameCopy)-1] = 0;

		TCHAR* lpPath = StrDuplicate(lpNameCopy);

		const TCHAR* lpNameOnly = FSF.PointToName(lpNameCopy);

		//if ( lpPath != lpNameOnly )
			//*(lpPath+1) = 0;

		CutToSlash(lpPath);

		if ( _tcschr(lpPath, _T('\\')) ||
			 _tcschr(lpPath, _T('/')) )
		{
		

		TCHAR* lpToken = _tcstok(lpPath, _T("\\/"));

		while ( lpToken != nullptr )
		{
			if ( !_tcscmp(lpToken, _T("..")) )
			{
				pPath = pPath->parent;

				if ( pPath == nullptr )
					return false;
			}
			else

			if ( !_tcscmp(lpToken, _T(".")) )
				pPath = this;
			else
			{
				ArchiveTreeNodesIterator iterator = pPath->children.find(lpToken);

				if ( iterator != pPath->children.end() )
					pPath = iterator->second;
				else //dummy
				{
					ArchiveTreeNode* pNode = new ArchiveTreeNode(pPath->level+1);

					pNode->parent = pPath;

					pNode->bDummy = true;
					pNode->strFileName = lpToken;

					pPath->children.insert(std::pair<string, ArchiveTreeNode*>(lpToken, pNode));

					pPath = pNode;
				}
			}

			lpToken = _tcstok(NULL, _T("\\/"));
		}
		}

		/////////////
		ArchiveTreeNodesIterator iterator = pPath->children.find(lpNameOnly);

		ArchiveTreeNode* pNode = nullptr;
		bool bNew = false;

		if ( (iterator != pPath->children.end()) && iterator->second->bDummy )
			pNode = iterator->second;
		else
		{
			pNode = new ArchiveTreeNode(pPath->level+1);
			bNew = true;
		}

		pNode->parent = pPath;
		pNode->bDummy = false;

		pNode->item = pItem;
		pNode->strFileName = lpNameOnly;

		if ( bNew )
			pPath->children.insert(std::pair<string, ArchiveTreeNode*>(lpNameOnly, pNode));
	
		StrFree(lpNameCopy);

		return true;
	}

	void GetPath(string& strPath)
	{
		strPath = strFileName;

		if ( level > 1 ) //первый после рута
		{
			string strPart;

			parent->GetPath(strPart);
			strPart += _T("\\");

			strPath = strPart+strPath;
		}
	}

	ArchiveTreeNode* GetNode(const TCHAR* lpName)
	{
		ArchiveTreeNode* pPath = this;

		TCHAR* lpNameCopy = StrDuplicate(lpName);
		TCHAR* lpToken = nullptr;

		lpToken = _tcstok(lpNameCopy, _T("\\/"));
		
		while ( lpToken != nullptr )
		{
			if ( !_tcscmp(lpToken, _T("..")) )
				pPath = pPath->parent;
			else

			if ( !_tcscmp(lpToken, _T(".")) )
				pPath = this;
			else
			{
				ArchiveTreeNodesIterator iterator = pPath->children.find(lpToken);
			
				if ( iterator == pPath->children.end() )
					return nullptr;
				else
					pPath = iterator->second;
			}

			lpToken = _tcstok(NULL, _T("\\/"));
		}

		StrFree(lpNameCopy);

		return pPath;
	}

	bool IsDummy()
	{
		return bDummy;
	}

	bool IsDirectory()
	{
		return bDummy || OptionIsOn(item->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	}

	const TCHAR* GetFileName()
	{
		return strFileName.GetString();
	}

	const ArchiveItem* GetOriginalItem()
	{
		return item;
	}
};


