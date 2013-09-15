/*
main.cpp

Функция main.
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "imports.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "clipboard.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "dirmix.hpp"
#include "elevation.hpp"
#include "cmdline.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "colormix.hpp"
#include "treelist.hpp"
#include "plugins.hpp"
#include "notification.hpp"
#include "message.hpp"

global *Global = nullptr;

static void show_help()
{
	WCHAR HelpMsg[]=
		L"Usage: far [switches] [apath [ppath]]\n\n"
		L"where\n"
		L"  apath - path to a folder (or a file or an archive or command with prefix)\n"
		L"          for the active panel\n"
		L"  ppath - path to a folder (or a file or an archive or command with prefix)\n"
		L"          for the passive panel\n\n"
		L"The following switches may be used in the command line:\n\n"
		L" /?   This help.\n"
		L" /a   Disable display of characters with codes 0 - 31 and 255.\n"
		L" /ag  Disable display of pseudographics characters.\n"
		L" /co  Forces FAR to load plugins from the cache only.\n"
#ifdef DIRECT_RT
		L" /do  Direct output.\n"
#endif
		L" /e[<line>[:<pos>]] <filename>\n"
		L"      Edit the specified file.\n"
		L" /m   Do not load macros.\n"
		L" /ma  Do not execute auto run macros.\n"
		L" /p[<path>]\n"
		L"      Search for \"common\" plugins in the directory, specified by <path>.\n"
		L" /s <profilepath> [<localprofilepath>]\n"
		L"      Custom location for Far configuration files - overrides Far.exe.ini.\n"
		L" /t <path>\n"
		L"      Location of Far template configuration file - overrides Far.exe.ini.\n"
#ifndef NO_WRAPPER
		L" /u <username>\n"
		L"      Allows to have separate registry settings for different users.\n"
		L"      Affects only 1.x Far Manager plugins\n"
#endif // NO_WRAPPER
		L" /v <filename>\n"
		L"      View the specified file. If <filename> is -, data is read from the stdin.\n"
		L" /w[-] Stretch to console window instead of console buffer or vise versa.\n"
		L" /clearcache [profilepath [localprofilepath]]\n"
		L"      Clear plugins cache.\n"
		L" /export <out.farconfig> [profilepath [localprofilepath]]\n"
		L"      Export settings.\n"
		L" /import <in.farconfig> [profilepath [localprofilepath]]\n"
		L"      Import settings.\n"
		L" /ro  Read-Only config mode.\n"
		L" /rw  Normal config mode.\n"
		;
	Global->Console->Write(HelpMsg, ARRAYSIZE(HelpMsg)-1);
	Global->Console->Commit();
}

static int MainProcess(
    const string& lpwszEditName,
    const string& lpwszViewName,
    const string& lpwszDestName1,
    const string& lpwszDestName2,
    int StartLine,
    int StartChar
)
{
		ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
		FarColor InitAttributes={};
		Global->Console->GetTextAttributes(InitAttributes);
		SetRealColor(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));

		string ename(lpwszEditName),vname(lpwszViewName), apanel(lpwszDestName1),ppanel(lpwszDestName2);
		if (Global->Db->ShowProblems() > 0)
		{
			ename.clear();
			vname.clear();
			StartLine = StartChar = -1;
			apanel = Global->Opt->ProfilePath;
			ppanel = Global->Opt->LocalProfilePath;
		}

		if (!ename.empty() || !vname.empty())
		{
			Panel* DummyPanel=new dummy_panel;
			_tran(SysLog(L"create dummy panels"));
			Global->CtrlObject->CreateFilePanels();
			Global->CtrlObject->Cp()->LeftPanel=Global->CtrlObject->Cp()->RightPanel=Global->CtrlObject->Cp()->ActivePanel=DummyPanel;
			Global->CtrlObject->Plugins->LoadPlugins();
			Global->CtrlObject->Macro.LoadMacros(true,false);

			if (!ename.empty())
			{
				Global->Opt->OnlyEditorViewerUsed=1;
				FileEditor *ShellEditor=new FileEditor(ename,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6,StartLine,StartChar);
				_tran(SysLog(L"make shelleditor %p",ShellEditor));

				if (!ShellEditor->GetExitCode())  // ????????????
				{
					FrameManager->ExitMainLoop(0);
				}
			}
			// TODO: Этот else убрать только после разборок с возможностью задавать несколько /e и /v в ком.строке
			else if (!vname.empty())
			{
				Global->Opt->OnlyEditorViewerUsed=2;
				FileViewer *ShellViewer=new FileViewer(vname,FALSE);

				if (!ShellViewer->GetExitCode())
				{
					FrameManager->ExitMainLoop(0);
				}

				_tran(SysLog(L"make shellviewer, %p",ShellViewer));
			}

			FrameManager->EnterMainLoop();
			Global->CtrlObject->Cp()->LeftPanel=Global->CtrlObject->Cp()->RightPanel=Global->CtrlObject->Cp()->ActivePanel=nullptr;
			DummyPanel->Destroy();
			_tran(SysLog(L"editor/viewer closed, delete dummy panels"));
		}
		else
		{
			Global->Opt->OnlyEditorViewerUsed=0;
			int DirCount=0;
			string strPath;

			// воспользуемся тем, что ControlObject::Init() создает панели
			// юзая Global->Opt->*

			auto SetupPanel = [&](bool active)
			{
				++DirCount;
				strPath = active? apanel : ppanel;
				CutToNameUNC(strPath);
				DeleteEndSlash(strPath); //BUGBUG!! если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

				bool Root = false;
				PATH_TYPE Type = ParsePath(strPath, nullptr, &Root);
				if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
				{
					AddEndSlash(strPath);
				}

				auto& CurrentPanelOptions = (Global->Opt->LeftFocus && active)? Global->Opt->LeftPanel : Global->Opt->RightPanel;
				CurrentPanelOptions.Type=FILE_PANEL;  // сменим моду панели
				CurrentPanelOptions.Visible=TRUE;     // и включим ее
				CurrentPanelOptions.Folder = strPath;
			};

			if (!apanel.empty())
			{
				SetupPanel(true);

				if (!ppanel.empty())
				{
					SetupPanel(false);
				}
			}

			// теперь все готово - создаем панели!
			Global->CtrlObject->Init(DirCount);

			// а теперь "провалимся" в каталог или хост-файл (если получится ;-)
			if (!apanel.empty())  // актиная панель
			{
				Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (!ppanel.empty())  // пассивная панель
				{
					FarChDir(AnotherPanel->GetCurDir());

					if (IsPluginPrefixPath(ppanel))
					{
						AnotherPanel->SetFocus();
						Global->CtrlObject->CmdLine->ExecString(ppanel,0);
						ActivePanel->SetFocus();
					}
					else
					{
						strPath = PointToName(ppanel);

						if (!strPath.empty())
						{
							if (AnotherPanel->GoToFile(strPath))
								AnotherPanel->ProcessKey(KEY_CTRLPGDN);
						}
					}
				}

				FarChDir(ActivePanel->GetCurDir());

				if (IsPluginPrefixPath(apanel))
				{
					Global->CtrlObject->CmdLine->ExecString(apanel,0);
				}
				else
				{
					strPath = PointToName(apanel);

					if (!strPath.empty())
					{
						if (ActivePanel->GoToFile(strPath))
							ActivePanel->ProcessKey(KEY_CTRLPGDN);
					}
				}

				// !!! ВНИМАНИЕ !!!
				// Сначала редравим пассивную панель, а потом активную!
				AnotherPanel->Redraw();
				ActivePanel->Redraw();
			}

			FrameManager->EnterMainLoop();
		}

		TreeList::FlushCache();

		// очистим за собой!
		SetScreen(0,0,ScrX,ScrY,L' ',ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		Global->Console->SetTextAttributes(InitAttributes);
		Global->ScrBuf->ResetShadow();
		Global->ScrBuf->ResetLockCount();
		Global->ScrBuf->Flush();
		MoveRealCursor(0,0);

		return 0;
}

#ifndef _MSC_VER
static LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	return xfilter(L"FarUnhandledExceptionFilter", ExceptionInfo);
}
#endif

static void InitTemplateProfile(string &strTemplatePath)
{
	if (strTemplatePath.empty())
	{
		strTemplatePath = GetFarIniString(L"General", L"TemplateProfile", L"%FARHOME%\\Default.farconfig");
	}

	if (!strTemplatePath.empty())
	{
		strTemplatePath = api::ExpandEnvironmentStrings(strTemplatePath);
		Unquote(strTemplatePath);
		ConvertNameToFull(strTemplatePath, strTemplatePath);
		DeleteEndSlash(strTemplatePath);

		DWORD attr = api::GetFileAttributes(strTemplatePath);
		if (INVALID_FILE_ATTRIBUTES != attr && 0 != (attr & FILE_ATTRIBUTE_DIRECTORY))
			strTemplatePath += L"\\Default.farconfig";

		Global->Opt->TemplateProfilePath = strTemplatePath;
	}
}

static void InitProfile(string &strProfilePath, string &strLocalProfilePath)
{
	if (!strProfilePath.empty())
	{
		strProfilePath = api::ExpandEnvironmentStrings(strProfilePath);
		Unquote(strProfilePath);
		ConvertNameToFull(strProfilePath,strProfilePath);
	}
	if (!strLocalProfilePath.empty())
	{
		strLocalProfilePath = api::ExpandEnvironmentStrings(strLocalProfilePath);
		Unquote(strLocalProfilePath);
		ConvertNameToFull(strLocalProfilePath,strLocalProfilePath);
	}

	if (strProfilePath.empty())
	{
		int UseSystemProfiles = GetPrivateProfileInt(L"General", L"UseSystemProfiles", 1, Global->g_strFarINI.data());
		if (UseSystemProfiles)
		{
			// roaming data default path: %APPDATA%\Far Manager\Profile
			wchar_t Buffer[MAX_PATH];
			SHGetFolderPath(nullptr, CSIDL_APPDATA|CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, Buffer);
			Global->Opt->ProfilePath = Buffer;
			AddEndSlash(Global->Opt->ProfilePath);
			Global->Opt->ProfilePath += L"Far Manager";

			if (UseSystemProfiles == 2)
			{
				Global->Opt->LocalProfilePath = Global->Opt->ProfilePath;
			}
			else
			{
				// local data default path: %LOCALAPPDATA%\Far Manager\Profile
				SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, Buffer);
				Global->Opt->LocalProfilePath = Buffer;
				AddEndSlash(Global->Opt->LocalProfilePath);
				Global->Opt->LocalProfilePath += L"Far Manager";
			}

			string* Paths[]={&Global->Opt->ProfilePath, &Global->Opt->LocalProfilePath};
			std::for_each(RANGE(Paths, i)
			{
				AddEndSlash(*i);
				*i += L"Profile";
				CreatePath(*i, true);
			});
		}
		else
		{
			string strUserProfileDir = GetFarIniString(L"General", L"UserProfileDir", L"%FARHOME%\\Profile");
			string strUserLocalProfileDir = GetFarIniString(L"General", L"UserLocalProfileDir", strUserProfileDir);
			strUserProfileDir = api::ExpandEnvironmentStrings(strUserProfileDir);
			strUserLocalProfileDir = api::ExpandEnvironmentStrings(strUserLocalProfileDir);
			Unquote(strUserProfileDir);
			Unquote(strUserLocalProfileDir);
			ConvertNameToFull(strUserProfileDir, strUserProfileDir);
			ConvertNameToFull(strUserLocalProfileDir, strUserLocalProfileDir);
			Global->Opt->ProfilePath = strUserProfileDir;
			Global->Opt->LocalProfilePath = strUserLocalProfileDir;
		}
	}
	else
	{
		Global->Opt->ProfilePath = strProfilePath;
		Global->Opt->LocalProfilePath = strLocalProfilePath.empty() ? strProfilePath : strLocalProfilePath;
	}

	CreatePath(Global->Opt->ProfilePath + L"\\PluginsData", true);
	if (Global->Opt->ProfilePath != Global->Opt->LocalProfilePath)
		CreatePath(Global->Opt->LocalProfilePath, true);

	Global->Opt->LoadPlug.strPersonalPluginsPath = Global->Opt->ProfilePath + L"\\Plugins";

	api::SetEnvironmentVariable(L"FARPROFILE", Global->Opt->ProfilePath);
	api::SetEnvironmentVariable(L"FARLOCALPROFILE", Global->Opt->LocalProfilePath);

	if (Global->Opt->ReadOnlyConfig < 0) // do not override 'far /ro', 'far /rw'
		Global->Opt->ReadOnlyConfig = GetPrivateProfileInt(L"General", L"ReadOnlyConfig", FALSE, Global->g_strFarINI.data());
}

static int mainImpl(int Argc, wchar_t *Argv[])
{
	global _g;

	SetErrorMode(Global->ErrorMode);

	TestPathParser();

	// Starting with Windows Vista, the system uses the low-fragmentation heap (LFH) as needed to service memory allocation requests.
	// Applications do not need to enable the LFH for their heaps.
	if(Global->WinVer() < _WIN32_WINNT_VISTA)
	{
		api::EnableLowFragmentationHeap();
	}

	if(!Global->Console->IsFullscreenSupported())
	{
		const BYTE ReserveAltEnter = 0x8;
		Global->ifn->SetConsoleKeyShortcuts(TRUE, ReserveAltEnter, nullptr, 0);
	}

	api::InitCurrentDirectory();

	if (api::GetModuleFileName(nullptr, Global->g_strFarModuleName))
	{
		ConvertNameToLong(Global->g_strFarModuleName, Global->g_strFarModuleName);
		PrepareDiskPath(Global->g_strFarModuleName);
	}

	Global->g_strFarINI = Global->g_strFarModuleName+L".ini";
	Global->g_strFarPath = Global->g_strFarModuleName;
	CutToSlash(Global->g_strFarPath,true);
	api::SetEnvironmentVariable(L"FARHOME", Global->g_strFarPath);
	AddEndSlash(Global->g_strFarPath);

#ifndef NO_WRAPPER
	// don't inherit from parent process in any case
	// for OEM plugins only!
	api::DeleteEnvironmentVariable(L"FARUSER");
#endif // NO_WRAPPER

	if (Global->IsUserAdmin())
		api::SetEnvironmentVariable(L"FARADMINMODE", L"1");
	else
		api::DeleteEnvironmentVariable(L"FARADMINMODE");

	if (Argc==5 && !StrCmp(Argv[1], L"/elevation")) // /elevation {GUID} PID UsePrivileges
	{
		return ElevationMain(Argv[2], _wtoi(Argv[3]), *Argv[4]==L'1');
	}
	else if (Argc <= 6 && Argc >= 3 && (!StrCmpI(Argv[1], L"/export") || !StrCmpI(Argv[1], L"/import")))
	{
		bool Export = !StrCmpI(Argv[1], L"/export");
		string strProfilePath(Argc>3 ? Argv[3] : L""), strLocalProfilePath(Argc>4 ? Argv[4] : L""), strTemplatePath(Argc>5 ? Argv[5] : L"");
		InitTemplateProfile(strTemplatePath);
		InitProfile(strProfilePath, strLocalProfilePath);
		Global->Db = new Database(Export ? 'e' : 'i');
		return !(Export? Global->Db->Export(Argv[2]) : Global->Db->Import(Argv[2]));
	}
	else if (Argc>=2 && Argc<=4 && !StrCmpI(Argv[1], L"/clearcache"))
	{
		string strProfilePath(Argc>2 ? Argv[2] : L"");
		string strLocalProfilePath(Argc>3 ? Argv[3] : L"");
		InitProfile(strProfilePath, strLocalProfilePath);
		Database::ClearPluginsCache();
		return 0;
	}

	listener EnvironmentListener(L"environment", []()
	{
		ReloadEnvironment();
	});

	_OT(SysLog(L"[[[[[[[[New Session of FAR]]]]]]]]]"));

	string strEditName;
	string strViewName;
	string DestNames[2];
	int StartLine=-1,StartChar=-1;
	int CntDestName=0; // количество параметров-имен каталогов

	string strProfilePath, strLocalProfilePath, strTemplatePath;

	for (int I=1; I<Argc; I++)
	{
		if ((Argv[I][0]==L'/' || Argv[I][0]==L'-') && Argv[I][1])
		{
			switch (Upper(Argv[I][1]))
			{
				case L'A':

					switch (Upper(Argv[I][2]))
					{
						case 0:
							Global->Opt->CleanAscii=TRUE;
							break;
						case L'G':

							if (!Argv[I][3])
								Global->Opt->NoGraphics=TRUE;

							break;
					}

					break;
				case L'E':

					if (iswdigit(Argv[I][2]))
					{
						StartLine=_wtoi(&Argv[I][2]);
						const wchar_t *ChPtr=wcschr(&Argv[I][2],L':');

						if (ChPtr)
							StartChar=_wtoi(ChPtr+1);
					}

					if (I+1<Argc)
					{
						strEditName = Argv[I+1];
						I++;
					}

					break;
				case L'V':

					if (I+1<Argc)
					{
						strViewName = Argv[I+1];
						I++;
					}

					break;
				case L'M':

					switch (Upper(Argv[I][2]))
					{
						case 0:
							Global->Opt->Macro.DisableMacro|=MDOL_ALL;
							break;
						case L'A':

							if (!Argv[I][3])
								Global->Opt->Macro.DisableMacro|=MDOL_AUTOSTART;

							break;
					}

					break;
#ifndef NO_WRAPPER
				case L'U':

					if (I+1<Argc)
					{
						//Affects OEM plugins only!
						Global->strRegRoot.append(L"\\Users\\").append(Argv[I+1]);
						api::SetEnvironmentVariable(L"FARUSER", Argv[I+1]);
						I++;
					}
					break;
#endif // NO_WRAPPER

				case L'S':

					if (I+1<Argc)
					{
						strProfilePath = Argv[I+1];
						I++;
						if (I+1<Argc && Argv[I+1][0] != L'-'  && Argv[I+1][0] != L'/')
						{
							strLocalProfilePath = Argv[I+1];
							I++;
						}
					}
					break;

				case L'T':
					if (I+1<Argc)
					{
						strTemplatePath = Argv[++I];
					}
					break;

				case L'P':
				{
					// Полиция 19 - BUGBUG ни кто эту опцию вообще не читал
					//if (Global->Opt->Policies.DisabledOptions&FFPOL_USEPSWITCH)
						//break;

					Global->Opt->LoadPlug.PluginsPersonal = false;
					Global->Opt->LoadPlug.MainPluginDir = false;

					if (Argv[I][2])
					{
						Global->Opt->LoadPlug.strCustomPluginsPath = api::ExpandEnvironmentStrings(&Argv[I][2]);
						Unquote(Global->Opt->LoadPlug.strCustomPluginsPath);
						ConvertNameToFull(Global->Opt->LoadPlug.strCustomPluginsPath, Global->Opt->LoadPlug.strCustomPluginsPath);
					}
					else
					{
						// если указан -P без <путь>, то, считаем, что основные
						//  плагины не загружать вооообще!!!
						Global->Opt->LoadPlug.strCustomPluginsPath.clear();
					}

					break;
				}
				case L'C':

					if (Upper(Argv[I][2])==L'O' && !Argv[I][3])
					{
						Global->Opt->LoadPlug.PluginsCacheOnly = true;
						Global->Opt->LoadPlug.PluginsPersonal = false;
					}

					break;
				case L'?':
				case L'H':
					ControlObject::ShowCopyright(1);
					show_help();
					return 0;
#ifdef DIRECT_RT
				case L'D':

					if (Upper(Argv[I][2])==L'O' && !Argv[I][3])
						Global->DirectRT=true;

					break;
#endif
				case L'W':
					{
						if(Argv[I][2] == L'-')
						{
							Global->Opt->WindowMode= false;
						}
						else if(!Argv[I][2])
						{
							Global->Opt->WindowMode= true;
						}
					}
					break;
				case L'R':
					if (Upper(Argv[I][2]) == L'O') // -ro
						Global->Opt->ReadOnlyConfig = TRUE;
					if (Upper(Argv[I][2]) == L'W') // -rw
						Global->Opt->ReadOnlyConfig = FALSE;
					break;
			}
		}
		else // простые параметры. Их может быть max две штукА.
		{
			if (CntDestName < 2)
			{
				string ArgvI(Argv[I]);
				if (IsPluginPrefixPath(ArgvI))
				{
					DestNames[CntDestName++] = ArgvI;
				}
				else
				{
					ArgvI = api::ExpandEnvironmentStrings(ArgvI);
					Unquote(ArgvI);
					ConvertNameToFull(ArgvI, ArgvI);

					if (api::GetFileAttributes(ArgvI) != INVALID_FILE_ATTRIBUTES)
					{
						DestNames[CntDestName++] = ArgvI;
					}
				}
			}
		}
	}

	InitTemplateProfile(strTemplatePath);
	InitProfile(strProfilePath, strLocalProfilePath);
	Global->Db = new Database;

	Global->Opt->Load();

	//Инициализация массива клавиш.
	InitKeysArray();
	WaitForInputIdle(GetCurrentProcess(),0);

	if (!Global->Opt->LoadPlug.MainPluginDir) //если есть ключ /p то он отменяет /co
		Global->Opt->LoadPlug.PluginsCacheOnly=false;

	if (Global->Opt->LoadPlug.PluginsCacheOnly)
	{
		Global->Opt->LoadPlug.strCustomPluginsPath.clear();
		Global->Opt->LoadPlug.MainPluginDir=false;
		Global->Opt->LoadPlug.PluginsPersonal=false;
	}

	InitConsole();

	if (!Global->Lang->Init(Global->g_strFarPath, MNewFileName))
	{
		ControlObject::ShowCopyright(1);
		const char* LngMsg;
		switch(Global->Lang->GetLastError())
		{
		case LERROR_BAD_FILE:
			LngMsg = "Language data is incorrect or damaged";
			break;
		case LERROR_FILE_NOT_FOUND:
			LngMsg = "Cannot find language data";
			break;
		default:
			LngMsg = "Cannot load language data";
			break;
		}
		throw FarException(LngMsg);
	}

	api::SetEnvironmentVariable(L"FARLANG", Global->Opt->strLanguage);

	Global->ErrorMode=SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX|SEM_NOGPFAULTERRORBOX;
	long long IgnoreDataAlignmentFaults = 0;
	Global->Db->GeneralCfg()->GetValue(L"System.Exception", L"IgnoreDataAlignmentFaults", &IgnoreDataAlignmentFaults, IgnoreDataAlignmentFaults);
	if (IgnoreDataAlignmentFaults)
	{
		Global->ErrorMode|=SEM_NOALIGNMENTFAULTEXCEPT;
	}
	SetErrorMode(Global->ErrorMode);

	long long InitDriveMenuHotkeys = 1;
	Global->Db->GeneralCfg()->GetValue(L"Interface", L"InitDriveMenuHotkeys", &InitDriveMenuHotkeys, InitDriveMenuHotkeys);
	if(InitDriveMenuHotkeys)
	{
		static const struct
		{
			const wchar_t *PluginId, *MenuId, *Hotkey;
		}
		DriveMenuHotkeys[] =
		{
			{L"1E26A927-5135-48C6-88B2-845FB8945484", L"61026851-2643-4C67-BF80-D3C77A3AE830", L"0"}, // ProcList
			{L"B77C964B-E31E-4D4C-8FE5-D6B0C6853E7C", L"F98C70B3-A1AE-4896-9388-C5C8E05013B7", L"1"}, // TmpPanel
			{L"42E4AEB1-A230-44F4-B33C-F195BB654931", L"C9FB4F53-54B5-48FF-9BA2-E8EB27F012A2", L"2"}, // NetBox
			{L"773B5051-7C5F-4920-A201-68051C4176A4", L"24B6DD41-DF12-470A-A47C-8675ED8D2ED4", L"3"}, // Network
		};

		std::for_each(CONST_RANGE(DriveMenuHotkeys, i)
		{
			Global->Db->PlHotkeyCfg()->SetHotkey(i.PluginId, i.MenuId, PluginsHotkeysConfig::DRIVE_MENU, i.Hotkey);
		});

		Global->Db->GeneralCfg()->SetValue(L"Interface",L"InitDriveMenuHotkeys", 0ull);
	}

	Global->CtrlObject = new ControlObject;

	int Result = 1;

	try
	{
		Result = MainProcess(strEditName, strViewName, DestNames[0], DestNames[1], StartLine, StartChar);
	}
	catch (const SException& e)
	{
		if (xfilter(L"mainImpl", e.GetInfo()) == EXCEPTION_EXECUTE_HANDLER)
			std::terminate();
	}
	catch (const std::exception& e)
	{
		if (Message(MSG_WARNING, 2, MSG(MExcTrappedException), wide(e.what()).data(), MSG(MExcTerminate), MSG(MExcDebugger)) == 0)
			std::terminate();
	}

	CloseConsole();

	EmptyInternalClipboard();

	_OT(SysLog(L"[[[[[Exit of FAR]]]]]]]]]"));

	return Result;
}

int wmain(int Argc, wchar_t *Argv[])
{
	try
	{
		atexit(PrintMemory);
		_wsetlocale(LC_ALL, L"");
		std::set_new_handler(nullptr);
		EnableSeTranslation();
#ifndef _MSC_VER
		SetUnhandledExceptionFilter(FarUnhandledExceptionFilter);
#endif

		return mainImpl(Argc, Argv);
	}
	catch (const SException& e)
	{
		if (xfilter(L"wmain", e.GetInfo()) == EXCEPTION_EXECUTE_HANDLER)
			std::terminate();
	}
	catch (const std::exception& e)
	{
		std::wcerr << L"\nException: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::wcerr << L"\nUnknown exception" << std::endl;
	}
	return 1;
}

#ifdef __GNUC__
int main()
{
	int nArgs;
	LPWSTR* wstrCmdLineArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	int Result=wmain(nArgs, wstrCmdLineArgs);
	LocalFree(wstrCmdLineArgs);
	return Result;
}
#endif
