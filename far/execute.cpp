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
#include "manager.hpp"

static const char strSystemExecutor[]="System\\Executor";

// Выдранный кусок из будущего GetFileInfo, получаем достоверную информацию о
// ГУЯХ PE-модуля
/* 14.06.2002 VVM
  + Возвращаем константы IMAGE_SUBSYSTEM_*
    Дабы консоль отличать */

// При выходе из процедуры IMAGE_SUBSYTEM_UNKNOWN означает
// "файл не является исполняемым".
// Для DOS-приложений определим еще одно значение флага.
#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

static int IsCommandPEExeGUI(const char *FileName,DWORD& ImageSubsystem)
{
  //_SVS(CleverSysLog clvrSLog("IsCommandPEExeGUI()"));
  //_SVS(SysLog("Param: FileName='%s'",FileName));
//  char NameFile[NM];
  HANDLE hFile;
  int Ret=FALSE;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if((hFile=FAR_CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
  {
    unsigned __int64 FileSize;
    DWORD ReadSize;
    IMAGE_DOS_HEADER dos_head;

    FAR_GetFileSize(hFile,&FileSize);

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

#if defined(__BORLANDC__) || defined(__GNUC__)
const IID IID_IApplicationAssociationRegistration = { 0x4E530B0A, 0xE611, 0x4C77, 0xA3, 0xAC, 0x90, 0x31, 0xD0, 0x22, 0x28, 0x1B };
#endif

bool GetShellType(const char *Ext, char *Type, LONG Size,ASSOCIATIONTYPE aType)
{
  bool bVistaType = false;
  typedef HRESULT (WINAPI *PSHCREATEASSOCIATIONREGISTRATION)(REFIID, void **);
  static PSHCREATEASSOCIATIONREGISTRATION pfnSHCreateAssociationRegistration=NULL;
  static bool bInit = false;

  if (!Ext || !Type)
    return false;

  *Type = 0;

  if (WinVer.dwMajorVersion >= 6)
  {
    if (!bInit)
    {
      bInit = true;
      HMODULE hShell = GetModuleHandle("shell32.dll");
      if (hShell)
      {
        pfnSHCreateAssociationRegistration = (PSHCREATEASSOCIATIONREGISTRATION)GetProcAddress(hShell, "SHCreateAssociationRegistration");
      }
    }

    if (pfnSHCreateAssociationRegistration)
    {
      IApplicationAssociationRegistration* pAAR;
      HRESULT hr = pfnSHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (void**)&pAAR);
      if (SUCCEEDED(hr))
      {
        wchar_t WExt[NM];
        MultiByteToWideChar (
                  CP_OEMCP,
                  0,
                  Ext,
                  -1,
                  WExt,
                  sizeof(WExt)/sizeof(wchar_t)
                  );

        wchar_t *p;
        if (pAAR->QueryCurrentDefault(WExt, aType, AL_EFFECTIVE, &p) == S_OK)
        {
          bVistaType = true;
          WideCharToMultiByte (
                  CP_OEMCP,
                  0,
                  p,
                  -1,
                  Type,
                  Size,
                  NULL,
                  NULL
                  );
          CoTaskMemFree(p);
        }
        pAAR->Release();
      }
    }
  }

  if (!bVistaType)
  {
    if (aType==AT_URLPROTOCOL)
    {
      xstrncpy(Type,Ext,Size);
    }
    else
    {
      if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)Ext,(LPTSTR)Type,&Size)!=ERROR_SUCCESS)
        return false;
    }
  }

  return *Type!=0;
}

// по имени файла (по его расширению) получить команду активации
// Дополнительно смотрится гуевость команды-активатора
// (чтобы не ждать завершения)
char* GetShellAction(const char *FileName,DWORD& ImageSubsystem,DWORD& Error)
{
  //_SVS(CleverSysLog clvrSLog("GetShellAction()"));
  //_SVS(SysLog("Param: FileName='%s'",FileName));

  char Value[1024];
  char NewValue[2048];
  const char *ExtPtr;
  char *RetPtr;
  LONG ValueSize;
  const char command_action[]="\\command";

  Error=ERROR_SUCCESS;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if ((ExtPtr=strrchr(FileName,'.'))==NULL)
    return(NULL);

  if (!GetShellType(ExtPtr, Value, sizeof(Value)))
    return(NULL);

  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CLASSES_ROOT,Value,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
  {
    int nResult=RegQueryValueEx(hKey,"IsShortcut",NULL,NULL,NULL,NULL);
    RegCloseKey(hKey);
    if(nResult==ERROR_SUCCESS)
      return NULL;
  }

  strcat(Value,"\\shell");
//_SVS(SysLog("[%d] Value='%s'",__LINE__,Value));

  if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static char Action[512];

  *Action=0;
  ValueSize=sizeof(Action);
  LONG RetQuery = RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)Action,(LPDWORD)&ValueSize);
  strcat(Value,"\\");
//_SVS(SysLog("[%d] Action='%s' Value='%s'",__LINE__,Action,Value));

  if (RetQuery == ERROR_SUCCESS)
  {
    UserDefinedList ActionList(0,0,ULF_UNIQUE);

    RetPtr=(*Action==0 ? NULL:Action);
    /* $ 03.10.2002 VVM
      + Команд в одной строке может быть несколько. */
    const char *ActionPtr;

    LONG RetEnum = ERROR_SUCCESS;
    if (RetPtr != NULL && ActionList.Set(Action))
    {
      HKEY hOpenKey;

      ActionList.Reset();
      while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) != NULL)
      {
        xstrncpy(NewValue, Value, sizeof(NewValue) - 1);
        xstrncat(NewValue, ActionPtr, sizeof(NewValue) - 1);
        xstrncat(NewValue, command_action, sizeof(NewValue) - 1);
        if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          xstrncat(Value, ActionPtr, sizeof(Value) - 1);
          RetPtr = xstrncpy(Action,ActionPtr,sizeof(Action)-1);
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* while */
    } /* if */
    else
      xstrncat(Value,Action, sizeof(Value) - 1);
    /* VVM $ */

//_SVS(SysLog("[%d] Value='%s'",__LINE__,Value));
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
    strcpy(Action,"open");
    xstrncpy(NewValue, Value, sizeof(NewValue) - 1);
    xstrncat(NewValue, Action, sizeof(NewValue) - 1);
    xstrncat(NewValue, command_action, sizeof(NewValue) - 1);
    if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
    {
      RegCloseKey(hOpenKey);
      xstrncat(Value, Action, sizeof(Value) - 1);
      RetPtr = Action;
      RetEnum = ERROR_NO_MORE_ITEMS;
//_SVS(SysLog("[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
    } /* if */

    // ... а теперь все остальное, если "open" нету
    while (RetEnum == ERROR_SUCCESS)
    {
      dwKeySize = sizeof(Action);
      RetEnum = RegEnumKeyEx(hKey, dwIndex++, Action, &dwKeySize, NULL, NULL, NULL, &ftLastWriteTime);
      if (RetEnum == ERROR_SUCCESS)
      {
        // Проверим наличие "команды" у этого ключа
        xstrncpy(NewValue, Value, sizeof(NewValue) - 1);
        xstrncat(NewValue, Action, sizeof(NewValue) - 1);
        xstrncat(NewValue, command_action, sizeof(NewValue) - 1);
        if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          xstrncat(Value, Action, sizeof(Value) - 1);
          RetPtr = Action;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* if */
    } /* while */
//_SVS(SysLog("[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
  } /* if */
  RegCloseKey(hKey);

  if (RetPtr != NULL)
  {
    xstrncat(Value,command_action, sizeof(Value) - 1);

    // а теперь проверим ГУЕвость запускаемой проги
    if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)==ERROR_SUCCESS)
    {
      ValueSize=sizeof(NewValue);
      RetQuery=RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)NewValue,(LPDWORD)&ValueSize);
      RegCloseKey(hKey);
      if(RetQuery == ERROR_SUCCESS && *NewValue)
      {
        char *Ptr;
        ExpandEnvironmentStr(NewValue,NewValue,sizeof(NewValue));
        // Выделяем имя модуля
        if (*NewValue=='\"')
        {
          FAR_OemToChar(NewValue+1,NewValue);
          if ((Ptr=strchr(NewValue,'\"'))!=NULL)
            *Ptr=0;
        }
        else
        {
          FAR_OemToChar(NewValue,NewValue);
          if ((Ptr=strpbrk(NewValue," \t/"))!=NULL)
            *Ptr=0;
        }
        IsCommandPEExeGUI(NewValue,ImageSubsystem);
      }
      else
      {
        Error=ERROR_NO_ASSOCIATION;
        RetPtr=NULL;
      }
    }
  }

//_SVS(SysLog("[%d] Action='%s' Value='%s'",__LINE__,Action,Value));
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
int WINAPI PrepareExecuteModule(const char *Command,char *Dest,int DestSize,DWORD& ImageSubsystem)
{
  //_SVS(CleverSysLog clvrSLog("PrepareExecuteModule()"));
  //_SVS(SysLog("Param: Command='%s'",Command));
  int Ret, I;
  char FileName[4096],FullName[4096], *Ptr;
  // int IsQuoted=FALSE;
  // int IsExistExt=FALSE;

  static char StdExecuteExt[NM]=".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH";
  static const char RegPath[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
  static int PreparePrepareExt=FALSE;

  if(!PreparePrepareExt) // самоинициилизирующийся кусок
  {
    GetEnvironmentVariable("PATHEXT",StdExecuteExt,sizeof(StdExecuteExt)-1);

    Ptr=strcat(StdExecuteExt,";");
    while(*Ptr)
    {
      if(*Ptr == ';')
        *Ptr=0;
      ++Ptr;
    }
    PreparePrepareExt=TRUE;
  }

  /* Берем "исключения" из реестра, которые должны исполянться директом,
     например, некоторые внутренние команды ком.процессора.
  */
  static char ExcludeCmds[4096]={0};
  static int PrepareExcludeCmds=FALSE;
  if(GetRegKey(strSystemExecutor,"Type",0))
  {
    if (!PrepareExcludeCmds)
    {
      GetRegKey(strSystemExecutor,"ExcludeCmds",(char*)ExcludeCmds,"",sizeof(ExcludeCmds));
      Ptr=strcat(ExcludeCmds,";"); //!!!
      ExcludeCmds[strlen(ExcludeCmds)]=0;
      while(*Ptr)
      {
        if(*Ptr == ';')
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

  /* $ 14.06.2002 VVM
     Имя модуля всегда передается без кавычек. Нефиг лишний раз гонять туда/сюда
  // Выделяем имя модуля
  if (*Command=='\"')
  {
    FAR_OemToChar(Command+1,FullName);
    if ((Ptr=strchr(FullName,'\"'))!=NULL)
      *Ptr=0;
    IsQuoted=TRUE;
  }
  else
  {
    FAR_OemToChar(Command,FullName);
    if ((Ptr=strpbrk(FullName," \t/|><"))!=NULL)
      *Ptr=0;
  } VVM $ */

  if(!*Command) // вот же, надо же... пустышку передали :-(
    return 0;

  FAR_OemToChar(Command,FileName);

  // нулевой проход - смотрим исключения
  {
    char *Ptr=ExcludeCmds;
    while(*Ptr)
    {
      if(!LocalStricmp(FileName,Ptr))
      {
        ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
        return TRUE;
      }
      Ptr+=strlen(Ptr)+1;
    }
  }

  // IsExistExt - если точки нету (расширения), то потом модифицировать не
  // будем.
  // IsExistExt=strrchr(FullName,'.')!=NULL;

  SetFileApisTo(APIS2ANSI);

  {
    char *FilePart;
    char *PtrFName=strrchr(PointToName(strcpy(FullName,FileName)),'.');
    char *WorkPtrFName=0;
    if(!PtrFName)
      WorkPtrFName=FullName+strlen(FullName);

    char *PtrExt=StdExecuteExt;
    while(*PtrExt) // первый проход - в текущем каталоге
    {
      if(!PtrFName)
        strcpy(WorkPtrFName,PtrExt);
      DWORD dwFileAttr = GetFileAttributes(FullName);
      if ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
      {
        // GetFullPathName - это нужно, т.к. если тыкаем в date.exe
        // в текущем каталоге, то нифига ничего доброго не получаем
        // cmd.exe по каким то причинам вызыват внутренний date
        GetFullPathName(FullName,sizeof(FullName),FullName,&FilePart);

        Ret=TRUE;
        break;
      }
      PtrExt+=strlen(PtrExt)+1;
    }

    if(!Ret) // второй проход - по правилам SearchPath
    {
      /* $ 26.09.2003 VVM
        ! Сначала поищем по переменной PATH, а уж потом везде */
      char PathEnv[4096];
      if (GetEnvironmentVariable("PATH",PathEnv,sizeof(PathEnv)-1) != 0)
      {
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          if(!PtrFName)
            strcpy(WorkPtrFName,PtrExt);
          if(SearchPath(PathEnv,FullName,PtrExt,sizeof(FullName),FullName,&FilePart))
          {
            Ret=TRUE;
            break;
          }
          PtrExt+=strlen(PtrExt)+1;
        }
      }

      if (!Ret)
      {
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          if(!PtrFName)
            strcpy(WorkPtrFName,PtrExt);
          if(SearchPath(NULL,FullName,PtrExt,sizeof(FullName),FullName,&FilePart))
          {
            Ret=TRUE;
            break;
          }
          PtrExt+=strlen(PtrExt)+1;
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

        for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
        {
          sprintf(FullName,"%s%s",RegPath,FileName);
          if (RegOpenKeyEx(RootFindKey[I], FullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
          {
            DWORD Type, DataSize=sizeof(FullName);
            RegQueryValueEx(hKey,"", 0, &Type, (LPBYTE)FullName,&DataSize);
            RegCloseKey(hKey);
            /* $ 03.10.2001 VVM Обработать переменные окружения */
            strcpy(FileName, FullName);
            ExpandEnvironmentStrings(FileName,FullName,sizeof(FullName));
            Unquote(FullName);
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
            for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
            {
              sprintf(FullName,"%s%s%s",RegPath,FileName,PtrExt);
              if (RegOpenKeyEx(RootFindKey[I], FullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
              {
                DWORD Type, DataSize=sizeof(FullName);
                RegQueryValueEx(hKey,"", 0, &Type, (LPBYTE)FullName,&DataSize);
                RegCloseKey(hKey);
                /* $ 03.10.2001 VVM Обработать переменные окружения */
                strcpy(FileName, FullName);
                ExpandEnvironmentStrings(FileName,FullName,sizeof(FullName));
                Unquote(FullName);
                Ret=TRUE;
                break;
              }
            } /* for */
            PtrExt+=strlen(PtrExt)+1;
          }
        } /* if */
      } /* if */
    }
  }

  if(Ret) // некоторые "подмены" данных
  {
    // char TempStr[4096];
    // сначала проверим...
    IsCommandPEExeGUI(FullName,ImageSubsystem);
    /* $ 14.06.2002 VVM
       Не надо квотить - взяли без кавычек - так и отдадим...
    QuoteSpaceOnly(FullName);
    QuoteSpaceOnly(FileName);
      VVM $ */

    // Для случая, когда встретились скобки:
    /* $ 14.06.2002 VVM
       Скобки - допустимый символ в имени файла...
    if(strpbrk(FullName,"()"))
      IsExistExt=FALSE;
      VVM $ */

    // xstrncpy(TempStr,Command,sizeof(TempStr)-1);
    FAR_CharToOem(FullName,FullName);
    // FAR_CharToOem(FileName,FileName);
    // ReplaceStrings(TempStr,FileName,FullName);
    if(!DestSize)
      DestSize=(int)strlen(FullName);
    // if(Dest && IsExistExt)
    if (Dest)
      xstrncpy(Dest,FullName,DestSize);
  }

  SetFileApisTo(APIS2OEM);
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
void SetCurrentDirectoryForPassivePanel(const char *Comspec,const char *CmdStr)
{
  Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);

  if (PassivePanel->GetType()==FILE_PANEL)
  {
    //for (int I=0;CmdStr[I]!=0;I++)
    //{
      //if (isalpha(CmdStr[I]) && CmdStr[I+1]==':' && CmdStr[I+2]!='\\')
      //{
        char SavePath[NM],PanelPath[NM],SetPathCmd[NM*2];
        FarGetCurDir(sizeof(SavePath),SavePath);
        PassivePanel->GetCurDir(PanelPath);
        sprintf(SetPathCmd,"%s /C chdir %s",Comspec,QuoteSpace(PanelPath));
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset (&si, 0, sizeof (STARTUPINFO));
        si.cb = sizeof (si);
        if (CreateProcess(NULL,SetPathCmd,NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE,NULL,NULL,&si,&pi))
        {
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
        }
        FarChDir(SavePath);
        //break;
      //}
    //}
  }
}

/* Функция-пускатель внешних процессов
   Возвращает -1 в случае ошибки или...
*/
int Execute(const char *CmdStr,    // Ком.строка для исполнения
            int AlwaysWaitFinish,  // Ждать завершение процесса?
            int SeparateWindow,    // Выполнить в отдельном окне? =2 для вызова ShellExecuteEx()
            int DirectRun,         // Выполнять директом? (без CMD)
            int FolderRun)         // Это фолдер?
{
  int nResult = -1;

  bool bIsNT = (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

  char NewCmdStr[4096];
  char NewCmdPar[4096];
  char ExecLine[4096];

  memset (&NewCmdStr, 0, sizeof (NewCmdStr));
  memset (&NewCmdPar, 0, sizeof (NewCmdPar));

  PartCmdLine (
          CmdStr,
          NewCmdStr,
          sizeof(NewCmdStr),
          NewCmdPar,
          sizeof(NewCmdPar)
          );

  /* $ 05.04.2005 AY: Это не правильно, надо убирать только первый пробел,
                      что теперь и делает PartCmdLine.
  if(*NewCmdPar)
    RemoveExternalSpaces(NewCmdPar);
  AY $ */

  DWORD dwAttr = GetFileAttributes(NewCmdStr);

  if ( SeparateWindow == 1 )
  {
      if ( !*NewCmdPar && dwAttr != -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
      {
          ConvertNameToFull(NewCmdStr,NewCmdStr,sizeof(NewCmdStr));
          SeparateWindow=2;
          FolderRun=TRUE;
      }
  }


  SHELLEXECUTEINFO seInfo;
  memset (&seInfo, 0, sizeof (seInfo));

  seInfo.cbSize = sizeof (SHELLEXECUTEINFO);

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (STARTUPINFO));

  si.cb = sizeof (si);

  char Comspec[NM] = {0};
  GetEnvironmentVariable("COMSPEC", Comspec, sizeof(Comspec));

  if ( !*Comspec && (SeparateWindow != 2) )
  {
    Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
    return -1;
  }

  int Visible, Size;

  GetCursorType(Visible,Size);
  SetInitialCursorType();

  int PrevLockCount=ScrBuf.GetLockCount();
  // BUGBUG: если команда начинается с "@", то эта строка херит все начинания
  // TODO: здесь необходимо подставить виртуальный буфер, а потом его корректно подсунуть в ScrBuf
  ScrBuf.SetLockCount(0);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  int ConsoleCP = GetConsoleCP();
  int ConsoleOutputCP = GetConsoleOutputCP();

  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  GetConsoleScreenBufferInfo(hConOut,&sbi);

  char OldTitle[512];
  GetConsoleTitle (OldTitle, sizeof(OldTitle));

  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && *Comspec)
    SetCurrentDirectoryForPassivePanel(Comspec,CmdStr);

  DWORD dwSubSystem;
  DWORD dwError = 0;

  HANDLE hProcess = NULL, hThread = NULL;

  if(FolderRun && SeparateWindow==2)
    AddEndSlash(NewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
  else
  {
    PrepareExecuteModule(NewCmdStr,NewCmdStr,sizeof(NewCmdStr)-1,dwSubSystem);

    if(/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
    {
      DWORD Error=0, dwSubSystem2=0;
      char *ExtPtr=strrchr(NewCmdStr,'.');

      if(ExtPtr)
      {
        if(!(stricmp(ExtPtr,".exe")==0 || stricmp(ExtPtr,".com")==0 || IsBatchExtType(ExtPtr)) )
        {
          if(GetShellAction(NewCmdStr,dwSubSystem2,Error) && Error != ERROR_NO_ASSOCIATION)
            dwSubSystem=dwSubSystem2;
        }
        if(dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN && !memicmp(NewCmdStr,"ECHO.",5)) // вариант "echo."
        {
          *ExtPtr=' ';
          PartCmdLine (NewCmdStr,NewCmdStr,sizeof(NewCmdStr),NewCmdPar,sizeof(NewCmdPar));
          PrepareExecuteModule(NewCmdStr,NewCmdStr,sizeof(NewCmdStr)-1,dwSubSystem);
        }

      }
    }
    if ( dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
      SeparateWindow = 2;
  }

  ScrBuf.Flush ();

  if ( SeparateWindow == 2 )
  {
    FAR_OemToChar (NewCmdStr, NewCmdStr);
    FAR_OemToChar (NewCmdPar, NewCmdPar);

    seInfo.lpFile = NewCmdStr;
    seInfo.lpParameters = NewCmdPar;
    seInfo.nShow = SW_SHOWNORMAL;

    seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?NULL:GetShellAction((char *)NewCmdStr, dwSubSystem, dwError);
    //seInfo.lpVerb = "open";
    seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOZONECHECKS;

    if ( !dwError )
    {
      SetFileApisTo(APIS2ANSI);

      if ( ShellExecuteEx (&seInfo) )
      {
        hProcess = seInfo.hProcess;
        StartExecTime=clock();
      }
      else
        dwError = GetLastError ();

      SetFileApisTo(APIS2OEM);
    }

    FAR_CharToOem (NewCmdStr, NewCmdStr);
    FAR_CharToOem (NewCmdPar, NewCmdPar);
  }
  else
  {
    char FarTitle[2048];
    if(!Opt.ExecuteFullTitle)
      xstrncpy(FarTitle,CmdStr,sizeof(FarTitle)-1);
    else
    {
      xstrncpy(FarTitle,NewCmdStr,sizeof(FarTitle)-1);
      if (*NewCmdPar)
      {
        xstrncat(FarTitle," ",sizeof(FarTitle)-1);
        xstrncat(FarTitle,NewCmdPar,sizeof(FarTitle)-1);
      }
    }

    if ( bIsNT )
      SetConsoleTitle(FarTitle);
    FAR_OemToChar(FarTitle,FarTitle);
    if ( !bIsNT )
      SetConsoleTitle(FarTitle);
    if (SeparateWindow)
      si.lpTitle=FarTitle;

    QuoteSpace (NewCmdStr);

    strcpy (ExecLine, Comspec);
    strcat (ExecLine, " /C ");

    bool bDoubleQ = false;

    if ( bIsNT && strpbrk (NewCmdStr, "&<>()@^|=;,") )
      bDoubleQ = true;

    if ( (bIsNT && *NewCmdPar) || bDoubleQ )
      strcat (ExecLine, "\"");

    strcat (ExecLine, NewCmdStr);

    if ( *NewCmdPar )
    {
      strcat (ExecLine, " ");
      strcat (ExecLine, NewCmdPar);
    }

    if ( (bIsNT && *NewCmdPar) || bDoubleQ)
      strcat (ExecLine, "\"");

    // // попытка борьбы с синим фоном в 4NT при старте консоль
    SetRealColor (COL_COMMANDLINEUSERSCREEN);

    if ( CreateProcess (
        NULL,
        ExecLine,
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

//      char s[100];

//      sprintf (s, "%d %d", AlwaysWaitFinish, SeparateWindow);

//      MessageBox (0, s, s, MB_OK);

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
            if ( PeekConsoleInput(hHandles[1],ir,256,&rd) && rd)
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
                      hSmallIcon = CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)0));
                      hLargeIcon = CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)0));
                    }

                    ReadConsoleInput(hInput,ir,256,&rd);

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
                      SendMessage(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);

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
                        char FarName[NM];
                        GetModuleFileName(NULL,FarName,sizeof(FarName));
                        ExtractIconEx(FarName,0,&hLargeIcon,&hSmallIcon,1);
                      }

                      if ( hLargeIcon != NULL )
                        SendMessage (hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

                      if ( hSmallIcon != NULL )
                        SendMessage (hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
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
    char OutStr[1024];

    if( Opt.ExecuteShowErrorMessage )
    {
      SetMessageHelp("ErrCannotExecute");

      xstrncpy(OutStr,NewCmdStr,sizeof(OutStr)-1);
      Unquote(OutStr);

      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),OutStr,MSG(MOk));
    }
    else
    {
      ScrBuf.Flush ();

      sprintf(OutStr,MSG(MExecuteErrorMessage),NewCmdStr);
      char *PtrStr=FarFormatText(OutStr,ScrX,OutStr,sizeof(OutStr),"\n",0);
      printf(PtrStr);
      ScrBuf.FillBuf();
    }

  }

  SetFarConsoleMode(TRUE);

  /* Принудительная установка курсора, т.к. SetCursorType иногда не спасает
      вследствие своей оптимизации, которая в данном случае выходит боком.
  */
  SetCursorType(Visible,Size);
  SetRealCursorType(Visible,Size);

  SetConsoleTitle(OldTitle);

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


int CommandLine::CmdExecute(char *CmdLine,int AlwaysWaitFinish,int SeparateWindow,int DirectRun)
{
  LastCmdPartLength=-1;
  if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
  {
    /* $ 12.05.2001 DJ
       рисуемся только если остались верхним фреймом
    */
    if (CtrlObject->Cp()->IsTopFrame())
    {
      //CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      ScrBuf.Flush();
    }
    return(-1);
  }
  int Code;

  CONSOLE_SCREEN_BUFFER_INFO sbi0,sbi1;
  GetConsoleScreenBufferInfo(hConOut,&sbi0);
  {
     RedrawDesktop *Redraw=NULL;
     if(IsVisible() /* && ScrBuf.GetLockCount()==0 */)
        Redraw=new RedrawDesktop(TRUE);

    ScrollScreen(1);
    MoveCursor(X1,Y1);
    if (CurDir[0] && CurDir[1]==':')
      FarChDir(CurDir);
    CmdStr.SetString("");
    if ((Code=ProcessOSCommands(CmdLine,SeparateWindow)) == TRUE)
      Code=-1;
    else
    {
      char TempStr[2048];
      xstrncpy(TempStr,CmdLine,sizeof(TempStr)-1);
      if(Code == -1)
        ReplaceStrings(TempStr,"/","\\",-1);
      Code=Execute(TempStr,AlwaysWaitFinish,SeparateWindow,DirectRun);
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

    if(Redraw)
      delete Redraw;
  }

  if(!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
    ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
  /*else
  {
    CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
    CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
    CtrlObject->Cp()->Redraw();
  }*/

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
const char* WINAPI PrepareOSIfExist(const char *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

  char Cmd[1024];
  const char *PtrCmd=CmdLine, *CmdStart;
  int Not=FALSE;
  int Exist=0; // признак наличия конструкции "IF [NOT] EXIST filename command"
               // > 0 - эсть такая конструкция

  /* $ 25.04.2001 DJ
     обработка @ в IF EXIST
  */
  if (*PtrCmd == '@')
  {
    // здесь @ игнорируется; ее вставит в правильное место функция
    // ExtractIfExistCommand в filetype.cpp
    PtrCmd++;
    while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
  }
  /* DJ $ */
  while(1)
  {
    if (!PtrCmd || !*PtrCmd || memicmp(PtrCmd,"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (memicmp(PtrCmd,"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !memicmp(PtrCmd,"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;

      /* $ 25.04.01 DJ
         обработка кавычек внутри имени файла в IF EXIST
      */
      BOOL InQuotes=FALSE;
      while (*PtrCmd)
      {
        if (*PtrCmd == '\"')
          InQuotes = !InQuotes;
        else if (*PtrCmd == ' ' && !InQuotes)
          break;
        PtrCmd++;
      }

      if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
      {
        char ExpandedStr[8192];
        memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
        Cmd[PtrCmd-CmdStart]=0;
        Unquote(Cmd);
//_SVS(SysLog("Cmd='%s'",Cmd));
        if (ExpandEnvironmentStr(Cmd,ExpandedStr,sizeof(ExpandedStr))!=0)
        {
          char FullPath[8192]="";
          if(!(Cmd[1] == ':' || (Cmd[0] == '\\' && Cmd[1]=='\\') || ExpandedStr[1] == ':' || (ExpandedStr[0] == '\\' && ExpandedStr[1]=='\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(FullPath);
            else
              FarGetCurDir(sizeof(FullPath),FullPath);
            AddEndSlash(FullPath);
          }
          strcat(FullPath,ExpandedStr);
          DWORD FileAttr=(DWORD)-1;
          if(strpbrk(&ExpandedStr[PathPrefix(ExpandedStr)?4:0],"*?")) // это маска?
          {
            WIN32_FIND_DATA wfd;
            if(GetFileWin32FindData(FullPath,&wfd))
              FileAttr=wfd.dwFileAttributes;
          }
          else
          {
            ConvertNameToFull(FullPath, FullPath, sizeof(FullPath));
            FileAttr=GetFileAttributes(FullPath);
          }
//_SVS(SysLog("%08X FullPath=%s",FileAttr,FullPath));
          if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
          {
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return "";
        }
      }
      /* DJ $ */
    }
    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !memicmp(PtrCmd,"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == '"')
        PtrCmd=strchr(PtrCmd+1,'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=strchr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          char ExpandedStr[8192];
          memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          DWORD ERet=GetEnvironmentVariable(Cmd,ExpandedStr,sizeof(ExpandedStr));
//_SVS(SysLog(Cmd));
          if(ERet && !Not || !ERet && Not)
          {
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return "";
        }
      }
    }
  }
  return Exist?PtrCmd:NULL;
}
/* SVS $ */
/* IS $ */

int CommandLine::ProcessOSCommands(char *CmdLine,int SeparateWindow)
{
  Panel *SetPanel;
  int Length;

  SetPanel=CtrlObject->Cp()->ActivePanel;

  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

  RemoveTrailingSpaces(CmdLine);

  bool SilentInt=false;
  if(*CmdLine == '@')
  {
    SilentInt=true;
    CmdLine++;
  }

  if (!SeparateWindow && isalpha(CmdLine[0]) && CmdLine[1]==':' && CmdLine[2]==0)
  {
    char NewDir[10];
    sprintf(NewDir,"%c:",toupper(CmdLine[0]));
    FarChDir(CmdLine);
    if (getdisk()!=NewDir[0]-'A')
    {
      strcat(NewDir,"\\");
      FarChDir(NewDir);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }

  // SET [переменная=[строка]]
  else if (!strnicmp(CmdLine,"SET",3) && IsSpace(CmdLine[3]))
  {
    char Cmd[1024];
#if 0
    // Вариант для "SET /P variable=[promptString]"
    int Offset=4, NeedInput=FALSE;
    char *ParamP=strchr(CmdLine,'/');
    if (ParamP && (ParamP[1] == 'P' || ParamP[1] == 'p') && ParamP[2] == ' ')
    {
      Offset=ParamP-CmdLine+3;
      NeedInput=TRUE;
    }

    xstrncpy(Cmd,CmdLine+Offset,sizeof(Cmd)-1);

    char *Value=strchr(Cmd,'=');
    if (Value==NULL)
      return(FALSE);

    *Value=0;
    if(NeedInput)
    {
      Offset=Value-Cmd+1;
      if(!::GetString("",Value+1,"PromptSetEnv","",Value+1,sizeof(Cmd)-Offset-1,NULL,FIB_ENABLEEMPTY))
        return TRUE;
    }
#else
    xstrncpy(Cmd,CmdLine+4,sizeof(Cmd)-1);
    char *Value=strchr(Cmd,'=');
    if (Value==NULL)
      return(FALSE);

    *Value=0;
#endif
    FAR_OemToChar(Cmd, Cmd);

    if (Value[1]==0)
      SetEnvironmentVariable(Cmd,NULL);
    else
    {
      char ExpandedStr[8192];
      /* $ 17.06.2001 IS
         ! Применяем ExpandEnvironmentStr, т.к. она корректно работает с
           русскими буквами.
         + Перекодируем строки перед SetEnvironmentVariable из OEM в ANSI
      */
      if (ExpandEnvironmentStr(Value+1,ExpandedStr,sizeof(ExpandedStr))!=0)
      {
        // переменные окружения должны быть в ANSI???
        FAR_OemToChar(ExpandedStr, ExpandedStr);
        SetEnvironmentVariable(Cmd,ExpandedStr);
      }
      /* IS $ */
    }
    return(TRUE);
  }

  else if ((!strnicmp(CmdLine,"REM",3) && IsSpace(CmdLine[3])) || !strnicmp(CmdLine,"::",2))
  {
    return TRUE;
  }

  else if (!stricmp(CmdLine,"CLS"))
  {
    ClearScreen(COL_COMMANDLINEUSERSCREEN);
    return TRUE;
  }

  // PUSHD путь | ..
  else if (!strnicmp(CmdLine,"PUSHD",5) && IsSpace(CmdLine[5]))
  {
    char *Ptr=RemoveExternalSpaces(CmdLine+6);
    if(!stricmp(Ptr,"/?") || !stricmp(Ptr,"-?"))
      return FALSE; // пусть cmd скажет про синтаксис
    PushPopRecord prec;
    prec.Name = xf_strdup(CurDir);
    int ret=IntChDir(Ptr,true,SilentInt);
    if(ret)
    {
      ppstack.Push(prec);
      FAR_OemToChar(prec.Name, prec.Name);
      SetEnvironmentVariable("FARDIRSTACK",prec.Name);
    }
    else
    {
      ;
    }
    return TRUE;
  }

  // POPD [n] <--todo
  else if (!stricmp(CmdLine,"POPD"))
  {
    PushPopRecord prec;
    if(ppstack.Pop(prec))
    {
      int Ret=IntChDir(prec.Name,true,SilentInt);
      PushPopRecord *ptrprec=ppstack.Peek();
      if(ptrprec)
        FAR_OemToChar(ptrprec->Name, ptrprec->Name);
      SetEnvironmentVariable("FARDIRSTACK",ptrprec?ptrprec->Name:NULL);
      return Ret;
    }
    return TRUE;
  }

  // CLRD
  else if (!stricmp(CmdLine,"CLRD"))
  {
    ppstack.Free();
    SetEnvironmentVariable("FARDIRSTACK",NULL);
    return TRUE;
  }

  /*
  Displays or sets the active code page number.
  CHCP [nnn]
    nnn   Specifies a code page number (Dec or Hex).
  Type CHCP without a parameter to display the active code page number.
  */
  else if (!strnicmp(CmdLine,"CHCP",4) && IsSpace(CmdLine[4]))
  {
    char *Ptr=RemoveExternalSpaces(CmdLine+5), Chr;

    if(!isdigit(*Ptr))
      return FALSE;

    while((Chr=*Ptr) != 0)
    {
      if(!isdigit(Chr))
        break;
      ++Ptr;
    }
    UINT cp=(UINT)strtol(CmdLine+5,&Ptr,10);
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

  /* $ 14.01.2001 SVS
     + В ProcessOSCommands добавлена обработка
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  */
  else if (!strnicmp(CmdLine,"IF",2) && IsSpace(CmdLine[2]))
  {
    const char *PtrCmd=PrepareOSIfExist(CmdLine);
    // здесь PtrCmd - уже готовая команда, без IF
    if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
    {
      //CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return TRUE;
    }
    return FALSE;
  }
  /* SVS $ */

  /* $ 16.04.2002 DJ
     пропускаем обработку, если нажат Shift-Enter
  */
  else if (!SeparateWindow &&  /* DJ $ */
      (strnicmp(CmdLine,"CD",Length=2)==0 || strnicmp(CmdLine,"CHDIR",Length=5)==0) &&
      (IsSpace(CmdLine[Length]) || CmdLine[Length]=='\\' || CmdLine[Length]=='/' || TestParentFolderName(CmdLine+Length)))
  {
    int ChDir=(Length==5);

    while (IsSpace(CmdLine[Length]))
      Length++;

    IntChDir(CmdLine+Length,ChDir,SilentInt);

    return(TRUE);
  }
  else if(!stricmp(CmdLine,"EXIT"))
  {
    FrameManager->ExitMainLoop(FALSE);
    return TRUE;
  }

  return(FALSE);
}

BOOL CommandLine::IntChDir(const char *CmdLine,int ClosePlugin,bool Selent)
{
  Panel *SetPanel;

  SetPanel=CtrlObject->Cp()->ActivePanel;

  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

  //RemoveTrailingSpaces(CmdLine);

  char ExpandedDir[8192];
  xstrncpy(ExpandedDir,CmdLine,sizeof(ExpandedDir)-1);

  Unquote(ExpandedDir);
  ExpandEnvironmentStr(ExpandedDir,ExpandedDir,sizeof(ExpandedDir));

  // скорректируем букву диска на "подступах"
  if(ExpandedDir[1] == ':' && isalpha(ExpandedDir[0]))
    ExpandedDir[0]=toupper(ExpandedDir[0]);

  if(SetPanel->GetMode()!=PLUGIN_PANEL && ExpandedDir[0] == '~' && ( !ExpandedDir[1] && GetFileAttributes(ExpandedDir) == (DWORD)-1 || IsSlash(ExpandedDir[1]) ) )
  {
    char tempExpandedDir[8192];
    GetRegKey(strSystemExecutor,"~",(char*)tempExpandedDir,FarPath,sizeof(tempExpandedDir)-1);
    if(ExpandedDir[1])
    {
      AddEndSlash(tempExpandedDir);
      xstrncat(tempExpandedDir,ExpandedDir+2,sizeof(tempExpandedDir)-1);
    }
    DeleteEndSlash(tempExpandedDir);
    xstrncpy(ExpandedDir,tempExpandedDir,sizeof(ExpandedDir)-1);
  }

  if(strpbrk(&ExpandedDir[PathPrefix(ExpandedDir)?4:0],"?*")) // это маска?
  {
    WIN32_FIND_DATA wfd;
    if(GetFileWin32FindData(ExpandedDir,&wfd))
    {
      char *Ptr=strrchr(ExpandedDir,'\\');
      if (!Ptr)
        Ptr=strrchr(ExpandedDir,'/');
      if(Ptr)
        *++Ptr=0;
      else
        *ExpandedDir=0;
      xstrncat(ExpandedDir,wfd.cFileName,sizeof(ExpandedDir)-1);
    }
  }

  // \\?\UNC\<hostname>\<sharename>\<dirname> , а также cd \\.\<disk>:\<dirname>

  /* $ 15.11.2001 OT
    Сначала проверяем есть ли такая "обычная" директория.
    если уж нет, то тогда начинаем думать, что это директория плагинная
  */
  DWORD DirAtt=GetFileAttributes(ExpandedDir);
  if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsolute(ExpandedDir))
  {
    ReplaceStrings(ExpandedDir,"/","\\",-1);
    SetPanel->SetCurDir(ExpandedDir,TRUE);
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
    //CmdStr.SetString("");
    GotoXY(X1,Y1);
    mprintf("%*s",X2-X1+1,"");
    Show();
    return(TRUE);
  }
  */
  /* SKV $ */

  if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
  {
    SetPanel->SetCurDir(ExpandedDir,ClosePlugin);
    return(TRUE);
  }

  if(FarChDir(ExpandedDir))
  {
    SetPanel->ChangeDirToCurrent();
    if(!SetPanel->IsVisible())
      SetPanel->SetTitle();
  }
  else
  {
    if(!Selent)
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),ExpandedDir,MSG(MOk));
    return FALSE;
  }
  return TRUE;
}

// Проверить "Это батник?"
BOOL IsBatchExtType(const char *ExtPtr)
{
  char *PtrBatchType=Opt.ExecuteBatchType;
  while(*PtrBatchType)
  {
    if(stricmp(ExtPtr,PtrBatchType)==0)
      return TRUE;
    PtrBatchType+=strlen(PtrBatchType)+1;
  }

  return FALSE;
}

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
        xstrncpy(DestName,FullName,SizeDestName);
        Result=TRUE;
        break;
      }

      PtrBatchType+=strlen(PtrBatchType)+1;
    }
  }

  return Result;
}
