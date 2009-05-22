/*
delete.cpp

Удаление файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

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
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"

static void ShellDeleteMsg(const wchar_t *Name,int Wipe);
static int AskDeleteReadOnly(const wchar_t *Name,DWORD Attr,int Wipe);
static int ShellRemoveFile(const wchar_t *Name,const wchar_t *ShortName,int Wipe);
static int ERemoveDirectory(const wchar_t *Name,const wchar_t *ShortName,int Wipe);
static int RemoveToRecycleBin(const wchar_t *Name);
static int WipeFile(const wchar_t *Name);
static int WipeDirectory(const wchar_t *Name);
static void PR_ShellDeleteMsg(void);

static int ReadOnlyDeleteMode,SkipMode,SkipFoldersMode,DeleteAllFolders;
static clock_t DeleteStartTime;

enum {DELETE_SUCCESS,DELETE_YES,DELETE_SKIP,DELETE_CANCEL};

void ShellDelete(Panel *SrcPanel,int Wipe)
{
  ChangePriority ChPriority(Opt.DelThreadPriority);
  TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellDeleteMsg);

  FAR_FIND_DATA_EX FindData;
  string strDeleteFilesMsg;
  string strSelName;
  string strSelShortName;
  string strDizName;
  string strFullName;
  DWORD FileAttr;
  int SelCount,UpdateDiz;
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
             (SrcPanel->IsDizDisplayed() &&
             Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED));

  if ((SelCount=SrcPanel->GetSelCount())==0)
    goto done;

  // Удаление в корзину только для  FIXED-дисков
  {
    string strRoot;
//    char FSysNameSrc[NM];
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(&strSelName,FileAttr);
    ConvertNameToFull(strSelName, strRoot);
    GetPathRoot(strRoot,strRoot);
//_SVS(SysLog(L"Del: SelName='%s' Root='%s'",SelName,Root));
    if(Opt.DeleteToRecycleBin && FAR_GetDriveType(strRoot) != DRIVE_FIXED)
      Opt.DeleteToRecycleBin=0;
  }

  if (SelCount==1)
  {
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(&strSelName,FileAttr);
    if (TestParentFolderName(strSelName) || strSelName.IsEmpty() )
    {
      NeedUpdate=FALSE;
      goto done;
    }
    strDeleteFilesMsg = strSelName;
  }
  else
  {
    // в зависимости от числа ставим нужное окончание
    const wchar_t *Ends;
    wchar_t StrItems[16];
    _itow(SelCount,StrItems,10);
    Ends=MSG(MAskDeleteItemsA);
    int LenItems=StrLength(StrItems);
    if (LenItems > 0)
    {
      if((LenItems >= 2 && StrItems[LenItems-2] == L'1') ||
             StrItems[LenItems-1] >= L'5' ||
             StrItems[LenItems-1] == L'0')
        Ends=MSG(MAskDeleteItemsS);
      else if(StrItems[LenItems-1] == L'1')
        Ends=MSG(MAskDeleteItems0);
    }

    strDeleteFilesMsg.Format (MSG(MAskDeleteItems),SelCount,Ends);
  }

  Ret=1;

  //   Обработка "удаления" линков
  if((FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && SelCount==1)
  {
    string strJuncName;
    ConvertNameToFull(strSelName,strJuncName);

    if(GetReparsePointInfo(strJuncName, strJuncName)) // ? SelName ?
    {
      if(!StrCmpN(strJuncName,L"\\??\\",4))
        strJuncName.LShift(4);

      //SetMessageHelp(L"DeleteLink");

			string strAskDeleteLink=MSG(MAskDeleteLink);
			DWORD dwAttr=apiGetFileAttributes(strJuncName);
			if(dwAttr!=INVALID_FILE_ATTRIBUTES)
			{
				strAskDeleteLink+=L" ";
				strAskDeleteLink+=dwAttr&FILE_ATTRIBUTE_DIRECTORY?MSG(MAskDeleteLinkFolder):MSG(MAskDeleteLinkFile);
			}

      Ret=Message(0,3,MSG(MDeleteLinkTitle),
                strDeleteFilesMsg,
                strAskDeleteLink,
                strJuncName,
                MSG(MDeleteLinkDelete),MSG(MDeleteLinkUnlink),MSG(MCancel));

      if(Ret == 1)
      {
        ConvertNameToFull(strSelName, strJuncName);
        if(Opt.Confirm.Delete)
        {
          ; //  ;-%
        }
        if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) != -1) //JuncName?
        {
          DeleteReparsePoint(strJuncName);
          ShellUpdatePanels(SrcPanel,NeedSetUpADir);
        }
        goto done;
      }
      if(Ret != 0)
        goto done;
    }
  }

  if (Ret && (Opt.Confirm.Delete || SelCount>1 || (FileAttr & FILE_ATTRIBUTE_DIRECTORY)))
  {
    const wchar_t *DelMsg;
    const wchar_t *TitleMsg=MSG(Wipe?MDeleteWipeTitle:MDeleteTitle);
    /* $ 05.01.2001 IS
       ! Косметика в сообщениях - разные сообщения в зависимости от того,
         какие и сколько элементов выделено.
    */
    BOOL folder=(FileAttr & FILE_ATTRIBUTE_DIRECTORY);

    if (SelCount==1)
    {
      if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
        DelMsg=MSG(folder?MAskWipeFolder:MAskWipeFile);
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
      {
        DelMsg=MSG(MAskWipe);
        TitleMsg=MSG(MDeleteWipeTitle);
      }
      else
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=MSG(MAskDeleteRecycle);
        else
          DelMsg=MSG(MAskDelete);
    }

    SetMessageHelp(L"DeleteFile");
    if (Message(0,2,TitleMsg,DelMsg,strDeleteFilesMsg,MSG(Wipe?MDeleteWipe:MDelete),MSG(MCancel))!=0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
  }

  if (Opt.Confirm.Delete && SelCount>1)
  {
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    SetMessageHelp(L"DeleteFile");
    if (Message(MSG_WARNING,2,MSG(Wipe?MWipeFilesTitle:MDeleteFilesTitle),MSG(Wipe?MAskWipe:MAskDelete),
                strDeleteFilesMsg,MSG(MDeleteFileAll),MSG(MDeleteFileCancel))!=0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
    ShellDeleteMsg(L"",Wipe);
  }

  if (UpdateDiz)
    SrcPanel->ReadDiz();

  SrcPanel->GetDizName(strDizName);
	DizPresent=( !strDizName.IsEmpty() && apiGetFileAttributes(strDizName)!=INVALID_FILE_ATTRIBUTES);

  DeleteTitle = new ConsoleTitle(MSG(MDeletingTitle));

  if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) == -1)
    goto done;

  if (SrcPanel->GetType()==TREE_PANEL)
    FarChDir(L"\\");

  {
		TaskBar TB;
    int Cancel=0;
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    ShellDeleteMsg(L"",Wipe);

    ReadOnlyDeleteMode=-1;
    SkipMode=-1;
    SkipFoldersMode=-1;

    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName) && !Cancel)
    {
      if(CheckForEscSilent())
      {
        int AbortOp = ConfirmAbortOp();
        if (AbortOp)
          break;
      }

      int Length=(int)strSelName.GetLength();
      if (Length==0 || (strSelName.At(0)==L'\\' && Length<2) ||
          (strSelName.At(1)==L':' && Length<4))
        continue;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (!DeleteAllFolders)
        {
          ConvertNameToFull(strSelName, strFullName);

          if (CheckFolder(strFullName) == CHKFLD_NOTEMPTY)
          {
            int MsgCode=0;
            // для symlink`а не нужно подтверждение
            if(!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
               MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                  MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strFullName,
                    MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
                    MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));
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
          string strFullName;
          ScanTree ScTree(TRUE,TRUE);
          ScTree.SetFindPath(strSelName,L"*.*", 0);
          while (ScTree.GetNextName(&FindData,strFullName))
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
            ShellDeleteMsg(strFullName,Wipe);
            string strShortName;
            strShortName = strFullName;
            if ( !FindData.strAlternateFileName.IsEmpty() )
            {
                CutToNameUNC(strShortName);

                strShortName += FindData.strAlternateFileName; //???
            }

            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              if(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);
                int MsgCode=ERemoveDirectory(strFullName,strShortName,Wipe);
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

                TreeList::DelTreeName(strFullName);
                if (UpdateDiz)
                  SrcPanel->DeleteDiz(strFullName,strSelShortName);
                ScTree.SkipDir(); // ??? ЭТО НУЖНО ДЛЯ ТОГО, ЧТОБЫ...
                continue;
              }
              if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && CheckFolder(strFullName) == CHKFLD_NOTEMPTY)
              {
                int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                      MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strFullName,
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
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);
                int MsgCode=ERemoveDirectory(strFullName,strShortName,Wipe);
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

                TreeList::DelTreeName(strFullName);
              }
            }
            else
            {
              int AskCode=AskDeleteReadOnly(strFullName,FindData.dwFileAttributes,Wipe);
              if (AskCode==DELETE_CANCEL)
              {
                Cancel=1;
                break;
              }
              if (AskCode==DELETE_YES)
                if (ShellRemoveFile(strFullName,strShortName,Wipe)==DELETE_CANCEL)
                {
                  Cancel=1;
                  break;
                }
            }
          }
        }

        if (!Cancel)
        {
          ShellDeleteMsg(strSelName,Wipe);
          if (FileAttr & FILE_ATTRIBUTE_READONLY)
						apiSetFileAttributes(strSelName,FILE_ATTRIBUTE_NORMAL);
          int DeleteCode;
          // нефига здесь выделываться, а надо учесть, что удаление
          // симлинка в корзину чревато потерей оригинала.
          if (DirSymLink || !Opt.DeleteToRecycleBin || Wipe)
          {
            DeleteCode=ERemoveDirectory(strSelName,strSelShortName,Wipe);
            if (DeleteCode==DELETE_CANCEL)
              break;
            else if (DeleteCode==DELETE_SUCCESS)
            {
              TreeList::DelTreeName(strSelName);

              if (UpdateDiz)
                SrcPanel->DeleteDiz(strSelName,strSelShortName);
            }
          }
          else
          {
            DeleteCode=RemoveToRecycleBin(strSelName);
            if (!DeleteCode)
              Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                      MSG(MCannotDeleteFolder),strSelName,MSG(MOk));
            else
            {
              TreeList::DelTreeName(strSelName);
              if (UpdateDiz)
                SrcPanel->DeleteDiz(strSelName,strSelShortName);
            }
          }
        }
      }
      else
      {
        ShellDeleteMsg(strSelName,Wipe);
        int AskCode=AskDeleteReadOnly(strSelName,FileAttr,Wipe);
        if (AskCode==DELETE_CANCEL)
          break;
        if (AskCode==DELETE_YES)
        {
          int DeleteCode=ShellRemoveFile(strSelName,strSelShortName,Wipe);
          if (DeleteCode==DELETE_SUCCESS && UpdateDiz)
            SrcPanel->DeleteDiz(strSelName,strSelShortName);
          if (DeleteCode==DELETE_CANCEL)
            break;
        }
      }
    }
  }

  if (UpdateDiz)
		if (DizPresent==( !strDizName.IsEmpty() && apiGetFileAttributes(strDizName)!=INVALID_FILE_ATTRIBUTES))
      SrcPanel->FlushDiz();

  delete DeleteTitle;

done:
  Opt.DeleteToRecycleBin=Opt_DeleteToRecycleBin;

  // Разрешить перерисовку фрейма
  FrameFromLaunched->Unlock();

  if(NeedUpdate)
  {
    ShellUpdatePanels(SrcPanel,NeedSetUpADir);
  }
}

static void PR_ShellDeleteMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  ShellDeleteMsg(static_cast<const wchar_t*>(preRedrawItem.Param.Param1),(int)preRedrawItem.Param.Param5);
}

void ShellDeleteMsg(const wchar_t *Name,int Wipe)
{
  static int Width=30;
  int WidthTemp;
  string strOutFileName;

  if (Name == NULL || *Name == 0 || (static_cast<DWORD>(clock() - DeleteStartTime) > Opt.ShowTimeoutDelFiles))
  {
    if(Name && *Name)
    {
      DeleteStartTime = clock();     // Первый файл рисуется всегда
      WidthTemp=Max(StrLength(Name),(int)30);
    }
    else
      Width=WidthTemp=30;

    if(WidthTemp > WidthNameForMessage)
      WidthTemp=WidthNameForMessage; // ширина месага - 38%
    if(Width < WidthTemp)
      Width=WidthTemp;

    if(Name)//???
    {
      strOutFileName = Name;
      TruncPathStr(strOutFileName,Width);
      CenterStr(strOutFileName,strOutFileName,Width+4);

      Message(0,0,MSG(Wipe?MDeleteWipeTitle:MDeleteTitle),MSG(Wipe?MDeletingWiping:MDeleting),strOutFileName);
    }
  }
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=static_cast<void*>(const_cast<wchar_t*>(Name));
  preRedrawItem.Param.Param5=(__int64)Wipe;
  PreRedraw.SetParam(preRedrawItem.Param);
}


int AskDeleteReadOnly(const wchar_t *Name,DWORD Attr,int Wipe)
{
  int MsgCode;
  if ((Attr & FILE_ATTRIBUTE_READONLY)==0)
    return(DELETE_YES);
  if (ReadOnlyDeleteMode!=-1)
    MsgCode=ReadOnlyDeleteMode;
  else
  {
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
	apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
  return(DELETE_YES);
}



int ShellRemoveFile(const wchar_t *Name,const wchar_t *ShortName,int Wipe)
{
  string strFullName;

  ConvertNameToFull (Name, strFullName);

  int MsgCode=0;

  while (1)
  {
    if (Wipe)
    {
      if (SkipMode!=-1)
        MsgCode=SkipMode;
      else
      {
        if(GetNumberOfLinks(strFullName) > 1)
        {
          /*
                            Файл
                         "имя файла"
                Файл имеет несколько жестких связей.
  Уничтожение файла приведет к обнулению всех ссылающихся на него файлов.
                        Уничтожать файл?
          */
          MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MError),strFullName,
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
      if (!Opt.DeleteToRecycleBin)
      {
/*
        HANDLE hDelete=FAR_CreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,
               FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_POSIX_SEMANTICS,NULL);
        if (hDelete!=INVALID_HANDLE_VALUE && CloseHandle(hDelete))
          break;
*/
				if (apiDeleteFile(Name))
          break;
      }
      else
        if (RemoveToRecycleBin(Name))
          break;
    if (SkipMode!=-1)
        MsgCode=SkipMode;
    else
    {
      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                      MSG(MCannotDeleteFile),Name,MSG(MDeleteRetry),
                      MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
    }
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


int ERemoveDirectory(const wchar_t *Name,const wchar_t *ShortName,int Wipe)
{
  string strFullName;

  ConvertNameToFull(Name,strFullName);

  while (1)
  {
    if (Wipe)
    {
      if (WipeDirectory(Name) || (_localLastError != ERROR_ACCESS_DENIED && WipeDirectory(ShortName)))
        break;
    }
    else
      if (apiRemoveDirectory(Name) || (_localLastError != ERROR_ACCESS_DENIED && apiRemoveDirectory(ShortName)))
        break;
    int MsgCode;
    if (SkipFoldersMode!=-1)
        MsgCode=SkipFoldersMode;
    else
    {
      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                  MSG(MCannotDeleteFolder),Name,MSG(MDeleteRetry),
                  MSG(MDeleteSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
    }
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

int RemoveToRecycleBin(const wchar_t *Name)
{
  SHFILEOPSTRUCTW fop;
  string strFullName;
  ConvertNameToFull(Name, strFullName);

  // При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
	if(WinVer.dwMajorVersion<6 && Opt.DeleteToRecycleBinKillLink && apiGetFileAttributes(Name) == FILE_ATTRIBUTE_DIRECTORY)
  {
    string strFullName2;
    FAR_FIND_DATA_EX FindData;
    ScanTree ScTree(TRUE,TRUE,FALSE);
    ScTree.SetFindPath(Name,L"*.*", 0);
    while (ScTree.GetNextName(&FindData,strFullName2))
    {
      if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
        ERemoveDirectory(strFullName2,FindData.strAlternateFileName,FALSE);
    }
  }


  memset(&fop,0,sizeof(fop)); // говорят помогает :-)

  wchar_t *lpwszName = strFullName.GetBuffer (strFullName.GetLength()+2);

  lpwszName[strFullName.GetLength()+1] = 0; //dirty trick to make strFullName end with DOUBLE zero!!!

  fop.wFunc=FO_DELETE;
  fop.pFrom=lpwszName;
  fop.pTo = L"\0\0";
  fop.fFlags=FOF_NOCONFIRMATION|FOF_SILENT;
  if (Opt.DeleteToRecycleBin)
    fop.fFlags|=FOF_ALLOWUNDO;
  DWORD RetCode=SHFileOperationW(&fop);
  DWORD RetCode2=RetCode;

  strFullName.ReleaseBuffer();
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
	if(RetCode && !fop.fAnyOperationsAborted && !PathPrefix(strFullName))
  {
		string strFullNameAlt=L"\\\\?\\";
		strFullNameAlt+=strFullName;
		lpwszName = strFullNameAlt.GetBuffer (strFullNameAlt.GetLength()+2);
		lpwszName[strFullNameAlt.GetLength()+1] = 0; //dirty trick to make strFullName end with DOUBLE zero!!!
		fop.pFrom=lpwszName;
    RetCode=SHFileOperationW(&fop);
		strFullNameAlt.ReleaseBuffer();
  }
  #endif

  if(RetCode)
  {
		SetLastError(SHErrorToWinError(RetCode2));
		return FALSE;
  }
	return !fop.fAnyOperationsAborted;

}

int WipeFile(const wchar_t *Name)
{
  unsigned __int64 FileSize;
  HANDLE WipeHandle;
	apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
  WipeHandle=apiCreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN);
  if (WipeHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  if ( !apiGetFileSizeEx(WipeHandle, &FileSize) )
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

	apiSetFilePointerEx(WipeHandle,0,NULL,FILE_BEGIN);
  SetEndOfFile(WipeHandle);
  CloseHandle(WipeHandle);

  string strTempName;

  FarMkTempEx(strTempName,NULL,FALSE);

  if(MoveFileW(Name,strTempName))
		return(apiDeleteFile(strTempName)); //BUGBUG
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


int WipeDirectory(const wchar_t *Name)
{
  string strTempName, strSavePath(Opt.strTempPath);

  BOOL usePath = FALSE;
	if(FirstSlash(Name)) {
    Opt.strTempPath = Name;
    CutToSlash(Opt.strTempPath);
    usePath = TRUE;
  }
  FarMkTempEx(strTempName,NULL,usePath);
  Opt.strTempPath = strSavePath;

  if(!MoveFileW(Name, strTempName))
  {
    SetLastError((_localLastError = GetLastError()));
    return FALSE;
  }
  return apiRemoveDirectory(strTempName);
}

int DeleteFileWithFolder(const wchar_t *FileName)
{
  string strFileOrFolderName;

  strFileOrFolderName = FileName;

  Unquote(strFileOrFolderName);

	BOOL Ret=apiSetFileAttributes(strFileOrFolderName,FILE_ATTRIBUTE_NORMAL);

  if(Ret)
  {
		if(apiDeleteFile(strFileOrFolderName)) //BUGBUG
    {
      CutToSlash(strFileOrFolderName,true);
      return apiRemoveDirectory(strFileOrFolderName);
    }
  }
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


void DeleteDirTree(const wchar_t *Dir)
{
  if (*Dir==0 ||
      (IsSlash(Dir[0]) && Dir[1]==0) ||
      (Dir[1]==L':' && IsSlash(Dir[2]) && Dir[3]==0))
    return;
  string strFullName;
  FAR_FIND_DATA_EX FindData;
  ScanTree ScTree(TRUE,TRUE);

  ScTree.SetFindPath(Dir,L"*.*",0);
  while (ScTree.GetNextName(&FindData, strFullName))
  {
		apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);
    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (ScTree.IsDirSearchDone())
        apiRemoveDirectory(strFullName);
    }
    else
      apiDeleteFile(strFullName);
  }
	apiSetFileAttributes(Dir,FILE_ATTRIBUTE_NORMAL);
  apiRemoveDirectory(Dir);
}
