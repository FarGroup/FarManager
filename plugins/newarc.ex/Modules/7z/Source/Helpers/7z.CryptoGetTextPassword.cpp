#include "7z.h"
#include <objbase.h>

CCryptoGetTextPassword::CCryptoGetTextPassword(
		SevenZipArchive* pArchive,
		int nType
		)
{
	m_pArchive = pArchive;

	m_nRefCount = 1;
	m_nType = nType;
}


ULONG __stdcall CCryptoGetTextPassword::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CCryptoGetTextPassword::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CCryptoGetTextPassword::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	/*if ( iid == IID_ICryptoGetTextPassword )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}*/

	return E_NOINTERFACE;
}

HRESULT __stdcall CCryptoGetTextPassword::CryptoGetTextPassword(BSTR *password)
{
	/*TCHAR szPassword[512];

	if ( m_pArchive->OnPasswordOperation(PASSWORD_FILE, szPassword, 512) ) //not that good
	{
		string strPassword = szPassword;
		*password = strPassword.ToBSTR();
	}*/

	return S_OK;
}

