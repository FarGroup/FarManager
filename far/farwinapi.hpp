#pragma once

/*
farwinapi.hpp

Враперы вокруг некоторых WinAPI функций
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


class api
{
public:
	static const size_t NT_MAX_PATH = 32768;

struct FAR_FIND_DATA
{
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	FILETIME ftChangeTime;
	unsigned __int64 nFileSize;
	unsigned __int64 nAllocationSize;
	unsigned __int64 FileId;
	string strFileName;
	string strAlternateFileName;
	DWORD dwFileAttributes;
	DWORD dwReserved0;

	FAR_FIND_DATA()
	{
		Clear();
	}

	void Clear()
	{
		ClearStruct(ftCreationTime);
		ClearStruct(ftLastAccessTime);
		ClearStruct(ftLastWriteTime);
		ClearStruct(ftChangeTime);
		nFileSize=0;
		nAllocationSize=0;
		FileId = 0;
		strFileName.clear();
		strAlternateFileName.clear();
		dwFileAttributes=0;
		dwReserved0=0;
	}
};

class FindFile:NonCopyable, public enumerator<FAR_FIND_DATA>
{
public:
	FindFile(const string& Object, bool ScanSymLink = true);
	~FindFile();

private:
	virtual bool get(size_t index, FAR_FIND_DATA& value) override;

	string m_Object;
	HANDLE m_Handle;
	bool m_ScanSymLink;
};

class File:NonCopyable
{
public:
	File();
	~File();
	bool Open(const string& Object, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes=0, File* TemplateFile=nullptr, bool ForceElevation=false);
	bool Read(LPVOID Buffer, DWORD NumberOfBytesToRead, DWORD& NumberOfBytesRead, LPOVERLAPPED Overlapped = nullptr);
	bool Write(LPCVOID Buffer, DWORD NumberOfBytesToWrite, DWORD& NumberOfBytesWritten, LPOVERLAPPED Overlapped = nullptr);
	bool Read(LPVOID Buffer, size_t Nr, size_t& NumberOfBytesRead);
	bool Write(LPCVOID Buffer, size_t Nw, size_t& NumberOfBytesWritten);
	bool SetPointer(INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod);
	INT64 GetPointer(){return Pointer;}
	bool SetEnd();
	bool GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	bool SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);
	bool GetSize(UINT64& Size);
	bool FlushBuffers();
	bool GetInformation(BY_HANDLE_FILE_INFORMATION& info);
	bool IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped = nullptr);
	bool GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed);
	bool NtQueryDirectoryFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, LPCWSTR FileName, bool RestartScan, NTSTATUS* Status = nullptr);
	bool NtQueryInformationFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status = nullptr);
	bool Close();
	bool Eof();
	bool Opened() const {return Handle != INVALID_HANDLE_VALUE;}

private:
	HANDLE Handle;
	INT64 Pointer;
	bool NeedSyncPointer;
	string name;
	DWORD share_mode;

	inline void SyncPointer();
};

class FileWalker: public File
{
public:
	FileWalker();
	bool InitWalk(size_t BlockSize);
	bool Step();
	UINT64 GetChunkOffset() const;
	DWORD GetChunkSize() const;
	int GetPercent() const;

private:
	struct Chunk
	{
		UINT64 Offset;
		DWORD Size;
		Chunk(UINT64 Offset, DWORD Size):Offset(Offset), Size(Size) {}
	};
	std::list<Chunk> ChunkList;
	UINT64 FileSize;
	UINT64 AllocSize;
	UINT64 ProcessedSize;
	std::list<Chunk>::iterator CurrentChunk;
	DWORD ChunkSize;
	bool Sparse;
};

class sid_object:NonCopyable
{
public:
	sid_object(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0 = 0, DWORD SubAuthority1 = 0, DWORD SubAuthority2 = 0, DWORD SubAuthority3 = 0, DWORD SubAuthority4 = 0, DWORD SubAuthority5 = 0, DWORD SubAuthority6 = 0, DWORD SubAuthority7 = 0)
	{
		if (!AllocateAndInitializeSid(IdentifierAuthority, SubAuthorityCount, SubAuthority0, SubAuthority1, SubAuthority2, SubAuthority3, SubAuthority4, SubAuthority5, SubAuthority6, SubAuthority7, &m_value))
		{
			throw FarRecoverableException("unable to allocate and initialize SID");
		}
	}

	~sid_object()
	{
		FreeSid(m_value);
	}

	PSID get() const { return m_value; }

private:
	PSID m_value;
};

static NTSTATUS GetLastNtStatus();

static bool GetEnvironmentVariable(const string& Name, string& Buffer);
static bool SetEnvironmentVariable(const string& Name, const string& Value);
static bool DeleteEnvironmentVariable(const string& Name);
static string ExpandEnvironmentStrings(const string& str);

static DWORD GetCurrentDirectory(
    string &strCurDir
);

static DWORD GetTempPath(
    string &strBuffer
);

static DWORD GetModuleFileName(
    HMODULE hModule,
    string &strFileName
);

static DWORD GetModuleFileNameEx(
    HANDLE hProcess,
    HMODULE hModule,
    string &strFileName
);

static DWORD WNetGetConnection(
    const string& LocalName,
    string &RemoteName
);

static BOOL GetVolumeInformation(
    const string& RootPathName,
    string *pVolumeName,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    string *pFileSystemName
);

static BOOL FindStreamClose(
    HANDLE hFindFile
);

static bool GetFindDataEx(
    const string& FileName,
    FAR_FIND_DATA& FindData,
    bool ScanSymLink=true);

static bool GetFileSizeEx(
    HANDLE hFile,
    UINT64 &Size);

static BOOL DeleteFile(
    const string& FileName
);

static BOOL RemoveDirectory(
    const string& DirName
);

static HANDLE CreateFile(
    const string& Object,
    DWORD DesiredAccess,
    DWORD ShareMode,
    LPSECURITY_ATTRIBUTES SecurityAttributes,
    DWORD CreationDistribution,
    DWORD FlagsAndAttributes=0,
    HANDLE TemplateFile=nullptr,
    bool ForceElevation = false
);

static BOOL CopyFileEx(
    const string& ExistingFileName,
    const string& NewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData,
    LPBOOL pbCancel,
    DWORD dwCopyFlags
);

static BOOL MoveFile(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName   // address of new name for the file
);

static BOOL MoveFileEx(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
);

static int RegEnumKeyEx(
    HKEY hKey,
    DWORD dwIndex,
    string &strName,
    PFILETIME lpftLastWriteTime=nullptr
);

static BOOL IsDiskInDrive(
    const string& Root
);

static int GetFileTypeByName(
    const string& Name
);

static bool GetDiskSize(
    const string& Path,
    unsigned __int64 *TotalSize,
    unsigned __int64 *TotalFree,
    unsigned __int64 *UserFree
);

static HANDLE FindFirstFileName(
    const string& FileName,
    DWORD dwFlags,
    string& LinkName
);

static BOOL FindNextFileName(
    HANDLE hFindStream,
    string& LinkName
);

static BOOL CreateDirectory(
    const string& PathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
);

static BOOL CreateDirectoryEx(
    const string& TemplateDirectory,
    const string& NewDirectory,
    LPSECURITY_ATTRIBUTES SecurityAttributes
);

static DWORD GetFileAttributes(
    const string& FileName
);

static BOOL SetFileAttributes(
    const string& FileName,
    DWORD dwFileAttributes
);

static void InitCurrentDirectory();

static BOOL SetCurrentDirectory(
    const string& PathName,
    bool Validate = true
);

static bool CreateSymbolicLink(
    const string& SymlinkFileName,
    const string& TargetFileName,
    DWORD dwFlags
);

static bool SetFileEncryption(const string& Name, bool Encrypt);

static BOOL CreateHardLink(
    const string& FileName,
    const string& ExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
);

static HANDLE FindFirstStream(
    const string& FileName,
    STREAM_INFO_LEVELS InfoLevel,
    LPVOID lpFindStreamData,
    DWORD dwFlags=0
);

static BOOL FindNextStream(
    HANDLE hFindStream,
    LPVOID lpFindStreamData
);

static std::vector<string> GetLogicalDriveStrings();

static bool GetFinalPathNameByHandle(
    HANDLE hFile,
    string& FinalFilePath
);

static bool SearchPath(
	const wchar_t *Path,
	const string& FileName,
	const wchar_t *Extension,
	string &strDest
);

static bool QueryDosDevice(
	const string& DeviceName,
	string& Path
);

static bool GetVolumeNameForVolumeMountPoint(
	const string& VolumeMountPoint,
	string& VolumeName
);

static bool GetFileTimeSimple(const string &FileName, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);

static void EnableLowFragmentationHeap();

static int RegQueryStringValue(HKEY hKey, const string& ValueName, string &strData, const wchar_t *lpwszDefault = L"");
static int EnumRegValueEx(HKEY hRegRootKey, const string& Key, DWORD Index, string &strDestName, string &strData, LPDWORD IData=nullptr,__int64* IData64=nullptr, DWORD *Type=nullptr);

typedef block_ptr<SECURITY_DESCRIPTOR> FAR_SECURITY_DESCRIPTOR;

static bool GetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);

static bool SetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);

static bool OpenVirtualDisk(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);

static string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName);

static DWORD GetAppPathsRedirectionFlag();

private:
	static bool CreateSymbolicLinkInternal(const string& Object,const string& Target, DWORD dwFlags);
	static bool SetFileEncryptionInternal(const wchar_t* Name, bool Encrypt);
	static bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	static bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);
	static bool OpenVirtualDiskInternal(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);

	friend class elevation;
	friend class elevated;
};