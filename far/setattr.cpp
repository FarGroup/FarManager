/*
setattr.cpp

Установка атрибутов файлов

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
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

static int ESetFileAttributes(char *Name,int Attr);
static int ESetFileCompression(char *Name,int State,int FileAttr);
static int SetFileCompression(char *Name,int State);
static int ReadFileTime(FILETIME *FileTime,char *SrcDate,char *SrcTime);
static int ESetFileTime(char *Name,FILETIME *LastWriteTime,
                         FILETIME *CreationTime,FILETIME *LastAccessTime,
                         int FileAttr);

void ShellSetFileAttributes(Panel *SrcPanel)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  static struct DialogData AttrDlgData[]={
    DI_DOUBLEBOX,3,1,41,18,0,0,0,0,(char *)MSetAttrTitle,
    DI_TEXT,-1,2,0,0,0,0,0,0,(char *)MSetAttrFor,
    DI_TEXT,-1,3,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,5,0,0,1,0,0,0,(char *)MSetAttrRO,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MSetAttrArchive,
    DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MSetAttrHidden,
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MSetAttrSystem,
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MSetAttrCompressed,
    DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,6,11,0,0,0,0,DIF_BOXCOLOR,0,(char *)MSetAttrFileTime,
    DI_TEXT,21,11,0,0,0,0,0,0,"",
    DI_TEXT,5,12,0,0,0,0,0,0,(char *)MSetAttrModification,
    DI_FIXEDIT,21,12,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,12,39,12,0,0,0,0,"",
    DI_TEXT,5,13,0,0,0,0,0,0,(char *)MSetAttrCreation,
    DI_FIXEDIT,21,13,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,13,39,12,0,0,0,0,"",
    DI_TEXT,5,14,0,0,0,0,0,0,(char *)MSetAttrLastAccess,
    DI_FIXEDIT,21,14,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,14,39,12,0,0,0,0,"",
    DI_BUTTON,21,15,0,0,0,0,0,0,(char *)MSetAttrCurrent,
    DI_TEXT,3,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
    DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(AttrDlgData,AttrDlg);

  static struct DialogData MultAttrDlgData[]={
    DI_DOUBLEBOX,3,1,41,20,0,0,0,0,(char *)MSetAttrTitle,
    DI_TEXT,-1,2,0,0,0,0,0,0,(char *)MSetAttrChange,
    DI_TEXT,3,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MSetAttrSetClear,
    DI_CHECKBOX,5,5,0,0,1,0,0,0,"",
    DI_CHECKBOX,5,6,0,0,0,0,0,0,"",
    DI_CHECKBOX,5,7,0,0,0,0,0,0,"",
    DI_CHECKBOX,5,8,0,0,0,0,0,0,"",
    DI_CHECKBOX,5,9,0,0,0,0,0,0,"",
    DI_CHECKBOX,11,5,0,0,0,0,0,0,(char *)MSetAttrRO,
    DI_CHECKBOX,11,6,0,0,0,0,0,0,(char *)MSetAttrArchive,
    DI_CHECKBOX,11,7,0,0,0,0,0,0,(char *)MSetAttrHidden,
    DI_CHECKBOX,11,8,0,0,0,0,0,0,(char *)MSetAttrSystem,
    DI_CHECKBOX,11,9,0,0,0,0,0,0,(char *)MSetAttrCompressed,
    DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,11,0,0,0,1,0,0,(char *)MSetAttrSubfolders,
    DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,6,13,0,0,0,0,DIF_BOXCOLOR,0,(char *)MSetAttrFileTime,
    DI_TEXT,21,13,0,0,0,0,0,0,"",
    DI_TEXT,5,14,0,0,0,0,0,0,(char *)MSetAttrModification,
    DI_FIXEDIT,21,14,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,14,39,12,0,0,0,0,"",
    DI_TEXT,5,15,0,0,0,0,0,0,(char *)MSetAttrCreation,
    DI_FIXEDIT,21,15,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,15,39,12,0,0,0,0,"",
    DI_TEXT,5,16,0,0,0,0,0,0,(char *)MSetAttrLastAccess,
    DI_FIXEDIT,21,16,30,12,0,0,0,0,"",
    DI_FIXEDIT,32,16,39,12,0,0,0,0,"",
    DI_BUTTON,21,17,0,0,0,0,0,0,(char *)MSetAttrCurrent,
    DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
    DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(MultAttrDlgData,MultAttrDlg);

  char DateMask[100];
  DWORD FileSystemFlags;
  int SelCount;

  if ((SelCount=SrcPanel->GetSelCount())==0)
    return;

  if (SelCount==1 || SrcPanel->GetMode()!=PLUGIN_PANEL)
    if (GetVolumeInformation(NULL,NULL,0,NULL,NULL,&FileSystemFlags,NULL,0))
      if ((FileSystemFlags & FS_FILE_COMPRESSION)==0)
      {
        AttrDlg[8].Type=DI_TEXT;
        AttrDlg[8].X1+=4;
        strcat(AttrDlg[8].Data,MSG(MSetAttrNTFSOnly));
        MultAttrDlg[8].Type=DI_TEXT;
        MultAttrDlg[13].Type=DI_TEXT;
        MultAttrDlg[13].X1+=4;
        strcat(MultAttrDlg[13].Data,MSG(MSetAttrNTFSOnly));
      }


  int DateSeparator=GetDateSeparator();
  int TimeSeparator=GetTimeSeparator();
  switch(GetDateFormat())
  {
    case 0:
      sprintf(DateMask,MSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
      break;
    case 1:
      sprintf(DateMask,MSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
      break;
    default:
      sprintf(DateMask,MSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
      break;
  }


  {
    char SelName[NM];
    int FileAttr;
    SaveScreen SaveScr;

    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);

    if (SelCount==0 || SelCount==1 && strcmp(SelName,"..")==0)
      return;

    if (SelCount==1 && (FileAttr & FA_DIREC)==0)
    {
      int NewAttr;
      strcpy(AttrDlg[2].Data,SelName);
      TruncStr(AttrDlg[2].Data,30);
      AttrDlg[4].Selected=(FileAttr & FA_RDONLY)!=0;
      AttrDlg[5].Selected=(FileAttr & FA_ARCH)!=0;
      AttrDlg[6].Selected=(FileAttr & FA_HIDDEN)!=0;
      AttrDlg[7].Selected=(FileAttr & FA_SYSTEM)!=0;
      AttrDlg[8].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
      strcpy(AttrDlg[11].Data,DateMask);
      {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;
        if ((FindHandle=FindFirstFile(SelName,&FindData))!=INVALID_HANDLE_VALUE)
        {
          FindClose(FindHandle);
          ConvertDate(&FindData.ftLastWriteTime,AttrDlg[13].Data,AttrDlg[14].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&FindData.ftCreationTime,AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&FindData.ftLastAccessTime,AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE);
        }
      }

      char TimeText[6][100];
      strcpy(TimeText[0],AttrDlg[13].Data);
      strcpy(TimeText[1],AttrDlg[14].Data);
      strcpy(TimeText[2],AttrDlg[16].Data);
      strcpy(TimeText[3],AttrDlg[17].Data);
      strcpy(TimeText[4],AttrDlg[19].Data);
      strcpy(TimeText[5],AttrDlg[20].Data);

      Dialog Dlg(AttrDlg,sizeof(AttrDlg)/sizeof(AttrDlg[0]));
      Dlg.SetHelp("FileAttrDlg");
      Dlg.SetPosition(-1,-1,45,20);

      while (1)
      {
        Dlg.Show();
        while (!Dlg.Done())
        {
          Dlg.ReadInput();
          Dlg.ProcessInput();
        }
        Dlg.GetDialogObjectsData();
        if (Dlg.GetExitCode()!=21)
          break;
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ConvertDate(&ft,AttrDlg[13].Data,AttrDlg[14].Data,8,FALSE,FALSE,TRUE);
        ConvertDate(&ft,AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE);
        ConvertDate(&ft,AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE);
        if (AttrDlg[21].Focus)
        {
          AttrDlg[21].Focus=0;
          AttrDlg[13].Focus=1;
        }
        Dlg.ClearDone();
        Dlg.InitDialogObjects();
      }

      if (Dlg.GetExitCode()!=23)
        return;
      NewAttr=FileAttr & FA_DIREC;
      if (AttrDlg[4].Selected)
        NewAttr|=FA_RDONLY;
      if (AttrDlg[5].Selected)
        NewAttr|=FA_ARCH;
      if (AttrDlg[6].Selected)
        NewAttr|=FA_HIDDEN;
      if (AttrDlg[7].Selected)
        NewAttr|=FA_SYSTEM;


      Dlg.Hide();
      Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting));
      if (AttrDlg[8].Selected != (FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0)
        ESetFileCompression(SelName,AttrDlg[8].Selected,FileAttr);
      int TimeChanged=strcmp(TimeText[0],AttrDlg[13].Data)!=0 ||
                      strcmp(TimeText[1],AttrDlg[14].Data)!=0 ||
                      strcmp(TimeText[2],AttrDlg[16].Data)!=0 ||
                      strcmp(TimeText[3],AttrDlg[17].Data)!=0 ||
                      strcmp(TimeText[4],AttrDlg[19].Data)!=0 ||
                      strcmp(TimeText[5],AttrDlg[20].Data)!=0;
      if (TimeChanged)
      {
        FILETIME LastWriteTime,CreationTime,LastAccessTime;
        int SetWriteTime,SetCreationTime,SetLastAccessTime;
        SetWriteTime=ReadFileTime(&LastWriteTime,AttrDlg[13].Data,AttrDlg[14].Data);
        SetCreationTime=ReadFileTime(&CreationTime,AttrDlg[16].Data,AttrDlg[17].Data);
        SetLastAccessTime=ReadFileTime(&LastAccessTime,AttrDlg[19].Data,AttrDlg[20].Data);
        ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                     SetCreationTime ? &CreationTime:NULL,
                     SetLastAccessTime ? &LastAccessTime:NULL,FileAttr);
      }
      ESetFileAttributes(SelName,NewAttr);

    }
    else
    {
      int SetAttr=0,ClearAttr=0,Cancel=0;

      strcpy(MultAttrDlg[18].Data,DateMask);

      {
        Dialog Dlg(MultAttrDlg,sizeof(MultAttrDlg)/sizeof(MultAttrDlg[0]));
        Dlg.SetHelp("FileAttrDlg");
        Dlg.SetPosition(-1,-1,45,22);

        Dlg.Show();
        while (1)
        {
          while (!Dlg.Done())
          {
            Dlg.ReadInput();
            Dlg.ProcessInput();
          }
          Dlg.GetDialogObjectsData();
          if (Dlg.GetExitCode()!=28)
            break;
          FILETIME ft;
          GetSystemTimeAsFileTime(&ft);
          ConvertDate(&ft,MultAttrDlg[20].Data,MultAttrDlg[21].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&ft,MultAttrDlg[23].Data,MultAttrDlg[24].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&ft,MultAttrDlg[26].Data,MultAttrDlg[27].Data,8,FALSE,FALSE,TRUE);
          if (MultAttrDlg[28].Focus)
          {
            MultAttrDlg[28].Focus=0;
            MultAttrDlg[20].Focus=1;
          }
          Dlg.ClearDone();
          Dlg.InitDialogObjects();
          Dlg.Show();
        }

        if (Dlg.GetExitCode()!=30)
          return;
      }
      CtrlObject->GetAnotherPanel(SrcPanel)->CloseFile();

      FILETIME LastWriteTime,CreationTime,LastAccessTime;
      int SetWriteTime,SetCreationTime,SetLastAccessTime;
      SetWriteTime=ReadFileTime(&LastWriteTime,MultAttrDlg[20].Data,MultAttrDlg[21].Data);
      SetCreationTime=ReadFileTime(&CreationTime,MultAttrDlg[23].Data,MultAttrDlg[24].Data);
      SetLastAccessTime=ReadFileTime(&LastAccessTime,MultAttrDlg[26].Data,MultAttrDlg[27].Data);

      if (MultAttrDlg[4].Selected)
        SetAttr|=FA_RDONLY;
      if (MultAttrDlg[5].Selected)
        SetAttr|=FA_ARCH;
      if (MultAttrDlg[6].Selected)
        SetAttr|=FA_HIDDEN;
      if (MultAttrDlg[7].Selected)
        SetAttr|=FA_SYSTEM;
      if (MultAttrDlg[9].Selected)
        ClearAttr|=FA_RDONLY;
      if (MultAttrDlg[10].Selected)
        ClearAttr|=FA_ARCH;
      if (MultAttrDlg[11].Selected)
        ClearAttr|=FA_HIDDEN;
      if (MultAttrDlg[12].Selected)
        ClearAttr|=FA_SYSTEM;
      Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting));
      SrcPanel->GetSelName(NULL,FileAttr);
      while (SrcPanel->GetSelName(SelName,FileAttr) && !Cancel)
      {
        if (CheckForEsc())
          break;
        if (MultAttrDlg[8].Selected)
          if (!ESetFileCompression(SelName,1,FileAttr))
            break;
        if (MultAttrDlg[13].Selected)
          if (!ESetFileCompression(SelName,0,FileAttr))
            break;
        if (!ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                     SetCreationTime ? &CreationTime:NULL,
                     SetLastAccessTime ? &LastAccessTime:NULL,FileAttr))
          break;
        if (!ESetFileAttributes(SelName,(FileAttr|SetAttr)&(~ClearAttr)))
          break;
        if ((FileAttr & FA_DIREC) && MultAttrDlg[15].Selected)
        {
          char FullName[NM];
          ScanTree ScTree(FALSE);
          WIN32_FIND_DATA FindData;

          ScTree.SetFindPath(SelName,"*.*");
          while (ScTree.GetNextName(&FindData,FullName))
          {
            if (CheckForEsc())
            {
              Cancel=1;
              break;
            }
            if (MultAttrDlg[8].Selected)
              if (!ESetFileCompression(FullName,1,FindData.dwFileAttributes))
              {
                Cancel=1;
                break;
              }
            if (MultAttrDlg[13].Selected)
              if (!ESetFileCompression(FullName,0,FindData.dwFileAttributes))
              {
                Cancel=1;
                break;
              }
            if (!ESetFileTime(FullName,SetWriteTime ? &LastWriteTime:NULL,
                         SetCreationTime ? &CreationTime:NULL,
                         SetLastAccessTime ? &LastAccessTime:NULL,
                         FindData.dwFileAttributes))
            {
              Cancel=1;
              break;
            }
            if (!ESetFileAttributes(FullName,(FindData.dwFileAttributes|SetAttr)&(~ClearAttr)))
            {
              Cancel=1;
              break;
            }
          }
        }
      }
    }
  }

  SrcPanel->SaveSelection();
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  SrcPanel->ClearSelection();
  SrcPanel->Redraw();
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}


int ESetFileAttributes(char *Name,int Attr)
{
  while (!SetFileAttributes(Name,Attr))
  {
    int Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
             MSG(MSetAttrCannotFor),Name,MSG(MRetry),MSG(MSkip),MSG(MCancel));
    if (Code==1 || Code<0)
      break;
    if (Code==2)
      return(FALSE);
  }
  return(TRUE);
}


int ESetFileCompression(char *Name,int State,int FileAttr)
{
  if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
    return(TRUE);
  if (FileAttr & FA_RDONLY)
    SetFileAttributes(Name,FileAttr & ~FA_RDONLY);
  while (!SetFileCompression(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
      break;
    int Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                MSG(MSetAttrCompressedCannotFor),Name,MSG(MRetry),
                MSG(MSkip),MSG(MCancel));
    if (Code==1 || Code<0)
      break;
    if (Code==2)
      return(FALSE);
  }
  return(TRUE);
}


static int SetFileCompression(char *Name,int State)
{
  HANDLE hFile=CreateFile(Name,FILE_READ_DATA|FILE_WRITE_DATA,
                 FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                 FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FALSE);
  USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
  UDWORD Result;
  int RetCode=DeviceIoControl(hFile,FSCTL_SET_COMPRESSION,&NewState,
                              sizeof(NewState),NULL,0,&Result,NULL);
  CloseHandle(hFile);
  return(RetCode);
}


int ReadFileTime(FILETIME *FileTime,char *SrcDate,char *SrcTime)
{
  FILETIME ft;
  SYSTEMTIME st;
  int DateN[3],TimeN[3],DigitCount,I;
  DateN[0]=DateN[1]=DateN[2]=0;
  for (I=0;I<sizeof(DateN)/sizeof(DateN[0]) && *SrcDate;I++)
  {
    DateN[I]=atoi(SrcDate);
    while (isdigit(*SrcDate))
      SrcDate++;
    while (*SrcDate && !isdigit(*SrcDate))
      SrcDate++;
  }
  TimeN[0]=TimeN[1]=TimeN[2]=0;
  for (DigitCount=I=0;I<sizeof(TimeN)/sizeof(TimeN[0]) && *SrcTime;I++)
  {
    TimeN[I]=atoi(SrcTime);
    while (isdigit(*SrcTime))
    {
      SrcTime++;
      DigitCount++;
    }
    while (*SrcTime && !isdigit(*SrcTime))
      SrcTime++;
  }
  if (DigitCount==0)
    return(FALSE);
  switch(GetDateFormat())
  {
    case 0:
      st.wMonth=DateN[0];
      st.wDay=DateN[1];
      st.wYear=DateN[2];;
      break;
    case 1:
      st.wDay=DateN[0];
      st.wMonth=DateN[1];
      st.wYear=DateN[2];;
      break;
    default:
      st.wYear=DateN[0];;
      st.wMonth=DateN[1];
      st.wDay=DateN[2];
      break;
  }
  if (st.wDay==0 || st.wMonth==0)
    return(FALSE);
  st.wHour=TimeN[0];
  st.wMinute=TimeN[1];
  st.wSecond=TimeN[2];
  st.wDayOfWeek=st.wMilliseconds=0;
  if (st.wYear<100)
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;
  SystemTimeToFileTime(&st,&ft);
  LocalFileTimeToFileTime(&ft,FileTime);
  return(TRUE);
}


int ESetFileTime(char *Name,FILETIME *LastWriteTime,FILETIME *CreationTime,
                  FILETIME *LastAccessTime,int FileAttr)
{
  if (LastWriteTime==NULL && CreationTime==NULL && LastAccessTime==NULL ||
      (FileAttr & FA_DIREC) && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    return(TRUE);
  while (1)
  {
    if (FileAttr & FA_RDONLY)
      SetFileAttributes(Name,FileAttr & ~FA_RDONLY);
    HANDLE hFile=CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
    int SetTime;
    if (hFile==INVALID_HANDLE_VALUE)
      SetTime=FALSE;
    else
    {
      SetTime=SetFileTime(hFile,CreationTime,LastAccessTime,LastWriteTime);
      CloseHandle(hFile);
    }
    if (SetTime)
      break;
    int Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                MSG(MSetAttrTimeCannotFor),Name,MSG(MRetry),
                MSG(MSkip),MSG(MCancel));
    if (Code==1 || Code<0)
      break;
    if (Code==2)
      return(FALSE);
  }
  return(TRUE);
}

