#include "ftpincl.hpp"

extern struct PluginStartupInfo Info;
extern int BusyMode;

char LastReply1[200],LastReply2[200];

Connection::Connection()
{
  memset(ListCache,0,sizeof(ListCache));
  ListCachePos=0;

  SocketStartup=FALSE;
  Output=NULL;
  OutputSize=0;
  OutputPos=0;

  SystemInfoFilled=0;

  *StartReply=0;
  *LastReply1=*LastReply2=0;

  ptabflg=0;
  *reply_string=0;
  abrtflag=ptflag=allbinary=0;
  cin=cout=0;
  portnum=0;
  autologin=0;
  slrflag=0;

  sendport=verbose=connected=fromatty=0;
  proxy=proxflag=sunique=runique=0;
  code=crflag=0;
  *pasv=0;
  altarg=NULL;
  type=0;
  stru=0;
  form=0;
  mode=0;
  *bytename=0;
  bytesize=0;

  *line=0;
  stringbase=NULL;
  *argbuf=0;
  argbase=NULL;
  margc=0;
  cpend=0;
  mflag=0;

  options=0;

  stype[0]="type";
  stype[1]="";
  stype[2]=NULL;
  restart_point=0;
  abrtflag=0;
  ptflag=0;
  sendport=-1;
  TableNum=0;
  LastUsedTableNum=0;
  PassiveMode=0;
  cmd_peer = data_peer = -1;
  brk_flag = 0;

  InitCmdTab();
}


Connection::~Connection()
{
  int LastError=GetLastError();
  ResetOutput();
  CacheReset();
  if (SocketStartup)
  {
    AbortAllRequest(0);
    WSACleanup();
  }
  if (LastError)
    SetLastError(LastError);
}


struct cmd cmdtabdata[] = {
  { "account",  1,  1},
  { "append",  1,  1},
  { "ascii",  1,  1},
  { "binary", 1,  1},
  { "bye",  0,  0},
  { "cd",  1,  1},
  { "cdup", 1,  1},
  { "chmod",  1,  1},
  { "close",  1,  1},
  { "cr",  0,  0},
  { "delete",  1,  1},
  { "dir",  1,  1},
  { "disconnect",  1,  1},
  { "get",  1,  1},
  { "idle",  1,  1},
  { "image", 1,  1},
  { "ls",   1,  1},
  { "mkdir",  1,  1},
  { "modtime", 1,  1},
  { "newer",  1,  1},
  { "nlist",  1,  1},
  { "open",  0,  1},
  { "proxy",  0,  1},
  { "sendport",  0,  0},
  { "put",  1,  1},
  { "pwd",  1,  1},
  { "quit",  0,  0},
  { "quote",  1,  1},
  { "recv",  1,  1},
  { "reget",  1,  1},
  { "rstatus",  1,  1},
  { "rhelp",  1,  1},
  { "rename", 1,  1},
  { "reset",  1,  1},
  { "restart",  1,  1},
  { "rmdir",   1,  1},
  { "runique",  0,  1},
  { "send",  1,  1},
  { "site",  1,  1},
  { "size",  1,  1},
  { "system",  1,  1},
  { "sunique",  0,  1},
  { "tenex",  1,  1},
  { "type",  1,  1},
  { "user",  1,  1},
  { "umask",  1,  1},
  { 0 },
};


void Connection::InitCmdTab()
{
  memcpy(cmdtab,cmdtabdata,sizeof(cmdtabdata));
}


void Connection::ExecCmdTab(struct cmd *c,int argc,char *argv[])
{
  int I;
  for (I=0;I<sizeof(cmdtabdata)/sizeof(cmdtabdata[0]);I++)
    if (c==&cmdtab[I])
    {
      BusyMode=TRUE;
      switch(I)
      {
        case 0:
          account(argc,argv);
          break;
        case 1:
          if (argc>2)
            EncodeLine(argv[2]);
          put(argc,argv);
          break;
        case 2:
          setascii();
          break;
        case 3:
          setbinary();
          break;
        case 4:
          quit();
          break;
        case 5:
          EncodeLine(argv[1]);
          cd(argc,argv);
          break;
        case 6:
          cdup();
          break;
        case 7:
          do_chmod(argc,argv);
          break;
        case 8:
          disconnect();
          break;
        case 9:
          setcr();
          break;
        case 10:
          EncodeLine(argv[1]);
          deleteFile(argc,argv);
          break;
        case 11:
          if (argc>1)
            EncodeLine(argv[1]);
          ls(argc,argv);
          break;
        case 12:
          disconnect();
          break;
        case 13:
          EncodeLine(argv[1]);
          get(argc,argv);
          break;
        case 14:
          idle(argc,argv);
          break;
        case 15:
          setbinary();
          break;
        case 16:
          if (argc>1)
            EncodeLine(argv[1]);
          ls(argc,argv);
          break;
        case 17:
          EncodeLine(argv[1]);
          makedir(argc,argv);
          break;
        case 18:
          modtime(argc,argv);
          break;
        case 19:
          newer(argc,argv);
          break;
        case 20:
          if (argc>1)
            EncodeLine(argv[1]);
          ls(argc,argv);
          break;
        case 21:
          setpeer(argc,argv);
          break;
        case 22:
          doproxy(argc,argv);
          break;
        case 23:
          setport();
          break;
        case 24:
          if (argc>2)
            EncodeLine(argv[2]);
          put(argc,argv);
          break;
        case 25:
          pwd();
          break;
        case 26:
          quit();
          break;
        case 27:
          quote(argc,argv);
          break;
        case 28:
          EncodeLine(argv[1]);
          get(argc,argv);
          break;
        case 29:
          EncodeLine(argv[1]);
          reget(argc,argv);
          break;
        case 30:
          rmtstatus(argc,argv);
          break;
        case 31:
          rmthelp(argc,argv);
          break;
        case 32:
          EncodeLine(argv[1]);
          EncodeLine(argv[2]);
          renamefile(argc,argv);
          break;
        case 33:
          reset();
          break;
        case 34:
          restart(argc,argv);
          break;
        case 35:
          EncodeLine(argv[1]);
          removedir(argc,argv);
          break;
        case 36:
          setrunique();
          break;
        case 37:
          if (argc>2)
            EncodeLine(argv[2]);
          put(argc,argv);
          break;
        case 38:
          site(argc,argv);
          break;
        case 39:
          sizecmd(argc,argv);
          break;
        case 40:
          syst();
          break;
        case 41:
          setsunique();
          break;
        case 42:
          settenex();
          break;
        case 43:
          settype(argc,argv);
          break;
        case 44:
          user(argc,argv);
          break;
        case 45:
          do_umask(argc,argv);
          break;
      }
      BusyMode=FALSE;
      break;
    }
}


void Connection::AddOutput(char *Data,int Size)
{
#ifdef DEBUGLIST
  static int ListRead=FALSE;
  if (!ListRead)
  {
    FILE *ListFile=fopen("list","rt");
    if (ListFile!=NULL)
    {
      Output=(char *)malloc(100000);
      OutputSize=fread(Output,1,100000,ListFile);
      fclose(ListFile);
      ListRead=TRUE;
    }
  }
  return;
#endif
  if (Size==0)
    return;
  char *NewOutput=(char *)realloc(Output,OutputSize+Size+1);
  if (NewOutput==NULL)
    return;
  Output=NewOutput;
  memcpy(Output+OutputSize,Data,Size);
  OutputSize+=Size;
}


void Connection::ResetOutput()
{
  delete Output;
  Output=NULL;
  OutputSize=0;
  OutputPos=0;
}


int Connection::GetOutput(char *Line,int MaxLength)
{
  if (OutputPos==0 && MaxLength>0)
  {
    int UseTable=FALSE;
    CharTableSet TableSet;
    if (TableNum==2)
    {
      int TabNum=Info.CharTable(FCT_DETECT,Output,OutputSize);
      if (TabNum!=-1)
        if (Info.CharTable(TabNum,(char *)&TableSet,sizeof(TableSet))!=-1)
        {
          UseTable=TRUE;
          LastUsedTableNum=TabNum;
        }
    }
    if (TableNum>2)
      if (Info.CharTable(TableNum-3,(char *)&TableSet,sizeof(TableSet))!=-1)
      {
        UseTable=TRUE;
        LastUsedTableNum=TableNum-3;
      }
    switch(TableNum)
    {
      case 0:
        CharToOemBuff(Output,Output,OutputSize);
        break;
      case 1:
        break;
      default:
        if (UseTable)
          for (int I=0;I<OutputSize;I++)
            Output[I]=TableSet.DecodeTable[Output[I]];
        break;
    }
  }
  while (OutputPos<OutputSize && (Output[OutputPos]=='\n' ||
         Output[OutputPos]=='\r'))
    OutputPos++;
  if (OutputPos>=OutputSize)
    return(FALSE);
  int CopyCount=0;
  while (OutputPos<OutputSize && (Output[OutputPos]!=0 && Output[OutputPos]!=10 &&
         Output[OutputPos]!=13) && --MaxLength>0)
  {
    *(Line++)=Output[OutputPos++];
    CopyCount++;
  }
  *Line=0;
  return(CopyCount!=0);
}


int Connection::GetReply(char *Line,int MaxLength)
{
  strncpy(Line,reply_string,MaxLength);
}


void Connection::CacheReset()
{
  for (int I=0;I<sizeof(ListCache)/sizeof(ListCache[0]);I++)
  {
    delete ListCache[I].Listing;
    ListCache[I].Listing=NULL;
    ListCache[I].ListingSize=0;
  }
  ListCachePos=0;
}


int Connection::CacheGet()
{
  for (int I=0;I<sizeof(ListCache)/sizeof(ListCache[0]);I++)
    if (ListCache[I].ListingSize>0 && strcmp(ListCache[I].DirName,CurDir)==0)
    {
      ResetOutput();
      char *NewOutput=(char *)malloc(ListCache[I].ListingSize+1);
      if (NewOutput==NULL)
        return(FALSE);
      Output=NewOutput;
      OutputSize=ListCache[I].ListingSize;
      memcpy(Output,ListCache[I].Listing,OutputSize);
      return(TRUE);
    }
  return(FALSE);
}


void Connection::CacheAdd()
{
  delete ListCache[ListCachePos].Listing;
  ListCache[ListCachePos].ListingSize=0;
  ListCache[ListCachePos].Listing=(char *)malloc(OutputSize+1);
  if (ListCache[ListCachePos].Listing==NULL)
    return;
  ListCache[ListCachePos].ListingSize=OutputSize;
  memcpy(ListCache[ListCachePos].Listing,Output,OutputSize);
  strcpy(ListCache[ListCachePos].DirName,CurDir);
  if (++ListCachePos>=sizeof(ListCache)/sizeof(ListCache[0]))
    ListCachePos=0;
}


int Connection::GetExitCode()
{
  static int FtpErrCodes[]={
    500,501,202,502,503,504,532,452,552,530,421,550,
    451,551,553
  };
  static int WinErrCodes[]={
    ERROR_BAD_COMMAND,ERROR_BAD_COMMAND,
    ERROR_CALL_NOT_IMPLEMENTED,ERROR_CALL_NOT_IMPLEMENTED,
    ERROR_BAD_COMMAND,ERROR_CALL_NOT_IMPLEMENTED,
    ERROR_ACCESS_DENIED,ERROR_DISK_FULL,ERROR_DISK_FULL,
    ERROR_INTERNET_LOGIN_FAILURE,ERROR_INTERNET_CONNECTION_ABORTED,
    ERROR_ACCESS_DENIED
  };

  if (ErrorCode)
  {
    SetLastError(ErrorCode);
    return(FALSE);
  }

  for (int I=0;code!=-1 && I<sizeof(FtpErrCodes)/sizeof(FtpErrCodes[0]);I++)
    if (code==FtpErrCodes[I])
    {
      if (I<sizeof(WinErrCodes)/sizeof(WinErrCodes[0]))
        SetLastError(WinErrCodes[I]);
      code=-1;
      break;
    }

  return(code!=-1);
}


void Connection::ResetLastReply()
{
  *LastReply1=*LastReply2;
}


void Connection::GetLastReply(char *Buf1,char *Buf2)
{
  strcpy(Buf1,LastReply1);
  strcpy(Buf2,LastReply2);
}


void Connection::EncodeLine(char *Line)
{
  switch(TableNum)
  {
    case 0:
      OemToChar(Line,Line);
      break;
    case 1:
      break;
    default:
      {
        CharTableSet TableSet;
        if (Info.CharTable(LastUsedTableNum,(char *)&TableSet,sizeof(TableSet))!=-1)
          for (int I=0;Line[I]!=0;I++)
            Line[I]=TableSet.EncodeTable[Line[I]];
        break;
      }
  }
}
