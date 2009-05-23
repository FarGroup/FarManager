/*
filetype.cpp

Работа с ассоциациями файлов
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

#include "filetype.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "vmenu.hpp"
#include "plognmn.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "history.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "CFileMask.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "fnparce.hpp"

static int DeleteTypeRecord(int DeletePos);
static int EditTypeRecord(int EditPos,int TotalRecords,int NewRec);
static int GetDescriptionWidth (const wchar_t *Name=NULL, const wchar_t *ShortName=NULL);

struct FileTypeStringsW
{
    const wchar_t *Help,*HelpModify,
        *Associations,*TypeFmt, *Type0,
        *Execute, *Desc, *Mask, *View, *Edit,
        *AltExec, *AltView, *AltEdit;
};


const FileTypeStringsW FTSW=
{
    L"FileAssoc",L"FileAssocModify",
    L"Associations",L"Associations\\Type%d",L"Associations\\Type",
    L"Execute",L"Description",L"Mask",L"View",L"Edit",
    L"AltExec",L"AltView",L"AltEdit"
};


/* $ 25.04.2001 DJ
   обработка @ в IF EXIST: функция, которая извлекает команду из строки
   с IF EXIST с учетом @ и возвращает TRUE, если условие IF EXIST
   выполено, и FALSE в противном случае/
*/

BOOL ExtractIfExistCommand (string &strCommandText)
{
  const wchar_t *wPtrCmd=PrepareOSIfExist(strCommandText);

  if (wPtrCmd)
  {
    // Во! Условие не выполнено!!!
    // (например, пока рассматривали менюху, в это время)
    // какой-то злобный чебурашка стер файл!
    if (!*wPtrCmd)
      return FALSE;

		size_t offset = wPtrCmd-(const wchar_t*)strCommandText;
		wchar_t *CommandText = strCommandText.GetBuffer();
		wchar_t *PtrCmd = CommandText+offset;
    // прокинем "if exist"
    if (*CommandText == L'@')
      wmemmove(CommandText+1,PtrCmd,StrLength(PtrCmd)+1);
    else
      wmemmove(CommandText,PtrCmd,StrLength(PtrCmd)+1);

    strCommandText.ReleaseBuffer ();
  }

  return TRUE;
}

/* $ 14.01.2001 SVS
   Добавим интелектуальности.
   Если встречается "IF" и оно выполняется, то команда
   помещается в список

   Вызывается для F3, F4 - ассоциации
   Enter в ком строке - ассоциации.
*/
/* $ 06.07.2001
   + Используем CFileMaskW вместо GetCommaWord, этим самым добиваемся того, что
     можно использовать маски исключения
   - Убрал непонятный мне запрет на использование маски файлов типа "*.*"
     (был когда-то, вроде, такой баг-репорт)
*/
int ProcessLocalFileTypes(const wchar_t *Name,const wchar_t *ShortName,int Mode,int AlwaysWaitFinish)
{
  CFileMask FMask; // для работы с масками файлов
  string Commands[32],Descriptions[32],strCommand;
  int NumCommands[32];
  int CommandCount=0;

  RenumKeyRecord(FTSW.Associations,FTSW.TypeFmt,FTSW.Type0);

  for (int I=0;;I++)
  {
    string strRegKey, strMask;
    strRegKey.Format (FTSW.TypeFmt,I);
    if (!GetRegKey(strRegKey,FTSW.Mask,strMask,L""))
      break;
    if(FMask.Set(strMask, FMF_SILENT))
    {
      string strNewCommand;
      if(FMask.Compare(Name))
      {
        switch(Mode)
        {
          case FILETYPE_EXEC:
            GetRegKey(strRegKey,FTSW.Execute,strNewCommand,L"");
            break;
          case FILETYPE_VIEW:
            GetRegKey(strRegKey,FTSW.View,strNewCommand, L"");
            break;
          case FILETYPE_EDIT:
            GetRegKey(strRegKey,FTSW.Edit,strNewCommand,L"");
            break;
          case FILETYPE_ALTEXEC:
            GetRegKey(strRegKey,FTSW.AltExec,strNewCommand,L"");
            break;
          case FILETYPE_ALTVIEW:
            GetRegKey(strRegKey,FTSW.AltView,strNewCommand,L"");
            break;
          case FILETYPE_ALTEDIT:
            GetRegKey(strRegKey,FTSW.AltEdit,strNewCommand,L"");
            break;
          default:
            strNewCommand=L""; // обнулим на всякий пожарный
        }

        if ( !strNewCommand.IsEmpty() && CommandCount<(int)countof(Commands))
        {
          strCommand = strNewCommand;
          Commands[CommandCount] = strCommand;
          GetRegKey(strRegKey,FTSW.Desc,Descriptions[CommandCount],L"");
          CommandCount++;
        }
      }
    }
  }
  if (CommandCount==0)
    return(FALSE);
  if (CommandCount>1)
  {
    MenuItemEx TypesMenuItem;
    VMenu TypesMenu(MSG(MSelectAssocTitle),NULL,0,ScrY-4);


    TypesMenu.SetHelp(FTSW.Help);
    TypesMenu.SetFlags(VMENU_WRAPMODE);
    TypesMenu.SetPosition(-1,-1,0,0);

    int DizWidth=GetDescriptionWidth (Name, ShortName);
    int ActualCmdCount=0; // отображаемых ассоциаций в меню

    for (int I=0;I<CommandCount;I++)
    {
      string strCommandText, strMenuText;

      TypesMenuItem.Clear ();

      strCommandText = Commands[I];
      SubstFileName(strCommandText,Name,ShortName,NULL,NULL,NULL,NULL,TRUE);

      // все "подставлено", теперь проверим условия "if exist"
      if (!ExtractIfExistCommand (strCommandText))
        continue;

      // запомним индекс оригинальной команды из мессива Commands
      NumCommands[ActualCmdCount++]=I;

      if (DizWidth==0)
        strMenuText=L"";
      else
      {
        string strTitle;
        if ( !Descriptions[I].IsEmpty() )
        {
          strTitle = Descriptions[I];
          SubstFileName(strTitle, Name,ShortName,NULL,NULL,NULL,NULL,TRUE);
        }
        else
          strTitle=L"";

        const wchar_t *PtrAmp;
        int Ampersand=(PtrAmp=wcschr(strTitle,L'&'))!=NULL;
        if(DizWidth+Ampersand > ScrX/2 && PtrAmp && PtrAmp-(const wchar_t*)strTitle > DizWidth)
          Ampersand=0;
        strMenuText.Format (L"%-*.*s %c ",DizWidth+Ampersand,DizWidth+Ampersand,(const wchar_t*)strTitle,BoxSymbols[BS_V1]);
      }
      TruncStr(strCommandText,ScrX-DizWidth-14);
      strMenuText += strCommandText;
      TypesMenuItem.strName = strMenuText;
      TypesMenuItem.SetSelect(I==0);
      TypesMenu.AddItem(&TypesMenuItem);
    }

    if(!ActualCmdCount)
      return(TRUE);

    int ExitCode;
    if(ActualCmdCount > 1)
    {
      TypesMenu.Process();
      ExitCode=TypesMenu.Modal::GetExitCode();
      if (ExitCode<0)
        return(TRUE);
    }
    else
      ExitCode=0;
    strCommand = Commands[NumCommands[ExitCode]];
  }

  string strListName, strAnotherListName;
  string strShortListName, strAnotherShortListName;
  {
      int PreserveLFN=SubstFileName(strCommand,Name,ShortName,&strListName,&strAnotherListName, &strShortListName, &strAnotherShortListName);

    // Снова все "подставлено", теперь проверим условия "if exist"
    if (!ExtractIfExistCommand (strCommand))
      return TRUE;

    PreserveLongName PreserveName(ShortName,PreserveLFN);
    if ( !strCommand.IsEmpty() )
    {
      if ( strCommand.At(0) != L'@')
      {
        CtrlObject->CmdLine->ExecString(strCommand,AlwaysWaitFinish);
        if (!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTFARASS) && !AlwaysWaitFinish) //AN
          CtrlObject->CmdHistory->AddToHistory(strCommand);
      }
      else
      {
#if 1
        SaveScreen SaveScr;
        CtrlObject->Cp()->LeftPanel->CloseFile();
        CtrlObject->Cp()->RightPanel->CloseFile();

				Execute(&strCommand[1],AlwaysWaitFinish);
#else
        // здесь была бага с прорисовкой (и... вывод данных
        // на команду "@type !@!" пропадал с экрана)
        // сделаем по аналогии с CommandLine::CmdExecute()
        {
          RedrawDesktop RdrwDesktop(TRUE);
          Execute((const wchar_t*)strCommand+1,AlwaysWaitFinish);
          ScrollScreen(1); // обязательно, иначе деструктор RedrawDesktop
                           // проредравив экран забьет последнюю строку вывода.
        }
        CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
        CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
        CtrlObject->Cp()->Redraw();
#endif
      }
    }
  }

  if ( !strListName.IsEmpty() )
		apiDeleteFile (strListName);

  if ( !strAnotherListName.IsEmpty() )
		apiDeleteFile (strAnotherListName);

  if ( !strShortListName.IsEmpty() )
		apiDeleteFile (strShortListName);

  if ( !strAnotherShortListName.IsEmpty() )
		apiDeleteFile (strAnotherShortListName);

  return(TRUE);
}


int ProcessGlobalFileTypes(const wchar_t *Name,int AlwaysWaitFinish)
{
  string strValue;
  const wchar_t *ExtPtr;
  HKEY hClassesKey;

  if ((ExtPtr=wcsrchr(Name,L'.'))==NULL)
    return(FALSE);

  if (RegOpenKeyW(HKEY_CLASSES_ROOT,ExtPtr,&hClassesKey)!=ERROR_SUCCESS)
      return(FALSE);

/*
  if (RegQueryStringValueEx(hClassesKey,L"",strValue)!=ERROR_SUCCESS)
  {
      RegCloseKey(hClassesKey);
      return(FALSE);
  }
*/
  RegCloseKey(hClassesKey);

  string strFullName;

  ConvertNameToFull(Name,strFullName);

  QuoteSpace(strFullName);

  CtrlObject->CmdLine->ExecString(strFullName,AlwaysWaitFinish,2,FALSE);
  if (!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTWINASS) && !AlwaysWaitFinish) //AN
  {
    string strQuotedName = Name;
    QuoteSpace(strQuotedName);
    CtrlObject->CmdHistory->AddToHistory(strQuotedName);
  }
  return TRUE;
}


/*
  Используется для запуска внешнего редактора и вьювера
*/
void ProcessExternal(const wchar_t *Command,const wchar_t *Name,const wchar_t *ShortName,int AlwaysWaitFinish)
{
  string strExecStr, strFullExecStr;
  string strListName, strAnotherListName;
  string strShortListName, strAnotherShortListName;

  string strFullName, strFullShortName;

  strExecStr = Command;
  strFullExecStr = Command;
  {
    int PreserveLFN=SubstFileName(strExecStr,Name,ShortName,&strListName,&strAnotherListName, &strShortListName, &strAnotherShortListName);
    // Снова все "подставлено", теперь проверим условия "if exist"
    if (!ExtractIfExistCommand (strExecStr))
      return;

    PreserveLongName PreserveName(ShortName,PreserveLFN);

    ConvertNameToFull(Name,strFullName);
    ConvertNameToShort(strFullName,strFullShortName);

    //BUGBUGBUGBUGBUGBUG !!! Same ListNames!!!
    SubstFileName(strFullExecStr,strFullName,strFullShortName,&strListName,&strAnotherListName, &strShortListName, &strAnotherShortListName);
    // Снова все "подставлено", теперь проверим условия "if exist"
    if (!ExtractIfExistCommand (strFullExecStr))
      return;

    CtrlObject->ViewHistory->AddToHistory(strFullExecStr,(AlwaysWaitFinish&1)+2);

    if ( strExecStr.At(0) != L'@')
      CtrlObject->CmdLine->ExecString(strExecStr,AlwaysWaitFinish);
    else
    {
      SaveScreen SaveScr;
      CtrlObject->Cp()->LeftPanel->CloseFile();
      CtrlObject->Cp()->RightPanel->CloseFile();

      Execute(strExecStr,AlwaysWaitFinish);
    }
  }

  if ( !strListName.IsEmpty() )
		apiDeleteFile (strListName);

  if ( !strAnotherListName.IsEmpty() )
		apiDeleteFile (strAnotherListName);

  if ( !strShortListName.IsEmpty() )
		apiDeleteFile (strShortListName);

  if ( !strAnotherShortListName.IsEmpty() )
		apiDeleteFile (strAnotherShortListName);
}

static int FillFileTypesMenu(VMenu *TypesMenu,int MenuPos)
{
  int NumLine=0;
  int DizWidth=GetDescriptionWidth();
  MenuItemEx TypesMenuItem;

  TypesMenu->DeleteItems();
  while (1)
  {
    string strRegKey, strMask, strMenuText;
    strRegKey.Format (FTSW.TypeFmt,NumLine);

    TypesMenuItem.Clear ();

    if (!GetRegKey(strRegKey,FTSW.Mask,strMask,L""))
      break;
    if (DizWidth==0)
      strMenuText=L"";
    else
    {
      string strTitle, strDescription;
      GetRegKey(strRegKey,FTSW.Desc,strDescription,L"");
      if ( !strDescription.IsEmpty() )
        strTitle = strDescription;
      else
        strTitle = L"";
      const wchar_t *PtrAmp;
      int Ampersand=(PtrAmp=wcschr(strTitle,L'&'))!=NULL;
      if(DizWidth+Ampersand > ScrX/2 && PtrAmp && PtrAmp-(const wchar_t*)strTitle > DizWidth)
        Ampersand=0;
      strMenuText.Format (L"%-*.*s %c ",DizWidth+Ampersand,DizWidth+Ampersand,(const wchar_t*)strTitle,BoxSymbols[BS_V1]);
    }
    //TruncStr(strMask,ScrX-DizWidth-14);
    strMenuText += strMask;
    TypesMenuItem.strName = strMenuText;
    TypesMenuItem.SetSelect(NumLine==MenuPos);
    TypesMenu->AddItem(&TypesMenuItem);
    NumLine++;
  }
  TypesMenuItem.strName=L"";
  TypesMenuItem.SetSelect(NumLine==MenuPos);
  TypesMenu->AddItem(&TypesMenuItem);
  return NumLine;
}

void EditFileTypes()
{
  int NumLine=0;
  int MenuPos=0;
  int m;
  BOOL MenuModified;

  RenumKeyRecord(FTSW.Associations,FTSW.TypeFmt,FTSW.Type0);

  VMenu TypesMenu(MSG(MAssocTitle),NULL,0,ScrY-4);
  TypesMenu.SetHelp(FTSW.Help);
  TypesMenu.SetFlags(VMENU_WRAPMODE);
  TypesMenu.SetPosition(-1,-1,0,0);
  TypesMenu.SetBottomTitle(MSG(MAssocBottom));

  {
    while (1)
    {
      MenuModified=TRUE;
      while (!TypesMenu.Done())
      {
        if (MenuModified==TRUE)
        {
          TypesMenu.Hide();
          NumLine=FillFileTypesMenu(&TypesMenu,MenuPos);
          TypesMenu.SetPosition(-1,-1,-1,-1);
          TypesMenu.Show();
          MenuModified=FALSE;
        }

        DWORD Key=TypesMenu.ReadInput();
        MenuPos=TypesMenu.GetSelectPos();
        switch(Key)
        {
          case KEY_NUMDEL:
          case KEY_DEL:
            if (MenuPos<NumLine)
              DeleteTypeRecord(MenuPos);
            MenuModified=TRUE;
            break;
          case KEY_NUMPAD0:
          case KEY_INS:
            EditTypeRecord(MenuPos,NumLine,1);
            MenuModified=TRUE;
            break;
          case KEY_NUMENTER:
          case KEY_ENTER:
          case KEY_F4:
            if (MenuPos<NumLine)
              EditTypeRecord(MenuPos,NumLine,0);
            MenuModified=TRUE;
            break;
          default:
            TypesMenu.ProcessInput();
            break;
        }
      }
      m=TypesMenu.Modal::GetExitCode();
      if (m!=-1)
      {
        MenuPos=m;
        TypesMenu.ClearDone();
        TypesMenu.WriteInput(KEY_F4);
        continue;
      }
      break;
    }
  }
}


int DeleteTypeRecord(int DeletePos)
{
  string strRecText, strItemName, strRegKey;
  strRegKey.Format (FTSW.TypeFmt,DeletePos);
  GetRegKey(strRegKey,FTSW.Mask,strRecText,L"");
  strItemName.Format (L"\"%s\"", (const wchar_t*)strRecText);
  if (Message(MSG_WARNING,2,MSG(MAssocTitle),MSG(MAskDelAssoc),
              strItemName,MSG(MDelete),MSG(MCancel))!=0)
    return(FALSE);
  DeleteKeyRecord(FTSW.TypeFmt,DeletePos);
  return(TRUE);
}

int EditTypeRecord(int EditPos,int TotalRecords,int NewRec)
{
  const wchar_t *HistoryName=L"Masks";

  static struct DialogDataEx EditDlgData[]={
/* 00 */ DI_DOUBLEBOX,3, 1,72,21,0,0,0,0,(const wchar_t *)MFileAssocTitle,
/* 01 */ DI_TEXT,     5, 2, 0, 2,0,0,0,0,(const wchar_t *)MFileAssocMasks,
/* 02 */ DI_EDIT,     5, 3,70, 3,1,(DWORD_PTR)HistoryName,DIF_HISTORY,0,L"",
/* 03 */ DI_TEXT,     5, 4, 0, 4,0,0,0,0,(const wchar_t *)MFileAssocDescr,
/* 04 */ DI_EDIT,     5, 5,70, 5,0,0,0,0,L"",
/* 05 */ DI_TEXT,     3, 6, 0, 6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
/* 06 */ DI_TEXT,     5, 7, 0, 7,0,0,0,0,(const wchar_t *)MFileAssocExec,
/* 07 */ DI_EDIT,     5, 8,70, 8,0,0,0,0,L"",
/* 08 */ DI_TEXT,     5, 9, 0, 9,0,0,0,0,(const wchar_t *)MFileAssocAltExec,
/* 09 */ DI_EDIT,     5,10,70,10,0,0,0,0,L"",
/* 10 */ DI_TEXT,     5,11, 0,11,0,0,0,0,(const wchar_t *)MFileAssocView,
/* 11 */ DI_EDIT,     5,12,70,12,0,0,0,0,L"",
/* 12 */ DI_TEXT,     5,13, 0,13,0,0,0,0,(const wchar_t *)MFileAssocAltView,
/* 13 */ DI_EDIT,     5,14,70,14,0,0,0,0,L"",
/* 14 */ DI_TEXT,     5,15, 0,15,0,0,0,0,(const wchar_t *)MFileAssocEdit,
/* 15 */ DI_EDIT,     5,16,70,16,0,0,0,0,L"",
/* 16 */ DI_TEXT,     5,17, 0,17,0,0,0,0,(const wchar_t *)MFileAssocAltEdit,
/* 17 */ DI_EDIT,     5,18,70,18,0,0,0,0,L"",
/* 18 */ DI_TEXT,     3,19, 0,19,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
/* 19 */ DI_BUTTON,   0,20, 0,20,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
/* 20 */ DI_BUTTON,   0,20, 0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(EditDlgData,EditDlg);

  string strRegKey;
  strRegKey.Format (FTSW.TypeFmt,EditPos);
  if (!NewRec)
  {
    GetRegKey(strRegKey,FTSW.Mask,EditDlg[2].strData,L"");
    GetRegKey(strRegKey,FTSW.Desc,EditDlg[4].strData,L"");
    GetRegKey(strRegKey,FTSW.Execute,EditDlg[7].strData,L"");
    GetRegKey(strRegKey,FTSW.AltExec,EditDlg[9].strData,L"");
    GetRegKey(strRegKey,FTSW.View,EditDlg[11].strData,L"");
    GetRegKey(strRegKey,FTSW.AltView,EditDlg[13].strData,L"");
    GetRegKey(strRegKey,FTSW.Edit,EditDlg[15].strData,L"");
    GetRegKey(strRegKey,FTSW.AltEdit,EditDlg[17].strData,L"");
  }

  {
		Dialog Dlg(EditDlg,countof(EditDlg));
    Dlg.SetHelp(FTSW.HelpModify);
    Dlg.SetPosition(-1,-1,76,23);

    CFileMask FMask;
    for(;;)
    {
      Dlg.ClearDone();
      Dlg.Process();

      if (Dlg.GetExitCode()!=19)
        return(FALSE);
      if ( EditDlg[2].strData.IsEmpty() )
      {
        Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),MSG(MAssocNeedMask), MSG(MOk));
        continue;
      }

      if(FMask.Set(EditDlg[2].strData, 0))
        break;
    }
  }

  if (NewRec)
    InsertKeyRecord(FTSW.TypeFmt,EditPos,TotalRecords);

  SetRegKey(strRegKey,FTSW.Mask,EditDlg[2].strData);
  SetRegKey(strRegKey,FTSW.Desc,EditDlg[4].strData);
  SetRegKey(strRegKey,FTSW.Execute,EditDlg[7].strData);
  SetRegKey(strRegKey,FTSW.AltExec,EditDlg[9].strData);
  SetRegKey(strRegKey,FTSW.View,EditDlg[11].strData);
  SetRegKey(strRegKey,FTSW.AltView,EditDlg[13].strData);
  SetRegKey(strRegKey,FTSW.Edit,EditDlg[15].strData);
  SetRegKey(strRegKey,FTSW.AltEdit,EditDlg[17].strData);

  return(TRUE);
}

int GetDescriptionWidth (const wchar_t *Name, const wchar_t *ShortName)
{
  int Width=0,NumLine=0;
  RenumKeyRecord(FTSW.Associations,FTSW.TypeFmt,FTSW.Type0);
  while (1)
  {
    CFileMask FMask;

    string strRegKey, strMask, strDescription;
    strRegKey.Format (FTSW.TypeFmt, NumLine);
    if (!GetRegKey(strRegKey,FTSW.Mask, strMask, L""))
      break;
    NumLine++;

    if(!FMask.Set(strMask, FMF_SILENT))
      continue;

    GetRegKey(strRegKey,FTSW.Desc,strDescription,L"");
    int CurWidth;
    if (Name == NULL)
      CurWidth = HiStrlen(strDescription);
    else
    {
      if(!FMask.Compare(Name))
        continue;
      string strExpandedDesc;

      strExpandedDesc = strDescription;
      SubstFileName(strExpandedDesc,Name,ShortName,NULL,NULL,NULL,NULL,TRUE);
      CurWidth = HiStrlen (strExpandedDesc);
    }
    if (CurWidth>Width)
      Width=CurWidth;
  }
  if (Width>ScrX/2)
    Width=ScrX/2;
  return(Width);
}
