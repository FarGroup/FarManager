/*
flink.cpp

Куча разных функций по обработке Link`ов - Hard&Sym
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "copy.hpp"
#include "flink.hpp"
#include "imports.hpp"
#include "cddrv.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "panelmix.hpp"

#include "message.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "dirmix.hpp"
#include "treelist.hpp"

bool WINAPI CreateVolumeMountPoint(const wchar_t *TargetVolume, const wchar_t *Object)
{
	bool Result=false;
	wchar_t Buf[50];
	if(GetVolumeNameForVolumeMountPoint(TargetVolume,Buf,countof(Buf)))
	{
		if(SetVolumeMountPoint(Object,Buf))
		{
			Result=true;
		}
	}
	return Result;
}

bool FillREPARSE_DATA_BUFFER(PREPARSE_DATA_BUFFER rdb,LPCWSTR PrintName,size_t PrintNameLength,LPCWSTR SubstituteName,size_t SubstituteNameLength)
{
	bool Result=false;
	rdb->Reserved=0;
	switch(rdb->ReparseTag)
	{
	case IO_REPARSE_TAG_MOUNT_POINT:
		rdb->MountPointReparseBuffer.SubstituteNameOffset=0;
		rdb->MountPointReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof (wchar_t));
		rdb->MountPointReparseBuffer.PrintNameOffset=rdb->MountPointReparseBuffer.SubstituteNameLength+2;
		rdb->MountPointReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
		rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,MountPointReparseBuffer.PathBuffer)+rdb->MountPointReparseBuffer.PrintNameOffset+rdb->MountPointReparseBuffer.PrintNameLength+1*sizeof(wchar_t)-REPARSE_DATA_BUFFER_HEADER_SIZE;
		if(rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
		{
			wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength+1);
			wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength+1);
			Result=true;
		}
		break;
	case IO_REPARSE_TAG_SYMLINK:
		rdb->SymbolicLinkReparseBuffer.PrintNameOffset=0;
		rdb->SymbolicLinkReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
		rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset=rdb->MountPointReparseBuffer.PrintNameLength;
		rdb->SymbolicLinkReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof (wchar_t));
		rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,SymbolicLinkReparseBuffer.PathBuffer)+rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset+rdb->SymbolicLinkReparseBuffer.SubstituteNameLength-REPARSE_DATA_BUFFER_HEADER_SIZE;
		if(rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
		{
			wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength);
			wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength);
			Result=true;
		}
		break;
	}
	return Result;
}

bool SetREPARSE_DATA_BUFFER(const wchar_t *Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	HANDLE hObject=apiCreateFile(Object,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT);
	if(hObject!=INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesReturned;
		if(IsReparseTagValid(rdb->ReparseTag))
		{
			if(rdb->ReparseTag==IO_REPARSE_TAG_SYMLINK)
			{
				SetPrivilege(L"SeCreateSymbolicLinkPrivilege",TRUE);
			}
			if(DeviceIoControl(hObject,FSCTL_SET_REPARSE_POINT,rdb,rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE,NULL,0,&dwBytesReturned,0))
			{
				Result=true;
			}
		}
		CloseHandle(hObject);
	}
	return Result;
}

bool WINAPI CreateReparsePoint(const wchar_t *Target, const wchar_t *Object,DWORD Type)
{
	bool Result=false;
	if(Object && *Object && Target && *Target)
	{
		switch(Type)
		{
		case RP_EXACTCOPY:
			Result=DuplicateReparsePoint(Target,Object);
			break;
		case RP_SYMLINKFILE:
		case RP_SYMLINKDIR:
			if(ifn.pfnCreateSymbolicLink)
			{
				Result=apiCreateSymbolicLink(Object,Target,Type==RP_SYMLINKDIR?SYMBOLIC_LINK_FLAG_DIRECTORY:0)!=FALSE;
			}
			else
			{
				bool ObjectCreated=false;
				if(Type==RP_SYMLINKDIR)
				{
					ObjectCreated=apiCreateDirectory(Object,NULL)!=FALSE;
				}
				else
				{
					HANDLE hFile=apiCreateFile(Object,0,0,NULL,CREATE_NEW,0);
					if(hFile!=INVALID_HANDLE_VALUE)
					{
						ObjectCreated=true;
						CloseHandle(hFile);
					}
				}
				if(ObjectCreated)
				{
					BYTE szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
					PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
					rdb->ReparseTag=IO_REPARSE_TAG_SYMLINK;
					string strPrintName=Target,strSubstituteName=Target;
					if(PathMayBeAbsolute(Target))
					{
						strSubstituteName=L"\\??\\";
						strSubstituteName+=(strPrintName.CPtr()+(PathPrefix(strPrintName)?4:0));
						rdb->SymbolicLinkReparseBuffer.Flags=0;
					}
					else
					{
						rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
					}
					if(FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength()))
					{
						Result=SetREPARSE_DATA_BUFFER(Object,rdb);
					}
					else
					{
						SetLastError(ERROR_INSUFFICIENT_BUFFER);
					}
				}
			}
			break;
		case RP_JUNCTION:
		case RP_VOLMOUNT:
			{
				string strPrintName,strSubstituteName;
				ConvertNameToFull(Target,strPrintName);
				strSubstituteName=L"\\??\\";
				strSubstituteName+=(strPrintName.CPtr()+(PathPrefix(strPrintName)?4:0));

				BYTE szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
				PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);

				rdb->ReparseTag=IO_REPARSE_TAG_MOUNT_POINT;
				if(FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength()))
				{
					Result=SetREPARSE_DATA_BUFFER(Object,rdb);
				}
				else
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
				}
			}
			break;
		}
	}
	return Result;
}

bool WINAPI DeleteReparsePoint(const wchar_t *Object)
{
	bool Result=false;
	DWORD ReparseTag;
	string strTmp;
	GetReparsePointInfo(Object,strTmp,&ReparseTag);
	HANDLE hObject=apiCreateFile(Object,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT);
	if(hObject!=INVALID_HANDLE_VALUE)
	{
		REPARSE_GUID_DATA_BUFFER rgdb={ReparseTag};
		DWORD dwBytes;
		Result=(DeviceIoControl(hObject,FSCTL_DELETE_REPARSE_POINT,&rgdb,REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,NULL,0,&dwBytes,0)==TRUE);
		CloseHandle(hObject);
	}
	return Result;
}

bool GetREPARSE_DATA_BUFFER(const wchar_t *Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	const DWORD FileAttr = apiGetFileAttributes(Object);
	/* $ 14.06.2003 IS
		Для нелокальных дисков получить корректную информацию о связи
		не представляется возможным
	*/
	if(FileAttr!=INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && IsLocalDrive(Object))
	{
		HANDLE hObject=apiCreateFile(Object,0,0,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT);
		if (hObject!=INVALID_HANDLE_VALUE)
		{
			DWORD dwBytesReturned;
			if(DeviceIoControl(hObject,FSCTL_GET_REPARSE_POINT,NULL,0,(LPVOID)rdb,MAXIMUM_REPARSE_DATA_BUFFER_SIZE,&dwBytesReturned,0) && IsReparseTagValid(rdb->ReparseTag))
			{
				Result=true;
			}
			CloseHandle(hObject);
		}
	}
	return Result;
}

bool SetPrivilege(LPCWSTR Privilege,BOOL bEnable)
{
	bool Result=false;
	HANDLE hToken=0;
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		TOKEN_PRIVILEGES tp;
		if(LookupPrivilegeValue(NULL,Privilege,&tp.Privileges->Luid))
		{
			tp.PrivilegeCount=1;
			tp.Privileges->Attributes=bEnable?SE_PRIVILEGE_ENABLED:0;
			if(AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL) && GetLastError()==ERROR_SUCCESS)
			{
				Result=true;
			}
		}
		CloseHandle(hToken);
	}
	return Result;
}

DWORD WINAPI GetReparsePointInfo(const wchar_t *Object, string &strDestBuff,LPDWORD lpReparseTag)
{
	BYTE szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	PREPARSE_DATA_BUFFER rdb = reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
	WORD NameLength=0;
	if(GetREPARSE_DATA_BUFFER(Object,rdb))
	{
		const wchar_t *PathBuffer;
		if(lpReparseTag)
			*lpReparseTag=rdb->ReparseTag;
		if(rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK)
		{
			NameLength = rdb->SymbolicLinkReparseBuffer.PrintNameLength/sizeof(wchar_t);
			if(NameLength)
			{
				PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
			}
			else
			{
				NameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
				PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
			}
		}
		else
		{
			NameLength = rdb->MountPointReparseBuffer.PrintNameLength/sizeof(wchar_t);
			if(NameLength)
			{
				PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
			}
			else
			{
				NameLength = rdb->MountPointReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
				PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
			}
		}
		wchar_t *lpwszDestBuff=strDestBuff.GetBuffer(NameLength+1);
		wcsncpy(lpwszDestBuff,PathBuffer,NameLength);
		strDestBuff.ReleaseBuffer(NameLength);
	}
	return NameLength;
}

int WINAPI GetNumberOfLinks(const wchar_t *Name)
{
	int NumberOfLinks=1;
	HANDLE hFile=apiCreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		if(GetFileInformationByHandle(hFile,&bhfi))
		{
			NumberOfLinks=bhfi.nNumberOfLinks;
		}
		CloseHandle(hFile);
	}
	return NumberOfLinks;
}


int WINAPI MkHardLink(const wchar_t *ExistingName,const wchar_t *NewName)
{
	return apiCreateHardLink(NewName,ExistingName,NULL)!=FALSE;
}

/*
  Функция EnumNTFSStreams позволяет получить количество потоков,
  привязанных к файлу FileName.
  Параметры:
   FileName - имя файла
   fpEnum   - функция, получающая 2 параметра - номер потока (Idx) и имя
              потока в Unicode. Перечисление прерывается, если функция
              вернет FALSE. Этот параметр может быть равен NULL.
              Простейший вариант CALLBACK-перечислителя:

              BOOL WINAPI EnumFileStreams(int Idx,WCHAR *StreamName,const WIN32_STREAM_ID *sid)
              {
                char Buf[260];
                UnicodeToANSI(StreamName,Buf,sizeof(Buf));
                printf("%2d) '%s' StreamId=%d",Idx,Buf,sid->dwStreamId);
                switch(sid->dwStreamId)
                {
                  case BACKUP_DATA: printf(" (Standard data)\n");break;
                  case BACKUP_EA_DATA: printf(" (Extended attribute data)\n");break;
                  case BACKUP_SECURITY_DATA: printf(" (Windows NT security descriptor data)\n");break;
                  case BACKUP_ALTERNATE_DATA: printf(" (Alternative data streams)\n");break;
                  case BACKUP_LINK: printf(" (Hard link information)\n");break;
                }
                return TRUE;
              }
   SizeStreams - указатель на паременную типа __int64, в которую будет помещен
                 общий размер всех потоков (актуально для копира!).
                 Параметр необязательный.

  Return: Количество потоков в файле.
          -1 - ошибка открытия файла
           0 - FileName - каталог и в нем еще нет потоков, по умолчанию
               каталоги не имеют потоков (файлы имеют один - {Data})
           1 - файл имеет основной поток данных...
          >1 - ... и несколько дополнительных.
*/

/*
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams)
{
  int StreamsCount=-1;

  HANDLE hFile = FAR_CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, 0, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    // Prepare for execution
    LPVOID lpContext = NULL;
    WIN32_STREAM_ID sid;
    DWORD dwRead;
    WCHAR wszStreamName[MAX_PATH];

    StreamsCount=0;

    // Enumerate the streams...
    BOOL bContinue = TRUE;
    while (bContinue)
    {
      // Calculate the size of the variable length
      // WIN32_STREAM_ID structure
      memset(&sid,0,sizeof(WIN32_STREAM_ID));
      memset(wszStreamName,0,MAX_PATH);
      DWORD dwStreamHeaderSize = (LPBYTE)&sid.cStreamName -
           (LPBYTE)&sid+ sid.dwStreamNameSize;

      bContinue = BackupRead(hFile, (LPBYTE) &sid,
        dwStreamHeaderSize, &dwRead, FALSE, FALSE,
        &lpContext);

      if (!dwRead)
        break;
      if (dwRead != dwStreamHeaderSize)
        break;
      // At this point, we've read the header of the i.th
      // stream. What follows is the name of the stream and
      // next its content.

      // Read the stream name
      BackupRead(hFile,(LPBYTE)wszStreamName,sid.dwStreamNameSize,&dwRead,FALSE,FALSE,&lpContext);
      if (dwRead != sid.dwStreamNameSize)
        break;

      // A stream name is stored enclosed in a pair of colons
      // plus a $DATA default trailer. If the stream name is
      // VersionInfo it's stored (and retrieved) as:
      // :VersionInfo:$DATA
      if (StrLength(wszStreamName))
      {
        WCHAR Wsz[MAX_PATH], *pwsz=Wsz;
				lstrcpy(pwsz, wszStreamName + sizeof(CHAR));
        LPWSTR wp = wcsstr(pwsz, L":");
        pwsz[wp-pwsz] = 0;
				lstrcpy(wszStreamName, pwsz);
      }

      StreamsCount++;
      if(SizeStreams)
        *SizeStreams+=sid.Size.QuadPart;
      if(fpEnum && !fpEnum(StreamsCount,wszStreamName,&sid))
        break;

      // Skip the stream body
      DWORD dw1, dw2;
      if(!BackupSeek(hFile, sid.Size.u.LowPart, sid.Size.u.HighPart,&dw1, &dw2, &lpContext))
         break;
    }

    CloseHandle(hFile);
  }
  return StreamsCount;
}

*/

bool EnumStreams(const wchar_t *FileName,UINT64 &StreamsSize,DWORD &StreamsCount)
{
	bool Result=false;
	WIN32_FIND_STREAM_DATA fsd;
	HANDLE hFind=apiFindFirstStream(FileName,FindStreamInfoStandard,&fsd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		StreamsCount=1;
		StreamsSize=fsd.StreamSize.QuadPart;
		while(apiFindNextStream(hFind,&fsd))
		{
			StreamsCount++;
			StreamsSize+=fsd.StreamSize.QuadPart;
		}
		apiFindStreamClose(hFind);
		Result=true;
	}
	return Result;
}

bool DelSubstDrive(const wchar_t *DeviceName)
{
	bool Result=false;
	string strTargetPath;
	if(GetSubstName(DRIVE_NOT_INIT,DeviceName,strTargetPath))
	{
		strTargetPath=(string)L"\\??\\"+strTargetPath;
		Result=(DefineDosDevice(DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|DDD_EXACT_MATCH_ON_REMOVE,DeviceName,strTargetPath)==TRUE);
	}
	return Result;
}

bool GetSubstName(int DriveType,const wchar_t *DeviceName, string &strTargetPath)
{
	bool Ret=false;
	/*
	+ Обработка в зависимости от Opt.SubstNameRule
	битовая маска:
	0 - если установлен, то опрашивать сменные диски
	1 - если установлен, то опрашивать все остальные
	*/
	bool DriveRemovable = (DriveType==DRIVE_REMOVABLE || DriveType==DRIVE_CDROM);
	if (DriveType==DRIVE_NOT_INIT || (((Opt.SubstNameRule & 1) || !DriveRemovable) && ((Opt.SubstNameRule & 2) || DriveRemovable)))
	{
		if(IsLocalPath(DeviceName))
		{
			wchar_t *Name=new wchar_t[NT_MAX_PATH];
			if(Name)
			{
				if(QueryDosDevice(DeviceName,Name,NT_MAX_PATH))
				{
					if(!StrCmpN(Name,L"\\??\\",4))
					{
						strTargetPath=Name+4;
						Ret=true;
					}
				}
				delete[] Name;
			}
		}
	}
	return Ret;
}

void GetPathRoot(const wchar_t *Path, string &strRoot)
{
  string RealPath;
  ConvertNameToReal(Path, RealPath);
  strRoot = ExtractPathRoot(RealPath);
}

bool ModifyReparsePoint(const wchar_t *Object,const wchar_t *NewData)
{
	bool Result=false;
	BYTE szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
	if(GetREPARSE_DATA_BUFFER(Object,rdb))
	{
		bool FillResult=false;
		switch(rdb->ReparseTag)
		{
		case IO_REPARSE_TAG_MOUNT_POINT:
			{
				string strPrintName,strSubstituteName;
				ConvertNameToFull(NewData,strPrintName);
				strSubstituteName=L"\\??\\";
				strSubstituteName+=(strPrintName.CPtr()+(PathPrefix(strPrintName)?4:0));
				FillResult=FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength());
			}
			break;
		case IO_REPARSE_TAG_SYMLINK:
			{
				string strPrintName=NewData,strSubstituteName=NewData;
				if(PathMayBeAbsolute(NewData))
				{
					strSubstituteName=L"\\??\\";
					strSubstituteName+=(strPrintName.CPtr()+(PathPrefix(strPrintName)?4:0));
					rdb->SymbolicLinkReparseBuffer.Flags=0;
				}
				else
				{
					rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
				}
				FillResult=FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength());
			}
			break;
		}
		if(FillResult)
		{
			Result=SetREPARSE_DATA_BUFFER(Object,rdb);
		}
		else
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
		}
	}
	return Result;
}

bool DuplicateReparsePoint(const wchar_t *Src,const wchar_t *Dst)
{
	bool Result=false;
	BYTE szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
	if(GetREPARSE_DATA_BUFFER(Src,rdb) && SetREPARSE_DATA_BUFFER(Dst,rdb))
	{
		Result=true;
	}
	return Result;
}

int WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
	int Result=0;

	if(Src && *Src && Dest && *Dest)
	{
		int Op=Flags&0xFFFF;

		switch(Op)
		{
			case FLINK_HARDLINK:
				Result=MkHardLink(Src,Dest);
				break;
			case FLINK_JUNCTION:
			case FLINK_VOLMOUNT:
			case FLINK_SYMLINKFILE:
			case FLINK_SYMLINKDIR:
				ReparsePointTypes LinkType=RP_JUNCTION;
				switch(Op)
				{
					case FLINK_VOLMOUNT:
						LinkType=RP_VOLMOUNT;
						break;
					case FLINK_SYMLINKFILE:
						LinkType=RP_SYMLINKFILE;
						break;
					case FLINK_SYMLINKDIR:
						LinkType=RP_SYMLINKDIR;
						break;
				}
				Result=MkSymLink(Src,Dest,LinkType,(Flags&FLINK_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
		}
	}

	if(Result && !(Flags&FLINK_DONOTUPDATEPANEL))
		ShellUpdatePanels(NULL,FALSE);

	return Result;
}

void NormalizeSymlinkName(string &strLinkName)
{
	if(!StrCmpN(strLinkName,L"\\??\\",4))
	{
		if(IsNetworkPath(strLinkName) || IsLocalVolumePath(strLinkName))
		{
			LPWSTR LinkName=strLinkName.GetBuffer();
			LinkName[1]=L'\\';
			strLinkName.ReleaseBuffer();
		}
		else
		{
			strLinkName.LShift(4);
		}
	}
}

// Кусок для создания SymLink для каталогов.
int MkSymLink(const wchar_t *SelName,const wchar_t *Dest,ReparsePointTypes LinkType,DWORD Flags)
{
	if(SelName && *SelName && Dest && *Dest)
  {
    string strSrcFullName, strDestFullName, strSelOnlyName;
    string strMsgBuf, strMsgBuf2;

    // выделим имя
    strSelOnlyName = SelName;

    DeleteEndSlash(strSelOnlyName);

		const wchar_t *PtrSelName=LastSlash(strSelOnlyName);

    if(!PtrSelName)
      PtrSelName=strSelOnlyName;
    else
      ++PtrSelName;

		if(SelName[1] == L':' && (SelName[2] == 0 || (IsSlash(SelName[2]) && SelName[3] == 0))) // C: или C:/
    {
//      if(Flags&FCOPY_VOLMOUNT)
      {
        strSrcFullName = SelName;
        AddEndSlash(strSrcFullName);
      }
      /*
        Вот здесь - ну очень умное поведение!
        Т.е. если в качестве SelName передали "C:", то в этом куске происходит
        коррекция типа линка - с symlink`а на volmount
      */
      LinkType=RP_VOLMOUNT;
    }
    else
      ConvertNameToFull(SelName,strSrcFullName);

    ConvertNameToFull(Dest,strDestFullName);

		if(IsSlash(strDestFullName.At(strDestFullName.GetLength()-1)))
    {
      if(LinkType!=RP_VOLMOUNT)
        strDestFullName += PtrSelName;
      else
      {
				const wchar_t Tmp[]={L'D',L'i',L's',L'k',L'_',*SelName,L'\0'};
				strDestFullName+=Tmp;
      }
    }

    if(LinkType==RP_VOLMOUNT)
    {
      AddEndSlash(strSrcFullName);
      AddEndSlash(strDestFullName);
    }

		DWORD JSAttr=apiGetFileAttributes(strDestFullName);
    if (JSAttr != INVALID_FILE_ATTRIBUTES) // Существует такой?
    {
      if((JSAttr&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                MSG(MCopyCannotCreateJunctionToFile),
                strDestFullName,MSG(MOk));
        }
        return 0;
      }

      if(CheckFolder(strDestFullName) == CHKFLD_NOTEMPTY) // а пустой?
      {
        // не пустой, ну что же, тогда пробуем сделать dest\srcname
        AddEndSlash(strDestFullName);
        if(LinkType==RP_VOLMOUNT)
        {
          string strTmpName;
          strTmpName.Format (MSG(MCopyMountName),*SelName);

          strDestFullName += strTmpName;
          AddEndSlash(strDestFullName);
        }
        else
          strDestFullName += PtrSelName;

				JSAttr=apiGetFileAttributes(strDestFullName);

        if(JSAttr != INVALID_FILE_ATTRIBUTES) // И такой тоже есть???
        {
          if(CheckFolder(strDestFullName) == CHKFLD_NOTEMPTY) // а пустой?
          {
            if(!(Flags&FCOPY_NOSHOWMSGLINK))
            {
              if(LinkType==RP_VOLMOUNT)
              {
                strMsgBuf.Format (MSG(MCopyMountVolFailed), SelName);
                strMsgBuf2.Format (MSG(MCopyMountVolFailed2), (const wchar_t *)strDestFullName);
                Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                   strMsgBuf,
                   strMsgBuf2,
                   MSG(MCopyFolderNotEmpty),
                   MSG(MOk));
              }
              else
                Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                      MSG(MCopyCannotCreateLink),strDestFullName,
                      MSG(MCopyFolderNotEmpty),MSG(MOk));
            }
            return 0; // однозначно в морг
          }
        }
        else // создаем.
        {
					if (apiCreateDirectory(strDestFullName,NULL))
            TreeList::AddTreeName(strDestFullName);
          else
            CreatePath(strDestFullName);
        }
				if(apiGetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // так, все очень даже плохо.
        {
          if(!(Flags&FCOPY_NOSHOWMSGLINK))
          {
            Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                      MSG(MCopyCannotCreateFolder),
                      strDestFullName,MSG(MOk));
          }
          return 0;
        }
      }
    }
    else
    {
			if(LinkType==RP_SYMLINKFILE || LinkType==RP_SYMLINKDIR)
			{
				// в этом случае создается путь, но не сам каталог
				string strPath=strDestFullName;
				if(CutToSlash(strPath))
				{
					if(apiGetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
						CreatePath(strPath);
				}
			}
			else
			{
				bool CreateDir=true;
				if(LinkType==RP_EXACTCOPY)
				{
					// в этом случае создается или каталог, или пустой файл
					DWORD dwSrcAttr=apiGetFileAttributes(strSrcFullName);
					if(dwSrcAttr!=INVALID_FILE_ATTRIBUTES && !(dwSrcAttr&FILE_ATTRIBUTE_DIRECTORY))
						CreateDir=false;
				}
				if(CreateDir)
				{
					if (apiCreateDirectory(strDestFullName,NULL))
						TreeList::AddTreeName(strDestFullName);
					else
						CreatePath(strDestFullName);
				}
				else
				{
					string strPath=strDestFullName;
					if(CutToSlash(strPath))
					{
						// создаём
						if(apiGetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
							CreatePath(strPath);
						HANDLE hFile=apiCreateFile(strDestFullName,0,0,0,CREATE_NEW,apiGetFileAttributes(strSrcFullName));
						if(hFile!=INVALID_HANDLE_VALUE)
						{
							CloseHandle(hFile);
						}
					}
				}
				if(apiGetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // так. все очень даже плохо.
				{
					if(!(Flags&FCOPY_NOSHOWMSGLINK))
					{
						Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
										 MSG(MCopyCannotCreateLink),strDestFullName,MSG(MOk));
					}
					return 0;
				}
			}
		}
		if(LinkType!=RP_VOLMOUNT)
    {
      if(CreateReparsePoint(strSrcFullName,strDestFullName,LinkType))
      {
        return 1;
      }
      else
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
					Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                 MSG(MCopyCannotCreateLink),strDestFullName,MSG(MOk));
        }
        return 0;
      }
    }
    else
    {
      if(CreateVolumeMountPoint(strSrcFullName,strDestFullName))
      {
        return 1;
      }
      else
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
					strMsgBuf.Format(MSG(MCopyMountVolFailed),SelName);
					strMsgBuf2.Format(MSG(MCopyMountVolFailed2),strDestFullName.CPtr());
					Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgBuf,strMsgBuf2,MSG(MOk));
        }
        return 0;
      }
    }
  }
  return 2;
}

