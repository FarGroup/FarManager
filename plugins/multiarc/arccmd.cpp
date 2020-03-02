#include "MultiArc.hpp"
#include "marclng.hpp"
#include <farkeys.hpp>

ArcCommand::ArcCommand(struct PluginPanelItem *PanelItem,int ItemsNumber,
                       const char *FormatString,const char *ArcName,const char *ArcDir,
                       const char *Password,const char *AllFilesMask,int IgnoreErrors,
                       int CommandType,int ASilent,const char *RealArcDir)
{

  Silent=ASilent;
//  CommentFile=INVALID_HANDLE_VALUE; //$ AA 25.11.2001
  *CommentFileName=0; //$ AA 25.11.2001
//  ExecCode=-1;
/* $ 28.11.2000 AS
*/
  ExecCode=(DWORD)-1;

  if (*FormatString==0)
    return;
  //char QPassword[NM+5],QTempPath[NM+5];
  char Command[MAX_COMMAND_LENGTH];
  ArcCommand::PanelItem=PanelItem;
  ArcCommand::ItemsNumber=ItemsNumber;
  lstrcpy(ArcCommand::ArcName,ArcName);
  lstrcpy(ArcCommand::ArcDir,ArcDir);
  lstrcpy(ArcCommand::RealArcDir,RealArcDir ? RealArcDir:"");
  FSF.QuoteSpaceOnly(lstrcpy(ArcCommand::Password,Password));
  lstrcpy(ArcCommand::AllFilesMask,AllFilesMask);
  GetTempPath(sizeof(TempPath),TempPath);
  *PrefixFileName=0;
  *ListFileName=0;
  NameNumber=-1;
  *NextFileName=0;
  do
  {
    PrevFileNameNumber=-1;
    lstrcpy(Command,FormatString);
    if (!ProcessCommand(Command,CommandType,IgnoreErrors,ListFileName))
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
                               char *ListFileName)
{
  MaxAllowedExitCode=0;
  DeleteBraces(Command);

  for (char *CurPtr=Command;*CurPtr;)
  {
    int Length=lstrlen(Command);
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
    if ((Hide==1 && CommandType==0) || CommandType==2)
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
      if(!Silent)
      {
        char ErrMsg[200];
        char NameMsg[NM];
        FSF.sprintf(ErrMsg,(char *)GetMsg(MArcNonZero),ExecCode);
        const char *MsgItems[]={GetMsg(MError),NameMsg,ErrMsg,GetMsg(MOk)};
        FSF.TruncPathStr(lstrcpyn(NameMsg,ArcName,sizeof(NameMsg)),MAX_WIDTH_MESSAGE);
        Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
      }
      return FALSE;
    }
  }
  else
  {
    if(!Silent)
    {
      const char *MsgItems[]={GetMsg(MError),GetMsg(MArcCommandNotFound),GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
    }
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
      lstrcpyn(CheckStr,CurPtr,4);
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
      lstrcpy(TmpStr,EndPtr+1);
      lstrcpy(Command,TmpStr);
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

  lstrcpy(LocalAllFilesMask,AllFilesMask);

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
        lstrcpy(LocalAllFilesMask,"*");
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
  lstrcpy(SaveStr,Command+VarLength);
  switch(Command[2])
  {
    case 'A':
      lstrcpy(Command,ArcName);
      if (AnsiCode)
        OemToChar(Command,Command);
      if (PathOnly)
      {
        char *NamePtr=(char *)FSF.PointToName(Command);
        if (NamePtr!=Command)
          *(NamePtr-1)=0;
        else
          lstrcpy(Command," ");
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
          lstrcpy(Path,ArcName);
          Path[Slash-ArcName]=0;
          ConvertNameToShort(Path,Command);
          lstrcat(Command,Slash);
        }
        if (Dot && strchr(FSF.PointToName(Command),'.')==NULL)
          lstrcat(Command,".");
        if (AnsiCode)
          OemToChar(Command,Command);
        if (PathOnly)
        {
          char *NamePtr=(char *)FSF.PointToName(Command);
          if (NamePtr!=Command)
            *(NamePtr-1)=0;
          else
            lstrcpy(Command," ");
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
      FSF.QuoteSpaceOnly(lstrcpy(QListName,ListFileName));
      lstrcpy(Command,QListName);
      break;
    case 'P':
      lstrcpy(Command,Password);
      break;
    case 'C':
      if(*CommentFileName) //второй раз сюда не лезем
        break;
      {
        *Command=0;

        HANDLE CommentFile;
        //char CommentFileName[MAX_PATH];
        char Buf[512];
        SECURITY_ATTRIBUTES sa;

        sa.nLength=sizeof(sa);
        sa.lpSecurityDescriptor=NULL;
        sa.bInheritHandle=TRUE;

        if(FSF.MkTemp(CommentFileName, "FAR") &&
          (CommentFile=CreateFile(CommentFileName, GENERIC_WRITE,
                       FILE_SHARE_READ|FILE_SHARE_WRITE, &sa, CREATE_ALWAYS,
                       /*FILE_ATTRIBUTE_TEMPORARY|*//*FILE_FLAG_DELETE_ON_CLOSE*/0, NULL))
                       != INVALID_HANDLE_VALUE)
        {
          DWORD Count;
          if(Info.InputBox(GetMsg(MComment), GetMsg(MInputComment), NULL, "", Buf, sizeof(Buf), NULL, 0))
          //??тут можно и заполнить строку комментарием, но надо знать, файловый
          //?? он или архивный. да и имя файла в архиве тоже надо знать...
          {
            WriteFile(CommentFile, Buf, lstrlen(Buf), &Count, NULL);
            lstrcpy(Command, CommentFileName);
            CloseHandle(CommentFile);
          }
          FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        }
      }
      break;
    case 'R':
      lstrcpy(Command,RealArcDir);
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
      lstrcpy(Command,TempPath);
      break;
    case 'f':
    case 'F':
      if (PanelItem!=NULL)
      {
        char CurArcDir[NM];
        lstrcpy(CurArcDir,ArcDir);
        int Length=lstrlen(CurArcDir);
        if (Length>0 && CurArcDir[Length-1]!='\\')
          lstrcat(CurArcDir,"\\");

        char Names[MAX_COMMAND_LENGTH];
        *Names=0;

        if (NameNumber==-1)
          NameNumber=0;

        while (NameNumber<ItemsNumber || Command[2]=='f')
        {
          char Name[NM*2];

          int IncreaseNumber=0,FileAttr;
          if (*NextFileName)
          {
            FSF.sprintf(Name,"%s%s%s",PrefixFileName,CurArcDir,NextFileName);
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

            *PrefixFileName=0;
            char *cFileName=PanelItem[N].FindData.cFileName;

            if(PanelItem[N].UserData && (PanelItem[N].Flags & PPIF_USERDATA))
            {
              struct ArcItemUserData *aud=(struct ArcItemUserData*)PanelItem[N].UserData;
              if(aud->SizeStruct == sizeof(struct ArcItemUserData))
              {
                if(aud->Prefix)
                  lstrcpyn(PrefixFileName,aud->Prefix,sizeof(PrefixFileName));
                if(aud->LinkName)
                  cFileName=aud->LinkName;
              }
            }
            // CHECK for BUGS!!
            if(*cFileName == '\\' || *cFileName == '/')
              FSF.sprintf(Name,"%s%s",PrefixFileName,cFileName+1);
            else
              FSF.sprintf(Name,"%s%s%s",PrefixFileName,CurArcDir,cFileName);
            NormalizePath(Name,Name);
            FileAttr=PanelItem[N].FindData.dwFileAttributes;
            PrevFileNameNumber=N;
          }
          if (AnsiCode)
            OemToChar(Name,Name);
          if (NameOnly)
          {
            char NewName[NM];
            lstrcpy(NewName,FSF.PointToName(Name));
            lstrcpy(Name,NewName);
          }
          if (PathOnly)
          {
            char *NamePtr=(char *)FSF.PointToName(Name);
            if (NamePtr!=Name)
              *(NamePtr-1)=0;
            else
              lstrcpy(Name," ");
          }
          if (*Names==0 || (lstrlen(Names)+lstrlen(Name)<MaxNamesLength && Command[2]!='f'))
          {
            NameNumber+=IncreaseNumber;
            if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
            {
              char FolderMaskName[NM];
              //lstrcpy(LocalAllFilesMask,PrefixFileName);
              FSF.sprintf(FolderMaskName,"%s\\%s",Name,LocalAllFilesMask);
              if (PathOnly)
              {
                lstrcpy(FolderMaskName,Name);
                char *NamePtr=(char *)FSF.PointToName(FolderMaskName);
                if (NamePtr!=FolderMaskName)
                  *(NamePtr-1)=0;
                else
                  lstrcpy(FolderMaskName," ");
              }
              if (FolderMask)
              {
                if (FolderName)
                  lstrcpy(NextFileName,FolderMaskName);
                else
                  lstrcpy(Name,FolderMaskName);
              }
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
              lstrcat(Names," ");
            lstrcat(Names,Name);
          }
          else
            break;
        }
        lstrcpy(Command,Names);
      }
      else
        *Command=0;
      break;
    default:
      return FALSE;
  }
  Length=lstrlen(Command);
  lstrcat(Command,SaveStr);
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

  if (FSF.MkTemp(ListFileName,"FAR")==NULL ||
     (ListFile=CreateFile(ListFileName,GENERIC_WRITE,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                   &sa,CREATE_ALWAYS,
                   FILE_FLAG_SEQUENTIAL_SCAN,NULL)) == INVALID_HANDLE_VALUE)

  {
    if(!Silent)
    {
      char NameMsg[NM];
      const char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),NameMsg,GetMsg(MOk)};
      FSF.TruncPathStr(lstrcpyn(NameMsg,ListFileName,sizeof(NameMsg)),MAX_WIDTH_MESSAGE);
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
    }
/* $ 25.07.2001 AA
    if(ListFile != INVALID_HANDLE_VALUE)
      CloseHandle(ListFile);
25.07.2001 AA $*/
    return FALSE;
  }

  char CurArcDir[NM];
  char Buf[3*NM];

  if(NameOnly)
    *CurArcDir=0;
  else
    lstrcpy( CurArcDir, ArcDir );

  int Length=lstrlen(CurArcDir);
  if (Length>0 && CurArcDir[Length-1]!='\\')
    lstrcat(CurArcDir,"\\");

  if (UseSlash)
    for (int I=0;CurArcDir[I];I++)
      if (CurArcDir[I]=='\\')
//        CurArcDir[I]='//';
        CurArcDir[I]='/';

  for (int I=0;I<ItemsNumber;I++)
  {
    char FileName[NM];
    if (ShortNames && *PanelItem[I].FindData.cAlternateFileName)
      lstrcpy(FileName,PanelItem[I].FindData.cAlternateFileName);
    else
      lstrcpy(FileName,PanelItem[I].FindData.cFileName);
    if (NameOnly)
    {
      char NewName[NM];
      lstrcpy(NewName,FSF.PointToName(FileName));
      lstrcpy(FileName,NewName);
    }
    if (PathOnly)
    {
      char *Ptr=(char*)FSF.PointToName(FileName);
      *Ptr=0;
    }
    int FileAttr=PanelItem[I].FindData.dwFileAttributes;

    *PrefixFileName=0;
    if(PanelItem[I].UserData && (PanelItem[I].Flags & PPIF_USERDATA))
    {
      struct ArcItemUserData *aud=(struct ArcItemUserData*)PanelItem[I].UserData;
      if(aud->SizeStruct == sizeof(struct ArcItemUserData))
      {
        if(aud->Prefix)
          lstrcpyn(PrefixFileName,aud->Prefix,sizeof(PrefixFileName));
        if(aud->LinkName)
           lstrcpyn(FileName,aud->LinkName,sizeof(FileName));
      }
    }

    int Error=FALSE;
    if (((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || FolderName))
    {
      char OutName[NM];
      // CHECK for BUGS!!
      if(*FileName == '\\' || *FileName == '/')
        FSF.sprintf(OutName,"%s%s",PrefixFileName,FileName+1);
      else
        FSF.sprintf(OutName,"%s%s%s",PrefixFileName,CurArcDir,FileName);
      NormalizePath(OutName,OutName);
      if (QuoteName==1)
        FSF.QuoteSpaceOnly(OutName);
      else
        if (QuoteName==2)
          QuoteText(OutName);
      if (AnsiCode)
        OemToChar(OutName,OutName);

      lstrcpy(Buf,OutName);lstrcat(Buf,"\r\n");
      Error=WriteFile(ListFile,Buf,lstrlen(Buf),&WriteSize,NULL) == FALSE;
      //Error=fwrite(Buf,1,lstrlen(Buf),ListFile) != lstrlen(Buf);
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
      lstrcpy(Buf,OutName);lstrcat(Buf,"\r\n");
      Error=WriteFile(ListFile,Buf,lstrlen(Buf),&WriteSize,NULL) == FALSE;
      //Error=fwrite(Buf,1,lstrlen(Buf),ListFile) != lstrlen(Buf);
    }
    if (Error)
    {
      CloseHandle(ListFile);
      DeleteFile(ListFileName);
      if(!Silent)
      {
        const char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
      }
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
    if(!Silent)
    {
      char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCreateListFile),GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
    }
    return FALSE;
  }
*/
  return TRUE;
}

ArcCommand::~ArcCommand() //$ AA 25.11.2001
{
/*  if(CommentFile!=INVALID_HANDLE_VALUE)
    CloseHandle(CommentFile);*/
    DeleteFile(CommentFileName);
}
