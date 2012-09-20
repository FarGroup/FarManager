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

#include "DList.hpp"

#define NT_MAX_PATH 32768

struct FAR_FIND_DATA_EX
{
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	FILETIME ftChangeTime;
	unsigned __int64 nFileSize;

	unsigned __int64 nAllocationSize;
	struct
	{
		DWORD dwReserved0;
		DWORD dwReserved1;
	};
	unsigned __int64 FileId;
	string   strFileName;
	string   strAlternateFileName;

	void Clear()
	{
		dwFileAttributes=0;
		ClearStruct(ftCreationTime);
		ClearStruct(ftLastAccessTime);
		ClearStruct(ftLastWriteTime);
		ClearStruct(ftChangeTime);
		nFileSize=0;
		nAllocationSize=0;
		dwReserved0=0;
		dwReserved1=0;
		FileId = 0;
		strFileName.Clear();
		strAlternateFileName.Clear();
	}

	FAR_FIND_DATA_EX& operator=(const FAR_FIND_DATA_EX &ffdexCopy)
	{
		if (this != &ffdexCopy)
		{
			dwFileAttributes=ffdexCopy.dwFileAttributes;
			ftCreationTime=ffdexCopy.ftCreationTime;
			ftLastAccessTime=ffdexCopy.ftLastAccessTime;
			ftLastWriteTime=ffdexCopy.ftLastWriteTime;
			ftChangeTime=ffdexCopy.ftChangeTime;
			nFileSize=ffdexCopy.nFileSize;
			nAllocationSize=ffdexCopy.nAllocationSize;
			dwReserved0=ffdexCopy.dwReserved0;
			dwReserved1=ffdexCopy.dwReserved1;
			FileId = ffdexCopy.FileId;
			strFileName=ffdexCopy.strFileName;
			strAlternateFileName=ffdexCopy.strAlternateFileName;
		}

		return *this;
	}
};

class FindFile:NonCopyable
{
public:
	FindFile(const string& Object, bool ScanSymLink = true);
	~FindFile();
	bool Get(FAR_FIND_DATA_EX& FindData);

private:
	HANDLE Handle;
	bool empty;
	FAR_FIND_DATA_EX Data;
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
	bool Close();
	bool Eof();
	bool Opened() const {return Handle != INVALID_HANDLE_VALUE;}

private:
	HANDLE Handle;
	INT64 Pointer;
	bool NeedSyncPointer;

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
	};
	DList<Chunk> ChunkList;
	Chunk SingleChunk;
	UINT64 FileSize;
	UINT64 AllocSize;
	UINT64 ProcessedSize;
	Chunk* CurrentChunk;
	DWORD ChunkSize;
	bool Sparse;
};

NTSTATUS GetLastNtStatus();

DWORD apiGetEnvironmentVariable(
    const string& Name,
    string &strBuffer
);

DWORD apiGetCurrentDirectory(
    string &strCurDir
);

DWORD apiGetTempPath(
    string &strBuffer
);

DWORD apiGetModuleFileName(
    HMODULE hModule,
    string &strFileName
);

DWORD apiGetModuleFileNameEx(
    HANDLE hProcess,
    HMODULE hModule,
    string &strFileName
);

bool apiExpandEnvironmentStrings(
    const string& Src,
    string &Dest
);

DWORD apiWNetGetConnection(
    const string& LocalName,
    string &RemoteName
);

BOOL apiGetVolumeInformation(
    const string& RootPathName,
    string *pVolumeName,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    string *pFileSystemName
);

BOOL apiFindStreamClose(
    HANDLE hFindFile
);

bool apiGetFindDataEx(
    const string& FileName,
    FAR_FIND_DATA_EX& FindData,
    bool ScanSymLink=true);

bool apiGetFileSizeEx(
    HANDLE hFile,
    UINT64 &Size);

//junk

BOOL apiDeleteFile(
    const string& FileName
);

BOOL apiRemoveDirectory(
    const string& DirName
);

HANDLE apiCreateFile(
    const string& Object,
    DWORD DesiredAccess,
    DWORD ShareMode,
    LPSECURITY_ATTRIBUTES SecurityAttributes,
    DWORD CreationDistribution,
    DWORD FlagsAndAttributes=0,
    HANDLE TemplateFile=nullptr,
    bool ForceElevation = false
);

BOOL apiCopyFileEx(
    const string& ExistingFileName,
    const string& NewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData,
    LPBOOL pbCancel,
    DWORD dwCopyFlags
);

BOOL apiMoveFile(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName   // address of new name for the file
);

BOOL apiMoveFileEx(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
);

int apiRegEnumKeyEx(
    HKEY hKey,
    DWORD dwIndex,
    string &strName,
    PFILETIME lpftLastWriteTime=nullptr
);

BOOL apiIsDiskInDrive(
    const string& Root
);

int apiGetFileTypeByName(
    const string& Name
);

bool apiGetDiskSize(
    const string& Path,
    unsigned __int64 *TotalSize,
    unsigned __int64 *TotalFree,
    unsigned __int64 *UserFree
);

HANDLE apiFindFirstFileName(
    const string& FileName,
    DWORD dwFlags,
    string& LinkName
);

BOOL apiFindNextFileName(
    HANDLE hFindStream,
    string& LinkName
);

BOOL apiCreateDirectory(
    const string& PathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
);

BOOL apiCreateDirectoryEx(
    const string& TemplateDirectory,
    const string& NewDirectory,
    LPSECURITY_ATTRIBUTES SecurityAttributes
);

DWORD apiGetFileAttributes(
    const string& FileName
);

BOOL apiSetFileAttributes(
    const string& FileName,
    DWORD dwFileAttributes
);

void InitCurrentDirectory();

BOOL apiSetCurrentDirectory(
    const string& PathName,
    bool Validate = true
);

// for elevation only, don't use outside.
bool CreateSymbolicLinkInternal(const string& Object,const string& Target, DWORD dwFlags);
bool apiSetFileEncryptionInternal(const wchar_t* Name, bool Encrypt);

bool apiCreateSymbolicLink(
    const string& SymlinkFileName,
    const string& TargetFileName,
    DWORD dwFlags
);

bool apiSetFileEncryption(const string& Name, bool Encrypt);

BOOL apiCreateHardLink(
    const string& FileName,
    const string& ExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
);

HANDLE apiFindFirstStream(
    const string& FileName,
    STREAM_INFO_LEVELS InfoLevel,
    LPVOID lpFindStreamData,
    DWORD dwFlags=0
);

BOOL apiFindNextStream(
    HANDLE hFindStream,
    LPVOID lpFindStreamData
);

bool apiGetLogicalDriveStrings(
    string& DriveStrings
);

bool apiGetFinalPathNameByHandle(
    HANDLE hFile,
    string& FinalFilePath
);

bool apiSearchPath(
	const wchar_t *Path,
	const string& FileName,
	const wchar_t *Extension,
	string &strDest
);

bool apiQueryDosDevice(
	const string& DeviceName,
	string& Path
);

bool apiGetVolumeNameForVolumeMountPoint(
	const string& VolumeMountPoint,
	string& VolumeName
);

// internal, dont' use outside.
bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);

void apiEnableLowFragmentationHeap();

int RegQueryStringValue(HKEY hKey, const string& ValueName, string &strData, const wchar_t *lpwszDefault = L"");
int EnumRegValueEx(HKEY hRegRootKey, const string& Key, DWORD Index, string &strDestName, string &strData, LPDWORD IData=nullptr,__int64* IData64=nullptr, DWORD *Type=nullptr);

struct FAR_SECURITY_DESCRIPTOR
{
	size_t Size;
	intptr_t SecurityDescriptorBaseAddress;
	PSECURITY_DESCRIPTOR SecurityDescriptor;
};

struct FAR_SECURITY_DESCRIPTOR_EX:public FAR_SECURITY_DESCRIPTOR
{
	FAR_SECURITY_DESCRIPTOR_EX() {Size = 0, SecurityDescriptor = nullptr;}
	~FAR_SECURITY_DESCRIPTOR_EX() {if(SecurityDescriptor) xf_free(SecurityDescriptor);}
};

bool apiGetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);

bool apiSetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);

// for elevation only, don't use outside.
bool apiOpenVirtualDiskInternal(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);

bool apiOpenVirtualDisk(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);
