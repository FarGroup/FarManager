#include <windows.h>
#include "strclass.hpp"

TStrList::TStrList()
{
	List=NULL;
	Count=0;
}

TStrList::~TStrList()
{
	DeleteList();
}

void TStrList::DeleteList()
{
	int i;

	if (Count)
	{
		for (i=Count-1; i>=0; i--) delete[] List[i];

		delete[] List;
		List=NULL;
		Count=0;
	}
}

void __fastcall TStrList::Clear()
{
	DeleteList();
}

BOOL __fastcall TStrList::Insert(const TCHAR *String,int Index)
{
	int i;
	int len=lstrlen(String);
	TCHAR **tmpList=new TCHAR*[Count+1];

	if (tmpList==NULL)
		return FALSE;

	for (i=0; i<Count; i++)
		tmpList[i]=List[i];

	if ((Index<Count) && (Index>=0))
		for (i=Count-1; i>=Index; i--) tmpList[i+1]=tmpList[i];

	tmpList[Index]=new TCHAR[len+1];

	if (String)
		lstrcpy(tmpList[Index],String);
	else
		tmpList[Index][0]=0;

	Count++;
	delete[] List;
	List=tmpList;
	return TRUE;
}

BOOL __fastcall TStrList::Add(const TCHAR *String)
{
	return(Insert(String,Count));
}

BOOL __fastcall TStrList::Delete(int Index)
{
	int i;
	TCHAR **tmpList;

	if (Count)
	{
		if (Count-1)
		{
			tmpList=new TCHAR*[Count-1];

			if ((Index<Count) && (Index>=0))
			{
				delete[] List[Index];

				for (i=0; i<Index; i++)
					tmpList[i]=List[i];

				for (i=Index+1; i<Count; i++)
					tmpList[i-1]=List[i];

				delete[] List;
				List=tmpList;
				Count--;
			}
		}
		else
			DeleteList();

		return TRUE;
	}

	return FALSE;
}

BOOL __fastcall TStrList::SetText(const TCHAR *String,int Index)
{
	int len=lstrlen(String);

	if (Count)
		if ((Index<Count) && (Index>=0))
		{
			delete[] List[Index];
			List[Index]=new TCHAR[len+1];

			if (String)
				lstrcpyn(List[Index],String,len+1);
			else
				List[Index][0]=0;

			return TRUE;
		}

	return FALSE;
}

TCHAR *__fastcall TStrList::GetText(TCHAR *String,int Index)
{
	if (Count)
		if ((Index<Count) && (Index>=0))
			return lstrcpy(String,List[Index]);

	String[0]=0;
	return String;
}

TCHAR *__fastcall TStrList::GetText(int Index)
{
	if (Count)
		if ((Index<Count) && (Index>=0))
			return List[Index];

	return NULL;
}

void _fastcall TStrList::Sort(int Low,int Up)
{
	int i,j;
	TCHAR *x;
	TCHAR *y;

	if (Count)
	{
		i=Low; j=Up;
		x=List[(Low+Up)/2];

		do
		{
			if (*List[i])
			{
				while (CmpStr(List[i],x)<0)
					i++;
			}
			else
				i++;

			if (*List[j])
			{
				while (CmpStr(x,List[j])<0)
					j--;
			}
			else
				j--;

			if (i<=j)
			{
				y=List[i];
				List[i]=List[j];
				List[j]=y;
				i++;
				j--;
			}
		}
		while (i<j);

		if (Low<j) Sort(Low,j);

		if (i<Up) Sort(i,Up);
	}
}


TStrList &TStrList::operator=(TStrList &lst)
{
	int i;

	if (this!=&lst)
	{
		DeleteList();

		for (i=0; i<lst.GetCount(); i++)
		{
			Add(lst.GetText(i));
		}
	}

	return *this;
}
