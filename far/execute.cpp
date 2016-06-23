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

#include "headers.hpp"
#pragma hdrstop

#include "execute.hpp"
#include "keyboard.hpp"
#include "ctrlobj.hpp"
#include "chgprior.hpp"
#include "cmdline.hpp"
#include "imports.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "constitle.hpp"
#include "console.hpp"
#include "language.hpp"
#include "filetype.hpp"

enum class image_type
{
	unknown,
	console,
	graphical,
};

static bool GetImageType(const string& FileName, image_type& ImageType)
{
	auto Result = image_type::unknown;
	os::fs::file ModuleFile;
	if(ModuleFile.Open(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
	{
		IMAGE_DOS_HEADER DOSHeader;
		size_t ReadSize;

		if (ModuleFile.Read(&DOSHeader, sizeof(DOSHeader), ReadSize) && ReadSize==sizeof(DOSHeader))
		{
			if (DOSHeader.e_magic==IMAGE_DOS_SIGNATURE)
			{
				Result = image_type::console;

				if (ModuleFile.SetPointer(DOSHeader.e_lfanew,nullptr,FILE_BEGIN))
				{
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

					if (ModuleFile.Read(&ImageHeader, sizeof(ImageHeader), ReadSize) && ReadSize==sizeof(ImageHeader))
					{
						if (ImageHeader.PeHeader.Signature == IMAGE_NT_SIGNATURE)
						{
							const auto& PeHeader = ImageHeader.PeHeader;

							if (!(PeHeader.FileHeader.Characteristics & IMAGE_FILE_DLL))
							{
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
						}
						else if (ImageHeader.Os2Header.ne_magic == IMAGE_OS2_SIGNATURE)
						{
							const auto& Os2Header = ImageHeader.Os2Header;

							if (!(HIBYTE(Os2Header.ne_flags) & 0x80)) // DLL or driver
							{
								enum
								{
									NE_WINDOWS = 0x2,
									NE_WIN386 = 0x4,
								};

								if (Os2Header.ne_exetyp == NE_WINDOWS || Os2Header.ne_exetyp == NE_WIN386)
								{
									Result = image_type::graphical;
								}
							}
						}
					}
				}
			}
		}
	}

	if (Result != image_type::unknown)
	{
		ImageType = Result;
		return true;
	}
	return false;
}

static bool IsProperProgID(const string& ProgID)
{
	return !ProgID.empty() && os::reg::open_key(HKEY_CLASSES_ROOT, ProgID.data(), KEY_QUERY_VALUE);
}

/*
Ищем валидный ProgID для файлового расширения по списку возможных
hExtKey - корневой ключ для поиска (ключ расширения)
strType - сюда запишется результат, если будет найден
*/
static bool SearchExtHandlerFromList(const os::reg::key& hExtKey, string &strType)
{
	for (const auto& i: os::reg::enum_value(hExtKey.get(), L"OpenWithProgids", KEY_ENUMERATE_SUB_KEYS))
	{
		if (i.Type() == REG_SZ && IsProperProgID(i.Name()))
		{
			strType = i.Name();
			return true;
		}
	}
	return false;
}

static bool FindObject(const string& Module, string &strDest, bool &Internal)
{
	bool Result=false;

	if (!Module.empty())
	{
		// нулевой проход - смотрим исключения
		// Берем "исключения" из реестра, которые должны исполняться директом,
		// например, некоторые внутренние команды ком. процессора.
		const auto ExcludeCmdsList = split<std::vector<string>>(os::env::expand_strings(Global->Opt->Exec.strExcludeCmds), STLF_UNIQUE);

		if (std::any_of(CONST_RANGE(ExcludeCmdsList, i) { return !StrCmpI(i, Module); }))
		{
			Result=true;
			Internal = true;
		}

		if (!Result)
		{
			auto strFullName = Module;
			const auto ModuleExt = wcsrchr(PointToName(Module), L'.');
			auto strPathExt = os::env::get_pathext();
			if (Global->Opt->UseRegisteredTypes)
			{
				// ";;" to also try no extension if nothing else matches
				strPathExt += L";;";
			}
			const auto PathExtList = split<std::vector<string>>(strPathExt, STLF_UNIQUE | STLF_ALLOWEMPTY);

			for (const auto& i: PathExtList) // первый проход - в текущем каталоге
			{
				string strTmpName=strFullName;

				if (!ModuleExt)
				{
					strTmpName += i;
				}

				if (os::fs::is_file(strTmpName))
				{
					ConvertNameToFull(strTmpName,strFullName);
					Result=true;
					break;
				}

				if (ModuleExt)
				{
					break;
				}
			}

			if (!Result) // второй проход - по правилам SearchPath
			{
				const auto strPathEnv(os::env::get_variable(L"PATH"));
				if (!strPathEnv.empty())
				{
					for (const auto& Path: split<std::vector<string>>(strPathEnv, STLF_UNIQUE))
					{
						for (const auto& Ext: PathExtList)
						{
							string Dest;

							if (os::SearchPath(Path.data(), strFullName, Ext.data(), Dest))
							{
								if (os::fs::is_file(Dest))
								{
									strFullName=Dest;
									Result=true;
									break;
								}
							}
						}
						if(Result)
							break;
					}
				}

				if (!Result)
				{
					for (const auto& Ext: PathExtList)
					{
						string Dest;

						if (os::SearchPath(nullptr, strFullName, Ext.data(), Dest))
						{
							if (os::fs::is_file(Dest))
							{
								strFullName=Dest;
								Result=true;
								break;
							}
						}
					}
				}

				// третий проход - лезем в реестр в "App Paths"
				if (!Result && Global->Opt->Exec.ExecuteUseAppPath && strFullName.find(L'\\') == string::npos)
				{
					static const WCHAR RegPath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
					// В строке Module заменить исполняемый модуль на полный путь, который
					// берется из SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
					// Сначала смотрим в HKCU, затем - в HKLM
					static const HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};
					strFullName=RegPath;
					strFullName+=Module;

					DWORD samDesired = KEY_QUERY_VALUE;

					for (size_t i=0; i<std::size(RootFindKey); i++)
					{
						if (i==std::size(RootFindKey)-1)
						{
							if (const auto RedirectionFlag = os::GetAppPathsRedirectionFlag())
							{
								samDesired|=RedirectionFlag;
							}
							else
							{
								break;
							}
						}

						if (os::reg::GetValue(RootFindKey[i], strFullName, L"", strFullName, samDesired))
						{
							strFullName = Unquote(os::env::expand_strings(strFullName));
							Result=true;
							break;
						}
					}

					if (!Result)
					{
						Result = std::any_of(CONST_RANGE(PathExtList, Ext)
						{
							strFullName=RegPath;
							strFullName+=Module;
							strFullName+=Ext;

							return std::any_of(CONST_RANGE(RootFindKey, i)
							{
								if (os::reg::GetValue(i, strFullName, L"", strFullName))
								{
									strFullName = Unquote(os::env::expand_strings(strFullName));
									return true;
								}
								return false;
							});
						});
					}
				}
			}

			if (Result)
			{
				strDest = strFullName;
			}
		}
	}

	return Result;
}

/*
 true: ok, found command & arguments.
 false: it's too complex, let's comspec deal with it.
*/
static bool PartCmdLine(const string& CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
	auto UseDefaultCondition = true;

	// Custom comspec condition logic, gives the user ability to provide his own rules in form of regular expression, for example ^(?:[^"]|"[^"]*")*?[<>|&]

	// Do not use std::regex here.
	// VC implementation has limited complexity and throws regex_error on long strings.
	// gcc implementation is total rubbish - it just causes a stack overflow. Shame on them.

	// If anything goes wrong, e. g. pattern is incorrect or search failed - default condition (checking for presence of <>|& characters outside the quotes) will be used.
	const auto Condition = os::env::expand_strings(Global->Opt->Exec.ComspecCondition);
	if (!Condition.empty())
	{
		auto& Re = Global->Opt->Exec.ComspecConditionRe;

		if (Re.Pattern != Condition)
		{
			Re.Re = std::make_unique<RegExp>();
			if (!Re.Re->Compile(Condition.data(), OP_OPTIMIZE))
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

	const auto Begin = CmdStr.cbegin() + CmdStr.find_first_not_of(L' ');
	const auto End = CmdStr.cend();
	auto CmdEnd = End;
	auto ParamsBegin = End;
	auto InQuotes = false;

	for (auto i = Begin; i != End; ++i)
	{
		if (*i == L'"')
		{
			InQuotes = !InQuotes;
		}

		if (!InQuotes && UseDefaultCondition && wcschr(L"<>|&", *i))
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

static bool RunAsSupported(LPCWSTR Name)
{
	string Extension(PointToExt(Name)), Type;
	return !Extension.empty() && GetShellType(Extension, Type) && os::reg::open_key(HKEY_CLASSES_ROOT, Type.append(L"\\shell\\runas\\command").data(), KEY_QUERY_VALUE);
}

// TODO: rewrite
static const wchar_t* GetShellActionAndAssociatedApplicationImpl(const string& FileName, string& strAction, string& Application, DWORD& Error)
{
	string strValue;
	string strNewValue;
	const wchar_t *RetPtr;
	const wchar_t command_action[]=L"\\command";
	Error = ERROR_SUCCESS;

	const auto ExtPtr = wcsrchr(FileName.data(), L'.');
	if (!ExtPtr)
		return nullptr;

	if (!GetShellType(ExtPtr, strValue))
		return nullptr;

	if (const auto Key = os::reg::open_key(HKEY_CLASSES_ROOT, strValue.data(), KEY_QUERY_VALUE))
	{
		if (os::reg::GetValue(Key, L"IsShortcut"))
			return nullptr;
	}

	strValue += L"\\shell";

	const auto Key = os::reg::open_key(HKEY_CLASSES_ROOT, strValue.data(), KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
	if (!Key)
		return nullptr;

	strValue += L"\\";

	if (os::reg::GetValue(Key, L"", strAction))
	{
		RetPtr = EmptyToNull(strAction.data());
		LONG RetEnum = ERROR_SUCCESS;
		const auto ActionList = split<std::vector<string>>(strAction, STLF_UNIQUE);

		if (RetPtr && !ActionList.empty())
		{
			for (const auto& i: ActionList)
			{
				strNewValue = strValue;
				strNewValue += i;
				strNewValue += command_action;

				if (os::reg::open_key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
				{
					strValue += i;
					strAction = i;
					RetPtr = strAction.data();
					RetEnum = ERROR_NO_MORE_ITEMS;
				}
				if (RetEnum != ERROR_SUCCESS)
					break;
			}
		}
		else
		{
			strValue += strAction;
		}

		if (RetEnum != ERROR_NO_MORE_ITEMS) // Если ничего не нашли, то...
			RetPtr=nullptr;
	}
	else
	{
		// This member defaults to "Open" if no verb is specified.
		// Т.е. если мы вернули nullptr, то подразумевается команда "Open"
		RetPtr=nullptr;
	}

	// Если RetPtr==nullptr - мы не нашли default action.
	// Посмотрим - есть ли вообще что-нибудь у этого расширения
	if (!RetPtr)
	{
		// Сначала проверим "open"...
		strAction = L"open";
		strNewValue = strValue;
		strNewValue += strAction;
		strNewValue += command_action;

		if (os::reg::open_key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
		{
			strValue += strAction;
			RetPtr = strAction.data();
		}
		else
		{
			// ... а теперь все остальное, если "open" нету
			for (const auto& i: os::reg::enum_key(Key))
			{
				strAction = i;

				// Проверим наличие "команды" у этого ключа
				strNewValue = strValue;
				strNewValue += strAction;
				strNewValue += command_action;

				if (os::reg::open_key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
				{
					strValue += strAction;
					RetPtr = strAction.data();
					break;
				}
			}
		}
	}

	if (RetPtr)
	{
		strValue += command_action;

		if (os::reg::GetValue(HKEY_CLASSES_ROOT, strValue, L"", strNewValue) && !strNewValue.empty())
		{
			strNewValue = os::env::expand_strings(strNewValue);

			// Выделяем имя модуля
			if (strNewValue.front() == L'\"')
			{
				size_t QuotePos = strNewValue.find(L'\"', 1);

				if (QuotePos != string::npos)
				{
					strNewValue.resize(QuotePos);
					strNewValue.erase(0, 1);
				}
			}
			else
			{
				const auto pos = strNewValue.find_first_of(L" \t/");
				if (pos != string::npos)
					strNewValue.resize(pos);
			}

			Application = strNewValue;
		}
		else
		{
			Error=ERROR_NO_ASSOCIATION;
			RetPtr=nullptr;
		}
	}

	return RetPtr;
}

string GetShellActionAndAssociatedApplication(const string& FileName, string& Application, DWORD& Error)
{
	string Action;
	return NullToEmpty(GetShellActionAndAssociatedApplicationImpl(FileName, Action, Application, Error));
}

bool GetShellType(const string& Ext, string &strType,ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;
	strType.clear();

	if (IsWindowsVistaOrGreater())
	{
		IApplicationAssociationRegistration* pAAR;
		HRESULT hr = Imports().SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (void**)&pAAR);

		if (SUCCEEDED(hr))
		{
			wchar_t *p;

			if (pAAR->QueryCurrentDefault(Ext.data(), aType, AL_EFFECTIVE, &p) == S_OK)
			{
				bVistaType = true;
				strType = p;
				CoTaskMemFree(p);
			}

			pAAR->Release();
		}
	}

	if (!bVistaType)
	{
		if (aType == AT_URLPROTOCOL)
		{
			strType = Ext;
			return true;
		}

		os::reg::key hCRKey, hUserKey;
		string strFoundValue;

		if (aType == AT_FILEEXTENSION)
		{
			// Смотрим дефолтный обработчик расширения в HKEY_CURRENT_USER
			if ((hUserKey = os::reg::open_key(HKEY_CURRENT_USER, (L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + Ext).data(), KEY_QUERY_VALUE)))
			{
				if (os::reg::GetValue(hUserKey, L"ProgID", strFoundValue) && IsProperProgID(strFoundValue))
				{
					strType = strFoundValue;
				}
			}
		}

		// Смотрим дефолтный обработчик расширения в HKEY_CLASSES_ROOT
		if (strType.empty() && (hCRKey = os::reg::open_key(HKEY_CLASSES_ROOT, Ext.data(), KEY_QUERY_VALUE)) != nullptr)
		{
			if (os::reg::GetValue(hCRKey, L"", strFoundValue) && IsProperProgID(strFoundValue))
			{
				strType = strFoundValue;
			}
		}

		if (strType.empty() && hUserKey)
			SearchExtHandlerFromList(hUserKey, strType);

		if (strType.empty() && hCRKey)
			SearchExtHandlerFromList(hCRKey, strType);
	}

	return !strType.empty();
}

void OpenFolderInShell(const string& Folder)
{
	execute_info Info;
	Info.Command = Folder;
	Info.NewWindow = true;
	Info.ExecMode = execute_info::exec_mode::direct;
	Info.SourceMode = execute_info::source_mode::known;

	Execute(Info, true, true);
}

void Execute(execute_info& Info, bool FolderRun, bool Silent, const std::function<void()>& ConsoleActivator)
{
	bool Result = false;
	string strNewCmdStr;
	string strNewCmdPar;

	if (Info.SourceMode == execute_info::source_mode::known)
	{
		strNewCmdStr = Info.Command;
	}
	else if (!PartCmdLine(Info.Command, strNewCmdStr, strNewCmdPar))
	{
		// Complex expression (pipe or redirection): fallback to comspec as is
		strNewCmdStr = Info.Command;
		Info.ExecMode = execute_info::exec_mode::external;
	}

	bool IsDirectory = false;

	if(Info.RunAs)
	{
		Info.NewWindow = true;
	}

	if(FolderRun)
	{
		Silent = true;
	}

	if (Info.NewWindow)
	{
		Silent = true;

		auto Unquoted = strNewCmdStr;
		Unquote(Unquoted);
		IsDirectory = os::fs::is_directory(Unquoted);

		if (strNewCmdPar.empty() && IsDirectory)
		{
			strNewCmdStr = Unquoted;
			ConvertNameToFull(strNewCmdStr, strNewCmdStr);
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
		auto GetAssociatedImageType = [&Verb](const string& Str, image_type& ImageType)
		{
			auto GetAssociatedImageTypeForBatCmd = [](const string& BatCmdStr, image_type& BatCmdImageType)
			{
				if (IsExecutable(BatCmdStr))
				{
					// We shouldn't get here if it's a PE image - only if bat / cmd
					BatCmdImageType = image_type::console;
					return true;
				}
				return false;
			};

			if (GetAssociatedImageTypeForBatCmd(Str, ImageType))
			{
				return true;
			}
			else
			{
				DWORD SaError = 0;
				string Application;
				Verb = GetShellActionAndAssociatedApplication(Str, Application, SaError);

				if (!Verb.empty() && !Application.empty() && SaError != ERROR_NO_ASSOCIATION)
				{
					return GetImageType(Application, ImageType) || GetAssociatedImageTypeForBatCmd(Application, ImageType);
				}
			}
			return false;
		};

		auto GetImageTypeFallback = [](image_type& ImageType)
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

		const auto ModuleName = Unquote(os::env::expand_strings(strNewCmdStr));
		auto FoundModuleName = ModuleName;

		bool Internal = false;

		auto ImageType = image_type::unknown;

		if (Info.SourceMode == execute_info::source_mode::known || FindObject(ModuleName, FoundModuleName, Internal))
		{
			if (Internal)
			{
				// Internal comspec command (one of ExcludeCmds): fallback to comspec as is
				Info.ExecMode = execute_info::exec_mode::external;
				strNewCmdStr = Info.Command;
			}
			else
			{
				auto FoundModuleNameShort = FoundModuleName;
				ConvertNameToShort(FoundModuleNameShort, FoundModuleNameShort);
				const auto LastX = WhereX(), LastY = WhereY();
				if (ProcessLocalFileTypes(FoundModuleName, FoundModuleNameShort, FILETYPE_EXEC, Info.WaitMode == execute_info::wait_mode::wait_finish, false, [&](execute_info& AssocInfo)
				{
					GotoXY(LastX, LastY);
					if (!strNewCmdPar.empty())
					{
						AssocInfo.Command.append(L" ").append(strNewCmdPar);
					}
					Execute(AssocInfo, FolderRun, Silent, ConsoleActivator);
				}))
				{
					return;
				}
				GotoXY(LastX, LastY);

				if (GetImageType(FoundModuleName, ImageType) || GetAssociatedImageType(FoundModuleName, ImageType) || GetImageTypeFallback(ImageType))
				{
					// We can run it directly
					Info.ExecMode = execute_info::exec_mode::direct;
					strNewCmdStr = FoundModuleName;
					strNewCmdPar = os::env::expand_strings(strNewCmdPar);

					if (ImageType == image_type::graphical)
					{
						Silent = true;
						Info.NewWindow = true;
					}
				}
			}
		}
		else
		{
			// Found nothing: fallback to comspec as is
			Info.ExecMode = execute_info::exec_mode::external;
			strNewCmdStr = Info.Command;
		}
	}

	if (Info.WaitMode == execute_info::wait_mode::wait_finish)
	{
		// It's better to show console rather than non-responding panels
		Silent = false;
	}

	bool Visible=false;
	DWORD CursorSize=0;
	SMALL_RECT ConsoleWindowRect;
	COORD ConsoleSize={};
	int ConsoleCP = CP_OEMCP;
	int ConsoleOutputCP = CP_OEMCP;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	SHELLEXECUTEINFO seInfo={sizeof(seInfo)};
	const auto strCurDir = os::GetCurrentDirectory();
	seInfo.lpDirectory=strCurDir.data();
	seInfo.nShow = SW_SHOWNORMAL;

	if(!Silent)
	{
		ConsoleActivator();

		ConsoleCP = Console().GetInputCodepage();
		ConsoleOutputCP = Console().GetOutputCodepage();
		FlushInputBuffer();
		ChangeConsoleMode(InitialConsoleMode);
		Console().GetWindowRect(ConsoleWindowRect);
		Console().GetSize(ConsoleSize);

		if (Global->Opt->Exec.ExecuteFullTitle)
		{
			auto strFarTitle = strNewCmdStr;
			if (!strNewCmdPar.empty())
			{
				strFarTitle.append(L" ").append(strNewCmdPar);
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
		seInfo.lpFile = strNewCmdStr.data();
		if(!strNewCmdPar.empty())
		{
			seInfo.lpParameters = strNewCmdPar.data();
		}

		auto GetVerb = [&Verb](const string& Str)
		{
			DWORD DummyError;
			string DummyString;
			return GetShellActionAndAssociatedApplication(Str, DummyString, DummyError);
		};

		seInfo.lpVerb = IsDirectory? nullptr : EmptyToNull((Verb.empty()? Verb = GetVerb(strNewCmdStr) : Verb).data());
	}
	else
	{
		strComspec = os::env::expand_strings(Global->Opt->Exec.Comspec);
		if (strComspec.empty())
		{
			strComspec = os::env::get_variable(L"COMSPEC");
			if (strComspec.empty())
			{
				Message(MSG_WARNING, 1, MSG(MError), MSG(MComspecNotFound), MSG(MOk));
				return;
			}
		}

		ComSpecParams = os::env::expand_strings(Global->Opt->Exec.ComspecArguments);
		ReplaceStrings(ComSpecParams, L"{0}", Info.Command);

		seInfo.lpFile = strComspec.data();
		seInfo.lpParameters = ComSpecParams.data();
		seInfo.lpVerb = nullptr;
	}

	if (Info.RunAs && RunAsSupported(seInfo.lpFile))
	{
		seInfo.lpVerb = L"runas";
	}

	seInfo.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS | (Info.NewWindow? 0 : SEE_MASK_NO_CONSOLE);

	// ShellExecuteEx fails if IE10 is installed and if current directory is symlink/junction
	wchar_t CurDir[MAX_PATH];
	bool NeedFixCurDir = os::fs::file_status(strCurDir).check(FILE_ATTRIBUTE_REPARSE_POINT);
	if (NeedFixCurDir)
	{
		if (!GetCurrentDirectory(static_cast<DWORD>(std::size(CurDir)), CurDir))
		{
			NeedFixCurDir = false;
		}
		else
		{
			string RealPath;
			ConvertNameToReal(strCurDir, RealPath);
			NeedFixCurDir = SetCurrentDirectory(RealPath.data()) != FALSE;
		}
	}

	Result = ShellExecuteEx(&seInfo) != FALSE;

	os::handle Process;

	if (Result)
	{
		Process.reset(seInfo.hProcess);
	}
	else
	{
		Global->CatchError();
	}

	// ShellExecuteEx fails if IE10 is installed and if current directory is symlink/junction
	if (NeedFixCurDir)
	{
		SetCurrentDirectory(CurDir);
	}

	if (Result)
	{
		if (Process)
		{
			if (Info.WaitMode == execute_info::wait_mode::wait_finish || !Info.NewWindow)
			{
				if (Global->Opt->ConsoleDetachKey.empty())
				{
					Process.wait();
				}
				else
				{
					/*$ 12.02.2001 SKV
					  супер фитча ;)
					  Отделение фаровской консоли от неинтерактивного процесса.
					  Задаётся кнопкой в System/ConsoleDetachKey
					*/
					HANDLE hOutput = Console().GetOutputHandle();
					HANDLE hInput = Console().GetInputHandle();
					INPUT_RECORD ir[256];
					size_t rd;
					int vkey=0,ctrl=0;
					TranslateKeyToVK(KeyNameToKey(Global->Opt->ConsoleDetachKey),vkey,ctrl,nullptr);
					int alt=ctrl&(PKF_ALT|PKF_RALT);
					int shift=ctrl&PKF_SHIFT;
					ctrl=ctrl&(PKF_CONTROL|PKF_RCONTROL);
					bool bAlt, bShift, bCtrl;
					DWORD dwControlKeyState;

					//Тут нельзя делать WaitForMultipleObjects из за бага в Win7 при работе в телнет
					while (!Process.wait(100))
					{
						if (WaitForSingleObject(hInput, 100)==WAIT_OBJECT_0 && Console().PeekInput(ir, 256, rd) && rd)
						{
							int stop=0;

							for (DWORD i=0; i<rd; i++)
							{
								PINPUT_RECORD pir=&ir[i];

								if (pir->EventType==KEY_EVENT)
								{
									dwControlKeyState = pir->Event.KeyEvent.dwControlKeyState;
									bAlt = (dwControlKeyState & LEFT_ALT_PRESSED) || (dwControlKeyState & RIGHT_ALT_PRESSED);
									bCtrl = (dwControlKeyState & LEFT_CTRL_PRESSED) || (dwControlKeyState & RIGHT_CTRL_PRESSED);
									bShift = (dwControlKeyState & SHIFT_PRESSED)!=0;

									if (vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
									        (alt ?bAlt:!bAlt) &&
									        (ctrl ?bCtrl:!bCtrl) &&
									        (shift ?bShift:!bShift))
									{
										auto Aliases = Console().GetAllAliases();

										ConsoleIcons().restorePreviousIcons();

										Console().ReadInput(ir, 256, rd);
										/*
										  Не будем вызывать CloseConsole, потому, что она поменяет
										  ConsoleMode на тот, что был до запуска Far'а,
										  чего работающее приложение могло и не ожидать.
										*/
										CloseHandle(hInput);
										CloseHandle(hOutput);
										KeyQueue().clear();
										Console().Free();
										Console().Allocate();

										HWND hWnd = Console().GetWindow();
										if (hWnd)   // если окно имело HOTKEY, то старое должно его забыть.
											SendMessage(hWnd, WM_SETHOTKEY, 0, 0);

										Console().SetSize(ConsoleSize);
										Console().SetWindowRect(ConsoleWindowRect);
										Console().SetSize(ConsoleSize);
										Sleep(100);
										InitConsole(0);

										ConsoleIcons().setFarIcons();
										Console().SetAllAliases(Aliases);
										stop=1;
										break;
									}
								}
							}

							if (stop)
								break;
						}
					}
				}
			}
			if(Info.WaitMode == execute_info::wait_mode::wait_idle)
			{
				WaitForInputIdle(Process.native_handle(), INFINITE);
			}
			Process.close();
		}

		Result = true;

	}

	SetFarConsoleMode(TRUE);
	/* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
	    вследствие своей оптимизации, которая в данном случае выходит боком.
	*/
	SetCursorType(Visible, CursorSize);
	CONSOLE_CURSOR_INFO cci = { CursorSize, Visible };
	Console().SetCursorInfo(cci);

	if (IsConsoleSizeChanged())
	{
		ChangeVideoMode(ScrY, ScrX);
	}

	if (Global->Opt->Exec.RestoreCPAfterExecute)
	{
		Console().SetInputCodepage(ConsoleCP);
		Console().SetOutputCodepage(ConsoleOutputCP);
	}

	if (!Result)
	{
		std::vector<string> Strings;
		if (Info.ExecMode == execute_info::exec_mode::direct)
			Strings = { MSG(MCannotExecute), strNewCmdStr };
		else
			Strings = { MSG(MCannotInvokeComspec), strComspec, MSG(MCheckComspecVar) };

		Message(MSG_WARNING | MSG_ERRORTYPE,
			MSG(MError),
			Strings,
			{ MSG(MOk) },
			L"ErrCannotExecute",
			nullptr,
			nullptr,
			{ Info.ExecMode == execute_info::exec_mode::direct? strNewCmdStr : strComspec });
	}
}


/* $ 14.01.2001 SVS
   + В ProcessOSCommands добавлена обработка
     "IF [NOT] EXIST filename command"
     "IF [NOT] DEFINED variable command"

   Эта функция предназначена для обработки вложенного IF`а
   CmdLine - полная строка вида
     if exist file if exist file2 command
   Return - указатель на "command"
            пуская строка - условие не выполнимо
            nullptr - не попался "IF" или ошибки в предложении, например
                   не exist, а exist или предложение неполно.

   DEFINED - подобно EXIST, но оперирует с переменными среды

   Исходная строка (CmdLine) не модифицируется!!! - на что явно указывает const
                                                    IS 20.03.2002 :-)
*/

static const wchar_t *PrepareOSIfExist(const string& CmdLine)
{
	if (CmdLine.empty())
		return nullptr;

	string strCmd;
	string strExpandedStr;
	const wchar_t *PtrCmd=CmdLine.data(), *CmdStart;
	int Not=FALSE;
	int Exist=0; // признак наличия конструкции "IF [NOT] EXIST filename command"
	// > 0 - есть такая конструкция

	/* $ 25.04.2001 DJ
	   обработка @ в IF EXIST
	*/
	if (*PtrCmd == L'@')
	{
		// здесь @ игнорируется; ее вставит в правильное место функция
		// ExtractIfExistCommand
		PtrCmd++;

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;
	}

	constexpr auto Token_If = L"IF "_sl;
	constexpr auto Token_Not = L"NOT "_sl;
	constexpr auto Token_Exist = L"EXIST "_sl;
	constexpr auto Token_Defined = L"DEFINED "_sl;

	for (;;)
	{
		if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd, Token_If.data(), Token_If.size())) //??? IF/I не обрабатывается
			break;

		PtrCmd+=3;

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;

		if (!*PtrCmd)
			break;

		if (!StrCmpNI(PtrCmd, Token_Not.data(), Token_Not.size()))
		{
			Not=TRUE;

			PtrCmd+=4;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;
		}

		if (*PtrCmd && !StrCmpNI(PtrCmd, Token_Exist.data(), Token_Exist.size()))
		{

			PtrCmd+=6;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;

			CmdStart=PtrCmd;
			/* $ 25.04.01 DJ
			   обработка кавычек внутри имени файла в IF EXIST
			*/
			BOOL InQuotes=FALSE;

			while (*PtrCmd)
			{
				if (*PtrCmd == L'\"')
					InQuotes = !InQuotes;
				else if (*PtrCmd == L' ' && !InQuotes)
					break;

				PtrCmd++;
			}

			if (PtrCmd && *PtrCmd && *PtrCmd == L' ')
			{
//_SVS(SysLog(L"Cmd='%s'", strCmd.data()));
				strExpandedStr = os::env::expand_strings(Unquote(strCmd.assign(CmdStart, PtrCmd - CmdStart)));
				string strFullPath;

				if (!(strCmd[1] == L':' || (strCmd[0] == L'\\' && strCmd[1]==L'\\') || strExpandedStr[1] == L':' || (strExpandedStr[0] == L'\\' && strExpandedStr[1]==L'\\')))
				{
					strFullPath = Global->CtrlObject? Global->CtrlObject->CmdLine()->GetCurDir() : os::GetCurrentDirectory();
					AddEndSlash(strFullPath);
				}

				strFullPath += strExpandedStr;
				bool FileExists = false;

				size_t DirOffset = 0;
				ParsePath(strExpandedStr, &DirOffset);
				if (strExpandedStr.find_first_of(L"*?", DirOffset) != string::npos) // это маска?
				{
					os::FAR_FIND_DATA wfd;
					FileExists = os::GetFindDataEx(strFullPath, wfd);
				}
				else
				{
					ConvertNameToFull(strFullPath, strFullPath);
					FileExists = os::fs::exists(strFullPath);
				}

//_SVS(SysLog(L"%08X FullPath=%s",FileAttr,FullPath));
				if ((FileExists && !Not) || (!FileExists && Not))
				{
					while (*PtrCmd && IsSpace(*PtrCmd))
						++PtrCmd;

					Exist++;
				}
				else
				{
					return L"";
				}
			}
		}
		// "IF [NOT] DEFINED variable command"
		else if (*PtrCmd && !StrCmpNI(PtrCmd, Token_Defined.data(), Token_Defined.size()))
		{

			PtrCmd+=8;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;

			CmdStart=PtrCmd;

			if (*PtrCmd == L'"')
				PtrCmd=wcschr(PtrCmd+1,L'"');

			if (PtrCmd && *PtrCmd)
			{
				PtrCmd=wcschr(PtrCmd,L' ');

				if (PtrCmd && *PtrCmd && *PtrCmd == L' ')
				{
					strCmd.assign(CmdStart,PtrCmd-CmdStart);

					const auto ERet = os::env::get_variable(strCmd, strExpandedStr);

//_SVS(SysLog(Cmd));
					if ((ERet && !Not) || (!ERet && Not))
					{
						while (*PtrCmd && IsSpace(*PtrCmd))
							++PtrCmd;

						Exist++;
					}
					else
					{
						return L"";
					}
				}
			}
		}
	}

	return Exist?PtrCmd:nullptr;
}

/* $ 25.04.2001 DJ
обработка @ в IF EXIST: функция, которая извлекает команду из строки
с IF EXIST с учетом @ и возвращает TRUE, если условие IF EXIST
выполено, и FALSE в противном случае/
*/

bool ExtractIfExistCommand(string &strCommandText)
{
	bool Result = true;
	const wchar_t *wPtrCmd = PrepareOSIfExist(strCommandText);

	// Во! Условие не выполнено!!!
	// (например, пока рассматривали менюху, в это время)
	// какой-то злобный чебурашка стер файл!
	if (wPtrCmd)
	{
		if (!*wPtrCmd)
		{
			Result = false;
		}
		else
		{
			// прокинем "if exist"
			if (strCommandText.front() == L'@')
			{
				strCommandText.resize(1);
				strCommandText += wPtrCmd;
			}
			else
			{
				strCommandText = wPtrCmd;
			}
		}
	}

	return Result;
}

bool IsExecutable(const string& Filename)
{
	auto DotPos = Filename.find_last_of('.');
	if (DotPos == string::npos || DotPos == Filename.size() - 1)
		return false;

	auto Extension = Lower(Filename.substr(DotPos + 1));

	// these guys have specific association in Windows Registry: "%1" %*
	// That means we can't find the associated program etc., so they shall be hard-coded.
	return Extension == L"exe" || Extension == L"com" || Extension == L"bat" || Extension == L"cmd";
}

bool ExpandOSAliases(string &strStr)
{
	string strNewCmdStr;
	string strNewCmdPar;

	if (!PartCmdLine(strStr, strNewCmdStr, strNewCmdPar))
		return false;

	const wchar_t* ExeName=PointToName(Global->g_strFarModuleName);
	wchar_t_ptr Buffer(4096);
	int ret = Console().GetAlias(strNewCmdStr.data(), Buffer.get(), Buffer.size() * sizeof(wchar_t), ExeName);

	if (!ret)
	{
		const auto strComspec(os::env::get_variable(L"COMSPEC"));
		if (!strComspec.empty())
		{
			ExeName=PointToName(strComspec);
			ret = Console().GetAlias(strNewCmdStr.data(), Buffer.get(), Buffer.size() * sizeof(wchar_t) , ExeName);
		}
	}

	if (!ret)
		return false;

	strNewCmdStr.assign(Buffer.get());

	if (!ReplaceStrings(strNewCmdStr, L"$*", strNewCmdPar) && !strNewCmdPar.empty())
		strNewCmdStr+=L" "+strNewCmdPar;

	strStr=strNewCmdStr;

	return true;
}
