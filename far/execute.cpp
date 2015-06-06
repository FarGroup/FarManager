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
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "chgprior.hpp"
#include "cmdline.hpp"
#include "imports.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "constitle.hpp"
#include "console.hpp"
#include "language.hpp"
#include "colormix.hpp"
#include "desktop.hpp"
#include "keybar.hpp"

struct IMAGE_HEADERS
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	union
	{
		IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
		IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
	};
};

static bool GetImageSubsystem(const string& FileName,DWORD& ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem=IMAGE_SUBSYSTEM_UNKNOWN;
	os::fs::file ModuleFile;
	if(ModuleFile.Open(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
	{
		IMAGE_DOS_HEADER DOSHeader;
		size_t ReadSize;

		if (ModuleFile.Read(&DOSHeader, sizeof(DOSHeader), ReadSize) && ReadSize==sizeof(DOSHeader))
		{
			if (DOSHeader.e_magic==IMAGE_DOS_SIGNATURE)
			{
				//ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
				Result=true;

				if (ModuleFile.SetPointer(DOSHeader.e_lfanew,nullptr,FILE_BEGIN))
				{
					IMAGE_HEADERS PEHeader;

					if (ModuleFile.Read(&PEHeader, sizeof(PEHeader), ReadSize) && ReadSize==sizeof(PEHeader))
					{
						if (PEHeader.Signature==IMAGE_NT_SIGNATURE)
						{
							switch (PEHeader.OptionalHeader32.Magic)
							{
								case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
								{
									ImageSubsystem=PEHeader.OptionalHeader32.Subsystem;
								}
								break;

								case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
								{
									ImageSubsystem=PEHeader.OptionalHeader64.Subsystem;
								}
								break;

								/*default:
									{
										// unknown magic
									}*/
							}
						}
						else if ((WORD)PEHeader.Signature==IMAGE_OS2_SIGNATURE)
						{
							/*
							NE,  хмм...  а как определить что оно ГУЕВОЕ?

							Andrzej Novosiolov <andrzej@se.kiev.ua>
							AN> ориентироваться по флагу "Target operating system" NE-заголовка
							AN> (1 байт по смещению 0x36). Если там Windows (значения 2, 4) - подразумеваем
							AN> GUI, если OS/2 и прочая экзотика (остальные значения) - подразумеваем консоль.
							*/
							auto OS2Hdr = reinterpret_cast<PIMAGE_OS2_HEADER>(&PEHeader);
							if (OS2Hdr->ne_exetyp==2 || OS2Hdr->ne_exetyp==4)
							{
								ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_GUI;
							}
						}

						/*else
						{
							// unknown signature
						}*/
					}

					/*else
					{
						// обломс вышел с чтением следующего заголовка ;-(
					}*/
				}

				/*else
				{
					// видимо улетели куда нить в трубу, т.к. dos_head.e_lfanew указал
					// слишком в неправдоподобное место (например это чистой воды DOS-файл)
				}*/
			}

			/*else
			{
				// это не исполняемый файл - у него нету заголовка MZ, например, NLM-модуль
				// TODO: здесь можно разбирать POSIX нотацию, например "/usr/bin/sh"
			}*/
		}

		/*else
		{
			// ошибка чтения
		}*/
		ModuleFile.Close();
	}

	/*else
	{
		// ошибка открытия
	}*/
	return Result;
}

static bool IsProperProgID(const string& ProgID)
{
	return !ProgID.empty() && os::reg::key(HKEY_CLASSES_ROOT, ProgID.data(), KEY_QUERY_VALUE);
}

/*
Ищем валидный ProgID для файлового расширения по списку возможных
hExtKey - корневой ключ для поиска (ключ расширения)
strType - сюда запишется результат, если будет найден
*/
static bool SearchExtHandlerFromList(HKEY hExtKey, string &strType)
{
	FOR(const auto& i, os::reg::enum_value(hExtKey, L"OpenWithProgids"))
	{
		if (i.Type() == REG_SZ && IsProperProgID(i.Name()))
		{
			strType = i.Name();
			return true;
		}
	}

	return false;
}

/*
Функция FindModule пытается найти исполняемый модуль (в т.ч. и по
%PATHEXT%). В случае успеха заменяет в Module порцию, ответственную за
исполняемый модуль на найденное значение, копирует результат в strDest и
пытается проверить заголовок PE на ГУЕВОСТЬ (чтобы запустить процесс
в отдельном окне и не ждать завершения).
В случае неудачи strDest не заполняется!
Return: true/false - нашли/не нашли
Команда в функцию передается уже без кавычек. Ничего не меняем.
И подменять ничего не надо, т.к. все параметры мы отсекли раньше
*/
static bool FindModule(const string& Module, string &strDest,DWORD &ImageSubsystem,bool &Internal)
{
	bool Result=false;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
	Internal = false;

	if (!Module.empty())
	{
		// нулевой проход - смотрим исключения
		// Берем "исключения" из реестра, которые должны исполняться директом,
		// например, некоторые внутренние команды ком. процессора.
		std::vector<string> ExcludeCmdsList;
		split(ExcludeCmdsList, os::env::expand_strings(Global->Opt->Exec.strExcludeCmds), STLF_UNIQUE);

		if (std::any_of(CONST_RANGE(ExcludeCmdsList, i) { return !StrCmpI(i, Module); }))
		{
			ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_CUI;
			Result=true;
			Internal = true;
		}

		if (!Result)
		{
			string strFullName=Module;
			LPCWSTR ModuleExt=wcsrchr(PointToName(Module),L'.');
			string strPathExt = os::env::get_pathext();
			std::vector<string> PathExtList;
			split(PathExtList, strPathExt, STLF_UNIQUE);

			FOR(const auto& i, PathExtList) // первый проход - в текущем каталоге
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
				string strPathEnv;

				if (os::env::get_variable(L"PATH", strPathEnv))
				{
					std::vector<string> Strings;
					split(Strings, strPathEnv, STLF_UNIQUE);
					FOR(const auto& Path, Strings)
					{
						FOR(const auto& Ext, PathExtList)
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
					FOR(const auto& Ext, PathExtList)
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

					for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
					{
						if (i==ARRAYSIZE(RootFindKey)-1)
						{
							if (DWORD RedirectionFlag = os::GetAppPathsRedirectionFlag())
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
						Result = std::any_of(CONST_RANGE(PathExtList, Ext) -> bool
						{
							strFullName=RegPath;
							strFullName+=Module;
							strFullName+=Ext;

							return std::any_of(CONST_RANGE(RootFindKey, i) -> bool
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

			if (Result) // некоторые "подмены" данных
			{
				GetImageSubsystem(strFullName,ImageSubsystem);
				strDest=strFullName;
			}
		}
	}

	return Result;
}

/*
 возвращает 2*PipeFound + 1*Escaped
*/
static int PartCmdLine(const string& CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
	int PipeFound = 0, Escaped = 0;
	bool quoted = false;
	strNewCmdStr = os::env::expand_strings(CmdStr);
	RemoveExternalSpaces(strNewCmdStr);
	const wchar_t * const NewCmdStr = strNewCmdStr.data();
	const wchar_t *CmdPtr = NewCmdStr;
	const wchar_t *ParPtr = nullptr;
	// Разделим собственно команду для исполнения и параметры.
	// При этом заодно определим наличие символов переопределения потоков
	// Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

	static const wchar_t ending_chars[] = L"/ <>|&";

	while (*CmdPtr)
	{
		if (*CmdPtr == L'"')
			quoted = !quoted;

		if (!quoted && *CmdPtr == L'^' && CmdPtr[1] > L' ') // "^>" и иже с ним
		{
			Escaped = 1; //
			CmdPtr++;    // ??? может быть '^' надо удалить...
		}
		else if (!quoted && CmdPtr != NewCmdStr)
		{
			const wchar_t *ending = wcschr(ending_chars, *CmdPtr);
			if ( ending )
			{
				if (!ParPtr)
					ParPtr = CmdPtr;

				if (ending >= ending_chars+2)
					PipeFound = 1;
			}
		}

		if (ParPtr && PipeFound) // Нам больше ничего не надо узнавать
			break;

		CmdPtr++;
	}

	if (ParPtr) // Мы нашли параметры и отделяем мух от котлет
	{
		size_t Pos = ParPtr - NewCmdStr;
		if (*ParPtr == L' ') //AY: первый пробел между командой и параметрами не нужен,
			++ParPtr;        //    он добавляется заново в Execute.

		strNewCmdPar = ParPtr;
		strNewCmdStr.resize(Pos);

	}

	Unquote(strNewCmdStr);

	return 2*PipeFound + 1*Escaped;
}

static bool RunAsSupported(LPCWSTR Name)
{
	string Extension(PointToExt(Name)), Type;
	return !Extension.empty() && GetShellType(Extension, Type) && os::reg::key(HKEY_CLASSES_ROOT, Type.append(L"\\shell\\runas\\command").data(), KEY_QUERY_VALUE);
}

/*
по имени файла (по его расширению) получить команду активации
Дополнительно смотрится гуевость команды-активатора
(чтобы не ждать завершения)
*/
static const wchar_t *GetShellAction(const string& FileName,DWORD& ImageSubsystem,DWORD& Error)
{
	string strValue;
	string strNewValue;
	const wchar_t *RetPtr;
	const wchar_t command_action[]=L"\\command";
	Error = ERROR_SUCCESS;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

	const auto ExtPtr = wcsrchr(FileName.data(), L'.');
	if (!ExtPtr)
		return nullptr;

	if (!GetShellType(ExtPtr, strValue))
		return nullptr;

	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT,strValue.data(),0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		int nResult=RegQueryValueEx(hKey,L"IsShortcut",nullptr,nullptr,nullptr,nullptr);
		RegCloseKey(hKey);

		if (nResult==ERROR_SUCCESS)
			return nullptr;
	}

	strValue += L"\\shell";

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, strValue.data(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey)!=ERROR_SUCCESS)
		return nullptr;

	strValue += L"\\";

	static string strAction;

	if (os::reg::GetValue(hKey, L"", strAction))
	{
		RetPtr = EmptyToNull(strAction.data());
		LONG RetEnum = ERROR_SUCCESS;
		std::vector<string> ActionList;
		split(ActionList, strAction, STLF_UNIQUE);

		if (RetPtr && !ActionList.empty())
		{
			FOR(const auto& i, ActionList)
			{
				strNewValue = strValue;
				strNewValue += i;
				strNewValue += command_action;

				if (os::reg::key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
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

		if (os::reg::key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
		{
			strValue += strAction;
			RetPtr = strAction.data();
		}
		else
		{
			// ... а теперь все остальное, если "open" нету
			FOR(const auto& i, os::reg::enum_key(hKey))
			{
				strAction = i;

				// Проверим наличие "команды" у этого ключа
				strNewValue = strValue;
				strNewValue += strAction;
				strNewValue += command_action;

				if (os::reg::key(HKEY_CLASSES_ROOT, strNewValue.data(), KEY_QUERY_VALUE))
				{
					strValue += strAction;
					RetPtr = strAction.data();
					break;
				}
			}
		}
	}

	RegCloseKey(hKey);

	if (RetPtr)
	{
		strValue += command_action;

		// а теперь проверим ГУЕвость запускаемой проги
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
				auto pos = strNewValue.find_first_of(L" \t/");
				if (pos != string::npos)
					strNewValue.resize(pos);
			}

			GetImageSubsystem(strNewValue,ImageSubsystem);
		}
		else
		{
			Error=ERROR_NO_ASSOCIATION;
			RetPtr=nullptr;
		}
	}

	return RetPtr;
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

		HKEY hCRKey = nullptr, hUserKey = nullptr;
		string strFoundValue;

		if (aType == AT_FILEEXTENSION)
		{
			// Смотрим дефолтный обработчик расширения в HKEY_CURRENT_USER
			if (os::reg::GetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + Ext, L"ProgID", strFoundValue) && IsProperProgID(strFoundValue))
			{
				strType = strFoundValue;
			}
		}

		// Смотрим дефолтный обработчик расширения в HKEY_CLASSES_ROOT
		if (strType.empty() && os::reg::GetValue(HKEY_CLASSES_ROOT, Ext, L"", strFoundValue) && IsProperProgID(strFoundValue))
		{
			strType = strFoundValue;
		}

		if (strType.empty() && hUserKey)
			SearchExtHandlerFromList(hUserKey, strType);

		if (strType.empty() && hCRKey)
			SearchExtHandlerFromList(hCRKey, strType);

		if (hUserKey)
			RegCloseKey(hUserKey);

		if (hCRKey)
			RegCloseKey(hCRKey);
	}

	return !strType.empty();
}

/*
Функция-пускатель внешних процессов
Возвращает -1 в случае ошибки или...
*/
int Execute(const string& CmdStr,  // Ком.строка для исполнения
            bool AlwaysWaitFinish, // Ждать завершение процесса?
            bool SeparateWindow,   // Выполнить в отдельном окне?
            bool DirectRun,        // Выполнять директом? (без CMD)
            bool FolderRun,        // Это фолдер?
            bool WaitForIdle,      // for list files
            bool Silent,
            bool RunAs             // elevation
            )
{
	int nResult = -1;
	string strNewCmdStr;
	string strNewCmdPar;

	int PipeOrEscaped = PartCmdLine(CmdStr, strNewCmdStr, strNewCmdPar);

	const bool IsDirectory = os::fs::is_directory(strNewCmdStr);

	if(RunAs)
	{
		SeparateWindow = true;
	}

	if(FolderRun)
	{
		Silent = true;
	}

	if (SeparateWindow)
	{
		if(Global->Opt->Exec.ExecuteSilentExternal)
		{
			Silent = true;
		}
		if (strNewCmdPar.empty() && IsDirectory)
		{
			ConvertNameToFull(strNewCmdStr, strNewCmdStr);
			DirectRun = true;
			FolderRun=true;
		}
	}

	string strComspec;
	os::env::get_variable(L"COMSPEC", strComspec);
	if (strComspec.empty() && !DirectRun)
	{
		Message(MSG_WARNING, 1, MSG(MError), MSG(MComspecNotFound), MSG(MOk));
		return -1;
	}

	DWORD dwSubSystem = IMAGE_SUBSYSTEM_UNKNOWN;
	HANDLE hProcess = nullptr;
	LPCWSTR lpVerb = nullptr;

	if (FolderRun && DirectRun)
	{
		AddEndSlash(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
	}
	else
	{
		bool internal;
		FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem,internal);

		if (/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
		{

			const wchar_t *ExtPtr=wcsrchr(PointToName(strNewCmdStr), L'.');
			if (ExtPtr)
			{
				if (!(!StrCmpI(ExtPtr,L".exe") || !StrCmpI(ExtPtr,L".com") || IsBatchExtType(ExtPtr)))
				{
					DWORD Error=0, dwSubSystem2=0;
					lpVerb=GetShellAction(strNewCmdStr,dwSubSystem2,Error);

					if (lpVerb && Error != ERROR_NO_ASSOCIATION)
					{
						dwSubSystem=dwSubSystem2;
					}
				}

				if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN && !StrCmpNI(strNewCmdStr.data(),L"ECHO.",5)) // вариант "echo."
				{
					strNewCmdStr.replace(4, 1, 1, L' ');
					PartCmdLine(strNewCmdStr,strNewCmdStr,strNewCmdPar);

					if (strNewCmdPar.empty())
						strNewCmdStr+=L'.';

					FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem,internal);
				}
			}
		}

		if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			if ( !DirectRun )
			{
				DirectRun = (PipeOrEscaped < 1); //??? <= 1 если бы '^' были удалены
			}
			if(DirectRun && Global->Opt->Exec.ExecuteSilentExternal)
			{
				Silent = true;
			}
			SeparateWindow = true;
		}
		else if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_CUI && !DirectRun && !internal)
		{
			DirectRun = (PipeOrEscaped < 1); //??? <= 1 если бы '^' были удалены
		}
	}

	bool Visible=false;
	DWORD CursorSize=0;
	SMALL_RECT ConsoleWindowRect;
	COORD ConsoleSize={};
	int ConsoleCP = CP_OEMCP;
	int ConsoleOutputCP = CP_OEMCP;
	int add_show_clock = 0;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SCOPED_ACTION(ConsoleTitle);

	SHELLEXECUTEINFO seInfo={sizeof(seInfo)};
	string strCurDir;
	os::GetCurrentDirectory(strCurDir);
	seInfo.lpDirectory=strCurDir.data();
	seInfo.nShow = SW_SHOWNORMAL;

	string strFarTitle;


	if(!Silent)
	{
		int X1, X2, Y1, Y2;
		Global->CtrlObject->CmdLine()->GetPosition(X1, Y1, X2, Y2);
		Global->ProcessShowClock += (add_show_clock = 1);
		Global->WindowManager->ShowBackground();
		Global->CtrlObject->CmdLine()->Redraw();
		GotoXY(X2+1,Y1);
		Text(L' ');
		MoveCursor(X1,Y1);
		GetCursorType(Visible, CursorSize);
		SetInitialCursorType();
	}

	Global->CtrlObject->CmdLine()->SetString(L"", Silent);

	if(!Silent)
	{
		// BUGBUG: если команда начинается с "@", то эта строка херит все начинания
		// TODO: здесь необходимо подставить виртуальный буфер, а потом его корректно подсунуть в ScrBuf
		Global->ScrBuf->SetLockCount(0);
		Global->ScrBuf->Flush(true);

		ConsoleCP = Console().GetInputCodepage();
		ConsoleOutputCP = Console().GetOutputCodepage();
		FlushInputBuffer();
		ChangeConsoleMode(InitialConsoleMode);
		Console().GetWindowRect(ConsoleWindowRect);
		Console().GetSize(ConsoleSize);

		if (Global->Opt->Exec.ExecuteFullTitle)
		{
			strFarTitle += strNewCmdStr;
			if (!strNewCmdPar.empty())
			{
				strFarTitle.append(L" ").append(strNewCmdPar);
			}
		}
		else
		{
			strFarTitle+=CmdStr;
		}
		ConsoleTitle::SetFarTitle(strFarTitle);
	}

	string ComSpecParams(Global->Opt->Exec.strComSpecParams);
	ComSpecParams += L" ";
	if (DirectRun)
	{
		seInfo.lpFile = strNewCmdStr.data();
		if(!strNewCmdPar.empty())
		{
			seInfo.lpParameters = strNewCmdPar.data();
		}
		//Maximus: рушилась dwSubSystem
		DWORD dwSubSystem2 = IMAGE_SUBSYSTEM_UNKNOWN;
		DWORD dwError = 0;
		seInfo.lpVerb = IsDirectory? nullptr : lpVerb? lpVerb : GetShellAction(strNewCmdStr, dwSubSystem2, dwError);
		if (dwSubSystem2!=IMAGE_SUBSYSTEM_UNKNOWN && dwSubSystem==IMAGE_SUBSYSTEM_UNKNOWN)
			dwSubSystem=dwSubSystem2;
	}
	else
	{
		std::vector<string> NotQuotedShellList;
		split(NotQuotedShellList, os::env::expand_strings(Global->Opt->Exec.strNotQuotedShell), STLF_UNIQUE);
		bool bQuotedShell = !(std::any_of(CONST_RANGE(NotQuotedShellList, i) { return !StrCmpI(i,PointToName(strComspec.data())); }));
		QuoteSpace(strNewCmdStr);
		bool bDoubleQ = strNewCmdStr.find_first_of(L"&<>()@^|=;, ") != string::npos;
		if ((!strNewCmdPar.empty() || bDoubleQ) && bQuotedShell)
		{
			ComSpecParams += L"\"";
		}
		ComSpecParams += strNewCmdStr;
		if (!strNewCmdPar.empty())
		{
			ComSpecParams.append(L" ").append(strNewCmdPar);
		}
		if ((!strNewCmdPar.empty() || bDoubleQ) && bQuotedShell)
		{
			ComSpecParams += L"\"";
		}

		seInfo.lpFile = strComspec.data();
		seInfo.lpParameters = ComSpecParams.data();
		seInfo.lpVerb = nullptr;
	}

	if(RunAs && RunAsSupported(seInfo.lpFile))
	{
		seInfo.lpVerb = L"runas";
	}

	seInfo.fMask = SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS|(SeparateWindow?0:SEE_MASK_NO_CONSOLE);
#if 0
	seInfo.fMask |= SEE_MASK_FLAG_NO_UI;
	if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)  // для .exe НЕ включать - бывают проблемы с запуском
		if (IsWindowsVistaOrGreater()) // ShellExecuteEx error, see
			seInfo.fMask |= SEE_MASK_INVOKEIDLIST; // http://us.generation-nt.com/answer/shellexecuteex-does-not-allow-openas-verb-windows-7-help-31497352.html
#endif
	if(!Silent)
	{
		Console().ScrollScreenBuffer(((DirectRun && dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || SeparateWindow)?2:1);
	}

	// ShellExecuteEx fails if IE10 is installed and if current directory is symlink/junction
	wchar_t CurDir[MAX_PATH];
	bool NeedFixCurDir = os::fs::file_status(strCurDir).check(FILE_ATTRIBUTE_REPARSE_POINT);
	if (NeedFixCurDir)
	{
		if (!GetCurrentDirectory(ARRAYSIZE(CurDir), CurDir))
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

	DWORD dwError = 0;
	if (ShellExecuteEx(&seInfo))
	{
		hProcess = seInfo.hProcess;
	}
	else
	{
		Global->CatchError();
		dwError = Global->CaughtError();
	}

	// ShellExecuteEx fails if IE10 is installed and if current directory is symlink/junction
	if (NeedFixCurDir)
	{
		SetCurrentDirectory(CurDir);
	}

	if (!dwError)
	{
		if (hProcess)
		{
			if (!Silent)
			{
				Global->ScrBuf->Flush();
			}
			if (AlwaysWaitFinish || !SeparateWindow)
			{
				if (Global->Opt->ConsoleDetachKey.empty())
				{
					WaitForSingleObject(hProcess,INFINITE);
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
					while (WaitForSingleObject(hProcess, 100) != WAIT_OBJECT_0)
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

				if(!Silent)
				{
					bool SkipScroll = false;
					COORD Size;
					if(Console().GetSize(Size))
					{
						matrix<FAR_CHAR_INFO> Buffer(Global->Opt->ShowKeyBar ? 3 : 2, Size.X);
						SMALL_RECT ReadRegion = {0, static_cast<SHORT>(Size.Y - Buffer.height()), static_cast<SHORT>(Size.X-1), static_cast<SHORT>(Size.Y-1)};
						if(Console().ReadOutput(Buffer, ReadRegion))
						{
							FarColor Attributes = Buffer.back().back().Attributes;
							SkipScroll = true;
							FOR(const auto& i, Buffer.vector())
							{
								if(i.Char != L' ' || i.Attributes.ForegroundColor != Attributes.ForegroundColor || i.Attributes.BackgroundColor != Attributes.BackgroundColor || i.Attributes.Flags != Attributes.Flags)
								{
									SkipScroll = false;
									break;
								}
							}
						}
					}
					if(!SkipScroll)
					{
						Console().ScrollScreenBuffer(Global->Opt->ShowKeyBar?2:1);
					}
				}

			}
			if(WaitForIdle)
			{
				WaitForInputIdle(hProcess, INFINITE);
			}
			CloseHandle(hProcess);
		}

		nResult = 0;

	}
	Global->ProcessShowClock -= add_show_clock;

	SetFarConsoleMode(TRUE);
	/* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
	    вследствие своей оптимизации, которая в данном случае выходит боком.
	*/
	SetCursorType(Visible, CursorSize);
	CONSOLE_CURSOR_INFO cci = { CursorSize, Visible };
	Console().SetCursorInfo(cci);

	COORD ConSize;
	Console().GetSize(ConSize);
	if(ConSize.X!=ScrX+1 || ConSize.Y!=ScrY+1)
	{
		ChangeVideoMode(ConSize.Y, ConSize.X);
	}
	else
	{
		if(!Silent)
		{
			Global->CtrlObject->Desktop->FillFromConsole();
		}
	}

	if (Global->Opt->Exec.RestoreCPAfterExecute)
	{
		// восстановим CP-консоли после исполнения проги
		Console().SetInputCodepage(ConsoleCP);
		Console().SetOutputCodepage(ConsoleOutputCP);
	}

	Console().SetTextAttributes(colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN));

	if(dwError)
	{
		if (!Silent)
		{
			Global->CtrlObject->Cp()->Redraw();
			if (Global->Opt->ShowKeyBar)
			{
				Global->CtrlObject->Cp()->GetKeybar().Show();
			}
			if (Global->Opt->Clock)
				ShowTime(1);
		}

		const auto Strings = DirectRun?
			make_vector<string>(MSG(MCannotExecute), strNewCmdStr) :
			make_vector<string>(MSG(MCannotInvokeComspec), strComspec, MSG(MCheckComspecVar));
		Message(MSG_WARNING | MSG_ERRORTYPE, MSG(MError), Strings, make_vector<string>(MSG(MOk)), L"ErrCannotExecute", nullptr, nullptr, make_vector(DirectRun ? strNewCmdStr : strComspec));
	}

	return nResult;
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
const wchar_t *PrepareOSIfExist(const string& CmdLine)
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
		// ExtractIfExistCommand в filetype.cpp
		PtrCmd++;

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;
	}

	for (;;)
	{
		if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd,L"IF ",3)) //??? IF/I не обрабатывается
			break;

		PtrCmd+=3;

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;

		if (!*PtrCmd)
			break;

		if (!StrCmpNI(PtrCmd,L"NOT ",4))
		{
			Not=TRUE;

			PtrCmd+=4;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;
		}

		if (*PtrCmd && !StrCmpNI(PtrCmd,L"EXIST ",6))
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
					if (Global->CtrlObject)
						strFullPath = Global->CtrlObject->CmdLine()->GetCurDir();
					else
						os::GetCurrentDirectory(strFullPath);

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
		else if (*PtrCmd && !StrCmpNI(PtrCmd,L"DEFINED ",8))
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

					DWORD ERet = os::env::get_variable(strCmd, strExpandedStr);

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

/*
Проверить "Это батник?"
*/
bool IsBatchExtType(const string& ExtPtr)
{
	string strExecuteBatchType = os::env::expand_strings(Global->Opt->Exec.strExecuteBatchType);
	if (strExecuteBatchType.empty())
		strExecuteBatchType = L".BAT;.CMD";
	std::vector<string> BatchExtList;
	split(BatchExtList, strExecuteBatchType, STLF_UNIQUE);
	return std::any_of(CONST_RANGE(BatchExtList, i) {return !StrCmpI(i, ExtPtr);});
}

bool ProcessOSAliases(string &strStr)
{
	string strNewCmdStr;
	string strNewCmdPar;

	PartCmdLine(strStr,strNewCmdStr,strNewCmdPar);

	const wchar_t *lpwszExeName=PointToName(Global->g_strFarModuleName);
	wchar_t_ptr Buffer(4096);
	int ret = Console().GetAlias(strNewCmdStr.data(), Buffer.get(), Buffer.size() * sizeof(wchar_t), lpwszExeName);

	if (!ret)
	{
		string strComspec;
		if (os::env::get_variable(L"COMSPEC", strComspec))
		{
			lpwszExeName=PointToName(strComspec);
			ret = Console().GetAlias(strNewCmdStr.data(), Buffer.get(), Buffer.size() * sizeof(wchar_t) , lpwszExeName);
		}
	}

	if (!ret)
		return false;

	strNewCmdStr.assign(Buffer.get());

	if (!ReplaceStrings(strNewCmdStr,L"$*",strNewCmdPar))
		strNewCmdStr+=L" "+strNewCmdPar;

	strStr=strNewCmdStr;

	return true;
}
