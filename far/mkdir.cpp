/*
mkdir.cpp

Создание каталога

*/

/* Revision: 1.13 18.03.2002 $ */

/*
Modify:
  18.03.2002 SVS
    + Заюзаем горячие клавиши в промте (FIB_NOAMPERSAND)
  07.12.2001 IS
    + Cоздание нескольких каталогов за раз теперь опционально.
  26.09.2001 SVS
    + Opt.AutoUpdateLimit -  выше этого количество не обновлять пассивную
      панель (если ее содержимое не равно активной).
  24.07.2001 IS
    ! Уточнение предыдущего изменения SVS. Первоначальный предложенный мною
      вариант не учитывал такую ситуацию, как пробелы, а потом имя каталога в
      кавычках. Теперь заключение в кавычки происходит только, если строка не
      содержит разделителей и _кавычек_
  23.07.2001 SVS
    ! уточнение поведения механизма создания каталогов
  05.06.2001 IS
    + Отмена предыдущего патча VVM
  05.06.2001 VVM
    + убрать кавычки в создаваемом каталоге
  04.06.2001 IS
    + научимся создавать несколько каталогов за один раз
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
#include "udlist.hpp"

/* $ 04.06.2001 IS
     научимся создавать несколько каталогов за один раз
*/
void ShellMakeDir(Panel *SrcPanel)
{
  /* $ 02.06.2001 KM
     + Маленькая модификация: добавлены кнопки [ OK ] и [ Cancel ]
       в диалог создания каталога
  */
  char DirName[NM];
  *DirName=0;
  UserDefinedList DirList;

  /* $ 07.12.2001 IS
     создание нескольких каталогов за раз теперь опционально
  */
  BOOL MultiMakeDir=Opt.MultiMakeDir;
  for(;;)
  {
    if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder",
         DirName,DirName,sizeof(DirName),"MakeFolder",
         FIB_NOAMPERSAND|FIB_BUTTONS|FIB_EXPANDENV|FIB_CHECKBOX,&MultiMakeDir,
         MSG(MMultiMakeDir)))
      return;

    Opt.MultiMakeDir=MultiMakeDir;

    // это по поводу создания одиночного каталога, который
    // начинается с пробела! Чтобы ручками не заключать
    // такой каталог в кавычки
    if(Opt.MultiMakeDir && strpbrk(DirName,";,\"") == NULL)
       QuoteSpaceOnly(DirName);

    if(!Opt.MultiMakeDir)   // нужно создать только ОДИН каталог
    {
      Unquote(DirName);     // уберем все лишние кавычки
      InsertQuote(DirName); // возьмем в кавычки, т.к. могут быть разделители
    }

    if(DirList.Set(DirName)) break;
    else Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),
                 MSG(MIncorrectDirList), MSG(MOk));
  }
  /* IS $ */
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
       создания директорий)
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
  int AnotherType=AnotherPanel->GetType();
  if(AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherType==QVIEW_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}
/* IS $ */
