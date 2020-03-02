#pragma once
#include "7z.h"

class CArchiveOpenVolumeCallback : public IArchiveOpenVolumeCallback {
private:

	int m_nRefCount;

	SevenZipArchive* m_pArchive;
	CInFile* m_pVolumeFile;

	unsigned int m_uOpenedVolumes;

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

