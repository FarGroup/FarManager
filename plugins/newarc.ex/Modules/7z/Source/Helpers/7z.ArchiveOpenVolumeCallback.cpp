#include "7z.h"
#include <objbase.h>


CArchiveOpenVolumeCallback::CArchiveOpenVolumeCallback(SevenZipArchive* pArchive)
{
	m_nRefCount = 1;

	m_pArchive = pArchive;
	m_pVolumeFile = NULL;
}

CArchiveOpenVolumeCallback::~CArchiveOpenVolumeCallback ()
{
}


ULONG __stdcall CArchiveOpenVolumeCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenVolumeCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveOpenVolumeCallback::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveOpenVolumeCallback )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}


HRESULT __stdcall CArchiveOpenVolumeCallback::GetProperty(PROPID propID, PROPVARIANT *value)
{
	if ( propID == kpidName )
	{
		string strNameOnly;

		if ( m_pVolumeFile )
			strNameOnly = FSF.PointToName(m_pVolumeFile->GetName());
		else
			strNameOnly = FSF.PointToName(m_pArchive->GetFileName());

		value->vt = VT_BSTR;
		value->bstrVal = strNameOnly.ToBSTR();
	}

	if ( propID == kpidSize )
	{
		/*value->vt = VT_UI8;

		if ( m_pVolumeFile )
			value->uhVal.QuadPart = m_pVolumeFile->GetSize();
		else
			value->uhVal.QuadPart = m_pHandle->pFile->GetSize(); */

		__debug(_T("BUGBUG!!!"));
	}

	return S_OK;
}

HRESULT __stdcall CArchiveOpenVolumeCallback::GetStream(const wchar_t *name, IInStream **inStream)
{
	string strFullName;
	string strFileName;

#ifdef UNICODE
	strFileName = name;
#else
	strFileName.SetData(name, CP_OEMCP);
#endif

	strFullName = m_pArchive->GetFileName();
	CutTo(strFullName, _T('\\'), false);
	strFullName += strFileName;

	CInFile *file = new CInFile (strFullName);

	bool bResult = file->Open();

	*inStream = file;
	m_pVolumeFile = file;

	return bResult?S_OK:S_FALSE;
}



