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
#include "palette.hpp"

#ifdef DIRECT_RT
int DirectRT=0;
#endif

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
		L" /s <path> [<localpath>]\n"
		L"      Custom location for Far configuration files - overrides Far.exe.ini.\n"
#ifndef NO_WRAPPER
		L" /u <username>\n"
		L"      Allows to have separate registry settings for different users.\n"
		L"      Affects only 1.x Far Manager plugins\n"
#endif // NO_WRAPPER
		L" /v <filename>\n"
		L"      View the specified file. If <filename> is -, data is read from the stdin.\n"
		L" /w[-] Stretch to console window instead of console buffer or vise versa.\n"
		L" /x   Disable exception handling.\n"
		L" /clearcache [profilepath]\n"
		L"      Clear plugins cache.\n"
		L" /export <out.farconfig> [profilepath [localprofilepath]]\n"
		L"      Export settings.\n"
		L" /import <in.farconfig> [profilepath [localprofilepath]]\n"
		L"      Import settings.\n"
#ifdef _DEBUGEXC
		L" /xd  Enable exception handling.\n"
#endif
		L" /ro  Read-Only config mode.\n"
		L" /rw  Normal config mode.\n"
		;
	Console.Write(HelpMsg, ARRAYSIZE(HelpMsg)-1);
	Console.Commit();
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
	{
		ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
		ControlObject CtrlObj;
		FarColor InitAttributes={};
		Console.GetTextAttributes(InitAttributes);
		SetRealColor(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		GetSystemInfo(&SystemInfo);

		string ename(lpwszEditName),vname(lpwszViewName), apanel(lpwszDestName1),ppanel(lpwszDestName2);
		if (ShowProblemDb() > 0)
		{
			ename = vname = "";
			StartLine = StartChar = -1;
			apanel = Opt.ProfilePath;
			ppanel = Opt.LocalProfilePath;
		}

		if (*ename || *vname)
		{
			Opt.OnlyEditorViewerUsed=1;
			Panel *DummyPanel=new Panel;
			_tran(SysLog(L"create dummy panels"));
			CtrlObj.CreateFilePanels();
			CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=DummyPanel;
			CtrlObj.Plugins->LoadPlugins();
			CtrlObj.Macro.LoadMacros(TRUE,FALSE);

			if (*ename)
			{
				FileEditor *ShellEditor=new FileEditor(ename,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6,StartLine,StartChar);
				_tran(SysLog(L"make shelleditor %p",ShellEditor));

				if (!ShellEditor->GetExitCode())  // ????????????
				{
					FrameManager->ExitMainLoop(0);
				}
			}
			// TODO: Этот else убрать только после разборок с возможностью задавать несколько /e и /v в ком.строке
			else if (*vname)
			{
				FileViewer *ShellViewer=new FileViewer(vname,FALSE);

				if (!ShellViewer->GetExitCode())
				{
					FrameManager->ExitMainLoop(0);
				}

				_tran(SysLog(L"make shellviewer, %p",ShellViewer));
			}

			FrameManager->EnterMainLoop();
			CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=nullptr;
			delete DummyPanel;
			_tran(SysLog(L"editor/viewer closed, delete dummy panels"));
		}
		else
		{
			Opt.OnlyEditorViewerUsed=0;
			int DirCount=0;
			string strPath;

			// воспользуемся тем, что ControlObject::Init() создает панели
			// юзая Opt.*
			if (*apanel)  // актиная панель
			{
				++DirCount;
				strPath = apanel;
				CutToNameUNC(strPath);
				DeleteEndSlash(strPath); //BUGBUG!! если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

				bool Root = false;
				PATH_TYPE Type = ParsePath(strPath, nullptr, &Root);
				if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
				{
					AddEndSlash(strPath);
				}

				// Та панель, которая имеет фокус - активна (начнем по традиции с Левой Панели ;-)
				if (Opt.LeftPanel.Focus)
				{
					Opt.LeftPanel.Type=FILE_PANEL;  // сменим моду панели
					Opt.LeftPanel.Visible=TRUE;     // и включим ее
					Opt.strLeftFolder = strPath;
				}
				else
				{
					Opt.RightPanel.Type=FILE_PANEL;
					Opt.RightPanel.Visible=TRUE;
					Opt.strRightFolder = strPath;
				}

				if (*ppanel)  // пассивная панель
				{
					++DirCount;
					strPath = ppanel;
					CutToNameUNC(strPath);
					DeleteEndSlash(strPath); //BUGBUG!! если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

					bool Root = false;
					PATH_TYPE Type = ParsePath(strPath, nullptr, &Root);
					if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
					{
						AddEndSlash(strPath);
					}

					// а здесь наоборот - обрабатываем пассивную панель
					if (Opt.LeftPanel.Focus)
					{
						Opt.RightPanel.Type=FILE_PANEL; // сменим моду панели
						Opt.RightPanel.Visible=TRUE;    // и включим ее
						Opt.strRightFolder = strPath;
					}
					else
					{
						Opt.LeftPanel.Type=FILE_PANEL;
						Opt.LeftPanel.Visible=TRUE;
						Opt.strLeftFolder = strPath;
					}
				}
			}

			// теперь все готово - создаем панели!
			CtrlObj.Init(DirCount);

			// а теперь "провалимся" в каталог или хост-файл (если получится ;-)
			if (*apanel)  // актиная панель
			{
				string strCurDir;
				Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (*ppanel)  // пассивная панель
				{
					AnotherPanel->GetCurDir(strCurDir);
					FarChDir(strCurDir);

					if (IsPluginPrefixPath(ppanel))
					{
						AnotherPanel->SetFocus();
						CtrlObject->CmdLine->ExecString(ppanel,0);
						ActivePanel->SetFocus();
					}
					else
					{
						strPath = PointToNameUNC(ppanel);

						if (!strPath.IsEmpty())
						{
							if (AnotherPanel->GoToFile(strPath))
								AnotherPanel->ProcessKey(KEY_CTRLPGDN);
						}
					}
				}

				ActivePanel->GetCurDir(strCurDir);
				FarChDir(strCurDir);

				if (IsPluginPrefixPath(apanel))
				{
					CtrlObject->CmdLine->ExecString(apanel,0);
				}
				else
				{
					strPath = PointToNameUNC(apanel);

					if (!strPath.IsEmpty())
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

		// очистим за собой!
		SetScreen(0,0,ScrX,ScrY,L' ',ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		Console.SetTextAttributes(InitAttributes);
		ScrBuf.ResetShadow();
		ScrBuf.Flush();
		MoveRealCursor(0,0);
	}
	CloseConsole();
	return 0;
}

LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	return xfilter(EXCEPT_KERNEL, ExceptionInfo, nullptr, 1);
}

int MainProcessSEH(string& strEditName,string& strViewName,string& DestName1,string& DestName2,int StartLine,int StartChar)
{
	int Result=0;
	__try
	{
		Result=MainProcess(strEditName,strViewName,DestName1,DestName2,StartLine,StartChar);
	}
	__except(xfilter(EXCEPT_KERNEL, GetExceptionInformation(), nullptr, 1))
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
	return Result;
}

void InitProfile(string &strProfilePath, string strLocalProfilePath)
{
	if (!strProfilePath.IsEmpty())
	{
		apiExpandEnvironmentStrings(strProfilePath, strProfilePath);
		Unquote(strProfilePath);
		ConvertNameToFull(strProfilePath,strProfilePath);
	}
	if (!strLocalProfilePath.IsEmpty())
	{
		apiExpandEnvironmentStrings(strLocalProfilePath, strLocalProfilePath);
		Unquote(strLocalProfilePath);
		ConvertNameToFull(strLocalProfilePath,strLocalProfilePath);
	}

	if (strProfilePath.IsEmpty())
	{
		int UseSystemProfiles = GetPrivateProfileInt(L"General", L"UseSystemProfiles", 1, g_strFarINI);
		if (UseSystemProfiles)
		{
			// roaming data default path: %APPDATA%\Far Manager\Profile
			wchar_t Buffer[MAX_PATH];
			SHGetFolderPath(nullptr, CSIDL_APPDATA|CSIDL_FLAG_CREATE, nullptr, 0, Buffer);
			AddEndSlash(Buffer);
			Opt.ProfilePath = Buffer;
			Opt.ProfilePath += L"Far Manager";

			if (UseSystemProfiles == 2)
			{
				Opt.LocalProfilePath = Opt.ProfilePath;
			}
			else
			{
				// local data default path: %LOCALAPPDATA%\Far Manager\Profile
				SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, nullptr, 0, Buffer);
				AddEndSlash(Buffer);
				Opt.LocalProfilePath = Buffer;
				Opt.LocalProfilePath += L"Far Manager";
			}

			string Paths[]={Opt.ProfilePath, Opt.LocalProfilePath};
			for (size_t i = 0; i< ARRAYSIZE(Paths); ++i)
			{
				AddEndSlash(Paths[i]);
				Paths[i] += L"Profile";
				CreatePath(Paths[i], true);
			}
			Opt.ProfilePath = Paths[0];
			Opt.LocalProfilePath = Paths[1];
		}
		else
		{
			string strUserProfileDir, strUserLocalProfileDir;
			strUserProfileDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"UserProfileDir", L"%FARHOME%\\Profile", strUserProfileDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, g_strFarINI));
			strUserLocalProfileDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"UserLocalProfileDir", strUserProfileDir.CPtr(), strUserLocalProfileDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, g_strFarINI));
			apiExpandEnvironmentStrings(strUserProfileDir, strUserProfileDir);
			apiExpandEnvironmentStrings(strUserLocalProfileDir, strUserLocalProfileDir);
			Unquote(strUserProfileDir);
			Unquote(strUserLocalProfileDir);
			ConvertNameToFull(strUserProfileDir, strUserProfileDir);
			ConvertNameToFull(strUserLocalProfileDir, strUserLocalProfileDir);
			Opt.ProfilePath = strUserProfileDir;
			Opt.LocalProfilePath = strUserLocalProfileDir;
		}
	}
	else
	{
		Opt.ProfilePath = strProfilePath;
		Opt.LocalProfilePath = strLocalProfilePath.IsEmpty() ? strProfilePath : strLocalProfilePath;
	}

	CreatePath(Opt.ProfilePath + L"\\PluginsData", true);
	if (Opt.ProfilePath != Opt.LocalProfilePath)
		CreatePath(Opt.LocalProfilePath, true);

	Opt.LoadPlug.strPersonalPluginsPath = Opt.ProfilePath + L"\\Plugins";

	SetEnvironmentVariable(L"FARPROFILE", Opt.ProfilePath);
	SetEnvironmentVariable(L"FARLOCALPROFILE", Opt.LocalProfilePath);

	if (Opt.ReadOnlyConfig < 0) // do not override 'far /ro', 'far /rw'
		Opt.ReadOnlyConfig = GetPrivateProfileInt(L"General", L"ReadOnlyConfig", FALSE, g_strFarINI);
}

int ExportImportMain(bool Export, const wchar_t *XML, const wchar_t *ProfilePath, const wchar_t *LocalProfilePath)
{
	string strProfilePath = ProfilePath, strLocalProfilePath = LocalProfilePath;

	InitProfile(strProfilePath, strLocalProfilePath);
	InitDb(true);

	bool ret = ExportImportConfig(Export, XML);

	ReleaseDb();

	return ret ? 0 : 1;
}

int _cdecl wmain(int Argc, wchar_t *Argv[])
{
	ErrorMode=SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;
	SetErrorMode(ErrorMode);

	std::set_new_handler(nullptr);
	QueryPerformanceCounter(&FarUpTime);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &MainThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	MainThreadId = GetCurrentThreadId();
	GetVersionEx(&WinVer);

	// Starting with Windows Vista, the system uses the low-fragmentation heap (LFH) as needed to service memory allocation requests.
	// Applications do not need to enable the LFH for their heaps.
	if(WinVer < _WIN32_WINNT_VISTA)
	{
		apiEnableLowFragmentationHeap();
	}

	if(!Console.IsFullscreenSupported())
	{
		// 0x8 - AltEnter
		ifn.SetConsoleKeyShortcuts(TRUE, 0x8, nullptr, 0);
	}

	InitCurrentDirectory();

	if (apiGetModuleFileName(nullptr, g_strFarModuleName))
	{
		ConvertNameToLong(g_strFarModuleName, g_strFarModuleName);
		PrepareDiskPath(g_strFarModuleName);
	}

	g_strFarINI = g_strFarModuleName+L".ini";
	g_strFarPath = g_strFarModuleName;
	CutToSlash(g_strFarPath,true);
	SetEnvironmentVariable(L"FARHOME", g_strFarPath);
	AddEndSlash(g_strFarPath);

	Opt.IsUserAdmin=IsUserAdmin();

#ifndef NO_WRAPPER
	// don't inherit from parent process in any case
	// for OEM plugins only!
	SetEnvironmentVariable(L"FARUSER", nullptr);
#endif // NO_WRAPPER

	SetEnvironmentVariable(L"FARADMINMODE", Opt.IsUserAdmin?L"1":nullptr);

	if (Argc==5 && !StrCmp(Argv[1], L"/elevation")) // /elevation {GUID} PID UsePrivileges
	{
		return ElevationMain(Argv[2], _wtoi(Argv[3]), *Argv[4]==L'1');
	}
	else if (Argc==5 || Argc==4 || Argc==3)
	{
		if (!StrCmpI(Argv[1], L"/export"))
		{
			return ExportImportMain(true, Argv[2], Argc>3 ? Argv[3] : L"", Argc>4 ? Argv[4] : L"");
		}
		else if (!StrCmpI(Argv[1], L"/import"))
		{
			return ExportImportMain(false, Argv[2], Argc>3 ? Argv[3] : L"", Argc>4 ? Argv[4] : L"");
		}
	}
	else if ((Argc==2 || Argc==3 || Argc==4) && !StrCmpI(Argv[1], L"/clearcache"))
	{
		string strProfilePath = Argc>2 ? Argv[2] : L"";
		string strLocalProfilePath = Argc>3 ? Argv[3] : L"";
		InitProfile(strProfilePath, strLocalProfilePath);
		ClearPluginsCache();
		return 0;
	}

	_OT(SysLog(L"[[[[[[[[New Session of FAR]]]]]]]]]"));
	string strEditName;
	string strViewName;
	string DestNames[2];
	int StartLine=-1,StartChar=-1;
	int CntDestName=0; // количество параметров-имен каталогов

#ifdef _MSC_VER
	/*$ 18.04.2002 SKV
	  Попользуем floating point что бы проинициализировался vc-ный fprtl.
	*/
	{
		float x=1.1f;
		wchar_t buf[15];
		wsprintf(buf,L"%f",x);
	}
#endif
	// если под дебагером, то отключаем исключения однозначно,
	//  иначе - смотря что указал юзвер.
	Opt.ExceptRules = -1;
	Opt.WindowMode = -1;
#ifndef _DEBUGEXC
	if(IsDebuggerPresent())
	{
		Opt.ExceptRules = 0;
	}
#endif
#ifndef NO_WRAPPER
	Opt.strRegRoot = L"Software\\Far Manager";
#endif // NO_WRAPPER
	// По умолчанию - брать плагины из основного каталога
	Opt.LoadPlug.MainPluginDir=TRUE;
	Opt.LoadPlug.PluginsPersonal=TRUE;
	Opt.LoadPlug.PluginsCacheOnly=FALSE;

	// макросы не дисаблим
	Opt.Macro.DisableMacro=0;

	Opt.ReadOnlyConfig = -1; // not initialized

	string strProfilePath, strLocalProfilePath;

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
							Opt.CleanAscii=TRUE;
							break;
						case L'G':

							if (!Argv[I][3])
								Opt.NoGraphics=TRUE;

							break;
					}

					break;
				case L'E':

					if (iswdigit(Argv[I][2]))
					{
						StartLine=_wtoi(&Argv[I][2]);
						wchar_t *ChPtr=wcschr(&Argv[I][2],L':');

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
							Opt.Macro.DisableMacro|=MDOL_ALL;
							break;
						case L'A':

							if (!Argv[I][3])
								Opt.Macro.DisableMacro|=MDOL_AUTOSTART;

							break;
					}

					break;
				case L'X':
					Opt.ExceptRules = 0;
#if defined(_DEBUGEXC)

					if (Upper(Argv[I][2])==L'D' && !Argv[I][3])
						Opt.ExceptRules = 1;

#endif
					break;

#ifndef NO_WRAPPER
				case L'U':

					if (I+1<Argc)
					{
						//Affects OEM plugins only!
						Opt.strRegRoot += L"\\Users\\";
						Opt.strRegRoot += Argv[I+1];
						SetEnvironmentVariable(L"FARUSER", Argv[I+1]);
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

				case L'P':
				{
					// Полиция 19 - BUGBUG ни кто эту опцию вообще не читал
					//if (Opt.Policies.DisabledOptions&FFPOL_USEPSWITCH)
						//break;

					Opt.LoadPlug.PluginsPersonal=FALSE;
					Opt.LoadPlug.MainPluginDir=FALSE;

					if (Argv[I][2])
					{
						apiExpandEnvironmentStrings(&Argv[I][2], Opt.LoadPlug.strCustomPluginsPath);
						Unquote(Opt.LoadPlug.strCustomPluginsPath);
						ConvertNameToFull(Opt.LoadPlug.strCustomPluginsPath, Opt.LoadPlug.strCustomPluginsPath);
					}
					else
					{
						// если указан -P без <путь>, то, считаем, что основные
						//  плагины не загружать вооообще!!!
						Opt.LoadPlug.strCustomPluginsPath.Clear();
					}

					break;
				}
				case L'C':

					if (Upper(Argv[I][2])==L'O' && !Argv[I][3])
					{
						Opt.LoadPlug.PluginsCacheOnly=TRUE;
						Opt.LoadPlug.PluginsPersonal=FALSE;
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
						DirectRT=1;

					break;
#endif
				case L'W':
					{
						if(Argv[I][2] == L'-')
						{
							Opt.WindowMode= false;
						}
						else if(!Argv[I][2])
						{
							Opt.WindowMode= true;
						}
					}
					break;
				case L'R':
					if (Upper(Argv[I][2]) == L'O') // -ro
						Opt.ReadOnlyConfig = TRUE;
					if (Upper(Argv[I][2]) == L'W') // -rw
						Opt.ReadOnlyConfig = FALSE;
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
					apiExpandEnvironmentStrings(ArgvI, DestNames[CntDestName]);
					Unquote(DestNames[CntDestName]);
					ConvertNameToFull(DestNames[CntDestName],DestNames[CntDestName]);

					if (apiGetFileAttributes(DestNames[CntDestName]) != INVALID_FILE_ATTRIBUTES)
						CntDestName++; //???
				}
			}
		}
	}

	InitProfile(strProfilePath, strLocalProfilePath);

	InitDb();

	ReadConfig();

	//Настройка OEM сортировки
#ifndef NO_WRAPPER
	wrapper::LocalUpperInit();
#endif // NO_WRAPPER

	//Инициализация массива клавиш.
	InitKeysArray();
	WaitForInputIdle(GetCurrentProcess(),0);

	if (!Opt.LoadPlug.MainPluginDir) //если есть ключ /p то он отменяет /co
		Opt.LoadPlug.PluginsCacheOnly=false;

	if (Opt.LoadPlug.PluginsCacheOnly)
	{
		Opt.LoadPlug.strCustomPluginsPath.Clear();
		Opt.LoadPlug.MainPluginDir=false;
		Opt.LoadPlug.PluginsPersonal=false;
	}

	InitConsole();

	if (!Lang.Init(g_strFarPath, MNewFileName))
	{
		ControlObject::ShowCopyright(1);
		LPCWSTR LngMsg;
		switch(Lang.GetLastError())
		{
		case LERROR_BAD_FILE:
			LngMsg = L"\nError: language data is incorrect or damaged.\n\nPress any key to exit...";
			break;
		case LERROR_FILE_NOT_FOUND:
			LngMsg = L"\nError: cannot find language data.\n\nPress any key to exit...";
			break;
		default:
			LngMsg = L"\nError: cannot load language data.\n\nPress any key to exit...";
			break;
		}
		Console.Write(LngMsg);
		Console.Commit();
		Console.FlushInputBuffer();
		WaitKey(); // А стоит ли ожидать клавишу??? Стоит
		return 1;
	}

	SetEnvironmentVariable(L"FARLANG",Opt.strLanguage);

#ifdef FAR_LUA
#else
	initMacroVarTable(1);
#endif

	ErrorMode=SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX|(Opt.ExceptRules?SEM_NOGPFAULTERRORBOX:0)|(GeneralCfg->GetValue(L"System.Exception", L"IgnoreDataAlignmentFaults", 0)?SEM_NOALIGNMENTFAULTEXCEPT:0);
	SetErrorMode(ErrorMode);
	SetUnhandledExceptionFilter(FarUnhandledExceptionFilter);

	int InitDriveMenuHotkeys = TRUE;
	GeneralCfg->GetValue(L"Interface", L"InitDriveMenuHotkeys", &InitDriveMenuHotkeys, InitDriveMenuHotkeys);
	if(InitDriveMenuHotkeys)
	{
		PlHotkeyCfg->SetHotkey(L"1E26A927-5135-48C6-88B2-845FB8945484", L"61026851-2643-4C67-BF80-D3C77A3AE830", PluginsHotkeysConfig::DRIVE_MENU, L"0"); // ProcList
		PlHotkeyCfg->SetHotkey(L"B77C964B-E31E-4D4C-8FE5-D6B0C6853E7C", L"F98C70B3-A1AE-4896-9388-C5C8E05013B7", PluginsHotkeysConfig::DRIVE_MENU, L"1"); // TmpPanel
		PlHotkeyCfg->SetHotkey(L"Plugins/FTP/FarFtp.dll"              , L"00000000-0000-0000-0000-000000000000", PluginsHotkeysConfig::DRIVE_MENU, L"2"); // FTP
		PlHotkeyCfg->SetHotkey(L"773B5051-7C5F-4920-A201-68051C4176A4", L"24B6DD41-DF12-470A-A47C-8675ED8D2ED4", PluginsHotkeysConfig::DRIVE_MENU, L"3"); // Network
		GeneralCfg->SetValue(L"Interface",L"InitDriveMenuHotkeys", 0ull);
	}

	int Result=MainProcessSEH(strEditName,strViewName,DestNames[0],DestNames[1],StartLine,StartChar);

	EmptyInternalClipboard();
#ifdef FAR_LUA
#else
	doneMacroVarTable(1);
#endif

	ReleaseDb();

	_OT(SysLog(L"[[[[[Exit of FAR]]]]]]]]]"));
	CloseHandle(MainThreadHandle);
	return Result;
}

#ifdef __GNUC__
int _cdecl main()
{
	int nArgs;
	LPWSTR* wstrCmdLineArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	int Result=wmain(nArgs, wstrCmdLineArgs);
	LocalFree(wstrCmdLineArgs);
	return Result;
}
#endif
