int __isspace(int Chr)
{
   return Chr == 0x09 || Chr == 0x0A || Chr == 0x0B || Chr == 0x0C || Chr == 0x0D || Chr == 0x20;
}


char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}


/* $ 13.09.2000 tran
   запуск треда для ожидания момента убийства лист файла */
void StartThreadForKillListFile(PROCESS_INFORMATION *pi,char *list)
{
    if ( pi==0 || list==0 || *list==0)
        return;
    KillStruct *ks;
    DWORD dummy;

    ks=(KillStruct*)GlobalAlloc(GPTR,sizeof(KillStruct));
    if ( ks==0 )
        return ;

    ks->hThread=pi->hThread;
    ks->hProcess=pi->hProcess;
    strcpy(ks->ListFileName,list);

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
  int ExitCode;

  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  HANDLE hChildStdoutRd,hChildStdoutWr;
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
      SetStdHandle(STD_OUTPUT_HANDLE,hChildStdoutWr);
      SetStdHandle(STD_ERROR_HANDLE,hChildStdoutWr);

      if (Silent)
      {
        hScreen=Info.SaveScreen(0,0,-1,0);
#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif
	Info.Text(2,0,LIGHTGRAY,GetMsg(MWaitForExternalProgram));
      }
      else
      {
        hScreen=Info.SaveScreen(0,0,-1,-1);
        const char *MsgItems[]={"",GetMsg(MWaitForExternalProgram)};
        Info.Message(Info.ModuleNumber,0,NULL,MsgItems,
                      sizeof(MsgItems)/sizeof(MsgItems[0]),0);
      }
    }
    else
      HideOutput=FALSE;
  }
  else
  {
    GetConsoleScreenBufferInfo(StdOutput,&csbi);

    char Blank[1024];
    FSF.sprintf(Blank,"%*s",csbi.dwSize.X,"");
    for (int Y=0;Y<csbi.dwSize.Y;Y++)
      Info.Text(0,Y,LIGHTGRAY,Blank);
    Info.Text(0,0,0,NULL);

    COORD C;
    C.X=0;
    C.Y=csbi.dwCursorPosition.Y;
    SetConsoleCursorPosition(StdOutput,C);
  }


  DWORD ConsoleMode;
  GetConsoleMode(StdInput,&ConsoleMode);
  SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                 ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);

  char ExpandedCmd[MAX_COMMAND_LENGTH];
  ExpandEnvironmentStrings(CmdStr,ExpandedCmd,sizeof(ExpandedCmd));

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

  ExitCode=CreateProcess(NULL,ExpandedCmd,NULL,NULL,HideOutput,
            Opt.Background?CREATE_NEW_CONSOLE:0,NULL,NULL,&si,&pi);

  if (HideOutput)
  {
    SetStdHandle(STD_OUTPUT_HANDLE,StdOutput);
    SetStdHandle(STD_ERROR_HANDLE,StdError);
    CloseHandle(hChildStdoutWr);
  }

  if (ExitCode)
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
    SMALL_RECT src;
    COORD dest;
    CHAR_INFO fill;
    src.Left=0;
    src.Top=2;
    src.Right=csbi.dwSize.X;
    src.Bottom=csbi.dwSize.Y;
    dest.X=dest.Y=0;
    fill.Char.AsciiChar=' ';
    fill.Attributes=7;
    ScrollConsoleScreenBuffer(StdOutput,&src,NULL,dest,&fill);
    Info.Control(hPlugin,FCTL_SETUSERSCREEN,NULL);
  }
  if (hScreen)
  {
    Info.RestoreScreen(NULL);
    Info.RestoreScreen(hScreen);
  }
  return(ExitCode);
}


char* QuoteSpace(char *Str)
{
  if (strchr(Str,' ')!=NULL)
    return(QuoteText(Str));
  return(Str);
}


char* QuoteText(char *Str)
{
  char *TmpStr=new char[strlen(Str)+3];
  FSF.sprintf(TmpStr,"\"%s\"",Str);
  strcpy(Str,TmpStr);
  delete TmpStr;
  return(Str);
}


void ConvertNameToShort(const char *Src,char *Dest)
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
    PItem->Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    if ((unsigned int)PInit->Data<2000)
      strcpy(PItem->Data,GetMsg((unsigned int)PInit->Data));
    else
      strcpy(PItem->Data,PInit->Data);
  }
}


void InsertCommas(unsigned long Number,char *Dest)
{
  int I;
  FSF.sprintf(Dest,"%u",Number);
  for (I=strlen(Dest)-4;I>=0;I-=3)
    if (Dest[I])
    {
      memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
      Dest[I+1]=',';
    }
}


int ToPercent(long N1,long N2)
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


int IsCaseMixed(const char *Str)
{
  while (*Str && !IsCharAlpha(*Str))
    Str++;
  int Case=IsCharLower(*Str);
  while (*(Str++))
    if (IsCharAlpha(*Str) && IsCharLower(*Str)!=Case)
      return(TRUE);
  return(FALSE);
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
  return(ExitCode);
}


int LocalStrnicmp(const char *Str1,const char *Str2,int Length)
{
  char AnsiStr1[8192],AnsiStr2[8192];
  OemToChar(Str1,AnsiStr1);
  OemToChar(Str2,AnsiStr2);
  AnsiStr1[Length]=AnsiStr2[Length]=0;
  CharLower(AnsiStr1);
  CharLower(AnsiStr2);
  return(strcmp(AnsiStr1,AnsiStr2));
}
