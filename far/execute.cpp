/*
execute.cpp

"Запускатель" программ.

*/

/* Revision: 1.108 15.12.2004 $ */

/*
Modify:
  15.12.2004 SVS
    - BugZ#1119 -  Неправильный разбор cmd строки для запуска
  14.12.2004 WARP
    ! Немного поломал executor. (see 01875.Executor.txt)
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  18.05.2004 SVS
    - BugZ#1077 - Падение при отсутствующей Win-ассоциации
  17.05.2004 SVS
    ! небольшие уточнение в запускаторе в порыве исправить падение на ассоциациях
  06.05.2004 SVS
    ! Кусок кода, разбивающий ком.строку на исполнятор и параметры вынесен в PartCmdLine()
  27.04.2004 SVS
    - BugZ#568 - Confusing message on wrong path in CHDIR in case of forward-slash delimiter
      Исправление не претендует на оригинальность - если первый раз для CD
      не получилось найти каталог, то влоб меняем все '/' на '\' и отдаем
      команду на откуп операционке.
      CommandLine::ProcessOSCommands() для CD возвращает -1, если нет каталога.
      Этим и будем пользоваться.
      Если что откатим.
  29.03.2004 SVS
    ! CHCP: сначала вызываем LocalUpperInit(), потом InitLCIDSort(), а не наоборот
  15.03.2004 SVS
    - не работает "C:\Program Files\Far\Far.exe" /p"C:\Program Files\Far\plugins"
      и вариации на эту тему (по Shift-Enter).
      Пока закомментил..., т.к. возможно повторение BugZ#752
  09.03.2004 SVS
    - SD> 2) окно far'a 102x37
      SD> запускаю батник с одной лишь строкой
      SD> if '%os%'=='Windows_NT' MODE CON: COLS=80 LINES=25
      VC-версия трапается
      BC-версия шнягу на экран выдает
      Проверим размеры окна и вызовем CtrlObject->CmdLine->CorrectRealScreenCoord()
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  09.02.2004 SVS
    - BugZ#993 - перекрытие сообщения рамок меню (уточнение)
    - BugZ#752 - Проблемы Shift-Enter для UNC ресурсов с пробелами и без (уточнение)
  15.01.2004 SVS
    - BugZ#993 - перекрытие сообщения рамок меню
  08.01.2004 SVS
    + учтем опцию Opt.ExecuteShowErrorMessage и выведем текст на экран, а не в месагбоксе
  05.01.2004 SVS
    ! Уточнение запускатора (про "far.exe/?")
  05.01.2004 SVS
    - BugZ#1007 - Не передаются параметры прогам когда Executor\Type=1
  11.11.2003 SVS
    - Правки в Execute() по поводу Shift-Enter
  16.10.2003 SVS
    ! Если в ассоциациях нету "command" или этот параметр реестра
      пуст - не вызываем ShellExecuteEx().
      Так же добавлен флаг SEE_MASK_FLAG_NO_UI, чтобы ФАР выбывал сообщения
      (если что - убрать!).
  09.10.2003 SVS
    ! SetFileApisToANSI() и SetFileApisToOEM() заменены на SetFileApisTo() с параметром
      APIS2ANSI или APIS2OEM - задел на будущее
  06.10.2003 SVS
    - BugZ#955 - Alt-F9 + Mode CON: Lines=12
      Достаточно "получить" данные о размере консоли... иначе вылетаем
      в ScrBuf.FillBuf(), там ScrY неверное значение имеет
  26.09.2003 VVM
    ! При поиске файла сначала ищем по переменной PATH и только потом в остальных местах
  03.09.2003 SVS
    - bugz#933 - задолбал этот strcpy :-(
    + В CommandLine::ProcessOSCommands() добавлен закомментированный кусок
      ...вариант для "SET /P variable=[promptString]" - что бы не потерялось...
  02.09.2003 SVS
    ! уточнение кода возврата функции CheckFolder()
  31.07.2003 VVM
    ! Некорректное определение типа исполняемого файла.
  05.06.2003 SVS
    ! SetFarConsoleMode имеет параметр - нужно ли активировать буфер
  06.05.2003 SVS
    ! при смене кодовой страницы так же переинициализируем некоторое массивы
    ! попытка борьбы с синим фоном в 4NT при старте консоль
  17.03.2003 SVS
    - BugZ#831 - Неверный заголовок окна при запуске из командной строки
  06.03.2003 SVS
    - BugZ#678 - Незапуск .msi по Shift-Enter
    ! Закоментим _SVS
  25.02.2003 SVS
    ! Вернем "старое" поведение для выставление титла консоли, ибо строка
      "gzip foo.bar" намного понятнее выглядит, нежели "c:\usr\bin\gzip.EXE"
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
  26.01.2003 IS
    ! FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
  17.01.2003 VVM
    ! Косметика
  17.12.2002 VVM
    - BugZ#678 - Незапуск .msi по Shift-Enter (вторая часть!)
  11.12.2002 VVM
    - Opps. Уберем грязь от экспериментов...
  11.12.2002 VVM
    - Исправлен баг с запуском приложений из архивов с русским именем.
      Для ГУИ будем пользовать ShellExcuteEx()
  04.10.2002 VVM
    ! Небольшой баг при поиске файла по списку расширений. Перед поиском расширения
      отделим имя файла от пути к нему.
  03.10.2002 VVM
    + Default action может содержать несколько команнд через запятую.
    + При поиске в App paths учтем кавычки и переменные окружения.
  20.09.2002 SKV
    | Отключил cd net:server
  03.09.2002 SVS
    - BugZ#606 - не работают переменные окружения в ассоциациях
      не стояла проверка на символы ":\" для "распахнутой" строки...
  21.08.2002 SVS
    - Исправления 1493 патча. Сначала нужно в обязательном порядке проверить
      кей "open", а если его нету, то... что первое попадется ;-)
  17.08.2002 VVM
    + GetShellAction() - если нет "Default action",  то возьмем первую,
      у которой будет ключ "Command"
  12.08.2002 SVS
    + Opt.ExecuteUseAppPath
  08.08.2002 VVM
    ! Вернем назад полный путь до текущего каталога.
  17.07.2002 VVM
    - Если пускаем из текущего каталога, то не делаем "развертку" пути.
    ! Команды из ExcludeCmds имеют ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI
    + Если ImageSubsystem == IMAGE_SUBSYSTEM_UNKNOWN, то детачим запускаемый процесс.
    - Плюс затычка для "\ Enter"
  15.07.2002 VVM
    + Для виндовс-ГУИ сделаем всегда запуск через ShellExecuteEx(). По идее SVS
  06.07.2002 VVM
    + Если не установлена переменная %COMSPEC% - предупредить и выйти.
  05.07.2002 SVS
    - ФАР ждет завершения GUI-прилад с NE-заголовком.
      Уточним этот факт (с подачи Andrzej Novosiolov <andrzej@se.kiev.ua>)
  19.02.2002 SVS
    - BugZ#558 - ShiftEnter из меню выбора диска
      Проверим так же на "корень диска" и выставим запуск через ShellExecuteEx()
  18.06.2002 SVS
    ! В обработчике команды "CD" (see CommandLine::ProcessOSCommands)
      посмотрим на CHKFLD_NOTACCESS (эту константу вернет нам та самая
      CheckFolder). Если все оби - едем дальше, иначе вернем FALSE и
      при этом ФАР матюкнется в консоль словами OS о том, что
      "Хреновый ты путь указал, малый. Отвали!".
      Тем самым реализуем ситуацию "пред-обработки", т.е. матюкаемся не
      тогда, когда ввалились в FileList::ReadFileNames, а... чуток пораньше! ;-)
      (эта побасенка про BugZ#513 - при cd на нечитаемую сетевую
      шару остаётся старое содержимое панели).
  14.06.2002 VVM
    + IsCommandPEExeGUI переделана. Теперь она возвращает не IsGUI битовый,
      а значения IMAGE_SUBSYSTEM_*
    + Переделана запускалка. Теперь в строке можно набрать "excel" и запустится ексель.
      Т.е. по реестру дополнительный поиск с расширениями из StdExecuteEx
  30.05.2002 SVS
    ! Для CHCP добавлен вызов InitRecodeOutTable().
  29.05.2002 SVS
    ! "Не справился с управлением" - откат IsLocalPath() до лучших времен.
  28.05.2002 SVS
    ! применим функцию  IsLocalPath()
  18.05.2002 SVS
    ! Возможность компиляции под BC 5.5
  10.05.2002 SVS
    + обработка CHCP
  17.04.2002 VVM
    ! Уточнение исполнятора.
  16.04.2002 DJ
    ! пропускаем встроенную обработку cd, если нажат Shift-Enter
  02.04.2002 SVS
    - BugZ#421 - Курсор в запускаемых программах
  30.03.2002 VVM
    ! Кавычим строку выполнения только если пускаем в отдельном окне.
  28.03.2002 SVS
    - CLS не выставляла атрибуты консоли
  26.03.2002 SVS
    - BugZ#393 - . Shift-Enter
  25.03.2002 VVM
    ! Очередная правка запускатора :)
  22.03.2002 IS
    - Устанавливали заголовок консоли крякозябрами, потому что не учили, что
      для 9x параметр у SetConsoleTitle должен быть в ANSI-кодировке
  21.03.2002 SVS
    - BugZ#365 - Глюк с открытием папок в проводнике по шифт+ентер
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  20.03.2002 IS
    + "if [not] exist" дружит теперь с масками файлов
    ! PrepareOSIfExist теперь принимает и возвращает const
  18.03.2002 SVS
    ! Бадяга про курсор - там где нужно гасить, выставлялася 1
  28.02.2002 SVS
    - BugZ#318 - dot Shift-Enter
  26.02.2002 SVS
    ! "." и ".." по Shift-Enter рисуем AS IS, без модификации.
  19.02.2002 SVS
    ! В исполняторе юзаем только *ConsoleTitle, т.е. апишные...
  18.02.2002 SVS
    - set хрен=редька
      Сабжевая команда под cmd.exe работает правильно, а под фаром - отнюдь.
      Левая часть оказывается в неверной кодировке.
  15.02.2002 DJ
    - еще про заголовок: запускаем файл по ассоциации и видим, что через
      несколько секунд после запуска заголовок с нормального меняется
      на название запущенной программы
  14.02.2002 SVS
    - BugZ#300 - Shift-Enter на папке меняет путь заголовок окна
  14.02.2002 VVM
    ! UpdateIfChanged принимает не булевый Force, а варианты из UIC_*
  07.02.2002 SKV
    - Не надо при отрыве консоли менять её моду и т.д.
  04.02.2002 SVS
    - BugZ#289 Неправильно определяется GUI/Console для *.exe с русскими
      именами
  30.01.2002 SVS
    ! Проверим, а не папку ли мы хотим открыть по Shift-Enter?
      Если так, то вместо CreateProcess запустим ShellExe...
  28.01.2002 tran
    ! тройные кавычки нужны не всегда.
  18.01.2002 VVM
    ! Избавимся от setdisk() -> FarChDir()
  16.01.2002 SVS
    - Тот же самый BugZ#238. Немного увеличим буфера (до 4096 размер под FullPath)
  15.01.2002 SVS
    - Не исполняется "C:\Program Files\Far\Far.exe" "C:\Program Files\Far\Plugins"
  14.01.2002 IS
    ! chdir -> FarChDir
  24.12.2001 SVS
    - BugZ#193: не работает <, >, | (в 9х)
    + проверка на вшивость на подступах - если введено нечто вроде "|" или ">"
      или "<" (т.е. один символ) - то... в морг.
  21.12.2001 SVS
    - Bug: после запуска компилятора Java от MS, jvc.exe симолы псевдографики
           в пользовательском интерфейсе Far'а заменяются на русские буквы.
      Кстати, этим же страдают проги типа sh.exe (bash) из портированного
      унихового утилия.
  20.12.2001 SVS
    ! Для Shift-Enter учтем перенаправления и каналы.
  14.12.2001 IS
    ! stricmp -> LocalStricmp
  08.12.2001 SVS
    ! Уточнения в новом исполняторе - теперь вид шаблона исполнения задается
      по другому.
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
    DWORD FileSizeLow, FileSizeHigh, ReadSize;
    IMAGE_DOS_HEADER dos_head;

    FileSizeLow=GetFileSize(hFile,&FileSizeHigh);

    if(ReadFile(hFile,&dos_head,sizeof(IMAGE_DOS_HEADER),&ReadSize,NULL) &&
       dos_head.e_magic == IMAGE_DOS_SIGNATURE)
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
               ImageSubsystem = header.opt_head.Subsystem;
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
      ; // это не исполняемый файл - у него нету заголовка MZ
        // например, NLM-модуль или ошибка чтения :-)
    }
    CloseHandle(hFile);
  }

  return Ret;
}
/* VVM $ */

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

  ValueSize=sizeof(Value);
  *Value=0;

  if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)!=ERROR_SUCCESS)
    return(NULL);

  strcat(Value,"\\shell");
//_SVS(SysLog("[%d] Value='%s'",__LINE__,Value));

  HKEY hKey;
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
        strncat(NewValue, ActionPtr, sizeof(NewValue) - 1);
        strncat(NewValue, command_action, sizeof(NewValue) - 1);
        if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strncat(Value, ActionPtr, sizeof(Value) - 1);
          RetPtr = xstrncpy(Action,ActionPtr,sizeof(Action)-1);
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* while */
    } /* if */
    else
      strncat(Value,Action, sizeof(Value) - 1);
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
    strncat(NewValue, Action, sizeof(NewValue) - 1);
    strncat(NewValue, command_action, sizeof(NewValue) - 1);
    if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
    {
      RegCloseKey(hOpenKey);
      strncat(Value, Action, sizeof(Value) - 1);
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
        strncat(NewValue, Action, sizeof(NewValue) - 1);
        strncat(NewValue, command_action, sizeof(NewValue) - 1);
        if (RegOpenKey(HKEY_CLASSES_ROOT,NewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strncat(Value, Action, sizeof(Value) - 1);
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
    strncat(Value,command_action, sizeof(Value) - 1);

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

  // Здесь порядок важен! Сначала батники,  а потом остальная фигня.
  static char StdExecuteExt[NM]=".BAT;.CMD;.EXE;.COM;";
  static const char RegPath[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
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
  static char ExcludeCmds[4096]={0};
  static int PrepareExcludeCmds=FALSE;
  if(GetRegKey(strSystemExecutor,"Type",0))
  {
    if (!PrepareExcludeCmds)
    {
      GetRegKey(strSystemExecutor,"ExcludeCmds",(char*)ExcludeCmds,"",0);
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

  FAR_OemToChar(Command,FullName);

  /* $ 07.09.2001 VVM Обработать переменные окружения */
  ExpandEnvironmentStrings(FullName,FileName,sizeof(FullName));

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
      if(GetFileAttributes(FullName) != -1)
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
            Unquote(FileName);
            ExpandEnvironmentStrings(FileName,FullName,sizeof(FullName));
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
                Unquote(FileName);
                ExpandEnvironmentStrings(FileName,FullName,sizeof(FullName));
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
      DestSize=strlen(FullName);
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
  SetFileApisTo(APIS2OEM);
  return(GUIType);
}
#endif
/* VVM $ */

/* Функция-пускатель внешних процессов
   Возвращает -1 в случае ошибки или...
*/
int Execute(const char *CmdStr,          // Ком.строка для исполнения
            int AlwaysWaitFinish,  // Ждать завершение процесса?
            int SeparateWindow,    // Выполнить в отдельном окне? =2 для вызова ShellExecuteEx()
            int DirectRun,         // Выполнять директом? (без CMD)
            int SetUpDirs)         // Нужно устанавливать каталоги?
{
  int NT = WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT;
  char NewCmdStr[4096];
  char NewCmdPar[4096];
  char TempStr[4096];
  char CommandName[NM];
  NewCmdPar[0] = 0;

  // ПРЕДпроверка на вшивость
  ////_tran(SysLog("Execute: CmdStr [%s]",CmdStr);)
  char *CmdPtr = NewCmdStr;
  char *ParPtr = NULL;
  int QuoteFound = FALSE;
  int PipeFound = FALSE;

  PipeFound=PartCmdLine(CmdStr,NewCmdStr,sizeof(NewCmdStr),NewCmdPar,sizeof(NewCmdPar));

  //_tran(SysLog("Execute: newCmdStr [%s]",NewCmdStr);)

  // Проверим, а не папку ли мы хотим открыть по Shift-Enter?
  //_SVS(SysLog("SeparateWindow=%d",SeparateWindow));
  DWORD Attr=GetFileAttributes(NewCmdStr);
  if(SeparateWindow) //???
  {
    if(!*NewCmdPar && Attr != -1 && (Attr&FILE_ATTRIBUTE_DIRECTORY) || (NewCmdStr[1]==':' && (NewCmdStr[2] == '\\' && !NewCmdStr[3] || !NewCmdStr[2])))
    {
      ConvertNameToFull(NewCmdStr,NewCmdStr,sizeof(NewCmdStr));
      SeparateWindow=2;
    }
  }

  // глянем на результат
  if(!*NewCmdStr || (strlen(NewCmdStr)==1 && strpbrk(NewCmdStr,"<>|:")!=NULL))
  {
    // А может просто запустить CMD или проводник?
    // если "да", то этот куско нужно ниже перенести.
    return -1;
  }
  // запомним CP-консоли перед стартом проги
  UINT ConsoleCP=GetConsoleCP();
  UINT ConsoleOutputCP=GetConsoleOutputCP();

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  STARTUPINFO si;
  PROCESS_INFORMATION pi={0,};
  int Visible,Size;
  int PrevLockCount;
  char ExecLine[4096];
  char OldTitle[512];
  DWORD ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
  int ExitCode=1;
  DWORD _LastErrCode=ERROR_SUCCESS;

  int ExecutorType = GetRegKey(strSystemExecutor,"Type",0);
  // частный случай - т.с. затычка, но нужно конкретное решение!
  if(*NewCmdStr && !((*NewCmdStr == '\\'|| *NewCmdStr == '/') && !NewCmdStr[1]))
  {
    ExitCode=PrepareExecuteModule(NewCmdStr,NewCmdStr,sizeof(NewCmdStr)-1,ImageSubsystem);
    Attr=GetFileAttributes(NewCmdStr);
  }
  // Для Виндовс-ГУИ всегда сделаем запуск через ShellExecuteEx()
//  if (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI && !AlwaysWaitFinish)
  if (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
    SeparateWindow=2;
  if (!ExecutorType || SeparateWindow==2)
    ExitCode = 1;

  ExpandEnvironmentStrings (NewCmdStr, TempStr, sizeof (TempStr)-1);
  strcpy (NewCmdStr, TempStr);

  QuoteSpace(NewCmdStr);

  QuoteFound = NewCmdStr[0] == '"';
  CmdPtr = NewCmdStr;

  CommandName[0]=0;
  GetEnvironmentVariable("COMSPEC",CommandName,sizeof(CommandName));
  if ((CommandName[0] == 0) && (SetUpDirs || (!ExecutorType && SeparateWindow != 2)))
  {
    Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
    return -1;
  }

  /* $ 13.04.2001 VVM
    + Флаг CREATE_DEFAULT_ERROR_MODE. Что-бы показывал все ошибки */
  DWORD CreateFlags = CREATE_DEFAULT_ERROR_MODE;
  /* VVM $ */

  GetCursorType(Visible,Size);
  //SetCursorType(0,-1);
  SetInitialCursorType();

  PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);
  ScrBuf.Flush();

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  if (SetUpDirs)
  {
    Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && PassivePanel->GetType()==FILE_PANEL)
      for (int I=0;CmdStr[I]!=0;I++)
        if (isalpha(CmdStr[I]) && CmdStr[I+1]==':' && CmdStr[I+2]!='\\')
        {
          char SavePath[NM],PanelPath[NM],SetPathCmd[NM];
          FarGetCurDir(sizeof(SavePath),SavePath);
          PassivePanel->GetCurDir(PanelPath);
          sprintf(SetPathCmd,"%s /C chdir %s",CommandName,QuoteSpace(PanelPath));
          CreateProcess(NULL,SetPathCmd,NULL,NULL,FALSE,CreateFlags,NULL,NULL,&si,&pi);
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
          FarChDir(SavePath);
        }
  }

//  strcpy(NewCmdStr,CmdStr);
//  if (ParPtr)
//    *ParPtr = 0;
  //while (IsSpace(*CmdPtr))
  //  CmdPtr++;

  *ExecLine=0;

  if (ExitCode)
  {
    if (DirectRun && !SeparateWindow)
      xstrncpy(ExecLine,CmdPtr,sizeof(ExecLine)-1);
    else
    {
      if(ExecutorType)
      {
        if(ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI && !AlwaysWaitFinish)
          xstrncpy(ExecLine,NewCmdStr,sizeof(ExecLine)-1);
        else
        {
          char TemplExecute[512];
          char TemplExecuteStart[512];
          char TemplExecuteWait[512];
          // <TODO: здесь надо по другому переделать>
          GetRegKey(strSystemExecutor,"Normal",TemplExecute,"%COMSPEC% /c",sizeof(TemplExecute));
          GetRegKey(strSystemExecutor,"Start",TemplExecuteStart,"%COMSPEC% /c start",sizeof(TemplExecuteStart));
          GetRegKey(strSystemExecutor,"Wait",TemplExecuteWait,"%COMSPEC% /c start /wait",sizeof(TemplExecuteWait));

          char *Fmt=TemplExecute;
          if (SeparateWindow || ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI && (NT || AlwaysWaitFinish))
          {
            Fmt=TemplExecuteStart;
            if (AlwaysWaitFinish)
              Fmt=TemplExecuteWait;
          }
          char *CmdEnd=NewCmdStr+strlen(NewCmdStr)-1;
          if (NT && *NewCmdStr == '\"' && *CmdEnd == '\"' &&
             strchr(NewCmdStr+1, '\"') != CmdEnd && SeparateWindow!=2)
          {
            char *Ptr=NewCmdStr;
            int NumSq=0;
            while(*Ptr)
            {
              if(*Ptr == '\"')
                NumSq++;
              ++Ptr;
            }
            InsertQuote(NewCmdStr);
            if(NumSq > 2)
              InsertQuote(NewCmdStr);
          }

//          if(SeparateWindow)
//            ReplaceStrings(NewCmdPar,"\"","\"\"",-1);

          if ( *NewCmdStr && strchr (NewCmdStr, ' ') )
          {
            int l = strlen (NewCmdStr);

            memmove (NewCmdStr+1, NewCmdStr, ++l);
            NewCmdStr[0] = '\"';
            NewCmdStr[l++] = '\"';
            NewCmdStr[l] = 0;
          }

          xstrncpy(ExecLine,Fmt,sizeof(ExecLine)-1);
          strncat(ExecLine,(Fmt != TemplExecute && NT && *CmdPtr=='\"'?" \"\" ":" "),sizeof(ExecLine)-1);
          strncat(ExecLine, NewCmdStr,sizeof(ExecLine)-1);
          strncat(ExecLine, NewCmdPar,sizeof(ExecLine)-1);
          ExpandEnvironmentStr(ExecLine,ExecLine,sizeof(ExecLine));

          // </TODO>
          //_tran(SysLog("Execute: ExecLine [%s]",ExecLine);)
        }
      }
      else if (SeparateWindow != 2)
      {
//        int Pipe=strpbrk(CmdPtr,"<>|")!=NULL;
        xstrncpy(ExecLine,CommandName,sizeof(ExecLine)-1);
        strncat(ExecLine," /C",sizeof(ExecLine)-1);
        //_tran(SysLog("1. execline='%s'",ExecLine);)

        int QuoteAll = FALSE;
// WINDOWS_GUI всегда пускается через ShellExecute
//        if ((PipeFound && (SeparateWindow || ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI && (NT || AlwaysWaitFinish))) ||
//            (QuoteFound && NT && ImageSubsystem != IMAGE_SUBSYSTEM_WINDOWS_GUI))
        if ((PipeFound && SeparateWindow) || (QuoteFound && NT))
          QuoteAll = TRUE;

        if (SeparateWindow)
        {
          strncat(ExecLine," start",sizeof(ExecLine)-1);
          if (AlwaysWaitFinish)
            strncat(ExecLine," /wait",sizeof(ExecLine)-1);
          if(PipeFound || QuoteAll)
            sprintf(ExecLine+strlen(ExecLine)," %s /C",CommandName);
          else if (NT && *CmdPtr=='\"')
            strncat(ExecLine," \"\"",sizeof(ExecLine)-1);
        }

        strncat(ExecLine," ",sizeof(ExecLine)-1);
        //_tran(SysLog("2. execline=[%s]",ExecLine);)
        //_tran(SysLog("3. cmdptr=[%s]",CmdPtr);)

        if (QuoteAll) strncat(ExecLine, "\"",sizeof(ExecLine)-1);
        strncat(ExecLine, CmdPtr,sizeof(ExecLine)-1);
        strncat(ExecLine, NewCmdPar,sizeof(ExecLine)-1);
        if (QuoteAll) strncat(ExecLine, "\"",sizeof(ExecLine)-1);
        //_tran(SysLog("Execute: ExecLine2 [%s]",ExecLine);)
      }
    }

    if (SeparateWindow)
    {
      ReplaceStrings(ExecLine,"^","^^",-1);
      ReplaceStrings(ExecLine,"&","^&",-1);
    }

//_SVS(SysLog("ExecLine='%s'",ExecLine));
    // если запуск через ShellExecuteEx(), то нефига ставить заголовок
    /* $ 22.03.2002 IS
         Для 9x параметр у SetConsoleTitle должен быть ANSI
    */
    if(SeparateWindow != 2)
    {
      if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
      {
        char FarTitle[2*NM];
        int size=Min((DWORD)strlen(CmdStr),(DWORD)sizeof(FarTitle)-1);
        FAR_OemToCharBuff(CmdStr,FarTitle,size);
        FarTitle[size]=0;
        SetConsoleTitle(FarTitle);
      }
      else
        SetConsoleTitle(CmdStr);//SetFarTitle(CmdPtr);
    }
    /* IS $ */
//_SVS(SysLog("CmdPtr  ='%s'",CmdPtr));
    FlushInputBuffer();

    /*$ 15.03.2001 SKV
      Надо запомнить параметры консоли ДО запуск и т.д.
    */
    GetConsoleScreenBufferInfo(hConOut,&sbi);
    /* SKV$*/

    ChangeConsoleMode(InitialConsoleMode);

    if (SeparateWindow && (ImageSubsystem == IMAGE_SUBSYSTEM_UNKNOWN))
        CreateFlags|=DETACHED_PROCESS;

    if (SeparateWindow==2)
    {
//_SVS(CleverSysLog clvrSLog("Execute() -> SeparateWindow==2"));
      char AnsiCmdStr[4096];
      SHELLEXECUTEINFO SEInfo;
      memset(&SEInfo,0,sizeof(SEInfo));

      xstrncpy(AnsiCmdStr,CmdPtr,sizeof(AnsiCmdStr)-1);
      FAR_OemToChar(AnsiCmdStr, AnsiCmdStr);
      FAR_OemToChar(NewCmdPar, NewCmdPar);

      if (PointToName(AnsiCmdStr)==AnsiCmdStr && strcmp(AnsiCmdStr,".") && !TestParentFolderName(AnsiCmdStr))
      {
        //char FullName[4096];
        //sprintf(FullName,".\\%s",AnsiCmdStr);
        //strcpy(AnsiCmdStr,FullName);
        memmove(AnsiCmdStr+2,AnsiCmdStr,Min((int)strlen(AnsiCmdStr),(int)(sizeof(AnsiCmdStr)-3))+1);
        memcpy(AnsiCmdStr,".\\",2);
      }
      Unquote(AnsiCmdStr); // т.к. нафиг это ненужно?
      // ???
      if(Attr != -1 && (Attr&FILE_ATTRIBUTE_DIRECTORY) && !(!strcmp(AnsiCmdStr,".") || TestParentFolderName(AnsiCmdStr))
         && !(AnsiCmdStr[0] == '\\' && AnsiCmdStr[1] == '\\') // Bug 752 - Проблемы Shift-Enter для UNC ресурсов с пробелами и без
      )
        strcat(AnsiCmdStr,".");
      // ???
//_SVS(SysLog("[%d] AnsiCmdStr='%s'  Attr=0x%08X",__LINE__,AnsiCmdStr,Attr));

      SEInfo.lpVerb=(Attr&FILE_ATTRIBUTE_DIRECTORY)?NULL:GetShellAction((char *)AnsiCmdStr,ImageSubsystem,_LastErrCode);
      if(!_LastErrCode)
      {
        SEInfo.cbSize=sizeof(SEInfo);
        SEInfo.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_FLAG_NO_UI;
        SEInfo.lpFile=AnsiCmdStr;
        SEInfo.nShow=SW_SHOWNORMAL;
        if (NewCmdPar[0])
          SEInfo.lpParameters = NewCmdPar;
#if 0
_SVS(SysLog("SHELLEXECUTEINFO{"));
_SVS(SysLog(" cbSize=       %d",SEInfo.cbSize));
_SVS(SysLog(" fMask=        %08X",SEInfo.fMask));
_SVS(SysLog(" hwnd;         %08X",SEInfo.hwnd));
_SVS(SysLog(" lpVerb=       %s",SEInfo.lpVerb));
_SVS(SysLog(" lpFile=       %s",SEInfo.lpFile));
_SVS(SysLog(" lpParameters= %s",SEInfo.lpParameters));
_SVS(SysLog(" lpDirectory=  %s",SEInfo.lpDirectory));
_SVS(SysLog(" nShow=        %d",SEInfo.nShow));
_SVS(SysLog(" hInstApp=     0x%08X",SEInfo.hInstApp));
_SVS(SysLog(" lpIDList=     0x%08X",SEInfo.lpIDList));
_SVS(SysLog(" lpClass=      %s",SEInfo.lpClass));
_SVS(SysLog(" hkeyClass=    0x%08X",SEInfo.hkeyClass));
_SVS(SysLog(" dwHotKey=     0x%08X",SEInfo.dwHotKey));
_SVS(SysLog(" hIcon=        0x%08X",SEInfo.hIcon));
_SVS(SysLog(" hProcess=     0x%08X (%d)",SEInfo.hProcess,SEInfo.hProcess));
_SVS(SysLog("};"));
#endif

        SetFileApisTo(APIS2ANSI);
        ExitCode=ShellExecuteEx(&SEInfo);
        if(!ExitCode)
          _LastErrCode=GetLastError();
        SetFileApisTo(APIS2OEM);
        pi.hProcess=SEInfo.hProcess;
      }
      else
        ExitCode=0;
    }
    else
    {
      SetRealColor(F_LIGHTGRAY|B_BLACK); // попытка борьбы с синим фоном в 4NT при старте консоль
      ExitCode=CreateProcess(NULL,ExecLine,NULL,NULL,0,CreateFlags,NULL,NULL,&si,&pi);
      if(!ExitCode)
        _LastErrCode=GetLastError();
    }

    StartExecTime=clock();
  }

  if (ExitCode)
  {
    if (!SeparateWindow && ImageSubsystem != IMAGE_SUBSYSTEM_WINDOWS_GUI || AlwaysWaitFinish)
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
                  /*$ 07.02.2002 SKV
                    Не будем вызыват CloseConsole, потому, что она поменяет
                    ConsoleMode на тот, что был до запуска Far'а,
                    чего работающее приложение могло и не ожидать.
                  */
                  //CloseConsole();
                  CloseHandle(hConInp);
                  CloseHandle(hConOut);
                  delete KeyQueue;
                  KeyQueue=NULL;
                  /* SKV $*/

                  FreeConsole();
                  AllocConsole();

                  /*$ 17.05.2001 SKV
                    если окно имело HOTKEY, то старое должно его забыть.
                  */
                  if(hFarWnd)
                  {
                    SendMessage(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);
                  }
                  /* SKV $*/


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
    else if(ExecutorType && !SeparateWindow)//!(GUIType&2))// && AlwaysWaitFinish)
    {
      // поставим 800 мс, думаю хватит... хотя...
      // при нынешнем положении дел - это нафиг ненать (надо проверить!)
      WaitForSingleObject(pi.hProcess,INFINITE);//INFINITE);
    }

    GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
    if (SeparateWindow!=2)
      CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      GetVideoMode(csbi);
    }
    ScrBuf.FillBuf();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    //if (SeparateWindow!=2)
    {
      char OutStr[2048];

      SetLastError(_LastErrCode);
      //Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),
      //        SeparateWindow==2 ? CmdPtr:ExecLine,MSG(MOk));
      //        ^^^^^^^^^^^^^^^^^ зачем? Это никогда не работает - см. выше
      if(Opt.ExecuteShowErrorMessage)
      {
        SetMessageHelp("ErrCannotExecute");
        // BugZ#993 - перекрытие сообщения рамок меню
        xstrncpy(OutStr,CmdPtr,sizeof(OutStr)-1);
        Unquote(OutStr);
        TruncPathStr(OutStr,ScrX-15);
        Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),OutStr,MSG(MOk));
      }
      else
      {
        sprintf(OutStr,MSG(MExecuteErrorMessage),CmdPtr);
        char *PtrStr=FarFormatText(OutStr,ScrX,OutStr,sizeof(OutStr),"\n",0);
        printf(PtrStr);
        ScrBuf.FillBuf();
      }

    }
    ExitCode=-1;
    //ScrBuf.FillBuf();
    //ScrBuf.SetLockCount(PrevLockCount);
  }
  SetFarConsoleMode(TRUE);
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

  if(Opt.RestoreCPAfterExecute)
  {
    // восстановим CP-консоли после исполнения проги
    SetConsoleCP(ConsoleCP);
    SetConsoleOutputCP(ConsoleOutputCP);
  }
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
    CONSOLE_SCREEN_BUFFER_INFO sbi0,sbi1;
    GetConsoleScreenBufferInfo(hConOut,&sbi0);
    {
      RedrawDesktop Redraw(TRUE);

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
    }
    CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
    CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
    CtrlObject->Cp()->Redraw();
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
          DWORD FileAttr=-1;
          if(strpbrk(ExpandedStr,"*?")) // это маска?
          {
            WIN32_FIND_DATA wfd;
            HANDLE hFile=FindFirstFile(FullPath, &wfd);
            if(hFile!=INVALID_HANDLE_VALUE)
            {
              FileAttr=wfd.dwFileAttributes;
              FindClose(hFile);
            }
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
  if (strnicmp(CmdLine,"SET ",4)==0)
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

  if (!memicmp(CmdLine,"REM ",4) || !memicmp(CmdLine,"::",2))
  {
    return TRUE;
  }

  if (!memicmp(CmdLine,"CLS",3))
  {
    if(CmdLine[3])
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
  if (!memicmp(CmdLine,"CHCP",4))
  {
    if(CmdLine[4] == 0 || !(CmdLine[4] == ' ' || CmdLine[4] == '\t'))
      return(FALSE);

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
  if (memicmp(CmdLine,"IF ",3)==0)
  {
    const char *PtrCmd=PrepareOSIfExist(CmdLine);
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

  /* $ 16.04.2002 DJ
     пропускаем обработку, если нажат Shift-Enter
  */
  if (!SeparateWindow &&  /* DJ $ */
      (strnicmp(CmdLine,"CD",Length=2)==0 || strnicmp(CmdLine,"CHDIR",Length=5)==0) &&
      (IsSpace(CmdLine[Length]) || CmdLine[Length]=='\\' || TestParentFolderName(CmdLine+Length)))
  {
    int ChDir=(Length==5);

    while (IsSpace(CmdLine[Length]))
      Length++;

    if (CmdLine[Length]=='\"')
      Length++;

    char ExpandedDir[8192];
    xstrncpy(ExpandedDir,&CmdLine[Length],sizeof(ExpandedDir)-1);

    // скорректируем букву диска на "подступах"
    if(ExpandedDir[1] == ':' && isalpha(ExpandedDir[0]))
      ExpandedDir[0]=toupper(ExpandedDir[0]);

    /* $ 15.11.2001 OT
      Сначала проверяем есть ли такая "обычная" директория.
      если уж нет, то тогда начинаем думать, что это директория плагинная
    */
    DWORD DirAtt=GetFileAttributes(ExpandedDir);
    if (DirAtt!=0xffffffff && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsolute(ExpandedDir))
    {
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
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }
    */
    /* SKV $ */

    char *ChPtr=strrchr(ExpandedDir,'\"');
    if (ChPtr!=NULL)
      *ChPtr=0;

    if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
    {
      SetPanel->SetCurDir(ExpandedDir,ChDir);
      return(TRUE);
    }

    if (ExpandEnvironmentStr(ExpandedDir,ExpandedDir,sizeof(ExpandedDir))!=0)
    {
      if(CheckFolder(ExpandedDir) <= CHKFLD_NOTACCESS)
        return -1;

      if (!FarChDir(ExpandedDir))
        return(FALSE);
    }

    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();

    return(TRUE);
  }
  return(FALSE);
}
