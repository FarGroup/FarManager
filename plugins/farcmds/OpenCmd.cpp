#ifndef __OpenFromCommandLine
#define __OpenFromCommandLine

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

#include "syslog.h"

static inline bool vh(HANDLE h)
{
  return h != INVALID_HANDLE_VALUE;
}

static void closeHandle(HANDLE& handle)
{
  if ( vh(handle) )
    CloseHandle(handle);
  handle = INVALID_HANDLE_VALUE;
}

static void killTemp(char *TempFileName)
{
  if ( FileExists(TempFileName) )
  {
    DeleteFile(TempFileName);
    *(PointToName(TempFileName)-1) = 0;
    RemoveDirectory(TempFileName);
  }
}

inline bool isDevice(const char* FileName, const char* dev_begin)
{
    const int len=lstrlen(dev_begin);
    if(LStrnicmp(FileName, dev_begin, len)) return false;
    FileName+=len;
    if(!*FileName) return false;
    while(*FileName>='0' && *FileName<='9') FileName++;
    return !*FileName;
}

static bool validForView(const char *FileName, int viewEmpty, int editNew)
{
  if ( !memcmp(FileName, "\\\\.\\", 4) && // специальная обработка имен
      FarIsAlpha(FileName[4]) &&          // вида: \\.\буква:
      FileName[5]==':' && FileName[6]==0 )
    return true;

  if (isDevice(FileName, "\\\\.\\PhysicalDrive")) return true;
  if (isDevice(FileName, "\\\\.\\cdrom")) return true;

  if ( *FileName && FileExists(FileName) )
  {
    if ( viewEmpty )
      return true;
    HANDLE Handle = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if ( vh(Handle) )
    {
      DWORD size = GetFileSize(Handle, NULL);
      CloseHandle(Handle);
      return size && ( size != 0xFFFFFFFF );
    }
  }
  else if ( editNew )
    return true;
  return false;
}

// нитка параллельного вывода на экран для ":<+"

#define THREADSLEEP  200
#define THREADREDRAW 10

struct TShowOutputStreamData
{
  HANDLE hRead;
  HANDLE hWrite;
  HANDLE hConsole;
};

enum enStream { enStreamOut, enStreamErr, enStreamMAX };
enum enThreadType { enThreadHideOutput, enThreadShowOutput };

struct TThreadData
{
  enThreadType type;
  bool processDone;
  TShowOutputStreamData stream[enStreamMAX];
  char title[80], cmd[1024];
};

static DWORD showPartOfOutput(TShowOutputStreamData *sd)
{
  DWORD Res = 0;
  if ( sd )
  {
    char ReadBuf[4096+1];
    DWORD BytesRead = 0;
    if ( ReadFile(sd->hRead, ReadBuf, sizeof(ReadBuf)-1, &BytesRead, NULL) )
    {
      if ( BytesRead )
      {
        DWORD dummy;
        ReadBuf[BytesRead] = 0;
        if ( vh(sd->hConsole) )
          WriteConsole(sd->hConsole, ReadBuf, BytesRead, &dummy, NULL);
        Res = BytesRead;
      }
    }
  }
  return Res;
}

DWORD WINAPI ThreadWhatUpdateScreen(LPVOID par)
{
  if ( par )
  {
    TThreadData *td = (TThreadData*)par;
    if ( td->type == enThreadShowOutput )
    {
      for ( ; ; )
      {
        if ( td->processDone )
          break;
        Sleep(THREADSLEEP);
        for ( int i = 0 ; i < enStreamMAX ; i++ )
        {
          TShowOutputStreamData *sd = &(td->stream[i]);
          if ( vh(sd->hRead) )
            showPartOfOutput(sd);
        }
      }
    }
    else
    {
      for ( ; ; )
      {
        for ( int j = 0 ; j < THREADREDRAW ; j++ )
        {
          if ( td->processDone )
            break;
          Sleep(THREADSLEEP);
        }
        if ( td->processDone )
          break;
        char buff[80];
        DWORD sCheck[enStreamMAX];
        for ( int i = 0 ; i < enStreamMAX ; i++ )
        {
          HANDLE hCheck = td->stream[i].hWrite;
          if ( vh(hCheck) )
          {
            sCheck[i] = GetFileSize(hCheck, NULL);
            if ( sCheck[i] == 0xFFFFFFFF )
              sCheck[i] = 0;
          }
          else
            sCheck[i] = 0;
        }
        if ( sCheck[enStreamOut] )
          if ( sCheck[enStreamErr] )
            FarSprintf(buff, "%lu/%lu", sCheck[enStreamOut], sCheck[enStreamErr]);
          else
            FarSprintf(buff, "%lu", sCheck[enStreamOut]);
        else
          if ( sCheck[enStreamErr] )
            FarSprintf(buff, "%lu", sCheck[enStreamErr]);
          else
            *buff = 0;
        const char *MsgItems[] = { td->title, td->cmd, buff };
        Info.Message(Info.ModuleNumber, 0, NULL, MsgItems, sizeof(MsgItems)/sizeof(*MsgItems), 0);
      }
    }
  }
  return 0;
}

static bool MakeTempNames(char* tempFileName1, char* tempFileName2)
{
  char *tmpPrefix="FCP";
  if ( MkTemp(tempFileName1, tmpPrefix) )
  {
    DeleteFile(tempFileName1);
    if ( CreateDirectory(tempFileName1, NULL) )
    {
      bool ok = true;
      if ( GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd) )
        lstrcpy(tempFileName2, fullcmd);
      else
        ok = false;
      if ( ok && GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd) )
        lstrcpy(tempFileName1, fullcmd);
      else
        ok = false;
      if ( ok )
        return true;
      RemoveDirectory(tempFileName1);
    }
  }
  return false;
}

static char *loadFile(const char *fn, char *buff, DWORD maxSize)
{
  char *p = NULL, FileName[NM*5];
  lstrcpy(FileName, fn);
  ExpandEnvironmentStr(FileName, FileName, sizeof(FileName));
  HANDLE Handle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if ( vh(Handle) )
  {
    DWORD size = GetFileSize(Handle, NULL);
    if ( size >= maxSize )
      size = maxSize-1;
    if ( size )
    {
      bool dyn = false;
      DWORD read;
      if ( buff == NULL )
      {
        buff = (char *)malloc(size+1);
        dyn = buff != NULL;
      }
      if ( buff )
      {
        if ( ReadFile(Handle, buff, size, &read, NULL) && read )
        {
          buff[read] = 0;
          p = buff;
        }
        else if ( dyn )
          free(buff);
      }
    }
    CloseHandle(Handle);
  }
  return p;
}

static void ParseCmdSyntax(char*& pCmd, int& ShowCmdOutput, int& stream)
{

  switch ( *pCmd )
  {
    case '*': stream = 0; ++pCmd; break;
    case '1': stream = 1; ++pCmd; break;
    case '2': stream = 2; ++pCmd; break;
    case '?': stream = 3; ++pCmd; break;
  }

  bool flg_stream = false;

  switch ( *pCmd )
  {
    case '>': ShowCmdOutput = 0; flg_stream = true; ++pCmd; break;
    case '<': ShowCmdOutput = 1; flg_stream = true; ++pCmd; break;
    case '+': ShowCmdOutput = 2; flg_stream = true; ++pCmd; break;
    case ' ': flg_stream = true; break;
    case '|': flg_stream = true; break;
    case '"': flg_stream = true; break;
  }

 if ( (!flg_stream) && (stream == 1 || stream == 2) ){  --pCmd; }

  FarLTrim(pCmd);
}

int OpenFromCommandLine(char *_farcmd)
{
  if ( !_farcmd ) return FALSE;

  static char farcmdbuf[NM*10], *farcmd;
  farcmd=farcmdbuf;
  lstrcpy(farcmdbuf, _farcmd);
  FarRTrim(farcmdbuf);

  BOOL showhelp=TRUE;
  if(lstrlen(farcmd) > 3)
  {
    HANDLE FileHandleOut, FileHandleErr;
    int StartLine=-1, StartChar=-1;
    int ShowCmdOutput=Opt.ShowCmdOutput;
    int stream=Opt.CatchMode;
    BOOL outputtofile=0, allOK=TRUE, View,Edit,Goto,Far,Clip,WhereIs,Macro,Link,Run;
    char *Ptr, *pCmd=NULL, Pref;

    Ptr=strchr(farcmd,':'); ++Ptr;

    Pref=(char)((*farcmd)&(~0x20));

    // far:<command>[<options>]<separator><object>
    Far =(Pref == 'F');
    if(Far)
    {
      farcmd=Ptr;
      Pref=(char)((*farcmd)&(~0x20));
      // farcmd = <command>[<options>]<separator><object>
      // farcmd = <command><separator><object>
      // Pref   = V|E|G|C|W|M|L|R
    }

    // view:[<separator>]<object>
    // view<separator><object>
    View=(Pref == 'V');
    // clip:[<separator>]<object>
    // clip:<separator><object>
    Clip=(Pref == 'C');
    // whereis:[<separator>]<object>
    // whereis:<separator><object>
    WhereIs=(Pref == 'W');
    // edit:[[<options>]<separator>]<object>
    // edit[<options>]<separator><object>
    Edit=(Pref == 'E');
    // goto:[<separator>]<object>
    // goto<separator><object>
    Goto=(Pref == 'G');
    // macro:[<separator>]<object>
    // macro<separator><object>
    Macro=(Pref == 'M');
    // link:[<separator>][<op>]<separator><source><separator><dest>
    // link<separator>[<op>]<separator><source><separator><dest>
    Link=(Pref == 'L');
    // run:[<separator>]<file> < <command>
    // run:<separator><file> < <command>
    Run=(Pref == 'R');

    if(!Far)
    {
      farcmd=Ptr;
      // farcmd = [[<options>]<separator>]<object>
      // farcmd = [<separator>]<object>
    }

    if(View||Edit||Goto||Clip||WhereIs||Macro||Link||Run)
    {
      int SeparatorLen=lstrlen(Opt.Separator);
      char *cBracket=NULL, runFile[NM]="";
      BOOL BracketsOk=TRUE;

      if(Edit)
      {
        // edit:['['<options>']'<separator>]<object>
        //  edit['['<options>']'<separator>]<object>
        //      ^---farcmd
        char *oBracket;
        BracketsOk=FALSE;
        oBracket=strchr(farcmd,'[');
        if(oBracket && oBracket<strstr(farcmd,Opt.Separator))
        {
          if((cBracket=strchr(oBracket,']')) != 0 && oBracket < cBracket)
          {
            farcmd=cBracket+1;
            char *comma=strchr(oBracket,',');

            if(comma)
            {
              if(comma > oBracket && comma < cBracket)
                if((StartLine=GetInt(oBracket+1,comma))>-2 &&
                   (StartChar=GetInt(comma+1,cBracket))>-2)
                  BracketsOk=TRUE;
            }
            else
              if((StartLine=GetInt(oBracket+1,cBracket))>-2)
                BracketsOk=TRUE;
          }
        }
        else
          BracketsOk=TRUE;
      }
      else if ( Run ) // пятница, 26 апреля 2002, 13:50:08
      {
        pCmd = strchr(farcmd,'<');
        if ( pCmd )
        {
          *pCmd = 0;
          lstrcpy(runFile, farcmd);
          *pCmd = '<';
          farcmd = pCmd;
          showhelp=FALSE;
        }
      }

      if(Far && BracketsOk && cBracket)
        farcmd=cBracket+1;

      if(*farcmd=='<')
      {
        pCmd=farcmd+1;
        outputtofile=1;
        ParseCmdSyntax(pCmd, ShowCmdOutput, stream);
      }
      else
      {
       /* $ 21.06.2001 SVS
          Это делаем только, если был префикс "far:",
          иначе глотается первое слово.
       */
       if(Far)
         pCmd=strstr(farcmd,Opt.Separator);
       /* SVS $ */
       if(pCmd)
       {
        if(Far) pCmd+=SeparatorLen;
        else
        {
         char *Quote=strchr(farcmd,'\"');
         if(Quote)
         {
          if(Quote<=pCmd) pCmd=farcmd;
          else pCmd+=SeparatorLen;
         }
         else pCmd+=SeparatorLen;
        }
       }
       else
       {
        if(Far)
        {
         if(BracketsOk && cBracket) pCmd=farcmd;
        }
        else pCmd=farcmd;
       }
      }
      if(pCmd)
      {
        FarLTrim(pCmd);
        if(!outputtofile)
        {
          if(*pCmd=='<') outputtofile=1;
          pCmd+=outputtofile;
        }
        if(View||Edit||Clip)
          ParseCmdSyntax(pCmd, ShowCmdOutput, stream);
        if(*pCmd && BracketsOk)
        {
          showhelp=FALSE;

          if(Goto)
          {
            if(outputtofile)
            {
              if ( loadFile(pCmd, selectItem, sizeof(selectItem)) )
              {
                 if(NULL==(Ptr=strchr(selectItem,'\r')))
                   Ptr=strchr(selectItem,'\n');
                 if(Ptr!=NULL) *Ptr=0;
              }
            }
            else lstrcpy(selectItem,pCmd);

            ExpandEnvironmentStr(selectItem,selectItem,sizeof(selectItem));
          }
          else if(WhereIs)
          {
             char *Path = NULL, *pFile, temp[NM*5];
             int Length=GetCurrentDirectory(sizeof(cmd), cmd),
                 PathLength=GetEnvironmentVariable("PATH", Path, 0);
             Unquote(pCmd);
             ExpandEnvironmentStr(pCmd,temp,sizeof(temp));
             if(Length+PathLength)
             {
               if(NULL!=(Path=(char *)malloc(Length+1+PathLength)))
               {
                 FarSprintf(Path,"%s;", cmd);
                 GetEnvironmentVariable("PATH", Path+Length+1, PathLength);
                 SearchPath(Path, temp, NULL, sizeof(selectItem), selectItem, &pFile);
                 CharToOem(selectItem, selectItem);
                 free(Path);
               }
             }

             if(*selectItem==0)
             {
               HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE},hKey;
               char FullKeyName[512];
               size_t I;
               for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
               {
                 FarSprintf(FullKeyName,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s",
                    pCmd);
                 OemToChar(FullKeyName, FullKeyName);
                 if (RegOpenKeyEx(RootFindKey[I], FullKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
                 {
                    DWORD Type, DataSize=sizeof(selectItem);
                    RegQueryValueEx(hKey,"", 0, &Type, (LPBYTE) selectItem,
                                    &DataSize);
                    RegCloseKey(hKey);
                    break;
                 }
               }
             }
          }
          else if(Macro)
          {
            ActlKeyMacro command;
            if(!LStrnicmp(pCmd,"LOAD",lstrlen(pCmd)))
            {
              command.Command=MCMD_LOADALL;
              Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
            }
            else if(!LStrnicmp(pCmd,"SAVE",lstrlen(pCmd)))
            {
              command.Command=MCMD_SAVEALL;
              Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
            }
            /* $ 21.06.2001 SVS
               Новая команда - поместить последовательность клавиш */
            else if(!LStrnicmp(pCmd,"POST",4))
            {
              pCmd+=4;

              command.Command=MCMD_POSTMACROSTRING;

              command.Param.PlainText.SequenceText=(char *)malloc(lstrlen(pCmd)+1);
              if(command.Param.PlainText.SequenceText)
              {
                command.Param.PlainText.Flags=KSFLAGS_DISABLEOUTPUT;
                lstrcpy(command.Param.PlainText.SequenceText,pCmd);
                Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
                free(command.Param.PlainText.SequenceText);
              }
            }
            /* SVS $ */
          }
          else if(Link) //link [/msg] [/n] источник назначение
          {
            DWORD LinkFlags=0;
            char *Arg2=NULL;
            while(*pCmd && (*pCmd == '/' || *pCmd == '-'))
            {
              if(!LStrnicmp(pCmd,"/MSG",4))
              {
                LinkFlags|=FLINK_SHOWERRMSG;
                pCmd=FarTrim(pCmd+4);
              }
              else if(!LStrnicmp(pCmd,"/N",2))
              {
                LinkFlags|=FLINK_DONOTUPDATEPANEL;
                pCmd=FarTrim(pCmd+2);
              }
            }

            if(*pCmd == '"')
            {
              Arg2=strchr(pCmd+1,'"');
              if(Arg2)
              {
                *++Arg2=0;
                Arg2=FarTrim(++Arg2);
                if(*Arg2 == '"')
                {
                  char *Arg3=strchr(Arg2+1,'"');
                  if(Arg3)
                    *++Arg3=0;
                  Unquote(Arg2);
                }
              }
              Unquote(pCmd);
            }
            else
            {
              Arg2=strchr(pCmd+1,' ');
              if(Arg2)
              {

                *Arg2=0;
                Arg2=FarTrim(++Arg2);
                if(*Arg2 == '"')
                {
                  char *Arg3=strchr(Arg2+1,'"');
                  if(Arg3)
                    *++Arg3=0;
                  Unquote(Arg2);
                }
              }
            }

            DWORD FTAttr=GetFileAttributes(pCmd);
            if(FTAttr != 0xFFFFFFFF && Arg2)
            {
              char Disk[16];
              if(pCmd[1] == ':' && ((pCmd[2] == '\\' && pCmd[3] == 0) || pCmd[2] == 0))
              {
                if(pCmd[2] == 0)
                {
                  lstrcpy(Disk,pCmd);
                  lstrcat(Disk,"\\");
                  pCmd=Disk;
                }
                LinkFlags|=FLINK_VOLMOUNT;
              }
              else if(FTAttr&FILE_ATTRIBUTE_DIRECTORY)
                LinkFlags|=FLINK_SYMLINK;
              else
                LinkFlags|=FLINK_HARDLINK;

              MkLink(pCmd,Arg2,LinkFlags);
            }
          }
          else
          {
            char *tempDir = NULL, temp[NM*5];
            char TempFileNameOut[NM*5] = "", TempFileNameErr[NM*5] = "";
            if ( outputtofile )
            {
              if ( *pCmd == '|' )
              {
                char *endTempDir = strchr(pCmd+1, '|');
                if ( endTempDir )
                {
                  *endTempDir = 0;
                  tempDir = pCmd+1;
                  pCmd = endTempDir;
                  FarLTrim(++pCmd);
                }
              }
              lstrcpy(temp,pCmd);
            }
            else
              Unquote(lstrcpy(temp,pCmd));

            ExpandEnvironmentStr(temp,temp,sizeof(temp));

            // разделение потоков
            int catchStdOutput = stream != 2;
            int catchStdError  = stream != 1;
            int catchSeparate  = ( stream == 3 ) && ( View || Edit );
            int catchAllInOne  = 0;
            if ( outputtofile )
            {
              if ( Run )
              {
                if ( *runFile )
                {
                  Unquote(runFile);
                  ExpandEnvironmentStr(runFile,runFile,sizeof(runFile));
                  lstrcpy(TempFileNameOut, lstrcpy(TempFileNameErr, runFile));
                  allOK = TRUE;
                }
              }
              else
                allOK = MakeTempNames(TempFileNameOut, TempFileNameErr);
              if ( allOK )
              {
                allOK = FALSE;
                lstrcat(lstrcpy(cmd,"%COMSPEC% /c "), temp);
                ExpandEnvironmentStr(cmd, fullcmd, sizeof(fullcmd));
                lstrcpy(cmd, temp);
                if ( catchStdOutput && catchStdError )
                {
                  if ( catchSeparate )
                  {
                    if ( ShowCmdOutput == 2 )
                      ShowCmdOutput = 1;
                  }
                  else
                  {
                    catchStdError = 0;
                    catchAllInOne = 1;
                  }
                }
                if ( catchStdOutput )
                {
                  static SECURITY_ATTRIBUTES sa;
                  memset(&sa, 0, sizeof(sa));
                  sa.nLength = sizeof(sa);
                  sa.lpSecurityDescriptor = NULL;
                  sa.bInheritHandle = TRUE;
                  FileHandleOut = CreateFile(TempFileNameOut,GENERIC_WRITE,
                                    FILE_SHARE_READ,&sa,CREATE_ALWAYS,
                                    FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                }
                else
                {
                  FileHandleOut = INVALID_HANDLE_VALUE;
                  killTemp(TempFileNameOut);
                  TempFileNameOut[0] = 0;
                }
                if ( catchStdError )
                {
                  static SECURITY_ATTRIBUTES sa;
                  memset(&sa, 0, sizeof(sa));
                  sa.nLength = sizeof(sa);
                  sa.lpSecurityDescriptor = NULL;
                  sa.bInheritHandle = TRUE;
                  FileHandleErr = CreateFile(TempFileNameErr,GENERIC_WRITE,
                                    FILE_SHARE_READ,&sa,CREATE_ALWAYS,
                                    FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                }
                else
                {
                  FileHandleErr = INVALID_HANDLE_VALUE;
                  killTemp(TempFileNameErr);
                  TempFileNameErr[0] = 0;
                }
                if ( ( !catchStdOutput || vh(FileHandleOut) ) && ( !catchStdError || vh(FileHandleErr) ) )
                {
                  HANDLE hScreen=NULL, StdInput, StdOutput;
                  CONSOLE_SCREEN_BUFFER_INFO csbi;
                  DWORD ConsoleMode;

                  StdInput  = GetStdHandle(STD_INPUT_HANDLE);
                  StdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

                  static STARTUPINFO si;
                  memset(&si,0,sizeof(si));
                  si.cb=sizeof(si);
                  si.dwFlags=STARTF_USESTDHANDLES;
                  si.hStdInput  = StdInput;
                  si.hStdOutput = catchStdOutput ? FileHandleOut : StdOutput;
                  si.hStdError  = catchStdError  ? FileHandleErr : StdOutput;
                  if ( catchAllInOne )
                    si.hStdError = si.hStdOutput;
                  TThreadData *td = NULL;
                  hScreen = Info.SaveScreen(0, 0, -1, -1);
                  if ( ShowCmdOutput )
                  {
                    GetConsoleScreenBufferInfo(StdOutput,&csbi);
                    char Blank[1024];
                    memset(Blank, ' ', csbi.dwSize.X);
                    for ( int Y = 0 ; Y < csbi.dwSize.Y ; Y++ )
                      Info.Text(0, Y, LIGHTGRAY, Blank);
                    Info.Text(0, 0, 0, NULL);
                    COORD C;
                    C.X = 0;
                    C.Y = csbi.dwCursorPosition.Y;
                    SetConsoleCursorPosition(StdOutput,C);
                    GetConsoleMode(StdInput,&ConsoleMode);
                    SetConsoleMode(StdInput,
                      ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                      ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
                  }
                  if ( ShowCmdOutput == 2 )
                  {
                    // данные для нитки параллельного вывода
                    td = (TThreadData*)malloc(sizeof(TThreadData));
                    if ( td )
                    {
                      td->type = enThreadShowOutput;
                      td->processDone = false;
                      for ( int i = 0 ; i < enStreamMAX ; i++ )
                      {
                        TShowOutputStreamData *sd = &(td->stream[i]);
                        sd->hRead = sd->hWrite = sd->hConsole = INVALID_HANDLE_VALUE;
                      }
                      if ( catchStdError )
                      {
                        TShowOutputStreamData *sd = &(td->stream[enStreamErr]);
                        static SECURITY_ATTRIBUTES sa;
                        memset(&sa, 0, sizeof(sa));
                        sa.nLength = sizeof(sa);
                        sa.lpSecurityDescriptor = NULL;
                        sa.bInheritHandle = TRUE;
                        sd->hRead = CreateFile(TempFileNameErr,GENERIC_READ,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,
                          FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                        sd->hWrite   = FileHandleErr;
                        sd->hConsole = StdOutput;
                      }
                      if ( catchStdOutput )
                      {
                        TShowOutputStreamData *sd = &(td->stream[enStreamOut]);
                        static SECURITY_ATTRIBUTES sa;
                        memset(&sa, 0, sizeof(sa));
                        sa.nLength = sizeof(sa);
                        sa.lpSecurityDescriptor = NULL;
                        sa.bInheritHandle = TRUE;
                        sd->hRead = CreateFile(TempFileNameOut,GENERIC_READ,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,
                          FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                        sd->hWrite   = FileHandleOut;
                        sd->hConsole = StdOutput;
                      }
                    }
                  }
                  char SaveDir[NM], workDir[NM];
                  static PROCESS_INFORMATION pi;
                  memset(&pi,0,sizeof(pi));
                  if ( tempDir )
                  {
                    GetCurrentDirectory(sizeof(SaveDir),SaveDir);
                    ExpandEnvironmentStr(tempDir,workDir,sizeof(workDir));
                    SetCurrentDirectory(workDir);
                  }

                  char consoleTitle[256];
                  DWORD tlen = GetConsoleTitle(consoleTitle, 256);
                  SetConsoleTitle(cmd);

                  if(CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
                  {
                    // нитка параллельного вывода
                    HANDLE hThread;
                    DWORD dummy;
                    if ( td )
                    {
                      hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
                      WaitForSingleObject(pi.hProcess, INFINITE);
                      closeHandle(FileHandleOut);
                      closeHandle(FileHandleErr);
                      td->processDone = true;
                      if ( hThread )
                        WaitForSingleObject(hThread, INFINITE);
                      for ( int i = 0 ; i < enStreamMAX ; i++ )
                      {
                        TShowOutputStreamData *sd = &(td->stream[i]);
                        if ( vh(sd->hWrite) && vh(sd->hRead) )
                          while ( showPartOfOutput(sd) )
                            ;
                        closeHandle(sd->hRead);
                      }
                      free(td);
                    }
                    else
                    {
                      if ( ShowCmdOutput == 0 )
                        td = (TThreadData*)malloc(sizeof(TThreadData));
                      if ( td )
                      {
                        td->type = enThreadHideOutput;
                        lstrcpy(td->title, GetMsg(MConfig));
                        lstrcpy(td->cmd, cmd);
                        td->stream[enStreamOut].hWrite = FileHandleOut;
                        td->stream[enStreamErr].hWrite = FileHandleErr;
                        td->processDone = false;
                        hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
                      }
                      WaitForSingleObject(pi.hProcess, INFINITE);
                      closeHandle(FileHandleOut);
                      closeHandle(FileHandleErr);
                      if ( td )
                      {
                        td->processDone = true;
                        if ( hThread )
                          WaitForSingleObject(hThread, INFINITE);
                        free(td);
                      }
                    }
                    closeHandle(pi.hThread);
                    closeHandle(pi.hProcess);
                    closeHandle(hThread);
                    allOK=TRUE;
                  }
                  else
                  {
                    closeHandle(FileHandleOut);
                    closeHandle(FileHandleErr);
                  }
                  if ( tlen )
                    SetConsoleTitle(consoleTitle);

                  if ( tempDir )
                    SetCurrentDirectory(SaveDir);

                  if ( ShowCmdOutput )
                  {
                    SetConsoleMode(StdInput, ConsoleMode);
                    SMALL_RECT src;
                    COORD dest;
                    CHAR_INFO fill;
                    src.Left = 0;
                    src.Top = 2;
                    src.Right = csbi.dwSize.X;
                    src.Bottom = csbi.dwSize.Y;
                    dest.X = dest.Y = 0;
                    fill.Char.AsciiChar = ' ';
                    fill.Attributes = LIGHTGRAY;
                    ScrollConsoleScreenBuffer(StdOutput, &src, NULL, dest, &fill);
                    Info.Control(NULL, FCTL_SETUSERSCREEN, NULL);
                  }
                  Info.RestoreScreen(hScreen);
                }
              }
            }
            else
              lstrcpy(TempFileNameOut, temp);

            if ( allOK )
            {
              if ( View || Edit )
              {
                char titleOut[NM] = "", titleErr[NM] = "";
                if ( catchStdError && ( ( catchStdOutput && catchSeparate ) || !catchStdOutput) )
                  lstrcpy(titleErr, GetMsg(MStdErr));
                if ( catchStdError && catchStdOutput && catchSeparate )
                  lstrcpy(titleOut, GetMsg(MStdOut));
                if ( View )
                {
                  DWORD Flags = VF_NONMODAL|VF_ENABLE_F6|VF_IMMEDIATERETURN;
                  if ( outputtofile )
                    Flags |= VF_DISABLEHISTORY|VF_DELETEONCLOSE;
                  if ( validForView(TempFileNameErr, Opt.ViewZeroFiles, 0) )
                  {
                    FarSprintf(fullcmd, "%s%s", titleErr, cmd);
                    Info.Viewer(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags);
                  }
                  else if(outputtofile)
                    killTemp(TempFileNameErr);
                  if ( validForView(TempFileNameOut, Opt.ViewZeroFiles, 0) )
                  {
                    FarSprintf(fullcmd, "%s%s", titleOut, cmd);
                    Info.Viewer(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags);
                  }
                  else if(outputtofile)
                    killTemp(TempFileNameOut);
                  outputtofile=FALSE;
                }
                else if ( Edit )
                {
                  DWORD Flags=EF_NONMODAL|EF_CREATENEW|EF_ENABLE_F6|EF_IMMEDIATERETURN;
                  if ( outputtofile )
                    Flags |= EF_DISABLEHISTORY|EF_DELETEONCLOSE;
                  if ( validForView(TempFileNameErr, Opt.ViewZeroFiles, Opt.EditNewFiles) )
                  {
                    FarSprintf(fullcmd, "%s%s", titleErr, cmd);
                    Info.Editor(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar);
                  }
                  else if(outputtofile)
                    killTemp(TempFileNameErr);
                  if ( validForView(TempFileNameOut, Opt.ViewZeroFiles, Opt.EditNewFiles) )
                  {
                    FarSprintf(fullcmd, "%s%s", titleOut, cmd);
                    Info.Editor(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar);
                  }
                  else if(outputtofile)
                    killTemp(TempFileNameOut);
                  outputtofile=FALSE;
                }
              }
              else if ( Run )
                outputtofile=FALSE;
              else if ( Clip )
              {
                char *Ptr = loadFile(TempFileNameOut, NULL, 1048576);
                if ( Ptr )
                {
                  CopyToClipboard(Ptr);
                  free(Ptr);
                }
              }
            }

            if ( outputtofile )
            {
              killTemp(TempFileNameOut);
              killTemp(TempFileNameErr);
            }

          }
        } // </if(*pCmd && BracketsOk)>
      } // </if(pCmd)>
    } // </if(View||Edit||Goto)>
  } // </if(lstrlen(farcmd) > 3)>

  if(showhelp)
  {
    Info.ShowHelp(Info.ModuleName,HlfId.CMD,0);
    return TRUE;
  }
  return FALSE;
}
#endif
