/*
setattr.cpp

Установка атрибутов файлов

*/

/* Revision: 1.02 20.10.2000 $ */

/*
Modify:
  20.10.2000 SVS
    + Новый атрибут Encripted (NTFS/Win2K)
  14.08.2000 KM
    ! Изменена инициализация диалога с учётом возможностей Mask.
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
static int ESetFileEncryption(char *Name,int State,int FileAttr);
static int SetFileEncryption(char *Name,int State);
static int ReadFileTime(FILETIME *FileTime,char *SrcDate,char *SrcTime);
static int ESetFileTime(char *Name,FILETIME *LastWriteTime,
                         FILETIME *CreationTime,FILETIME *LastAccessTime,
                         int FileAttr);

static void IncludeExcludeAttrib(int FocusPos,struct DialogItem *Item, int FocusPosSet, int FocusPosSkip)
{
  if(FocusPos == FocusPosSet && Item[FocusPosSet].Selected && Item[FocusPosSkip].Selected)
    Item[FocusPosSkip].Selected=0;
  if(FocusPos == FocusPosSkip && Item[FocusPosSkip].Selected && Item[FocusPosSet].Selected)
    Item[FocusPosSet].Selected=0;
}


// Compress & Encripted
static int CEAttrib(char *SelName,int FileAttr,struct DialogItem *Item,int C, int E)
{
  int FocusPos=1;

  if(Item[C].Selected && !(FileAttr & FILE_ATTRIBUTE_COMPRESSED))
  {
    if(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)
      FocusPos=ESetFileEncryption(SelName,0,FileAttr);
    if(FocusPos)
      return ESetFileCompression(SelName,1,FileAttr);
  }
  else if(Item[E].Selected && !(FileAttr & FILE_ATTRIBUTE_ENCRYPTED))
  {
// Этот кусок не нужен, т.к. функция криптования сама умеет разжимать
// сжатые файлы.
//    if(FileAttr & FILE_ATTRIBUTE_COMPRESSED)
//      FocusPos=ESetFileCompression(SelName,0,FileAttr);
//    if(FocusPos)
    return ESetFileEncryption(SelName,1,FileAttr);
  }
  else if(!Item[C].Selected && (FileAttr & FILE_ATTRIBUTE_COMPRESSED))
  {
    return ESetFileCompression(SelName,0,FileAttr);
  }
  else if(!Item[E].Selected && (FileAttr & FILE_ATTRIBUTE_ENCRYPTED))
  {
    return ESetFileEncryption(SelName,0,FileAttr);
  }
  return FALSE;
}

static void EmptyDialog(struct DialogItem *MultAttrDlg)
{
  MultAttrDlg[4].Selected=
  MultAttrDlg[5].Selected=
  MultAttrDlg[6].Selected=
  MultAttrDlg[7].Selected=
  MultAttrDlg[8].Selected=
  MultAttrDlg[9].Selected=
  MultAttrDlg[10].Selected=
  MultAttrDlg[11].Selected=
  MultAttrDlg[12].Selected=
  MultAttrDlg[13].Selected=
  MultAttrDlg[14].Selected=
  MultAttrDlg[15].Selected=0;

  MultAttrDlg[22].Data[0]=
  MultAttrDlg[23].Data[0]=
  MultAttrDlg[25].Data[0]=
  MultAttrDlg[26].Data[0]=
  MultAttrDlg[28].Data[0]=
  MultAttrDlg[29].Data[0]='\0';
}

void ShellSetFileAttributes(Panel *SrcPanel)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  /* $ 14.08.2000 KM
     Перменные перенесены сюда для установления маски в
     Edit controls диалога. Добавлены в диалог маски ввода.
  */
  int DateSeparator=GetDateSeparator();
  int TimeSeparator=GetTimeSeparator();
  char DMask[20],TMask[20];

  sprintf(DMask,"99%c99%c9999",DateSeparator,DateSeparator);
  sprintf(TMask,"99%c99%c99",TimeSeparator,TimeSeparator);

  static struct DialogData AttrDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,41,19,0,0,0,0,(char *)MSetAttrTitle,
  /* 01 */DI_TEXT,-1,2,0,0,0,0,0,0,(char *)MSetAttrFor,
  /* 02 */DI_TEXT,-1,3,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_CHECKBOX,5,5,0,0,1,0,0,0,(char *)MSetAttrRO,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MSetAttrArchive,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MSetAttrHidden,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MSetAttrSystem,
  /* 08 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MSetAttrCompressed,
  /* 09 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MSetAttrEncrypted,
  /* 10 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 11 */DI_TEXT,6,12,0,0,0,0,DIF_BOXCOLOR,0,(char *)MSetAttrFileTime,
  /* 12 */DI_TEXT,21,12,0,0,0,0,0,0,"",
  /* 13 */DI_TEXT,    5,13,0,0,0,0,0,0,(char *)MSetAttrModification,
  /* 14 */DI_FIXEDIT,21,13,30,12,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 15 */DI_FIXEDIT,32,13,39,12,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 16 */DI_TEXT,    5,14,0,0,0,0,0,0,(char *)MSetAttrCreation,
  /* 17 */DI_FIXEDIT,21,14,30,12,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 18 */DI_FIXEDIT,32,14,39,12,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 19 */DI_TEXT,    5,15,0,0,0,0,0,0,(char *)MSetAttrLastAccess,
  /* 20 */DI_FIXEDIT,21,15,30,12,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 21 */DI_FIXEDIT,32,15,39,12,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 22 */DI_BUTTON,21,16,0,0,0,0,0,0,(char *)MSetAttrCurrent,
  /* 23 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 24 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
  /* 25 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(AttrDlgData,AttrDlg);

  static struct DialogData MultAttrDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,41,21,0,0,0,0,(char *)MSetAttrTitle,
  /* 01 */DI_TEXT,-1,2,0,0,0,0,0,0,(char *)MSetAttrChange,
  /* 02 */DI_TEXT,3,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 03 */DI_TEXT,5,4,0,0,0,0,0,0,(char *)MSetAttrSetClear,
  /* 04 */DI_CHECKBOX,5,5,0,0,1,0,0,0,"",
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,"",
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,"",
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,0,0,"",
  /* 08 */DI_CHECKBOX,5,9,0,0,0,0,0,0,"",
  /* 09 */DI_CHECKBOX,5,10,0,0,0,0,0,0,"",
  /* 10 */DI_CHECKBOX,11,5,0,0,0,0,0,0,(char *)MSetAttrRO,
  /* 11 */DI_CHECKBOX,11,6,0,0,0,0,0,0,(char *)MSetAttrArchive,
  /* 12 */DI_CHECKBOX,11,7,0,0,0,0,0,0,(char *)MSetAttrHidden,
  /* 13 */DI_CHECKBOX,11,8,0,0,0,0,0,0,(char *)MSetAttrSystem,
  /* 14 */DI_CHECKBOX,11,9,0,0,0,0,0,0,(char *)MSetAttrCompressed,
  /* 15 */DI_CHECKBOX,11,10,0,0,0,0,0,0,(char *)MSetAttrEncrypted,
  /* 16 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 17 */DI_CHECKBOX,5,12,0,0,0,1,0,0,(char *)MSetAttrSubfolders,
  /* 18 */DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 19 */DI_TEXT,6,14,0,0,0,0,DIF_BOXCOLOR,0,(char *)MSetAttrFileTime,
  /* 20 */DI_TEXT,21,14,0,0,0,0,0,0,"",
  /* 21 */DI_TEXT,5,15,0,0,0,0,0,0,(char *)MSetAttrModification,
  /* 22 */DI_FIXEDIT,21,15,30,13,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 23 */DI_FIXEDIT,32,15,39,13,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 24 */DI_TEXT,5,16,0,0,0,0,0,0,(char *)MSetAttrCreation,
  /* 25 */DI_FIXEDIT,21,16,30,13,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 26 */DI_FIXEDIT,32,16,39,13,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 27 */DI_TEXT,5,17,0,0,0,0,0,0,(char *)MSetAttrLastAccess,
  /* 28 */DI_FIXEDIT,21,17,30,13,0,(int)DMask,DIF_MASKEDIT,0,"",
  /* 29 */DI_FIXEDIT,32,17,39,13,0,(int)TMask,DIF_MASKEDIT,0,"",
  /* 30 */DI_BUTTON,21,18,0,0,0,0,0,0,(char *)MSetAttrCurrent,
  /* 31 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 32 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
  /* 33 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  /* KM $ */
  MakeDialogItems(MultAttrDlgData,MultAttrDlg);

  char DateMask[100];
  DWORD FileSystemFlags;
  int SelCount;

  if ((SelCount=SrcPanel->GetSelCount())==0)
    return;

  strcpy(AttrDlg[14].Data,DateMask);
  strcpy(MultAttrDlg[20].Data,DateMask);

  if (SelCount==1 || SrcPanel->GetMode()!=PLUGIN_PANEL)
  {
    if (GetVolumeInformation(NULL,NULL,0,NULL,NULL,&FileSystemFlags,NULL,0))
    {
      if ((FileSystemFlags & FS_FILE_COMPRESSION)==0)
      {
        AttrDlg[8].Type=DI_TEXT;
        // хотелка - не показывать про компрессию
        //strcat(AttrDlg[8].Data,MSG(MSetAttrNTFSOnly));
        AttrDlg[8].Data[0]=0;

        MultAttrDlg[8].Type=DI_TEXT;
        MultAttrDlg[14].Type=DI_TEXT;
        // хотелка - не показывать про компрессию
        //MultAttrDlg[13].X1+=4;
        //strcat(MultAttrDlg[13].Data,MSG(MSetAttrNTFSOnly));
        MultAttrDlg[14].Data[0]=0;
      }
      if ((FileSystemFlags & FS_FILE_ENCRYPTION)==0)
      {
        AttrDlg[9].Type=DI_TEXT;
        AttrDlg[9].Data[0]=0;

        MultAttrDlg[9].Type=DI_TEXT;
        MultAttrDlg[15].Type=DI_TEXT;
        MultAttrDlg[15].Data[0]=0;
      }
    }
  }
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

    int FocusPos;

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
      AttrDlg[9].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
      {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;
        if ((FindHandle=FindFirstFile(SelName,&FindData))!=INVALID_HANDLE_VALUE)
        {
          FindClose(FindHandle);
          ConvertDate(&FindData.ftLastWriteTime,AttrDlg[14].Data,AttrDlg[15].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&FindData.ftCreationTime,AttrDlg[17].Data,AttrDlg[18].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&FindData.ftLastAccessTime,AttrDlg[20].Data,AttrDlg[21].Data,8,FALSE,FALSE,TRUE);
        }
      }

      char TimeText[6][100];
      strcpy(TimeText[0],AttrDlg[14].Data);
      strcpy(TimeText[1],AttrDlg[15].Data);
      strcpy(TimeText[2],AttrDlg[17].Data);
      strcpy(TimeText[3],AttrDlg[18].Data);
      strcpy(TimeText[4],AttrDlg[20].Data);
      strcpy(TimeText[5],AttrDlg[21].Data);

      Dialog Dlg(AttrDlg,sizeof(AttrDlg)/sizeof(AttrDlg[0]));
      Dlg.SetHelp("FileAttrDlg");
      Dlg.SetPosition(-1,-1,45,21);

      while (1)
      {
        Dlg.Show();
        while (!Dlg.Done())
        {
          Dlg.ReadInput();
          Dlg.ProcessInput();
          FocusPos=Dialog::SendDlgMessage((HANDLE)&Dlg,DM_GETFOCUS,0,0);
          if(FocusPos == 8 || FocusPos == 9)
          {
            IncludeExcludeAttrib(FocusPos,AttrDlg,8,9);
            Dlg.FastShow();
          }
        }
        Dlg.GetDialogObjectsData();
        if (Dlg.GetExitCode()!=22)
          break;
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ConvertDate(&ft,AttrDlg[14].Data,AttrDlg[15].Data,8,FALSE,FALSE,TRUE);
        ConvertDate(&ft,AttrDlg[17].Data,AttrDlg[18].Data,8,FALSE,FALSE,TRUE);
        ConvertDate(&ft,AttrDlg[20].Data,AttrDlg[21].Data,8,FALSE,FALSE,TRUE);
        if (AttrDlg[22].Focus)
        {
          AttrDlg[22].Focus=0;
          AttrDlg[14].Focus=1;
        }
        Dlg.ClearDone();
        Dlg.InitDialogObjects();
      }

      if (Dlg.GetExitCode()!=24)
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

      int TimeChanged=strcmp(TimeText[0],AttrDlg[14].Data)!=0 ||
                      strcmp(TimeText[1],AttrDlg[15].Data)!=0 ||
                      strcmp(TimeText[2],AttrDlg[17].Data)!=0 ||
                      strcmp(TimeText[3],AttrDlg[18].Data)!=0 ||
                      strcmp(TimeText[4],AttrDlg[20].Data)!=0 ||
                      strcmp(TimeText[5],AttrDlg[21].Data)!=0;
      if (TimeChanged)
      {
        FILETIME LastWriteTime,CreationTime,LastAccessTime;
        int SetWriteTime,SetCreationTime,SetLastAccessTime;
        SetWriteTime=ReadFileTime(&LastWriteTime,AttrDlg[14].Data,AttrDlg[15].Data);
        SetCreationTime=ReadFileTime(&CreationTime,AttrDlg[17].Data,AttrDlg[18].Data);
        SetLastAccessTime=ReadFileTime(&LastAccessTime,AttrDlg[20].Data,AttrDlg[21].Data);
        ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                     SetCreationTime ? &CreationTime:NULL,
                     SetLastAccessTime ? &LastAccessTime:NULL,FileAttr);
      }
      CEAttrib(SelName,FileAttr,AttrDlg,8,9);
      ESetFileAttributes(SelName,NewAttr);
    }

    /*** Multi ***/
    else
    {
      int SetAttr=0,ClearAttr=0,Cancel=0;

      EmptyDialog(MultAttrDlg);

      {
        Dialog Dlg(MultAttrDlg,sizeof(MultAttrDlg)/sizeof(MultAttrDlg[0]));
        Dlg.SetHelp("FileAttrDlg");
        Dlg.SetPosition(-1,-1,45,23);

        Dlg.Show();
        while (1)
        {
          int Sel17=MultAttrDlg[17].Selected;
          while (!Dlg.Done())
          {
            Dlg.ReadInput();
            Dlg.ProcessInput();
            FocusPos=Dialog::SendDlgMessage((HANDLE)&Dlg,DM_GETFOCUS,0,0);
            // отработаем взаимоисключения
            if(FocusPos >= 4 && FocusPos <= 15)
            {
              IncludeExcludeAttrib(FocusPos,MultAttrDlg,4,10); // Read only
              IncludeExcludeAttrib(FocusPos,MultAttrDlg,5,11); // Archive
              IncludeExcludeAttrib(FocusPos,MultAttrDlg,6,12); // Hidden
              IncludeExcludeAttrib(FocusPos,MultAttrDlg,7,13); // System

              // Compressed/Encrypted
              if(FocusPos == 8 && MultAttrDlg[8].Selected)
              {
                 MultAttrDlg[9].Selected=0;
                 MultAttrDlg[14].Selected=0;
                 MultAttrDlg[15].Selected=1;
              }
              else if(FocusPos == 9 && MultAttrDlg[9].Selected)
              {
                 MultAttrDlg[8].Selected=0;
                 MultAttrDlg[14].Selected=1;
                 MultAttrDlg[15].Selected=0;
              }
              else if(FocusPos == 14 && MultAttrDlg[14].Selected)
              {
                 MultAttrDlg[8].Selected=0;
                 MultAttrDlg[15].Selected=0;
              }
              else if(FocusPos == 15 && MultAttrDlg[15].Selected)
              {
                 MultAttrDlg[9].Selected=0;
                 MultAttrDlg[14].Selected=0;
              }

              Dlg.FastShow();
            }
            // если снимаем атрибуты для SubFolders
            if(FocusPos == 17 && (FileAttr & FA_DIREC) && Sel17 != MultAttrDlg[17].Selected && SelCount==1)
            {
              EmptyDialog(MultAttrDlg);

              if(!MultAttrDlg[17].Selected)
              {
                HANDLE FindHandle;
                WIN32_FIND_DATA FindData;
                if ((FindHandle=FindFirstFile(SelName,&FindData))!=INVALID_HANDLE_VALUE)
                {
                  FindClose(FindHandle);
                  ConvertDate(&FindData.ftLastWriteTime, MultAttrDlg[22].Data,MultAttrDlg[23].Data,8,FALSE,FALSE,TRUE);
                  ConvertDate(&FindData.ftCreationTime,  MultAttrDlg[25].Data,MultAttrDlg[26].Data,8,FALSE,FALSE,TRUE);
                  ConvertDate(&FindData.ftLastAccessTime,MultAttrDlg[28].Data,MultAttrDlg[29].Data,8,FALSE,FALSE,TRUE);
                }
                MultAttrDlg[4].Selected=(FileAttr & FA_RDONLY)!=0;
                MultAttrDlg[5].Selected=(FileAttr & FA_ARCH)!=0;
                MultAttrDlg[6].Selected=(FileAttr & FA_HIDDEN)!=0;
                MultAttrDlg[7].Selected=(FileAttr & FA_SYSTEM)!=0;
                MultAttrDlg[8].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
                MultAttrDlg[9].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
              }
              Dlg.InitDialogObjects();
              Dlg.Show();
              Sel17=MultAttrDlg[17].Selected;
            }
          }
          Dlg.GetDialogObjectsData();
          if (Dlg.GetExitCode()!=30)
            break;
          FILETIME ft;
          GetSystemTimeAsFileTime(&ft);
          ConvertDate(&ft,MultAttrDlg[22].Data,MultAttrDlg[23].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&ft,MultAttrDlg[25].Data,MultAttrDlg[26].Data,8,FALSE,FALSE,TRUE);
          ConvertDate(&ft,MultAttrDlg[28].Data,MultAttrDlg[29].Data,8,FALSE,FALSE,TRUE);
          if (MultAttrDlg[30].Focus)
          {
            MultAttrDlg[30].Focus=0;
            MultAttrDlg[22].Focus=1;
          }
          Dlg.ClearDone();
          Dlg.InitDialogObjects();
          Dlg.Show();
        }

        if (Dlg.GetExitCode()!=32)
          return;
      }
      CtrlObject->GetAnotherPanel(SrcPanel)->CloseFile();

      FILETIME LastWriteTime,CreationTime,LastAccessTime;
      int SetWriteTime,SetCreationTime,SetLastAccessTime;
      SetWriteTime=ReadFileTime(&LastWriteTime,MultAttrDlg[22].Data,MultAttrDlg[23].Data);
      SetCreationTime=ReadFileTime(&CreationTime,MultAttrDlg[25].Data,MultAttrDlg[26].Data);
      SetLastAccessTime=ReadFileTime(&LastAccessTime,MultAttrDlg[28].Data,MultAttrDlg[29].Data);

      if (MultAttrDlg[4].Selected)
        SetAttr|=FA_RDONLY;
      if (MultAttrDlg[5].Selected)
        SetAttr|=FA_ARCH;
      if (MultAttrDlg[6].Selected)
        SetAttr|=FA_HIDDEN;
      if (MultAttrDlg[7].Selected)
        SetAttr|=FA_SYSTEM;
      if (MultAttrDlg[10].Selected)
        ClearAttr|=FA_RDONLY;
      if (MultAttrDlg[11].Selected)
        ClearAttr|=FA_ARCH;
      if (MultAttrDlg[12].Selected)
        ClearAttr|=FA_HIDDEN;
      if (MultAttrDlg[13].Selected)
        ClearAttr|=FA_SYSTEM;
      Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting));
      SrcPanel->GetSelName(NULL,FileAttr);
      while (SrcPanel->GetSelName(SelName,FileAttr) && !Cancel)
      {
        if (CheckForEsc())
          break;
        if (MultAttrDlg[8].Selected) // -E +C
        {
          if (!ESetFileEncryption(SelName,0,FileAttr))
            break; // неудача дешифровать :-(
          if (!ESetFileCompression(SelName,1,FileAttr))
            break; // неудача сжать :-(
        }
        else if (MultAttrDlg[9].Selected) // +E -C
        {
          // Этот кусок не нужен, т.к. функция криптования сама умеет
          // разжимать сжатые файлы.
          //if (!ESetFileCompression(SelName,0,FileAttr))
          //  break; // неудача разжать :-(
          if (!ESetFileEncryption(SelName,1,FileAttr))
            break; // неудача зашифровать :-(
        }
        else //???
        if (MultAttrDlg[14].Selected) // -C ?E
        {
          if (!ESetFileCompression(SelName,0,FileAttr))
            break; // неудача разжать :-(
        }
        else if (MultAttrDlg[15].Selected) // ?C -E
        {
          if (!ESetFileEncryption(SelName,0,FileAttr))
            break; // неудача разшифровать :-(
        }

        if (!ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                     SetCreationTime ? &CreationTime:NULL,
                     SetLastAccessTime ? &LastAccessTime:NULL,FileAttr))
          break;
        if (!ESetFileAttributes(SelName,(FileAttr|SetAttr)&(~ClearAttr)))
          break;
        if ((FileAttr & FA_DIREC) && MultAttrDlg[17].Selected)
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
            if (MultAttrDlg[8].Selected) // -E +C
            {
              if (!ESetFileEncryption(SelName,0,FindData.dwFileAttributes))
              {
                Cancel=1;
                break; // неудача дешифровать :-(
              }
              if (!ESetFileCompression(SelName,1,FindData.dwFileAttributes))
              {
                Cancel=1;
                break; // неудача сжать :-(
              }
            }
            else if (MultAttrDlg[9].Selected) // +E -C
            {
              // Этот кусок не нужен, т.к. функция криптования сама умеет
              // разжимать сжатые файлы.
              //if (!ESetFileCompression(SelName,0,FileAttr))
              //  break; // неудача разжать :-(
              if (!ESetFileEncryption(SelName,1,FindData.dwFileAttributes))
              {
                Cancel=1;
                break; // неудача зашифровать :-(
              }
            }
            else //???
            if (MultAttrDlg[14].Selected) // -C ?E
            {
              if (!ESetFileCompression(SelName,0,FindData.dwFileAttributes))
              {
                Cancel=1;
                break; // неудача разжать :-(
              }
            }
            else if (MultAttrDlg[15].Selected) // ?C -E
            {
              if (!ESetFileEncryption(SelName,0,FindData.dwFileAttributes))
              {
                Cancel=1;
                break; // неудача разшифровать :-(
              }
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

  int Ret=TRUE;
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributes(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));
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
    {
      Ret=FALSE;
      break;
    }
  }
  // Set ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributes(Name,FileAttr);
  return(Ret);
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

/* $ 20.10.2000 SVS
   Новый атрибут Encripted
*/
static int ESetFileEncryption(char *Name,int State,int FileAttr)
{
  if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
    return(TRUE);

  int Ret=TRUE;

  // Drop ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributes(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));
  while (!SetFileEncryption(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
      break;
    int Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                MSG(MSetAttrEncryptedCannotFor),Name,MSG(MRetry),
                MSG(MSkip),MSG(MCancel));
    if (Code==1 || Code<0)
      break;
    if (Code==2)
    {
      Ret=FALSE;
      break;
    }
  }

  // Set ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributes(Name,FileAttr);
  return(Ret);
}

static int SetFileEncryption(char *Name,int State)
{
  // работает только под Win2000! Если не 2000, то не надо и показывать эту опцию.
  typedef BOOL (WINAPI *PEncryptFileA)(LPCSTR lpFileName);
  typedef BOOL (WINAPI *PDecryptFileA)(LPCSTR lpFileName, DWORD dwReserved);

  PEncryptFileA pEncryptFileA = (PEncryptFileA)GetProcAddress(GetModuleHandle("kernel32"),"EncryptFileA");
  if(!pEncryptFileA)
    pEncryptFileA = (PEncryptFileA)GetProcAddress(GetModuleHandle("ADVAPI32"),"EncryptFileA");

  PDecryptFileA pDecryptFileA = (PDecryptFileA)GetProcAddress(GetModuleHandle("kernel32"),"DecryptFileA");
  if(!pDecryptFileA)
    pDecryptFileA = (PDecryptFileA)GetProcAddress(GetModuleHandle("ADVAPI32"),"DecryptFileA");

  // заодно и проверяется успешность получения адреса API...
  if(State)
     return pEncryptFileA ? (*pEncryptFileA)(Name) : FALSE;
  else
     return pDecryptFileA ? (*pDecryptFileA)(Name, 0) : FALSE;
}
/* SVS $ */

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

