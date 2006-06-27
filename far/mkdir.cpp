/*
mkdir.cpp

—оздание каталога

*/

/* Revision: 1.24 31.03.2006 $ */

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
  /* $ 02.06.2001 KM
     + ћаленька€ модификаци€: добавлены кнопки [ OK ] и [ Cancel ]
       в диалог создани€ каталога
  */
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
  /* KM $ */

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
    /* $ 25.07.2000 IG
       Bug 24 (не перечитывалась панель, после неудачного вложенного
       создани€ директорий)
    */
    while (Length>0 && lpwszDirName[Length-1]==' ')
      Length--;
    lpwszDirName[Length]=0;

    char bSuccess = 0;
    int Error=FALSE;

    if (Length>0 && (lpwszDirName[Length-1]==L'/' || lpwszDirName[Length-1]==L'\\'))
      lpwszDirName[Length-1]=0;

    for (wchar_t *ChPtr=lpwszDirName;*ChPtr!=0;ChPtr++)
      if (*ChPtr==L'\\' || *ChPtr==L'/')
      {
        *ChPtr=0;

        if (CreateDirectoryW(lpwszDirName,NULL))
        {
          TreeList::AddTreeName(lpwszDirName);
          bSuccess = 1;
        }
        *ChPtr=L'\\';
      }

    strDirName.ReleaseBuffer ();

    while (!CreateDirectoryW(strDirName,NULL))
    {
      int LastError=GetLastError();
      if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
          LastError==ERROR_INVALID_NAME)
      {
        MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MOk));
        if (bSuccess) break;
          else return;
      }
      else
        if (MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MRetry),UMSG(MCancel))!=0)
          if (bSuccess) break;
            else return;
    }
    /* IG $ */

    TreeList::AddTreeName(strDirName);
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
