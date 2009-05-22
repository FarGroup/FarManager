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

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator)
{
  string strDigit;
  const wchar_t *PtrDigit;
  int I;

  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const wchar_t *Ptr=Src;
  while((Ptr=GetCommaWord(Ptr,strDigit,Separator)) != NULL)
  {
    PtrDigit=strDigit;
    while (*PtrDigit && !iswdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=_wtoi(PtrDigit);
    if(++I > 2) //не должно быть больше трёх чисел
      break;
  }
}

void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative)
{
  unsigned DateN[3],TimeN[3];
  SYSTEMTIME st;
  FILETIME lft;

  // Преобразуем введённые пользователем дату и время
  GetFileDateAndTime(CDate,DateN,DateSeparator);
  GetFileDateAndTime(CTime,TimeN,TimeSeparator);

  if (!bRelative)
  {
    if(DateN[0] == (unsigned)-1 || DateN[1] == (unsigned)-1 || DateN[2] == (unsigned)-1)
    {
      // Пользователь оставил дату пустой, значит обнулим дату и время.
      memset(&ft,0,sizeof(ft));
      return;
    }

    memset(&st,0,sizeof(st));

    // "Оформим"
    switch(DateFormat)
    {
      case 0:
        st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
        st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
        st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
        break;
      case 1:
        st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
        st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
        st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
        break;
      default:
        st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
        st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
        st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
        break;
    }

    if (st.wYear<100)
    {
      if (st.wYear<80)
        st.wYear+=2000;
      else
        st.wYear+=1900;
    }
  }
  else
  {
    st.wDay = DateN[0]!=(unsigned)-1?DateN[0]:0;
  }

  st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
  st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
  st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

  // преобразование в "удобоваримый" формат
  if (bRelative)
  {
    ULARGE_INTEGER time;

    time.QuadPart  = (unsigned __int64)st.wSecond * _ui64(10000000);
    time.QuadPart += (unsigned __int64)st.wMinute * _ui64(10000000) * _ui64(60);
    time.QuadPart += (unsigned __int64)st.wHour   * _ui64(10000000) * _ui64(60) * _ui64(60);
    time.QuadPart += (unsigned __int64)st.wDay    * _ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24);
    ft.dwLowDateTime  = time.u.LowPart;
    ft.dwHighDateTime = time.u.HighPart;
  }
  else
  {
    SystemTimeToFileTime(&st,&lft);
    LocalFileTimeToFileTime(&lft,&ft);
  }
}

void ConvertDate (const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear,int DynInit)
{
  static int WDateFormat,WDateSeparator,WTimeSeparator;
  static int Init=FALSE;
  static SYSTEMTIME lt;
  int DateFormat,DateSeparator,TimeSeparator;
  if (!Init)
  {
    WDateFormat=GetDateFormat();
    WDateSeparator=GetDateSeparator();
    WTimeSeparator=GetTimeSeparator();
    GetLocalTime(&lt);
    Init=TRUE;
  }
  DateFormat=DynInit?GetDateFormat():WDateFormat;
  DateSeparator=DynInit?GetDateSeparator():WDateSeparator;
  TimeSeparator=DynInit?GetTimeSeparator():WTimeSeparator;

  int CurDateFormat=DateFormat;
  if (Brief && CurDateFormat==2)
    CurDateFormat=0;

  SYSTEMTIME st;
  FILETIME ct;

  if (ft.dwHighDateTime==0)
  {
    strDateText=L"";
    strTimeText=L"";
    return;
  }

  FileTimeToLocalFileTime(&ft,&ct);
  FileTimeToSystemTime(&ct,&st);

  //if ( !strTimeText.IsEmpty() )
  {
    const wchar_t *Letter=L"";
    if (TimeLength==6)
    {
      Letter=(st.wHour<12) ? L"a":L"p";
      if (st.wHour>12)
        st.wHour-=12;
      if (st.wHour==0)
        st.wHour=12;
    }
    if (TimeLength<7)
      strTimeText.Format (L"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
    else
    {
      string strFullTime;
      strFullTime.Format (L"%02d%c%02d%c%02d.%03d",st.wHour,TimeSeparator,
              st.wMinute,TimeSeparator,st.wSecond,st.wMilliseconds);
      strTimeText.Format (L"%.*s",TimeLength, (const wchar_t*)strFullTime);
    }
  }

  //if ( !strDateText.IsEmpty() )
  {
    int Year=st.wYear;
    if (!FullYear)
      Year%=100;
    if (TextMonth)
    {
      const wchar_t *Month=MSG(MMonthJan+st.wMonth-1);
      switch(CurDateFormat)
      {
        case 0:
          strDateText.Format (L"%3.3s %2d %02d",Month,st.wDay,Year);
          break;
        case 1:
          strDateText.Format (L"%2d %3.3s %02d",st.wDay,Month,Year);
          break;
        default:
          strDateText.Format (L"%02d %3.3s %2d",Year,Month,st.wDay);
          break;
      }
    }
    else
    {
      int p1,p2,p3=Year;
      switch(CurDateFormat)
      {
        case 0:
          p1=st.wMonth;
          p2=st.wDay;
          break;
        case 1:
          p1=st.wDay;
          p2=st.wMonth;
          break;
        default:
          p1=Year;
          p2=st.wMonth;
          p3=st.wDay;
          break;
      }
      strDateText.Format (L"%02d%c%02d%c%02d",p1,DateSeparator,p2,DateSeparator,p3);
    }
  }

  if (Brief)
  {
    strDateText.SetLength(TextMonth ? 6 : 5);

    if (lt.wYear!=st.wYear)
      strTimeText.Format (L"%5d",st.wYear);
  }
}

void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText)
{
  WORD d,h,m,s;
  ULARGE_INTEGER time;

  time.u.LowPart  = ft.dwLowDateTime;
  time.u.HighPart = ft.dwHighDateTime;

  d = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24)));
  time.QuadPart = time.QuadPart - ((unsigned __int64)d * _ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24));
  h = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60) * _ui64(60)));
  time.QuadPart = time.QuadPart - ((unsigned __int64)h * _ui64(10000000) * _ui64(60) * _ui64(60));
  m = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60)));
  time.QuadPart = time.QuadPart - ((unsigned __int64)m * _ui64(10000000) * _ui64(60));
  s = (WORD)(time.QuadPart / _ui64(10000000));

  strDaysText.Format(L"%u",d);
  strTimeText.Format(L"%02d%c%02d%c%02d", h, GetTimeSeparator(), m, GetTimeSeparator(), s);
}

int GetDateFormat()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_IDATE,Info, sizeof(Info)/sizeof (wchar_t));
  return(_wtoi(Info));
}


int GetDateSeparator()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info,sizeof(Info)/sizeof (wchar_t));
  return(*Info);
}


int GetTimeSeparator()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_STIME,Info,sizeof(Info)/sizeof (wchar_t));
  return(*Info);
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

static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name,const UINT64 Size)
{
	string strSize;
	FileSizeToStr(strSize,Size,8,COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize);
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=(void*)Title;
  preRedrawItem.Param.Param2=(void*)Name;
	preRedrawItem.Param.Param3=reinterpret_cast<LPCVOID>(Size);
  PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_DrawGetDirInfoMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
	DrawGetDirInfoMsg((const wchar_t*)preRedrawItem.Param.Param1,(const wchar_t *)preRedrawItem.Param.Param2,reinterpret_cast<const UINT64>(preRedrawItem.Param.Param3));
}

int GetDirInfo(const wchar_t *Title,
               const wchar_t *DirName,
               unsigned long &DirCount,
               unsigned long &FileCount,
               unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,
               unsigned __int64 &RealSize,
               unsigned long &ClusterSize,
               clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags)
{
  string strFullDirName, strDriveRoot;
  string strFullName, strCurDirName, strLastDirName;

  ConvertNameToFull(DirName, strFullDirName);

  SaveScreen SaveScr;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);
  TPreRedrawFuncGuard preRedrawFuncGuard(PR_DrawGetDirInfoMsg);
	TaskBar TB;

  ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)));
  FAR_FIND_DATA_EX FindData;
  clock_t StartTime=clock();

  SetCursorType(FALSE,0);
  GetPathRoot(strFullDirName,strDriveRoot);

  /* $ 20.03.2002 DJ
     для . - покажем имя родительского каталога
  */
  const wchar_t *ShowDirName = DirName;
  if (DirName[0] == L'.' && DirName[1] == 0)
  {
		const wchar_t *p = LastSlash(strFullDirName);
    if (p)
      ShowDirName = p + 1;
  }

  ConsoleTitle OldTitle;
  RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);

  DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

  if (GetDiskFreeSpaceW(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
    ClusterSize=SectorsPerCluster*BytesPerSector;

  // Временные хранилища имён каталогов
  strLastDirName=L"";
  strCurDirName=L"";

  DirCount=FileCount=0;
  FileSize=CompressedFileSize=RealSize=_i64(0);
  ScTree.SetFindPath(DirName,L"*.*");

  while (ScTree.GetNextName(&FindData,strFullName))
  {
    if (!CtrlObject->Macro.IsExecuting())
    {
      INPUT_RECORD rec;
      switch(PeekInputRecord(&rec))
      {
        case 0:
        case KEY_IDLE:
          break;
        case KEY_NONE:
        case KEY_ALT:
        case KEY_CTRL:
        case KEY_SHIFT:
        case KEY_RALT:
        case KEY_RCTRL:
          GetInputRecord(&rec);
          break;
        case KEY_ESC:
        case KEY_BREAK:
          GetInputRecord(&rec);
          return(0);
        default:
          if (Flags&GETDIRINFO_ENHBREAK)
          {
            return(-1);
          }
          GetInputRecord(&rec);
          break;
      }
    }

		clock_t CurTime=clock();
    if (MsgWaitTime!=-1 && CurTime-StartTime > MsgWaitTime)
    {
			StartTime=CurTime;
			MsgWaitTime=500;
      OldTitle.Set(L"%s %s",MSG(MScanningFolder), ShowDirName); // покажем заголовок консоли
      SetCursorType(FALSE,0);
      DrawGetDirInfoMsg(Title,ShowDirName,FileSize);
    }

    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      // Счётчик каталогов наращиваем только если не включен фильтр,
      // в противном случае это будем делать в подсчёте количества файлов
      if (!(Flags&GETDIRINFO_USEFILTER))
        DirCount++;
      else
      {
        // Если каталог не попадает под фильтр то его надо полностью
        // пропустить - иначе при включенном подсчёте total
        // он учтётся (mantis 551)
        if (!Filter->FileInFilter(&FindData))
          ScTree.SkipDir();
      }
    }
    else
    {
      /* $ 17.04.2005 KM
         Проверка попадания файла в условия фильра
      */
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        if (!Filter->FileInFilter(&FindData))
          continue;
      }

      // Наращиваем счётчик каталогов при включенном фильтре только тогда,
      // когда в таком каталоге найден файл, удовлетворяющий условиям
      // фильтра.
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        strCurDirName = strFullName;

        CutToSlash(strCurDirName); //???

        if (StrCmpI(strCurDirName,strLastDirName)!=0)
        {
          DirCount++;
          strLastDirName = strCurDirName;
        }
      }

      FileCount++;

      unsigned __int64 CurSize = FindData.nFileSize;
      FileSize+=CurSize;
      if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (FindData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE))
      {
        DWORD CompressedSize,CompressedSizeHigh;
				CompressedSize=apiGetCompressedFileSize(strFullName,&CompressedSizeHigh);
        if (CompressedSize!=INVALID_FILE_SIZE || GetLastError()==NO_ERROR)
          CurSize = CompressedSizeHigh*_ui64(0x100000000)+CompressedSize;
      }
      CompressedFileSize+=CurSize;
      if (ClusterSize>0)
      {
        RealSize+=CurSize;
        int Slack=(__int32)(CurSize%ClusterSize);
        if (Slack>0)
          RealSize+=ClusterSize-Slack;
      }
    }
  }

  return(1);
}


int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize)
{
  struct PluginPanelItem *PanelItem=NULL;
  int ItemsNumber,ExitCode;
  DirCount=FileCount=0;
  FileSize=CompressedFileSize=0;

  PluginHandle *ph = (PluginHandle*)hPlugin;

  if ((ExitCode=FarGetPluginDirList((INT_PTR)ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //INT_PTR - BUGBUG
  {
    for (int I=0;I<ItemsNumber;I++)
    {
      if (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        DirCount++;
      else
      {
        FileCount++;
        unsigned __int64 CurSize = PanelItem[I].FindData.nFileSize;
        FileSize+=CurSize;
        if (PanelItem[I].FindData.nPackSize)
          CompressedFileSize+=CurSize;
        else
        {
          unsigned __int64 AddSize = PanelItem[I].FindData.nPackSize;
          CompressedFileSize+=AddSize;
        }
      }
    }
  }
  if (PanelItem!=NULL)
    FarFreePluginDirList(PanelItem, ItemsNumber);
  return(ExitCode);
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

/* $ 13.10.2002 IS
   Переписано заново с учетом того, чтобы избавиться от strstr и
   GetCommaWord - от них только проблемы, в частности, не работала раскраска
   по маске "%pathext%,*.lnk,*.pif,*.awk,*.pln", если %pathext% содержала
   ".pl", т.к. эта подстрока входила в "*.pln"
*/

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


void CreatePath(string &strPath)
{
  wchar_t *ChPtr = strPath.GetBuffer ();
  wchar_t *DirPart = ChPtr;

  BOOL bEnd = FALSE;

  while ( TRUE )
  {
		if ( (*ChPtr == 0) || IsSlash(*ChPtr) )
    {
      if ( *ChPtr == 0 )
        bEnd = TRUE;

      *ChPtr = 0;

			if ( Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && apiGetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES) //BUGBUG
        CharUpperW (DirPart);

			if ( apiCreateDirectory(strPath, NULL) )
        TreeList::AddTreeName(strPath);

      if ( bEnd )
        break;

      *ChPtr = L'\\';
      DirPart = ChPtr+1;
    }

    ChPtr++;
  }
}


int PathMayBeAbsolute(const wchar_t *Path)
{
	return (Path && ((IsAlpha(*Path) && Path[1]==L':' && IsSlash(Path[2])) || (Path[0]==L'\\' && Path[1]==L'\\')));
}

BOOL IsNetworkPath(const wchar_t *Path)
{
  return (Path && Path[0] == L'\\' && Path[1] == L'\\' && Path[2] != L'\\' && wcsrchr(Path+2,L'\\'));
}

BOOL IsLocalPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':');
}

BOOL IsLocalRootPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':' && IsSlash(Path[2]) && !Path[3]);
}

bool PathPrefix(const wchar_t *Path)
{
/*
	\\?\
	\\.\
	\??\
*/
	return Path && Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

BOOL IsLocalPrefixPath(const wchar_t *Path)
{
	return PathPrefix(Path) && Path[4] && Path[5] == L':' && Path[6] == L'\\';
}

BOOL IsLocalVolumePath(const wchar_t *Path)
{
	return PathPrefix(Path) && !wcsnicmp(&Path[4],L"Volume{",7) && Path[47] == L'}';
}

BOOL IsLocalVolumeRootPath(const wchar_t *Path)
{
	return IsLocalVolumePath(Path) && !Path[48];
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR

string& PrepareDiskPath(string &strPath,BOOL CheckFullPath)
{
	if( !strPath.IsEmpty() )
	{
		if(((IsAlpha(strPath.At(0)) && strPath.At(1)==L':') || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\')))
		{
			if(CheckFullPath)
			{
				ConvertNameToFull(strPath,strPath);
				wchar_t *lpwszPath=strPath.GetBuffer(),*Src=lpwszPath,*Dst=lpwszPath;
				if(IsLocalPath(lpwszPath))
				{
					Src+=2;
					if(IsSlash(*Src))
						Src++;
					Dst+=2;
					if(IsSlash(*Dst))
						Dst++;
				}
				if(*Src)
				{
					for(wchar_t c=*Src;;Src++,c=*Src)
					{
						if (!c||IsSlash(c))
						{
							*Src=0;
							FAR_FIND_DATA_EX fd;
							BOOL find=apiGetFindDataEx(lpwszPath,&fd,false);
							*Src=c;
							if(find)
							{
								size_t n=fd.strFileName.GetLength();
								size_t n1 = n-(Src-Dst);
								if((int)n1>0)
								{
									size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
									strPath.ReleaseBuffer();
									lpwszPath=strPath.GetBuffer(int(strPath.GetLength()+n1));
									Src=lpwszPath+dSrc;
									Dst=lpwszPath+dDst;
									wmemmove(Src+n1,Src,StrLength(Src)+1);
									Src+=n1;
								}
								wcsncpy(Dst,fd.strFileName,n);
								Dst+=n;
								wcscpy(Dst,Src);
								if(c)
								{
									Dst++;
									Src=Dst;
								}
							}
							else
							{
								if(c)
								{
									Src++;
									Dst=Src;
								}
							}
						}
						if(!*Src)
							break;
					}
				}
				strPath.ReleaseBuffer();
			}

			wchar_t *lpwszPath = strPath.GetBuffer ();

			if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
			{
				if(IsLocalPrefixPath(lpwszPath))
				{
					lpwszPath[4] = Upper(lpwszPath[4]);
				}
				else
				{
					wchar_t *ptr=&lpwszPath[2];
					while (*ptr && !IsSlash(*ptr))
					{
						*ptr=Upper(*ptr);
						ptr++;
					}
				}
			}
			else
			{
				lpwszPath[0]=Upper(lpwszPath[0]);
			}

			strPath.ReleaseBuffer ();
		}
	}
	return strPath;
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

/*
 возвращает PipeFound
*/
int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
  int PipeFound = FALSE;
  int QuoteFound = FALSE;

  apiExpandEnvironmentStrings (CmdStr, strNewCmdStr);
  RemoveExternalSpaces(strNewCmdStr);

  wchar_t *NewCmdStr = strNewCmdStr.GetBuffer();
  wchar_t *CmdPtr = NewCmdStr;
  wchar_t *ParPtr = NULL;

  // Разделим собственно команду для исполнения и параметры.
  // При этом заодно определим наличие символов переопределения потоков
  // Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

  while (*CmdPtr)
  {
    if (*CmdPtr == L'"')
      QuoteFound = !QuoteFound;
    if (!QuoteFound && CmdPtr != NewCmdStr)
    {
      if (*CmdPtr == L'>' ||
          *CmdPtr == L'<' ||
          *CmdPtr == L'|' ||
          *CmdPtr == L' ' ||
          *CmdPtr == L'/' || // вариант "far.exe/?"
          *CmdPtr == L'&'    // обработаем разделитель команд
         )
      {
        if (!ParPtr)
          ParPtr = CmdPtr;
        if (*CmdPtr != L' ' && *CmdPtr != L'/')
          PipeFound = TRUE;
      }
    }

    if (ParPtr && PipeFound)
    // Нам больше ничего не надо узнавать
      break;
    CmdPtr++;
  }

  if (ParPtr) // Мы нашли параметры и отделяем мух от котлет
  {
    if (*ParPtr == L' ') //AY: первый пробел между командой и параметрами не нужен,
      *(ParPtr++)=0;     //    он добавляется заново в Execute.

    strNewCmdPar = ParPtr;
    *ParPtr = 0;
  }

  strNewCmdStr.ReleaseBuffer ();

  Unquote(strNewCmdStr);

  return PipeFound;
}


BOOL ProcessOSAliases(string &strStr)
{
  string strNewCmdStr;
  string strNewCmdPar;

  PartCmdLine(strStr,strNewCmdStr,strNewCmdPar);

  string strModuleName;
  apiGetModuleFileName(NULL,strModuleName);
  const wchar_t* lpwszExeName=PointToName(strModuleName);
  int nSize=(int)strNewCmdStr.GetLength()+4096;
  wchar_t* lpwszNewCmdStr=strNewCmdStr.GetBuffer(nSize);
  int ret=GetConsoleAliasW(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),(wchar_t*)lpwszExeName);
  if(!ret)
  {
    if(apiExpandEnvironmentStrings(L"%COMSPEC%",strModuleName))
    {
      lpwszExeName=PointToName(strModuleName);
      ret=GetConsoleAliasW(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),(wchar_t*)lpwszExeName);
    }
  }
  strNewCmdStr.ReleaseBuffer();
  if(!ret)
    return FALSE;
  if(!ReplaceStrings(strNewCmdStr,L"$*",strNewCmdPar))
    strNewCmdStr+=L" "+strNewCmdPar;
  strStr=strNewCmdStr;
  return TRUE;
}


int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
  int RetCode=FALSE;
  int NeedRealName=FALSE;

  strPathName = L"";
  switch(Key)
  {
    case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
    case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
    case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
    case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
      NeedRealName=TRUE;
    case KEY_CTRLBRACKET:          // Вставить путь из левой панели
    case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
    case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
    case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели

    case KEY_CTRLSHIFTNUMENTER:       // Текущий файл с пасс.панели
    case KEY_SHIFTNUMENTER:           // Текущий файл с актив.панели
    case KEY_CTRLSHIFTENTER:       // Текущий файл с пасс.панели
    case KEY_SHIFTENTER:           // Текущий файл с актив.панели
    {
      Panel *SrcPanel=NULL;
      FilePanels *Cp=CtrlObject->Cp();
      switch(Key)
      {
        case KEY_CTRLALTBRACKET:
        case KEY_CTRLBRACKET:
          SrcPanel=Cp->LeftPanel;
          break;
        case KEY_CTRLALTBACKBRACKET:
        case KEY_CTRLBACKBRACKET:
          SrcPanel=Cp->RightPanel;
          break;
        case KEY_SHIFTNUMENTER:
        case KEY_SHIFTENTER:
        case KEY_ALTSHIFTBRACKET:
        case KEY_CTRLSHIFTBRACKET:
          SrcPanel=Cp->ActivePanel;
          break;
        case KEY_CTRLSHIFTNUMENTER:
        case KEY_CTRLSHIFTENTER:
        case KEY_ALTSHIFTBACKBRACKET:
        case KEY_CTRLSHIFTBACKBRACKET:
          SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
          break;
      }

      if (SrcPanel!=NULL)
      {
        if(Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER)
        {
          string strShortFileName;
          SrcPanel->GetCurName(strPathName,strShortFileName);
          if(SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
            strPathName = strShortFileName;
        }
        else
        {
          /* TODO: Здесь нужно учесть, что у TreeList тоже есть путь :-) */
          if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
            return(FALSE);

          SrcPanel->GetCurDir(strPathName);
          if (SrcPanel->GetMode()!=PLUGIN_PANEL)
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            SrcFilePanel->GetCurDir(strPathName);

            {
                if(NeedRealName)
                    SrcFilePanel->CreateFullPathName(strPathName, strPathName,FILE_ATTRIBUTE_DIRECTORY, strPathName,TRUE,ShortNameAsIs);
            }


            if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
              ConvertNameToShort(strPathName,strPathName);
          }
          else
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            struct OpenPluginInfo Info;

            CtrlObject->Plugins.GetOpenPluginInfo(SrcFilePanel->GetPluginHandle(),&Info);
            FileList::AddPluginPrefix(SrcFilePanel,strPathName);

            strPathName += Info.CurDir;

          }
          AddEndSlash(strPathName);
        }

        if(Opt.QuotedName&QUOTEDNAME_INSERT)
          QuoteSpace(strPathName);

        if ( Param2 )
            strPathName += Param2;

        RetCode=TRUE;
      }
    }
    break;
  }
  return RetCode;
}


string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName)
{
  string strNetDir;

  strComputerName=L"";

  if ( CurDir[0]==L'\\' && CurDir[1]==L'\\')
    strNetDir = CurDir;
  else
  {
    /* $ 28.03.2002 KM
       - Падение VC на
         char *LocalName="A:";
         *LocalName=*CurDir;
         Так как память в LocalName ReadOnly.
    */
    wchar_t LocalName[3];

    xwcsncpy (LocalName, CurDir, 2);

    apiWNetGetConnection (LocalName, strNetDir);
  }

  if ( strNetDir.At(0)==L'\\' && strNetDir.At(1) == L'\\')
  {
    strComputerName = (const wchar_t*)strNetDir+2;

    size_t pos;
		if (!FirstSlash(strComputerName,pos))
    {
      strComputerName.SetLength(0);
    }
    else
    {
      strComputerName.SetLength(pos);
    }
  }

  return strComputerName;
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

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b)
{
	LARGE_INTEGER A, B;

	A.u.LowPart  = a->dwLowDateTime;
	A.u.HighPart = a->dwHighDateTime;
	B.u.LowPart  = b->dwLowDateTime;
	B.u.HighPart = b->dwHighDateTime;

	return A.QuadPart - B.QuadPart;
}

unsigned __int64 FileTimeToUI64(const FILETIME *ft)
{
	ULARGE_INTEGER A;

	A.u.LowPart  = ft->dwLowDateTime;
	A.u.HighPart = ft->dwHighDateTime;

	return A.QuadPart;
}

bool IsDriveTypeRemote(UINT DriveType)
{
	return DriveType == DRIVE_REMOTE || DriveType == DRIVE_REMOTE_NOT_CONNECTED;
}
