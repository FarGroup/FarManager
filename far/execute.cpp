/*
execute.cpp

"Запускатель" программ.
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

// Self:
#include "execute.hpp"

// Internal:
#include "keyboard.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "encoding.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "console.hpp"
#include "lang.hpp"
#include "filetype.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "RegExp.hpp"
#include "scrbuf.hpp"
#include "global.hpp"
#include "keys.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.process.hpp"

// Common:
#include "common.hpp"
#include "common/enum_tokens.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static string short_name_if_too_long(const string& LongName)
{
	return LongName.size() >= MAX_PATH - 1? ConvertNameToShort(LongName) : LongName;
}

static bool FindObject(string_view const Command, string& strDest)
{
	const auto Module = unquote(Command);

	if (Module.empty())
		return false;

	const auto ModuleExt = name_ext(Module).second;
	const auto PathExtList = enum_tokens(lower(os::env::get_pathext()), L";"sv);

	const auto TryWithExtOrPathExt = [&](string_view const Name, const auto& Predicate)
	{
		if (!ModuleExt.empty())
		{
			const auto Result = Predicate(Name, true);
			if (Result.first)
				return Result;
		}

		// Try all the %PATHEXT%:
		for (const auto& Ext: PathExtList)
		{
			const auto Result = Predicate(Name + Ext, !Ext.empty());
			if (Result.first)
				return Result;
		}

		// Try "as is".
		// Even though it could be the best possible match, picking a name without extension
		// is rather unexpected on the current target platform, it's better to disable it for good.
		// This comment is kept for historic purposes and to stop trying this again in future.
		// If you really want to look for files w/o extension - add ";;" to the %PATHEXT%.
		// return Predicate(Name);

		return std::pair(false, L""s);
	};

	if (IsAbsolutePath(Module))
	{
		// If absolute path has been specified it makes no sense to walk through the %PATH%.
		// Just try all the extensions and we are done here:
		const auto [Found, FoundName] = TryWithExtOrPathExt(Module, [](string_view const NameWithExt, bool)
		{
			return std::pair(os::fs::is_file(NameWithExt), string(NameWithExt));
		});

		if (Found)
		{
			strDest = FoundName;
			return true;
		}
	}

	{
		// Look in the current directory:
		const auto FullName = ConvertNameToFull(Module);
		const auto [Found, FoundName] = TryWithExtOrPathExt(FullName, [](string_view const NameWithExt, bool)
		{
			return std::pair(os::fs::is_file(NameWithExt), string(NameWithExt));
		});

		if (Found)
		{
			strDest = FoundName;
			return true;
		}
	}

	{
		// Look in the %PATH%:
		const auto PathEnv = os::env::get(L"PATH"sv);
		if (!PathEnv.empty())
		{
			for (const auto& Path: enum_tokens_with_quotes(PathEnv, L";"sv))
			{
				if (Path.empty())
					continue;

				const auto[Found, FoundName] = TryWithExtOrPathExt(path::join(Path, Module), [](string_view const NameWithExt, bool)
				{
					return std::pair(os::fs::is_file(NameWithExt), string(NameWithExt));
				});

				if (Found)
				{
					strDest = FoundName;
					return true;
				}
			}
		}
	}

	{
		// Use SearchPath:
		const auto [Found, FoundName] = TryWithExtOrPathExt(Module, [](string_view const NameWithExt, bool const HasExt)
		{
			string Str;
			return std::pair(os::fs::SearchPath(nullptr, NameWithExt, HasExt? nullptr : L".", Str) && os::fs::is_file(Str), Str);
		});

		if (Found)
		{
			strDest = FoundName;
			return true;
		}
	}

	return false;
}

/*
 true: ok, found command & arguments.
 false: it's too complex, let comspec deal with it.
*/
static bool PartCmdLine(string_view const FullCommand, string& Command, string& Parameters)
{
	auto UseDefaultCondition = true;

	// Custom comspec condition logic, gives the user ability to provide his own rules in form of regular expression, for example ^(?:[^"]|"[^"]*")*?[<>|&]

	// Do not use std::regex here.
	// VC implementation has limited complexity and throws regex_error on long strings.
	// gcc implementation is total rubbish - it just causes a stack overflow. Shame on them.

	// If anything goes wrong, e. g. pattern is incorrect or search failed - default condition (checking for presence of <>|& characters outside the quotes) will be used.
	const auto Condition = os::env::expand(Global->Opt->Exec.ComspecCondition);
	if (!Condition.empty())
	{
		auto& Re = Global->Opt->Exec.ComspecConditionRe;

		if (Re.Pattern != Condition)
		{
			Re.Re = std::make_unique<RegExp>();
			if (!Re.Re->Compile(Condition, OP_OPTIMIZE))
			{
				Re.Re.reset();
			}
			Re.Pattern = Condition;
		}

		if (Re.Re)
		{
			if (Re.Re->Search(FullCommand))
				return false;

			if (Re.Re->LastError() == errNone)
			{
				UseDefaultCondition = false;
			}
		}
	}

	const auto Begin = std::find_if(ALL_CONST_RANGE(FullCommand), [](wchar_t i){ return i != L' '; });
	const auto End = FullCommand.cend();
	auto CmdEnd = End;
	auto ParamsBegin = End;
	auto InQuotes = false;

	for (auto i = Begin; i != End; ++i)
	{
		if (*i == L'"')
		{
			InQuotes = !InQuotes;
			continue;
		}

		if (!InQuotes && UseDefaultCondition && contains(L"<>|&"sv, *i))
		{
			return false;
		}

		if (!InQuotes && *i == L' ')
		{
			// First unquoted space is definitely a command / parameter separator, iterators shall be updated now (and only once):
			if (CmdEnd == End)
			{
				CmdEnd = i;
				ParamsBegin = i + 1;
			}

			// However, if we are in 'default condition' mode, we can't exit early as there still might be unquoted special characters in the tail.
			if (!UseDefaultCondition)
			{
				break;
			}
		}
	}

	const auto Cmd = make_string_view(Begin, CmdEnd);
	const auto ExcludeCmds = enum_tokens(os::env::expand(Global->Opt->Exec.strExcludeCmds), L";"sv);
	if (std::any_of(ALL_CONST_RANGE(ExcludeCmds), [&](string_view const i){ return equal_icase(i, Cmd); }))
		return false;

	Command.assign(Begin, CmdEnd);
	Parameters.assign(ParamsBegin, End);
	return true;
}

static auto full_command(string_view const Command, string_view const Parameters)
{
	return Parameters.empty()? string(Command) : concat(Command, L' ', Parameters);
}

void OpenFolderInShell(string_view const Folder)
{
	execute_info Info;
	auto FullFolderName = ConvertNameToFull(Folder);
	// To avoid collisions with bat/cmd/etc.
	AddEndSlash(FullFolderName);
	Info.DisplayCommand = Folder;
	Info.Command = Folder;
	Info.WaitMode = execute_info::wait_mode::no_wait;
	Info.SourceMode = execute_info::source_mode::known;

	Execute(Info);
}

static void wait_for_process_or_detach(os::handle const& Process, int const ConsoleDetachKey, point const& ConsoleSize, rectangle const& ConsoleWindowRect)
{
	const auto
		hInput = console.GetInputHandle(),
		hOutput = console.GetOutputHandle();

	const auto ConfigVKey = TranslateKeyToVK(ConsoleDetachKey);

	enum class dual_key_t
	{
		none,
		right,
		any
	};

	const auto dual_key = [](DWORD const Mask, const DWORD Left, const DWORD Right)
	{
		return Mask & Left? dual_key_t::any : Mask & Right? dual_key_t::right : dual_key_t::none;
	};

	const auto match = [](dual_key_t Expected, dual_key_t Actual)
	{
		return Expected == dual_key_t::any? Actual != dual_key_t::none : Expected == Actual;
	};

	const auto
		ConfigCtrl  = dual_key(ConsoleDetachKey, KEY_CTRL,  KEY_RCTRL),
		ConfigAlt   = dual_key(ConsoleDetachKey, KEY_ALT,   KEY_RALT),
		ConfigShift = dual_key(ConsoleDetachKey, KEY_SHIFT, KEY_RSHIFT);

	const auto is_detach_key = [&](INPUT_RECORD const& i)
	{
		if (i.EventType != KEY_EVENT)
			return false;

		const auto ControlKeyState = i.Event.KeyEvent.dwControlKeyState;

		const auto
			Ctrl  = dual_key(ControlKeyState, LEFT_CTRL_PRESSED, RIGHT_CTRL_PRESSED),
			Alt   = dual_key(ControlKeyState, LEFT_ALT_PRESSED,  RIGHT_ALT_PRESSED),
			Shift = dual_key(ControlKeyState, SHIFT_PRESSED,     SHIFT_PRESSED); // BUGBUG

		return ConfigVKey == i.Event.KeyEvent.wVirtualKeyCode && match(ConfigCtrl, Ctrl) && match(ConfigAlt, Alt) && match(ConfigShift, Shift);
	};

	std::vector<INPUT_RECORD> Buffer(256);

	//Тут нельзя делать WaitForMultipleObjects из за бага в Win7 при работе в телнет
	while (!Process.is_signaled(100ms))
	{
		const auto NumberOfEvents = []
		{
			size_t Result;
			return console.GetNumberOfInputEvents(Result)? Result : 0;
		}();

		if (Buffer.size() < NumberOfEvents)
		{
			Buffer.clear();
			Buffer.resize(NumberOfEvents + NumberOfEvents / 2);
		}

		if (!os::handle::is_signaled(hInput, 100ms))
			continue;

		size_t EventsRead;
		if (!console.PeekInput(Buffer, EventsRead) || !EventsRead)
			continue;

		if (std::none_of(Buffer.cbegin(), Buffer.cbegin() + EventsRead, is_detach_key))
			continue;

		const auto Aliases = console.GetAllAliases();

		consoleicons::instance().restore_icon();

		FlushInputBuffer();
		ClearKeyQueue();

		/*
		  Не будем вызывать CloseConsole, потому, что она поменяет
		  ConsoleMode на тот, что был до запуска Far'а,
		  чего работающее приложение могло и не ожидать.
		*/

		os::handle{ hInput };
		os::handle{ hOutput };

		console.Free();
		console.Allocate();

		if (const auto Window = console.GetWindow())   // если окно имело HOTKEY, то старое должно его забыть.
			SendMessage(Window, WM_SETHOTKEY, 0, 0);

		console.SetSize(ConsoleSize);
		console.SetWindowRect(ConsoleWindowRect);
		console.SetSize(ConsoleSize);

		os::chrono::sleep_for(100ms);
		InitConsole();

		consoleicons::instance().set_icon();
		console.SetAllAliases(Aliases);

		return;
	}
}


static void after_process_creation(os::handle Process, execute_info::wait_mode const WaitMode, HANDLE Thread, point const& ConsoleSize, rectangle const& ConsoleWindowRect, function_ref<void(bool)> const ConsoleActivator)
{
	switch (WaitMode)
	{
	case execute_info::wait_mode::no_wait:
		ConsoleActivator(false);
		if (Thread)
			ResumeThread(Thread);
		return;

	case execute_info::wait_mode::if_needed:
		{
			const auto NeedWaiting = os::process::get_process_subsystem(Process.get()) != os::process::image_type::graphical;
			ConsoleActivator(NeedWaiting);
			if (Thread)
				ResumeThread(Thread);

			if (!NeedWaiting)
				return;

			if (const auto ConsoleDetachKey = KeyNameToKey(Global->Opt->ConsoleDetachKey))
				wait_for_process_or_detach(Process, ConsoleDetachKey, ConsoleSize, ConsoleWindowRect);
			else
				Process.wait();
		}
		return;

	case execute_info::wait_mode::wait_finish:
		ConsoleActivator(true);
		if (Thread)
			ResumeThread(Thread);
		Process.wait();
		return;
	}
}

static bool UseComspec(string& FullCommand, string& Command, string& Parameters)
{
	Command = os::env::expand(Global->Opt->Exec.Comspec);
	if (Command.empty())
	{
		Command = os::env::get(L"COMSPEC"sv);
		if (Command.empty())
		{
			Message(MSG_WARNING,
				msg(lng::MError),
				{
					msg(lng::MComspecNotFound)
				},
				{ lng::MOk });
			return false;
		}
	}

	try
	{
		Parameters = format(os::env::expand(Global->Opt->Exec.ComspecArguments), FullCommand);
	}
	catch (fmt::format_error const& e)
	{
		// The user entered something weird
		// TODO: use the default?
		Message(MSG_WARNING,
			msg(lng::MError),
			{
				concat(L"System.Executor.ComspecArguments: \""sv, Global->Opt->Exec.ComspecArguments, L'"'),
				encoding::utf8::get_chars(e.what())
			},
			{ lng::MOk });
		return false;
	}

	FullCommand = full_command(Command, Parameters);
	return true;
}

static bool execute_createprocess(string const& Command, string const& Parameters, string const& Directory, bool const RunAs, bool const Wait, PROCESS_INFORMATION& pi)
{
	if (RunAs)
		return false;

	STARTUPINFO si{ sizeof(si) };

	return CreateProcess(
		// We can't pass ApplicationName - if it's a bat file with a funny name (e.g. containing '&')
		// it will fail because CreateProcess doesn't quote it properly when spawning comspec,
		// and we can't quote it ourselves because it's not supported.
		{},
		full_command(quote(Command), Parameters).data(),
		{},
		{},
		false,
		CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED | (Wait? 0 : CREATE_NEW_CONSOLE),
		{},
		Directory.c_str(),
		&si,
		&pi
	);
}

static bool execute_shell(string const& Command, string const& Parameters, string const& Directory, bool const RunAs, bool const Wait, HANDLE& Process)
{
	SHELLEXECUTEINFO Info = { sizeof(Info) };
	Info.lpFile = Command.c_str();
	Info.lpParameters = EmptyToNull(Parameters);
	Info.lpDirectory = Directory.c_str();
	Info.nShow = SW_SHOWNORMAL;
	Info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS | (Wait? SEE_MASK_NO_CONSOLE : 0);
	Info.lpVerb = RunAs? L"runas" : nullptr;

	if (!ShellExecuteEx(&Info))
		return false;

	Process = Info.hProcess;

	return true;
}

class external_execution_context
{
public:
	NONCOPYABLE(external_execution_context);

	external_execution_context()
	{
		FlushInputBuffer();

		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);

		GetCursorType(CursorVisible, CursorSize);
	}

	~external_execution_context()
	{
		SetFarConsoleMode(true);

		SetCursorType(CursorVisible, CursorSize);

		point ConSize;
		if (console.GetSize(ConSize) && (ConSize.x != ScrX + 1 || ConSize.y != ScrY + 1))
		{
			ChangeVideoMode(ConSize.y, ConSize.x);
		}

		if (Global->Opt->Exec.RestoreCPAfterExecute)
		{
			console.SetInputCodepage(ConsoleCP);
			console.SetOutputCodepage(ConsoleOutputCP);
		}

		// The title could be changed by the external program
		Global->ScrBuf->Flush(flush_type::cursor | flush_type::title);
	}

private:
	int ConsoleCP = console.GetInputCodepage();
	int ConsoleOutputCP = console.GetOutputCodepage();
	bool CursorVisible{};
	size_t CursorSize{};
};

static bool execute_impl(
	execute_info& Info,
	function_ref<void(bool)> const ConsoleActivator,
	string& FullCommand,
	string& Command,
	string& Parameters,
	string const& CurrentDirectory,
	bool& UsingComspec
)
{
	rectangle ConsoleWindowRect;
	point ConsoleSize;
	std::optional<external_execution_context> Context;

	const auto ExtendedActivator = [&](bool const Consolise)
	{
		if (Context)
			return;

		if (Consolise)
		{
			console.GetWindowRect(ConsoleWindowRect);
			console.GetSize(ConsoleSize);
			Context.emplace();
		}

		ConsoleActivator(Consolise);
	};

	const auto execute_process = [&]
	{
		PROCESS_INFORMATION pi{};
		if (!execute_createprocess(Command, Parameters, CurrentDirectory, Info.RunAs, Info.WaitMode != execute_info::wait_mode::no_wait, pi))
			return false;

		after_process_creation(os::handle(pi.hProcess), Info.WaitMode, pi.hThread, ConsoleSize, ConsoleWindowRect, ExtendedActivator);
		return true;

	};

	if (execute_process())
		return true;

	if (error_state::fetch().Win32Error == ERROR_EXE_MACHINE_TYPE_MISMATCH)
		return false;

	ExtendedActivator(Info.WaitMode != execute_info::wait_mode::no_wait);

	const auto execute_shell = [&]
	{
		HANDLE Process;
		if (!::execute_shell(Command, Parameters, CurrentDirectory, Info.RunAs, Info.WaitMode != execute_info::wait_mode::no_wait, Process))
			return false;

		if (Process)
			after_process_creation(os::handle(Process), Info.WaitMode, {}, ConsoleSize, ConsoleWindowRect, [](bool){});
		return true;
	};

	if (execute_shell())
		return true;

	if (error_state::fetch().Win32Error != ERROR_FILE_NOT_FOUND || UsingComspec || !UseComspec(FullCommand, Command, Parameters))
		return false;

	UsingComspec = true;
	return execute_process() || execute_shell();
}

void Execute(execute_info& Info, function_ref<void(bool)> const ConsoleActivator)
{
	// CreateProcess retardedly doesn't take into account CurrentDirectory when searching for the executable.
	// SearchPath looks there as well and if it's set to something else we could get unexpected results.
	const auto CurrentDirectory = short_name_if_too_long(os::fs::GetCurrentDirectory());
	os::fs::process_current_directory_guard const Guard(CurrentDirectory);


	string FullCommand, Command, Parameters;

	bool UsingComspec = false;

	if (Info.RunAs)
	{
		Info.WaitMode = execute_info::wait_mode::no_wait;
	}

	if (Info.SourceMode == execute_info::source_mode::known)
	{
		FullCommand = Info.Command;
		Command = short_name_if_too_long(Info.Command);
	}
	else
	{
		FullCommand = os::env::expand(Info.Command);

		if (PartCmdLine(FullCommand, Command, Parameters))
		{
			string FullName;
			// Unfortunately it's not possible to avoid the manual search, see gh-290.
			if (FindObject(Command, FullName))
			{
				Command = FullName;
			}
		}
		else
		{
			// Complex expression (pipe or redirection): fallback to comspec as is
			if (!UseComspec(FullCommand, Command, Parameters))
				return;
			UsingComspec = true;
		}
	}

	const auto IgnoreInternalAssociations =
		!Info.UseAssociations ||
		Info.SourceMode == execute_info::source_mode::known ||
		Info.WaitMode == execute_info::wait_mode::no_wait ||
		UsingComspec ||
		!Global->Opt->Exec.UseAssociations;

	static unsigned ProcessingAsssociation = 0;

	if (!IgnoreInternalAssociations && !ProcessingAsssociation)
	{
		++ProcessingAsssociation;
		SCOPE_EXIT{ --ProcessingAsssociation; };

		const auto ObjectNameShort = ConvertNameToShort(Command);
		const auto LastX = WhereX(), LastY = WhereY();
		if (ProcessLocalFileTypes(Command, ObjectNameShort, FILETYPE_EXEC, Info.WaitMode == execute_info::wait_mode::wait_finish, false, Info.RunAs, [&](execute_info& AssocInfo)
		{
			GotoXY(LastX, LastY);

			if (!Parameters.empty())
			{
				AssocInfo.Command = full_command(AssocInfo.Command, Parameters);
				AssocInfo.DisplayCommand = Info.DisplayCommand;
			}

			Global->CtrlObject->CmdLine()->ExecString(AssocInfo);
		}))
		{
			return;
		}
		GotoXY(LastX, LastY);
	}

	if (execute_impl(Info, ConsoleActivator, FullCommand, Command, Parameters, CurrentDirectory, UsingComspec))
		return;

	const auto ErrorState = error_state::fetch();

	if (ErrorState.Win32Error == ERROR_CANCELLED)
		return;

	std::vector<string> Strings;
	if (UsingComspec)
		Strings = { msg(lng::MCannotInvokeComspec), Command, msg(lng::MCheckComspecVar) };
	else
		Strings = { msg(lng::MCannotExecute), Command };

	Message(MSG_WARNING, ErrorState,
		msg(lng::MError),
		std::move(Strings),
		{ lng::MOk },
		L"ErrCannotExecute"sv,
		nullptr,
		{ Command });
}

static bool exist_predicate(string_view const Str)
{
	auto ExpandedStr = os::env::expand(Str);

	if (!IsAbsolutePath(ExpandedStr))
		ExpandedStr = ConvertNameToFull(ExpandedStr);

	os::fs::find_data Data;
	return os::fs::exists(ExpandedStr) || os::fs::get_find_data(ExpandedStr, Data);
}

static bool defined_predicate(string_view const Str)
{
	string Value;
	return os::env::get(Str, Value);
}

/*
CmdLine:
- "IF [NOT] EXIST filename command"
- "IF [NOT] DEFINED variable command"

Returns:
- a view to the "command" part if the condition is met
- an empty view if the condition is not met
- a view to the whole string in case of any errors
 */
using predicate = bool(string_view);
static string_view PrepareOSIfExist(string_view const CmdLine, predicate IsExists, predicate IsDefined)
{
	if (CmdLine.empty())
		return CmdLine;

	auto Command = CmdLine;

	while (starts_with(Command, L'@'))
		Command.remove_prefix(1);

	const auto skip_spaces = [](string_view& Str)
	{
		while (!Str.empty() && std::iswblank(Str.front()))
			Str.remove_prefix(1);

		return !Str.empty();
	};

	const auto get_token = [](string_view & Str, string_view const Token)
	{
		if (!starts_with_icase(Str, Token))
			return false;

		Str.remove_prefix(Token.size());
		return true;
	};

	bool SkippedSomethingValid = false;

	for (;;)
	{
		if (!skip_spaces(Command))
			break;

		if (!get_token(Command, L"if "sv))
			break;

		if (!skip_spaces(Command))
			break;

		const auto IsNot = get_token(Command, L"not "sv);

		if (!skip_spaces(Command))
			break;

		const auto IsExistToken = get_token(Command, L"exist "sv);

		if (!IsExistToken && !get_token(Command, L"defined "sv))
			break;

		if (!skip_spaces(Command))
			break;

		const auto ExpressionStart = Command;

		auto InQuotes = false;
		while (!Command.empty())
		{
			if (Command.front() == L'"')
				InQuotes = !InQuotes;
			else if (Command.front() == L' ' && !InQuotes)
				break;

			Command.remove_prefix(1);
		}

		if (Command.empty())
			break;

		if ((IsExistToken? IsExists : IsDefined)(unquote(ExpressionStart.substr(0, ExpressionStart.size() - Command.size()))) == IsNot)
			return {};

		SkippedSomethingValid = true;
	}

	return SkippedSomethingValid? CmdLine.substr(CmdLine.size() - Command.size()) : CmdLine;
}

/* $ 25.04.2001 DJ
обработка @ в IF EXIST: функция, которая извлекает команду из строки
с IF EXIST с учетом @ и возвращает TRUE, если условие IF EXIST
выполено, и FALSE в противном случае/
*/

bool ExtractIfExistCommand(string& strCommandText)
{
	const auto Cmd = PrepareOSIfExist(strCommandText, exist_predicate, defined_predicate);

	if (Cmd.empty())
		return false; // Do not execute

	if (Cmd.size() == strCommandText.size())
		return true; // Something went wrong, continue as is

	const auto Pos = strCommandText.find_first_not_of(L'@');
	strCommandText.erase(Pos, strCommandText.size() - Cmd.size() - Pos);

	return true;
}

bool ExpandOSAliases(string& FullCommand)
{
	string Command, Parameters;

	if (!PartCmdLine(FullCommand, Command, Parameters))
		return false;

	if (!console.GetAlias(Command, Command, PointToName(Global->g_strFarModuleName)))
	{
		const auto Comspec = os::env::get(L"COMSPEC"sv);
		if (Comspec.empty() || !console.GetAlias(Command, Command, PointToName(Comspec)))
			return false;
	}

	FullCommand = ReplaceStrings(Command, L"$*"sv, Parameters)?
		Command :
		full_command(Command, Parameters);

	return true;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("execute.exist.defined")
{
	static const struct
	{
		string_view From;
		string_view To_ed, To_eD, To_Ed, To_ED;
	}
	Tests[]
	{
		{
			{},

			{}, // ed
			{}, // eD
			{}, // Ed
			{}, // ED
		},
		{
			L" "sv,

			L" "sv, // ed
			L" "sv, // eD
			L" "sv, // Ed
			L" "sv, // ED
		},
		{
			L"foo"sv,

			L"foo"sv, // ed
			L"foo"sv, // eD
			L"foo"sv, // Ed
			L"foo"sv, // ED
		},
		{
			L"  if  exist  bar  foo"sv,

			{},       // ed
			{},       // eD
			L"foo"sv, // Ed
			L"foo"sv  // ED
		},
		{
			L"  if  exist  bar  if  defined  ham  foo"sv,

			{},       // ed
			{},       // eD
			{},       // Ed
			L"foo"sv, // ED
		},
		{
			L"  if  exist  bar  if  not  defined  ham  foo"sv,

			{},       // ed
			{},       // eD
			L"foo"sv, // Ed
			{},       // ED
		},
		{
			L"  if  not  exist  bar  foo"sv,

			L"foo"sv, // ed
			L"foo"sv, // eD
			{},       // Ed
			{},       // ED
		},
		{
			L"  if  not  exist  bar  if  defined  ham  foo"sv,

			{},       // ed
			L"foo"sv, // eD
			{},       // Ed
			{},       // ED
		},
		{
			L"  if  not  exist  bar  if  not  defined  ham  foo"sv,

			L"foo"sv, // ed
			{},       // eD
			{},       // Ed
			{},       // ED
		},
	};

	const auto Exist      = [](string_view const Str) { REQUIRE(Str == L"bar"sv); return true; };
	const auto NotExist   = [](string_view const Str) { REQUIRE(Str == L"bar"sv); return false; };
	const auto Defined    = [](string_view const Str) { REQUIRE(Str == L"ham"sv); return true; };
	const auto NotDefined = [](string_view const Str) { REQUIRE(Str == L"ham"sv); return false; };

	for (const auto& i: Tests)
	{
		REQUIRE(i.To_ed == PrepareOSIfExist(i.From, NotExist, NotDefined));
		REQUIRE(i.To_eD == PrepareOSIfExist(i.From, NotExist, Defined));
		REQUIRE(i.To_Ed == PrepareOSIfExist(i.From, Exist, NotDefined));
		REQUIRE(i.To_ED == PrepareOSIfExist(i.From, Exist, Defined));
	}
}
#endif
