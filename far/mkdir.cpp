/*
mkdir.cpp

—оздание каталога

*/

/* Revision: 1.09 23.07.2001 $ */

/*
Modify:
  23.07.2001 SVS
    ! уточнение поведени€ механизма создани€ каталогов
  05.06.2001 IS
    + ќтмена предыдущего патча VVM
  05.06.2001 VVM
    + убрать кавычки в создаваемом каталоге
  04.06.2001 IS
    + научимс€ создавать несколько каталогов за один раз
  02.06.2001 KM
    + ћаленька€ модификаци€: добавлены кнопки [ OK ] и [ Cancel ]
      в диалог создани€ каталога
  06.05.2001 DJ
    ! перетр€х #include
  29.04.2001 ќ“
    + ¬недрение NWZ от “реть€кова
  31.07.2000 SVS
    ! –асширим переменные среды в диалоге создани€ каталога
  25.07.2000 IG
    - Bug 24 (не перечитывалась панель, после неудачного вложенного создани€ директорий)
  25.06.2000 SVS
    ! ѕодготовка Master Copy
    ! ¬ыделение в качестве самосто€тельного модул€
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
  /* $ 02.06.2001 KM
     + ћаленька€ модификаци€: добавлены кнопки [ OK ] и [ Cancel ]
       в диалог создани€ каталога
  */
  char DirName[NM];
  *DirName=0;
  UserDefinedList DirList;

  for(;;)
  {
    if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder",DirName,DirName,sizeof(DirName),"MakeFolder",FIB_BUTTONS|FIB_EXPANDENV))
      return;

    // это по поводу создани€ одиночного каталога, который
    // начинаетс€ с пробела! „тобы ручками не заключать
    // такой каталог в кавычки
    if(strpbrk(DirName,";,") == NULL)
      if(*DirName != '"')
         QuoteSpaceOnly(DirName);

    // оставил в назидание потомкам. ни в коем случае нельз€ убирать кавычки из
    // DirName, т.к. из-за этого нарушаетс€ логика работы в DirList.Set
    //Unquote(DirName);

    if(DirList.Set(DirName)) break;
    else Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),
                 MSG(MIncorrectDirList), MSG(MOk));
  }
  /* KM $ */
  *DirName=0;
  const char *OneDir;
  DirList.Start();
  while(NULL!=(OneDir=DirList.GetNext()))
  {
    strncpy(DirName, OneDir, sizeof(DirName)-1);
    //Unquote(DirName);
    if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
      LocalStrupr(DirName);

    int Length=strlen(DirName);
    /* $ 25.07.2000 IG
       Bug 24 (не перечитывалась панель, после неудачного вложенного
       создани€ директорий)
    */
    while (Length>0 && DirName[Length-1]==' ')
      Length--;
    DirName[Length]=0;
    if (Length>0 && (DirName[Length-1]=='/' || DirName[Length-1]=='\\'))
      DirName[Length-1]=0;

    char bSuccess = 0;

    int Error=FALSE;

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
  }

  SrcPanel->Update(UPDATE_KEEP_SELECTION);

  if(*DirName)
  {
    char *Slash=strchr(DirName,'\\');
    if (Slash!=NULL)
      *Slash=0;
    SrcPanel->GoToFile(DirName);
  }
  SrcPanel->Redraw();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}
/* IS $ */
