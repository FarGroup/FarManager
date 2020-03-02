#include "7z.h"
#include <objbase.h>

CArchiveOpenCallback::CArchiveOpenCallback(SevenZipArchive* pArchive)
{
	m_nRefCount = 1;

	m_pArchive = pArchive;

	m_bProgressMessage = false;
	m_dwStartTime = GetTickCount();

	m_hScreen = Info.SaveScreen(0, 0, -1, -1);

	m_pGetTextPassword = NULL;
	m_pArchiveOpenVolumeCallback = NULL;
}

CArchiveOpenCallback::~CArchiveOpenCallback()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release();

	if ( m_pArchiveOpenVolumeCallback )
		m_pArchiveOpenVolumeCallback->Release();

	Info.RestoreScreen(m_hScreen);
}


ULONG __stdcall CArchiveOpenCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveOpenCallback::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveOpenCallback )
	{
		*ppvObject = this;
		AddRef();

		return S_OK;
	}

	if ( iid == IID_IArchiveOpenVolumeCallback )
	{
		if ( !m_pArchiveOpenVolumeCallback )
			m_pArchiveOpenVolumeCallback = new CArchiveOpenVolumeCallback(m_pArchive);

		m_pArchiveOpenVolumeCallback->AddRef();

		*ppvObject = m_pArchiveOpenVolumeCallback;

		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		if ( !m_pGetTextPassword )
			m_pGetTextPassword = new CCryptoGetTextPassword(m_pArchive, TYPE_LISTING);

		m_pGetTextPassword->AddRef();

		*ppvObject = m_pGetTextPassword;

		return S_OK;
	}


	return E_NOINTERFACE;
}



HRESULT __stdcall CArchiveOpenCallback::SetTotal(const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}


bool CheckForEsc ()
{
	bool EC = false;

	/*INPUT_RECORD rec;
	DWORD ReadCount;

	while (true)
	{
		PeekConsoleInput (GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);

		if ( ReadCount==0 )
			break;

		ReadConsoleInput (GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);

		if ( rec.EventType==KEY_EVENT )
		{
			if ( (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) &&
				 rec.Event.KeyEvent.bKeyDown )
				EC = true;
		}
	}*/

	return EC;
}


HRESULT __stdcall CArchiveOpenCallback::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
	if ( CheckForEsc() )
		return E_FAIL;

	if ( files && !(*files & 0x1f)  && (GetTickCount ()-m_dwStartTime > 500) )
	{
		//это надо нафиг устранить, здесь должен быть callback
		FarMessage message(m_bProgressMessage?FMSG_KEEPBACKGROUND:0);

		string strFileCount;

		strFileCount.Format(_T("%I64u файлов"), *files);

		message.Add(_T("Подождите"));
		message.Add(_T("Чтение архива [7z.all]"));
		message.Add(m_pArchive->GetFileName());
		message.Add(strFileCount);

		message.Run();

		m_bProgressMessage = true;
	}

	return S_OK;
}

