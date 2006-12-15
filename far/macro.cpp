/*
macro.cpp

Макросы

*/

#include "headers.hpp"
#pragma hdrstop

#include "macro.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "plugin.hpp"
#include "fn.hpp"
#include "lockscrn.hpp"
#include "viewer.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "flink.hpp"

// для диалога назначения клавиши
struct DlgParam{
  KeyMacro *Handle;
  DWORD Key;
  int Mode;
};

struct TMacroKeywords MKeywordsArea[] ={
  {0,  "Other",              MACRO_OTHER,0},
  {0,  "Shell",              MACRO_SHELL,0},
  {0,  "Viewer",             MACRO_VIEWER,0},
  {0,  "Editor",             MACRO_EDITOR,0},
  {0,  "Dialog",             MACRO_DIALOG,0},
  {0,  "Search",             MACRO_SEARCH,0},
  {0,  "Disks",              MACRO_DISKS,0},
  {0,  "MainMenu",           MACRO_MAINMENU,0},
  {0,  "Menu",               MACRO_MENU,0},
  {0,  "Help",               MACRO_HELP,0},
  {0,  "Info",               MACRO_INFOPANEL,0},
  {0,  "QView",              MACRO_QVIEWPANEL,0},
  {0,  "Tree",               MACRO_TREEPANEL,0},
  {0,  "FindFolder",         MACRO_FINDFOLDER,0},
  {0,  "UserMenu",           MACRO_USERMENU,0},
  {0,  "Common",             MACRO_COMMON,0},
};

struct TMacroKeywords MKeywords[] ={
  {0,  "Other",              MCODE_C_AREA_OTHER,0},
  {0,  "Shell",              MCODE_C_AREA_SHELL,0},
  {0,  "Viewer",             MCODE_C_AREA_VIEWER,0},
  {0,  "Editor",             MCODE_C_AREA_EDITOR,0},
  {0,  "Dialog",             MCODE_C_AREA_DIALOG,0},
  {0,  "Search",             MCODE_C_AREA_SEARCH,0},
  {0,  "Disks",              MCODE_C_AREA_DISKS,0},
  {0,  "MainMenu",           MCODE_C_AREA_MAINMENU,0},
  {0,  "Menu",               MCODE_C_AREA_MENU,0},
  {0,  "Help",               MCODE_C_AREA_HELP,0},
  {0,  "Info",               MCODE_C_AREA_INFOPANEL,0},
  {0,  "QView",              MCODE_C_AREA_QVIEWPANEL,0},
  {0,  "Tree",               MCODE_C_AREA_TREEPANEL,0},
  {0,  "FindFolder",         MCODE_C_AREA_FINDFOLDER,0},
  {0,  "UserMenu",           MCODE_C_AREA_USERMENU,0},

  // ПРОЧЕЕ
  {2,  "Bof",                MCODE_C_BOF,0},
  {2,  "Eof",                MCODE_C_EOF,0},
  {2,  "Empty",              MCODE_C_EMPTY,0},
  {2,  "DisableOutput",      MCODE_C_DISABLEOUTPUT,0},
  {2,  "Selected",           MCODE_C_SELECTED,0},
  {2,  "IClip",              MCODE_C_ICLIP,0},

  {2,  "Far.Width",          MCODE_V_FAR_WIDTH,0},
  {2,  "Far.Height",         MCODE_V_FAR_HEIGHT,0},
  {2,  "Far.Title",          MCODE_V_FAR_TITLE,0},

  {2,  "ItemCount",          MCODE_V_ITEMCOUNT,0},  // ItemCount - число элементов в текущем объекте
  {2,  "CurPos",             MCODE_V_CURPOS,0},    // CurPos - текущий индекс в текущем объекте
  {2,  "Title",              MCODE_V_TITLE,0},
  {2,  "Height",             MCODE_V_HEIGHT,0},
  {2,  "Width",              MCODE_V_WIDTH,0},

  {2,  "APanel.Empty",       MCODE_C_APANEL_ISEMPTY,0},
  {2,  "PPanel.Empty",       MCODE_C_PPANEL_ISEMPTY,0},
  {2,  "APanel.Bof",         MCODE_C_APANEL_BOF,0},
  {2,  "PPanel.Bof",         MCODE_C_PPANEL_BOF,0},
  {2,  "APanel.Eof",         MCODE_C_APANEL_EOF,0},
  {2,  "PPanel.Eof",         MCODE_C_PPANEL_EOF,0},
  {2,  "APanel.Root",        MCODE_C_APANEL_ROOT,0},
  {2,  "PPanel.Root",        MCODE_C_PPANEL_ROOT,0},
  {2,  "APanel.Visible",     MCODE_C_APANEL_VISIBLE,0},
  {2,  "PPanel.Visible",     MCODE_C_PPANEL_VISIBLE,0},
  {2,  "APanel.Plugin",      MCODE_C_APANEL_PLUGIN,0},
  {2,  "PPanel.Plugin",      MCODE_C_PPANEL_PLUGIN,0},
  {2,  "APanel.FilePanel",   MCODE_C_APANEL_FILEPANEL,0},
  {2,  "PPanel.FilePanel",   MCODE_C_PPANEL_FILEPANEL,0},
  {2,  "APanel.Folder",      MCODE_C_APANEL_FOLDER,0},
  {2,  "PPanel.Folder",      MCODE_C_PPANEL_FOLDER,0},
  {2,  "APanel.Selected",    MCODE_C_APANEL_SELECTED,0},
  {2,  "PPanel.Selected",    MCODE_C_PPANEL_SELECTED,0},
  {2,  "APanel.Left",        MCODE_C_APANEL_LEFT,0},
  {2,  "PPanel.Left",        MCODE_C_PPANEL_LEFT,0},
  {2,  "APanel.LFN",         MCODE_C_APANEL_LFN,0},
  {2,  "PPanel.LFN",         MCODE_C_PPANEL_LFN,0},

  {2,  "APanel.Type",        MCODE_V_APANEL_TYPE,0},
  {2,  "PPanel.Type",        MCODE_V_PPANEL_TYPE,0},
  {2,  "APanel.ItemCount",   MCODE_V_APANEL_ITEMCOUNT,0},
  {2,  "PPanel.ItemCount",   MCODE_V_PPANEL_ITEMCOUNT,0},
  {2,  "APanel.CurPos",      MCODE_V_APANEL_CURPOS,0},
  {2,  "PPanel.CurPos",      MCODE_V_PPANEL_CURPOS,0},
  {2,  "APanel.Current",     MCODE_V_APANEL_CURRENT,0},
  {2,  "PPanel.Current",     MCODE_V_PPANEL_CURRENT,0},
  {2,  "APanel.SelCount",    MCODE_V_APANEL_SELCOUNT,0},
  {2,  "PPanel.SelCount",    MCODE_V_PPANEL_SELCOUNT,0},
  {2,  "APanel.Path",        MCODE_V_APANEL_PATH,0},
  {2,  "PPanel.Path",        MCODE_V_PPANEL_PATH,0},
  {2,  "APanel.UNCPath",     MCODE_V_APANEL_UNCPATH,0},
  {2,  "PPanel.UNCPath",     MCODE_V_PPANEL_UNCPATH,0},
  {2,  "APanel.Height",      MCODE_V_APANEL_HEIGHT,0},
  {2,  "PPanel.Height",      MCODE_V_PPANEL_HEIGHT,0},
  {2,  "APanel.Width",       MCODE_V_APANEL_WIDTH,0},
  {2,  "PPanel.Width",       MCODE_V_PPANEL_WIDTH,0},
  {2,  "APanel.OPIFlags",    MCODE_V_APANEL_OPIFLAGS,0},
  {2,  "PPanel.OPIFlags",    MCODE_V_PPANEL_OPIFLAGS,0},
  {2,  "APanel.DriveType",   MCODE_V_APANEL_DRIVETYPE,0}, // APanel.DriveType - активная панель: тип привода
  {2,  "PPanel.DriveType",   MCODE_V_PPANEL_DRIVETYPE,0}, // PPanel.DriveType - пассивная панель: тип привода


  {2,  "CmdLine.Bof",        MCODE_C_CMDLINE_BOF,0}, // курсор в начале cmd-строки редактирования?
  {2,  "CmdLine.Eof",        MCODE_C_CMDLINE_EOF,0}, // курсор в конеце cmd-строки редактирования?
  {2,  "CmdLine.Empty",      MCODE_C_CMDLINE_EMPTY,0},
  {2,  "CmdLine.Selected",   MCODE_C_CMDLINE_SELECTED,0},
  {2,  "CmdLine.ItemCount",  MCODE_V_CMDLINE_ITEMCOUNT,0},
  {2,  "CmdLine.CurPos",     MCODE_V_CMDLINE_CURPOS,0},
  {2,  "CmdLine.Value",      MCODE_V_CMDLINE_VALUE,0},

  {2,  "Editor.FileName",    MCODE_V_EDITORFILENAME,0},
  {2,  "Editor.CurLine",     MCODE_V_EDITORCURLINE,0},  // текущая линия в редакторе (в дополнении к Count)
  {2,  "Editor.Lines",       MCODE_V_EDITORLINES,0},
  {2,  "Editor.CurPos",      MCODE_V_EDITORCURPOS,0},
  {2,  "Editor.State",       MCODE_V_EDITORSTATE,0},
  {2,  "Editor.Value",       MCODE_V_EDITORVALUE,0},

  {2,  "Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
  {2,  "Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
  {2,  "Dlg.CurPos",         MCODE_V_DLGCURPOS,0},

  {2,  "Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
  {2,  "Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

  {2,  "Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
  {2,  "Viewer.State",       MCODE_V_VIEWERSTATE,0},

  {2,  "Windowed",           MCODE_C_WINDOWEDMODE,0},
},

MKeywordsFlags[] ={
  // ФЛАГИ
  {1,  "DisableOutput",      MFLAGS_DISABLEOUTPUT,0},
  {1,  "RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART,0},
  {1,  "EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE,0},
  {1,  "NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE,0},
  {1,  "EVSelection",        MFLAGS_EDITSELECTION,0},
  {1,  "NoEVSelection",      MFLAGS_EDITNOSELECTION,0},

  {1,  "NoFilePanels",       MFLAGS_NOFILEPANELS,0},
  {1,  "NoPluginPanels",     MFLAGS_NOPLUGINPANELS,0},
  {1,  "NoFolders",          MFLAGS_NOFOLDERS,0},
  {1,  "NoFiles",            MFLAGS_NOFILES,0},
  {1,  "Selection",          MFLAGS_SELECTION,0},
  {1,  "NoSelection",        MFLAGS_NOSELECTION,0},

  {1,  "NoFilePPanels",      MFLAGS_PNOFILEPANELS,0},
  {1,  "NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS,0},
  {1,  "NoPFolders",         MFLAGS_PNOFOLDERS,0},
  {1,  "NoPFiles",           MFLAGS_PNOFILES,0},
  {1,  "PSelection",         MFLAGS_PSELECTION,0},
  {1,  "NoPSelection",       MFLAGS_PNOSELECTION,0},

  {1,  "ReuseMacro",         MFLAGS_REUSEMACRO,0},
  {1,  "NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS,0},
};

int MKeywordsSize = sizeof(MKeywords)/sizeof(*MKeywords);

// транслирующая таблица - имя <-> код макроклавиши
static struct TKeyCodeName{
  int Key;
  int Len;
  char *Name;
} KeyMacroCodes[]={
   { MCODE_OP_AKEY,                 5, "$AKey"    }, // клавиша, которой вызвали макрос
   { MCODE_OP_DATE,                 5, "$Date"    }, // $Date "%d-%a-%Y"
   { MCODE_OP_ELSE,                 5, "$Else"    },
   { MCODE_OP_END,                  4, "$End"     },
   { MCODE_OP_EXIT,                 5, "$Exit"    },
   { MCODE_OP_ICLIP,                6, "$IClip"   },
   { MCODE_OP_IF,                   3, "$If"      },
   { MCODE_OP_SWITCHKBD,           10, "$KbdSwitch"},
   { MCODE_OP_MACROMODE,            6, "$MMode"   },
   { MCODE_OP_REP,                  4, "$Rep"     },
#if defined(MOUSEKEY)
   { MCODE_OP_SELWORD,              8, "$SelWord" },
#endif
   { MCODE_OP_PLAINTEXT,            5, "$Text"    }, // $Text "Plain Text"
   { MCODE_OP_WHILE,                6, "$While"   },
   { MCODE_OP_XLAT,                 5, "$XLat"    },
};


TVarTable glbVarTable, locVarTable;
TVar eStack[MAXEXEXSTACK];

static char __code2symbol(BYTE b1, BYTE b2);
static const char* ParsePlainText(char *CurKeyText, const char *BufPtr);
static const char *__GetNextWord(const char *BufPtr,char *CurKeyText);


// функция преобразования кода макроклавиши в текст
BOOL WINAPI KeyMacroToText(int Key,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;

  char KeyText[128];

  KeyText[0]='\0';
  for (int I=0;I<sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0]);I++)
    if (Key==KeyMacroCodes[I].Key)
    {
      strcpy(KeyText,KeyMacroCodes[I].Name);
      break;
    }

  if(!KeyText[0])
  {
    *KeyText0='\0';
    return FALSE;
  }
  if(Size > 0)
    xstrncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);

  return TRUE;
}

KeyMacro::KeyMacro()
{
  _KEYMACRO(SysLog("[%p] KeyMacro::KeyMacro()", this));
  MacroVersion=GetRegKey("KeyMacros","MacroVersion",0);
  CurPCStack=-1;
  Work.MacroWORKCount=0;
  Work.MacroWORK=NULL;
  LockScr=NULL;
  MacroLIB=NULL;
  RecBuffer=NULL;
  Mode=MACRO_SHELL;
  LoadMacros();
}

KeyMacro::~KeyMacro()
{
  _KEYMACRO(SysLog("[%p] KeyMacro::~KeyMacro()", this));
  InitInternalVars();
}

void KeyMacro::InitInternalLIBVars()
{
  if(MacroLIB)
  {
    for (int I=0;I<MacroLIBCount;I++)
    {
      if(MacroLIB[I].BufferSize > 1 && MacroLIB[I].Buffer)
        xf_free(MacroLIB[I].Buffer);
      if(MacroLIB[I].Src)
        xf_free(MacroLIB[I].Src);
    }
    xf_free(MacroLIB);
  }
  if(RecBuffer) xf_free(RecBuffer);
  MacroLIBCount=0;
  MacroLIB=NULL;
}

// инициализация всех переменных
void KeyMacro::InitInternalVars(BOOL InitedRAM)
{
  InitInternalLIBVars();

  if(LockScr)
  {
    delete LockScr;
    LockScr=NULL;
  }

  if(InitedRAM)
  {
    ReleaseWORKBuffer(TRUE);
    Work.Executing=MACROMODE_NOMACRO;
  }

  RecBuffer=NULL;
  RecBufferSize=0;

  Recording=MACROMODE_NOMACRO;
  InternalInput=FALSE;
}

// удаление временного буфера, если он создавался динамически
// (динамически - значит в PlayMacros передали строку.
void KeyMacro::ReleaseWORKBuffer(BOOL All)
{
  if(Work.MacroWORK)
  {
    if(All || Work.MacroWORKCount <= 1)
    {
      for (int I=0;I<Work.MacroWORKCount;I++)
      {
        if(Work.MacroWORK[I].BufferSize > 1 && Work.MacroWORK[I].Buffer)
          xf_free(Work.MacroWORK[I].Buffer);
        if(Work.MacroWORK[I].Src)
          xf_free(Work.MacroWORK[I].Src);
      }
      xf_free(Work.MacroWORK);
      Work.MacroWORK=NULL;
      Work.MacroWORKCount=0;
    }
    else
    {
      if(Work.MacroWORK->BufferSize > 1 && Work.MacroWORK->Buffer)
        xf_free(Work.MacroWORK->Buffer);
      if(Work.MacroWORK->Src)
        xf_free(Work.MacroWORK->Src);
      Work.MacroWORKCount--;

      memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(struct MacroRecord),sizeof(struct MacroRecord)*Work.MacroWORKCount);
      xf_realloc(Work.MacroWORK,sizeof(struct MacroRecord)*Work.MacroWORKCount);
    }
  }
}

// загрузка ВСЕХ макросов из реестра
int KeyMacro::LoadMacros(BOOL InitedRAM)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::LoadMacros()"));
  int Ret=FALSE;
  InitInternalVars(InitedRAM);

  #define TEMP_BUFFER_SIZE 32768
  char *Buffer=new char[TEMP_BUFFER_SIZE];

  if(Buffer)
  {
    int I;
    ReadVarsConst(MACRO_VARS,Buffer,TEMP_BUFFER_SIZE);
    for(I=MACRO_OTHER; I < MACRO_LAST; ++I)
      if(!ReadMacros(I,Buffer,TEMP_BUFFER_SIZE))
        break;
    delete[] Buffer;
    // выставим код возврата - если не все ВСЕ загрузились, то
    // будет FALSE
    Ret=(I == MACRO_LAST)?TRUE:FALSE;
    if(Ret)
      KeyMacro::Sort();
  }
//  Mode=MACRO_SHELL;
  return Ret;
}

// функция преобразования названия в код макроклавиши
// вернет -1, если нет эквивалента!
int WINAPI KeyNameMacroToKey(const char *Name)
{
  // пройдемся по всем модификаторам
  for(int I=0; I < sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0]); ++I)
    if(!memicmp(Name,KeyMacroCodes[I].Name,KeyMacroCodes[I].Len))
      return KeyMacroCodes[I].Key;
  return -1;
}

int KeyMacro::ProcessKey(int Key)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::ProcessKey()"));
  _KEYMACRO(SysLog("Param: Key=%s",_FARKEY_ToName(Key)));
  if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE || !FrameManager->GetCurrentFrame())
    return(FALSE);

  if (Recording) // Идет запись?
  {
    _KEYMACRO(SysLog("Recording..."));
    if (Key==Opt.KeyMacroCtrlDot || Key==Opt.KeyMacroCtrlShiftDot) // признак конца записи?
    {
      DWORD MacroKey;
      int WaitInMainLoop0=WaitInMainLoop;
      InternalInput=TRUE;
      WaitInMainLoop=FALSE;
      /* $ 23.11.2001 VVM
        ! Залочить _текущий_ фрейм, а не _последний немодальный_ */
      FrameManager->GetCurrentFrame()->Lock(); // отменим прорисовку фрейма
      MacroKey=AssignMacroKey();
      FrameManager->ResetLastInputRecord();
      FrameManager->GetCurrentFrame()->Unlock(); // теперь можно :-)
      /* VVM $ */

      // выставляем флаги по умолчанию.
      DWORD Flags=MFLAGS_DISABLEOUTPUT; // ???

      // добавим проверку на удаление
      // если удаляем, то не нужно выдавать диалог настройки.
      //if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
      if (MacroKey != (DWORD)-1 && Key==Opt.KeyMacroCtrlShiftDot && RecBufferSize)
      {
        if (!GetMacroSettings(MacroKey,Flags))
          MacroKey=(DWORD)-1;
      }
      WaitInMainLoop=WaitInMainLoop0;
      InternalInput=FALSE;

      if (MacroKey==(DWORD)-1)
      {
        if(RecBuffer)  xf_free(RecBuffer);
      }
      else
      {
        int Pos;
        for (Pos=0;Pos<MacroLIBCount;Pos++)
          if (MacroLIB[Pos].Key==MacroKey && (MacroLIB[Pos].Flags&MFLAGS_MODEMASK)==StartMode)
            break;

        // теперь смотрим Common
        if (Pos==MacroLIBCount)
          for (Pos=0;Pos<MacroLIBCount;Pos++)
            if (MacroLIB[Pos].Key==MacroKey && (MacroLIB[Pos].Flags&MFLAGS_MODEMASK)==MACRO_COMMON)
            {
              StartMode=MACRO_COMMON;
              break;
            }

        if (Pos==MacroLIBCount)
        {
          MacroLIB=(struct MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
          if (MacroLIB==NULL)
          {
            MacroLIBCount=0;
            WaitInFastFind++;
            return(FALSE);
          }
          MacroLIBCount++;
        }
        else
        {
          if(MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
            xf_free(MacroLIB[Pos].Buffer);
          if(MacroLIB[Pos].Src)
            xf_free(MacroLIB[Pos].Src);
        }
        MacroLIB[Pos].Key=MacroKey;
        if(RecBufferSize > 1)
          MacroLIB[Pos].Buffer=RecBuffer;
        else if(RecBuffer)
          MacroLIB[Pos].Buffer=reinterpret_cast<DWORD*>(*RecBuffer);
        MacroLIB[Pos].BufferSize=RecBufferSize;
        MacroLIB[Pos].Src=MkTextSequence(MacroLIB[Pos].Buffer,MacroLIB[Pos].BufferSize);
        MacroLIB[Pos].Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
      }

      Recording=MACROMODE_NOMACRO;
      RecBuffer=NULL;
      RecBufferSize=0;
      ScrBuf.RestoreMacroChar();
      WaitInFastFind++;
      KeyMacro::Sort();

      if (Opt.AutoSaveSetup)
        SaveMacros(FALSE); // записать только изменения!

      return(TRUE);
    }
    else // процесс записи продолжается.
    {
      if (Key>=KEY_NONE && Key<=KEY_END_SKEY) // специальные клавиши прокинем
        return(FALSE);

      RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+1));
      if (RecBuffer==NULL)
        return(FALSE);

      if(ReturnAltValue) // "подтасовка" фактов ;-)
        Key|=KEY_ALTDIGIT;

      RecBuffer[RecBufferSize++]=Key;
      return(FALSE);
    }
  }
  else if (Key==Opt.KeyMacroCtrlDot || Key==Opt.KeyMacroCtrlShiftDot) // Начало записи?
  {
    _KEYMACRO(SysLog("Begin Record..."));
    // Полиция 18
    if(Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
      return FALSE;
//    if(CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN))
//      return FALSE;

    if(LockScr) delete LockScr;
    LockScr=NULL;

    // Где мы?
    StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
    // тип записи - с вызовом диалога настроек или...
    // В зависимости от того, КАК НАЧАЛИ писать макрос, различаем общий режим (Ctrl-.
    // с передачей плагину кеев) или специальный (Ctrl-Shift-. - без передачи клавиш плагину)
    Recording=(Key==Opt.KeyMacroCtrlDot) ? MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

    if(RecBuffer)
      xf_free(RecBuffer);

    RecBuffer=NULL;
    RecBufferSize=0;
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    WaitInFastFind--;
    return(TRUE);
  }
  else
  {
    if (Work.Executing == MACROMODE_NOMACRO) // Это еще не режим исполнения?
    {
      _KEYMACRO(SysLog("Executing? Find Record..."));
      DWORD CurFlags;
      _KEYMACRO(SysLog("Start modify Key=%s",_FARKEY_ToName(Key)));
      if((Key&0x00FFFFFF) > 0x01 && (Key&0x00FFFFFF) < 0xFF)
      {
//        Key=LocalKeyToKey(Key&0x000000FF)|(Key&(~0x000000FF));
        Key=LocalUpper(Key&0x000000FF)|(Key&(~0x000000FF));
        if((Key&0x00FFFFFF) > 0x7F)
          Key=LocalKeyToKey(Key&0x000000FF)|(Key&(~0x000000FF));
      }
      _KEYMACRO(SysLog("Start GetIndex(Key=%s)",_FARKEY_ToName(Key)));
      int I=GetIndex(Key,(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);
      if(I != -1 && !((CurFlags=MacroLIB[I].Flags)&MFLAGS_DISABLEMACRO) && CtrlObject)
      {
        _KEYMACRO(SysLog("KeyMacro: %d (I=%d Key=%s,%s)",__LINE__,I,_FARKEY_ToName(Key),_FARKEY_ToName(MacroLIB[I].Key)));
        if(!CheckAll(Mode,CurFlags))
        {
          _KEYMACRO(SysLog("!CheckAll(Mode=%d,CurFlags=%08X); return FALSE",Mode,CurFlags));
          return FALSE;
        }

        // Скопируем текущее исполнение в MacroWORK
        //PostNewMacro(MacroLIB+I);
        // Подавлять вывод?
        if (CurFlags&MFLAGS_DISABLEOUTPUT)
        {
          if(LockScr) delete LockScr;
          LockScr=new LockScreen;
        }

        // различаем общий режим (с передачей плагину кеев) или специальный (без передачи клавиш плагину)
        Work.ExecLIBPos=0;
        PostNewMacro(MacroLIB+I);
        Work.MacroPC=I;

        IsRedrawEditor=CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE;

        _KEYMACRO(SysLog("**** Start Of Execute Macro ****"));
        _KEYMACRO(SysLog(1));
        doneMacroVarTable(0);
        initMacroVarTable(0);
        return(TRUE);
      }
      _KEYMACRO(SysLog("Macro not found"));
    }
    return(FALSE);
  }
}

char *KeyMacro::GetPlainText(char *Dest)
{
  if(!Work.MacroWORK)
    return NULL;

  struct MacroRecord *MR=Work.MacroWORK;

  int LenTextBuf=strlen((char*)&MR->Buffer[Work.ExecLIBPos]);
  Dest[0]=0;
  if(LenTextBuf && MR->Buffer[Work.ExecLIBPos])
  {
    strcpy(Dest,(char *)&MR->Buffer[Work.ExecLIBPos]);
    Work.ExecLIBPos+=(LenTextBuf+1)/sizeof(DWORD);
    if(((LenTextBuf+1)%sizeof(DWORD)) != 0)
      ++Work.ExecLIBPos;
    return Dest;
  }
  else
    Work.ExecLIBPos++;
  return NULL;
}

int KeyMacro::GetPlainTextSize()
{
  if(!Work.MacroWORK)
    return 0;
  struct MacroRecord *MR=Work.MacroWORK;
  return strlen((char*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(DWORD Flags,DWORD CheckCode)
{
  int I;
  TVar Cond(_i64(0));
  char FileName[NM*2];
  int FileAttr=-1;

  // Найдем индекс нужного кейворда
  for ( I=0 ; I < sizeof(MKeywords)/sizeof(MKeywords[0]) ; ++I )
    if ( MKeywords[I].Value == CheckCode )
      break;
  if ( I == sizeof(MKeywords)/sizeof(MKeywords[0]) )
    return Cond; // здесь TRUE обязательно, чтобы прекратить выполнение
                 // макроса, ибо код не распознан.

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

  // теперь сделаем необходимые проверки
  switch(MKeywords[I].Type)
  {
    case 0: // проверка на область
    {
      if(WaitInMainLoop) // здесь надо учесть тот самый WaitInMainLoop, хотя могу и ошибаться!!!
        Cond=(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == FrameManager->GetCurrentFrame()->GetMacroMode()?1:0;
      else
        Cond=(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == CtrlObject->Macro.GetMode()?1:0;
      break;
    }

    case 2:
    {
      Panel *PassivePanel=NULL;
      if(ActivePanel!=NULL)
        PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
      Frame* CurFrame=FrameManager->GetCurrentFrame();

      switch(CheckCode)
      {
        case MCODE_V_FAR_WIDTH:
          Cond=(__int64)(ScrX+1);
          break;

        case MCODE_V_FAR_HEIGHT:
          Cond=(__int64)(ScrY+1);
          break;

        case MCODE_V_FAR_TITLE:
          GetConsoleTitle(FileName,sizeof(FileName));
          Cond=FileName;
          break;

        case MCODE_C_DISABLEOUTPUT: // DisableOutput?
          Cond=Flags&CheckCode?1:0;
          break;

        case MCODE_C_WINDOWEDMODE: // Windowed?
          Cond=FarAltEnter(-2)==0?1:0;
          break;

        case MCODE_C_ICLIP:
          Cond=(__int64)UsedInternalClipboard;
          break;

        case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
          Cond=(__int64)Macro_DskShowPosType;
          break;

        case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
          Cond=(__int64)Opt.ChangeDriveMode;
          break;

        case MCODE_C_BOF:
        case MCODE_C_EOF:
        {
#if 0
          int CurMMode=CtrlObject->Macro.GetMode();
          if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS)
          {
            Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
//            f=f->GetTopModal();
            while(f)
            {
              fo=f;
              f=f->GetTopModal();
            }
            if(!f)
              f=fo;
            if(f)
              Cond=(__int64)f->ProcessKey(CheckCode);
          }
          else
            if(CurFrame)
              Cond=CurFrame->ProcessKey(CheckCode==MCODE_C_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=(__int64)f->ProcessKey(CheckCode);
#endif
          break;
        }

        case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - курсор в начале cmd-строки редактирования?
        case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - курсор в конеце cmd-строки редактирования?
        case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
        case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
        case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
        case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
        {
          Cond=(__int64)CtrlObject->CmdLine->ProcessKey(CheckCode);
          break;
        }

        case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
        {
          CtrlObject->CmdLine->GetString(FileName,sizeof(FileName)-1);
          Cond=FileName;
          break;
        }

        case MCODE_C_APANEL_ROOT:  // APanel.Root
        case MCODE_C_PPANEL_ROOT:  // PPanel.Root
        {
          Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->ProcessKey(MCODE_C_ROOTFOLDER)?1:0;
          break;
        }

        case MCODE_C_APANEL_BOF:
        case MCODE_C_PPANEL_BOF:
        case MCODE_C_APANEL_EOF:
        case MCODE_C_PPANEL_EOF:
        {
          Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->ProcessKey(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
          break;
        }

        case MCODE_C_SELECTED:    // Selected?
        {
#if 1
          int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MACRO_SHELL));
          if (CurFrame && CurFrame->GetType()==NeedType)
          {
            int CurSelected;
            if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
              CurSelected=CtrlObject->CmdLine->ProcessKey(MCODE_C_SELECTED);
            else
              CurSelected=CurFrame->ProcessKey(MCODE_C_SELECTED);
            Cond=CurSelected?1:0;
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=(__int64)f->ProcessKey(CheckCode);
#endif
          break;
        }

        case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
        case MCODE_V_DLGCURPOS:    // Dlg.CurPos
        case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
        {
          if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
          {
            Cond=(__int64)CurFrame->ProcessKey(CheckCode);
          }
          break;
        }

        case MCODE_C_EMPTY:   // Empty
          if(CurFrame->GetType() == MACRO_SHELL)
            Cond=CtrlObject->CmdLine->GetLength()==0?1:0;
          else
          {
#if 0
            Cond=(__int64)CurFrame->ProcessKey(MCODE_C_EMPTY);
#else
            Frame *f=FrameManager->GetTopModal();
            if(f)
              Cond=(__int64)f->ProcessKey(CheckCode);
#endif
          }
          break;

        case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
        case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond = SelPanel->IsVisible()?1:0;
          break;
        }

        case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
        case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            SelPanel->GetFileName(FileName,SelPanel->GetCurrentPos(),FileAttr);
            int GetFileCount=SelPanel->GetFileCount();
            Cond=GetFileCount == 0 ||
                 GetFileCount == 1 && TestParentFolderName(FileName)
                 ?1:0;
          }
          break;
        }

        case MCODE_C_APANEL_LFN:
        case MCODE_C_PPANEL_LFN:
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = SelPanel->GetShowShortNamesMode()?0:1;
          break;
        }

        case MCODE_C_APANEL_LEFT: // APanel.Left
        case MCODE_C_PPANEL_LEFT: // PPanel.Left
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = SelPanel == CtrlObject->Cp()->LeftPanel?1:0;
          break;
        }

        case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
        case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=SelPanel->GetType() == FILE_PANEL?1:0;
          break;
        }

        case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
        case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->GetMode() == PLUGIN_PANEL?1:0;
          break;
        }

        case MCODE_C_APANEL_FOLDER: // APanel.Folder
        case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            SelPanel->GetFileName(FileName,SelPanel->GetCurrentPos(),FileAttr);
            if(FileAttr != -1)
              Cond=(FileAttr&FA_DIREC)?1:0;
          }
          break;
        }

        case MCODE_C_APANEL_SELECTED: // APanel.Selected
        case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            int SelCount=SelPanel->GetRealSelCount();
            Cond=SelCount >= 1?1:0; //??
          }
          break;
        }

        //- AN -----------------------------------------------------
        //  Пара новых псевдопеременных не логического типа на пробу
        //- AN -----------------------------------------------------
        case MCODE_V_APANEL_CURRENT: // APanel.Current
        case MCODE_V_PPANEL_CURRENT: // PPanel.Current
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            SelPanel->GetFileName(FileName,SelPanel->GetCurrentPos(),FileAttr);
            if ( FileAttr != -1 )
              Cond = FileName;
          }
          break;
        }

        case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
        case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = (__int64)SelPanel->GetRealSelCount();
          break;
        }

        case MCODE_V_APANEL_WIDTH: // APanel.Width
        case MCODE_V_PPANEL_WIDTH: // PPanel.Width
        case MCODE_V_APANEL_HEIGHT: // APanel.Height
        case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            int X1, Y1, X2, Y2;
            SelPanel->GetPosition(X1,Y1,X2,Y2);
            if(CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)
              Cond = (__int64)(Y2-Y1+1);
            else
              Cond = (__int64)(X2-X1+1);
          }
          break;
        }

        case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
        case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_OPIFLAGS ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            struct OpenPluginInfo Info;
            SelPanel->GetOpenPluginInfo(&Info);
            Cond = (__int64)Info.Flags;
          }
          break;
        }


        case MCODE_V_APANEL_PATH: // APanel.Path
        case MCODE_V_PPANEL_PATH: // PPanel.Path
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            SelPanel->GetCurDir(FileName);
            DeleteEndSlash(FileName); // - чтобы у корня диска было C:, тогда можно писать так: APanel.Path + "\\file"
            Cond = FileName;
          }
          break;
        }

        case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
        case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
        {
          if(_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,FileName,sizeof(FileName)-1,""))
          {
            UnquoteExternal(FileName);
            DeleteEndSlash(FileName);
            Cond = FileName;
          }
          break;
        }

        //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
        case MCODE_V_APANEL_TYPE: // APanel.Type
        case MCODE_V_PPANEL_TYPE: // PPanel.Type
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=(__int64)SelPanel->GetType();
          break;
        }

        case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - активная панель: тип привода
        case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - пассивная панель: тип привода
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
          Cond=_i64(-1);
          if ( SelPanel != NULL && SelPanel->GetMode() != PLUGIN_PANEL)
          {
            SelPanel->GetCurDir(FileName);
            GetPathRoot(FileName,FileName);
            UINT DriveType=FAR_GetDriveType(FileName,NULL,0);
            if(IsLocalPath(FileName))
            {
              FileName[2]=0;
              if(GetSubstName(DriveType,FileName,NULL,0))
                DriveType=DRIVE_SUBSTITUTE;
            }
            Cond=TVar((__int64)DriveType);
          }

          break;
        }

        case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
        case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=(__int64)SelPanel->GetFileCount();
          break;
        }

        case MCODE_V_APANEL_CURPOS: // APanel.CurPos
        case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0);
          break;
        }

        case MCODE_V_TITLE: // Title
        {
          FileName[0]=0;
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
            if(CtrlObject->Cp() == f)
            {
              ActivePanel->GetTitle(FileName,sizeof(FileName)-1);
            }
            else
            {
              switch(f->GetTypeAndName(NULL,FileName))
              {
                case MODALTYPE_EDITOR:
                case MODALTYPE_VIEWER:
                  f->GetTitle(FileName,sizeof(FileName)-1,0);
                  break;
              }
            }
          }
          RemoveExternalSpaces(FileName);
          Cond=FileName;
          break;
        }

        case MCODE_V_HEIGHT:  // Height - высота текущего объекта
        case MCODE_V_WIDTH:   // Width - ширина текущего объекта
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
            int X1, Y1, X2, Y2;
            f->GetPosition(X1,Y1,X2,Y2);
            if(CheckCode == MCODE_V_HEIGHT)
              Cond = (__int64)(Y2-Y1+1);
            else
              Cond = (__int64)(X2-X1+1);
          }
        }

        case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
        case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
            if(f->GetType() == MODALTYPE_VIEWER)
            {
              if(CheckCode == MCODE_V_ITEMCOUNT)
                Cond=(__int64)((FileViewer*)f)->GetViewFileSize();
              else if(CheckCode == MCODE_V_CURPOS)
                Cond=(__int64)((FileViewer*)f)->GetViewFilePos()+1;
            }
            else
              Cond=(__int64)f->ProcessKey(CheckCode);
          }
        }
        // *****************

        case MCODE_V_EDITORCURLINE: // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
        case MCODE_V_EDITORSTATE:   // Editor.State
        case MCODE_V_EDITORLINES:   // Editor.Lines
        case MCODE_V_EDITORCURPOS:  // Editor.CurPos
        case MCODE_V_EDITORFILENAME: // Editor.FileName
        case MCODE_V_EDITORVALUE:   // Editor.Value
        {
          if(CtrlObject->Macro.GetMode()==MACRO_EDITOR &&
             CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
          {
            if(CheckCode == MCODE_V_EDITORFILENAME)
            {
              CtrlObject->Plugins.CurEditor->GetTypeAndName(NULL,FileName);
              Cond=FileName;
            }
            else if(CheckCode == MCODE_V_EDITORVALUE)
            {
              struct EditorGetString egs;
              egs.StringNumber=-1;
              CtrlObject->Plugins.CurEditor->EditorControl(ECTL_GETSTRING,&egs);
              Cond=(char *)egs.StringText;
            }
            else
              Cond=(__int64)CtrlObject->Plugins.CurEditor->ProcessKey(CheckCode);
          }
          break;
        }

        case MCODE_V_VIEWERFILENAME: // Viewer.FileName
        case MCODE_V_VIEWERSTATE: // Viewer.State
        {
          if((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
             CtrlObject->Plugins.CurViewer && CtrlObject->Plugins.CurViewer->IsVisible())
          {
            if(CheckCode == MCODE_V_VIEWERFILENAME)
            {
              CtrlObject->Plugins.CurViewer->GetFileName(FileName);//GetTypeAndName(NULL,FileName);
              Cond=FileName;
            }
            else
              Cond=(__int64)CtrlObject->Plugins.CurViewer->ProcessKey(MCODE_V_VIEWERSTATE);
          }
          break;
        }

      }
    }
  }

  return Cond;
}

// S=substr(S,N1,N2)
static TVar substrFunc(TVar *param)
{
  char *p = (char*)(param[0].toString());
  int len = strlen(p);
  int p1 = (int)(param[1].toInteger());
  if ( ( p1 >= 0 ) && ( p1 < len ) )
  {
    p += p1;
    len = strlen(p);
    int p2 = (int)(param[2].toInteger());
    if ( ( p2 > 0 ) && ( p2 < len ) )
      p[p2] = 0;
    return TVar(p);
  }
  return TVar("");
}

#define FLAG_DISK   1
#define FLAG_PATH   2
#define FLAG_NAME   4
#define FLAG_EXT    8

static BOOL SplitFileName (const char *lpFullName,char *lpDest,int nFlags)
{
  char *s = (char*)lpFullName; //start of sub-string
  char *p = s; //current string pointer

  char *es = s+strlen(s); //end of string
  char *e; //end of sub-string

  if ( !*p )
      return FALSE;

  if ( (*p == '\\') && (*(p+1) == '\\') ) //share
  {
    p += 2;

    p = strchr(p, '\\');

    if ( !p )
      return FALSE; //invalid share (\\server\)

    p = strchr (p+1, '\\');

    if ( !p )
      p = es;

    if ( (nFlags & FLAG_DISK) == FLAG_DISK )
      strncpy (lpDest, s, p-s);
  }
  else
  {
    if ( *(p+1) == ':' )
    {
      if ( !isalpha (*p) )
        return FALSE; // 1:\ is not a valid disk

      p += 2;

      if ( (nFlags & FLAG_DISK) == FLAG_DISK )
        strncat (lpDest, s, p-s);
    }
  }

  e = NULL;
  s = p;

  while ( p )
  {
    p = strchr (p, '\\');

    if ( p )
    {
      e = p;
      p++;
    }
  };

  if ( e )
  {
    if ( (nFlags & FLAG_PATH) )
      strncat (lpDest, s, e-s);

    s = e+1;
    p = s;
  }

  if ( !p )
    p = s;

  e = NULL;

  while ( p )
  {
    p = strchr (p+1, '.');

    if ( p )
       e = p;
  }

  if ( !e )
      e = es;

  //FSF.AddEndSlash replacement

  if ( *lpDest && (lpDest[strlen(lpDest)] != '\\') ) //hack, just in case of "disk+name" etc.
      strcat (lpDest, "\\");

  if ( nFlags & FLAG_NAME )
  {
    char *ptr=strpbrk(s,":");
    if(ptr)
      s=ptr+1;
    strncat (lpDest, s, e-s);
  }

  if ( nFlags & FLAG_EXT )
      strcat (lpDest, e);

  return TRUE;
}


// S=fsplit(S,N)
static TVar fsplitFunc(TVar *param)
{
  char path[NM*2];
  const char *s = param[0].toString();
  int         m = (int)param[1].toInteger();
  *path=0;
  if(!SplitFileName(s,path,m))
    *path=0;
  return TVar(path);
}

#if 0
// S=Meta("!.!") - в макросах юзаем ФАРовы метасимволы
static TVar metaFunc(TVar *param)
{
  const char *s = param[0].toString();

  if(s && *s)
  {
    char SubstText[512];
    char Name[NM],ShortName[NM];
    xstrncpy(SubstText,s,sizeof(SubstText)-1);
    SubstFileName(SubstText,sizeof (SubstText),Name,ShortName,NULL,NULL,TRUE);
    return TVar(SubstText);
  }
  return TVar("");
}
#endif


// N=index(S1,S2)
static TVar indexFunc(TVar *param)
{
  const char *s = param[0].toString();
  const char *p = param[1].toString();
  //const char *i = strstr(s, p);
  const char *i = LocalStrstri(s,p);
  return TVar((__int64)(i ? i-s : -1));
}

// S=itoa(N,radix)
static TVar itoaFunc(TVar *param)
{
  char value[NM];
  if(param[0].isInteger())
    return TVar(_i64toa((int)param[0].toInteger(),value,(int)param[1].toInteger()));
  return param[0];
}

// N=sleep(N)
static TVar sleepFunc(TVar *param)
{
  long Period=(long)param[0].toInteger();
  if(Period > 0)
  {
    Sleep((DWORD)Period);
    return TVar(_i64(1));
  }
  return TVar(_i64(0));
}

// S=rindex(S1,S2)
static TVar rindexFunc(TVar *param)
{
  const char *s = param[0].toString();
  const char *p = param[1].toString();
  const char *i = LocalRevStrstri(s,p);
  return TVar((__int64)(i ? i-s : -1));
}

// S=date(S)
static TVar dateFunc(TVar *param)
{
  const char *s = param[0].toString();
  int SizeMacroText = 16+(*s ? 0 : strlen(Opt.DateFormat));
  SizeMacroText*=4;
  char *TStr=(char*)alloca(SizeMacroText);
  if(TStr && MkStrFTime(TStr,SizeMacroText,s))
    return TVar(TStr);
  return TVar("");
}

// S=xlat(S)
static TVar xlatFunc(TVar *param)
{
  char *Str = (char *)param[0].toString();
  return TVar(::Xlat(Str,0,strlen(Str),NULL,Opt.XLat.Flags));
}

// N=msgbox("Title","Text",flags)
static TVar msgBoxFunc(TVar *param)
{
  DWORD Flags = (DWORD)param[2].toInteger();
  Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
  Flags|=FMSG_ALLINONE;
  if(HIWORD(Flags) == 0 || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
    Flags|=FMSG_MB_OK;

  const char *title=NullToEmpty(param[0].toString());
  const char *text=NullToEmpty(param[1].toString());
  char *TempBuf=(char *)alloca(strlen(title)+strlen(text)+16);
  int Result=0;
  if(TempBuf)
  {
    strcpy(TempBuf,title);
    strcat(TempBuf,"\n");
    strcat(TempBuf,text);
    Result=FarMessageFn(-1,Flags,NULL,(const char * const *)TempBuf,0,0)+1;
  }
  return TVar((__int64)Result);
}


// S=env(S)
static TVar environFunc(TVar *param)
{
  char Env[2048];
  if(GetEnvironmentVariable(param->toString(),Env,sizeof(Env)))
  {
    FAR_CharToOem(Env,Env);
    return TVar(Env);
  }
  return TVar("");
}

static TVar _fattrFunc(int Type,TVar *param)
{
  if(Type == 0) // не панели
  {
    WIN32_FIND_DATA FindData;
    UINT  PrevErrMode;
    char *Str = (char *)param[0].toString();
    // дабы не выскакивал гуевый диалог, если диск эжектед.
    PrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    GetFileWin32FindData(Str,&FindData);
    SetErrorMode(PrevErrMode);
    return TVar((__int64)(long)FindData.dwFileAttributes);
  }
  else
  {
    int typePanel=(int)param[0].toInteger();
    char *Str = (char *)param[1].toString();
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    Panel *PassivePanel=NULL;
    if(ActivePanel!=NULL)
      PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
    //Frame* CurFrame=FrameManager->GetCurrentFrame();

    Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
    if(SelPanel)
    {
      long Pos=-1;
      if(strpbrk(Str,"*?")!=NULL)
        Pos=SelPanel->FindFirst(Str);
      else
        Pos=SelPanel->FindFile(Str,strpbrk(Str,"\\/:")?FALSE:TRUE);
      if(Pos >= 0)
      {
        int FileAttr;
        SelPanel->GetFileName(NULL,Pos,FileAttr);
        return TVar((__int64)(long)FileAttr);
      }
    }
  }

  return TVar((__int64)-1);
}

// N=fattr(S)
static TVar fattrFunc(TVar *param)
{
  return _fattrFunc(0,param);
}

// N=fexist(S)
static TVar fexistFunc(TVar *param)
{
  TVar attr=_fattrFunc(0,param);
  return TVar(attr.toInteger() != -1 ? 1 : 0);
}

// N=panel.fattr(S)
static TVar panelfattrFunc(TVar *param)
{
  return _fattrFunc(1,param);
}

// N=panel.fexist(S)
static TVar panelfexistFunc(TVar *param)
{
  TVar attr=_fattrFunc(1,param);
  return TVar(attr.toInteger() != -1 ? 1 : 0);
}


// V=Dlg.GetValue(ID,N)
static TVar dlggetvalueFunc(TVar *param)
{
  TVar Ret(-1);

  Frame* CurFrame=FrameManager->GetCurrentFrame();
  if(CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
  {
    int Index=(int)param[0].toInteger()-1;
    int TypeInf=(int)param[1].toInteger();
    int DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
    const struct DialogItem *DlgItem=((Dialog*)CurFrame)->GetAllItem();
    if(Index == -1)
    {
      SMALL_RECT Rect;
      if(((Dialog*)CurFrame)->SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,(LONG_PTR)&Rect))
      {
        switch(TypeInf)
        {
          case 0: Ret=(__int64)DlgItemCount; break;
          case 2: Ret=(__int64)Rect.Left; break;
          case 3: Ret=(__int64)Rect.Top; break;
          case 4: Ret=(__int64)Rect.Right; break;
          case 5: Ret=(__int64)Rect.Bottom; break;
          case 6: Ret=(__int64)((Dialog*)CurFrame)->GetDlgFocusPos()+1; break;
        }
      }
    }
    else if(Index < DlgItemCount && DlgItem)
    {
      const struct DialogItem *Item=DlgItem+Index;
      int ItemType=Item->Type;
      DWORD ItemFlags=Item->Flags;

      if(TypeInf == 0)
      {
        if(ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
          TypeInf=7;
        else if(ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
        {
          struct FarListGetItem ListItem;
          ListItem.ItemIndex=Item->ListPtr->GetSelectPos();
          if(((Dialog*)CurFrame)->SendDlgMessage((HANDLE)CurFrame,DM_LISTGETITEM,0,(LONG_PTR)&ListItem))
          {
            Ret=(char *)ListItem.Item.Text;
          }
          else
            Ret="";
          TypeInf=-1;
        }
        else
          TypeInf=10;
      }

      switch(TypeInf)
      {
        case 1: Ret=(__int64)ItemType;    break;
        case 2: Ret=(__int64)Item->X1;    break;
        case 3: Ret=(__int64)Item->Y1;    break;
        case 4: Ret=(__int64)Item->X2;    break;
        case 5: Ret=(__int64)Item->Y2;    break;
        case 6: Ret=(__int64)Item->Focus; break;
        case 7:
        {
          if(ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
            Ret=(__int64)Item->Selected;
          else if(ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
          {
            Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
          }
          else
          {
            Ret=_i64(0);
/*
    int Item->Selected;
    const char *Item->History;
    const char *Item->Mask;
    struct FarList *Item->ListItems;
    int  Item->ListPos;
    CHAR_INFO *Item->VBuf;
*/
          }
          break;
        }
        case 8: Ret=(__int64)ItemFlags; break;
        case 9: Ret=(__int64)Item->DefaultButton; break;
        case 10:
        {
          if((ItemType == DI_COMBOBOX || ItemType == DI_EDIT) && (ItemFlags&DIF_VAREDIT))
          {
/*
      DWORD Item->Ptr.PtrFlags;
      int   Item->Ptr.PtrLength;
      char *Item->Ptr.PtrData;
      char  Item->Ptr.PtrTail[1];
*/
            Ret=(char *)Item->Ptr.PtrData;
          }
          else
            Ret=(char *)Item->Data;
          break;
        }
      }
    }
  }

  return TVar(Ret);
}

// OldVar=Editor.Set(Idx,Var)
static TVar editorsetFunc(TVar *param)
{
  TVar Ret(-1);

  if(CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
  {
    int Index=(int)param[0].toInteger();
    long longState=-1L;

    if(Index != 12)
      longState=(long)param[1].toInteger();

    struct EditorOptions EdOpt;
    CtrlObject->Plugins.CurEditor->GetEditorOptions(EdOpt);

    switch(Index)
    {
      case 0:  // TabSize;
        Ret=(__int64)EdOpt.TabSize; break;
      case 1:  // ExpandTabs;
        Ret=(__int64)EdOpt.ExpandTabs; break;
      case 2:  // PersistentBlocks;
        Ret=(__int64)EdOpt.PersistentBlocks; break;
      case 3:  // DelRemovesBlocks;
        Ret=(__int64)EdOpt.DelRemovesBlocks; break;
      case 4:  // AutoIndent;
        Ret=(__int64)EdOpt.AutoIndent; break;
      case 5:  // AutoDetectTable;
        Ret=(__int64)EdOpt.AutoDetectTable; break;
      case 6:  // AnsiTableForNewFile;
        Ret=(__int64)EdOpt.AnsiTableForNewFile; break;
      case 7:  // CursorBeyondEOL;
        Ret=(__int64)EdOpt.CursorBeyondEOL; break;
      case 8:  // BSLikeDel;
        Ret=(__int64)EdOpt.BSLikeDel; break;
      case 9:  // CharCodeBase;
        Ret=(__int64)EdOpt.CharCodeBase; break;
      case 10: // SavePos;
        Ret=(__int64)EdOpt.SavePos; break;
      case 11: // SaveShortPos;
        Ret=(__int64)EdOpt.SaveShortPos; break;
      case 12: // char WordDiv[256];
        Ret=TVar(EdOpt.WordDiv); break;
      case 13: // F7Rules;
        Ret=(__int64)EdOpt.F7Rules; break;
      case 14: // AllowEmptySpaceAfterEof;
        Ret=(__int64)EdOpt.AllowEmptySpaceAfterEof; break;
      default:
        Ret=-1;
    }

    if(Index != 12 && longState != -1 || Index == 12 && param[1].i() == -1)
    {
      switch(Index)
      {
        case 0:  // TabSize;
          EdOpt.TabSize=longState; break;
        case 1:  // ExpandTabs;
          EdOpt.ExpandTabs=longState; break;
        case 2:  // PersistentBlocks;
          EdOpt.PersistentBlocks=longState; break;
        case 3:  // DelRemovesBlocks;
          EdOpt.DelRemovesBlocks=longState; break;
        case 4:  // AutoIndent;
          EdOpt.AutoIndent=longState; break;
        case 5:  // AutoDetectTable;
          EdOpt.AutoDetectTable=longState; break;
        case 6:  // AnsiTableForNewFile;
          EdOpt.AnsiTableForNewFile=longState; break;
        case 7:  // CursorBeyondEOL;
          EdOpt.CursorBeyondEOL=longState; break;
        case 8:  // BSLikeDel;
          EdOpt.BSLikeDel=longState; break;
        case 9:  // CharCodeBase;
          EdOpt.CharCodeBase=longState; break;
        case 10: // SavePos;
          EdOpt.SavePos=longState; break;
        case 11: // SaveShortPos;
          EdOpt.SaveShortPos=longState; break;
        case 12: // char WordDiv[256];
          xstrncpy(EdOpt.WordDiv,param[1].toString(),sizeof(EdOpt.WordDiv)-1); break;
        case 13: // F7Rules;
          EdOpt.F7Rules=longState; break;
        case 14: // AllowEmptySpaceAfterEof;
          EdOpt.AllowEmptySpaceAfterEof=longState; break;
        default:
          Ret=-1L;
          break;
      }
      CtrlObject->Plugins.CurEditor->SetEditorOptions(EdOpt);
      CtrlObject->Plugins.CurEditor->ShowStatus();
    }

  }

  return TVar(Ret);
}

// b=msave(var)
static TVar msaveFunc(TVar *param)
{
  static int errVar;

  TVarTable *t = &glbVarTable;
  const char *Name=param[0].s();
  if(!Name || *Name!= '%')
    return TVar(_i64(0));

  TVar Result=varLook(*t, Name+1, errVar)->value;
  if(errVar)
    return TVar(_i64(0));

  DWORD Ret=(DWORD)-1;
  char ValueName[129];
  xstrncpy(ValueName,param[0].s(),sizeof(ValueName)-2);
  switch(Result.type())
  {
    case vtInteger:
    {
      __int64 rrr=Result.toInteger();
      Ret=SetRegKey64("KeyMacros\\Vars",ValueName,rrr);
      break;
    }
    case vtString:
    {
      Ret=SetRegKey("KeyMacros\\Vars",ValueName,Result.toString());
      break;
    }
  }
  return TVar(Ret==ERROR_SUCCESS?1:0);
}

// V=Clip(N,S)
static TVar clipFunc(TVar *param)
{
  int cmdType=(int)param[0].toInteger();

  switch(cmdType)
  {
    case 0: // Get from Clipboard, "S" - ignore
    {
      char *ClipText=InternalPasteFromClipboard(0); // 0!  ???
      if(ClipText)
      {
        TVar varClip(ClipText);
        delete [] ClipText;
        return varClip;
      }
      break;
    }
    case 1: // Put "S" into Clipboard
      return TVar((__int64)InternalCopyToClipboard(param[1].s(),0)); // 0!  ???
    case 2: // Add "S" into Clipboard
    {
      TVar varClip(param[1].s());
      char *CopyData=InternalPasteFromClipboard(0); // 0!  ???
      if(CopyData)
      {
        int DataSize=strlen(CopyData);
        char *NewPtr=(char *)xf_realloc(CopyData,DataSize+strlen(param[1].s())+2);
        if (NewPtr)
        {
          CopyData=NewPtr;
          strcpy(CopyData+DataSize,param[1].s());
          varClip=CopyData;
          delete CopyData;
        }
        else
          delete CopyData;
      }
      return TVar((__int64)InternalCopyToClipboard(varClip.s(),0)); // 0!  ???
    }
    case 3: // Copy Win to internal, "S" - ignore
    case 4: // Copy internal to Win, "S" - ignore
    {
      int _UsedInternalClipboard=UsedInternalClipboard;
      int Ret=0;

      {
        TVar varClip("");
        UsedInternalClipboard=cmdType-3;
        char *ClipText=InternalPasteFromClipboard(0); // 0!  ???
        if(ClipText)
        {
          varClip=ClipText;
          delete [] ClipText;
        }
        UsedInternalClipboard=UsedInternalClipboard==0?1:0;
        Ret=InternalCopyToClipboard(varClip.s(),0); // 0!  ???
      }

      UsedInternalClipboard=_UsedInternalClipboard;
      return TVar((__int64)Ret);
    }
      break;
  }
  return TVar(_i64(0));
}

// N=Panel.SetPos(panelType,fileName)
static TVar panelsetposFunc(TVar *param)
{
  int typePanel=(int)param[0].toInteger();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  if(!SelPanel)
    return TVar(_i64(0));

  __int64 Ret=0;
  int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
  if(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
  {
    const char *fileName=param[1].s();
    if(SelPanel->GoToFile(fileName))
    {
      SelPanel->Show();
      Ret=(__int64)(SelPanel->GetCurrentPos()+1);
    }
  }
  return TVar(Ret);
}

// V=PanelItem(typePanel,Index,TypeInfo)
static TVar panelitemFunc(TVar *param)
{
  int typePanel=(int)param[0].toInteger();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  if(!SelPanel)
    return TVar(_i64(0));

  int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
  if(!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
    return TVar(_i64(0));

  int Index=(int)(param[1].toInteger())-1;
  int TypeInfo=(int)param[2].toInteger();

  struct FileListItem filelistItem;
  if(TypePanel == TREE_PANEL)
  {
    struct TreeItem treeItem;
    if(SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
      return TVar(treeItem.Name);
    return TVar(_i64(0));
  }
  else
  {
    char Date[128], Time[64];

    if(!SelPanel->GetItem(Index,&filelistItem))
      TypeInfo=-1;
    switch(TypeInfo)
    {
      case 0:  // Name
        return TVar((const char*)filelistItem.Name);
      case 1:  // ShortName
        return TVar((const char*)filelistItem.ShortName);
      case 2:  // FileAttr
        return TVar((__int64)(long)filelistItem.FileAttr);
      case 3:  // CreationTime
        ConvertDate(filelistItem.CreationTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        return TVar((const char*)Date);
      case 4:  // AccessTime
        ConvertDate(filelistItem.AccessTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        return TVar((const char*)Date);
      case 5:  // WriteTime
        ConvertDate(filelistItem.WriteTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        return TVar((const char*)Date);
      case 6:  // UnpSize
      {
        __int64 UnpSize=MKUINT64(filelistItem.UnpSizeHigh,filelistItem.UnpSize);
        return TVar(UnpSize);
      }
      case 7:  // PackSize
      {
        __int64 PackSize=MKUINT64(filelistItem.PackSizeHigh,filelistItem.PackSize);
        return TVar(PackSize);
      }
      case 8:  // Selected
        return TVar((__int64)((DWORD)filelistItem.Selected));
      case 9:  // NumberOfLinks
        return TVar((__int64)filelistItem.NumberOfLinks);
      case 10:  // SortGroup
        return TVar((__int64)filelistItem.SortGroup);
      case 11:  // DizText
      {
        const char *LPtr=filelistItem.DizText?filelistItem.DizText:"";
        return TVar(LPtr);
      }
      case 12:  // Owner
        return TVar((const char*)filelistItem.Owner);
      case 13:  // CRC32
        return TVar((__int64)filelistItem.CRC32);
      case 14:  // Position
        return TVar((__int64)filelistItem.Position);

    }
  }

  return TVar(_i64(0));
}

static int ePos;

const char *eStackAsString(int Pos)
{
  const char *s=eStack[Pos?ePos:0].toString();
  return !s?"":s;
}

int KeyMacro::GetKey()
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::GetKey()"));
  _KEYMACRO(SysLog("InternalInput=%d Executing=%d (CurrentFrame=%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
  struct MacroRecord *MR;
  int RetKey=0;
  if (InternalInput || !FrameManager->GetCurrentFrame())
    return(FALSE);

  if(Work.Executing == MACROMODE_NOMACRO)
  {
    if(!Work.MacroWORK)
    {
      if(CurPCStack >= 0)
      {
        PopState();
        return RetKey;
      }
      if(Mode==MACRO_EDITOR &&
         IsRedrawEditor &&
         CtrlObject->Plugins.CurEditor &&
         CtrlObject->Plugins.CurEditor->IsVisible() &&
         LockScr)
      {
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
        CtrlObject->Plugins.CurEditor->Show();
      }
      if(CurPCStack < 0)
      {
        if(LockScr) delete LockScr;
        LockScr=NULL;
      }
      if(TitleModified) SetFarTitle(NULL);
      UsedInternalClipboard=0; //??
      return RetKey;
    }
/*
    else if(Work.ExecLIBPos>=MR->BufferSize)
    {
      ReleaseWORKBuffer();
      Work.Executing=MACROMODE_NOMACRO;
      return(FALSE);
    }
    else
*/
    //if(Work.MacroWORK)
    {
      Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
      Work.ExecLIBPos=0; //?????????????????????????????????
    }
    //else
    //  return FALSE;
  }

initial:
  if((MR=Work.MacroWORK) == NULL)
    return FALSE;
_KEYMACRO(SysLog("KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

  // ВНИМАНИЕ! Возможны глюки!
  if(!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
    LockScr=new LockScreen;

begin:
  if (Work.ExecLIBPos>=MR->BufferSize || MR->Buffer==NULL)
  {
done:
    /*$ 10.08.2000 skv
      If we are in editor mode, and CurEditor defined,
      we need to call this events.
      EE_REDRAW 2 - to notify that text changed.
      EE_REDRAW 0 - to notify that whole screen updated
      ->Show() to actually update screen.

      This duplication take place since ShowEditor method
      will NOT send this event while screen is locked.
    */
    if(Mode==MACRO_EDITOR &&
       IsRedrawEditor &&
       CtrlObject->Plugins.CurEditor &&
       CtrlObject->Plugins.CurEditor->IsVisible() &&
       LockScr)
    {
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
      CtrlObject->Plugins.CurEditor->Show();
    }
    /* skv$*/
    if(CurPCStack < 0)
    {
      if(LockScr) delete LockScr;
      LockScr=NULL;
    }
    UsedInternalClipboard=0; //??
    Work.Executing=MACROMODE_NOMACRO;
    ReleaseWORKBuffer();
    // проверим - "а есть ли в временном стеке еще макрЫсы"?
    if(Work.MacroWORKCount > 0)
    {
      // нашлось, запустим механизму по новой
      Work.ExecLIBPos=0;
    }
    if(TitleModified) SetFarTitle(NULL); // выставим нужный заголовок по завершению макроса
    //FrameManager->RefreshFrame();
    //FrameManager->PluginCommit();
    _KEYMACRO(SysLog(-1); SysLog("**** End Of Execute Macro ****"));
    if(CurPCStack >= 0)
    {
      PopState();
      goto initial;
    }
    return RetKey;
  }

  if(Work.ExecLIBPos==0)
    Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;

  DWORD Key=GetOpCode(MR,Work.ExecLIBPos++);

  _KEYMACRO(char KeyText[50]; ::KeyToText(Key,KeyText); SysLog("%s",KeyText));

  if(Key&KEY_ALTDIGIT) // "подтасовка" фактов ;-)
  {
    Key&=~KEY_ALTDIGIT;
    ReturnAltValue=1;
  }

  static int errVar;
  char value[2048];

  switch(Key)
  {
    case MCODE_OP_EXIT:
      goto done;

    case MCODE_OP_AKEY: //$AKey
    {
      return MR->Key;
    }

    /* $IClip
       0: MCODE_OP_ICLIP
    */
    case MCODE_OP_ICLIP:
    {
      UsedInternalClipboard=UsedInternalClipboard==0?1:0;
      goto begin;
    }

    case MCODE_OP_SWITCHKBD:
    {
      if(hFarWnd)
      {
        PostMessage(hFarWnd,WM_INPUTLANGCHANGEREQUEST, 1, HKL_NEXT);
        //if(Flags & XLAT_SWITCHKEYBBEEP)
        //  MessageBeep(0);
      }
      goto begin;
    }

    // переходы
    case MCODE_OP_JMP:
      Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      goto begin;
    case MCODE_OP_JZ:
      if ( eStack->toInteger() == 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;
    case MCODE_OP_JNZ:
      if ( eStack->toInteger() != 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;
    case MCODE_OP_LT:
      if ( eStack->toInteger() < 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;
    case MCODE_OP_LE:
      if ( eStack->toInteger() <= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;
    case MCODE_OP_GT:
      if ( eStack->toInteger() > 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;
    case MCODE_OP_GE:
      if ( eStack->toInteger() >= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    // вычислить выражение
    case MCODE_OP_EXPR:
      _KEYMACRO(SysLog("  --- expr %d", Work.ExecLIBPos));
      ePos = 0;
      while ( ( Key=GetOpCode(MR,Work.ExecLIBPos++) ) != MCODE_OP_DOIT && Work.ExecLIBPos < MR->BufferSize )
      {
        switch ( Key )
        {
          case MCODE_OP_PUSHINT:  // Положить целое значение на стек.
          {
            FARINT64 i64;
            i64.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos++);   //???
            i64.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos++);    //???
            ++ePos;
            eStack[ePos] = i64.i64;
            break;
          }
          case MCODE_OP_PUSHVAR:  // Положить на стек переменную.
          {
            GetPlainText(value);
            TVarTable *t = ( *value == '%' ) ? &glbVarTable : &locVarTable;
            // %%name - глобальная переменная
            ++ePos;
            eStack[ePos] = varLook(*t, value, errVar)->value;
            break;
          }
          case MCODE_OP_PUSHSTR: // Положить на стек строку-константу.
            GetPlainText(value);
            ++ePos;
            eStack[ePos] = TVar(value);
            break;

          // операции
          case MCODE_OP_NEGATE: eStack[ePos] = -eStack[ePos]; break;
          case MCODE_OP_NOT:    eStack[ePos] = !eStack[ePos]; break;

          case MCODE_OP_LT:     ePos--; eStack[ePos] = eStack[ePos] <  eStack[ePos+1]; break;
          case MCODE_OP_LE:     ePos--; eStack[ePos] = eStack[ePos] <= eStack[ePos+1]; break;
          case MCODE_OP_GT:     ePos--; eStack[ePos] = eStack[ePos] >  eStack[ePos+1]; break;
          case MCODE_OP_GE:     ePos--; eStack[ePos] = eStack[ePos] >= eStack[ePos+1]; break;
          case MCODE_OP_EQ:     ePos--; eStack[ePos] = eStack[ePos] == eStack[ePos+1]; break;
          case MCODE_OP_NE:     ePos--; eStack[ePos] = eStack[ePos] != eStack[ePos+1]; break;

          case MCODE_OP_ADD:    ePos--; eStack[ePos] = eStack[ePos] +  eStack[ePos+1]; break;
          case MCODE_OP_SUB:    ePos--; eStack[ePos] = eStack[ePos] -  eStack[ePos+1]; break;
          case MCODE_OP_MUL:    ePos--; eStack[ePos] = eStack[ePos] *  eStack[ePos+1]; break;
          case MCODE_OP_DIV:
            ePos--;
            if(eStack[ePos+1] == _i64(0)) //???
              goto done;
            eStack[ePos] = eStack[ePos] /  eStack[ePos+1];
            break;

          case MCODE_OP_AND:    ePos--; eStack[ePos] = eStack[ePos] && eStack[ePos+1]; break;
          case MCODE_OP_OR:     ePos--; eStack[ePos] = eStack[ePos] || eStack[ePos+1]; break;
          case MCODE_OP_BITAND: ePos--; eStack[ePos] = eStack[ePos] &  eStack[ePos+1]; break;
          case MCODE_OP_BITOR:  ePos--; eStack[ePos] = eStack[ePos] |  eStack[ePos+1]; break;
          case MCODE_OP_BITXOR: ePos--; eStack[ePos] = eStack[ePos] ^  eStack[ePos+1]; break;


          // Function
          case MCODE_F_ABS: // N=abs(N)
            if ( eStack[ePos].isInteger() )
            {
              __int64 v=eStack[ePos].i();
              if(v < 0)
                v=-v;
              eStack[ePos] = v;
            }
            break;
          case MCODE_F_ITOA: // S=itoa(N,radix)
            ePos--;
            eStack[ePos] = itoaFunc(eStack+ePos);
            break;
          case MCODE_F_MIN:  // N=min(N1,N2)
            ePos--;
            eStack[ePos] = ( eStack[ePos+1] < eStack[ePos] ) ? eStack[ePos+1] : eStack[ePos];
            break;
          case MCODE_F_MAX:  // N=max(N1,N2)
            ePos--;
            eStack[ePos] = ( eStack[ePos+1] > eStack[ePos] ) ? eStack[ePos+1] : eStack[ePos];
            break;
          case MCODE_F_IIF:  // V=iif(Condition,V1,V2)
            ePos -= 2;
            eStack[ePos] = eStack[ePos].toInteger() ? eStack[ePos+1] : eStack[ePos+2];
            break;
          case MCODE_F_SUBSTR: // S=substr(S,N1,N2)
            ePos -= 2;
            eStack[ePos] = substrFunc(eStack+ePos);
            break;
          case MCODE_F_RINDEX: // S=rindex(S1,S2)
            ePos--;
            eStack[ePos] = rindexFunc(eStack+ePos);
            break;
          case MCODE_F_INDEX: // S=index(S1,S2)
            ePos--;
            eStack[ePos] = indexFunc(eStack+ePos);
            break;
          case MCODE_F_PANELITEM:  // V=panelitem(Panel,Index,TypeInfo)
            ePos-=2;
            eStack[ePos] = panelitemFunc(eStack+ePos);
            break;
          case MCODE_F_PANEL_SETPOS: // N=Panel.SetPos(panelType,fileName)
            ePos-=1;
            eStack[ePos] = panelsetposFunc(eStack+ePos);
            break;
          case MCODE_F_PANEL_FATTR:         // N=Panel.FAttr(panelType,fileMask)
            ePos-=1;
            eStack[ePos] = panelfattrFunc(eStack+ePos);
            break;
          case MCODE_F_PANEL_FEXIST:        // N=Panel.FExist(panelType,fileMask)
            ePos-=1;
            eStack[ePos] = panelfexistFunc(eStack+ePos);
            break;
          case MCODE_F_SLEEP: // N=Sleep(N)
            eStack[ePos] = sleepFunc(eStack+ePos);
            break;
          case MCODE_F_ENVIRON: // S=env(S)
            eStack[ePos] = environFunc(eStack+ePos);
            break;
          case MCODE_F_LEN:  // N=len(S)
            eStack[ePos] = strlen(eStack[ePos].toString());
            break;
          case MCODE_F_UCASE: // S=ucase(S1)
            LocalStrupr((char *)eStack[ePos].toString()); //??? strupr
            break;
          case MCODE_F_LCASE: // S=lcase(S1)
            LocalStrlwr((char *)eStack[ePos].toString()); //??? strlwr
            break;
          case MCODE_F_FEXIST:  // S=fexist(S)
            eStack[ePos] = fexistFunc(eStack+ePos);
            break;
          case MCODE_F_FSPLIT:  // S=fsplit(S,N)
            ePos--;
            eStack[ePos] = fsplitFunc(eStack+ePos);
            break;
          case MCODE_F_FATTR:   // N=fattr(S)
            eStack[ePos] = fattrFunc(eStack+ePos);
            break;
          case MCODE_F_MSAVE:   // N=msave(S)
            eStack[ePos] = msaveFunc(eStack+ePos);
            break;
          case MCODE_F_DLG_GETVALUE:        // V=Dlg.GetValue(ID,N)
            ePos--;
            eStack[ePos] = dlggetvalueFunc(eStack+ePos);
            break;
          case MCODE_F_EDITOR_SET: // N=Editor.Set(N,Var)
            ePos--;
            eStack[ePos] = editorsetFunc(eStack+ePos);
            break;
          case MCODE_F_STRING:  // S=string(V)
            eStack[ePos].toString();
            break;
          case MCODE_F_CLIP: // V=Clip(N,S)
            ePos--;
            eStack[ePos] = clipFunc(eStack+ePos);
            break;
          case MCODE_F_INT: // N=int(V)
            eStack[ePos].toInteger();
            break;
          case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S)
          {
             long Result=0;
             int CurMMode=CtrlObject->Macro.GetMode();
             if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS)
             {
               Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
               //f=f->GetTopModal();
               while(f)
               {
                 fo=f;
                 f=f->GetTopModal();
               }
               if(!f)
                 f=fo;

               if(f)
                 Result=f->ProcessKey(MCODE_F_MENU_CHECKHOTKEY);
             }
             eStack[ePos]=(__int64)Result;
             break;
          }
          case MCODE_F_DATE:  // // S=date(S)
            eStack[ePos] = dateFunc(eStack + ePos);
            break;
          case MCODE_F_XLAT: // S=xlat(S)
            eStack[ePos] = xlatFunc(eStack + ePos);
            break;
          case MCODE_F_MSGBOX:  // N=msgbox("Title","Text",flags)
          {
              DWORD Flags=MR->Flags;
              if(Flags&MFLAGS_DISABLEOUTPUT) // если был - удалим
              {
                if(LockScr) delete LockScr;
                LockScr=NULL;
              }
              ePos-=2; // 3 параметра!
              InternalInput++; // InternalInput - ограничитель того, чтобы макрос не продолжал свое исполнение
              eStack[ePos] = msgBoxFunc(eStack + ePos);
              InternalInput--;
              if(Flags&MFLAGS_DISABLEOUTPUT) // если стал - залочим
              {
                if(LockScr) delete LockScr;
                LockScr=new LockScreen;
              }
              break;
          }

          //
          default:
            eStack[++ePos] = FARPseudoVariable(MR->Flags, Key);
            break;
        }
      }
      _KEYMACRO(SysLog("  --- expr end"));
      *eStack = eStack[ePos];
      _KEYMACRO(SysLog("      ePos       =%d", ePos));
      _KEYMACRO(SysLog("      eStack->i()=%d", eStack->i()));
      _KEYMACRO(SysLog("      eStack->s()='%s'", eStack->s()));
      _KEYMACRO(SysLog(" Work.ExecLIBPos =%d (%0X)",Work.ExecLIBPos,Work.ExecLIBPos));
      goto begin;

// $Rep (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_SAVEREPCOUNT       1
// +--------> MCODE_OP_REP                2   p1=*
// |          <counter>                   3
// |          <counter>                   4
// |          MCODE_OP_JZ  ------------+  5   p2=*+2
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+
    case MCODE_OP_SAVEREPCOUNT:
    {
      // получим оригинальное значение счетчика
      // со стека и запишем его в рабочее место
      FARINT64 Counter;
      if((Counter.i64=eStack->toInteger()) < 0)
        Counter.i64=0;
      SetOpCode(MR,Work.ExecLIBPos+1,Counter.Part.HighPart); //???
      SetOpCode(MR,Work.ExecLIBPos+2,Counter.Part.LowPart); //???
      goto begin;
    }

    case MCODE_OP_REP:
    {
      // получим текущее значение счетчика
      FARINT64 Counter;
      Counter.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos); //???
      Counter.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos+1); //???
      // и положим его на вершину стека
      *eStack = Counter.i64;
      // уменьшим его и пойдем на MCODE_OP_JZ
      Counter.i64--;
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.HighPart); //???
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.LowPart); //???
      goto begin;
    }

    case MCODE_OP_END:
      // просто пропустим этот рудимент синтаксиса :)
      goto begin;

    case MCODE_OP_SAVE:
    {
      GetPlainText(value);
      // здесь проверка нужна, т.к. существует вариант вызова функции, без присвоения переменной
      if(*value)
      {
        TVarTable *t = ( *value == '%' ) ? &glbVarTable : &locVarTable;
        varInsert(*t, value)->value = *eStack;
      }
      goto begin;
    }

    case KEY_ALTINS:
    {
      if(RunGraber())
        return KEY_NONE;
      break;
    }

    /* $MMode 1
       0: MCODE_OP_MACROMODE
       1: '1'
    */
    case MCODE_OP_MACROMODE:
      if (Work.ExecLIBPos<MR->BufferSize)
      {
        Key=GetOpCode(MR,Work.ExecLIBPos++);
        if(Key == '1') // Изменяет режим отображения ("DisableOutput").
        {
          DWORD Flags=MR->Flags;
          if(Flags&MFLAGS_DISABLEOUTPUT) // если был - удалим
          {
            if(LockScr) delete LockScr;
            LockScr=NULL;
          }

          SwitchFlags(MR->Flags,MFLAGS_DISABLEOUTPUT);

          if(MR->Flags&MFLAGS_DISABLEOUTPUT) // если стал - залочим
          {
            if(LockScr) delete LockScr;
            LockScr=new LockScreen;
          }
        }
        goto begin;
      }
      break;

    default:
      ;
  }

  if(MR==Work.MacroWORK && ( Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE))
  {
    RetKey=Key;
    goto done;
  }

  return(Key);
}

// Проверить - еслть ли еще клавиша?
int KeyMacro::PeekKey()
{
  if (InternalInput || !Work.MacroWORK)
    return(0);

  struct MacroRecord *MR=Work.MacroWORK;
  if(Work.Executing == MACROMODE_NOMACRO && !Work.MacroWORK || Work.ExecLIBPos >= MR->BufferSize)
    return(FALSE);

  DWORD OpCode=GetOpCode(MR,Work.ExecLIBPos);
  return OpCode;
}

DWORD KeyMacro::SwitchFlags(DWORD& Flags,DWORD Value)
{
  if(Flags&Value) Flags&=~Value;
  else Flags|=Value;
  return Flags;
}


char *KeyMacro::MkRegKeyName(int IdxMacro,char *RegKeyName)
{
  char KeyText[50];
  ::KeyToText(MacroLIB[IdxMacro].Key,KeyText);
  sprintf(RegKeyName,"KeyMacros\\%s\\%s%s",
       GetSubKey(MacroLIB[IdxMacro].Flags&MFLAGS_MODEMASK),  (MacroLIB[IdxMacro].Flags&MFLAGS_DISABLEMACRO?"~":""),  KeyText);
  return RegKeyName;
}

/*
  после вызова этой функции нужно удалить память!!!
  функция декомпилит только простые последовательности, т.к.... клавиши
  в противном случае возвращает Src
*/
char *KeyMacro::MkTextSequence(DWORD *Buffer,int BufferSize,const char *Src)
{
  int J, Key;
  char MacroKeyText[50], *TextBuffer;

  // выделяем заведомо большой кусок
  if((TextBuffer=(char*)xf_malloc(BufferSize*64+16)) == NULL)
    return NULL;

  TextBuffer[0]=0;

  if(BufferSize == 1)
  {
    if((((DWORD)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE)
    {
      xf_free(TextBuffer);
      return Src?xf_strdup(Src):NULL;
    }

    if(KeyToText((DWORD)Buffer,MacroKeyText))
      strcpy(TextBuffer,MacroKeyText);
    return TextBuffer;
  }

  for (J=0; J < BufferSize; J++)
  {
    Key=Buffer[J];

    if((Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE || !KeyToText(Key,MacroKeyText))
    {
      xf_free(TextBuffer);
      return Src?xf_strdup(Src):NULL;
    }
    if(J)
      strcat(TextBuffer," ");
    strcat(TextBuffer,MacroKeyText);
  }
  return TextBuffer;
}

// Сохранение ВСЕХ макросов
void KeyMacro::SaveMacros(BOOL AllSaved)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::SaveMacros()"));
  //char *TextBuffer;
  char RegKeyName[150];

  //WriteVarsConst(MACRO_VARS);

  for (int I=0;I<MacroLIBCount;I++)
  {
    if(!AllSaved  && !(MacroLIB[I].Flags&MFLAGS_NEEDSAVEMACRO))
      continue;

    MkRegKeyName(I,RegKeyName);

    if (MacroLIB[I].BufferSize==0 || !MacroLIB[I].Src)
    {
      DeleteRegKey(RegKeyName);
      continue;
    }
#if 0
    if((TextBuffer=MkTextSequence(MacroLIB[I].Buffer,MacroLIB[I].BufferSize,MacroLIB[I].Src)) == NULL)
      continue;

    SetRegKey(RegKeyName,"Sequence",TextBuffer);
    //_SVS(SysLog("%3d) %s|Sequence='%s'",I,RegKeyName,TextBuffer));
    if(TextBuffer)
      xf_free(TextBuffer);
#endif
    SetRegKey(RegKeyName,"Sequence",MacroLIB[I].Src);

    // подсократим кодУ...
    for(int J=0; J < sizeof(MKeywordsFlags)/sizeof(MKeywordsFlags[0]); ++J)
    {
      if (MacroLIB[I].Flags & MKeywordsFlags[J].Value)
        SetRegKey(RegKeyName,MKeywordsFlags[J].Name,1);
      else
        DeleteRegValue(RegKeyName,MKeywordsFlags[J].Name);
    }
  }
}


int KeyMacro::WriteVarsConst(int ReadMode)
{
  if(ReadMode!=MACRO_VARS) // пока так :-)
    return FALSE;

  char UpKeyName[100];
  char ValueName[129];
  *ValueName='%';

  sprintf(UpKeyName,"KeyMacros\\%s",(ReadMode==MACRO_VARS?"Vars":""));

  TVarTable *t = &glbVarTable;
  for (int I=0;I < V_TABLE_SIZE;I++)
    for(int J=0;;++J)
    {
      TVarSet *var=varEnum(*t,I,J);
      if(!var)
        break;
      xstrncpy(ValueName+1,var->str,sizeof(ValueName)-2);
      switch(var->value.type())
      {
        case vtInteger:
          SetRegKey64(UpKeyName,ValueName,var->value.i());
          break;
        case vtString:
          SetRegKey(UpKeyName,ValueName,var->value.s());
          break;
      }

    }
  return TRUE;
}

/*
   KeyMacros\\Vars
     "StringName":REG_SZ
     "IntName":REG_DWORD
*/

int KeyMacro::ReadVarsConst(int ReadMode, char *SData, int SDataSize)
{
  if(ReadMode!=MACRO_VARS) // пока так :-)
    return FALSE;

  int I;
  char UpKeyName[100];
  char ValueName[129];
  long IData;
  __int64 IData64;

  sprintf(UpKeyName,"KeyMacros\\%s",(ReadMode==MACRO_VARS?"Vars":""));

  for (I=0;;I++)
  {
    IData=0;
    *ValueName=0;
    *SData=0;

    int Type=EnumRegValue(UpKeyName,I,ValueName,sizeof(ValueName),(LPBYTE)SData,SDataSize,(LPDWORD)&IData,(__int64*)&IData64);

    if (Type == REG_NONE)
      break;

    if(*ValueName != '%')
      continue;

    TVarTable *t = ( ValueName[1] == '%' ) ? &glbVarTable : &locVarTable;

    if (Type == REG_SZ)
      varInsert(*t, ValueName+1)->value = SData;
    else if (Type == REG_DWORD)
      varInsert(*t, ValueName+1)->value = (__int64)IData;
    else if (Type == REG_QWORD)
      varInsert(*t, ValueName+1)->value = IData64;
  }
  return TRUE;
}

/* $ 10.09.2000 SVS
  ! Исправим ситуацию с макросами в связи с переработаными кодами клавиш
  ! Функция ReadMacros имеет дополнительные аргументы
*/
int KeyMacro::ReadMacros(int ReadMode,char *Buffer,int BufferSize)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::ReadMacros()"));
  int I, J;
  struct MacroRecord CurMacro;
  /* $ 20.12.2003 IS
       Принудительно обнулим, иначе падаем при xf_free(Src)
  */
  memset(&CurMacro,0,sizeof(CurMacro));
  /* IS $ */

  char UpKeyName[100];
  char RegKeyName[150],KeyText[50];

  sprintf(UpKeyName,"KeyMacros\\%s",GetSubKey(ReadMode));

  for (I=0;;I++)
  {
    DWORD MFlags=0;

    if (!EnumRegKey(UpKeyName,I,RegKeyName,sizeof(RegKeyName)))
      break;

    char *KeyNamePtr=strrchr(RegKeyName,'\\');
    if (KeyNamePtr!=NULL)
    {
      strcpy(KeyText,KeyNamePtr+1);
      // ПОМНИМ! что название макроса, начинающееся на символ ~ - это
      // блокированный макрос!!!
      if(*KeyText == '~' && KeyText[1])
      {
        char *Ptr=KeyText+1;
        while(*Ptr && *Ptr == '~')// && IsSpace(KeyText[1]))
          ++Ptr;
        memmove(KeyText,Ptr,strlen(Ptr)+1);
        MFlags|=MFLAGS_DISABLEMACRO;
      }
    }
    else
      *KeyText=0;
    int KeyCode=KeyNameToKey(KeyText);
    if (KeyCode==-1)
      continue;

    GetRegKey(RegKeyName,"Sequence",Buffer,"",BufferSize);
    RemoveExternalSpaces(Buffer);
    if(!strlen(Buffer))
      continue;

    CurMacro.Key=KeyCode;
    CurMacro.Buffer=NULL;
    CurMacro.Src=NULL;
    CurMacro.BufferSize=0;
    CurMacro.Flags=MFlags|(ReadMode&MFLAGS_MODEMASK);

    for(J=0; J < sizeof(MKeywordsFlags)/sizeof(MKeywordsFlags[0]); ++J)
      CurMacro.Flags|=GetRegKey(RegKeyName,MKeywordsFlags[J].Name,0)?MKeywordsFlags[J].Value:0;

    if(ReadMode == MACRO_EDITOR || ReadMode == MACRO_DIALOG || ReadMode == MACRO_VIEWER)
    {
      if(CurMacro.Flags&MFLAGS_SELECTION)
      {
        CurMacro.Flags&=~MFLAGS_SELECTION;
        CurMacro.Flags|=MFLAGS_EDITSELECTION;
      }
      if(CurMacro.Flags&MFLAGS_NOSELECTION)
      {
        CurMacro.Flags&=~MFLAGS_NOSELECTION;
        CurMacro.Flags|=MFLAGS_EDITNOSELECTION;
      }
    }

    if(!ParseMacroString(&CurMacro,Buffer))
      continue;

    struct MacroRecord *NewMacros=(struct MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
    if (NewMacros==NULL)
    {
      return FALSE;
    }
    MacroLIB=NewMacros;
    CurMacro.Src=xf_strdup(Buffer);
    memcpy(MacroLIB+MacroLIBCount,&CurMacro,sizeof(CurMacro));
    MacroLIBCount++;
  }
  return TRUE;
}
/* SVS $ */

// эта функция будет вызываться из тех классов, которым нужен перезапуск макросов
void KeyMacro::RestartAutoMacro(int /*Mode*/)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::RestartAutoMacro()"));
#if 0
/*
Область      Рестарт
-------------------------------------------------------
Other         0
Shell         1 раз, при запуске ФАРа
Viewer        для каждой новой копии вьювера
Editor        для каждой новой копии редатора
Dialog        0
Search        0
Disks         0
MainMenu      0
Menu          0
Help          0
Info          1 раз, при запуске ФАРа и выставлении такой панели
QView         1 раз, при запуске ФАРа и выставлении такой панели
Tree          1 раз, при запуске ФАРа и выставлении такой панели
Common        0
*/
#endif
}

// Функция, запускающая макросы при старте ФАРа
// если уж вставлять предупреждение о недопустимости выполения
// подобных макросов, то именно сюды!
void KeyMacro::RunStartMacro()
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::RunStartMacro()"));
// временно отсавим старый вариант
#if 1
  if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !CmdMode && CtrlObject->Plugins.IsPluginsLoaded()))
    return;

  static int IsRunStartMacro=FALSE;
  if(IsRunStartMacro)
    return;
  IsRunStartMacro=TRUE;

  if(!IndexMode[MACRO_SHELL][1])
    return;

  struct MacroRecord *MR=MacroLIB+IndexMode[MACRO_SHELL][0];
  for(int I=0; I < IndexMode[MACRO_SHELL][1]; ++I)
  {
    DWORD CurFlags;
    if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==MACRO_SHELL &&
        MR[I].BufferSize>0 &&
        // исполняем не задисабленные макросы
        !(CurFlags&MFLAGS_DISABLEMACRO) &&
        (CurFlags&MFLAGS_RUNAFTERFARSTART) && CtrlObject)
    {
      if(CheckAll(MACRO_SHELL,CurFlags))
        PostNewMacro(MR+I);
    }
  }
#else
  static int AutoRunMacroStarted=FALSE;
  if(AutoRunMacroStarted || !MacroLIB || !IndexMode[Mode][1])
    return;

  //if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !CmdMode && CtrlObject->Plugins.IsPluginsLoaded()))
  if (!(CtrlObject && CtrlObject->Plugins.IsPluginsLoaded()))
    return;

  int I;
  struct MacroRecord *MR=MacroLIB+IndexMode[Mode][0];
  for(I=0; I < IndexMode[Mode][1]; ++I)
  {
    DWORD CurFlags;
    if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==Mode &&   // этот макрос из этой оперы?
        MR[I].BufferSize > 0 &&                             // что-то должно быть
        !(CurFlags&MFLAGS_DISABLEMACRO) &&                  // исполняем не задисабленные макросы
        (CurFlags&MFLAGS_RUNAFTERFARSTART) &&               // и тока те, что должны стартовать
        !(CurFlags&MFLAGS_RUNAFTERFARSTARTED)      // и тем более, которые еще не стартовали
       )
    {
      if(CheckAll(Mode,CurFlags)) // прежде чем запостить - проверим флаги
      {
        PostNewMacro(MR+I);
        MR[I].Flags|=MFLAGS_RUNAFTERFARSTARTED; // этот макрос успешно запулили на старт
      }
    }
  }

  // посчитаем количество оставшихся автостартующих макросов
  int CntStart=0;
  for(I=0; I < MacroLIBCount; ++I)
    if((MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTART) && !(MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTARTED))
      CntStart++;

  if(!CntStart) // теперь можно сказать, что все стартануло и в функцию RunStartMacro() нефига лазить
    AutoRunMacroStarted=TRUE;

#endif
  Work.ExecLIBPos=0;
}

// обработчик диалогового окна назначения клавиши
LONG_PTR WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  char KeyText[50];
  static int LastKey;
  static struct DlgParam *KMParam=NULL;
  int Index, I;

  if(Msg == DN_INITDIALOG)
  {
    KMParam=(struct DlgParam *)Param2;
    LastKey=0;

    // <Клавиши, которые не введешь в диалоге назначения>
    static const char * const PreDefKeyName[]={
      "CtrlDown", "Enter", "Esc", "F1", "CtrlF5",
      "CtrlMsWheelUp","ShiftMsWheelUp","AltMsWheelUp","CtrlShiftMsWheelUp","CtrlAltMsWheelUp","AltShiftMsWheelUp",
      "CtrlMsWheelDown","ShiftMsWheelDown","AltMsWheelDown","CtrlShiftMsWheelDown","CtrlAltMsWheelDown","AltShiftMsWheelDown"
    };
    for(I=0; I < sizeof(PreDefKeyName)/sizeof(PreDefKeyName[0]); ++I)
      Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(LONG_PTR)PreDefKeyName[I]);
/*
    int KeySize=GetRegKeySize("KeyMacros","DlgKeys");
    char *KeyStr;
    if(KeySize &&
       (KeyStr=(char*)xf_malloc(KeySize+1)) != NULL &&
       GetRegKey("KeyMacros","DlgKeys",KeyStr,"",KeySize)
      )
    {
      UserDefinedList KeybList;
      if(KeybList.Set(KeyStr))
      {
        KeybList.Start();
        const char *OneKey;
        *KeyText=0;
        while(NULL!=(OneKey=KeybList.GetNext()))
        {
          xstrncpy(KeyText, OneKey, sizeof(KeyText)-1);
          Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(long)KeyText);
        }
      }
      xf_free(KeyStr);
    }
*/
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)"");
    // </Клавиши, которые не введешь в диалоге назначения>

  }
  else if(Param1 == 2 && Msg == DN_EDITCHANGE)
  {
    LastKey=0;
    Param2=KeyNameToKey(((struct FarDialogItem*)Param2)->Data.Data);
    if(Param2 != -1)
      goto M1;
  }
  else if(Msg == DN_KEY && (Param2&KEY_END_SKEY) < KEY_END_FKEY)
  {
//    if((Param2&0x00FFFFFF) >= 'A' && (Param2&0x00FFFFFF) <= 'Z' && ShiftPressed)
//      Param2|=KEY_SHIFT;

//_SVS(SysLog("Macro: Key=%s",_FARKEY_ToName(Param2)));
    // <Обработка особых клавиш: F1 & Enter>
    // Esc & (Enter и предыдущий Enter) - не обрабатываем
    if(Param2 == KEY_ESC ||
       Param2 == KEY_ENTER && LastKey == KEY_ENTER ||
       Param2 == KEY_CTRLDOWN ||
       Param2 == KEY_F1
      )
    {
      return FALSE;
    }
/*
    // F1 - особый случай - нужно жать 2 раза
    // первый раз будет выведен хелп,
    // а второй раз - второй раз уже назначение
    if(Param2 == KEY_F1 && LastKey!=KEY_F1)
    {
      LastKey=KEY_F1;
      return FALSE;
    }
*/
    // Было что-то уже нажато и Enter`ом подтверждаем
    if(Param2 == KEY_ENTER && LastKey && LastKey != KEY_ENTER)
      return FALSE;
    // </Обработка особых клавиш: F1 & Enter>
M1:
    KeyMacro *MacroDlg=KMParam->Handle;

    if((Param2&0x00FFFFFF) > 0x7F && (Param2&0x00FFFFFF) < 0xFF)
      Param2=LocalKeyToKey(Param2&0x000000FF)|(Param2&(~0x000000FF));

    KMParam->Key=(DWORD)Param2;
    KeyToText(Param2,KeyText);

    // если УЖЕ есть такой макрос...
    if((Index=MacroDlg->GetIndex(Param2,KMParam->Mode)) != -1)
    {
      struct MacroRecord *Mac=MacroDlg->MacroLIB+Index;
      DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
      char Buf[256];
      char BufKey[64];
      char RegKeyName[150];
      MacroDlg->MkRegKeyName(Index,RegKeyName);

#if 0
      char *TextBuffer;

      // берем из памяти.
      if((TextBuffer=MacroDlg->MkTextSequence(Mac->Buffer,Mac->BufferSize)) != NULL)
      {
        int F=0;
        I=strlen(TextBuffer);
        if(I > 45) { I=45; F++; }
        sprintf(Buf,"\"%*.*s%s\"",I,I,TextBuffer,(F?"...":""));
        strcpy(BufKey,Buf);
        xf_free(TextBuffer);
      }
      else
        BufKey[0]=0;
#else
      if(Mac->Src != NULL)
      {
        int F=0;
        I=strlen(Mac->Src);
        if(I > 45) { I=45; F++; }
        sprintf(Buf,"\"%*.*s%s\"",I,I,Mac->Src,(F?"...":""));
        strcpy(BufKey,Buf);
      }
      else
        BufKey[0]=0;
#endif

      if((Mac->Flags&0xFF)==MACRO_COMMON)
        sprintf(Buf,
          MSG(!MacroDlg->RecBufferSize?
             (DisFlags?MMacroCommonDeleteAssign:MMacroCommonDeleteKey):
             MMacroCommonReDefinedKey),
          KeyText);
      else
        sprintf(Buf,
          MSG(!MacroDlg->RecBufferSize?
             (DisFlags?MMacroDeleteAssign:MMacroDeleteKey):
             MMacroReDefinedKey),
          KeyText);

      // проверим "а не совпадает ли всё?"
      if(!DisFlags &&
         Mac->Buffer && MacroDlg->RecBuffer &&
         Mac->BufferSize == MacroDlg->RecBufferSize &&
         (
           Mac->BufferSize >  1 && !memcmp(Mac->Buffer,MacroDlg->RecBuffer,MacroDlg->RecBufferSize*sizeof(DWORD)) ||
           Mac->BufferSize == 1 && (DWORD)Mac->Buffer == (DWORD)MacroDlg->RecBuffer
         )
        )
        I=0;
      else
        I=Message(MSG_WARNING,2,MSG(MWarning),
            Buf,
            MSG(MMacroSequence),
            BufKey,
            MSG(!MacroDlg->RecBufferSize?MMacroDeleteKey2:
                  (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
            MSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisOverwrite:MYes),
            MSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisAnotherKey:MNo));

      if(!I)
      {
        if(DisFlags)
        {
          if (Opt.AutoSaveSetup) // удаляем из реестра только в случае
          {                      // когда включен автосейв
            // удалим старую запись из реестра
            DeleteRegKey(RegKeyName);
          }
          // раздисаблим
          Mac->Flags&=~MFLAGS_DISABLEMACRO;
        }
        // в любом случае - вываливаемся
        Dialog::SendDlgMessage(hDlg,DM_CLOSE,1,0);
        return TRUE;
      }
      // здесь - здесь мы нажимали "Нет", ну а на нет и суда нет
      //  и значит очистим поле ввода.
      KeyText[0]=0;
    }
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)KeyText);
//    if(Param2 == KEY_F1 && LastKey == KEY_F1)
//      LastKey=-1;
//    else
      LastKey=Param2;
    return TRUE;
  }
  else if(Msg == DN_CTLCOLORDLGITEM) // сбросим Unchanged
  {
    Param2&=0xFF00FFFFU;      // Unchanged у нас сидит в младшем байте старшего слова
    Param2|=(Param2&0xFF)<<16;
    return Param2;
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

DWORD KeyMacro::AssignMacroKey()
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::AssignMacroKey()"));
/*
  +------ Define macro ------+
  | Press the desired key    |
  | ________________________ |
  +--------------------------+
*/
  static struct DialogData MacroAssignDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,30,4,0,0,0,0,(char *)MDefineMacroTitle,
  /* 01 */ DI_TEXT,-1,2,0,0,0,0,DIF_BOXCOLOR|DIF_READONLY,0,(char *)MDefineMacro,
  /* 02 */ DI_COMBOBOX,5,3,28,3,1,0,0,1,"",
  };
  MakeDialogItems(MacroAssignDlgData,MacroAssignDlg);
  struct DlgParam Param={this,0,StartMode};
//_SVS(SysLog("StartMode=%d",StartMode));

  IsProcessAssignMacroKey++;
  Dialog Dlg(MacroAssignDlg,sizeof(MacroAssignDlg)/sizeof(MacroAssignDlg[0]),AssignMacroDlgProc,(LONG_PTR)&Param);
  Dlg.SetPosition(-1,-1,34,6);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  IsProcessAssignMacroKey--;
  /* $ 30.01.2001 SVS
     Забыл сделать проверку на код возврата из диалога назначения
  */
  if(Dlg.GetExitCode() == -1)
    return (DWORD)-1;
  /* SVS $ */
  return Param.Key;
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
  DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;
  if(FlagsChk12 == Chk12 || FlagsChk12 == 0)
    return (2);
  else
    return (Flags&Chk1?1:0);
}


LONG_PTR WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  static struct DlgParam *KMParam=NULL;

  if(Msg == DN_INITDIALOG)
  {
    KMParam=(struct DlgParam *)Param2;
  }
  else if(Msg == DN_BTNCLICK && (Param1 == 4 || Param1 == 8))
  {
    for(int I=1; I <= 3; ++I)
      Dialog::SendDlgMessage(hDlg,DM_ENABLE,Param1+I,Param2);
  }
#if 0
  else if(Msg==DN_KEY && Param2==KEY_ALTF4)
  {
    if (RegVer)
    {
      KeyMacro *MacroDlg=KMParam->Handle;
      (*FrameManager)[0]->UnlockRefresh();
      FILE *MacroFile;
      char MacroFileName[NM];
      if (!FarMkTempEx(MacroFileName) || (MacroFile=fopen(MacroFileName,"wb"))==NULL)
        return TRUE;

      char *TextBuffer;
      DWORD Buf[1];
      Buf[0]=MacroDlg->RecBuffer[0];
      if((TextBuffer=MacroDlg->MkTextSequence((MacroDlg->RecBufferSize==1?Buf:MacroDlg->RecBuffer),MacroDlg->RecBufferSize)) != NULL)
      {
        fwrite(TextBuffer,strlen(TextBuffer),1,MacroFile);
        fclose(MacroFile);
        xf_free(TextBuffer);
        {
          //ConsoleTitle *OldTitle=new ConsoleTitle;
          FileEditor ShellEditor(MacroFileName,FALSE,FALSE,-1,-1,TRUE,NULL);
          //delete OldTitle;
          ShellEditor.SetDynamicallyBorn(false);
          FrameManager->EnterModalEV();
          FrameManager->ExecuteModal();
          FrameManager->ExitModalEV();
          if (!ShellEditor.IsFileChanged() || (MacroFile=fopen(MacroFileName,"rb"))==NULL)
            ;
          else
          {
            struct MacroRecord NewMacroWORK2={0};
            long FileSize=filelen(MacroFile);
            TextBuffer=(char*)malloc(FileSize);
            if(TextBuffer)
            {
              fread(TextBuffer,FileSize,1,MacroFile);
              if(!MacroDlg->ParseMacroString(&NewMacroWORK2,TextBuffer))
              {
                if(NewMacroWORK2.BufferSize > 1)
                  xf_free(NewMacroWORK2.Buffer);
              }
              else
              {
                MacroDlg->RecBuffer=NewMacroWORK2.Buffer;
                MacroDlg->RecBufferSize=NewMacroWORK2.BufferSize;
              }
            }
            fclose(MacroFile);
          }
        }
        FrameManager->ResizeAllFrame();
        FrameManager->PluginCommit();
      }
      else
        fclose(MacroFile);
      remove(MacroFileName);
    }
    else
      Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
    return TRUE;
  }
#endif
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

/* $ 03.01.2001 IS
   ! Устранение "двойного отрицания"
*/
int KeyMacro::GetMacroSettings(int Key,DWORD &Flags)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::GetMacroSettings()"));
  _KEYMACRO(SysLog("Param: Key=%s",_FARKEY_ToName(Key)));
/*
           1         2         3         4         5         6
    3456789012345678901234567890123456789012345678901234567890123456789
  1 г=========== Параметры макрокоманды для 'CtrlP' ==================¬
  2 ¦ [ ] Разрешить во время выполнения вывод на экран                ¦
  3 ¦ [ ] Выполнять после запуска FAR                                 ¦
  4 ¦-----------------------------------------------------------------¦
  5 ¦ [ ] Активная панель             [ ] Пассивная панель            ¦
  6 ¦   [?] На панели плагина           [?] На панели плагина         ¦
  7 ¦   [?] Выполнять для папок         [?] Выполнять для папок       ¦
  8 ¦   [?] Отмечены файлы              [?] Отмечены файлы            ¦
  9 ¦-----------------------------------------------------------------¦
 10   [?] Пустая командная строка
 11 ¦ [?] Отмечен блок                                                ¦
 12 ¦-----------------------------------------------------------------¦
 13 ¦               [ Продолжить ]  [ Отменить ]                      ¦
 14 L=================================================================+

*/
  static struct DialogData MacroSettingsDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,69,14,0,0,0,0,"",
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MMacroSettingsEnableOutput,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MMacroSettingsRunAfterStart,
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MMacroSettingsActivePanel,
  /* 05 */DI_CHECKBOX,7,6,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsPluginPanel,
  /* 06 */DI_CHECKBOX,7,7,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsFolders,
  /* 07 */DI_CHECKBOX,7,8,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsSelectionPresent,
  /* 08 */DI_CHECKBOX,37,5,0,0,0,0,0,0,(char *)MMacroSettingsPassivePanel,
  /* 09 */DI_CHECKBOX,39,6,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsPluginPanel,
  /* 10 */DI_CHECKBOX,39,7,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsFolders,
  /* 11 */DI_CHECKBOX,39,8,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsSelectionPresent,
  /* 12 */DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 13 */DI_CHECKBOX,5,10,0,0,0,2,DIF_3STATE,0,(char *)MMacroSettingsCommandLine,
  /* 14 */DI_CHECKBOX,5,11,0,0,0,2,DIF_3STATE,0,(char *)MMacroSettingsSelectionBlockPresent,
  /* 15 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 16 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 17 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(MacroSettingsDlgData,MacroSettingsDlg);

  char KeyText[66];
  KeyToText(Key,KeyText);
  sprintf(MacroSettingsDlg[0].Data,MSG(MMacroSettingsTitle),KeyText);
//  if(!(Key&0x7F000000))
//    MacroSettingsDlg[3].Flags|=DIF_DISABLE;

  MacroSettingsDlg[1].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
  MacroSettingsDlg[2].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;

  MacroSettingsDlg[5].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
  MacroSettingsDlg[6].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
  MacroSettingsDlg[7].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);

  MacroSettingsDlg[9].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
  MacroSettingsDlg[10].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
  MacroSettingsDlg[11].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);

  MacroSettingsDlg[13].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
  MacroSettingsDlg[14].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);

  struct DlgParam Param={this,0,0};
  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]),ParamMacroDlgProc,(LONG_PTR)&Param);
  Dlg.SetPosition(-1,-1,73,16);
  Dlg.SetHelp("KeyMacroSetting");
  FrameManager->GetBottomFrame()->Lock(); // отменим прорисовку фрейма
  Dlg.Process();
  FrameManager->GetBottomFrame()->Unlock(); // теперь можно :-)
  if (Dlg.GetExitCode()!=16)
    return(FALSE);

  Flags=MacroSettingsDlg[1].Selected?0:MFLAGS_DISABLEOUTPUT;
  Flags|=MacroSettingsDlg[2].Selected?MFLAGS_RUNAFTERFARSTART:0;
  if(MacroSettingsDlg[4].Selected)
  {
    Flags|=MacroSettingsDlg[5].Selected==2?0:
            (MacroSettingsDlg[5].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
    Flags|=MacroSettingsDlg[6].Selected==2?0:
            (MacroSettingsDlg[6].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
    Flags|=MacroSettingsDlg[7].Selected==2?0:
            (MacroSettingsDlg[7].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
  }
  if(MacroSettingsDlg[8].Selected)
  {
    Flags|=MacroSettingsDlg[9].Selected==2?0:
            (MacroSettingsDlg[9].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
    Flags|=MacroSettingsDlg[10].Selected==2?0:
            (MacroSettingsDlg[10].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
    Flags|=MacroSettingsDlg[11].Selected==2?0:
            (MacroSettingsDlg[11].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
  }
  Flags|=MacroSettingsDlg[13].Selected==2?0:
          (MacroSettingsDlg[13].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
  Flags|=MacroSettingsDlg[14].Selected==2?0:
          (MacroSettingsDlg[14].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);

  return(TRUE);
}
/* IS $ */

int KeyMacro::PostNewMacro(char *PlainText,DWORD Flags)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::PostNewMacro(char *PlainText,DWORD Flags)"));
  _KEYMACRO(SysLog("Param: PlainText=\"%s\"",PlainText));
  struct MacroRecord NewMacroWORK2={0};

  // сначала смотрим на парсер
  if(!ParseMacroString(&NewMacroWORK2,PlainText))
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }
  NewMacroWORK2.Flags=Flags;

  // теперь попробуем выделить немного нужной памяти
  struct MacroRecord *NewMacroWORK;
  if((NewMacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))) == NULL)
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }

  // теперь добавим в нашу "очередь" новые данные
  Work.MacroWORK=NewMacroWORK;
  NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
  memcpy(NewMacroWORK,&NewMacroWORK2,sizeof(struct MacroRecord));
  Work.MacroWORKCount++;

//  Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
  if(Work.ExecLIBPos == Work.MacroWORK->BufferSize)
    Work.ExecLIBPos=0;
  return TRUE;
}

int KeyMacro::PostNewMacro(struct MacroRecord *MRec,BOOL /*NeedAddSendFlag*/)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::PostNewMacro(struct MacroRecord *MRec)"));
  _KEYMACRO(SysLog("Param: MRec=%p",MRec));
  if(!MRec)
    return FALSE;

  struct MacroRecord NewMacroWORK2={0};
  memcpy(&NewMacroWORK2,MRec,sizeof(struct MacroRecord));
  NewMacroWORK2.Src=NULL;

//  if(MRec->BufferSize > 1)
  {
    if((NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+1)*sizeof(DWORD))) == NULL)
    {
      return FALSE;
    }
  }

  // теперь попробуем выделить немного нужной памяти
  struct MacroRecord *NewMacroWORK;
  if((NewMacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))) == NULL)
  {
//    if(MRec->BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }

  // теперь добавим в нашу "очередь" новые данные
  if((MRec->BufferSize+1) > 2)
    memcpy(NewMacroWORK2.Buffer,MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
  else if(MRec->Buffer)
    NewMacroWORK2.Buffer[0]=(DWORD)MRec->Buffer;
  NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize++]=KEY_NONE; // доп.клавиша/пустышка

  Work.MacroWORK=NewMacroWORK;
  NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
  memcpy(NewMacroWORK,&NewMacroWORK2,sizeof(struct MacroRecord));
  Work.MacroWORKCount++;

//  Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
  if(Work.ExecLIBPos == Work.MacroWORK->BufferSize)
    Work.ExecLIBPos=0;
  return TRUE;
}

// Парсер строковых эквивалентов в коды клавиш
//- AN ----------------------------------------------
//  Парсер строковых эквивалентов в байткод
//  Переписан практически с нуля 15.11.2003
//- AN ----------------------------------------------

// Стек структурных операторов
enum TExecMode
{
  emmMain, emmWhile, emmThen, emmElse, emmRep
};

struct TExecItem
{
  TExecMode state;
  DWORD pos1, pos2;
};

class TExec
{
  private:
    TExecItem stack[MAXEXEXSTACK];
  public:
    int current;
    void init()
    {
      current = 0;
      stack[current].state = emmMain;
      stack[current].pos1 = stack[current].pos2 = 0;
    }
    TExec() { init(); }
    TExecItem& operator()() { return stack[current]; }
    int add(TExecMode, DWORD, DWORD = 0);
    int del();
};

int TExec::add(TExecMode s, DWORD p1, DWORD p2)
{
  if ( ++current < MAXEXEXSTACK )
  {
    stack[current].state = s;
    stack[current].pos1 = p1;
    stack[current].pos2 = p2;
    return TRUE;
  }
  // Stack Overflow
  return FALSE;
};

int TExec::del()
{
  if ( --current < 0 )
  {
    // Stack Underflow ???
    current = 0;
    return FALSE;
  }
  return TRUE;
};

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static char *printfStr(DWORD* k, int& i)
{
  i++;
  char *s = (char*)&k[i];
  while ( strlen((char*)&k[i]) > 3 )
    i++;
  return s;
}

static void sprint(int i, char *fmt, char *data = NULL)
{
  static char tmp[256];
  strcat(strcpy(tmp, "%08X: "), fmt);
  SysLog(tmp, i, data);
}

static void printKeyValue(DWORD* k, int& i)
{
  int ii = i;
  if ( !k[i] )                                 sprint(ii, "<null>");
  else if ( k[i] == MCODE_OP_END )             sprint(ii, "END");
  else if ( k[i] == MCODE_OP_EXPR )            sprint(ii, "<expr> (");
  else if ( k[i] == MCODE_OP_DOIT )            sprint(ii, ") </expr>");
  else if ( k[i] == MCODE_OP_SAVE )            sprint(ii, "SAVE \"%%%s\"", printfStr(k, i));
  else if ( k[i] == MCODE_OP_SAVEREPCOUNT )    sprint(ii, "SAVE REP COUNT");
  else if ( k[i] == MCODE_OP_REP )             sprint(ii, "REP (*)", (char*)(i++));
  else if ( k[i] == MCODE_OP_PUSHVAR )         sprint(ii, " PUSH VAR \"%%%s\"", printfStr(k, i));
  else if ( k[i] == MCODE_OP_PUSHINT )
  {
    FARINT64 i64;
    i64.Part.HighPart=k[++i]; //???
    i64.Part.LowPart=k[++i]; //???
    SysLog("%08X:  PUSH INT %I64d", ii, i64.i64);
  }
  else if ( k[i] == MCODE_OP_PUSHSTR )         sprint(ii, " PUSH STR \"%s\"", printfStr(k, i));
  else if ( k[i] == MCODE_OP_JMP )             sprint(ii, "JMP %08X", (char*)k[++i]);
  else if ( k[i] == MCODE_OP_JZ  )             sprint(ii, "JZ %08X", (char*)k[++i]);

  else if ( k[i] == MCODE_OP_DATE )            sprint(ii, "$date ''");
  else if ( k[i] == MCODE_OP_PLAINTEXT )       sprint(ii, "$text ''");

  else if ( k[i] == MCODE_OP_NEGATE )          sprint(ii, " /-/");
  else if ( k[i] == MCODE_OP_ADD )             sprint(ii, " +");
  else if ( k[i] == MCODE_OP_SUB )             sprint(ii, " -");
  else if ( k[i] == MCODE_OP_MUL )             sprint(ii, " *");
  else if ( k[i] == MCODE_OP_DIV )             sprint(ii, " /");
  else if ( k[i] == MCODE_OP_LT )              sprint(ii, " <");
  else if ( k[i] == MCODE_OP_LE )              sprint(ii, " <=");
  else if ( k[i] == MCODE_OP_GE )              sprint(ii, " >=");
  else if ( k[i] == MCODE_OP_GT )              sprint(ii, " >");
  else if ( k[i] == MCODE_OP_EQ )              sprint(ii, " ==");
  else if ( k[i] == MCODE_OP_NE )              sprint(ii, " !=");
  else if ( k[i] == MCODE_OP_NOT )             sprint(ii, " !");
  else if ( k[i] == MCODE_OP_DIV )             sprint(ii, " /");
  else if ( k[i] == MCODE_OP_AND )             sprint(ii, " &&");
  else if ( k[i] == MCODE_OP_OR )              sprint(ii, " ||");
  else if ( k[i] == MCODE_OP_BITAND )          sprint(ii, " &");
  else if ( k[i] == MCODE_OP_BITOR )           sprint(ii, " |");
  else if ( k[i] == MCODE_OP_BITXOR )          sprint(ii, " ^");

  else if ( k[i] == MCODE_F_ABS )              sprint(ii, " N=abs(N)");
  else if ( k[i] == MCODE_F_MENU_CHECKHOTKEY ) sprint(ii, " N=checkhotkey(S)");
  else if ( k[i] == MCODE_F_DATE )             sprint(ii, " S=date(S)");
  else if ( k[i] == MCODE_F_EDITOR_SET )       sprint(ii, " N=Editor.Set(N,Var)");
  else if ( k[i] == MCODE_F_ENVIRON )          sprint(ii, " S=env(S)");
  else if ( k[i] == MCODE_F_FATTR )            sprint(ii, " N=fattr(S)");
  else if ( k[i] == MCODE_F_FEXIST )           sprint(ii, " S=fexist(S)");
  else if ( k[i] == MCODE_F_FSPLIT )           sprint(ii, " S=fsplit(S,N)");
  else if ( k[i] == MCODE_F_IIF )              sprint(ii, " V=iif(Condition,V1,V2)");
  else if ( k[i] == MCODE_F_INDEX )            sprint(ii, " S=index(S1,S2)");
  else if ( k[i] == MCODE_F_INT )              sprint(ii, " N=int(V)");
  else if ( k[i] == MCODE_F_ITOA )             sprint(ii, " S=itoa(N,radix)");
  else if ( k[i] == MCODE_F_SLEEP )            sprint(ii, " N=Sleep(N)");
  else if ( k[i] == MCODE_F_LEN )              sprint(ii, " N=len(S)");
  else if ( k[i] == MCODE_F_MAX )              sprint(ii, " N=max(N1,N2)");
  else if ( k[i] == MCODE_F_MSAVE )            sprint(ii, " N=msave(S)");
  else if ( k[i] == MCODE_F_MSGBOX )           sprint(ii, " N=msgbox(\"Title\",\"Text\",flags)");
  else if ( k[i] == MCODE_F_MIN )              sprint(ii, " N=min(N1,N2)");
  else if ( k[i] == MCODE_F_PANELITEM )        sprint(ii, " V=panelitem(Panel,Index,TypeInfo)");
  else if ( k[i] == MCODE_F_PANEL_SETPOS )     sprint(ii, " N=panel.SetPos(panelType,fileName)");
  else if ( k[i] == MCODE_F_PANEL_FATTR )      sprint(ii, " N=panel.fattr(panelType,S)");
  else if ( k[i] == MCODE_F_PANEL_FEXIST )     sprint(ii, " S=panel.fexist(panelType,S)");
  else if ( k[i] == MCODE_F_RINDEX )           sprint(ii, " S=rindex(S1,S2)");
  else if ( k[i] == MCODE_F_DLG_GETVALUE )     sprint(ii, " V=Dlg.GetValue(ID,N)");
  else if ( k[i] == MCODE_F_STRING )           sprint(ii, " S=string(V)");
  else if ( k[i] == MCODE_F_SUBSTR )           sprint(ii, " S=substr(S1,S2,N)");
  else if ( k[i] == MCODE_F_UCASE )            sprint(ii, " S=ucase(S1)");
  else if ( k[i] == MCODE_F_LCASE )            sprint(ii, " S=lcase(S1)");
  else if ( k[i] == MCODE_F_XLAT )             sprint(ii, " S=xlat(S)");
  else
  {
    int FARFunc = 0;
    for ( int j = 0 ; j < MKeywordsSize ; j++ )
      if ( k[i] == MKeywords[j].Value)
      {
        FARFunc = 1;
        sprint(ii, " %s", MKeywords[j].Name);
        break;
      }
    if ( !FARFunc )
    {
      char tmp[128];
      if ( KeyToText(k[i], tmp, sizeof(tmp)) )
        sprint(ii, "%s", tmp);
      else
        sprint(ii, "0x%08X", (char*)k[i]);
    }
  }
}
#endif
#endif

//- AN ----------------------------------------------
//  Компиляция строки BufPtr в байткод CurMacroBuffer
//- AN ----------------------------------------------
static int parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const char *BufPtr)
{
  _KEYMACRO(CleverSysLog Clev("parseMacroString"));
  _KEYMACRO(SysLog("BufPtr[%p]='%s'", BufPtr,BufPtr));
  _macro_nErr = 0;
  if ( BufPtr == NULL || !*BufPtr)
    return FALSE;
  int SizeCurKeyText = strlen(BufPtr)*2;
  char *CurrKeyText = (char*)xf_malloc(SizeCurKeyText);
  if ( CurrKeyText == NULL )
  {
    return FALSE;
  }
  //- AN ----------------------------------------------
  //  Буфер под парсинг выражений
  //- AN ----------------------------------------------
  DWORD *exprBuff = (DWORD*)xf_malloc(SizeCurKeyText*sizeof(DWORD));
  if ( exprBuff == NULL )
  {
    xf_free(CurrKeyText);
    return FALSE;
  }
  TExec exec;
  char varName[256];
  DWORD KeyCode, *CurMacro_Buffer = NULL;

  for (;;)
  {
    int Size = 1;
    int SizeVarName = 0;
    const char *oldBufPtr = BufPtr;

    if ( ( BufPtr = __GetNextWord(BufPtr, CurrKeyText) ) == NULL )
       break;

    //- AN ----------------------------------------------
    //  Проверка на строковый литерал
    //  Сделаем $Text опциональным
    //- AN ----------------------------------------------
    if ( *CurrKeyText == '\"' && CurrKeyText[1] )
    {
      KeyCode = MCODE_OP_PLAINTEXT;
      BufPtr = oldBufPtr;
    }
    else if ( ( KeyCode = KeyNameToKey(CurrKeyText) ) == (DWORD)-1 )
    {
      int ProcError=0;

      if ( *CurrKeyText == '%' && ( ( isalpha(CurrKeyText[1]) || CurrKeyText[1] == '_' ) || ( CurrKeyText[1] == '%' && ( isalpha(CurrKeyText[2]) || CurrKeyText[2]=='_' ) ) ) )
      {
        BufPtr = oldBufPtr;
        while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)))
          BufPtr++;
        memset(varName, 0, sizeof(varName));
        KeyCode = MCODE_OP_SAVE;
        char* p = varName;
        char* s = CurrKeyText+1;
        if ( *s == '%' )
          *p++ = *s++;
        char ch;
        *p++ = *s++;
        while ( ( isalnum(ch = *s++) || ( ch == '_') ) )
          *p++ = ch;
        *p = 0;
        int Length = strlen(varName)+1;
        // строка должна быть выровнена на 4
        SizeVarName = Length/sizeof(DWORD);
        if ( Length == 1 || ( Length % sizeof(DWORD)) != 0 ) // дополнение до sizeof(DWORD) нулями.
          SizeVarName++;
        BufPtr += Length;
        Size += parseExpr(BufPtr, exprBuff, '=', ';');
        if(_macro_nErr)
        {
          ProcError++;
        }
      }
      else
      {
        // проверим вариант, когда вызвали функцию, но результат не присвоили,
        // например, вызвали MsgBox(), но результат неважен
        // тогда SizeVarName=1 и varName=""
        int __nParam;

        char *Brack=strpbrk(CurrKeyText,"( "), Chr=0;
        if(Brack)
        {
          Chr=*Brack;
          *Brack=0;
        }

        if(funcLook(CurrKeyText, __nParam) != MCODE_F_NOFUNC)
        {
          if(Brack) *Brack=Chr;
          BufPtr = oldBufPtr;
          while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
            BufPtr++;
          Size += parseExpr(BufPtr, exprBuff, 0, 0);
          //Size--; //???
          if(_macro_nErr)
          {
            ProcError++;
          }
          else
          {
            KeyCode=MCODE_OP_SAVE;
            SizeVarName=1;
            memset(varName, 0, sizeof(varName));
          }
        }
        else
        {
          if(Brack) *Brack=Chr;
          ProcError++;
        }
      }

      if(ProcError)
      {
        if(!_macro_nErr)
          keyMacroParseError(err_Unrecognized_keyword, CurrKeyText, CurrKeyText,CurrKeyText);

        if ( CurMacro_Buffer != NULL )
        {
          xf_free(CurMacro_Buffer);
          CurMacroBuffer = NULL;
        }
        CurMacroBufferSize = 0;
        xf_free(CurrKeyText);
        xf_free(exprBuff);
        return FALSE;
      }

    }

    switch ( KeyCode )
    {
      //- AN ----------------------------------------------
      //  Опциональность аргумента
      //- AN ----------------------------------------------
      case MCODE_OP_DATE:
        while ( *BufPtr && IsSpace(*BufPtr) )
          BufPtr++;
        if ( *BufPtr == '\"' && BufPtr[1] )
          Size += parseExpr(BufPtr, exprBuff, 0, 0);
        else
        {
          Size += 4;
          exprBuff[0] = MCODE_OP_EXPR;
          exprBuff[1] = MCODE_OP_PUSHSTR;
          exprBuff[2] = 0;
          exprBuff[3] = MCODE_OP_DOIT;
        }
        break;
      case MCODE_OP_PLAINTEXT:
        Size += parseExpr(BufPtr, exprBuff, 0, 0);
        break;

// $Rep (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_SAVEREPCOUNT       1
// +--------> MCODE_OP_REP                    p1=*
// |          <counter>                   3
// |          <counter>                   4
// |          MCODE_OP_JZ  ------------+  5   p2=*+2
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_REP:
        Size += parseExpr(BufPtr, exprBuff, '(', ')');
        if ( !exec.add(emmRep, CurMacroBufferSize+Size, CurMacroBufferSize+Size+4) ) //??? 3
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(exprBuff);
          return FALSE;
        }
        Size += 5;  // естественно, размер будет больше = 4
        break;

// $If (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
// +--------- MCODE_OP_JMP             |
// |          ...          <-----------+
// +--------> MCODE_OP_END

// или

//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
//            MCODE_OP_END <-----------+

      case MCODE_OP_IF:
        Size += parseExpr(BufPtr, exprBuff, '(', ')');
        if ( !exec.add(emmThen, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(exprBuff);
          return FALSE;
        }
        Size++;
        break;

      case MCODE_OP_ELSE:
        Size++;
        break;

// $While (expr) ... $End
// -------------------------------------
// +--------> <expr>
// |          MCODE_OP_JZ  ------------+
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_WHILE:
        Size += parseExpr(BufPtr, exprBuff, '(', ')');
        if ( !exec.add(emmWhile, CurMacroBufferSize, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(exprBuff);
          return FALSE;
        }
        Size++;
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmRep:
          case emmWhile:
            Size += 2; // Место под дополнительный JMP
            break;
        }
        break;
    }
    if(_macro_nErr)
    {
      if ( CurMacro_Buffer != NULL )
      {
        xf_free(CurMacro_Buffer);
        CurMacroBuffer = NULL;
      }
      CurMacroBufferSize = 0;
      xf_free(CurrKeyText);
      xf_free(exprBuff);
      return FALSE;
    }

    if ( BufPtr == NULL ) // ???
      break;
    // код найден, добавим этот код в буфер последовательности.
    CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+Size+SizeVarName));
    if ( CurMacro_Buffer == NULL )
    {
      CurMacroBuffer = NULL;
      CurMacroBufferSize = 0;
      xf_free(CurrKeyText);
      xf_free(exprBuff);
      return FALSE;
    }
    switch ( KeyCode )
    {
      case MCODE_OP_DATE:
      case MCODE_OP_PLAINTEXT:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        break;
      case MCODE_OP_SAVE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        memcpy(CurMacro_Buffer+CurMacroBufferSize+Size, varName, SizeVarName*sizeof(DWORD));
        break;
      case MCODE_OP_IF:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_REP:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-6] = MCODE_OP_SAVEREPCOUNT;
        CurMacro_Buffer[CurMacroBufferSize+Size-5] = KeyCode;
        CurMacro_Buffer[CurMacroBufferSize+Size-4] = 0; // Initilize 0
        CurMacro_Buffer[CurMacroBufferSize+Size-3] = 0;
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_WHILE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_ELSE:
        if ( exec().state == emmThen )
        {
          exec().state = emmElse;
          CurMacro_Buffer[exec().pos1] = CurMacroBufferSize+2;
          exec().pos1 = CurMacroBufferSize;
          CurMacro_Buffer[CurMacroBufferSize] = 0;
        }
        else // тут $else и не предвиделось :-/
        {
          keyMacroParseError(err_Not_expected_ELSE, oldBufPtr+1, oldBufPtr); // CurrKeyText
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(exprBuff);
          return FALSE;
        }
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmMain:
            // тут $end и не предвиделось :-/
            keyMacroParseError(err_Not_expected_END, oldBufPtr+1, oldBufPtr); // CurrKeyText
            if ( CurMacro_Buffer != NULL )
            {
              xf_free(CurMacro_Buffer);
              CurMacroBuffer = NULL;
            }
            CurMacroBufferSize = 0;
            xf_free(CurrKeyText);
            xf_free(exprBuff);
            return FALSE;
          case emmThen:
            CurMacro_Buffer[exec().pos1-1] = MCODE_OP_JZ;
            CurMacro_Buffer[exec().pos1+0] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmElse:
            CurMacro_Buffer[exec().pos1-0] = MCODE_OP_JMP; //??
            CurMacro_Buffer[exec().pos1+1] = CurMacroBufferSize+Size-1; //??
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmRep:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;   //??????
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmWhile:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
        }

        if ( !exec.del() )  // Вообще-то этого быть не должно,  но подстрахуемся
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(exprBuff);
          return FALSE;
        }
        break;

      default:

        CurMacro_Buffer[CurMacroBufferSize]=KeyCode;

    } // end switch(KeyCode)

    CurMacroBufferSize += Size+SizeVarName;
  } // END for (;;)
#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
  SysLog("--- macro buffer out (%d)", CurMacroBufferSize);
  SysLogDump("",0,(LPBYTE)CurMacro_Buffer,CurMacroBufferSize*sizeof(DWORD),NULL);
  if ( CurMacro_Buffer )
  {
    int ii;
    for ( ii = 0 ; ii < CurMacroBufferSize ; ii++ )
      SysLog("%08X: %08X",ii,CurMacro_Buffer[ii]);

    SysLog("------------------------");
    for ( ii = 0 ; ii < CurMacroBufferSize ; ii++ )
      printKeyValue(CurMacro_Buffer, ii);
  }
  else
    SysLog("??? is NULL");
  SysLog("--- macro buffer end");
#endif
#endif
  if ( CurMacroBufferSize > 1 )
    CurMacroBuffer = CurMacro_Buffer;
  else if ( CurMacro_Buffer )
  {
    CurMacroBuffer = reinterpret_cast<DWORD*>(*CurMacro_Buffer);
    xf_free(CurMacro_Buffer);
  }
  xf_free(exprBuff);
  if ( exec().state != emmMain )
  {
    keyMacroParseError(err_Unexpected_EOS, CurrKeyText, CurrKeyText);
    return FALSE;
  }
  xf_free(CurrKeyText);
  if ( _macro_nErr )
    return FALSE;
  return TRUE;
}

int KeyMacro::ParseMacroString(struct MacroRecord *CurMacro,const char *BufPtr)
{
  if ( CurMacro )
    return parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);
  return FALSE;
}


// Функция получения индекса нужного макроса в массиве
// Ret=-1 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
int KeyMacro::GetIndex(int Key, int ChechMode)
{
  if(MacroLIB)
  {
    for(int I=0; I < 2; ++I)
    {
      int Pos,Len;
      struct MacroRecord *MPtr;
      if(ChechMode == -1)
      {
        Len=MacroLIBCount;
        MPtr=MacroLIB;
      }
      else
      {
        Len=IndexMode[ChechMode][1];
        if(Len)
          MPtr=MacroLIB+IndexMode[ChechMode][0];
  //_SVS(SysLog("ChechMode=%d (%d,%d)",ChechMode,IndexMode[ChechMode][0],IndexMode[ChechMode][1]));
      }

      if(Len)
      {
        for(Pos=0; Pos < Len; ++Pos, ++MPtr)
        {
          if (LocalUpper(MPtr->Key)==LocalUpper(Key) && MPtr->BufferSize > 0)
          {
    //        && (ChechMode == -1 || (MPtr->Flags&MFLAGS_MODEMASK) == ChechMode))
    //_SVS(SysLog("GetIndex: Pos=%d MPtr->Key=0x%08X", Pos,MPtr->Key));
            if(!(MPtr->Flags&MFLAGS_DISABLEMACRO))
              return Pos+((ChechMode >= 0)?IndexMode[ChechMode][0]:0);
          }
        }
      }
      // здесь смотрим на MACRO_COMMON
      if(ChechMode != -1 && !I)
        ChechMode=MACRO_COMMON;
      else
        break;
    }
  }
  return -1;
}

// получение размера, занимаемого указанным макросом
// Ret= 0 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
int KeyMacro::GetRecordSize(int Key, int CheckMode)
{
  int Pos=GetIndex(Key,CheckMode);
  if(Pos == -1)
    return 0;
  return sizeof(struct MacroRecord)+MacroLIB[Pos].BufferSize;
}

/* $ 21.12.2000 SVS
   Подсократим код.
*/
// получить название моды по коду
char* KeyMacro::GetSubKey(int Mode)
{
  return (char *)((Mode >= MACRO_OTHER && Mode < MACRO_LAST)?MKeywordsArea[Mode].Name:"");
}

// получить код моды по имени
int KeyMacro::GetSubKey(char *Mode)
{
  int I;
  for(I=MACRO_OTHER; I < MACRO_LAST; ++I)
    if(!stricmp(MKeywordsArea[I].Name,Mode))
      return I;
  return -1;
}

int KeyMacro::GetMacroKeyInfo(int Mode,int Pos,char *KeyName,char *Description,int DescriptionSize)
{
  if(Mode >= MACRO_OTHER && Mode < MACRO_LAST)
  {
    char UpKeyName[100];
    char RegKeyName[150];
    sprintf(UpKeyName,"KeyMacros\\%s",GetSubKey(Mode));

    if (!EnumRegKey(UpKeyName,Pos,RegKeyName,sizeof(RegKeyName)))
      return -1;

    char *KeyNamePtr=strrchr(RegKeyName,'\\');
    if (KeyNamePtr!=NULL)
      strcpy(KeyName,KeyNamePtr+1);
    GetRegKey(RegKeyName,"Description",Description,"",DescriptionSize);
    return Pos+1;
  }
  return -1;
}

/* $ 20.03.2002 DJ
   задействуем механизм также и для диалогов
*/

BOOL KeyMacro::CheckEditSelected(DWORD CurFlags)
{
  if(Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
  {
    int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MACRO_SHELL));
    Frame* CurFrame=FrameManager->GetCurrentFrame();
    if (CurFrame && CurFrame->GetType()==NeedType)
    {
      int CurSelected;
      if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
        CurSelected=CtrlObject->CmdLine->ProcessKey(MCODE_C_SELECTED);
      else
        CurSelected=CurFrame->ProcessKey(MCODE_C_SELECTED);

      if((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected ||
         (CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected)
          return FALSE;
    }
  }
  return TRUE;
}

/* DJ $ */

BOOL KeyMacro::CheckInsidePlugin(DWORD CurFlags)
{
  if(CtrlObject && CtrlObject->Plugins.CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
  //if(CtrlObject && CtrlObject->Plugins.CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
    return FALSE;
  return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,DWORD CurFlags)
{
 if ((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength!=0 ||
     (CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0)
      return FALSE;
  return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,DWORD CurFlags,BOOL IsPassivePanel)
{
  if(IsPassivePanel)
  {
    if(PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS) ||
       PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS))
      return FALSE;
  }
  else
  {
    if(PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS) ||
       PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS))
      return FALSE;
  }
  return TRUE;
}

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,DWORD CurFlags, BOOL IsPassivePanel)
{
  char FileName[NM*2];
  int FileAttr=-1;
  CheckPanel->GetFileName(FileName,CheckPanel->GetCurrentPos(),FileAttr);
  if(FileAttr != -1)
  {
    if(IsPassivePanel)
    {
      if((FileAttr&FA_DIREC) && (CurFlags&MFLAGS_PNOFOLDERS) || !(FileAttr&FA_DIREC) && (CurFlags&MFLAGS_PNOFILES))
        return FALSE;
    }
    else
    {
      if((FileAttr&FA_DIREC) && (CurFlags&MFLAGS_NOFOLDERS) || !(FileAttr&FA_DIREC) && (CurFlags&MFLAGS_NOFILES))
        return FALSE;
    }
  }
  return TRUE;
}

BOOL KeyMacro::CheckAll(int /*CheckMode*/,DWORD CurFlags)
{
/* $TODO:
     Здесь вместо Check*() попробовать заюзать IfCondition()
     для исключения повторяющегося кода.
*/
  if(!CheckInsidePlugin(CurFlags))
    return FALSE;

  // проверка на пусто/не пусто в ком.строке (а в редакторе? :-)
  if(CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
    if(!CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
      return FALSE;

  FilePanels *Cp=CtrlObject->Cp();
  if(!Cp)
    return FALSE;

  // проверки панели и типа файла
  Panel *ActivePanel=Cp->ActivePanel;
  Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);
  if(ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
  {

    if(CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
      if(!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
        return FALSE;

    if(CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
      if(!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
        return FALSE;

    if(CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
      if(!CheckFileFolder(ActivePanel,CurFlags,FALSE))
        return FALSE;

    if(CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
      if(!CheckFileFolder(PassivePanel,CurFlags,TRUE))
        return FALSE;

    /* $ 20.03.2002 DJ
       для диалогов - на панель смотреть тоже не надо
    */
    if(CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
      if(Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
      {
        int SelCount=ActivePanel->GetRealSelCount();
        if((CurFlags&MFLAGS_SELECTION) && SelCount < 1 ||
           (CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1)
          return FALSE;

        SelCount=PassivePanel->GetRealSelCount();
        if((CurFlags&MFLAGS_PSELECTION) && SelCount < 1 ||
           (CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1)
          return FALSE;
      }
    /* DJ $ */
  }

  if(!CheckEditSelected(CurFlags))
    return FALSE;

  return TRUE;
}

/*
  Return: FALSE - если тестируемый MFLAGS_* не установлен или
                  это не режим исполнения макроса!
          TRUE  - такой флаг(и) установлен(ы)
*/
BOOL KeyMacro::CheckCurMacroFlags(DWORD Flags)
{
  if(Work.Executing && Work.MacroWORK)
  {
    return (Work.MacroWORK->Flags&Flags)?TRUE:FALSE;
  }
  return(FALSE);

}

/*
  Return: 0 - не в режиме макро, 1 - Executing, 2 - Executing common, 3 - Recording, 4 - Recording common
  See farconst.hpp::MacroRecordAndExecuteType
*/
int KeyMacro::GetCurRecord(struct MacroRecord* RBuf,int *KeyPos)
{
  if(KeyPos && RBuf)
  {
    *KeyPos=Work.Executing?Work.ExecLIBPos:0;
    memset(RBuf,0,sizeof(struct MacroRecord));
    if(Recording == MACROMODE_NOMACRO)
    {
      if(Work.Executing)
      {
        memcpy(RBuf,MacroLIB+Work.MacroPC,sizeof(struct MacroRecord)); //????
        return Work.Executing;
      }
      memset(RBuf,0,sizeof(struct MacroRecord));
      return MACROMODE_NOMACRO;
    }
    RBuf->BufferSize=RecBufferSize;
    RBuf->Buffer=RecBuffer;
    return Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON;
  }
  return Recording?(Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON):(Work.Executing?Work.Executing:MACROMODE_NOMACRO);
}

static int __cdecl SortMacros(const struct MacroRecord *el1,
                           const struct MacroRecord *el2)
{
  int Mode1, Mode2;
  if((Mode1=(el1->Flags&MFLAGS_MODEMASK)) == (Mode2=(el2->Flags&MFLAGS_MODEMASK)))
    return 0;
  if(Mode1 < Mode2)
    return -1;
  return 1;
}

// Сортировка элементов списка
void KeyMacro::Sort(void)
{
  typedef int (__cdecl *qsort_fn)(const void*,const void*);
  // сортируем
  far_qsort(MacroLIB,
        MacroLIBCount,
        sizeof(struct MacroRecord),
        (qsort_fn)SortMacros);
  // перестраиваем индекс начал
  struct MacroRecord *MPtr;
  int I,J;
  int CurMode=MACRO_OTHER;
  memset(IndexMode,0,sizeof(IndexMode));
  for(MPtr=MacroLIB,I=0; I < MacroLIBCount; ++I,++MPtr)
  {
    J=MPtr->Flags&MFLAGS_MODEMASK;
    if(CurMode != J)
    {
      IndexMode[J][0]=I;
      CurMode=J;
    }
    IndexMode[J][1]++;
  }

//_SVS(for(I=0; I < sizeof(IndexMode)/sizeof(IndexMode[0]); ++I)SysLog("IndexMode[%02d.%s]=%d,%d",I,GetSubKey(I),IndexMode[I][0],IndexMode[I][1]));
}

DWORD KeyMacro::GetOpCode(struct MacroRecord *MR,int PC)
{
  DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)MR->Buffer;
  return OpCode;
}

// кинуть OpCode в буфер. Возвращает предыдущее значение
DWORD KeyMacro::SetOpCode(struct MacroRecord *MR,int PC,DWORD OpCode)
{
  DWORD OldOpCode;
  if(MR->BufferSize > 1)
  {
    OldOpCode=MR->Buffer[PC];
    MR->Buffer[PC]=OpCode;
  }
  else
  {
    OldOpCode=(DWORD)MR->Buffer;
    MR->Buffer=(DWORD*)OpCode;
  }
  return OldOpCode;
}

// Вот это лечит вот ЭТО:
// BugZ#873 - ACTL_POSTKEYSEQUENCE и заголовок окна
int KeyMacro::IsExecutingLastKey()
{
  if(Work.Executing && Work.MacroWORK)
  {
    return (Work.ExecLIBPos == Work.MacroWORK->BufferSize-1);
  }
  return FALSE;
}

void KeyMacro::DropProcess()
{
  if(Work.Executing)
  {
    if(LockScr) delete LockScr;
    LockScr=NULL;
    UsedInternalClipboard=0; //??
    Work.Executing=MACROMODE_NOMACRO;
    ReleaseWORKBuffer();
  }
}

static char __code2symbol(BYTE b1, BYTE b2)
{
  if(b1 >= '0' && b1 <= '9') b1-='0';
  else { b1&=~0x20; b1=b1-'A'+10; }
  if(b2 >= '0' && b2 <= '9') b2-='0';
  else { b2&=~0x20; b2=b2-'A'+10; }
  return ((b1<<4)&0x00F0)|(b2&0x000F);
}

static const char* ParsePlainText(char *CurKeyText, const char *BufPtr)
{
  char *PtrCurKeyText=CurKeyText;
  ++BufPtr;
  while (*BufPtr)
  {
    if(*BufPtr == '\\')
    {
      switch(BufPtr[1])
      {
        case '\\':
          *PtrCurKeyText++='\\';
          break;
        case '"':
          *PtrCurKeyText++='"';
          break;
        case 'n':
          *PtrCurKeyText++='\n';
          break;
        case 't':
          *PtrCurKeyText++='\t';
          break;
        case 'x':
          BufPtr+=2;
          if(isxdigit(BufPtr[0]) && isxdigit(BufPtr[1]))
            *PtrCurKeyText++=__code2symbol((BYTE)BufPtr[0],(BYTE)BufPtr[1]);
          break;
        default:
          *PtrCurKeyText++=BufPtr[1];
          break;
      }
      BufPtr+=2;
    }
    else if(*BufPtr == '"')
    {
      *PtrCurKeyText=0;
      BufPtr++;
      break;
    }
    else
      *PtrCurKeyText++=*BufPtr++;
  }
  if(*BufPtr)
    BufPtr++;
  return BufPtr;
}

static const char *__GetNextWord(const char *BufPtr,char *CurKeyText)
{
   // пропускаем ведущие пробельные символы
   while (IsSpace(*BufPtr) || IsEol(*BufPtr))
   {
     if(IsEol(*BufPtr))
       ; // TODO!!!
     BufPtr++;
   }

   if (*BufPtr==0)
     return NULL;

   const char *CurBufPtr=BufPtr;
   char Chr=*BufPtr, Chr2=BufPtr[1];
   BOOL SpecMacro=Chr=='$' && Chr2 && !(IsSpace(Chr2) || IsEol(Chr2));

   // ищем конец очередного названия клавиши
   while (Chr && !(IsSpace(Chr) || IsEol(Chr))) // удалить IsEol(*BufPtr)?
   {
     if(SpecMacro && (Chr == '[' || Chr == '(' || Chr == '{'))
       break;
     BufPtr++;
     Chr=*BufPtr;
   }
   int Length=BufPtr-CurBufPtr;

   memcpy(CurKeyText,CurBufPtr,Length);
   CurKeyText[Length]=0;
   return BufPtr;
}

int KeyMacro::PushState()
{
  if(CurPCStack+1 >= STACKLEVEL)
    return FALSE;
  ++CurPCStack;
  memcpy(PCStack+CurPCStack,&Work,sizeof(struct MacroState));
  memset(&Work,0,sizeof(struct MacroState));
  return TRUE;
}

int KeyMacro::PopState()
{
  if(CurPCStack < 0)
    return FALSE;
  memcpy(&Work,PCStack+CurPCStack,sizeof(struct MacroState));
  CurPCStack--;
  return TRUE;
}

void initMacroVarTable(int global)
{
  initVTable(global ? glbVarTable : locVarTable);
}

void doneMacroVarTable(int global)
{
  deleteVTable(global ? glbVarTable : locVarTable);
}
