/*
execute.cpp

"Запускатель" программ.

*/

/* Revision: 1.22 07.12.2001 $ */

/*
Modify:
  07.12.2001 SVS
    ! Уточнения в новом исполняторе (их еще будет море ;-))
    ! Из CommandLine::CmdExecute() гашение панелей перенесено в RedrawDesktop
    ! В новом исполняторе введено понятие исклительных команд, которые,
      если попадаются, то не исполняются!
    ! DWORD* переделан в DWORD&
    ! У Execute команда (первый параметр) - const
  06.12.2001 SVS
    ! Откат к старому обработчику с возможностью работы с новым.
      Детельное описание читайте в 01104.Mix.txt
  05.12.2001 SVS
    - При определении ассоциации забыл "расширить" переменные среды :-(
  04.12.2001 SVS
    - забыл выделить имя модуля (не учитывались кавычки в активаторе)
  04.12.2001 SVS
    ! Очередное уточнение пусковика. На этот раз... при старте DOC-файлов
      ФАР ждет завершения. Выход из положения - "посмотрить" на гуевость
      пусковика.
  03.12.2001 SVS
    ! Уточнение для... пути со скобками :-)
    ! Новое поведение - убрали DETACHED_PROCESS и ждем завершение процесса.
  02.12.2001 SVS
    ! Неверный откат. :-(( Вернем все обратно, но с небольшой правкой,
      не будем изменять строку запуска, если явно не указано расширение, т.е.
      если вводим "date" - исполняется внутренняя команда ком.проц., если
      вводим "date.exe", то будет искаться и исполняться именно "date.exe"
      В остальном все должно быть как и раньше.
  30.11.2001 SVS
    ! Почти полный откат предыдущих наворотов с пусковиком
  29.11.2001 SVS
    - Бага с русскими буковками - забыли конвертнуть путь обратно в OEM.
  28.11.2001 SVS
    - BugZ#129 не запускаются программым с пробелом в названии
    ! небольшие уточнения в PrepareExecuteModule()
  22.11.2001 SVS
    - Как последний гад этот самый Екзекутеор валит ФАР на простой формуле:
      >"" Enter
      Ок. Будем вылавливать еще на подходе.
    + У Execute() добавлен параметр - SetUpDirs "Нужно устанавливать каталоги?"
      Это как раз про ту войну, когда Костя "отлучил" кусок кода про
      установку каталогов. Это понадобится гораздо позже.
  21.11.2001 SVS
    ! Объединение и небольшое "усиление" кода пусковика, а так же
      переименование IsCommandExeGUI в PrepareExecuteModule (фактически
      новая функция). GetExistAppPaths удалена за ненадобностью.
  21.11.2001 VVM
    ! Очереднйо перетрях прорисовки при запуске программ.
  20.11.2001 SVS
    - BugZ#111 - для cd Це: скорректируем букву диска - сделаем ее Upper.
  20.11.2001 SVS
    ! Уточнение пусковика.
  19.11.2001 SVS
    + GetExistAppPaths() - получить если надо путь из App Paths
    ! Функция IsCommandExeGUI() вторым параметром возврпащает полный путь
      к наденному файлу
  15.11.2001 OT
    - Исправление поведения cd c:\ на активном панельном плагине
  14.11.2001 SVS
    - Последствия исправлений BugZ#90 - панели не обновлялись
  12.11.2001 SVS
    - BugZ#90: панель остается на экране
  12.11.2001 SVS
    ! откат 1033 и 1041 до лучших времен.
  08.11.2001 SVS
    - неудачная попытка (возможно и ЭТОТ патч неудачный) запуска (про каталоги)
  31.10.2001 VVM
    + Попытка переделать запуск программ. Стараемся пускать не через "start.exe",
      а через CREATE_NEW_CONSOLE
  10.10.2001 SVS
    + Создан
*/

#include "headers.hpp"
#pragma hdrstop

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

// Выдранный кусок из будущего GetFileInfo, получаем достоверную информацию о
// ГУЯХ PE-модуля
static int IsCommandPEExeGUI(const char *FileName,DWORD& IsPEGUI)
{
  char NameFile[NM];
  HANDLE hFile;
  int Ret=FALSE;
  IsPEGUI=0;

  if((hFile=CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
  {
    DWORD FileSizeLow, FileSizeHigh, ReadSize;
    IMAGE_DOS_HEADER dos_head;

    FileSizeLow=GetFileSize(hFile,&FileSizeHigh);

    if(ReadFile(hFile,&dos_head,sizeof(IMAGE_DOS_HEADER),&ReadSize,NULL) &&
       dos_head.e_magic == IMAGE_DOS_SIGNATURE)
    {
      Ret=TRUE;
      /*  Если значение слова по смещению 18h (OldEXE - MZ) >= 40h,
      то значение слова в 3Ch является смещением заголовка Windows. */
      if (dos_head.e_lfarlc >= 0x40)
      {
        DWORD signature;
        #include <pshpack1.h>
        struct __HDR
        {
           DWORD signature;
           IMAGE_FILE_HEADER _head;
           IMAGE_OPTIONAL_HEADER opt_head;
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
              IsPEGUI=1;
              IsPEGUI|=(header.opt_head.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)?2:0;
            }
            else if((WORD)signature == IMAGE_OS2_SIGNATURE) // NE
            {
              ; // NE,  хмм...  а как определить что оно ГУЕВОЕ?
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
      else
      {
        ; // Это конечно EXE, но не виндовое EXE
      }
    }
    else
    {
      ; // это не исполняемый файл - у него нету заголовка MZ
        // например, NLM-модуль или ошибка чтения :-)
    }
    CloseHandle(hFile);
  }
  return Ret;
}

// по имени файла (по его расширению) получить команду активации
// Дополнительно смотрится гуевость команды-активатора
// (чтобы не ждать завершения)
char* GetShellAction(const char *FileName,DWORD& GUIType)
{
  char Value[512];
  const char *ExtPtr;
  char *RetPtr;
  LONG ValueSize;

  GUIType=0;

  if ((ExtPtr=strrchr(FileName,'.'))==NULL)
    return(NULL);

  ValueSize=sizeof(Value);

  if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)!=ERROR_SUCCESS)
    return(NULL);

  strcat(Value,"\\shell");

  HKEY hKey;
  if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static char Action[80];
  ValueSize=sizeof(Action);
  LONG RetQuery=RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)Action,(LPDWORD)&ValueSize);
  RegCloseKey(hKey);

  if (RetQuery == ERROR_SUCCESS)
  {
    RetPtr=(*Action==0 ? NULL:Action);
    strcat(Value,"\\");
    strcat(Value,Action);
  }
  else
  {
    // This member defaults to "Open" if no verb is specified.
    // Т.е. если мы вернули NULL, то подразумевается команда "Open"
    RetPtr=NULL;
    strcat(Value,"\\open");
  }
  strcat(Value,"\\command");

  // а теперь проверим ГУЕвость запускаемой проги
  if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)==ERROR_SUCCESS)
  {
    char Command[2048];
    ValueSize=sizeof(Command);
    RetQuery=RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)Command,(LPDWORD)&ValueSize);
    RegCloseKey(hKey);
    if(RetQuery == ERROR_SUCCESS)
    {
      char *Ptr;
      ExpandEnvironmentStr(Command,Command,sizeof(Command));
      // Выделяем имя модуля
      if (*Command=='\"')
      {
        OemToChar(Command+1,Command);
        if ((Ptr=strchr(Command,'\"'))!=NULL)
          *Ptr=0;
      }
      else
      {
        OemToChar(Command,Command);
        if ((Ptr=strpbrk(Command," \t/"))!=NULL)
          *Ptr=0;
      }
      IsCommandPEExeGUI(Command,GUIType);
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
int WINAPI PrepareExecuteModule(const char *Command,char *Dest,int DestSize,DWORD& GUIType)
{
  int Ret, I;
  char FileName[4096],FullName[4096], *Ptr;
  int IsQuoted=FALSE;
  int IsExistExt=FALSE;

  // Здесь порядок важен! Сначала батники,  а потом остальная фигня.
  static char StdExecuteExt[NM]=".BAT;.CMD;.EXE;.COM;";
  static int PreparePrepareExt=FALSE;

  if(!PreparePrepareExt) // самоинициилизирующийся кусок
  {
    // если переменная %PATHEXT% доступна...
    if((I=GetEnvironmentVariable("PATHEXT",FullName,sizeof(FullName)-1)) != 0)
    {
      FullName[I]=0;
      // удаляем дубляжи из PATHEXT
      static char const * const StdExecuteExt0[4]={".BAT;",".CMD;",".EXE;",".COM;"};
      for(I=0; I < sizeof(StdExecuteExt0)/sizeof(StdExecuteExt0[0]); ++I)
        ReplaceStrings(FullName,StdExecuteExt0[I],"",-1);
    }

    Ptr=strcat(StdExecuteExt,strcat(FullName,";")); //!!!
    StdExecuteExt[strlen(StdExecuteExt)]=0;
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
  static char ExcludeCmds[4096];
  static int PrepareExcludeCmds=FALSE;
  if(!PrepareExcludeCmds)
  {
    GetRegKey("System\\Executor","ExcludeCmds",(char*)ExcludeCmds,"",0);
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

  GUIType=0; // GUIType всегда вначале инициализируется в FALSE
  Ret=FALSE;

  // Выделяем имя модуля
  if (*Command=='\"')
  {
    OemToChar(Command+1,FileName);
    if ((Ptr=strchr(FileName,'\"'))!=NULL)
      *Ptr=0;
    IsQuoted=TRUE;
  }
  else
  {
    OemToChar(Command,FileName);
    if ((Ptr=strpbrk(FileName," \t/|><"))!=NULL)
      *Ptr=0;
  }

  if(!*FileName) // вот же, надо же... пустышку передали :-(
    return 0;

  /* $ 07.09.2001 VVM Обработать переменные окружения */
  ExpandEnvironmentStr(FileName,FileName,sizeof(FileName));

  // нулевой проход - смотрим исключения
  {
    char *Ptr=ExcludeCmds;
    while(*Ptr)
    {
      if(!stricmp(FileName,Ptr))
      {
        return TRUE;
      }
      Ptr+=strlen(Ptr)+1;
    }
  }

  // IsExistExt - если точки нету (расширения), то потом модифицировать не
  // будем.
  IsExistExt=strrchr(FileName,'.')!=NULL;

  SetFileApisToANSI();

  {
    char *FilePart;
    char *PtrFName=strrchr(strcpy(FullName,FileName),'.');
    char *WorkPtrFName;
    if(!PtrFName)
      WorkPtrFName=FullName+strlen(FullName);

    char *PtrExt=StdExecuteExt;
    while(*PtrExt) // первый проход - в текущем каталоге
    {
      if(!PtrFName)
        strcpy(WorkPtrFName,PtrExt);
      if(GetFileAttributes(FullName)!=-1)
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

      if(!Ret) // третий проход - лезим в реестр в "App Paths"
      {
        // В строке Command заменть исполняемый модуль на полный путь, который
        // берется из SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
        // Сначала смотрим в HKCU, затем - в HKLM
        HKEY hKey;
        HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          if(!PtrFName)
            strcpy(WorkPtrFName,PtrExt);
          for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
          {
            sprintf(FullName,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s",FileName);
            if (RegOpenKeyEx(RootFindKey[I], FullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
              DWORD Type, DataSize=sizeof(FullName);
              RegQueryValueEx(hKey,"", 0, &Type, (LPBYTE)FullName,&DataSize);
              RegCloseKey(hKey);
              Ret=TRUE;
              break;
            }
          }
          if(Ret)
            break;
          PtrExt+=strlen(PtrExt)+1;
        }
      }
    }
  }

  if(Ret) // некоторые "подмены" данных
  {
    char TempStr[4096];
    // сначала проверим...
    IsCommandPEExeGUI(FullName,GUIType);
    QuoteSpaceOnly(FullName);
    QuoteSpaceOnly(FileName);

    // Для случая, когда встретились скобки:
    if(strpbrk(FullName,"()"))
      IsExistExt=FALSE;

    strncpy(TempStr,Command,sizeof(TempStr)-1);
    CharToOem(FullName,FullName);
    CharToOem(FileName,FileName);
    ReplaceStrings(TempStr,FileName,FullName);
    if(!DestSize)
      DestSize=strlen(TempStr);
    if(Dest && IsExistExt)
      strncpy(Dest,TempStr,DestSize);
  }

  SetFileApisToOEM();
  return(Ret);
}

DWORD IsCommandExeGUI(const char *Command)
{
  char FileName[4096],FullName[4096],*EndName,*FilePart;
  if (*Command=='\"')
  {
    OemToChar(Command+1,FileName);
    if ((EndName=strchr(FileName,'\"'))!=NULL)
      *EndName=0;
  }
  else
  {
    OemToChar(Command,FileName);
    if ((EndName=strpbrk(FileName," \t/"))!=NULL)
      *EndName=0;
  }
  int GUIType=0;

  /* $ 07.09.2001 VVM
    + Обработать переменные окружения */
  ExpandEnvironmentStr(FileName,FileName,sizeof(FileName));
  /* VVM $ */

  SetFileApisToANSI();

  /*$ 18.09.2000 skv
    + to allow execution of c.bat in current directory,
      if gui program c.exe exists somewhere in PATH,
      in FAR's console and not in separate window.
      for(;;) is just to prevent multiple nested ifs.
  */
  for(;;)
  {
    sprintf(FullName,"%s.bat",FileName);
    if(GetFileAttributes(FullName)!=-1)break;
    sprintf(FullName,"%s.cmd",FileName);
    if(GetFileAttributes(FullName)!=-1)break;
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
  SetFileApisToOEM();
  return(GUIType);
}

/* Функция-пускатель внешних процессов
   Возвращает -1 в случае ошибки или...
*/
int Execute(const char *CmdStr,          // Ком.строка для исполнения
            int AlwaysWaitFinish,  // Ждать завершение процесса?
            int SeparateWindow,    // Выполнить в отдельном окне? =2 для вызова ShellExecuteEx()
            int DirectRun,         // Выполнять директом? (без CMD)
            int SetUpDirs)         // Нужно устанавливать каталоги?
{
  char NewCmdStr[4096];

  // ПРЕДпроверка на вшивость
  Unquote(strcpy(NewCmdStr,CmdStr));
  RemoveExternalSpaces(NewCmdStr);
  // глянем на результат
  if(!*NewCmdStr)
  {
    // А может просто запустить CMD или проводник?
    // если "да", то этот куско нужно ниже перенести.
    return -1;
  }

  CONSOLE_SCREEN_BUFFER_INFO sbi;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  int Visible,Size;
  int PrevLockCount;
  char ExecLine[1024],CommandName[NM];
  char OldTitle[512];
  DWORD GUIType;
  int ExitCode=1;
  int NT;
  int OldNT;
  DWORD CreateFlags;
  char *CmdPtr;

  /* $ 13.04.2001 VVM
    + Флаг CREATE_DEFAULT_ERROR_MODE. Что-бы показывал все ошибки */
  CreateFlags=CREATE_DEFAULT_ERROR_MODE;
  /* VVM $ */

  GetCursorType(Visible,Size);
  SetCursorType(TRUE,-1);

  PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);
  ScrBuf.Flush();

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  NT=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT;
  OldNT=NT && WinVer.dwMajorVersion<4;

  *CommandName=0;
  GetEnvironmentVariable("COMSPEC",CommandName,sizeof(CommandName));

  if(SetUpDirs)
  {
    Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && PassivePanel->GetType()==FILE_PANEL)
      for (int I=0;CmdStr[I]!=0;I++)
        if (isalpha(CmdStr[I]) && CmdStr[I+1]==':' && CmdStr[I+2]!='\\')
        {
          char SavePath[NM],PanelPath[NM],SetPathCmd[NM];
          GetCurrentDirectory(sizeof(SavePath),SavePath);
          PassivePanel->GetCurDir(PanelPath);
          sprintf(SetPathCmd,"%s /C chdir %s",CommandName,QuoteSpace(PanelPath));
          CreateProcess(NULL,SetPathCmd,NULL,NULL,FALSE,CreateFlags,NULL,NULL,&si,&pi);
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
          chdir(SavePath);
        }
  }

  CmdPtr=strcpy(NewCmdStr,CmdStr);
  //while (isspace(*CmdPtr))
  //  CmdPtr++;

  int  ExecutorType;
  ExecutorType=GetRegKey("System\\Executor","Type",0);

  if(ExecutorType)
  {
    // Поиск исполнятора....
    if (SeparateWindow!=2)
      ExitCode=PrepareExecuteModule(NewCmdStr,NewCmdStr,sizeof(NewCmdStr)-1,GUIType);
    /*
      Если ExitCode=0, значит ненашли и, следовательно нефига
      запусками заниматься
    */
  }
  else
    GUIType=IsCommandExeGUI(CmdPtr);

  if(ExitCode)
  {
    if (DirectRun && !SeparateWindow)
      strcpy(ExecLine,CmdPtr);
    else
    {
      if(ExecutorType)
      {
        if(GUIType && !AlwaysWaitFinish)
          strcpy(ExecLine,NewCmdStr);
        else
        {
          char TemplExecute[512];
          char TemplExecuteStart[512];
          char TemplExecuteWait[512];
          // <TODO: здесь надо по другому переделать>
          GetRegKey("System\\Executor","Normal",TemplExecute,"%s /c %s %s",sizeof(TemplExecute));
          GetRegKey("System\\Executor","Start",TemplExecuteStart,"%s /c start %s %s",sizeof(TemplExecuteStart));
          GetRegKey("System\\Executor","Wait",TemplExecuteWait,"%s /c start /wait %s %s",sizeof(TemplExecuteWait));

          char *Fmt=TemplExecute;
          if (!OldNT && (SeparateWindow || GUIType && (NT || AlwaysWaitFinish)))
          {
            Fmt=TemplExecuteStart;
            if (AlwaysWaitFinish)
              Fmt=TemplExecuteWait;
          }
          char *CmdEnd=NewCmdStr+strlen(NewCmdStr)-1;
          if (NT && *NewCmdStr == '\"' && *CmdEnd == '\"' &&
             strchr(NewCmdStr+1, '\"') != CmdEnd && SeparateWindow!=2)
            InsertQuote(NewCmdStr);
          sprintf(ExecLine,Fmt,
                           CommandName,
                           (Fmt!=TemplExecute && NT && *CmdPtr=='\"'?"\"\"":""),
                           NewCmdStr);
          // </TODO>
        }
      }
      else
      {
        sprintf(ExecLine,"%s /C",CommandName);
        if (!OldNT && (SeparateWindow || GUIType && (NT || AlwaysWaitFinish)))
        {
          strcat(ExecLine," start");
          if (AlwaysWaitFinish)
            strcat(ExecLine," /wait");
          if (NT && *CmdPtr=='\"')
            strcat(ExecLine," \"\"");
        }
        strcat(ExecLine," ");

        char *CmdEnd=CmdPtr+strlen (CmdPtr)-1;
        if (NT && *CmdPtr == '\"' && *CmdEnd == '\"' && strchr (CmdPtr+1, '\"') != CmdEnd)
        {
          strcat (ExecLine, "\"");
          strcat (ExecLine, CmdPtr);
          strcat (ExecLine, "\"");
        }
        else
          strcat(ExecLine,CmdPtr);
      }
    }

    SetFarTitle(CmdPtr);
    FlushInputBuffer();

    /*$ 15.03.2001 SKV
      Надо запомнить параметры консоли ДО запуск и т.д.
    */
    GetConsoleScreenBufferInfo(hConOut,&sbi);
    /* SKV$*/

    ChangeConsoleMode(InitialConsoleMode);

    if (SeparateWindow)
      CreateFlags|=(OldNT)?CREATE_NEW_CONSOLE:0;//DETACHED_PROCESS;

    if (SeparateWindow==2)
    {
      char AnsiLine[4096];
      SHELLEXECUTEINFO si;
      OemToChar(CmdPtr,AnsiLine);

      if (PointToName(AnsiLine)==AnsiLine)
      {
        char FullName[2*NM];
        sprintf(FullName,".\\%s",AnsiLine);
        strcpy(AnsiLine,FullName);
      }
      Unquote(AnsiLine); // т.к. нафиг это ненужно?

      memset(&si,0,sizeof(si));
      si.cbSize=sizeof(si);
      si.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
      si.lpFile=AnsiLine;
      si.lpVerb=GetShellAction((char *)si.lpFile,GUIType);
      si.nShow=SW_SHOWNORMAL;
      SetFileApisToANSI();
      ExitCode=ShellExecuteEx(&si);
      SetFileApisToOEM();
      pi.hProcess=si.hProcess;
    }
    else
      ExitCode=CreateProcess(NULL,ExecLine,NULL,NULL,0,CreateFlags,
                             NULL,NULL,&si,&pi);

    StartExecTime=clock();
  }

  if (ExitCode)
  {
    if (!SeparateWindow && !GUIType || AlwaysWaitFinish)
    {
      /*$ 12.02.2001 SKV
        супер фитча ;)
        Отделение фаровской консоли от неинтерактивного процесса.
        Задаётся кнопкой в System/ConsoleDetachKey
      */
      if(Opt.ConsoleDetachKey>0)
      {
        HANDLE h[2];
        HANDLE hConOut=GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
        h[0]=pi.hProcess;
        h[1]=hConInp;
        INPUT_RECORD ir[256];
        DWORD rd;

        int vkey=0,ctrl=0;
        TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,NULL);
        int alt=ctrl&PKF_ALT;
        int shift=ctrl&PKF_SHIFT;
        ctrl=ctrl&PKF_CONTROL;

        while(WaitForMultipleObjects(2,h,FALSE,INFINITE)!=WAIT_OBJECT_0)
        {
          if(PeekConsoleInput(h[1],ir,256,&rd) && rd)
          {
            int stop=0;
            for(int i=0;i<rd;i++)
            {
              PINPUT_RECORD pir=&ir[i];
              if(pir->EventType==KEY_EVENT)
              {
                if(vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
                  (alt?(pir->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED)) &&
                  (ctrl?(pir->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)) &&
                  (shift?(pir->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED))
                  )
                {

                  HICON hSmallIcon=NULL,hLargeIcon=NULL;
                  if(hFarWnd)
                  {
                    hSmallIcon=CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)0));
                    hLargeIcon=CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)0));
                  }
                  ReadConsoleInput(hConInp,ir,256,&rd);
                  CloseConsole();
                  FreeConsole();
                  AllocConsole();

                  /*$ 17.05.2001 SKV
                    если окно имело HOTKEY, то старое должно его забыть.
                  */
                  if(hFarWnd)
                  {
                    SendMessage(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);
                  }
                  /* SKV$*/


                  /*$ 20.03.2001 SKV
                    вот такой вот изврат :-\
                  */
                  SetConsoleScreenBufferSize(hConOut,sbi.dwSize);
                  SetConsoleWindowInfo(hConOut,TRUE,&sbi.srWindow);
                  SetConsoleScreenBufferSize(hConOut,sbi.dwSize);

                  /* SKV$*/

                  Sleep(100);
                  InitConsole(0);

                  hFarWnd=0;
                  InitDetectWindowedMode();

                  if (hFarWnd)
                  {
                    if(Opt.SmallIcon)
                    {
                      char FarName[NM];
                      GetModuleFileName(NULL,FarName,sizeof(FarName));
                      ExtractIconEx(FarName,0,&hLargeIcon,&hSmallIcon,1);
                    }
                    if (hLargeIcon!=NULL)
                      SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);
                    if (hSmallIcon!=NULL)
                      SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
                  }

                  stop=1;
                  break;
                }
              }
            }
            if(stop)break;
          }
          Sleep(100);
        }
      }
      else
      {
        WaitForSingleObject(pi.hProcess,INFINITE);
      }
      /* SKV$*/
    }
    else if(ExecutorType && !(GUIType&2))// && AlwaysWaitFinish)
    {
      // поставим 800 мс, думаю хватит... хотя...
      // при нынешнем положении дел - это нафиг ненать (надо проверить!)
      WaitForSingleObject(pi.hProcess,800);//INFINITE);
    }

//    int CurScrX=ScrX,CurScrY=ScrY;
//    ReopenConsole();

//OT    GetVideoMode();
//OT    if (CurScrX!=ScrX || CurScrY!=ScrY)
//OT      CtrlObject->Cp()->SetScreenPositions();
    GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
    if (SeparateWindow!=2)
      CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ScrBuf.FillBuf();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    if (SeparateWindow!=2)
    {
      //Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),
      //        SeparateWindow==2 ? CmdPtr:ExecLine,MSG(MOk));
      //        ^^^^^^^^^^^^^^^^^ зачем? Это никогда не работает - см. выше
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),
                CmdPtr,MSG(MOk));
    }
    ExitCode=-1;
    //ScrBuf.FillBuf();
    //ScrBuf.SetLockCount(PrevLockCount);
  }
  SetFarConsoleMode();
  /* $ 05.10.2001 IS
     - Опечатка
     + Принудительная установка курсора, т.к. SetCursorType иногда не спасает
       вследствие своей оптимизации, которая в данном случае выходит боком.
  */
  SetCursorType(Visible,Size);
  SetRealCursorType(Visible,Size);
  /* IS $ */
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
      WinVer.dwBuildNumber<=0x4000457)
    WriteInput(VK_F16,SKEY_VK_KEYS);
  /* Если юзер выполнил внешнюю команду, например
     mode con lines=50 cols=100
     то ФАР не знал об изменении размера консоли.
     Для этого надо ФАРу напомнить лишний раз :-)
  */
  GenerateWINDOW_BUFFER_SIZE_EVENT(-1,-1);
  SetConsoleTitle(OldTitle);
  return(ExitCode);
}

int CommandLine::CmdExecute(char *CmdLine,int AlwaysWaitFinish,
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
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
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
    {
      RedrawDesktop Redraw(TRUE);

      ScrollScreen(1);
      MoveCursor(X1,Y1);
      if (CurDir[0] && CurDir[1]==':')
        chdir(CurDir);
      CmdStr.SetString("");
      if (ProcessOSCommands(CmdLine))
        Code=-1;
      else
        Code=Execute(CmdLine,AlwaysWaitFinish,SeparateWindow,DirectRun);

      //if(Code != -1)
      {
        int CurX,CurY;
        GetCursorPos(CurX,CurY);
        if (CurY>=Y1-1)
          ScrollScreen(Min(CurY-Y1+2,2/*Opt.ShowKeyBar ? 2:1*/));
      }
    }
    CtrlObject->Cp()->LeftPanel->UpdateIfChanged(1);
    CtrlObject->Cp()->RightPanel->UpdateIfChanged(1);
    CtrlObject->Cp()->Redraw();
  }
  /* VVM $ */
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
                   не exist, а esist или предложение неполно.

   DEFINED - подобно EXIST, но оперирует с переменными среды

   Исходная строка (CmdLine) не модифицируется!!!
*/
char* WINAPI PrepareOSIfExist(char *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

  char Cmd[1024], *PtrCmd=CmdLine, *CmdStart;
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
    while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
  }
  /* DJ $ */
  while(1)
  {
    if (!PtrCmd || !*PtrCmd || memicmp(PtrCmd,"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (memicmp(PtrCmd,"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !memicmp(PtrCmd,"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
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
          if(!(Cmd[1] == ':' || (Cmd[0] == '\\' && Cmd[1]=='\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(FullPath);
            else
              GetCurrentDirectory(sizeof(FullPath),FullPath);
            AddEndSlash(FullPath);
          }
          strcat(FullPath,ExpandedStr);
          ConvertNameToFull(FullPath,FullPath, sizeof(FullPath));
          DWORD FileAttr=GetFileAttributes(FullPath);
//_SVS(SysLog("%08X FullPath=%s",FileAttr,FullPath));
          if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
          {
            while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
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
      PtrCmd+=8; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
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
            while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
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


int CommandLine::ProcessOSCommands(char *CmdLine)
{
  Panel *SetPanel;
  int Length;
  SetPanel=CtrlObject->Cp()->ActivePanel;
  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);
  RemoveTrailingSpaces(CmdLine);
  if (isalpha(CmdLine[0]) && CmdLine[1]==':' && CmdLine[2]==0)
  {
    int NewDisk=toupper(CmdLine[0])-'A';
    setdisk(NewDisk);
    if (getdisk()!=NewDisk)
    {
      char NewDir[10];
      sprintf(NewDir,"%c:\\",NewDisk+'A');
      chdir(NewDir);
      setdisk(NewDisk);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }
  if (strnicmp(CmdLine,"SET ",4)==0)
  {
    char Cmd[1024];
    strcpy(Cmd,CmdLine+4);
    char *Value=strchr(Cmd,'=');
    if (Value==NULL)
      return(FALSE);
    *Value=0;
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
        OemToChar(ExpandedStr, ExpandedStr);
        SetEnvironmentVariable(Cmd,ExpandedStr);
      }
      /* IS $ */
    }
    return(TRUE);
  }

  if (!memicmp(CmdLine,"REM ",4) || !memicmp(CmdLine,"::",2))
  {
    return TRUE;
  }

  if (!memicmp(CmdLine,"CLS",3))
  {
    SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    return TRUE;
  }

  /* $ 14.01.2001 SVS
     + В ProcessOSCommands добавлена обработка
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  */
  if (memicmp(CmdLine,"IF ",3)==0)
  {
    char *PtrCmd=PrepareOSIfExist(CmdLine);
    // здесь PtrCmd - уже готовая команда, без IF
    if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return TRUE;
    }
    return FALSE;
  }
  /* SVS $ */

  if ((strnicmp(CmdLine,"CD",Length=2)==0 || strnicmp(CmdLine,"CHDIR",Length=5)==0) &&
      (isspace(CmdLine[Length]) || CmdLine[Length]=='\\' || strcmp(CmdLine+Length,"..")==0))
  {
    int ChDir=(Length==5);
    while (isspace(CmdLine[Length]))
      Length++;
    if (CmdLine[Length]=='\"')
      Length++;
    char NewDir[NM];
    strcpy(NewDir,&CmdLine[Length]);

    // скорректируем букву диска на "подступах"
    if(NewDir[1] == ':' && isalpha(NewDir[0]))
      NewDir[0]=toupper(NewDir[0]);

    /* $ 15.11.2001 OT
      Сначала проверяем есть ли такая "обычная" директория.
      если уж нет, то тогда начинаем думать, что это директория плагинная
    */
    DWORD DirAtt=GetFileAttributes(NewDir);
    if (DirAtt!=0xffffffff && DirAtt & FILE_ATTRIBUTE_DIRECTORY && PathMayBeAbsolute(NewDir))
    {
      SetPanel->SetCurDir(NewDir,TRUE);
      return TRUE;
    }
    /* OT $ */

    if (CtrlObject->Plugins.ProcessCommandLine(NewDir))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }

    char *ChPtr=strrchr(NewDir,'\"');
    if (ChPtr!=NULL)
      *ChPtr=0;
    if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
    {
      SetPanel->SetCurDir(NewDir,ChDir);
      return(TRUE);
    }
    char ExpandedDir[8192];
    if (ExpandEnvironmentStr(NewDir,ExpandedDir,sizeof(ExpandedDir))!=0)
      if (chdir(ExpandedDir)==-1)
        return(FALSE);
    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();
    return(TRUE);
  }
  return(FALSE);
}
