/*
mkdir.cpp

Создание каталога

*/

/* Revision: 1.05 02.06.2001 $ */

/*
Modify:
  02.06.2001 KM
    + Маленькая модификация: добавлены кнопки [ OK ] и [ Cancel ]
      в диалог создания каталога
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  31.07.2000 SVS
    ! Расширим переменные среды в диалоге создания каталога
  25.07.2000 IG
    - Bug 24 (не перечитывалась панель, после неудачного вложенного создания директорий)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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

void ShellMakeDir(Panel *SrcPanel)
{
  /* $ 02.06.2001 KM
     + Маленькая модификация: добавлены кнопки [ OK ] и [ Cancel ]
       в диалог создания каталога
  */
  char DirName[NM];
  if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder","",DirName,sizeof(DirName),"MakeFolder",FIB_BUTTONS|FIB_EXPANDENV))
    return;
  /* KM $ */
  Unquote(DirName);
  if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
    LocalStrupr(DirName);

  int Length=strlen(DirName);
  /* $ 25.07.2000 IG
     Bug 24 (не перечитывалась панель, после неудачного вложенного создания директорий)
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
