/*
mkdir.cpp

—оздание каталога

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

/* $ 04.06.2001 IS
     научимс€ создавать несколько каталогов за один раз
*/
void ShellMakeDir(Panel *SrcPanel)
{
  char DirName[NM*2], OriginalDirName[NM*2];
  *DirName=0;
  /* $ 15.08.2002 IS запретить дубли */
  UserDefinedList DirList(0,0,ULF_UNIQUE);

  /* $ 07.12.2001 IS
     создание нескольких каталогов за раз теперь опционально
  */
  BOOL MultiMakeDir=Opt.MultiMakeDir;
  for(;;)
  {
    if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder",
         DirName,DirName,sizeof(DirName),"MakeFolder",
         FIB_NOAMPERSAND|FIB_BUTTONS|FIB_EXPANDENV|FIB_CHECKBOX/*|FIB_EDITPATH*/,&MultiMakeDir,
         MSG(MMultiMakeDir)))
      return;

    Opt.MultiMakeDir=MultiMakeDir;

    // это по поводу создани€ одиночного каталога, который
    // начинаетс€ с пробела! „тобы ручками не заключать
    // такой каталог в кавычки
    if(Opt.MultiMakeDir && strpbrk(DirName,";,\"") == NULL)
       QuoteSpaceOnly(DirName);

    if(!Opt.MultiMakeDir)   // нужно создать только ќƒ»Ќ каталог
    {
      Unquote(DirName);     // уберем все лишние кавычки
      InsertQuote(DirName); // возьмем в кавычки, т.к. могут быть разделители
    }

    if(DirList.Set(DirName))
      break;
    else
      Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning), MSG(MIncorrectDirList), MSG(MOk));
  }

  *DirName=0;
  const char *OneDir;

  DirList.Reset();
  while(NULL!=(OneDir=DirList.GetNext()))
  {
    xstrncpy(DirName, OneDir, sizeof(DirName)-1);
    strcpy(OriginalDirName,DirName);

    //Unquote(DirName);
    if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
      LocalStrupr(DirName);

    int Length=(int)strlen(DirName);

    while (Length>0 && DirName[Length-1]==' ')
      Length--;
    DirName[Length]=0;

    bool bSuccess = false;
    int Error=FALSE;

    if (Length>0 && (DirName[Length-1]=='/' || DirName[Length-1]=='\\'))
      DirName[Length-1]=0;

    for (char *ChPtr=DirName;*ChPtr!=0;ChPtr++)
      if (*ChPtr=='\\' || *ChPtr=='/')
      {
        *ChPtr=0;
        if (*DirName && CreateDirectory(DirName,NULL))
        {
          TreeList::AddTreeName(DirName);
          bSuccess = true;
        }
        *ChPtr='\\';
      }


    BOOL bSuccess2;
    bool bSkip=false;
    while (!(bSuccess2=CreateDirectory(DirName,NULL)))
    {
      int LastError=GetLastError();
      if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
          LastError==ERROR_INVALID_NAME || LastError == ERROR_DIRECTORY)
      {
        int ret;
        if (DirList.IsEmpty())
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),OriginalDirName,MSG(MCancel));
        else
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),OriginalDirName,MSG(MCancel),MSG(MSkip));
        bSkip = ret==1;
        if (bSuccess || bSkip) break;
        else return;
      }
      else
      {
        int ret;
        if (DirList.IsEmpty())
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),OriginalDirName,MSG(MRetry),MSG(MCancel));
        else
        {
          ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),MSG(MCannotCreateFolder),OriginalDirName,MSG(MRetry),MSG(MSkip),MSG(MCancel));
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
      TreeList::AddTreeName(DirName);
    else if (!bSkip)
      break;
  }

  SrcPanel->Update(UPDATE_KEEP_SELECTION);

  if(*DirName)
  {
    char *Slash=strchr(DirName,'\\');
    if (Slash!=NULL)
      *Slash=0;
    if(!SrcPanel->GoToFile(DirName) && DirName[strlen(DirName)-1]=='.')
    {
      DirName[strlen(DirName)-1]=0;
      SrcPanel->GoToFile(DirName);
    }
  }
  SrcPanel->Redraw();

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  int AnotherType=AnotherPanel->GetType();
  if(AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherType==QVIEW_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}
