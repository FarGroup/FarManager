/*
mix.cpp

Куча разных вспомогательных функций

*/

/* Revision: 1.01 28.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  28.06.2000 IS
    ! Функция Unquote стала универсальной
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static DWORD IsCommandExeGUI(char *Command);

long filelen(FILE *FPtr)
{
  SaveFilePos SavePos(FPtr);
  fseek(FPtr,0,SEEK_END);
  return(ftell(FPtr));
}


DWORD NTTimeToDos(FILETIME *ft)
{
  WORD DosDate,DosTime;
  FILETIME ct;
  FileTimeToLocalFileTime(ft,&ct);
  FileTimeToDosDateTime(&ct,&DosDate,&DosTime);
  return(((DWORD)DosDate<<16)|DosTime);
}


void ConvertDate(FILETIME *ft,char *DateText,char *TimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear)
{
  static int DateFormat,DateSeparator,TimeSeparator;
  static int Init=FALSE;
  static SYSTEMTIME lt;
  if (!Init)
  {
    DateFormat=GetDateFormat();
    DateSeparator=GetDateSeparator();
    TimeSeparator=GetTimeSeparator();
    GetLocalTime(&lt);
    Init=TRUE;
  }
  int CurDateFormat=DateFormat;
  if (Brief && CurDateFormat==2)
    CurDateFormat=0;

  SYSTEMTIME st;
  FILETIME ct;

  if (ft->dwHighDateTime==0)
  {
    if (DateText!=NULL)
      *DateText=0;
    if (TimeText!=NULL)
      *TimeText=0;
    return;
  }

  FileTimeToLocalFileTime(ft,&ct);
  FileTimeToSystemTime(&ct,&st);

  if (TimeText!=NULL)
  {
    char *Letter="";
    if (TimeLength==6)
    {
      Letter=(st.wHour<12) ? "a":"p";
      if (st.wHour>12)
        st.wHour-=12;
      if (st.wHour==0)
        st.wHour=12;
    }
    if (TimeLength<7)
      sprintf(TimeText,"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
    else
    {
      char FullTime[100];
      sprintf(FullTime,"%02d%c%02d%c%02d.%03d",st.wHour,TimeSeparator,
              st.wMinute,TimeSeparator,st.wSecond,st.wMilliseconds);
      sprintf(TimeText,"%.*s",TimeLength,FullTime);
    }
  }

  if (DateText!=NULL)
  {
    int Year=st.wYear;
    if (!FullYear)
      Year%=100;
    if (TextMonth)
    {
      char *Month=MSG(MMonthJan+st.wMonth-1);
      switch(CurDateFormat)
      {
        case 0:
          sprintf(DateText,"%3.3s %2d %02d",Month,st.wDay,Year);
          break;
        case 1:
          sprintf(DateText,"%2d %3.3s %02d",st.wDay,Month,Year);
          break;
        default:
          sprintf(DateText,"%02d %3.3s %2d",Year,Month,st.wDay);
          break;
      }
    }
    else
      switch(CurDateFormat)
      {
        case 0:
          sprintf(DateText,"%02d%c%02d%c%02d",st.wMonth,DateSeparator,st.wDay,DateSeparator,Year);
          break;
        case 1:
          sprintf(DateText,"%02d%c%02d%c%02d",st.wDay,DateSeparator,st.wMonth,DateSeparator,Year);
          break;
        default:
          sprintf(DateText,"%02d%c%02d%c%02d",Year,DateSeparator,st.wMonth,DateSeparator,st.wDay);
          break;
      }
  }

  if (Brief)
  {
    DateText[TextMonth ? 6:5]=0;
    if (lt.wYear!=st.wYear)
      sprintf(TimeText,"%5d",st.wYear);
  }
}


int GetDateFormat()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE,Info,sizeof(Info));
  return(atoi(Info));
}


int GetDateSeparator()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info,sizeof(Info));
  return(*Info);
}


int GetTimeSeparator()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STIME,Info,sizeof(Info));
  return(*Info);
}


int Execute(char *CmdStr,int AlwaysWaitFinish,int SeparateWindow,int DirectRun)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char ExecLine[1024],CommandName[NM];
  char OldTitle[512];
  int ExitCode;

  int Visible,Size;
  GetCursorType(Visible,Size);
  SetCursorType(TRUE,-1);

  int PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);
  ScrBuf.Flush();

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  int NT=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT;
  int OldNT=NT && WinVer.dwMajorVersion<4;

  *CommandName=0;
  GetEnvironmentVariable("COMSPEC",CommandName,sizeof(CommandName));

  Panel *PassivePanel=CtrlObject->GetAnotherPanel(CtrlObject->ActivePanel);
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
      PassivePanel->GetType()==FILE_PANEL)
    for (int I=0;CmdStr[I]!=0;I++)
      if (isalpha(CmdStr[I]) && CmdStr[I+1]==':' && CmdStr[I+2]!='\\')
      {
        char SavePath[NM],PanelPath[NM],SetPathCmd[NM];
        GetCurrentDirectory(sizeof(SavePath),SavePath);
        PassivePanel->GetCurDir(PanelPath);
        sprintf(SetPathCmd,"%s /C chdir %s",CommandName,QuoteSpace(PanelPath));
        CreateProcess(NULL,SetPathCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        chdir(SavePath);
      }

  char *CmdPtr=CmdStr;
//  while (isspace(*CmdPtr))
//    CmdPtr++;

  DWORD GUIType=IsCommandExeGUI(CmdPtr);

  if (DirectRun && !SeparateWindow)
    strcpy(ExecLine,CmdPtr);
  else
  {
    sprintf(ExecLine,"%s /C",CommandName);
    if (!OldNT && (SeparateWindow || GUIType && (NT || AlwaysWaitFinish)))
    {
      strcat(ExecLine," start");
      if (AlwaysWaitFinish)
        strcat(ExecLine," /wait");
      if (NT && *CmdPtr=='\"')
        strcat(ExecLine," \"\"");
    }
    strcat(ExecLine," ");
    strcat(ExecLine,CmdPtr);
  }

  SetFarTitle(CmdPtr);
  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  DWORD CreateFlags=0;
  if (SeparateWindow && OldNT)
    CreateFlags=CREATE_NEW_CONSOLE;


  if (SeparateWindow==2)
  {
    char AnsiLine[NM];
    SHELLEXECUTEINFO si;
    OemToChar(CmdPtr,AnsiLine);

    if (PointToName(AnsiLine)==AnsiLine)
    {
      char FullName[2*NM];
      sprintf(FullName,".\\%s",AnsiLine);
      strcpy(AnsiLine,FullName);
    }

    memset(&si,0,sizeof(si));
    si.cbSize=sizeof(si);
    si.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
    si.lpFile=AnsiLine;
    si.lpVerb=GetShellAction((char *)si.lpFile);
    si.nShow=SW_SHOWNORMAL;
    SetFileApisToANSI();
    ExitCode=ShellExecuteEx(&si);
    SetFileApisToOEM();
    pi.hProcess=si.hProcess;
  }
  else
    ExitCode=CreateProcess(NULL,ExecLine,NULL,NULL,0,CreateFlags,
                           NULL,NULL,&si,&pi);

  StartExecTime=clock();
  if (ExitCode)
  {
    if (!SeparateWindow && !GUIType || AlwaysWaitFinish)
      WaitForSingleObject(pi.hProcess,INFINITE);
    int CurScrX=ScrX,CurScrY=ScrY;
//    ReopenConsole();
    GetVideoMode();
    if (CurScrX!=ScrX || CurScrY!=ScrY)
      CtrlObject->SetScreenPositions();
    GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
    if (SeparateWindow!=2)
      CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ScrBuf.FillBuf();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    if (SeparateWindow!=2)
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),SeparateWindow==2 ? CmdPtr:ExecLine,MSG(MOk));
    ExitCode=-1;
  }
  SetFarConsoleMode();
  GetCursorType(Visible,Size);
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
      WinVer.dwBuildNumber<=0x4000457)
    WriteInput(VK_F16);
  SetConsoleTitle(OldTitle);
  return(ExitCode);
}


char* GetShellAction(char *FileName)
{
  char Value[80],*ExtPtr;
  LONG ValueSize;
  if ((ExtPtr=strrchr(FileName,'.'))==NULL)
    return(NULL);
  ValueSize=sizeof(Value);
  if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)!=ERROR_SUCCESS)
    return(NULL);
  strcat(Value,"\\shell");

  HKEY hKey;
  if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static char Action[80];
  ValueSize=sizeof(Action);
  if (RegQueryValueEx(hKey,"",NULL,NULL,(LPTSTR)Action,(LPDWORD)&ValueSize)!=ERROR_SUCCESS)
    return(NULL);
  RegCloseKey(hKey);
  return(*Action==0 ? NULL:Action);
}


DWORD IsCommandExeGUI(char *Command)
{
  char FileName[4096],FullName[4096],*EndName,*FilePart;
  if (*Command=='\"')
  {
    OemToChar(Command+1,FileName);
    if ((EndName=strchr(FileName,'\"'))!=NULL)
      *EndName=0;
  }
  else
  {
    OemToChar(Command,FileName);
    if ((EndName=strpbrk(FileName," \t/"))!=NULL)
      *EndName=0;
  }
  int GUIType=FALSE;

  SetFileApisToANSI();
  if (SearchPath(NULL,FileName,".exe",sizeof(FullName),FullName,&FilePart))
  {
    SHFILEINFO sfi;
    DWORD ExeType=SHGetFileInfo(FullName,0,&sfi,sizeof(sfi),SHGFI_EXETYPE);
    GUIType=HIWORD(ExeType)>=0x0300 && HIWORD(ExeType)<=0x1000 &&
            (LOWORD(ExeType)=='NE' || LOWORD(ExeType)=='PE');
  }
  SetFileApisToOEM();
  return(GUIType);
}


void InsertCommas(unsigned long Number,char *Dest)
{
  sprintf(Dest,"%u",Number);
  for (int I=strlen(Dest)-4;I>=0;I-=3)
    if (Dest[I])
    {
      memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
      Dest[I+1]=',';
    }
}


void InsertCommas(int64 li,char *Dest)
{
  if (li<1000000000 && 0)
    InsertCommas(li.LowPart,Dest);
  else
  {
    li.itoa(Dest);
    for (int I=strlen(Dest)-4;I>=0;I-=3)
      if (Dest[I])
      {
        memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
        Dest[I+1]=',';
      }
  }
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


char* PointToName(char *Path)
{
  char *NamePtr=Path;
  while (*Path)
  {
    if (*Path=='\\' || *Path=='/' || *Path==':' && Path==NamePtr+1)
      NamePtr=Path+1;
    Path++;
  }
  return(NamePtr);
}


void GetPathRoot(char *Path,char *Root)
{
  char TempRoot[NM],*ChPtr;
  strncpy(TempRoot,Path,NM);
  if (*TempRoot==0)
    strcpy(TempRoot,"\\");
  else
    if (TempRoot[0]=='\\' && TempRoot[1]=='\\')
    {
      if ((ChPtr=strchr(TempRoot+2,'\\'))!=NULL)
        if ((ChPtr=strchr(ChPtr+1,'\\'))!=NULL)
          *(ChPtr+1)=0;
        else
          strcat(TempRoot,"\\");
    }
    else
      if ((ChPtr=strchr(TempRoot,'\\'))!=NULL)
        *(ChPtr+1)=0;
      else
        if ((ChPtr=strchr(TempRoot,':'))!=NULL)
          strcpy(ChPtr+1,"\\");
  strncpy(Root,TempRoot,NM);
}


int CmpName(char *pattern,char *string,int skippath)
{
  char stringc,patternc,rangec;
  int match;
  static int depth=0;

  if (skippath)
    string=PointToName(string);

  for (;; ++string)
  {
    stringc=LocalUpper(*string);
    patternc=LocalUpper(*pattern++);
    switch (patternc)
    {
      case 0:
        return(stringc==0);
      case '?':
        if (stringc == 0)
          return(FALSE);

        break;
      case '*':
        if (!*pattern)
          return(TRUE);

        if (*pattern=='.')
        {
          if (pattern[1]=='*' && pattern[2]==0 && depth==0)
            return(TRUE);
          char *dot=strchr(string,'.');
          if (pattern[1]==0)
            return (dot==NULL || dot[1]==0);
          if (dot!=NULL)
          {
            string=dot;
            if (strpbrk(pattern,"*?[")==NULL && strchr(string+1,'.')==NULL)
              return(LocalStricmp(pattern+1,string+1)==0);
          }
        }

        while (*string)
        {
          depth++;
          int CmpCode=CmpName(pattern,string++,FALSE);
          depth--;
          if (CmpCode)
            return(TRUE);
        }
        return(FALSE);
      case '[':
        if (strchr(pattern,']')==NULL)
        {
          if (patternc != stringc)
            return (FALSE);
          break;
        }
        if (*pattern && *(pattern+1)==']')
        {
          if (*pattern!=*string)
            return(FALSE);
          pattern+=2;
          break;
        }
        match = 0;
        while ((rangec = LocalUpper(*pattern++))!=0)
        {
          if (rangec == ']')
            if (match)
              break;
            else
              return(FALSE);
          if (match)
            continue;
          if (rangec == '-' && *(pattern - 2) != '[' && *pattern != ']')
          {
            match = (stringc <= LocalUpper(*pattern) &&
                     LocalUpper(*(pattern - 2)) <= stringc);
            pattern++;
          }
          else
            match = (stringc == rangec);
        }
        if (rangec == 0)
          return(FALSE);
        break;
      default:
        if (patternc != stringc)
          if (patternc=='.' && stringc==0 && !CmpNameSearchMode)
            return(*pattern!='.' && CmpName(pattern,string));
          else
            return(FALSE);
        break;
    }
  }
}


/*
void ShowHeap()
{
  _HEAPINFO hi;

  Log( "" );
  Log( "   Size   Status" );
  Log( "   ----   ------" );
  hi._pentry=NULL;
  while( _rtl_heapwalk( &hi ) == _HEAPOK )
    Log( "%7u    %s", hi._size, hi._useflag ? "used" : "free" );
}


void CheckHeap(int NumLine)
{
  if (_heapchk()==_HEAPBADNODE)
  {
    char Line[10];
    sprintf(Line,"%d",NumLine);
    Message(MSG_WARNING,1,MSG(MError),"Heap broken",Line,MSG(MOk));
  }
}


void Log(char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  char OutStr[1024];
  vsprintf(OutStr,fmt,argptr);

  char FarFileName[NM];
  GetModuleFileName(NULL,FarFileName,sizeof(FarFileName));
  strcpy(strrchr(FarFileName,'\\')+1,"far.log");
  FILE *LogFile=fopen(FarFileName,"ab");
  if (LogFile!=NULL)
  {
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    fprintf(LogFile,"%02d:%02d:%02d %s\r\n",tm.wHour,tm.wMinute,tm.wSecond,OutStr);
    fclose(LogFile);
  }
  va_end(argptr);
}
*/


char* QuoteSpace(char *Str)
{
  if (*Str=='-' || *Str=='^' || strpbrk(Str," &+,")!=NULL)
  {
    char *TmpStr=new char[strlen(Str)+3];
    sprintf(TmpStr,"\"%s\"",Str);
    strcpy(Str,TmpStr);
    delete TmpStr;
  }
  return(Str);
}


char* QuoteSpaceOnly(char *Str)
{
  if (strchr(Str,' ')!=NULL)
  {
    char *TmpStr=new char[strlen(Str)+3];
    sprintf(TmpStr,"\"%s\"",Str);
    strcpy(Str,TmpStr);
    delete TmpStr;
  }
  return(Str);
}


char* TruncStr(char *Str,int MaxLength)
{
  int Length;
  if (MaxLength<0)
    MaxLength=0;
  if ((Length=strlen(Str))>MaxLength)
    if (MaxLength>3)
    {
      char *TmpStr=new char[MaxLength+5];
      sprintf(TmpStr,"...%s",Str+Length-MaxLength+3);
      strcpy(Str,TmpStr);
      delete TmpStr;
    }
    else
      Str[MaxLength]=0;
  return(Str);
}


char* TruncPathStr(char *Str,int MaxLength)
{
  char *Root=NULL;
  if (Str[0]!=0 && Str[1]==':' && Str[2]=='\\')
    Root=Str+3;
  else
    if (Str[0]=='\\' && Str[1]=='\\' && (Root=strchr(Str+2,'\\'))!=NULL &&
        (Root=strchr(Root+1,'\\'))!=NULL)
      Root++;
  if (Root==NULL || Root-Str+5>MaxLength)
    return(TruncStr(Str,MaxLength));
  int Length=strlen(Str);
  if (Length>MaxLength)
  {
    char *MovePos=Root+Length-MaxLength+3;
    memmove(Root+3,MovePos,strlen(MovePos)+1);
    memcpy(Root,"...",3);
  }
  return(Str);
}


void RemoveLeadingSpaces(unsigned char *Str)
{
  char *ChPtr;
  for (ChPtr=Str;isspace(*ChPtr);ChPtr++)
         ;
  if (ChPtr!=Str)
    memmove(Str,ChPtr,strlen(ChPtr)+1);
}


void RemoveTrailingSpaces(unsigned char *Str)
{
  for (int I=strlen(Str)-1;I>=0;I--)
    if (isspace(Str[I]) || iseol(Str[I]))
      Str[I]=0;
    else
      break;
}


void ConvertNameToFull(char *Src,char *Dest)
{
  char FullName[NM],AnsiName[NM],*NamePtr=PointToName(Src);
  if (NamePtr==Src && (NamePtr[0]!='.' || NamePtr[1]!=0))
  {
    GetCurrentDirectory(sizeof(FullName),FullName);
    AddEndSlash(FullName);
    strcat(FullName,Src);
    strcpy(Dest,FullName);
    return;
  }
  if (isalpha(Src[0]) && Src[1]==':' || Src[0]=='\\' && Src[1]=='\\')
    if (*NamePtr && (*NamePtr!='.' || NamePtr[1]!=0 && (NamePtr[1]!='.' || NamePtr[2]!=0)))
      if (strstr(Src,"\\..\\")==NULL && strstr(Src,"\\.\\")==NULL)
      {
        if (Dest!=Src)
          strcpy(Dest,Src);
        return;
      }

  SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  if (GetFullPathName(AnsiName,sizeof(FullName),FullName,&NamePtr))
    CharToOem(FullName,Dest);
  else
    strcpy(Dest,Src);
  SetFileApisToOEM();
}


void ConvertNameToShort(char *Src,char *Dest)
{
  char ShortName[NM],AnsiName[NM];
  SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)))
    CharToOem(ShortName,Dest);
  else
    strcpy(Dest,Src);
  SetFileApisToOEM();
}


int HiStrlen(char *Str)
{
  int Length=0;
  while (*Str)
  {
    if (*Str!='&')
      Length++;
    Str++;
  }
  return(Length);
}


int CopyToClipboard(char *Data)
{
  long DataSize;
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!OpenClipboard(NULL))
      return(FALSE);
    EmptyClipboard();
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        GlobalUnlock(hData);
        SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        OemToChar((LPCSTR)GData,(LPTSTR)GData);
        GlobalUnlock(hData);
        SetClipboardData(CF_TEXT,(HANDLE)hData);
      }
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize*2))!=NULL)
        if ((GData=GlobalLock(hData))!=NULL)
        {
          MultiByteToWideChar(CP_OEMCP,0,Data,-1,(LPWSTR)GData,BufferSize);
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
        }
    CloseClipboard();
  }
  return(TRUE);
}


int CopyFormatToClipboard(char *Format,char *Data)
{
  int FormatType=RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(FALSE);

  long DataSize;
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!OpenClipboard(NULL))
      return(FALSE);
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
        SetClipboardData(FormatType,(HANDLE)hData);
      }
    CloseClipboard();
  }
  return(TRUE);
}


char* PasteFromClipboard()
{
  HANDLE hClipData;
  if (!OpenClipboard(NULL))
    return(NULL);
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;
  while ((Format=EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }
  char *ClipText=NULL;
  if ((hClipData=GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
  {
    int BufferSize;
    char *ClipAddr=(char *)GlobalLock(hClipData);
    if (Unicode)
      BufferSize=lstrlenW((LPCWSTR)ClipAddr)+1;
    else
      BufferSize=strlen(ClipAddr)+1;
    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
      if (Unicode)
        WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)ClipAddr,-1,ClipText,BufferSize,NULL,NULL);
      else
        if (ReadType==CF_TEXT)
          CharToOem(ClipAddr,ClipText);
        else
          strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  CloseClipboard();
  return(ClipText);
}


char* PasteFormatFromClipboard(char *Format)
{
  int FormatType=RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(NULL);
  if (!OpenClipboard(NULL))
    return(NULL);
  HANDLE hClipData;
  char *ClipText=NULL;
  if ((hClipData=GetClipboardData(FormatType))!=NULL)
  {
    char *ClipAddr=(char *)GlobalLock(hClipData);
    int BufferSize=strlen(ClipAddr)+1;
    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
      strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  CloseClipboard();
  return(ClipText);
}


int GetFileTypeByName(char *Name)
{
  HANDLE hFile=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FILE_TYPE_UNKNOWN);
  int Type=GetFileType(hFile);
  CloseHandle(hFile);
  return(Type);
}


void SetFarTitle(char *Title)
{
  char FarTitle[2*NM];
  sprintf(FarTitle,"%.256s - Far",Title);
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    OemToChar(FarTitle,FarTitle);
  SetConsoleTitle(FarTitle);
}


int GetDirInfo(char *Title,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize,int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               int EnhBreak)
{
  SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA FindData;
  char FullName[NM];
  int MsgOut=0;
  clock_t StartTime=clock();
  char FullDirName[NM],DriveRoot[NM];

  SetCursorType(FALSE,0);
  ConvertNameToFull(DirName,FullDirName);
  GetPathRoot(FullDirName,DriveRoot);

  if ((ClusterSize=GetClusterSize(DriveRoot))==0)
  {
    DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

    if (GetDiskFreeSpace(DriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
      ClusterSize=SectorsPerCluster*BytesPerSector;
  }

  DirCount=FileCount=0;
  FileSize=CompressedFileSize=RealSize=0;
  ScTree.SetFindPath(DirName,"*.*");
  while (ScTree.GetNextName(&FindData,FullName))
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
          if (EnhBreak)
            return(-1);
          GetInputRecord(&rec);
          break;
      }
    }
    if (!MsgOut && MsgWaitTime!=0xffffffff && clock()-StartTime > MsgWaitTime)
    {
      SetCursorType(FALSE,0);
      Message(0,0,Title,MSG(MScanningFolder),DirName);
      MsgOut=1;
    }
    if (FindData.dwFileAttributes & FA_DIREC)
      DirCount++;
    else
    {
      FileCount++;
      int64 CurSize(FindData.nFileSizeHigh,FindData.nFileSizeLow);
      FileSize+=CurSize;
      if (FindData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
      {
        DWORD CompressedSize,CompressedSizeHigh;
        CompressedSize=GetCompressedFileSize(FullName,&CompressedSizeHigh);
        if (CompressedSize!=0xFFFFFFFF || GetLastError()==NO_ERROR)
          CurSize.Set(CompressedSizeHigh,CompressedSize);
      }
      CompressedFileSize+=CurSize;
      if (ClusterSize>0)
      {
        RealSize+=CurSize;
        int Slack=(CurSize%ClusterSize).LowPart;
        if (Slack>0)
          RealSize+=ClusterSize-Slack;
      }
    }
  }
  return(1);
}


int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize)
{
  struct PluginPanelItem *PanelItem=NULL;
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
        int64 CurSize(PanelItem[I].FindData.nFileSizeHigh,PanelItem[I].FindData.nFileSizeLow);
        FileSize+=CurSize;
        if (PanelItem[I].PackSize==0 && PanelItem[I].PackSizeHigh==0)
          CompressedFileSize+=CurSize;
        else
        {
          int64 AddSize(PanelItem[I].PackSizeHigh,PanelItem[I].PackSize);
          CompressedFileSize+=AddSize;
        }
      }
    }
  }
  if (PanelItem!=NULL)
    FarFreeDirList(PanelItem);
  return(ExitCode);
}


void AddEndSlash(char *Path)
{
  int Length=strlen(Path);
  if (Length==0 || Path[Length-1]!='\\')
    strcat(Path,"\\");
}


char *NullToEmpty(char *Str)
{
  return (Str==NULL) ? "":Str;
}


void CenterStr(char *Src,char *Dest,int Length)
{
  char TempSrc[512];
  int SrcLength=strlen(Src);
  strcpy(TempSrc,Src);
  if (SrcLength>=Length)
    strcpy(Dest,TempSrc);
  else
  {
    int Space=(Length-SrcLength)/2;
    sprintf(Dest,"%*s%s%*s",Space,"",TempSrc,Length-Space-SrcLength,"");
  }
}


char *GetCommaWord(char *Src,char *Word)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='[' && strchr(Src+1,']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==']')
      SkipBrackets=FALSE;
    if (*Src==',' && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (isspace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}


int GetString(char *Title,char *SubTitle,char *HistoryName,char *SrcText,
    char *DestText,int DestLength,char *HelpTopic,int EnableEmpty,int Password)
{
  static struct DialogData StrDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,4,0,0,0,0,"",
    DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
    DI_EDIT,5,3,70,3,1,0,0,1,""
  };
  MakeDialogItems(StrDlgData,StrDlg);
  if (HistoryName!=NULL)
  {
    StrDlg[2].Selected=(int)HistoryName;
    StrDlg[2].Flags|=DIF_HISTORY;
  }
  if (Password)
    StrDlg[2].Type=DI_PSWEDIT;
  strcpy(StrDlg[0].Data,Title);
  strcpy(StrDlg[1].Data,SubTitle);
  strncpy(StrDlg[2].Data,SrcText,sizeof(StrDlg[2].Data));
  StrDlg[2].Data[sizeof(StrDlg[2].Data)-1]=0;
  Dialog Dlg(StrDlg,sizeof(StrDlg)/sizeof(StrDlg[0]));
  Dlg.SetPosition(-1,-1,76,6);
  if (HelpTopic!=NULL)
    Dlg.SetHelp(HelpTopic);
  Dlg.Process();
  if (DestLength<1 || Dlg.GetExitCode()!=2 || !EnableEmpty && *StrDlg[2].Data==0)
    return(FALSE);
  strncpy(DestText,StrDlg[2].Data,DestLength-1);
  DestText[DestLength-1]=0;
  return(TRUE);
}


void ScrollBar(int X1,int Y1,int Length,int Current,int Total)
{
  char OutStr[200];
  int ThumbPos;
  if ((Length-=2)<1)
    return;
  if (Total>0)
    ThumbPos=Length*Current/Total;
  else
    ThumbPos=0;
  if (ThumbPos>=Length)
    ThumbPos=Length-1;
  GotoXY(X1,Y1);
  memset(OutStr,'░',Length);
  OutStr[ThumbPos]='▓';
  OutStr[Length]=0;
  vmprintf("%s",OutStr);
}


int GetFileOwner(char *Computer,char *Name,char *Owner)
{
  SECURITY_INFORMATION si;
  SECURITY_DESCRIPTOR *sd;
  char sddata[500];
  DWORD Needed;
  *Owner=0;
  si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;
  sd=(SECURITY_DESCRIPTOR *)sddata;

  char AnsiName[NM];
  OemToChar(Name,AnsiName);
  SetFileApisToANSI();
  int GetCode=GetFileSecurity(AnsiName,si,sd,sizeof(sddata),&Needed);
  SetFileApisToOEM();

  if (!GetCode)
    return(FALSE);
  PSID pOwner;
  BOOL OwnerDefaulted;
  if (!GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
    return(FALSE);
  char AccountName[200],DomainName[200];
  DWORD AccountLength=sizeof(AccountName),DomainLength=sizeof(DomainName);
  SID_NAME_USE snu;
  if (!LookupAccountSid(Computer,pOwner,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
    return(FALSE);
  CharToOem(AccountName,Owner);
  return(TRUE);
}


int GetNumberOfLinks(char *Name)
{
  HANDLE hFile=CreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(1);
  BY_HANDLE_FILE_INFORMATION bhfi;
  int GetCode=GetFileInformationByHandle(hFile,&bhfi);
  CloseHandle(hFile);
  return(GetCode ? bhfi.nNumberOfLinks:0);
}


void ShowSeparator(int Length)
{
  if (Length>1)
  {
    char Separator[1024];
    memset(Separator,'─',Length);
    Separator[0]='╟';
    Separator[Length-1]='╢';
    Separator[Length]=0;
    BoxText(Separator);
  }
}


int IsFolderNotEmpty(char *Name)
{
  WIN32_FIND_DATA fdata;
  char FileName[NM];
  ScanTree ScTree(FALSE,FALSE);
  ScTree.SetFindPath(Name,"*.*");
  while (ScTree.GetNextName(&fdata,FileName))
    if (strcmp(FileName,".")!=0 && strcmp(FileName,"..")!=0)
      return(TRUE);
  return(FALSE);
}


void RemoveHighlights(char *Str)
{
  int HCount=0;
  while (1)
  {
    if (*Str=='&')
      HCount++;
    else
      *(Str-HCount)=*Str;
    if (*Str==0)
      break;
    Str++;
  }
}


int IsCaseMixed(char *Str)
{
  while (*Str && !LocalIsalpha(*Str))
    Str++;
  int Case=LocalIslower(*Str);
  while (*(Str++))
    if (LocalIsalpha(*Str) && LocalIslower(*Str)!=Case)
      return(TRUE);
  return(FALSE);
}


int IsCaseLower(char *Str)
{
  for (;*Str!=0;Str++)
    if (LocalIsalpha(*Str) && !LocalIslower(*Str))
      return(FALSE);
  return(TRUE);
}


int DeleteFileWithFolder(char *FileName)
{
  char FolderName[NM],*Slash;
  SetFileAttributes(FileName,0);
  remove(FileName);
  strcpy(FolderName,FileName);
  if ((Slash=strrchr(FolderName,'\\'))!=NULL)
    *Slash=0;
  return(RemoveDirectory(FolderName));
}


char* FarMSG(int MsgID)
{
  return(Lang.GetMsg(MsgID));
}


BOOL GetDiskSize(char *Root,int64 *TotalSize,int64 *TotalFree,int64 *UserFree)
{
typedef BOOL (WINAPI *GETDISKFREESPACEEX)(
    LPCTSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
   );
  static GETDISKFREESPACEEX pGetDiskFreeSpaceEx=NULL;
  static int LoadAttempt=FALSE;
  int ExitCode;

  ULARGE_INTEGER uiTotalSize,uiTotalFree,uiUserFree;
  uiUserFree.u.LowPart=uiUserFree.u.HighPart=0;
  uiTotalSize.u.LowPart=uiTotalSize.u.HighPart=0;
  uiTotalFree.u.LowPart=uiTotalFree.u.HighPart=0;

  if (!LoadAttempt && pGetDiskFreeSpaceEx==NULL)
  {
    HMODULE hKernel=GetModuleHandle("kernel32.dll");
    if (hKernel!=NULL)
      pGetDiskFreeSpaceEx=(GETDISKFREESPACEEX)GetProcAddress(hKernel,"GetDiskFreeSpaceExA");
    LoadAttempt=TRUE;
  }
  if (pGetDiskFreeSpaceEx!=NULL)
  {
    ExitCode=pGetDiskFreeSpaceEx(Root,&uiUserFree,&uiTotalSize,&uiTotalFree);
    if (uiUserFree.u.HighPart>uiTotalFree.u.HighPart)
      uiUserFree.u=uiTotalFree.u;
  }

  if (pGetDiskFreeSpaceEx==NULL || ExitCode==0 ||
      uiTotalSize.u.HighPart==0 && uiTotalSize.u.LowPart==0)
  {
    DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
    ExitCode=GetDiskFreeSpace(Root,&SectorsPerCluster,&BytesPerSector,
                              &FreeClusters,&Clusters);
    uiTotalSize.u.LowPart=SectorsPerCluster*BytesPerSector*Clusters;
    uiTotalSize.u.HighPart=0;
    uiTotalFree.u.LowPart=SectorsPerCluster*BytesPerSector*FreeClusters;
    uiTotalFree.u.HighPart=0;
    uiUserFree.u=uiTotalFree.u;
  }
  TotalSize->Set(uiTotalSize.u.HighPart,uiTotalSize.u.LowPart);
  TotalFree->Set(uiTotalFree.u.HighPart,uiTotalFree.u.LowPart);
  UserFree->Set(uiUserFree.u.HighPart,uiUserFree.u.LowPart);
  return(ExitCode);
}


void DeleteDirTree(char *Dir)
{
  if (*Dir==0 || (Dir[0]=='\\' || Dir[0]=='/') && Dir[1]==0 ||
      Dir[1]==':' && (Dir[2]=='\\' || Dir[2]=='/') && Dir[3]==0)
    return;
  char FullName[NM];
  WIN32_FIND_DATA FindData;
  ScanTree ScTree(TRUE);

  ScTree.SetFindPath(Dir,"*.*");
  while (ScTree.GetNextName(&FindData,FullName))
  {
    SetFileAttributes(FullName,0);
    if (FindData.dwFileAttributes & FA_DIREC)
    {
      if (ScTree.IsDirSearchDone())
        RemoveDirectory(FullName);
    }
    else
      DeleteFile(FullName);
  }
  SetFileAttributes(Dir,0);
  RemoveDirectory(Dir);
}


#pragma option -a4
int MkLink(char *Src,char *Dest)
{
  struct CORRECTED_WIN32_STREAM_ID
  {
    DWORD          dwStreamId ;
    DWORD          dwStreamAttributes ;
    LARGE_INTEGER  Size ;
    DWORD          dwStreamNameSize ;
    WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
  } StreamId;

  char FileSource[NM],FileDest[NM];
  WCHAR FileLink[NM];

  HANDLE hFileSource;

  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD cbPathLen;
  DWORD StreamSize;

  BOOL bSuccess;

  ConvertNameToFull(Src,FileSource);
  ConvertNameToFull(Dest,FileDest);
  MultiByteToWideChar(CP_OEMCP,0,FileDest,-1,FileLink,sizeof(FileLink)/sizeof(FileLink[0]));

  hFileSource = CreateFile(FileSource,FILE_WRITE_ATTRIBUTES,
                FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

  if(hFileSource == INVALID_HANDLE_VALUE)
    return(FALSE);

  lpContext = NULL;
  cbPathLen = (lstrlenW(FileLink) + 1) * sizeof(WCHAR);

  StreamId.dwStreamId = BACKUP_LINK;
  StreamId.dwStreamAttributes = 0;
  StreamId.dwStreamNameSize = 0;
  StreamId.Size.u.HighPart = 0;
  StreamId.Size.u.LowPart = cbPathLen;

  StreamSize=sizeof(StreamId)-sizeof(WCHAR **)+StreamId.dwStreamNameSize;

  bSuccess = BackupWrite(hFileSource,(LPBYTE)&StreamId,StreamSize,
             &dwBytesWritten,FALSE,FALSE,&lpContext);

  int LastError=0;

  if (bSuccess)
  {
    bSuccess = BackupWrite(hFileSource,(LPBYTE)FileLink,cbPathLen,
                &dwBytesWritten,FALSE,FALSE,&lpContext);
    if (!bSuccess)
      LastError=GetLastError();

    BackupWrite(hFileSource,NULL,0,&dwBytesWritten,TRUE,FALSE,&lpContext);
  }
  else
    LastError=GetLastError();

  CloseHandle(hFileSource);

  if (LastError)
    SetLastError(LastError);

  return(bSuccess);
}
#pragma option -a.


int GetClusterSize(char *Root)
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

  HANDLE hDevice = CreateFile("\\\\.\\vwin32", 0, 0, NULL, 0,
                              FILE_FLAG_DELETE_ON_CLOSE, NULL);

  if (hDevice==INVALID_HANDLE_VALUE)
    return(0);

  DiskInfo.ExtFree_Level=0;

  reg.reg_EAX = 0x7303;
  reg.reg_EDX = (DWORD)Root;
  reg.reg_EDI = (DWORD)&DiskInfo;
  reg.reg_ECX = sizeof(DiskInfo);
  reg.reg_Flags = 0x0001;

  fResult=DeviceIoControl(hDevice,6,&reg,sizeof(reg),&reg,sizeof(reg),&cb,0);

  CloseHandle(hDevice);
  if (!fResult || (reg.reg_Flags & 0x0001))
    return(0);
  return(DiskInfo.ExtFree_SectorsPerCluster*DiskInfo.ExtFree_BytesPerSector);
}


/* $ 28.06.2000 IS
  Теперь функция Unquote убирает ВСЕ начальные и заключительные кавычки
*/
void Unquote(char *Str)
{
 if(Str)
  {
   if(int Length=lstrlen(Str))
   {
    /*убираем заключительные кавычки*/
    Length--;
    while(Str[Length]=='\"')
    {
     Str[Length]='\0';
     if(!Length)break;
     Length--;
    }
    /*убираем начальные кавычки*/
    char *start=Str;
    while(*start=='\"') start++;
    if(start!=Str) memcpy(Str,start,Length+Str-start+2);
   }
  }
}
/* IS $ */


bool GetSubstName(char *LocalName,char *SubstName,int SubstSize)
{
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
  {
    char Name[512];
    if (QueryDosDevice(LocalName,Name,sizeof(Name))==0)
      return(false);
    if (strncmp(Name,"\\??\\",4)!=0)
      return(false);
    strncpy(SubstName,Name+4,SubstSize);
    return(true);
  }
  return(false);
}

