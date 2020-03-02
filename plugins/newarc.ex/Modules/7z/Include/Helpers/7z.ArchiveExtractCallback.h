#pragma once
#include "7z.h"

class CArchiveExtractCallback : public IArchiveExtractCallback {

public:

	int m_nRefCount;

	SevenZipArchive* m_pArchive;

	const ArchiveItem* m_pItems;
	unsigned int m_uItemsNumber;

	string m_strDestDiskPath;
	string m_strPathInArchive;

	unsigned __int64 m_uProcessedBytesTotal;
	unsigned __int64 m_uTotalBytes;
	unsigned __int64 m_uTotalBytesFile;
	unsigned __int64 m_uProcessedBytesFile;

	CCryptoGetTextPassword* m_pGetTextPassword;

	bool m_bUserAbort;
	unsigned int m_uSuccessCount;
	bool m_bExtractMode;

public:

	CArchiveExtractCallback(
			SevenZipArchive* pArchive,
			const ArchiveItem* pItems,
			unsigned int uItemsNumber,
			const TCHAR* lpDestDiskPath,
			const TCHAR* lpPathInArchive
			);

	~CArchiveExtractCallback();

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	//IProgress

	virtual HRESULT __stdcall SetTotal(unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted(const unsigned __int64* completeValue);

	//IArchiveExtractCallback

	virtual HRESULT __stdcall GetStream(unsigned int index, ISequentialOutStream **outStream, int askExtractMode);
  // GetStream OUT: S_OK - OK, S_FALSE - skeep this file
	virtual HRESULT __stdcall PrepareOperation(int askExtractMode);
	virtual HRESULT __stdcall SetOperationResult(int resultEOperationResult);

	int GetResult();
};

