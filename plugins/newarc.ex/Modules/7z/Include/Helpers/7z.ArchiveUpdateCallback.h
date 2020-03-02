#pragma once
#include "7z.h"

struct ArchiveUpdateItem {
	unsigned int index;
	bool bNewFile;
	const ArchiveItem *pItem;
};

class CArchiveUpdateCallback : public IArchiveUpdateCallback2, public ICryptoGetTextPassword2 {

private:

	int m_nRefCount;
	const Array<ArchiveUpdateItem*>& m_indicies;

	string m_strSourceDiskPath;
	string m_strPathInArchive;

	string m_strPassword;

	SevenZipArchive* m_pArchive;

	CCryptoGetTextPassword* m_pGetTextPassword;

	unsigned int m_uSuccessCount;
	unsigned int m_uItemsNumber;

	unsigned __int64 m_uProcessedBytesTotal;
	unsigned __int64 m_uTotalBytes;
	unsigned __int64 m_uTotalBytesFile;
	unsigned __int64 m_uProcessedBytesFile;

public:

	CArchiveUpdateCallback(
			SevenZipArchive* pArchive,
			const TCHAR* lpPassword,
			const Array<ArchiveUpdateItem*>& indicies,  ///это какое-то УГ
			const TCHAR* lpSourceDiskPath,
			const TCHAR* lpPathInArchive
			);

	~CArchiveUpdateCallback();

	//IUnknown

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	//IProgress

	virtual HRESULT __stdcall SetTotal(unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted(const unsigned __int64* completeValue);

	//IArchiveUpdateCallback

	virtual HRESULT __stdcall GetUpdateItemInfo(
			unsigned int index,
			int* newData, // 1 - new data, 0 - old data
			int* newProperties, // 1 - new properties, 0 - old properties
			unsigned int* indexInArchive // -1 if there is no in archive, or if doesn't matter
			);

	virtual HRESULT __stdcall GetProperty(unsigned int index, PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream(unsigned int index, ISequentialInStream** inStream);
	virtual HRESULT __stdcall SetOperationResult(int operationResult);

	//IArchiveUpdateCallback2
	virtual HRESULT __stdcall GetVolumeSize(unsigned int index, unsigned __int64* size);
	virtual HRESULT __stdcall GetVolumeStream(unsigned int index, ISequentialOutStream** volumeStream);

	//ICryptoGetTextPassword2

	virtual HRESULT __stdcall CryptoGetTextPassword2(int* passwordIsDefined, BSTR* password);

	int GetResult();
};
