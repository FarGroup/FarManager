/*
main.cpp

Функция main.

*/

/* Revision: 1.70 06.06.2003 $ */

/*
Modify:
  06.06.2003 SVS
    ! Отделение мух от котлет - ключ /p приоритетный
  19.05.2003 SVS
    ! DetectTTFFont уехал из main.cpp в interf.cpp
  06.05.2003 SVS
    ! /aw удален, вместо него /8, который в масдае не действует, а в NT
      заставляет работать ФАР в 8-битной консоли (по умолчанию в NT
      ФАР стартует as Unicode - как для Out, так и для Inp
    ! Автодетект TTF отключен нафиг
  16.04.2003 SVS
    ! /co преобладает над /p
    ! Уточнение для _beginthread() - вызовем функции напрямую, иначе
      зарегистрированные пользователи в пролете :-(
  31.03.2003 SVS
    ! Проверим код возврата _beginthread() и выставим "Евалюшин копия"
  17.03.2003 SVS
    ! применим новые флаги FFPOL_*
  04.02.2003 SVS
    + В общем, теперь в дебажной версии есть ключ "/cr", отключающий трид
      проверки регистрации. Под TD32 иногда жутчайшие тормоза наблюдаются.
  18.01.2003 IS
    - Не правильно обрабатывалась команда /p[<path>], если в пути были
      буквы национального алфавита.
  10.01.2003 SVS
    - BugZ#765 - Ключи командной строки парсятся неоднозначно.
    ! Научим исключатор понимать (и реагировать предсмертным воплем) исключения
      в самом ФАРе. Для этого часть кода вынесено из main в MainProcess.
      Вызов MainProcess обволочен стандартным способом для исключений
      (учет опции Opt.ExceptRules)
  23.12.2002 SVS
    ! OnlyEditorViewerUsed стал частью структуры Options
  07.10.2002 SVS
    - BugZ#674 - far.exe c:\dir [c:\dir2] - не работает
  21.08.2002 SVS
    ! Уточнение про WaitKey
  02.07.2002 SVS
    - /u USER отломали
  27.06.2002 SVS
    - Падение в масдае при старте ФАРа - нефига автодект TTF-шрифта
      в этой гребанной оси вызывать
  21.06.2002 SVS
    + автодетект TTF-шрифта
    ! /w  - отключена (может потом пригодится)
    + /aw - Отключить автоопределение TrueType шрифта для консоли.
    + В командной строке можно указать не более двух путей к каталогам,
      файлам или архивам. Первый путь для активной панели, второй -
      для пассивной.
  17.06.2002 SVS
    ! Опция /ttf заменена на /w
    ! Для "far /?" в масдае не выводится
  04.06.2002 SVS
    - BugZ#547 - No dot in help description
  30.05.2002 SVS
    ! Новая опция /ttf - для выставленного TTF-фонта для консольного окна
  21.05.2002 IS
    + Получим реальное значение полного длинного пути с учетом
      символических связей для MainPluginsPath
  18.04.2002 SKV
    + потрогаем floating point что бы VC++ его подключил.
    + ifdef что бы компилировалось под VC 7.0
  08.04.2002 SVS
    ! Расквочим строку MainPluginsPath
  26.03.2002 IS
    + Вызов InitLCIDSort, теперь здесь настраивается сортировка, а не в
      LocalUpperInit
  28.01.2002 VVM
    + Если не смогли считать .лнг файл - то перед выдачей сообщения очистим
      буфер ввода - на всякий случай...
  22.01.2002 SVS
    + Опция /xd  "Enable exception handling" - эт, чтобы в отладчике с
      исключениями работать. В "нормальном" ФАРе ЭТОГО нету. Для включения
      нужно скомпилить ФАР с макросом _DEBUGEXC
  22.01.2002 SVS
    - BugZ#201 - Shift of command prompt after exiting FAR
    + OnliEditorViewerUsed,  =TRUE, если старт был /e или /v
  22.01.2002 SVS
    - BugZ#263 - Opening non-existent file in viewer (beta 4 only)
  11.01.2002 IS
    - "Сделал", блин :-(
      Не в LocalUpperInit это нужно вставлять, а в отдельную функцию, а
      LocalUpperInit должно делать только то, что положено и ни грамма больше.
  10.01.2002 SVS
    - "Сделали", блин. :-(
      Считывание Opt.HotkeyRules должно быть раньше вызова LocalUpperInit
      Хмм... а может его в этот самый LocalUpperInit и впиндюлить?
  09.01.2002 IS
    ! Сделаем LocalUpperInit в самом начале main, в противном случае ловим
      глюки, если используем LocalStricmp.
  24.12.2001 VVM
    ! При ожидании окончания регистрации отдаем время виндам...
  08.11.2001 SVS
    ! Народ не понял предыдущей фишки :-(((
  06.11.2001 SVS
    + В момент старта ФАРа смотрим в [HKCU\Software\Far\System\Environment]
      Если определен, и в этой ветке имеются Values, то FAR при старте
      считывает пары Value-Data и устанавливает переменные окружения.
  18.10.2001 SVS
    ! "причесан" хелп
  05.10.2001 SVS
    ! Opt.ExceptRules и бедагер
  03.10.2001 SVS
    ! если под дебагером, то отключаем исключения однозначно, иначе - смотря
      что указал юзвер.
  26.09.2001 SVS
    + Полиция 19 - если у "полиции" выставлен 19-й бит - игнорировать /p
  16.09.2001 SVS
    - Отключаемые исключения (+ спец опция ком.строки для этих целей)
  05.09.2001 SVS
    ! SetHighlighting() переехала в hilight.cpp
  08.08.2001 SVS
    + позволим для "far /e" работать переключалке F6
  06.08.2001 SVS
    + Ключ "/do" документирован только для "меня" :-)
      Он появится в far /? только тогда, когда ФАР скомпилен с макросом
      DIRECT_RT
      (все время забываю что это за хрень :-)
    - для очистки после выхода нужен Flush() :-(
  27.07.2001 SVS
    + Ключ "/co" документирован.
  24.07.2001 SVS
    ! Заюзаем флаг NotUseCAS - чтобы не гасилось ничего для одиночного
      редатора/вьювера (far /e) - иначе БАГА!
    + Чистим экран после выхода из ФАРа
  11.07.2001 SVS
    + переменные среды: FARLANG и FARUSER
  10.07.2001 SVS
    ! Изменены (обобщены) расширения для архиваторов.
  07.07.2001 IS
    ! В 806 патче я выкинул один костыль, связанный с обработкой каталогов с
      именем "..", а взамен ничего не дал. Исправляю ситуацию :-)
  06.07.2001 IS
    ! Изменения в SetHighlighting в соответствие с изменениями в структуре
      HighlightData
  02.07.2001 IS
    - Баг: дублировались записи в PluginsCache в том случае, если far.exe был
      запущен с указанием короткого пути.
  29.06.2001 OT
    ! Косметика оказалась ... с душком-с :( Отменяем :)
  23.06.2001 OT
    ! косметика для команд far -v/-e. Не нужны нам теперь панели-болванки.
  29.05.2001 tran
    + DIRECT_RT
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  23.04.2001 SVS
    ! КХЕ! Новый вз<ляд на %PATHEXT% - то что редактируем и то, что
      юзаем - разные сущности.
  08.04.2001 SVS
    ! Изменена обработка PATHEXT - при старте FAR`а преобразуем к нужному виду.
  04.04.2001 SVS
    ! Немного опитимизации кода в SetHighlighting()
  03.04.2001 SVS
    ! CmdExt - полностью переработан с учетом %PATHEXT% и WinNT (*.cmd)
    ! Уточнение архиваторных расширений.
  02.04.2001 SVS
    ! Добавлено "некоторое" количество расширений в раскраску для архивов.
    + *.vbs,*.js - подсветка как у исполняемых файлов.
  07.03.2001 IS
    - Избавление от падения при удалении папок в корзину (на основе информации
      от Веши).Удаляем неугодную ветку, если она пуста, потому что в этом
      случае Фар может упасть при удалении каталогов в корзину.
  05.03.2001 SVS
    + Закомментированный /co для /?
  02.03.2001 SVS
    - А слеш поставить забыли :-((((
  01.03.2001 SVS
    ! Унифицирована функция SetHighlighting() - в последствии проще будет
      изменять или добавлять - меняем лишь массив.
    ! При старте ФАР переносит данные в реестре из "Highlight" в
      "Colors\Highlight". Ветка "Highlight" прибивается.
  19.01.2001 SVS
    + FAR.EXE /?
    + проверка на количество строк (MListEval)
  30.12.2000 SVS
    + Проинициализируем функции работы с атрибутами Encryped сразу после
      старта FAR
  28.12.2000 SVS
    + Opt.HotkeyRules - Правило на счет выбора механизма хоткеев
  21.12.2000 SVS
    Если не нашли LNG, то после выдачи сообщения ожидаем любую клавишу.
  15.12.2000 SVS
    + В случае, если не нашли нужных LNG-файлов - выдаем простое сообщение
      и не выпендрючиваемся.
  01.09.2000 tran
    + /co switch
  03.08.2000 SVS
    ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
    ! Не срабатывал шаблон поиска файлов для под-юзеров
    + Новый параметр "Вместо стандарного %FAR%\Plugins искать плагины из
      указанного пути". При этом персональные тоже грузится будут.
      IMHO нехрен этому параметру делать в системных настройках!!!
  07.07.2000 IS
    - Декларация SetHighlighting перешла в fn.cpp
  07.07.2000 SVS
    + Получить разграничитель слов из реестра (общий для редактирования)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "chgprior.hpp"
#include "colors.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "lockscrn.hpp"
#include "hilight.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "language.hpp"
#include "farexcpt.hpp"

#ifdef DIRECT_RT
int DirectRT=0;
#endif

static void ConvertOldSettings();
/* $ 07.07.2000 IS
  Вынес эту декларацию в fn.cpp, чтобы была доступна отовсюду...
*/
//static void SetHighlighting();
/* IS $ */
static void CopyGlobalSettings();

static void show_help(void)
{
printf(
"The following switches may be used in the command line:\n\n"
" /?   This help.\n"
" /a   Disable display of characters with codes 0 - 31 and 255.\n"
" /ag  Disable display of pseudographics characters.\n");
#if defined(USE_WFUNC)
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    printf(
" /8   Forces FAR to work in ANSI (non-Unicode) console.\n"
    );
  }
#endif
printf(
" /e[<line>[:<pos>]] <filename>\n"
"      Edit the specified file.\n"
" /i   Set small (16x16) icon for FAR console window.\n"
" /p[<path>]\n"
"      Search for \"common\" plugins in the directory, specified by <path>.\n"
" /co  Forces FAR to load plugins from the cache only.\n"
" /u <username>\n"
"      Allows to have separate settings for different users.\n"
" /v <filename>\n"
"      View the specified file. If <filename> is -, data is read from the stdin.\n"
" /x   Disable exception handling.\n"
#if defined(_DEBUGEXC)
" /xd  Enable exception handling.\n"
" /cr  Disable check registration.\n"
#endif
#ifdef DIRECT_RT
" /do  Direct output.\n"
#endif
);
#if 0
#if defined(USE_WFUNC)
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    printf(
" /w   Specify this if you are using a TrueType font for the console.\n"
    );
  }
#endif
#endif
}

static int MainProcess(char *EditName,char *ViewName,char *DestName1,char *DestName2,int StartLine,int StartChar,int RegOpt)
{
  {
    ChangePriority ChPriority(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? THREAD_PRIORITY_ABOVE_NORMAL:THREAD_PRIORITY_NORMAL);
    ControlObject CtrlObj;
    if (*EditName || *ViewName)
    {
      NotUseCAS=TRUE;
      Opt.OnlyEditorViewerUsed=1;
      Panel *DummyPanel=new Panel;
      CmdMode=TRUE;
      _tran(SysLog("create dummy panels"));
      CtrlObj.CreateFilePanels();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=DummyPanel;
      CtrlObj.Plugins.LoadPlugins();
      if (*EditName)
      {
        FileEditor *ShellEditor=new FileEditor(EditName,TRUE,TRUE,StartLine,StartChar);
        _tran(SysLog("make shelleditor %p",ShellEditor));
        if (!ShellEditor->GetExitCode()){ // ????????????
          FrameManager->ExitMainLoop(0);
        }
      }
      if (*ViewName)
      {
        FileViewer *ShellViewer=new FileViewer(ViewName,FALSE);
        if (!ShellViewer->GetExitCode()){
          FrameManager->ExitMainLoop(0);
        }
        _tran(SysLog("make shellviewer, %p",ShellViewer));
      }
      FrameManager->EnterMainLoop();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=NULL;
      delete DummyPanel;
      _tran(SysLog("editor/viewer closed, delete dummy panels"));
    }
    else
    {
      char Path[NM];
      Opt.OnlyEditorViewerUsed=0;
      NotUseCAS=FALSE;
#ifdef _DEBUGEXC
      if(CheckRegistration)
      {
#endif
        if (RegOpt)
          Register();
        static struct RegInfo Reg;
        if(_beginthread(CheckReg,0x10000,&Reg) == -1)
        {
          RegistrationBugs=TRUE;
          CheckReg(&Reg);
        }
        else
          RegistrationBugs=FALSE;
        while (!Reg.Done)
          Sleep(10);
#ifdef _DEBUGEXC
      }
#endif

      Opt.SetupArgv=0;

      // воспользуемся тем, что ControlObject::Init() создает панели
      // юзая Opt.*
      if(*DestName1) // актиная панель
      {
        Opt.SetupArgv++;
        strcpy(Path,DestName1);
        *PointToName(Path)=0;
        DeleteEndSlash(Path); // если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."
        if(Path[1]==':' && !Path[2])
          AddEndSlash(Path);

        // Та панель, которая имеет фокус - активна (начнем по традиции с Левой Панели ;-)
        if(Opt.LeftPanel.Focus)
        {
          Opt.LeftPanel.Type=FILE_PANEL;  // сменим моду панели
          Opt.LeftPanel.Visible=TRUE;     // и включим ее
          strcpy(Opt.LeftFolder,Path);
        }
        else
        {
          Opt.RightPanel.Type=FILE_PANEL;
          Opt.RightPanel.Visible=TRUE;
          strcpy(Opt.RightFolder,Path);
        }

        if(*DestName2) // пассивная панель
        {
          Opt.SetupArgv++;
          strcpy(Path,DestName2);
          *PointToName(Path)=0;
          DeleteEndSlash(Path);
          if(Path[1]==':' && !Path[2])
            AddEndSlash(Path);

          // а здесь с точнотью наоборот - обрабатываем пассивную панель
          if(Opt.LeftPanel.Focus)
          {
            Opt.RightPanel.Type=FILE_PANEL; // сменим моду панели
            Opt.RightPanel.Visible=TRUE;    // и включим ее
            strcpy(Opt.RightFolder,Path);
          }
          else
          {
            Opt.LeftPanel.Type=FILE_PANEL;
            Opt.LeftPanel.Visible=TRUE;
            strcpy(Opt.LeftFolder,Path);
          }
        }
      }

      // теперь все готово - создаем панели!
      CtrlObj.Init();

      // а теперь "провалимся" в каталог или хост-файл (если получится ;-)
      if(*DestName1) // актиная панель
      {
        LockScreen LockScr;
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

        strcpy(Path,PointToName(DestName1));
        if (*Path)
        {
          if (ActivePanel->GoToFile(Path))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }

        if(*DestName2) // пассивная панель
        {
          strcpy(Path,PointToName(DestName2));
          if (*Path)
          {
            if (AnotherPanel->GoToFile(Path))
              AnotherPanel->ProcessKey(KEY_CTRLPGDN);
          }
        }

        // !!! ВНИМАНИЕ !!!
        // Сначала редравим пассивную панель, а потом активную!
        AnotherPanel->Redraw();
        ActivePanel->Redraw();
      }

      FrameManager->EnterMainLoop();
    }

    // очистим за собой!
    SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    MoveRealCursor(0,0);
  }

  CloseConsole();
  RestoreIcons();
  return(0);
}


int _cdecl main(int Argc, char *Argv[])
{
  _OT(SysLog("[[[[[[[[New Session of FAR]]]]]]]]]"));
  char EditName[NM],ViewName[NM];
  char DestName[2][NM];
  int StartLine=-1,StartChar=-1,RegOpt=FALSE;
  int CntDestName=0; // количество параметров-имен каталогов

  *EditName=*ViewName=DestName[0][0]=DestName[1][0]=0;
  CmdMode=FALSE;

  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);

  /*$ 18.04.2002 SKV
    Попользуем floating point что бы проинициализировался vc-ный fprtl.
  */
#ifdef _MSC_VER
  float x=1.1f;
  char buf[15];
  sprintf(buf,"%f",x);
#endif

  // если под дебагером, то отключаем исключения однозначно,
  //  иначе - смотря что указал юзвер.
#if defined(_DEBUGEXC)
  Opt.ExceptRules=-1;
#else
  if(!pIsDebuggerPresent)
    pIsDebuggerPresent=(PISDEBUGGERPRESENT)GetProcAddress(GetModuleHandle("KERNEL32"),"IsDebuggerPresent");
  Opt.ExceptRules=(pIsDebuggerPresent && pIsDebuggerPresent()?0:-1);
#endif

#if defined(USE_WFUNC)
  Opt.UseUnicodeConsole=(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT /* && WinVer.dwMajorVersion >= 5 */)?TRUE:FALSE;
#endif

//  Opt.ExceptRules=-1;
//_SVS(SysLog("Opt.ExceptRules=%d",Opt.ExceptRules));

  /* $ 30.12.2000 SVS
     Проинициализируем функции работы с атрибутами Encryped сразу после
     старта FAR
  */
  GetEncryptFunctions();
  /* SVS $ */

  strcpy(Opt.RegRoot,"Software\\Far");
  /* $ 03.08.2000 SVS
     По умолчанию - брать плагины из основного каталога
  */
  Opt.LoadPlug.MainPluginDir=TRUE;
  /* SVS $ */
  Opt.LoadPlug.PluginsPersonal=TRUE;
  /* $ 01.09.2000 tran
     /co - cache only, */
  Opt.LoadPlug.PluginsCacheOnly=FALSE;
  /* tran $ */
  /* $ 09.01.2002 IS только после этого можно использовать Local* */
  LocalUpperInit();
  /* IS $ */

  for (int I=1;I<Argc;I++)
  {
    if ((Argv[I][0]=='/' || Argv[I][0]=='-') && Argv[I][1])
    {
      switch(toupper(Argv[I][1]))
      {
        case 'A':
          switch (toupper(Argv[I][2]))
          {
            case 0:
              Opt.CleanAscii=TRUE;
              break;
            case 'G':
              Opt.NoGraphics=TRUE;
              break;
          }
          break;
#if defined(USE_WFUNC)
        case '8':
          Opt.UseUnicodeConsole=FALSE;
          break;
#endif
        case 'E':
          if (isdigit(Argv[I][2]))
          {
            StartLine=atoi(&Argv[I][2]);
            char *ChPtr=strchr(&Argv[I][2],':');
            if (ChPtr!=NULL)
              StartChar=atoi(ChPtr+1);
          }
          if (I+1<Argc)
          {
            CharToOem(Argv[I+1],EditName);
            I++;
          }
          break;
        case 'V':
          if (I+1<Argc)
          {
            CharToOem(Argv[I+1],ViewName);
            I++;
          }
          break;
        case 'R':
          RegOpt=TRUE;
          break;
        case 'I':
          Opt.SmallIcon=TRUE;
          break;
        case 'X':
          Opt.ExceptRules=0;
#if defined(_DEBUGEXC)
          if ( toupper(Argv[I][2])=='D' )
            Opt.ExceptRules=1;
#endif
          break;
        case 'U':
          if (I+1<Argc)
          {
            strcat(Opt.RegRoot,"\\Users\\");
            strcat(Opt.RegRoot,Argv[I+1]);
            SetEnvironmentVariable("FARUSER",Argv[I+1]);
            CopyGlobalSettings();
            I++;
          }
          break;
        case 'P':
        {
          // Полиция 19
          if(Opt.Policies.DisabledOptions&FFPOL_USEPSWITCH)
             break;
          Opt.LoadPlug.PluginsPersonal=FALSE;
          Opt.LoadPlug.MainPluginDir=FALSE;
          /* $ 03.08.2000 SVS
            + Новый параметр "Вместо стандарного %FAR%\Plugins искать плагины из
              указанного пути". При этом персональные тоже грузится будут.
              IMHO нехрен этому параметру делать в системных настройках!!!
              /P[<путь>]
              Причем, <путь> может содержать Env-переменные
          */
          if (Argv[I][2])
          {
            strncpy(Opt.LoadPlug.CustomPluginsPath,&Argv[I][2],sizeof(Opt.LoadPlug.CustomPluginsPath));
            /* 18.01.2003 IS
               - Не правильно обрабатывалась команда /p[<path>], если в пути
                 были буквы национального алфавита.
            */
            CharToOem(Opt.LoadPlug.CustomPluginsPath,Opt.LoadPlug.CustomPluginsPath);
            /* IS $ */
          }
          else
          {
            // если указан -P без <путь>, то, считаем, что основные
            //  плагины не загружать вооообще!!!
            Opt.LoadPlug.CustomPluginsPath[0]=0;
          }
          break;
          /* SVS $*/
        }
        /* $ 01.09.2000 tran
           /co switch support */
        case 'C':
            if (toupper(Argv[I][2])=='O')
            {
              Opt.LoadPlug.PluginsCacheOnly=TRUE;
              Opt.LoadPlug.PluginsPersonal=FALSE;
            }
#ifdef _DEBUGEXC
            else if (toupper(Argv[I][2])=='R')
              CheckRegistration=FALSE;
#endif
            break;
        /* tran $ */
        /* $ 19.01.2001 SVS
           FAR.EXE /?
        */
        case '?':
        case 'H':
          ControlObject::ShowCopyright(1);
          show_help();
          exit(0);
        /* SVS $ */
#ifdef DIRECT_RT
        case 'D':
          if ( toupper(Argv[I][2])=='O' )
            DirectRT=1;
          break;
#endif
      }
    }
    else // простые параметры. Их может быть max две штукА.
    {
      if(CntDestName < 2)
      {
        CharToOem(Argv[I],DestName[CntDestName++]);
      }
    }
  }

  /* $ 26.03.2002 IS
     Настройка сортировки.
     Должна быть после CopyGlobalSettings и перед InitKeysArray!
  */
  InitLCIDSort();
  /* IS $ */
  /* $ 11.01.2002 IS
     Инициализация массива клавиш. Должна быть после CopyGlobalSettings!
  */
  InitKeysArray();
  /* IS $ */

  WaitForInputIdle(GetCurrentProcess(),0);
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));

#if _MSC_VER>=1300
  _set_new_handler(0);
#else
  set_new_handler(0);
#endif

  /* $ 08.01.2003 SVS
     BugZ#765 - Ключи командной строки парсятся неоднозначно.
  */
  if(Opt.LoadPlug.MainPluginDir)
    Opt.LoadPlug.PluginsCacheOnly=FALSE;

  if(Opt.LoadPlug.PluginsCacheOnly)
  {
    *Opt.LoadPlug.CustomPluginsPath=0;
    Opt.LoadPlug.MainPluginDir=FALSE;
    Opt.LoadPlug.PluginsPersonal=FALSE;
  }

  SetFileApisToOEM();
  GetModuleFileName(NULL,FarPath,sizeof(FarPath));
#if defined(USE_WFUNC)
//  if(Opt.CleanAscii || Opt.NoGraphics)
//    Opt.UseUnicodeConsole=FALSE;
#endif
  /* $ 02.07.2001 IS
     Учтем то, что GetModuleFileName иногда возвращает короткое имя, которое
     нам нафиг не нужно.
  */
  *(PointToName(FarPath)-1)=0;
  {
     char tmpFarPath[sizeof(FarPath)];
     DWORD s=RawConvertShortNameToLongName(FarPath, tmpFarPath,
                                           sizeof(tmpFarPath));
     if(s && s<sizeof(tmpFarPath))
        strcpy(FarPath, tmpFarPath);
  }
  AddEndSlash(FarPath);
  /* IS $ */
  /* $ 03.08.2000 SVS
     Если не указан параметр -P
  */
//  if(Opt.LoadPlug.MainPluginDir)
//    sprintf(Opt.LoadPlug.CustomPluginsPath,"%s%s",FarPath,PluginsFolderName);
  /* SVS $*/
  InitDetectWindowedMode();
  InitConsole();
  GetRegKey("Language","Main",Opt.Language,"English",sizeof(Opt.Language));
  if (!Lang.Init(FarPath,MListEval))
  {
    /* $ 15.12.2000 SVS
       В случае, если не нашли нужных LNG-файлов - выдаем простое сообщение
       и не выпендрючиваемся.
    */
    //Message(MSG_WARNING,1,"Error","Cannot load language data","Ok");
    ControlObject::ShowCopyright(1);
    fprintf(stderr,"\nError: Cannot load language data\n\nPress any key...");
    FlushConsoleInputBuffer(hConInp);
    WaitKey(); // А стоит ли ожидать клавишу??? Стоит
    exit(0);
    /* SVS $ */
  }
  SetEnvironmentVariable("FARLANG",Opt.Language);
  ConvertOldSettings();
  SetHighlighting();
  /* $ 07.03.2001 IS
     - Избавление от падения при удалении папок в корзину (на основе информации
       от Веши).Удаляем неугодную ветку, если она пуста, потому что в этом
       случае Фар может упасть при удалении каталогов в корзину.
  */
  {
    DeleteEmptyKey(HKEY_CLASSES_ROOT,"Directory\\shellex\\CopyHookHandlers");
  }
  /* IS $ */


  int Result;
  if(Opt.ExceptRules)
  {
    TRY{
      Result=MainProcess(EditName,ViewName,DestName[0],DestName[1],StartLine,StartChar,RegOpt);
    }
    EXCEPT(xfilter((int)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1)){
       TerminateProcess( GetCurrentProcess(), 1);
    }
  }
  else
    Result=MainProcess(EditName,ViewName,DestName[0],DestName[1],StartLine,StartChar,RegOpt);

  SetConsoleTitle(OldTitle);
  _OT(SysLog("[[[[[Exit of FAR]]]]]]]]]"));
  return Result;
}

void ConvertOldSettings()
{
  // Конвертим реестр :-) Бывает же такое...
  if(!CheckRegKey(RegColorsHighlight))
    if(CheckRegKey("Highlight"))
    {
      char NameSrc[NM],NameDst[NM];
      strcpy(NameSrc,Opt.RegRoot);
      strcat(NameSrc,"\\Highlight");
      strcpy(NameDst,Opt.RegRoot);
      strcat(NameDst,"\\");
      strcat(NameDst,RegColorsHighlight);
      CopyKeyTree(NameSrc,NameDst,"\0");
    }
  DeleteKeyTree("Highlight");
}

/* $ 03.08.2000 SVS
  ! Не срабатывал шаблон поиска файлов для под-юзеров
*/
void CopyGlobalSettings()
{
  if (CheckRegKey("")) // при существующем - вываливаемся
    return;
  // такого извера нету - перенесем данные!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0");
  SetRegRootKey(HKEY_CURRENT_USER);
  CopyKeyTree("Software\\Far",Opt.RegRoot,"Software\\Far\\Users\0Software\\Far\\PluginsCache\0");
  //  "Вспомним" путь по шаблону!!!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKey("System","TemplatePluginsPath",Opt.LoadPlug.PersonalPluginsPath,"",sizeof(Opt.LoadPlug.PersonalPluginsPath));
  // удалим!!!
  DeleteRegKey("System");
  // запишем новое значение!
  SetRegRootKey(HKEY_CURRENT_USER);
  SetRegKey("System","PersonalPluginsPath",Opt.LoadPlug.PersonalPluginsPath);
}
/* SVS $ */
