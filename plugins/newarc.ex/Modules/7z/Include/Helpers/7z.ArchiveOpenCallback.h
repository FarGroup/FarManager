#pragma once
#include "7z.h"

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


