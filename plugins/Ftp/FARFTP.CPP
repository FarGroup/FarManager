#include "ftpincl.hpp"
#include "farftp.hpp"

static void GetFtpDirectory(Connection *Connect);
static int ParseDirLine(Connection *Connect,WIN32_FIND_DATA *FindData,
                        DWORD *Mode, LPSTR lpcszOwner);

static SYSTEMTIME LocalTime;
int BusyMode;

/*void main()
{
  HINTERNET hConnect=InternetConnect(NULL,"localhost",21,"z","z",0,0,0);
  if (!hConnect)
    return;
  ((Connection *)hConnect)->ProcessCommand("pwd");
}*/


INTERNETAPI void WINAPI FtpConnect(HANDLE *hConnect,LPCSTR lpszServerName,
            LPCSTR lpszUserName,LPCSTR lpszPassword)
{
  Connection *Connect=new Connection;
  Connect->GetCallback=NULL;
  *hConnect=(HANDLE)Connect;
  if (!Connect)
    return;
  char HostName[300];
  strcpy(HostName,lpszServerName);
  BusyMode=TRUE;
  int ConnectCode=Connect->Init(HostName,lpszUserName,lpszPassword);
  BusyMode=FALSE;
  if (!ConnectCode)
  {
    *hConnect=NULL;
    delete Connect;
    return;
  }
  ConnectMessage(3);
  GetFtpDirectory(Connect);
}


INTERNETAPI BOOL WINAPI FtpFindFirstFile(HINTERNET hConnect,
            LPCSTR lpszSearchFile,LPWIN32_FIND_DATA lpFindFileData,
            DWORD ResetCache,DWORD *Mode, LPSTR lpszOwner)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[NM+10];
  int AllFiles=strcmp(lpszSearchFile,"*")==0 || strcmp(lpszSearchFile,"*.*")==0;
  int FromCache;

  Connect->FindAllFiles=AllFiles;

  if (ResetCache)
    Connect->CacheReset();

  if (AllFiles)
    strcpy(Command,"dir");
  else
  {
    if (*lpszSearchFile=='\\' || lpszSearchFile[0] && lpszSearchFile[1]==':')
      lpszSearchFile=PointToName((char *)lpszSearchFile);
    sprintf(Command,"dir \"%s\"",lpszSearchFile);
  }
  if (!AllFiles || !(FromCache=Connect->CacheGet()))
    if (!Connect->ProcessCommand(Command))
      return(NULL);
  if (AllFiles && !FromCache)
    Connect->CacheAdd();
  GetLocalTime(&LocalTime);
  return(ParseDirLine(Connect,lpFindFileData,Mode,lpszOwner));
}


INTERNETAPI BOOL WINAPI FtpFindNextFile(HINTERNET hConnect,
            LPVOID lpvFindData,DWORD *Mode, LPSTR lpszOwner)
{
  Connection *Connect=(Connection *)hConnect;
  return(ParseDirLine(Connect,(WIN32_FIND_DATA *)lpvFindData,Mode,lpszOwner));
}


INTERNETAPI BOOL WINAPI FtpGetCurrentDirectory(HINTERNET hConnect,
            LPSTR lpszCurrentDirectory,LPDWORD lpdwCurrentDirectory)
{

  Connection *Connect=(Connection *)hConnect;
  strncpy(lpszCurrentDirectory,Connect->CurDir,*lpdwCurrentDirectory);
  return(*lpszCurrentDirectory!=0);
}


void GetFtpDirectory(Connection *Connect)
{
  *Connect->CurDir=0;
  if (!Connect->ProcessCommand("pwd"))
    return;
  char Line[512],*ChPtr;
  Connect->GetReply(Line,sizeof(Line));
  if ((ChPtr=strchr(Line,'\"'))==NULL)
    return;
  strncpy(Connect->CurDir,ChPtr+1,sizeof(Connect->CurDir));
  if ((ChPtr=strchr(Connect->CurDir,'\"'))!=NULL)
    *ChPtr=0;
}


INTERNETAPI BOOL WINAPI FtpSetCurrentDirectory(HINTERNET hConnect,
            LPCSTR lpszDirectory)
{
  if (*lpszDirectory==0)
    return(FALSE);
  Connection *Connect=(Connection *)hConnect;
  char NewDir[NM],Command[NM+10];
  strcpy(NewDir,lpszDirectory);
  for (int I=0;NewDir[I]!=0;I++)
    if (NewDir[I]=='\\')
      NewDir[I]='/';
  sprintf(Command,"cd \"%s\"",NewDir);
  int RetCode=Connect->ProcessCommand(Command);
  GetFtpDirectory(Connect);
  return(RetCode);
}


INTERNETAPI BOOL WINAPI FtpCreateDirectory(HINTERNET hConnect,
            LPCSTR lpszDirectory)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[NM+10];
  Connect->CacheReset();
  sprintf(Command,"mkdir \"%s\"",lpszDirectory);
  return(Connect->ProcessCommand(Command));
}


INTERNETAPI BOOL WINAPI FtpRemoveDirectory(HINTERNET hConnect,
            LPCSTR lpszDirectory)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[NM+10];
  Connect->CacheReset();
  sprintf(Command,"rmdir \"%s\"",lpszDirectory);
  return(Connect->ProcessCommand(Command));
}


INTERNETAPI BOOL WINAPI FtpRenameFile(HINTERNET hConnect,
            LPCSTR lpszExisting,LPCSTR lpszNew)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[2*NM+10];
  Connect->CacheReset();
  sprintf(Command,"ren \"%s\" \"%s\"",lpszExisting,lpszNew);
  return(Connect->ProcessCommand(Command));
}


INTERNETAPI BOOL WINAPI FtpDeleteFile(HINTERNET hConnect,
            LPCSTR lpszFileName)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[NM+10];
  Connect->CacheReset();
  sprintf(Command,"del \"%s\"",lpszFileName);
  return(Connect->ProcessCommand(Command));
}


INTERNETAPI BOOL WINAPI FtpChmod(HINTERNET hConnect,
            LPCSTR lpszFileName,DWORD Mode)
{
  Connection *Connect=(Connection *)hConnect;
  char Command[NM+10];
  Connect->CacheReset();
  sprintf(Command,"chmod %o \"%s\"",Mode,lpszFileName);
  return(Connect->ProcessCommand(Command));
}


INTERNETAPI BOOL WINAPI FtpDisconnect(HINTERNET hInternet)
{
  delete (Connection *)hInternet;
  return(TRUE);
}


INTERNETAPI BOOL WINAPI FtpGetFile(HINTERNET hConnect,
            LPCSTR lpszRemoteFile,LPCSTR lpszNewFile,int Reget,
            int AsciiMode,int (*GetCallback)(int Size))
{
  Connection *Connect=(Connection *)hConnect;
  char Command[2*NM+10];
  if (AsciiMode && !Connect->ProcessCommand("ascii") ||
      !AsciiMode && !Connect->ProcessCommand("bin"))
    return(FALSE);
  Connect->GetCallback=GetCallback;
  sprintf(Command,"%s \"%s\" \"%s\"",Reget ? "reget":"get",
          lpszRemoteFile,lpszNewFile);
  int ExitCode=Connect->ProcessCommand(Command);
  Connect->GetCallback=NULL;
  return(ExitCode);
}


INTERNETAPI BOOL WINAPI FtpPutFile(HINTERNET hConnect,
            LPCSTR lpszLocalFile,LPCSTR lpszNewRemoteFile,long Position,
            int AsciiMode,int (*PutCallback)(int Size))
{
  Connection *Connect=(Connection *)hConnect;
  char Command[2*NM+10];
  Connect->CacheReset();
  if (AsciiMode)
    Position=0;
  if (AsciiMode && !Connect->ProcessCommand("ascii") ||
      !AsciiMode && !Connect->ProcessCommand("bin"))
    return(FALSE);
  if (Position>0)
  {
    sprintf(Command,"size \"%s\"",lpszNewRemoteFile);
    if (Connect->ProcessCommand(Command))
    {
      char Line[512];
      Connect->GetReply(Line,sizeof(Line));
      if (isdigit(Line[0]) && isdigit(Line[1]) && isdigit(Line[2]) &&
          isspace(Line[3]) && isdigit(Line[4]))
        Position=atoi(&Line[4]);
    }

    sprintf(Command,"rest %u",Position);
    if (!Connect->ProcessCommand(Command))
      return(FALSE);
  }
  Connect->PutCallback=PutCallback;

  if (*lpszNewRemoteFile=='\\' || lpszNewRemoteFile[0] && lpszNewRemoteFile[1]==':')
    lpszNewRemoteFile=PointToName((char *)lpszNewRemoteFile);
  sprintf(Command,"%s \"%s\" \"%s\"",Position>0 ? "appe":"put",
          lpszLocalFile,lpszNewRemoteFile);
  int ExitCode=Connect->ProcessCommand(Command);
  Connect->PutCallback=NULL;
  return(ExitCode);
}


INTERNETAPI BOOL WINAPI FtpSystemInfo(HINTERNET hConnect,
            char *Buffer,int MaxSize)
{
  Connection *Connect=(Connection *)hConnect;
  if (MaxSize<1)
    return(FALSE);
  if (!Connect->SystemInfoFilled)
  {
    if (Connect->ProcessCommand("syst"))
      Connect->GetReply(Connect->SystemInfo,sizeof(Connect->SystemInfo));
    else
      *Connect->SystemInfo=0;
    Connect->SystemInfoFilled=TRUE;
  }
  if (isdigit(Connect->SystemInfo[0]) && isdigit(Connect->SystemInfo[1]) &&
      isdigit(Connect->SystemInfo[2]))
    strncpy(Buffer,Connect->SystemInfo+3,MaxSize-1);
  else
    strncpy(Buffer,Connect->SystemInfo,MaxSize-1);
  Buffer[MaxSize-1]=0;
  char *ChPtr;
  if ((ChPtr=strchr(Buffer,'\r'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strchr(Buffer,'\n'))!=NULL)
    *ChPtr=0;
  return(*Connect->SystemInfo!=0);
}


INTERNETAPI BOOL WINAPI FtpStartReply(HINTERNET hConnect,
            char *Buffer,int MaxSize)
{
  Connection *Connect=(Connection *)hConnect;
  if (MaxSize<1)
    return(FALSE);
  if (isdigit(Connect->StartReply[0]) && isdigit(Connect->StartReply[1]) &&
      isdigit(Connect->StartReply[2]))
    strncpy(Buffer,Connect->StartReply+3,MaxSize-1);
  else
    strncpy(Buffer,Connect->StartReply,MaxSize-1);
  Buffer[MaxSize-1]=0;
  char *ChPtr;
  if ((ChPtr=strchr(Buffer,'\r'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strchr(Buffer,'\n'))!=NULL)
    *ChPtr=0;
  return(*Connect->StartReply!=0);
}


INTERNETAPI void WINAPI FtpResetLastReply()
{
  Connection::ResetLastReply();
}


INTERNETAPI void WINAPI FtpLastReply(char *Buffer1,int MaxSize1,
                                     char *Buffer2,int MaxSize2)
{
  char LastReply1[512],LastReply2[512];
  *Buffer1=*Buffer2=0;
  if (MaxSize1<1 || MaxSize2<1)
    return;
  Connection::GetLastReply(LastReply1,LastReply2);
  if (isdigit(LastReply1[0]) && isdigit(LastReply1[1]) &&
      isdigit(LastReply1[2]))
    strncpy(Buffer1,LastReply1+3,MaxSize1-1);
  else
    strncpy(Buffer1,LastReply1,MaxSize1-1);
  Buffer1[MaxSize1-1]=0;
  if (isdigit(LastReply2[0]) && isdigit(LastReply2[1]) &&
      isdigit(LastReply2[2]))
    strncpy(Buffer2,LastReply2+3,MaxSize2-1);
  else
    strncpy(Buffer2,LastReply2,MaxSize2-1);
  Buffer2[MaxSize2-1]=0;
  char *ChPtr;
  if ((ChPtr=strchr(Buffer1,'\r'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strchr(Buffer1,'\n'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strchr(Buffer2,'\r'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strchr(Buffer2,'\n'))!=NULL)
    *ChPtr=0;
}


int ParseDirLine(Connection *Connect,WIN32_FIND_DATA *FindData,DWORD *Mode,
                                                            LPSTR lpszOwner)
{
  static char *Months[12]={"Jan ","Feb ","Mar ","Apr ","May ","Jun ",
                           "Jul ","Aug ","Sep ","Oct ","Nov ","Dec "};
  char LineBuf[512];
  int I;
  while (Connect->GetOutput(LineBuf,sizeof(LineBuf)))
  {
    memset(FindData,0,sizeof(*FindData));
    if (Mode!=NULL)
      *Mode=0;
    if (lpszOwner!=NULL)
      *lpszOwner = '\0';

    char *Line=LineBuf;
    char *argv[100];
    int argc=0;
    int LineLength=strlen(Line);
    if (LineLength<20)
      continue;
    if (LineLength<40 && strnicmp(Line,"Total ",6)==0)
      continue;
    if (isdigit(Line[0]) && isdigit(Line[1]) && isdigit(Line[2]) &&
        Line[3]==' ' && strchr(Line,':')==NULL)
      continue;
    while (isspace(*Line))
      *(Line++)=0;

    {
      int MsgFound=FALSE;
      char *FTPMsg[]={"data connection","transfer complete","bytes received",
                      "DEVICE:[","Total of "};
      for (int I=0;Line[I];I++)
        for (int J=0;J<sizeof(FTPMsg)/sizeof(FTPMsg[0]);J++)
          if (Line[I]==*FTPMsg[J] || Line[I]==toupper(*FTPMsg[J]))
            if (strnicmp(Line+I,FTPMsg[J],strlen(FTPMsg[J]))==0)
            {
              MsgFound=TRUE;
              break;
            }
      if (MsgFound)
        continue;
    }

    SYSTEMTIME FileTime=LocalTime;
    FileTime.wHour=FileTime.wMinute=0;
    int MonthFound=FALSE,StrMonthFound=FALSE,YearFound=FALSE,DayFound=FALSE;
    int HourFound=FALSE,SizeFound=FALSE;
    while (argc<sizeof(argv)/sizeof(argv[0]))
    {
      while (isspace(*Line))
        *(Line++)=0;
      if (*Line==0)
        break;
      argv[argc]=Line;
      int Length=0;
      while (Line[Length]>0x20)
        Length++;
//      int LikeName=FALSE;
      char *VerPtr;
      if (argc==0 && (VerPtr=(char *)memchr(Line,';',Length))!=NULL && isdigit(VerPtr[1]))
      {
        strncpy(FindData->cFileName,Line,Length);
        FindData->cFileName[Length]=0;
        char *VMSDir;
        if ((VMSDir=strstr(FindData->cFileName,".DIR;1"))!=NULL)
        {
          *VMSDir=0;
          FindData->dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
        }
      }
      else
        if (argc==0 && *Line=='d' || strnicmp(Line,"<DIR>",5)==0 ||
            strnicmp(Line,"DIR ",4)==0)
          FindData->dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
        else
        {
          DWORD CurValue=atoi(Line);
          if (isdigit(Line[0]) && !isalpha(Line[1]) &&
              !(MonthFound && (YearFound || CurValue>99 && CurValue<1900 || CurValue>2100 || StrMonthFound) &&
              DayFound && (HourFound || YearFound && (!Line[1] || !isdigit(Line[1]) || Line[2]!=':')) && SizeFound) &&
              (*FindData->cFileName || strpbrk(Line," :-")!=NULL))
          {
            unsigned int Value=atol(Line);
            if (argc>1 || Value>100 || *FindData->cFileName ||
                Length==8 && Line[2]=='-' && Line[5]=='-' ||
                Length>=5 && Length<=7 && Line[2]==':')
            {
              int DigitCount=0,NonDigitCount=0,PrevNonDigit=TRUE;
              char *Digits[100],*FirstNonDigit=NULL;

              for (char *ChPtr=Line;*ChPtr>0x20;ChPtr++)
                if (!isdigit(*ChPtr))
                {
                  if (FirstNonDigit==NULL)
                    FirstNonDigit=ChPtr;
                  if (isalpha(*ChPtr) && ChPtr[1]>0x20)
                  {
                    DigitCount=NonDigitCount=0;
                    break;
                  }
                  NonDigitCount++;
                  PrevNonDigit=TRUE;
                }
                else
                  if (PrevNonDigit)
                  {
                    Digits[DigitCount++]=ChPtr;
                    if (DigitCount>=sizeof(Digits)/sizeof(Digits[0]))
                      break;
                    PrevNonDigit=FALSE;
                  }
              if (NonDigitCount>0)
              {
                int MonthText=FALSE,VMSSize=FALSE;
                if (NonDigitCount==5 && FirstNonDigit[0]=='-' && FirstNonDigit[4]=='-')
                  for (I=0;I<sizeof(Months)/sizeof(Months[0]);I++)
                    if (strnicmp(FirstNonDigit+1,Months[I],3)==0 &&
                        (!isspace(FirstNonDigit[4]) || !isspace(FirstNonDigit[5]) || !isspace(FirstNonDigit[6])))
                    {
                      FileTime.wMonth=I+1;
                      MonthText=StrMonthFound=MonthFound=TRUE;
                      FileTime.wDay=Value;
                      FileTime.wYear=atoi(Digits[1]);
                      MonthFound=YearFound=DayFound=TRUE;
                      break;
                    }
                if (NonDigitCount==1 && *FirstNonDigit=='/' && !SizeFound)
                {
                  FindData->nFileSizeLow=Value;
                  SizeFound=VMSSize=TRUE;
                }
                if (!MonthText && !VMSSize)
                  if (NonDigitCount>=1 && NonDigitCount<=3 && DigitCount==2)
                  {
                    FileTime.wHour=Value;
                    FileTime.wMinute=atoi(Digits[1]);
                    if (Length>5 && toupper(Line[5])=='P' && FileTime.wHour<12)
                      FileTime.wHour+=12;
                    if (FileTime.wHour<24)
                      HourFound=TRUE;
                  }
                  else
                    if (NonDigitCount==2 && DigitCount==3)
                    {
                      if (MonthFound && YearFound && DayFound)
                      {
                        FileTime.wHour=Value;
                        FileTime.wMinute=atoi(Digits[1]);
                        FileTime.wSecond=atoi(Digits[2]);
                        if (FileTime.wHour<24)
                          HourFound=TRUE;
                      }
                      else
                      {
                        FileTime.wMonth=Value;
                        FileTime.wDay=atoi(Digits[1]);
                        FileTime.wYear=atoi(Digits[2]);
                        MonthFound=TRUE;
                        YearFound=TRUE;
                        DayFound=TRUE;
                      }
                    }
                    else
                      if (argc>7 && MonthFound)
                      {
                        strcpy(FindData->cFileName,Line);
                        break;
                      }
              }
              else
                if (Value>31)
                {
                  if (SizeFound && MonthFound)
                  {
                    FileTime.wYear=Value;
                    YearFound=TRUE;
                  }
                  else
                  {
                    FindData->nFileSizeLow=Value;
                    SizeFound=TRUE;
                  }
                }
                else
                  if (Value>0 && (SizeFound || MonthFound))
                  {
                    if (DayFound)
                      if (FileTime.wDay>FindData->nFileSizeLow)
                        FindData->nFileSizeLow=FileTime.wDay;
                    FileTime.wDay=Value;
                    DayFound=TRUE;
                  }
                  else
                  {
                    FindData->nFileSizeLow=Value;
                    SizeFound=TRUE;
                  }
            }
          }
          else
            if (MonthFound || DayFound && HourFound && Length>3)
            {
              if (/* *Line!='[' && */!isalpha(*FindData->cFileName))
                strcpy(FindData->cFileName,Line);
              break;
            }
            else
              for (I=0;I<sizeof(Months)/sizeof(Months[0]);I++)
                if (strnicmp(Line,Months[I],4)==0 && (!isspace(Line[4]) || !isspace(Line[5])))
                {
                  FileTime.wMonth=I+1;
                  StrMonthFound=MonthFound=TRUE;
                  break;
                }
        }

      while (*Line>0x20)
        Line++;
      argc++;
    }

    if (argc>4 && (*argv[0]=='d' || *argv[0]=='-' || *argv[0]=='l') &&
        strlen(argv[0])==10)
    {
      if(lpszOwner!=NULL && isalnum((unsigned char)*argv[2]) &&
                            isalnum((unsigned char)*argv[3]))
      {
        char *p = stpcpy(lpszOwner, argv[2]);
        *p++ = ':';
        strcpy(p, argv[3]);
      }
      if(Mode!=NULL)
      {
        if (*argv[0]=='l')
          *Mode|=0x200;
        for(int I = 1; I <= 9; I += 3)
        {
          static char uattr[] = "rwx";
          for(int J = 0; J < 3; J++)
            if (argv[0][I+J] == uattr[J])
              *Mode |= 1<<(9-I-J);
            else
              if (argv[0][I+J] != '-')
              {
                *Mode=0;
                I=10;
                break;
              }
        }
      }
    }

    int NameLength=strlen(FindData->cFileName);
    if (NameLength==0)
      if (argc>0 && (Connect->FindAllFiles || SizeFound))
        if (strchr(argv[argc-1],':')==NULL)
          strcpy(FindData->cFileName,argv[argc-1]);
        else
          strcpy(FindData->cFileName,argv[0]);
      else
        continue;
    if (FindData->cFileName[0]=='.' && FindData->cFileName[1]==0)
      continue;
    if (FindData->cFileName[NameLength-1]=='/')
    {
      FindData->cFileName[NameLength-1]=0;
      FindData->dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
    }
    while (NameLength>0 && FindData->cFileName[NameLength-1]<=0x20)
      FindData->cFileName[NameLength--]=0;
    if (FileTime.wYear<100)
      if (FileTime.wYear<50)
        FileTime.wYear+=2000;
      else
        FileTime.wYear+=1900;
    if (FileTime.wMinute>59)
      FileTime.wMinute=LocalTime.wMinute;
    if (FileTime.wHour>23)
      FileTime.wHour=LocalTime.wHour;
    if (FileTime.wDay>31 || FileTime.wDay<1)
      FileTime.wDay=LocalTime.wDay;
    if (FileTime.wMonth>12 || FileTime.wMonth<1)
      FileTime.wMonth=LocalTime.wMonth;
    if (!YearFound && FileTime.wMonth>LocalTime.wMonth)
      FileTime.wYear--;
    FileTime.wSecond=FileTime.wMilliseconds=0;
    SystemTimeToFileTime(&FileTime,&FindData->ftLastWriteTime);
    char *LinkName;
    if (*argv[0]=='l' && (LinkName=strstr(FindData->cFileName," -> "))!=NULL)
    {
      char ResolvedName[NM];
      strcpy(ResolvedName,LinkName+4);
      *LinkName=0;
      if (strcmp(PointToName(ResolvedName),".")==0 ||
          *ResolvedName && *PointToName(ResolvedName)==0)
        FindData->dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
    }
    *FindData->cAlternateFileName=0;
    return(TRUE);
  }
  SetLastError(ERROR_NO_MORE_FILES);
  return(FALSE);
}
