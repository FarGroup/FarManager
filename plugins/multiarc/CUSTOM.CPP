#include <windows.h>
#include <string.h>
#include <dos.h>
#include "plugin.hpp"
#include "fmt.hpp"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,1)
  #if _MSC_VER
    #define _export
  #endif
#endif

#undef isspace
#define isspace(c) ((c)==' ' || (c)=='\t')

#ifdef _MSC_VER
//#pragma comment(linker, "-subsystem:console")
//#pragma comment(linker, "-merge:.rdata=.text")
#endif


BOOL WINAPI OpenArchivePipe(const char *Name,int *Type);
int GetString(char *Str,int MaxSize);
int HexCharToNum(int HexChar);
void ConvertNameToShort(const char *Src,char *Dest);
int GetSectionName(int Num,char *Name);

int CurType;

char *OutData;
DWORD OutDataPos,OutDataSize;

char FormatFileName[NM];
char StartText[256],EndText[256],Format[8][256];
int FormatNumber,IgnoreErrors;

DWORD WINAPI _export LoadFormatModule(const char *ModuleName)
{
  lstrcpy(FormatFileName,ModuleName);
  lstrcpy(strrchr(FormatFileName,'\\')+1,"custom.ini");
  return(0);
}


BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  char *Dot=strrchr((char*)Name,'.');
  for (int I=0;;I++)
  {
    char TypeName[100],Name[64],Ext[NM],ID[512];
    int IDPos;
    if (!GetSectionName(I,TypeName))
      break;
    GetPrivateProfileString(TypeName,"TypeName",TypeName,Name,sizeof(Name),FormatFileName);
    if (*Name==0)
      break;
    GetPrivateProfileString(TypeName,"ID","",ID,sizeof(ID),FormatFileName);
    IDPos=GetPrivateProfileInt(TypeName,"IDPos",-1,FormatFileName);
    if (*ID)
    {
      unsigned char IDData[256],*CurID=(unsigned char*)&ID[0];
      int IDLength=0;
      while (1)
      {
        while (isspace(*CurID))
          CurID++;
        if (*CurID==0)
          break;
        IDData[IDLength++]=HexCharToNum(CurID[0])*16+HexCharToNum(CurID[1]);
        while (*CurID && !isspace(*CurID))
          CurID++;
      }
      int Found=FALSE;
      if (IDPos>=0)
        Found=IDPos<=DataSize-IDLength && memcmp(Data+IDPos,IDData,IDLength)==0;
      else
        for (int I=0;I<=DataSize-IDLength;I++)
          if (memcmp(Data+I,IDData,IDLength)==0)
          {
            Found=TRUE;
            break;
          }
      if (Found)
      {
        if (GetPrivateProfileInt(TypeName,"IDOnly",0,FormatFileName))
        {
          CurType=I;
          return(TRUE);
        }
      }
      else
        continue;
    }
    GetPrivateProfileString(TypeName,"Extension","",Ext,sizeof(Ext),FormatFileName);
    if (Dot!=NULL && *Ext!=0 && lstrcmpi(Dot+1,Ext)==0)
    {
      CurType=I;
      return(TRUE);
    }
  }
  return(FALSE);
}

DWORD WINAPI _export GetSFXPos(void)
{
  return 0;
}

BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  char TypeName[100],Command[512];
  if (!GetSectionName(CurType,TypeName))
    return(FALSE);
  GetPrivateProfileString(TypeName,"List","",Command,sizeof(Command),FormatFileName);
  if (*Command==0)
    return(FALSE);
  IgnoreErrors=GetPrivateProfileInt(TypeName,"IgnoreErrors",0,FormatFileName);
  *Type=CurType;

  char ShortName[NM];
  ConvertNameToShort(Name,ShortName);
  lstrcat(Command," ");
  lstrcat(Command,ShortName);

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  HANDLE StdInput=GetStdHandle(STD_INPUT_HANDLE);
  char TempPath[NM],TempName[NM];
  GetTempPath(sizeof(TempPath),TempPath);
  GetTempFileName(TempPath,"FAR",0,TempName);
  HANDLE OutHandle=CreateFile(TempName,GENERIC_READ|GENERIC_WRITE,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,
                   FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
  if (OutHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);
  si.dwFlags=STARTF_USESTDHANDLES;
  si.hStdInput=StdInput;
  si.hStdOutput=OutHandle;
  si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
  DWORD ConsoleMode;
  GetConsoleMode(StdInput,&ConsoleMode);
  SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                 ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
  char SaveTitle[512];
  GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
  SetConsoleTitle(Command);

  char ExpandedCmd[512];
  ExpandEnvironmentStrings(Command,ExpandedCmd,sizeof(ExpandedCmd));

  DWORD ExitCode=CreateProcess(NULL,ExpandedCmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

  if (ExitCode)
  {
    WaitForSingleObject(pi.hProcess,INFINITE);
    GetExitCodeProcess(pi.hProcess,&ExitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ExitCode=(ExitCode<GetPrivateProfileInt(TypeName,"Errorlevel",1000,FormatFileName));
  }
  if (ExitCode)
  {
    OutData=NULL;
    DWORD FileSize=GetFileSize(OutHandle,NULL);
    if (FileSize!=0xFFFFFFFF)
    {
      SetFilePointer(OutHandle,0,NULL,FILE_BEGIN);
      OutData=(char *)GlobalAlloc(GMEM_FIXED,FileSize);
      ReadFile(OutHandle,OutData,FileSize,&OutDataSize,NULL);
      OutDataPos=0;
    }
    if (OutData==NULL)
      ExitCode=0;
  }
  SetConsoleTitle(SaveTitle);
  SetConsoleMode(StdInput,ConsoleMode);
  CloseHandle(OutHandle);

  GetPrivateProfileString(TypeName,"Start","",StartText,sizeof(StartText),FormatFileName);
  GetPrivateProfileString(TypeName,"End","",EndText,sizeof(EndText),FormatFileName);
  for (FormatNumber=0;FormatNumber<sizeof(Format)/sizeof(Format[0]);FormatNumber++)
  {
    char FormatName[100];
    wsprintf(FormatName,"Format%d",FormatNumber);
    GetPrivateProfileString(TypeName,FormatName,"",Format[FormatNumber],sizeof(Format[0]),FormatFileName);
    if (*Format[FormatNumber]==0)
      break;
  }
  if (ExitCode && OutDataSize==0)
  {
    GlobalFree((HGLOBAL)OutData);
    return(OpenArchivePipe(Name,Type));
  }
  return(ExitCode);
}


BOOL WINAPI OpenArchivePipe(const char *Name,int *Type)
{
  char TypeName[100],Command[512];
  if (!GetSectionName(CurType,TypeName))
    return(FALSE);
  GetPrivateProfileString(TypeName,"List","",Command,sizeof(Command),FormatFileName);
  if (*Command==0)
    return(FALSE);
  *Type=CurType;

  lstrcat(Command," ");
  char ShortName[NM];
  lstrcat(Command,GetShortPathName(Name,ShortName,sizeof(ShortName))?
                 ShortName:Name);

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  HANDLE hChildStdoutRd,hChildStdoutWr;
  HANDLE StdInput=GetStdHandle(STD_INPUT_HANDLE);
  HANDLE StdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE StdError=GetStdHandle(STD_ERROR_HANDLE);
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 32768))
    return(FALSE);
  SetStdHandle(STD_OUTPUT_HANDLE,hChildStdoutWr);
  SetStdHandle(STD_ERROR_HANDLE,hChildStdoutWr);

  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  DWORD ConsoleMode;
  GetConsoleMode(StdInput,&ConsoleMode);
  SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                 ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);

  char SaveTitle[512];
  GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
  SetConsoleTitle(Command);

  char ExpandedCmd[512];
  ExpandEnvironmentStrings(Command,ExpandedCmd,sizeof(ExpandedCmd));

  DWORD ExitCode=CreateProcess(NULL,ExpandedCmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

  SetStdHandle(STD_OUTPUT_HANDLE,StdOutput);
  SetStdHandle(STD_ERROR_HANDLE,StdError);
  CloseHandle(hChildStdoutWr);

  if (ExitCode)
  {
    const int ReadSize=32768;
    OutDataSize=OutDataPos=0;
    OutData=(char *)GlobalAlloc(GMEM_FIXED,0);
    while (1)
    {
      DWORD Read;
      if ((OutData=(char *)GlobalReAlloc(OutData,OutDataSize+ReadSize,GMEM_MOVEABLE))==NULL)
        return(FALSE);
      if (!ReadFile(hChildStdoutRd,OutData+OutDataSize,ReadSize,&Read,NULL))
        break;
      OutDataSize+=Read;
    }
    CloseHandle(hChildStdoutRd);
    WaitForSingleObject(pi.hProcess,INFINITE);
    GetExitCodeProcess(pi.hProcess,&ExitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ExitCode=(ExitCode<GetPrivateProfileInt(TypeName,"Errorlevel",1000,FormatFileName));
    if (!ExitCode)
      GlobalFree((HGLOBAL)OutData);
  }
  SetConsoleTitle(SaveTitle);
  SetConsoleMode(StdInput,ConsoleMode);

  GetPrivateProfileString(TypeName,"Start","",StartText,sizeof(StartText),FormatFileName);
  GetPrivateProfileString(TypeName,"End","",EndText,sizeof(EndText),FormatFileName);
  for (FormatNumber=0;FormatNumber<sizeof(Format)/sizeof(Format[0]);FormatNumber++)
  {
    char FormatName[100];
    wsprintf(FormatName,"Format%d",FormatNumber);
    GetPrivateProfileString(TypeName,FormatName,"",Format[FormatNumber],sizeof(Format[0]),FormatFileName);
    if (*Format[FormatNumber]==0)
      break;
  }
  return(ExitCode);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  char Str[512];
  int FormatLine=0;
  SYSTEMTIME st,syst;

  memset(&st,0,sizeof(st));
  GetSystemTime(&syst);

  while (GetString(Str,sizeof(Str)))
  {
    if (*StartText)
    {
      if (*StartText=='^' && strncmp(Str,StartText+1,lstrlen(StartText+1))==0 ||
          *StartText!='^' && strstr(Str,StartText)!=NULL)
        *StartText=0;
      continue;
    }
    if (*EndText)
    {
      if (*EndText=='^')
      {
        if (strncmp(Str,EndText+1,lstrlen(EndText+1))==0)
          break;
      }
      else
        if (strstr(Str,EndText)!=NULL)
          break;
    }

    char *CurFormat=Format[FormatLine];
    char *CurStr=Str;
    for (;*CurStr && *CurFormat;CurFormat++,CurStr++)
    {
      switch(*CurFormat)
      {
        case '*':
          while (CurStr[0] && CurStr[1] && !isspace(CurStr[0]) && !isspace(CurStr[1]))
            CurStr++;
          break;
        case 'n':
          strncat(Item->FindData.cFileName,CurStr,1);
          break;
        case 'c':
          strncat(Info->Description,CurStr,1);
          break;
        case '.':
          {
            for (int I=lstrlen(Item->FindData.cFileName);I>=0;I--)
              if (isspace(Item->FindData.cFileName[I]))
                Item->FindData.cFileName[I]=0;
            if (*Item->FindData.cFileName)
              lstrcat(Item->FindData.cFileName,".");
          }
          break;
        case 'z':
          if (isdigit(*CurStr))
            Item->FindData.nFileSizeLow=Item->FindData.nFileSizeLow*10+(*CurStr-'0');
          break;
        case 'p':
          if (isdigit(*CurStr))
            Item->PackSize=Item->PackSize*10+(*CurStr-'0');
          break;
        case 'a':
          switch(toupper(*CurStr))
          {
            case 'D':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
              break;
            case 'H':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_HIDDEN;
              break;
            case 'A':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_ARCHIVE;
              break;
            case 'R':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_READONLY;
              break;
            case 'S':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_SYSTEM;
              break;
            case 'C':
              Item->FindData.dwFileAttributes|=FILE_ATTRIBUTE_COMPRESSED;
              break;
          }
          break;
        case 'y':
          if (isdigit(*CurStr))
            st.wYear=st.wYear*10+(*CurStr-'0');
          break;
        case 'd':
          if (isdigit(*CurStr))
            st.wDay=st.wDay*10+(*CurStr-'0');
          break;
        case 't':
          if (isdigit(*CurStr))
            st.wMonth=st.wMonth*10+(*CurStr-'0');
          break;
        case 'T':
          {
            static char *Months[12]={"Jan","Feb","Mar","Apr","May","Jun",
                                     "Jul","Aug","Sep","Oct","Nov","Dec"};
            for (int I=0;I<sizeof(Months)/sizeof(Months[0]);I++)
              if (strnicmp(CurStr,Months[I],3)==0)
              {
                st.wMonth=I+1;
                while (CurFormat[1]=='T' && CurStr[1])
                {
                  CurStr++;
                  CurFormat++;
                }
                break;
              }
          }
          break;
        case 'h':
          if (isdigit(*CurStr))
            st.wHour=st.wHour*10+(*CurStr-'0');
          break;
        case 'H':
          switch(toupper(*CurStr))
          {
            case 'A':
              if (st.wHour==12)
                st.wHour-=12;
              break;
            case 'P':
              if (st.wHour<12)
                st.wHour+=12;
              break;
          }
          break;
        case 'm':
          if (isdigit(*CurStr))
            st.wMinute=st.wMinute*10+(*CurStr-'0');
          break;
        case 's':
          if (isdigit(*CurStr))
            st.wSecond=st.wSecond*10+(*CurStr-'0');
          break;
      }
    }
    if (++FormatLine==FormatNumber)
    {
      if (st.wDay==0)
        st.wDay=syst.wDay;
      if (st.wMonth==0)
        st.wMonth=syst.wMonth;
      if (st.wYear==0)
        st.wYear=syst.wYear;
      else
        if (st.wYear<50)
          st.wYear+=2000;
        else
          if (st.wYear<100)
            st.wYear+=1900;
      FILETIME ft;
      SystemTimeToFileTime(&st,&ft);
      LocalFileTimeToFileTime(&ft,&Item->FindData.ftLastWriteTime);
      for (int I=lstrlen(Item->FindData.cFileName)-1;I>=0;I--)
      {
        int Ch=Item->FindData.cFileName[I];
        if (Ch==' ' || Ch=='\t')
          Item->FindData.cFileName[I]=0;
        else
          break;
      }
      return(GETARC_SUCCESS);
    }
  }
  return(GETARC_EOF);
}


int GetString(char *Str,int MaxSize)
{
  if (OutDataPos>=OutDataSize)
    return(FALSE);
  int StartPos=OutDataPos;
  while (OutDataPos<OutDataSize)
  {
    int Ch=OutData[OutDataPos];
    if (Ch=='\r' || Ch=='\n')
      break;
    OutDataPos++;
  }
  int Length=OutDataPos-StartPos;
  int DestLength=Length>=MaxSize ? MaxSize-1:Length;
  lstrcpyn(Str,OutData+StartPos,DestLength);
  Str[DestLength]=0;
  while (OutDataPos<OutDataSize)
  {
    int Ch=OutData[OutDataPos];
    if (Ch!='\r' && Ch!='\n')
      break;
    OutDataPos++;
  }
  return(TRUE);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  if (IgnoreErrors)
    Info->Flags|=AF_IGNOREERRORS;
  GlobalFree((HGLOBAL)OutData);
  return(TRUE);
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  char TypeName[100];
  if (!GetSectionName(Type,TypeName))
    return(FALSE);
  GetPrivateProfileString(TypeName,"TypeName",TypeName,FormatName,64,FormatFileName);
  GetPrivateProfileString(TypeName,"Extension","",DefaultExt,NM,FormatFileName);
  return(*FormatName!=0);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  char TypeName[100],FormatName[100];
  if (!GetSectionName(Type,TypeName))
    return(FALSE);
  GetPrivateProfileString(TypeName,"TypeName",TypeName,FormatName,64,FormatFileName);
  if (*FormatName==0)
    return(FALSE);
  static char *CmdNames[]={"Extract","ExtractWithoutPath","Test","Delete",
              "Comment","CommentFiles","SFX","Lock","Protect","Recover",
              "Add","Move","AddRecurse","MoveRecurse","AllFilesMask"};
  if (Command<sizeof(CmdNames)/sizeof(CmdNames[0]))
  {
    GetPrivateProfileString(TypeName,CmdNames[Command],"",Dest,512,FormatFileName);
    return(TRUE);
  }
  return(FALSE);
}


int HexCharToNum(int HexChar)
{
  HexChar=toupper(HexChar);
  if (HexChar>='0' && HexChar<='9')
    return(HexChar-'0');
  else
    if (HexChar>='A' && HexChar<='F')
      return(HexChar-'A'+10);
  return(0);
}


void ConvertNameToShort(const char *Src,char *Dest)
{
  char ShortName[NM],AnsiName[NM];
  int AnsiApis=AreFileApisANSI();
  if (!AnsiApis)
    SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)) && *ShortName)
    CharToOem(ShortName,Dest);
  else
    lstrcpy(Dest,Src);
  if (!AnsiApis)
    SetFileApisToOEM();
}


int GetSectionName(int Num,char *Name)
{
  char Buf[8192],*Section=Buf;
  GetPrivateProfileSectionNames(Buf,sizeof(Buf),FormatFileName);
  while (Num-- > 0)
  {
    if (*Section==0)
      return(FALSE);
    Section+=lstrlen(Section)+1;
  }
  lstrcpy(Name,Section);
  return(TRUE);
}
