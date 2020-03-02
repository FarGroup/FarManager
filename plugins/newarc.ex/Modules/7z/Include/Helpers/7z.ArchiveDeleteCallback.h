#pragma once
#include "7z.h"


class CArchiveDeleteCallback : public IArchiveUpdateCallback, public ICryptoGetTextPassword2 {

private:

	int m_nRefCount;
	const Array<ArchiveUpdateItem*>& m_indicies;

	SevenZipArchive* m_pArchive;

	string m_strPassword;
	CCryptoGetTextPassword* m_pGetTextPassword;

	unsigned __int64 m_uProcessedBytesTotal;
	unsigned __int64 m_uTotalBytes;

public:

	CArchiveDeleteCallback(
			SevenZipArchive* pArchive,
			const Array<ArchiveUpdateItem*>& indicies
			);

	~CArchiveDeleteCallback();

	//IUnknown

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	//IProgress

	virtual HRESULT __stdcall SetTotal(unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted(const unsigned __int64* completeValue);

	//IArchiveDeleteCallback

	virtual HRESULT __stdcall GetUpdateItemInfo(
			unsigned int index,
			int* newData, // 1 - new data, 0 - old data
			int* newProperties, // 1 - new properties, 0 - old properties
			unsigned int* indexInArchive // -1 if there is no in archive, or if doesn't matter
			);

	virtual HRESULT __stdcall GetProperty(unsigned int index, PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream(unsigned int index, ISequentialInStream** inStream);
	virtual HRESULT __stdcall SetOperationResult(int operationResult);

	//ICryptoGetTextPassword2
	virtual HRESULT __stdcall CryptoGetTextPassword2(int* passwordIsDefined, BSTR* password);

	int GetResult();
};
