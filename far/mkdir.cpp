/*
mkdir.cpp

+þ÷ôðýøõ úð²ðûþóð

*/

/* Revision: 1.03 29.04.2001 $ */

/*
Modify:
  29.04.2001 ++
    + +ýõô¨õýøõ NWZ þ² +¨õ²¼¿úþòð
  31.07.2000 SVS
    ! +ð¸¸ø¨øü ÿõ¨õüõýý»õ ¸¨õô» ò ôøðûþóõ ¸þ÷ôðýø¿ úð²ðûþóð
  25.07.2000 IG
    - Bug 24 (ýõ ÿõ¨õ…ø²»òðûð¸¼ ÿðýõû¼, ÿþ¸ûõ ýõ³ôð…ýþóþ òûþöõýýþóþ ¸þ÷ôðýø¿ ôø¨õú²þ¨øù)
  25.06.2000 SVS
    ! +þôóþ²þòúð Master Copy
    ! +»ôõûõýøõ ò úð…õ¸²òõ ¸ðüþ¸²þ¿²õû¼ýþóþ üþô³û¿
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   +²ðýôð¨²ý»õ ÷ðóþûþòúø
*/
#include "internalheaders.hpp"
/* IS $ */

void ShellMakeDir(Panel *SrcPanel)
{
  char DirName[NM];
  if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder","",DirName,sizeof(DirName)))
    return;
  /* $ 31.07.2000 SVS
     +ð¸¸ø¨øü ÿõ¨õüõýý»õ ¸¨õô»!
  */
  ExpandEnvironmentStr(DirName, DirName, sizeof(DirName));
  /* SVS $ */
  Unquote(DirName);
  if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
    LocalStrupr(DirName);

  int Length=strlen(DirName);
  /* $ 25.07.2000 IG
     Bug 24 (ýõ ÿõ¨õ…ø²»òðûð¸¼ ÿðýõû¼, ÿþ¸ûõ ýõ³ôð…ýþóþ òûþöõýýþóþ ¸þ÷ôðýø¿ ôø¨õú²þ¨øù)
  */
  while (Length>0 && DirName[Length-1]==' ')
    Length--;
  DirName[Length]=0;
  if (Length>0 && (DirName[Length-1]=='/' || DirName[Length-1]=='\\'))
    DirName[Length-1]=0;

  char bSuccess = 0;

  for (char *ChPtr=DirName;*ChPtr!=0;ChPtr++)
    if (*ChPtr=='\\' || *ChPtr=='/')
    {
      *ChPtr=0;
      if (CreateDirectory(DirName,NULL))
      {
        TreeList::AddTreeName(DirName);
        bSuccess = 1;
      }
      *ChPtr='\\';
    }

  while (!CreateDirectory(DirName,NULL))
  {
    int LastError=GetLastError();
    if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
        LastError==ERROR_INVALID_NAME)
    {
      Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),DirName,MSG(MOk));
      if (bSuccess) break;
        else return;
    }
    else
      if (Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),DirName,MSG(MRetry),MSG(MCancel))!=0)
        if (bSuccess) break;
          else return;
  }
  /* IG $ */
  TreeList::AddTreeName(DirName);
  SrcPanel->Update(UPDATE_KEEP_SELECTION);

  char *Slash=strchr(DirName,'\\');
  if (Slash!=NULL)
    *Slash=0;
  SrcPanel->GoToFile(DirName);
  SrcPanel->Redraw();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}
