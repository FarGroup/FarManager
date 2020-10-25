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
#include "imports.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "constitle.hpp"
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
#include "platform.reg.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/io.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

enum class image_type
{
	unknown,
	console,
	graphical,
};

static string short_name_if_too_long(const string& LongName)
{
	return LongName.size() >= MAX_PATH - 1? ConvertNameToShort(LongName) : LongName;
}

static bool GetImageType(std::istream& Stream, image_type& ImageType)
{
	IMAGE_DOS_HEADER DOSHeader;
	if (io::read(Stream, edit_bytes(DOSHeader)) != sizeof(DOSHeader))
		return false;

	if (DOSHeader.e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	Stream.seekg(DOSHeader.e_lfanew);

	union
	{
		struct
		{
			DWORD Signature;
			IMAGE_FILE_HEADER FileHeader;
			union
			{
				IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
				IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
			};
		}
		PeHeader;

		IMAGE_OS2_HEADER Os2Header;
	}
	ImageHeader;

	if (io::read(Stream, edit_bytes(ImageHeader)) != sizeof(ImageHeader))
		return false;

	auto Result = image_type::console;

	if (ImageHeader.PeHeader.Signature == IMAGE_NT_SIGNATURE)
	{
		const auto& PeHeader = ImageHeader.PeHeader;

		if (PeHeader.FileHeader.Characteristics & IMAGE_FILE_DLL)
			return false;

		auto ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

		switch (PeHeader.OptionalHeader32.Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			ImageSubsystem = PeHeader.OptionalHeader32.Subsystem;
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			ImageSubsystem = PeHeader.OptionalHeader64.Subsystem;
			break;
		}

		if (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			Result = image_type::graphical;
		}
	}
	else if (ImageHeader.Os2Header.ne_magic == IMAGE_OS2_SIGNATURE)
	{
		const auto& Os2Header = ImageHeader.Os2Header;

		enum { DllOrDriverFlag = 7_bit };
		if (HIBYTE(Os2Header.ne_flags) & DllOrDriverFlag)
			return false;

		enum
		{
			NE_WINDOWS = 1_bit,
			NE_WIN386  = 2_bit,
		};

		if (Os2Header.ne_exetyp == NE_WINDOWS || Os2Header.ne_exetyp == NE_WIN386)
		{
			Result = image_type::graphical;
		}
	}

	ImageType = Result;
	return true;
}

static bool GetImageType(string_view const FileName, image_type& ImageType)
{
	const os::fs::file ModuleFile(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
	if (!ModuleFile)
		return false;

	try
	{
		os::fs::filebuf StreamBuffer(ModuleFile, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);
		return GetImageType(Stream, ImageType);

	}
	catch (const std::exception&)
	{
		return false;
	}
}

static bool IsProperProgID(string_view const ProgID)
{
	return !ProgID.empty() && os::reg::key::open(os::reg::key::classes_root, ProgID, KEY_QUERY_VALUE);
}

/*
Ищем валидный ProgID для файлового расширения по списку возможных
hExtKey - корневой ключ для поиска (ключ расширения)
strType - сюда запишется результат, если будет найден
*/
static string SearchExtHandlerFromList(const os::reg::key& ExtKey)
{
	const auto Enumerator = os::reg::enum_value(ExtKey, L"OpenWithProgIds"sv, KEY_ENUMERATE_SUB_KEYS);
	const auto Iterator = std::find_if(ALL_CONST_RANGE(Enumerator), [](const os::reg::value& i)
	{
			return i.type() == REG_SZ && IsProperProgID(i.name());
	});

	return Iterator != Enumerator.cend()? Iterator->name() : L""s;
}

static bool FindObject(string_view const Module, string& strDest, bool* Internal)
{
	if (Module.empty())
		return false;

	const auto ModuleExt = name_ext(Module).second;
	const auto PathExtList = enum_tokens(lower(os::env::get_pathext()), L";"sv);

	const auto TryWithExtOrPathExt = [&](string_view const Name, const auto& Predicate)
	{
		if (!ModuleExt.empty())
		{
			// Extension has been specified, we don't have to do anything here.
			return Predicate(Name);
		}

		// Try all the %PATHEXT%:
		for (const auto& Ext: PathExtList)
		{
			const auto NameWithExt = Name + Ext;
			const auto Result = Predicate(NameWithExt);
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
		// If absolute path has been specified it makes no sense to walk through the %PATH%, App Paths etc.
		// Just try all the extensions and we are done here:
		const auto [Found, FoundName] = TryWithExtOrPathExt(Module, [](string_view const NameWithExt)
		{
			return std::pair(os::fs::is_file(NameWithExt), string(NameWithExt));
		});

		if (Found)
		{
			strDest = FoundName;
			return true;
		}
	}

	if (Internal && ModuleExt.empty())
	{
		// Neither path nor extension has been specified, it could be some internal %COMSPEC% command:
		const auto ExcludeCmdsList = enum_tokens(os::env::expand(Global->Opt->Exec.strExcludeCmds), L";"sv);

		if (std::any_of(CONST_RANGE(ExcludeCmdsList, i) { return equal_icase(i, Module); }))
		{
			*Internal = true;
			return true;
		}
	}

	{
		// Look in the current directory:
		const auto FullName = ConvertNameToFull(Module);
		const auto[Found, FoundName] = TryWithExtOrPathExt(FullName, [](string_view const NameWithExt)
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
			for (const auto& Path : enum_tokens_with_quotes(PathEnv, L";"sv))
			{
				if (Path.empty())
					continue;

				const auto[Found, FoundName] = TryWithExtOrPathExt(path::join(Path, Module), [](string_view const NameWithExt)
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
		const auto[Found, FoundName] = TryWithExtOrPathExt(Module, [](string_view const NameWithExt)
		{
			string Str;
			return std::pair(os::fs::SearchPath(nullptr, NameWithExt, nullptr, Str) && !os::fs::is_directory(Str), Str);
		});

		if (Found)
		{
			strDest = FoundName;
			return true;
		}
	}

	{
		// Look in the App Paths registry keys:
		if (Global->Opt->Exec.ExecuteUseAppPath && !contains(Module, L'\\'))
		{
			static const os::reg::key* RootFindKey[] = { &os::reg::key::current_user, &os::reg::key::local_machine, &os::reg::key::local_machine };
			const auto FullName = concat(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"sv, Module);

			DWORD samDesired = KEY_QUERY_VALUE;

			for (size_t i = 0; i != std::size(RootFindKey); i++)
			{
				if (i == std::size(RootFindKey) - 1)
				{
					if (const auto RedirectionFlag = os::GetAppPathsRedirectionFlag())
					{
						samDesired |= RedirectionFlag;
					}
					else
					{
						break;
					}
				}

				const auto[Found, FoundName] = TryWithExtOrPathExt(FullName, [&](string_view const NameWithExt)
				{
					string RealName;
					if (RootFindKey[i]->get(NameWithExt, {}, RealName, samDesired))
					{
						RealName = unquote(os::env::expand(RealName));
						return std::pair(os::fs::is_file(RealName), RealName);
					}

					return std::pair(false, L""s);
				});

				if (Found)
				{
					strDest = FoundName;
					return true;
				}
			}
		}
	}

	return false;
}

/*
 true: ok, found command & arguments.
 false: it's too complex, let comspec deal with it.
*/
static bool PartCmdLine(string_view const CmdStr, string& strNewCmdStr, string& strNewCmdPar)
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
			if (Re.Re->Search(CmdStr))
				return false;

			if (Re.Re->LastError() == errNone)
			{
				UseDefaultCondition = false;
			}
		}
	}

	const auto Begin = std::find_if(ALL_CONST_RANGE(CmdStr), [](wchar_t i){ return i != L' '; });
	const auto End = CmdStr.cend();
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

	strNewCmdStr.assign(Begin, CmdEnd);
	strNewCmdPar.assign(ParamsBegin, End);
	return true;
}

static bool RunAsSupported(const wchar_t* Name)
{
	const auto Extension = name_ext(Name).second;
	string Type;
	return !Extension.empty() &&
		GetShellType(Extension, Type) &&
		os::reg::key::open(os::reg::key::classes_root, concat(Type, L"\\shell\\runas\\command"sv), KEY_QUERY_VALUE);
}

const auto CommandName = L"command"sv;

static bool IsShortcutType(string_view const Type)
{
	const auto Key = os::reg::key::open(os::reg::key::classes_root, Type, KEY_QUERY_VALUE);
	if (!Key)
		return false;

	return Key.get(L"IsShortcut"sv);
}

static string GetShellActionForType(string_view const TypeName, string& KeyName)
{
	if (TypeName.empty())
		return {};

	KeyName = concat(TypeName, L"\\shell"sv);

	const auto Key = os::reg::key::open(os::reg::key::classes_root, KeyName, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
	if (!Key)
		return {};

	const auto TryAction = [&](string_view const Action)
	{
		if (!os::reg::key::open(os::reg::key::classes_root, concat(KeyName, L'\\', Action, L'\\', CommandName), KEY_QUERY_VALUE))
			return false;

		append(KeyName, L'\\', Action);
		return true;
	};

	string Action;
	if (Key.get({}, Action) && !Action.empty())
	{
		// Need to clarify if we need to support quotes here
		for (const auto& i: enum_tokens_with_quotes(Action, L","sv))
		{
			if (!i.empty() && TryAction(i))
				return string(i);
		}

		if (TryAction(Action))
			return Action;
	}
	else
	{
		// Сначала проверим "open"...
		const auto OpenAction = L"open"sv;
		if (TryAction(OpenAction))
			return string(OpenAction);

		// ... а теперь все остальное, если "open" нету
		for (const auto& i: os::reg::enum_key(Key))
		{
			if (TryAction(i))
				return string(i);
		}
	}

	return {};
}

static string GetShellTypeFromExtension(string_view const FileName)
{
	auto Ext = name_ext(FileName).second;
	if (Ext.empty())
	{
		// Yes, no matter how mad it looks - it is possible to specify actions for empty extension too
		Ext = L"."sv;
	}

	string Type;
	if (!GetShellType(Ext, Type))
	{
		// Type is absent, however, verbs could be specified right in the extension key
		Type = Ext;
	}

	return Type;
}

static string GetAssociatedApplicationFromType(string_view const TypeName, string& Action)
{
	string KeyName;
	Action = GetShellActionForType(TypeName, KeyName);
	if (Action.empty())
		return {};

	string Application;
	if (!os::reg::key::classes_root.get(concat(KeyName, L'\\', CommandName), {}, Application) || Application.empty())
		return {};

	Application = os::env::expand(Application);

	// Выделяем имя модуля
	if (Application.front() == L'"')
	{
		const auto QuotePos = Application.find(L'"', 1);

		if (QuotePos != string::npos)
		{
			Application.resize(QuotePos);
			Application.erase(0, 1);
		}
	}
	else
	{
		const auto pos = Application.find_first_of(L" \t/"sv);
		if (pos != string::npos)
			Application.resize(pos);
	}

	return Application;
}

static bool ResolveShortcut(string_view const ShortcutPath, string& FilePath, bool& IsDirectory)
{
	os::com::ptr<IShellLinkW> ShellLink;
	if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, IID_PPV_ARGS_Helper(&ptr_setter(ShellLink)))))
		return false;

	os::com::ptr<IPersistFile> PersistFile;
	if (FAILED(ShellLink->QueryInterface(IID_IPersistFile, IID_PPV_ARGS_Helper(&ptr_setter(PersistFile)))))
		return false;

	if (FAILED(PersistFile->Load(null_terminated(ShortcutPath).c_str(), STGM_READ)))
		return false;

	if (FAILED(ShellLink->Resolve(nullptr, SLR_UPDATE)))
		return false;

	wchar_t Path[MAX_PATH];
	WIN32_FIND_DATA Data;
	if (FAILED(ShellLink->GetPath(Path, MAX_PATH, &Data, SLGP_RAWPATH)))
		return false;

	FilePath = os::env::expand(Path);
	IsDirectory = (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	return true;
}

bool GetShellType(const string_view Ext, string& strType, const ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;
	strType.clear();

	if (IsWindowsVistaOrGreater())
	{
		os::com::ptr<IApplicationAssociationRegistration> AAR;
		if (SUCCEEDED(imports.SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, IID_PPV_ARGS_Helper(&ptr_setter(AAR)))))
		{
			os::com::memory<wchar_t*> Association;
			if (SUCCEEDED(AAR->QueryCurrentDefault(null_terminated(Ext).c_str(), aType, AL_EFFECTIVE, &ptr_setter(Association))))
			{
				bVistaType = true;
				strType = Association.get();
			}
		}
	}

	if (!bVistaType)
	{
		if (aType == AT_URLPROTOCOL)
		{
			strType = Ext;
			return true;
		}

		os::reg::key UserKey;
		string strFoundValue;

		if (aType == AT_FILEEXTENSION)
		{
			// Смотрим дефолтный обработчик расширения в HKEY_CURRENT_USER
			if ((UserKey = os::reg::key::open(os::reg::key::current_user, concat(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"sv, Ext), KEY_QUERY_VALUE)))
			{
				if (UserKey.get(L"ProgID"sv, strFoundValue) && IsProperProgID(strFoundValue))
				{
					strType = strFoundValue;
				}
			}
		}

		os::reg::key CRKey;
		// Смотрим дефолтный обработчик расширения в HKEY_CLASSES_ROOT
		if (strType.empty() && ((CRKey = os::reg::key::open(os::reg::key::classes_root, Ext, KEY_QUERY_VALUE))))
		{
			if (CRKey.get({}, strFoundValue) && IsProperProgID(strFoundValue))
			{
				strType = strFoundValue;
			}
		}

		if (strType.empty() && UserKey)
			strType = SearchExtHandlerFromList(UserKey);

		if (strType.empty() && CRKey)
			strType = SearchExtHandlerFromList(CRKey);
	}

	return !strType.empty();
}

void OpenFolderInShell(string_view const Folder)
{
	execute_info Info;
	Info.DisplayCommand = Folder;
	Info.Command = Folder;
	Info.NewWindow = true;
	Info.ExecMode = execute_info::exec_mode::direct;
	Info.SourceMode = execute_info::source_mode::known;
	Info.Silent = true;

	Execute(Info, true);
}

static bool GetAssociatedImageTypeForBatCmd(string_view const Str, image_type& ImageType)
{
	if (!IsExecutable(Str))
		return false;

	ImageType = image_type::console;
	return true;
}

static bool FindAndGetApplicationType(string_view const Application, image_type& ImageType)
{
	if (Application.empty())
		return false;

	string FoundApplication;
	if (!FindObject(Application, FoundApplication, nullptr))
		return false;

	return GetImageType(FoundApplication, ImageType) || GetAssociatedImageTypeForBatCmd(FoundApplication, ImageType);
}

static bool GetAssociatedImageType(string_view const FileName, image_type& ImageType, string& Action)
{
	if (GetAssociatedImageTypeForBatCmd(FileName, ImageType))
		return true;

	const auto Type = GetShellTypeFromExtension(FileName);
	if (IsShortcutType(Type))
	{
		string ShortcutPath;
		bool IsDirectory = false;
		if (ResolveShortcut(FileName, ShortcutPath, IsDirectory))
		{
			// Or shall we try to go there? :)
			if (IsDirectory)
			{
				ImageType = image_type::graphical;
				return true;
			}

			return GetImageType(ShortcutPath, ImageType) || GetAssociatedImageType(ShortcutPath, ImageType, Action);
		}
	}

	return FindAndGetApplicationType(GetAssociatedApplicationFromType(Type, Action), ImageType);
}

static bool GetProtocolType(string_view const Str, image_type& ImageType)
{
	const auto Unquoted = unquote(Str);
	const auto SemicolonPos = Unquoted.find(L':');
	if (!SemicolonPos || SemicolonPos == 1 || SemicolonPos == Str.npos)
		return false;

	const auto Protocol = string_view(Unquoted).substr(0, SemicolonPos);

	string Type;
	if (!GetShellType(Protocol, Type, AT_URLPROTOCOL))
		return false;

	string Action;
	if (FindAndGetApplicationType(GetAssociatedApplicationFromType(Type, Action), ImageType))
		return true;

	if (equal_icase(Type, L"file"sv))
	{
		// file protocol is handled by IE but cannot be detected using conventional approach
		ImageType = image_type::graphical;
		return true;
	}

	return false;
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

		console.FlushInputBuffer();
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

void Execute(execute_info& Info, bool FolderRun, function_ref<void(bool)> const ConsoleActivator)
{
	bool Result = false;
	string strNewCmdStr;
	string strNewCmdPar;

	// Info.NewWindow may be changed later
	const auto IgnoreInternalAssociations = Info.NewWindow || !Info.UseAssociations;

	const auto TryProtocolOrFallToComspec = [&]
	{
		auto ImageType = image_type::unknown;
		if (GetProtocolType(Info.Command, ImageType))
		{
			Info.ExecMode = execute_info::exec_mode::direct;

			if (ImageType == image_type::graphical)
			{
				Info.Silent = true;
				Info.NewWindow = true;
			}
		}
		else
		{
			Info.ExecMode = execute_info::exec_mode::external;
		}
		strNewCmdStr = Info.Command;
	};

	if (Info.SourceMode == execute_info::source_mode::known)
	{
		strNewCmdStr = Info.Command;
	}
	else if (!PartCmdLine(Info.Command, strNewCmdStr, strNewCmdPar))
	{
		// Complex expression (pipe or redirection): fallback to comspec as is
		TryProtocolOrFallToComspec();
	}

	bool IsDirectory = false;

	if(Info.RunAs)
	{
		Info.NewWindow = true;
	}

	if(FolderRun)
	{
		Info.Silent = true;
	}

	if (Info.NewWindow)
	{
		Info.Silent = true;

		const auto Unquoted = unquote(strNewCmdStr);
		IsDirectory = os::fs::is_directory(Unquoted);

		if (strNewCmdPar.empty() && IsDirectory)
		{
			strNewCmdStr = ConvertNameToFull(Unquoted);
			Info.ExecMode = execute_info::exec_mode::direct;
			Info.SourceMode = execute_info::source_mode::known;
			FolderRun=true;
		}
	}

	string Verb;

	if (FolderRun && Info.ExecMode == execute_info::exec_mode::direct)
	{
		AddEndSlash(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
	}
	else if (Info.ExecMode == execute_info::exec_mode::detect)
	{
		const auto GetImageTypeFallback = [](image_type& ImageType)
		{
			// Object is found, but its type is unknown.
			// Decision is controversial:

			// - we can say it's console to be ready for some output
			// ImageType = image_type::console;
			// return true;

			// - we can say it's graphical and launch it in a neat silent way
			ImageType = image_type::graphical;
			return true;

			// - we can give up and let comspec take it
			// return false;
		};

		const auto ModuleName = unquote(os::env::expand(strNewCmdStr));
		auto FoundModuleName = ModuleName;

		bool Internal = false;

		auto ImageType = image_type::unknown;

		if (Info.SourceMode == execute_info::source_mode::known || FindObject(ModuleName, FoundModuleName, &Internal))
		{
			if (Internal)
			{
				// Internal comspec command (one of ExcludeCmds): fallback to comspec as is
				Info.ExecMode = execute_info::exec_mode::external;
				strNewCmdStr = Info.Command;
			}
			else
			{
				static unsigned ProcessingAsssociation = 0;

				if (!IgnoreInternalAssociations && !ProcessingAsssociation)
				{
					++ProcessingAsssociation;
					SCOPE_EXIT{ --ProcessingAsssociation; };

					const auto FoundModuleNameShort = ConvertNameToShort(FoundModuleName);
					const auto LastX = WhereX(), LastY = WhereY();
					if (ProcessLocalFileTypes(FoundModuleName, FoundModuleNameShort, FILETYPE_EXEC, Info.WaitMode == execute_info::wait_mode::wait_finish, false, Info.RunAs, [&](execute_info& AssocInfo)
					{
						GotoXY(LastX, LastY);

						if (!strNewCmdPar.empty())
						{
							append(AssocInfo.Command, L' ', strNewCmdPar);
						}

						Global->CtrlObject->CmdLine()->ExecString(AssocInfo);
						//Execute(AssocInfo, FolderRun, Silent, ConsoleActivator);
					}))
					{
						return;
					}
					GotoXY(LastX, LastY);
				}

				if (GetImageType(FoundModuleName, ImageType) || GetProtocolType(FoundModuleName, ImageType) || GetAssociatedImageType(FoundModuleName, ImageType, Verb) || GetImageTypeFallback(ImageType))
				{
					// We can run it directly
					Info.ExecMode = execute_info::exec_mode::direct;
					strNewCmdStr = FoundModuleName;
					strNewCmdPar = os::env::expand(strNewCmdPar);

					if (ImageType == image_type::graphical)
					{
						Info.Silent = true;
						Info.NewWindow = true;
					}
				}
			}
		}
		else
		{
			// Found nothing: fallback to comspec as is
			TryProtocolOrFallToComspec();
		}
	}

	if (Info.WaitMode == execute_info::wait_mode::wait_finish)
	{
		// It's better to show console rather than non-responding panels
		Info.Silent = false;
	}

	bool Visible=false;
	DWORD CursorSize=0;
	rectangle ConsoleWindowRect;
	point ConsoleSize;
	int ConsoleCP = CP_OEMCP;
	int ConsoleOutputCP = CP_OEMCP;


	SHELLEXECUTEINFO seInfo={sizeof(seInfo)};

	const auto strCurDir = short_name_if_too_long(os::fs::GetCurrentDirectory());

	seInfo.lpDirectory = strCurDir.c_str();
	seInfo.nShow = SW_SHOWNORMAL;

	if (ConsoleActivator)
	{
		ConsoleActivator(!Info.Silent);
	}

	if(!Info.Silent)
	{
		ConsoleCP = console.GetInputCodepage();
		ConsoleOutputCP = console.GetOutputCodepage();
		FlushInputBuffer();

		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);

		console.GetWindowRect(ConsoleWindowRect);
		console.GetSize(ConsoleSize);

		if (Global->Opt->Exec.ExecuteFullTitle)
		{
			auto strFarTitle = strNewCmdStr;
			if (!strNewCmdPar.empty())
			{
				append(strFarTitle, L' ', strNewCmdPar);
			}
			ConsoleTitle::SetFarTitle(strFarTitle, true);
		}
	}

	// ShellExecuteEx Win8.1+ wrongly opens symlinks in a separate console window
	// Workaround: execute through %comspec%
	if (Info.ExecMode == execute_info::exec_mode::direct && !Info.NewWindow && IsWindows8Point1OrGreater())
	{
		os::fs::file_status fstatus(strNewCmdStr);
		if (os::fs::is_file(fstatus) && fstatus.check(FILE_ATTRIBUTE_REPARSE_POINT))
		{
			Info.ExecMode = execute_info::exec_mode::external;
		}
	}

	string strComspec, ComSpecParams;

	if (Info.ExecMode == execute_info::exec_mode::direct)
	{
		strNewCmdStr = short_name_if_too_long(strNewCmdStr);
		seInfo.lpFile = strNewCmdStr.c_str();

		if(!strNewCmdPar.empty())
		{
			seInfo.lpParameters = strNewCmdPar.c_str();
		}
	}
	else
	{
		strComspec = os::env::expand(Global->Opt->Exec.Comspec);
		if (strComspec.empty())
		{
			strComspec = os::env::get(L"COMSPEC"sv);
			if (strComspec.empty())
			{
				Message(MSG_WARNING,
					msg(lng::MError),
					{
						msg(lng::MComspecNotFound)
					},
					{ lng::MOk });
				return;
			}
		}

		try
		{
			ComSpecParams = format(os::env::expand(Global->Opt->Exec.ComspecArguments), Info.Command);
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
			return;
		}

		seInfo.lpFile = strComspec.c_str();
		seInfo.lpParameters = ComSpecParams.c_str();
	}

	if (Info.RunAs && RunAsSupported(seInfo.lpFile))
	{
		seInfo.lpVerb = L"runas";
	}

	seInfo.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS | (Info.NewWindow? 0 : SEE_MASK_NO_CONSOLE);

	os::handle Process;
	error_state ErrorState;

	{
		// ShellExecuteEx fails if IE10 is installed and if current directory is symlink/junction
		std::optional<os::fs::process_current_directory_guard> Guard;
		if (os::fs::file_status(strCurDir).check(FILE_ATTRIBUTE_REPARSE_POINT))
		{
			Guard.emplace(short_name_if_too_long(ConvertNameToReal(strCurDir)));
		}

		Result = ShellExecuteEx(&seInfo) != FALSE;

		if (Result)
		{
			Process.reset(seInfo.hProcess);
		}
		else
		{
			ErrorState = error_state::fetch();
		}
	}

	if (Result)
	{
		if (Process)
		{
			if (Info.WaitMode == execute_info::wait_mode::wait_finish || !Info.NewWindow)
			{
				if (const auto ConsoleDetachKey = KeyNameToKey(Global->Opt->ConsoleDetachKey))
				{
					wait_for_process_or_detach(Process, ConsoleDetachKey, ConsoleSize, ConsoleWindowRect);
				}
				else
				{
					Process.wait();
				}
			}
			if(Info.WaitMode == execute_info::wait_mode::wait_idle)
			{
				WaitForInputIdle(Process.native_handle(), INFINITE);
			}
			Process.close();
		}
	}

	SetFarConsoleMode(true);
	/* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
	    вследствие своей оптимизации, которая в данном случае выходит боком.
	*/
	SetCursorType(Visible, CursorSize);

	{
		// Could be changed by external program
		const auto CurrentTitle = Global->ScrBuf->GetTitle();
		// Invalidate cache:
		Global->ScrBuf->SetTitle({});
		Global->ScrBuf->SetTitle(CurrentTitle);
	}

	CONSOLE_CURSOR_INFO cci = { CursorSize, Visible };
	console.SetCursorInfo(cci);

	{
		point ConSize;
		if (console.GetSize(ConSize) && (ConSize.x != ScrX + 1 || ConSize.y != ScrY + 1))
		{
			ChangeVideoMode(ConSize.y, ConSize.x);
		}
	}

	if (Global->Opt->Exec.RestoreCPAfterExecute)
	{
		console.SetInputCodepage(ConsoleCP);
		console.SetOutputCodepage(ConsoleOutputCP);
	}

	if (!Result && ErrorState.Win32Error != ERROR_CANCELLED)
	{
		std::vector<string> Strings;
		if (Info.ExecMode == execute_info::exec_mode::direct)
			Strings = { msg(lng::MCannotExecute), strNewCmdStr };
		else
			Strings = { msg(lng::MCannotInvokeComspec), strComspec, msg(lng::MCheckComspecVar) };

		Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			std::move(Strings),
			{ lng::MOk },
			L"ErrCannotExecute"sv,
			nullptr,
			{ Info.ExecMode == execute_info::exec_mode::direct? strNewCmdStr : strComspec });
	}
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

bool IsExecutable(string_view const Filename)
{
	const auto Ext = name_ext(Filename).second;
	if (Ext.empty())
		return false;

	// these guys have specific association in Windows Registry: "%1" %*
	// That means we can't find the associated program etc., so they shall be hard-coded.
	static const string_view Executables[] = { L"exe"sv, L"cmd"sv, L"com"sv, L"bat"sv };
	const auto ExtWithoutDot = Ext.substr(1);
	return std::any_of(ALL_CONST_RANGE(Executables), [&](const string_view Extension)
	{
		return equal_icase(Extension, ExtWithoutDot);
	});
}

bool ExpandOSAliases(string& strStr)
{
	string strNewCmdStr;
	string strNewCmdPar;

	if (!PartCmdLine(strStr, strNewCmdStr, strNewCmdPar))
		return false;

	auto ExeName = PointToName(Global->g_strFarModuleName);
	const wchar_t_ptr Buffer(4096);
	int ret = console.GetAlias(strNewCmdStr, Buffer.data(), Buffer.size() * sizeof(wchar_t), ExeName);

	if (!ret)
	{
		const auto strComspec(os::env::get(L"COMSPEC"sv));
		if (!strComspec.empty())
		{
			ExeName=PointToName(strComspec);
			ret = console.GetAlias(strNewCmdStr, Buffer.data(), Buffer.size() * sizeof(wchar_t), ExeName);
		}
	}

	if (!ret)
		return false;

	strNewCmdStr.assign(Buffer.data());

	if (!ReplaceStrings(strNewCmdStr, L"$*"sv, strNewCmdPar) && !strNewCmdPar.empty())
		append(strNewCmdStr, L' ', strNewCmdPar);

	strStr=strNewCmdStr;

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
