/*
usermenu.cpp

User menu и есть
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

#include "lang.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "plognmn.hpp"
#include "savefpos.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "usermenu.hpp"
#include "filetype.hpp"
#include "fnparce.hpp"
#include "execute.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "filestr.hpp"
#include "mix.hpp"

#if defined(PROJECT_DI_MEMOEDIT)
/*
  Идея в следующем.
  1. Строки в реестре храняться как и раньше, т.к. CommandXXX
  2. Для DI_MEMOEDIT мы из только преобразовываем в один массив
*/
#endif

static int ProcessSingleMenu(const wchar_t *MenuKey,int MenuPos,const wchar_t *Title=NULL);
static int FillUserMenu(VMenu& UserMenu, const wchar_t *MenuKey,int MenuPos,int *FuncPos,const wchar_t *Name,const wchar_t *ShortName);
static int DeleteMenuRecord(const wchar_t *MenuKey,int DeletePos);
static int EditMenuRecord(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec);
static int EditSubMenu(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec);
static void MenuRegToFile(const wchar_t *MenuKey,FILE *MenuFile);
static void MenuFileToReg(const wchar_t *MenuKey,FILE *MenuFile);

UINT MenuCP=CP_OEMCP;

static int MenuModified;
static int MenuNeedRefresh;
static string strMenuRootKey, strLocalMenuKey;

/* $ 14.07.2000 VVM
   + Режимы показа меню (Menu mode) и Коды выхода из меню (Exit codes)
*/
enum {MM_LOCAL=0,           // Локальное меню
      MM_FAR=1,             // Меню из каталога ФАРа
      MM_MAIN=2};           // Главное меню

enum {EC_CLOSE_LEVEL      = -1,   // Выйти из меню на один уровень вверх
      EC_CLOSE_MENU       = -2,   // Выйти из меню по SHIFT+F10
      EC_PARENT_MENU      = -3,   // Показать меню родительского каталога
      EC_MAIN_MENU        = -4,   // Показать главное меню
      EC_COMMAND_SELECTED = -5};  // Выбрана команда - закрыть меню и
                                  // обновить папку

static int MenuMode;

static wchar_t SubMenuSymbol[]={0x0020,0x25BA,0x0000};

const wchar_t LocalMenuFileName[]=L"FarMenu.Ini";

void ProcessUserMenu(int EditMenu)
{
  FILE *MenuFile;

  string strMenuFilePath;    // Путь к текущему каталогу с файлом LocalMenuFileName
  string strMenuFileFullPath;
  int  ExitCode = 0;
  int RunFirst  = 1;

  CtrlObject->CmdLine->GetCurDir(strMenuFilePath);

  strLocalMenuKey.Format (L"UserMenu\\LocalMenu%u",clock());
  MenuMode=MM_LOCAL;

  DeleteKeyTree(strLocalMenuKey);

  MenuModified=MenuNeedRefresh=FALSE;

  if (EditMenu)
  {
    int EditChoice=Message(0,3,MSG(MUserMenuTitle),MSG(MChooseMenuType),
                   MSG(MChooseMenuMain),MSG(MChooseMenuLocal),MSG(MCancel));
    if (EditChoice<0 || EditChoice==2)
      return;
    if (EditChoice==0)
    {
      MenuMode=MM_FAR;
      strMenuFilePath = g_strFarPath;
    }
  }

  while((ExitCode != EC_CLOSE_LEVEL) && (ExitCode != EC_CLOSE_MENU) &&
      (ExitCode != EC_COMMAND_SELECTED))
  {
    strMenuFileFullPath = strMenuFilePath;
    AddEndSlash(strMenuFileFullPath);
    strMenuFileFullPath += LocalMenuFileName;

    if (MenuMode!=MM_MAIN)
    {
      // Пытаемся открыть файл на локальном диске
      if ((MenuFile=_wfopen(strMenuFileFullPath,L"rb"))!=NULL)
      {
        MenuFileToReg(strLocalMenuKey, MenuFile);
        fclose(MenuFile);
      }
      else
      {
        // Файл не открылся. Смотрим дальше.
        if (MenuMode==MM_FAR)
          MenuMode=MM_MAIN;
        else
        {
          if (!EditMenu)
          {
            if (!RunFirst)
            {
              size_t pos;
							if (LastSlash(strMenuFilePath,pos))
              {
                strMenuFilePath.SetLength(pos--);
                if (strMenuFilePath.At(pos)!=L':')
                  continue;
              }
            }
            RunFirst=0;
            strMenuFilePath = g_strFarPath;
            MenuMode=MM_FAR;
            continue;
          }
        }
      }
    }


    strMenuRootKey =(MenuMode==MM_MAIN) ? L"UserMenu\\MainMenu":strLocalMenuKey;

    int PrevMacroMode=CtrlObject->Macro.GetMode();
    int _CurrentFrame=FrameManager->GetCurrentFrame()->GetType();
    CtrlObject->Macro.SetMode(MACRO_USERMENU);
    ExitCode=ProcessSingleMenu(strMenuRootKey, 0);
    if(_CurrentFrame == FrameManager->GetCurrentFrame()->GetType()) //???
      CtrlObject->Macro.SetMode(PrevMacroMode);

    // Фаровский кусок по записи файла
    if ((MenuMode!=MM_MAIN) && (MenuModified))
    {
			DWORD FileAttr=apiGetFileAttributes(strMenuFileFullPath);
      if (FileAttr!=INVALID_FILE_ATTRIBUTES)
      {
        if (FileAttr & FILE_ATTRIBUTE_READONLY)
        {
          int AskOverwrite;
          AskOverwrite=Message(MSG_WARNING,2,MSG(MUserMenuTitle),LocalMenuFileName,
                       MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo));
          if (AskOverwrite==0)
						apiSetFileAttributes(strMenuFileFullPath,FileAttr & ~FILE_ATTRIBUTE_READONLY);
        }
        if (FileAttr & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
					apiSetFileAttributes(strMenuFileFullPath,FILE_ATTRIBUTE_NORMAL);
      }
      if ((MenuFile=_wfopen(strMenuFileFullPath,L"wb"))!=NULL)
      {
        MenuRegToFile(strLocalMenuKey,MenuFile);
        long Length=filelen(MenuFile);
        fclose(MenuFile);
        if (Length==0)
					apiDeleteFile (strMenuFileFullPath);
      }
    }
    if (MenuMode!=MM_MAIN)
      DeleteKeyTree(strLocalMenuKey);

    switch(ExitCode)
    {
      case EC_PARENT_MENU:
      {
        if (MenuMode==MM_LOCAL)
        {
          size_t pos;
					if(LastSlash(strMenuFilePath,pos))
          {
            strMenuFilePath.SetLength(pos--);
            if (strMenuFilePath.At(pos)!=L':')
              continue;
          }

          strMenuFilePath = g_strFarPath;
          MenuMode=MM_FAR;
        }
        else
          MenuMode=MM_MAIN;
        break;
      }

      case EC_MAIN_MENU:
      {
        // $ 14.07.2000 VVM + SHIFT+F2 переключает Главное меню/локальное в цикле
        switch(MenuMode)
        {
          case MM_LOCAL:
          {
            strMenuFilePath = g_strFarPath;
            MenuMode=MM_FAR;
            break;
          }
          case MM_FAR:
          {
            MenuMode=MM_MAIN;
            break;
          }

          default: // MM_MAIN
          {
            CtrlObject->CmdLine->GetCurDir(strMenuFilePath);
            MenuMode=MM_LOCAL;
          }
        }

        break;
      }
    }
  }

  if (FrameManager->IsPanelsActive() && (ExitCode == EC_COMMAND_SELECTED || MenuModified))
  {
    ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
  }
}


int FillUserMenu(VMenu& UserMenu,const wchar_t *MenuKey,int MenuPos,int *FuncPos,const wchar_t *Name,const wchar_t *ShortName)
{
  int NumLine;
  MenuItemEx UserMenuItem;

  UserMenuItem.Clear ();

  UserMenu.DeleteItems();

  /* $ 20.07.2000 tran
     + лишний проход для вычисления максимальной длины строки */
  int MaxLen=20;
  BOOL HotKeyPresent=FALSE;

  // лишний проход - выясняем есть ли хотя бы один хоткей
  NumLine=0;
  while (1)
  {
    string strItemKey, strHotKey;
    strItemKey.Format (L"%s\\Item%d",MenuKey,NumLine);
    if(!GetRegKey(strItemKey,L"HotKey",strHotKey,L""))
      break;
    if( !strHotKey.IsEmpty() )
    {
      HotKeyPresent=TRUE;
      break;
    }
    NumLine++;
  }

  NumLine=0;
  while (1)
  {
    int MenuTextLen;
    string strItemKey, strHotKey, strLabel, strMenuText;
    strItemKey.Format (L"%s\\Item%d", MenuKey,NumLine);
    if (!GetRegKey(strItemKey,L"HotKey",strHotKey,L""))
      break;
    if (!GetRegKey(strItemKey,L"Label",strLabel,L""))
      break;
    SubstFileName(strLabel,Name,ShortName,NULL,NULL,NULL,NULL,TRUE);

    apiExpandEnvironmentStrings (strLabel, strLabel);

    int FuncNum=0;
    if ( strHotKey.GetLength()>1)
    {
      FuncNum=_wtoi((const wchar_t*)strHotKey+1);
      if (FuncNum<1 || FuncNum>12)
        FuncNum=1;
      strHotKey.Format (L"F%d",FuncNum);
    }
    else
      if( strHotKey.At(0) == L'&')
         strHotKey += L"&";

    /* $ 14.10.2000 VVM
       + Разделитель меню, если Метка пуста, а ХотКей="-"
    */
    if ( (StrLength(strLabel)==0) && (StrCmp(strHotKey, L"-")==0))
    {
     // Nothing to do
    }
    else
    {
      /* $ 20.08.2001 VVM
        ! Хоткей в метке работает при Fx (возвращаем, как було)
        ! Ошибка при выравнивании указателя подменю. */
//      if(FuncNum>0) // Для Fx ограничим хоткеи
//        sprintf(MenuText,"%-3.3s& %-20.*s",HotKey,ScrX-12,Label);
//      else
//        sprintf(MenuText,"%s%-3.3s %-20.*s",(*HotKey?"&":""),HotKey,ScrX-12,Label);
      if(HotKeyPresent)
      {
        int AddHotKey = ( !strHotKey.IsEmpty() ) && (!FuncNum);
        strMenuText.Format (L"%s%-*.*s %-20.*s%s",
                             (AddHotKey?L"&":L""),
                             ( strHotKey.At(0)==L'&'?4:3),( strHotKey.At(0)==L'&'?4:3), (const wchar_t*)strHotKey,
                             ScrX-12,
                             (const wchar_t*)strLabel,
                             ((wcschr(strLabel, L'&')==NULL)||(AddHotKey))?L"":L" ");
      }
      else
      {
        const wchar_t *Ptr=(wcschr(strLabel, L'&')==NULL?L"":L" ");
        strMenuText.Format (L"%-20.*s%s",ScrX-12,(const wchar_t*)strLabel,Ptr);
      }

      MenuTextLen=(int)strMenuText.GetLength()-(FuncNum>0?1:0);
      MaxLen=(MaxLen<MenuTextLen ? MenuTextLen : MaxLen);
    } /* else */

    NumLine++;
  }

  // коррекция максимальной длины
  if(MaxLen > ScrX-14) // по полной программе!
    MaxLen = ScrX-14;

  NumLine=0;
  while (1)
  {
    UserMenuItem.Clear ();

    string strItemKey, strHotKey, strLabel, strMenuText;
    strItemKey.Format (L"%s\\Item%d", (const wchar_t*)MenuKey,NumLine);
    if (!GetRegKey(strItemKey,L"HotKey",strHotKey,L""))
      break;
    if (!GetRegKey(strItemKey,L"Label",strLabel,L""))
      break;
    SubstFileName(strLabel,Name,ShortName,NULL,NULL,NULL,NULL,TRUE);

    apiExpandEnvironmentStrings (strLabel, strLabel);

    int SubMenu;
    GetRegKey(strItemKey,L"Submenu",SubMenu,0);

    int FuncNum=0;
    if ( StrLength(strHotKey)>1)
    {
      FuncNum=_wtoi((const wchar_t*)strHotKey+1);
      if (FuncNum<1 || FuncNum>12)
        FuncNum=1;
      strHotKey.Format (L"F%d",FuncNum);
    }
    else
      if( strHotKey.At(0)== L'&')
        strHotKey += L"&";

    /* $ 14.10.2000 VVM
       + Разделитель меню, если Метка пуста, а ХотКей="-"
    */
    if ((StrLength(strLabel)==0) && (StrCmp(strHotKey,L"-")==0))
    {
      UserMenuItem.Flags|=LIF_SEPARATOR;
      UserMenuItem.Flags&=~LIF_SELECTED;
      UserMenuItem.strName=L"";
      if (NumLine==MenuPos)
        MenuPos++;
    }
    else
    {
    /* $ 20.07.2000 tran
       %-20.*s поменял на %-*.*s и используется MaxLen как максимальная длина */
      /* $ 20.08.2001 VVM
        ! Хоткей в метке работает при Fx (возвращаем, как було)
        ! Ошибка при выравнивании указателя подменю. */
//      if(FuncNum>0) // Для Fx ограничим хоткеи
//        sprintf(MenuText,"%-3.3s& %-*.*s",HotKey,MaxLen,MaxLen,Label);
//      else
//        sprintf(MenuText,"%s%-3.3s %-*.*s",(*HotKey?"&":""),HotKey,MaxLen,MaxLen,Label);
      if(HotKeyPresent)
      {
        int AddHotKey = ( !strHotKey.IsEmpty() ) && (!FuncNum);
        strMenuText.Format (L"%s%-*.*s %-*.*s%s",
                             (AddHotKey?L"&":L""),
                             ( strHotKey.At(0)==L'&'?4:3),( strHotKey.At(0)==L'&'?4:3),(const wchar_t*)strHotKey,
                             MaxLen,MaxLen,(const wchar_t*)strLabel,
                             ((wcschr(strLabel, L'&')==NULL)||(AddHotKey))?L"":L" ");
      }
      else
      {
        const wchar_t *Ptr=(wcschr(strLabel, L'&')==NULL?L"":L" ");
        strMenuText.Format (L"%-*.*s%s",MaxLen,MaxLen,(const wchar_t*)strLabel,Ptr);
      }

      if (SubMenu)
      {
        strMenuText += SubMenuSymbol;
//_SVS(SysLog(L"%2d - '%s'",HiStrlen(MenuText),MenuText));
      }
      UserMenuItem.strName = strMenuText;
      UserMenuItem.SetSelect(NumLine==MenuPos);
      UserMenuItem.Flags&=~LIF_SEPARATOR;
    }
    int ItemPos=UserMenu.AddItem(&UserMenuItem);
    if (FuncNum>0)
      FuncPos[FuncNum-1]=ItemPos;
    NumLine++;
  }

  UserMenuItem.strName=L"";
  UserMenuItem.Flags&=~LIF_SEPARATOR;
  UserMenuItem.SetSelect(NumLine==MenuPos);
  UserMenu.AddItem(&UserMenuItem);
  return NumLine;
}

int ProcessSingleMenu(const wchar_t *MenuKey,int MenuPos,const wchar_t *Title)
{
  MenuItemEx UserMenuItem;

  while (1)
  {
    UserMenuItem.Clear ();
    int NumLine=0,ExitCode,FuncPos[12];

    for (size_t I=0;I<countof(FuncPos);I++)
      FuncPos[I]=-1;

    string strName,strShortName;
    CtrlObject->Cp()->ActivePanel->GetCurName(strName,strShortName);
    {
      /* $ 24.07.2000 VVM
       + При показе главного меню в заголовок добавляет тип - FAR/Registry
      */
      string strMenuTitle;
      if(Title && *Title)
        strMenuTitle = Title;
      else
        switch (MenuMode)
        {
        case MM_LOCAL:
          strMenuTitle = MSG(MLocalMenuTitle);
          break;
        case MM_FAR:
          strMenuTitle.Format (L"%s (%s)",MSG(MMainMenuTitle),MSG(MMainMenuFAR));
          break;
        default:
          {
            const wchar_t *Ptr=MSG(MMainMenuREG);
            if(*Ptr)
              strMenuTitle.Format (L"%s (%s)",MSG(MMainMenuTitle),Ptr);
            else
              strMenuTitle.Format (L"%s", MSG(MMainMenuTitle));
          }
        } /* switch */
      VMenu UserMenu(strMenuTitle,NULL,0,ScrY-4);

      UserMenu.SetFlags(VMENU_WRAPMODE);
      UserMenu.SetHelp(L"UserMenu");
      UserMenu.SetPosition(-1,-1,0,0);
      UserMenu.SetBottomTitle(MSG(MMainMenuBottomTitle));

//      NumLine=FillUserMenu(UserMenu,MenuKey,MenuPos,FuncPos,Name,ShortName);

      {
        MenuNeedRefresh=TRUE;
        while (!UserMenu.Done())
        {
          if (MenuNeedRefresh)
          {
            UserMenu.Hide(); // спрячем
            // "изнасилуем" (перезаполним :-)
            NumLine=FillUserMenu(UserMenu,MenuKey,MenuPos,FuncPos,strName,strShortName);
            // заставим манагер менюхи корректно отрисовать ширину и
            // высоту, а заодно и скорректировать вертикальные позиции
            UserMenu.SetPosition(-1,-1,-1,-1);
            UserMenu.Show();
            MenuNeedRefresh=FALSE;
          }
          int Key=UserMenu.ReadInput();
          MenuPos=UserMenu.GetSelectPos();
          if ((unsigned int)Key>=KEY_F1 && (unsigned int)Key<=KEY_F12)
          {
            int FuncItemPos;
            if ((FuncItemPos=FuncPos[Key-KEY_F1])!=-1)
            {
              UserMenu.Modal::SetExitCode(FuncItemPos);
              continue;
            }
          }
          else if(Key == L' ') // исключаем пробел из "хоткеев"!
            continue;

          switch(Key)
          {
            /* $ 24.08.2001 VVM
              + Стрелки вправо/влево открывают/закрывают подменю соответственно */
            case KEY_RIGHT:
            case KEY_NUMPAD6:
            case KEY_MSWHEEL_RIGHT:
            {
              string strCurrentKey;
              int SubMenu;
              strCurrentKey.Format (L"%s\\Item%d",MenuKey,MenuPos);
              GetRegKey(strCurrentKey,L"Submenu",SubMenu,0);
              if (SubMenu)
                UserMenu.SetExitCode(MenuPos);
              break;
            }
            case KEY_LEFT:
            case KEY_NUMPAD4:
            case KEY_MSWHEEL_LEFT:
              if (Title && *Title)
                UserMenu.SetExitCode(-1);
              break;
            case KEY_NUMDEL:
            case KEY_DEL:
              if (MenuPos<NumLine)
                DeleteMenuRecord(MenuKey,MenuPos);
//              MenuModified=TRUE;
              break;
            case KEY_INS:
            case KEY_F4:
            case KEY_SHIFTF4:
            case KEY_NUMPAD0:
              if (Key != KEY_INS && Key != KEY_NUMPAD0 && MenuPos>=NumLine)
                break;
              EditMenuRecord(MenuKey,MenuPos,NumLine,Key == KEY_INS || Key == KEY_NUMPAD0);
//              MenuModified=TRUE;
              break;
            case KEY_ALTF4:
            	{
								(*FrameManager)[0]->Unlock();
								FILE *MenuFile;
								string strMenuFileName;
								if (!FarMkTempEx(strMenuFileName) || (MenuFile=_wfopen(strMenuFileName,L"wb"))==NULL)
									break;
								MenuRegToFile(strMenuRootKey,MenuFile);
								MenuNeedRefresh=TRUE;
								fclose(MenuFile);

								{
									ConsoleTitle *OldTitle=new ConsoleTitle;
									string strFileName = strMenuFileName;
									FileEditor ShellEditor(strFileName,CP_UNICODE,FFILEEDIT_DISABLEHISTORY,-1,-1,NULL);
									delete OldTitle;
									ShellEditor.SetDynamicallyBorn(false);
									FrameManager->EnterModalEV();
									FrameManager->ExecuteModal();
									FrameManager->ExitModalEV();
									if (!ShellEditor.IsFileChanged() || (MenuFile=_wfopen(strMenuFileName,L"rb"))==NULL)
									{
										apiDeleteFile(strMenuFileName);
										return(0);
									}
								}

								DeleteKeyTree(strMenuRootKey);
								MenuFileToReg(strMenuRootKey,MenuFile);
								fclose(MenuFile);
								apiDeleteFile (strMenuFileName);
								/* $ 14.12.2001 IS Меню изменили, зачем же это скрывать? */
								MenuModified=TRUE;
								UserMenu.Hide();
							}
              return(0); // Закрыть меню

            /* $ 28.06.2000 tran
               выход из пользовательского меню по ShiftF10 из любого уровня
               вложенности просто задаем ExitCode -1, и возвращаем FALSE -
               по FALSE оно и выйдет откуда угодно */
            case KEY_SHIFTF10:
//              UserMenu.SetExitCode(-1);
              return(EC_CLOSE_MENU);
            case KEY_SHIFTF2: // Показать главное меню
                return(EC_MAIN_MENU);
            case KEY_BS: // Показать меню из родительского каталога только в MM_LOCAL режиме
              if (MenuMode!=MM_MAIN)
                return(EC_PARENT_MENU);
            default:
              UserMenu.ProcessInput();
              if(Key == KEY_F1)
              {
                MenuNeedRefresh=TRUE;
              }
              break;
          }
        }
      }
      ExitCode=UserMenu.Modal::GetExitCode();
    }

    if (ExitCode<0 || ExitCode>=NumLine)
      return(EC_CLOSE_LEVEL); //  вверх на один уровень

    string strCurrentKey;
    int SubMenu;
    strCurrentKey.Format (L"%s\\Item%d",MenuKey,ExitCode);
    GetRegKey(strCurrentKey,L"Submenu",SubMenu,0);

    if (SubMenu)
    {
      /* $ 20.08.2001 VVM
        + При вложенных меню показывает заголовки предыдущих */
      string strSubMenuKey, strSubMenuLabel, strSubMenuTitle;
      strSubMenuKey.Format (L"%s\\Item%d",MenuKey,ExitCode);

      if(GetRegKey(strSubMenuKey,L"Label",strSubMenuLabel,L""))
      {
        SubstFileName(strSubMenuLabel,strName,strShortName,NULL,NULL,NULL,NULL,TRUE);
        apiExpandEnvironmentStrings (strSubMenuLabel, strSubMenuLabel);

        size_t pos;
        if (strSubMenuLabel.Pos(pos,L'&'))
          strSubMenuLabel.LShift(1,pos);

        if (Title && *Title)
          strSubMenuTitle.Format (L"%s -> %s", Title, (const wchar_t*)strSubMenuLabel);
        else
          strSubMenuTitle = strSubMenuLabel;
      } /* if */

      /* $ 14.07.2000 VVM
         ! Если закрыли подменю, то остаться. Инече передать управление выше
      */
      MenuPos=ProcessSingleMenu(strSubMenuKey,0,strSubMenuTitle);
      if (MenuPos!=EC_CLOSE_LEVEL)
        return(MenuPos);
      MenuPos=ExitCode;
      continue;
    }

    /* $ 01.05.2001 IS
         Отключим до лучших времен
    */
    //int LeftVisible,RightVisible,PanelsHidden=0;
    int CurLine=0;

    string strCmdLineDir;
    CtrlObject->CmdLine->GetCurDir(strCmdLineDir);

    string strOldCmdLine;
    CtrlObject->CmdLine->GetString(strOldCmdLine);
    int OldCmdLineCurPos = CtrlObject->CmdLine->GetCurPos();
    int OldCmdLineLeftPos = CtrlObject->CmdLine->GetLeftPos();
    int OldCmdLineSelStart, OldCmdLineSelEnd;
    CtrlObject->CmdLine->GetSelection(OldCmdLineSelStart,OldCmdLineSelEnd);
    CtrlObject->CmdLine->LockUpdatePanel(TRUE);
    while (1)
    {
      string strLineName, strCommand;
      strLineName.Format (L"Command%d",CurLine);
      if (!GetRegKey(strCurrentKey,strLineName,strCommand,L""))
        break;

      string strListName, strAnotherListName;
      string strShortListName, strAnotherShortListName;
      if (StrCmpNI (strCommand,L"REM ",4) && StrCmpNI(strCommand,L"::",2))
      {
        /*
          Осталось корректно обработать ситуацию, например:
          if exist !#!\!^!.! far:edit < diff -c -p !#!\!^!.! !\!.!
          Т.е. сначала "вычислить" кусок "if exist !#!\!^!.!", ну а если
          выполнится, то делать дальше.
          Или еще пример,
          if exist ..\a.bat D:\FAR\170\DIFF.MY\mkdiff.bat !?&Номер патча?!
          ЭТО выполняется всегда, т.к. парсинг всей строки идет, а надо
          проверить фазу "if exist ..\a.bat", а уж потом делать выводы...
        */
        //if(ExtractIfExistCommand(Command))
        {
          int PreserveLFN=SubstFileName(strCommand,strName,strShortName,&strListName,&strAnotherListName, &strShortListName,&strAnotherShortListName, FALSE, strCmdLineDir);
          PreserveLongName PreserveName(strShortName,PreserveLFN);
          /* $ 01.05.2001 IS
             Отключим до лучших времен
          */
          /*if (!PanelsHidden)
          {
            LeftVisible=CtrlObject->Cp()->LeftPanel->IsVisible();
            RightVisible=CtrlObject->Cp()->RightPanel->IsVisible();
            CtrlObject->Cp()->LeftPanel->Hide();
            CtrlObject->Cp()->RightPanel->Hide();
            CtrlObject->Cp()->LeftPanel->SetUpdateMode(FALSE);
            CtrlObject->Cp()->RightPanel->SetUpdateMode(FALSE);
            PanelsHidden=TRUE;
          }*/
//          ;
//_SVS(SysLog(L"!%s!",Command));
          if(ExtractIfExistCommand(strCommand))
          {
            if ( !strCommand.IsEmpty() )
            {
              ProcessOSAliases(strCommand);
              CtrlObject->CmdLine->ExecString(strCommand,FALSE);
            }
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

      CurLine++;
    }
    CtrlObject->CmdLine->LockUpdatePanel(FALSE);
    if( !strOldCmdLine.IsEmpty() ) // восстановим сохраненную командную строку
    {
       CtrlObject->CmdLine->SetString(strOldCmdLine, FrameManager->IsPanelsActive());
       CtrlObject->CmdLine->SetCurPos(OldCmdLineCurPos, OldCmdLineLeftPos);
       CtrlObject->CmdLine->Select(OldCmdLineSelStart, OldCmdLineSelEnd);
    }

    /* $ 01.05.2001 IS
         Отключим до лучших времен
    */
    /*if (PanelsHidden)
    {
      CtrlObject->Cp()->LeftPanel->SetUpdateMode(TRUE);
      CtrlObject->Cp()->RightPanel->SetUpdateMode(TRUE);
      CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
      CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
      if (RightVisible)
        CtrlObject->Cp()->RightPanel->Show();
      if (LeftVisible)
        CtrlObject->Cp()->LeftPanel->Show();
    }*/

/* $ 14.07.2000 VVM
   ! Закрыть меню
*/
/* $ 25.04.2001 DJ
   сообщаем, что была выполнена команда (нужно перерисовать панели)
*/
    return(EC_COMMAND_SELECTED);
  }
}


int DeleteMenuRecord(const wchar_t *MenuKey,int DeletePos)
{
  string strRecText, strItemName, strRegKey;
  int SubMenu;
  strRegKey.Format (L"%s\\Item%d",MenuKey,DeletePos);
  GetRegKey(strRegKey,L"Label",strRecText,L"");
  GetRegKey(strRegKey,L"Submenu",SubMenu,0);
  strItemName.Format (L"\"%s\"", (const wchar_t*)strRecText);
  if (Message(MSG_WARNING,2,MSG(MUserMenuTitle),
          MSG(!SubMenu?MAskDeleteMenuItem:MAskDeleteSubMenuItem),
              strItemName,MSG(MDelete),MSG(MCancel))!=0)
    return(FALSE);
  MenuModified=MenuNeedRefresh=TRUE;
  strRegKey.Format (L"%s\\Item%%d", MenuKey);
  DeleteKeyRecord(strRegKey,DeletePos);
  return(TRUE);
}

/* $ 29.08.2001 VVM
  + Добавим немного логики на закрытие диалога */
// возвращает: 0 -все ОБИ, 1 или 2 - ошибка и, соответственно, на какой контрол поставить фокус
int CanCloseDialog(const wchar_t *Hotkey, const wchar_t *Label)
{
  if (StrCmp(Hotkey,L"-") == 0)
    return 0;
  if (StrLength(Label) == 0)
    return 2;
  if (StrLength(Hotkey) < 2)
    return 0;
  /* Проверить на правильность задания функциональной клавиши */
  int FuncNum=_wtoi(Hotkey+1);
  if (((*Hotkey == L'f') || (*Hotkey == L'F')) &&
      ((FuncNum > 0) && (FuncNum < 13)))
    return 0;
  return 1;
}

static LONG_PTR WINAPI UserMenuDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
#if defined(PROJECT_DI_MEMOEDIT)
  Dialog* Dlg=(Dialog*)hDlg;
  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      break;
    }
  }
#endif
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int EditMenuRecord(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogDataEx EditDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,72,20,0,0,0,0,(const wchar_t *)MEditMenuTitle,
  /* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MEditMenuHotKey,
  /* 02 */DI_FIXEDIT,5,3,7,3,1,0,0,0,L"",
  /* 03 */DI_TEXT,5,4,0,4,0,0,0,0,(const wchar_t *)MEditMenuLabel,
  /* 04 */DI_EDIT,5,5,70,5,0,0,0,0,L"",
  /* 05 */DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 06 */DI_TEXT,5,7,0,7,0,0,0,0,(const wchar_t *)MEditMenuCommands,
#if defined(PROJECT_DI_MEMOEDIT)
  /* 07 */DI_MEMOEDIT,5, 8,70,17,0,0,0,0,L"",
  /* 08 */DI_TEXT,3,18,0,18,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 09 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 10 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
#else
  /* 07 */DI_EDIT,5, 8,70,8,0,0,DIF_EDITOR,0,L"",
  /* 08 */DI_EDIT,5, 9,70,9,0,0,DIF_EDITOR,0,L"",
  /* 09 */DI_EDIT,5,10,70,10,0,0,DIF_EDITOR,0,L"",
  /* 10 */DI_EDIT,5,11,70,11,0,0,DIF_EDITOR,0,L"",
  /* 11 */DI_EDIT,5,12,70,12,0,0,DIF_EDITOR,0,L"",
  /* 12 */DI_EDIT,5,13,70,13,0,0,DIF_EDITOR,0,L"",
  /* 13 */DI_EDIT,5,14,70,14,0,0,DIF_EDITOR,0,L"",
  /* 14 */DI_EDIT,5,15,70,15,0,0,DIF_EDITOR,0,L"",
  /* 15 */DI_EDIT,5,16,70,16,0,0,DIF_EDITOR,0,L"",
  /* 16 */DI_EDIT,5,17,70,17,0,0,DIF_EDITOR,0,L"",
  /* 17 */DI_TEXT,3,18,0,18,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 18 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 19 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
#endif

  };
  MakeDialogItemsEx(EditDlgData,EditDlg);

  int I;
  string strItemKey;
  strItemKey.Format (L"%s\\Item%d", MenuKey,EditPos);

  MenuModified=MenuNeedRefresh=TRUE;

  if (NewRec)
  {
    switch (Message(0,2,MSG(MUserMenuTitle),MSG(MAskInsertMenuOrCommand),
                    MSG(MMenuInsertCommand),MSG(MMenuInsertMenu)))
    {
      case -1:
      case -2:
        return(FALSE);
      case 1:
        return(EditSubMenu(MenuKey,EditPos,TotalRecords,TRUE));
    }
  }
  else
  {
    int SubMenu;
    GetRegKey(strItemKey,L"Submenu",SubMenu,0);
    if (SubMenu)
      return(EditSubMenu(MenuKey,EditPos,TotalRecords,FALSE));
    GetRegKey(strItemKey,L"HotKey",EditDlg[2].strData,L"");
    GetRegKey(strItemKey,L"Label",EditDlg[4].strData,L"");
#if defined(PROJECT_DI_MEMOEDIT)
    /*
      ...
      здесь добавка строк из "Command%d" в 7-й итем
      ...
    */
    string strBuffer7;
    int CommandNumber=0;
    while (1)
    {
      string strCommandName, strCommand;
      strCommandName.Format (L"Command%d",CommandNumber);
      if (!GetRegKey(strItemKey,strCommandName,strCommand,L""))
        break;
      strBuffer7+=strCommand;
      strBuffer7+=L"\n";    //??? "\n\r"
      CommandNumber++;
    }
    EditDlg[7].strData = strBuffer7; //???

#else
    int CommandNumber=0;
    while (CommandNumber<10)
    {
      string strCommandName, strCommand;
      strCommandName.Format (L"Command%d",CommandNumber);
      if (!GetRegKey(strItemKey,strCommandName,strCommand,L""))
        break;
      EditDlg[7+CommandNumber].strData = strCommand;
      CommandNumber++;
    }
#endif
  }

  {
		Dialog Dlg(EditDlg,countof(EditDlg),UserMenuDlgProc);
    Dlg.SetHelp(L"UserMenu");
    Dlg.SetPosition(-1,-1,76,22);
    /* $ 22.12.2000 IS
       ! Если не ввели метку и нажали "продолжить", то не выходим из диалога
         редактирования команд, т.к. теряем те команды, что, возможно, ввели.
         Для выхода из меню нужно воспользоваться esc или кнопкой "отменить".
    */
    while(1)
    {
      Dlg.Process();
#if defined(PROJECT_DI_MEMOEDIT)
  #define DLGOK_CONTROL	9
#else
  #define DLGOK_CONTROL	18
#endif
      if(DLGOK_CONTROL==Dlg.GetExitCode())
      {
         if ((I=CanCloseDialog(EditDlg[2].strData, EditDlg[4].strData)) == 0)
           break;
         Message(MSG_WARNING,1,MSG(MUserMenuTitle),MSG((I==1?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),MSG(MOk));
         Dlg.ClearDone();
         Dialog::SendDlgMessage((HANDLE)&Dlg,DM_SETFOCUS,I*2,0); // Здесь внимательно, если менять дизайн диалога
      }
      else
        return FALSE;
    }
  }

  if (NewRec)
  {
    string strKeyMask;
    strKeyMask.Format (L"%s\\Item%%d",MenuKey);
    InsertKeyRecord(strKeyMask,EditPos,TotalRecords);
  }

  SetRegKey(strItemKey,L"HotKey",EditDlg[2].strData);
  SetRegKey(strItemKey,L"Label",EditDlg[4].strData);
  SetRegKey(strItemKey,L"Submenu",(DWORD)0);

#if defined(PROJECT_DI_MEMOEDIT)
  /*
    ...
    здесь преобразование содержимого 7-го итема в "Command%d"
    ...
  */
#else
  int CommandNumber=0;
  for (I=0;I<10;I++)
    if ( !EditDlg[I+7].strData.IsEmpty() )
      CommandNumber=I+1;
  for (I=0;I<10;I++)
  {
    string strCommandName;
    strCommandName.Format (L"Command%d",I);
    if (I>=CommandNumber)
      DeleteRegValue(strItemKey,strCommandName);
    else
      SetRegKey(strItemKey,strCommandName,EditDlg[I+7].strData);
  }
#endif
  return(TRUE);
}


int EditSubMenu(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogDataEx EditDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(const wchar_t *)MEditSubmenuTitle,
    DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MEditSubmenuHotKey,
    DI_FIXEDIT,5,3,7,3,1,0,0,0,L"",
    DI_TEXT,5,4,0,4,0,0,0,0,(const wchar_t *)MEditSubmenuLabel,
    DI_EDIT,5,5,70,5,0,0,0,0,L"",
    DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    DI_BUTTON,0,7,0,7,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    DI_BUTTON,0,7,0,7,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(EditDlgData,EditDlg);

  int I;
  string strItemKey;
  strItemKey.Format (L"%s\\Item%d",MenuKey,EditPos);
  if (NewRec)
  {
    EditDlg[2].strData=L"";
    EditDlg[4].strData=L"";
  }
  else
  {
    GetRegKey(strItemKey,L"HotKey",EditDlg[2].strData,L"");
    GetRegKey(strItemKey,L"Label",EditDlg[4].strData,L"");
  }
  {
		Dialog Dlg(EditDlg,countof(EditDlg));
    Dlg.SetHelp(L"UserMenu");
    Dlg.SetPosition(-1,-1,76,10);
    while(1)
    {
      Dlg.Process();
      if(6==Dlg.GetExitCode())
      {
        if ((I=CanCloseDialog(EditDlg[2].strData, EditDlg[4].strData)) == 0)
          break;
        Message(MSG_WARNING,1,MSG(MUserMenuTitle),MSG((I==1?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),MSG(MOk));
        Dlg.ClearDone();
        Dialog::SendDlgMessage((HANDLE)&Dlg,DM_SETFOCUS,I*2,0); // Здесь внимательно, если менять дизайн диалога
      }
      else
        return FALSE;
    }
/*
    Dlg.Process();
    if (Dlg.GetExitCode()!=6 || *EditDlg[4].Data==0)
      return(FALSE);
*/
  }
  if (NewRec)
  {
    string strKeyMask;
    strKeyMask.Format (L"%s\\Item%%d", MenuKey);
    InsertKeyRecord(strKeyMask,EditPos,TotalRecords);
  }

  SetRegKey(strItemKey,L"HotKey",EditDlg[2].strData);
  SetRegKey(strItemKey,L"Label",EditDlg[4].strData);
  SetRegKey(strItemKey,L"Submenu",(DWORD)1);
  return(TRUE);
}


void MenuRegToFile(const wchar_t *MenuKey,FILE *MenuFile)
{
	if(!ftell(MenuFile))
		fputwc(0xFEFF,MenuFile);

  for (int I=0;;I++)
  {
    string strItemKey, strHotKey, strLabel;
    int SubMenu;
    strItemKey.Format (L"%s\\Item%d",MenuKey,I);
    if (!GetRegKey(strItemKey,L"Label",strLabel,L""))
      break;
    GetRegKey(strItemKey,L"Label",strLabel,L"");
    GetRegKey(strItemKey,L"HotKey",strHotKey,L"");
    GetRegKey(strItemKey,L"Submenu",SubMenu,0);
    fwprintf(MenuFile,L"%s:  %s\r\n",(const wchar_t*)strHotKey,(const wchar_t*)strLabel);
    if (SubMenu)
    {
      fwprintf(MenuFile,L"{\r\n");
      MenuRegToFile(strItemKey,MenuFile);
      fwprintf(MenuFile,L"}\r\n");
    }
    else
      for (int J=0;;J++)
      {
        string strLineName, strCommand;
        strLineName.Format (L"Command%d",J);
        if (!GetRegKey(strItemKey,strLineName,strCommand,L""))
          break;
        fwprintf(MenuFile,L"    %s\r\n",(const wchar_t *)strCommand);
      }
  }
}


void MenuFileToReg(const wchar_t *MenuKey,FILE *MenuFile)
{
  wchar_t MenuStr[4096]; //BUGBUG
  int KeyNumber=-1,CommandNumber=0;

	if(!ftell(MenuFile))
	{
		if(!GetFileFormat(MenuFile,MenuCP))
			MenuCP=CP_OEMCP;
	}
	while(ReadString(MenuFile,MenuStr,countof(MenuStr),MenuCP))
  {
    string strItemKey;
    strItemKey.Format (L"%s\\Item%d",MenuKey,KeyNumber);
    RemoveTrailingSpaces(MenuStr);
    if (*MenuStr==0)
      continue;
    if (*MenuStr==L'{' && KeyNumber>=0)
    {
      MenuFileToReg(strItemKey,MenuFile);
      continue;
    }
    if (*MenuStr==L'}')
      break;
    if (!IsSpace(*MenuStr))
    {
      string strHotKey, strLabel;

      wchar_t *ChPtr;
      int SubMenu;
      if ((ChPtr=wcschr(MenuStr,L':'))==NULL)
        continue;
      strItemKey.Format (L"%s\\Item%d",MenuKey,++KeyNumber);
      *ChPtr=0;
      strHotKey = MenuStr;
      strLabel = ChPtr+1;
      RemoveLeadingSpaces(strLabel);
      SaveFilePos SavePos(MenuFile);
			SubMenu=(ReadString(MenuFile,MenuStr,countof(MenuStr),MenuCP) && *MenuStr==L'{');
      UseSameRegKey();
      SetRegKey(strItemKey,L"HotKey",strHotKey);
      SetRegKey(strItemKey,L"Label",strLabel);
      SetRegKey(strItemKey,L"Submenu",SubMenu);
      CloseSameRegKey();
      CommandNumber=0;
    }
    else
      if (KeyNumber>=0)
      {
        string strLineName;
        strLineName.Format (L"Command%d",CommandNumber++);
        RemoveLeadingSpaces(MenuStr);
        SetRegKey(strItemKey,strLineName,MenuStr);
      }
  }
}
