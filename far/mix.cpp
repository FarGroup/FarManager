/*
mix.cpp

Куча разных вспомогательных функций

*/

/* Revision: 1.37 23.10.2000 $ */

/*
Modify:
  23.10.2000 SVS
    ! Узаконненая версия SysLog :-)
      Задолбался я ставить комметарии после AT ;-)
  20.10.2000 SVS
    ! ProcessName: Flags должен быть DWORD, а не int
  20.10.2000 SVS
    + SysLog
  16.10.2000 tran
    + PasteFromClipboardEx(int max);
  09.10.2000 IS
    + Новые функции для обработки имени файла: ProcessName, ConvertWildcards
  27.09.2000 skv
   + DeleteBuffer. Удалять то, что вернул PasteFromClipboard.
  24.09.2000 SVS
   + Функция KeyNameToKey - получение кода клавиши по имени
     Если имя не верно или нет такого - возвращается -1
  20.09.2000 SVS
   ! удалил FolderPresent (блин, совсем крышу сорвало :-(
  20.09.2000 SVS
    ! Уточнения в функции Xlat()
  19.09.2000 SVS
    + функция FolderPresent - "сужествует ли каталог"
  19.09.2000 SVS
    + IsFolderNotEmpty немного "ускорим"
    ! Функция FarMkTemp - уточнение!
  18.09.2000 skv
    + в IsCommandExeGUI проверка на наличие .bat и .cmd в тек. директории
  18.09.2000 SVS
    ! FarRecurseSearch -> FarRecursiveSearch
    ! Исправление ошибочки в функции FarMkTemp :-)))
  14.09.2000 SVS
    + Функция FarMkTemp - получение имени временного файла с полным путем.
  10.09.2000 SVS
    ! KeyToText возвращает BOOL
  10.09.2000 tran
    + FSF/FarRecurseSearch
  10.09.2000 SVS
    ! Наконец-то нашлось приемлемое имя для QWERTY -> Xlat.
  08.09.2000 SVS
    - QWERTY - ошибочка вралась :-)))
    ! QWERTY -> Transliterate
    ! QWED_SWITCHKEYBLAYER -> EDTR_SWITCHKEYBLAYER
    + KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL
  07.09.2000 SVS
    + Функция GetFileOwner тоже доступна плагинам :-)
    + Функция GetNumberOfLinks тоже доступна плагинам :-)
    + Оболочка FarBsearch для плагинов (функция bsearch)
  05.09.2000 SVS 1.19
    + QWERTY - функция перекодировки QWERTY<->ЙЦУКЕН
  31.08.2000 tran 1.18
    + InputRecordToKey
  29.08.2000 SVS
    - Неверно отрабатывала функция FarSscanf
  28.08.2000 SVS
    ! уточнение для FarQsort
    ! Не FarAtoa64, но FarAtoi64
    + FarItoa64
  25.08.2000 SVS
    ! Функция GetString может при соответсвующем флаге (FIB_BUTTONS) отображать
      сепаратор и кнопки <Ok> & <Cancel>
  24.08.2000 SVS
    + В функции KeyToText добавлена обработка клавиш
      KEY_CTRLALTSHIFTPRESS, KEY_CTRLALTSHIFTRELEASE
  09.08.2000 SVS
    + FIB_NOUSELASTHISTORY - флаг для использовании пред значения из
      истории задается отдельно!!! (int function GetString())
  01.08.2000 SVS
    ! Функция ввода строки GetString имеет один параметр для всех флагов
    ! дополнительный параметра у KeyToText - размер данных
  31.07.2000 SVS
    ! Функция GetString имеет еще один параметр - расширять ли переменные среды!
    ! Если в GetString указан History, то добавляется еще и DIF_USELASTHISTORY
  25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
    ! Функции, попадающие в разряд FSF должны иметь WINAPI!!!
  23.07.2000 SVS
    ! Функция GetString имеет вызов WINAPI
  18.07.2000 tran 1.08
    ! изменил типа аргумента у ScrollBar
  13.07.2000 IG
    - в VC, похоже, нельзя сказать так: 0x4550 == 'PE', надо
      делать проверку побайтово (функция IsCommandExeGUI)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 tran
    - trap under win2000, or console height > 210
      bug was in ScrollBar ! :)))
  07.07.2000 SVS
    + Дополнительная функция обработки строк: RemoveExternalSpaces
    ! Изменен тип 2-х функций:
        RemoveLeadingSpaces
        RemoveTrailingSpaces
      Возвращают char*
  05.07.2000 SVS
    + Добавлена функция ExpandEnvironmentStr
  28.06.2000 IS
    ! Функция Unquote стала универсальной
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
  if (RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)Action,(LPDWORD)&ValueSize)!=ERROR_SUCCESS)
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

  /*$ 18.09.2000 skv
    + to allow execution of c.bat in current directory,
      if gui program c.exe exists somewhere in PATH,
      in FAR's console and not in separate window.
      for(;;) is just to prevent multiple nested ifs.
  */
  for(;;)
  {
    sprintf(FullName,"%s.bat",FileName);
    if(GetFileAttributes(FullName)!=-1)break;
    sprintf(FullName,"%s.cmd",FileName);
    if(GetFileAttributes(FullName)!=-1)break;
  /* skv$*/

    if (SearchPath(NULL,FileName,".exe",sizeof(FullName),FullName,&FilePart))
    {
      SHFILEINFO sfi;
      DWORD ExeType=SHGetFileInfo(FullName,0,&sfi,sizeof(sfi),SHGFI_EXETYPE);
      GUIType=HIWORD(ExeType)>=0x0300 && HIWORD(ExeType)<=0x1000 &&
              /* $ 13.07.2000 IG
                 в VC, похоже, нельзя сказать так: 0x4550 == 'PE', надо
                 делать проверку побайтово.
              */
              HIBYTE(ExeType)=='E' && (LOBYTE(ExeType)=='N' || LOBYTE(ExeType)=='P');
              /* IG $ */
    }
/*$ 18.09.2000 skv
    little trick.
*/
    break;
  }
  /* skv$*/
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


char* WINAPI PointToName(char *Path)
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


void WINAPI GetPathRoot(char *Path,char *Root)
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

/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (Src) и маски (Dest) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")
int ConvertWildcards(char *Src,char *Dest, int SelectedFolderNameLength)
{
  char WildName[2*NM],*CurWildPtr,*DestNamePtr,*SrcNamePtr;
  char PartBeforeName[NM],PartAfterFolderName[NM];
  DestNamePtr=PointToName(Dest);
  strcpy(WildName,DestNamePtr);
  if (strchr(WildName,'*')==NULL && strchr(WildName,'?')==NULL)
    return(FALSE);

  if (SelectedFolderNameLength!=0)
  {
    strcpy(PartAfterFolderName,Src+SelectedFolderNameLength);
    Src[SelectedFolderNameLength]=0;
  }

  SrcNamePtr=PointToName(Src);

  int BeforeNameLength=DestNamePtr==Dest ? SrcNamePtr-Src:0;
  strncpy(PartBeforeName,Src,BeforeNameLength);
  PartBeforeName[BeforeNameLength]=0;

  char *SrcNameDot=strrchr(SrcNamePtr,'.');
  CurWildPtr=WildName;
  while (*CurWildPtr)
    switch(*CurWildPtr)
    {
      case '?':
        CurWildPtr++;
        if (*SrcNamePtr)
          *(DestNamePtr++)=*(SrcNamePtr++);
        break;
      case '*':
        CurWildPtr++;
        while (*SrcNamePtr)
        {
          if (*CurWildPtr=='.' && SrcNameDot!=NULL && strchr(CurWildPtr+1,'.')==NULL)
          {
            if (SrcNamePtr==SrcNameDot)
              break;
          }
          else
            if (*SrcNamePtr==*CurWildPtr)
              break;
          *(DestNamePtr++)=*(SrcNamePtr++);
        }
        break;
      case '.':
        CurWildPtr++;
        *(DestNamePtr++)='.';
        if (strpbrk(CurWildPtr,"*?")!=NULL)
          while (*SrcNamePtr)
            if (*(SrcNamePtr++)=='.')
              break;
        break;
      default:
        *(DestNamePtr++)=*(CurWildPtr++);
        if (*SrcNamePtr && *SrcNamePtr!='.')
          SrcNamePtr++;
        break;
    }

  *DestNamePtr=0;
  if (DestNamePtr!=Dest && *(DestNamePtr-1)=='.')
    *(DestNamePtr-1)=0;
  if (*PartBeforeName)
  {
    strcat(PartBeforeName,Dest);
    strcpy(Dest,PartBeforeName);
  }
  if (SelectedFolderNameLength!=0)
    strcat(Src,PartAfterFolderName);
  return(TRUE);
}
/* IS $ */

/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(char *param1, char *param2, DWORD flags)
{
 int skippath=flags&PN_SKIPPATH;

 if(flags&PN_CMPNAME)
    return CmpName(param1, param2, skippath);

 if(flags&PN_CMPNAMELIST)
 {
  int Found=FALSE;
  static char FileMask[NM],*MaskPtr;
  MaskPtr=param1;
  while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
  if (CmpName(FileMask,param2,skippath))
  {
    Found=TRUE;
    break;
  }
  return Found;
 }

 if(flags&PN_GENERATENAME)
    return ConvertWildcards(param1, param2, flags & 0xFF);

 return FALSE;
}
/* IS $ */

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
    /* $ 13.07.2000 SVS
       ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
    delete[] TmpStr;
    /* SVS $ */
  }
  return(Str);
}


char* WINAPI QuoteSpaceOnly(char *Str)
{
  if (strchr(Str,' ')!=NULL)
  {
    char *TmpStr=new char[strlen(Str)+3];
    sprintf(TmpStr,"\"%s\"",Str);
    strcpy(Str,TmpStr);
    /* $ 13.07.2000 SVS
       ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
    delete[] TmpStr;
    /* SVS $ */
  }
  return(Str);
}


char* WINAPI TruncStr(char *Str,int MaxLength)
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
    /* $ 13.07.2000 SVS
       ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
    delete[] TmpStr;
    /* SVS $ */
    }
    else
      Str[MaxLength]=0;
  return(Str);
}


char* WINAPI TruncPathStr(char *Str,int MaxLength)
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

/* $ 07.07.2000 SVS
    + Дополнительная функция обработки строк: RemoveExternalSpaces
    ! Функции Remove*Spaces возвращают char*
*/
// удалить ведущие пробелы
char* WINAPI RemoveLeadingSpaces(char *Str)
{
  char *ChPtr;
  for (ChPtr=Str;isspace(*ChPtr);ChPtr++)
         ;
  if (ChPtr!=Str)
    memmove(Str,ChPtr,strlen(ChPtr)+1);
  return Str;
}


// удалить конечные пробелы
char* WINAPI RemoveTrailingSpaces(char *Str)
{
  for (int I=strlen((char *)Str)-1;I>=0;I--)
    if (isspace(Str[I]) || iseol(Str[I]))
      Str[I]=0;
    else
      break;
  return Str;
}

// удалить пробелы снаружи
char* WINAPI RemoveExternalSpaces(char *Str)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}
/* SVS $ */


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


int WINAPI CopyToClipboard(char *Data)
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


char* WINAPI PasteFromClipboard(void)
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


char* WINAPI PasteFromClipboardEx(int max)
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
    if ( BufferSize>max )
        BufferSize=max+1;
    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
    {
      memset(ClipText,0,BufferSize);
      if (Unicode)
        WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)ClipAddr,-1,ClipText,BufferSize-1,NULL,NULL);
      else
      {
        if (ReadType==CF_TEXT)
        {
          char *tmp=new char[BufferSize];
          strncpy(tmp,ClipAddr,BufferSize-1);
          //tmp[BufferSize]=0;
          CharToOem(ClipAddr,tmp);
          strcpy(ClipText,tmp);
        }
        else
        {
            strncpy(ClipText,ClipAddr,BufferSize-1);
            //ClipText[BufferSize]=0;
        }
      }
    }
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


void WINAPI AddEndSlash(char *Path)
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

/* $ 25.08.2000 SVS
   ! Функция GetString может при соответсвующем флаге (FIB_BUTTONS) отображать
     сепаратор и кнопки <Ok> & <Cancel>
*/
/* $ 01.08.2000 SVS
  ! Функция ввода строки GetString имеет один параметр для всех флагов
*/
/* $ 31.07.2000 SVS
   ! Функция GetString имеет еще один параметр - расширять ли переменные среды!
*/
int WINAPI GetString(char *Title,char *Prompt,char *HistoryName,char *SrcText,
    char *DestText,int DestLength,char *HelpTopic,DWORD Flags)
{
  int Substract=3; // дополнительная величина :-)
  int ExitCode;
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
║0                                                                             ║
║1   ╔═══════════════════════════════ Title ═══════════════════════════════╗   ║
║2   ║ Prompt                                                              ║   ║
║3   ║ ███████████████████████████████████████████████████████████████████║   ║
║4   ╟─────────────────────────────────────────────────────────────────────╢   ║
║5   ║                         [ Ok ]   [ Cancel ]                         ║   ║
║6   ╚═════════════════════════════════════════════════════════════════════╝   ║
║7                                                                             ║
*/
  static struct DialogData StrDlgData[]=
  {
/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
                                      Selected               Data
*/
/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 4, 0, 0, 0,                0,"",
/* 1 */ DI_TEXT,      5, 2,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"",
/* 2 */ DI_EDIT,      5, 3, 70, 3, 1, 0, 0,                1,"",
/* 3 */ DI_TEXT,      0, 4,  0, 4, 0, 0, DIF_SEPARATOR,    0,"",
/* 4 */ DI_BUTTON,    0, 5,  0, 0, 0, 0, DIF_CENTERGROUP,  0,"",
/* 5 */ DI_BUTTON,    0, 5,  0, 0, 0, 0, DIF_CENTERGROUP,  0,""
  };
  MakeDialogItems(StrDlgData,StrDlg);

  if(Flags&FIB_BUTTONS)
  {
    Substract=0;
    StrDlg[0].Y2+=2;
    StrDlg[2].DefaultButton=0;
    StrDlg[4].DefaultButton=1;
    strcpy(StrDlg[4].Data,FarMSG(MOk));
    strcpy(StrDlg[5].Data,FarMSG(MCancel));
  }

  if(Flags&FIB_EXPANDENV)
  {
    StrDlg[2].Flags|=DIF_EDITEXPAND;
  }

  if (HistoryName!=NULL)
  {
    StrDlg[2].Selected=(int)HistoryName;
    /* $ 09.08.2000 SVS
       флаг для использовании пред значения из истории задается отдельно!!!
    */
    StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
    /* SVS $ */
  }

  if (Flags&FIB_PASSWORD)
    StrDlg[2].Type=DI_PSWEDIT;

  if(Title)
    strcpy(StrDlg[0].Data,Title);
  if(Prompt)
    strcpy(StrDlg[1].Data,Prompt);
  if(SrcText)
    strncpy(StrDlg[2].Data,SrcText,sizeof(StrDlg[2].Data));
  StrDlg[2].Data[sizeof(StrDlg[2].Data)-1]=0;

  Dialog Dlg(StrDlg,sizeof(StrDlg)/sizeof(StrDlg[0])-Substract);
  Dlg.SetPosition(-1,-1,76,(Flags&FIB_BUTTONS)?8:6);

  if (HelpTopic!=NULL)
    Dlg.SetHelp(HelpTopic);

  Dlg.Process();
  ExitCode=Dlg.GetExitCode();

  if (DestLength >= 1 && (ExitCode == 2 || ExitCode == 4))
  {
    if(!(Flags&FIB_ENABLEEMPTY) && *StrDlg[2].Data==0)
      return(FALSE);
    strncpy(DestText,StrDlg[2].Data,DestLength-1);
    DestText[DestLength-1]=0;
    return(TRUE);
  }
  return(FALSE);
}
/* SVS $*/
/* 01.08.2000 SVS $*/
/* 25.08.2000 SVS $*/


void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total)
{
  /* $ 06.07.2000 tran
     - trap under NT with console height > 210
       was char OutStr[200] :) */
  char OutStr[4096];
  /* tran 06.07.2000 $ */

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


/* $ 07.09.2000 SVS
   Функция GetFileOwner тоже доступна плагинам :-)
*/
int WINAPI GetFileOwner(char *Computer,char *Name,char *Owner)
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

  if (!GetCode || Needed!=0)
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
/* SVS $*/


/* $ 07.09.2000 SVS
   Функция GetNumberOfLinks тоже доступна плагинам :-)
*/
int WINAPI GetNumberOfLinks(char *Name)
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
/* SVS $*/


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

/* $ 19.09.2000 SVS
   немного "ускорим" за счет сокращения вызова функций `strcmp'
*/
int IsFolderNotEmpty(char *Name)
{
  register DWORD P;
  WIN32_FIND_DATA fdata;
  char FileName[NM];
  ScanTree ScTree(FALSE,FALSE);
  ScTree.SetFindPath(Name,"*.*");
  while (ScTree.GetNextName(&fdata,FileName))
  {
    // немного ускорим.
    P=(*(DWORD*)FileName)&0x00FFFFFF;
    if((P&0xFFFF) != 0x002E && P != 0x002E2E )
//    if (strcmp(FileName,".")!=0 && strcmp(FileName,"..")!=0)
      return(TRUE);
  }
  return(FALSE);
}
/* SVS $ */

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


#if defined(__BORLANDC__)
#pragma option -a4
#endif
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
#if defined(__BORLANDC__)
#pragma option -a.
#endif


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
/* $ 25.07.2000 SVS
   Вызов WINAPI
*/
void WINAPI Unquote(char *Str)
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

/* $ 05.07.2000 SVS
   Расширение переменной среды
   Вынесена в качестве самостоятельной вместо прямого вызова
     ExpandEnvironmentStrings.
*/
/* $ 25.07.2000 SVS
   Вызов WINAPI
*/
DWORD WINAPI ExpandEnvironmentStr(char *src, char *dest, size_t size)
{
 DWORD ret=0;
 char *tmp=(char *)malloc(size);
 if(tmp)
  {
   ret=ExpandEnvironmentStrings(src,tmp,size);
   strcpy(dest,tmp);
   free(tmp);
  }
 return ret;
}
/* SVS $ */


/* $ 25.07.2000 SVS
   Оболочки вокруг вызовов стандартных функцйи, приведенных к WINAPI
*/
char *WINAPI FarItoa(int value, char *string, int radix)
{
  return itoa(value,string,radix);
}
/* $ 28.08.2000 SVS
  + FarItoa64
*/
char *WINAPI FarItoa64(__int64 value, char *string, int radix)
{
  return _i64toa(value, string, radix);
}
/* SVS $ */
int WINAPI FarAtoi(const char *s)
{
  return atoi(s);
}
__int64 WINAPI FarAtoi64(const char *s)
{
  return _atoi64(s);
}
void WINAPI FarQsort(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *))
{
  qsort(base,nelem,width,fcmp);
}
int WINAPIV FarSprintf(char *buffer,const char *format,...)
{
  va_list argptr;
  va_start(argptr,format);
  int ret=vsprintf(buffer,format,argptr);
  va_end(argptr);
  return ret;
}

/* $ 29.08.2000 SVS
   - Неверно отрабатывала функция FarSscanf
   Причина - т.к. у VC нету vsscanf, то пришлось смоделировать (взять из
   исходников VC sscanf и "нарисовать" ее сюда
*/
#if defined(_MSC_VER)
extern "C" {
int __cdecl _input (FILE *stream,const unsigned char *format,va_list arglist);
};
#endif

int WINAPIV FarSscanf(const char *buffer, const char *format,...)
{
#if defined(_MSC_VER)
  // полная копия внутренностей sscanf :-)
  va_list arglist;
  FILE str;
  FILE *infile = &str;
  int retval;

  va_start(arglist, format);

  infile->_flag = _IOREAD|_IOSTRG|_IOMYBUF;
  infile->_ptr = infile->_base = (char *) buffer;
  infile->_cnt = strlen(buffer);

  retval = (_input(infile,(const unsigned char *)format,arglist));

  return(retval);
#else
  va_list argptr;
  va_start(argptr,format);
  int ret=vsscanf(buffer,format,argptr);
  va_end(argptr);
  return ret;
#endif
}
/* 29.08.2000 SVS $ */
/* SVS $ */

/* $ 25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
*/
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
   Size=0 - по максимуму!
*/
/* $ 10.09.2000 SVS
  ! KeyToText возвращает BOOL
*/
BOOL WINAPI KeyToText(int Key0,char *KeyText0,int Size)
{
  int I;
  char KeyText[32];
  int fmtNum, Key=Key0;
  char *fmtKey[]={
  /* 00 */ "F%d",
  /* 01 */ "CtrlF%d",
  /* 02 */ "AltF%d",
  /* 03 */ "ShiftF%d",
  /* 04 */ "Ctrl%c",
  /* 05 */ "RCtrl%c",
  /* 06 */ "CtrlShiftF%d",
  /* 07 */ "AltShiftF%d",
  /* 08 */ "CtrlAltF%d",
  /* 09 */ "CtrlShift%c",
  /* 10 */ "AltShift%c",
  /* 11 */ "CtrlAlt%c",
  /* 12 */ "Alt%c",
  /* 13 */ "%s",
  /* 14 */ "%c",
  };

  if (Key>=KEY_F1 && Key<=KEY_F12)
  {
    fmtNum=0;
    Key=Key0-KEY_F1+1;
  }
  else if (Key>=KEY_CTRLF1 && Key<=KEY_CTRLF12)
  {
    fmtNum=1;
    Key=Key0-KEY_CTRLF1+1;
  }
  else if (Key>=KEY_ALTF1 && Key<=KEY_ALTF12)
  {
    fmtNum=2;
    Key=Key0-KEY_ALTF1+1;
  }
  else if (Key>=KEY_SHIFTF1 && Key<=KEY_SHIFTF12)
  {
    fmtNum=3;
    Key=Key0-KEY_SHIFTF1+1;
  }
  else if (Key>=KEY_CTRLA && Key<=KEY_CTRLZ)
  {
    fmtNum=4;
    Key=Key0-KEY_CTRLA+'A';
  }
  else if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    fmtNum=4;
    Key=Key0-KEY_CTRL0+'0';
  }
  else if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    fmtNum=5;
    Key=Key0-KEY_RCTRL0+'0';
  }
  else if (Key>=KEY_CTRLSHIFTF1 && Key<=KEY_CTRLSHIFTF12)
  {
    fmtNum=6;
    Key=Key0-KEY_CTRLSHIFTF1+1;
  }
  else if (Key>=KEY_ALTSHIFTF1 && Key<=KEY_ALTSHIFTF12)
  {
    fmtNum=7;
    Key=Key0-KEY_ALTSHIFTF1+1;
  }
  else if (Key>=KEY_CTRLALTF1 && Key<=KEY_CTRLALTF12)
  {
    fmtNum=8;
    Key=Key0-KEY_CTRLALTF1+1;
  }
  else if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
  {
    fmtNum=9;
    Key=Key0-KEY_CTRLSHIFT0+'0';
  }
  else if (Key>=KEY_CTRLSHIFTA && Key<=KEY_CTRLSHIFTZ)
  {
    fmtNum=9;
    Key=Key0-KEY_CTRLSHIFTA+'A';
  }
  else if (Key>=KEY_ALTSHIFTA && Key<=KEY_ALTSHIFTZ)
  {
    fmtNum=10;
    Key=Key0-KEY_ALTSHIFTA+'A';
  }
  else if (Key>=KEY_CTRLALTA && Key<=KEY_CTRLALTZ)
  {
    fmtNum=11;
    Key=Key0-KEY_CTRLALTA+'A';
  }
  else if (Key>=KEY_ALT0 && Key<=KEY_ALT9)
  {
    fmtNum=12;
    Key=Key0-KEY_ALT0+'0';
  }
  else if (Key>=KEY_ALTA && Key<=KEY_ALTZ)
  {
    fmtNum=12;
    Key=Key0-KEY_ALTA+'A';
  }
  else
  {
    /* $ 23.07.2000 SVS
       + KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
    */
    /* $ 08.09.2000 SVS
       + KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL
    */
    static int KeyCodes[]={
      KEY_BS,KEY_TAB,KEY_ENTER,KEY_ESC,KEY_SPACE,KEY_HOME,KEY_END,KEY_UP,
      KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_PGUP,KEY_PGDN,KEY_INS,KEY_DEL,KEY_NUMPAD5,
      KEY_CTRLBRACKET,KEY_CTRLBACKBRACKET,KEY_CTRLCOMMA,KEY_CTRLDOT,KEY_CTRLBS,
      KEY_CTRLQUOTE,KEY_CTRLSLASH,
      KEY_CTRLENTER,KEY_CTRLTAB,KEY_CTRLSHIFTINS,KEY_CTRLSHIFTDOWN,
      KEY_CTRLSHIFTLEFT,KEY_CTRLSHIFTRIGHT,KEY_CTRLSHIFTUP,KEY_CTRLSHIFTEND,
      KEY_CTRLSHIFTHOME,KEY_CTRLSHIFTPGDN,KEY_CTRLSHIFTPGUP,
      KEY_CTRLSHIFTSLASH,KEY_CTRLSHIFTBACKSLASH,
      KEY_CTRLSHIFTSUBTRACT,KEY_CTRLSHIFTADD,KEY_CTRLSHIFTENTER,KEY_ALTADD,
      KEY_ALTSUBTRACT,KEY_ALTMULTIPLY,KEY_ALTDOT,KEY_ALTCOMMA,KEY_ALTINS,
      KEY_ALTDEL,KEY_ALTBS,KEY_ALTHOME,KEY_ALTEND,KEY_ALTPGUP,KEY_ALTPGDN,
      KEY_ALTUP,KEY_ALTDOWN,KEY_ALTLEFT,KEY_ALTRIGHT,
      KEY_CTRLDOWN,KEY_CTRLLEFT,KEY_CTRLRIGHT,KEY_CTRLUP,
      KEY_CTRLEND,KEY_CTRLHOME,KEY_CTRLPGDN,KEY_CTRLPGUP,KEY_CTRLBACKSLASH,
      KEY_CTRLSUBTRACT,KEY_CTRLADD,KEY_CTRLMULTIPLY,KEY_CTRLCLEAR,KEY_ADD,
      KEY_SUBTRACT,KEY_MULTIPLY,KEY_BREAK,KEY_SHIFTINS,KEY_SHIFTDEL,
      KEY_SHIFTEND,KEY_SHIFTHOME,KEY_SHIFTLEFT,KEY_SHIFTUP,KEY_SHIFTRIGHT,
      KEY_SHIFTDOWN,KEY_SHIFTPGUP,KEY_SHIFTPGDN,KEY_SHIFTENTER,KEY_SHIFTTAB,
      KEY_SHIFTADD,KEY_SHIFTSUBTRACT,KEY_CTRLINS,KEY_CTRLDEL,KEY_CTRLSHIFTDOT,
      KEY_CTRLSHIFTTAB,KEY_DIVIDE,KEY_CTRLSHIFTBS,KEY_ALT,KEY_CTRL,KEY_SHIFT,
      KEY_RALT,KEY_RCTRL,KEY_CTRLSHIFTBRACKET,KEY_CTRLSHIFTBACKBRACKET,
      KEY_ALTSHIFTINS,KEY_ALTSHIFTDOWN,KEY_ALTSHIFTLEFT,KEY_ALTSHIFTRIGHT,
      KEY_ALTSHIFTUP,KEY_ALTSHIFTEND,KEY_ALTSHIFTHOME,KEY_ALTSHIFTPGDN,
      KEY_ALTSHIFTPGUP,KEY_ALTSHIFTENTER,
      KEY_CTRLALTINS,KEY_CTRLALTDOWN,KEY_CTRLALTLEFT,KEY_CTRLALTRIGHT,
      KEY_CTRLALTUP,KEY_CTRLALTEND,KEY_CTRLALTHOME,KEY_CTRLALTPGDN,
      KEY_CTRLALTPGUP,KEY_CTRLALTENTER,KEY_SHIFTBS,KEY_APPS,
      KEY_CTRLAPPS,KEY_ALTAPPS,KEY_SHIFTAPPS,
      KEY_CTRLSHIFTAPPS,KEY_ALTSHIFTAPPS,KEY_CTRLALTAPPS,
      KEY_LWIN,KEY_RWIN,
      KEY_CTRLALTSHIFTPRESS,KEY_CTRLALTSHIFTRELEASE,
      KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL
    };
    static char *KeyNames[]={
      "BS","Tab","Enter","Esc","Space","Home","End","Up",
      "Down","Left","Right","PgUp","PgDn","Ins","Del","Clear",
      "Ctrl[","Ctrl]","Ctrl,","Ctrl.","CtrlBS",
      "Ctrl\"","Ctrl/",
      "CtrlEnter","CtrlTab","CtrlShiftIns","CtrlShiftDown",
      "CtrlShiftLeft","CtrlShiftRight","CtrlShiftUp","CtrlShiftEnd",
      "CtrlShiftHome","CtrlShiftPgDn","CtrlShiftPgUp",
      "CtrlShiftSlash","CtrlShiftBackSlash",
      "CtrlShiftSubtract","CtrlShiftAdd","CtrlShiftEnter","AltAdd",
      "AltSubtract","AltMultiply","Alt.","Alt,","AltIns",
      "AltDel","AltBS","AltHome","AltEnd","AltPgUp","AltPgDn",
      "AltUp","AltDown","AltLeft","AltRight",
      "CtrlDown","CtrlLeft","CtrlRight","CtrlUp",
      "CtrlEnd","CtrlHome","CtrlPgDn","CtrlPgUp","CtrlBackSlash",
      "CtrlSubtract","CtrlAdd","CtrlMultiply","CtrlClear","Add",
      "Subtract","Multiply","Break","ShiftIns","ShiftDel",
      "ShiftEnd","ShiftHome","ShiftLeft","ShiftUp","ShiftRight",
      "ShiftDown","ShiftPgUp","ShiftPgDn","ShiftEnter","ShiftTab",
      "ShiftAdd","ShiftSubtract","CtrlIns","CtrlDel","CtrlShiftDot",
      "CtrlShiftTab","Divide","CtrlShiftBS","Alt","Ctrl","Shift",
      "RAlt","RCtrl","CtrlShift[","CtrlShift]",
      "AltShiftIns","AltShiftDown","AltShiftLeft","AltShiftRight",
      "AltShiftUp","AltShiftEnd","AltShiftHome","AltShiftPgDn",
      "AltShiftPgUp","AltShiftEnter",
      "CtrlAltIns","CtrlAltDown","CtrlAltLeft","CtrlAltRight",
      "CtrlAltUp","CtrlAltEnd","CtrlAltHome","CtrlAltPgDn","CtrlAltPgUp",
      "CtrlAltEnter","ShiftBS",
      "Apps","CtrlApps","AltApps","ShiftApps",
      "CtrlShiftApps","AltShiftApps","CtrlAltApps",
      "LWin","RWin",
      "CtrlAltShiftPress","CtrlAltShiftRelease",
      "CtrlShiftDel", "AltShiftDel"
    };
    /* SVS 08.09.2000 $ */
    /* SVS $ */

    for (I=0;I<sizeof(KeyCodes)/sizeof(KeyCodes[0]);I++)
      if (Key==KeyCodes[I])
      {
        strcpy(KeyText,KeyNames[I]);
        break;
      }
    if(I  == sizeof(KeyCodes)/sizeof(KeyCodes[0]))
    {
      if (Key<256)
      {
        fmtNum=14;
        Key=Key0;
      }
      else if (Key>KEY_CTRL_BASE && Key<KEY_END_CTRL_BASE)
      {
        fmtNum=4;
        Key=Key0-KEY_CTRL_BASE;
      }
      else if (Key>KEY_ALT_BASE && Key<KEY_END_ALT_BASE)
      {
        fmtNum=12;
        Key=Key0-KEY_ALT_BASE;
      }
      else if (Key>KEY_CTRLSHIFT_BASE && Key<KEY_END_CTRLSHIFT_BASE)
      {
        fmtNum=9;
        Key=Key0-KEY_CTRLSHIFT_BASE;
      }
      else if (Key>KEY_ALTSHIFT_BASE && Key<KEY_END_ALTSHIFT_BASE)
      {
        fmtNum=10;
        Key=Key0-KEY_ALTSHIFT_BASE;
      }
      else if (Key>KEY_CTRLALT_BASE && Key<KEY_END_CTRLALT_BASE)
      {
        fmtNum=11;
        Key=Key0-KEY_CTRLALT_BASE;
      }
      else
      {
        Key=-1;
        *KeyText=0;
      }
    }
    else
      Key=-1;
  }
  if(Key != -1)
  {
    sprintf(KeyText,fmtKey[fmtNum],Key);
    for (I=0;KeyText[I]!=0;I++)
      if (KeyText[I]=='\\')
      {
        strcpy(KeyText+I,"BackSlash");
        break;
      }
  }

  if(!KeyText[0])
  {
    *KeyText0='\0';
    return FALSE;
  }
  if(Size > 0)
    strncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);
  return TRUE;
}
/* SVS 10.09.2000 $ */
/* SVS $ */

/* tran 31.08.2000 $
  FarInputRecordToKey */
int WINAPI InputRecordToKey(INPUT_RECORD *r)
{
    return CalcKeyCode(r,TRUE);
}
/* tran 31.08.2000 $ */


/* $ 05.09.2000 SVS
  XLat-перекодировка!
  На основе плагина EditSwap by SVS :-)))
*/
char* WINAPI Xlat(
   char *Line,                    // исходная строка
   int StartPos,                  // начало переконвертирования
   int EndPos,                    // конец переконвертирования
   struct CharTableSet *TableSet, // перекодировочная таблица (может быть NULL)
   DWORD Flags)                   // флаги (см. enum XLATMODE)
{
  BYTE Chr,ChrOld;
  int J,I;
  int PreLang=2,CurLang=2; // uncnown
  int LangCount[2]={0,0};
  int IsChange=0;

  /* $ 08.09.2000 SVS
     Ошибочка вкралась :-)))
  */
  if(!Line || *Line == 0)
    return NULL;
  /* SVS $ */

  I=strlen(Line);

  if(EndPos > I)
    EndPos=I;

  if(StartPos < 0)
    StartPos=0;

  if(StartPos > EndPos || StartPos >= I)
    return Line;


//  OemToCharBuff(Opt.QWERTY.Table[0],Opt.QWERTY.Table[0],80);???
  if(!Opt.XLat.Table[0][0] || !Opt.XLat.Table[1][0])
    return Line;


  I=strlen((char *)Opt.XLat.Table[1]),
  J=strlen((char *)Opt.XLat.Table[0]);
  int MinLenTable=(I > J?J:I);

  if (TableSet)
    // из текущей кодировки в OEM
    DecodeString(Line+StartPos,(LPBYTE)TableSet->DecodeTable,EndPos-StartPos+1);

  // цикл по всей строке
  for(J=StartPos; J < EndPos; J++)
  {
    ChrOld=Chr=(BYTE)Line[J];
    // ChrOld - пред символ
    IsChange=0;
    // цикл по просмотру Chr в таблицах
    for(I=0; I < MinLenTable; ++I)
    {
      // символ из латиницы?
      if(Chr == (BYTE)Opt.XLat.Table[1][I])
      {
        Chr=(char)Opt.XLat.Table[0][I];
        IsChange=1;
        CurLang=1; // pred - english
        LangCount[1]++;
        break;
      }
      // символ из русской?
      else if(Chr == (BYTE)Opt.XLat.Table[0][I])
      {
        Chr=(char)Opt.XLat.Table[1][I];
        CurLang=0; // pred - russian
        LangCount[0]++;
        IsChange=1;
        break;
      }
    }

    if(!IsChange) // особые случаи...
    {
      PreLang=CurLang;
      if(LangCount[0] > LangCount[1])
        CurLang=0;
      else
        CurLang=1;
      if(PreLang != CurLang)
        CurLang=PreLang;

        for(I=0; I < 20; I+=2)
          if(ChrOld == (BYTE)Opt.XLat.Rules[CurLang][I])
          {
             Chr=(BYTE)Opt.XLat.Rules[CurLang][I+1];
             break;
          }
    }

    Line[J]=(char)Chr;
  }

  if (TableSet)
    // из OEM в текущую кодировку
    EncodeString(Line+StartPos,(LPBYTE)TableSet->EncodeTable,EndPos-StartPos+1);

  // переключаем раскладку клавиатуры?
  //  к сожалению не работает под Win9x - ставьте WinNT и наслаждайтесь :-)
  /* $ 20.09.2000 SVS
     Немного изменим условия и возьмем окно именно FAR.
  */
  if(hFarWnd && (Flags & XLAT_SWITCHKEYBLAYOUT))
    PostMessage(hFarWnd,WM_INPUTLANGCHANGEREQUEST, 1, HKL_NEXT);
  /* SVS $ */

  return Line;
}
/* SVS $ */

/* $ 07.09.2000 SVS
   Оболочка FarBsearch для плагинов (функция bsearch)
*/
void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
  return bsearch(key,base,nelem,width,fcmp);
}
/* SVS $ */


/* $ 10.09.2000 tran
   FSF/FarRecurseSearch */
void WINAPI FarRecursiveSearch(char *initdir,char *mask,FRSUSERFUNC func,DWORD flags)
{
    ScanTree ScTree(flags& FRS_RETUPDIR,flags & FRS_RECUR);
    WIN32_FIND_DATA FindData;
    char FullName[NM];

    ScTree.SetFindPath(initdir,mask);
    while (ScTree.GetNextName(&FindData,FullName))
    {
        //if (func(FindData,FullName)==0)
        if (func(&FindData,FullName)==0)
            break;
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
char* WINAPI FarMkTemp(char *Dest, char *Template)
{
  if(Dest && Template && *Template)
  {
    char TempPath[NM];
    int Len;
    TempPath[Len=GetTempPath(sizeof(TempPath),TempPath)]=0;
    strcat(TempPath,Template);
    if(mktemp(TempPath+Len) != NULL)
    {
      strcpy(Dest,TempPath);
      return Dest;
    }
  }
  return NULL;
}
/* SVS 18.09.2000 $ */
/* SVS $ */

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!
*/
int WINAPI KeyNameToKey(char *Name)
{
   char KeyName[33];

   for (int I=0; I < KEY_LAST_BASE;++I)
   {
     if(KeyToText(I,KeyName))
       if(!strcmp(Name,KeyName))
         return I;
   }
   return -1;
}
/* SVS $*/

/*$ 27.09.2000 skv
  + Удаление буфера выделенного через new char[n];
    Сделано для удаления возвращенного PasteFromClipboard
*/
void WINAPI DeleteBuffer(char *Buffer)
{
  if(Buffer)delete [] Buffer;
}
/* skv$*/

#if defined(SYSLOG)
char         LogFileName[MAX_FILE];
static FILE *LogStream=0;
static int   Indent=0;
#endif

FILE * OpenLogStream(char *file)
{
#if defined(SYSLOG)
    time_t t;
    struct tm *time_now;
    char RealLogName[MAX_FILE];
//    char rfile[MAX_FILE];

    time(&t);
    time_now=localtime(&t);

    strftime(RealLogName,MAX_FILE,file,time_now);
    return _fsopen(RealLogName,"a+t",SH_DENYWR);
#endif
}

void OpenSysLog()
{
#if defined(SYSLOG)
    if ( LogStream )
        fclose(LogStream);

    GetModuleFileName(NULL,LogFileName,sizeof(LogFileName));
    strcpy(strrchr(LogFileName,'\\')+1,"far.log");
    LogStream=OpenLogStream(LogFileName);
    //if ( !LogStream )
    //{
    //    fprintf(stderr,"Can't open log file '%s'\n",LogFileName);
    //}
#endif
}

void CloseSysLog(void)
{
#if defined(SYSLOG)
    fclose(LogStream);
    LogStream=0;
#endif
}

void SysLog(int i)
{
#if defined(SYSLOG)
    Indent+=i;
    if ( Indent<0 )
        Indent=0;
#endif
}

void SysLog(char *fmt,...)
{
#if defined(SYSLOG)
    char spaces[]="                                                                                                                                                       ";
    char msg[MAX_LOG_LINE];
    time_t t;
    struct tm *tm;
    char timebuf[64];

    va_list argptr;
    va_start( argptr, fmt );

    vsprintf( msg, fmt, argptr );
    va_end(argptr);

    time (&t);
    tm = localtime (&t);
    strftime (timebuf, sizeof (timebuf), "%d.%m.%Y %H:%M:%S", tm);

    OpenSysLog();
    if ( LogStream )
    {
        spaces[Indent]=0;
        fprintf(LogStream,"%s %s %s\n",timebuf,spaces,msg);
        fflush(LogStream);
    }
    CloseSysLog();
#endif
}

