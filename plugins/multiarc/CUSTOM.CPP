/*
  CUSTOM.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2002 FAR group
*/
/* Revision: 1.17 15.08.2002 $ */

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>
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
int GetSectionName(int Num,char *Name,int MaxSize);
void FillFormat(const char *TypeName);

int CurType;

char *OutData;
DWORD OutDataPos,OutDataSize;

char FormatFileName[NM];
enum {PROF_STR_LEN=256};
char StartText[PROF_STR_LEN],EndText[PROF_STR_LEN];

class StringList
{
public:
	StringList() : pNext(0) {str[0]='\0';}
	~StringList() {delete pNext;}
	StringList* Add();
	StringList* Next() {return pNext;}
	char* Str() {return str;}
	void Empty();
	void* operator new(size_t nSize)
	{
		return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nSize);
	}
	void operator delete(void* p)
	{
		HeapFree(GetProcessHeap(), 0, p);
	}
protected:
	char str[PROF_STR_LEN];
	StringList* pNext;
};

StringList* StringList::Add()
{
	delete pNext;
	pNext=new StringList;
	return pNext;
}

void StringList::Empty()
{
	delete pNext;
	pNext=0;
	str[0]='\0';
}

StringList* Format=0;
StringList* IgnoreStrings=0;

int IgnoreErrors;
int ArcChapters;

const char Str_TypeName[]="TypeName";

FARSTDLOCALSTRICMP LStricmp;
FARSTDLOCALSTRNICMP LStrnicmp;
FARSTDSPRINTF SPrintf;
FARSTDMKTEMP MkTemp;

void  WINAPI _export SetFarInfo(const struct PluginStartupInfo *Info)
{
  LStricmp=Info->FSF->LStricmp;
  LStrnicmp=Info->FSF->LStrnicmp;
  SPrintf=Info->FSF->sprintf;
  MkTemp=Info->FSF->MkTemp;
}

DWORD WINAPI _export LoadFormatModule(const char *ModuleName)
{
  strcpy(FormatFileName,ModuleName);
  strcpy(strrchr(FormatFileName,'\\')+1,"custom.ini");
  return(0);
}


BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  char *Dot=strrchr((char*)Name,'.');

  for (int I=0;;I++)
  {
    char TypeName[NM],Name[NM],Ext[NM],ID[512];
    int IDPos;

    if (!GetSectionName(I,TypeName,sizeof(TypeName)))
      break;

    GetPrivateProfileString(TypeName,Str_TypeName,TypeName,Name,sizeof(Name),FormatFileName);

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
      {
        for (int I=0;I<=DataSize-IDLength;I++)
          if (memcmp(Data+I,IDData,IDLength)==0)
          {
            Found=TRUE;
            break;
          }
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

    if (Dot!=NULL && *Ext!=0 && LStricmp(Dot+1,Ext)==0)
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
  char TypeName[NM],Command[512];

  if (!GetSectionName(CurType,TypeName,sizeof(TypeName)))
    return(FALSE);

  GetPrivateProfileString(TypeName,"List","",Command,sizeof(Command),FormatFileName);

  if (*Command==0)
    return(FALSE);

  IgnoreErrors=GetPrivateProfileInt(TypeName,"IgnoreErrors",0,FormatFileName);
  *Type=CurType;

  ArcChapters=-1;

  char ShortName[NM];
  ConvertNameToShort(Name,ShortName);
  strcat(Command," ");
  strcat(Command,ShortName);

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  HANDLE StdInput=GetStdHandle(STD_INPUT_HANDLE);

  char TempName[NM];
  if (MkTemp(TempName,"FAR")==NULL)
    return(FALSE);

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
  si.hStdError=/*GetStdHandle(STD_ERROR_HANDLE)*/OutHandle;
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

  FillFormat(TypeName);

  if (ExitCode && OutDataSize==0)
  {
    GlobalFree((HGLOBAL)OutData);
    return(OpenArchivePipe(Name,Type));
  }

  return(ExitCode);
}


BOOL WINAPI OpenArchivePipe(const char *Name,int *Type)
{
  char TypeName[NM],Command[512];
  if (!GetSectionName(CurType,TypeName,sizeof(TypeName)))
    return(FALSE);
  GetPrivateProfileString(TypeName,"List","",Command,sizeof(Command),FormatFileName);
  if (*Command==0)
    return(FALSE);
  *Type=CurType;

  strcat(Command," ");
  char ShortName[NM];
  strcat(Command,GetShortPathName(Name,ShortName,sizeof(ShortName))?
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

  FillFormat(TypeName);

  return(ExitCode);
}

void MakeFiletime(SYSTEMTIME st, SYSTEMTIME syst, LPFILETIME pft)
{
      if (st.wDay==0)
        st.wDay=syst.wDay;
      if (st.wMonth==0)
        st.wMonth=syst.wMonth;
      if (st.wYear==0)
        st.wYear=syst.wYear;
      else
      {
        if (st.wYear<50)
          st.wYear+=2000;
        else if (st.wYear<100)
          st.wYear+=1900;
      }

      FILETIME ft;
      if (SystemTimeToFileTime(&st,&ft))
	  {
		LocalFileTimeToFileTime(&ft,pft);
	  }
}

int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  char Str[512];
  StringList* CurFormatNode=Format;
  SYSTEMTIME stModification,stCreation,stAccess,syst;

  memset(&stModification,0,sizeof(stModification));
  memset(&stCreation,0,sizeof(stCreation));
  memset(&stAccess,0,sizeof(stAccess));
  GetSystemTime(&syst);

  while (GetString(Str,sizeof(Str)))
  {
    if (*StartText)
    {
      if (*StartText=='^' && strncmp(Str,StartText+1,strlen(StartText+1))==0 ||
          *StartText!='^' && strstr(Str,StartText)!=NULL)
        *StartText=0;
      continue;
    }

    if (*EndText)
    {
      if (*EndText=='^')
      {
        if (strncmp(Str,EndText+1,strlen(EndText+1))==0)
          break;
      }
      else
        if (strstr(Str,EndText)!=NULL)
          break;
    }

	bool bFoundIgnoreString=false;
	for (StringList* CurIgnoreString=IgnoreStrings; CurIgnoreString->Next(); CurIgnoreString=CurIgnoreString->Next())
	{
      if (*CurIgnoreString->Str()=='^')
      {
        if (strncmp(Str,CurIgnoreString->Str()+1,strlen(CurIgnoreString->Str()+1))==0)
          bFoundIgnoreString=true;
      }
      else
        if (strstr(Str,CurIgnoreString->Str())!=NULL)
          bFoundIgnoreString=true;
	}
	if (bFoundIgnoreString) continue;

    const char *CurFormat=CurFormatNode->Str();
    char *CurStr=Str;

	enum {OP_OUTSIDE, OP_INSIDE, OP_SKIP} OptionalPart=OP_OUTSIDE;
	int IsChapter=0;
    for (;*CurStr && *CurFormat;CurFormat++,CurStr++)
    {
		if (OptionalPart==OP_SKIP)
		{
			if (*CurFormat==')') OptionalPart=OP_OUTSIDE;
			CurStr--;
			continue;
		}
      switch(*CurFormat)
      {
        case '*':
          if(isspace(*CurStr) || !CurStr)
            CurStr--;
          else
            while (/*CurStr[0] &&*/ CurStr[1] /*&& !isspace(CurStr[0])*/ && !isspace(CurStr[1]))
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
            for (int I=strlen(Item->FindData.cFileName);I>=0;I--)
              if (isspace(Item->FindData.cFileName[I]))
                Item->FindData.cFileName[I]=0;
            if (*Item->FindData.cFileName)
              strcat(Item->FindData.cFileName,".");
          }
          break;
        case 'z':
          if (isdigit(*CurStr))
            Item->FindData.nFileSizeLow=Item->FindData.nFileSizeLow*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'p':
          if (isdigit(*CurStr))
            Item->PackSize=Item->PackSize*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
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
// MODIFICATION DATETIME
        case 'y':
          if (isdigit(*CurStr))
            stModification.wYear=stModification.wYear*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'd':
          if (isdigit(*CurStr))
            stModification.wDay=stModification.wDay*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 't':
          if (isdigit(*CurStr))
            stModification.wMonth=stModification.wMonth*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'T':
          {
            static const char *Months[12]={"Jan","Feb","Mar","Apr","May","Jun",
                                     "Jul","Aug","Sep","Oct","Nov","Dec"};
            for (int I=0;I<sizeof(Months)/sizeof(Months[0]);I++)
              if (LStrnicmp(CurStr,Months[I],3)==0)
              {
                stModification.wMonth=I+1;
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
            stModification.wHour=stModification.wHour*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'H':
          switch(toupper(*CurStr))
          {
            case 'A':
              if (stModification.wHour==12)
                stModification.wHour-=12;
              break;
            case 'P':
              if (stModification.wHour<12)
                stModification.wHour+=12;
              break;
          }
          break;
        case 'm':
          if (isdigit(*CurStr))
            stModification.wMinute=stModification.wMinute*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 's':
          if (isdigit(*CurStr))
            stModification.wSecond=stModification.wSecond*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
// ACCESS DATETIME
		case 'b':
          if (isdigit(*CurStr))
            stAccess.wDay=stAccess.wDay*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'v':
          if (isdigit(*CurStr))
            stAccess.wMonth=stAccess.wMonth*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'e':
          if (isdigit(*CurStr))
            stAccess.wYear=stAccess.wYear*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'x':
          if (isdigit(*CurStr))
            stAccess.wHour=stAccess.wHour*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'l':
          if (isdigit(*CurStr))
            stAccess.wMinute=stAccess.wMinute*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'k':
          if (isdigit(*CurStr))
            stAccess.wSecond=stAccess.wSecond*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
// CREATION DATETIME
        case 'j':
          if (isdigit(*CurStr))
            stCreation.wDay=stCreation.wDay*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'g':
          if (isdigit(*CurStr))
            stCreation.wMonth=stCreation.wMonth*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'f':
          if (isdigit(*CurStr))
            stCreation.wYear=stCreation.wYear*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'o':
          if (isdigit(*CurStr))
            stCreation.wHour=stCreation.wHour*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'i':
          if (isdigit(*CurStr))
            stCreation.wMinute=stCreation.wMinute*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
        case 'u':
          if (isdigit(*CurStr))
            stCreation.wSecond=stCreation.wSecond*10+(*CurStr-'0');
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
          break;
		case 'r':
		  if (isxdigit(toupper(*CurStr)))
		  {
		    char dig_sub=(*CurStr >= 'a'?'a':(*CurStr >= 'A'?'A':'0'));
            Item->CRC32=Item->CRC32*16+(*CurStr-dig_sub);
          }
		  else if (OP_INSIDE==OptionalPart)
			{
				CurStr--;
				OptionalPart=OP_SKIP;
			}
			break;
		case 'C':
		  if (*CurStr == '-')
		  {
		    IsChapter=1;
	        ArcChapters=0;
		  }
		  else if(isdigit(*CurStr))
		  {
		    if(IsChapter)
  		       ArcChapters=ArcChapters*10+ (*CurStr-'0');
  		    else
               Info->Chapter=Info->Chapter*10+ (*CurStr-'0');
		  }
          break;
		case '(':
			OptionalPart=OP_INSIDE;
			CurStr--;
			break;
		case ')':
			OptionalPart=OP_OUTSIDE;
			CurStr--;
			break;
      }
    }

	CurFormatNode=CurFormatNode->Next();
    if (!CurFormatNode || !CurFormatNode->Next())
    {
	  MakeFiletime(stModification, syst, &Item->FindData.ftLastWriteTime);
	  MakeFiletime(stCreation, syst, &Item->FindData.ftCreationTime);
	  MakeFiletime(stAccess, syst, &Item->FindData.ftLastAccessTime);

      for (int I=strlen(Item->FindData.cFileName)-1;I>=0;I--)
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
  strncpy(Str,OutData+StartPos,DestLength);
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

  if(ArcChapters < 0)
    ArcChapters=0;
  Info->Chapters=ArcChapters;

  GlobalFree((HGLOBAL)OutData);

  delete Format;
  delete IgnoreStrings;
  Format=0;
  IgnoreStrings=0;

  return(TRUE);
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  char TypeName[NM];

  if (!GetSectionName(Type,TypeName,sizeof(TypeName)))
    return(FALSE);

  GetPrivateProfileString(TypeName,Str_TypeName,TypeName,FormatName,64,FormatFileName);
  GetPrivateProfileString(TypeName,"Extension","",DefaultExt,NM,FormatFileName);

  return(*FormatName!=0);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  char TypeName[NM],FormatName[NM];

  if (!GetSectionName(Type,TypeName,sizeof(TypeName)))
    return(FALSE);

  GetPrivateProfileString(TypeName,Str_TypeName,TypeName,FormatName,64,FormatFileName);

  if (*FormatName==0)
    return(FALSE);

  static const char *CmdNames[]={"Extract","ExtractWithoutPath","Test","Delete",
              "Comment","CommentFiles","SFX","Lock","Protect","Recover",
              "Add","Move","AddRecurse","MoveRecurse","AllFilesMask"};

  if (Command < sizeof(CmdNames)/sizeof(CmdNames[0]))
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
    strcpy(Dest,Src);

  if (!AnsiApis)
    SetFileApisToOEM();
}


int GetSectionName(int Num, char *Name, int MaxSize)
{
  char Buf[8192];
  char *Section=Buf;
  GetPrivateProfileSectionNames(Buf, sizeof(Buf), FormatFileName);
  while(*Section)
  {
    if(*Section!=';')
    {
      if(!Num)
      {
        strncpy(Name, Section, MaxSize-1);
        return TRUE;
      }
      Num--;
    }
    Section+=strlen(Section)+1;
  }
  return FALSE;
}

void FillFormat(const char *TypeName)
{
  GetPrivateProfileString(TypeName,"Start","",StartText,sizeof(StartText),FormatFileName);
  GetPrivateProfileString(TypeName,"End","",EndText,sizeof(EndText),FormatFileName);

  int FormatNumber=0;
  delete Format;
  Format=new StringList;
  for (StringList* CurFormat=Format; ; CurFormat=CurFormat->Add())
  {
    char FormatName[100];
    SPrintf(FormatName,"Format%d",FormatNumber++);
    GetPrivateProfileString(TypeName,FormatName,"",CurFormat->Str(),PROF_STR_LEN,FormatFileName);
    if (*CurFormat->Str()==0)
      break;
  }

  int Number=0;
  delete IgnoreStrings;
  IgnoreStrings=new StringList;
  for (StringList* CurIgnoreString=IgnoreStrings; ; CurIgnoreString=CurIgnoreString->Add())
  {
    char Name[100];
    SPrintf(Name,"IgnoreString%d",Number++);
    GetPrivateProfileString(TypeName,Name,"",CurIgnoreString->Str(),PROF_STR_LEN,FormatFileName);
    if (*CurIgnoreString->Str()==0)
      break;
  }
}
