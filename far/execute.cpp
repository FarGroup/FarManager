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
#include "log.hpp"
#include "char_width.hpp"
#include "string_sort.hpp"

// Platform:
#include "platform.hpp"
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

static string short_name_if_too_long(string_view const LongName, size_t const MaxSize)
{
	return LongName.size() >= MaxSize? ConvertNameToShort(LongName) : string(LongName);
}

static auto short_file_name_if_too_long(const string& LongName)
{
	return short_name_if_too_long(LongName, MAX_PATH - 1);
}

static auto short_directory_name_if_too_long(const string& LongName)
{
	assert(!LongName.empty());

	const auto HasEndSlash = path::is_separator(LongName.back());

	auto Dir = short_name_if_too_long(LongName, MAX_PATH - (HasEndSlash? 1 : 2));

	// For funny names that end with spaces
	// We do this for SetCurrentDirectory already
	// Here it is for paths that go into CreateProcess & ShellExecuteEx
	if (!HasEndSlash)
		AddEndSlash(Dir);

	return Dir;
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

	const auto IsWithPath = ContainsSlash(Module);

	if (IsWithPath)
	{
		// If a path has been specified it makes no sense to walk through the %PATH%.
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

	if (!IsWithPath)
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

static string get_comspec()
{
	if (auto Comspec = os::env::expand(Global->Opt->Exec.Comspec); !Comspec.empty())
		return Comspec;

	if (auto Comspec = os::env::get(L"COMSPEC"sv); !Comspec.empty())
		return Comspec;

	return {};
}

static std::span<string_view const> exclude_cmds()
{
	if (!Global->Opt->Exec.strExcludeCmds.empty())
		return Global->Opt->Exec.ExcludeCmds;

	if (equal_icase(PointToName(get_comspec()), L"cmd.exe"sv))
	{
		static constexpr std::array PredefinedCmdCommands
		{
			L"ASSOC"sv,
			L"CALL"sv,
			L"CD"sv,
			L"CHCP"sv,
			L"CHDIR"sv,
			L"CLS"sv,
			L"COLOR"sv,
			L"COPY"sv,
			L"DATE"sv,
			L"DEL"sv,
			L"DIR"sv,
			L"DPATH"sv,
			L"ECHO"sv,
			L"ERASE"sv,
			L"EXIT"sv,
			L"FOR"sv,
			L"FTYPE"sv,
			L"IF"sv,
			L"KEYS"sv,
			L"MD"sv,
			L"MKDIR"sv,
			L"MKLINK"sv,
			L"MOVE"sv,
			L"PATH"sv,
			L"PAUSE"sv,
			L"POPD"sv,
			L"PROMPT"sv,
			L"PUSHD"sv,
			L"RD"sv,
			L"REM"sv,
			L"REN"sv,
			L"RENAME"sv,
			L"RMDIR"sv,
			L"SET"sv,
			L"START"sv,
			L"TIME"sv,
			L"TITLE"sv,
			L"TYPE"sv,
			L"VER"sv,
			L"VERIFY"sv,
			L"VOL"sv,
		};

		return PredefinedCmdCommands;
	}

	return {};
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

			try
			{
				Re.Re->Compile(Condition, OP_OPTIMIZE);
			}
			catch (regex_exception const& e)
			{
				LOGERROR(L"ComspecCondition regex error: {}; position: {}"sv, e.message(), e.position());
				Re.Re.reset();
			}
			Re.Pattern = Condition;
		}

		if (Re.Re)
		{
			if (Re.Re->Search(FullCommand))
				return false;

			UseDefaultCondition = false;
		}
	}

	const auto Begin = std::ranges::find_if(FullCommand, [](wchar_t i){ return i != L' '; });
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

	string_view const Cmd{ Begin, CmdEnd };
	if (std::ranges::binary_search(exclude_cmds(), Cmd, string_sort::less_icase))
		return false;

	Command = Cmd;
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
	Info.DisplayCommand = Folder;
	Info.Command = Folder;
	// To avoid collisions with bat/cmd/etc.
	AddEndSlash(Info.Command);
	Info.WaitMode = execute_info::wait_mode::no_wait;
	Info.SourceMode = execute_info::source_mode::known_external;

	Execute(Info);
}

[[nodiscard]]
static bool wait_for_process(os::handle const& Process, int const ConsoleDetachKey)
{
	if (!ConsoleDetachKey)
	{
		Process.wait();
		return true;
	}

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

	// Everywhere else we peek & read input records one by one,
	// so it does not make much sense to complicate things
	// and support multiple records everywhere because of this single case.
	::console_detail::console::input_queue_inspector QueueInspector;

	//Тут нельзя делать WaitForMultipleObjects из за бага в Win7 при работе в телнет
	while (!Process.is_signaled(100ms))
	{
		if (QueueInspector.search(is_detach_key))
			return false;
	}

	return true;
}

static void detach(point const& ConsoleSize, rectangle const& ConsoleWindowRect)
{
	auto Aliases = console.GetAllAliases();

	consoleicons::instance().restore_icon();

	FlushInputBuffer();
	ClearKeyQueue();

	/*
	  Не будем вызывать CloseConsole, потому, что она поменяет
	  ConsoleMode на тот, что был до запуска Far'а,
	  чего работающее приложение могло и не ожидать.
	*/

	if (const auto Window = console.GetWindow())   // если окно имело HOTKEY, то старое должно его забыть.
		SendMessage(Window, WM_SETHOTKEY, 0, 0);

	console.Free();
	console.Allocate();

	InitConsole();
	Global->ScrBuf->FillBuf();

	console.SetSize(ConsoleSize);
	console.SetWindowRect(ConsoleWindowRect);
	console.SetSize(ConsoleSize);

	console.SetAllAliases(std::move(Aliases));
}

[[nodiscard]]
static os::handle wait_for_process_or_detach(os::handle Process, int const ConsoleDetachKey, point const& ConsoleSize, rectangle const& ConsoleWindowRect)
{
	if (wait_for_process(Process, ConsoleDetachKey))
	{
		if (point Size; !console.GetSize(Size) && GetLastError() == ERROR_PIPE_NOT_CONNECTED)
		{
			// The process has crashed the conhost. Well done. *slow clap*
			detach(ConsoleSize, ConsoleWindowRect);
		}

		return Process;
	}

	detach(ConsoleSize, ConsoleWindowRect);
	return {};
}

static void log_process_exit_code(execute_info const& Info, os::handle const& Process, bool const UsingComspec)
{
	DWORD ExitCode;
	if (!GetExitCodeProcess(Process.native_handle(), &ExitCode))
	{
		LOGWARNING(L"GetExitCodeProcess({}): {}"sv, Info.Command, os::last_error());
		return;
	}

	LOG(
		ExitCode == EXIT_SUCCESS?
			logging::level::debug :
			logging::level::warning,
		L"Exit code: {}"sv,
		ExitCode
	);

	if (ExitCode != EXIT_SUCCESS && ExitCode != EXIT_FAILURE)
	{
		LOGWARNING(L"{}"sv, os::format_error(ExitCode));
		LOGWARNING(L"{}"sv, os::format_ntstatus(ExitCode));
	}

	console.command_finished(ExitCode);

	if (UsingComspec && ExitCode == EXIT_FAILURE)
		console.command_not_found(Info.Command);
}

static void after_process_creation(
	execute_info const& Info,
	os::handle Process,
	execute_info::wait_mode const WaitMode,
	os::handle Thread,
	point const& ConsoleSize,
	rectangle const& ConsoleWindowRect,
	function_ref<void(bool)> const ConsoleActivator,
	bool const UsingComspec
)
{
	const auto resume_process = [&](bool const Consolise)
	{
		ConsoleActivator(Consolise);

		if (Thread)
		{
			ResumeThread(Thread.native_handle());
			Thread = {};
		}
	};

	switch (WaitMode)
	{
	case execute_info::wait_mode::no_wait:
		resume_process(false);
		return;

	case execute_info::wait_mode::if_needed:
		{
			const auto NeedWaiting = os::process::get_process_subsystem(Process.get()) != os::process::image_type::graphical;

			resume_process(NeedWaiting);

			if (!NeedWaiting)
				return;

			Process = wait_for_process_or_detach(std::move(Process), KeyNameToKey(Global->Opt->ConsoleDetachKey), ConsoleSize, ConsoleWindowRect);
			if (Process)
				log_process_exit_code(Info, Process, UsingComspec);
		}
		return;

	case execute_info::wait_mode::wait_finish:
		resume_process(true);
		Process.wait();
		log_process_exit_code(Info, Process, UsingComspec);
		return;
	}
}

static bool UseComspec(string& FullCommand, string& Command, string& Parameters)
{
	Command = get_comspec();

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

	try
	{
		Parameters = far::vformat(os::env::expand(Global->Opt->Exec.ComspecArguments), FullCommand);
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

	auto FullCommand = full_command(quote(Command), Parameters);
	STARTUPINFO si{ sizeof(si) };

	LOGDEBUG(L"CreateProcess({})"sv, FullCommand);

	if (!CreateProcess(
		// We can't pass ApplicationName - if it's a bat file with a funny name (e.g. containing '&')
		// it will fail because CreateProcess doesn't quote it properly when spawning comspec,
		// and we can't quote it ourselves because it's not supported.
		{},
		FullCommand.data(),
		{},
		{},
		false,
		CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED | (Wait? 0 : CREATE_NEW_CONSOLE),
		{},
		Directory.c_str(),
		&si,
		&pi
	))
	{
		LOGDEBUG(L"CreateProcess({}): {}"sv, FullCommand, os::last_error());
		return false;
	}

	return true;
}

static bool execute_shell(string const& Command, string const& Parameters, string const& Directory, execute_info::source_mode const SourceMode, bool const RunAs, bool const Wait, HANDLE& Process)
{
	SHELLEXECUTEINFO Info{ sizeof(Info) };
	Info.lpFile = Command.c_str();
	Info.lpParameters = EmptyToNull(Parameters);
	Info.lpDirectory = Directory.c_str();
	Info.nShow = SW_SHOWNORMAL;
	Info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS | (Wait? SEE_MASK_NO_CONSOLE : 0);
	Info.lpVerb = RunAs? L"runas" : nullptr;

	if (any_of(SourceMode, execute_info::source_mode::known, execute_info::source_mode::known_executable))
	{
		assert(Parameters.empty());

		// In this mode we know exactly what we're launching,
		// but ShellExecuteEx might still resort to some AI
		// and turn path\file_without_ext into path\file_without_ext.exe.
		// To prevent that, we specify the extension explicitly.
		if (auto Extension = name_ext(Command).second; !equal_icase(Extension, L".lnk"sv))
		{
			if (Extension.empty())
				Extension = L"."sv;
			// .data() is fine, the underlying string is in the outer scope and null-terminated.
			Info.lpClass = Extension.data();
			Info.fMask |= SEE_MASK_CLASSNAME;
		}
	}

	LOGDEBUG(L"ShellExecuteEx({})"sv, Command);

	if (!ShellExecuteEx(&Info))
	{
		LOGDEBUG(L"ShellExecuteEx({}): {}"sv, Command, os::last_error());
		return false;
	}

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
	}

	~external_execution_context()
	{
		SCOPED_ACTION(os::last_error_guard);

		SetFarConsoleMode(true);
		SetPalette();

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
		else
		{
			if (console.GetOutputCodepage() != ConsoleOutputCP)
			{
				char_width::invalidate();
			}
		}

		// Could be changed by the external program
		Global->ScrBuf->Invalidate();
	}

private:
	uintptr_t ConsoleCP = console.GetInputCodepage();
	uintptr_t ConsoleOutputCP = console.GetOutputCodepage();
};

static bool execute_impl(
	const execute_info& Info,
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
	auto ConsoleActivatorInvoked = false;

	const auto ExtendedActivator = [&](bool const Consolise)
	{
		if (Context)
			return;

		if (!ConsoleActivatorInvoked)
		{
			ConsoleActivator(Consolise);
			ConsoleActivatorInvoked = true;
		}

		if (Consolise)
		{
			console.GetWindowRect(ConsoleWindowRect);
			console.GetSize(ConsoleSize);
			Context.emplace();
		}
	};

	const auto execute_process = [&]
	{
		PROCESS_INFORMATION pi{};
		if (!execute_createprocess(Command, Parameters, CurrentDirectory, Info.RunAs, Info.WaitMode != execute_info::wait_mode::no_wait, pi))
			return false;

		after_process_creation(Info, os::handle(pi.hProcess), Info.WaitMode, os::handle(pi.hThread), ConsoleSize, ConsoleWindowRect, ExtendedActivator, UsingComspec);
		return true;

	};

	// Filter out the cases where the source is known, but is not a known executable (exe, com, bat, cmd, see IsExecutable in filelist.cpp)
	// This should cover gh-449 and is usually pointless anyway.
	if (
		Info.SourceMode == execute_info::source_mode::known_executable ||
		Info.SourceMode == execute_info::source_mode::unknown)
	{
		if (execute_process())
			return true;

		if (os::last_error().Win32Error == ERROR_EXE_MACHINE_TYPE_MISMATCH)
			return false;
	}

	ExtendedActivator(Info.WaitMode != execute_info::wait_mode::no_wait);

	const auto execute_shell = [&]
	{
		HANDLE Process;
		if (!::execute_shell(Command, Parameters, CurrentDirectory, Info.SourceMode, Info.RunAs, Info.WaitMode != execute_info::wait_mode::no_wait, Process))
			return false;

		if (Process)
			after_process_creation(Info, os::handle(Process), Info.WaitMode, {}, ConsoleSize, ConsoleWindowRect, [](bool){}, UsingComspec);
		return true;
	};

	if (execute_shell())
		return true;

	if (os::last_error().Win32Error != ERROR_FILE_NOT_FOUND || UsingComspec || !UseComspec(FullCommand, Command, Parameters))
		return false;

	UsingComspec = true;
	return execute_process() || execute_shell();
}

void Execute(execute_info& Info, function_ref<void(bool)> const ConsoleActivator)
{
	// CreateProcess retardedly doesn't take into account CurrentDirectory when searching for the executable.
	// SearchPath looks there as well and if it's set to something else we could get unexpected results.
	const auto CurrentDirectory = short_directory_name_if_too_long(Info.Directory.empty()? os::fs::get_current_directory() : Info.Directory);
	os::fs::process_current_directory_guard const Guard(CurrentDirectory);


	string FullCommand, Command, Parameters;

	bool UsingComspec = false;

	if (Info.RunAs)
	{
		Info.WaitMode = execute_info::wait_mode::no_wait;
	}

	if (Info.SourceMode != execute_info::source_mode::unknown)
	{
		FullCommand = Info.Command;
		Command = short_file_name_if_too_long(Info.Command);
	}
	else
	{
		FullCommand = os::env::expand(Info.Command);

		if (PartCmdLine(FullCommand, Command, Parameters))
		{
			// Can happen if the user entered only spaces.
			// No point in going further.
			if (Command.empty())
				return;

			// Unfortunately it's not possible to avoid the manual search, see gh-290.
			if (string FullName; FindObject(Command, FullName))
			{
				Command = std::move(FullName);
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
		Info.SourceMode != execute_info::source_mode::unknown ||
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
		if (ProcessLocalFileTypes(Command, ObjectNameShort, FILETYPE_EXEC, Info.WaitMode == execute_info::wait_mode::wait_finish, Info.Directory, false, Info.RunAs, [&](execute_info& AssocInfo)
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

	const auto ErrorState = os::last_error();

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

	while (Command.starts_with(L'@'))
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
