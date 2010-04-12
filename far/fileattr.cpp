/*
fileattr.cpp

Работа с атрибутами файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "flink.hpp"

typedef BOOL (WINAPI *PEncryptFileA)(LPCSTR lpFileName);
typedef BOOL (WINAPI *PDecryptFileA)(LPCSTR lpFileName, DWORD dwReserved);

static PEncryptFileA pEncryptFileA=NULL;
static PDecryptFileA pDecryptFileA=NULL;

static int SetFileEncryption(const char *Name,int State);
static int SetFileCompression(const char *Name,int State);

// получим функции криптования
int GetEncryptFunctions(void)
{
	const char *Names[]=
	{
		"KERNEL32","ADVAPI32","EncryptFileA","DecryptFileA",
	};

	if (!pEncryptFileA)
	{
		// работает только под Win2000! Если не 2000, то не надо и показывать эту опцию.
		pEncryptFileA = (PEncryptFileA)GetProcAddress(GetModuleHandle(Names[0]),Names[2]);

		if (!pEncryptFileA)
			pEncryptFileA = (PEncryptFileA)GetProcAddress(GetModuleHandle(Names[1]),Names[2]);
	}

	if (!pDecryptFileA)
	{
		pDecryptFileA = (PDecryptFileA)GetProcAddress(GetModuleHandle(Names[0]),Names[3]);

		if (!pDecryptFileA)
			pDecryptFileA = (PDecryptFileA)GetProcAddress(GetModuleHandle(Names[1]),Names[3]);
	}

	if (pDecryptFileA && pEncryptFileA)
		IsCryptFileASupport=TRUE;

	return IsCryptFileASupport;
}

int ESetFileAttributes(const char *Name,int Attr,int SkipMode)
{
//_SVS(SysLog("Attr=0x%08X",Attr));
	while (!SetFileAttributes(Name,Attr))
	{
		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrCannotFor),(char *)Name,MSG(MHRetry),MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		switch (Code)
		{
			case -2:
			case -1:
			case 1:
				return SETATTR_RET_SKIP;
			case 2:
				return SETATTR_RET_SKIPALL;
			case 3:
				return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}

static int SetFileCompression(const char *Name,int State)
{
	HANDLE hFile=FAR_CreateFile(Name,FILE_READ_DATA|FILE_WRITE_DATA,
	                            FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
	                            FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	if (hFile==INVALID_HANDLE_VALUE)
		return(FALSE);

	USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
	UDWORD Result;
	int RetCode=DeviceIoControl(hFile,FSCTL_SET_COMPRESSION,&NewState,
	                            sizeof(NewState),NULL,0,&Result,NULL);
	CloseHandle(hFile);
	return(RetCode);
}

/*
  Для безусловного выставления атрибута FILE_ATTRIBUTE_COMPRESSED
  необходимо в качестве параметра FileAttr передать значение 0
*/
int ESetFileCompression(const char *Name,int State,int FileAttr,int SkipMode)
{
	if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
		return SETATTR_RET_OK;

	int Ret=SETATTR_RET_OK;

	if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
		SetFileAttributes(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));

	// Drop Encryption
	if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
		SetFileEncryption(Name,0);

	while (!SetFileCompression(Name,State))
	{
		if (GetLastError()==ERROR_INVALID_FUNCTION)
		{
			Ret=SETATTR_RET_OK;
			break;
		}

		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrCompressedCannotFor),(char *)Name,MSG(MHRetry),
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		if (Code==1 || Code<0)
		{
			Ret=SETATTR_RET_SKIP;
			break;
		}
		else if (Code==2)
		{
			Ret=SETATTR_RET_SKIPALL;
			break;
		}
		else if (Code==3)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}
	}

	// Set ReadOnly
	if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
		SetFileAttributes(Name,FileAttr);

	return(Ret);
}

/* $ 20.10.2000 SVS
   Новый атрибут Encrypted
*/
static int SetFileEncryption(const char *Name,int State)
{
	class ApisToANSI
	{
		public:
			ApisToANSI() {SetFileApisTo(APIS2ANSI);}
			~ApisToANSI() {SetFileApisTo(APIS2OEM);}
	};
	char AnsiName[NM];
	ApisToANSI Apis;
	FAR_OemToChar(Name,AnsiName);

	// заодно и проверяется успешность получения адреса API...
	if (State)
		return pEncryptFileA ? (*pEncryptFileA)(AnsiName) : FALSE;
	else
		return pDecryptFileA ? (*pDecryptFileA)(AnsiName, 0) : FALSE;
}
/*
  Для безусловного выставления атрибута FILE_ATTRIBUTE_ENCRYPTED
  необходимо в качестве параметра FileAttr передать значение 0
*/
int ESetFileEncryption(const char *Name,int State,int FileAttr,int SkipMode,int Silent)
{
	if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
		return SETATTR_RET_OK;

	if (!IsCryptFileASupport)
		return SETATTR_RET_OK;

	int Ret=SETATTR_RET_OK;
	// Drop Compress
	// Этот кусок не нужен, т.к. функция криптования сама умеет
	// разжимать сжатые файлы.
	//if ((FileAttr & FILE_ATTRIBUTE_COMPRESSED) && State)
	//  SetFileCompression(Name,0);

	// Drop ReadOnly
	if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
		SetFileAttributes(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));

	while (!SetFileEncryption(Name,State))
	{
		if (Silent)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}

		if ((_localLastError=GetLastError())==ERROR_INVALID_FUNCTION)
			break;

		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrEncryptedCannotFor),(char *)Name,MSG(MHRetry),
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		if (Code==1 || Code<0)
		{
			Ret=SETATTR_RET_SKIP;
			break;
		}

		if (Code==2)
		{
			Ret=SETATTR_RET_SKIPALL;
			break;
		}

		if (Code==3)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}
	}

	// Set ReadOnly
	if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
		SetFileAttributes(Name,FileAttr);

	return(Ret);
}

int ESetFileTime(const char *Name,FILETIME *LastWriteTime,FILETIME *CreationTime,
                 FILETIME *LastAccessTime,int FileAttr,int SkipMode)
{
	if (LastWriteTime==NULL && CreationTime==NULL && LastAccessTime==NULL ||
	        ((FileAttr & FA_DIREC) && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT))
		return SETATTR_RET_OK;

	while (1)
	{
		if (FileAttr & FA_RDONLY)
			SetFileAttributes(Name,FileAttr & ~FA_RDONLY);

		HANDLE hFile=FAR_CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
		                            NULL,OPEN_EXISTING,
		                            (FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
		int SetTime;
		DWORD LastError=0;

		if (hFile==INVALID_HANDLE_VALUE)
		{
			SetTime=FALSE;
			LastError=GetLastError();
		}
		else
		{
			SetTime=SetFileTime(hFile,CreationTime,LastAccessTime,LastWriteTime);
			LastError=GetLastError();
			CloseHandle(hFile);

			if ((FileAttr & FA_DIREC) && LastError==ERROR_NOT_SUPPORTED)   // FIX: Mantis#223
			{
				char DriveRoot[NM];
				GetPathRoot(Name, DriveRoot);

				if (GetDriveType(DriveRoot)==DRIVE_REMOTE) break;
			}
		}

		if (FileAttr & FA_RDONLY)
			SetFileAttributes(Name,FileAttr);

		SetLastError(LastError);

		if (SetTime)
			break;

		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrTimeCannotFor),(char *)Name,MSG(MHRetry),
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		switch (Code)
		{
			case -2:
			case -1:
			case 3:
				return SETATTR_RET_ERROR;
			case 2:
				return SETATTR_RET_SKIPALL;
			case 1:
				return SETATTR_RET_SKIP;
		}
	}

	return SETATTR_RET_OK;
}
