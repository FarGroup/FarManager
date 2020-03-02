#include "7z.h"

class CInFile : //public IUnknown,
				public IInStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;
	char *m_lpFileName;

public:

	CInFile (const char *lpFileName);
	~CInFile ();

	bool Open ();
	bool Create ();
	void Close ();

	unsigned __int64 GetSize ();
	const char *GetName ();

	virtual HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	virtual HRESULT __stdcall Read (void *data, unsigned int size, unsigned int *processedSize);
	virtual HRESULT __stdcall Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
};


struct ArchiveItem {
	unsigned int nIndex;
	PluginPanelItem *pItem;
};


class CArchiveOpenVolumeCallback : public IArchiveOpenVolumeCallback {
private:

	int m_nRefCount;
	SevenZipArchive *m_pArchive;

	CInFile *m_pVolumeFile;

public:

	CArchiveOpenVolumeCallback (SevenZipArchive *pArchive);
	~CArchiveOpenVolumeCallback();


	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	// IArchiveOpenVolumeCallback

	virtual HRESULT __stdcall GetProperty (PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream (const wchar_t *name, IInStream **inStream);

};

#define TYPE_FILE		1
#define TYPE_LISTING	2


class CCryptoGetTextPassword : public ICryptoGetTextPassword {
private:

	int m_nRefCount;
	SevenZipArchive *m_pArchive;

	int m_nType;

public:

	CCryptoGetTextPassword (SevenZipArchive *pArchive, int nType);

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	// ICryptoGetTextPassword

	virtual HRESULT __stdcall CryptoGetTextPassword (BSTR *password);
};


class CArchiveExtractCallback : public IArchiveExtractCallback {

public:

	int m_nRefCount;

	SevenZipArchive *m_pArchive;
	ArchiveItem *m_pItems;
	int m_nItemsNumber;

	char *m_lpDestPath;
	char *m_lpCurrentFolder;

	unsigned __int64 m_nLastProcessed;

	CCryptoGetTextPassword *m_pGetTextPassword;

	int m_nState; //0 - skip, 1 - extract

public:

	CArchiveExtractCallback (SevenZipArchive *pArchive, ArchiveItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	~CArchiveExtractCallback ();

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	//IProgress

	virtual HRESULT __stdcall SetTotal (unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted (const unsigned __int64* completeValue);

	//IArchiveExtractCallback

	virtual HRESULT __stdcall GetStream (unsigned int index, ISequentialOutStream **outStream, int askExtractMode);
  // GetStream OUT: S_OK - OK, S_FALSE - skeep this file
	virtual HRESULT __stdcall PrepareOperation (int askExtractMode);
	virtual HRESULT __stdcall SetOperationResult (int resultEOperationResult);

};


class CArchiveOpenCallback : public IArchiveOpenCallback/*, public IArchiveOpenVolumeCallback*/ {

private:
	int m_nRefCount;

	CCryptoGetTextPassword *m_pGetTextPassword;
	CArchiveOpenVolumeCallback *m_pArchiveOpenVolumeCallback;

	bool m_bProgressMessage;
	DWORD m_dwStartTime;

	SevenZipArchive *m_pArchive;

	HANDLE m_hScreen;

public:

	CArchiveOpenCallback (SevenZipArchive *pArchive);
	~CArchiveOpenCallback ();


	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	// IArchiveOpenCallback

	virtual HRESULT __stdcall SetTotal (const UInt64 *files, const UInt64 *bytes);
	virtual HRESULT __stdcall SetCompleted (const UInt64 *files, const UInt64 *bytes);


/*	// IArchiveOpenVolumeCallback

	virtual HRESULT __stdcall GetProperty (PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream (const wchar_t *name, IInStream **inStream);*/

};



class COutFile : public IOutStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;
	char *m_lpFileName;

public:

	COutFile (const char *lpFileName);
	~COutFile ();

	bool Open ();
	void Close ();

	bool SetTime (const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime);
	bool SetAttributes (DWORD dwFileAttributes);

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	//ISequentialOutStream

	virtual HRESULT __stdcall Write (const void *data, unsigned int size, unsigned int* processedSize);

	//IOutStream

	virtual HRESULT __stdcall Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
	virtual HRESULT __stdcall SetSize (__int64 newSize);
};


struct ArchiveUpdateItem {
	unsigned int index;
	bool bNewFile;
	PluginPanelItem *pItem;
};

class CArchiveUpdateCallback : public IArchiveUpdateCallback2, public ICryptoGetTextPassword2 {


private:

	int m_nRefCount;
	pointer_array<ArchiveUpdateItem*> *m_indicies;

	const char *m_lpSourcePath;
	const char *m_lpCurrentPath;

	SevenZipArchive *m_pArchive;

	unsigned __int64 m_nLastProcessed;

public:

	CArchiveUpdateCallback (
			SevenZipArchive *pArchive, 
			pointer_array<ArchiveUpdateItem*> *indicies,
			const char *lpSourcePath,
			const char *lpCurrentPath
			);

	~CArchiveUpdateCallback();

	//IUnknown

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	//IProgress

	virtual HRESULT __stdcall SetTotal (unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted (const unsigned __int64* completeValue);

	//IArchiveUpdateCallback

	virtual HRESULT __stdcall GetUpdateItemInfo (
			unsigned int index,
			int *newData, // 1 - new data, 0 - old data
			int *newProperties, // 1 - new properties, 0 - old properties
			unsigned int *indexInArchive // -1 if there is no in archive, or if doesn't matter
			);

	virtual HRESULT __stdcall GetProperty (unsigned int index, PROPID propID, PROPVARIANT *value);
	virtual HRESULT __stdcall GetStream (unsigned int index, ISequentialInStream **inStream);
	virtual HRESULT __stdcall SetOperationResult (int operationResult);

	//IArchiveUpdateCallback2
	virtual HRESULT __stdcall GetVolumeSize (unsigned int index, unsigned __int64 *size);
	virtual HRESULT __stdcall GetVolumeStream (unsigned int index, ISequentialOutStream **volumeStream);

	//ICryptoGetTextPassword2

	virtual HRESULT __stdcall CryptoGetTextPassword2 (int *passwordIsDefined, BSTR *password);
};
