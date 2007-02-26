/*
main.cpp

Функция main.

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

class TConsoleRestore{
  private:
    string strOldTitle;
    CONSOLE_SCREEN_BUFFER_INFO sbi;
    HANDLE hOutput;
    BOOL IsRectoreConsole;
  public:
    TConsoleRestore(BOOL RectoreConsole){
      hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      apiGetConsoleTitle (strOldTitle);
      GetConsoleScreenBufferInfo(hOutput,&sbi);
      IsRectoreConsole=RectoreConsole;
    };
    ~TConsoleRestore(){
      if(IsRectoreConsole)
      {
        SetConsoleTitleW(strOldTitle);
        //SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
        SetConsoleWindowInfo(hOutput,TRUE,&sbi.srWindow);
        SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
      }
    };
};

static void ConvertOldSettings();
/* $ 07.07.2000 IS
  Вынес эту декларацию в fn.cpp, чтобы была доступна отовсюду...
*/
//static void SetHighlighting();
/* IS $ */
static void CopyGlobalSettings();

static void show_help(void)
{
wprintf(
L"Usage: far [switches] [apath [ppath]]\n\n"
L"wheren\n"
L"  apath - path to a folder (or a file or an archive) for the active panel\n"
L"  ppath - path to a folder (or a file or an archive) for the passive panel\n\n"
L"The following switches may be used in the command line:\n\n"
L" /?   This help.\n"
L" /a   Disable display of characters with codes 0 - 31 and 255.\n"
L" /ag  Disable display of pseudographics characters.\n");
#if defined(USE_WFUNC)
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    wprintf(
L" /8   Forces FAR to work in ANSI (non-Unicode) console.\n"
    );
  }
#endif
wprintf(
L" /e[<line>[:<pos>]] <filename>\n"
L"      Edit the specified file.\n"
L" /i   Set small (16x16) icon for FAR console window.\n"
L" /p[<path>]\n"
L"      Search for \"common\" plugins in the directory, specified by <path>.\n"
L" /co  Forces FAR to load plugins from the cache only.\n"
L" /rc  Restore console windows settings upon exiting FAR.\n"
L" /u <username>\n"
L"      Allows to have separate settings for different users.\n"
L" /v <filename>\n"
L"      View the specified file. If <filename> is -, data is read from the stdin.\n"
L" /x   Disable exception handling.\n"
#if defined(_DEBUGEXC)
L" /xd  Enable exception handling.\n"
L" /cr  Disable check registration.\n"
#endif
#ifdef DIRECT_RT
L" /do  Direct output.\n"
#endif
);
}

void QueryRegistration ()
{
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
}

static int MainProcess(
        const wchar_t *lpwszEditName,
        const wchar_t *lpwszViewName,
        const wchar_t *lpwszDestName1,
        const wchar_t *lpwszDestName2,
        int StartLine,
        int StartChar,
        int RegOpt
        )
{
  {
    ChangePriority ChPriority(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? THREAD_PRIORITY_ABOVE_NORMAL:THREAD_PRIORITY_NORMAL);
    ControlObject CtrlObj;

    if ( *lpwszEditName || *lpwszViewName )
    {
      NotUseCAS=TRUE;
      Opt.OnlyEditorViewerUsed=1;
      Panel *DummyPanel=new Panel;
      CmdMode=TRUE;
      _tran(SysLog(L"create dummy panels"));
      CtrlObj.CreateFilePanels();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=DummyPanel;
      CtrlObj.Plugins.LoadPlugins();

      if ( *lpwszEditName )
      {
        FileEditor *ShellEditor=new FileEditor(lpwszEditName,CP_AUTODETECT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6,StartLine,StartChar);
        _tran(SysLog(L"make shelleditor %p",ShellEditor));
        if (!ShellEditor->GetExitCode()){ // ????????????
          FrameManager->ExitMainLoop(0);
        }
      }
      else // TODO: Этот else убрать только после разборок с возможностью задавать несколько /e и /v в ком.строке
      if ( *lpwszViewName )
      {
        FileViewer *ShellViewer=new FileViewer(lpwszViewName,FALSE);
        if (!ShellViewer->GetExitCode()){
          FrameManager->ExitMainLoop(0);
        }
        _tran(SysLog(L"make shellviewer, %p",ShellViewer));
      }

      FrameManager->EnterMainLoop();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=NULL;
      delete DummyPanel;
      _tran(SysLog(L"editor/viewer closed, delete dummy panels"));
    }
    else
    {
      Opt.OnlyEditorViewerUsed=0;
      NotUseCAS=FALSE;
#ifdef _DEBUGEXC
      if(CheckRegistration)
      {
#endif
        if (RegOpt)
        {
          Register();
          QueryRegistration ();
        }
#ifdef _DEBUGEXC
      }
#endif

      Opt.SetupArgv=0;

      string strPath;
      // воспользуемся тем, что ControlObject::Init() создает панели
      // юзая Opt.*
      if( *lpwszDestName1 ) // актиная панель
      {
        Opt.SetupArgv++;

        strPath = lpwszDestName1;

        CutToNameUNCW (strPath);
        DeleteEndSlashW(strPath); // BUGBUG!! если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

        if( strPath.At(1)==L':' && !strPath.At(2))
          AddEndSlashW(strPath);

        // Та панель, которая имеет фокус - активна (начнем по традиции с Левой Панели ;-)
        if(Opt.LeftPanel.Focus)
        {
          Opt.LeftPanel.Type=FILE_PANEL;  // сменим моду панели
          Opt.LeftPanel.Visible=TRUE;     // и включим ее

          Opt.strLeftFolder = strPath;
        }
        else
        {
          Opt.RightPanel.Type=FILE_PANEL;
          Opt.RightPanel.Visible=TRUE;

          Opt.strRightFolder = strPath;
        }

        if( *lpwszDestName2 ) // пассивная панель
        {
          Opt.SetupArgv++;

          strPath = lpwszDestName2;

          CutToNameUNCW (strPath);
          DeleteEndSlashW(strPath); //BUGBUG

          if ( strPath.At(1)==L':' && !strPath.At(2))
            AddEndSlashW(strPath);

          // а здесь с точнотью наоборот - обрабатываем пассивную панель
          if(Opt.LeftPanel.Focus)
          {
            Opt.RightPanel.Type=FILE_PANEL; // сменим моду панели
            Opt.RightPanel.Visible=TRUE;    // и включим ее
            Opt.strRightFolder = strPath;
          }
          else
          {
            Opt.LeftPanel.Type=FILE_PANEL;
            Opt.LeftPanel.Visible=TRUE;
            Opt.strLeftFolder = strPath;
          }
        }
      }

      // теперь все готово - создаем панели!
      CtrlObj.Init();

      // а теперь "провалимся" в каталог или хост-файл (если получится ;-)
      if( *lpwszDestName1 ) // актиная панель
      {
        LockScreen LockScr;
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

        strPath = PointToNameUNCW (lpwszDestName1);

        if ( !strPath.IsEmpty() )
        {
          if (ActivePanel->GoToFileW(strPath))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }

        if( *lpwszDestName2 ) // пассивная панель
        {
          strPath = PointToNameUNCW (lpwszDestName2);

          if ( !strPath.IsEmpty() )
          {
            if (AnotherPanel->GoToFileW(strPath))
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
    SetScreen(0,0,ScrX,ScrY,L' ',F_LIGHTGRAY|B_BLACK);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    MoveRealCursor(0,0);
  }

  CloseConsole();
  RestoreIcons();
  return(0);
}

#ifdef __GNUC__
int _cdecl wmain(int Argc, wchar_t *Argv[]);

int main()
{
    LPWSTR* wstrCmdLineArgs;
    int nArgs;
    wstrCmdLineArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);

    return wmain(nArgs, wstrCmdLineArgs);
}
#endif
int wmain_sehed(string& strEditName,string& strViewName,string& DestName1,string& DestName2,int StartLine,int StartChar,int RegOpt)
{
  TRY{
    return MainProcess(strEditName,strViewName,DestName1,DestName2,StartLine,StartChar,RegOpt);
  }
  EXCEPT(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1)){
     TerminateProcess( GetCurrentProcess(), 1);
  }
  return -1; // Никогда сюда не попадем
}
int _cdecl wmain(int Argc, wchar_t *Argv[])
{
  _OT(SysLog(L"[[[[[[[[New Session of FAR]]]]]]]]]"));

  string strEditName;
  string strViewName;

  string DestNames[2];

  int StartLine=-1,StartChar=-1,RegOpt=FALSE,RectoreConsole=FALSE;
  int CntDestName=0; // количество параметров-имен каталогов

  CmdMode=FALSE;

  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionExW(&WinVer);

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
    pIsDebuggerPresent=(PISDEBUGGERPRESENT)GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"),"IsDebuggerPresent");
  Opt.ExceptRules=(pIsDebuggerPresent && pIsDebuggerPresent()?0:-1);
#endif


//  Opt.ExceptRules=-1;
//_SVS(SysLog(L"Opt.ExceptRules=%d",Opt.ExceptRules));

  /* $ 30.12.2000 SVS
     Проинициализируем функции работы с атрибутами Encryped сразу после
     старта FAR
  */
  GetEncryptFunctions();
  /* SVS $ */

  SetRegRootKey(HKEY_CURRENT_USER);
  Opt.strRegRoot = L"Software\\Far18";
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

  if ( apiGetModuleFileName (NULL, g_strFarPath) )
  {
    CutToSlashW (g_strFarPath);

    ConvertNameToLongW (g_strFarPath, g_strFarPath);
    SetEnvironmentVariableW (L"FARHOME", g_strFarPath);

    AddEndSlashW(g_strFarPath);
  }

  for (int I=1;I<Argc;I++)
  {
    if ((Argv[I][0]==L'/' || Argv[I][0]==L'-') && Argv[I][1])
    {
      switch(LocalUpperW(Argv[I][1]))
      {
        case L'A':
          switch (LocalUpperW(Argv[I][2]))
          {
            case 0:
              Opt.CleanAscii=TRUE;
              break;
            case L'G':
              Opt.NoGraphics=TRUE;
              break;
          }
          break;
        case L'E':
          if (iswdigit(Argv[I][2]))
          {
            StartLine=_wtoi(&Argv[I][2]);
            wchar_t *ChPtr=wcschr(&Argv[I][2],L':');
            if (ChPtr!=NULL)
              StartChar=_wtoi(ChPtr+1);
          }
          if (I+1<Argc)
          {
            strEditName = Argv[I+1];
            I++;
          }
          break;
        case L'V':
          if (I+1<Argc)
          {
            strViewName = Argv[I+1];
            I++;
          }
          break;
        case L'R':
          switch (LocalUpperW(Argv[I][2]))
          {
            case 0:
              RegOpt=TRUE;
              break;
            case L'C':
              RectoreConsole=TRUE;
              break;
          }
          break;
        case L'I':
          Opt.SmallIcon=TRUE;
          break;
        case L'X':
          Opt.ExceptRules=0;
#if defined(_DEBUGEXC)
          if ( LocalUpperW(Argv[I][2])==L'D' )
            Opt.ExceptRules=1;
#endif
          break;
        case L'U':
          if (I+1<Argc)
          {
            Opt.strRegRoot += L"\\Users\\";
            Opt.strRegRoot += Argv[I+1];
            SetEnvironmentVariableW(L"FARUSER", Argv[I+1]);
            CopyGlobalSettings();
            I++;
          }
          break;
        case L'P':
        {
          // Полиция 19
          if(Opt.Policies.DisabledOptions&FFPOL_USEPSWITCH)
             break;
          Opt.LoadPlug.PluginsPersonal=FALSE;
          Opt.LoadPlug.MainPluginDir=FALSE;

          if (Argv[I][2])
          {
            apiExpandEnvironmentStrings (&Argv[I][2], Opt.LoadPlug.strCustomPluginsPath);
            UnquoteW(Opt.LoadPlug.strCustomPluginsPath);
            ConvertNameToFullW(Opt.LoadPlug.strCustomPluginsPath,Opt.LoadPlug.strCustomPluginsPath);
/*
            if(Argv[I][2]==L'.' && (Argv[I][3]==0 || Argv[I][3]==L'\\' || Argv[I][3]==L'.'))
            {
              GetCurrentDirectoryExW(Opt.LoadPlug.strCustomPluginsPath);
              AddEndSlashW(Opt.LoadPlug.strCustomPluginsPath);
              Opt.LoadPlug.strCustomPluginsPath += Argv[I][2];
            }
            else
                Opt.LoadPlug.strCustomPluginsPath = Argv[I][2];
*/
              //xstrncpy(Opt.LoadPlug.CustomPluginsPath,szPath,sizeof(Opt.LoadPlug.CustomPluginsPath));

            //FAR_CharToOem(Opt.LoadPlug.CustomPluginsPath,Opt.LoadPlug.CustomPluginsPath);
          }
          else
          {
            // если указан -P без <путь>, то, считаем, что основные
            //  плагины не загружать вооообще!!!
            Opt.LoadPlug.strCustomPluginsPath=L"";
          }
          break;
        }
        /* $ 01.09.2000 tran
           /co switch support */
        case L'C':
            if (LocalUpperW(Argv[I][2])==L'O')
            {
              Opt.LoadPlug.PluginsCacheOnly=TRUE;
              Opt.LoadPlug.PluginsPersonal=FALSE;
            }
#ifdef _DEBUGEXC
            else if (LocalUpperW(Argv[I][2])==L'R')
              CheckRegistration=FALSE;
#endif
            break;
        /* tran $ */
        /* $ 19.01.2001 SVS
           FAR.EXE /?
        */
        case L'?':
        case L'H':
          ControlObject::ShowCopyright(1);
          show_help();
          exit(0);
        /* SVS $ */
#ifdef DIRECT_RT
        case L'D':
          if ( LocalUpperW(Argv[I][2])==L'O' )
            DirectRT=1;
          break;
#endif
      }
    }
    else // простые параметры. Их может быть max две штукА.
    {
      if(CntDestName < 2)
      {
        apiExpandEnvironmentStrings (Argv[I], DestNames[CntDestName]);
        UnquoteW(DestNames[CntDestName]);
        ConvertNameToFullW(Argv[I],DestNames[CntDestName]);
        if(GetFileAttributesW(DestNames[CntDestName]) != -1)
          CntDestName++; //???
      }
    }
  }

  InitializeSetupAPI ();

  //Инициализация массива клавиш. Должна быть после CopyGlobalSettings!
  InitKeysArray();

  WaitForInputIdle(GetCurrentProcess(),0);

  TConsoleRestore __ConsoleRestore(RectoreConsole);

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
    Opt.LoadPlug.strCustomPluginsPath=L"";
    Opt.LoadPlug.MainPluginDir=FALSE;
    Opt.LoadPlug.PluginsPersonal=FALSE;
  }

  InitDetectWindowedMode();
  InitConsole();
  GetRegKeyW(L"Language",L"Main",Opt.strLanguage,L"English");
  if (!Lang.Init(g_strFarPath,MListEval))
  {
    ControlObject::ShowCopyright(1);
    fprintf(stderr,"\nError: Cannot load language data\n\nPress any key...");
    FlushConsoleInputBuffer(hConInp);
    WaitKey(); // А стоит ли ожидать клавишу??? Стоит
    exit(0);
  }
  SetEnvironmentVariableW(L"FARLANG",Opt.strLanguage);
  ConvertOldSettings();
  SetHighlighting();


  DeleteEmptyKeyW(HKEY_CLASSES_ROOT,L"Directory\\shellex\\CopyHookHandlers");

  initMacroVarTable(0);
  initMacroVarTable(1);

#ifdef _DEBUGEXC
  if(CheckRegistration)
  {
#endif
    RegVer=-1;
    QueryRegistration ();
#ifdef _DEBUGEXC
  }
  else
    RegVer = 1;
#endif

  int Result=0;
  if(Opt.ExceptRules)
  {
    Result=wmain_sehed(strEditName,strViewName,DestNames[0],DestNames[1],StartLine,StartChar,RegOpt);
  }
  else
    Result=MainProcess(strEditName,strViewName,DestNames[0],DestNames[1],StartLine,StartChar,RegOpt);

  UsedInternalClipboard=TRUE;
  FAR_EmptyClipboard();

  FinalizeSetupAPI ();

  doneMacroVarTable(1);
  doneMacroVarTable(0);

  _OT(SysLog(L"[[[[[Exit of FAR]]]]]]]]]"));
  return Result;
}

void ConvertOldSettings()
{
  // Конвертим реестр :-) Бывает же такое...
  if(!CheckRegKeyW(RegColorsHighlight))
    if(CheckRegKeyW(L"Highlight"))
    {
      string strNameSrc, strNameDst;
      strNameSrc = Opt.strRegRoot;
      strNameSrc += L"\\Highlight";
      strNameDst = Opt.strRegRoot;
      strNameDst += L"\\";
      strNameDst += RegColorsHighlight;
      CopyKeyTreeW(strNameSrc,strNameDst,L"\0");
    }
  DeleteKeyTreeW(L"Highlight");
}

/* $ 03.08.2000 SVS
  ! Не срабатывал шаблон поиска файлов для под-юзеров
*/
void CopyGlobalSettings()
{
  if (CheckRegKeyW(L"")) // при существующем - вываливаемся
    return;
  // такого извера нету - перенесем данные!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  CopyKeyTreeW(L"Software\\Far18",Opt.strRegRoot,L"Software\\Far\\Users18\0");
  SetRegRootKey(HKEY_CURRENT_USER);
  CopyKeyTreeW(L"Software\\Far18",Opt.strRegRoot,L"Software\\Far\\Users18\0Software\\Far\\PluginsCache\0");
  //  "Вспомним" путь по шаблону!!!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKeyW(L"System",L"TemplatePluginsPath",Opt.LoadPlug.strPersonalPluginsPath,L"");
  // удалим!!!
  DeleteRegKeyW(L"System");
  // запишем новое значение!
  SetRegRootKey(HKEY_CURRENT_USER);
  SetRegKeyW(L"System",L"PersonalPluginsPath",Opt.LoadPlug.strPersonalPluginsPath);
}
/* SVS $ */
