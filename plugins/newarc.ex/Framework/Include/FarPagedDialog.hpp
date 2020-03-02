#pragma once
#include "FarDialogBase.hpp"

struct PageInfo {
	int StartItem;
	int nItems;
};


class FarPagedDialog : public FarDialog {

private:

	array<PageInfo*> *m_Pages;

public:

	int m_CurrentPage; 

	FarPagedDialog (int X1, int Y1, int X2, int Y2);
	~FarPagedDialog ();

	void NewPage (int nItems);

	virtual int ShowEx (void *DlgProc = NULL, void *Param = NULL, const TCHAR *lpHelpTopic = NULL);

friend class FarPagedDialogHandler;
friend class FarDialogHandler;
};


class FarPagedDialogHandler : public FarDialogHandler {

public:

	FarPagedDialog *m_Owner;

public:

	void PreparePage (bool bDirect);

	void SetPage (int nPage)
	{
		m_Owner->m_CurrentPage = nPage;
		PreparePage (true);
	}

};

