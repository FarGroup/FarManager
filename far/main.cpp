/*
main.cpp

Функция main.

*/

/* Revision: 1.37 05.10.2001 $ */

/*
Modify:
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
" /?   This help\n"
" /a   Disable display of characters with codes 0 - 31 and 255.\n"
" /ag  Disable display of pseudographics characters.\n"
//" /co  Forces FAR to load plugins from the cache only.\n"
" /e[<line>[:<pos>]] <filename>\n"
"      Edit the specified file.\n"
" /i   Set small (16x16) icon for FAR console window.\n"
" /p[<path>]\n"
"      Search for \"common\" plugins in the directory, specified by <path>.\n"
" /u <username>\n"
"      Allows to have separate settings for different users.\n"
" /v <filename>\n"
"      View the specified file. If <filename> is -, data is read from the stdin.\n"
" /co\n"
"      Forces FAR to load plugins from the cache only.\n"
" /x\n"
"      Disable exception handling.\n"
#ifdef DIRECT_RT
" /do\n"
"      Direct output.\n"
#endif
);
}


int _cdecl main(int Argc, char *Argv[])
{
  _OT(SysLog("[[[[[[[[New Session of FAR]]]]]]]]]"));
  char EditName[NM],ViewName[NM],DestName[NM];
  int StartLine=-1,StartChar=-1,RegOpt=FALSE;
  *EditName=*ViewName=*DestName=0;
  CmdMode=FALSE;

  // если под дебагером, то отключаем исключения однозначно,
  //  иначе - смотря что указал юзвер.
#if defined(_DEBUGEXC)
  Opt.ExceptRules=-1;
#else
  if(!pIsDebuggerPresent)
    pIsDebuggerPresent=(PISDEBUGGERPRESENT)GetProcAddress(GetModuleHandle("KERNEL32"),"IsDebuggerPresent");
  Opt.ExceptRules=(pIsDebuggerPresent && pIsDebuggerPresent()?0:-1);
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
  Opt.MainPluginDir=TRUE;
  /* SVS $ */
  /* $ 01.09.2000 tran
     /co - cache only, */
  Opt.PluginsCacheOnly=FALSE;
  /* tran $ */
  for (int I=1;I<Argc;I++)
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
           if((Opt.Policies.DisabledOptions >> 19) & 1)
              break;
          /* $ 03.08.2000 SVS
            + Новый параметр "Вместо стандарного %FAR%\Plugins искать плагины из
              указанного пути". При этом персональные тоже грузится будут.
              IMHO нехрен этому параметру делать в системных настройках!!!
              /P[<путь>]
              Причем, <путь> может содержать Env-переменные
          */
          if (Argv[I][2])
          {
            ExpandEnvironmentStrings(&Argv[I][2],MainPluginsPath,sizeof(MainPluginsPath));
          }
          else
          {
            // если указан -P без <путь>, то, считаем, что основные
            //  плагины не загружать вооообще!!!
            MainPluginsPath[0]=0;
          }
          Opt.MainPluginDir=FALSE;
          break;
          /* SVS $*/
        }
        /* $ 01.09.2000 tran
           /co switch support */
        case 'C':
            if (toupper(Argv[I][2])=='O')
            {
                Opt.PluginsCacheOnly=TRUE;
            }
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
    else
      CharToOem(Argv[I],DestName);

  WaitForInputIdle(GetCurrentProcess(),0);
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));

  set_new_handler(0);

  SetFileApisToOEM();
  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionEx(&WinVer);
  /* $ 28.12.2000 SVS
   + Opt.HotkeyRules - Правило на счет выбора механизма хоткеев */
  GetRegKey("Interface","HotkeyRules",Opt.HotkeyRules,1);
  /* SVS $*/
  LocalUpperInit();
  GetModuleFileName(NULL,FarPath,sizeof(FarPath));
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
  if(Opt.MainPluginDir)
    sprintf(MainPluginsPath,"%s%s",FarPath,PluginsFolderName);
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
    WaitKey(-1); // А стоит ли ожидать клавишу??? Стоит
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
  {
    ChangePriority ChPriority(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? THREAD_PRIORITY_ABOVE_NORMAL:THREAD_PRIORITY_NORMAL);
    ControlObject CtrlObj;
    if (*EditName || *ViewName)
    {
      NotUseCAS=TRUE;
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
      }
      if (*ViewName)
      {
        FileViewer *ShellViewer=new FileViewer(ViewName,FALSE);
        _tran(SysLog("make shellviewer, %p",ShellViewer));
      }
      FrameManager->EnterMainLoop();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=NULL;
      delete DummyPanel;
      _tran(SysLog("editor/viewer closed, delete dummy panels"));
    }
    else
    {
      NotUseCAS=FALSE;
      if (RegOpt)
        Register();
      static struct RegInfo Reg;
      _beginthread(CheckReg,0x10000,&Reg);
      while (!Reg.Done)
        Sleep(0);
      CtrlObj.Init();
      if (*DestName)
      {
        LockScreen LockScr;
        char Path[NM];
        strcpy(Path,DestName);
        *PointToName(Path)=0;
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        if (*Path)
          ActivePanel->SetCurDir(Path,TRUE);
        strcpy(Path,PointToName(DestName));
        if (*Path)
        {
          if (ActivePanel->GoToFile(Path))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }
        CtrlObject->Cp()->LeftPanel->Redraw();
        CtrlObject->Cp()->RightPanel->Redraw();
      }
      FrameManager->EnterMainLoop();
    }
    // очистим за собой!
    SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
  }

  SetConsoleTitle(OldTitle);
  CloseConsole();
  RestoreIcons();
  _OT(SysLog("[[[[[Exit of FAR]]]]]]]]]"));
  return(0);
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
  GetRegKey("System","TemplatePluginsPath",Opt.PersonalPluginsPath,"",sizeof(Opt.PersonalPluginsPath));
  // удалим!!!
  DeleteRegKey("System");
  // запишем новое значение!
  SetRegRootKey(HKEY_CURRENT_USER);
  SetRegKey("System","PersonalPluginsPath",Opt.PersonalPluginsPath);
}
/* SVS $ */
