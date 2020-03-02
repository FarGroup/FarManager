#pragma once
#include "FarPluginBase.hpp"

class FarMessage 
{

private:

	TCHAR *m_lpTitle;
	int m_buttons;
	DWORD m_dwFlags;

	ObjectArray<TCHAR*> m_items;

public:

	FarMessage(DWORD Flags)
	{
		m_buttons = 0;
		m_dwFlags = Flags;
	}

	void Add(const TCHAR *lpStr)
	{
		size_t length = _tcslen(lpStr);
		TCHAR *lpCopy = new TCHAR[length+1];

		_tcscpy(lpCopy, lpStr);

		m_items.add(lpCopy);
	}

	void AddButton(const TCHAR *lpStr)
	{
		Add(lpStr);
		m_buttons++;
	}

	int Run()
	{
		return Info.Message(
				Info.ModuleNumber,
				m_dwFlags,
				NULL,
				m_items.data(),
				m_items.count(),
				m_buttons
				);
	}

	void Done()
	{
		m_items.free();
	}

	~FarMessage()
	{
		Done();
	}
};
