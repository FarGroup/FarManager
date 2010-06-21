/*
execute.cpp

"Запускатель" программ.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "cmdline.hpp"
#include "panel.hpp"
#include "rdrwdsk.hpp"
#include "udlist.hpp"
#include "imports.hpp"
#include "registry.hpp"
#include "localOEM.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "syslog.hpp"
#include "constitle.hpp"
#include "console.hpp"

static const wchar_t strSystemExecutor[]=L"System\\Executor";

// Выдранный кусок из будущего GetFileInfo, получаем достоверную информацию о ГУЯХ PE-модуля

// Возвращаем константы IMAGE_SUBSYSTEM_* дабы консоль отличать
// При выходе из процедуры IMAGE_SUBSYTEM_UNKNOWN означает
// "файл не является исполняемым".
// Для DOS-приложений определим еще одно значение флага.

//#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

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

static bool GetImageSubsystem(const wchar_t *FileName,DWORD& ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem=IMAGE_SUBSYSTEM_UNKNOWN;
	File ModuleFile;
	if(ModuleFile.Open(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
	{
		IMAGE_DOS_HEADER DOSHeader;
		DWORD ReadSize;

		if (ModuleFile.Read(&DOSHeader, sizeof(DOSHeader), &ReadSize) && ReadSize==sizeof(DOSHeader))
		{
			if (DOSHeader.e_magic==IMAGE_DOS_SIGNATURE)
			{
				//ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
				Result=true;

				if (ModuleFile.SetPointer(DOSHeader.e_lfanew,nullptr,FILE_BEGIN))
				{
					IMAGE_HEADERS PEHeader;

					if (ModuleFile.Read(&PEHeader, sizeof(PEHeader), &ReadSize) && ReadSize==sizeof(PEHeader))
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
							BYTE ne_exetyp=reinterpret_cast<PIMAGE_OS2_HEADER>(&PEHeader)->ne_exetyp;

							if (ne_exetyp==2||ne_exetyp==4)
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
					// слишком в неправдоподное место (например это чистой воды DOS-файл)
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

bool IsProperProgID(const wchar_t* ProgID)
{
	if (ProgID && *ProgID)
	{
		HKEY hProgID;

		if (RegOpenKey(HKEY_CLASSES_ROOT, ProgID, &hProgID) == ERROR_SUCCESS)
		{
			RegCloseKey(hProgID);
			return true;
		}
	}

	return false;
}

// Ищем валидный ProgID для файлового расширения по списку возможных
// hExtKey - корневой ключ для поиска (ключ расширения)
// strType - сюда запишется результат, если будет найден
bool SearchExtHandlerFromList(HKEY hExtKey, string &strType)
{
	const DWORD dwNameBufSize = 64;
	HKEY hExtIDListSubKey;

	if (RegOpenKey(hExtKey, L"OpenWithProgids", &hExtIDListSubKey) == ERROR_SUCCESS)
	{
		DWORD nValueIndex = 0;
		LONG nRet;
		wchar_t wszValueName[dwNameBufSize];
		DWORD nValueNameSize = dwNameBufSize;
		DWORD nValueType;

		// Пройдемся по всем значениям и проверим имена на пренадлежность к валидным типам
		while ((nRet = RegEnumValue(hExtIDListSubKey, nValueIndex, wszValueName, &nValueNameSize, nullptr, &nValueType, nullptr, nullptr)) != ERROR_NO_MORE_ITEMS)
		{
			if (nRet != ERROR_SUCCESS) break;

			if ((nValueType == REG_SZ || nValueType == REG_NONE) && IsProperProgID(wszValueName))
			{
				strType = wszValueName;
				RegCloseKey(hExtIDListSubKey);
				return true;
			}

			nValueIndex++;
			nValueNameSize = dwNameBufSize;	// сбросим значение до полного размера буфера (после функции тут будет размер строки)
		}

		RegCloseKey(hExtIDListSubKey);
	}

	return false;
}

bool GetShellType(const wchar_t *Ext, string &strType,ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;
	strType.Clear();

	if (WinVer.dwMajorVersion >= 6 && ifn.pfnSHCreateAssociationRegistration)
	{
		IApplicationAssociationRegistration* pAAR;
		HRESULT hr = ifn.pfnSHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (void**)&pAAR);

		if (SUCCEEDED(hr))
		{
			wchar_t *p;

			if (pAAR->QueryCurrentDefault(Ext, aType, AL_EFFECTIVE, &p) == S_OK)
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

		HKEY hCRKey = 0, hUserKey = 0;
		string strFoundValue;

		if (aType == AT_FILEEXTENSION)
		{
			string strExplorerTypeKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\");
			strExplorerTypeKey.Append(Ext);

			// Смотрим дефолтный обработчик расширения в HKEY_CURRENT_USER
			if (RegOpenKey(HKEY_CURRENT_USER, strExplorerTypeKey, &hUserKey) == ERROR_SUCCESS)
			{
				if ((RegQueryStringValue(hUserKey, L"ProgID", strFoundValue) == ERROR_SUCCESS) && IsProperProgID(strFoundValue))
				{
					strType = strFoundValue;
				}
			}
		}

		// Смотрим дефолтный обработчик расширения в HKEY_CLASSES_ROOT
		if (strType.IsEmpty() && (RegOpenKey(HKEY_CLASSES_ROOT, Ext, &hCRKey) == ERROR_SUCCESS))
		{
			if ((RegQueryStringValue(hCRKey, L"", strFoundValue) == ERROR_SUCCESS) && IsProperProgID(strFoundValue))
			{
				strType = strFoundValue;
			}
		}

		if (strType.IsEmpty() && hUserKey)
			SearchExtHandlerFromList(hUserKey, strType);

		if (strType.IsEmpty() && hCRKey)
			SearchExtHandlerFromList(hCRKey, strType);

		if (hUserKey)
			RegCloseKey(hUserKey);

		if (hCRKey)
			RegCloseKey(hCRKey);
	}

	return !strType.IsEmpty();
}

// по имени файла (по его расширению) получить команду активации
// Дополнительно смотрится гуевость команды-активатора
// (чтобы не ждать завершения)
const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error)
{
	string strValue;
	string strNewValue;
	const wchar_t *ExtPtr;
	const wchar_t *RetPtr;
	const wchar_t command_action[]=L"\\command";
	Error = ERROR_SUCCESS;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

	if (!(ExtPtr=wcsrchr(FileName,L'.')))
		return nullptr;

	if (!GetShellType(ExtPtr, strValue))
		return nullptr;

	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT,strValue,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		int nResult=RegQueryValueEx(hKey,L"IsShortcut",nullptr,nullptr,nullptr,nullptr);
		RegCloseKey(hKey);

		if (nResult==ERROR_SUCCESS)
			return nullptr;
	}

	strValue += L"\\shell";

	if (RegOpenKey(HKEY_CLASSES_ROOT,strValue,&hKey)!=ERROR_SUCCESS)
		return nullptr;

	static string strAction;
	int RetQuery = RegQueryStringValueEx(hKey,L"",strAction,L"");
	strValue += L"\\";

	if (RetQuery == ERROR_SUCCESS)
	{
		UserDefinedList ActionList(0,0,ULF_UNIQUE);
		RetPtr = (strAction.IsEmpty() ? nullptr : strAction.CPtr());
		const wchar_t *ActionPtr;
		LONG RetEnum = ERROR_SUCCESS;

		if (RetPtr  && ActionList.Set(strAction))
		{
			HKEY hOpenKey;
			ActionList.Reset();

			while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) )
			{
				strNewValue = strValue;
				strNewValue += ActionPtr;
				strNewValue += command_action;

				if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
				{
					RegCloseKey(hOpenKey);
					strValue += ActionPtr;
					strAction = ActionPtr;
					RetPtr = strAction;
					RetEnum = ERROR_NO_MORE_ITEMS;
				} /* if */
			} /* while */
		} /* if */
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
		LONG RetEnum = ERROR_SUCCESS;
		DWORD dwIndex = 0;
		HKEY hOpenKey;
		// Сначала проверим "open"...
		strAction = L"open";
		strNewValue = strValue;
		strNewValue += strAction;
		strNewValue += command_action;

		if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
		{
			RegCloseKey(hOpenKey);
			strValue += strAction;
			RetPtr = strAction;
			RetEnum = ERROR_NO_MORE_ITEMS;
		} /* if */

		// ... а теперь все остальное, если "open" нету
		while (RetEnum == ERROR_SUCCESS)
		{
			RetEnum=apiRegEnumKeyEx(hKey, dwIndex++,strAction);

			if (RetEnum == ERROR_SUCCESS)
			{
				// Проверим наличие "команды" у этого ключа
				strNewValue = strValue;
				strNewValue += strAction;
				strNewValue += command_action;

				if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
				{
					RegCloseKey(hOpenKey);
					strValue += strAction;
					RetPtr = strAction;
					RetEnum = ERROR_NO_MORE_ITEMS;
				} /* if */
			} /* if */
		} /* while */
	} /* if */

	RegCloseKey(hKey);

	if (RetPtr )
	{
		strValue += command_action;

		// а теперь проверим ГУЕвость запускаемой проги
		if (RegOpenKey(HKEY_CLASSES_ROOT,strValue,&hKey)==ERROR_SUCCESS)
		{
			RetQuery=RegQueryStringValueEx(hKey,L"",strNewValue,L"");
			RegCloseKey(hKey);

			if (RetQuery == ERROR_SUCCESS && !strNewValue.IsEmpty())
			{
				apiExpandEnvironmentStrings(strNewValue,strNewValue);
				wchar_t *Ptr = strNewValue.GetBuffer();

				// Выделяем имя модуля
				if (*Ptr==L'\"')
				{
					wchar_t *QPtr = wcschr(Ptr + 1,L'\"');

					if (QPtr)
					{
						*QPtr=0;
						wmemmove(Ptr, Ptr + 1, QPtr-Ptr);
					}
				}
				else
				{
					if ((Ptr=wcspbrk(Ptr,L" \t/")))
						*Ptr=0;
				}

				strNewValue.ReleaseBuffer();
				GetImageSubsystem(strNewValue,ImageSubsystem);
			}
			else
			{
				Error=ERROR_NO_ASSOCIATION;
				RetPtr=nullptr;
			}
		}
	}

	return RetPtr;
}

/*
Фунция FindModule пытается найти исполняемый модуль (в т.ч. и по
%PATHEXT%). В случае успеха заменяет в Module порцию, ответственную за
исполянемый модуль на найденное значение, копирует результат в strDest и
пытается проверить заголовок PE на ГУЕВОСТЬ (чтобы запустить процесс
в отдельном окне и не ждать завершения).
В случае неудачи strDest не заполняется!
Return: true/false - нашли/не нашли
Команда в функцию передается уже разкавыченная. Ничего не меняем.
И подменять ничего не надо, т.к. все параметры мы отсекли раньше
*/

bool WINAPI FindModule(const wchar_t *Module, string &strDest,DWORD &ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

	if (Module && *Module)
	{
		// нулевой проход - смотрим исключения
		// Берем "исключения" из реестра, которые должны исполняться директом,
		// например, некоторые внутренние команды ком. процессора.
		string strExcludeCmds;
		GetRegKey(strSystemExecutor,L"ExcludeCmds",strExcludeCmds,L"");
		UserDefinedList ExcludeCmdsList;
		ExcludeCmdsList.Set(strExcludeCmds);

		while (!ExcludeCmdsList.IsEmpty())
		{
			if (!StrCmpI(Module,ExcludeCmdsList.GetNext()))
			{
				ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_CUI;
				Result=true;
				break;
			}
		}

		if (!Result)
		{
			string strFullName=Module;
			LPCWSTR ModuleExt=wcsrchr(Module,L'.');
			string strPathExt(L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH");
			apiGetEnvironmentVariable(L"PATHEXT",strPathExt);
			UserDefinedList PathExtList;
			PathExtList.Set(strPathExt);
			PathExtList.Reset();

			while (!PathExtList.IsEmpty()) // первый проход - в текущем каталоге
			{
				LPCWSTR Ext=PathExtList.GetNext();
				string strTmpName=strFullName;

				if (!ModuleExt)
				{
					strTmpName+=Ext;
				}

				DWORD Attr=apiGetFileAttributes(strTmpName);

				if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
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

				if (apiGetEnvironmentVariable(L"PATH",strPathEnv))
				{
					UserDefinedList PathList;
					PathList.Set(strPathEnv);

					while (!PathList.IsEmpty() && !Result)
					{
						LPCWSTR Path=PathList.GetNext();
						PathExtList.Reset();

						while (!PathExtList.IsEmpty())
						{
							string strDest;
							LPCWSTR Ext=PathExtList.GetNext();

							if (apiSearchPath(Path,strFullName,Ext,strDest))
							{
								DWORD Attr=apiGetFileAttributes(strDest);

								if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
								{
									strFullName=strDest;
									Result=true;
									break;
								}
							}
						}
					}
				}

				if (!Result)
				{
					PathExtList.Reset();

					while (!PathExtList.IsEmpty())
					{
						string strDest;
						LPCWSTR Ext=PathExtList.GetNext();

						if (apiSearchPath(nullptr,strFullName,Ext,strDest))
						{
							DWORD Attr=apiGetFileAttributes(strDest);

							if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
							{
								strFullName=strDest;
								Result=true;
								break;
							}
						}
					}
				}

				// третий проход - лезим в реестр в "App Paths"
				if (!Result && Opt.ExecuteUseAppPath && !strFullName.Contains(L'\\'))
				{
					LPCWSTR RegPath=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
					// В строке Module заменить исполняемый модуль на полный путь, который
					// берется из SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
					// Сначала смотрим в HKCU, затем - в HKLM
					HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};
					strFullName=RegPath;
					strFullName+=Module;

					for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
					{
						HKEY hKey;

						if (RegOpenKeyEx(RootFindKey[i],strFullName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
						{
							int RegResult=RegQueryStringValueEx(hKey,L"",strFullName,L"");
							RegCloseKey(hKey);

							if (RegResult==ERROR_SUCCESS)
							{
								apiExpandEnvironmentStrings(strFullName,strFullName);
								Unquote(strFullName);
								Result=true;
								break;
							}
						}
					}

					if (!Result)
					{
						PathExtList.Reset();

						while (!PathExtList.IsEmpty() && !Result)
						{
							LPCWSTR Ext=PathExtList.GetNext();
							strFullName=RegPath;
							strFullName+=Module;
							strFullName+=Ext;

							for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
							{
								HKEY hKey;

								if (RegOpenKeyEx(RootFindKey[i],strFullName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
								{
									int RegResult=RegQueryStringValueEx(hKey,L"",strFullName,L"");
									RegCloseKey(hKey);

									if (RegResult==ERROR_SUCCESS)
									{
										apiExpandEnvironmentStrings(strFullName,strFullName);
										Unquote(strFullName);
										Result=true;
										break;
									}
								}
							}
						}
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
 возвращает PipeFound
*/
int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
	int PipeFound = FALSE;
	int QuoteFound = FALSE;
	apiExpandEnvironmentStrings(CmdStr, strNewCmdStr);
	RemoveExternalSpaces(strNewCmdStr);
	wchar_t *NewCmdStr = strNewCmdStr.GetBuffer();
	wchar_t *CmdPtr = NewCmdStr;
	wchar_t *ParPtr = nullptr;
	// Разделим собственно команду для исполнения и параметры.
	// При этом заодно определим наличие символов переопределения потоков
	// Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

	while (*CmdPtr)
	{
		if (*CmdPtr == L'"')
			QuoteFound = !QuoteFound;

		if (!QuoteFound && *CmdPtr == L'^') // для "^>" и еже с ним
		{
			CmdPtr++;
		}
		else if (!QuoteFound && CmdPtr != NewCmdStr)
		{
			if (*CmdPtr == L'>' ||
			        *CmdPtr == L'<' ||
			        *CmdPtr == L'|' ||
			        *CmdPtr == L' ' ||
			        *CmdPtr == L'/' || // вариант "far.exe/?"
			        *CmdPtr == L'&'    // обработаем разделитель команд
			   )
			{
				if (!ParPtr)
					ParPtr = CmdPtr;

				if (*CmdPtr != L' ' && *CmdPtr != L'/')
					PipeFound = TRUE;
			}
		}

		if (ParPtr && PipeFound)
			// Нам больше ничего не надо узнавать
			break;

		CmdPtr++;
	}

	if (ParPtr) // Мы нашли параметры и отделяем мух от котлет
	{
		if (*ParPtr == L' ') //AY: первый пробел между командой и параметрами не нужен,
			*(ParPtr++)=0;   //    он добавляется заново в Execute.

		strNewCmdPar = ParPtr;
		*ParPtr = 0;
	}

	strNewCmdStr.ReleaseBuffer();
	Unquote(strNewCmdStr);
	return PipeFound;
}

/* Функция-пускатель внешних процессов
   Возвращает -1 в случае ошибки или...
*/
int Execute(const wchar_t *CmdStr,    // Ком.строка для исполнения
            int AlwaysWaitFinish,  // Ждать завершение процесса?
            int SeparateWindow,    // Выполнить в отдельном окне? =2 для вызова ShellExecuteEx()
            int DirectRun,         // Выполнять директом? (без CMD)
            int FolderRun,         // Это фолдер?
            bool WaitForIdle,      // for list files
            bool Silent)
{
	int nResult = -1;
	string strNewCmdStr;
	string strNewCmdPar;
	string strExecLine;

	PartCmdLine(CmdStr, strNewCmdStr, strNewCmdPar);

	/* $ 05.04.2005 AY: Это не правильно, надо убирать только первый пробел,
	                    что теперь и делает PartCmdLine.
	if(*NewCmdPar)
	  RemoveExternalSpaces(NewCmdPar);
	AY $ */

	DWORD dwAttr = apiGetFileAttributes(strNewCmdStr);

	if (SeparateWindow == 1)
	{
		if (strNewCmdPar.IsEmpty() && dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			ConvertNameToFull(strNewCmdStr, strNewCmdStr);
			SeparateWindow=2;
			FolderRun=TRUE;
		}
	}

	string strComspec;
	apiGetEnvironmentVariable(L"COMSPEC", strComspec);

	if (strComspec.IsEmpty() && (SeparateWindow != 2))
	{
		Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
		return -1;
	}


	int X1, X2, Y1, Y2;
	CtrlObject->CmdLine->GetPosition(X1, Y1, X2, Y2);
	if(!Silent && !FolderRun)
	{
		ProcessShowClock++;
		CtrlObject->CmdLine->ShowBackground();
		CtrlObject->CmdLine->Redraw();
		GotoXY(X2+1,Y1);
		Text(L" ");
		MoveCursor(X1,Y1);
	}
	CtrlObject->CmdLine->SetString(L"", FALSE);

	bool Visible=false;
	DWORD Size=0;
	GetCursorType(Visible,Size);
	SetInitialCursorType();

	// BUGBUG: если команда начинается с "@", то эта строка херит все начинания
	// TODO: здесь необходимо подставить виртуальный буфер, а потом его корректно подсунуть в ScrBuf
	ScrBuf.SetLockCount(0);

	ScrBuf.Flush();

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	int ConsoleCP = Console.GetInputCodepage();
	int ConsoleOutputCP = Console.GetOutputCodepage();

	FlushInputBuffer();
	ChangeConsoleMode(InitialConsoleMode);

	SMALL_RECT ConsoleWindowRect;
	COORD ConsoleSize;
	Console.GetWindowRect(ConsoleWindowRect);
	Console.GetSize(ConsoleSize);

	ConsoleTitle OldTitle;

	DWORD dwSubSystem;
	DWORD dwError = 0;
	HANDLE hProcess = nullptr, hThread = nullptr;
	LPCWSTR lpVerb = nullptr;

	if (FolderRun && SeparateWindow==2)
	{
		AddEndSlash(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
	}
	else
	{
		FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

		if (/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
		{
			DWORD Error=0, dwSubSystem2=0;
			size_t pos;

			if (strNewCmdStr.RPos(pos,L'.'))
			{
				const wchar_t *ExtPtr=strNewCmdStr.CPtr()+pos;
				if (!(!StrCmpI(ExtPtr,L".exe") || !StrCmpI(ExtPtr,L".com") || IsBatchExtType(ExtPtr)))
				{
					lpVerb=GetShellAction(strNewCmdStr,dwSubSystem2,Error);

					if (lpVerb && Error != ERROR_NO_ASSOCIATION)
					{
						dwSubSystem=dwSubSystem2;
					}
				}

				if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN && !StrCmpNI(strNewCmdStr,L"ECHO.",5)) // вариант "echo."
				{
					strNewCmdStr.Replace(pos,1,L' ');
					PartCmdLine(strNewCmdStr,strNewCmdStr,strNewCmdPar);

					if (strNewCmdPar.IsEmpty())
						strNewCmdStr+=L'.';

					FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem);
				}
			}
		}

		if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
			SeparateWindow = 2;
	}

	if (SeparateWindow == 2)
	{
		SHELLEXECUTEINFO seInfo={sizeof(seInfo)};
		seInfo.lpFile = strNewCmdStr;
		seInfo.lpParameters = strNewCmdPar;
		seInfo.nShow = SW_SHOWNORMAL;
		string strCurDir;
		apiGetCurrentDirectory(strCurDir);
		seInfo.lpDirectory=strCurDir;
		seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?nullptr:lpVerb?lpVerb:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
		//seInfo.lpVerb = "open";
		seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS;


		if (!dwError)
		{
			if(!Silent && !FolderRun)
			{
				Console.ScrollScreenBuffer(2);
			}
			if (ShellExecuteEx(&seInfo))
			{
				hProcess = seInfo.hProcess;
				StartExecTime=clock();
			}
			else
			{
				dwError = GetLastError();
			}
		}
	}
	else
	{
		string strFarTitle;

		if (!Opt.ExecuteFullTitle)
		{
			strFarTitle=CmdStr;
		}
		else
		{
			strFarTitle = strNewCmdStr;

			if (!strNewCmdPar.IsEmpty())
			{
				strFarTitle += L" ";
				strFarTitle += strNewCmdPar;
			}
		}

		Console.SetTitle(strFarTitle);

		QuoteSpace(strNewCmdStr);
		strExecLine = strComspec;
		strExecLine += L" /C ";
		bool bDoubleQ = false;

		if (wcspbrk(strNewCmdStr, L"&<>()@^|=;, "))
			bDoubleQ = true;

		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
			strExecLine += L"\"";

		strExecLine += strNewCmdStr;

		if (!strNewCmdPar.IsEmpty())
		{
			strExecLine += L" ";
			strExecLine += strNewCmdPar;
		}

		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
			strExecLine += L"\"";

		// // попытка борьбы с синим фоном в 4NT при старте консоль
		SetRealColor(COL_COMMANDLINEUSERSCREEN);

		string strCurDir;
		apiGetCurrentDirectory(strCurDir);

		PROCESS_INFORMATION pi;
		STARTUPINFO si={sizeof(si)};

		if (SeparateWindow)
			si.lpTitle=(wchar_t*)strFarTitle.CPtr();

		if(!Silent && !FolderRun)
		{
			Console.ScrollScreenBuffer(1);
		}
		if (CreateProcess(
		            nullptr,
		            (wchar_t*)strExecLine.CPtr(),
		            nullptr,
		            nullptr,
		            false,
		            SeparateWindow?CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE:CREATE_DEFAULT_ERROR_MODE,
		            nullptr,
		            strCurDir,
		            &si,
		            &pi
		        ))
		{
			hProcess = pi.hProcess;
			hThread = pi.hThread;
			StartExecTime=clock();
		}
		else
		{
			dwError = GetLastError();
		}
	}

	DWORD ErrorCode=0;
	bool ErrMsg=false;

	if (!dwError)
	{
		if (hProcess)
		{
			ScrBuf.Flush();

			if (AlwaysWaitFinish || !SeparateWindow)
			{
				if (!Opt.ConsoleDetachKey)
				{
					WaitForSingleObject(hProcess,INFINITE);
					if(!Silent)
					{
						Console.ScrollScreenBuffer(Opt.ShowKeyBar?2:1);
					}
				}
				else
				{
					/*$ 12.02.2001 SKV
					  супер фитча ;)
					  Отделение фаровской консоли от неинтерактивного процесса.
					  Задаётся кнопкой в System/ConsoleDetachKey
					*/
					HANDLE hOutput = Console.GetOutputHandle();
					HANDLE hInput = Console.GetInputHandle();
					INPUT_RECORD ir[256];
					DWORD rd;
					int vkey=0,ctrl=0;
					TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,nullptr);
					int alt=ctrl&PKF_ALT;
					int shift=ctrl&PKF_SHIFT;
					ctrl=ctrl&PKF_CONTROL;
					bool bAlt, bShift, bCtrl;
					DWORD dwControlKeyState;

					//Тут нельзя делать WaitForMultipleObjects из за бага в Win7 при работе в телнет
					while (WaitForSingleObject(hProcess, 100) != WAIT_OBJECT_0)
					{
						if (WaitForSingleObject(hInput, 100)==WAIT_OBJECT_0 && Console.PeekInput(*ir, 256, rd) && rd)
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
										HICON hSmallIcon=nullptr,hLargeIcon=nullptr;
										HWND hWnd = Console.GetWindow();

										if (hWnd)
										{
											hSmallIcon = CopyIcon((HICON)SendMessage(hWnd,WM_SETICON,0,(LPARAM)0));
											hLargeIcon = CopyIcon((HICON)SendMessage(hWnd,WM_SETICON,1,(LPARAM)0));
										}

										Console.ReadInput(*ir, 256, rd);
										/*
										  Не будем вызыват CloseConsole, потому, что она поменяет
										  ConsoleMode на тот, что был до запуска Far'а,
										  чего работающее приложение могло и не ожидать.
										*/
										CloseHandle(hInput);
										CloseHandle(hOutput);
										delete KeyQueue;
										KeyQueue=nullptr;
										Console.Free();
										Console.Allocate();

										if (hWnd)   // если окно имело HOTKEY, то старое должно его забыть.
											SendMessage(hWnd,WM_SETHOTKEY,0,(LPARAM)0);

										Console.SetSize(ConsoleSize);
										Console.SetWindowRect(ConsoleWindowRect);
										Console.SetSize(ConsoleSize);
										Sleep(100);
										InitConsole(0);

										hWnd = Console.GetWindow();

										if (hWnd)
										{
											if (Opt.SmallIcon)
											{
												ExtractIconEx(g_strFarModuleName,0,&hLargeIcon,&hSmallIcon,1);
											}

											if (hLargeIcon )
												SendMessage(hWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

											if (hSmallIcon )
												SendMessage(hWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
										}

										stop=1;
										break;
									}
								}
							}

							if (stop)
								break;
						}
					}
					if(!Silent)
					{
						Console.ScrollScreenBuffer(Opt.ShowKeyBar?2:1);
					}
				}
			}
			if(WaitForIdle)
			{
				WaitForInputIdle(hProcess, INFINITE);
			}
			CloseHandle(hProcess);
		}

		if (hThread)
			CloseHandle(hThread);

		nResult = 0;
	}
	else
	{

		if (Opt.ExecuteShowErrorMessage)
		{
			ErrorCode=GetLastError();
			ErrMsg=true;
		}
		else
		{
			string strOutStr;
			strOutStr.Format(MSG(MExecuteErrorMessage),strNewCmdStr.CPtr());
			string strPtrStr=FarFormatText(strOutStr,ScrX,strPtrStr,L"\n",0);
			Console.Write(strPtrStr, static_cast<DWORD>(strPtrStr.GetLength()));
		}
	}

	ScrBuf.FillBuf();

	if(!Silent && !FolderRun)
	{
		CtrlObject->CmdLine->SaveBackground();
		ProcessShowClock--;
	}

	if(ErrMsg)
	{
		SetLastError(ErrorCode);
		SetMessageHelp(L"ErrCannotExecute");
		string strOutStr = strNewCmdStr;
		Unquote(strOutStr);
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),strOutStr,MSG(MOk));
	}

	SetFarConsoleMode(TRUE);
	/* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
	    вследствие своей оптимизации, которая в данном случае выходит боком.
	*/
	SetCursorType(Visible,Size);
	SetRealCursorType(Visible,Size);

	COORD ConSize;
	Console.GetSize(ConSize);
	if(ConSize.X!=ScrX+1 || ConSize.Y!=ScrY+1)
	{
		ChangeVideoMode(ConSize.Y, ConSize.X);
	}

	if (Opt.RestoreCPAfterExecute)
	{
		// восстановим CP-консоли после исполнения проги
		Console.SetInputCodepage(ConsoleCP);
		Console.SetOutputCodepage(ConsoleOutputCP);
	}

	return nResult;
}


int CommandLine::CmdExecute(const wchar_t *CmdLine,int AlwaysWaitFinish,int SeparateWindow,int DirectRun, bool WaitForIdle, bool Silent)
{
	LastCmdPartLength=-1;

	if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
	{
		/* $ 12.05.2001 DJ - рисуемся только если остались верхним фреймом */
		if (CtrlObject->Cp()->IsTopFrame())
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			FS<<fmt::Width(X2-X1+1)<<L"";
			Show();
			ScrBuf.Flush();
		}

		return -1;
	}

	int Code;
	COORD Size0;
	Console.GetSize(Size0);

	if (!strCurDir.IsEmpty() && strCurDir.At(1)==L':')
		FarChDir(strCurDir);

	string strPrevDir=strCurDir;
	bool PrintCommand=true;
	if ((Code=ProcessOSCommands(CmdLine,SeparateWindow,PrintCommand)) == TRUE)
	{
		ShowBackground();

		string strNewDir=strCurDir;
		strCurDir=strPrevDir;
		Redraw();
		strCurDir=strNewDir;

		if (PrintCommand)
		{
			GotoXY(X2+1,Y1);
			Text(L" ");
			ScrollScreen(2);
		}

		SetString(L"", FALSE);
		SaveBackground();
		Code=-1;
	}
	else
	{
		string strTempStr;
		strTempStr = CmdLine;

		if (Code == -1)
			ReplaceStrings(strTempStr,L"/",L"\\",-1);

		Code=Execute(strTempStr,AlwaysWaitFinish,SeparateWindow,DirectRun, 0, WaitForIdle, Silent);
	}

	COORD Size1;
	Console.GetSize(Size1);

	if (Size0.X != Size1.X || Size0.Y != Size1.Y)
	{
		GotoXY(X2+1,Y1);
		Text(L" ");
		CtrlObject->CmdLine->CorrectRealScreenCoord();
	}

	if (!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
	{
		ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
		if (Opt.ShowKeyBar)
		{
			CtrlObject->MainKeyBar->Show();
		}
	}
	ScrBuf.Flush();
	return(Code);
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
const wchar_t *PrepareOSIfExist(const wchar_t *CmdLine)
{
	if (!CmdLine || !*CmdLine)
		return nullptr;

	string strCmd;
	string strExpandedStr;
	const wchar_t *PtrCmd=CmdLine, *CmdStart;
	int Not=FALSE;
	int Exist=0; // признак наличия конструкции "IF [NOT] EXIST filename command"
	// > 0 - эсть такая конструкция

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
		if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd,L"IF ",3))
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
				strCmd.Copy(CmdStart,PtrCmd-CmdStart);
				Unquote(strCmd);

//_SVS(SysLog(L"Cmd='%s'", strCmd.CPtr()));
				if (apiExpandEnvironmentStrings(strCmd,strExpandedStr))
				{
					string strFullPath;

					if (!(strCmd.At(1) == L':' || (strCmd.At(0) == L'\\' && strCmd.At(1)==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
					{
						if (CtrlObject)
							CtrlObject->CmdLine->GetCurDir(strFullPath);
						else
							apiGetCurrentDirectory(strFullPath);

						AddEndSlash(strFullPath);
					}

					strFullPath += strExpandedStr;
					DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

					if (wcspbrk(strExpandedStr.CPtr()+(HasPathPrefix(strExpandedStr)?4:0), L"*?")) // это маска?
					{
						FAR_FIND_DATA_EX wfd;

						if (apiGetFindDataEx(strFullPath, wfd))
							FileAttr=wfd.dwFileAttributes;
					}
					else
					{
						ConvertNameToFull(strFullPath, strFullPath);
						FileAttr=apiGetFileAttributes(strFullPath);
					}

//_SVS(SysLog(L"%08X FullPath=%s",FileAttr,FullPath));
					if ((FileAttr != INVALID_FILE_ATTRIBUTES && !Not) || (FileAttr == INVALID_FILE_ATTRIBUTES && Not))
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
					strCmd.Copy(CmdStart,PtrCmd-CmdStart);

					DWORD ERet=apiGetEnvironmentVariable(strCmd,strExpandedStr);

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

int CommandLine::ProcessOSCommands(const wchar_t *CmdLine, int SeparateWindow, bool &PrintCommand)
{
	int Length;
	string strCmdLine = CmdLine;
	Panel *SetPanel=CtrlObject->Cp()->ActivePanel;
	PrintCommand=true;

	if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	RemoveTrailingSpaces(strCmdLine);
	bool SilentInt=false;

	if (*CmdLine == L'@')
	{
		SilentInt=true;
		strCmdLine.LShift(1);
	}

	if (!SeparateWindow && strCmdLine.At(0) && strCmdLine.At(1)==L':' && !strCmdLine.At(2))
	{
		if(!FarChDir(strCmdLine))
		{
			wchar_t NewDir[]={Upper(strCmdLine.At(0)),L':',L'\\',0};
			{
				FarChDir(NewDir);
			}
		}
		SetPanel->ChangeDirToCurrent();
		return TRUE;
	}
	// SET [переменная=[строка]]
	else if (!StrCmpNI(strCmdLine,L"SET",3) && IsSpaceOrEos(strCmdLine.At(3)))
	{
		size_t pos;
		strCmdLine.LShift(3);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine) || strCmdLine.IsEmpty())
			return FALSE; // отдадимся COMSPEC`у

		if (CheckCmdLineForSet(strCmdLine)) // вариант для /A и /P
			return FALSE;

		if (!strCmdLine.Pos(pos,L'='))
			return FALSE;

		if (strCmdLine.GetLength() == pos+1) //set var=
		{
			strCmdLine.SetLength(pos);
			SetEnvironmentVariable(strCmdLine,nullptr);
		}
		else
		{
			string strExpandedStr;

			if (apiExpandEnvironmentStrings(strCmdLine.CPtr()+pos+1,strExpandedStr) )
			{
				strCmdLine.SetLength(pos);
				SetEnvironmentVariable(strCmdLine,strExpandedStr);
			}
		}

		return TRUE;
	}
	// REM все остальное
	else if ((!StrCmpNI(strCmdLine,L"REM",Length=3) && IsSpaceOrEos(strCmdLine.At(3))) || !StrCmpNI(strCmdLine,L"::",Length=2))
	{
		if (Length == 3 && CheckCmdLineForHelp(strCmdLine.CPtr()+Length))
			return FALSE; // отдадимся COMSPEC`у

		return TRUE;
	}
	else if (!StrCmpNI(strCmdLine,L"CLS",3) && IsSpaceOrEos(strCmdLine.At(3)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+3))
			return FALSE; // отдадимся COMSPEC`у

		ClearScreen(COL_COMMANDLINEUSERSCREEN);
		SaveBackground();
		PrintCommand=false;
		return TRUE;
	}
	// PUSHD путь | ..
	else if (!StrCmpNI(strCmdLine,L"PUSHD",5) && IsSpaceOrEos(strCmdLine.At(5)))
	{
		strCmdLine.LShift(5);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine))
			return FALSE; // отдадимся COMSPEC`у

		PushPopRecord prec;
		prec.strName = strCurDir;

		if (IntChDir(strCmdLine,true,SilentInt))
		{
			ppstack.Push(prec);
			SetEnvironmentVariable(L"FARDIRSTACK",prec.strName);
		}
		else
		{
			;
		}

		return TRUE;
	}
	// POPD
	// TODO: добавить необязательный параметр - число, сколько уровней пропустить, после чего прыгнуть.
	else if (!StrCmpNI(CmdLine,L"POPD",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+4))
			return FALSE; // отдадимся COMSPEC`у

		PushPopRecord prec;

		if (ppstack.Pop(prec))
		{
			int Ret=IntChDir(prec.strName,true,SilentInt);
			PushPopRecord *ptrprec=ppstack.Peek();
			SetEnvironmentVariable(L"FARDIRSTACK",(ptrprec?ptrprec->strName.CPtr():nullptr));
			return Ret;
		}

		return TRUE;
	}
	// CLRD
	else if (!StrCmpI(CmdLine,L"CLRD"))
	{
		ppstack.Free();
		SetEnvironmentVariable(L"FARDIRSTACK",nullptr);
		return TRUE;
	}
	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	else if (!StrCmpNI(strCmdLine,L"CHCP",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		strCmdLine.LShift(4);

		const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine);

		if (CheckCmdLineForHelp(Ptr))
			return FALSE; // отдадимся COMSPEC`у

		if (!iswdigit(*Ptr))
			return FALSE;

		wchar_t Chr;

		while ((Chr=*Ptr) )
		{
			if (!iswdigit(Chr))
				break;

			++Ptr;
		}

		wchar_t *Ptr2;
		UINT cp=(UINT)wcstol(strCmdLine,&Ptr2,10); //BUGBUG
		BOOL r1=Console.SetInputCodepage(cp);
		BOOL r2=Console.SetOutputCodepage(cp);

		if (r1 && r2) // Если все ОБИ, то так  и...
		{
			InitRecodeOutTable(cp);
			LocalUpperInit();
			InitLCIDSort();
			InitKeysArray();
			ScrBuf.ResetShadow();
			ScrBuf.Flush();
			return TRUE;
		}
		else  // про траблы внешняя chcp сама скажет ;-)
		{
			return FALSE;
		}
	}
	else if (!StrCmpNI(strCmdLine,L"IF",2) && IsSpaceOrEos(strCmdLine.At(2)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+2))
			return FALSE; // отдадимся COMSPEC`у

		const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
		// здесь PtrCmd - уже готовая команда, без IF

		if (PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			FS<<fmt::Width(X2-X1+1)<<L"";
			Show();
			return TRUE;
		}

		return FALSE;
	}
	// пропускаем обработку, если нажат Shift-Enter
	else if (!SeparateWindow && (!StrCmpNI(strCmdLine,L"CD",Length=2) || !StrCmpNI(strCmdLine,L"CHDIR",Length=5)))
	{
		if (!IsSpaceOrEos(strCmdLine.At(Length)))
		{
			if (!IsSlash(strCmdLine.At(Length)))
				return FALSE;
		}

		strCmdLine.LShift(Length);
		RemoveLeadingSpaces(strCmdLine);

		//проигнорируем /D
		//мы и так всегда меняем диск а некоторые в алайсах или по привычке набирают этот ключ
		if (!StrCmpNI(strCmdLine,L"/D",2) && IsSpaceOrEos(strCmdLine.At(2)))
		{
			strCmdLine.LShift(2);
			RemoveLeadingSpaces(strCmdLine);
		}

		if (strCmdLine.IsEmpty() || CheckCmdLineForHelp(strCmdLine))
			return FALSE; // отдадимся COMSPEC`у

		IntChDir(strCmdLine,Length==5,SilentInt);
		return TRUE;
	}
	else if (!StrCmpNI(strCmdLine,L"EXIT",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+4))
			return FALSE; // отдадимся COMSPEC`у

		FrameManager->ExitMainLoop(FALSE);
		return TRUE;
	}

	return FALSE;
}

bool CommandLine::CheckCmdLineForHelp(const wchar_t *CmdLine)
{
	if (CmdLine && *CmdLine)
	{
		while (IsSpace(*CmdLine))
			CmdLine++;

		if (*CmdLine && (CmdLine[0] == L'/' || CmdLine[0] == L'-') && CmdLine[1] == L'?')
			return true;
	}

	return false;
}

bool CommandLine::CheckCmdLineForSet(const string& CmdLine)
{
	if (CmdLine.GetLength()>1 && CmdLine.At(0)==L'/' && IsSpaceOrEos(CmdLine.At(2)))
		return true;

	return false;
}

BOOL CommandLine::IntChDir(const wchar_t *CmdLine,int ClosePlugin,bool Selent)
{
	Panel *SetPanel;
	SetPanel=CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir(CmdLine);
	Unquote(strExpandedDir);
	apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && ((!strExpandedDir.At(1) && apiGetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES) || IsSlash(strExpandedDir.At(1))))
	{
		string strTemp;
		GetRegKey(strSystemExecutor,L"~",strTemp,g_strFarPath);

		if (strExpandedDir.At(1))
		{
			AddEndSlash(strTemp);
			strTemp += strExpandedDir.CPtr()+2;
		}

		DeleteEndSlash(strTemp);
		strExpandedDir=strTemp;
	}

	if (wcspbrk(&strExpandedDir[HasPathPrefix(strExpandedDir)?4:0],L"?*")) // это маска?
	{
		FAR_FIND_DATA_EX wfd;

		if (apiGetFindDataEx(strExpandedDir, wfd))
		{
			size_t pos;

			if (FindLastSlash(pos,strExpandedDir))
				strExpandedDir.SetLength(pos+1);
			else
				strExpandedDir.Clear();

			strExpandedDir += wfd.strFileName;
		}
	}

	/* $ 15.11.2001 OT
		Сначала проверяем есть ли такая "обычная" директория.
		если уж нет, то тогда начинаем думать, что это директория плагинная
	*/
	DWORD DirAtt=apiGetFileAttributes(strExpandedDir);

	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && IsAbsolutePath(strExpandedDir))
	{
		ReplaceSlashToBSlash(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,TRUE);
		return TRUE;
	}

	/* $ 20.09.2002 SKV
	  Это отключает возможность выполнять такие команды как:
	  cd net:server и cd ftp://server/dir
	  Так как под ту же гребёнку попадают и
	  cd s&r:, cd make: и т.д., которые к смене
	  каталога не имеют никакого отношения.
	*/
	/*
	if (CtrlObject->Plugins.ProcessCommandLine(ExpandedDir))
	{
	  //CmdStr.SetString(L"");
	  GotoXY(X1,Y1);
	  FS<<fmt::Width(X2-X1+1)<<L"";
	  Show();
	  return TRUE;
	}
	*/
	strExpandedDir.ReleaseBuffer();

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePlugin);
		return TRUE;
	}

	if (FarChDir(strExpandedDir))
	{
		SetPanel->ChangeDirToCurrent();

		if (!SetPanel->IsVisible())
			SetPanel->SetTitle();
	}
	else
	{
		if (!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir,MSG(MOk));

		return FALSE;
	}

	return TRUE;
}

// Проверить "Это батник?"
bool IsBatchExtType(const wchar_t *ExtPtr)
{
	UserDefinedList BatchExtList;
	BatchExtList.Set(Opt.strExecuteBatchType);

	while (!BatchExtList.IsEmpty())
	{
		if (!StrCmpI(ExtPtr,BatchExtList.GetNext()))
			return true;
	}

	return false;
}

bool ProcessOSAliases(string &strStr)
{
	string strNewCmdStr;
	string strNewCmdPar;

	PartCmdLine(strStr,strNewCmdStr,strNewCmdPar);

	const wchar_t *lpwszExeName=PointToName(g_strFarModuleName);
	int nSize=(int)strNewCmdStr.GetLength()+4096;
	wchar_t* lpwszNewCmdStr=strNewCmdStr.GetBuffer(nSize);
	int ret=Console.GetAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),lpwszExeName);

	if (!ret)
	{
		string strComspec;
		if (apiGetEnvironmentVariable(L"COMSPEC",strComspec))
		{
			lpwszExeName=PointToName(strComspec);
			ret=Console.GetAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),lpwszExeName);
		}
	}

	strNewCmdStr.ReleaseBuffer();

	if (!ret)
		return false;

	if (!ReplaceStrings(strNewCmdStr,L"$*",strNewCmdPar))
		strNewCmdStr+=L" "+strNewCmdPar;

	strStr=strNewCmdStr;

	return true;
}
