#pragma once
#include "7z.h"

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


