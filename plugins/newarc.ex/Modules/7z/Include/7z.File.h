#pragma once
#include "7z.h"

class CInFile : public IInStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;

	string m_strFileName;

public:

	CInFile(const TCHAR *lpFileName);
	~CInFile();

	bool Open();
	bool Create();
	void Close();

	bool GetFindData(WIN32_FIND_DATA& fData);
	const TCHAR *GetName();

	virtual HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	virtual HRESULT __stdcall Read(void *data, unsigned int size, unsigned int *processedSize);
	virtual HRESULT __stdcall Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
};


class COutFile : public IOutStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;
	string m_strFileName;

public:

	COutFile(const TCHAR *lpFileName);
	~COutFile();

	bool Open();
	void Close();

	bool SetTime(const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime);
	bool SetAttributes(DWORD dwFileAttributes);

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	//ISequentialOutStream

	virtual HRESULT __stdcall Write(const void *data, unsigned int size, unsigned int* processedSize);

	//IOutStream

	virtual HRESULT __stdcall Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
	virtual HRESULT __stdcall SetSize(__int64 newSize);
};


struct VolumeInfo {
	COutFile* stream;
	string strFileName;
	unsigned __int64 pos;
	unsigned __int64 realSize;
};

class CVolumeOutFile : public IOutStream {
private:

	int _streamIndex; // required stream
	unsigned __int64 _offsetPos; // offset from start of _streamIndex index
	unsigned __int64 _absPos;
	unsigned __int64 _length;
    
	ObjectArray<VolumeInfo*> m_streams;

	unsigned __int64 m_uVolumeSize;
	string m_strFileName;

	int m_nRefCount;

public:

	CVolumeOutFile(const TCHAR* lpFileName, unsigned __int64 uVolumeSize);
	~CVolumeOutFile();

	bool Open() { return true; }  
	void Close();

	virtual HRESULT __stdcall QueryInterface(const IID &iid, void** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	//ISequentialOutStream

	virtual HRESULT __stdcall Write(const void *data, unsigned int size, unsigned int* processedSize);

	//IOutStream

	virtual HRESULT __stdcall Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
	virtual HRESULT __stdcall SetSize(__int64 newSize);
};
