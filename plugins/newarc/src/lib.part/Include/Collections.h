#pragma once

#pragma warning(disable:4800) // force value to bool
#pragma warning(disable:4018) // signed/unsigned mismatch

#include <Rtl.Base.h>
#include <Rtl.Memory.h>
#include <FarPluginBase.h>

#define INVALID_INDEX -1

typedef int (__cdecl *SORTFUNC) (const void *, const void *, void *);

template <typename Type> class ViewCollection {

//private:
public:

	Type *m_Data;
	int m_Count;
	int m_RealCount;
	int m_Delta;

public:

	virtual void Create (int Delta);

	virtual void Add (Type Data);
	virtual void Remove (Type Data);
	virtual void RemoveRef (Type Data);

	virtual Type At (int Index);
	virtual int IndexOf (Type Data);

	virtual Type operator [] (int Index)  { return At(Index); }

	virtual int GetCount () { return m_RealCount; }

//__declspec (property (get=GetCount)) int Count;

	virtual void Sort (void *SortFunc, void *Param = NULL);
	virtual void FreeItem (Type Data) {}

	virtual void Free ();

	virtual ~ViewCollection() {}

private:

	void SetLimit  (int Limit);
};


template <typename Type>
class Collection : public ViewCollection<Type> {
public:
	virtual void FreeItem (Type Data) {	delete Data; };
};

template <typename Type>
class PointerCollection : public ViewCollection <Type> {
public:
	virtual void FreeItem (Type Data) { free(Data); };
};


template <typename Type>
void ViewCollection<Type>::Create (int Delta)
{
	Delta ? m_Delta = Delta : m_Delta = 1;

	m_Count = 0;
	m_RealCount = 0;
	m_Data = NULL;

	SetLimit (m_Delta);
}



template <typename Type>
void ViewCollection<Type>::Free ()
{
	for (int i = 0; i < m_RealCount; i++)
		FreeItem (m_Data[i]);

	m_Count = 0;
	m_RealCount = 0;

	free (m_Data);

	m_Data = NULL;

}


template <typename Type>
void ViewCollection<Type>::Add (Type Data)
{
	if ( m_RealCount == m_Count )
		SetLimit (m_Count+m_Delta);

	m_Data[m_RealCount] = Data;
	m_RealCount++;
}

template <typename Type>
void ViewCollection<Type>::Remove (Type Data)
{
	int Index = IndexOf (Data);

	if ( Index != INVALID_INDEX )
	{
		FreeItem (m_Data[Index]);

		for (int i = Index; i < m_RealCount-1; i++)
			m_Data[i] = m_Data[i+1];

		m_RealCount--;

		if ( (m_Count-m_RealCount) == m_Delta )
			SetLimit (m_Count-m_Delta);
	}
}


template <typename Type>
void ViewCollection<Type>::RemoveRef (Type Data)
{
	int Index = IndexOf (Data);

	if ( Index != INVALID_INDEX )
	{
//		FreeItem (m_Data[Index]);

		for (int i = Index; i < m_RealCount-1; i++)
			m_Data[i] = m_Data[i+1];

		m_RealCount--;

		if ( (m_Count-m_RealCount) == m_Delta )
			SetLimit (m_Count-m_Delta);
	}
}


template <typename Type>
int ViewCollection<Type>::IndexOf (Type Data)
{
	for (int i = 0; i < m_RealCount; i++)
		if (m_Data[i] == Data)
			return i;

	return INVALID_INDEX;
}

template <typename Type>
Type ViewCollection<Type>::At (int Index)
{
	if (Index < m_RealCount)
		return m_Data[Index];

	return NULL;
}

template <typename Type> void
ViewCollection<Type>::SetLimit (int Limit)
{
	m_Data = (Type*)realloc (m_Data, Limit*4);

	if (Limit > m_Count)
		for (int i = m_Count; i < Limit; i++)
			m_Data[i] = NULL;

	m_Count = Limit;
}


struct InternalParam {
	PVOID    Param;
	SORTFUNC fcmp;
};

inline int __cdecl InternalSortFunction (
		void **p1,
		void **p2,
		void *Param
		)
{
	InternalParam *IP = (InternalParam*)Param;
	return IP->fcmp (*p1, *p2, IP->Param);
}

template <typename Type>
void ViewCollection<Type>::Sort (void *SortFunc, void *Param)
{
	InternalParam IP;

	IP.Param = Param;
	IP.fcmp  = (SORTFUNC)SortFunc;

//#ifdef __FAR__
	FSF.qsortex (m_Data, m_RealCount, 4, (SORTFUNC)InternalSortFunction, &IP);
//#else
//	qsortex (m_Data, m_RealCount, 4, (SORTFUNC)InternalSortFunction, &IP);
//#endif
}
