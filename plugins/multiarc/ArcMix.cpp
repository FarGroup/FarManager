#include "MultiArc.hpp"
#include "marclng.hpp"
#include <dos.h>

#include <cstdlib>

BOOL FileExists(const char* Name)
{
  return GetFileAttributes(Name)!=0xFFFFFFFF;
}

BOOL GoToFile(const char *Target, BOOL AllowChangeDir)
{
  if(!Target || !*Target)
    return FALSE;
  BOOL rc=FALSE, search=TRUE;
  PanelRedrawInfo PRI;
  PanelInfo PInfo;
  char Name[NM], Dir[NM*5];
  int pathlen;

  lstrcpy(Name,FSF.PointToName(const_cast<char*>(Target)));
  pathlen=(int)(FSF.PointToName(const_cast<char*>(Target))-Target);
  if(pathlen)
    memcpy(Dir,Target,pathlen);
  Dir[pathlen]=0;

  FSF.Trim(Name);
  FSF.Trim(Dir);
  FSF.Unquote(Name);
  FSF.Unquote(Dir);

  Info.Control(INVALID_HANDLE_VALUE,FCTL_UPDATEPANEL,(void*)1);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
  pathlen=lstrlen(Dir);
  if(pathlen)
  {
    if(*PInfo.CurDir &&
       PInfo.CurDir[lstrlen(PInfo.CurDir)-1]!='\\' // old path != "*\"
       && Dir[pathlen-1]=='\\')
      Dir[pathlen-1]=0;

    if(0!=FSF.LStricmp(Dir,PInfo.CurDir))
    {
      if(AllowChangeDir)
      {
        Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,&Dir);
        Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
      }
      else
        search=FALSE;
    }
  }

  PRI.CurrentItem=PInfo.CurrentItem;
  PRI.TopPanelItem=PInfo.TopPanelItem;
  if(search)
  {
    for(int J=0; J < PInfo.ItemsNumber; J++)
    {
      if(!FSF.LStricmp(Name,
         FSF.PointToName(PInfo.PanelItems[J].FindData.cFileName)))
      {
        PRI.CurrentItem=J;
        PRI.TopPanelItem=J;
        rc=TRUE;
        break;
      }
    }
  }
  return rc?Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,&PRI):FALSE;
}

int __isspace(int Chr)
{
   return Chr == 0x09 || Chr == 0x0A || Chr == 0x0B || Chr == 0x0C || Chr == 0x0D || Chr == 0x20;
}


const char *GetMsg(int MsgId)
{
  return Info.GetMsg(Info.ModuleNumber,MsgId);
}


/* $ 13.09.2000 tran
   запуск треда для ожидания момента убийства лист файла */
void StartThreadForKillListFile(PROCESS_INFORMATION *pi,char *list)
{
    if (!pi || !list || !*list)
        return;
    KillStruct *ks;
    DWORD dummy;

    ks=(KillStruct*)GlobalAlloc(GPTR,sizeof(KillStruct));
    if (!ks)
        return ;

    ks->hThread=pi->hThread;
    ks->hProcess=pi->hProcess;
    lstrcpy(ks->ListFileName,list);

    CreateThread(NULL,0xf000,ThreadWhatWaitingForKillListFile,ks,0 /* no flags */,&dummy);
}

DWORD WINAPI ThreadWhatWaitingForKillListFile(LPVOID par)
{
    KillStruct *ks=(KillStruct*)par;

    WaitForSingleObject(ks->hProcess,INFINITE);
    CloseHandle(ks->hThread);
    CloseHandle(ks->hProcess);
    DeleteFile(ks->ListFileName);
    GlobalFree((LPVOID)ks);
    return SUPER_PUPER_ZERO;
}
/* tran 13.09.2000 $ */

int Execute(HANDLE hPlugin,char *CmdStr,int HideOutput,int Silent,int ShowTitle,char *ListFileName)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  int ExitCode,ExitCode2,LastError;

  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  HANDLE hChildStdoutRd{}, hChildStdoutWr{};
  HANDLE StdInput=GetStdHandle(STD_INPUT_HANDLE);
  HANDLE StdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE StdError=GetStdHandle(STD_ERROR_HANDLE);
  HANDLE hScreen=NULL;
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (HideOutput)
  {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 32768))
    {
      if (Silent)
      {
        /*hScreen=Info.SaveScreen(0,0,-1,0);
        Info.Text(2,0,LIGHTGRAY,GetMsg(MWaitForExternalProgram))*/;
      }
      else
      {
        hScreen=Info.SaveScreen(0,0,-1,-1);
        const char *MsgItems[]={"",GetMsg(MWaitForExternalProgram)};
        Info.Message(Info.ModuleNumber,0,NULL,MsgItems,
                      ARRAYSIZE(MsgItems),0);
      }
      SetStdHandle(STD_OUTPUT_HANDLE,hChildStdoutWr);
      SetStdHandle(STD_ERROR_HANDLE,hChildStdoutWr);
    }
    else
      HideOutput=FALSE;
  }
  else
  {
    Info.Control(hPlugin, FCTL_GETUSERSCREEN, NULL);
    GetConsoleScreenBufferInfo(StdOutput, &csbi);
    COORD C = { 0, csbi.dwCursorPosition.Y };
    SetConsoleCursorPosition(StdOutput, C);
  }


  DWORD ConsoleMode;
  GetConsoleMode(StdInput,&ConsoleMode);
  SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                 ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);

  char ExpandedCmd[MAX_COMMAND_LENGTH];
  ExpandEnvironmentStrings(CmdStr,ExpandedCmd,sizeof(ExpandedCmd));
  FSF.LTrim(ExpandedCmd); //$ AA 12.11.2001

  char SaveTitle[512];
  GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
  if (ShowTitle)
    SetConsoleTitle(ExpandedCmd);

  /* $ 14.02.2001 raVen
     делать окошку minimize, если в фоне */
  if (Opt.Background)
  {
    si.dwFlags=si.dwFlags | STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_MINIMIZE;
  }
  /* raVen $ */

  ExitCode2=ExitCode=CreateProcess(NULL,ExpandedCmd,NULL,NULL,HideOutput,
            (Opt.Background?CREATE_NEW_CONSOLE:0)|PriorityProcessCode[Opt.PriorityClass],
            NULL,NULL,&si,&pi);

  LastError=!ExitCode?GetLastError():0;
  if (HideOutput)
  {
    SetStdHandle(STD_OUTPUT_HANDLE,StdOutput);
    SetStdHandle(STD_ERROR_HANDLE,StdError);
    CloseHandle(hChildStdoutWr);
  }

  SetLastError(LastError);
  if(!ExitCode2 /*|| !FindExecuteFile(ExecuteName,NULL,0)*/) //$ 06.03.2002 AA
  {
    char Msg[100];
    char ExecuteName[NM];
    lstrcpyn(ExecuteName,ExpandedCmd+(*ExpandedCmd=='"'), NM);
    char *Ptr;
    Ptr=strchr(ExecuteName,(*ExpandedCmd=='"')?'"':' ');
    if(Ptr)
      *Ptr=0;
    FindExecuteFile(ExecuteName,NULL,0);

    char NameMsg[NM];
    FSF.TruncPathStr(lstrcpyn(NameMsg,ExecuteName,sizeof(NameMsg)),MAX_WIDTH_MESSAGE);
    FSF.sprintf(Msg,GetMsg(MCannotFindArchivator),NameMsg);
    const char *MsgItems[]={GetMsg(MError),Msg, GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,
                 NULL,MsgItems,ARRAYSIZE(MsgItems),1);
    ExitCode=RETEXEC_ARCNOTFOUND;
  }

  if (ExitCode && ExitCode!=RETEXEC_ARCNOTFOUND)
  {
    if (HideOutput)
    {
      WaitForSingleObject(pi.hProcess,1000);

      char PipeBuf[32768];
      DWORD Read;
      while (ReadFile(hChildStdoutRd,PipeBuf,sizeof(PipeBuf),&Read,NULL))
        ;
      CloseHandle(hChildStdoutRd);
    }
    /* $ 13.09.2000 tran
       фоновой выполнение */
    if ( !Opt.Background )
    {
        WaitForSingleObject(pi.hProcess,INFINITE);
        GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else
    {
        StartThreadForKillListFile(&pi,ListFileName); // нехай за процессом тред следит, и файл бъет тапком
        ExitCode=0;
    }
    /* tran 13.09.2000 $ */
  }
  SetConsoleTitle(SaveTitle);
  SetConsoleMode(StdInput,ConsoleMode);
  if (!HideOutput)
  {
    Info.Control(hPlugin,FCTL_SETUSERSCREEN,NULL);
  }
  if (hScreen)
  {
    Info.RestoreScreen(NULL);
    Info.RestoreScreen(hScreen);
  }

  return ExitCode;
}


char* QuoteText(char *Str)
{
  int LastPos=lstrlen(Str);
  memmove(Str+1,Str,LastPos+1);
  Str[LastPos+1]=*Str='"';
  Str[LastPos+2]=0;
  return Str;
}


void ConvertNameToShort(const char *Src,char *Dest)
{
  char ShortName[NM],AnsiName[NM];
  SetFileApisToANSI();
  OemToChar(Src,AnsiName);
  if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)))
    CharToOem(ShortName,Dest);
  else
    lstrcpy(Dest,Src);
  SetFileApisToOEM();
}


void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  int I;
  struct FarDialogItem *PItem=Item;
  const struct InitDialogItem *PInit=Init;
  for (I=0;I<ItemsNumber;I++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Focus=PInit->Focus;
    PItem->History=(const char *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    lstrcpy(PItem->Data,((DWORD_PTR)PInit->Data<2000)?GetMsg((unsigned int)(DWORD_PTR)PInit->Data):PInit->Data);
  }
}


static void __InsertCommas(char *Dest)
{
  int I;
  for (I=lstrlen(Dest)-4;I>=0;I-=3)
    if (Dest[I])
    {
      memmove(Dest+I+2,Dest+I+1,lstrlen(Dest+I));
      Dest[I+1]=',';
    }
}

void InsertCommas(unsigned long Number,char *Dest)
{
  __InsertCommas(FSF.itoa(Number,Dest,10));
}

void InsertCommas(__int64 Number,char *Dest)
{
  __InsertCommas(FSF.itoa64(Number,Dest,10));
}


int ToPercent(long N1,long N2)
{
  if (N1 > 10000)
  {
    N1/=100;
    N2/=100;
  }
  if (N2==0)
    return 0;
  if (N2<N1)
    return 100;
  return (int)(N1*100/N2);
}

int ToPercent(__int64 N1,__int64 N2)
{
  if (N1 > 10000)
  {
    N1/=100;
    N2/=100;
  }
  if (N2==0)
    return 0;
  if (N2<N1)
    return 100;
  return (int)(N1*100/N2);
}

int IsCaseMixed(const char *Str)
{
  while (*Str && !IsCharAlpha(*Str))
    Str++;
  int Case=IsCharLower(*Str);
  while (*(Str++))
    if (IsCharAlpha(*Str) && IsCharLower(*Str)!=Case)
      return TRUE;
  return FALSE;
}


int CheckForEsc()
{
  int ExitCode=FALSE;
  while (1)
  {
    INPUT_RECORD rec;
    /*static*/ HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
    DWORD ReadCount;
    PeekConsoleInput(hConInp,&rec,1,&ReadCount);
    if (ReadCount==0)
      break;
    ReadConsoleInput(hConInp,&rec,1,&ReadCount);
    if (rec.EventType==KEY_EVENT)
      if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE &&
          rec.Event.KeyEvent.bKeyDown)
        ExitCode=TRUE;
  }
  return ExitCode;
}


int LocalStrnicmp(const char *Str1,const char *Str2,int Length)
{
  char AnsiStr1[8192],AnsiStr2[8192];
  OemToChar(Str1,AnsiStr1);
  OemToChar(Str2,AnsiStr2);
  AnsiStr1[Length]=AnsiStr2[Length]=0;
  CharLower(AnsiStr1);
  CharLower(AnsiStr2);
  return lstrcmp(AnsiStr1,AnsiStr2);
}

char *GetCommaWord(char *Src,char *Word,char Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return NULL;
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='[' && strchr(Src+1,']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==']')
      SkipBrackets=FALSE;
    if (*Src==Separator && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (__isspace(*Src))
        Src++;
      return Src;
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return Src;
}

int FindExecuteFile(char *OriginalName,char *DestName,int SizeDest)
{
  char Env[512];
  char TempDestName[1024],*FilePart;
  char Ext[64],*PtrExt;

  if((PtrExt=strrchr(OriginalName,'.')) == NULL)
  {
    if(!GetEnvironmentVariable("PATHEXT",Env,sizeof(Env)))
      lstrcpy(Env,".exe;.com;.bat;.cmd");

    PtrExt=Env;
    while(1)
    {
      if((PtrExt=GetCommaWord(PtrExt,Ext,';')) == NULL)
        break;
      if(SearchPath(NULL,OriginalName,Ext,
               sizeof(TempDestName),  // size, in characters, of buffer
               TempDestName,  // address of buffer for found filename
               &FilePart))  // address of pointer to file component
      {
        if(DestName)
          lstrcpyn(DestName,TempDestName,SizeDest);
        return TRUE;
      }
    }
  }
  else if(SearchPath(NULL,OriginalName,PtrExt,
               sizeof(TempDestName),  // size, in characters, of buffer
               TempDestName,  // address of buffer for found filename
               &FilePart))  // address of pointer to file component
  {
    if(DestName)
      lstrcpyn(DestName,TempDestName,SizeDest);
    return TRUE;
  }
  return FALSE;
}


char *SeekDefExtPoint(char *Name, char *DefExt/*=NULL*/, char **Ext/*=NULL*/)
{
  FSF.Unquote(Name); //$ AA 15.04.2003 для правильной обработки имен в кавычках
  Name=FSF.PointToName(Name);
  char *TempExt=strrchr(Name, '.');
  if(!DefExt)
    return TempExt;
  if(Ext)
    *Ext=TempExt;
  return (TempExt!=NULL)?(FSF.LStricmp(TempExt+1, DefExt)?NULL:TempExt):NULL;
}

BOOL AddExt(char *Name, char *Ext)
{
  char *ExtPnt = {};
  FSF.Unquote(Name); //$ AA 15.04.2003 для правильной обработки имен в кавычках
  if(Name && *Name && !SeekDefExtPoint(Name, Ext, &ExtPnt))
  {
    // transform Ext
    char NewExt[NM], *Ptr;
    lstrcpyn(NewExt,Ext,sizeof(NewExt));

    int Up=0, Lw=0;
    Ptr=FSF.PointToName(Name); // yjh 11.04.2020
    while(*Ptr)
    {
      if(FSF.LIsAlpha(*Ptr))
      {
        if(FSF.LIsLower(*Ptr)) Lw++;
        if(FSF.LIsUpper(*Ptr)) Up++;
      }
      ++Ptr;
    }

    if (Lw)
      FSF.LStrlwr (NewExt);
    else  if (Up)
      FSF.LStrupr (NewExt);

    if(ExtPnt && !*(ExtPnt+1))
      lstrcpy(ExtPnt+1, NewExt);
    else
      FSF.sprintf(Name+lstrlen(Name), ".%s", NewExt);
    return TRUE;
  }
  return FALSE;
}


#ifdef _NEW_ARC_SORT_
void WritePrivateProfileInt(char *Section, char *Key, int Value, char *Ini)
{
  char Buf32[32];
  wsprintf(Buf32, "%d", Value);
  WritePrivateProfileString(Section, Key, Buf32, Ini);
}
#endif

int WINAPI GetPassword(char *Password,const char *FileName)
{
  char Prompt[2*NM],InPass[512];
  FSF.sprintf(Prompt,GetMsg(MGetPasswordForFile),FileName);
  if(Info.InputBox((const char*)GetMsg(MGetPasswordTitle),
                  (const char*)Prompt,NULL,NULL,
                  InPass,sizeof(InPass),NULL,FIB_PASSWORD|FIB_ENABLEEMPTY))
  {
    lstrcpy(Password,InPass);
    return TRUE;
  }
  return FALSE;
}


// Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
#define EPOCH_BIAS    I64(116444736000000000)

void WINAPI UnixTimeToFileTime( DWORD time, FILETIME * ft )
{
  *(__int64*)ft = EPOCH_BIAS + time * I64(10000000);
}

int GetScrX(void)
{
  CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
  if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ConsoleScreenBufferInfo))
    return ConsoleScreenBufferInfo.dwSize.X;
  return 0;
}



static int PathMayBeAbsolute(const char *Path)
{
  return (Path &&
           (
             (((*Path >= 'a' && *Path <= 'z') || (*Path >= 'A' && *Path <= 'Z')) && Path[1]==':') ||
             (Path[0]=='\\'  && Path[1]=='\\') ||
             (Path[0]=='/'   && Path[1]=='/')
           )
         );
}

/*
  преобразует строку
    "cdrecord-1.6.1/mkisofs-1.12b4/../cdrecord/cd_misc.c"
  в
    "cdrecord-1.6.1/cdrecord/cd_misc.c"
*/
void NormalizePath(const char *lpcSrcName,char *lpDestName)
{
  char *DestName=lpDestName;
  char *Ptr;
  char *SrcName=strdup(lpcSrcName);
  char *oSrcName=SrcName;
  int dist;

  while(*SrcName)
  {
    Ptr=strchr(SrcName,'\\');
    if(!Ptr)
      Ptr=strchr(SrcName,'/');

    if(!Ptr)
      Ptr=SrcName+lstrlen(SrcName);

    dist=(int)(Ptr-SrcName)+1;

    if(dist == 1 && (*SrcName == '\\' || *SrcName == '/'))
    {
      *DestName=*SrcName;
      DestName++;
      SrcName++;
    }
    else if(dist == 2 && *SrcName == '.')
    {
      SrcName++;

      if(*SrcName == 0)
        DestName--;
      else
        SrcName++;
    }
    else if(dist == 3 && *SrcName == '.' && SrcName[1] == '.')
    {
      if(!PathMayBeAbsolute(lpDestName))
      {
        char *ptrCurDestName=lpDestName, *Temp=NULL;

        for ( ; ptrCurDestName < DestName-1; ptrCurDestName++)
        {
           if (*ptrCurDestName == '\\' || *ptrCurDestName == '/')
             Temp = ptrCurDestName;
        }

        if(!Temp)
          Temp=lpDestName;

        DestName=Temp;
      }
      else
      {
         if(SrcName[2] == '\\' || SrcName[2] == '/')
           SrcName++;
      }

      SrcName+=2;
    }
    else
    {
      lstrcpyn(DestName, SrcName, dist);
      dist--;
      DestName += dist;
      SrcName  += dist;
    }

    *DestName=0;
  }

  free(oSrcName);
}
