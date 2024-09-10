// validator: no-self-include
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

// BUGBUG
#include "platform.headers.hpp"

// Internal:
#include "keys.hpp"
#include "farcolor.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "imports.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "elevation.hpp"
#include "cmdline.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "colormix.hpp"
#include "treelist.hpp"
#include "plugins.hpp"
#include "notification.hpp"
#include "exception_handler.hpp"
#include "exception_handler_test.hpp"
#include "constitle.hpp"
#include "cvtname.hpp"
#include "drivemix.hpp"
#include "new_handler.hpp"
#include "global.hpp"
#include "locale.hpp"
#include "farversion.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.debug.hpp"
#include "platform.env.hpp"
#include "platform.memory.hpp"
#include "platform.process.hpp"
#include "platform.security.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"

// External:

#ifdef ENABLE_TESTS
#include "testing.hpp"
#endif

//----------------------------------------------------------------------------

global *Global = nullptr;

static void show_help()
{
	static const auto HelpMsg =
		//------------------------------------------------------------------------------
		L"Usage: far [switches] [apath [ppath]]\n\n"
		L"where\n"
		L"  apath - path to a folder (or a file or an archive or command with prefix)\n"
		L"          for the active panel\n"
		L"  ppath - path to a folder (or a file or an archive or command with prefix)\n"
		L"          for the passive panel\n"
		L"The following switches may be used in the command line:\n"
		L" -?   This help.\n"
		L" -clearcache [profilepath [localprofilepath]]\n"
		L"      Clear plugins cache.\n"
		L" -co  Load plugins from the cache only.\n"
#ifdef DIRECT_RT
		L" -do  Direct output.\n"
#endif
		L" -e[<line>[:<pos>]] <filename>\n"
		L"      Edit the specified file.\n"
		L" -export <out.farconfig> [profilepath [localprofilepath]]\n"
		L"      Export settings.\n"
		L" -import <in.farconfig> [profilepath [localprofilepath]]\n"
		L"      Import settings.\n"
		L" -m   Do not load macros.\n"
		L" -ma  Do not execute auto run macros.\n"
		L" -p[<path>]\n"
		L"      Search for \"common\" plugins in the directory, specified by <path>.\n"
		L" -ro[-] Read-only or normal config mode (overrides the ini file).\n"
		L" -s <profilepath> [<localprofilepath>]\n"
		L"      Custom location for Far configuration files (overrides the ini file).\n"
		L" -set:<parameter>=<value>\n"
		L"      Override the configuration parameter, see far:config for details.\n"
		L" -t <path>\n"
		L"      Location of Far template configuration file (overrides the ini file).\n"
		L" -title[:<title>]\n"
		L"      If <title> string is provided, use it as the window title; otherwise\n"
		L"      inherit the console window's title. Macro \"%Default\" in the custom\n"
		L"      title string will be replaced with the standard context-dependent\n"
		L"      Far window's title.\n"
#ifndef NO_WRAPPER
		L" -u <username>\n"
		L"      Allows to have separate registry settings for different users.\n"
		L"      Affects only Far Manager 1.x plugins.\n"
#endif // NO_WRAPPER
		L" -v <filename>\n"
		L"      View the specified file. If <filename> is -, data is read from the stdin.\n"
		L" -w[-] Show the interface within the console window instead of the console\n"
		L"      buffer or vise versa.\n"
		L" -x   Disable exception handling.\n"
		L""sv;

	std::wcout << HelpMsg << std::flush;
}

static int MainProcess(
	const string_view EditName,
	const string_view ViewName,
	const string_view DestName1,
	const string_view DestName2,
	int StartLine,
	int StartChar
)
{
		string ename(EditName),vname(ViewName), apanel(DestName1),ppanel(DestName2);
		if (ConfigProvider().ShowProblems())
		{
			ename.clear();
			vname.clear();
			StartLine = StartChar = -1;
			apanel = Global->Opt->ProfilePath;
			ppanel = Global->Opt->LocalProfilePath;
		}

		if (!ename.empty() || !vname.empty())
		{
			Global->OnlyEditorViewerUsed = true;

			Global->CtrlObject->CreateDummyFilePanels();
			Global->WindowManager->PluginCommit();

			Global->CtrlObject->Plugins->LoadPlugins();
			Global->CtrlObject->Macro.LoadMacros(true, true);

			if (!ename.empty())
			{
				const auto ShellEditor = FileEditor::create(ename, CP_DEFAULT, FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6, StartLine, StartChar);

				if (!ShellEditor->GetExitCode())  // ????????????
				{
					Global->WindowManager->ExitMainLoop(0);
				}
			}
			// TODO: Этот else убрать только после разборок с возможностью задавать несколько /e и /v в ком.строке
			else if (!vname.empty())
			{
				const auto ShellViewer = FileViewer::create(vname, true);

				if (!ShellViewer->GetExitCode())
				{
					Global->WindowManager->ExitMainLoop(0);
				}
			}

			Global->WindowManager->EnterMainLoop();
		}
		else
		{
			// воспользуемся тем, что ControlObject::Init() создает панели
			// юзая Global->Opt->*

			const auto
				IsFileA = !apanel.empty() && os::fs::is_file(apanel),
				IsFileP = !ppanel.empty() && os::fs::is_file(ppanel);

			const auto SetupPanel = [&](bool active)
			{
				string strPath = active? apanel : ppanel;
				if (active? IsFileA : IsFileP)
				{
					CutToParent(strPath);
				}

				bool Root = false;
				const auto Type = ParsePath(strPath, nullptr, &Root);
				if(Root && (Type == root_type::drive_letter || Type == root_type::win32nt_drive_letter || Type == root_type::volume))
				{
					AddEndSlash(strPath);
				}

				auto& CurrentPanelOptions = (Global->Opt->LeftFocus == active)? Global->Opt->LeftPanel : Global->Opt->RightPanel;
				CurrentPanelOptions.m_Type = static_cast<int>(panel_type::FILE_PANEL);  // сменим моду панели
				CurrentPanelOptions.Visible = true;     // и включим ее
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
			Global->CtrlObject->Init();

			// а теперь "провалимся" в каталог или хост-файл (если получится ;-)
			if (!apanel.empty())  // активная панель
			{
				const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
				const auto AnotherPanel = Global->CtrlObject->Cp()->PassivePanel();

				if (!ppanel.empty())  // пассивная панель
				{
					FarChDir(AnotherPanel->GetCurDir());

					if (GetPluginPrefixPath(ppanel))
					{
						AnotherPanel->Parent()->SetActivePanel(AnotherPanel);

						execute_info Info;
						Info.DisplayCommand = ppanel;
						Info.Command = ppanel;

						Global->CtrlObject->CmdLine()->ExecString(Info);
						ActivePanel->Parent()->SetActivePanel(ActivePanel);
					}
					else if (IsFileP)
					{
						const auto strPath = PointToName(ppanel);

						if (!strPath.empty())
						{
							if (AnotherPanel->GoToFile(strPath))
								AnotherPanel->ProcessKey(Manager::Key(KEY_CTRLPGDN));
						}
					}
				}

				FarChDir(ActivePanel->GetCurDir());

				if (GetPluginPrefixPath(apanel))
				{
					execute_info Info;
					Info.DisplayCommand = apanel;
					Info.Command = apanel;

					Global->CtrlObject->CmdLine()->ExecString(Info);
				}
				else if (IsFileA)
				{
					const auto strPath = PointToName(apanel);

					if (!strPath.empty())
					{
						if (ActivePanel->GoToFile(strPath))
							ActivePanel->ProcessKey(Manager::Key(KEY_CTRLPGDN));
					}
				}

				// !!! ВНИМАНИЕ !!!
				// Сначала редравим пассивную панель, а потом активную!
				AnotherPanel->Redraw();
				ActivePanel->Redraw();
			}

			Global->WindowManager->EnterMainLoop();
		}

		TreeList::FlushCache();

		Global->ScrBuf->ResetLockCount();
		Global->ScrBuf->Flush();

		return Global->FarExitCode;
}

static auto full_path_expanded(string_view const Str)
{
	return ConvertNameToFull(unquote(os::env::expand(Str)));
}

static void InitTemplateProfile(string &strTemplatePath)
{
	if (strTemplatePath.empty())
	{
		strTemplatePath = GetFarIniString(L"General"sv, L"TemplateProfile"sv, path::join(L"%FARHOME%"sv, L"Default.farconfig"sv));
	}

	if (!strTemplatePath.empty())
	{
		strTemplatePath = full_path_expanded(strTemplatePath);
		DeleteEndSlash(strTemplatePath);

		if (os::fs::is_directory(strTemplatePath))
			path::append(strTemplatePath, L"Default.farconfig"sv);

		Global->Opt->TemplateProfilePath = strTemplatePath;
	}
}

static void InitProfile(string &strProfilePath, string &strLocalProfilePath)
{
	if (Global->Opt->ReadOnlyConfig < 0) // do not override 'far /ro', 'far /ro-'
		Global->Opt->ReadOnlyConfig = GetFarIniInt(L"General"sv, L"ReadOnlyConfig"sv, 0);

	if (!strProfilePath.empty())
	{
		strProfilePath = full_path_expanded(strProfilePath);
	}
	if (!strLocalProfilePath.empty())
	{
		strLocalProfilePath = full_path_expanded(strLocalProfilePath);
	}

	if (strProfilePath.empty())
	{
		if (const auto UseSystemProfiles = GetFarIniInt(L"General"sv, L"UseSystemProfiles"sv, 1))
		{
			const auto GetShellProfilePath = [](int Idl)
			{
				wchar_t Buffer[MAX_PATH];
				SHGetFolderPath(nullptr, Idl | (Global->Opt->ReadOnlyConfig? 0 : CSIDL_FLAG_CREATE), nullptr, SHGFP_TYPE_CURRENT, Buffer);
				return path::join(Buffer, L"Far Manager"sv, L"Profile"sv);
			};

			// roaming data default path: %APPDATA%\Far Manager\Profile
			Global->Opt->ProfilePath = GetShellProfilePath(CSIDL_APPDATA);

			Global->Opt->LocalProfilePath = UseSystemProfiles == 2?
				Global->Opt->ProfilePath :
				// local data default path: %LOCALAPPDATA%\Far Manager\Profile
				GetShellProfilePath(CSIDL_LOCAL_APPDATA);
		}
		else
		{
			const auto strUserProfileDir = GetFarIniString(L"General"sv, L"UserProfileDir"sv, path::join(L"%FARHOME%"sv, L"Profile"sv));
			const auto strUserLocalProfileDir = GetFarIniString(L"General"sv, L"UserLocalProfileDir"sv, strUserProfileDir);

			Global->Opt->ProfilePath = full_path_expanded(strUserProfileDir);
			Global->Opt->LocalProfilePath = full_path_expanded(strUserLocalProfileDir);
		}
	}
	else
	{
		Global->Opt->ProfilePath = strProfilePath;
		Global->Opt->LocalProfilePath = !strLocalProfilePath.empty()? strLocalProfilePath : strProfilePath;
	}

	Global->Opt->LoadPlug.strPersonalPluginsPath = path::join(Global->Opt->ProfilePath, L"Plugins"sv);

	os::env::set(L"FARPROFILE"sv, Global->Opt->ProfilePath);
	os::env::set(L"FARLOCALPROFILE"sv, Global->Opt->LocalProfilePath);

	if (!Global->Opt->ReadOnlyConfig)
	{
		CreatePath(path::join(Global->Opt->ProfilePath, L"PluginsData"sv), false);

		const auto SingleProfile = equal_icase(Global->Opt->ProfilePath, Global->Opt->LocalProfilePath);

		if (!SingleProfile)
			CreatePath(path::join(Global->Opt->LocalProfilePath, L"PluginsData"sv), false);

		const auto RandomName = uuid::str(os::uuid::generate());

		if (!os::fs::can_create_file(path::join(Global->Opt->ProfilePath, RandomName)) ||
			(!SingleProfile && !os::fs::can_create_file(path::join(Global->Opt->LocalProfilePath, RandomName))))
		{
			Global->Opt->ReadOnlyConfig = true;
		}
	}
}

static bool is_arg(string_view const Str)
{
	return !Str.empty() && any_of(Str.front(), L'-', L'/');
}

static void ShowVersion(bool const Direct)
{
	bool EnoughSpace{};

	if (!Direct)
	{
		// Version, copyright, empty line, command line, keybar
		if (const auto SpaceNeeded = 5; NumberOfEmptyLines(SpaceNeeded) == SpaceNeeded)
		{
			EnoughSpace = true;
			console.SetCursorPosition({ 0, ScrY - (SpaceNeeded - 1) });
		}
	}

	std::wcout <<
		build::version_string() << L'\n' <<
		build::copyright() << L"\n\n"sv.substr(Direct || EnoughSpace? 1 : 0) <<
		std::endl;
}

static std::optional<int> ProcessServiceModes(std::span<const wchar_t* const> const Args)
{
	const auto isArg = [&](string_view const Name)
	{
		return is_arg(Args[0]) && equal_icase(Args[0] + 1, Name);
	};

	if (Args.size() == 4 && IsElevationArgument(Args[0])) // /service:elevation {UUID} PID UsePrivileges
	{
		return ElevationMain(Args[1], from_string<DWORD>(Args[2]), *Args[3] == L'1');
	}

	if (in_closed_range(2u, Args.size(), 5u) && (isArg(L"export"sv) || isArg(L"import"sv)))
	{
		const auto Export = isArg(L"export"sv);
		string strProfilePath(Args.size() > 2? Args[2] : L""sv), strLocalProfilePath(Args.size() > 3 ? Args[3] : L""), strTemplatePath(Args.size() > 4 ? Args[4] : L"");
		InitTemplateProfile(strTemplatePath);
		InitProfile(strProfilePath, strLocalProfilePath);
		Global->m_ConfigProvider = std::make_unique<config_provider>(Export? config_provider::mode::m_export : config_provider::mode::m_import);
		ConfigProvider().ServiceMode(Args[1]);
		return EXIT_SUCCESS;
	}

	if (in_closed_range(1u, Args.size(), 3u) && isArg(L"clearcache"sv))
	{
		string strProfilePath(Args.size() > 1? Args[1] : L""sv);
		string strLocalProfilePath(Args.size() > 2? Args[2] : L""sv);
		InitProfile(strProfilePath, strLocalProfilePath);
		(void)config_provider{config_provider::clear_cache{}};
		return EXIT_SUCCESS;
	}

	if (Args.size() == 2 && logging::is_log_argument(Args[0]))
	{
		return logging::main(Args[1]);
	}

	if (Args.size() == 1 && (isArg(L"?") || isArg(L"h")))
	{
		ShowVersion(true);
		show_help();
		return EXIT_SUCCESS;
	}

	return {};
}

[[noreturn]]
static void handle_exception(function_ref<bool()> const Handler)
{
	if (Handler())
		os::process::terminate_by_user();

	throw;
}

#ifdef _M_IX86
std::pair<string_view, DWORD> get_hook_wow64_error();

static void log_hook_wow64_status()
{
	const auto [Msg, Error] = get_hook_wow64_error();
	LOG(
		Error == ERROR_SUCCESS? logging::level::debug : logging::level::warning,
		L"hook_wow64: {}: {}"sv,
		Msg,
		os::format_error(Error)
	);

	if (Error == ERROR_INVALID_DATA)
	{
		if (const auto NtDll = GetModuleHandle(L"ntdll"))
		{
			if (const auto LdrLoadDll = GetProcAddress(NtDll, "LdrLoadDll"))
			{
				const auto FunctionData = std::bit_cast<std::byte const*>(LdrLoadDll);
				LOGWARNING(L"LdrLoadDll: {}"sv, BlobToHexString({ FunctionData, 32 }));
			}
		}
	}
}
#endif

struct args_context
{
	int* E_Line;
	int* E_Pos;
	string* E_Param;
	string* V_Param;
	string* U_Param;
	string* S_Param1;
	string* S_Param2;
	string* T_Param;
	std::optional<string>* Title_Param;
	int* MacroOptions;
	Options::overrides* Overrides;
	bool* PluginsCacheOnly;
	bool* PluginsPersonal;
	bool* MainPluginDir;
	string* CustomPluginsPath;
	int* WindowMode;
	int* ReadOnlyConfig;
};

[[noreturn]]
static void invalid_argument(string_view const Argument, string_view const Str)
{
	throw far_known_exception(far::format(L"Error processing \"{}\": {}"sv, Argument, Str));
}

namespace args
{
	static const auto
		unknown_argument  = L"unknown argument"sv,
		invalid_format    = L"invalid format"sv,
		number_expected   = L"a number is expected"sv,
		parameter_expected = L"a parameter is expected"sv;
}

static void parse_argument(std::span<const wchar_t* const>::iterator& Iterator, std::span<const wchar_t* const>::iterator End, args_context const& Context)
{
	if (Iterator == End)
		return;

	string_view const FullArgument = *Iterator++, Argument = FullArgument.substr(1);

	switch (upper(Argument.front()))
	{
	case L'E':
		{
			if (Argument.size() != 1)
			{
				size_t LineEnd;
				if (!from_string(Argument.substr(1), *Context.E_Line, &LineEnd))
					invalid_argument(FullArgument, args::number_expected);

				if (LineEnd + 1 != Argument.size())
				{
					if (Argument[LineEnd + 1] != L':')
						invalid_argument(FullArgument, args::invalid_format);

					size_t PosEnd;
					if (!from_string(Argument.substr(LineEnd + 2), *Context.E_Pos, &PosEnd))
						invalid_argument(FullArgument, args::number_expected);

					if (PosEnd + LineEnd + 2 != Argument.size())
						invalid_argument(FullArgument, args::invalid_format);
				}
			}

			if (Iterator == End)
				invalid_argument(FullArgument, args::parameter_expected);

			*Context.E_Param = *Iterator++;
			return;
		}
	case 'V':
		{
			if (Argument.size() != 1)
				invalid_argument(FullArgument, args::unknown_argument);

			if (Iterator == End)
				invalid_argument(FullArgument, args::parameter_expected);

			*Context.V_Param = *Iterator++;
			return;
		}

	case L'M':
		{
			if (Argument.size() == 1)
			{
				*Context.MacroOptions |= MDOL_ALL;
				return;
			}

			if (equal_icase(Argument.substr(1), L"A"))
			{
				*Context.MacroOptions |= MDOL_AUTOSTART;
				return;
			}

			invalid_argument(FullArgument, args::unknown_argument);
		}

#ifndef NO_WRAPPER
	case L'U':
		{
			if (Argument.size() != 1)
				invalid_argument(FullArgument, args::unknown_argument);

			if (Iterator == End)
				invalid_argument(FullArgument, args::parameter_expected);

			*Context.U_Param = *Iterator++;
			return;
		}
#endif // NO_WRAPPER

	case L'S':
		{
			if (const auto SetParam = L"set:"sv; starts_with_icase(Argument, SetParam))
			{
				const auto Tail = Argument.substr(SetParam.size());
				const auto& [Name, Value] = split(Tail);
				if (Name.size() == Tail.size())
					invalid_argument(FullArgument, args::invalid_format);

				Context.Overrides->emplace(Name, Value);
				return;
			}

			if (equal_icase(Argument, L"service"sv))
			{
				// Processed earlier
				return;
			}

			if (Argument.size() != 1)
				invalid_argument(FullArgument, args::unknown_argument);

			if (Iterator == End)
				invalid_argument(FullArgument, args::parameter_expected);

			*Context.S_Param1 = *Iterator++;

			if (Iterator != End && !is_arg(*Iterator))
				*Context.S_Param2 = *Iterator++;

			return;
		}

	case L'T':
		{
			if (Argument.size() == 1)
			{
				if (Iterator == End)
					invalid_argument(FullArgument, args::parameter_expected);

				*Context.T_Param = *Iterator++;
				return;
			}

			if (const auto Title = L"title"sv; starts_with_icase(Argument, Title))
			{
				if (Argument.size() == Title.size())
				{
					*Context.Title_Param = L""sv;
					return;
				}

				if (Argument[Title.size()] != L':')
					invalid_argument(FullArgument, args::unknown_argument);

				*Context.Title_Param = Argument.substr(Title.size() + 1);
				return;
			}

			invalid_argument(FullArgument, args::unknown_argument);
		}

	case L'P':
		{
			*Context.PluginsPersonal = false;
			*Context.MainPluginDir = false;

			// we can't expand it here - some environment variables might not be available yet
			*Context.CustomPluginsPath = Argument.substr(1);
			return;
		}

	case L'C':
		{
			if (!equal_icase(Argument.substr(1), L"O"sv))
				invalid_argument(FullArgument, args::unknown_argument);

			*Context.PluginsCacheOnly = true;
			*Context.PluginsPersonal = false;
			return;
		}

#ifdef DIRECT_RT
	case L'D':
		{
			if (!equal_icase(Argument.substr(1), L"O"sv))
				invalid_argument(FullArgument, args::unknown_argument);

			Global->DirectRT = true;
			return;
		}
#endif
	case L'W':
		{
			if (Argument.size() == 1)
			{
				*Context.WindowMode = true;
				return;
			}

			if (Argument.substr(1) != L"-"sv)
				invalid_argument(FullArgument, args::unknown_argument);

			*Context.WindowMode = false;
			return;
		}

	case L'R':
		{
			if (Argument.size() == 1)
				invalid_argument(FullArgument, args::unknown_argument);

			if (upper(Argument[1]) != L'O')
				invalid_argument(FullArgument, args::unknown_argument);

			if (Argument.size() == 2)
			{
				*Context.ReadOnlyConfig = true;
				return;
			}

			if (Argument.size() == 3 && Argument[2] == L'-')
			{
				*Context.ReadOnlyConfig = false;
				return;
			}

			invalid_argument(FullArgument, args::unknown_argument);
		}

	case L'X':
		{
			if (Argument.size() == 1)
				// Processed earlier
				return;

			invalid_argument(FullArgument, args::unknown_argument);
		}

	default:
		invalid_argument(FullArgument, args::unknown_argument);
	}
}

static void parse_command_line(std::span<const wchar_t* const> const Args, std::span<string> const SimpleArgs, args_context const& Context)
{
	size_t SimpleArgsCount{};

	for (auto Iter = Args.begin(); Iter != Args.end();)
	{
		if (is_arg(*Iter))
		{
			parse_argument(Iter, Args.end(), Context);
			continue;
		}

		if (SimpleArgsCount == 2)
			invalid_argument(*Iter, args::unknown_argument);

		if (GetPluginPrefixPath(*Iter))
		{
			SimpleArgs[SimpleArgsCount++] = *Iter++;
			continue;
		}

		SimpleArgs[SimpleArgsCount++] = full_path_expanded(*Iter++);
	}
}

static int mainImpl(std::span<const wchar_t* const> const Args)
{
	setlocale(LC_ALL, "");

	if (FarColor InitAttributes; console.GetTextAttributes(InitAttributes))
		colors::store_default_color(InitAttributes);

	SCOPE_EXIT{ console.SetTextAttributes(colors::default_color()); };

	SCOPED_ACTION(global);

	std::optional<elevation::suppress> NoElevationDuringBoot(std::in_place);

	RegisterTestExceptionsHook();

	os::memory::enable_low_fragmentation_heap();

#ifdef _M_IX86
	log_hook_wow64_status();
#endif

	if(!console.IsFullscreenSupported() && imports.SetConsoleKeyShortcuts)
	{
		const BYTE ReserveAltEnter = 0x8;
		imports.SetConsoleKeyShortcuts(TRUE, ReserveAltEnter, nullptr, 0);
	}

	os::fs::InitCurrentDirectory();

	PrepareDiskPath(Global->g_strFarModuleName);

	Global->g_strFarINI = concat(Global->g_strFarModuleName, L".ini"sv);
	Global->g_strFarPath = Global->g_strFarModuleName;
	CutToSlash(Global->g_strFarPath,true);
	os::env::set(L"FARHOME"sv, Global->g_strFarPath);
	AddEndSlash(Global->g_strFarPath);

	if (const auto FarAdminMode = L"FARADMINMODE"sv; os::security::is_admin())
		os::env::set(FarAdminMode, L"1"sv);
	else
		os::env::del(FarAdminMode);

	if (const auto Result = ProcessServiceModes(Args))
		return *Result;

	SCOPED_ACTION(listener)(update_environment, [] { if (Global->Opt->UpdateEnvironment) ReloadEnvironment(); });
	SCOPED_ACTION(listener)(update_intl, [] { locale.invalidate(); });
	SCOPED_ACTION(listener)(update_devices, &UpdateSavedDrives);

	string strEditName;
	string strViewName;
	string DestNames[2];
	int StartLine=-1,StartChar=-1;

	string strProfilePath, strLocalProfilePath, strTemplatePath;

	std::optional<string> CustomTitle;

	Options::overrides Overrides;


	args_context Context;
	Context.E_Line = &StartLine;
	Context.E_Pos = &StartChar;
	Context.E_Param = &strEditName;
	Context.V_Param = &strViewName;
	Context.U_Param = &Global->strRegUser;
	Context.S_Param1 = &strProfilePath;
	Context.S_Param2 = &strLocalProfilePath;
	Context.T_Param = &strTemplatePath;
	Context.Title_Param = &CustomTitle;
	Context.MacroOptions = &Global->Opt->Macro.DisableMacro;
	Context.Overrides = &Overrides;
	Context.PluginsCacheOnly = &Global->Opt->LoadPlug.PluginsCacheOnly;
	Context.PluginsPersonal = &Global->Opt->LoadPlug.PluginsPersonal;
	Context.MainPluginDir = &Global->Opt->LoadPlug.MainPluginDir;
	Context.CustomPluginsPath = &Global->Opt->LoadPlug.strCustomPluginsPath;
	Context.WindowMode = &Global->Opt->WindowMode;
	Context.ReadOnlyConfig = &Global->Opt->ReadOnlyConfig;

	parse_command_line(Args, DestNames, Context);

	InitTemplateProfile(strTemplatePath);
	InitProfile(strProfilePath, strLocalProfilePath);
	Global->m_ConfigProvider = std::make_unique<config_provider>();

	Global->Opt->Load(std::move(Overrides));

	//Инициализация массива клавиш.
	InitKeysArray();

	if (!Global->Opt->LoadPlug.MainPluginDir) //если есть ключ /p то он отменяет /co
		Global->Opt->LoadPlug.PluginsCacheOnly=false;

	if (Global->Opt->LoadPlug.PluginsCacheOnly)
	{
		Global->Opt->LoadPlug.strCustomPluginsPath.clear();
		Global->Opt->LoadPlug.MainPluginDir=false;
		Global->Opt->LoadPlug.PluginsPersonal=false;
	}

	InitConsole();
	Global->ScrBuf->FillBuf();
	ShowVersion(false);
	Global->ScrBuf->FillBuf();

	SCOPE_EXIT
	{
		CloseConsole();
	};

	if (CustomTitle.has_value())
		ConsoleTitle::SetUserTitle(CustomTitle->empty() ? Global->strInitTitle : *CustomTitle);

	far_language::instance().load(Global->g_strFarPath, Global->Opt->strLanguage, static_cast<int>(lng::MNewFileName + 1));

	os::env::set(L"FARLANG"sv, Global->Opt->strLanguage);

	if (!Global->Opt->LoadPlug.strCustomPluginsPath.empty())
		Global->Opt->LoadPlug.strCustomPluginsPath = full_path_expanded(Global->Opt->LoadPlug.strCustomPluginsPath);

	ControlObject CtrlObj;
	Global->CtrlObject = &CtrlObj;

	SCOPE_EXIT
	{
		// close() can throw, but we need to clear it anyway
		SCOPE_EXIT { Global->CtrlObject = {}; };

		CtrlObj.close();
	};

	NoElevationDuringBoot.reset();

	return cpp_try(
	[&]
	{
		return MainProcess(strEditName, strViewName, DestNames[0], DestNames[1], StartLine, StartChar);
	},
	[&](source_location const& Location) -> int
	{
		handle_exception([&]{ return handle_unknown_exception({}, Location); });
	},
	[&](std::exception const& e, source_location const& Location) -> int
	{
		handle_exception([&]{ return handle_std_exception(e, {}, Location); });
	});
}

static void configure_exception_handling(std::span<wchar_t const* const> const Args)
{
	os::debug::crt_report_to_ui();

	for (const auto& i: Args)
	{
		if (!is_arg(i))
			continue;

		if (upper(i[1]) == L'X' && !i[2])
		{
			disable_exception_handling();
			continue;
		}

		if (equal_icase(i + 1, L"service"sv))
		{
			os::debug::crt_report_to_stderr();
			continue;
		}
	}
}

[[noreturn]]
static void handle_exception_final(function_ref<bool()> const Handler)
{
	if (Handler())
		os::process::terminate_by_user();

	restore_system_exception_handler();
	throw;
}

#ifdef _DEBUG
static void premain();
#endif

static int wmain_seh()
{
	os::set_error_mode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX);

	// wmain is a non-standard extension and not available in gcc.
	int Argc = 0;
	os::memory::local::ptr<wchar_t const* const> const Argv(CommandLineToArgvW(GetCommandLine(), &Argc));
	std::span const AllArgs(Argv.get(), Argc), Args(AllArgs.subspan(1));

	configure_exception_handling(Args);

	SCOPED_ACTION(unhandled_exception_filter);
	SCOPED_ACTION(vectored_exception_handler);
	SCOPED_ACTION(signal_handler);
	SCOPED_ACTION(invalid_parameter_handler);
	SCOPED_ACTION(new_handler);

#ifdef ENABLE_TESTS
	if (const auto Result = testing_main(AllArgs))
	{
		return *Result;
	}
#endif

#ifdef _DEBUG
	premain();
#endif

#ifdef __SANITIZE_ADDRESS__
	os::env::set(L"ASAN_VCASAN_DEBUGGING"sv, L"1"sv);
#endif

	return cpp_try(
	[&]
	{
		try
		{
			return mainImpl(Args);
		}
		catch (far_known_exception const& e)
		{
			std::wcout << build::version_string() << L'\n' << std::endl;
			std::wcerr << e.message() << std::endl;
			return EXIT_FAILURE;
		}
	},
	[&](source_location const& Location) -> int
	{
		handle_exception_final([&]{ return handle_unknown_exception({}, Location); });
	},
	[&](std::exception const& e, source_location const& Location) -> int
	{
		handle_exception_final([&]{ return handle_std_exception(e, {}, Location); });
	});
}

int main()
{
	return seh_try_with_ui(
	[]
	{
		os::debug::set_thread_name(L"Main Thread");
		return wmain_seh();
	},
	[](DWORD const ExceptionCode) -> int
	{
		os::process::terminate_by_user(ExceptionCode);
	});
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("Args")
{
	int E_Line;
	int E_Pos;
	string E_Param;
	string V_Param;
	string U_Param;
	string S_Param1;
	string S_Param2;
	string T_Param;
	std::optional<string> Title_Param;
	int MacroOptions{};
	Options::overrides Overrides;
	bool PluginsCacheOnly{};
	bool PluginsPersonal{};
	bool MainPluginDir{};
	string CustomPluginsPath;
	int WindowMode{};
	int ReadOnlyConfig{};

	args_context Context;

	Context.E_Line = &E_Line;
	Context.E_Pos = &E_Pos;
	Context.E_Param = &E_Param;
	Context.V_Param = &V_Param;
	Context.U_Param = &U_Param;
	Context.S_Param1 = &S_Param1;
	Context.S_Param2 = &S_Param2;
	Context.T_Param = &T_Param;
	Context.Title_Param = &Title_Param;
	Context.MacroOptions = &MacroOptions;
	Context.Overrides = &Overrides;
	Context.PluginsCacheOnly = &PluginsCacheOnly;
	Context.PluginsPersonal = &PluginsPersonal;
	Context.MainPluginDir = &MainPluginDir;
	Context.CustomPluginsPath = &CustomPluginsPath;
	Context.WindowMode = &WindowMode;
	Context.ReadOnlyConfig = &ReadOnlyConfig;

	static const struct
	{
		std::initializer_list<const wchar_t*> Args;
		std::variant<std::function<bool()>, string_view> Validator;
	}
	Tests[]
	{
		{ {} },
		{ { L"-e" },                     args::parameter_expected },
		{ { L"-e", L"file" },            [&]{ return E_Param == L"file"sv; } },
		{ { L"-ew" },                    args::number_expected },
		{ { L"-e1" },                    args::parameter_expected },
		{ { L"-e1", L"foo" },            [&]{ return E_Line == 1 && E_Param == L"foo"sv; } },
		{ { L"-e1:" },                   args::number_expected },
		{ { L"-e1:b" },                  args::number_expected },
		{ { L"-e2:3" },                  args::parameter_expected },
		{ { L"-e2:3", L"bar" },          [&]{ return E_Line == 2 && E_Pos == 3 && E_Param == L"bar"sv; } },
		{ { L"-v" },                     args::parameter_expected },
		{ { L"-vw" },                    args::unknown_argument },
		{ { L"-v", L"file" },            [&]{ return V_Param == L"file"sv; } },
		{ { L"-m" },                     [&]{ return flags::check_one(MacroOptions, MDOL_ALL); } },
		{ { L"-mb" },                    args::unknown_argument },
		{ { L"-ma" },                    [&]{ return flags::check_one(MacroOptions, MDOL_AUTOSTART); } },
#ifndef NO_WRAPPER
		{ { L"-u" },                     args::parameter_expected },
		{ { L"-ua" },                    args::unknown_argument },
		{ { L"-u", L"user" },            [&]{ return U_Param == L"user"sv; } },
#endif // NO_WRAPPER
		{ { L"-set" },                   args::unknown_argument },
		{ { L"-set:" },                  args::invalid_format },
		{ { L"-set:beep" },              args::invalid_format },
		{ { L"-set:beep=" },             [&]{ return Overrides[L"beep"s].empty(); } },
		{ { L"-set:foo=bar" },           [&]{ return Overrides[L"foo"s] == L"bar"sv; } },
		{ { L"-service" } },
		{ { L"-service1" },              args::unknown_argument },
		{ { L"-s" },                     args::parameter_expected },
		{ { L"-s1" },                    args::unknown_argument },
		{ { L"-s", L"dir1" },            [&]{ return S_Param1 == L"dir1"sv; }},
		{ { L"-s", L"dir1", L"-dir2" },  [&]{ return S_Param1 == L"dir1"sv && S_Param2.empty(); } },
		{ { L"-s", L"dir1", L"dir2" },   [&]{ return S_Param1 == L"dir1"sv && S_Param2 == L"dir2"sv; } },
		{ { L"-t" },                     args::parameter_expected },
		{ { L"-t1" },                    args::unknown_argument },
		{ { L"-t", L"path" },            [&]{ return T_Param == L"path"sv; } },
		{ { L"-title" },                 [&]{ return Title_Param->empty(); } },
		{ { L"-title1" },                args::unknown_argument },
		{ { L"-title:" },                [&]{ return Title_Param->empty(); } },
		{ { L"-title:foo" },             [&]{ return Title_Param == L"foo"sv; } },
		{ { L"-p" },                     [&]{ return !PluginsPersonal && !MainPluginDir && CustomPluginsPath.empty(); } },
		{ { L"-pfoo" },                  [&]{ return !PluginsPersonal && !MainPluginDir && CustomPluginsPath == L"foo"sv; } },
		{ { L"-c" },                     args::unknown_argument },
		{ { L"-c1" },                    args::unknown_argument },
		{ { L"-co" },                    [&]{ return !PluginsPersonal && PluginsCacheOnly; } },
		{ { L"-co1" },                   args::unknown_argument },
		{ { L"-w" },                     [&]{ return WindowMode != false; } },
		{ { L"-w1" },                    args::unknown_argument },
		{ { L"-w-" },                    [&]{ return WindowMode == false; } },
		{ { L"-w-1" },                   args::unknown_argument },
		{ { L"-r" },                     args::unknown_argument },
		{ { L"-r1" },                    args::unknown_argument },
		{ { L"-ro" },                    [&]{ return ReadOnlyConfig != false; } },
		{ { L"-ro1" },                   args::unknown_argument },
		{ { L"-ro-" },                   [&]{ return ReadOnlyConfig == false; } },
		{ { L"-ro-1" },                  args::unknown_argument },
		{ { L"-x" } },
		{ { L"-x1" },                    args::unknown_argument },
		{ { L"-q" },                     args::unknown_argument },
	};

	for (const auto& i: Tests)
	{
		std::span const Args = i.Args;
		auto Iterator = Args.begin();

		std::visit(overload
		{
			[&](std::function<bool()> const& Validator)
			{
				REQUIRE_NOTHROW(parse_argument(Iterator, Args.end(), Context));
				if (Validator)
					REQUIRE(Validator());
			},
			[&](string_view const& Validator)
			{
				REQUIRE_THROWS_MATCHES(parse_argument(Iterator, Args.end(), Context), far_known_exception, generic_exception_matcher([Validator](std::any const& e)
				{
					return !Validator.empty() && contains(std::any_cast<far_known_exception const&>(e).message(), Validator);
				}));
			}
		}, i.Validator);
	}
}
#endif

#ifdef _DEBUG
static void premain()
{
}
#endif
