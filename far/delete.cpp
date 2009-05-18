/*
delete.cpp

Удаление файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "lang.hpp"
#include "flink.hpp"
#include "panel.hpp"
#include "chgprior.hpp"
#include "filepanels.hpp"
#include "scantree.hpp"
#include "treelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "fn.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

static void ShellDeleteMsg(const char *Name,int Wipe);
static int AskDeleteReadOnly(const char *Name,DWORD Attr,int Wipe);
static int ShellRemoveFile(const char *Name,const char *ShortName,int Wipe);
static int ERemoveDirectory(const char *Name,const char *ShortName,int Wipe);
static int RemoveToRecycleBin(const char *Name);
static int WipeFile(const char *Name);
static int WipeDirectory(const char *Name);
static void PR_ShellDeleteMsg(void);

static int ReadOnlyDeleteMode,SkipMode,SkipFoldersMode,DeleteAllFolders;
static clock_t DeleteStartTime;

enum {DELETE_SUCCESS,DELETE_YES,DELETE_SKIP,DELETE_CANCEL};

void ShellDelete(Panel *SrcPanel,int Wipe)
{
  ChangePriority ChPriority(Opt.DelThreadPriority);
  TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellDeleteMsg);

  WIN32_FIND_DATA FindData;
  char DeleteFilesMsg[300],SelName[NM],SelShortName[NM],DizName[NM];
  char FullName[2058];
  int SelCount,FileAttr,UpdateDiz;
  int DizPresent;
  int Ret;
  BOOL NeedUpdate=TRUE, NeedSetUpADir=FALSE;
  ConsoleTitle *DeleteTitle;

  int Opt_DeleteToRecycleBin=Opt.DeleteToRecycleBin;

/*& 31.05.2001 OT Запретить перерисовку текущего фрейма*/
  Frame *FrameFromLaunched=FrameManager->GetCurrentFrame();
  FrameFromLaunched->Lock();

  DeleteAllFolders=!Opt.Confirm.DeleteFolder;

  UpdateDiz=(Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS ||
             SrcPanel->IsDizDisplayed() &&
             Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED);

  if ((SelCount=SrcPanel->GetSelCount())==0)
    goto done;

  // Удаление в корзину только для  FIXED-дисков
  {
    char Root[1024];
//    char FSysNameSrc[NM];
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);
    ConvertNameToFull(SelName,Root, sizeof(Root));
    GetPathRoot(Root,Root);
//_SVS(SysLog("Del: SelName='%s' Root='%s'",SelName,Root));
    if(Opt.DeleteToRecycleBin && FAR_GetDriveType(Root) != DRIVE_FIXED)
      Opt.DeleteToRecycleBin=0;
  }

  if (SelCount==1)
  {
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);
    if (TestParentFolderName(SelName) || *SelName==0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
    strcpy(DeleteFilesMsg,SelName);
  }
  else
  /* $ 05.01.2001 SVS
     в зависимости от числа ставим нужное окончание*/
  {
  /* $ 05.01.2001 IS
  Вместо "файлов" пишем нейтральное - "элементов"
  */
    char *Ends;
    char StrItems[16];
    itoa(SelCount,StrItems,10);
    int LenItems=(int)strlen(StrItems);
    if((LenItems >= 2 && StrItems[LenItems-2] == '1') ||
           StrItems[LenItems-1] >= '5' ||
           StrItems[LenItems-1] == '0')
      Ends=MSG(MAskDeleteItemsS);
    else if(StrItems[LenItems-1] == '1')
      Ends=MSG(MAskDeleteItems0);
    else
      Ends=MSG(MAskDeleteItemsA);
    sprintf(DeleteFilesMsg,MSG(MAskDeleteItems),SelCount,Ends);
  /* IS $ */
  }
  /* SVS $ */
  Ret=1;

  /* $ 13.02.2001 SVS
     Обработка "удаления" линков
  */
  if((FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && SelCount==1)
  {
    char JuncName[1024];
    ConvertNameToFull(SelName,JuncName, sizeof(JuncName));
    if(GetReparsePointInfo(JuncName,JuncName,sizeof(JuncName))) // ? SelName ?
    {
      int offset = 0;
      if (!strncmp(JuncName,"\\??\\",4))
        offset = 4;

      //SetMessageHelp("DeleteLink");

			char AskDeleteLink[1024];
			strcpy(AskDeleteLink,MSG(MAskDeleteLink));
			DWORD dwAttr=GetFileAttributes(JuncName);
			if(dwAttr!=INVALID_FILE_ATTRIBUTES)
			{
				strcat(AskDeleteLink," ");
				strcat(AskDeleteLink,dwAttr&FILE_ATTRIBUTE_DIRECTORY?MSG(MAskDeleteLinkFolder):MSG(MAskDeleteLinkFile));
			}

      Ret=Message(0,3,MSG(MDeleteLinkTitle),
                DeleteFilesMsg,
                AskDeleteLink,
                JuncName+offset,
                MSG(MDeleteLinkDelete),MSG(MDeleteLinkUnlink),MSG(MCancel));

      if(Ret == 1)
      {
        ConvertNameToFull(SelName, JuncName, sizeof(JuncName));
        if(Opt.Confirm.Delete)
        {
          ; //  ;-%
        }
        if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,SelName)) != -1) //JuncName?
        {
          DeleteReparsePoint(JuncName);
          ShellUpdatePanels(SrcPanel,NeedSetUpADir);
        }
        goto done;
      }
      if(Ret != 0)
        goto done;
    }
  }
  /* SVS $ */

  if (Ret && (Opt.Confirm.Delete || SelCount>1 || (FileAttr & FA_DIREC)))
  {
    char *DelMsg;
    char *TitleMsg=MSG(Wipe?MDeleteWipeTitle:MDeleteTitle);
    /* $ 05.01.2001 IS
       ! Косметика в сообщениях - разные сообщения в зависимости от того,
         какие и сколько элементов выделено.
    */
    BOOL folder=(FileAttr & FA_DIREC);

    if (SelCount==1)
    {
      if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
      {
        DelMsg=MSG(folder?MAskWipeFolder:MAskWipeFile);
        TitleMsg=MSG(MDeleteWipeTitle);
      }
      else
      {
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=MSG(folder?MAskDeleteRecycleFolder:MAskDeleteRecycleFile);
        else
          DelMsg=MSG(folder?MAskDeleteFolder:MAskDeleteFile);
      }
    }
    else
    {
      if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
        DelMsg=MSG(MAskWipe);
      else
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=MSG(MAskDeleteRecycle);
        else
          DelMsg=MSG(MAskDelete);
    }
    /* IS $ */
    SetMessageHelp("DeleteFile");
    if (Message(0,2,TitleMsg,DelMsg,DeleteFilesMsg,MSG(Wipe?MDeleteWipe:MDelete),MSG(MCancel))!=0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
  }

  if (Opt.Confirm.Delete && SelCount>1)
  {
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    SetMessageHelp("DeleteFile");
    if (Message(MSG_WARNING,2,MSG(Wipe?MWipeFilesTitle:MDeleteFilesTitle),MSG(Wipe?MAskWipe:MAskDelete),
                DeleteFilesMsg,MSG(MDeleteFileAll),MSG(MDeleteFileCancel))!=0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
    ShellDeleteMsg("",Wipe);
  }

  if (UpdateDiz)
    SrcPanel->ReadDiz();

  SrcPanel->GetDizName(DizName);
  DizPresent=(*DizName && GetFileAttributes(DizName)!=INVALID_FILE_ATTRIBUTES);

  DeleteTitle = new ConsoleTitle(MSG(MDeletingTitle));

  if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,SelName)) == -1)
    goto done;

  if (SrcPanel->GetType()==TREE_PANEL)
    FarChDir("\\");

  {
    TaskBar TB;
    int Cancel=0;
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    ShellDeleteMsg("",Wipe);

    ReadOnlyDeleteMode=-1;
    SkipMode=-1;
    SkipFoldersMode=-1;

    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName) && !Cancel)
    {
      if(CheckForEscSilent())
      {
        int AbortOp = ConfirmAbortOp();
        if (AbortOp)
          break;
      }

      int Length=(int)strlen(SelName);
      if (Length==0 || SelName[0]=='\\' && Length<2 ||
          SelName[1]==':' && Length<4)
        continue;
      if (FileAttr & FA_DIREC)
      {
        if (!DeleteAllFolders)
        {
          if (ConvertNameToFull(SelName,FullName, sizeof(FullName)) >= sizeof(FullName))
            goto done;

          if (CheckFolder(FullName) == CHKFLD_NOTEMPTY)
          {
            int MsgCode=0;
            // для symlink`а не нужно подтверждение
            if(!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
            {
              //SetMessageHelp("DeleteFile");
              MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                  MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),FullName,
                    MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
                    MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));
            }
            /* IS $ */
            if (MsgCode<0 || MsgCode==3)
            {
              NeedSetUpADir=FALSE;
              break;
            }
            if (MsgCode==1)
              DeleteAllFolders=1;
            if (MsgCode==2)
              continue;
          }
        }

        bool DirSymLink=(FileAttr&FILE_ATTRIBUTE_DIRECTORY && FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)!=0;
        if (!DirSymLink && (!Opt.DeleteToRecycleBin || Wipe))
        {
          char FullName[NM];
          ScanTree ScTree(TRUE,TRUE,-1,TRUE);
          ScTree.SetFindPath(SelName,"*.*", 0);
          while (ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
          {
            if(CheckForEscSilent())
            {
              int AbortOp = ConfirmAbortOp();
              if (AbortOp)
              {
                Cancel=1;
                break;
              }
            }
            ShellDeleteMsg(FullName,Wipe);
            char ShortName[NM];
            xstrncpy(ShortName,FullName,sizeof(ShortName)-1);
            if (*FindData.cAlternateFileName)
              strcpy(PointToName(ShortName),FindData.cAlternateFileName); //???
            if (FindData.dwFileAttributes & FA_DIREC)
            {
              /* $ 28.11.2000 SVS
                 Обеспечим корректную работу с SymLink
                 (т.н. "Directory Junctions")
              */
              if(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                if (FindData.dwFileAttributes & FA_RDONLY)
                  SetFileAttributes(FullName,FILE_ATTRIBUTE_NORMAL);
                /* $ 19.01.2003 KM
                   Обработка кода возврата из ERemoveDirectory.
                */
                int MsgCode=ERemoveDirectory(FullName,ShortName,Wipe);
                if (MsgCode==DELETE_CANCEL)
                {
                  Cancel=1;
                  break;
                }
                else if (MsgCode==DELETE_SKIP)
                {
                  ScTree.SkipDir();
                  continue;
                }
                TreeList::DelTreeName(FullName);
                if (UpdateDiz)
                  SrcPanel->DeleteDiz(FullName,SelShortName);
                ScTree.SkipDir(); // ??? ЭТО НУЖНО ДЛЯ ТОГО, ЧТОБЫ...
                continue;
                /* KM $ */
              }
              /* SVS $ */
              if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && CheckFolder(FullName) == CHKFLD_NOTEMPTY)
              {
                //SetMessageHelp("DeleteFile");
                int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                      MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),FullName,
                      MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
                      MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));
                if (MsgCode<0 || MsgCode==3)
                {
                  Cancel=1;
                  break;
                }
                if (MsgCode==1)
                  DeleteAllFolders=1;
                if (MsgCode==2)
                {
                  ScTree.SkipDir();
                  continue;
                }
              }

              if (ScTree.IsDirSearchDone())
              {
                if (FindData.dwFileAttributes & FA_RDONLY)
                  SetFileAttributes(FullName,FILE_ATTRIBUTE_NORMAL);
                /* $ 19.01.2003 KM
                   Обработка кода возврата из ERemoveDirectory.
                */
                int MsgCode=ERemoveDirectory(FullName,ShortName,Wipe);
                if (MsgCode==DELETE_CANCEL)
                {
                  Cancel=1;
                  break;
                }
                else if (MsgCode==DELETE_SKIP)
                {
                  //ScTree.SkipDir();
                  continue;
                }
                TreeList::DelTreeName(FullName);
                /* KM $ */
              }
            }
            else
            {
              int AskCode=AskDeleteReadOnly(FullName,FindData.dwFileAttributes,Wipe);
              if (AskCode==DELETE_CANCEL)
              {
                Cancel=1;
                break;
              }
              if (AskCode==DELETE_YES)
                if (ShellRemoveFile(FullName,ShortName,Wipe)==DELETE_CANCEL)
                {
                  Cancel=1;
                  break;
                }
            }
          }
        }

        if (!Cancel)
        {
          ShellDeleteMsg(SelName,Wipe);
          if (FileAttr & FA_RDONLY)
            SetFileAttributes(SelName,FILE_ATTRIBUTE_NORMAL);
          int DeleteCode;
          // нефига здесь выделываться, а надо учесть, что удаление
          // симлинка в корзину чревато потерей оригинала.
          if (DirSymLink || !Opt.DeleteToRecycleBin || Wipe)
          {
            /* $ 19.01.2003 KM
               Обработка кода возврата из ERemoveDirectory.
            */
            DeleteCode=ERemoveDirectory(SelName,SelShortName,Wipe);
            if (DeleteCode==DELETE_CANCEL)
              break;
            else if (DeleteCode==DELETE_SUCCESS)
            {
              TreeList::DelTreeName(SelName);
              if (UpdateDiz)
                SrcPanel->DeleteDiz(SelName,SelShortName);
            }
          }
          else
          {
            DeleteCode=RemoveToRecycleBin(SelName);
            if (!DeleteCode)// && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
            {
              //SetMessageHelp("DeleteFile");
              Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                      MSG(MCannotDeleteFolder),SelName,MSG(MOk));
            }
            else
            {
              TreeList::DelTreeName(SelName);
              if (UpdateDiz)
                SrcPanel->DeleteDiz(SelName,SelShortName);
            }
            /* KM $ */
          }
        }
      }
      else
      {
        ShellDeleteMsg(SelName,Wipe);
        int AskCode=AskDeleteReadOnly(SelName,FileAttr,Wipe);
        if (AskCode==DELETE_CANCEL)
          break;
        if (AskCode==DELETE_YES)
        {
          int DeleteCode=ShellRemoveFile(SelName,SelShortName,Wipe);
          if (DeleteCode==DELETE_SUCCESS && UpdateDiz)
            SrcPanel->DeleteDiz(SelName,SelShortName);
          if (DeleteCode==DELETE_CANCEL)
            break;
        }
      }
    }
  }

  if (UpdateDiz)
    if (DizPresent==(*DizName && GetFileAttributes(DizName)!=INVALID_FILE_ATTRIBUTES))
      SrcPanel->FlushDiz();

  delete DeleteTitle;

done:
  Opt.DeleteToRecycleBin=Opt_DeleteToRecycleBin;
/*& 31.05.2001 OT Разрешить перерисовку фрейма */
  FrameFromLaunched->Unlock();
/* OT &*/
  /* $ 01.10.2001 IS перерисуемся, чтобы не было артефактов */
  if(NeedUpdate)
  {
    ShellUpdatePanels(SrcPanel,NeedSetUpADir);
  }
  /* IS $ */
}

static void PR_ShellDeleteMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  ShellDeleteMsg(static_cast<const char*>(preRedrawItem.Param.Param1),(int)preRedrawItem.Param.Param5);
}

void ShellDeleteMsg(const char *Name,int Wipe)
{
  static int Width=30;
  int WidthTemp;
  char OutFileName[NM];

  if (Name == NULL || *Name == 0 || (static_cast<DWORD>(clock() - DeleteStartTime) > Opt.ShowTimeoutDelFiles))
  {
    if(Name && *Name)
    {
      DeleteStartTime = clock();     // Первый файл рисуется всегда
      WidthTemp=Max((int)strlen(Name),(int)30);
    }
    else
      Width=WidthTemp=30;

    if(WidthTemp > WidthNameForMessage)
      WidthTemp=WidthNameForMessage; // ширина месага - 38%
    if(WidthTemp >= sizeof(OutFileName)-4)
      WidthTemp=sizeof(OutFileName)-5;
    if(Width < WidthTemp)
      Width=WidthTemp;

    if(Name)
    {
      xstrncpy(OutFileName,Name,sizeof(OutFileName)-1);
      TruncPathStr(OutFileName,Width);
      CenterStr(OutFileName,OutFileName,Width+4);

      Message(0,0,MSG(Wipe?MDeleteWipeTitle:MDeleteTitle),MSG(Wipe?MDeletingWiping:MDeleting),OutFileName);
    }
  }

  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=static_cast<void*>(const_cast<char*>(Name));
  preRedrawItem.Param.Param5=(__int64)Wipe;
  PreRedraw.SetParam(preRedrawItem.Param);
}


int AskDeleteReadOnly(const char *Name,DWORD Attr,int Wipe)
{
  int MsgCode;
  if ((Attr & FA_RDONLY)==0)
    return(DELETE_YES);
  if (ReadOnlyDeleteMode!=-1)
    MsgCode=ReadOnlyDeleteMode;
  else
  {
    //SetMessageHelp("DeleteFile");
    MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MWarning),MSG(MDeleteRO),Name,
            MSG(Wipe?MAskWipeRO:MAskDeleteRO),MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
            MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),
            MSG(MDeleteFileCancel));
  }
  switch(MsgCode)
  {
    case 1:
      ReadOnlyDeleteMode=1;
      break;
    case 2:
      return(DELETE_SKIP);
    case 3:
      ReadOnlyDeleteMode=3;
      return(DELETE_SKIP);
    case -1:
    case -2:
    case 4:
      return(DELETE_CANCEL);
  }
  SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
  return(DELETE_YES);
}



int ShellRemoveFile(const char *Name,const char *ShortName,int Wipe)
{
  char FullName[NM*2];
  if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName))
    return DELETE_CANCEL;

  int MsgCode=0;

  while (1)
  {
    if (Wipe)
    {
      if (SkipMode!=-1)
        MsgCode=SkipMode;
      else
      {
        if(GetNumberOfLinks(FullName) > 1)
        {
          /*
                            Файл
                         "имя файла"
                Файл имеет несколько жестких связей.
  Уничтожение файла приведет к обнулению всех ссылающихся на него файлов.
                        Уничтожать файл?
          */
          // SetMessageHelp("DeleteFile");
          MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MError),FullName,
                          MSG(MDeleteHardLink1),MSG(MDeleteHardLink2),MSG(MDeleteHardLink3),
                          MSG(MDeleteFileWipe),MSG(MDeleteFileAll),MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
        }

        switch(MsgCode)
        {
          case -1:
          case -2:
          case 4:
            return DELETE_CANCEL;
          case 2:
            return DELETE_SKIP;
          case 3:
            SkipMode=3;
            return DELETE_SKIP;
          default:
            if (WipeFile(Name) || WipeFile(ShortName))
              return DELETE_SUCCESS;
        }
      }
    }
    else
      if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion<4 || !Opt.DeleteToRecycleBin)
      {
/*
        if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
        {
          HANDLE hDelete=FAR_CreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,
                 FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_POSIX_SEMANTICS,NULL);
          if (hDelete!=INVALID_HANDLE_VALUE && CloseHandle(hDelete))
            break;
        }
*/
        if (FAR_DeleteFile(Name) || FAR_DeleteFile(ShortName))
          break;
      }
      else
        if (RemoveToRecycleBin(Name))
          break;
    /* $ 19.01.2003 KM
       Добавлен Skip all
    */
    if (SkipMode!=-1)
        MsgCode=SkipMode;
    else
    {
      //SetMessageHelp("DeleteFile");
      if(strlen(FullName) > NM-1)
      {
        MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MError),MSG(MErrorFullPathNameLong),
                      MSG(MCannotDeleteFile),Name,MSG(MDeleteRetry),
                      MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
      }
      else
        MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                      MSG(MCannotDeleteFile),Name,MSG(MDeleteRetry),
                      MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
    }
    /* KM $ */
    switch(MsgCode)
    {
      case -1:
      case -2:
      case 3:
        return DELETE_CANCEL;
      case 1:
        return DELETE_SKIP;
      case 2:
        SkipMode=2;
        return DELETE_SKIP;
    }
  }
  return DELETE_SUCCESS;
}


int ERemoveDirectory(const char *Name,const char *ShortName,int Wipe)
{
  char FullName[NM*2];
  if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName))
    return DELETE_CANCEL;

  while (1)
  {
    if (Wipe)
    {
      if (WipeDirectory(Name) || (_localLastError != ERROR_ACCESS_DENIED && WipeDirectory(ShortName)))
        break;
    }
    else
      if (FAR_RemoveDirectory(Name) || (_localLastError != ERROR_ACCESS_DENIED && FAR_RemoveDirectory(ShortName)))
        break;
    /* $ 19.01.2003 KM
       Добавлен Skip и Skip all
    */
    int MsgCode;
    if (SkipFoldersMode!=-1)
        MsgCode=SkipFoldersMode;
    else
    {
      //SetMessageHelp("DeleteFile");
      if(strlen(FullName) > NM-1)
      {
        MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MError),MSG(MErrorFullPathNameLong),
                  MSG(MCannotDeleteFolder),Name,MSG(MDeleteRetry),
                  MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
      }
      else
      {
        MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                    MSG(MCannotDeleteFolder),Name,MSG(MDeleteRetry),
                    MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
      }
    }
    /* KM $ */
    switch(MsgCode)
    {
      case -1:
      case -2:
      case 3:
        return DELETE_CANCEL;
      case 1:
        return DELETE_SKIP;
      case 2:
        SkipFoldersMode=2;
        return DELETE_SKIP;
    }
  }
  return DELETE_SUCCESS;
}

DWORD SHErrorToWinError(DWORD SHError)
{
  DWORD WinError=SHError;
  switch(SHError)
  {
    case 0x71:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_SAMEFILE         The source and destination files are the same file.
    case 0x72:    WinError=ERROR_INVALID_PARAMETER; break; // DE_MANYSRC1DEST     Multiple file paths were specified in the source buffer, but only one destination file path.
    case 0x73:    WinError=ERROR_NOT_SAME_DEVICE;   break; // DE_DIFFDIR          Rename operation was specified but the destination path is a different directory. Use the move operation instead.
    case 0x74:    WinError=ERROR_ACCESS_DENIED;     break; // DE_ROOTDIR          The source is a root directory, which cannot be moved or renamed.
    case 0x75:    WinError=ERROR_CANCELLED;         break; // DE_OPCANCELLED      The operation was cancelled by the user, or silently cancelled if the appropriate flags were supplied to SHFileOperation.
    case 0x76:    WinError=ERROR_BAD_PATHNAME;      break; // DE_DESTSUBTREE      The destination is a subtree of the source.
    case 0x78:    WinError=ERROR_ACCESS_DENIED;     break; // DE_ACCESSDENIEDSRC  Security settings denied access to the source.
    case 0x79:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_PATHTOODEEP      The source or destination path exceeded or would exceed MAX_PATH.
    case 0x7A:    WinError=ERROR_INVALID_PARAMETER; break; // DE_MANYDEST         The operation involved multiple destination paths, which can fail in the case of a move operation.
    case 0x7C:    WinError=ERROR_BAD_PATHNAME;      break; // DE_INVALIDFILES     The path in the source or destination or both was invalid.
    case 0x7D:    WinError=ERROR_INVALID_PARAMETER; break; // DE_DESTSAMETREE     The source and destination have the same parent folder.
    case 0x7E:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_FLDDESTISFILE    The destination path is an existing file.
    case 0x80:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_FILEDESTISFLD    The destination path is an existing folder.
    case 0x81:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_FILENAMETOOLONG  The name of the file exceeds MAX_PATH.
    case 0x82:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_CDROM    The destination is a read-only CD-ROM, possibly unformatted.
    case 0x83:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_DVD      The destination is a read-only DVD, possibly unformatted.
    case 0x84:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_CDRECORD The destination is a writable CD-ROM, possibly unformatted.
    case 0x85:    WinError=ERROR_DISK_FULL;         break; // DE_FILE_TOO_LARGE   The file involved in the operation is too large for the destination media or file system.
    case 0x86:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_CDROM     The source is a read-only CD-ROM, possibly unformatted.
    case 0x87:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_DVD       The source is a read-only DVD, possibly unformatted.
    case 0x88:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_CDRECORD  The source is a writable CD-ROM, possibly unformatted.
    case 0xB7:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_ERROR_MAX        MAX_PATH was exceeded during the operation.
    case 0x402:   WinError=ERROR_PATH_NOT_FOUND;    break; //                     An unknown error occurred. This is typically due to an invalid path in the source or destination. This error does not occur on Windows Vista and later.
    case 0x10000: WinError=ERROR_GEN_FAILURE;       break; // ERRORONDEST         An unspecified error occurred on the destination.
  }
  return WinError;
}

int RemoveToRecycleBin(const char *Name)
{
  SHFILEOPSTRUCT fop;
  char FullName[NM+1];
//  ConvertNameToFull(Name,FullName, sizeof(FullName));
  if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName)){
    return 1;
  }

  // При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
  if(WinVer.dwMajorVersion<6 && Opt.DeleteToRecycleBinKillLink && GetFileAttributes(Name) == FA_DIREC)
  {
    char FullName2[NM];
    WIN32_FIND_DATA FindData;
    ScanTree ScTree(TRUE,TRUE,FALSE,TRUE);
    ScTree.SetFindPath(Name,"*.*", 0);
    while (ScTree.GetNextName(&FindData,FullName2, sizeof (FullName2)-1))
    {
      if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
        ERemoveDirectory(FullName2,FindData.cAlternateFileName,FALSE);
    }
  }

  FAR_OemToChar(FullName,FullName);
  FullName[strlen(FullName)+1]=0;

  memset(&fop,0,sizeof(fop)); // говорят помогает :-)
  fop.wFunc=FO_DELETE;
  fop.pFrom=FullName;
  fop.fFlags=FOF_NOCONFIRMATION|FOF_SILENT;
  if (Opt.DeleteToRecycleBin)
    fop.fFlags|=FOF_ALLOWUNDO;
  SetFileApisTo(APIS2ANSI);
  DWORD RetCode=SHFileOperation(&fop);
  DWORD RetCode2=RetCode;
  /* $ 26.01.2003 IS
       + Если не удалось удалить объект (например, имя имеет пробел на конце)
         и это не связано с прерыванием удаления пользователем, то попробуем
         еще разок, но с указанием полного имени вместе с "\\?\"
  */
  // IS: Следует заметить, что при подобном удалении объект, имя которого
  // IS: с пробелом на конце, в корзину не попадает даже, если включено
  // IS: удаление в нее! Почему - ХЗ, проблема скорее всего где-то в Windows
  // IS: Если этот момент вы посчитаете это поведение плохим и пусть уж лучше
  // IS: в подобной ситуации пользователь обломится, то просто поменяете 1 на 0
  
  // Похоже, в висте этот трюк больше не работает.
  // TODO: делать или обычное удаление, или через IFileOperation.

  #if 1
  if(RetCode && !fop.fAnyOperationsAborted && !PathPrefix(FullName))
  {
    char FullNameAlt[sizeof(FullName)+16];
    strcpy(FullNameAlt,"\\\\?\\");
    strcat(FullNameAlt,FullName);
    FullNameAlt[strlen(FullNameAlt)+1]=0;
    fop.pFrom=FullNameAlt;
    RetCode=SHFileOperation(&fop);
  }
  #endif
  /* IS $ */
  SetFileApisTo(APIS2OEM);
  if(RetCode)
  {
    SetLastError(SHErrorToWinError(RetCode2));
    return FALSE;
  }
  return !fop.fAnyOperationsAborted;
}
/* SVS $ */

int WipeFile(const char *Name)
{
  unsigned __int64 FileSize;
  HANDLE WipeHandle;
  SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
  WipeHandle=FAR_CreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (WipeHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  if ( !FAR_GetFileSize(WipeHandle, &FileSize) )
  {
    CloseHandle(WipeHandle);
    return(FALSE);
  }

  const int BufSize=65536;
  char *Buf=new char[BufSize];
  memset(Buf,(BYTE)Opt.WipeSymbol,BufSize); // используем символ заполнитель
  DWORD Written;
  while (FileSize>0)
  {
    DWORD WriteSize=(DWORD)Min((unsigned __int64)BufSize,FileSize);
    WriteFile(WipeHandle,Buf,WriteSize,&Written,NULL);
    FileSize-=WriteSize;
  }
  WriteFile(WipeHandle,Buf,BufSize,&Written,NULL);
  delete[] Buf;

  SetFilePointer(WipeHandle,0,NULL,FILE_BEGIN);
  SetEndOfFile(WipeHandle);
  CloseHandle(WipeHandle);

  char TempName[NM];
  if(MoveFile(Name,FarMkTempEx(TempName,NULL,FALSE)))
    return(FAR_DeleteFile(TempName));
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


int WipeDirectory(const char *Name)
{
  char TempName[NM], saveTempPath[NM], *ptmp;
  // переименовывам в том же каталоге - так быстрее, убирает ошибку
  // "восстановления имени" при невозможности удаления (захвачено кем-то ещё) и
  // ответе Skip, ну и в досе работает :)
  strcpy(saveTempPath, Opt.TempPath);
  strcpy(Opt.TempPath, Name);
  ptmp = strrchr(Opt.TempPath, '\\');
  if(ptmp) ptmp[1] = 0;
  ptmp = FarMkTempEx(TempName, NULL, ptmp != NULL);
  strcpy(Opt.TempPath, saveTempPath);
  if(!ptmp) { // при невозможности сгенерировать временное имя получали INVALID_ARGUMENT
    _localLastError = ERROR_ACCESS_DENIED;
    goto ret_error;
  }
  if(!MoveFile(Name, ptmp))
  {
    _localLastError = GetLastError();
ret_error:
    SetLastError(_localLastError);
    return FALSE;
  }
  return(FAR_RemoveDirectory(TempName));
}

int DeleteFileWithFolder(const char *FileName)
{
  char FileOrFolderName[NM],*Slash;

  xstrncpy(FileOrFolderName,FileName,sizeof(FileOrFolderName)-1);

  Unquote (FileOrFolderName);

  BOOL Ret=SetFileAttributes(FileOrFolderName,FILE_ATTRIBUTE_NORMAL);

  if(Ret)
  {
    if(!remove(FileOrFolderName))
    {
      if ((Slash=strrchr(FileOrFolderName,'\\'))!=NULL)
        *Slash=0;
      return(FAR_RemoveDirectory(FileOrFolderName));
    }
  }
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


void DeleteDirTree(const char *Dir)
{
  if (*Dir==0 || (Dir[0]=='\\' || Dir[0]=='/') && Dir[1]==0 ||
      Dir[1]==':' && (Dir[2]=='\\' || Dir[2]=='/') && Dir[3]==0)
    return;
  char FullName[NM];
  WIN32_FIND_DATA FindData;
  ScanTree ScTree(TRUE,TRUE,-1,TRUE);

  ScTree.SetFindPath(Dir,"*.*",0);
  while (ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
  {
    SetFileAttributes(FullName,FILE_ATTRIBUTE_NORMAL);
    if (FindData.dwFileAttributes & FA_DIREC)
    {
      if (ScTree.IsDirSearchDone())
        FAR_RemoveDirectory(FullName);
    }
    else
      FAR_DeleteFile(FullName);
  }
  SetFileAttributes(Dir,FILE_ATTRIBUTE_NORMAL);
  FAR_RemoveDirectory(Dir);
}
