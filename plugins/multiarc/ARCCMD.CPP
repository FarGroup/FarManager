/*
  ARCCMD.CPP

*/

/* $Revision: 1.01 13.04.2001 $ */

/*
Modify:
  13.04.2001 DJ
   * вместо QuoteSpace используется FSF.QuoteSpaceOnly
  28.11.2000 AS
   ! правка '//' комбинаций
*/

ArcCommand::ArcCommand(struct PluginPanelItem *PanelItem,int ItemsNumber,
                       char *FormatString,char *ArcName,char *ArcDir,
                       char *Password,char *AllFilesMask,int IgnoreErrors,
                       int CommandType,int Silent,char *RealArcDir,char *Prefix)
{
//  ExecCode=-1;
/* $ 28.11.2000 AS
*/
  ExecCode=(DWORD)-1;
/* AS $*/
  if (*FormatString==0)
    return;
  //char QPassword[NM+5],QTempPath[NM+5];
  char Command[MAX_COMMAND_LENGTH];
  ArcCommand::PanelItem=PanelItem;
  ArcCommand::ItemsNumber=ItemsNumber;
  *ArcCommand::PrefixFileName=0;
  if(Prefix)
    strcpy(ArcCommand::PrefixFileName,Prefix);
  strcpy(ArcCommand::ArcName,ArcName);
  strcpy(ArcCommand::ArcDir,ArcDir);
  strcpy(ArcCommand::RealArcDir,RealArcDir ? RealArcDir:"");
  FSF.QuoteSpaceOnly(strcpy(ArcCommand::Password,Password));
  strcpy(ArcCommand::AllFilesMask,AllFilesMask);
  GetTempPath(sizeof(TempPath),TempPath);
  *ListFileName=0;
  NameNumber=-1;
  *NextFileName=0;
  do
  {
    PrevFileNameNumber=-1;
    strcpy(Command,FormatString);
    if (!ProcessCommand(Command,CommandType,IgnoreErrors,Silent,ListFileName))
      NameNumber=-1;
    if (*ListFileName)
    {
      if ( !Opt.Background )
        DeleteFile(ListFileName);
      *ListFileName=0;
    }
  } while (NameNumber!=-1 && NameNumber<ItemsNumber);
}


int ArcCommand::ProcessCommand(char *Command,int CommandType,int IgnoreErrors,
                               int Silent,char *ListFileName)
{
  MaxAllowedExitCode=0;
  DeleteBraces(Command);
  for (char *CurPtr=Command;*CurPtr;)
  {
    int Length=strlen(Command);
    switch(ReplaceVar(CurPtr,Length))
    {
      case 1:
        CurPtr+=Length;
        break;
      case -1:
        return FALSE;
      default:
        CurPtr++;
        break;
    }
  }
  if (*Command)
  {
    int Hide=Opt.HideOutput;
    if (Hide==1 && CommandType==0 || CommandType==2)
      Hide=0;
    ExecCode=Execute(this,Command,Hide,Silent,!*Password,ListFileName);
    if(ExecCode==RETEXEC_ARCNOTFOUND)
    {
      return FALSE;
    }
    if (ExecCode<=MaxAllowedExitCode)
      ExecCode=0;
    if (!IgnoreErrors && ExecCode!=0)
    {
      char ErrMsg[200];
      FSF.sprintf(ErrMsg,(char*)GetMsg(MArcNonZero),ExecCode);
      const char *MsgItems[]={GetMsg(MError),ArcName,ErrMsg,GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return FALSE;
    }
  }
  else
  {
    const char *MsgItems[]={GetMsg(MError),GetMsg(MArcCommandNotFound),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    return FALSE;
  }
  return TRUE;
}


void ArcCommand::DeleteBraces(char *Command)
{
  char CheckStr[512],*CurPtr,*EndPtr;
  int NonEmptyVar;
  while (1)
  {
    if ((Command=strchr(Command,'{'))==NULL)
      return;
    if ((EndPtr=strchr(Command+1,'}'))==NULL)
      return;
    for (NonEmptyVar=0,CurPtr=Command+1;CurPtr<EndPtr-2;CurPtr++)
    {
      int Length;
      strncpy(CheckStr,CurPtr,3);
      CheckStr[3]=0;
      if (CheckStr[0]=='%' && CheckStr[1]=='%' && strchr("FfLl",CheckStr[2])!=NULL)
      {
        NonEmptyVar=(ItemsNumber>0);
        break;
      }
      Length=0;
      if (ReplaceVar(CheckStr,Length))
        if (Length>0)
        {
          NonEmptyVar=1;
          break;
        }
    }

    if (NonEmptyVar)
    {
      *Command=*EndPtr=' ';
      Command=EndPtr+1;
    }
    else
    {
      char TmpStr[MAX_COMMAND_LENGTH];
      strcpy(TmpStr,EndPtr+1);
      strcpy(Command,TmpStr);
    }
  }
}


int ArcCommand::ReplaceVar(char *Command,int &Length)
{
  char Chr=Command[2]&(~0x20);
  if (Command[0]!='%' || Command[1]!='%' || Chr < 'A' || Chr > 'Z')
    return FALSE;
  char SaveStr[MAX_COMMAND_LENGTH],LocalAllFilesMask[NM];
  int QuoteName=0,UseSlash=FALSE,FolderMask=FALSE,FolderName=FALSE;
  int NameOnly=FALSE,PathOnly=FALSE,AnsiCode=FALSE;
  int MaxNamesLength=127;

  int VarLength=3;

  strcpy(LocalAllFilesMask,AllFilesMask);

  while (1)
  {
    int BreakScan=FALSE;
    Chr=Command[VarLength];
    if (Command[2]=='F' && Chr >= '0' && Chr <= '9')
    {
      MaxNamesLength=FSF.atoi(&Command[VarLength]);
      while (Chr >= '0' && Chr <= '9')
        Chr=Command[++VarLength];
      continue;
    }
    if (Command[2]=='E' && Chr >= '0' && Chr <= '9')
    {
      MaxAllowedExitCode=FSF.atoi(&Command[VarLength]);
      while (Chr >= '0' && Chr <= '9')
        Chr=Command[++VarLength];
      continue;
    }
    switch(Command[VarLength])
    {
      case 'A':
        AnsiCode=TRUE;
        break;
      case 'Q':
        QuoteName=1;
        break;
      case 'q':
        QuoteName=2;
        break;
      case 'S':
        UseSlash=TRUE;
        break;
      case 'M':
        FolderMask=TRUE;
        break;
      case 'N':
        FolderName=TRUE;
        break;
      case 'W':
        NameOnly=TRUE;
        break;
      case 'P':
        PathOnly=TRUE;
        break;
      case '*':
        strcpy(LocalAllFilesMask,"*");
        break;
      default:
        BreakScan=TRUE;
        break;
    }
    if (BreakScan)
      break;
    VarLength++;
  }
  if ((MaxNamesLength-=Length)<=0)
    MaxNamesLength=1;
  if (MaxNamesLength>MAX_COMMAND_LENGTH-512)
    MaxNamesLength=MAX_COMMAND_LENGTH-512;
  if (FolderMask==FALSE && FolderName==FALSE)
    FolderName=TRUE;
  strcpy(SaveStr,Command+VarLength);
  switch(Command[2])
  {
    case 'A':
      strcpy(Command,ArcName);
      if (AnsiCode)
        OemToChar(Command,Command);
      if (PathOnly)
      {
        char *NamePtr=(char *)FSF.PointToName(Command);
        if (NamePtr!=Command)
          *(NamePtr-1)=0;
        else
          strcpy(Command," ");
      }
      FSF.QuoteSpaceOnly(Command);
      break;
    case 'a':
      {
        int Dot=strchr(FSF.PointToName(ArcName),'.')!=NULL;
        ConvertNameToShort(ArcName,Command);
        char *Slash=strrchr(ArcName,'\\');
        if (GetFileAttributes(ArcName)==0xFFFFFFFF && Slash!=NULL && Slash!=ArcName)
        {
          char Path[NM];
          strcpy(Path,ArcName);
          Path[Slash-ArcName]=0;
          ConvertNameToShort(Path,Command);
          strcat(Command,Slash);
        }
        if (Dot && strchr(FSF.PointToName(Command),'.')==NULL)
          strcat(Command,".");
        if (AnsiCode)
          OemToChar(Command,Command);
        if (PathOnly)
        {
          char *NamePtr=(char *)FSF.PointToName(Command);
          if (NamePtr!=Command)
            *(NamePtr-1)=0;
          else
            strcpy(Command," ");
        }
      }
      FSF.QuoteSpaceOnly(Command);
      break;
    case 'D':
      *Command=0;
      break;
    case 'E':
      *Command=0;
      break;
    case 'l':
    case 'L':
      if (!MakeListFile(ListFileName,Command[2]=='l',QuoteName,UseSlash,
                        FolderName,NameOnly,PathOnly,FolderMask,
                        LocalAllFilesMask,AnsiCode))
        return -1;
      char QListName[NM+2];
      FSF.QuoteSpaceOnly(strcpy(QListName,ListFileName));
      strcpy(Command,QListName);
      break;
    case 'P':
      strcpy(Command,Password);
      break;
    case 'R':
      strcpy(Command,RealArcDir);
      if (UseSlash)
      {
        for (int I=0;Command[I];I++)
          if (Command[I]=='\\')
//            Command[I]='//';
/* $ 28.11.2000 AS
*/
            Command[I]='/';
/* AS $*/
      }
      FSF.QuoteSpaceOnly(Command);
      break;
    case 'W':
      strcpy(Command,TempPath);
      break;
    case 'f':
    case 'F':
      if (PanelItem!=NULL)
      {
        char CurArcDir[NM];
        strcpy(CurArcDir,PrefixFileName);
        strcat(CurArcDir,ArcDir);
        int Length=strlen(CurArcDir);
        if (Length>0 && CurArcDir[Length-1]!='\\')
          strcat(CurArcDir,"\\");

        char Names[MAX_COMMAND_LENGTH];
        *Names=0;

        if (NameNumber==-1)
          NameNumber=0;

        while (NameNumber<ItemsNumber || Command[2]=='f')
        {
          char Name[NM];
          int IncreaseNumber=0,FileAttr;
          if (*NextFileName)
          {
            FSF.sprintf(Name,"%s%s",CurArcDir,NextFileName);
            *NextFileName=0;
            FileAttr=0;
          }
          else
          {
            int N;
            if (Command[2]=='f' && PrevFileNameNumber!=-1)
              N=PrevFileNameNumber;
            else
            {
              N=NameNumber;
              IncreaseNumber=1;
            }
            if (N>=ItemsNumber)
              break;

            FSF.sprintf(Name,"%s%s",CurArcDir,PanelItem[N].FindData.cFileName);
            FileAttr=PanelItem[N].FindData.dwFileAttributes;
            PrevFileNameNumber=N;
          }
          if (AnsiCode)
            OemToChar(Name,Name);
          if (NameOnly)
          {
            char NewName[NM];
            strcpy(NewName,FSF.PointToName(Name));
            strcpy(Name,NewName);
          }
          if (PathOnly)
          {
            char *NamePtr=(char *)FSF.PointToName(Name);
            if (NamePtr!=Name)
              *(NamePtr-1)=0;
            else
              strcpy(Name," ");
          }
          if (*Names==0 ||
              strlen(Names)+strlen(Name)<static_cast<size_t>(MaxNamesLength) &&
              Command[2]!='f')
          {
            NameNumber+=IncreaseNumber;
            if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
            {
              char FolderMaskName[NM];
              //strcpy(LocalAllFilesMask,PrefixFileName);
              FSF.sprintf(FolderMaskName,"%s%s\\%s",PrefixFileName,Name,LocalAllFilesMask);
              if (PathOnly)
              {
                strcpy(FolderMaskName,Name);
                char *NamePtr=(char *)FSF.PointToName(FolderMaskName);
                if (NamePtr!=FolderMaskName)
                  *(NamePtr-1)=0;
                else
                  strcpy(FolderMaskName," ");
              }
              if (FolderMask)
                if (FolderName)
                  strcpy(NextFileName,FolderMaskName);
                else
                  strcpy(Name,FolderMaskName);
            }

            if (QuoteName==1)
              FSF.QuoteSpaceOnly(Name);
            else
              if (QuoteName==2)
                QuoteText(Name);
            if (UseSlash)
              for (int I=0;Name[I];I++)
                if (Name[I]=='\\')
//                  Name[I]='//';
/* $ 28.11.2000 AS
*/
                  Name[I]='/';
/* AS $*/


            if (*Names)
              strcat(Names," ");
            strcat(Names,Name);
          }
          else
            break;
        }
        strcpy(Command,Names);
      }
      else
        *Command=0;
      break;
    default:
      return FALSE;
  }
  Length=strlen(Command);
  strcat(Command,SaveStr);
  return TRUE;
}


int ArcCommand::MakeListFile(char *ListFileName,int ShortNames,int QuoteName,
                int UseSlash,int FolderName,int NameOnly,int PathOnly,
                int FolderMask,char *LocalAllFilesMask,int AnsiCode)
{
//  FILE *ListFile;
  HANDLE ListFile;
  DWORD WriteSize;
  SECURITY_ATTRIBUTES sa;

  sa.nLength=sizeof(sa);
  sa.lpSecurityDescriptor=NULL;
  sa.bInheritHandle=TRUE;

  if (FSF.MkTemp(ListFileName,"Far")==NULL ||
     (ListFile=CreateFile(ListFileName,GENERIC_WRITE,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                   &sa,CREATE_ALWAYS,
                   FILE_FLAG_SEQUENTIAL_SCAN,NULL)) == INVALID_HANDLE_VALUE)

  {
    const char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),ListFileName,GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
/* $ 25.07.2001 AA
    if(ListFile != INVALID_HANDLE_VALUE)
      CloseHandle(ListFile);
25.07.2001 AA $*/
    return FALSE;
  }

  char CurArcDir[NM];
  char Buf[3*NM];

/* $ 23.10.2001 AA */
  if(NameOnly)
    *CurArcDir=0;
  else
    strcpy( CurArcDir, ArcDir );
/* 23.10.2001 AA $ */

  int Length=strlen(CurArcDir);
  if (Length>0 && CurArcDir[Length-1]!='\\')
    strcat(CurArcDir,"\\");

  if (UseSlash)
    for (int I=0;CurArcDir[I];I++)
      if (CurArcDir[I]=='\\')
//        CurArcDir[I]='//';
/* $ 28.11.2000 AS
*/
        CurArcDir[I]='/';
/* AS $*/

  for (int I=0;I<ItemsNumber;I++)
  {
    char FileName[NM];
    if (ShortNames && *PanelItem[I].FindData.cAlternateFileName)
      strcpy(FileName,PanelItem[I].FindData.cAlternateFileName);
    else
      strcpy(FileName,PanelItem[I].FindData.cFileName);
    if (NameOnly)
    {
      char NewName[NM];
      strcpy(NewName,FSF.PointToName(FileName));
      strcpy(FileName,NewName);
    }
    if (PathOnly)
    {
      char *Ptr=(char*)FSF.PointToName(FileName);
      *Ptr=0;
    }
    int FileAttr=PanelItem[I].FindData.dwFileAttributes;

    int Error=FALSE;
    if (((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || FolderName))
    {
      char OutName[NM];
      FSF.sprintf(OutName,"%s%s%s",PrefixFileName,CurArcDir,FileName);
      if (QuoteName==1)
        FSF.QuoteSpaceOnly(OutName);
      else
        if (QuoteName==2)
          QuoteText(OutName);
      if (AnsiCode)
        OemToChar(OutName,OutName);

      strcpy(Buf,OutName);strcat(Buf,"\r\n");
      Error=WriteFile(ListFile,Buf,strlen(Buf),&WriteSize,NULL) == FALSE;
      //Error=fwrite(Buf,1,strlen(Buf),ListFile) != strlen(Buf);
    }
    if (!Error && (FileAttr & FILE_ATTRIBUTE_DIRECTORY) && FolderMask)
    {
      char OutName[NM];
      FSF.sprintf(OutName,"%s%s%s%c%s",PrefixFileName,CurArcDir,FileName,UseSlash ? '/':'\\',LocalAllFilesMask);
      if (QuoteName==1)
        FSF.QuoteSpaceOnly(OutName);
      else
        if (QuoteName==2)
          QuoteText(OutName);
      if (AnsiCode)
        OemToChar(OutName,OutName);
      strcpy(Buf,OutName);strcat(Buf,"\r\n");
      Error=WriteFile(ListFile,Buf,strlen(Buf),&WriteSize,NULL) == FALSE;
      //Error=fwrite(Buf,1,strlen(Buf),ListFile) != strlen(Buf);
    }
    if (Error)
    {
      CloseHandle(ListFile);
      DeleteFile(ListFileName);
      const char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return FALSE;
    }
  }

  CloseHandle(ListFile);
/*
  if (!CloseHandle(ListFile))
  {
    // clearerr(ListFile);
    CloseHandle(ListFile);
    DeleteFile(ListFileName);
    char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    return(FALSE);
  }
*/
  return TRUE;
}
