/*
main.cpp

Функция main.

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "chgprior.hpp"
#include "colors.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "lockscrn.hpp"
#include "hilight.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "language.hpp"
#include "farexcpt.hpp"

#ifdef DIRECT_RT
int DirectRT=0;
#endif


class TConsoleRestore
{
	private:
		char OldTitle[512];
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		HANDLE hOutput;
		BOOL IsRectoreConsole;
	public:
		TConsoleRestore(BOOL RectoreConsole)
		{
			hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleTitle(OldTitle,sizeof(OldTitle));
			GetConsoleScreenBufferInfo(hOutput,&sbi);
			IsRectoreConsole=RectoreConsole;
		};
		~TConsoleRestore()
		{
			if (IsRectoreConsole)
			{
				SetConsoleTitle(OldTitle);
				//SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
				SetConsoleWindowInfo(hOutput,TRUE,&sbi.srWindow);
				SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
			}
		};
};

static void ConvertOldSettings();
/* $ 07.07.2000 IS
  Вынес эту декларацию в fn.cpp, чтобы была доступна отовсюду...
*/
//static void SetHighlighting();
/* IS $ */
static void CopyGlobalSettings();

static void show_help(void)
{
	printf(
	    "Usage: far [switches] [apath [ppath]]\n\n"
	    "where\n"
	    "  apath - path to a folder (or a file or an archive) for the active panel\n"
	    "  ppath - path to a folder (or a file or an archive) for the passive panel\n\n"
	    "The following switches may be used in the command line:\n\n"
	    " /?   This help.\n"
	    " /a   Disable display of characters with codes 0 - 31 and 255.\n"
	    " /ag  Disable display of pseudographics characters.\n");
#if defined(USE_WFUNC)

	if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		printf(
		    " /8   Forces FAR to work in ANSI (non-Unicode) console.\n"
		);
	}

#endif
	printf(
	    " /e[<line>[:<pos>]] <filename>\n"
	    "      Edit the specified file.\n"
	    " /i   Set small (16x16) icon for FAR console window.\n"
	    " /p[<path>]\n"
	    "      Search for \"common\" plugins in the directory, specified by <path>.\n"
	    " /co  Forces FAR to load plugins from the cache only.\n"
	    " /rc  Restore console windows settings upon exiting FAR.\n"
	    " /m   Do not load macros.\n"
	    " /ma  Do not execute auto run macros.\n"
	    " /u <username>\n"
	    "      Allows to have separate settings for different users.\n"
	    " /v <filename>\n"
	    "      View the specified file. If <filename> is -, data is read from the stdin.\n"
	    " /x   Disable exception handling.\n");
#if defined(_DEBUGEXC)
	printf(" /xd  Enable exception handling.\n");
#endif
#ifdef DIRECT_RT
	printf(
	    " /do  Direct output.\n");
#endif
#if 0
#if defined(USE_WFUNC)

	if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		printf(
		    " /w   Specify this if you are using a TrueType font for the console.\n"
		);
	}

#endif
#endif
}

static int MainProcess(char *EditName,char *ViewName,char *DestName1,char *DestName2,int StartLine,int StartChar)
{
	{
		ChangePriority ChPriority(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? THREAD_PRIORITY_ABOVE_NORMAL:THREAD_PRIORITY_NORMAL);
		ControlObject CtrlObj;
		CONSOLE_SCREEN_BUFFER_INFO InitCsbi;
		GetConsoleScreenBufferInfo(hConOut,&InitCsbi);
		SetRealColor(COL_COMMANDLINEUSERSCREEN);

		// учтем настройки максимизации окна при старте
		if (IsZoomed(hFarWnd)) ChangeVideoMode(1);

		if (*EditName || *ViewName)
		{
			NotUseCAS=TRUE;
			Opt.OnlyEditorViewerUsed=1;
			Panel *DummyPanel=new Panel;
			CmdMode=TRUE;
			_tran(SysLog("create dummy panels"));
			CtrlObj.CreateFilePanels();
			CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=DummyPanel;
			CtrlObj.Plugins.LoadPlugins();

			if (*EditName)
			{
				FileEditor *ShellEditor=new FileEditor(EditName,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6,StartLine,StartChar);
				_tran(SysLog("make shelleditor %p",ShellEditor));

				if (!ShellEditor->GetExitCode())  // ????????????
				{
					FrameManager->ExitMainLoop(0);
				}
			}
			else // TODO: Этот else убрать только после разборок с возможностью задавать несколько /e и /v в ком.строке
				if (*ViewName)
				{
					FileViewer *ShellViewer=new FileViewer(ViewName,FALSE);

					if (!ShellViewer->GetExitCode())
					{
						FrameManager->ExitMainLoop(0);
					}

					_tran(SysLog("make shellviewer, %p",ShellViewer));
				}

			FrameManager->EnterMainLoop();
			CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=NULL;
			delete DummyPanel;
			_tran(SysLog("editor/viewer closed, delete dummy panels"));
		}
		else
		{
			char Path[NM];
			Opt.OnlyEditorViewerUsed=0;
			NotUseCAS=FALSE;
			Opt.SetupArgv=0;

			// воспользуемся тем, что ControlObject::Init() создает панели
			// юзая Opt.*
			if (*DestName1) // актиная панель
			{
				Opt.SetupArgv++;
				strcpy(Path,DestName1);
				*PointToNameUNC(Path)=0;
				DeleteEndSlash(Path); // если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

				if ((Path[1]==':' && !Path[2]) || (PathPrefix(Path) && Path[5]==':' && !Path[6]))
					AddEndSlash(Path);

				// Та панель, которая имеет фокус - активна (начнем по традиции с Левой Панели ;-)
				if (Opt.LeftPanel.Focus)
				{
					Opt.LeftPanel.Type=FILE_PANEL;  // сменим моду панели
					Opt.LeftPanel.Visible=TRUE;     // и включим ее
					strcpy(Opt.LeftFolder,Path);
				}
				else
				{
					Opt.RightPanel.Type=FILE_PANEL;
					Opt.RightPanel.Visible=TRUE;
					strcpy(Opt.RightFolder,Path);
				}

				if (*DestName2) // пассивная панель
				{
					Opt.SetupArgv++;
					strcpy(Path,DestName2);
					*PointToNameUNC(Path)=0;
					DeleteEndSlash(Path);

					if ((Path[1]==':' && !Path[2]) || (PathPrefix(Path) && Path[5]==':' && !Path[6]))
						AddEndSlash(Path);

					// а здесь с точнотью наоборот - обрабатываем пассивную панель
					if (Opt.LeftPanel.Focus)
					{
						Opt.RightPanel.Type=FILE_PANEL; // сменим моду панели
						Opt.RightPanel.Visible=TRUE;    // и включим ее
						strcpy(Opt.RightFolder,Path);
					}
					else
					{
						Opt.LeftPanel.Type=FILE_PANEL;
						Opt.LeftPanel.Visible=TRUE;
						strcpy(Opt.LeftFolder,Path);
					}
				}
			}

			// теперь все готово - создаем панели!
			CtrlObj.Init();

			// а теперь "провалимся" в каталог или хост-файл (если получится ;-)
			if (*DestName1) // актиная панель
			{
				LockScreen LockScr;
				Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (*DestName2) // пассивная панель
				{
					AnotherPanel->GetCurDir(Path);
					FarChDir(Path, TRUE);
					strcpy(Path,PointToNameUNC(DestName2));

					if (*Path)
					{
						if (AnotherPanel->GoToFile(Path))
							AnotherPanel->ProcessKey(KEY_CTRLPGDN);
					}
				}

				ActivePanel->GetCurDir(Path);
				FarChDir(Path, TRUE);
				strcpy(Path,PointToNameUNC(DestName1));

				if (*Path)
				{
					if (ActivePanel->GoToFile(Path))
						ActivePanel->ProcessKey(KEY_CTRLPGDN);
				}

				// !!! ВНИМАНИЕ !!!
				// Сначала редравим пассивную панель, а потом активную!
				AnotherPanel->Redraw();
				ActivePanel->Redraw();
			}

			FrameManager->EnterMainLoop();
		}

		// очистим за собой!
		SetScreen(0,0,ScrX,ScrY,' ',COL_COMMANDLINEUSERSCREEN);
		SetConsoleTextAttribute(hConOut,InitCsbi.wAttributes);
		ScrBuf.ResetShadow();
		ScrBuf.Flush();
		MoveRealCursor(0,0);
	}
	CloseConsole();
	RestoreIcons();
	return(0);
}

int main_sehed(char *EditName,char *ViewName,char *DestName1,char *DestName2,int StartLine,int StartChar)
{
	TRY
	{
		return MainProcess(EditName,ViewName,DestName1,DestName2,StartLine,StartChar);
	}
	EXCEPT(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
	{
		TerminateProcess(GetCurrentProcess(), 1);
		return -1; // сюда никогда не дойдем
	}
}

int _cdecl main(int Argc, char *Argv[])
{
	_OT(SysLog("[[[[[[[[New Session of FAR]]]]]]]]]"));
	//char EditName[NM],ViewName[NM];
	char *EditName="",*ViewName="";
	char DestName[2][NM];
	int StartLine=-1,StartChar=-1,RectoreConsole=FALSE;
	int CntDestName=0; // количество параметров-имен каталогов
	//*EditName=*ViewName=0;
	DestName[0][0]=DestName[1][0]=0;
	CmdMode=FALSE;
	WinVer.dwOSVersionInfoSize=sizeof(WinVer);
	GetVersionEx(&WinVer);
	/*$ 18.04.2002 SKV
	  Попользуем floating point что бы проинициализировался vc-ный fprtl.
	*/
#ifdef _MSC_VER
	float x=1.1f;
	char buf[15];
	sprintf(buf,"%f",x);
#endif
	// если под дебагером, то отключаем исключения однозначно,
	//  иначе - смотря что указал юзвер.
#if defined(_DEBUGEXC)
	Opt.ExceptRules=-1;
#else

	if (!pIsDebuggerPresent)
		pIsDebuggerPresent=(PISDEBUGGERPRESENT)GetProcAddress(GetModuleHandle("KERNEL32"),"IsDebuggerPresent");

	Opt.ExceptRules=(pIsDebuggerPresent && pIsDebuggerPresent()?0:-1);
#endif
#if defined(USE_WFUNC)
	Opt.UseUnicodeConsole=(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT /* && WinVer.dwMajorVersion >= 5 */)?TRUE:FALSE;
#endif
//  Opt.ExceptRules=-1;
//_SVS(SysLog("Opt.ExceptRules=%d",Opt.ExceptRules));
	/* $ 30.12.2000 SVS
	   Проинициализируем функции работы с атрибутами Encryped сразу после
	   старта FAR
	*/
	GetEncryptFunctions();
	/* SVS $ */
	SetRegRootKey(HKEY_CURRENT_USER);
	strcpy(Opt.RegRoot,"Software\\Far");
	/* $ 03.08.2000 SVS
	   По умолчанию - брать плагины из основного каталога
	*/
	Opt.LoadPlug.MainPluginDir=TRUE;
	/* SVS $ */
	Opt.LoadPlug.PluginsPersonal=TRUE;
	/* $ 01.09.2000 tran
	   /co - cache only, */
	Opt.LoadPlug.PluginsCacheOnly=FALSE;
	/* tran $ */
	/* $ 09.01.2002 IS только после этого можно использовать Local* */
	LocalUpperInit();
	/* IS $ */
	SetFileApisTo(APIS2OEM);
	GetModuleFileName(NULL,FarPath,sizeof(FarPath));
#if defined(USE_WFUNC)
//  if(Opt.CleanAscii || Opt.NoGraphics)
//    Opt.UseUnicodeConsole=FALSE;
#endif
	// $ 02.07.2001 IS - Учтем то, что GetModuleFileName иногда возвращает короткое имя, которое нам нафиг не нужно.
	*(PointToName(FarPath)-1)=0;
	{
		char tmpFarPath[sizeof(FarPath)];
		DWORD s=RawConvertShortNameToLongName(FarPath, tmpFarPath,
		                                      sizeof(tmpFarPath));

		if (s && s<sizeof(tmpFarPath))
			strcpy(FarPath, tmpFarPath);
	}
	{
		FAR_OemToChar(FarPath, FarPath);
		SetEnvironmentVariable("FARHOME",FarPath);
		FAR_CharToOem(FarPath, FarPath);
		SetEnvironmentVariable("FARUSER",NULL);     
	}
	AddEndSlash(FarPath);
	// макросы не дисаблим
	Opt.DisableMacro=0;

	_SVS(for (int I0=1; I0<Argc; I0++)SysLog("I=%d Argv[I]=\"%s\"",I0,Argv[I0]));

	for (int I=1; I<Argc; I++)
	{
		if ((Argv[I][0]=='/' || Argv[I][0]=='-') && Argv[I][1])
		{
			switch (toupper(Argv[I][1]))
			{
				case 'A':

					switch (toupper(Argv[I][2]))
					{
						case 0:
							Opt.CleanAscii=TRUE;
							break;
						case 'G':

							if (!Argv[I][3])
								Opt.NoGraphics=TRUE;

							break;
					}

					break;
#if defined(USE_WFUNC)
				case '8':
					Opt.UseUnicodeConsole=FALSE;
					break;
#endif
				case 'E':

					if (isdigit(Argv[I][2]))
					{
						StartLine=atoi(&Argv[I][2]);
						char *ChPtr=strchr(&Argv[I][2],':');

						if (ChPtr!=NULL)
							StartChar=atoi(ChPtr+1);
					}

					if (I+1<Argc)
					{
						//FAR_CharToOem(Argv[I+1],EditName);
						FAR_CharToOem(Argv[I+1],Argv[I+1]);
						EditName=Argv[I+1];
						I++;
					}

					break;
				case 'V':

					if (I+1<Argc)
					{
						//FAR_CharToOem(Argv[I+1],ViewName);
						FAR_CharToOem(Argv[I+1],Argv[I+1]);
						ViewName=Argv[I+1];
						I++;
					}

					break;
				case 'R':

					switch (toupper(Argv[I][2]))
					{
						case 'C':

							if (!Argv[I][3])
								RectoreConsole=TRUE;

							break;
					}

					break;
				case 'M':

					switch (toupper(Argv[I][2]))
					{
						case 0:
							Opt.DisableMacro|=MDOL_ALL;
							break;
						case 'A':

							if (!Argv[I][3])
								Opt.DisableMacro|=MDOL_AUTOSTART;

							break;
					}

					break;
				case 'I':
					Opt.SmallIcon=TRUE;
					break;
				case 'X':
					Opt.ExceptRules=0;
#if defined(_DEBUGEXC)

					if (toupper(Argv[I][2])=='D' && !Argv[I][3])
						Opt.ExceptRules=1;

#endif
					break;
				case 'U':

					if (I+1<Argc)
					{
						xstrncat(Opt.RegRoot,"\\Users\\",sizeof(Opt.RegRoot)-1);
						xstrncat(Opt.RegRoot,Argv[I+1],sizeof(Opt.RegRoot)-1);
						SetEnvironmentVariable("FARUSER",Argv[I+1]);
						CopyGlobalSettings();
						I++;
					}

					break;
				case 'P':
				{
					// Полиция 19
					if (Opt.Policies.DisabledOptions&FFPOL_USEPSWITCH)
						break;

					Opt.LoadPlug.PluginsPersonal=FALSE;
					Opt.LoadPlug.MainPluginDir=FALSE;

					/* $ 03.08.2000 SVS
					  + Новый параметр "Вместо стандарного %FAR%\Plugins искать плагины из
					    указанного пути". При этом персональные тоже грузится будут.
					    IMHO нехрен этому параметру делать в системных настройках!!!
					    /P[<путь>]
					    Причем, <путь> может содержать Env-переменные
					*/
					if (Argv[I][2])
					{
						ExpandEnvironmentStr(&Argv[I][2], Opt.LoadPlug.CustomPluginsPath, sizeof(Opt.LoadPlug.CustomPluginsPath));
						Unquote(Opt.LoadPlug.CustomPluginsPath);
						ConvertNameToFull(Opt.LoadPlug.CustomPluginsPath,Opt.LoadPlug.CustomPluginsPath,sizeof(Opt.LoadPlug.CustomPluginsPath));
						_SVS(SysLog("Opt.LoadPlug.CustomPluginsPath=\"%s\" (0x%08X)",Opt.LoadPlug.CustomPluginsPath,GetFileAttributes(Opt.LoadPlug.CustomPluginsPath)));

						/*
						            if(Argv[I][2]=='.' && (Argv[I][3]==0 || Argv[I][3]=='\\' || Argv[I][3]=='/' || Argv[I][3]=='.'))
						            {
						              GetCurrentDirectory(sizeof(Opt.LoadPlug.CustomPluginsPath),Opt.LoadPlug.CustomPluginsPath);
						              AddEndSlash(Opt.LoadPlug.CustomPluginsPath);
						              strcat(Opt.LoadPlug.CustomPluginsPath,&Argv[I][2]);
						              //printf("%s\n",Opt.LoadPlug.CustomPluginsPath);
						              //Sleep(5000);
						            }
						            else
						              xstrncpy(Opt.LoadPlug.CustomPluginsPath,&Argv[I][2],sizeof(Opt.LoadPlug.CustomPluginsPath));
						*/

						/* 18.01.2003 IS
						   - Не правильно обрабатывалась команда /p[<path>], если в пути
						     были буквы национального алфавита.
						*/
						if (GetFileAttributes(Opt.LoadPlug.CustomPluginsPath) == -1)
							FAR_CharToOem(Opt.LoadPlug.CustomPluginsPath,Opt.LoadPlug.CustomPluginsPath);

						_SVS(SysLog("Opt.LoadPlug.CustomPluginsPath=\"%s\" (0x%08X)",Opt.LoadPlug.CustomPluginsPath,GetFileAttributes(Opt.LoadPlug.CustomPluginsPath)));
						/* IS $ */
					}
					else
					{
						// если указан -P без <путь>, то, считаем, что основные
						//  плагины не загружать вооообще!!!
						Opt.LoadPlug.CustomPluginsPath[0]=0;
					}

					break;
					/* SVS $*/
				}
				/* $ 01.09.2000 tran
				   /co switch support */
				case 'C':

					if (toupper(Argv[I][2])=='O' && !Argv[I][3])
					{
						Opt.LoadPlug.PluginsCacheOnly=TRUE;
						Opt.LoadPlug.PluginsPersonal=FALSE;
					}
					break;
					/* tran $ */
					/* $ 19.01.2001 SVS
					   FAR.EXE /?
					*/
				case '?':
				case 'H':
					ControlObject::ShowCopyright(1);
					show_help();
					exit(0);
					/* SVS $ */
#ifdef DIRECT_RT
				case 'D':

					if (toupper(Argv[I][2])=='O' && !Argv[I][3])
						DirectRT=1;

					break;
#endif
			}
		}
		else // простые параметры. Их может быть max две штукА.
		{
			if (CntDestName < 2)
			{
				_SVS(int tempCntDestName=CntDestName);
				ExpandEnvironmentStr(Argv[I], DestName[CntDestName],sizeof(DestName[CntDestName]));
				Unquote(DestName[CntDestName]);
				ConvertNameToFull(DestName[CntDestName],DestName[CntDestName],sizeof(DestName[CntDestName]));
				_SVS(SysLog("CntDestName=%d DestName[CntDestName]=\"%s\" (0x%08X)",CntDestName,DestName[CntDestName],GetFileAttributes(DestName[CntDestName])));

				if (GetFileAttributes(DestName[CntDestName]) == -1)
					FAR_CharToOem(DestName[CntDestName],DestName[CntDestName]);

				if (GetFileAttributes(DestName[CntDestName]) != -1)
					CntDestName++;

				_SVS(SysLog("CntDestName=%d DestName[CntDestName]=\"%s\" (0x%08X)",tempCntDestName,DestName[tempCntDestName],GetFileAttributes(DestName[tempCntDestName])));
			}
		}
	}

	InitializeSetupAPI();
	/* $ 26.03.2002 IS
	   Настройка сортировки.
	   Должна быть после CopyGlobalSettings и перед InitKeysArray!
	*/
	InitLCIDSort();
	/* IS $ */
	/* $ 11.01.2002 IS
	   Инициализация массива клавиш. Должна быть после CopyGlobalSettings!
	*/
	InitKeysArray();
	/* IS $ */
	WaitForInputIdle(GetCurrentProcess(),0);
	TConsoleRestore __ConsoleRestore(RectoreConsole);
#if (defined(__BORLANDC__) && !defined(_WIN64)) || (defined(_MSC_VER) && _MSC_VER<1400)
#if (defined(_MSC_VER) && _MSC_VER<1400)
	_set_new_handler(0);
#else
	set_new_handler(0);
#endif
#else
	std::set_new_handler(0);
#endif

	if (!Opt.LoadPlug.MainPluginDir) //если есть ключ /p то он отменяет /co
		Opt.LoadPlug.PluginsCacheOnly=FALSE;

	if (Opt.LoadPlug.PluginsCacheOnly)
	{
		*Opt.LoadPlug.CustomPluginsPath=0;
		Opt.LoadPlug.MainPluginDir=FALSE;
		Opt.LoadPlug.PluginsPersonal=FALSE;
	}

#if 0
	SetFileApisTo(APIS2OEM);
	GetModuleFileName(NULL,FarPath,sizeof(FarPath));
#if defined(USE_WFUNC)
//  if(Opt.CleanAscii || Opt.NoGraphics)
//    Opt.UseUnicodeConsole=FALSE;
#endif
	// $ 02.07.2001 IS - Учтем то, что GetModuleFileName иногда возвращает короткое имя, которое нам нафиг не нужно.
	*(PointToName(FarPath)-1)=0;
	{
		char tmpFarPath[sizeof(FarPath)];
		DWORD s=RawConvertShortNameToLongName(FarPath, tmpFarPath,
		                                      sizeof(tmpFarPath));

		if (s && s<sizeof(tmpFarPath))
			strcpy(FarPath, tmpFarPath);
	}
	{
		FAR_OemToChar(FarPath, FarPath);
		SetEnvironmentVariable("FARHOME",FarPath);
		FAR_CharToOem(FarPath, FarPath);
	}
	AddEndSlash(FarPath);
#endif
	/* $ 03.08.2000 SVS
	   Если не указан параметр -P
	*/
//  if(Opt.LoadPlug.MainPluginDir)
//    sprintf(Opt.LoadPlug.CustomPluginsPath,"%s%s",FarPath,PluginsFolderName);
	/* SVS $*/
	InitDetectWindowedMode();
	InitConsole();
	GetRegKey("Language","Main",Opt.Language,"English",sizeof(Opt.Language));

	if (!Lang.Init(FarPath,MNewFileName))
	{
		/* $ 15.12.2000 SVS
		   В случае, если не нашли нужных LNG-файлов - выдаем простое сообщение
		   и не выпендрючиваемся.
		*/
		//Message(MSG_WARNING,1,"Error","Cannot load language data","Ok");
		ControlObject::ShowCopyright(1);
		fprintf(stderr,"\nError: Cannot load language data\n\nPress any key...");
		FlushConsoleInputBuffer(hConInp);
		WaitKey(); // А стоит ли ожидать клавишу??? Стоит
		exit(0);
		/* SVS $ */
	}

	strcpy(InitedLanguage,Opt.Language); // скорректируем инициализирующую строку лэнгвича
	SetEnvironmentVariable("FARLANG",Opt.Language);
	ConvertOldSettings();
	SetHighlighting();
	/* $ 07.03.2001 IS
	   - Избавление от падения при удалении папок в корзину (на основе информации
	     от Веши).Удаляем неугодную ветку, если она пуста, потому что в этом
	     случае Фар может упасть при удалении каталогов в корзину.
	*/
	{
		DeleteEmptyKey(HKEY_CLASSES_ROOT,"Directory\\shellex\\CopyHookHandlers");
	}
	/* IS $ */
	initMacroVarTable(1);
	int Result=0;
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX | (Opt.ExceptRules?SEM_NOGPFAULTERRORBOX:0)
#if defined (_M_IA64) && defined (_WIN64)
	             | (GetRegKey("System\\Exception","IgnoreDataAlignmentFaults",0)? SEM_NOALIGNMENTFAULTEXCEPT:0)
#endif
	            );

	if (Opt.ExceptRules)
	{
		Result=main_sehed(EditName,ViewName,DestName[0],DestName[1],StartLine,StartChar);
	}
	else
		Result=MainProcess(EditName,ViewName,DestName[0],DestName[1],StartLine,StartChar);

	UsedInternalClipboard=TRUE;
	FAR_EmptyClipboard();
	FinalizeSetupAPI();
	doneMacroVarTable(1);
	_OT(SysLog("[[[[[Exit of FAR]]]]]]]]]"));
	return Result;
}

void ConvertOldSettings()
{
	// Конвертим реестр :-) Бывает же такое...
	if (!CheckRegKey(RegColorsHighlight))
		if (CheckRegKey("Highlight"))
		{
			char NameSrc[NM],NameDst[NM];
			strcpy(NameSrc,Opt.RegRoot);
			strcat(NameSrc,"\\Highlight");
			strcpy(NameDst,Opt.RegRoot);
			strcat(NameDst,"\\");
			strcat(NameDst,RegColorsHighlight);
			CopyKeyTree(NameSrc,NameDst,"\0");
		}

	DeleteKeyTree("Highlight");
}

/* $ 03.08.2000 SVS
  ! Не срабатывал шаблон поиска файлов для под-юзеров
*/
void CopyGlobalSettings()
{
	if (CheckRegKey("")) // при существующем - вываливаемся
		return;

	// такого извера нету - перенесем данные!
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0");
	SetRegRootKey(HKEY_CURRENT_USER);
	CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0Software\\Far\\PluginsCache\0");
	//  "Вспомним" путь по шаблону!!!
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	GetRegKey("System","TemplatePluginsPath",Opt.LoadPlug.PersonalPluginsPath,"",sizeof(Opt.LoadPlug.PersonalPluginsPath));
	// удалим!!!
	DeleteRegKey("System");
	// запишем новое значение!
	SetRegRootKey(HKEY_CURRENT_USER);
	SetRegKey("System","PersonalPluginsPath",Opt.LoadPlug.PersonalPluginsPath);
}
/* SVS $ */
