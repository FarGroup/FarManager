/*
delete.cpp

Удаление файлов

*/

/* Revision: 1.35 08.11.2001 $ */

/*
Modify:
  08.11.2001 SVS
    ! Уточнение ширины - прагала падлюка.
  06.11.2001 SVS
    ! Ширина месага при удалении файлов и выставлении атрибутов динамически
      меняется в сторону увеличения (с подачи SF), начиная с min 30 символов.
  24.10.2001 SVS
    - Уберем флаг MSG_KEEPBACKGROUND для месага.
  23.10.2001 SVS
    ! немного уточнений по поводу вывода текущего обрабатываемого файла
  22.10.2001 SVS
    - Артефакт с прорисовкой после внедрения CALLBACK-функции (когда 1 панель
      погашена - остается кусок месагбокса)
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  01.10.2001 IS
    ! перерисовка панелей после операции удаления, чтобы убрать все ошметки от
      сообщений
  26.09.2001 SVS
    + Opt.AutoUpdateLimit -  выше этого количество не обновлять пассивную
      панель (если ее содержимое не равно активной).
  25.07.2001 IS
    ! При удалении размер сообщения такой же как и раньше (до 820).
  19.07.2001 SVS
    ! отмена 826-го до лучших времен (по просьбе VVM)
  18.07.2001 VVM
    ! При удалении выравниваем не по ширине экрана, а по длине надписи MDeleting + 8
  13.07.2001 SVS
    - "Гадим в память" ;-(
  13.07.2001 IS
    ! Приводим сообщения, содержащие имя удаляемого файла, в божеский вид при
      помощи TruncPathStr
  11.07.2001 OT
    Перенос CtrlAltShift в Manager
  09.07.2001 SVS
    ! Небольшой антураж вокруг имени удаляемого файла.
  19.06.2001 SVS
    ! Удаление в корзину только для  FIXED-дисков
  06.06.2001 SVS
    ! Mix/Max
  31.05.2001 OT
    - Отрисовка MessageBox во время удаления.
  31.05.2001 SVS
    - не обновлялась панель после операции Unlink
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  24.04.2001 SVS
    ! для symlink`а не нужно дополниетльное подтверждение на удаление
  14.03.2001 SVS
    - Неверный анализ кода возврата функции SHFileOperation(),
      коей файл удаляется в корзину.
  13.03.2001 SVS
    + Обработка "удаления" линков - Part I
  13.03.2001 SVS
    - удаление симлинка в корзину чревато потерей оригинала!!!!!!
  12.03.2001 SVS
    + Opt.DeleteSymbolWipe -> Opt.WipeSymbol
  12.03.2001 SVS
    + Opt.DeleteSymbolWipe символ заполнитель для "ZAP-операции"
  07.03.2001 SVS
    - Падение ФАРа у Веши :-)))
  05.01.2001 SVS
    ! в зависимости от числа ставим нужное окончание для удаления
  05.01.2001 IS
    ! Косметика в сообщениях - разные сообщения в зависимости от того,
      какие и сколько элементов выделено.
  28.11.2000 SVS
    + Обеспечим корректную работу с SymLink (т.н. "Directory Junctions")
  11.11.2000 SVS
    ! Косметика: "FarTmpXXXXXX" заменена на переменную FarTmpXXXXXX
    - исправлен небольшой баг в функциях Wipe*
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "panel.hpp"
#include "chgprior.hpp"
#include "filepanels.hpp"
#include "scantree.hpp"
#include "treelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"

static void ShellDeleteMsg(char *Name);
static int AskDeleteReadOnly(char *Name,DWORD Attr);
static int ShellRemoveFile(char *Name,char *ShortName,int Wipe);
static int ERemoveDirectory(char *Name,char *ShortName,int Wipe);
static int RemoveToRecycleBin(char *Name);
static int WipeFile(char *Name);
static int WipeDirectory(char *Name);
static void ShellDeleteUpdatePanels(Panel *SrcPanel);
static void PR_ShellDeleteMsg(void);

static int ReadOnlyDeleteMode,DeleteAllFolders;

enum {DELETE_SUCCESS,DELETE_YES,DELETE_SKIP,DELETE_CANCEL};

void ShellDelete(Panel *SrcPanel,int Wipe)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  WIN32_FIND_DATA FindData;
  char DeleteFilesMsg[300],SelName[NM],SelShortName[NM],DizName[NM];
  int SelCount,FileAttr,UpdateDiz, UnlinkFolder=FALSE;
  int DizPresent;
  int Ret;

  int Opt_DeleteToRecycleBin=Opt.DeleteToRecycleBin;

/*& 31.05.2001 OT Запретить перерисовку текущего фрейма*/
  Frame *FrameFromLaunched=FrameManager->GetCurrentFrame();
  FrameFromLaunched->LockRefresh();
/* OT &*/

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
    if(GetDriveType(Root) != DRIVE_FIXED)
      Opt.DeleteToRecycleBin=0;
  }

  if (SelCount==1)
  {
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);
    if (strcmp(SelName,"..")==0 || *SelName==0)
      goto done;
    strcpy(DeleteFilesMsg,SelName);
    TruncPathStr(DeleteFilesMsg,Min((int)sizeof(DeleteFilesMsg),ScrX-16));
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
    int LenItems=strlen(StrItems);
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
    char JuncName[NM];
    if(GetJunctionPointInfo(DeleteFilesMsg,JuncName,sizeof(JuncName)))
    {
      TruncPathStr(JuncName+4,sizeof(JuncName)-4);

      //SetMessageHelp("?????");
      Ret=Message(0,3,MSG(MDeleteTitle),
                DeleteFilesMsg,
                MSG(MAskDeleteLink),
                JuncName+4,
                MSG(MDeleteLinkDelete),MSG(MDeleteLinkUnlink),MSG(MCancel));

      if(Ret == 1)
      {
        ConvertNameToFull(SelName, JuncName, sizeof(JuncName));
        if(Opt.Confirm.Delete)
        {
          ; //  ;-%
        }
        DeleteJunctionPoint(JuncName);
        ShellDeleteUpdatePanels(SrcPanel);
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
    /* $ 05.01.2001 IS
       ! Косметика в сообщениях - разные сообщения в зависимости от того,
         какие и сколько элементов выделено.
    */
    BOOL folder=(FileAttr & FA_DIREC);

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
        DelMsg=MSG(MAskWipe);
      else
        if (Opt.DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
          DelMsg=MSG(MAskDeleteRecycle);
        else
          DelMsg=MSG(MAskDelete);
    }
    /* IS $ */
    if (Message(0,2,MSG(MDeleteTitle),DelMsg,DeleteFilesMsg,MSG(MDelete),MSG(MCancel))!=0)
      goto done;
  }

  if (Opt.Confirm.Delete && SelCount>1)
  {
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(PR_ShellDeleteMsg);
    ShellDeleteMsg("");
    if (Message(MSG_DOWN|MSG_WARNING,2,MSG(MDeleteFilesTitle),MSG(MAskDelete),
                DeleteFilesMsg,MSG(MDeleteFileAll),MSG(MDeleteFileCancel))!=0)
      goto done;
  }

  if (UpdateDiz)
    SrcPanel->ReadDiz();

  SrcPanel->GetDizName(DizName);
  DizPresent=(*DizName && GetFileAttributes(DizName)!=0xFFFFFFFF);

  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  SetFarTitle(MSG(MDeletingTitle));

  CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();

  if (SrcPanel->GetType()==TREE_PANEL)
    chdir("\\");

  {
    int Cancel=0;
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(PR_ShellDeleteMsg);
    ShellDeleteMsg("");

    ReadOnlyDeleteMode=-1;

    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName) && !Cancel)
    {
      if (CheckForEsc())
        break;
      int Length=strlen(SelName);
      if (Length==0 || SelName[0]=='\\' && Length<2 ||
          SelName[1]==':' && Length<4)
        continue;
      if (FileAttr & FA_DIREC)
      {
        if (!DeleteAllFolders)
        {
          char FullName[NM];
//          ConvertNameToFull(SelName,FullName, sizeof(FullName));
          if (ConvertNameToFull(SelName,FullName, sizeof(FullName)) >= sizeof(FullName)){
            goto done;
          }
          if (IsFolderNotEmpty(FullName))
          {
            int MsgCode=0;
            /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
            char MsgFullName[NM];
            strcpy(MsgFullName, FullName);
            TruncPathStr(MsgFullName, ScrX-16);
            // для symlink`а не нужно подтверждение
            if(!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
               MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MDeleteFolderTitle),
                  MSG(MDeleteFolderConfirm),MsgFullName,
                    MSG(MDeleteFileDelete),MSG(MDeleteFileAll),
                    MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));
            /* IS $ */
            if (MsgCode<0 || MsgCode==3)
              break;
            if (MsgCode==1)
              DeleteAllFolders=1;
            if (MsgCode==2)
              continue;
          }
        }

        bool SymLink=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)!=0;
        if (!SymLink && (!Opt.DeleteToRecycleBin || Wipe))
        {
          char FullName[NM];
          ScanTree ScTree(TRUE);
          ScTree.SetFindPath(SelName,"*.*");
          while (ScTree.GetNextName(&FindData,FullName))
          {
            if (CheckForEsc())
            {
              Cancel=1;
              break;
            }
            ShellDeleteMsg(FullName);
            char ShortName[NM];
            strcpy(ShortName,FullName);
            if (*FindData.cAlternateFileName)
              strcpy(PointToName(ShortName),FindData.cAlternateFileName);
            if (FindData.dwFileAttributes & FA_DIREC)
            {
              /* $ 28.11.2000 SVS
                 Обеспечим корректную работу с SymLink
                 (т.н. "Directory Junctions")
              */
              if(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                if (FindData.dwFileAttributes & FA_RDONLY)
                  SetFileAttributes(FullName,0);
                if (ERemoveDirectory(FullName,ShortName,Wipe))
                {
                  TreeList::DelTreeName(FullName);
                  if (UpdateDiz)
                    SrcPanel->DeleteDiz(FullName,SelShortName);
                }
                continue;
              }
              /* SVS $ */
              if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && IsFolderNotEmpty(FullName))
              {
                /* $ 13.07.2001 IS
                     усекаем имя, чтоб оно поместилось в сообщение
                */
                char MsgFullName[NM];
                strcpy(MsgFullName, FullName);
                TruncPathStr(MsgFullName, ScrX-16);
                int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MDeleteFolderTitle),
                      MSG(MDeleteFolderConfirm),MsgFullName,
                      MSG(MDeleteFileDelete),MSG(MDeleteFileAll),
                      MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));
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
                  SetFileAttributes(FullName,0);
                if (ERemoveDirectory(FullName,ShortName,Wipe))
                  TreeList::DelTreeName(FullName);
              }
            }
            else
            {
              int AskCode=AskDeleteReadOnly(FullName,FindData.dwFileAttributes);
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
          ShellDeleteMsg(SelName);
          if (FileAttr & FA_RDONLY)
            SetFileAttributes(SelName,0);
          int DeleteCode;
          // нефига здесь выделываться, а надо учесть, что удаление
          // симлинка в корзину чревато потерей оригинала.
          if (SymLink || !Opt.DeleteToRecycleBin || Wipe)
            DeleteCode=ERemoveDirectory(SelName,SelShortName,Wipe);
          else
          {
            DeleteCode=RemoveToRecycleBin(SelName);
            if (!DeleteCode && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
              Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                      MSG(MCannotDeleteFolder),SelName,MSG(MOk));
          }
          if (DeleteCode)
          {
            TreeList::DelTreeName(SelName);
            if (UpdateDiz)
              SrcPanel->DeleteDiz(SelName,SelShortName);
          }
        }
      }
      else
      {
        ShellDeleteMsg(SelName);
        int AskCode=AskDeleteReadOnly(SelName,FileAttr);
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
    if (DizPresent==(*DizName && GetFileAttributes(DizName)!=0xFFFFFFFF))
      SrcPanel->FlushDiz();

  SetConsoleTitle(OldTitle);

done:
  SetPreRedrawFunc(NULL);
  Opt.DeleteToRecycleBin=Opt_DeleteToRecycleBin;
/*& 31.05.2001 OT Разрешить перерисовку фрейма */
  FrameFromLaunched->UnlockRefresh();
/* OT &*/
  /* $ 01.10.2001 IS перерисуемся, чтобы не было артефактов */
  ShellDeleteUpdatePanels(SrcPanel);
  /* IS $ */
}


void ShellDeleteUpdatePanels(Panel *SrcPanel)
{
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  int AnotherType=AnotherPanel->GetType();
  if (AnotherType!=QVIEW_PANEL)
  {
    if(AnotherPanel->NeedUpdatePanel(SrcPanel))
    {
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//      AnotherPanel->Redraw();
    }
  }
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
//  SrcPanel->Redraw();
  if (AnotherType==QVIEW_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//    AnotherPanel->Redraw();
  }
  CtrlObject->Cp()->Redraw();
}

static void PR_ShellDeleteMsg(void)
{
  ShellDeleteMsg((char*)PreRedrawParam.Param1);
}

void ShellDeleteMsg(char *Name)
{
  static int Width=30;
  int WidthTemp;
  char OutFileName[NM];

  if(Name && *Name)
    WidthTemp=Max((int)strlen(Name),(int)30);
  else
    Width=WidthTemp=30;

  if(WidthTemp > WidthNameForMessage)
    WidthTemp=WidthNameForMessage; // ширина месага - 38%
  if(WidthTemp >= sizeof(OutFileName)-4)
    WidthTemp=sizeof(OutFileName)-5;
  if(Width < WidthTemp)
    Width=WidthTemp;

  strncpy(OutFileName,Name,sizeof(OutFileName)-1);
  TruncPathStr(OutFileName,Width);
  CenterStr(OutFileName,OutFileName,Width+4);

  Message(0,0,MSG(MDeleteTitle),MSG(MDeleting),OutFileName);
  PreRedrawParam.Param1=Name;
}


int AskDeleteReadOnly(char *Name,DWORD Attr)
{
  int MsgCode;
  if ((Attr & FA_RDONLY)==0)
    return(DELETE_YES);
  if (ReadOnlyDeleteMode!=-1)
    MsgCode=ReadOnlyDeleteMode;
  else
  {
    /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
    char MsgName[NM];
    strcpy(MsgName, Name);
    TruncPathStr(MsgName, ScrX-16);
    MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MWarning),MSG(MDeleteRO),MsgName,
            MSG(MAskDeleteRO),MSG(MDeleteFileDelete),MSG(MDeleteFileAll),
            MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),
            MSG(MDeleteFileCancel));
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
  SetFileAttributes(Name,0);
  return(DELETE_YES);
}



int ShellRemoveFile(char *Name,char *ShortName,int Wipe)
{
  while (1)
  {
    if (Wipe)
    {
      if (WipeFile(Name) || WipeFile(ShortName))
        break;
    }
    else
      if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion<4 ||
          !Opt.DeleteToRecycleBin)
      {
/*
        if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
        {
          HANDLE hDelete=CreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,
                 FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_POSIX_SEMANTICS,NULL);
          if (hDelete!=INVALID_HANDLE_VALUE && CloseHandle(hDelete))
            break;
        }
*/
        if (DeleteFile(Name) || DeleteFile(ShortName))
          break;
      }
      else
        if (RemoveToRecycleBin(Name))
          break;
    int MsgCode;
    /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
    char MsgName[NM];
    strcpy(MsgName, Name);
    TruncPathStr(MsgName, ScrX-16);
    MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                    MSG(MCannotDeleteFile),MsgName,MSG(MDeleteRetry),
                    MSG(MDeleteSkip),MSG(MDeleteCancel));
    /* IS */
    switch(MsgCode)
    {
      case -1:
      case -2:
      case 2:
        return(DELETE_CANCEL);
      case 1:
        return(DELETE_SKIP);
    }
  }
  return(DELETE_SUCCESS);
}


int ERemoveDirectory(char *Name,char *ShortName,int Wipe)
{
  int RetCode;
  while (1)
  {
    if (Wipe)
    {
      if ((RetCode=WipeDirectory(Name))!=0 || (RetCode=WipeDirectory(ShortName))!=0)
        break;
    }
    else
      if ((RetCode=RemoveDirectory(Name))!=0 || (RetCode=RemoveDirectory(ShortName))!=0)
        break;
    /* $ 13.07.2001 IS усекаем имя, чтоб оно поместилось в сообщение */
    char MsgName[NM];
    strcpy(MsgName, Name);
    TruncPathStr(MsgName, ScrX-16);
    if (Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                MSG(MCannotDeleteFolder),MsgName,MSG(MDeleteRetry),
                MSG(MDeleteCancel))!=0)
    /* IS $ */
      break;
  }
  return(RetCode);
}

/* 14.03.2001 SVS
   Неверный анализ кода возврата функции SHFileOperation(),
   коей файл удаляется в корзину.
*/
int RemoveToRecycleBin(char *Name)
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
    {SE_ERR_DLLNOTFOUND,ERROR_FILE_NOT_FOUND},
    {SE_ERR_SHARE,ERROR_SHARING_VIOLATION},
    {SE_ERR_NOASSOC,ERROR_BAD_COMMAND},
  };

  SHFILEOPSTRUCT fop;
  char FullName[NM+1];
//  ConvertNameToFull(Name,FullName, sizeof(FullName));
  if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName)){
    return 1;
  }

  OemToChar(FullName,FullName);
  FullName[strlen(FullName)+1]=0;

  memset(&fop,0,sizeof(fop)); // говорят помогает :-)
  fop.wFunc=FO_DELETE;
  fop.pFrom=FullName;
  fop.fFlags=FOF_NOCONFIRMATION|FOF_SILENT;
  if (Opt.DeleteToRecycleBin)
    fop.fFlags|=FOF_ALLOWUNDO;
  SetFileApisToANSI();
  DWORD RetCode=SHFileOperation(&fop);
  SetFileApisToOEM();
  if(RetCode)
  {
    for(int I=0; I < sizeof(SHErrorCode2LastErrorCode)/sizeof(SHErrorCode2LastErrorCode[0]); ++I)
      if(SHErrorCode2LastErrorCode[I].SHError == RetCode)
      {
        SetLastError(SHErrorCode2LastErrorCode[I].LCError);
        return FALSE;
      }
  }
  RetCode=!fop.fAnyOperationsAborted;
  return(RetCode);
}
/* SVS $ */

int WipeFile(char *Name)
{
  DWORD FileSize;
  HANDLE WipeHandle;
  SetFileAttributes(Name,0);
  WipeHandle=CreateFile(Name,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (WipeHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  if ((FileSize=GetFileSize(WipeHandle,NULL))==0xFFFFFFFF)
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
    DWORD WriteSize=Min((DWORD)BufSize,FileSize);
    WriteFile(WipeHandle,Buf,WriteSize,&Written,NULL);
    FileSize-=WriteSize;
  }
  WriteFile(WipeHandle,Buf,BufSize,&Written,NULL);
  /* $ 13.07.2000 SVS
       раз уж вызвали new[], то в придачу и delete[] надо... */
  delete[] Buf;
  /* SVS $ */
  SetFilePointer(WipeHandle,0,NULL,FILE_BEGIN);
  SetEndOfFile(WipeHandle);
  CloseHandle(WipeHandle);
  char TempName[NM];
  strcpy(TempName,FarTmpXXXXXX);
  mktemp(TempName);
  MoveFile(Name,TempName);
  return(DeleteFile(TempName));
}


int WipeDirectory(char *Name)
{
  char TempName[NM];
  strcpy(TempName,FarTmpXXXXXX);
  mktemp(TempName);
  MoveFile(Name,TempName);
  return(RemoveDirectory(TempName));
}
