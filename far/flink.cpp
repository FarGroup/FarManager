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
#include "lasterror.hpp"
#include "cddrv.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "panelmix.hpp"

struct TMN_REPARSE_DATA_BUFFER
{
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
  union {
    struct {
      WORD   SubstituteNameOffset;
      WORD   SubstituteNameLength;
      WORD   PrintNameOffset;
      WORD   PrintNameLength;
      ULONG  Flags;
      WCHAR PathBuffer[1];
    } SymbolicLinkReparseBuffer;
    struct {
      WORD   SubstituteNameOffset;
      WORD   SubstituteNameLength;
      WORD   PrintNameOffset;
      WORD   PrintNameLength;
      WCHAR PathBuffer[1];
    } MountPointReparseBuffer;
    struct {
      BYTE   DataBuffer[1];
    } GenericReparseBuffer;
  };
};

#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, GenericReparseBuffer)

/*
 SrcVolume
   Pointer to a string that indicates the volume mount point where the
   volume is to be mounted. This may be a root directory (X:\) or a
   directory on a volume (X:\mnt\). The string must end with a trailing
   backslash ('\').
*/

int WINAPI CreateVolumeMountPoint(const wchar_t *SrcVolume, const wchar_t *LinkFolder)
{
  wchar_t Buf[50]; //MS says 50           // temporary buffer for volume name

  // We should do some error checking on the inputs. Make sure
  // there are colons and backslashes in the right places, etc.

  if ( !GetVolumeNameForVolumeMountPointW(SrcVolume, Buf, 50) )
    return 1;

  if ( !SetVolumeMountPointW(LinkFolder, Buf) ) // volume to be mounted
    return 2;

  return 0;
}


BOOL WINAPI CreateReparsePoint(const wchar_t *SrcFolder, const wchar_t *LinkFolder,DWORD Type)
{
  if (!LinkFolder || !SrcFolder || !*LinkFolder || !*SrcFolder)
    return FALSE;

  string strDestDir;

  if (SrcFolder[0] == L'\\' && SrcFolder[1] == L'?')
     strDestDir = SrcFolder;
  else
  {
    string strFullDir;

    if(Type==RP_JUNCTION)
      strDestDir = L"\\??\\";

    ConvertNameToFull (SrcFolder, strFullDir); //??? было GetFullPathName

    if ( apiGetFileAttributes (strFullDir) == INVALID_FILE_ATTRIBUTES )
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
    }

    const wchar_t *PtrFullDir=(const wchar_t*)strFullDir;
    // проверка на subst
    if(IsLocalPath(strFullDir))
    {
      wchar_t LocalName[8];

      string strSubstName;

      swprintf (LocalName,L"%c:",*(const wchar_t*)strFullDir);

      if(GetSubstName(DRIVE_NOT_INIT,LocalName, strSubstName))
      {
        strDestDir += strSubstName;
        AddEndSlash(strDestDir);
        PtrFullDir=(const wchar_t*)strFullDir+3;
      }
    }
    else if(IsLocalPrefixPath(strFullDir) || IsLocalVolumePath(strFullDir))
      PtrFullDir=(const wchar_t*)strFullDir+4;
    strDestDir += PtrFullDir;
  }

  switch(Type)
  {
  case RP_EXACTCOPY:
    return DuplicateReparsePoint(strDestDir,LinkFolder);
  case RP_SYMLINKFILE:
  case RP_SYMLINKDIR:
    {
      if(ifn.pfnCreateSymbolicLink)
        return apiCreateSymbolicLink(LinkFolder,strDestDir,Type==RP_SYMLINKDIR?SYMBOLIC_LINK_FLAG_DIRECTORY:0);
      else
        return FALSE;
    }
    break;
  case RP_JUNCTION:
    {
      char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
      TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;

      WORD nDestMountPointBytes = (WORD)(strDestDir.GetLength()*sizeof (wchar_t));
      rdb.ReparseTag            = IO_REPARSE_TAG_MOUNT_POINT;
      rdb.Reserved              = 0;
      rdb.MountPointReparseBuffer.SubstituteNameOffset = 0;
      rdb.MountPointReparseBuffer.SubstituteNameLength = nDestMountPointBytes;
      rdb.MountPointReparseBuffer.PrintNameOffset      = rdb.MountPointReparseBuffer.SubstituteNameLength+2;
      rdb.MountPointReparseBuffer.PrintNameLength      = nDestMountPointBytes-4*sizeof(wchar_t);
      rdb.ReparseDataLength     = sizeof(rdb.MountPointReparseBuffer)+rdb.MountPointReparseBuffer.PrintNameOffset+rdb.MountPointReparseBuffer.PrintNameLength;
      if(rdb.ReparseDataLength+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)>MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof (wchar_t))
      {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
      }
      wcscpy(rdb.MountPointReparseBuffer.PathBuffer, strDestDir);
      wcscpy(&rdb.MountPointReparseBuffer.PathBuffer[rdb.MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)],&strDestDir[4]);

      HANDLE hDir=apiCreateFile(LinkFolder,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,
              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT);

      if (hDir == INVALID_HANDLE_VALUE)
      {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
      }
      DWORD dwBytes;
      if(!DeviceIoControl(hDir,
          FSCTL_SET_REPARSE_POINT,
          (LPVOID)&rdb,
          rdb.ReparseDataLength + TMN_REPARSE_DATA_BUFFER_HEADER_SIZE,
          NULL,
          0,
          &dwBytes,
          0))
      {
        DWORD LastErr=GetLastError();
        CloseHandle(hDir);
        apiDeleteFile(LinkFolder); // А нужно ли убивать, когда создали каталог, но симлинк не удалось ???
        SetLastError(LastErr);
        return 0;
      }
      CloseHandle(hDir);
      return TRUE;
    }
    break;
  }
  return FALSE;
}


BOOL WINAPI DeleteReparsePoint(const wchar_t *szDir)
{
  DWORD ReparseTag;
  string strTmp;
  GetReparsePointInfo(szDir,strTmp,&ReparseTag);
  HANDLE hDir=apiCreateFile(szDir,
          GENERIC_READ | GENERIC_WRITE,
          0,
          0,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT);
  if (hDir == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return FALSE;
  }

  REPARSE_GUID_DATA_BUFFER rgdb = { 0 };
  rgdb.ReparseTag = ReparseTag;
  DWORD dwBytes;
  const BOOL bOK =
    DeviceIoControl(hDir,
            FSCTL_DELETE_REPARSE_POINT,
            &rgdb,
            REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
            NULL,
            0,
            &dwBytes,
            0);
  CloseHandle(hDir);
  return bOK != 0;
}

bool GetREPARSE_DATA_BUFFER(const wchar_t *Object,TMN_REPARSE_DATA_BUFFER *rdb)
{
  bool RetVal=false;

  const DWORD FileAttr = apiGetFileAttributes(Object);
  /* $ 14.06.2003 IS
     Для нелокальных дисков получить корректную информацию о связи
     не представляется возможным
  */
  if(FileAttr!=INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && IsLocalDrive(Object))
  {
    HANDLE hObject=apiCreateFile(Object,0,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT);
    if (hObject!=INVALID_HANDLE_VALUE)
    {
      DWORD dwBytesReturned;
      if(DeviceIoControl(hObject,FSCTL_GET_REPARSE_POINT,NULL,0,(LPVOID)rdb,MAXIMUM_REPARSE_DATA_BUFFER_SIZE,&dwBytesReturned,0) && IsReparseTagValid(rdb->ReparseTag))
      {
        RetVal=true;
      }
      GuardLastError LastError;
      CloseHandle(hObject);
    }
  }
  return RetVal;
}

bool SetPrivilege(LPCWSTR Privilege,BOOL bEnable)
{
  bool RetVal=FALSE;
  HANDLE hToken=0;
  if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
  {
    TOKEN_PRIVILEGES tp;
    if(LookupPrivilegeValueW(NULL,Privilege,&tp.Privileges->Luid))
    {
      tp.PrivilegeCount=1;
      tp.Privileges->Attributes=bEnable?SE_PRIVILEGE_ENABLED:0;
      if(AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL))
      {
        RetVal=(GetLastError()==ERROR_SUCCESS);
      }
    }
    CloseHandle(hToken);
  }
  return RetVal;
}

DWORD WINAPI GetReparsePointInfo(const wchar_t *szMountDir, string &strDestBuff,LPDWORD lpReparseTag)
{
  char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;
  WORD SubstituteNameLength=0;
  if(GetREPARSE_DATA_BUFFER(szMountDir,&rdb))
  {
    const wchar_t *PathBuffer;
    if(lpReparseTag)
      *lpReparseTag=rdb.ReparseTag;
    if (rdb.ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
      SubstituteNameLength = rdb.SymbolicLinkReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
      PathBuffer = &rdb.SymbolicLinkReparseBuffer.PathBuffer[rdb.SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
    }
    else
    {
      SubstituteNameLength = rdb.MountPointReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
      PathBuffer = &rdb.MountPointReparseBuffer.PathBuffer[rdb.MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
    }
    wchar_t *lpwszDestBuff=strDestBuff.GetBuffer(SubstituteNameLength+1);
    wcsncpy(lpwszDestBuff,PathBuffer,SubstituteNameLength);
    strDestBuff.ReleaseBuffer(SubstituteNameLength);
  }
  return SubstituteNameLength;
}

int WINAPI GetNumberOfLinks(const wchar_t *Name)
{
  HANDLE hFile=apiCreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0);
  if (hFile==INVALID_HANDLE_VALUE)
    return(1);
  BY_HANDLE_FILE_INFORMATION bhfi;
  int GetCode=GetFileInformationByHandle(hFile,&bhfi);
  CloseHandle(hFile);
  return(GetCode ? bhfi.nNumberOfLinks:0);
}



int WINAPI MkHardLink(const wchar_t *Src,const wchar_t *Dest)
{
  string strFileSource,strFileDest;

  ConvertNameToFull(Src,strFileSource);
  ConvertNameToFull(Dest,strFileDest);

  return apiCreateHardLink(strFileDest, strFileSource, NULL) != 0;
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
                     OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
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
        lstrcpyW(pwsz, wszStreamName + sizeof(CHAR));
        LPWSTR wp = wcsstr(pwsz, L":");
        pwsz[wp-pwsz] = 0;
        lstrcpyW(wszStreamName, pwsz);
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
  bool Ret=false;
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
    Ret=true;
  }
  return Ret;
}

/*
   Функция DelSubstDrive - удаление Subst драйвера
   Return: -1 - это либо не SUBST-драйвер, либо OS не та.
            0 - все удалено на ура
            1 - ошибка при удалении.
*/
int DelSubstDrive(const wchar_t *DosDeviceName)
{
  string strNtDeviceName;
  if(GetSubstName(DRIVE_NOT_INIT, DosDeviceName, strNtDeviceName))
  {
    strNtDeviceName = (string)L"\\??\\"+strNtDeviceName;

    return !DefineDosDeviceW(DDD_RAW_TARGET_PATH|
                       DDD_REMOVE_DEFINITION|
                       DDD_EXACT_MATCH_ON_REMOVE,
                       DosDeviceName, strNtDeviceName)?1:0;
  }
  return(-1);
}

bool GetSubstName(int DriveType,const wchar_t *LocalName, string &strSubstName)
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
    if(IsLocalPath(LocalName))
    {
      wchar_t *Name=new wchar_t[NT_MAX_PATH];
      if(Name)
      {
        if (QueryDosDeviceW(LocalName,Name,NT_MAX_PATH))
        {
            if(!StrCmpN(Name,L"\\??\\",4))
            {
              strSubstName=Name+4;
              Ret=true;
            }
        }
        delete[] Name;
      }
    }
  }
  return Ret;
}

void GetPathRootOne(const wchar_t *Path,string &strRoot)
{
  string strTempRoot;

  strTempRoot = Path;

  // обработка mounted volume
  if(!StrCmpNI(Path,L"Volume{",7))
  {
    // For the maximum size of the volume ID
    // see http://msdn2.microsoft.com/en-us/library/aa364994(VS.85).aspx
    const DWORD MAX_VOLUME_ID = 50;
    wchar_t pVolumeName[MAX_VOLUME_ID];

    wchar_t szDrive[] = L"?:\\"; // \\?\Volume{...
    for (wchar_t chDrive = L'A'; chDrive <= L'Z';  chDrive++ )
    {
      *szDrive = chDrive;
      if ( GetVolumeNameForVolumeMountPointW(
              szDrive,            // input volume mount point or directory
              pVolumeName,        // output volume name buffer
              MAX_VOLUME_ID       // size of volume name buffer
           )
         )
      {
        if ( !StrCmpI(pVolumeName+4, Path) )  // +4 - for "\\?\"
        {
          strRoot = szDrive;
          return;
        }
      }
    }

    // Ops. Диск то не имеет буковки
    strRoot = L"\\\\?\\";
    strRoot += Path;
    return;
  }

  if ( strTempRoot.IsEmpty() )
    strTempRoot = L"\\";
  else
  {
    // ..2 <> ...\2
    if(!PathMayBeAbsolute(strTempRoot))
    {
      string strTemp;
      apiGetCurrentDirectory(strTemp);
      AddEndSlash(strTemp);
      strTemp += strTempRoot; //+(*TempRoot=='\\' || *TempRoot == '/'?1:0)); //??
      strTempRoot = strTemp;
    }

    const wchar_t *TempRoot = strTempRoot;
    const wchar_t *ChPtr = NULL;

    if (TempRoot[0]==L'\\' && TempRoot[1]==L'\\')
    {
      if ((ChPtr=FirstSlash(TempRoot+2))!=NULL)
      {
        const wchar_t *ChPtr2 = NULL;
        if ((ChPtr2=FirstSlash(ChPtr+1))!=NULL)
        {
          ChPtr=ChPtr2;
          strTempRoot.SetLength((ChPtr - TempRoot) + 1);
        }
        else
          strTempRoot += L"\\";
      }
    }
    else
    {
      if ((ChPtr=FirstSlash(TempRoot))!=NULL)
      {
        strTempRoot.SetLength((ChPtr - TempRoot) + 1);
      }
      else if ((ChPtr=wcschr(TempRoot,L':'))!=NULL)
      {
        strTempRoot.SetLength((ChPtr - TempRoot) + 1);
        strTempRoot += L"\\";
      }
    }
  }
  strRoot = strTempRoot;
}


// полный проход ПО!!!
void GetPathRoot(const wchar_t *Path, string &strRoot, int Reenter)
{
  string strTempRoot;
  string strNewPath;

  int IsUNC = FALSE;
  int PathLen = StrLength(Path);

  ConvertNameToFull(Path,strNewPath);
  // Проверим имя на UNC
  if (PathLen > 2 && Path[0] == L'\\' && Path[1] == L'\\')
  {
    if (PathLen > 3 && Path[2] == L'?' && Path[3] == L'\\' &&
        //"\\?\Volume{GUID}" не трогаем
        StrCmpNI(&Path[4],L"Volume{",7))
    { // Проверим на длинное UNC имя
      strNewPath = &Path[4];
      if (PathLen > 8 && StrCmpNI(strNewPath, L"UNC\\", 4)==0)
      {
        IsUNC = TRUE;
        strNewPath = L"\\";
        strNewPath += &Path[7];
      }
    }
    else
      IsUNC = TRUE;
  }

  DWORD FileAttr;
  string strJuncName;

  strTempRoot = strNewPath;

  size_t posCtlChar = 0; // позиция начала реального пути в UNC. Без имени сервера
  if (!IsUNC || strTempRoot.Pos(posCtlChar,L'\\',2))
  {
    size_t pos = 0;
    bool bFound = LastSlash(strTempRoot,pos);
    while (pos >= posCtlChar && strTempRoot.GetLength() > 2)
    {
      FileAttr=apiGetFileAttributes(strTempRoot);

      if (FileAttr != INVALID_FILE_ATTRIBUTES && FileAttr&FILE_ATTRIBUTE_DIRECTORY && FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
      {
        if (GetReparsePointInfo(strTempRoot,strJuncName))
        {
          if(IsLocalVolumePath(strJuncName))
            Reenter=TRUE;
          if (!StrCmpN(strJuncName,L"\\??\\",4))
            strJuncName.LShift(4);

          if (strJuncName.At(0) == L'.') //BUGBUG
          {
            //AY: вся эта заморочка во первых кривая (не всегда работает как надо)
            //    а во вторых нужна потому что в висте симлинки могут содержать относительный
            //    путь. Тут видимо надо как то заюзать GetFullPathName.
            string strTempJunc;
            if (bFound)
            {
              if (strTempRoot.GetLength() == pos+1)
              {
                strTempRoot.SetLength(pos);
                if (LastSlash(strTempRoot,pos))
                  strTempRoot.SetLength(pos);
              }
              else
              {
                strTempRoot.SetLength(pos);
              }
            }
            strJuncName=strTempRoot+strJuncName;
          }

          if(!Reenter)
            GetPathRoot(strJuncName,strRoot,TRUE);
          else
            GetPathRootOne(strJuncName,strRoot);

          return;
        }
      } /* if */

      if (bFound)
        strTempRoot.SetLength(pos);
      else
        break;

      bFound = LastSlash(strTempRoot,pos);
    } /* while */
  } /* if */

  GetPathRootOne(strNewPath, strRoot);
}


BOOL WINAPI CanCreateHardLinks(const wchar_t *TargetFile,const wchar_t *HardLinkName)
{
  if(!TargetFile)
    return FALSE;

  string strRoot1;
  string strRoot2;
  string strFSysName;

  GetPathRoot(TargetFile,strRoot1);

  if(HardLinkName)
    GetPathRoot(HardLinkName,strRoot2);
  else
    strRoot2 = strRoot1;

   // same drive?
  if( !StrCmp(strRoot1, strRoot2))
  {
    // NTFS drive?
    DWORD FileSystemFlags;
    if(apiGetVolumeInformation (strRoot1,NULL,NULL,NULL,&FileSystemFlags,&strFSysName))
    {
      if(!StrCmp(strFSysName,L"NTFS"))
        return TRUE;
    }
  }
  return FALSE;
}

bool DuplicateReparsePoint(const wchar_t *Src,const wchar_t *Dst)
{
  bool RetVal=false;
  char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;
  if(GetREPARSE_DATA_BUFFER(Src,&rdb))
  {
    if(rdb.ReparseTag==IO_REPARSE_TAG_SYMLINK)
      SetPrivilege(L"SeCreateSymbolicLinkPrivilege",TRUE);
    HANDLE hLink=apiCreateFile(Dst,FILE_WRITE_ATTRIBUTES,0,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT);
    if(hLink!=INVALID_HANDLE_VALUE)
    {
      DWORD dwBytes;
      RetVal=DeviceIoControl(hLink,FSCTL_SET_REPARSE_POINT,(LPVOID)&rdb,rdb.ReparseDataLength + TMN_REPARSE_DATA_BUFFER_HEADER_SIZE,NULL,0,&dwBytes,0)!=0;
      GuardLastError LastError;
      CloseHandle(hLink);
    }
  }
  return RetVal;
}

int WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
  int RetCode=0;

  if(Src && *Src && Dest && *Dest)
  {
//    int Delete=Flags&FLINK_DELETE;
    int Op=Flags&0xFFFF;

    switch(Op)
    {
      case FLINK_HARDLINK:
//        if(Delete)
//          RetCode=FAR_DeleteFile(Src);
//        else
          if(CanCreateHardLinks(Src,Dest))
            RetCode=MkHardLink(Src,Dest);
        break;
      case FLINK_JUNCTION:
      case FLINK_VOLMOUNT:
      case FLINK_SYMLINKFILE:
      case FLINK_SYMLINKDIR:
//        if(Delete)
//          RetCode=FAR_RemoveDirectory(Src);
//        else
        ReparsePointTypes LinkType=RP_JUNCTION;
        switch(Op)
        {
          case FLINK_VOLMOUNT:    LinkType=RP_VOLMOUNT;break;
          case FLINK_SYMLINKFILE: LinkType=RP_SYMLINKFILE;break;
          case FLINK_SYMLINKDIR:  LinkType=RP_SYMLINKDIR;break;
        }
        RetCode=ShellCopy::MkSymLink(Src,Dest,LinkType,(Flags&FLINK_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
    }
  }

  if(RetCode && !(Flags&FLINK_DONOTUPDATEPANEL))
    ShellUpdatePanels(NULL,FALSE);

  return RetCode;
}
