/*
mkdir.cpp

Создание каталога

*/

/* Revision: 1.03 29.04.2001 $ */

/*
Modify:
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  31.07.2000 SVS
    ! Расширим переменные среды в диалоге создания каталога
  25.07.2000 IG
    - Bug 24 (не переµитывалась панель, после неудаµного вложенного создания директорий)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в каµестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

void ShellMakeDir(Panel *SrcPanel)
{
  char DirName[NM];
  if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder","",DirName,sizeof(DirName)))
    return;
  /* $ 31.07.2000 SVS
     Расширим переменные среды!
  */
  ExpandEnvironmentStr(DirName, DirName, sizeof(DirName));
  /* SVS $ */
  Unquote(DirName);
  if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
    LocalStrupr(DirName);

  int Length=strlen(DirName);
  /* $ 25.07.2000 IG
     Bug 24 (не переµитывалась панель, после неудаµного вложенного создания директорий)
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


