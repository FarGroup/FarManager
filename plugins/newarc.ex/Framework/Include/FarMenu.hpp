#pragma once
#include "FarDialogBase.hpp"

class FarMenu 
{

private:

	Array<FarMenuItemEx> m_items;

	struct UserDataStruct {
		void *UserData;
		TCHAR *Text;
	};


	TCHAR *m_lpTitle;

public:

	FarMenu(const TCHAR *lpTitle)
	{
		m_lpTitle = StrDuplicate(lpTitle);
	}

	void Add(const TCHAR *lpStr, DWORD dwFlags = 0, void *UserData = NULL)
	{
		FarMenuItemEx *item = m_items.add();

		if ( item )
		{
			size_t length = _tcslen(lpStr);

			item->Flags = dwFlags;
			item->Flags |= MIF_USETEXTPTR;

			UserDataStruct *uds = new UserDataStruct;

			uds->Text = new TCHAR[length+1];
			_tcscpy(uds->Text, lpStr);
			uds->UserData = UserData;

#ifdef UNICODE
			item->Text = uds->Text;
#else
			item->Text.TextPtr = uds->Text;
#endif

			item->UserData = (DWORD_PTR)uds;
		}
	}

	int Run()
	{
		return Info.Menu(
				Info.ModuleNumber,
				-1,
				-1,
				0,
				FMENU_WRAPMODE|FMENU_USEEXT,
				m_lpTitle,
				NULL,
				NULL,
				NULL,
				NULL,
				(const FarMenuItem*)m_items.data(),
				m_items.count()
				);
	}

	void* GetData(int item)
	{
		if ( item < m_items.count() )
		{
			UserDataStruct *uds = (UserDataStruct*)m_items[item].UserData;
			return uds->UserData;
		}

		return NULL;
	}

	void Done()
	{
		for (int i = 0; i < m_items.count(); i++)
		{
			if ( m_items[i].UserData )
			{
				UserDataStruct *uds = (UserDataStruct*)m_items[i].UserData;
				delete [] uds->Text;
			}
		}

		m_items.free();
	}

	~FarMenu()
	{
		Done();

		if ( m_lpTitle )
			::StrFree(m_lpTitle);
	}
};

