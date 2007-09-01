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

#include "global.hpp"
#include "fn.hpp"
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
  FAR_FIND_DATA_EX FindData;
  string strDeleteFilesMsg;
  string strSelName;
  string strSelShortName;
  string strDizName;
  string strFullName;
  int SelCount,FileAttr,UpdateDiz;
  int DizPresent;
  int Ret;
  BOOL NeedUpdate=TRUE, NeedSetUpADir=FALSE;
  ConsoleTitle *DeleteTitle;

  int Opt_DeleteToRecycleBin=Opt.DeleteToRecycleBin;

/*& 31.05.2001 OT Запретить перерисовку текущего фрейма*/
  Frame *FrameFromLaunched=FrameManager->GetCurrentFrame();
  FrameFromLaunched->Lock();
/* OT &*/

  DeleteAllFolders=!Opt.Confirm.DeleteFolder;

  UpdateDiz=(Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS ||
             SrcPanel->IsDizDisplayed() &&
             Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED);

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
    TruncPathStr(strDeleteFilesMsg, ScrX-16);
  }
  else
  /* $ 05.01.2001 SVS
     в зависимости от числа ставим нужное окончание*/
  {
  /* $ 05.01.2001 IS
  Вместо "файлов" пишем нейтральное - "элементов"
  */
    const wchar_t *Ends;
    wchar_t StrItems[16];
    _itow(SelCount,StrItems,10);
    int LenItems=StrLength(StrItems);
    if((LenItems >= 2 && StrItems[LenItems-2] == L'1') ||
           StrItems[LenItems-1] >= L'5' ||
           StrItems[LenItems-1] == L'0')
      Ends=UMSG(MAskDeleteItemsS);
    else if(StrItems[LenItems-1] == L'1')
      Ends=UMSG(MAskDeleteItems0);
    else
      Ends=UMSG(MAskDeleteItemsA);


    strDeleteFilesMsg.Format (UMSG(MAskDeleteItems),SelCount,Ends);
  }
  /* SVS $ */
  Ret=1;

  /* $ 13.02.2001 SVS
     Обработка "удаления" линков
  */
  if((FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && SelCount==1)
  {
    string strJuncName;
    ConvertNameToFull(strSelName,strJuncName);

    if(GetJunctionPointInfo(strJuncName, strJuncName)) // ? SelName ?
    {
      strJuncName.LShift(4);
      //TruncPathStr(strJuncName, strJuncName.GetLength()-4); //

      //SetMessageHelp(L"DeleteLink");
      Ret=Message(0,3,UMSG(MDeleteLinkTitle),
                strDeleteFilesMsg,
                UMSG(MAskDeleteLink),
                strJuncName,
                UMSG(MDeleteLinkDelete),UMSG(MDeleteLinkUnlink),UMSG(MCancel));

      if(Ret == 1)
      {
        ConvertNameToFull(strSelName, strJuncName);
        if(Opt.Confirm.Delete)
        {
          ; //  ;-%
        }
        if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) != -1) //JuncName?
        {
          DeleteJunctionPoint(strJuncName);
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
    const wchar_t *DelMsg;
    const wchar_t *TitleMsg=UMSG(Wipe?MDeleteWipeTitle:MDeleteTitle);
    /* $ 05.01.2001 IS
       ! Косметика в сообщениях - разные сообщения в зависимости от того,
         какие и сколько элементов выделено.
    */
    BOOL folder=(FileAttr & FA_DIREC);

    if (SelCount==1)
    {
      if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
        DelMsg=UMSG(folder?MAskWipeFolder:MAskWipeFile);
      else
      {
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=UMSG(folder?MAskDeleteRecycleFolder:MAskDeleteRecycleFile);
        else
          DelMsg=UMSG(folder?MAskDeleteFolder:MAskDeleteFile);
      }
    }
    else
    {
      if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
      {
        DelMsg=UMSG(MAskWipe);
        TitleMsg=UMSG(MDeleteWipeTitle);
      }
      else
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=UMSG(MAskDeleteRecycle);
        else
          DelMsg=UMSG(MAskDelete);
    }
    /* IS $ */
    SetMessageHelp(L"DeleteFile");
    if (Message(0,2,TitleMsg,DelMsg,strDeleteFilesMsg,UMSG(Wipe?MDeleteWipe:MDelete),UMSG(MCancel))!=0)
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
    if (Message(MSG_WARNING,2,UMSG(Wipe?MWipeFilesTitle:MDeleteFilesTitle),UMSG(Wipe?MAskWipe:MAskDelete),
                strDeleteFilesMsg,UMSG(MDeleteFileAll),UMSG(MDeleteFileCancel))!=0)
    {
      NeedUpdate=FALSE;
      goto done;
    }
    SetPreRedrawFunc(PR_ShellDeleteMsg);
    ShellDeleteMsg(L"",Wipe);
  }

  if (UpdateDiz)
    SrcPanel->ReadDiz();

  SrcPanel->GetDizName(strDizName);
  DizPresent=( !strDizName.IsEmpty() && GetFileAttributesW(strDizName)!=0xFFFFFFFF);

  DeleteTitle = new ConsoleTitle(UMSG(MDeletingTitle));

  if((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) == -1)
    goto done;

  if (SrcPanel->GetType()==TREE_PANEL)
    FarChDir(L"\\");

  {
    int Cancel=0;
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(PR_ShellDeleteMsg);
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
      if (Length==0 || strSelName.At(0)==L'\\' && Length<2 ||
          strSelName.At(1)==L':' && Length<4)
        continue;
      if (FileAttr & FA_DIREC)
      {
        if (!DeleteAllFolders)
        {
          ConvertNameToFull(strSelName, strFullName);

          if (CheckFolder(strFullName) == CHKFLD_NOTEMPTY)
          {
            int MsgCode=0;
            /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
            string strMsgFullName;

            strMsgFullName = strFullName;
            TruncPathStr(strMsgFullName, ScrX-16);
            // для symlink`а не нужно подтверждение
            if(!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
               MsgCode=Message(MSG_DOWN|MSG_WARNING,4,UMSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                  UMSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strMsgFullName,
                    UMSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),UMSG(MDeleteFileAll),
                    UMSG(MDeleteFileSkip),UMSG(MDeleteFileCancel));
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

        bool SymLink=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)!=0;
        if (!SymLink && (!Opt.DeleteToRecycleBin || Wipe))
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
                wchar_t *lpwszDot = strShortName.GetBuffer();

                lpwszDot = wcsrchr (lpwszDot, L'.');

                if ( lpwszDot )
                    *(lpwszDot+1) = 0;

                strShortName.ReleaseBuffer ();

                strShortName += FindData.strAlternateFileName; //???
            }

            if (FindData.dwFileAttributes & FA_DIREC)
            {
              /* $ 28.11.2000 SVS
                 Обеспечим корректную работу с SymLink
                 (т.н. "Directory Junctions")
              */
              if(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                if (FindData.dwFileAttributes & FA_RDONLY)
                  SetFileAttributesW(strFullName,FILE_ATTRIBUTE_NORMAL);
                /* $ 19.01.2003 KM
                   Обработка кода возврата из ERemoveDirectory.
                */
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
                /* KM $ */
              }
              /* SVS $ */
              if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && CheckFolder(strFullName) == CHKFLD_NOTEMPTY)
              {
                /* $ 13.07.2001 IS
                     усекаем имя, чтоб оно поместилось в сообщение
                */
                string strMsgFullName;
                strMsgFullName = strFullName;
                TruncPathStr(strMsgFullName, ScrX-16);
                int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,UMSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
                      UMSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strMsgFullName,
                      UMSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),UMSG(MDeleteFileAll),
                      UMSG(MDeleteFileSkip),UMSG(MDeleteFileCancel));
                /* IS $ */
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
                  SetFileAttributesW(strFullName,FILE_ATTRIBUTE_NORMAL);
                /* $ 19.01.2003 KM
                   Обработка кода возврата из ERemoveDirectory.
                */
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
                /* KM $ */
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
          if (FileAttr & FA_RDONLY)
            SetFileAttributesW(strSelName,FILE_ATTRIBUTE_NORMAL);
          int DeleteCode;
          // нефига здесь выделываться, а надо учесть, что удаление
          // симлинка в корзину чревато потерей оригинала.
          if (SymLink || !Opt.DeleteToRecycleBin || Wipe)
          {
            /* $ 19.01.2003 KM
               Обработка кода возврата из ERemoveDirectory.
            */
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
            if (!DeleteCode)// && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
              Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),
                      UMSG(MCannotDeleteFolder),strSelName,UMSG(MOk));
            else
            {
              TreeList::DelTreeName(strSelName);
              if (UpdateDiz)
                SrcPanel->DeleteDiz(strSelName,strSelShortName);
            }
            /* KM $ */
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
    if (DizPresent==( !strDizName.IsEmpty() && GetFileAttributesW(strDizName)!=0xFFFFFFFF))
      SrcPanel->FlushDiz();

  delete DeleteTitle;

done:
  SetPreRedrawFunc(NULL);
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
  ShellDeleteMsg(static_cast<const wchar_t*>(PreRedrawParam.Param1),(int)PreRedrawParam.Param5);
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

    strOutFileName = Name;
    TruncPathStr(strOutFileName,Width);
    CenterStr(strOutFileName,strOutFileName,Width+4);

    Message(0,0,UMSG(Wipe?MDeleteWipeTitle:MDeleteTitle),UMSG(Wipe?MDeletingWiping:MDeleting),strOutFileName);
  }
  PreRedrawParam.Param1=static_cast<void*>(const_cast<wchar_t*>(Name));
  PreRedrawParam.Param5=(__int64)Wipe;
}


int AskDeleteReadOnly(const wchar_t *Name,DWORD Attr,int Wipe)
{
  int MsgCode;
  if ((Attr & FA_RDONLY)==0)
    return(DELETE_YES);
  if (ReadOnlyDeleteMode!=-1)
    MsgCode=ReadOnlyDeleteMode;
  else
  {
    /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
    string strMsgName;
    strMsgName = Name;
    TruncPathStr(strMsgName, ScrX-16);
    MsgCode=Message(MSG_DOWN|MSG_WARNING,5,UMSG(MWarning),UMSG(MDeleteRO),strMsgName,
            UMSG(Wipe?MAskWipeRO:MAskDeleteRO),UMSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),UMSG(MDeleteFileAll),
            UMSG(MDeleteFileSkip),UMSG(MDeleteFileSkipAll),
            UMSG(MDeleteFileCancel));
    /* IS $ */
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
  SetFileAttributesW(Name,FILE_ATTRIBUTE_NORMAL);
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
          string strMsgName;
          strMsgName = Name;
          TruncPathStr(strMsgName, ScrX-16);
          /*
                            Файл
                         "имя файла"
                Файл имеет несколько жестких связей.
  Уничтожение файла приведет к обнулению всех ссылающихся на него файлов.
                        Уничтожать файл?
          */
          MsgCode=Message(MSG_DOWN|MSG_WARNING,5,UMSG(MError),
                          UMSG(MDeleteHardLink1),UMSG(MDeleteHardLink2),UMSG(MDeleteHardLink3),
                          UMSG(MDeleteFileWipe),UMSG(MDeleteFileAll),UMSG(MDeleteFileSkip),UMSG(MDeleteFileSkipAll),UMSG(MDeleteCancel));
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
        if (DeleteFileW(Name) || DeleteFileW(ShortName)) //BUGBUG
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
      /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
      string strMsgName;
      strMsgName = Name;
      TruncPathStr(strMsgName, ScrX-16);

      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
                      UMSG(MCannotDeleteFile),strMsgName,UMSG(MDeleteRetry),
                      UMSG(MDeleteSkip),UMSG(MDeleteFileSkipAll),UMSG(MDeleteCancel));
      /* IS */
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
    /* $ 19.01.2003 KM
       Добавлен Skip и Skip all
    */
    int MsgCode;
    if (SkipFoldersMode!=-1)
        MsgCode=SkipFoldersMode;
    else
    {
      /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
      string strMsgName;
      strMsgName = Name;
      TruncPathStr(strMsgName, ScrX-16);

      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
                  UMSG(MCannotDeleteFolder),strMsgName,UMSG(MDeleteRetry),
                  UMSG(MDeleteSkip),UMSG(MDeleteFileSkipAll),UMSG(MDeleteCancel));
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

/* 14.03.2001 SVS
   Неверный анализ кода возврата функции SHFileOperation(),
   коей файл удаляется в корзину.
*/
int RemoveToRecycleBin(const wchar_t *Name)
{
  static struct {
    DWORD SHError;
    DWORD LCError;
  }
  SHErrorCode2LastErrorCode[]=
  {
    {SE_ERR_FNF,ERROR_FILE_NOT_FOUND},
    {SE_ERR_PNF,ERROR_PATH_NOT_FOUND},
    {SE_ERR_ACCESSDENIED,ERROR_ACCESS_DENIED},
    {SE_ERR_OOM,ERROR_OUTOFMEMORY},
    {SE_ERR_DLLNOTFOUND,ERROR_SHARING_VIOLATION},
    {SE_ERR_SHARE,ERROR_SHARING_VIOLATION},
    {SE_ERR_NOASSOC,ERROR_BAD_COMMAND},
  };

  SHFILEOPSTRUCTW fop;
  string strFullName;
  ConvertNameToFull(Name, strFullName);

  // При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
  if(Opt.DeleteToRecycleBinKillLink && GetFileAttributesW(Name) == FA_DIREC)
  {
    string strFullName2;
    FAR_FIND_DATA_EX FindData;
    ScanTree ScTree(TRUE,TRUE,FALSE);
    ScTree.SetFindPath(Name,L"*.*", 0);
    while (ScTree.GetNextName(&FindData,strFullName2))
    {
      if(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        ERemoveDirectory(strFullName2,FindData.strAlternateFileName,FALSE);
    }
  }


  memset(&fop,0,sizeof(fop)); // говорят помогает :-)

  wchar_t *lpwszName = strFullName.GetBuffer ((int)strFullName.GetLength()+1);

  lpwszName[StrLength(lpwszName)+1] = 0; //dirty trick to make strFullName ends with DOUBLE zero!!!

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
  #if 1
  if(RetCode && !fop.fAnyOperationsAborted &&
     !((strFullName.At(0)==L'/' && strFullName.At(1)==L'/') || // проверим, если слеши уже
     (strFullName.At(0)==L'\\' && strFullName.At(1)==L'\\'))   // есть, то и рыпаться не стоит
    )
  {
    string strFullNameAlt;

    strFullNameAlt.Format (L"\\\\?\\%s",(const wchar_t*)strFullName);

    fop.pFrom=strFullNameAlt;
    RetCode=SHFileOperationW(&fop);
  }
  #endif
  /* IS $ */
  if(RetCode)
  {
    for(int I=0; I < sizeof(SHErrorCode2LastErrorCode)/sizeof(SHErrorCode2LastErrorCode[0]); ++I)
      if(SHErrorCode2LastErrorCode[I].SHError == RetCode2)
      {
        SetLastError(SHErrorCode2LastErrorCode[I].LCError);
        return FALSE;
      }
  }
  RetCode=!fop.fAnyOperationsAborted;
  return(RetCode);
}
/* SVS $ */

int WipeFile(const wchar_t *Name)
{
  unsigned __int64 FileSize;
  HANDLE WipeHandle;
  SetFileAttributesW(Name,FILE_ATTRIBUTE_NORMAL);
  WipeHandle=apiCreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (WipeHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  if ( !apiGetFileSize(WipeHandle, &FileSize) )
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

  string strTempName;

  FarMkTempEx(strTempName,NULL,FALSE);

  if(MoveFileW(Name,strTempName))
    return(DeleteFileW(strTempName)); //BUGBUG
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


int WipeDirectory(const wchar_t *Name)
{
  string strTempName;

  FarMkTempEx(strTempName,NULL,FALSE);

  BOOL Ret=MoveFileW(Name, strTempName);
  if(!Ret)
  {
    SetLastError((_localLastError = GetLastError()));
    return FALSE;
  }
  return apiRemoveDirectory(strTempName);
}

int DeleteFileWithFolder(const wchar_t *FileName)
{
  string strFileOrFolderName;
  wchar_t *Slash;

  strFileOrFolderName = FileName;

  Unquote(strFileOrFolderName);

  BOOL Ret=SetFileAttributesW(strFileOrFolderName,FILE_ATTRIBUTE_NORMAL);

  if(Ret)
  {
    if(!DeleteFileW(strFileOrFolderName)) //BUGBUG
    {
      Slash = strFileOrFolderName.GetBuffer ();
      if ((Slash=wcsrchr(Slash,L'\\'))!=NULL)
        *Slash=0;

      strFileOrFolderName.ReleaseBuffer();

      return apiRemoveDirectory(strFileOrFolderName);
    }
  }
  SetLastError((_localLastError = GetLastError()));
  return FALSE;
}


void DeleteDirTree(const wchar_t *Dir)
{
  if (*Dir==0 || (Dir[0]==L'\\' || Dir[0]==L'/') && Dir[1]==0 ||
      Dir[1]==L':' && (Dir[2]==L'\\' || Dir[2]==L'/') && Dir[3]==0)
    return;
  string strFullName;
  FAR_FIND_DATA_EX FindData;
  ScanTree ScTree(TRUE,TRUE);

  ScTree.SetFindPath(Dir,L"*.*",0);
  while (ScTree.GetNextName(&FindData, strFullName))
  {
    SetFileAttributesW(strFullName,FILE_ATTRIBUTE_NORMAL);
    if (FindData.dwFileAttributes & FA_DIREC)
    {
      if (ScTree.IsDirSearchDone())
        apiRemoveDirectory(strFullName);
    }
    else
      apiDeleteFile(strFullName);
  }
  SetFileAttributesW(Dir,FILE_ATTRIBUTE_NORMAL);
  apiRemoveDirectory(Dir);
}
