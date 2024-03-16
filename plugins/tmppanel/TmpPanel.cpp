/*
TMPPANEL.CPP

Temporary panel main plugin code

*/

#include "TmpPanel.hpp"
#include "plugin.hpp"
#include <shellapi.h>

#include "TmpLng.hpp"
#include "TmpCfg.hpp"
#include "TmpClass.hpp"
#include "version.hpp"

#include <algorithm.hpp>
#include <scope_exit.hpp>
#include <string_utils.hpp>

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

static bool get_console_title(string& Title)
{
	const auto ConsoleWindow = GetConsoleWindow();

	const size_t Length = GetWindowTextLength(ConsoleWindow);

	if (!Length)
		return GetLastError() == ERROR_SUCCESS;

	Title.resize(Length + 1);
	Title.resize(GetWindowText(ConsoleWindow, Title.data(), static_cast<int>(Title.size())));
	return true;
}

[[nodiscard]]
constexpr bool IsEol(wchar_t x) noexcept { return x == L'\r' || x == L'\n'; }

static std::vector<string> ReadFileLines(const HANDLE FileMapping, const DWORD FileSizeLow)
{
	const auto FileData = static_cast<char*>(MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, FileSizeLow));

	if (!FileData)
		return {};

	string Line;

	const auto Ptr = reinterpret_cast<wchar_t*>(FileData);

	auto cp = CP_OEMCP;
	DWORD Pos = 0;
	DWORD Size = FileSizeLow;

	if (Ptr[0] == SIGN_UNICODE)
	{
		Pos += 2;
		cp = CP_UNICODE;
	}
	else if (Ptr[0] == SIGN_REVERSEBOM)
	{
		Pos += 2;
		cp = CP_REVERSEBOM;
	}
	else if (Ptr[0] == SIGN_UTF8_LO && (Ptr[1] & 0xff) == SIGN_UTF8_HI)
	{
		Pos += 3;
		cp = CP_UTF8;
	}
	else
	{
		if (IsTextUTF8(FileData, Size))
		{
			cp = CP_UTF8;
		}
		else
		{
			int test = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK;
			IsTextUnicode(Ptr, Size, &test); // return value is ignored, it's ok.

			if (!(test & IS_TEXT_UNICODE_NOT_UNICODE_MASK) || (test & IS_TEXT_UNICODE_ODD_LENGTH)) // ignore odd
			{
				if (test & IS_TEXT_UNICODE_STATISTICS) // !!! допускаем возможность, что это Unicode
				{
					cp = CP_UNICODE;
				}
			}
		}
	}

	std::vector<string> Lines;

	while (Pos < Size)
	{
#if 0
		if (cp == CP_REVERSEBOM)
		{
			swab((char*)&FileData[Off], (char*)TMP.Ptr(), Len * sizeof(wchar_t));
			cp = CP_UNICODE;
		}
#endif
		if (cp == CP_UNICODE)
		{
			--Size;

			while (Pos < Size && IsEol(*reinterpret_cast<wchar_t*>(FileData + Pos)))
				Pos += sizeof(wchar_t);

			const auto Off = Pos;

			while (Pos < Size && !IsEol(*reinterpret_cast<wchar_t*>(FileData + Pos)))
				Pos += sizeof(wchar_t);

			if (Pos < Size)
				++Size;

			Line.assign(reinterpret_cast<const wchar_t*>(FileData + Off), (Pos - Off) / sizeof(wchar_t));
		}
		else
		{
			while (Pos < Size && IsEol(FileData[Pos]))
				Pos++;

			const auto Off = Pos;

			while (Pos < Size && !IsEol(FileData[Pos]))
				Pos++;

			const auto Required = MultiByteToWideChar(cp, 0, FileData + Off, Pos - Off, {}, 0);
			Line.resize(Required);
			Line.resize(MultiByteToWideChar(cp, 0, FileData + Off, Pos - Off, Line.data(), static_cast<DWORD>(Line.size())));
		}

		if (Line.empty())
			continue;

		Lines.emplace_back(std::move(Line));
	}

	UnmapViewOfFile(FileData);

	return Lines;
}

static std::vector<string> ReadFileList(const string_view Filename)
{
	const auto NtPath = FormNtPath(GetFullPath(Filename));
	const auto File = CreateFile(NtPath.c_str(), GENERIC_READ, FILE_SHARE_READ, {}, OPEN_EXISTING, 0, {});
	if (File == INVALID_HANDLE_VALUE)
		return {};
	SCOPE_EXIT{ CloseHandle(File); };

	const auto FileMapping = CreateFileMapping(File, {}, PAGE_READONLY, 0, 0, {});
	if (!FileMapping)
		return {};
	SCOPE_EXIT{ CloseHandle(FileMapping); };

	const auto FileSizeLow = GetFileSize(File, {});
	return ReadFileLines(FileMapping, FileSizeLow);
}

static void ShowMenuFromList(const string& Name)
{
	const auto Args = ReadFileList(Name);
	std::vector<FarMenuItem> fmi(Args.size());
	std::vector<string> MenuStrings, Commands;
	MenuStrings.reserve(Args.size());
	Commands.reserve(Args.size());

	for (size_t i = 0, size = Args.size(); i != size; ++i)
	{
		const auto Tmp = ExpandEnvStrs(Args[i]);
		const auto [Param, Cmd] = ParseParam(Tmp);

		fmi[i].Flags = Param == L"-"sv? MIF_SEPARATOR : 0;
		fmi[i].Text = MenuStrings.emplace_back(fmi[i].Flags & MIF_SEPARATOR? L""sv : Param.empty()? Tmp : Param).c_str();
		Commands.emplace_back(Cmd.empty()? Tmp : Cmd);
	}

	string Title = FSF.PointToName(Name.c_str());
	if (const auto DotPos = Title.rfind(L'.'); DotPos != Title.npos)
	{
		Title.resize(DotPos);
	}

	FarKey BreakKeys[]{ { VK_RETURN, SHIFT_PRESSED }, {} };
	intptr_t BreakCode;

	const auto ExitCode = PsInfo.Menu(&MainGuid, {}, -1, -1, 0, FMENU_WRAPMODE, Title.c_str(), {}, L"Contents", &BreakKeys[0], &BreakCode, fmi.data(), fmi.size());
	if (ExitCode < 0)
		return;

	PluginPanelItem FindData;
	string NameData;
	auto UseShellExecute = BreakCode != -1;

	if (!UseShellExecute)
	{
		if (GetFileInfoAndValidate(Commands[ExitCode], FindData, NameData, false))
		{
			if (FindData.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FarPanelDirectory DirInfo{ sizeof(DirInfo), Commands[ExitCode].c_str() };
				PsInfo.PanelControl(PANEL_ACTIVE, FCTL_SETPANELDIRECTORY, 0, &DirInfo);
			}
			else
			{
				UseShellExecute = true;
			}
		}
		else
		{
			PsInfo.PanelControl(PANEL_ACTIVE, FCTL_SETCMDLINE, 0, const_cast<wchar_t*>(Commands[ExitCode].c_str()));
		}
	}

	if (UseShellExecute)
		ShellExecute({}, L"open", Commands[ExitCode].c_str(), {}, {}, SW_SHOW);
}

static void ProcessList(TmpPanel& Panel, const string_view Name, const bool Replace)
{
	if (Replace)
		Panel.clear();

	const auto Args = ReadFileList(Name);
	const auto Screen = Panel.BeginPutFiles();

	for (const auto& i: Args)
		Panel.PutOneFile(i);

	Panel.CommitPutFiles(Screen, true);
}

static string run_command(const string& Command)
{
	string TempFilename(MAX_PATH, 0);

	for (;;)
	{
		const auto Size = FSF.MkTemp(TempFilename.data(), TempFilename.size(), {});
		if (!Size)
			return {};

		const auto CurrentSize = TempFilename.size();
		TempFilename.resize(Size - 1);
		if (Size - 1 <= CurrentSize)
			break;
	}

	SECURITY_ATTRIBUTES sa{ sizeof(sa), {}, TRUE };
	const auto FileHandle = CreateFile(TempFilename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, {});
	if (FileHandle == INVALID_HANDLE_VALUE)
		return {};

	SCOPE_EXIT{ CloseHandle(FileHandle); };

	STARTUPINFO si{ sizeof(si) };
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = FileHandle;
	si.hStdError = FileHandle;

	PROCESS_INFORMATION pi{};
	string WorkDir; //make empty string just in case

	if (const auto [TempDir, _] = ParseParam(Command); !TempDir.empty())
	{
		WorkDir = ExpandEnvStrs(TempDir);
	}
	else
	{
		WorkDir.resize(MAX_PATH);
		for (;;)
		{
			const size_t Size = FSF.GetCurrentDirectory(WorkDir.size(), WorkDir.data());
			if (!Size)
				return {};

			const auto CurrentSize = WorkDir.size();
			WorkDir.resize(Size - 1);
			if (Size - 1 <= CurrentSize)
				break;
		}
	}

	string ConsoleTitle;
	const auto NeedRestoreTitle = get_console_title(ConsoleTitle) && SetConsoleTitle(Command.c_str());
	SCOPE_EXIT{ if (NeedRestoreTitle) SetConsoleTitle(ConsoleTitle.c_str()); };

	const auto FullCmd = ExpandEnvStrs(L"%COMSPEC% /c "sv + Command);
	if (!CreateProcess({}, const_cast<wchar_t*>(FullCmd.c_str()), {}, {}, true, 0, {}, WorkDir.c_str(), &si, &pi))
		return {};

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TempFilename;
}

static std::unique_ptr<TmpPanel> OpenPanelFromOutput(const string& Command)
{
	const auto TempFilename = run_command(Command);
	if (TempFilename.empty())
		return {};

	SCOPE_EXIT{ DeleteFile(TempFilename.c_str()); };

	if (Opt.MenuForFilelist)
	{
		ShowMenuFromList(TempFilename);
		return {};
	}

	auto Plugin = std::make_unique<TmpPanel>();
	ProcessList(*Plugin, TempFilename, Opt.Replace);
	return Plugin;
}

void WINAPI GetGlobalInfoW(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;
	Info->Version = PLUGIN_VERSION;
	Info->Guid = MainGuid;
	Info->Title = PLUGIN_NAME;
	Info->Description = PLUGIN_DESC;
	Info->Author = PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* Info)
{
	PsInfo = *Info;
	FSF = *PsInfo.FSF;
	PsInfo.FSF = &FSF;

	GetOptions();

	Opt.LastSearchResultsPanel = 0;
	SharedData = std::make_unique<shared_data>().release();
}

HANDLE WINAPI OpenW(const OpenInfo* Info)
{
	GetOptions();

	if (Info->OpenFrom == OPEN_COMMANDLINE)
	{
		const std::pair<string_view, bool*> Params[]
		{
			{ L"safe"sv,    &Opt.SafeModePanel, },
			{ L"any"sv,     &Opt.AnyInPanel, },
			{ L"replace"sv, &Opt.Replace, },
			{ L"menu"sv,    &Opt.MenuForFilelist, },
			{ L"full"sv,    &Opt.FullScreenPanel, },
		};

		string_view CommandLine = reinterpret_cast<const OpenCommandLineInfo*>(Info->Data)->CommandLine;
		inplace::trim_left(CommandLine);

		while (CommandLine.size() > 1 && any_of(CommandLine.front(), L'+', L'-'))
		{
			const auto Mode = CommandLine.front();
			CommandLine.remove_prefix(1);
			const auto Cmd = CommandLine.substr(0, CommandLine.find_first_of(L" <"));
			CommandLine.remove_prefix(Cmd.size());

			for (const auto& [Name, Value]: Params)
			{
				if (Cmd == Name)
				{
					*Value = Mode == L'+';
					break;
				}
			}

			if (Mode == L'+' && Cmd.size() == 1 && in_closed_range(L'0', Cmd.front(), L'9'))
			{
				SharedData->CurrentCommonPanel = Cmd.front() - L'0';
			}

			inplace::trim_left(CommandLine);
		}

		inplace::trim(CommandLine);

		if (!CommandLine.empty())
		{
			if (CommandLine.starts_with(L'<'))
			{
				CommandLine.remove_prefix(1);
				auto Plugin = OpenPanelFromOutput(string(CommandLine));
				return Opt.MenuForFilelist? nullptr : Plugin.release();
			}

			const auto TmpOut = FindListFile(unquote(ExpandEnvStrs(CommandLine)));
			if (TmpOut.empty())
				return {};

			if (Opt.MenuForFilelist)
			{
				ShowMenuFromList(TmpOut);
				return {};
			}

			auto Plugin = std::make_unique<TmpPanel>(TmpOut);
			ProcessList(*Plugin, TmpOut, Opt.Replace);
			return Plugin.release();
		}
	}
	else if (Info->OpenFrom == OPEN_ANALYSE)
	{
		const auto AnalyseFileName = reinterpret_cast<OpenAnalyseInfo*>(Info->Data)->Info->FileName;

		if (AnalyseFileName && *AnalyseFileName)
		{
			string Name(AnalyseFileName);

			if (!FSF.ProcessName(Opt.Mask.c_str(), const_cast<wchar_t*>(Name.c_str()), 0, PN_CMPNAMELIST))
				return {};

			if (!Opt.MenuForFilelist)
			{
				auto Plugin = std::make_unique<TmpPanel>(Name);
				ProcessList(*Plugin, Name, Opt.Replace);
				return Plugin.release();
			}

			ShowMenuFromList(Name);
			return PANEL_STOP;
		}

		return {};
	}

	return std::make_unique<TmpPanel>().release();
}

HANDLE WINAPI AnalyseW(const AnalyseInfo* Info)
{
	if (!Info->FileName || !Info->BufferSize)
		return {};

	if (!FSF.ProcessName(Opt.Mask.c_str(), const_cast<wchar_t*>(Info->FileName), lstrlen(Info->FileName), PN_CMPNAMELIST))
		return {};

	return reinterpret_cast<HANDLE>(1);
}

void WINAPI ClosePanelW(const ClosePanelInfo* Info)
{
	std::unique_ptr<TmpPanel>{static_cast<TmpPanel*>(Info->hPanel)};
}

void WINAPI ExitFARW(const ExitInfo* Info)
{
	std::unique_ptr<shared_data>{SharedData};
}

intptr_t WINAPI GetFindDataW(GetFindDataInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->GetFindData(Info->PanelItem, Info->ItemsNumber, Info->OpMode);
}

void WINAPI GetPluginInfoW(PluginInfo* Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_PRELOAD;

	if (Opt.AddToDisksMenu)
	{
		static const wchar_t* DiskMenuStrings[1];
		DiskMenuStrings[0] = GetMsg(MDiskMenuString);
		Info->DiskMenu.Guids = &MenuGuid;
		Info->DiskMenu.Strings = DiskMenuStrings;
		Info->DiskMenu.Count = std::size(DiskMenuStrings);
	}

	if (Opt.AddToPluginsMenu)
	{
		static const wchar_t* PluginMenuStrings[1];
		PluginMenuStrings[0] = GetMsg(MTempPanel);
		Info->PluginMenu.Guids = &MenuGuid;
		Info->PluginMenu.Strings = PluginMenuStrings;
		Info->PluginMenu.Count = std::size(PluginMenuStrings);
	}

	static const wchar_t* PluginCfgStrings[1];
	PluginCfgStrings[0] = GetMsg(MTempPanel);
	Info->PluginConfig.Guids = &MenuGuid;
	Info->PluginConfig.Strings = PluginCfgStrings;
	Info->PluginConfig.Count = std::size(PluginCfgStrings);
	Info->CommandPrefix = Opt.Prefix.c_str();
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->GetOpenPanelInfo(*Info);
}

intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->SetDirectory(Info->Dir, Info->OpMode);
}

intptr_t WINAPI PutFilesW(const PutFilesInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->PutFiles({ Info->PanelItem, Info->ItemsNumber }, Info->SrcPath, Info->OpMode);
}

intptr_t WINAPI SetFindListW(const SetFindListInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->SetFindList({ Info->PanelItem, Info->ItemsNumber });
}

intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->ProcessEvent(Info->Event, Info->Param);
}

intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	return static_cast<TmpPanel*>(Info->hPanel)->ProcessKey(&Info->Rec);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info)
{
	return Config();
}
