/*
usermenu.cpp

User menu и есть

*/

/* Revision: 1.83 23.05.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
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

static int ProcessSingleMenu(const wchar_t *MenuKey,int MenuPos,const wchar_t *Title=NULL);
static int FillUserMenu(VMenu& UserMenu, const wchar_t *MenuKey,int MenuPos,int *FuncPos,const wchar_t *Name,const wchar_t *ShortName);
static int DeleteMenuRecord(const wchar_t *MenuKey,int DeletePos);
static int EditMenuRecord(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec);
static int EditSubMenu(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec);
static void MenuRegToFile(const wchar_t *MenuKey,FILE *MenuFile);
static void MenuFileToReg(const wchar_t *MenuKey,FILE *MenuFile);

static int MenuModified;
static int MenuNeedRefresh;
static string strMenuRootKey, strLocalMenuKey;

/* $ 14.07.2000 VVM
   + Режимы показа меню (Menu mode) и Коды выхода из меню (Exit codes)
*/
enum {MM_LOCAL=0,           // Локальное меню
      MM_FAR=1,             // Меню из каталога ФАРа
      MM_MAIN=2};           // Главное меню

/* $ 25.04.2001 DJ
   новая константа EC_COMMAND_SELECTED
*/
enum {EC_CLOSE_LEVEL      = -1,   // Выйти из меню на один уровень вверх
      EC_CLOSE_MENU       = -2,   // Выйти из меню по SHIFT+F10
      EC_PARENT_MENU      = -3,   // Показать меню родительского каталога
      EC_MAIN_MENU        = -4,   // Показать главное меню
      EC_COMMAND_SELECTED = -5};  // Выбрана команда - закрыть меню и
                                  // обновить папку
/* DJ $ */

static int MenuMode;
/* VVM $ */

static wchar_t SubMenuSymbol[]={0x020,0x010,0x000};

const wchar_t LocalMenuFileName[]=L"FarMenu.Ini";

void ProcessUserMenu(int EditMenu)
{
  FILE *MenuFile;

  string strMenuFilePath;    // Путь к текущему каталогу с файлом LocalMenuFileName
  string strPrevPath;
  wchar_t *ChPtr;
  int  ExitCode = 0;
  int RunFirst  = 1;

  CtrlObject->CmdLine->GetCurDirW(strPrevPath);
  strMenuFilePath = strPrevPath;

  strLocalMenuKey.Format (L"UserMenu\\LocalMenu%u",clock());
  MenuMode=MM_LOCAL;

  DeleteKeyTreeW(strLocalMenuKey);

  MenuModified=MenuNeedRefresh=FALSE;

  if (EditMenu)
  {
    int EditChoice=MessageW(0,3,UMSG(MUserMenuTitle),UMSG(MChooseMenuType),
                   UMSG(MChooseMenuMain),UMSG(MChooseMenuLocal),UMSG(MCancel));
    if (EditChoice<0 || EditChoice==2)
      return;
    if (EditChoice==0)
    {
      MenuMode=MM_FAR;
      strMenuFilePath = g_strFarPath;
    } /* if */
  }

  UserDefinedListW *SavedCurDirs=SaveAllCurDir();

/* $ 14.07.2000 VVM
   + Почти полностью переписан алгоритм функции ProcessUserMenu. Добавлен цикл.
*/
/* $ 25.04.2001 DJ
   добавлена EC_COMMAND_EXECUTED
*/
  while((ExitCode != EC_CLOSE_LEVEL) && (ExitCode != EC_CLOSE_MENU) &&
      (ExitCode != EC_COMMAND_SELECTED))
/* DJ $ */
  {

    if (MenuMode!=MM_MAIN)
    {
      // Пытаемся открыть файл на локальном диске
      if (FarChDirW(strMenuFilePath) &&
         ((MenuFile=_wfopen(LocalMenuFileName,L"rb"))!=NULL))
      {
        MenuFileToReg(strLocalMenuKey, MenuFile);
        fclose(MenuFile);
      } /* if */
      else
      {
      // Файл не открылся. Смотрим дальше.
        if (MenuMode==MM_FAR)
          MenuMode=MM_MAIN;
        else
        {
          if (!EditMenu)
          {
/* $ 14.07.2000 VVM
    + При первом вызове не ищет меню из родительского каталога
*/
/* $ 28.07.2000 VVM
    + Введен флаг для первого вызова
*/
            if (!RunFirst)
            {
              ChPtr = strMenuFilePath.GetBuffer();

              ChPtr=wcsrchr(ChPtr, L'\\');
              if (ChPtr!=NULL)
              {
                *(ChPtr--)=0;
                if (*ChPtr!=L':')
                {
                    strMenuFilePath.ReleaseBuffer();
                    continue;
                }
              } /* if */

              strMenuFilePath.ReleaseBuffer();
            } /* if */
            RunFirst=0;
/* VVM $ */
            strMenuFilePath = g_strFarPath;
            MenuMode=MM_FAR;
            continue;
          } /* if */
        } /* else */
      } /* else */
      FarChDirW(strPrevPath); // Вернем папку на место !
    } /* if */


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
      FarChDirW(strMenuFilePath);
      int FileAttr=GetFileAttributesW(LocalMenuFileName);
      if (FileAttr!=-1)
      {
        if (FileAttr & FA_RDONLY)
        {
          int AskOverwrite;
          AskOverwrite=MessageW(MSG_WARNING,2,UMSG(MUserMenuTitle),LocalMenuFileName,
                       UMSG(MEditRO),UMSG(MEditOvr),UMSG(MYes),UMSG(MNo));
          if (AskOverwrite==0)
            SetFileAttributesW(LocalMenuFileName,FileAttr & ~FA_RDONLY);
        }
        if (FileAttr & (FA_HIDDEN|FA_SYSTEM))
          SetFileAttributesW(LocalMenuFileName,FILE_ATTRIBUTE_NORMAL);
      }
      if ((MenuFile=_wfopen(LocalMenuFileName,L"wb"))!=NULL)
      {
        MenuRegToFile(strLocalMenuKey,MenuFile);
        long Length=filelen(MenuFile);
        fclose(MenuFile);
        if (Length==0)
          DeleteFileW (LocalMenuFileName);
      }
    }
    if (MenuMode!=MM_MAIN)
      DeleteKeyTreeW(strLocalMenuKey);

    switch(ExitCode)
    {
      case EC_PARENT_MENU:
      {
        if (MenuMode==MM_LOCAL)
        {
          ChPtr = strMenuFilePath.GetBuffer();

          ChPtr=wcsrchr(ChPtr, L'\\');
          if (ChPtr!=NULL)
          {
            *(ChPtr--)=0;
            if (*ChPtr!=L':')
            {
                strMenuFilePath.ReleaseBuffer();
                continue;
            }
          } /* if */

          strMenuFilePath.ReleaseBuffer();

          strMenuFilePath = g_strFarPath;
          MenuMode=MM_FAR;
        } /* if */
        else
          MenuMode=MM_MAIN;
        break;
      }
      case EC_MAIN_MENU:
      {
/* $ 14.07.2000 VVM
    + SHIFT+F2 переключает Главное меню/локальное в цикле
*/
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
            CtrlObject->CmdLine->GetCurDirW(strMenuFilePath);
            MenuMode=MM_LOCAL;
          }
        } /* switch */
//      if (MenuMode==MM_LOCAL)
//      {
//        strcpy(MenuFilePath, FarPath);
//        MenuMode=MM_FAR;
//      }
//      else
//        MenuMode=MM_MAIN;
/* VVM $ */
        break;
      } /* case */
    } /* switch */
  } /* while */

  CtrlObject->CmdLine->SetCurDirW(strPrevPath);
  FarChDirW(strPrevPath);

  if (FrameManager->IsPanelsActive() && (ExitCode == EC_COMMAND_SELECTED || MenuModified))
  {
    ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
  }

/*
  if ((!EditMenu || !MainMenu) && (MenuFile=fopen(MenuFileName,"rb"))!=NULL)
  {
    MenuFileToReg(LocalMenuKey,MenuFile);
    fclose(MenuFile);
    MainMenuTitle=MainMenu=FALSE;
  }
  else
    if (!EditMenu || MainMenu)
    {
      sprintf(MenuFileName,"%s%s",FarPath,LocalMenuFileName);
      if ((MenuFile=fopen(MenuFileName,"rb"))!=NULL)
      {
        MenuFileToReg(LocalMenuKey,MenuFile);
        fclose(MenuFile);
        MainMenu=FALSE;
      }
    }

  char MenuKey[512];
  strcpy(MenuKey,MainMenu ? "MainMenu":LocalMenuName);
  sprintf(MenuRootKey,"UserMenu\\%s",MenuKey);
  ProcessSingleMenu(MenuKey,0);

  chdir(CurDir);

  if (!MainMenu && MenuModified)
  {
    int FileAttr=GetFileAttributes(MenuFileName);
    if (FileAttr!=-1)
    {
      if (FileAttr & FA_RDONLY)
      {
        int AskOverwrite;
        AskOverwrite=Message(MSG_WARNING,2,MSG(MUserMenuTitle),MenuFileName,
                     MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo));
        if (AskOverwrite==0)
          SetFileAttributes(MenuFileName,FileAttr & ~FA_RDONLY);
      }
      if (FileAttr & (FA_HIDDEN|FA_SYSTEM))
        SetFileAttributes(MenuFileName,FILE_ATTRIBUTE_NORMAL);
    }
    if ((MenuFile=fopen(MenuFileName,"wb"))!=NULL)
    {
      MenuRegToFile(LocalMenuKey,MenuFile);
      long Length=filelen(MenuFile);
      fclose(MenuFile);
      if (Length==0)
        remove(LocalMenuFileName);
      CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
      CtrlObject->Cp()->ActivePanel->Redraw();
    }
  }
  if (!MainMenu)
    DeleteKeyTree(LocalMenuKey);
*/

  RestoreAllCurDir(SavedCurDirs);
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
    if(!GetRegKeyW(strItemKey,L"HotKey",strHotKey,L""))
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
    if (!GetRegKeyW(strItemKey,L"HotKey",strHotKey,L""))
      break;
    if (!GetRegKeyW(strItemKey,L"Label",strLabel,L""))
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
    if ( (wcslen(strLabel)==0) && (wcscmp(strHotKey, L"-")==0))
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
        strMenuText.Format (L"%s%-*.*s %-20.*s%s",(AddHotKey?L"&":L""),( strHotKey.At(0)==L'&'?4:3),( strHotKey.At(0)==L'&'?4:3), (const wchar_t*)strHotKey,ScrX-12,(const wchar_t*)strLabel,((wcschr(strLabel, L'&')==NULL)||(AddHotKey))?L"":L" ");
      }
      else
      {
        const wchar_t *Ptr=(wcschr(strLabel, L'&')==NULL?L"":L" ");
        strMenuText.Format (L"%-20.*s%s",ScrX-12,(const wchar_t*)strLabel,Ptr);
      }
      /* VVM $ */
      MenuTextLen=strMenuText.GetLength()-(FuncNum>0?1:0);
      MaxLen=(MaxLen<MenuTextLen ? MenuTextLen : MaxLen);
    } /* else */
    /* VVM $ */
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
    if (!GetRegKeyW(strItemKey,L"HotKey",strHotKey,L""))
      break;
    if (!GetRegKeyW(strItemKey,L"Label",strLabel,L""))
      break;
    SubstFileName(strLabel,Name,ShortName,NULL,NULL,NULL,NULL,TRUE);

    apiExpandEnvironmentStrings (strLabel, strLabel);

    int SubMenu;
    GetRegKeyW(strItemKey,L"Submenu",SubMenu,0);

    int FuncNum=0;
    if ( wcslen(strHotKey)>1)
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
    if ((wcslen(strLabel)==0) && (wcscmp(strHotKey,L"-")==0))
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
        strMenuText.Format (L"%s%-*.*s %-*.*s%s",(AddHotKey?L"&":L""),( strHotKey.At(0)==L'&'?4:3),( strHotKey.At(0)==L'&'?4:3),(const wchar_t*)strHotKey,MaxLen,MaxLen,(const wchar_t*)strLabel,((wcschr(strLabel, L'&')==NULL)||(AddHotKey))?L"":L" ");
      }
      else
      {
        const wchar_t *Ptr=(wcschr(strLabel, L'&')==NULL?L"":L" ");
        strMenuText.Format (L"%-*.*s%s",MaxLen,MaxLen,(const wchar_t*)strLabel,Ptr);
      }
      /* VVM $ */

    /* tran 20.07.2000 $ */
      if (SubMenu)
      {
        strMenuText += SubMenuSymbol;
//_SVS(SysLog("%2d - '%s'",HiStrlen(MenuText),MenuText));
      }
      UserMenuItem.strName = strMenuText;
      UserMenuItem.SetSelect(NumLine==MenuPos);
      UserMenuItem.Flags&=~LIF_SEPARATOR;
    }
    /* VVM $ */
    int ItemPos=UserMenu.AddItemW(&UserMenuItem);
    if (FuncNum>0)
      FuncPos[FuncNum-1]=ItemPos;
    NumLine++;
  }

  UserMenuItem.strName=L"";
  UserMenuItem.Flags&=~LIF_SEPARATOR;
  UserMenuItem.SetSelect(NumLine==MenuPos);
  UserMenu.AddItemW(&UserMenuItem);
  return NumLine;
}

/* $ 14.07.2000 VVM
   + Вместо TRUE/FALSE возвращает коды EC_*
*/
/* VVM $ */
int ProcessSingleMenu(const wchar_t *MenuKey,int MenuPos,const wchar_t *Title)
{
  MenuItemEx UserMenuItem;

  while (1)
  {
    UserMenuItem.Clear ();
    int NumLine=0,ExitCode,FuncPos[12];

    for (int I=0;I<sizeof(FuncPos)/sizeof(FuncPos[0]);I++)
      FuncPos[I]=-1;

    string strName,strShortName;
    CtrlObject->Cp()->ActivePanel->GetCurNameW(strName,strShortName);
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
          strMenuTitle = UMSG(MLocalMenuTitle);
          break;
        case MM_FAR:
          strMenuTitle.Format (L"%s (%s)",UMSG(MMainMenuTitle),UMSG(MMainMenuFAR));
          break;
        default:
          {
            const wchar_t *Ptr=UMSG(MMainMenuREG);
            if(*Ptr)
              strMenuTitle.Format (L"%s (%s)",UMSG(MMainMenuTitle),Ptr);
            else
              strMenuTitle.Format (L"%s", UMSG(MMainMenuTitle));
          }
        } /* switch */
      VMenu UserMenu(strMenuTitle,NULL,0,TRUE,ScrY-4);
      /* VVM $ */

      /* $ 05.06.2001 KM
         ! Поправочка. UserMenu не выставлял флаг VMENU_WRAPMODE
      */
      UserMenu.SetFlags(VMENU_WRAPMODE);
      /* KM $ */
      UserMenu.SetHelp(L"UserMenu");
      UserMenu.SetPosition(-1,-1,0,0);
      UserMenu.SetBottomTitle(UMSG(MMainMenuBottomTitle));

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
          MenuPos=UserMenu.GetSelectPos();
          int Key=UserMenu.ReadInput();
          if (Key>=KEY_F1 && Key<=KEY_F12)
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
            {
              string strCurrentKey;
              int SubMenu;
              strCurrentKey.Format (L"%s\\Item%d",MenuKey,MenuPos);
              GetRegKeyW(strCurrentKey,L"Submenu",SubMenu,0);
              if (SubMenu)
                UserMenu.SetExitCode(MenuPos);
              break;
            }
            case KEY_LEFT:
              if (Title && *Title)
                UserMenu.SetExitCode(-1);
              break;
            /* VVM $ */
            case KEY_DEL:
              if (MenuPos<NumLine)
                DeleteMenuRecord(MenuKey,MenuPos);
//              MenuModified=TRUE;
              break;
            case KEY_INS:
            case KEY_F4:
            case KEY_SHIFTF4:
              if (Key != KEY_INS && MenuPos>=NumLine)
                break;
              EditMenuRecord(MenuKey,MenuPos,NumLine,Key == KEY_INS);
//              MenuModified=TRUE;
              break;
            case KEY_ALTF4:
              if (RegVer)
              {
                (*FrameManager)[0]->Unlock();
                FILE *MenuFile;
                string strMenuFileName;
                if (!FarMkTempExW(strMenuFileName) || (MenuFile=_wfopen(strMenuFileName,L"wb"))==NULL)
                  break;
                MenuRegToFile(strMenuRootKey,MenuFile);
                MenuNeedRefresh=TRUE;
                fclose(MenuFile);
                {
                  ConsoleTitle *OldTitle=new ConsoleTitle;
                  string strFileName = strMenuFileName;
                  FileEditor ShellEditor(strFileName,FALSE,FALSE,-1,-1,TRUE,NULL);
                  delete OldTitle;
                  ShellEditor.SetDynamicallyBorn(false);
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal();
                  FrameManager->ExitModalEV();
                  if (!ShellEditor.IsFileChanged() || (MenuFile=_wfopen(strMenuFileName,L"rb"))==NULL)
                  {
                    DeleteFileW(strMenuFileName);
                    return(0);
                  }
                }
                DeleteKeyTreeW(strMenuRootKey);
                MenuFileToReg(strMenuRootKey,MenuFile);
                fclose(MenuFile);
                DeleteFileW (strMenuFileName);
                /* $ 14.12.2001 IS Меню изменили, зачем же это скрывать? */
                MenuModified=TRUE;
                /* IS $ */
                UserMenu.Hide();
/* $ 14.07.2000 VVM
   ! Закрыть меню
*/
                return(0);
/* VVM $ */
              }
              else
                MessageW(MSG_WARNING,1,UMSG(MWarning),UMSG(MRegOnly),UMSG(MOk));
              break;
            /* $ 28.06.2000 tran
               выход из пользовательского меню по ShiftF10 из любого уровня
               вложенности просто задаем ExitCode -1, и возвращаем FALSE -
               по FALSE оно и выйдет откуда угодно */
            case KEY_SHIFTF10:
//              UserMenu.SetExitCode(-1);
              return(EC_CLOSE_MENU);
             /* tran $ */
/* $ 14.07.2000 VVM
   + Показать главное меню
*/
            case KEY_SHIFTF2:
                return(EC_MAIN_MENU);
/* $ 17.07.2000 VVM
   + Показать меню из родительского каталога только в MM_LOCAL режиме
*/
            case KEY_BS:
              if (MenuMode!=MM_MAIN)
                return(EC_PARENT_MENU);
/* VVM $ */
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
/* $ 14.07.2000 VVM
   ! вверх на один уровень
*/
      return(EC_CLOSE_LEVEL);
/* VVM $ */

    string strCurrentKey;
    int SubMenu;
    strCurrentKey.Format (L"%s\\Item%d",MenuKey,ExitCode);
    GetRegKeyW(strCurrentKey,L"Submenu",SubMenu,0);

    if (SubMenu)
    {
      /* $ 20.08.2001 VVM
        + При вложенных меню показывает заголовки предыдущих */
      string strSubMenuKey, strSubMenuLabel, strSubMenuTitle;
      strSubMenuKey.Format (L"%s\\Item%d",MenuKey,ExitCode);

      if(GetRegKeyW(strSubMenuKey,L"Label",strSubMenuLabel,L""))
      {
        SubstFileName(strSubMenuLabel,strName,strShortName,NULL,NULL,NULL,NULL,TRUE);
        apiExpandEnvironmentStrings (strSubMenuLabel, strSubMenuLabel);

        wchar_t *HotKeyInLabel = strSubMenuLabel.GetBuffer();

        HotKeyInLabel = wcschr(HotKeyInLabel, L'&');

        if (HotKeyInLabel)
          wcscpy(HotKeyInLabel, HotKeyInLabel+1);

        strSubMenuLabel.ReleaseBuffer();

        if (Title && *Title)
          strSubMenuTitle.Format (L"%s -> %s", Title, (const wchar_t*)strSubMenuLabel);
        else
          strSubMenuTitle = strSubMenuLabel;
      } /* if */
      /* VVM $ */
/* $ 14.07.2000 VVM
   ! Если закрыли подменю, то остаться. Инече передать управление выше
*/
      MenuPos=ProcessSingleMenu(strSubMenuKey,0,strSubMenuTitle);
      if (MenuPos!=EC_CLOSE_LEVEL)
        return(MenuPos);
/* VVM $ */
      MenuPos=ExitCode;
      continue;
    }

    /* $ 01.05.2001 IS
         Отключим до лучших времен
    */
    //int LeftVisible,RightVisible,PanelsHidden=0;
    /* IS $ */
    int CurLine=0;

    string strCmdLineDir;

    CtrlObject->CmdLine->GetCurDirW(strCmdLineDir);

    string strOldCmdLine;

    CtrlObject->CmdLine->GetStringW(strOldCmdLine);
    int OldCmdLineSelStart, OldCmdLineSelEnd;
    CtrlObject->CmdLine->GetSelection(OldCmdLineSelStart,OldCmdLineSelEnd);
    CtrlObject->CmdLine->LockUpdatePanel(TRUE);
    while (1)
    {
      string strLineName, strCommand;
      strLineName.Format (L"Command%d",CurLine);
      if (!GetRegKeyW(strCurrentKey,strLineName,strCommand,L""))
        break;

      string strListName, strAnotherListName;
      string strShortListName, strAnotherShortListName;
      if (LocalStrnicmpW (strCommand,L"REM ",4) && LocalStrnicmpW(strCommand,L"::",2))
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
          PreserveLongNameW PreserveName(strShortName,PreserveLFN);
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
          /* IS $ */
//          ;
//_SVS(SysLog("!%s!",Command));
          if(ExtractIfExistCommand(strCommand))
          {
            if ( !strCommand.IsEmpty() )
              CtrlObject->CmdLine->ExecString(strCommand,FALSE);
          }
        }
      }
      if ( !strListName.IsEmpty() )
          DeleteFileW (strListName);
      if ( !strAnotherListName.IsEmpty() )
          DeleteFileW (strAnotherListName);

      if ( !strShortListName.IsEmpty() )
          DeleteFileW (strShortListName);
      if ( !strAnotherShortListName.IsEmpty() )
          DeleteFileW (strAnotherShortListName);

      CurLine++;
    }
    CtrlObject->CmdLine->LockUpdatePanel(FALSE);
    if( !strOldCmdLine.IsEmpty() ) // восстановим сохраненную командную строку
    {


       CtrlObject->CmdLine->SetStringW(strOldCmdLine,FrameManager->IsPanelsActive());
       CtrlObject->CmdLine->Select(OldCmdLineSelStart,OldCmdLineSelEnd);
    }
    /* IS $ */
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
    /* IS $ */
/* $ 14.07.2000 VVM
   ! Закрыть меню
*/
/* $ 25.04.2001 DJ
   сообщаем, что была выполнена команда (нужно перерисовать панели)
*/
    return(EC_COMMAND_SELECTED);
/* DJ $ */
/* VVM $ */
  }
}


int DeleteMenuRecord(const wchar_t *MenuKey,int DeletePos)
{
  string strRecText, strItemName, strRegKey;
  int SubMenu;
  strRegKey.Format (L"%s\\Item%d",MenuKey,DeletePos);
  GetRegKeyW(strRegKey,L"Label",strRecText,L"");
  GetRegKeyW(strRegKey,L"Submenu",SubMenu,0);
  strItemName.Format (L"\"%s\"", (const wchar_t*)strRecText);
  if (MessageW(MSG_WARNING,2,UMSG(MUserMenuTitle),
          UMSG(!SubMenu?MAskDeleteMenuItem:MAskDeleteSubMenuItem),
              strItemName,UMSG(MDelete),UMSG(MCancel))!=0)
    return(FALSE);
  MenuModified=MenuNeedRefresh=TRUE;
  strRegKey.Format (L"%s\\Item%%d", MenuKey);
  DeleteKeyRecordW(strRegKey,DeletePos);
  return(TRUE);
}

/* $ 29.08.2001 VVM
  + Добавим немного логики на закрытие диалога */
// возвращает: 0 -все ОБИ, 1 или 2 - ошибка и, соответственно, на какой контрол поставить фокус
int CanCloseDialog(const wchar_t *Hotkey, const wchar_t *Label)
{
  if (wcscmp(Hotkey,L"-") == 0)
    return 0;
  if (wcslen(Label) == 0)
    return 2;
  if (wcslen(Hotkey) < 2)
    return 0;
  /* Проверить на правильность задания функциональной клавиши */
  int FuncNum=_wtoi(Hotkey+1);
  if (((*Hotkey == L'f') || (*Hotkey == L'F')) &&
      ((FuncNum > 0) && (FuncNum < 13)))
    return 0;
  return 1;
}
/* VVM $ */

int EditMenuRecord(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogDataEx EditDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,72,20,0,0,0,0,(const wchar_t *)MEditMenuTitle,
  /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MEditMenuHotKey,
  /* 02 */DI_FIXEDIT,5,3,7,3,1,0,0,0,L"",
  /* 03 */DI_TEXT,5,4,0,0,0,0,0,0,(const wchar_t *)MEditMenuLabel,
  /* 04 */DI_EDIT,5,5,70,3,0,0,0,0,L"",
  /* 05 */DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 06 */DI_TEXT,5,7,0,0,0,0,0,0,(const wchar_t *)MEditMenuCommands,
  /* 07 */DI_EDIT,5, 8,70,3,0,0,DIF_EDITOR,0,L"",
  /* 08 */DI_EDIT,5, 9,70,3,0,0,DIF_EDITOR,0,L"",
  /* 09 */DI_EDIT,5,10,70,3,0,0,DIF_EDITOR,0,L"",
  /* 10 */DI_EDIT,5,11,70,3,0,0,DIF_EDITOR,0,L"",
  /* 11 */DI_EDIT,5,12,70,3,0,0,DIF_EDITOR,0,L"",
  /* 12 */DI_EDIT,5,13,70,3,0,0,DIF_EDITOR,0,L"",
  /* 13 */DI_EDIT,5,14,70,3,0,0,DIF_EDITOR,0,L"",
  /* 14 */DI_EDIT,5,15,70,3,0,0,DIF_EDITOR,0,L"",
  /* 15 */DI_EDIT,5,16,70,3,0,0,DIF_EDITOR,0,L"",
  /* 16 */DI_EDIT,5,17,70,3,0,0,DIF_EDITOR,0,L"",
  /* 17 */DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 18 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 19 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel

  };
  MakeDialogItemsEx(EditDlgData,EditDlg);

  int I;
  string strItemKey;
  strItemKey.Format (L"%s\\Item%d", MenuKey,EditPos);

  MenuModified=MenuNeedRefresh=TRUE;

  if (NewRec)
  {
    switch (MessageW(0,2,UMSG(MUserMenuTitle),UMSG(MAskInsertMenuOrCommand),
                    UMSG(MMenuInsertCommand),UMSG(MMenuInsertMenu)))
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
    GetRegKeyW(strItemKey,L"Submenu",SubMenu,0);
    if (SubMenu)
      return(EditSubMenu(MenuKey,EditPos,TotalRecords,FALSE));
    GetRegKeyW(strItemKey,L"HotKey",EditDlg[2].strData,L"");
    GetRegKeyW(strItemKey,L"Label",EditDlg[4].strData,L"");
    int CommandNumber=0;
    while (CommandNumber<10)
    {
      string strCommandName, strCommand;
      strCommandName.Format (L"Command%d",CommandNumber);
      if (!GetRegKeyW(strItemKey,strCommandName,strCommand,L""))
        break;
      EditDlg[7+CommandNumber].strData = strCommand;
      CommandNumber++;
    }
  }

  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
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
      if(18==Dlg.GetExitCode())
      {
         if ((I=CanCloseDialog(EditDlg[2].strData, EditDlg[4].strData)) == 0)
           break;
         MessageW(MSG_WARNING,1,UMSG(MUserMenuTitle),UMSG((I==1?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),UMSG(MOk));
         Dlg.ClearDone();
         Dialog::SendDlgMessage((HANDLE)&Dlg,DM_SETFOCUS,I*2,0); // Здесь внимательно, если менять дизайн диалога
      }
      else
        return FALSE;
    }
    /* IS $ */
  }

  if (NewRec)
  {
    string strKeyMask;
    strKeyMask.Format (L"%s\\Item%%d",MenuKey);
    InsertKeyRecordW(strKeyMask,EditPos,TotalRecords);
  }

  SetRegKeyW(strItemKey,L"HotKey",EditDlg[2].strData);
  SetRegKeyW(strItemKey,L"Label",EditDlg[4].strData);
  SetRegKeyW(strItemKey,L"Submenu",(DWORD)0);

  int CommandNumber=0;
  for (I=0;I<10;I++)
    if ( !EditDlg[I+7].strData.IsEmpty() )
      CommandNumber=I+1;
  for (I=0;I<10;I++)
  {
    string strCommandName;
    strCommandName.Format (L"Command%d",I);
    if (I>=CommandNumber)
      DeleteRegValueW(strItemKey,strCommandName);
    else
      SetRegKeyW(strItemKey,strCommandName,EditDlg[I+7].strData);
  }
  return(TRUE);
}


int EditSubMenu(const wchar_t *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogDataEx EditDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(const wchar_t *)MEditSubmenuTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MEditSubmenuHotKey,
    DI_FIXEDIT,5,3,7,3,1,0,0,0,L"",
    DI_TEXT,5,4,0,0,0,0,0,0,(const wchar_t *)MEditSubmenuLabel,
    DI_EDIT,5,5,70,3,0,0,0,0,L"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
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
    GetRegKeyW(strItemKey,L"HotKey",EditDlg[2].strData,L"");
    GetRegKeyW(strItemKey,L"Label",EditDlg[4].strData,L"");
  }
  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp(L"UserMenu");
    Dlg.SetPosition(-1,-1,76,10);
    while(1)
    {
      Dlg.Process();
      if(6==Dlg.GetExitCode())
      {
        if ((I=CanCloseDialog(EditDlg[2].strData, EditDlg[4].strData)) == 0)
          break;
        MessageW(MSG_WARNING,1,UMSG(MUserMenuTitle),UMSG((I==1?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),UMSG(MOk));
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
    InsertKeyRecordW(strKeyMask,EditPos,TotalRecords);
  }

  SetRegKeyW(strItemKey,L"HotKey",EditDlg[2].strData);
  SetRegKeyW(strItemKey,L"Label",EditDlg[4].strData);
  SetRegKeyW(strItemKey,L"Submenu",(DWORD)1);
  return(TRUE);
}


void MenuRegToFile(const wchar_t *MenuKey,FILE *MenuFile)
{
  for (int I=0;;I++)
  {
    string strItemKey, strHotKey, strLabel;
    int SubMenu;
    strItemKey.Format (L"%s\\Item%d",MenuKey,I);
    if (!GetRegKeyW(strItemKey,L"Label",strLabel,L""))
      break;
    GetRegKeyW(strItemKey,L"Label",strLabel,L"");
    GetRegKeyW(strItemKey,L"HotKey",strHotKey,L"");
    GetRegKeyW(strItemKey,L"Submenu",SubMenu,0);
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
        if (!GetRegKeyW(strItemKey,strLineName,strCommand,L""))
          break;
        fwprintf(MenuFile,L"    %s\r\n",(const wchar_t *)strCommand);
      }
  }
}


void MenuFileToReg(const wchar_t *MenuKey,FILE *MenuFile)
{
  wchar_t MenuStr[4096]; //BUGBUG
  int KeyNumber=-1,CommandNumber=0;
  while (fgetws(MenuStr,sizeof(MenuStr)*sizeof(wchar_t),MenuFile)!=NULL)
  {
    string strItemKey;
    strItemKey.Format (L"%s\\Item%d",MenuKey,KeyNumber);
    RemoveTrailingSpacesW(MenuStr);
    if (*MenuStr==0)
      continue;
    if (*MenuStr==L'{' && KeyNumber>=0)
    {
      MenuFileToReg(strItemKey,MenuFile);
      continue;
    }
    if (*MenuStr==L'}')
      break;
    if (!IsSpaceW(*MenuStr))
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
      RemoveLeadingSpacesW(strLabel);
      SaveFilePos SavePos(MenuFile);
      SubMenu=(fgetws(MenuStr,sizeof(MenuStr)*sizeof(wchar_t),MenuFile)!=NULL && *MenuStr==L'{');
      UseSameRegKey();
      SetRegKeyW(strItemKey,L"HotKey",strHotKey);
      SetRegKeyW(strItemKey,L"Label",strLabel);
      SetRegKeyW(strItemKey,L"Submenu",SubMenu);
      CloseSameRegKey();
      CommandNumber=0;
    }
    else
      if (KeyNumber>=0)
      {
        string strLineName;
        strLineName.Format (L"Command%d",CommandNumber++);
        RemoveLeadingSpacesW(MenuStr);
        SetRegKeyW(strItemKey,strLineName,MenuStr);
      }
  }
}
