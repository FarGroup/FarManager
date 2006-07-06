/*
mkdir.cpp

—оздание каталога

*/

/* Revision: 1.25 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "treelist.hpp"
#include "ctrlobj.hpp"
#include "plugin.hpp"
#include "udlist.hpp"

/* $ 04.06.2001 IS
     научимс€ создавать несколько каталогов за один раз
*/
void ShellMakeDir(Panel *SrcPanel)
{
  string strDirName;
  string strOriginalDirName;
  wchar_t *lpwszDirName;
  /* $ 15.08.2002 IS запретить дубли */
  UserDefinedListW DirList(0,0,ULF_UNIQUE);
  /* IS $ */

  /* $ 07.12.2001 IS
     создание нескольких каталогов за раз теперь опционально
  */
  BOOL MultiMakeDir=Opt.MultiMakeDir;
  for(;;)
  {
    if (!GetStringW(UMSG(MMakeFolderTitle),UMSG(MCreateFolder),L"NewFolder",
         L"",strDirName,NM*2,L"MakeFolder", //BUGBUG, no size!!!
         FIB_NOAMPERSAND|FIB_BUTTONS|FIB_EXPANDENV|FIB_CHECKBOX/*|FIB_EDITPATH*/,&MultiMakeDir,
         UMSG(MMultiMakeDir)))
      return;

    Opt.MultiMakeDir=MultiMakeDir;

    // это по поводу создани€ одиночного каталога, который
    // начинаетс€ с пробела! „тобы ручками не заключать
    // такой каталог в кавычки
    if(Opt.MultiMakeDir && wcspbrk(strDirName,L";,\"") == NULL)
       QuoteSpaceOnlyW(strDirName);

    if(!Opt.MultiMakeDir)   // нужно создать только ќƒ»Ќ каталог
    {
      UnquoteW(strDirName);     // уберем все лишние кавычки
      InsertQuoteW(strDirName); // возьмем в кавычки, т.к. могут быть разделители
    }

    if(DirList.Set(strDirName) && !wcspbrk(strDirName, ReservedFilenameSymbolsW))
      break;
    else
      MessageW(MSG_DOWN|MSG_WARNING,1,UMSG(MWarning),
                 UMSG(MIncorrectDirList), UMSG(MOk));
  }
  /* IS $ */

  const wchar_t *OneDir;

  DirList.Reset();

  while(NULL!=(OneDir=DirList.GetNext()))
  {
    strDirName = OneDir;
    strOriginalDirName = strDirName;

    //Unquote(DirName);
    if (Opt.CreateUppercaseFolders && !IsCaseMixedW(strDirName))
      strDirName.Upper();

    int Length=strDirName.GetLength();


    lpwszDirName = strDirName.GetBuffer ();
    while (Length>0 && lpwszDirName[Length-1]==' ')
      Length--;
    lpwszDirName[Length]=0;

    bool bSuccess = false;
    int Error=FALSE;

    if (Length>0 && (lpwszDirName[Length-1]==L'/' || lpwszDirName[Length-1]==L'\\'))
      lpwszDirName[Length-1]=0;

    for (wchar_t *ChPtr=lpwszDirName;*ChPtr!=0;ChPtr++)
      if (*ChPtr==L'\\' || *ChPtr==L'/')
      {
        *ChPtr=0;

        if (*lpwszDirName && CreateDirectoryW(lpwszDirName,NULL))
        {
          TreeList::AddTreeName(lpwszDirName);
          bSuccess = true;
        }
        *ChPtr=L'\\';
      }

    strDirName.ReleaseBuffer ();

    BOOL bSuccess2;
    bool bSkip=false;
    while (!(bSuccess2=CreateDirectoryW(strDirName,NULL)))
    {
      int LastError=GetLastError();
      if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
          LastError==ERROR_INVALID_NAME)
      {
        int ret;
        if (DirList.IsEmpty())
          ret=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MCancel));
        else
          ret=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MOk),UMSG(MSkip));
        bSkip = ret==1;
        if (bSuccess || bSkip) break;
        else return;
      }
      else
      {
        int ret;
        if (DirList.IsEmpty())
          ret=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MRetry),UMSG(MCancel));
        else
        {
          ret=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MRetry),UMSG(MSkip),UMSG(MCancel));
          bSkip = ret==1;
        }
        if (ret!=0)
        {
          if (bSuccess || bSkip) break;
          else return;
        }
      }
    }
    if (bSuccess2)
      TreeList::AddTreeName(strDirName);
    else if (!bSkip)
      break;
  }

  SrcPanel->Update(UPDATE_KEEP_SELECTION);

  lpwszDirName = strDirName.GetBuffer ();

  if(*lpwszDirName)
  {
    wchar_t *Slash=wcschr(lpwszDirName,L'\\');
    if (Slash!=NULL)
      *Slash=0;
    if(!SrcPanel->GoToFileW(lpwszDirName) && lpwszDirName[wcslen(lpwszDirName)-1]==L'.')
    {
      lpwszDirName[wcslen(lpwszDirName)-1]=0;
      SrcPanel->GoToFileW(lpwszDirName);
    }
  }

  strDirName.ReleaseBuffer ();

  SrcPanel->Redraw();

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  int AnotherType=AnotherPanel->GetType();
  if(AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherType==QVIEW_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}
/* IS $ */
