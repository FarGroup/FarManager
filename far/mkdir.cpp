/*
mkdir.cpp

Создание каталога
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
#include "filepanels.hpp"
#include "panel.hpp"
#include "treelist.hpp"
#include "ctrlobj.hpp"
#include "plugin.hpp"
#include "udlist.hpp"

void ShellMakeDir(Panel *SrcPanel)
{
  string strDirName;
  string strOriginalDirName;
  wchar_t *lpwszDirName;

  UserDefinedList DirList(0,0,ULF_UNIQUE);

  BOOL MultiMakeDir=Opt.MultiMakeDir;
  for(;;)
  {
    if (!GetString(UMSG(MMakeFolderTitle),UMSG(MCreateFolder),L"NewFolder",
         L"",strDirName,NM*2,L"MakeFolder", //BUGBUG, no size!!!
         FIB_NOAMPERSAND|FIB_BUTTONS|FIB_EXPANDENV|FIB_CHECKBOX/*|FIB_EDITPATH*/,&MultiMakeDir,
         UMSG(MMultiMakeDir)))
      return;

    Opt.MultiMakeDir=MultiMakeDir;

    // это по поводу создания одиночного каталога, который
    // начинается с пробела! Чтобы ручками не заключать
    // такой каталог в кавычки
    if(Opt.MultiMakeDir && wcspbrk(strDirName,L";,\"") == NULL)
       QuoteSpaceOnly(strDirName);

    if(!Opt.MultiMakeDir)   // нужно создать только ОДИН каталог
    {
      Unquote(strDirName);     // уберем все лишние кавычки
      InsertQuote(strDirName); // возьмем в кавычки, т.к. могут быть разделители
    }

    if(DirList.Set(strDirName) && !wcspbrk(strDirName, ReservedFilenameSymbols))
      break;
    else
      Message(MSG_DOWN|MSG_WARNING,1,UMSG(MWarning),
                 UMSG(MIncorrectDirList), UMSG(MOk));
  }

  const wchar_t *OneDir;

  DirList.Reset();

  while(NULL!=(OneDir=DirList.GetNext()))
  {
    strDirName = OneDir;
    strOriginalDirName = strDirName;

    //Unquote(DirName);
    if (Opt.CreateUppercaseFolders && !IsCaseMixed(strDirName))
      strDirName.Upper();

    int Length=(int)strDirName.GetLength();


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
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MCancel));
        else
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MOk),UMSG(MSkip));
        bSkip = ret==1;
        if (bSuccess || bSkip) break;
        else return;
      }
      else
      {
        int ret;
        if (DirList.IsEmpty())
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MRetry),UMSG(MCancel));
        else
        {
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),UMSG(MCannotCreateFolder),strOriginalDirName,UMSG(MRetry),UMSG(MSkip),UMSG(MCancel));
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
    if(!SrcPanel->GoToFile(lpwszDirName) && lpwszDirName[StrLength(lpwszDirName)-1]==L'.')
    {
      lpwszDirName[StrLength(lpwszDirName)-1]=0;
      SrcPanel->GoToFile(lpwszDirName);
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
