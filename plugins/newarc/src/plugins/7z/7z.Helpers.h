#include "7z.h"

class CInFile : //public IUnknown, 
				public IInStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;

public:

	CInFile ();
	~CInFile ();

	bool Open (const char *lpFileName);

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

class CArchiveExtractCallback : public IArchiveExtractCallback {

public:

	int m_nRefCount;

	SevenZipArchive *m_pArchive;
	ArchiveItem *m_pItems;
	int m_nItemsNumber;

	char *m_lpDestPath;
	char *m_lpCurrentFolder;

	unsigned __int64 m_nLastProcessed;

public:

	CArchiveExtractCallback (SevenZipArchive *pArchive, ArchiveItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	~CArchiveExtractCallback ();

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	virtual HRESULT __stdcall SetTotal (unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted (const unsigned __int64* completeValue);

	virtual HRESULT __stdcall GetStream (unsigned int index, ISequentialOutStream **outStream, int askExtractMode);
  // GetStream OUT: S_OK - OK, S_FALSE - skeep this file
	virtual HRESULT __stdcall PrepareOperation (int askExtractMode);
	virtual HRESULT __stdcall SetOperationResult (int resultEOperationResult);
};

class COutFile : //public IUnknown, 
				public ISequentialOutStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;

public:

	COutFile ();
	~COutFile ();

	bool Open (const char *lpFileName);

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	virtual HRESULT __stdcall Write (const void *data, unsigned int size, unsigned int* processedSize);
};

