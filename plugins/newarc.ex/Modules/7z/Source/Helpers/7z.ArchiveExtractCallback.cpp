#include "7z.h"
#include <objbase.h>

CArchiveExtractCallback::CArchiveExtractCallback(
		SevenZipArchive* pArchive,
		const ArchiveItem* pItems,
		unsigned int uItemsNumber,
		const TCHAR *lpDestDiskPath,
		const TCHAR *lpPathInArchive
		)
{
	m_pArchive = pArchive;

	m_nRefCount = 1;

	m_pItems = pItems;
	m_uItemsNumber = uItemsNumber;

	m_strDestDiskPath = lpDestDiskPath;
	m_strPathInArchive = lpPathInArchive;

	m_pGetTextPassword = NULL;

	m_bUserAbort = false;
	m_uSuccessCount = 0;
	m_bExtractMode = false;
}


CArchiveExtractCallback::~CArchiveExtractCallback()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release();
}

ULONG __stdcall CArchiveExtractCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveExtractCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveExtractCallback::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveExtractCallback )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		if ( !m_pGetTextPassword )
			m_pGetTextPassword = new CCryptoGetTextPassword(m_pArchive, TYPE_FILE);

		m_pGetTextPassword->AddRef ();
		*ppvObject = m_pGetTextPassword; //????

		return S_OK;
	}


	return E_NOINTERFACE;
}


extern unsigned __int64 VariantToInt64 (CPropVariant *value);

HRESULT __stdcall CArchiveExtractCallback::SetTotal(unsigned __int64 total)
{
	m_uProcessedBytesTotal = 0;
	m_uTotalBytes = total;

	m_pArchive->OnStartOperation(OPERATION_EXTRACT, m_uTotalBytes, m_uItemsNumber);

	return S_OK;
}

HRESULT CArchiveExtractCallback::SetCompleted(const unsigned __int64* completeValue)
{
	m_uProcessedBytesFile += *completeValue-m_uProcessedBytesTotal;
	m_uProcessedBytesTotal = *completeValue;

	if ( !m_pArchive->OnProcessData(
			m_uProcessedBytesFile,
			m_uTotalBytesFile,
			m_uProcessedBytesTotal,
			m_uTotalBytes
			) )
		return E_ABORT;

	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::GetStream(
		unsigned int index,
		ISequentialOutStream** outStream,
		int askExtractMode
		)

{
	*outStream = nullptr;

	ArchiveItem item;
	memset(&item, 0, sizeof(ArchiveItem));

	if ( !m_pArchive->GetArchiveItem(index, &item) )
		return S_OK;

	string strFileName = item.lpFileName;
	string strFullName = m_strDestDiskPath;

	if ( !FSF.LStrnicmp (strFileName, m_strPathInArchive, m_strPathInArchive.GetLength()) )
		strFullName += (const TCHAR*)strFileName+m_strPathInArchive.GetLength(); //FIX ASAP!!!
	else
		strFullName += strFileName;

	m_uTotalBytesFile = item.nFileSize;
	m_uProcessedBytesFile = 0;

	if ( askExtractMode == NArchive::NExtract::NAskMode::kSkip ) 
	{
		m_pArchive->OnEnterStage(STAGE_SKIPPING);
		m_pArchive->OnProcessFile(&item, nullptr);
	}

	if ( askExtractMode == NArchive::NExtract::NAskMode::kExtract )
	{
		m_pArchive->OnEnterStage(STAGE_EXTRACTING);
		int nOverwrite = m_pArchive->OnProcessFile(&item, strFullName);

		if ( nOverwrite == PROCESS_CANCEL )
		{
			m_bUserAbort = true;
			*outStream = NULL;
		}
		else
		{
			if ( nOverwrite == PROCESS_SKIP )
			{
				m_uSuccessCount++;
				*outStream = NULL;
				return S_OK;
			}
			else
			{

				if ( OptionIsOn(item.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
					apiCreateDirectoryEx(strFullName); //, dwFileAttributes);
				else
				{
					apiCreateDirectoryForFile(strFullName);

					COutFile *file = new COutFile(strFullName);

					if ( file->Open() )
					{
						file->SetAttributes(item.dwFileAttributes);
						file->SetTime(&item.ftCreationTime, &item.ftLastAccessTime, &item.ftLastWriteTime);
						*outStream = file;
					}
					else
						delete file;
				}
			}
		}
	}

	m_pArchive->FreeArchiveItem(&item);

	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::PrepareOperation(int askExtractMode)
{
	m_bExtractMode = (askExtractMode == NArchive::NExtract::NAskMode::kExtract);
	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::SetOperationResult(int resultEOperationResult)
{
	switch( resultEOperationResult )
	{
		case NArchive::NExtract::NOperationResult::kOK:
			
			if ( m_bExtractMode )
				m_uSuccessCount++;

			return S_OK;

		case NArchive::NExtract::NOperationResult::kDataError:

			//remove to callback if possible
			m_pArchive->OnReportError(nullptr, EXTRACT_ERROR_DATA);
			return S_OK; //??

		case NArchive::NExtract::NOperationResult::kCRCError:

			//remove to callback if possible

			m_pArchive->OnPasswordOperation(PASSWORD_RESET, NULL, 0);
			m_pArchive->OnReportError(nullptr, EXTRACT_ERROR_CRC);
			
			return S_OK; //??

		default:
			m_pArchive->OnReportError(nullptr, EXTRACT_ERROR_UNKNOWN);
			return S_OK;
	}

	return S_OK;
}

int CArchiveExtractCallback::GetResult()
{
	if ( m_bUserAbort )
		return RESULT_CANCEL;

	if ( m_uSuccessCount > m_uItemsNumber )
		__debug(_T("LOGIC ERROR, PLEASE REPORT"));

	if ( m_uSuccessCount == 0 )
		return RESULT_ERROR;

	if ( m_uSuccessCount < m_uItemsNumber )
		return RESULT_PARTIAL;

	return RESULT_SUCCESS;
}

