/*
main.cpp

Функция main.
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
L" /x   Disable exception handling.\n");
#if defined(_DEBUGEXC)
wprintf(
L" /xd  Enable exception handling.\n");
#endif
#ifdef DIRECT_RT
wprintf(
L" /do  Direct output.\n");
#endif
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

      Opt.SetupArgv=0;

      string strPath;
      // воспользуемся тем, что ControlObject::Init() создает панели
      // юзая Opt.*
      if( *lpwszDestName1 ) // актиная панель
      {
        Opt.SetupArgv++;

        strPath = lpwszDestName1;

        CutToNameUNC(strPath);
        DeleteEndSlash(strPath); // BUGBUG!! если конечный слешь не убрать - получаем забавный эффект - отсутствует ".."

        if( strPath.At(1)==L':' && !strPath.At(2))
          AddEndSlash(strPath);

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

          CutToNameUNC(strPath);
          DeleteEndSlash(strPath); //BUGBUG

          if ( strPath.At(1)==L':' && !strPath.At(2))
            AddEndSlash(strPath);

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

        strPath = PointToNameUNC(lpwszDestName1);

        if ( !strPath.IsEmpty() )
        {
          if (ActivePanel->GoToFile(strPath))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }

        if( *lpwszDestName2 ) // пассивная панель
        {
          strPath = PointToNameUNC(lpwszDestName2);

          if ( !strPath.IsEmpty() )
          {
            if (AnotherPanel->GoToFile(strPath))
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

  // Проинициализируем функции работы с атрибутами Encryped сразу после старта FAR
  GetEncryptFunctions();

  SetRegRootKey(HKEY_CURRENT_USER);
  Opt.strRegRoot = L"Software\\Far18";
  // По умолчанию - брать плагины из основного каталога
  Opt.LoadPlug.MainPluginDir=TRUE;
  Opt.LoadPlug.PluginsPersonal=TRUE;
  Opt.LoadPlug.PluginsCacheOnly=FALSE;

  if ( apiGetModuleFileName (NULL, g_strFarPath) )
  {
    CutToSlash(g_strFarPath);

    ConvertNameToLong (g_strFarPath, g_strFarPath);
    SetEnvironmentVariableW (L"FARHOME", g_strFarPath);

    AddEndSlash(g_strFarPath);
  }

  for (int I=1;I<Argc;I++)
  {
    if ((Argv[I][0]==L'/' || Argv[I][0]==L'-') && Argv[I][1])
    {
      switch(Upper(Argv[I][1]))
      {
        case L'A':
          switch (Upper(Argv[I][2]))
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
          switch (Upper(Argv[I][2]))
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
          if ( Upper(Argv[I][2])==L'D' )
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
            Unquote(Opt.LoadPlug.strCustomPluginsPath);
            ConvertNameToFull(Opt.LoadPlug.strCustomPluginsPath,Opt.LoadPlug.strCustomPluginsPath);
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
        case L'C':
            if (Upper(Argv[I][2])==L'O')
            {
              Opt.LoadPlug.PluginsCacheOnly=TRUE;
              Opt.LoadPlug.PluginsPersonal=FALSE;
            }
            break;
        case L'?':
        case L'H':
          ControlObject::ShowCopyright(1);
          show_help();
          exit(0);
#ifdef DIRECT_RT
        case L'D':
          if ( Upper(Argv[I][2])==L'O' )
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
        Unquote(DestNames[CntDestName]);
        ConvertNameToFull(Argv[I],DestNames[CntDestName]);
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
#elif __GNUC__ > 3
  std::set_new_handler(0);
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
  GetRegKey(L"Language",L"Main",Opt.strLanguage,L"English");
  if (!Lang.Init(g_strFarPath,MNewFileName))
  {
    ControlObject::ShowCopyright(1);
    fprintf(stderr,"\nError: Cannot load language data\n\nPress any key...");
    FlushConsoleInputBuffer(hConInp);
    WaitKey(); // А стоит ли ожидать клавишу??? Стоит
    exit(0);
  }
  wcscpy(InitedLanguage,Opt.strLanguage); // ?????? wcsncpy
  SetEnvironmentVariableW(L"FARLANG",Opt.strLanguage);
  ConvertOldSettings();
  SetHighlighting();


  DeleteEmptyKey(HKEY_CLASSES_ROOT,L"Directory\\shellex\\CopyHookHandlers");

  initMacroVarTable(1);

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

  _OT(SysLog(L"[[[[[Exit of FAR]]]]]]]]]"));
  return Result;
}

void ConvertOldSettings()
{
  // Конвертим реестр :-) Бывает же такое...
  if(!CheckRegKey(RegColorsHighlight))
    if(CheckRegKey(L"Highlight"))
    {
      string strNameSrc, strNameDst;
      strNameSrc = Opt.strRegRoot;
      strNameSrc += L"\\Highlight";
      strNameDst = Opt.strRegRoot;
      strNameDst += L"\\";
      strNameDst += RegColorsHighlight;
      CopyKeyTree(strNameSrc,strNameDst,L"\0");
    }
  DeleteKeyTree(L"Highlight");
}

/* $ 03.08.2000 SVS
  ! Не срабатывал шаблон поиска файлов для под-юзеров
*/
void CopyGlobalSettings()
{
  if (CheckRegKey(L"")) // при существующем - вываливаемся
    return;
  // такого извера нету - перенесем данные!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  CopyKeyTree(L"Software\\Far18",Opt.strRegRoot,L"Software\\Far18\\Users\0");
  SetRegRootKey(HKEY_CURRENT_USER);
  CopyKeyTree(L"Software\\Far18",Opt.strRegRoot,L"Software\\Far18\\Users\0Software\\Far\\PluginsCache\0");
  //  "Вспомним" путь по шаблону!!!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKey(L"System",L"TemplatePluginsPath",Opt.LoadPlug.strPersonalPluginsPath,L"");
  // удалим!!!
  DeleteRegKey(L"System");
  // запишем новое значение!
  SetRegRootKey(HKEY_CURRENT_USER);
  SetRegKey(L"System",L"PersonalPluginsPath",Opt.LoadPlug.strPersonalPluginsPath);
}
