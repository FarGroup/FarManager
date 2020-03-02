#include "7z.h"
#include <objbase.h>


CArchiveOpenVolumeCallback::CArchiveOpenVolumeCallback(SevenZipArchive* pArchive)
{
	m_nRefCount = 1;

	m_pArchive = pArchive;
	m_pVolumeFile = pArchive->GetFile();

	m_uOpenedVolumes = 0;
}

CArchiveOpenVolumeCallback::~CArchiveOpenVolumeCallback()
{
}


ULONG __stdcall CArchiveOpenVolumeCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenVolumeCallback::Release()
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


HRESULT __stdcall CArchiveOpenVolumeCallback::GetProperty(PROPID propID, PROPVARIANT* value)
{
	WIN32_FIND_DATA fData;
	CPropVariant val;

	if ( m_pVolumeFile->GetFindData(fData) )
	{
		switch (propID) {

		case kpidName:
			val = fData.cFileName;
			break;

		case kpidSize:
			val = ((unsigned __int64)fData.nFileSizeHigh << 32)+fData.nFileSizeLow;
			break;

		case kpidCTime:
			val = fData.ftCreationTime;
			break;

		case kpidATime:
			val = fData.ftLastAccessTime;
			break;

		case kpidMTime:
			val = fData.ftLastWriteTime;
			break;

		case kpidAttrib:
			val = (unsigned int)fData.dwFileAttributes;
			break;
		}

		val.Detach(value);
	}

	return S_OK;
}

HRESULT __stdcall CArchiveOpenVolumeCallback::GetStream(const wchar_t *name, IInStream **inStream)
{
//	if ( m_uOpenedVolumes == m_pArchive->GetNumberOfVolumes() )
//		return S_FALSE;

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

	//bool bResult = false;

	//CInFile* file = nullptr;

	CInFile* file = new CInFile(strFullName);

	bool bResult = file->Open();

/*	while ( true )
	{
		file = new CInFile(strFullName);

		if ( file->Open() )
		{
			m_uOpenedVolumes++;

			bResult = true;
			break;
		}
		else
		{
			TCHAR cBuffer[512];
			
			if ( m_pArchive->OnNeedVolume(strFullName, 512, (TCHAR*)&cBuffer) )
			{
				strFullName = (const TCHAR*)&cBuffer;
				delete file;
			}
			else
			{
				bResult = false;
				break;
			}
		}
	} */

	*inStream = file;
	m_pVolumeFile = file;

	return bResult?S_OK:S_FALSE;
}



