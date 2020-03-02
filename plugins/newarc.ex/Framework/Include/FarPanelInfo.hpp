#include "FarPluginBase.hpp"

#ifdef UNICODE
	#define FINDDATA_GET_NAME(fdata) (fdata.lpwszFileName)
	#define FINDDATA_GET_NAME_PTR(fdata) (fdata->lpwszFileName)
	#define FINDDATA_GET_SIZE(fdata) (fdata.nFileSize)
	#define FINDDATA_GET_SIZE_PTR(fdata) (fdata->nFileSize)
#else
	#define FINDDATA_GET_NAME(fdata) ((const char*)fdata.cFileName)
	#define FINDDATA_GET_NAME_PTR(fdata) ((const char*)fdata->cFileName)
	#define FINDDATA_GET_SIZE(fdata) (((unsigned __int64)fdata.nFileSizeHigh << 32)+(unsigned __int64)fdata.nFileSizeLow)
	#define FINDDATA_GET_SIZE_PTR(fdata) (((unsigned __int64)fdata->nFileSizeHigh << 32)+(unsigned __int64)fdata->nFileSizeLow)
#endif



class FarPanelInfo {
private:

	bool m_bActivePanel;
	HANDLE m_hPanel;
	bool m_bUserPanel;

	//CACHE
	TCHAR* m_lpCurrentDirectory; 
	PanelInfo *m_short;

#ifndef UNICODE
	PanelInfo *m_info;
#endif


public:

	FarPanelInfo(bool bActivePanel = true)
	{
		m_bUserPanel = false;

		if ( bActivePanel )
			m_hPanel = INVALID_HANDLE_VALUE; //PANEL_ACTIVE
		else
			m_hPanel = (HANDLE)-2; //PANEL_PASSIVE

		m_lpCurrentDirectory = NULL;
		m_short = NULL;

#ifndef UNICODE
		m_info = NULL;
#endif
	}

	FarPanelInfo(HANDLE hPanel)
	{
		m_hPanel = hPanel;
		m_bUserPanel = true;

		m_lpCurrentDirectory = NULL;
		m_short = NULL;

#ifndef UNICODE
		m_info = NULL;
#endif
	}

	void Reset()
	{
		if ( m_short )
			delete m_short;

		if ( m_lpCurrentDirectory )
			delete [] m_lpCurrentDirectory;

		m_short = NULL;
		m_lpCurrentDirectory = NULL;


#ifndef UNICODE
		if ( m_info )
			delete m_info;

		m_info = NULL;
#endif
	}

	void Redraw()
	{
#ifdef UNICODE
		Info.Control(m_hPanel, FCTL_REDRAWPANEL, 0, NULL);
#else
		if ( m_hPanel == (HANDLE)-2 )
			Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWANOTHERPANEL, NULL);
		else
			Info.Control(m_hPanel, FCTL_REDRAWPANEL, NULL);
#endif										
	}

	void Update(bool bKeepSelection = true)
	{
#ifdef UNICODE
		Info.Control(m_hPanel, FCTL_UPDATEPANEL, 0, (LONG_PTR)bKeepSelection);
#else
		if ( m_hPanel == (HANDLE)-2 )
			Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEANOTHERPANEL, (void*)bKeepSelection);
		else
			Info.Control(m_hPanel, FCTL_UPDATEPANEL, (void*)bKeepSelection);
#endif										
	}

	void GetPanelItem(int nNumber, PluginPanelItem* pItem)
	{
#ifdef UNICODE
		DWORD dwSize = Info.Control(m_hPanel, FCTL_GETPANELITEM, nNumber, 0);
		PluginPanelItem* pBuffer = (PluginPanelItem*)malloc(dwSize);
		
		Info.Control(m_hPanel, FCTL_GETPANELITEM, nNumber, (LONG_PTR)pBuffer);

		memcpy(pItem, pBuffer, sizeof(PluginPanelItem));

		pItem->FindData.lpwszFileName = StrDuplicate(pBuffer->FindData.lpwszFileName);
		pItem->FindData.lpwszAlternateFileName = StrDuplicate(pBuffer->FindData.lpwszAlternateFileName);

		free(pBuffer);
#else
		UpdatePanelInfo();
		memcpy(pItem, &m_info->PanelItems[nNumber], sizeof(PluginPanelItem));
#endif
	}

	void GetSelectedItem(int nNumber, PluginPanelItem* pItem)
	{
#ifdef UNICODE
		DWORD dwSize = Info.Control(m_hPanel, FCTL_GETSELECTEDPANELITEM, nNumber, 0);
		PluginPanelItem* pBuffer = (PluginPanelItem*)malloc(dwSize);
		
		Info.Control(m_hPanel, FCTL_GETSELECTEDPANELITEM, nNumber, (LONG_PTR)pBuffer);

		memcpy(pItem, pBuffer, sizeof(PluginPanelItem));

		pItem->FindData.lpwszFileName = StrDuplicate(pBuffer->FindData.lpwszFileName);
		pItem->FindData.lpwszAlternateFileName = StrDuplicate(pBuffer->FindData.lpwszAlternateFileName);

		free(pBuffer);
#else
		UpdatePanelInfo();
		memcpy(pItem, &m_info->SelectedItems[nNumber], sizeof(PluginPanelItem));
#endif
	}

	void GetCurrentItem(PluginPanelItem* pItem)
	{
#ifdef UNICODE
		DWORD dwSize = Info.Control(m_hPanel, FCTL_GETCURRENTPANELITEM, 0, 0);
		PluginPanelItem* pBuffer = (PluginPanelItem*)malloc(dwSize);
		
		Info.Control(m_hPanel, FCTL_GETCURRENTPANELITEM, 0, (LONG_PTR)pBuffer);

		memcpy(pItem, pBuffer, sizeof(PluginPanelItem));

		pItem->FindData.lpwszFileName = StrDuplicate(pBuffer->FindData.lpwszFileName);
		pItem->FindData.lpwszAlternateFileName = StrDuplicate(pBuffer->FindData.lpwszAlternateFileName);

		free(pBuffer);
#else
		UpdatePanelInfo();
		memcpy(pItem, &m_info->PanelItems[m_info->CurrentItem], sizeof(PluginPanelItem));
#endif
	}

	void FreePanelItem(PluginPanelItem* pItem)
	{
#ifdef UNICODE
		StrFree((void*)pItem->FindData.lpwszFileName);
		StrFree((void*)pItem->FindData.lpwszAlternateFileName);
#else
#endif
	}

	int GetSelectedItemsCount()
	{
		UpdatePanelShortInfo();
		return m_short->SelectedItemsNumber;
	}

	int GetItemsCount()
	{
		UpdatePanelShortInfo();
		return m_short->ItemsNumber;
	}

	int GetType()
	{
		UpdatePanelShortInfo();
		return m_short->PanelType;
	}		

	const RECT& GetRect()
	{
		UpdatePanelShortInfo();
		return m_short->PanelRect;
	}

	bool IsVisible()
	{
		UpdatePanelShortInfo();
		return (bool)m_short->Visible;
	}

	bool IsPlugin()
	{
		UpdatePanelShortInfo();
		return (bool)m_short->Plugin;
	}

	int GetFlags()
	{
		UpdatePanelShortInfo();
		return m_short->Flags;
	}

	const TCHAR* GetCurrentDirectory()
	{
		if ( m_lpCurrentDirectory )
			return m_lpCurrentDirectory;

#ifdef UNICODE
		int nLength = Info.Control(m_hPanel, FCTL_GETPANELDIR, 0, NULL);
		m_lpCurrentDirectory = new TCHAR[nLength+1];

		Info.Control(m_hPanel, FCTL_GETPANELDIR, nLength, (LONG_PTR)m_lpCurrentDirectory);

		return m_lpCurrentDirectory;
#else
		UpdatePanelShortInfo();

		m_lpCurrentDirectory = new TCHAR[_tcslen(m_short->CurDir)+1];
		_tcscpy(m_lpCurrentDirectory, m_short->CurDir);

		return m_lpCurrentDirectory;
#endif
	}                                         

	~FarPanelInfo()
	{
		Reset();
	}

private:


#ifndef UNICODE
	void UpdatePanelInfo()
	{
		if ( m_info == NULL )
		{
			m_info = new PanelInfo;

			if ( m_hPanel == (HANDLE)-2 )
				Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, m_info);
			else
				Info.Control(m_hPanel, FCTL_GETPANELINFO, m_info);
		}
	}
#endif

	void UpdatePanelShortInfo()
	{
		if ( m_short == NULL )
		{
			m_short = new PanelInfo;

#ifdef UNICODE
			Info.Control(m_hPanel, FCTL_GETPANELINFO, 0, (LONG_PTR)m_short);
#else
			if ( m_hPanel == (HANDLE)-2 )
				Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELSHORTINFO, m_short);
			else
				Info.Control(m_hPanel, FCTL_GETPANELSHORTINFO, m_short);
#endif
		}
	}

};

