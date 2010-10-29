#pragma once
#include "7z.h"

struct ArchiveItemEx {
	unsigned int nIndex;
	const ArchiveItem *pItem;
};


class CArchiveOpenVolumeCallback : public IArchiveOpenVolumeCallback {
private:

	int m_nRefCount;

	SevenZipArchive* m_pArchive;
	CInFile* m_pVolumeFile;

public:

	CArchiveOpenVolumeCallback(SevenZipArchive* pArchive);
	~CArchiveOpenVolumeCallback();

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IArchiveOpenVolumeCallback

	virtual HRESULT __stdcall GetProperty(PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream(const wchar_t *name, IInStream **inStream);

};

#define TYPE_FILE		1
#define TYPE_LISTING	2


class CCryptoGetTextPassword : public ICryptoGetTextPassword {
private:

	int m_nRefCount;
	SevenZipArchive* m_pArchive;

	int m_nType;

public:

	CCryptoGetTextPassword (SevenZipArchive* pArchive, int nType);

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// ICryptoGetTextPassword

	virtual HRESULT __stdcall CryptoGetTextPassword(BSTR* password);
};


class CArchiveExtractCallback : public IArchiveExtractCallback {

public:

	int m_nRefCount;

	SevenZipArchive* m_pArchive;

	ArchiveItemEx* m_pItems;
	int m_nItemsNumber;

	string m_strDestDiskPath;
	string m_strPathInArchive;

	unsigned __int64 m_uProcessedBytes;

	CCryptoGetTextPassword* m_pGetTextPassword;

	bool m_bUserAbort;
	int m_nSuccessCount;
	bool m_bExtractMode;

public:

	CArchiveExtractCallback(
			SevenZipArchive* pArchive, 
			ArchiveItemEx *pItems, 
			int nItemsNumber, 
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


class CArchiveOpenCallback : public IArchiveOpenCallback/*, public IArchiveOpenVolumeCallback*/ {

private:

	int m_nRefCount;

	SevenZipArchive* m_pArchive;

	CCryptoGetTextPassword* m_pGetTextPassword;
	CArchiveOpenVolumeCallback* m_pArchiveOpenVolumeCallback;

	bool m_bProgressMessage;
	DWORD m_dwStartTime;

	HANDLE m_hScreen;

public:

	CArchiveOpenCallback(SevenZipArchive* pArchive);
	~CArchiveOpenCallback();

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IArchiveOpenCallback

	virtual HRESULT __stdcall SetTotal(const unsigned __int64* files, const unsigned __int64* bytes);
	virtual HRESULT __stdcall SetCompleted(const unsigned __int64* files, const unsigned __int64* bytes);
};


struct ArchiveUpdateItem {
	unsigned int index;
	bool bNewFile;
	const ArchiveItem *pItem;
};

class CArchiveUpdateCallback : public IArchiveUpdateCallback2, public ICryptoGetTextPassword2 {

private:

	int m_nRefCount;
	const Array<ArchiveUpdateItem*>* m_indicies;

	string m_strSourceDiskPath;
	string m_strPathInArchive;

	string m_strPassword;

	SevenZipArchive* m_pArchive;

	unsigned __int64 m_uProcessedBytes;

	CCryptoGetTextPassword* m_pGetTextPassword;


public:

	CArchiveUpdateCallback (
			SevenZipArchive* pArchive,
			const TCHAR* lpPassword,
			const Array<ArchiveUpdateItem*>* indicies,  ///это какое-то УГ
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
};
