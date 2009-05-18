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

static const wchar_t strSystemExecutor[]=L"System\\Executor";

// Выдранный кусок из будущего GetFileInfo, получаем достоверную информацию о ГУЯХ PE-модуля

// Возвращаем константы IMAGE_SUBSYSTEM_* дабы консоль отличать
// При выходе из процедуры IMAGE_SUBSYTEM_UNKNOWN означает
// "файл не является исполняемым".
// Для DOS-приложений определим еще одно значение флага.
#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

static int IsCommandPEExeGUI(const wchar_t *FileName,DWORD& ImageSubsystem)
{
  //_SVS(CleverSysLog clvrSLog(L"IsCommandPEExeGUI()"));
  //_SVS(SysLog(L"Param: FileName='%s'",FileName));
  HANDLE hFile;
  int Ret=FALSE;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if((hFile=apiCreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0)) != INVALID_HANDLE_VALUE)
  {
    DWORD ReadSize;
    IMAGE_DOS_HEADER dos_head;

    BOOL RetReadFile=ReadFile(hFile,&dos_head,sizeof(IMAGE_DOS_HEADER),&ReadSize,NULL);

    if(RetReadFile && dos_head.e_magic == IMAGE_DOS_SIGNATURE)
    {
      Ret=TRUE;
      ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
      /*  Если значение слова по смещению 18h (OldEXE - MZ) >= 40h,
      то значение слова в 3Ch является смещением заголовка Windows. */
      /* 31.07.2003 VVM
        ! Перерыл весь MSDN - этого условия не нашел */
//      if (dos_head.e_lfarlc >= 0x40)
      {
        DWORD signature;
        #include <pshpack1.h>
        struct __HDR
        {
           DWORD signature;
           IMAGE_FILE_HEADER _head;
           union
           {
             IMAGE_OPTIONAL_HEADER32 opt_head32;
             IMAGE_OPTIONAL_HEADER64 opt_head64;
           };
           // IMAGE_SECTION_HEADER section_header[];  /* actual number in NumberOfSections */
        } header, *pheader;
        #include <poppack.h>

				if(apiSetFilePointerEx(hFile,dos_head.e_lfanew,NULL,FILE_BEGIN))
        {
          // читаем очередной заголовок
          if(ReadFile(hFile,&header,sizeof(struct __HDR),&ReadSize,NULL))
          {
            signature=header.signature;
            pheader=&header;

            if(signature == IMAGE_NT_SIGNATURE) // PE
            {
               if (header.opt_head32.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
                 ImageSubsystem = header.opt_head64.Subsystem;
               else
                 ImageSubsystem = header.opt_head32.Subsystem;
            }
//            {
//              IsPEGUI=1;
//              IsPEGUI|=(header.opt_head.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)?2:0;
//            }
            else if((WORD)signature == IMAGE_OS2_SIGNATURE) // NE
            {
              /*
                 NE,  хмм...  а как определить что оно ГУЕВОЕ?

                 Andrzej Novosiolov <andrzej@se.kiev.ua>
                 AN> ориентироваться по флагу "Target operating system" NE-заголовка
                 AN> (1 байт по смещению 0x36). Если там Windows (значения 2, 4) - подразумеваем
                 AN> GUI, если OS/2 и прочая экзотика (остальные значения) - подразумеваем консоль.
              */
              BYTE ne_exetyp=((IMAGE_OS2_HEADER *)pheader)->ne_exetyp;
              if(ne_exetyp == 2 || ne_exetyp == 4)
                ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_GUI;
            }
          }
          else
          {
            ; // обломс вышел с чтением следующего заголовка ;-(
          }
        }
        else
        {
          ; // видимо улетиели куда нить в трубу, т.к. dos_head.e_lfanew
            // указал слишком в неправдоподное место (например это чистой
            // воды DOS-файл
        }
      }
/*      else
      {
        ; // Это конечно EXE, но не виндовое EXE
      }
*/
    }
    else
    {
      if(!RetReadFile)
        ; // ошибка чтения
      else
        ; // это не исполняемый файл - у него нету заголовка MZ, например, NLM-модуль
        // TODO: здесь можно разбирать POSIX нотацию, например "/usr/bin/sh"
    }
    CloseHandle(hFile);
  }

  return Ret;
}

bool GetShellType(const wchar_t *Ext, string &strType,ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;

	strType.SetLength(0);

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
		HKEY hKey;
		if (RegOpenKeyW(HKEY_CLASSES_ROOT,Ext,&hKey)!=ERROR_SUCCESS)
			return false;
		if(aType==AT_URLPROTOCOL)
		{
			strType=Ext;
		}
		else
		{
			if (RegQueryStringValue(hKey,L"",strType,L"")!=ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				return false;
			}
		}
		RegCloseKey(hKey);
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

  if ((ExtPtr=wcsrchr(FileName,L'.'))==NULL)
    return(NULL);

  if (!GetShellType(ExtPtr, strValue))
  	return NULL;

	HKEY hKey;
	if(RegOpenKeyExW(HKEY_CLASSES_ROOT,(const wchar_t *)strValue,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		int nResult=RegQueryValueExW(hKey,L"IsShortcut",NULL,NULL,NULL,NULL);
		RegCloseKey(hKey);
		if(nResult==ERROR_SUCCESS)
			return NULL;
	}

  strValue += L"\\shell";

  if (RegOpenKeyW(HKEY_CLASSES_ROOT,(const wchar_t *)strValue,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static string strAction;

  int RetQuery = RegQueryStringValueEx(hKey,L"",strAction,L"");

  strValue += L"\\";

  if (RetQuery == ERROR_SUCCESS)
  {
    UserDefinedList ActionList(0,0,ULF_UNIQUE);

    RetPtr = (strAction.IsEmpty() ? NULL : (const wchar_t *)strAction);
    const wchar_t *ActionPtr;

    LONG RetEnum = ERROR_SUCCESS;
    if (RetPtr != NULL && ActionList.Set(strAction))
    {
      HKEY hOpenKey;

      ActionList.Reset();
      while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) != NULL)
      {
        strNewValue = strValue;
        strNewValue += ActionPtr;
        strNewValue += command_action;

        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strValue += ActionPtr;
          strAction = ActionPtr;
          RetPtr = (const wchar_t *)strAction;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* while */
    } /* if */
    else
      strValue += strAction;

    if(RetEnum != ERROR_NO_MORE_ITEMS) // Если ничего не нашли, то...
      RetPtr=NULL;
  }
  else
  {
    // This member defaults to "Open" if no verb is specified.
    // Т.е. если мы вернули NULL, то подразумевается команда "Open"
    RetPtr=NULL;
  }

  // Если RetPtr==NULL - мы не нашли default action.
  // Посмотрим - есть ли вообще что-нибудь у этого расширения
  if (RetPtr==NULL)
  {
    LONG RetEnum = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    HKEY hOpenKey;

    // Сначала проверим "open"...
    strAction = L"open";

    strNewValue = strValue;
    strNewValue += strAction;
    strNewValue += command_action;

    if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
    {
      RegCloseKey(hOpenKey);
      strValue += strAction;
      RetPtr = (const wchar_t *)strAction;
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
        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strValue += strAction;
          RetPtr = (const wchar_t *)strAction;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* if */
    } /* while */
  } /* if */

  RegCloseKey(hKey);

  if (RetPtr != NULL)
  {
    strValue += command_action;

    // а теперь проверим ГУЕвость запускаемой проги
    if (RegOpenKeyW(HKEY_CLASSES_ROOT,strValue,&hKey)==ERROR_SUCCESS)
    {
      RetQuery=RegQueryStringValueEx(hKey,L"",strNewValue,L"");
      RegCloseKey(hKey);

      if(RetQuery == ERROR_SUCCESS && !strNewValue.IsEmpty())
      {
        apiExpandEnvironmentStrings(strNewValue,strNewValue);

        wchar_t *Ptr = strNewValue.GetBuffer ();
        // Выделяем имя модуля
        if (*Ptr==L'\"')
        {
          wchar_t *QPtr = wcschr(Ptr + 1,L'\"');
          if (QPtr!=NULL)
          {
            *QPtr=0;
            wmemmove(Ptr, Ptr + 1, QPtr-Ptr);
          }
        }
        else
        {
          if ((Ptr=wcspbrk(Ptr,L" \t/"))!=NULL)
            *Ptr=0;
        }

        strNewValue.ReleaseBuffer ();

        IsCommandPEExeGUI(strNewValue,ImageSubsystem);
      }
      else
      {
        Error=ERROR_NO_ASSOCIATION;
        RetPtr=NULL;
      }
    }
  }

  return RetPtr;
}

/*
 Фунция PrepareExecuteModule пытается найти исполняемый модуль (в т.ч. и по
 %PATHEXT%). В случае успеха заменяет в Command порцию, ответственную за
 исполянемый модуль на найденное значение, копирует результат в Dest и
 пытается проверить заголовок PE на ГУЕВОСТЬ (чтобы запустить процесс
 в отдельном окне и не ждать завершения).
 В случае неудачи Dest не заполняется!
 Return: TRUE/FALSE - нашли/не нашли
*/
/* $ 14.06.2002 VVM
 Команда в функцию передается уже разкавыченная. Ничего не меняем.
 И подменять ничего не надо, т.к. все параметры мы отсекли раньше
*/
int WINAPI PrepareExecuteModule(const wchar_t *Command, string &strDest,DWORD& ImageSubsystem)
{
  int Ret;
  wchar_t *Ptr;

  string strCommand = Command;

  string strFileName;
  string strFullName;

  // Здесь порядок важен! Сначала батники,  а потом остальная фигня.
  static wchar_t StdExecuteExt[NM]=L".BAT;.CMD;.EXE;.COM;";
  static const wchar_t RegPath[]=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
  static int PreparePrepareExt=FALSE;

  if(!PreparePrepareExt) // самоинициилизирующийся кусок
  {
    // если переменная %PATHEXT% доступна...
    if(apiGetEnvironmentVariable(L"PATHEXT",strFullName) != 0)
    {
      static wchar_t const * const StdExecuteExt0[4]={L".BAT;",L".CMD;",L".EXE;",L".COM;"};
			for(size_t I=0; I < countof(StdExecuteExt0); ++I)
        ReplaceStrings(strFullName,StdExecuteExt0[I],L"",-1);
    }

    strFullName += ";";

    Ptr=wcscat(StdExecuteExt, strFullName);  //BUGBUG
    StdExecuteExt[StrLength(StdExecuteExt)]=0;
    while(*Ptr)
    {
      if(*Ptr == L';')
        *Ptr=0;
      ++Ptr;
    }
    PreparePrepareExt=TRUE;
  }

  /* Берем "исключения" из реестра, которые должны исполянться директом,
     например, некоторые внутренние команды ком.процессора.
  */
  static wchar_t ExcludeCmds[4096]={0};
  static int PrepareExcludeCmds=FALSE;
  if(GetRegKey(strSystemExecutor,L"Type",0))
  {
    if (!PrepareExcludeCmds)
    {
      GetRegKey(strSystemExecutor,L"ExcludeCmds",(PBYTE)ExcludeCmds,(PBYTE)L"",sizeof(ExcludeCmds));
      Ptr=wcscat(ExcludeCmds,L";"); //!!!
      ExcludeCmds[StrLength(ExcludeCmds)]=0;
      while(*Ptr)
      {
        if(*Ptr == L';')
          *Ptr=0;
        ++Ptr;
      }
      PrepareExcludeCmds=TRUE;
    }
  }
  else
  {
    *ExcludeCmds=0;
    PrepareExcludeCmds=FALSE;
  }

  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN; // GUIType всегда вначале инициализируется в FALSE
  Ret=FALSE;

  if( strCommand.IsEmpty() ) // вот же, надо же... пустышку передали :-(
    return 0;

  strFileName = strCommand;

  // нулевой проход - смотрим исключения
  {
    wchar_t *Ptr=ExcludeCmds;
    while(*Ptr)
    {
      if(!StrCmpI(strFileName,Ptr))
      {
        ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
        return TRUE;
      }
      Ptr+=StrLength(Ptr)+1;
    }
  }


  {
    wchar_t *lpwszFilePart;

    strFullName = strFileName;

    const wchar_t *PtrFName = PointToName(strFullName);
    PtrFName = wcsrchr (PtrFName, L'.');

    string strWorkName;

    wchar_t *PtrExt=StdExecuteExt;
    while(*PtrExt) // первый проход - в текущем каталоге
    {
      strWorkName = strFullName;

      if (!PtrFName)
        strWorkName += PtrExt;

			DWORD dwFileAttr = apiGetFileAttributes(strWorkName);
			if ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
      {
        ConvertNameToFull (strWorkName, strFullName);
        Ret=TRUE;
        break;
      }
      PtrExt+=StrLength(PtrExt)+1;
    }

    if(!Ret) // второй проход - по правилам SearchPath
    {
      string strPathEnv;
      if (apiGetEnvironmentVariable(L"PATH",strPathEnv) != 0)
      {
        PtrExt=StdExecuteExt;

        DWORD dwSize = 0;

        while(*PtrExt)
        {
          if ( (dwSize = SearchPathW (strPathEnv, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

						SearchPathW(strPathEnv,string(lpwszFullName),PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=StrLength(PtrExt)+1;
        }
      }

      DWORD dwSize;

      if (!Ret)
      {
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          if ( (dwSize = SearchPathW (NULL, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

						SearchPathW(NULL,string(lpwszFullName),PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=StrLength(PtrExt)+1;
        }
      }

      if (!Ret && Opt.ExecuteUseAppPath) // третий проход - лезим в реестр в "App Paths"
      {
        // В строке Command заменть исполняемый модуль на полный путь, который
        // берется из SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
        // Сначала смотрим в HKCU, затем - в HKLM
        HKEY hKey;
        HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};

        strFullName = RegPath+strFileName;

				for (size_t I=0; I < countof(RootFindKey); ++I)
        {
          if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
          {
            RegQueryStringValueEx(hKey, L"", strFullName, L"");

            RegCloseKey(hKey);
            /* $ 03.10.2001 VVM Обработать переменные окружения */
            strFileName = strFullName;
            apiExpandEnvironmentStrings(strFileName, strFullName);
            Unquote(strFullName);
            Ret=TRUE;
            break;
          }
        }

        if (!Ret && Opt.ExecuteUseAppPath)
        {
          PtrExt=StdExecuteExt;

          while(*PtrExt && !Ret)
          {
            strFullName = RegPath+strFileName+PtrExt;

						for(size_t I=0; I < countof(RootFindKey); ++I)
            {
              if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
              {
                RegQueryStringValueEx(hKey, L"", strFullName, L"");

                RegCloseKey(hKey);
                /* $ 03.10.2001 VVM Обработать переменные окружения */
                strFileName = strFullName;
                apiExpandEnvironmentStrings(strFileName, strFullName);
                Unquote(strFullName);
                Ret=TRUE;
                break;
              }
            } /* for */
            PtrExt+=StrLength(PtrExt)+1;
          }
        } /* if */
      } /* if */
    }
  }

  if(Ret) // некоторые "подмены" данных
  {
    IsCommandPEExeGUI(strFullName,ImageSubsystem);

    strDest = strFullName;
  }

  return(Ret);
}

/* Функция-пускатель внешних процессов
   Возвращает -1 в случае ошибки или...
*/
int Execute(const wchar_t *CmdStr,    // Ком.строка для исполнения
            int AlwaysWaitFinish,  // Ждать завершение процесса?
            int SeparateWindow,    // Выполнить в отдельном окне? =2 для вызова ShellExecuteEx()
            int DirectRun,         // Выполнять директом? (без CMD)
            int FolderRun)         // Это фолдер?
{
  int nResult = -1;

  string strNewCmdStr;
  string strNewCmdPar;
  string strExecLine;


  PartCmdLine(
          CmdStr,
          strNewCmdStr,
          strNewCmdPar
          );

  /* $ 05.04.2005 AY: Это не правильно, надо убирать только первый пробел,
                      что теперь и делает PartCmdLine.
  if(*NewCmdPar)
    RemoveExternalSpaces(NewCmdPar);
  AY $ */

	DWORD dwAttr = apiGetFileAttributes(strNewCmdStr);

  if ( SeparateWindow == 1 )
  {
      if ( strNewCmdPar.IsEmpty() && dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
      {
          ConvertNameToFull(strNewCmdStr, strNewCmdStr);
          SeparateWindow=2;
          FolderRun=TRUE;
      }
  }

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (si));

  si.cb = sizeof (si);

  string strComspec;
  apiGetEnvironmentVariable (L"COMSPEC", strComspec);

  if ( strComspec.IsEmpty() && (SeparateWindow != 2) )
  {
    Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
    return -1;
  }

  int Visible, Size;

  GetCursorType(Visible,Size);
  SetInitialCursorType();

  ScrBuf.SetLockCount(0);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  int ConsoleCP = GetConsoleCP();
  int ConsoleOutputCP = GetConsoleOutputCP();

  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  GetConsoleScreenBufferInfo(hConOut,&sbi);

  wchar_t OldTitle[512];
	GetConsoleTitleW(OldTitle, countof(OldTitle));

  DWORD dwSubSystem;
  DWORD dwError = 0;

  HANDLE hProcess = NULL, hThread = NULL;

  if(FolderRun && SeparateWindow==2)
    AddEndSlash(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
  else
  {
    PrepareExecuteModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

    if(/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
    {
      DWORD Error=0, dwSubSystem2=0;
      const wchar_t *ExtPtr=wcsrchr(strNewCmdStr,L'.');

      if(ExtPtr && !(StrCmpI(ExtPtr,L".exe")==0 || StrCmpI(ExtPtr,L".com")==0 ||
         IsBatchExtType(ExtPtr)))
        if(GetShellAction(strNewCmdStr,dwSubSystem2,Error) && Error != ERROR_NO_ASSOCIATION)
          dwSubSystem=dwSubSystem2;
    }

    if ( dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
      SeparateWindow = 2;
  }

  ScrBuf.Flush ();

  if ( SeparateWindow == 2 )
  {
		SHELLEXECUTEINFOW seInfo;
		memset (&seInfo, 0, sizeof (seInfo));
		seInfo.cbSize = sizeof (seInfo);
    seInfo.lpFile = strNewCmdStr;
    seInfo.lpParameters = strNewCmdPar;
    seInfo.nShow = SW_SHOWNORMAL;

    seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?NULL:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
    //seInfo.lpVerb = "open";
		seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOZONECHECKS;

    if ( !dwError )
    {
      if ( ShellExecuteExW (&seInfo) )
      {
        hProcess = seInfo.hProcess;
        StartExecTime=clock();
      }
      else
        dwError = GetLastError ();
    }
  }
  else
  {
    string strFarTitle;
    if(!Opt.ExecuteFullTitle)
    {
      strFarTitle=CmdStr;
    }
    else
    {
      strFarTitle = strNewCmdStr;
      if ( !strNewCmdPar.IsEmpty() )
      {
        strFarTitle += L" ";
        strFarTitle += strNewCmdPar;
      }
    }
    SetConsoleTitleW(strFarTitle);

    if (SeparateWindow)
      si.lpTitle=(wchar_t*)(const wchar_t*)strFarTitle;

    QuoteSpace(strNewCmdStr);

    strExecLine = strComspec;
    strExecLine += L" /C ";

    bool bDoubleQ = false;

    if ( wcspbrk (strNewCmdStr, L"&<>()@^|=;,") )
      bDoubleQ = true;

    if ( !strNewCmdPar.IsEmpty() || bDoubleQ )
      strExecLine += L"\"";

    strExecLine += strNewCmdStr;

    if ( !strNewCmdPar.IsEmpty() )
    {
      strExecLine += L" ";
      strExecLine += strNewCmdPar;
    }

    if ( !strNewCmdPar.IsEmpty() || bDoubleQ )
      strExecLine += L"\"";

    // // попытка борьбы с синим фоном в 4NT при старте консоль
    SetRealColor (COL_COMMANDLINEUSERSCREEN);

    if ( CreateProcessW (
        NULL,
        (wchar_t*)(const wchar_t*)strExecLine,
        NULL,
        NULL,
        false,
        SeparateWindow?CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE:CREATE_DEFAULT_ERROR_MODE,
        NULL,
        NULL,
        &si,
        &pi
        ) )
     {
       hProcess = pi.hProcess;
       hThread = pi.hThread;

       StartExecTime=clock();
     }
    else
       dwError = GetLastError ();
  }

  if ( !dwError )
  {
    if ( hProcess )
    {
      ScrBuf.Flush ();

      if ( AlwaysWaitFinish || !SeparateWindow )
      {
        if ( Opt.ConsoleDetachKey == 0 )
          WaitForSingleObject(hProcess,INFINITE);
        else
        {
          /*$ 12.02.2001 SKV
            супер фитча ;)
            Отделение фаровской консоли от неинтерактивного процесса.
            Задаётся кнопкой в System/ConsoleDetachKey
          */
          HANDLE hHandles[2];
          HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
          HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

          INPUT_RECORD ir[256];
          DWORD rd;

          int vkey=0,ctrl=0;
          TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,NULL);
          int alt=ctrl&PKF_ALT;
          int shift=ctrl&PKF_SHIFT;
          ctrl=ctrl&PKF_CONTROL;

          hHandles[0] = hProcess;
          hHandles[1] = hInput;

          bool bAlt, bShift, bCtrl;
          DWORD dwControlKeyState;

          while( WaitForMultipleObjects (
              2,
              hHandles,
              FALSE,
              INFINITE
              ) != WAIT_OBJECT_0
              )
          {
            if ( PeekConsoleInputW(hHandles[1],ir,256,&rd) && rd)
            {
              int stop=0;

              for(DWORD i=0;i<rd;i++)
              {
                PINPUT_RECORD pir=&ir[i];

                if(pir->EventType==KEY_EVENT)
                {
                  dwControlKeyState = pir->Event.KeyEvent.dwControlKeyState;

                  bAlt = (dwControlKeyState & LEFT_ALT_PRESSED) || (dwControlKeyState & RIGHT_ALT_PRESSED);
                  bCtrl = (dwControlKeyState & LEFT_CTRL_PRESSED) || (dwControlKeyState & RIGHT_CTRL_PRESSED);
                  bShift = (dwControlKeyState & SHIFT_PRESSED) != 0;

                  if ( vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
                     (alt ?bAlt:!bAlt) &&
                     (ctrl ?bCtrl:!bCtrl) &&
                     (shift ?bShift:!bShift) )
                  {
                    HICON hSmallIcon=NULL,hLargeIcon=NULL;

                    if ( hFarWnd )
                    {
                      hSmallIcon = CopyIcon((HICON)SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)0));
                      hLargeIcon = CopyIcon((HICON)SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)0));
                    }

                    ReadConsoleInputW(hInput,ir,256,&rd);

                    /*
                      Не будем вызыват CloseConsole, потому, что она поменяет
                      ConsoleMode на тот, что был до запуска Far'а,
                      чего работающее приложение могло и не ожидать.
                    */
                    CloseHandle(hInput);
                    CloseHandle(hOutput);

                    delete KeyQueue;
                    KeyQueue=NULL;

                    FreeConsole();
                    AllocConsole();

                    if ( hFarWnd ) // если окно имело HOTKEY, то старое должно его забыть.
                      SendMessageW(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);

                    SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
                    SetConsoleWindowInfo(hOutput,TRUE,&sbi.srWindow);
                    SetConsoleScreenBufferSize(hOutput,sbi.dwSize);

                    Sleep(100);
                    InitConsole(0);

                    hFarWnd = 0;
                    InitDetectWindowedMode();

                    if ( hFarWnd )
                    {
                      if ( Opt.SmallIcon )
                      {
                        string strFarName;
                        apiGetModuleFileName (NULL, strFarName);
                        ExtractIconExW(strFarName,0,&hLargeIcon,&hSmallIcon,1);
                      }

                      if ( hLargeIcon != NULL )
                        SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

                      if ( hSmallIcon != NULL )
                        SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
                    }

                    stop=1;
                    break;
                  }
                }
              }

              if ( stop )
                break;
            }

            Sleep(100);
          }
        }
      }

//      MessageBox (0, "close", "asd", MB_OK);

      ScrBuf.FillBuf();

      CloseHandle (hProcess);
    }

     if ( hThread )
       CloseHandle (hThread);

    nResult = 0;
  }
  else
  {
    string strOutStr;

    if( Opt.ExecuteShowErrorMessage )
    {
      SetMessageHelp(L"ErrCannotExecute");

      strOutStr = strNewCmdStr;
      Unquote(strOutStr);

      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),strOutStr,MSG(MOk));
    }
    else
    {
      ScrBuf.Flush ();

      strOutStr.Format (MSG(MExecuteErrorMessage),(const wchar_t *)strNewCmdStr);
      string strPtrStr=FarFormatText(strOutStr,ScrX, strPtrStr,L"\n",0);

      wprintf(strPtrStr);

      ScrBuf.FillBuf();
    }

  }

  SetFarConsoleMode(TRUE);

  /* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
      вследствие своей оптимизации, которая в данном случае выходит боком.
  */
  SetCursorType(Visible,Size);
  SetRealCursorType(Visible,Size);

  SetConsoleTitleW(OldTitle);

  /* Если юзер выполнил внешнюю команду, например
     mode con lines=50 cols=100
     то ФАР не знал об изменении размера консоли.
     Для этого надо ФАРу напомнить лишний раз :-)
  */
  GenerateWINDOW_BUFFER_SIZE_EVENT(-1,-1); //бред...

  if( Opt.RestoreCPAfterExecute )
  {
    // восстановим CP-консоли после исполнения проги
    SetConsoleCP(ConsoleCP);
    SetConsoleOutputCP(ConsoleOutputCP);
  }


  return nResult;
}


int CommandLine::CmdExecute(const wchar_t *CmdLine,int AlwaysWaitFinish,
                            int SeparateWindow,int DirectRun)
{
  LastCmdPartLength=-1;

  if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
  {
    /* $ 12.05.2001 DJ
       рисуемся только если остались верхним фреймом
    */
    if (CtrlObject->Cp()->IsTopFrame())
    {
      //CmdStr.SetString(L"");
      GotoXY(X1,Y1);
      mprintf(L"%*s",X2-X1+1,L"");
      Show();
      ScrBuf.Flush();
    }
    return(-1);
  }
  int Code;
  /* 21.11.2001 VVM
    ! В очередной раз проблемы с прорисовкой фона.
      Вроде бы теперь полегче стало :) */
  {
    CONSOLE_SCREEN_BUFFER_INFO sbi0,sbi1;
    GetConsoleScreenBufferInfo(hConOut,&sbi0);
    {
      RedrawDesktop Redraw(TRUE);

      ScrollScreen(1);
      MoveCursor(X1,Y1);
      if ( !strCurDir.IsEmpty() && strCurDir.At(1)==L':')
        FarChDir(strCurDir);
      CmdStr.SetString(L"");
      if ((Code=ProcessOSCommands(CmdLine,SeparateWindow)) == TRUE)
        Code=-1;
      else
      {
        string strTempStr;
        strTempStr = CmdLine;
        if(Code == -1)
          ReplaceStrings(strTempStr,L"/",L"\\",-1);
        Code=Execute(strTempStr,AlwaysWaitFinish,SeparateWindow,DirectRun);
      }

      GetConsoleScreenBufferInfo(hConOut,&sbi1);
      if(!(sbi0.dwSize.X == sbi1.dwSize.X && sbi0.dwSize.Y == sbi1.dwSize.Y))
        CtrlObject->CmdLine->CorrectRealScreenCoord();

      //if(Code != -1)
      {
        int CurX,CurY;
        GetCursorPos(CurX,CurY);
        if (CurY>=Y1-1)
          ScrollScreen(Min(CurY-Y1+2,2/*Opt.ShowKeyBar ? 2:1*/));
      }
    }

    if(!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
      ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
    /*else
    {
      CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
      CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
      CtrlObject->Cp()->Redraw();
    }*/
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
            NULL - не попался "IF" или ошибки в предложении, например
                   не exist, а exist или предложение неполно.

   DEFINED - подобно EXIST, но оперирует с переменными среды

   Исходная строка (CmdLine) не модифицируется!!! - на что явно указывает const
                                                    IS 20.03.2002 :-)
*/
const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

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
    while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
  }

  while(1)
  {
    if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd,L"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (StrCmpNI(PtrCmd,L"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !StrCmpNI(PtrCmd,L"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
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

      if(PtrCmd && *PtrCmd && *PtrCmd == L' ')
      {
        {
          wchar_t *lpwszCmd = strCmd.GetBuffer(PtrCmd-CmdStart+1);
          wmemcpy(lpwszCmd,CmdStart,PtrCmd-CmdStart);
          strCmd.ReleaseBuffer(PtrCmd-CmdStart);
        }
        Unquote(strCmd);
//_SVS(SysLog(L"Cmd='%s'",(const wchar_t *)strCmd));
        if (apiExpandEnvironmentStrings(strCmd,strExpandedStr)!=0)
        {
          string strFullPath;
          if (!(strCmd.At(1) == L':' || (strCmd.At(0) == L'\\' && strCmd.At(1)==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(strFullPath);
            else
							apiGetCurrentDirectory(strFullPath);
            AddEndSlash(strFullPath);
          }
          strFullPath += strExpandedStr;
          DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
          if (wcspbrk((const wchar_t *)strExpandedStr+(PathPrefix(strExpandedStr)?4:0), L"*?")) // это маска?
          {
            FAR_FIND_DATA_EX wfd;

            if ( apiGetFindDataEx (strFullPath, &wfd) )
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
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return L"";
        }
      }
    }

    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !StrCmpNI(PtrCmd,L"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == L'"')
        PtrCmd=wcschr(PtrCmd+1,L'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=wcschr(PtrCmd,L' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == L' ')
        {
          {
            wchar_t *lpwszCmd = strCmd.GetBuffer(PtrCmd-CmdStart+1);
            wmemcpy(lpwszCmd,CmdStart,PtrCmd-CmdStart);
            strCmd.ReleaseBuffer(PtrCmd-CmdStart);
          }
          DWORD ERet=apiGetEnvironmentVariable(strCmd,strExpandedStr);
//_SVS(SysLog(Cmd));
          if ((ERet && !Not) || (!ERet && Not))
          {
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return L"";
        }
      }
    }
  }
  return Exist?PtrCmd:NULL;
}

int CommandLine::ProcessOSCommands(const wchar_t *CmdLine,int SeparateWindow)
{
  Panel *SetPanel;
  int Length;

  string strCmdLine = CmdLine;

  SetPanel=CtrlObject->Cp()->ActivePanel;

  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

  RemoveTrailingSpaces(strCmdLine);

  bool SilentInt=false;
  if(*CmdLine == L'@')
  {
    SilentInt=true;
    strCmdLine.LShift(1);
  }

	if (!SeparateWindow && strCmdLine.At(0) && strCmdLine.At(1)==L':' && strCmdLine.At(2)==0)
  {
    wchar_t NewDir[10];
    swprintf(NewDir,L"%c:",Upper(strCmdLine.At(0)));
    FarChDir(strCmdLine);
    if (getdisk()!=(int)(NewDir[0]-L'A'))
    {
      wcscat(NewDir,L"\\");
      FarChDir(NewDir);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }

  // SET [переменная=[строка]]
	else if (!StrCmpNI(strCmdLine,L"SET",3) && IsSpace(strCmdLine.At(3)))
  {
    size_t pos;
    string strCmd = (const wchar_t *)strCmdLine+4;

    if (!strCmd.Pos(pos,L'='))
      return FALSE;

    if (strCmd.GetLength() == pos+1) //set var=
    {
    	strCmd.SetLength(pos);
      SetEnvironmentVariableW(strCmd,NULL);
    }
    else
    {
      string strExpandedStr;

      if (apiExpandEnvironmentStrings((const wchar_t *)strCmd+pos+1,strExpandedStr) != 0)
      {
      	strCmd.SetLength(pos);
        SetEnvironmentVariableW(strCmd,strExpandedStr);
      }
    }

    return TRUE;
  }

	else if ((!StrCmpNI(strCmdLine,L"REM",3) && IsSpace(strCmdLine.At(3))) || !StrCmpNI(strCmdLine,L"::",2))
  {
    return TRUE;
  }

	else if (!StrCmpI(strCmdLine,L"CLS"))
  {
    ClearScreen(COL_COMMANDLINEUSERSCREEN);
    return TRUE;
  }

	// PUSHD путь | ..
	else if (!StrCmpNI(strCmdLine,L"PUSHD",5) && IsSpace(strCmdLine.At(5)))
	{
		string strCmd = (const wchar_t *)CmdLine+6;
		RemoveExternalSpaces(strCmd);

		if(!StrCmpI(strCmd,L"/?") || !StrCmpI(strCmd,L"-?"))
			return FALSE; // пусть cmd скажет про синтаксис

		PushPopRecord prec;
		prec.strName = strCurDir;
		if(IntChDir(strCmd,true,SilentInt))
		{
			ppstack.Push(prec);
			SetEnvironmentVariableW(L"FARDIRSTACK",prec.strName);
		}
		else
		{
			;
		}
		return TRUE;
	}

	// POPD
	else if (!StrCmpI(CmdLine,L"POPD"))
	{
		PushPopRecord prec;
		if(ppstack.Pop(prec))
		{
			int Ret=IntChDir(prec.strName,true,SilentInt);
			PushPopRecord *ptrprec=ppstack.Peek();
			SetEnvironmentVariableW(L"FARDIRSTACK",(ptrprec?ptrprec->strName.CPtr():NULL));
			return Ret;
		}
		return TRUE;
	}

	// CLRD
	else if (!StrCmpI(CmdLine,L"CLRD"))
	{
		ppstack.Free();
		SetEnvironmentVariableW(L"FARDIRSTACK",NULL);
		return TRUE;
	}

  /*
  Displays or sets the active code page number.
  CHCP [nnn]
    nnn   Specifies a code page number (Dec or Hex).
  Type CHCP without a parameter to display the active code page number.
  */
	else if (!StrCmpNI(strCmdLine,L"CHCP",4) && IsSpace(strCmdLine.At(4)))
  {
    strCmdLine = (const wchar_t*)strCmdLine+5;

    const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine);
    wchar_t Chr;

    if(!iswdigit(*Ptr))
      return FALSE;

    while((Chr=*Ptr) != 0)
    {
      if(!iswdigit(Chr))
        break;
      ++Ptr;
    }

    wchar_t *Ptr2;

    UINT cp=(UINT)wcstol((const wchar_t*)strCmdLine+5,&Ptr2,10); //BUGBUG
    BOOL r1=SetConsoleCP(cp);
    BOOL r2=SetConsoleOutputCP(cp);
    if(r1 && r2) // Если все ОБИ, то так  и...
    {
      InitRecodeOutTable(cp);
      LocalUpperInit();
      InitLCIDSort();
      InitKeysArray();
      CtrlObject->Cp()->Redraw();
      ScrBuf.Flush();
      return TRUE;
    }
    else  // про траблы внешняя chcp сама скажет ;-)
     return FALSE;
  }

	else if (!StrCmpNI(strCmdLine,L"IF",2) && IsSpace(strCmdLine.At(2)))
	{
		const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
		// здесь PtrCmd - уже готовая команда, без IF

		if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			mprintf(L"%*s",X2-X1+1,L"");
			Show();
			return TRUE;
		}
		return FALSE;
	}

  /* $ 16.04.2002 DJ
     пропускаем обработку, если нажат Shift-Enter
  */
	else if (!SeparateWindow &&
			(StrCmpNI(strCmdLine,L"CD",Length=2)==0 || StrCmpNI(strCmdLine,L"CHDIR",Length=5)==0) &&
			(IsSpace(strCmdLine.At(Length)) || IsSlash(strCmdLine.At(Length)) ||
			TestParentFolderName((const wchar_t*)strCmdLine+Length)))
	{
		int ChDir=(Length==5);

		while (IsSpace(strCmdLine.At(Length)))
			Length++;

		IntChDir((const wchar_t *)strCmdLine+Length,ChDir,SilentInt);

		return(TRUE);
	}


	else if(!StrCmpI(strCmdLine,L"EXIT"))
	{
		FrameManager->ExitMainLoop(FALSE);
		return TRUE;
	}

  return(FALSE);
}

BOOL CommandLine::IntChDir(const wchar_t *CmdLine,int ClosePlugin,bool Selent)
{
	Panel *SetPanel;

	SetPanel=CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir;
	strExpandedDir = (const wchar_t*)CmdLine;

	Unquote(strExpandedDir);
	apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);


    // скорректируем букву диска на "подступах"
//    if(ExpandedDir[1] == L':' && iswalpha(ExpandedDir[0])) //BUGBUG
//      ExpandedDir[0]=towupper(ExpandedDir[0]);

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && (!strExpandedDir.At(1) || IsSlash(strExpandedDir.At(1))) && apiGetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES)
	{
		string strTemp;
		GetRegKey(strSystemExecutor,L"~",strTemp,g_strFarPath);
		if(strExpandedDir.At(1))
		{
			AddEndSlash(strTemp);
			strTemp += (const wchar_t*)strExpandedDir+2;
		}
		DeleteEndSlash(strTemp);
		strExpandedDir=strTemp;
	}

	if (wcspbrk(&strExpandedDir[PathPrefix(strExpandedDir)?4:0],L"?*")) // это маска?
	{
		FAR_FIND_DATA_EX wfd;
		if (apiGetFindDataEx(strExpandedDir, &wfd))
		{
			size_t pos;
			if(LastSlash(strExpandedDir,pos))
				strExpandedDir.SetLength(pos+1);
			else
				strExpandedDir.SetLength(0);

			strExpandedDir += wfd.strFileName;
		}
	}
	/* $ 15.11.2001 OT
		Сначала проверяем есть ли такая "обычная" директория.
		если уж нет, то тогда начинаем думать, что это директория плагинная
	*/
	DWORD DirAtt=apiGetFileAttributes(strExpandedDir);
	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsolute(strExpandedDir))
	{
		ReplaceStrings(strExpandedDir,L"/",L"\\",-1);
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
      //CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }
    */

	strExpandedDir.ReleaseBuffer ();

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePlugin);
		return(TRUE);
	}

	if(FarChDir(strExpandedDir))
	{
		SetPanel->ChangeDirToCurrent();
		if(!SetPanel->IsVisible())
			SetPanel->SetTitle();
	}
	else
	{
		if(!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir,MSG(MOk));
		return FALSE;
	}
	return TRUE;
}

// Проверить "Это батник?"
BOOL IsBatchExtType(const wchar_t *ExtPtr)
{
	string strExecuteBatchType(Opt.strExecuteBatchType);
	wchar_t *token = wcstok(strExecuteBatchType.GetBuffer(), L";");

	while(token)
	{
		if(StrCmpI(ExtPtr,token)==0)
			return TRUE;
		token = wcstok(NULL, L";");
	}

	return FALSE;
}
