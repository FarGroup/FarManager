/*
usermenu.cpp

User menu и есть

*/

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

#if defined(PROJECT_DI_MEMOEDIT)
/*
  Идея в следующем.
  1. Строки в реестре храняться как и раньше, т.к. CommandXXX
  2. Для DI_MEMOEDIT мы из только преобразовываем в один массив
*/
#endif

static int ProcessSingleMenu(char *MenuKey,int MenuPos,char *Title=NULL);
static int FillUserMenu(VMenu& UserMenu,char *MenuKey,int MenuPos,int *FuncPos,char *Name,char *ShortName);
static int DeleteMenuRecord(char *MenuKey,int DeletePos);
static int EditMenuRecord(char *MenuKey,int EditPos,int TotalRecords,int NewRec);
static int EditSubMenu(char *MenuKey,int EditPos,int TotalRecords,int NewRec);
static void MenuRegToFile(char *MenuKey,FILE *MenuFile);
static void MenuFileToReg(char *MenuKey,FILE *MenuFile);

static int MenuModified;
static int MenuNeedRefresh;
static char MenuRootKey[NM],LocalMenuKey[NM];

/* $ 14.07.2000 VVM
   + Режимы показа меню (Menu mode) и Коды выхода из меню (Exit codes)
*/
enum {MM_LOCAL=0,           // Локальное меню
      MM_FAR=1,             // Меню из каталога ФАРа
      MM_MAIN=2
     };           // Главное меню

enum {EC_CLOSE_LEVEL      = -1,   // Выйти из меню на один уровень вверх
      EC_CLOSE_MENU       = -2,   // Выйти из меню по SHIFT+F10
      EC_PARENT_MENU      = -3,   // Показать меню родительского каталога
      EC_MAIN_MENU        = -4,   // Показать главное меню
      EC_COMMAND_SELECTED = -5
     };  // Выбрана команда - закрыть меню и
// обновить папку

static int MenuMode;

static char SubMenuSymbol[]={0x020,0x010,0x000};

const char LocalMenuFileName[]="FarMenu.Ini";

void ProcessUserMenu(int EditMenu)
{
	FILE *MenuFile;
	char MenuFilePath[NM];    // Путь к текущему каталогу с файлом LocalMenuFileName
	char MenuFileFullPath[NM+sizeof(LocalMenuFileName)];
	char *ChPtr;
	int  ExitCode = 0;
	int RunFirst  = 1;
	CtrlObject->CmdLine->GetCurDir(MenuFilePath);
	sprintf(LocalMenuKey,"UserMenu\\LocalMenu%u",clock());
	MenuMode=MM_LOCAL;
	DeleteKeyTree(LocalMenuKey);
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
			strcpy(MenuFilePath, FarPath);
		} /* if */
	}

	while ((ExitCode != EC_CLOSE_LEVEL) && (ExitCode != EC_CLOSE_MENU) &&
	        (ExitCode != EC_COMMAND_SELECTED))
	{
		strcpy(MenuFileFullPath,MenuFilePath);
		AddEndSlash(MenuFileFullPath);
		strcat(MenuFileFullPath,LocalMenuFileName);

		if (MenuMode!=MM_MAIN)
		{
			// Пытаемся открыть файл на локальном диске
			if ((MenuFile=fopen(MenuFileFullPath,"rb"))!=NULL)
			{
				MenuFileToReg(LocalMenuKey, MenuFile);
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
						if (!RunFirst)
						{
							ChPtr=strrchr(MenuFilePath, '\\');

							if (ChPtr!=NULL)
							{
								*(ChPtr--)=0;

								if (*ChPtr!=':')
									continue;
							} /* if */
						} /* if */

						RunFirst=0;
						strcpy(MenuFilePath, FarPath);
						MenuMode=MM_FAR;
						continue;
					} /* if */
				} /* else */
			} /* else */
		} /* if */

		strcpy(MenuRootKey,(MenuMode==MM_MAIN) ? "UserMenu\\MainMenu":LocalMenuKey);
		int PrevMacroMode=CtrlObject->Macro.GetMode();
		int _CurrentFrame=FrameManager->GetCurrentFrame()->GetType();
		CtrlObject->Macro.SetMode(MACRO_USERMENU);
		ExitCode=ProcessSingleMenu(MenuRootKey, 0);

		if (_CurrentFrame == FrameManager->GetCurrentFrame()->GetType()) //???
			CtrlObject->Macro.SetMode(PrevMacroMode);

		// Фаровский кусок по записи файла
		if ((MenuMode!=MM_MAIN) && (MenuModified))
		{
			DWORD FileAttr=GetFileAttributes(MenuFileFullPath);

			if (FileAttr!=(DWORD)-1)
			{
				if (FileAttr & FA_RDONLY)
				{
					int AskOverwrite;
					AskOverwrite=Message(MSG_WARNING,2,MSG(MUserMenuTitle),LocalMenuFileName,
					                     MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo));

					if (AskOverwrite==0)
						SetFileAttributes(MenuFileFullPath,FileAttr & ~FA_RDONLY);
				}

				if (FileAttr & (FA_HIDDEN|FA_SYSTEM))
					SetFileAttributes(MenuFileFullPath,FILE_ATTRIBUTE_NORMAL);
			}

			if ((MenuFile=fopen(MenuFileFullPath,"wb"))!=NULL)
			{
				MenuRegToFile(LocalMenuKey,MenuFile);
				long Length=filelen(MenuFile);
				fclose(MenuFile);

				if (Length==0)
					remove(MenuFileFullPath);
				else
					SetFileAttributes(MenuFileFullPath,FileAttr);
			}
		}

		if (MenuMode!=MM_MAIN)
			DeleteKeyTree(LocalMenuKey);

		switch (ExitCode)
		{
			case EC_PARENT_MENU:
			{
				if (MenuMode==MM_LOCAL)
				{
					ChPtr=strrchr(MenuFilePath, '\\');

					if (ChPtr!=NULL)
					{
						*(ChPtr--)=0;

						if (*ChPtr!=':')
							continue;
					} /* if */

					strcpy(MenuFilePath, FarPath);
					MenuMode=MM_FAR;
				} /* if */
				else
					MenuMode=MM_MAIN;

				break;
			}
			case EC_MAIN_MENU:
			{
				// $ 14.07.2000 VVM + SHIFT+F2 переключает Главное меню/локальное в цикле
				switch (MenuMode)
				{
					case MM_LOCAL:
					{
						strcpy(MenuFilePath, FarPath);
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
						CtrlObject->CmdLine->GetCurDir(MenuFilePath);
						MenuMode=MM_LOCAL;
					}
				} /* switch */

				break;
			} /* case */
		} /* switch */
	} /* while */

	/* $ 25.04.2001 DJ
	   не перерисовываем панель, если пользователь ничего не сделал
	   в меню
	*/
	if (FrameManager->IsPanelsActive() && (ExitCode == EC_COMMAND_SELECTED || MenuModified))
	{
		ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
	}
}


int FillUserMenu(VMenu& UserMenu,char *MenuKey,int MenuPos,int *FuncPos,char *Name,char *ShortName)
{
	int NumLine;
	struct MenuItem UserMenuItem;
	memset(&UserMenuItem,0,sizeof(UserMenuItem));
	UserMenu.DeleteItems();
	/* $ 20.07.2000 tran
	   + лишний проход для вычисления максимальной длины строки */
	int MaxLen=20;
	BOOL HotKeyPresent=FALSE;
	// лишний проход - выясняем есть ли хотя бы один хоткей
	NumLine=0;

	while (1)
	{
		char ItemKey[512],HotKey[10];
		sprintf(ItemKey,"%s\\Item%d",MenuKey,NumLine);

		if (!GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey)))
			break;

		if (*HotKey)
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
		char ItemKey[512],HotKey[10],Label[512],MenuText[512];
		sprintf(ItemKey,"%s\\Item%d",MenuKey,NumLine);

		if (!GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey)))
			break;

		if (!GetRegKey(ItemKey,"Label",Label,"",sizeof(Label)))
			break;

		SubstFileName(Label,sizeof(Label),Name,ShortName,NULL,NULL,TRUE);
		/* $ 28.07.2000 VVM
		   + Обработка переменных окружения
		*/
		ExpandEnvironmentStr(Label, Label, sizeof(Label));
		/* VVM $ */
		int FuncNum=0;

		if (strlen(HotKey)>1)
		{
			FuncNum=atoi(&HotKey[1]);

			if (FuncNum<1 || FuncNum>12)
				FuncNum=1;

			sprintf(HotKey,"F%d",FuncNum);
		}
		else if (*HotKey == '&')
			strcat(HotKey,"&");

		/* $ 14.10.2000 VVM
		   + Разделитель меню, если Метка пуста, а ХотКей="-"
		*/
		if ((strlen(Label)==0) && (strcmp(HotKey,"-")==0))
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
			if (HotKeyPresent)
			{
				int AddHotKey = (*HotKey) && (!FuncNum);
				sprintf(MenuText,"%s%-*.*s %-20.*s%s",
				        (AddHotKey?"&":""),
				        (*HotKey=='&'?4:3),(*HotKey=='&'?4:3),HotKey,
				        ScrX-12,Label,
				        ((strchr(Label, '&')==NULL)||(AddHotKey))?"":" ");
			}
			else
			{
				const char *Ptr=(strchr(Label, '&')==NULL?"":" ");
				sprintf(MenuText,"%-20.*s%s",ScrX-12,Label,Ptr);
			}

			/* VVM $ */
			MenuTextLen=(int)strlen(MenuText)-(FuncNum>0?1:0);
			MaxLen=(MaxLen<MenuTextLen ? MenuTextLen : MaxLen);
		} /* else */

		/* VVM $ */
		NumLine++;
	}

	// коррекция максимальной длины
	if (MaxLen > ScrX-14) // по полной программе!
		MaxLen = ScrX-14;

	NumLine=0;

	while (1)
	{
		char ItemKey[512],HotKey[10],Label[512],MenuText[512];
		sprintf(ItemKey,"%s\\Item%d",MenuKey,NumLine);

		if (!GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey)))
			break;

		if (!GetRegKey(ItemKey,"Label",Label,"",sizeof(Label)))
			break;

		SubstFileName(Label,sizeof(Label),Name,ShortName,NULL,NULL,TRUE);
		/* $ 28.07.2000 VVM
		   + Обработка переменных окружения
		*/
		ExpandEnvironmentStr(Label, Label, sizeof(Label));
		/* VVM $ */
		int SubMenu;
		GetRegKey(ItemKey,"Submenu",SubMenu,0);
		int FuncNum=0;

		if (strlen(HotKey)>1)
		{
			FuncNum=atoi(&HotKey[1]);

			if (FuncNum<1 || FuncNum>12)
				FuncNum=1;

			sprintf(HotKey,"F%d",FuncNum);
		}
		else if (*HotKey == '&')
			strcat(HotKey,"&");

		/* $ 14.10.2000 VVM
		   + Разделитель меню, если Метка пуста, а ХотКей="-"
		*/
		if ((strlen(Label)==0) && (strcmp(HotKey,"-")==0))
		{
			UserMenuItem.Flags|=LIF_SEPARATOR;
			UserMenuItem.Flags&=~LIF_SELECTED;
			UserMenuItem.Name[0]=0;

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
			if (HotKeyPresent)
			{
				int AddHotKey = (*HotKey) && (!FuncNum);
				sprintf(MenuText,"%s%-*.*s %-*.*s%s",
				        (AddHotKey?"&":""),
				        (*HotKey=='&'?4:3),(*HotKey=='&'?4:3),HotKey,
				        MaxLen,MaxLen,Label,
				        ((strchr(Label, '&')==NULL)||(AddHotKey))?"":" ");
			}
			else
			{
				const char *Ptr=(strchr(Label, '&')==NULL?"":" ");
				sprintf(MenuText,"%-*.*s%s",MaxLen,MaxLen,Label,Ptr);
			}

			/* VVM $ */

			/* tran 20.07.2000 $ */
			if (SubMenu)
			{
				strcat(MenuText,SubMenuSymbol);
//_SVS(SysLog("%2d - '%s'",HiStrlen(MenuText),MenuText));
			}

			xstrncpy(UserMenuItem.Name,MenuText,sizeof(UserMenuItem.Name)-1);
			UserMenuItem.SetSelect(NumLine==MenuPos);
			UserMenuItem.Flags&=~LIF_SEPARATOR;
		}

		/* VVM $ */
		int ItemPos=UserMenu.AddItem(&UserMenuItem);

		if (FuncNum>0)
			FuncPos[FuncNum-1]=ItemPos;

		NumLine++;
	}

	*UserMenuItem.Name=0;
	UserMenuItem.Flags&=~LIF_SEPARATOR;
	UserMenuItem.SetSelect(NumLine==MenuPos);
	UserMenu.AddItem(&UserMenuItem);
	return NumLine;
}

/* $ 14.07.2000 VVM
   + Вместо TRUE/FALSE возвращает коды EC_*
*/
/* VVM $ */
int ProcessSingleMenu(char *MenuKey,int MenuPos,char *Title)
{
	while (1)
	{
		struct MenuItem UserMenuItem;
		memset(&UserMenuItem,0,sizeof(UserMenuItem));
		int NumLine=0,ExitCode,FuncPos[12];

		for (int I=0; I<sizeof(FuncPos)/sizeof(FuncPos[0]); I++)
			FuncPos[I]=-1;

		char Name[NM],ShortName[NM];
		CtrlObject->Cp()->ActivePanel->GetCurName(Name,ShortName);
		{
			/* $ 24.07.2000 VVM
			 + При показе главного меню в заголовок добавляет тип - FAR/Registry
			*/
			char MenuTitle[128];

			if (Title && *Title)
				xstrncpy(MenuTitle,Title,sizeof(MenuTitle)-1);
			else
				switch (MenuMode)
				{
					case MM_LOCAL:
						strcpy(MenuTitle,MSG(MLocalMenuTitle));
						break;
					case MM_FAR:
						sprintf(MenuTitle,"%s (%s)",MSG(MMainMenuTitle),MSG(MMainMenuFAR));
						break;
					default:
					{
						char *Ptr=MSG(MMainMenuREG);

						if (*Ptr)
							sprintf(MenuTitle,"%s (%s)",MSG(MMainMenuTitle),Ptr);
						else
							sprintf(MenuTitle,"%s",MSG(MMainMenuTitle));
					}
				} /* switch */

			VMenu UserMenu(MenuTitle,NULL,0,ScrY-4);
			/* VVM $ */
			/* $ 05.06.2001 KM
			   ! Поправочка. UserMenu не выставлял флаг VMENU_WRAPMODE
			*/
			UserMenu.SetFlags(VMENU_WRAPMODE);
			/* KM $ */
			UserMenu.SetHelp("UserMenu");
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
						NumLine=FillUserMenu(UserMenu,MenuKey,MenuPos,FuncPos,Name,ShortName);
						// заставим манагер менюхи корректно отрисовать ширину и
						// высоту, а заодно и скорректировать вертикальные позиции
						UserMenu.SetPosition(-1,-1,-1,-1);
						UserMenu.Show();
						MenuNeedRefresh=FALSE;
					}

					int Key=UserMenu.ReadInput();
					MenuPos=UserMenu.GetSelectPos();

					if (Key>=KEY_F1 && Key<=KEY_F12)
					{
						int FuncItemPos;

						if ((FuncItemPos=FuncPos[Key-KEY_F1])!=-1)
						{
							UserMenu.Modal::SetExitCode(FuncItemPos);
							continue;
						}
					}
					else if (Key == ' ') // исключаем пробел из "хоткеев"!
						continue;

					switch (Key)
					{
							/* $ 24.08.2001 VVM
							  + Стрелки вправо/влево открывают/закрывают подменю соответственно */
						case KEY_RIGHT:
						case KEY_NUMPAD6:
						case KEY_MSWHEEL_RIGHT:
						{
							char CurrentKey[512];
							int SubMenu;
							sprintf(CurrentKey,"%s\\Item%d",MenuKey,MenuPos);
							GetRegKey(CurrentKey,"Submenu",SubMenu,0);

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
							/* VVM $ */
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
								char MenuFileName[NM];

								if (!FarMkTempEx(MenuFileName) || (MenuFile=fopen(MenuFileName,"wb"))==NULL)
									break;

								MenuRegToFile(MenuRootKey,MenuFile);
								MenuNeedRefresh=TRUE;
								fclose(MenuFile);
								{
									ConsoleTitle *OldTitle=new ConsoleTitle;
									FileEditor ShellEditor(MenuFileName,FFILEEDIT_DISABLEHISTORY,-1,-1,NULL);
									delete OldTitle;
									ShellEditor.SetDynamicallyBorn(false);
									FrameManager->EnterModalEV();
									FrameManager->ExecuteModal();
									FrameManager->ExitModalEV();

									if (!ShellEditor.IsFileChanged() || (MenuFile=fopen(MenuFileName,"rb"))==NULL)
									{
										remove(MenuFileName);
//                    break;
										return(0);
									}
								}
								DeleteKeyTree(MenuRootKey);
								MenuFileToReg(MenuRootKey,MenuFile);
								fclose(MenuFile);
								remove(MenuFileName);
								/* $ 14.12.2001 IS Меню изменили, зачем же это скрывать? */
								MenuModified=TRUE;
								/* IS $ */
								UserMenu.Hide();
								/* $ 14.07.2000 VVM
								   ! Закрыть меню
								*/
								return(0);
							}	/* VVM $ */

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

							if (Key == KEY_F1)
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
		char CurrentKey[512];
		int SubMenu;
		sprintf(CurrentKey,"%s\\Item%d",MenuKey,ExitCode);
		GetRegKey(CurrentKey,"Submenu",SubMenu,0);

		if (SubMenu)
		{
			/* $ 20.08.2001 VVM
			  + При вложенных меню показывает заголовки предыдущих */
			char SubMenuKey[512],SubMenuLabel[512],SubMenuTitle[512];
			sprintf(SubMenuKey,"%s\\Item%d",MenuKey,ExitCode);
			SubMenuTitle[0]=0;

			if (GetRegKey(SubMenuKey,"Label",SubMenuLabel,"",sizeof(SubMenuLabel)))
			{
				SubstFileName(SubMenuLabel,sizeof(SubMenuLabel),Name,ShortName,NULL,NULL,TRUE);
				ExpandEnvironmentStr(SubMenuLabel, SubMenuLabel, sizeof(SubMenuLabel));
				char *HotKeyInLabel = strchr(SubMenuLabel, '&');

				if (HotKeyInLabel)
					strcpy(HotKeyInLabel, HotKeyInLabel+1);

				if (Title && *Title)
					sprintf(SubMenuTitle, "%s -> %s", Title, SubMenuLabel);
				else
					strcpy(SubMenuTitle, SubMenuLabel);
			} /* if */

			/* VVM $ */
			/* $ 14.07.2000 VVM
			   ! Если закрыли подменю, то остаться. Инече передать управление выше
			*/
			MenuPos=ProcessSingleMenu(SubMenuKey,0,SubMenuTitle);

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
		char CmdLineDir[NM];
		CtrlObject->CmdLine->GetCurDir(CmdLineDir);
		/* $ 14.12.2001 IS
		     При выполнении команды меню не теряем старую командную строку.
		*/
		char *OldCmdLine=xf_strdup(CtrlObject->CmdLine->GetStringAddr());
		int OldCmdLineCurPos = CtrlObject->CmdLine->GetCurPos();
		int OldCmdLineLeftPos = CtrlObject->CmdLine->GetLeftPos();
		int OldCmdLineSelStart, OldCmdLineSelEnd;
		CtrlObject->CmdLine->GetSelection(OldCmdLineSelStart,OldCmdLineSelEnd);
		CtrlObject->CmdLine->LockUpdatePanel(TRUE);

		while (1)
		{
			char LineName[50],Command[4096];
			sprintf(LineName,"Command%d",CurLine);

			if (!GetRegKey(CurrentKey,LineName,Command,"",sizeof(Command)))
				break;

			char ListName[NM*2]="",ShortListName[NM*2]="";

			if (memicmp(Command,"REM ",4) && memicmp(Command,"::",2))
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
					int PreserveLFN=SubstFileName(Command,sizeof(Command),Name,ShortName,ListName,ShortListName,FALSE,CmdLineDir);
					PreserveLongName PreserveName(ShortName,PreserveLFN);

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
					if (ExtractIfExistCommand(Command))
					{
						if (*Command)
						{
							ProcessOSAliases(Command,sizeof(Command));
							CtrlObject->CmdLine->ExecString(Command,FALSE);
						}
					}
				}
			}

			if (*ListName)
				remove(ListName);

			if (ListName[NM])
				remove(ListName+NM);

			if (*ShortListName)
				remove(ShortListName);

			if (ShortListName[NM])
				remove(ShortListName+NM);

			CurLine++;
		}

		CtrlObject->CmdLine->LockUpdatePanel(FALSE);

		if (OldCmdLine) // восстановим сохраненную командную строку
		{
			CtrlObject->CmdLine->SetString(OldCmdLine, FrameManager->IsPanelsActive());
			CtrlObject->CmdLine->SetCurPos(OldCmdLineCurPos, OldCmdLineLeftPos);
			CtrlObject->CmdLine->Select(OldCmdLineSelStart, OldCmdLineSelEnd);
			xf_free(OldCmdLine);
			OldCmdLine=NULL;
			FrameManager->GetCurrentFrame()->Redraw();
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


int DeleteMenuRecord(char *MenuKey,int DeletePos)
{
	char RecText[200],ItemName[200],RegKey[512];
	int SubMenu;
	sprintf(RegKey,"%s\\Item%d",MenuKey,DeletePos);
	GetRegKey(RegKey,"Label",RecText,"",sizeof(RecText));
	GetRegKey(RegKey,"Submenu",SubMenu,0);
	sprintf(ItemName,"\"%s\"",RecText);

	if (Message(MSG_WARNING,2,MSG(MUserMenuTitle),
	            MSG(!SubMenu?MAskDeleteMenuItem:MAskDeleteSubMenuItem),
	            ItemName,MSG(MDelete),MSG(MCancel))!=0)
		return(FALSE);

	MenuModified=MenuNeedRefresh=TRUE;
	sprintf(RegKey,"%s\\Item%%d",MenuKey);
	DeleteKeyRecord(RegKey,DeletePos);
	return(TRUE);
}

/* $ 29.08.2001 VVM
  + Добавим немного логики на закрытие диалога */
// возвращает: 0 -все ОБИ, 1 или 2 - ошибка и, соответственно, на какой контрол поставить фокус
int CanCloseDialog(char *Hotkey, char *Label)
{
	if (strcmp(Hotkey,"-") == 0)
		return 0;

	if (strlen(Label) == 0)
		return 2;

	if (strlen(Hotkey) < 2)
		return 0;

	int FuncNum=atoi(&Hotkey[1]); // Проверить на правильность задания функциональной клавиши

	if (((*Hotkey == 'f') || (*Hotkey == 'F')) && ((FuncNum > 0) && (FuncNum < 13)))
		return 0;

	return 1;
}
/* VVM $ */

static LONG_PTR WINAPI UserMenuDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
#if defined(PROJECT_DI_MEMOEDIT)
	Dialog* Dlg=(Dialog*)hDlg;

	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			break;
		}
	}

#endif
	return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


int EditMenuRecord(char *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
	static struct DialogData EditDlgData[]=
	{
		/* 00 */DI_DOUBLEBOX,3,1,72,20,0,0,0,0,(char *)MEditMenuTitle,
		/* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(char *)MEditMenuHotKey,
		/* 02 */DI_FIXEDIT,5,3,7,3,1,0,0,0,"",
		/* 03 */DI_TEXT,5,4,0,4,0,0,0,0,(char *)MEditMenuLabel,
		/* 04 */DI_EDIT,5,5,70,5,0,0,0,0,"",
		/* 05 */DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		/* 06 */DI_TEXT,5,7,0,7,0,0,0,0,(char *)MEditMenuCommands,
#if defined(PROJECT_DI_MEMOEDIT)
		/* 07 */DI_MEMOEDIT,5, 8,70,17,0,0,0,0,"",
		/* 08 */DI_TEXT,3,18,0,18,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		/* 09 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(char *)MOk,
		/* 10 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(char *)MCancel
#else
		/* 07 */DI_EDIT,5, 8,70,8,0,0,DIF_EDITOR,0,"",
		/* 08 */DI_EDIT,5, 9,70,9,0,0,DIF_EDITOR,0,"",
		/* 09 */DI_EDIT,5,10,70,10,0,0,DIF_EDITOR,0,"",
		/* 10 */DI_EDIT,5,11,70,11,0,0,DIF_EDITOR,0,"",
		/* 11 */DI_EDIT,5,12,70,12,0,0,DIF_EDITOR,0,"",
		/* 12 */DI_EDIT,5,13,70,13,0,0,DIF_EDITOR,0,"",
		/* 13 */DI_EDIT,5,14,70,14,0,0,DIF_EDITOR,0,"",
		/* 14 */DI_EDIT,5,15,70,15,0,0,DIF_EDITOR,0,"",
		/* 15 */DI_EDIT,5,16,70,16,0,0,DIF_EDITOR,0,"",
		/* 16 */DI_EDIT,5,17,70,17,0,0,DIF_EDITOR,0,"",
		/* 17 */DI_TEXT,3,18,0,18,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		/* 18 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(char *)MOk,
		/* 19 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(char *)MCancel
#endif

	};
	MakeDialogItems(EditDlgData,EditDlg);
	int I;
	char ItemKey[512];
	sprintf(ItemKey,"%s\\Item%d",MenuKey,EditPos);
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
		GetRegKey(ItemKey,"Submenu",SubMenu,0);

		if (SubMenu)
			return(EditSubMenu(MenuKey,EditPos,TotalRecords,FALSE));

		GetRegKey(ItemKey,"HotKey",EditDlg[2].Data,"",sizeof(EditDlg[2].Data));
		GetRegKey(ItemKey,"Label",EditDlg[4].Data,"",sizeof(EditDlg[4].Data));
#if defined(PROJECT_DI_MEMOEDIT)
		/*
		  ...
		  здесь добавка строк из "Command%d" в 7-й итем
		  ...
		*/
		char *Buffer7=NULL;
		int Buffer7Size=0;
		int CommandNumber=0;

		while (1)
		{
			char CommandName[20],Command[4096];
			sprintf(CommandName,"Command%d",CommandNumber);

			if (!GetRegKey(ItemKey,CommandName,Command,"",sizeof(Command)))
				break;

			int lenCommand=strlen(Command);
			char *NewBuffer7=(char *)xf_realloc(Buffer7,Buffer7Size+lenCommand+8);

			if (!NewBuffer7)
				break;

			if (!Buffer7 || !*Buffer7)
				*NewBuffer7=0;

			Buffer7=NewBuffer7;
			strcat(Buffer7,Command);
			strcat(Buffer7,"\n"); //??? "\n\r"
			Buffer7Size+=lenCommand+1; // ??? 2
			CommandNumber++;
		}

		EditDlg[7].PtrData=Buffer7;
		EditDlg[7].PtrLength=Buffer7Size;
#else
		int CommandNumber=0;

		while (CommandNumber<10)
		{
			char CommandName[20],Command[4096];
			sprintf(CommandName,"Command%d",CommandNumber);

			if (!GetRegKey(ItemKey,CommandName,Command,"",sizeof(Command)))
				break;

			xstrncpy(EditDlg[7+CommandNumber].Data,Command,sizeof(EditDlg[0].Data)-1);
			CommandNumber++;
		}

#endif
	}

	{
		Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]),UserMenuDlgProc);
		Dlg.SetHelp("UserMenu");
		Dlg.SetPosition(-1,-1,76,22);

		/* $ 22.12.2000 IS
		   ! Если не ввели метку и нажали "продолжить", то не выходим из диалога
		     редактирования команд, т.к. теряем те команды, что, возможно, ввели.
		     Для выхода из меню нужно воспользоваться esc или кнопкой "отменить".
		*/
		while (1)
		{
			Dlg.Process();
#if defined(PROJECT_DI_MEMOEDIT)
#define DLGOK_CONTROL 9
#else
#define DLGOK_CONTROL 18
#endif

			if (DLGOK_CONTROL==Dlg.GetExitCode())
			{
				if ((I=CanCloseDialog(EditDlg[2].Data, EditDlg[4].Data)) == 0)
					break;

				Message(MSG_WARNING,1,MSG(MUserMenuTitle),MSG((I==1?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),MSG(MOk));
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
		char KeyMask[512];
		sprintf(KeyMask,"%s\\Item%%d",MenuKey);
		InsertKeyRecord(KeyMask,EditPos,TotalRecords);
	}

	SetRegKey(ItemKey,"HotKey",EditDlg[2].Data);
	SetRegKey(ItemKey,"Label",EditDlg[4].Data);
	SetRegKey(ItemKey,"Submenu",(DWORD)0);
#if defined(PROJECT_DI_MEMOEDIT)
	/*
	  ...
	  здесь преобразование содержимого 7-го итема в "Command%d"
	  ...
	*/
#else
	int CommandNumber=0;

	for (I=0; I<10; I++)
		if (*EditDlg[I+7].Data)
			CommandNumber=I+1;

	for (I=0; I<10; I++)
	{
		char CommandName[20];
		sprintf(CommandName,"Command%d",I);

		if (I>=CommandNumber)
			DeleteRegValue(ItemKey,CommandName);
		else
			SetRegKey(ItemKey,CommandName,EditDlg[I+7].Data);
	}

#endif
	return(TRUE);
}


int EditSubMenu(char *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
	static struct DialogData EditDlgData[]=
	{
		DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MEditSubmenuTitle,
		DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditSubmenuHotKey,
		DI_FIXEDIT,5,3,7,3,1,0,0,0,"",
		DI_TEXT,5,4,0,0,0,0,0,0,(char *)MEditSubmenuLabel,
		DI_EDIT,5,5,70,3,0,0,0,0,"",
		DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
		DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
	};
	MakeDialogItems(EditDlgData,EditDlg);
	int I;
	char ItemKey[512];
	sprintf(ItemKey,"%s\\Item%d",MenuKey,EditPos);

	if (NewRec)
	{
		*EditDlg[2].Data=0;
		*EditDlg[4].Data=0;
	}
	else
	{
		GetRegKey(ItemKey,"HotKey",EditDlg[2].Data,"",sizeof(EditDlg[2].Data));
		GetRegKey(ItemKey,"Label",EditDlg[4].Data,"",sizeof(EditDlg[4].Data));
	}

	{
		Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
		Dlg.SetHelp("UserMenu");
		Dlg.SetPosition(-1,-1,76,10);

		while (1)
		{
			Dlg.Process();

			if (6==Dlg.GetExitCode())
			{
				if ((I=CanCloseDialog(EditDlg[2].Data, EditDlg[4].Data)) == 0)
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
		char KeyMask[512];
		sprintf(KeyMask,"%s\\Item%%d",MenuKey);
		InsertKeyRecord(KeyMask,EditPos,TotalRecords);
	}

	SetRegKey(ItemKey,"HotKey",EditDlg[2].Data);
	SetRegKey(ItemKey,"Label",EditDlg[4].Data);
	SetRegKey(ItemKey,"Submenu",(DWORD)1);
	return(TRUE);
}


void MenuRegToFile(char *MenuKey,FILE *MenuFile)
{
	for (int I=0;; I++)
	{
		char ItemKey[512],HotKey[10],Label[512];
		int SubMenu;
		sprintf(ItemKey,"%s\\Item%d",MenuKey,I);

		if (!GetRegKey(ItemKey,"Label",Label,"",sizeof(Label)))
			break;

		GetRegKey(ItemKey,"Label",Label,"",sizeof(Label));
		GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey));
		GetRegKey(ItemKey,"Submenu",SubMenu,0);
		fprintf(MenuFile,"%s:  %s\r\n",HotKey,Label);

		if (SubMenu)
		{
			fprintf(MenuFile,"{\r\n");
			MenuRegToFile(ItemKey,MenuFile);
			fprintf(MenuFile,"}\r\n");
		}
		else
			for (int J=0;; J++)
			{
				char LineName[50],Command[4096];
				sprintf(LineName,"Command%d",J);

				if (!GetRegKey(ItemKey,LineName,Command,"",sizeof(Command)))
					break;

				fprintf(MenuFile,"    %s\r\n",Command);
			}
	}
}


void MenuFileToReg(char *MenuKey,FILE *MenuFile)
{
	char MenuStr[4096];
	int KeyNumber=-1,CommandNumber=0;

	while (fgets(MenuStr,sizeof(MenuStr),MenuFile)!=NULL)
	{
		char ItemKey[512];
		sprintf(ItemKey,"%s\\Item%d",MenuKey,KeyNumber);
		RemoveTrailingSpaces(MenuStr);

		if (*MenuStr==0)
			continue;

		if (*MenuStr=='{' && KeyNumber>=0)
		{
			MenuFileToReg(ItemKey,MenuFile);
			continue;
		}

		if (*MenuStr=='}')
			break;

		if (!IsSpace(*MenuStr))
		{
			char HotKey[10],Label[512],*ChPtr;
			int SubMenu;

			if ((ChPtr=strchr(MenuStr,':'))==NULL)
				continue;

			sprintf(ItemKey,"%s\\Item%d",MenuKey,++KeyNumber);
			*ChPtr=0;
			xstrncpy(HotKey,MenuStr,sizeof(HotKey)-1);
			xstrncpy(Label,ChPtr+1,sizeof(Label)-1);
			RemoveLeadingSpaces(Label);
			SaveFilePos SavePos(MenuFile);
			SubMenu=(fgets(MenuStr,sizeof(MenuStr),MenuFile)!=NULL && *MenuStr=='{');
			UseSameRegKey();
			SetRegKey(ItemKey,"HotKey",HotKey);
			SetRegKey(ItemKey,"Label",Label);
			SetRegKey(ItemKey,"Submenu",SubMenu);
			CloseSameRegKey();
			CommandNumber=0;
		}
		else if (KeyNumber>=0)
		{
			char LineName[50];
			sprintf(LineName,"Command%d",CommandNumber++);
			RemoveLeadingSpaces(MenuStr);
			SetRegKey(ItemKey,LineName,MenuStr);
		}
	}
}
