/*
execute.cpp

"Запускатель" программ.

*/

#include "headers.hpp"
#pragma hdrstop

#include "farqueue.hpp"
#include "filepanels.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "global.hpp"
#include "cmdline.hpp"
#include "panel.hpp"
#include "fn.hpp"
#include "rdrwdsk.hpp"
#include "udlist.hpp"

static const wchar_t strSystemExecutor[]=L"System\\Executor";

// Выдранный кусок из будущего GetFileInfo, получаем достоверную информацию о
// ГУЯХ PE-модуля
/* 14.06.2002 VVM
  + Возвращаем константы IMAGE_SUBSYSTEM_*
    Дабы консоль отличать */

// При выходе из процедуры IMAGE_SUBSYTEM_UNKNOWN означает
// "файл не является исполняемым".
// Для DOS-приложений определим еще одно значение флага.
#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

static int IsCommandPEExeGUI(const wchar_t *FileName,DWORD& ImageSubsystem)
{
  //_SVS(CleverSysLog clvrSLog(L"IsCommandPEExeGUI()"));
  //_SVS(SysLog(L"Param: FileName='%s'",FileName));
//  char NameFile[NM];
  HANDLE hFile;
  int Ret=FALSE;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if((hFile=FAR_CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
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
      /* VVM $ */
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

        if(SetFilePointer(hFile,dos_head.e_lfanew,NULL,FILE_BEGIN) != -1)
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
/* VVM $ */

// по имени файла (по его расширению) получить команду активации
// Дополнительно смотрится гуевость команды-активатора
// (чтобы не ждать завершения)
wchar_t* GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error)
{
  //_SVS(CleverSysLog clvrSLog(L"GetShellAction()"));
  //_SVS(SysLog(L"Param: FileName='%s'",FileName));

  wchar_t Value[1024]; //BUGBUG
  string strNewValue;
  const wchar_t *ExtPtr;
  wchar_t *RetPtr;
  LONG ValueSize;
  const wchar_t command_action[]=L"\\command";

  Error=ERROR_SUCCESS;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if ((ExtPtr=wcsrchr(FileName,L'.'))==NULL)
    return(NULL);

  ValueSize=sizeof(Value);
  *Value=0;

  if (RegQueryValueW(HKEY_CLASSES_ROOT,ExtPtr,Value,&ValueSize)!=ERROR_SUCCESS)
    return(NULL);

  wcscat(Value,L"\\shell");
//_SVS(SysLog(L"[%d] Value='%s'",__LINE__,Value));

  HKEY hKey;
  if (RegOpenKeyW(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static wchar_t Action[512];

  *Action=0;
  ValueSize=sizeof(Action);
  LONG RetQuery = RegQueryValueExW(hKey,L"",NULL,NULL,(PBYTE)Action,(LPDWORD)&ValueSize);
  wcscat(Value,L"\\");
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,Action,Value));

  if (RetQuery == ERROR_SUCCESS)
  {
    UserDefinedListW ActionList(0,0,ULF_UNIQUE);

    RetPtr=(*Action==0 ? NULL:Action);
    /* $ 03.10.2002 VVM
      + Команд в одной строке может быть несколько. */
    const wchar_t *ActionPtr;

    LONG RetEnum = ERROR_SUCCESS;
    if (RetPtr != NULL && ActionList.Set(Action))
    {
      HKEY hOpenKey;

      ActionList.Reset();
      while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) != NULL)
      {
        strNewValue = Value;
        strNewValue += ActionPtr;
        strNewValue += command_action;

        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          wcsncat(Value, ActionPtr, (sizeof(Value) - 1)/2);
          RetPtr = xwcsncpy(Action,ActionPtr,(sizeof(Action)-1)/2);
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* while */
    } /* if */
    else
      wcsncat(Value,Action, (sizeof(Value) - 1)/2);
    /* VVM $ */

//_SVS(SysLog(L"[%d] Value='%s'",__LINE__,Value));
    if(RetEnum != ERROR_NO_MORE_ITEMS) // Если ничего не нашли, то...
      RetPtr=NULL;
  }
  else
  {
    // This member defaults to "Open" if no verb is specified.
    // Т.е. если мы вернули NULL, то подразумевается команда "Open"
      RetPtr=NULL;
//    strcat(Value,"\\open");
  }

  // Если RetPtr==NULL - мы не нашли default action.
  // Посмотрим - есть ли вообще что-нибудь у этого расширения
  if (RetPtr==NULL)
  {
    LONG RetEnum = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    DWORD dwKeySize = 0;
    FILETIME ftLastWriteTime;
    HKEY hOpenKey;

    // Сначала проверим "open"...
    wcscpy(Action,L"open");

    strNewValue = Value;
    strNewValue += Action;
    strNewValue += command_action;

    if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
    {
      RegCloseKey(hOpenKey);
      wcsncat(Value, Action, (sizeof(Value) - 1)/2);
      RetPtr = Action;
      RetEnum = ERROR_NO_MORE_ITEMS;
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
    } /* if */

    // ... а теперь все остальное, если "open" нету
    while (RetEnum == ERROR_SUCCESS)
    {
      dwKeySize = sizeof(Action);
      RetEnum = RegEnumKeyExW(hKey, dwIndex++, Action, &dwKeySize, NULL, NULL, NULL, &ftLastWriteTime);
      if (RetEnum == ERROR_SUCCESS)
      {
        // Проверим наличие "команды" у этого ключа
        strNewValue = Value;
        strNewValue += Action;
        strNewValue += command_action;
        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          wcsncat(Value, Action, (sizeof(Value) - 1)/2);
          RetPtr = Action;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* if */
    } /* while */
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
  } /* if */
  RegCloseKey(hKey);

  if (RetPtr != NULL)
  {
    wcsncat(Value,command_action, (sizeof(Value) - 1)/2);

    // а теперь проверим ГУЕвость запускаемой проги
    if (RegOpenKeyW(HKEY_CLASSES_ROOT,Value,&hKey)==ERROR_SUCCESS)
    {
      ValueSize=1024*sizeof (wchar_t);

      wchar_t *lpwszNewValue = strNewValue.GetBuffer (1024); //BUGBUG

      RetQuery=RegQueryValueExW(hKey,L"",NULL,NULL,(PBYTE)lpwszNewValue,(LPDWORD)&ValueSize);

      strNewValue.ReleaseBuffer ();

      RegCloseKey(hKey);
      if(RetQuery == ERROR_SUCCESS && !strNewValue.IsEmpty())
      {
        apiExpandEnvironmentStrings(strNewValue,strNewValue);

        wchar_t *Ptr = strNewValue.GetBuffer ();
        // Выделяем имя модуля
        if (*Ptr==L'\"')
        {
          if ((Ptr=wcschr(Ptr,L'\"'))!=NULL)
            *Ptr=0;
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

//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
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
  int Ret, I;
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
    if((I=apiGetEnvironmentVariable(L"PATHEXT",strFullName)) != 0)
    {
      static wchar_t const * const StdExecuteExt0[4]={L".BAT;",L".CMD;",L".EXE;",L".COM;"};
      for(I=0; I < sizeof(StdExecuteExt0)/sizeof(StdExecuteExt0[0]); ++I)
        ReplaceStringsW(strFullName,StdExecuteExt0[I],L"",-1);
    }


    strFullName += ";";

    Ptr=wcscat(StdExecuteExt, strFullName);  //BUGBUG
    StdExecuteExt[wcslen(StdExecuteExt)]=0;
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
  if(GetRegKeyW(strSystemExecutor,L"Type",0))
  {
    if (!PrepareExcludeCmds)
    {
      GetRegKeyW(strSystemExecutor,L"ExcludeCmds",(PBYTE)ExcludeCmds,(PBYTE)L"",sizeof(ExcludeCmds));
      Ptr=wcscat(ExcludeCmds,L";"); //!!!
      ExcludeCmds[wcslen(ExcludeCmds)]=0;
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
      if(!LocalStricmpW(strFileName,Ptr))
      {
        ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
        return TRUE;
      }
      Ptr+=wcslen(Ptr)+1;
    }
  }


  {
    wchar_t *lpwszFilePart;

    strFullName = strFileName;

    const wchar_t *PtrFName = PointToNameW (strFullName);
    PtrFName = wcsrchr (PtrFName, L'.');

    string strWorkName;

    wchar_t *PtrExt=StdExecuteExt;
    while(*PtrExt) // первый проход - в текущем каталоге
    {
      strWorkName = strFullName;

      if (!PtrFName)
        strWorkName += PtrExt;

      if(GetFileAttributesW(strFullName) != -1)
      {
        ConvertNameToFullW (strFullName, strFullName);
        Ret=TRUE;
        break;
      }
      PtrExt+=wcslen(PtrExt)+1;
    }

    if(!Ret) // второй проход - по правилам SearchPath
    {
      /* $ 26.09.2003 VVM
        ! Сначала поищем по переменной PATH, а уж потом везде */
      string strPathEnv;
      if (apiGetEnvironmentVariable(L"PATH",strPathEnv) != 0)
      {
        PtrExt=StdExecuteExt;

        DWORD dwSize = 0;

        while(*PtrExt)
        {
          strWorkName = strFullName;

          if (!PtrFName)
            strWorkName += PtrExt;

          if ( (dwSize = SearchPathW (strPathEnv, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

            SearchPathW(strPathEnv,strFullName,PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=wcslen(PtrExt)+1;
        }
      }

      DWORD dwSize;

      if (!Ret)
      {
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          strWorkName = strFullName;

          if (!PtrFName)
            strWorkName += PtrExt;



          if ( (dwSize = SearchPathW (NULL, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

            SearchPathW(NULL,strFullName,PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=wcslen(PtrExt)+1;
        }
      }
      /* VVM $ */

      if (!Ret && Opt.ExecuteUseAppPath) // третий проход - лезим в реестр в "App Paths"
      {
        // В строке Command заменть исполняемый модуль на полный путь, который
        // берется из SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
        // Сначала смотрим в HKCU, затем - в HKLM
        HKEY hKey;
        HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};

        strFullName = RegPath+strFileName;

        for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
        {
          if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
          {
            DWORD Type, DataSize;

            DataSize = strFullName.GetLength()*2; //???

            wchar_t *lpwszFullName = strFullName.GetBuffer ();

            RegQueryValueExW(hKey,L"", 0, &Type, (LPBYTE)lpwszFullName, &DataSize);

            strFullName.ReleaseBuffer ();

            RegCloseKey(hKey);
            /* $ 03.10.2001 VVM Обработать переменные окружения */
            strFileName = strFullName;
            apiExpandEnvironmentStrings(strFileName, strFullName);
            UnquoteW(strFullName);
            Ret=TRUE;
            break;
          }
        }

        if (!Ret && Opt.ExecuteUseAppPath)
        /* $ 14.06.2002 VVM
           Не нашли - попробуем с расширением */
        {
          PtrExt=StdExecuteExt;

          while(*PtrExt && !Ret)
          {
            strFullName = RegPath+strFileName+PtrExt;

            for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
            {
              if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
              {
                DWORD Type, DataSize;

                DataSize = strFullName.GetLength()*2;

                wchar_t *lpwszFullName = strFullName.GetBuffer ();

                RegQueryValueExW(hKey,L"", 0, &Type, (LPBYTE)lpwszFullName,&DataSize);

                strFullName.GetBuffer ();

                RegCloseKey(hKey);
                /* $ 03.10.2001 VVM Обработать переменные окружения */
                strFileName = strFullName;
                apiExpandEnvironmentStrings(strFileName, strFullName);
                UnquoteW(strFullName);
                Ret=TRUE;
                break;
              }
            } /* for */
            PtrExt+=wcslen(PtrExt)+1;
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

/* $ 14.06.2002 VVM
   Отключим эту функцию, т.к. ее никто не пользует */
#ifdef ADD_GUI_CHECK
DWORD IsCommandExeGUI(const char *Command)
{
  char FileName[4096],FullName[4096],*EndName,*FilePart;

  if (*Command=='\"')
  {
    FAR_OemToChar(Command+1,FullName);
    if ((EndName=strchr(FullName,'\"'))!=NULL)
      *EndName=0;
  }
  else
  {
    FAR_OemToChar(Command,FullName);
    if ((EndName=strpbrk(FullName," \t/"))!=NULL)
      *EndName=0;
  }
  int GUIType=0;

  /* $ 07.09.2001 VVM
    + Обработать переменные окружения */
  ExpandEnvironmentStrings(FullName,FileName,sizeof(FileName));
  /* VVM $ */

  SetFileApisTo(APIS2ANSI);
  /*$ 18.09.2000 skv
    + to allow execution of c.bat in current directory,
      if gui program c.exe exists somewhere in PATH,
      in FAR's console and not in separate window.
      for(;;) is just to prevent multiple nested ifs.
  */
  for(;;)
  {
    if(BatchFileExist(FileName,FullName,sizeof(FullName)-1))
      break;
  /* skv$*/

    if (SearchPath(NULL,FileName,".exe",sizeof(FullName),FullName,&FilePart))
    {
      SHFILEINFO sfi;
      DWORD ExeType=SHGetFileInfo(FullName,0,&sfi,sizeof(sfi),SHGFI_EXETYPE);
      GUIType=HIWORD(ExeType)>=0x0300 && HIWORD(ExeType)<=0x1000 &&
              /* $ 13.07.2000 IG
                 в VC, похоже, нельзя сказать так: 0x4550 == 'PE', надо
                 делать проверку побайтово.
              */
              HIBYTE(ExeType)=='E' && (LOBYTE(ExeType)=='N' || LOBYTE(ExeType)=='P');
              /* IG $ */
    }
/*$ 18.09.2000 skv
    little trick.
*/
    break;
  }
  /* skv$*/
  SetFileApisTo(APIS2OEM);
  return(GUIType);
}
#endif
/* VVM $ */


/* Функция для выставления пути для пассивной панели
   чтоб путь на пассивной панели был доступен по DriveLetter:
   для запущенных из фара программ в Win9x
*/
void SetCurrentDirectoryForPassivePanel(string &strComspec,const wchar_t *CmdStr)
{
  Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);

  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && PassivePanel->GetType()==FILE_PANEL)
  {
    //for (int I=0;CmdStr[I]!=0;I++)
    //{
      //if (LocalIsalphaW(CmdStr[I]) && CmdStr[I+1]==L':' && CmdStr[I+2]!=L'\\')
      //{
        string strSetPathCmd;
        string strSavePath;
        string strPanelPath;

        FarGetCurDirW(strSavePath);
        PassivePanel->GetCurDirW(strPanelPath);

        QuoteSpaceW (strPanelPath);

        strSetPathCmd = strComspec+L" /C chdir %s"+strPanelPath;

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        memset (&si, 0, sizeof (si));
        si.cb = sizeof (si);

        CreateProcessW(
              NULL,
              (wchar_t*)(const wchar_t*)strSetPathCmd,
              NULL,
              NULL,
              FALSE,
              CREATE_DEFAULT_ERROR_MODE,
              NULL,
              NULL,
              &si,
              &pi
              );

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        FarChDirW(strSavePath);
        //break;
      //}
    //}
  }
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

  bool bIsNT = (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

  string strNewCmdStr;
  string strNewCmdPar;
  string strExecLine;


  PartCmdLineW (
          CmdStr,
          strNewCmdStr,
          strNewCmdPar
          );

  /* $ 05.04.2005 AY: Это не правильно, надо убирать только первый пробел,
                      что теперь и делает PartCmdLine.
  if(*NewCmdPar)
    RemoveExternalSpaces(NewCmdPar);
  AY $ */

  DWORD dwAttr = GetFileAttributesW(strNewCmdStr);

  if ( SeparateWindow == 1 )
  {
      if ( strNewCmdPar.IsEmpty() && dwAttr != -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
      {
          ConvertNameToFullW(strNewCmdStr, strNewCmdStr);
          SeparateWindow=2;
          FolderRun=TRUE;
      }
  }


  SHELLEXECUTEINFOW seInfo;
  memset (&seInfo, 0, sizeof (seInfo));

  seInfo.cbSize = sizeof (seInfo);

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (si));

  si.cb = sizeof (si);

  string strComspec;
  apiGetEnvironmentVariable (L"COMSPEC", strComspec);

  if ( strComspec.IsEmpty() && (SeparateWindow != 2) )
  {
    MessageW(MSG_WARNING, 1, UMSG(MWarning), UMSG(MComspecNotFound), UMSG(MErrorCancelled), UMSG(MOk));
    return -1;
  }

  int Visible, Size;

  GetCursorType(Visible,Size);
  SetInitialCursorType();

  int PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  int ConsoleCP = GetConsoleCP();
  int ConsoleOutputCP = GetConsoleOutputCP();

  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  GetConsoleScreenBufferInfo(hConOut,&sbi);

  wchar_t OldTitle[512];
  GetConsoleTitleW(OldTitle, sizeof(OldTitle)/sizeof(wchar_t));

  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && !strComspec.IsEmpty())
    SetCurrentDirectoryForPassivePanel(strComspec,CmdStr);

  DWORD dwSubSystem;
  DWORD dwError = 0;

  HANDLE hProcess = NULL, hThread = NULL;

  if(FolderRun && SeparateWindow==2)
    AddEndSlashW(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
  else
  {
    PrepareExecuteModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

    if(/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
    {
      DWORD Error=0, dwSubSystem2=0;
      wchar_t *ExtPtr=wcsrchr(strNewCmdStr,L'.');

      if(ExtPtr && !(LocalStricmpW(ExtPtr,L".exe")==0 || LocalStricmpW(ExtPtr,L".com")==0 ||
         IsBatchExtTypeW(ExtPtr)))
        if(GetShellAction(strNewCmdStr,dwSubSystem2,Error) && Error != ERROR_NO_ASSOCIATION)
          dwSubSystem=dwSubSystem2;
    }

    if ( dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
      SeparateWindow = 2;
  }

  ScrBuf.Flush ();

  if ( SeparateWindow == 2 )
  {
    seInfo.lpFile = strNewCmdStr;
    seInfo.lpParameters = strNewCmdPar;
    seInfo.nShow = SW_SHOWNORMAL;

    seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?NULL:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
    //seInfo.lpVerb = "open";
    seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS;

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

    QuoteSpaceW (strNewCmdStr);

    strExecLine = strComspec;
    strExecLine += L" /C ";

    bool bDoubleQ = false;

    if ( bIsNT && wcspbrk (strNewCmdStr, L"&<>()@^|") )
      bDoubleQ = true;

    if ( (bIsNT && !strNewCmdPar.IsEmpty()) || bDoubleQ )
      strExecLine += L"\"";

    strExecLine += strNewCmdStr;

    if ( !strNewCmdPar.IsEmpty() )
    {
      strExecLine += L" ";
      strExecLine += strNewCmdPar;
    }

    if ( (bIsNT && !strNewCmdPar.IsEmpty()) || bDoubleQ)
      strExecLine += L"\"";

    // // попытка борьбы с синим фоном в 4NT при старте консоль
    SetRealColor (F_LIGHTGRAY|B_BLACK);

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
      UnquoteW(strOutStr);
      TruncPathStrW(strOutStr,ScrX-15);

      MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotExecute),strOutStr,UMSG(MOk));
    }
    else
    {
      ScrBuf.Flush ();

      strOutStr.Format (UMSG(MExecuteErrorMessage),(const wchar_t *)strNewCmdStr);
      string strPtrStr=FarFormatTextW(strOutStr,ScrX, strPtrStr,L"\n",0);

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
      CmdStr.SetStringW(L"");
      GotoXY(X1,Y1);
      mprintfW(L"%*s",X2-X1+1,L"");
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
        FarChDirW(strCurDir);
      CmdStr.SetStringW(L"");
      if ((Code=ProcessOSCommands(CmdLine,SeparateWindow)) == TRUE)
        Code=-1;
      else
      {
        string strTempStr;
        strTempStr = CmdLine;
        if(Code == -1)
          ReplaceStringsW(strTempStr,L"/",L"\\",-1);
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
  /* VVM $ */
  ScrBuf.Flush();
  return(Code);
}


/* 20.03.2002 IS
   "if [not] exist" дружит теперь с масками файлов
   PrepareOSIfExist теперь принимает и возвращает const
*/
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

  wchar_t Cmd[1024]; //BUGBUG
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
    while(*PtrCmd && IsSpaceW(*PtrCmd)) ++PtrCmd;
  }
  /* DJ $ */
  while(1)
  {
    if (!PtrCmd || !*PtrCmd || LocalStrnicmpW(PtrCmd,L"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (LocalStrnicmpW(PtrCmd,L"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !LocalStrnicmpW(PtrCmd,L"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && IsSpaceW(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
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
        string strExpandedStr;
        wmemmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
        Cmd[PtrCmd-CmdStart]=0;
        //UnquoteW(Cmd); BUGBUG
//_SVS(SysLog(L"Cmd='%s'",Cmd));
        if (apiExpandEnvironmentStrings(Cmd,strExpandedStr)!=0)
        {
          string strFullPath;
          if(!(Cmd[1] == L':' || (Cmd[0] == L'\\' && Cmd[1]==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDirW(strFullPath);
            else
              FarGetCurDirW(strFullPath);
            AddEndSlashW(strFullPath);
          }
          strFullPath += strExpandedStr;
          DWORD FileAttr=(DWORD)-1;
          if(wcspbrk(strExpandedStr,L"*?")) // это маска?
          {
            FAR_FIND_DATA_EX wfd;

            if ( apiGetFindDataEx (strFullPath, &wfd) )
              FileAttr=wfd.dwFileAttributes;
          }
          else
          {
            ConvertNameToFullW(strFullPath, strFullPath);
            FileAttr=GetFileAttributesW(strFullPath);
          }
//_SVS(SysLog(L"%08X FullPath=%s",FileAttr,FullPath));
          if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
          {
            while(*PtrCmd && IsSpaceW(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return L"";
        }
      }
      /* DJ $ */
    }
    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !LocalStrnicmpW(PtrCmd,L"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && IsSpaceW(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == L'"')
        PtrCmd=wcschr(PtrCmd+1,L'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=wcschr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          string strExpandedStr;
          wmemmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          DWORD ERet=apiGetEnvironmentVariable(Cmd,strExpandedStr);
//_SVS(SysLog(Cmd));
          if(ERet && !Not || !ERet && Not)
          {
            while(*PtrCmd && IsSpaceW(*PtrCmd)) ++PtrCmd;
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
/* SVS $ */
/* IS $ */

int CommandLine::ProcessOSCommands(const wchar_t *CmdLine,int SeparateWindow)
{
  Panel *SetPanel;
  int Length;

  string strCmdLine = CmdLine;

  SetPanel=CtrlObject->Cp()->ActivePanel;

  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

  RemoveTrailingSpacesW(strCmdLine);

  if (!SeparateWindow && LocalIsalphaW(strCmdLine.At(0)) && strCmdLine.At(1)==L':' && strCmdLine.At(2)==0)
  {
    wchar_t NewDir[10];
    swprintf(NewDir,L"%c:",LocalUpperW(strCmdLine.At(0)));
    FarChDirW(strCmdLine);
    if (getdisk()!=NewDir[0]-L'A')
    {
      wcscat(NewDir,L"\\");
      FarChDirW(NewDir);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }

  // SET [переменная=[строка]]
  if (LocalStrnicmpW(strCmdLine,L"SET ",4)==0)
  {
    string strCmd;

    strCmd = (const wchar_t*)strCmdLine+4;

    wchar_t *lpwszValue = strCmd.GetBuffer ();

    lpwszValue=wcschr(lpwszValue,L'=');

    if (lpwszValue==NULL)
      return(FALSE);

    *lpwszValue=0;

    if (lpwszValue[1]==0) //??
      SetEnvironmentVariableW(strCmd,NULL);
    else
    {
      string strExpandedStr;

      if (apiExpandEnvironmentStrings(lpwszValue+1,strExpandedStr) != 0)
        SetEnvironmentVariableW(strCmd,strExpandedStr);
    }

    strCmd.ReleaseBuffer ();

    return(TRUE);
  }

  if (!LocalStrnicmpW(strCmdLine,L"REM ",4) || !LocalStrnicmpW(strCmdLine,L"::",2))
  {
    return TRUE;
  }

  if (!LocalStrnicmpW(strCmdLine,L"CLS",3))
  {
    if(strCmdLine.At(3))
      return FALSE;

    ClearScreen(F_LIGHTGRAY|B_BLACK);
    return TRUE;
  }

  /*
  Displays or sets the active code page number.
  CHCP [nnn]
    nnn   Specifies a code page number (Dec or Hex).
  Type CHCP without a parameter to display the active code page number.
  */
  if (!LocalStrnicmpW(strCmdLine,L"CHCP",4))
  {
    if(strCmdLine.At(4) == 0 || !(strCmdLine.At(4) == L' ' || strCmdLine.At(4) == L'\t'))
      return(FALSE);

    strCmdLine = (const wchar_t*)strCmdLine+5;

    const wchar_t *Ptr=RemoveExternalSpacesW(strCmdLine);
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
      InitKeysArray();
      CtrlObject->Cp()->Redraw();
      ScrBuf.Flush();
      return TRUE;
    }
    else  // про траблы внешняя chcp сама скажет ;-)
     return FALSE;
  }

  /* $ 14.01.2001 SVS
     + В ProcessOSCommands добавлена обработка
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  */
  if (LocalStrnicmpW(strCmdLine,L"IF ",3)==0)
  {
    const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
    // здесь PtrCmd - уже готовая команда, без IF

    if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
    {
      CmdStr.SetStringW(L"");
      GotoXY(X1,Y1);
      mprintfW(L"%*s",X2-X1+1,L"");
      Show();
      return TRUE;
    }
    return FALSE;
  }
  /* SVS $ */

  /* $ 16.04.2002 DJ
     пропускаем обработку, если нажат Shift-Enter
  */
  if (!SeparateWindow &&  /* DJ $ */
      (LocalStrnicmpW(strCmdLine,L"CD",Length=2)==0 || LocalStrnicmpW(strCmdLine,L"CHDIR",Length=5)==0) &&
      (IsSpaceW(strCmdLine.At(Length)) || strCmdLine.At(Length)==L'\\' || strCmdLine.At(Length)==L'/' ||
      TestParentFolderNameW((const wchar_t*)strCmdLine+Length)))
  {
    int ChDir=(Length==5);

    while (IsSpaceW(strCmdLine.At(Length)))
      Length++;

    string strExpandedDir;
    strExpandedDir = (const wchar_t*)strCmdLine+Length;

    UnquoteW(strExpandedDir);
    apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);


    // скорректируем букву диска на "подступах"
//    if(ExpandedDir[1] == L':' && iswalpha(ExpandedDir[0])) //BUGBUG
//      ExpandedDir[0]=towupper(ExpandedDir[0]);

    if(SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && !strExpandedDir.At(1) && GetFileAttributesW(strExpandedDir) == (DWORD)-1)
    {
      GetRegKeyW(strSystemExecutor,L"~",strExpandedDir,g_strFarPath);
      DeleteEndSlashW(strExpandedDir);
    }

    if(wcspbrk(strExpandedDir,L"?*")) // это маска?
    {
      FAR_FIND_DATA_EX wfd;
      HANDLE hFile=apiFindFirstFile(strExpandedDir, &wfd);
      if(hFile!=INVALID_HANDLE_VALUE)
      {
        wchar_t *Ptr = strExpandedDir.GetBuffer (), *Ptr2;

        Ptr2=wcsrchr(Ptr,L'\\');
        if(!Ptr2)
          Ptr2=wcsrchr(Ptr,L'/');
        Ptr=Ptr2;

        if(Ptr)
        {
          *++Ptr=0;
          strExpandedDir.ReleaseBuffer ();
        }
        else
        {
          strExpandedDir.ReleaseBuffer ();
          strExpandedDir=L"";
        }


        strExpandedDir += wfd.strFileName;
        FindClose(hFile);
      }
    }
    /* $ 15.11.2001 OT
      Сначала проверяем есть ли такая "обычная" директория.
      если уж нет, то тогда начинаем думать, что это директория плагинная
    */
    DWORD DirAtt=GetFileAttributesW(strExpandedDir);
    if (DirAtt!=0xffffffff && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsoluteW(strExpandedDir))
    {
      ReplaceStringsW(strExpandedDir,L"/",L"\\",-1);
      SetPanel->SetCurDirW(strExpandedDir,TRUE);
      return TRUE;
    }
    /* OT $ */

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
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }
    */
    /* SKV $ */

    strExpandedDir.ReleaseBuffer ();

    if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
    {
      SetPanel->SetCurDirW(strExpandedDir,ChDir);
      return(TRUE);
    }

    //if (ExpandEnvironmentStrW(strExpandedDir,strExpandedDir)!=0)
    {
      if(CheckFolderW(strExpandedDir) <= CHKFLD_NOTACCESS)
        return -1;

      if (!FarChDirW(strExpandedDir))
        return(FALSE);
    }

    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();

    return(TRUE);
  }
  return(FALSE);
}

// Проверить "Это батник?"
BOOL IsBatchExtTypeW(const wchar_t *ExtPtr)
{
  const wchar_t *PtrBatchType=Opt.strExecuteBatchType;
  while(*PtrBatchType)
  {
    if(LocalStricmpW(ExtPtr,PtrBatchType)==0)
      return TRUE;
    PtrBatchType+=wcslen(PtrBatchType)+1;
  }

  return FALSE;
}

#ifdef ADD_GUI_CHECK
// батник существует? (и вернем полное имя - добавляется расширение)
BOOL BatchFileExist(const char *FileName,char *DestName,int SizeDestName)
{
  char *PtrBatchType=Opt.ExecuteBatchType;
  BOOL Result=FALSE;

  char *FullName=(char*)alloca(strlen(FileName)+64);
  if(FullName)
  {
    strcpy(FullName,FileName);
    char *FullNameExt=FullName+strlen(FullName);

    while(*PtrBatchType)
    {
      strcat(FullNameExt,PtrBatchType);

      if(GetFileAttributes(FullName)!=-1)
      {
        strncpy(DestName,FullName,SizeDestName);
        Result=TRUE;
        break;
      }

      PtrBatchType+=strlen(PtrBatchType)+1;
    }
  }

  return Result;
}
#endif
