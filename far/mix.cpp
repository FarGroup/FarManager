/*
mix.cpp

Куча разных вспомогательных функций
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

#include "plugapi.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "chgprior.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "constitle.hpp"
#include "udlist.hpp"
#include "manager.hpp"
#include "lockscrn.hpp"
#include "lasterror.hpp"
#include "RefreshFrameManager.hpp"
#include "filefilter.hpp"
#include "imports.hpp"
#include "TPreRedrawFunc.hpp"
#include "nsUniversalDetectorEx.h"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"

BOOL FarChDir(const wchar_t *NewDir, BOOL ChangeDir)
{
  if(!NewDir || *NewDir == 0)
    return FALSE;

  BOOL rc=FALSE;
  wchar_t Drive[4]=L"=A:";

  string strCurDir;

	if(*NewDir && NewDir[1]==L':' && NewDir[2]==0)// если указана только
  {                                                     // буква диска, то путь
    Drive[1]=Upper(*NewDir);                          // возьмем из переменной

    if ( !apiGetEnvironmentVariable (Drive, strCurDir) )
    {
      strCurDir = NewDir;
      AddEndSlash(strCurDir);
      ReplaceSlashToBSlash(strCurDir);
    }
    //*CurDir=toupper(*CurDir); бред!
    if(ChangeDir)
    {
			rc=apiSetCurrentDirectory(strCurDir);
    }
  }
  else
  {
    if(ChangeDir)
    {
		strCurDir = NewDir;

		if(!StrCmp(strCurDir,L"\\"))
			apiGetCurrentDirectory(strCurDir); // здесь берем корень

		ReplaceSlashToBSlash(strCurDir);
		apiGetFullPathName(NewDir,strCurDir);
		PrepareDiskPath(strCurDir,FALSE); // TRUE ???
		rc=apiSetCurrentDirectory(strCurDir);
    }
  }

  if(rc || !ChangeDir)
  {
		if ((!ChangeDir || apiGetCurrentDirectory(strCurDir)) &&
			strCurDir.At(0) && strCurDir.At(1)==L':')
    {
			Drive[1]=Upper(strCurDir.At(0));
			SetEnvironmentVariableW(Drive,strCurDir);
    }
  }
  return rc;
}

int ToPercent(unsigned long N1,unsigned long N2)
{
  if (N1 > 10000)
  {
    N1/=100;
    N2/=100;
  }
  if (N2==0)
    return(0);
  if (N2<N1)
    return(100);
  return((int)(N1*100/N2));
}

int ToPercent64(unsigned __int64 N1, unsigned __int64 N2)
{
  if (N1 > _ui64(10000))
  {
    N1/=_ui64(100);
    N2/=_ui64(100);
  }
  if (N2==_ui64(0))
    return(_ui64(0));
  if (N2<N1)
    return(100);
  return((int)(N1*_ui64(100)/N2));
}



/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName (const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
  int skippath=flags&PN_SKIPPATH;
  flags &= ~PN_SKIPPATH;

  if (flags == PN_CMPNAME)
    return CmpName (param1, param2, skippath);

  if (flags == PN_CMPNAMELIST)
  {
    int Found=FALSE;
    string strFileMask;
    const wchar_t *MaskPtr;
    MaskPtr=param1;

    while ((MaskPtr=GetCommaWord(MaskPtr,strFileMask))!=NULL)
    {
      if (CmpName(strFileMask,param2,skippath))
      {
        Found=TRUE;
        break;
      }
    }
    return Found;
  }

  if (flags&PN_GENERATENAME)
  {
    string strResult;

    int nResult = ConvertWildcards(param1, strResult, (flags&0xFFFF)|(skippath?PN_SKIPPATH:0));

    xwcsncpy(param2, strResult, size); //?? а разве не size-1

    return nResult;
  }

  return FALSE;
}


/*
  Функция CheckFolder возвращает одно состояний тестируемого каталога:

    CHKFLD_NOTFOUND   (2) - нет такого
    CHKFLD_NOTEMPTY   (1) - не пусто
    CHKFLD_EMPTY      (0) - пусто
    CHKFLD_NOTACCESS (-1) - нет доступа
    CHKFLD_ERROR     (-2) - ошибка (параметры - дерьмо или нехватило памяти для выделения промежуточных буферов)
*/

int CheckFolder(const wchar_t *Path)
{
  if(!(Path && *Path)) // проверка на вшивость
    return CHKFLD_ERROR;

  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  int Done=FALSE;

  string strFindPath = Path;

  // сообразим маску для поиска.
  AddEndSlash(strFindPath);

  strFindPath += L"*.*";

  // первая проверка - че-нить считать можем?
  if((FindHandle=apiFindFirstFile(strFindPath,&fdata)) == INVALID_HANDLE_VALUE)
  {
    GuardLastError lstError;
    if(lstError.Get() == ERROR_FILE_NOT_FOUND)
      return CHKFLD_EMPTY;

    // собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
    // поэтому посмотрим на Root
    GetPathRootOne(Path,strFindPath);

    if(!StrCmp(Path,strFindPath))
    {
      // проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
			if(apiGetFileAttributes(strFindPath)!=INVALID_FILE_ATTRIBUTES)
      {
        if(lstError.Get() == ERROR_ACCESS_DENIED)
          return CHKFLD_NOTACCESS;
        return CHKFLD_EMPTY;
      }
    }

    strFindPath = Path;

    if(CheckShortcutFolder(&strFindPath,FALSE,TRUE))
    {
      if(StrCmp(Path,strFindPath))
        return CHKFLD_NOTFOUND;
    }

    return CHKFLD_NOTACCESS;
  }

  // Ок. Что-то есть. Попробуем ответить на вопрос "путой каталог?"
  while(!Done)
  {
    if (fdata.strFileName.At(0) == L'.' && (fdata.strFileName.At(1) == 0 || (fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)))
      ; // игнорируем "." и ".."
    else
    {
      // что-то есть, отличное от "." и ".." - каталог не пуст
      apiFindClose(FindHandle);
      return CHKFLD_NOTEMPTY;
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  // однозначно каталог пуст
  apiFindClose(FindHandle);
  return CHKFLD_EMPTY;
}

/* $ 30.07.2001 IS
     1. Проверяем правильность параметров.
     2. Теперь обработка каталогов не зависит от маски файлов
     3. Маска может быть стандартного фаровского вида (со скобками,
        перечислением и пр.). Может быть несколько масок файлов, разделенных
        запятыми или точкой с запятой, можно указывать маски исключения,
        можно заключать маски в кавычки. Короче, все как и должно быть :-)
*/
void WINAPI FarRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNCW Func,DWORD Flags,void *Param)
{
  if(Func && InitDir && *InitDir && Mask && *Mask)
  {
    CFileMask FMask;
    if(!FMask.Set(Mask, FMF_SILENT)) return;

    Flags=Flags&0x000000FF; // только младший байт!
    ScanTree ScTree(Flags & FRS_RETUPDIR,Flags & FRS_RECUR, Flags & FRS_SCANSYMLINK);
    FAR_FIND_DATA_EX FindData;

    string strFullName;

    ScTree.SetFindPath(InitDir,L"*");
    while (ScTree.GetNextName(&FindData,strFullName))
    {
      if ( FMask.Compare(FindData.strFileName) || FMask.Compare(FindData.strAlternateFileName) )
      {
          FAR_FIND_DATA fdata;

          apiFindDataExToData (&FindData, &fdata);

          if ( Func(&fdata,strFullName,Param) == 0)
          {
            apiFreeFindData(&fdata);
            break;
          }

          apiFreeFindData(&fdata);
      }
    }
  }
}

/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
    Dest - приемник результата (должен быть достаточно большим, например NM
    Template - шаблон по правилам функции mktemp, например "FarTmpXXXXXX"
   Вернет либо NULL, либо указатель на Dest.
*/
wchar_t* __stdcall FarMkTemp (wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
    string strDest;

    if ( FarMkTempEx(strDest, Prefix, TRUE) )
    {
				xwcsncpy (Dest, strDest, size-1);
        return Dest;
    }

    return NULL;
}

/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
string& FarMkTempEx(string &strDest, const wchar_t *Prefix, BOOL WithPath)
{
  if(!(Prefix && *Prefix))
    Prefix=L"FTMP";

  string strPath = L".";
  if(WithPath)
      strPath = Opt.strTempPath;

  wchar_t *lpwszDest = strDest.GetBuffer (StrLength(Prefix)+strPath.GetLength()+13);

  UINT uniq = GetCurrentProcessId(), savePid = uniq;
  for(;;) {
    if(!uniq) ++uniq;
    if(   GetTempFileNameW (strPath, Prefix, uniq, lpwszDest)
			&& apiGetFileAttributes (lpwszDest) == INVALID_FILE_ATTRIBUTES) break;
    if(++uniq == savePid) {
      *lpwszDest = 0;
      break;
    }
  }

  strDest.ReleaseBuffer ();

  return strDest;
}


/*
  FarGetLogicalDrives
  оболочка вокруг GetLogicalDrives, с учетом скрытых логических дисков
  HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer
  NoDrives:DWORD
    Последние 26 бит определяют буквы дисков от A до Z (отсчет справа налево).
    Диск виден при установленном 0 и скрыт при значении 1.
    Диск A представлен правой последней цифрой при двоичном представлении.
    Например, значение 00000000000000000000010101(0x7h)
    скрывает диски A, C, и E
*/
DWORD WINAPI FarGetLogicalDrives(void)
{
  static DWORD LogicalDrivesMask = 0;
  DWORD NoDrives=0;
  if ((!Opt.RememberLogicalDrives) || (LogicalDrivesMask==0))
    LogicalDrivesMask=GetLogicalDrives();

  if(!Opt.Policies.ShowHiddenDrives)
  {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS && hKey)
    {
      int ExitCode;
      DWORD Type,Size=sizeof(NoDrives);
      ExitCode=RegQueryValueExW(hKey,L"NoDrives",0,&Type,(BYTE *)&NoDrives,&Size);
      RegCloseKey(hKey);
      if(ExitCode != ERROR_SUCCESS)
        NoDrives=0;
    }
  }
  return LogicalDrivesMask&(~NoDrives);
}

// Преобразование корявого формата PATHEXT в ФАРовский :-)
// Функции передается нужные расширения, она лишь добавляет то, что есть
// в %PATHEXT%
// IS: Сравнений на совпадение очередной маски с тем, что имеется в Dest
// IS: не делается, т.к. дубли сами уберутся при компиляции маски
string &Add_PATHEXT(string &strDest)
{
  string strBuf;
  size_t curpos=strDest.GetLength()-1;
  UserDefinedList MaskList(0,0,ULF_UNIQUE);
  if( apiGetEnvironmentVariable(L"PATHEXT",strBuf) && MaskList.Set(strBuf))
  {
    /* $ 13.10.2002 IS проверка на '|' (маски исключения) */
    if( !strDest.IsEmpty() && strDest.At(curpos)!=L',' && strDest.At(curpos)!=L'|')
      strDest += L",";
    const wchar_t *Ptr;
    MaskList.Reset();
    while(NULL!=(Ptr=MaskList.GetNext()))
    {
      strDest += L"*";
      strDest += Ptr;
      strDest += L",";
    }
  }
  // лишняя запятая - в морг!
  /* $ 13.10.2002 IS Оптимизация по скорости */
  curpos=strDest.GetLength()-1;
  if(strDest.At(curpos) == L',')
    strDest.SetLength(curpos);
  return strDest;
}



/*
   Проверка пути или хост-файла на существование
   Если идет проверка пути (IsHostFile=FALSE), то будет
   предпринята попытка найти ближайший путь. Результат попытки
   возвращается в переданном TestPath.

   Return: 0 - бЯда.
           1 - ОБИ!,
          -1 - Почти что ОБИ, но ProcessPluginEvent вернул TRUE
   TestPath может быть пустым, тогда просто исполним ProcessPluginEvent()

*/

int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent)
{
	if( pTestPath && !pTestPath->IsEmpty() && apiGetFileAttributes(*pTestPath) == INVALID_FILE_ATTRIBUTES)
  {
    int FoundPath=0;

    string strTarget = *pTestPath;

    TruncPathStr(strTarget, ScrX-16);

    if(IsHostFile)
    {
      SetLastError(ERROR_FILE_NOT_FOUND);
      if(!Silent)
        Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG (MError), strTarget, MSG (MOk));
    }
    else // попытка найти!
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      if(Silent || Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG (MError), strTarget, MSG (MNeedNearPath), MSG(MHYes),MSG(MHNo)) == 0)
      {
        string strTestPathTemp = *pTestPath;

        while ( true )
        {
					if (!CutToSlash(strTestPathTemp,true))
						break;

					if(apiGetFileAttributes(strTestPathTemp) != INVALID_FILE_ATTRIBUTES)
					{
						int ChkFld=CheckFolder(strTestPathTemp);
						if(ChkFld > CHKFLD_ERROR && ChkFld < CHKFLD_NOTFOUND)
						{
							if(!(pTestPath->At(0) == L'\\' && pTestPath->At(1) == L'\\' && strTestPathTemp.At(1) == 0))
							{
								*pTestPath = strTestPathTemp;

								if( pTestPath->GetLength() == 2) // для случая "C:", иначе попадем в текущий каталог диска C:
									AddEndSlash(*pTestPath);
								FoundPath=1;
							}
							break;
						}
					}
        }
      }
    }
    if(!FoundPath)
      return 0;
  }
  if(CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return -1;
  return 1;
}

void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
  if(!SrcPanel)
    SrcPanel=CtrlObject->Cp()->ActivePanel;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  switch ( SrcPanel->GetType() ) {
    case QVIEW_PANEL:
    case INFO_PANEL:
      SrcPanel=CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
  }

  int AnotherType=AnotherPanel->GetType();

  if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
  {
    if(NeedSetUpADir)
    {
      string strCurDir;
      SrcPanel->GetCurDir(strCurDir);
      AnotherPanel->SetCurDir(strCurDir,TRUE);
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    }
    else
    {
      // TODO: ???
      //if(AnotherPanel->NeedUpdatePanel(SrcPanel))
      //  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
      //else
      {
        // Сбросим время обновления панели. Если там есть нотификация - обновится сама.
        if (AnotherType==FILE_PANEL)
          ((FileList *)AnotherPanel)->ResetLastUpdateTime();
        AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
      }
    }
  }
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  if (AnotherType==QVIEW_PANEL)
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName)
{
  if(!SrcPanel)
    SrcPanel=CtrlObject->Cp()->ActivePanel;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->CloseFile();
  if(AnotherPanel->GetMode() == NORMAL_PANEL)
  {
    string strAnotherCurDir;
    string strFullName;

    AnotherPanel->GetCurDir(strAnotherCurDir);
    AddEndSlash(strAnotherCurDir);

    ConvertNameToFull(SelName, strFullName);
    AddEndSlash(strFullName);

    if(wcsstr(strAnotherCurDir,strFullName))
    {
      ((FileList*)AnotherPanel)->CloseChangeNotification();
      return TRUE;
    }
  }
  return FALSE;
}

/* $ 21.09.2003 KM
   Трансформация строки по заданному типу.
*/
void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType)
{
	string strTemp;
  switch(TransformType)
  {
    case L'X': // Convert common string to hexadecimal string representation
    {
			string strHex;
      while(*ConvStr)
      {
				strHex.Format(L"%02X",*ConvStr);
				strTemp += strHex;
        ConvStr++;
      }
      break;
    }
    case L'S': // Convert hexadecimal string representation to common string
    {
      const wchar_t *ptrConvStr=ConvStr;
      while(*ptrConvStr)
      {
        if(*ptrConvStr != L' ')
        {
					WCHAR Hex[]={ptrConvStr[0],ptrConvStr[1],0};
					size_t l=strTemp.GetLength();
					wchar_t *Temp=strTemp.GetBuffer(l+2);
					Temp[l]=(wchar_t)wcstoul(Hex,NULL,16)&0xFFFF;
					strTemp.ReleaseBuffer(l+1);
          ptrConvStr++;
        }
        ptrConvStr++;
      }
      break;
    }
    default:
      break;
  }
  strBuffer=strTemp;
}

int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType)
{
  string strSrcRoot, strDestRoot;
  int SrcDriveType, DestDriveType;

  DWORD SrcVolumeNumber=0, DestVolumeNumber=0;
  string strSrcVolumeName, strDestVolumeName;
  string strSrcFileSystemName, strDestFileSystemName;
  DWORD SrcFileSystemFlags, DestFileSystemFlags;
  DWORD SrcMaximumComponentLength, DestMaximumComponentLength;

  strSrcRoot=SrcPath;
  strDestRoot=DestPath;
  ConvertNameToUNC(strSrcRoot);
  ConvertNameToUNC(strDestRoot);
  GetPathRoot(strSrcRoot,strSrcRoot);
  GetPathRoot(strDestRoot,strDestRoot);

  SrcDriveType=FAR_GetDriveType(strSrcRoot,NULL,TRUE);
  DestDriveType=FAR_GetDriveType(strDestRoot,NULL,TRUE);

  if (!apiGetVolumeInformation(strSrcRoot,&strSrcVolumeName,&SrcVolumeNumber,&SrcMaximumComponentLength,&SrcFileSystemFlags,&strSrcFileSystemName))
    return(FALSE);

  if (!apiGetVolumeInformation(strDestRoot,&strDestVolumeName,&DestVolumeNumber,&DestMaximumComponentLength,&DestFileSystemFlags,&strDestFileSystemName))
    return(FALSE);

  if(CheckedType == CHECKEDPROPS_ISSAMEDISK)
  {
    if (wcspbrk(DestPath,L"\\:")==NULL)
      return TRUE;

    if (((strSrcRoot.At(0)==L'\\' && strSrcRoot.At(1)==L'\\') || (strDestRoot.At(0)==L'\\' && strDestRoot.At(1)==L'\\')) &&
        StrCmpI(strSrcRoot,strDestRoot)!=0)
      return FALSE;

    if ( *SrcPath == 0 || *DestPath == 0 || (SrcPath[1]!=L':' && DestPath[1]!=L':')) //????
      return TRUE;

    if (Upper(strDestRoot.At(0))==Upper(strSrcRoot.At(0)))
        return TRUE;

    unsigned __int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
    unsigned __int64 DestTotalSize,DestTotalFree,DestUserFree;

		if (!apiGetDiskSize(SrcPath,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
      return FALSE;
		if (!apiGetDiskSize(DestPath,&DestTotalSize,&DestTotalFree,&DestUserFree))
      return FALSE;

    if (!(SrcVolumeNumber!=0 &&
        SrcVolumeNumber==DestVolumeNumber &&
        StrCmpI(strSrcVolumeName, strDestVolumeName)==0 &&
        SrcTotalSize==DestTotalSize))
      return FALSE;
  }

  else if(CheckedType == CHECKEDPROPS_ISDST_ENCRYPTION)
  {
    if(!(DestFileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
      return FALSE;
    if(!(DestDriveType==DRIVE_REMOVABLE || DestDriveType==DRIVE_FIXED || DestDriveType==DRIVE_REMOTE))
      return FALSE;
  }

  return TRUE;
}

bool IsTextUTF8(const LPBYTE Buffer,size_t Length)
{
	bool Ascii=true;
	UINT Octets=0;
	for(size_t i=0;i<Length;i++)
	{
		BYTE c=Buffer[i];
		if(c&0x80)
			Ascii=false;
		if(Octets)
		{
			if((c&0xC0)!=0x80)
				return false;
			Octets--;
		}
		else
		{
			if(c&0x80)
			{
				while(c&0x80)
				{
					c<<=1;
					Octets++;
				}
				Octets--;
				if(!Octets)
					return false;
			}
		}
	}
	return (Octets>0||Ascii)?false:true;
}

bool GetFileFormat (FILE *file, UINT &nCodePage, bool *pSignatureFound, bool bUseHeuristics)
{
	DWORD dwTemp=0;

	bool bSignatureFound = false;
	bool bDetect=false;

	if ( fread (&dwTemp, 1, 4, file) )
	{
		if ( LOWORD (dwTemp) == SIGN_UNICODE )
		{
			nCodePage = CP_UNICODE;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( LOWORD (dwTemp) == SIGN_REVERSEBOM )
		{
			nCodePage = CP_REVERSEBOM;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( (dwTemp & 0x00FFFFFF) == SIGN_UTF8 )
		{
			nCodePage = CP_UTF8;
			fseek (file, 3, SEEK_SET);
			bSignatureFound = true;
		}
		else
			fseek (file, 0, SEEK_SET);
	}

	if( bSignatureFound )
	{
		bDetect = true;
	}
	else

	if ( bUseHeuristics )
	{
		fseek (file, 0, SEEK_SET);
		size_t sz=0x8000; // BUGBUG. TODO: configurable
		LPVOID Buffer=xf_malloc(sz);
		sz=fread(Buffer,1,sz,file);
		fseek (file,0,SEEK_SET);

		if ( sz )
		{
			int test=
				IS_TEXT_UNICODE_STATISTICS|
				IS_TEXT_UNICODE_REVERSE_STATISTICS|
				IS_TEXT_UNICODE_CONTROLS|
				IS_TEXT_UNICODE_REVERSE_CONTROLS|
				IS_TEXT_UNICODE_ILLEGAL_CHARS|
				IS_TEXT_UNICODE_ODD_LENGTH|
				IS_TEXT_UNICODE_NULL_BYTES;

			if ( IsTextUnicode (Buffer, (int)sz, &test) )
			{
				if ( !(test&IS_TEXT_UNICODE_ODD_LENGTH) && !(test&IS_TEXT_UNICODE_ILLEGAL_CHARS) )
				{
					if( (test&IS_TEXT_UNICODE_NULL_BYTES) ||
						(test&IS_TEXT_UNICODE_CONTROLS) ||
						(test&IS_TEXT_UNICODE_REVERSE_CONTROLS) )
					{
						if ( (test&IS_TEXT_UNICODE_CONTROLS) || (test&IS_TEXT_UNICODE_STATISTICS) )
						{
							nCodePage=CP_UNICODE;
							bDetect=true;
						}
						else

						if ( (test&IS_TEXT_UNICODE_REVERSE_CONTROLS) || (test&IS_TEXT_UNICODE_REVERSE_STATISTICS) )
						{
							nCodePage=CP_REVERSEBOM;
							bDetect=true;
						}
					}
				}
			}
			else

			if ( IsTextUTF8 ((const LPBYTE)Buffer, sz) )
			{
				nCodePage=CP_UTF8;
				bDetect=true;
			}
			else
			{
				nsUniversalDetectorEx *ns = new nsUniversalDetectorEx();

				ns->HandleData((const char*)Buffer,(PRUint32)sz);
				ns->DataEnd();

				int cp = ns->getCodePage();

				if ( cp != -1 )
				{
					nCodePage = cp;
					bDetect = true;
				}

				delete ns;

			}
		}

		xf_free(Buffer);
	}

	if ( pSignatureFound )
		*pSignatureFound = bSignatureFound;

	return bDetect;
}

bool IsDriveTypeRemote(UINT DriveType)
{
	return DriveType == DRIVE_REMOTE || DriveType == DRIVE_REMOTE_NOT_CONNECTED;
}
