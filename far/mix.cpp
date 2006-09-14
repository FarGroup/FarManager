/*
mix.cpp

Куча разных вспомогательных функций

*/

/* Revision: 1.204 25.08.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "savefpos.hpp"
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

long filelen(FILE *FPtr)
{
  SaveFilePos SavePos(FPtr);
  fseek(FPtr,0,SEEK_END);
  return(ftell(FPtr));
}

__int64 filelen64(FILE *FPtr)
{
  SaveFilePos SavePos(FPtr);
  fseek64(FPtr,0,SEEK_END);
  return(ftell64(FPtr));
}


UserDefinedListW *SaveAllCurDir(void)
{
  UserDefinedListW *DirList=new UserDefinedListW(0,0,0);
  if(!DirList)
    return NULL;

  string strCurDir;
  wchar_t Drive[4]=L"=A:";
  for(int I=L'A'; I <= L'Z'; ++I)
  {
    Drive[1]=I;
    DirList->AddItem(Drive);

    const wchar_t *Ptr;

    if(!apiGetEnvironmentVariable(Drive,strCurDir))  // окружения
      Ptr=L"\x01";
    else
      Ptr=strCurDir;

    DirList->AddItem(Ptr);
  }
  return DirList;
}

void RestoreAllCurDir(UserDefinedListW *DirList)
{
  if(!DirList)
    return;

  string strDrive;
  const wchar_t *NamePtr;
  while(NULL!=(NamePtr=DirList->GetNext()))
  {
    strDrive = NamePtr;
    if((NamePtr=DirList->GetNext()) != NULL)
      SetEnvironmentVariableW (strDrive,(*NamePtr == L'\x1'?L"":NamePtr));
  }
  delete DirList;
}



BOOL FarChDirW(const wchar_t *NewDir, BOOL ChangeDir)
{
  if(!NewDir || *NewDir == 0)
    return FALSE;

  BOOL rc=FALSE;
  wchar_t Drive[4]=L"=A:";

  string strCurDir;
  wchar_t *lpwszCurDir;

  if(LocalIsalphaW(*NewDir) && NewDir[1]==L':' && NewDir[2]==0)// если указана только
  {                                                     // буква диска, то путь
    Drive[1]=LocalUpperW(*NewDir);                          // возьмем из переменной

    if ( !apiGetEnvironmentVariable (Drive, strCurDir) )
    {
      strCurDir = NewDir;
      AddEndSlashW (strCurDir);

      wchar_t *lpwszChr = strCurDir.GetBuffer();

      while(*lpwszChr)
      {
        if(*lpwszChr==L'/')
           *lpwszChr=L'\\';
        ++lpwszChr;
      }

      strCurDir.ReleaseBuffer ();
    }
    //*CurDir=toupper(*CurDir); бред!
    if(ChangeDir)
    {
      if(CheckFolderW(strCurDir) > CHKFLD_NOTACCESS)
        rc=SetCurrentDirectoryW(strCurDir);
    }
  }
  else
  {
    strCurDir = NewDir;

    if(!wcscmp(strCurDir,L"\\"))
      FarGetCurDirW(strCurDir); // здесь берем корень

    wchar_t *lpwszChr = strCurDir.GetBuffer();

    while(*lpwszChr)
    {
      if(*lpwszChr==L'/')
         *lpwszChr=L'\\';
      ++lpwszChr;
    }

    strCurDir.ReleaseBuffer ();

    if(ChangeDir)
    {
      wchar_t *ptr;

      int nSize = GetFullPathNameW(NewDir,0,NULL,&ptr);
      lpwszCurDir = strCurDir.GetBuffer (nSize+1);
      GetFullPathNameW(NewDir,nSize+1,lpwszCurDir,&ptr);
      AddEndSlashW(strCurDir); //???????????????
      strCurDir.ReleaseBuffer ();

      if(CheckFolderW((const wchar_t*)strCurDir) > CHKFLD_NOTACCESS)
      {
        PrepareDiskPathW(strCurDir);
        rc=SetCurrentDirectoryW((const wchar_t*)strCurDir);
      }

    }
  }

  if(rc || !ChangeDir)
  {
    int nSize = GetCurrentDirectoryW (0, NULL);

    lpwszCurDir = strCurDir.GetBuffer (nSize+1);

    if ((!ChangeDir || GetCurrentDirectoryW(nSize+1,lpwszCurDir)) &&
        LocalIsalphaW(*lpwszCurDir) && lpwszCurDir[1]==L':')
    {
      Drive[1]=LocalUpperW(*lpwszCurDir);
      SetEnvironmentVariableW(Drive,lpwszCurDir);
    }

    strCurDir.ReleaseBuffer ();
  }
  return rc;
}

/* $ 20.03.2002 SVS
 обертка вокруг функции получения текущего пути.
 для локального пути переводит букву диска в uppercase
*/

DWORD FarGetCurDirW (string &strBuffer)
{
    int nLength = GetCurrentDirectoryW (0, NULL)+1;

    wchar_t *lpwszBuffer = strBuffer.GetBuffer (nLength);

    DWORD Result = GetCurrentDirectoryW (nLength, lpwszBuffer);

    if ( Result &&
         LocalIsalphaW (*lpwszBuffer) &&
         lpwszBuffer[1] == L':' &&
         (lpwszBuffer[2] == 0 || lpwszBuffer[2] == '\\')
         )
         *lpwszBuffer = LocalUpperW (*lpwszBuffer);

    strBuffer.ReleaseBuffer ();

    return Result;
}

/* SVS 20.03.2002 $ */

DWORD NTTimeToDos(FILETIME *ft)
{
  WORD DosDate,DosTime;
  FILETIME ct;
  FileTimeToLocalFileTime(ft,&ct);
  FileTimeToDosDateTime(&ct,&DosDate,&DosTime);
  return(((DWORD)DosDate<<16)|DosTime);
}

void ConvertDateW (const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
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
      const wchar_t *Month=UMSG(MMonthJan+st.wMonth-1);
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
    wchar_t *lpwszDateText = strDateText.GetBuffer (max (strDateText.GetLength(), 7));

    lpwszDateText[TextMonth ? 6:5]=0;

    strDateText.ReleaseBuffer ();

    if (lt.wYear!=st.wYear)
      strTimeText.Format (L"%5d",st.wYear);
  }
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

  if(flags&PN_CMPNAME)
    return CmpNameW (param1, param2, skippath);

  if(flags&PN_CMPNAMELIST)
  {
    int Found=FALSE;
    string strFileMask;
    const wchar_t *MaskPtr;
    MaskPtr=param1;

    while ((MaskPtr=GetCommaWordW(MaskPtr,strFileMask))!=NULL)
      if (CmpNameW(strFileMask,param2,skippath))
      {
        Found=TRUE;
        break;
      }
    return Found;
  }

  if(flags&PN_GENERATENAME)
  {
      string strResult;

      int nResult = ConvertWildcardsW(param1, strResult, flags & 0xFFFF);

      xwcsncpy(param2, strResult, size);

      return nResult;
  }

  return FALSE;
}
/* IS $ */


int GetFileTypeByNameW(const wchar_t *Name)
{
  HANDLE hFile=FAR_CreateFileW(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FILE_TYPE_UNKNOWN);
  int Type=GetFileType(hFile);
  CloseHandle(hFile);
  return(Type);
}


static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name)
{
  MessageW(0,0,Title,UMSG(MScanningFolder),Name);
  PreRedrawParam.Param1=(void*)Title;
  PreRedrawParam.Param2=(void*)Name;
}

static void PR_DrawGetDirInfoMsg(void)
{
  DrawGetDirInfoMsg((const wchar_t*)PreRedrawParam.Param1,(const wchar_t *)PreRedrawParam.Param2);
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
               DWORD Flags)
{
  string strFullDirName, strDriveRoot;
  string strFullName, strCurDirName, strLastDirName;

  ConvertNameToFullW(DirName, strFullDirName);

  SaveScreen SaveScr;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);

  ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?-1:(Flags&GETDIRINFO_SCANSYMLINK)));
  FAR_FIND_DATA_EX FindData;
  int MsgOut=0;
  clock_t StartTime=clock();

  SetCursorType(FALSE,0);
  GetPathRootW(strFullDirName,strDriveRoot);

  /* $ 20.03.2002 DJ
     для . - покажем имя родительского каталога
  */
  const wchar_t *ShowDirName = DirName;
  if (DirName[0] == L'.' && DirName[1] == 0)
  {
    wchar_t *p = wcsrchr (strFullDirName, L'\\');
    if (p)
      ShowDirName = p + 1;
  }
  /* DJ */

  ConsoleTitle OldTitle;
  RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);

  PREREDRAWFUNC OldPreRedrawFunc=PreRedrawFunc;

  if ((ClusterSize=GetClusterSizeW(strDriveRoot))==0)
  {
    DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

    if (GetDiskFreeSpaceW(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
      ClusterSize=SectorsPerCluster*BytesPerSector;
  }

  // Временные хранилища имён каталогов
  strLastDirName=L"";
  strCurDirName=L"";

  // Создадим объект фильтра
  FileFilter Filter;

  DirCount=FileCount=0;
  FileSize=CompressedFileSize=RealSize=0;
  ScTree.SetFindPathW(DirName,L"*.*");

  while (ScTree.GetNextNameW(&FindData,strFullName))
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
          SetPreRedrawFunc(OldPreRedrawFunc);
          return(0);
        default:
          if (Flags&GETDIRINFO_ENHBREAK)
          {
            SetPreRedrawFunc(OldPreRedrawFunc);
            return(-1);
          }
          GetInputRecord(&rec);
          break;
      }
    }

    if (!MsgOut && MsgWaitTime!=0xffffffff && clock()-StartTime > MsgWaitTime)
    {
      OldTitle.Set(L"%s %s",UMSG(MScanningFolder), ShowDirName); // покажем заголовок консоли
      SetCursorType(FALSE,0);
      SetPreRedrawFunc(PR_DrawGetDirInfoMsg);
      DrawGetDirInfoMsg(Title,ShowDirName);
      MsgOut=1;
    }

    if (FindData.dwFileAttributes & FA_DIREC)
    {
      // Счётчик каталогов наращиваем только если не включен фильтр,
      // в противном случае это будем делать в подсчёте количества файлов
      if (!(Flags&GETDIRINFO_USEFILTER))
        DirCount++;
    }
    else
    {
      /* $ 17.04.2005 KM
         Проверка попадания файла в условия фильра
      */
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        if (!Filter.FileInFilter(&FindData))
          continue;
      }

      // Наращиваем счётчик каталогов при включенном фильтре только тогда,
      // когда в таком каталоге найден файл, удовлетворяющий условиям
      // фильтра.
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        strCurDirName = strFullName;

        CutToSlashW (strCurDirName); //???

        if (LocalStricmpW(strCurDirName,strLastDirName)!=0)
        {
          DirCount++;
          strLastDirName = strCurDirName;
        }
      }
      /* KM $ */

      FileCount++;

      unsigned __int64 CurSize = FindData.nFileSize;
      FileSize+=CurSize;
      if (FindData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
      {
        DWORD CompressedSize,CompressedSizeHigh;
        CompressedSize=GetCompressedFileSizeW(strFullName,&CompressedSizeHigh);
        if (CompressedSize!=0xFFFFFFFF || GetLastError()==NO_ERROR)
          CurSize = CompressedSizeHigh*0x100000000+CompressedSize;
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

  SetPreRedrawFunc(OldPreRedrawFunc);
  return(1);
}


int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize)
{
  struct PluginPanelItemW *PanelItem=NULL;
  int ItemsNumber,ExitCode;
  DirCount=FileCount=0;
  FileSize=CompressedFileSize=0;
  if ((ExitCode=FarGetPluginDirList(((struct PluginHandle *)hPlugin)->PluginNumber,
      ((struct PluginHandle *)hPlugin)->InternalHandle,DirName,
      &PanelItem,&ItemsNumber))==TRUE)
  {
    for (int I=0;I<ItemsNumber;I++)
    {
      if (PanelItem[I].FindData.dwFileAttributes & FA_DIREC)
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

int CheckFolderW(const wchar_t *Path)
{
  if(!(Path || *Path)) // проверка на вшивость
    return CHKFLD_ERROR;

/*  int LenFindPath=Max((int)wcslen(Path),2048)+8;
  char *FindPath=(char *)alloca(LenFindPath); // здесь alloca - чтобы _точно_ хватило на все про все.
  if(!FindPath)
    return CHKFLD_ERROR;*/

  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  int Done=FALSE;

  string strFindPath = Path;


  // сообразим маску для поиска.
  AddEndSlashW(strFindPath);

  strFindPath += L"*.*";

  // первая проверка - че-нить считать можем?
  if((FindHandle=apiFindFirstFile(strFindPath,&fdata)) == INVALID_HANDLE_VALUE)
  {
    GuardLastError lstError;
    if(lstError.Get() == ERROR_FILE_NOT_FOUND)
      return CHKFLD_EMPTY;

    // собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
    // поэтому посмотрим на Root
    GetPathRootOneW(Path,strFindPath);


    if(!wcscmp(Path,strFindPath))
    {
      // проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
      if(GetFileAttributesW(strFindPath)!=0xFFFFFFFF)
        return CHKFLD_EMPTY;
    }

    strFindPath = Path;

    if(CheckShortcutFolderW(&strFindPath,FALSE,TRUE))
    {
      if(wcscmp(Path,strFindPath))
        return CHKFLD_NOTFOUND;
    }

    return CHKFLD_NOTACCESS;
  }

  // Ок. Что-то есть. Попробуем ответить на вопрос "путой каталог?"
  while(!Done)
  {
    if (fdata.strFileName.At(0) == L'.' && (fdata.strFileName.At(1) == 0 || fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0))
      ; // игнорируем "." и ".."
    else
    {
      // что-то есть, отличное от "." и ".." - каталог не пуст
      FindClose(FindHandle);
      return CHKFLD_NOTEMPTY;
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  // однозначно каталог пуст
  FindClose(FindHandle);
  return CHKFLD_EMPTY;
}


char* GetAnsiLanguageString (int nID)
{
    return Lang.GetMsg(nID);
}

wchar_t* GetUnicodeLanguageString (int nID)
{
    return Lang.GetMsgW(nID);
}



BOOL GetDiskSizeW (const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
  typedef BOOL (WINAPI *GETDISKFREESPACEEXW)(
    const wchar_t *lpwszDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
   );
  static GETDISKFREESPACEEXW pGetDiskFreeSpaceExW=NULL;
  static int LoadAttempt=FALSE;
  int ExitCode=0;

  ULARGE_INTEGER uiTotalSize,uiTotalFree,uiUserFree;

  uiUserFree.QuadPart = 0;
  uiTotalSize.QuadPart = 0;
  uiTotalFree.QuadPart = 0;

  if (!LoadAttempt && pGetDiskFreeSpaceExW==NULL)
  {
    HMODULE hKernel=GetModuleHandleW(L"kernel32.dll");
    if (hKernel!=NULL)
      pGetDiskFreeSpaceExW=(GETDISKFREESPACEEXW)GetProcAddress(hKernel,"GetDiskFreeSpaceExW");
    LoadAttempt=TRUE;
  }
  if (pGetDiskFreeSpaceExW!=NULL)
  {
    ExitCode=pGetDiskFreeSpaceExW(Root,&uiUserFree,&uiTotalSize,&uiTotalFree);
    if (uiUserFree.QuadPart > uiTotalFree.QuadPart)
      uiUserFree.QuadPart=uiTotalFree.QuadPart;
  }

  if (pGetDiskFreeSpaceExW==NULL || ExitCode==0 || uiTotalSize.QuadPart==0)
  {
    DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
    ExitCode=GetDiskFreeSpaceW(Root,&SectorsPerCluster,&BytesPerSector,
                              &FreeClusters,&Clusters);
    uiTotalSize.QuadPart=SectorsPerCluster*BytesPerSector*Clusters;
    uiTotalFree.QuadPart=SectorsPerCluster*BytesPerSector*FreeClusters;
    uiUserFree.QuadPart=uiTotalFree.QuadPart;
  }

  if ( TotalSize )
    *TotalSize = uiTotalSize.QuadPart;
  if ( TotalFree )
    *TotalFree = uiTotalFree.QuadPart;
  if ( UserFree )
    *UserFree = uiUserFree.QuadPart;
  return(ExitCode);
}


int GetClusterSizeW(const wchar_t *Root)
{
  struct ExtGetDskFreSpc
  {
    WORD ExtFree_Size;
    WORD ExtFree_Level;
    DWORD ExtFree_SectorsPerCluster;
    DWORD ExtFree_BytesPerSector;
    DWORD ExtFree_AvailableClusters;
    DWORD ExtFree_TotalClusters;
    DWORD ExtFree_AvailablePhysSectors;
    DWORD ExtFree_TotalPhysSectors;
    DWORD ExtFree_AvailableAllocationUnits;
    DWORD ExtFree_TotalAllocationUnits;
    DWORD ExtFree_Rsvd[2];
  } DiskInfo;

  struct _DIOC_REGISTERS
  {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
  } reg;

  BOOL fResult;
  DWORD cb;

  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
      WinVer.dwBuildNumber<0x04000457)
    return(0);

  HANDLE hDevice = FAR_CreateFileW(L"\\\\.\\vwin32", 0, 0, NULL, 0,
                              FILE_FLAG_DELETE_ON_CLOSE, NULL);

  if (hDevice==INVALID_HANDLE_VALUE)
    return(0);

  DiskInfo.ExtFree_Level=0;

  char *lpRoot = UnicodeToAnsi (Root);

  reg.reg_EAX = 0x7303;
  reg.reg_EDX = (DWORD)lpRoot;
  reg.reg_EDI = (DWORD)&DiskInfo;
  reg.reg_ECX = sizeof(DiskInfo);
  reg.reg_Flags = 0x0001;

  fResult=DeviceIoControl(hDevice,6,&reg,sizeof(reg),&reg,sizeof(reg),&cb,0);

  xf_free (lpRoot);

  CloseHandle(hDevice);
  if (!fResult || (reg.reg_Flags & 0x0001))
    return(0);
  return(DiskInfo.ExtFree_SectorsPerCluster*DiskInfo.ExtFree_BytesPerSector);
}





/* IS $ */

#if 0
/*
In: "C:\WINNT\SYSTEM32\FOO.TXT", "%SystemRoot%"
Out: "%SystemRoot%\SYSTEM32\FOO.TXT"
*/
BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize)
{
  int I;
  char Temp[NM*2];
  Temp[0] = 0;

  ExpandEnvironmentStr(EnvVar, Temp, sizeof(Temp));
  I = strlen(Temp);

  if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, Temp, I, Path, I) == 2)
  {
    if (strlen(Path)-I+strlen(EnvVar) < DestSize)
    {
      xstrncpy(Dest, EnvVar, DestSize-1);
      strncat(Dest, Path + I, DestSize-1);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize)
{
  const char *StdEnv[]={
     "%TEMP%",               // C:\Documents and Settings\Administrator\Local Settings\Temp
     "%APPDATA%",            // C:\Documents and Settings\Administrator\Application Data
     "%USERPROFILE%",        // C:\Documents and Settings\Administrator
     "%ALLUSERSPROFILE%",    // C:\Documents and Settings\All Users
     "%CommonProgramFiles%", // C:\Program Files\Common Files
     "%ProgramFiles%",       // C:\Program Files
     "%SystemRoot%",         // C:\WINNT
  };

  for(int I=0; I < sizeof(StdEnv)/sizeof(StdEnv[0]); ++I)
  {
    if(UnExpandEnvironmentString(Path, StdEnv[I], Dest, DestSize))
      return TRUE;
  }
  xstrncpy(Dest, Path, DestSize-1);
  return FALSE;

}
#endif

/* $ 10.09.2000 tran
   FSF/FarRecurseSearch */
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
    CFileMaskW FMask;
    if(!FMask.Set(Mask, FMF_SILENT)) return;

    Flags=Flags&0x000000FF; // только младший байт!
    ScanTree ScTree(Flags & FRS_RETUPDIR,Flags & FRS_RECUR, Flags & FRS_SCANSYMLINK);
    FAR_FIND_DATA_EX FindData;

    string strFullName;

    ScTree.SetFindPathW(InitDir,L"*");
    while (ScTree.GetNextNameW(&FindData,strFullName))
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
/* tran 10.09.2000 $ */

/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
    Dest - приемник результата (должен быть достаточно большим, например NM
    Template - шаблон по правилам функции mktemp, например "FarTmpXXXXXX"
   Вернет либо NULL, либо указатель на Dest.
*/
/* $ 18.09.2000 SVS
  Не ту функцию впихнул :-)))
*/
/* $ 25.10.2000 IS
 ! Заменил mktemp на вызов соответствующей апишной функции, т.к. предыдущий
   вариант приводил к ошибке (заметили на Multiarc'е)
   Параметр Prefix - строка, указывающая на первые символы имени временного
   файла. Используются только первые 3 символа из этой строки.
*/

/*
char* WINAPI FarMkTemp(char *Dest, const char *Prefix)
{
  return FarMkTempEx(Dest,Prefix,TRUE);
}

*/
/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
/*
char* FarMkTempEx(char *Dest, const char *Prefix, BOOL WithPath)
{
  if(Dest)
  {
    if(!(Prefix && *Prefix))
      Prefix="FTMP";

    char TempName[NM];
    TempName[0]=0;
    if(WithPath)
      strcpy(TempName,Opt.TempPath);
    strcat(TempName,"0000XXXXXXXX");
    memcpy(TempName+strlen(TempName)-12,Prefix,Min((int)strlen(Prefix),4));
    if (farmktemp(TempName)!=NULL)
    {
      strcpy(Dest,strupr(TempName));
      return Dest;
    }
  }
  return NULL;
}
*/

wchar_t* __stdcall FarMkTemp (wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
    string strDest;

    if ( FarMkTempExW (strDest, Prefix, TRUE) )
    {
        xwcsncpy (Dest, strDest, size);
        return Dest;
    }

    return NULL;
}

string& FarMkTempExW(string &strDest, const wchar_t *Prefix, BOOL WithPath)
{
  if(!(Prefix && *Prefix))
    Prefix=L"FTMP";

  string strPath = Opt.strTempPath;

  wchar_t *lpwszDest = strDest.GetBuffer (wcslen(Prefix)+strPath.GetLength()+13);

  do {
    GetTempFileNameW (strPath, Prefix, GetCurrentProcessId(), lpwszDest);
  } while ( GetFileAttributesW (lpwszDest) != -1 );

  strDest.ReleaseBuffer ();

  return strDest;
}

/* IS $ */
/* SVS 18.09.2000 $ */
/* SVS $ */

/*$ 27.09.2000 skv
  + Удаление буфера выделенного через new char[n];
    Сделано для удаления возвращенного PasteFromClipboard
*/
void WINAPI DeleteBuffer(char *Buffer)
{
  if(Buffer)delete [] Buffer;
}
/* skv$*/


string &DriveLocalToRemoteNameW(int DriveType,wchar_t Letter,string &strDest)
{
  int NetPathShown=FALSE, IsOK=FALSE;
  wchar_t LocalName[8]=L" :\0\0\0", RemoteName[NM]; //BUGBUG
  DWORD RemoteNameSize=sizeof(RemoteName)/sizeof (wchar_t);

  *LocalName=Letter;
  strDest=L"";

  if(DriveType == DRIVE_UNKNOWN)
  {
    LocalName[2]='\\';
    DriveType = FAR_GetDriveTypeW(LocalName);
    LocalName[2]=0;
  }

  if (DriveType==DRIVE_REMOTE)
  {
    if (WNetGetConnectionW(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
    {
      NetPathShown=TRUE;
      IsOK=TRUE;
    }
  }
  string strRemoteName = RemoteName;

  if (!NetPathShown)
    if (GetSubstNameW(DriveType,LocalName,strRemoteName))
      IsOK=TRUE;

  if(IsOK)
    strDest = strRemoteName;

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
  int curpos=strDest.GetLength()-1;
  UserDefinedListW MaskList(0,0,ULF_UNIQUE);
  if( apiGetEnvironmentVariable(L"PATHEXT",strBuf) && MaskList.Set(strBuf))
  {
    /* $ 13.10.2002 IS проверка на '|' (маски исключения) */
    if( !strDest.IsEmpty() && strDest.At(curpos)!=L',' && strDest.At(curpos)!=L'|')
      strDest += L',';
    /* IS $ */
    const wchar_t *Ptr;
    MaskList.Reset();
    while(NULL!=(Ptr=MaskList.GetNext()))
    {
      strDest += L'*';
      strDest += Ptr;
      strDest += L',';
    }
  }
  // лишняя запятая - в морг!
  /* $ 13.10.2002 IS Оптимизация по скорости */
  if(strDest.At(curpos) == L',')
  {
    wchar_t *lpwszDest = strDest.GetBuffer ();
    lpwszDest[curpos]=0;
    strDest.ReleaseBuffer ();
  }
  /* IS $ */
  return strDest;
}


void CreatePathW (string &strPath)
{
  wchar_t *ChPtr = strPath.GetBuffer ();
  wchar_t *DirPart = ChPtr;

  BOOL bEnd = FALSE;

  while ( TRUE )
  {
    if ( (*ChPtr == 0) || (*ChPtr == L'\\') )
    {
      if ( *ChPtr == 0 )
        bEnd = TRUE;

      *ChPtr = 0;

      if ( Opt.CreateUppercaseFolders && !IsCaseMixedW(DirPart) && GetFileAttributesW(strPath) == (DWORD)-1) //BUGBUG
        CharUpperW (DirPart);

      if ( CreateDirectoryW(strPath, NULL) )
        TreeList::AddTreeName(strPath);

      if ( bEnd )
        break;

      *ChPtr = L'\\';
      DirPart = ChPtr+1;
    }

    ChPtr++;
  }
}


void SetPreRedrawFunc(PREREDRAWFUNC Func)
{
  if((PreRedrawFunc=Func) == NULL)
    memset(&PreRedrawParam,0,sizeof(PreRedrawParam));
}

int PathMayBeAbsoluteW (const wchar_t *Path)
{
    return (Path &&
           (
             (LocalIsalphaW(*Path) && Path[1]==L':') ||
             (Path[0]==L'\\'  && Path[1]==L'\\') ||
             (Path[0]==L'/'   && Path[1]==L'/')
           )
         );
}


BOOL IsLocalPathW(const wchar_t *Path)
{
  return (Path && LocalIsalphaW(*Path) && Path[1]==L':' && Path[2]);
}

BOOL IsLocalRootPathW(const wchar_t *Path)
{
  return (Path && LocalIsalphaW(*Path) && Path[1]==L':' && Path[2] == L'\\' && !Path[3]);
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR

string& PrepareDiskPathW(string &strPath,BOOL CheckFullPath)
{
  if( !strPath.IsEmpty() )
  {
    if((LocalIsalphaW(strPath.At(0)) && strPath.At(1)==L':') || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\'))
    {
      if(CheckFullPath)
      {
        string strNPath;
        RawConvertShortNameToLongNameW(strPath,strNPath);
        if( !strNPath.IsEmpty() )
          strPath = strNPath;
      }

      wchar_t *lpwszPath = strPath.GetBuffer ();

      if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
      {
        wchar_t *ptr=&lpwszPath[2];
        while (*ptr && *ptr!=L'\\')
          *(ptr++)=LocalUpperW(*ptr);
      }
      else
        lpwszPath[0]=LocalUpperW(lpwszPath[0]);

      strPath.ReleaseBuffer ();
      /* DJ $ */
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

int CheckShortcutFolderW(string *pTestPath,int IsHostFile, BOOL Silent)
{
  if( pTestPath && !pTestPath->IsEmpty() && GetFileAttributesW(*pTestPath) == -1)
  {
    int FoundPath=0;

    string strTarget = *pTestPath;

    TruncPathStrW(strTarget, ScrX-16);

    if(IsHostFile)
    {
      SetLastError(ERROR_FILE_NOT_FOUND);
      if(!Silent)
        MessageW (MSG_WARNING | MSG_ERRORTYPE, 1, UMSG (MError), strTarget, UMSG (MOk));
    }
    else // попытка найти!
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      if(Silent || MessageW (MSG_WARNING | MSG_ERRORTYPE, 2, UMSG (MError), strTarget, UMSG (MNeedNearPath), UMSG(MHYes),UMSG(MHNo)) == 0)
      {
        string strTestPathTemp = *pTestPath;

        wchar_t *Ptr = strTestPathTemp.GetBuffer ();

        while ( true )
        {
            if ( (Ptr=(wchar_t*)wcsrchr(strTestPathTemp,L'\\')) == NULL )
            {
                strTestPathTemp.ReleaseBuffer ();
                break;
            }

            *Ptr=0;

            strTestPathTemp.ReleaseBuffer ();

           if(GetFileAttributesW(strTestPathTemp) != -1)
           {
            int ChkFld=CheckFolderW(strTestPathTemp);
             if(ChkFld > CHKFLD_NOTACCESS && ChkFld < CHKFLD_NOTFOUND)
             {
               if(!(pTestPath->At(0) == L'\\' && pTestPath->At(1) == L'\\' && strTestPathTemp.At(1) == 0))
               {
                 *pTestPath = strTestPathTemp;

                 if( pTestPath->GetLength() == 2) // для случая "C:", иначе попадем в текущий каталог диска C:
                   AddEndSlashW(*pTestPath);
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


BOOL IsDiskInDriveW(const wchar_t *Root)
{
  string strVolName;
  string strDrive;
  DWORD  MaxComSize;
  DWORD  Flags;
  string strFS;

  strDrive = Root;

  AddEndSlashW(strDrive);
  UINT ErrMode = SetErrorMode ( SEM_FAILCRITICALERRORS );
  //если не сделать SetErrorMode - выскочит стандартное окошко "Drive Not Ready"
  BOOL Res = apiGetVolumeInformation (strDrive, &strVolName, NULL, &MaxComSize, &Flags, &strFS);
  SetErrorMode(ErrMode);
  return Res;
}


void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
  if(!SrcPanel)
    SrcPanel=CtrlObject->Cp()->ActivePanel;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  int AnotherType=AnotherPanel->GetType();
  if (AnotherType!=QVIEW_PANEL)
  {
    if(NeedSetUpADir)
    {
      string strCurDir;
      SrcPanel->GetCurDirW(strCurDir);
      AnotherPanel->SetCurDirW(strCurDir,TRUE);
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    }
    else
    {
      if(AnotherPanel->NeedUpdatePanel(SrcPanel))
        AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
      else
      {
        // Сбросим время обновления панели. Если там есть нотификация - обновится сама.
        if (AnotherType==FILE_PANEL)
          ((FileList *)AnotherPanel)->ResetLastUpdateTime();
        AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
      }
    }
  }
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
//  SrcPanel->Redraw();
  if (AnotherType==QVIEW_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//    AnotherPanel->Redraw();
  }
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

    AnotherPanel->GetCurDirW(strAnotherCurDir);
    AddEndSlashW(strAnotherCurDir);

    ConvertNameToFullW(SelName, strFullName);
    AddEndSlashW(strFullName);

    if(wcsstr(strAnotherCurDir,strFullName))
    {
      ((FileList*)AnotherPanel)->CloseChangeNotification();
      return TRUE;
    }
  }
  return FALSE;
}

/* $ 26.10.2003 KM
   Исправление и изменение внутренней логики
*/
/* $ 21.09.2003 KM
   Трансформация строки по заданному типу.
*/
void Transform(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType)
{
  int I,J,L,N;
  char *stop,HexNum[3];

  switch(TransformType)
  {
    case 'X': // Convert common string to hexadecimal string representation
    {
      *(char *)Buffer=0;
      L=strlen(ConvStr);
      N=min((BufLen-1)/2,L);
      for (I=0,J=0;I<N;I++,J+=2)
      {
        // "%02X" - два выходящих символа на каждый один входящий
        sprintf((char *)Buffer+J,"%02X",ConvStr[I]);
        BufLen=J+1;
      }

      RemoveTrailingSpaces((char *)Buffer);
      break;
    }
    case 'S': // Convert hexadecimal string representation to common string
    {
      *(char *)Buffer=0;

      L=strlen(ConvStr);
      char *NewStr=new char[L+1];
      if (NewStr==NULL)
        return;

      // Подготовка временной строки
      memset(NewStr,0,L+1);

      // Обработка hex-строки: убираем пробелы между байтами.
      for (I=0,J=0;ConvStr[I];++I)
      {
        if (ConvStr[I]==' ')
          continue;
        NewStr[J]=ConvStr[I];
        ++J;
      }

      L=strlen(NewStr);
      N=min(BufLen-1,L);
      for (I=0,J=0;I<N;I+=2,J++)
      {
        // "HH" - два входящих символа на каждый один выходящий
        xstrncpy(HexNum,&NewStr[I],2);
        HexNum[2]=0;
        unsigned long value=strtoul(HexNum,&stop,16);
        Buffer[J]=static_cast<unsigned char>(value);
        BufLen=J+1;
      }
      Buffer[J]=0;

      delete []NewStr;
      break;
    }
    default:
      break;
  }
}
/* KM $ */
/* KM $ */

/*
 возвращает PipeFound
*/
int PartCmdLineW(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
  int PipeFound = FALSE;
  int QuoteFound = FALSE;

  apiExpandEnvironmentStrings (CmdStr, strNewCmdStr);
  RemoveExternalSpacesW(strNewCmdStr);

  wchar_t *CmdPtr = strNewCmdStr.GetBuffer();
  wchar_t *ParPtr = NULL;

  // Разделим собственно команду для исполнения и параметры.
  // При этом заодно определим наличие символов переопределения потоков
  // Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

  while (*CmdPtr)
  {
    if (*CmdPtr == L'"')
      QuoteFound = !QuoteFound;
    if (!QuoteFound)
    {
      if (*CmdPtr == L'>' || *CmdPtr == L'<' ||
          *CmdPtr == L'|' || *CmdPtr == L' ' ||
          *CmdPtr == L'/' ||      // вариант "far.exe/?"
          (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && *CmdPtr == L'&') // Для НТ/2к обработаем разделитель команд
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

  UnquoteW(strNewCmdStr);

  return PipeFound;
}


BOOL ProcessOSAliasesW(string &strStr)
{
#if 0
  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
    return FALSE;

  typedef DWORD (WINAPI *PGETCONSOLEALIAS)(
    LPTSTR lpSource,
    LPTSTR lpTargetBuffer,
    DWORD TargetBufferLength,
    LPTSTR lpExeName
  );

  static PGETCONSOLEALIAS GetConsoleAlias=NULL;
  if(!GetConsoleAlias)
  {
    GetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress(GetModuleHandle("kernel32"),"GetConsoleAliasA");
    if(!GetConsoleAlias)
      return FALSE;
  }

  char NewCmdStr[4096];
  char NewCmdPar[2048];

  PartCmdLine(Str,NewCmdStr,sizeof(NewCmdStr),NewCmdPar,sizeof(NewCmdPar));

  string strModuleName;
  apiGetModuleFileName (NULL, strModuleName);
//  if(GetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ModuleName) > 0)
  int b=GetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),"cmd.exe");
  if(b > 0)
  {
    strncat(NewCmdStr,NewCmdPar,sizeof(NewCmdStr)-1);
    xstrncpy(Str,NewCmdStr,SizeStr-1);
    return TRUE;
  }
#endif
  return FALSE;
}


int _MakePath1W(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
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
        case KEY_SHIFTENTER:
        case KEY_ALTSHIFTBRACKET:
        case KEY_CTRLSHIFTBRACKET:
          SrcPanel=Cp->ActivePanel;
          break;
        case KEY_CTRLSHIFTENTER:
        case KEY_ALTSHIFTBACKBRACKET:
        case KEY_CTRLSHIFTBACKBRACKET:
          SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
          break;
      }

      if (SrcPanel!=NULL)
      {
        if(Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER)
        {
          string strShortFileName;
          SrcPanel->GetCurNameW(strPathName,strShortFileName);
          if(SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
            strPathName = strShortFileName;
        }
        else
        {
          /* TODO: Здесь нужно учесть, что у TreeList тоже есть путь :-) */
          if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
            return(FALSE);

          SrcPanel->GetCurDirW(strPathName);
          if (SrcPanel->GetMode()!=PLUGIN_PANEL)
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            SrcFilePanel->GetCurDirW(strPathName);

            {
                if(NeedRealName)
                    SrcFilePanel->CreateFullPathNameW(strPathName, strPathName,FA_DIREC, strPathName,TRUE,ShortNameAsIs);
            }


            if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
              ConvertNameToShortW(strPathName,strPathName);
          }
          else
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            struct OpenPluginInfoW Info;

            CtrlObject->Plugins.GetOpenPluginInfo(SrcFilePanel->GetPluginHandle(),&Info);
            FileList::AddPluginPrefix(SrcFilePanel,strPathName);

            strPathName += NullToEmptyW(Info.CurDir);

          }
          AddEndSlashW(strPathName);
        }

        if(Opt.QuotedName&QUOTEDNAME_INSERT)
          QuoteSpaceW(strPathName);

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

    wcsncpy (LocalName, CurDir, 2);
    LocalName[3] = 0;

    DWORD RemoteNameSize = 0;

    if ( WNetGetConnectionW (LocalName, NULL, &RemoteNameSize) == NO_ERROR)
    {
        wchar_t *NetDir = strNetDir.GetBuffer (RemoteNameSize);

        WNetGetConnectionW (LocalName, NetDir, &RemoteNameSize);

        strNetDir.ReleaseBuffer ();
    }
  }

  if ( strNetDir.At(0)==L'\\' && strNetDir.At(1) == L'\\')
  {
    strComputerName = (const wchar_t*)strNetDir+2;

    wchar_t *ComputerName = strComputerName.GetBuffer ();

    wchar_t *EndSlash=wcschr (ComputerName, L'\\');

    if (EndSlash==NULL)
    {
      strComputerName.ReleaseBuffer ();
      strComputerName=L"";
    }
    else
    {
      *EndSlash=0;
      strComputerName.ReleaseBuffer ();
    }
  }

  return strComputerName;
}

int CheckDisksPropsW(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType)
{
  string strSrcRoot, strDestRoot;
  int SrcDriveType, DestDriveType;

  DWORD SrcVolumeNumber=0, DestVolumeNumber=0;
  string strSrcVolumeName, strDestVolumeName;
  string strSrcFileSystemName, strDestFileSystemName;
  DWORD SrcFileSystemFlags, DestFileSystemFlags;
  DWORD SrcMaximumComponentLength, DestMaximumComponentLength;


  GetPathRootW(SrcPath,strSrcRoot);
  GetPathRootW(DestPath,strDestRoot);

  SrcDriveType=FAR_GetDriveTypeW(strSrcRoot,NULL,TRUE);
  DestDriveType=FAR_GetDriveTypeW(strDestRoot,NULL,TRUE);

  if (!apiGetVolumeInformation(strSrcRoot,&strSrcVolumeName,&SrcVolumeNumber,&SrcMaximumComponentLength,&SrcFileSystemFlags,&strSrcFileSystemName))
    return(FALSE);

  if (!apiGetVolumeInformation(strDestRoot,&strDestVolumeName,&DestVolumeNumber,&DestMaximumComponentLength,&DestFileSystemFlags,&strDestFileSystemName))
    return(FALSE);

  if(CheckedType == CHECKEDPROPS_ISSAMEDISK)
  {
    if (wcspbrk(DestPath,L"\\:")==NULL)
      return TRUE;

    if ((strSrcRoot.At(0)==L'\\' && strSrcRoot.At(1)==L'\\' || strDestRoot.At(0)==L'\\' && strDestRoot.At(1)==L'\\') &&
        LocalStricmpW(strSrcRoot,strDestRoot)!=0)
      return FALSE;

    if ( *SrcPath == 0 || *DestPath == 0 || (SrcPath[1]!=L':' && DestPath[1]!=L':')) //????
      return TRUE;

    if (LocalUpperW(strDestRoot.At(0))==LocalUpperW(strSrcRoot.At(0)))
        return TRUE;

    unsigned __int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
    unsigned __int64 DestTotalSize,DestTotalFree,DestUserFree;

    if (!GetDiskSizeW(strSrcRoot,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
      return FALSE;
    if (!GetDiskSizeW(strDestRoot,&DestTotalSize,&DestTotalFree,&DestUserFree))
      return FALSE;

    if (!(SrcVolumeNumber!=0 &&
        SrcVolumeNumber==DestVolumeNumber &&
        LocalStricmpW(strSrcVolumeName, strDestVolumeName)==0 &&
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

int GetFileFormat (FILE *file, bool *pSignatureFound)
{
	DWORD dwTemp;
	bool bSignatureFound = false;
	int nCodePage = CP_OEMCP;

	if ( fread (&dwTemp, 4, 1, file) == 1 )
	{
		if ( LOWORD (dwTemp) == 0xFEFF )
		{
			nCodePage = CP_UNICODE;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( LOWORD (dwTemp) == 0xFFFE )
		{
			nCodePage = CP_REVERSEBOM;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( (dwTemp & 0x00FFFFFF) == 0xBFBBEF )
		{
			nCodePage = CP_UTF8;
			fseek (file, 3, SEEK_SET);
			bSignatureFound = true;
		}
		else
			fseek (file, 0, SEEK_SET);
	}
	else
		fseek (file, 0, SEEK_SET);

	if ( pSignatureFound )
		*pSignatureFound = bSignatureFound;

	return nCodePage;
}