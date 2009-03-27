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
#include "TVMStack.hpp"

// для диалога назначения клавиши
struct DlgParam{
  KeyMacro *Handle;
  DWORD Key;
  int Mode;
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
  {2,  "MacroArea",          MCODE_V_MACROAREA,0},

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
  {2,  "APanel.ColumnCount", MCODE_V_APANEL_COLUMNCOUNT,0}, // APanel.ColumnCount - активная панель:  количество колонок
  {2,  "PPanel.ColumnCount", MCODE_V_PPANEL_COLUMNCOUNT,0}, // PPanel.ColumnCount - пассивная панель: количество колонок

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
  {2,  "Editor.RealPos",     MCODE_V_EDITORREALPOS,0},
  {2,  "Editor.State",       MCODE_V_EDITORSTATE,0},
  {2,  "Editor.Value",       MCODE_V_EDITORVALUE,0},

  {2,  "Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
  {2,  "Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
  {2,  "Dlg.CurPos",         MCODE_V_DLGCURPOS,0},

  {2,  "Help.FileName",      MCODE_V_HELPFILENAME, 0},
  {2,  "Help.Topic",         MCODE_V_HELPTOPIC, 0},
  {2,  "Help.SelTopic",      MCODE_V_HELPSELTOPIC, 0},

  {2,  "Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
  {2,  "Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

  {2,  "Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
  {2,  "Viewer.State",       MCODE_V_VIEWERSTATE,0},

  {2,  "Windowed",           MCODE_C_WINDOWEDMODE,0},
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

struct TMacroKeywords MKeywordsFlags[] ={
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

int MKeywordsFlagsSize = sizeof(MKeywordsFlags)/sizeof(*MKeywordsFlags);
int MKeywordsSize = sizeof(MKeywords)/sizeof(*MKeywords);


static const char constRegError[]="RegError";

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
   { MCODE_OP_SELWORD,              8, "$SelWord" },
   { MCODE_OP_PLAINTEXT,            5, "$Text"    }, // $Text "Plain Text"
   { MCODE_OP_WHILE,                6, "$While"   },
   { MCODE_OP_XLAT,                 5, "$XLat"    },
};


TVarTable glbVarTable;
TVarTable glbConstTable;

static TVar __varTextDate;
TVMStack VMStack;

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


KeyMacro::KeyMacro()
{
  _KEYMACRO(SysLog("[%p] KeyMacro::KeyMacro()", this));
  MacroVersion=GetRegKey("KeyMacros","MacroVersion",0);
  CurPCStack=-1;
  Work.Init(NULL);
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
  if(Work.AllocVarTable && Work.locVarTable)
    xf_free(Work.locVarTable);
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
      if(MacroLIB[I].Description)
        xf_free(MacroLIB[I].Description);
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
        if(Work.MacroWORK[I].Description)
          xf_free(Work.MacroWORK[I].Description);
      }
      xf_free(Work.MacroWORK);
      if(Work.AllocVarTable)
      {
        deleteVTable(*Work.locVarTable);
        //xf_free(Work.locVarTable);
        //Work.locVarTable=NULL;
        //Work.AllocVarTable=false;
      }
      Work.MacroWORK=NULL;
      Work.MacroWORKCount=0;
    }
    else
    {
      if(Work.MacroWORK->BufferSize > 1 && Work.MacroWORK->Buffer)
        xf_free(Work.MacroWORK->Buffer);
      if(Work.MacroWORK->Src)
        xf_free(Work.MacroWORK->Src);
      if(Work.MacroWORK->Description)
        xf_free(Work.MacroWORK->Description);
      if(Work.AllocVarTable)
      {
        deleteVTable(*Work.locVarTable);
        //xf_free(Work.locVarTable);
        //Work.locVarTable=NULL;
        //Work.AllocVarTable=false;
      }
      Work.MacroWORKCount--;

      memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(struct MacroRecord),sizeof(struct MacroRecord)*Work.MacroWORKCount);
      Work.MacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(struct MacroRecord)*Work.MacroWORKCount);
    }
  }
}

// загрузка ВСЕХ макросов из реестра
int KeyMacro::LoadMacros(BOOL InitedRAM)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::LoadMacros()"));
  int Ret=FALSE;
  InitInternalVars(InitedRAM);

  if(Opt.DisableMacro&MDOL_ALL)
    return Ret;

  #define TEMP_BUFFER_SIZE (128*1024)
  char *Buffer=new char[TEMP_BUFFER_SIZE];

  if(Buffer)
  {
    int I;
    ReadVarsConst(MACRO_VARS,Buffer,TEMP_BUFFER_SIZE);
    ReadVarsConst(MACRO_CONSTS,Buffer,TEMP_BUFFER_SIZE);
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

      // Залочить _текущий_ фрейм, а не _последний немодальный_
      FrameManager->GetCurrentFrame()->Lock(); // отменим прорисовку фрейма
      MacroKey=AssignMacroKey();
      FrameManager->ResetLastInputRecord();
      FrameManager->GetCurrentFrame()->Unlock();

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
        if(RecBuffer)
          xf_free(RecBuffer);
        RecBuffer=NULL;
      }
      else
      {
        // в области common будем искать только при удалении
        int Pos=GetIndex(MacroKey,StartMode,!(RecBuffer && RecBufferSize));

        if (Pos == -1)
        {
          Pos=MacroLIBCount;
          if(RecBufferSize > 0)
          {
            struct MacroRecord *NewMacroLIB=(struct MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
            if (NewMacroLIB==NULL)
            {
              WaitInFastFind++;
              return(FALSE);
            }
            MacroLIB=NewMacroLIB;
            MacroLIBCount++;
          }
        }
        else
        {
          if(MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
            xf_free(MacroLIB[Pos].Buffer);
          if(MacroLIB[Pos].Src)
            xf_free(MacroLIB[Pos].Src);
          if(MacroLIB[Pos].Description)
            xf_free(MacroLIB[Pos].Description);
          MacroLIB[Pos].Buffer=NULL;
          MacroLIB[Pos].Src=NULL;
          MacroLIB[Pos].Description=NULL;
        }

        if(Pos < MacroLIBCount)
        {
          MacroLIB[Pos].Key=MacroKey;
          if(RecBufferSize > 0)
            RecBuffer[RecBufferSize++]=MCODE_OP_ENDKEYS;

          if(RecBufferSize > 1)
            MacroLIB[Pos].Buffer=RecBuffer;
          else if(RecBuffer && RecBufferSize > 0)
            MacroLIB[Pos].Buffer=reinterpret_cast<DWORD*>((DWORD_PTR)(*RecBuffer));
          else if(!RecBufferSize)
            MacroLIB[Pos].Buffer=NULL;

          MacroLIB[Pos].BufferSize=RecBufferSize;
          MacroLIB[Pos].Src=MkTextSequence(MacroLIB[Pos].Buffer,MacroLIB[Pos].BufferSize);
          MacroLIB[Pos].Description=NULL;

          // если удаляем макрос - скорректируем StartMode,
          // иначе макрос из common получит ту область, в которой его решили удалить.
          if(!MacroLIB[Pos].BufferSize||!MacroLIB[Pos].Src)
            StartMode=MacroLIB[Pos].Flags&MFLAGS_MODEMASK;

          MacroLIB[Pos].Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
        }
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

      RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+3));
      if (RecBuffer==NULL)
        return(FALSE);

      if(ReturnAltValue) // "подтасовка" фактов ;-)
        Key|=KEY_ALTDIGIT;

      if(!RecBufferSize)
        RecBuffer[RecBufferSize++]=MCODE_OP_KEYS;
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

  int LenTextBuf=(int)strlen((char*)&MR->Buffer[Work.ExecLIBPos]);
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
  return (int)strlen((char*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(DWORD Flags,DWORD CheckCode,DWORD& Err)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::FARPseudoVariable()"));
  int I;
  TVar Cond(_i64(0));
  char FileName[NM*2];
  int FileAttr=-1;

  // Найдем индекс нужного кейворда
  for ( I=0 ; I < sizeof(MKeywords)/sizeof(MKeywords[0]) ; ++I )
    if ( MKeywords[I].Value == CheckCode )
      break;
  if ( I == sizeof(MKeywords)/sizeof(MKeywords[0]) )
  {
    Err=1;
    _KEYMACRO(SysLog("return; Err=%d",Err));
    return Cond; // здесь TRUE обязательно, чтобы прекратить выполнение
                 // макроса, ибо код не распознан.
  }

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

        case MCODE_V_MACROAREA:
          Cond=GetSubKey(CtrlObject->Macro.GetMode());
          break;

        case MCODE_C_DISABLEOUTPUT: // DisableOutput?
          //Cond=Flags&CheckCode?1:0;
          Cond=LockScr?1:0;
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

        case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - курсор в начале cmd-строки редактирования?
        case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - курсор в конеце cmd-строки редактирования?
        case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
        case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
        case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
        case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
        {
          Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
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
            Cond=SelPanel->VMProcess(MCODE_C_ROOTFOLDER)?1:0;
          break;
        }

        case MCODE_C_APANEL_BOF:
        case MCODE_C_PPANEL_BOF:
        case MCODE_C_APANEL_EOF:
        case MCODE_C_PPANEL_EOF:
        {
          Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
          break;
        }

        case MCODE_C_SELECTED:    // Selected?
        {
#if 1
          int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
          if (CurFrame && CurFrame->GetType()==NeedType)
          {
            int CurSelected;
            if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
              CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
            else
              CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);
            Cond=CurSelected?1:0;
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=f->VMProcess(CheckCode);
#endif
          break;
        }

        case MCODE_C_EMPTY:   // Empty
        {
#if 1
          int CurMMode=CtrlObject->Macro.GetMode();
          if(CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
            Cond=CtrlObject->CmdLine->GetLength()==0?1:0;
          else
          {
            Frame *f=FrameManager->GetTopModal();
            if(f)
              Cond=f->VMProcess(CheckCode);
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=f->VMProcess(CheckCode);
#endif
          break;
        }

        case MCODE_C_BOF:
        case MCODE_C_EOF:
        {
#if 1
          int CurMMode=CtrlObject->Macro.GetMode();
          if(CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
            Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
          else
          {
            Frame *f=FrameManager->GetTopModal();
            if(f)
              Cond=f->VMProcess(CheckCode);
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=f->VMProcess(CheckCode);
#endif
          break;
        }

        case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
        case MCODE_V_DLGCURPOS:    // Dlg.CurPos
        case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
        {
          if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
          {
            Cond=CurFrame->VMProcess(CheckCode);
          }
          break;
        }

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

        case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - активная панель:  количество колонок
        case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - пассивная панель: количество колонок
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = (__int64)SelPanel->GetColumnsCount();
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
            RemoveExternalSpaces(FileName);
          }
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
          break;
        }

        case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
        case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
          /*
            if(f->GetType() == MODALTYPE_VIEWER)
            {
              if(CheckCode == MCODE_V_ITEMCOUNT)
                Cond=(__int64)((FileViewer*)f)->GetViewFileSize();
              else if(CheckCode == MCODE_V_CURPOS)
                Cond=(__int64)((FileViewer*)f)->GetViewFilePos()+1;
            }
            else
              Cond=f->VMProcess(CheckCode);
          */
             Cond=f->VMProcess(CheckCode);
          }
          break;
        }
        // *****************

        case MCODE_V_EDITORCURLINE: // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
        case MCODE_V_EDITORSTATE:   // Editor.State
        case MCODE_V_EDITORLINES:   // Editor.Lines
        case MCODE_V_EDITORCURPOS:  // Editor.CurPos
        case MCODE_V_EDITORREALPOS: // Editor.RealPos
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
              #if 0
              struct EditorConvertText ect;
              ect.Text=xf_strdup(egs.StringText);
              ect.TextLength=egs.StringLength;
              CtrlObject->Plugins.CurEditor->EditorControl(ECTL_EDITORTOOEM,&ect);
              Cond=(char *)ect.Text;
              xf_free(ect.Text);
              #else
              Cond=(char *)egs.StringText;
              #endif
            }
            else
              Cond=CtrlObject->Plugins.CurEditor->VMProcess(CheckCode);
          }
          break;
        }

        case MCODE_V_HELPFILENAME:  // Help.FileName
        case MCODE_V_HELPTOPIC:     // Help.Topic
        case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
        {
          if(CtrlObject->Macro.GetMode() == MACRO_HELP)
          {
            CurFrame->VMProcess(CheckCode,FileName,sizeof(FileName)-1);
            Cond=FileName;
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
              Cond=CtrlObject->Plugins.CurViewer->VMProcess(MCODE_V_VIEWERSTATE);
          }
          break;
        }

      }
      break;

    }

    default:
      Err=1;
      break;
  }

  _KEYMACRO(SysLog("return; Err=%d",Err));
  return Cond;
}

// S=trim(S[,N])
static bool trimFunc()
{
  int  mode = (int) VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();

  char *p = (char *)Val.toString();
  bool Ret=true;

  switch(mode)
  {
    case 0: p=RemoveExternalSpaces(p); break;  // alltrim
    case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
    case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
    default: Ret=false;
  }

  VMStack.Push((const char*)p);
  return Ret;
}

// S=substr(S,N1[,N2])
static bool substrFunc()
{
  int  p2 = (int)   VMStack.Pop().toInteger();
  int  p1 = (int)   VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();

  char *p = (char *)Val.toString();
  bool Ret=false;

  int len = (int)strlen(p);
  if ( p2 != 0 && p1 >= 0 &&  p1 < len )
  {
    if(p1 > 0)
      p += p1;
    len = (int)strlen(p);
    if ( ( p2 > 0 ) && ( p2 < len ) )
      p[p2] = 0;
    Ret=true;
  }
  else
    p="";

  VMStack.Push((const char*)p);
  return Ret;
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
static bool fsplitFunc()
{
  char path[NM*2];
  int m = (int)VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();
  const char *s = Val.toString();
  bool Ret=false;
  *path=0;
  if(!SplitFileName(s,path,m))
    *path=0;
  else
    Ret=true;
  VMStack.Push(path);
  return Ret;
}

#if 0
// S=Meta("!.!") - в макросах юзаем ФАРовы метасимволы
static bool metaFunc()
{
  TVar Val= VMStack.Pop();
  const char *s = Val.toString();

  if(s && *s)
  {
    char SubstText[512];
    char Name[NM],ShortName[NM];
    xstrncpy(SubstText,s,sizeof(SubstText)-1);
    SubstFileName(SubstText,sizeof (SubstText),Name,ShortName,NULL,NULL,TRUE);
    return TVar(SubstText);
  }
  return TVar("");
  return false;
}
#endif


// N=atoi(S[,radix])
static bool atoiFunc()
{
  bool Ret=true;
  char *endptr;

  TVar R = VMStack.Pop();
  TVar S = VMStack.Pop();

  VMStack.Push(TVar(_strtoi64(S.toString(),&endptr,(int)R.toInteger())));

  return Ret;
}

// S=itoa(N[,radix])
static bool itoaFunc()
{
  bool Ret=false;

  TVar R = VMStack.Pop();
  TVar N = VMStack.Pop();
  if(N.isInteger())
  {
    char value[NM];
    int Radix=(int)R.toInteger();
    if(Radix == 0)
      Radix=10;
    Ret=true;
    N=TVar(_i64toa(N.toInteger(),value,Radix));
  }
  VMStack.Push(N);

  return Ret;
}

// N=sleep(N)
static bool sleepFunc()
{
  long Period=(long)VMStack.Pop().toInteger();
  if(Period > 0)
  {
    Sleep((DWORD)Period);
    VMStack.Push(_i64(1));
    return true;
  }
  VMStack.Push(_i64(0));
  return false;
}

// N=eval(S[,N])
static bool evalFunc()
{
  bool Ret=true;
  DWORD Cmd=(DWORD)VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();

  struct MacroRecord RBuf;
  int KeyPos;

  if(Cmd&1)
  {
    CtrlObject->Macro.PostNewMacro(Val.toString(),0,0,TRUE);
    Ret=false;  // всегда! т.к. мы проверяем, а не исполняем
  }
  else
  {
    CtrlObject->Macro.GetCurRecord(&RBuf,&KeyPos);
    CtrlObject->Macro.PushState(true);
    if(!CtrlObject->Macro.PostNewMacro(Val.toString(),RBuf.Flags&(~MFLAGS_REG_MULTI_SZ),RBuf.Key))
    {
      CtrlObject->Macro.PopState();
      Ret=false;
    }
  }
  VMStack.Push((__int64)__getMacroErrorCode());

  return Ret;
}

// S=key(V)
static bool keyFunc()
{
  TVar VarKey=VMStack.Pop();
  char KeyName[128];
  *KeyName=0;

  if(VarKey.isInteger())
  {
    if(VarKey.i())
      KeyToText((int)VarKey.i(),KeyName,sizeof(KeyName)-1);
  }
  else
  {
    // Проверим...
    DWORD Key=(DWORD)KeyNameToKey(VarKey.s());
    if(Key != (DWORD)-1 && Key==(DWORD)VarKey.i())
      xstrncpy(KeyName,VarKey.s(),sizeof(KeyName)-1);
  }

  VMStack.Push((const char *)KeyName);
  return *KeyName==0;
}

// V=waitkey([N,[T]])
static bool waitkeyFunc()
{
  long Type=(long)VMStack.Pop().toInteger();
  long Period=(long)VMStack.Pop().toInteger();
  _SVS(SysLog("waitkeyFunc :: Period=%u",Period));
  DWORD Key=WaitKey((DWORD)-1,Period);
  if(!Type)
  {
    char KeyName[128];
    *KeyName=0;
    if(Key != KEY_NONE)
    {
     if(!KeyToText(Key,KeyName,sizeof(KeyName)-1))
       *KeyName=0;
    }
    VMStack.Push((const char *)KeyName);
    return *KeyName==0;
  }
  if(Key == KEY_NONE)
    Key=-1;
  VMStack.Push((__int64)Key);
  return Key != -1;
}


// n=min(n1,n2)
static bool minFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  VMStack.Push( V2 < V1  ? V2 : V1);
  return true;
}

// n=max(n1.n2)
static bool maxFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  VMStack.Push( V2 > V1  ? V2 : V1);
  return true;
}

// n=modFunc(n1,n2)
static bool modFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  if(!V2.i())
  {
    _KEYMACRO(SysLog("[%d] modFunc() Error: Divide (mod) by zero",__LINE__));
    VMStack.Push(_i64(0));
    return false;
  }
  VMStack.Push( V1 % V2 );
  return true;
}

// n=iif(expression,n1,n2)
static bool iifFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  TVar E  = VMStack.Pop();
  VMStack.Push( E.toInteger() ? V1 : V2 );
  return true;
}

// N=index(S1,S2)
static bool indexFunc()
{
  TVar S2 = VMStack.Pop();
  TVar S1 = VMStack.Pop();
  const char *s = S1.toString();
  const char *p = S2.toString();
  const char *i = LocalStrstri(s,p);
  bool Ret= i ? true : false;
  VMStack.Push(TVar((__int64)(i ? i-s : -1)));
  return Ret;
}

// S=rindex(S1,S2)
static bool rindexFunc()
{
  TVar S2 = VMStack.Pop();
  TVar S1 = VMStack.Pop();
  const char *s = S1.toString();
  const char *p = S2.toString();
  const char *i = LocalRevStrstri(s,p);
  bool Ret= i ? true : false;
  VMStack.Push(TVar((__int64)(i ? i-s : -1)));
  return Ret;
}

// S=date(S)
static bool dateFunc()
{
  TVar Val = VMStack.Pop();
  const char *s = Val.toString();
  int SizeMacroText = 16+(*s ? 0 : (int)strlen(Opt.DateFormat));
  bool Ret=false;
  SizeMacroText*=4;
  char *TStr=(char*)alloca(SizeMacroText);
  if(TStr && MkStrFTime(TStr,SizeMacroText,s))
    Ret=true;
  else
    TStr="";
  VMStack.Push(TVar(TStr));
  return Ret;
}

// S=xlat(S)
static bool xlatFunc()
{
  TVar Val = VMStack.Pop();
  char *Str = (char *)Val.toString();
  bool Ret=::Xlat(Str,0,(int)strlen(Str),NULL,Opt.XLat.Flags)==NULL?false:true;
  VMStack.Push(Str);
  return Ret;
}

// S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
static bool promptFunc()
{
  TVar ValHistory = VMStack.Pop();
  TVar ValSrc = VMStack.Pop();
  DWORD Flags = (DWORD)VMStack.Pop().toInteger();
  TVar ValPrompt = VMStack.Pop();
  TVar ValTitle = VMStack.Pop();
  TVar Result("");
  bool Ret=false;

  if(!(ValTitle.isInteger() && !ValTitle.i()))
  {
    const char *history=NULL;
    if(!(ValHistory.isInteger() && !ValHistory.i()))
      history=ValHistory.s();

    const char *src="";
    if(!(ValSrc.isInteger() && !ValSrc.i()))
       src=ValSrc.s();

    const char *prompt="";
    if(!(ValPrompt.isInteger() && !ValPrompt.i()))
       prompt=ValPrompt.s();

    const char *title=NullToEmpty(ValTitle.toString());

    char *TempBuf=(char *)xf_malloc(sizeof(char)*2048+1);
    if(TempBuf)
    {
      if(GetString(title,prompt,history,src,TempBuf,sizeof(char)*2048+1,NULL,Flags&~FIB_CHECKBOX,NULL,NULL))
      {
        Result=(const char *)TempBuf;
        Result.toString();
        Ret=true;
      }
      xf_free(TempBuf);
    }
  }
  VMStack.Push(Result);
  return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc()
{
  DWORD Flags = (DWORD)VMStack.Pop().toInteger();
  TVar ValB = VMStack.Pop();
  TVar ValT = VMStack.Pop();

  const char *title = "";
  if(!(ValT.isInteger() && !ValT.i()))
    title=NullToEmpty(ValT.toString());

  const char *text = "";
  if(!(ValB.isInteger() && !ValB.i()))
    text =NullToEmpty(ValB.toString());

  bool Ret=false;

  Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
  Flags|=FMSG_ALLINONE;
  if(HIWORD(Flags) == 0 || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
    Flags|=FMSG_MB_OK;

  char *TempBuf=(char *)alloca(strlen(title)+strlen(text)+16);
  int Result=0;
  if(TempBuf)
  {
    strcpy(TempBuf,title);
    strcat(TempBuf,"\n");
    strcat(TempBuf,text);
    Result=FarMessageFn(-1,Flags,NULL,(const char * const *)TempBuf,0,0)+1;
    Ret=true;
  }
  VMStack.Push((__int64)Result);
  return Ret;
}


// S=env(S)
static bool environFunc()
{
  char Env[2048];
  TVar S = VMStack.Pop();
  bool Ret=false;
  if(GetEnvironmentVariable(S.toString(),Env,sizeof(Env)))
  {
    Ret=true;
    FAR_CharToOem(Env,Env);
  }
  else
    *Env=0;
  VMStack.Push(Env);
  return Ret;
}

static bool _fattrFunc(int Type)
{
  bool Ret=false;
  DWORD FileAttr=(DWORD)-1;

  if(Type == 0 || Type == 2) // не панели
  {
    TVar Str = VMStack.Pop();

    WIN32_FIND_DATA FindData;
    //UINT  PrevErrMode;
    // дабы не выскакивал гуевый диалог, если диск эжектед.
    //PrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    GetFileWin32FindData((char *)Str.toString(),&FindData);
    //SetErrorMode(PrevErrMode);
    Ret=true;
    FileAttr=FindData.dwFileAttributes;
  }
  else
  {
    TVar S = VMStack.Pop();
    int typePanel=(int)VMStack.Pop().toInteger();
    char *Str = (char *)S.toString();

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
        SelPanel->GetFileName(NULL,Pos,(int&)FileAttr);
        Ret=true;
      }
    }
  }

  if(Type == 2 || Type == 3)
    FileAttr=(FileAttr!=(DWORD)-1)?1:0;

  VMStack.Push(TVar((__int64)(long)FileAttr));

  return Ret;
}

// N=fattr(S)
static bool fattrFunc()
{
  return _fattrFunc(0);
}

// N=fexist(S)
static bool fexistFunc()
{
  return _fattrFunc(2);
}

// N=panel.fattr(panelType,S)
static bool panelfattrFunc()
{
  return _fattrFunc(1);
}

// N=panel.fexist(panelType,S)
static bool panelfexistFunc()
{
  return _fattrFunc(3);
}

// N=FLock(Nkey,NState)
/*
  Nkey:
     0 - NumLock
     1 - CapsLock
     2 - ScrollLock

  State:
    -1 get state
     0 off
     1 on
     2 flip
*/
static bool flockFunc()
{
  TVar Ret(-1);

  int stateFLock=(int)VMStack.Pop().toInteger();
  UINT vkKey=(UINT)VMStack.Pop().toInteger();

  switch(vkKey)
  {
    case 0:
      vkKey=VK_NUMLOCK;
      break;
    case 1:
      vkKey=VK_CAPITAL;
      break;
    case 2:
      vkKey=VK_SCROLL;
      break;
    default:
      vkKey=0;
      break;
  }

  if(vkKey)
    Ret=(__int64)SetFLockState(vkKey,stateFLock);

  VMStack.Push(Ret);
  return Ret.i() != _i64(-1);
}

// V=Dlg.GetValue(ID,N)
static bool dlggetvalueFunc()
{
  TVar Ret(-1);

  int TypeInf=(int)VMStack.Pop().toInteger();
  int Index=(int)VMStack.Pop().toInteger()-1;

  Frame* CurFrame=FrameManager->GetCurrentFrame();

  if(CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
  {
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
          if(((Dialog*)CurFrame)->SendDlgMessage((HANDLE)CurFrame,DM_LISTGETITEM,Index,(LONG_PTR)&ListItem))
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

  VMStack.Push(Ret);

  return Ret.i() != _i64(-1);
}

// OldVar=Editor.Set(Idx,Var)
static bool editorsetFunc()
{
  TVar Ret(-1);
  TVar _longState=VMStack.Pop();
  int Index=(int)VMStack.Pop().toInteger();

  if(CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
  {
    long longState=-1L;

    if(Index != 12)
      longState=(long)_longState.toInteger();

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

    if(Index != 12 && longState != -1 || Index == 12 && _longState.i() == -1)
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
          xstrncpy(EdOpt.WordDiv,_longState.toString(),sizeof(EdOpt.WordDiv)-1); break;
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

  VMStack.Push(Ret);

  return Ret.i() == _i64(-1);
}

// b=msave(var)
static bool msaveFunc()
{
  TVar Val=VMStack.Pop();
  TVarTable *t = &glbVarTable;
  const char *Name=Val.s();
  if(!Name || *Name!= '%')
  {
    VMStack.Push(_i64(0));
    return false;
  }

  TVarSet *tmpVarSet=varLook(*t, Name+1);
  if(!tmpVarSet)
  {
    VMStack.Push(_i64(0));
    return false;
  }

  TVar Result=tmpVarSet->value;

  DWORD Ret=(DWORD)-1;
  char ValueName[129];
  xstrncpy(ValueName,Val.s(),sizeof(ValueName)-2);
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
  VMStack.Push(TVar(Ret==ERROR_SUCCESS?1:0));
  return Ret==ERROR_SUCCESS;
}

// V=Clip(N[,S])
static bool clipFunc()
{
  TVar Val=VMStack.Pop();

  // принудительно второй параметр ставим AS string
  if(Val.isInteger() && Val.i() == 0)
  {
    Val=(const char *)"";
    Val.toString();
  }

  int cmdType=(int)VMStack.Pop().toInteger();
  int Ret=0;

  switch(cmdType)
  {
    case 0: // Get from Clipboard, "S" - ignore
    {
      char *ClipText=InternalPasteFromClipboard(0); // 0!  ???
      if(ClipText)
      {
        TVar varClip(ClipText);
        xf_free(ClipText);
        VMStack.Push(varClip);
        return true;
      }
      break;
    }
    case 1: // Put "S" into Clipboard
    {
      Ret=InternalCopyToClipboard(Val.s(),0);
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }
    case 2: // Add "S" into Clipboard
    {
      TVar varClip(Val.s());
      char *CopyData=InternalPasteFromClipboard(0); // 0!  ???
      if(CopyData)
      {
        int DataSize=(int)strlen(CopyData);
        char *NewPtr=(char *)xf_realloc(CopyData,DataSize+strlen(Val.s())+2);
        if (NewPtr)
        {
          CopyData=NewPtr;
          strcpy(CopyData+DataSize,Val.s());
          varClip=CopyData;
          xf_free(CopyData);
        }
        else
        {
          xf_free(CopyData);
        }
      }
      Ret=InternalCopyToClipboard(varClip.s(),0);
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }
    case 3: // Copy Win to internal, "S" - ignore
    case 4: // Copy internal to Win, "S" - ignore
    {
      int _UsedInternalClipboard=UsedInternalClipboard;

      {
        TVar varClip("");
        UsedInternalClipboard=cmdType-3;
        char *ClipText=InternalPasteFromClipboard(0); // 0!  ???
        if(ClipText)
        {
          varClip=ClipText;
          xf_free(ClipText);
        }
        UsedInternalClipboard=UsedInternalClipboard==0?1:0;
        Ret=InternalCopyToClipboard(varClip.s(),0); // 0!  ???
      }

      UsedInternalClipboard=_UsedInternalClipboard;
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }
  }

  return Ret?true:false;
}

// N=Panel.SetPosIdx(panelType,Idx)
static bool panelsetposidxFunc()
{
  long idxItem=(long)VMStack.Pop().toInteger();
  int typePanel=(int)VMStack.Pop().toInteger();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  __int64 Ret=0;

  if(SelPanel)
  {
    int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
    if(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
    {
      if(SelPanel->GoToFile(idxItem-1))
      {
        //SelPanel->Show();
        // <Mantis#0000289> - грозно, но со вкусом :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=(__int64)(SelPanel->GetCurrentPos()+1);
      }
    }
  }
  VMStack.Push(Ret);
  return Ret?true:false;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc()
{
  _KEYMACRO(CleverSysLog Clev("panelsetpathFunc()"));
  TVar ValFileName=VMStack.Pop();
  TVar Val=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();
  __int64 Ret=_i64(0);

  if(!(Val.isInteger() && !Val.i()))
  {
    const char *pathName=Val.s();

    const char *fileName="";
    if(!ValFileName.isInteger())
      fileName=ValFileName.s();

    //_KEYMACRO(SysLog("pathName='%s', fileName='%s'",pathName,fileName));
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    Panel *PassivePanel=NULL;
    if(ActivePanel!=NULL)
      PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

    Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);

    if(SelPanel)
    {
      if(SelPanel->SetCurDir(pathName,TRUE))
      {
                       // PointToName()???
        SelPanel->GoToFile(fileName); // здесь без проверки, т.к. параметр fileName аля опциональный

        //SelPanel->Show();
        // <Mantis#0000289> - грозно, но со вкусом :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=_i64(1);
      }
    }
  }
  VMStack.Push(Ret);
  return Ret?true:false;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc()
{
  TVar Val=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();
  const char *fileName=Val.s();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  __int64 Ret=_i64(0);

  if(SelPanel)
  {
    int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
    if(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
    {
      if(SelPanel->GoToFile(fileName))
      {
        //SelPanel->Show();
        // <Mantis#0000289> - грозно, но со вкусом :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=(__int64)(SelPanel->GetCurrentPos()+1);
      }
    }
  }
  VMStack.Push(Ret);
  return Ret?true:false;
}


// V=Editor.Sel(Action[,Opt])
static bool editorselFunc()
{
 /*
  MCODE_F_EDITOR_SEL
    Action: 0 = Get Param
                Opt:  0 = return FirstLine
                      1 = return FirstPos
                      2 = return LastLine
                      3 = return LastPos
                      4 = return block type (0=nothing 1=stream, 2=column)
                return: 0 = failure, 1... request value

            1 = Set Pos
                Opt:  0 = begin block (FirstLine & FirstPos)
                      1 = end block (LastLine & LastPos)
                return: 0 = failure, 1 = success

            2 = Set Stream Selection Edge
                Opt:  0 = selection start
                      1 = selection finish
                return: 0 = failure, 1 = success

            3 = Set Column Selection Edge
                Opt:  0 = selection start
                      1 = selection finish
                return: 0 = failure, 1 = success
            4 = Unmark selected block
                Opt: ignore
                return 1
  */
  TVar Ret(_i64(0));
  TVar Opt    = VMStack.Pop();
  TVar Action = VMStack.Pop();

  int Mode=CtrlObject->Macro.GetMode();

  Frame* CurFrame=FrameManager->GetCurrentFrame();

  int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
  if (CurFrame && CurFrame->GetType()==NeedType)
  {
    if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
      Ret=CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,(void*)Action.toInteger(),Opt.i());
    else
      Ret=CurFrame->VMProcess(MCODE_F_EDITOR_SEL,(void*)Action.toInteger(),Opt.i());
  }

  VMStack.Push(Ret);

  return Ret.i() == _i64(1);
}


// V=callplugin(SysID[,param])
static bool callpluginFunc()
{
  __int64 Ret=_i64(0);
  TVar Param     = VMStack.Pop();
  TVar SysID     = VMStack.Pop();

  int PlugNum=CtrlObject->Plugins.FindPlugin((DWORD)SysID.i());
  if(PlugNum >= 0)
  {
    /*
      OPEN_DISKMENU     = 0,
      OPEN_PLUGINSMENU  = 1,
      OPEN_FINDLIST     = 2,
      OPEN_SHORTCUT     = 3,
      OPEN_COMMANDLINE  = 4,
      OPEN_EDITOR       = 5,
      OPEN_VIEWER       = 6,
      OPEN_FILEPANEL    = 7,
      OPEN_DIALOG       = 8,
    */
    int OpenFrom = -1;
    Frame* frame = FrameManager->GetCurrentFrame();
    if(frame) switch(frame->GetType()) {
      case MODALTYPE_PANELS:
        OpenFrom = OPEN_COMMANDLINE | OPEN_FROMMACRO;
        break;
      case MODALTYPE_EDITOR:
        OpenFrom = OPEN_EDITOR      | OPEN_FROMMACRO;
        break;
      case MODALTYPE_VIEWER:
        OpenFrom = OPEN_VIEWER      | OPEN_FROMMACRO;
        break;
      case MODALTYPE_DIALOG:
        OpenFrom = OPEN_DIALOG      | OPEN_FROMMACRO;
        break;
      default:
        break;
    }
    if(OpenFrom != -1) {
      if(CtrlObject->Plugins.CallPlugin(PlugNum,OpenFrom,
                                        Param.isString() ? (void*)Param.s() :
                                                           (void*)(size_t)Param.i()))
      {
        Ret=_i64(1);
      }
    }
  }

  VMStack.Push(Ret);

  return Ret?true:false;
}

//*************************
// Ахтунг, недоделано!
//*************************
// V=reg.get(iRoot, "Key"[, "Value"])
/* iRoot=0:
0 = FAR Root, HKEY_CURRENT_USER (в зависимости от юзера)
1 = FAR Root, HKEY_LOCAL_MACHINE
2 = HKEY_CLASSES_ROOT
3 = HKEY_CURRENT_USER
4 = HKEY_LOCAL_MACHINE
*/
static bool reggetFunc()
{
  bool Ret=false;
#if 0
  bool checkKey=false;

  HKEY SaveRootKey=GetRegRootKey();
  HKEY NewRootKey=SaveRootKey;

  char SaveRegRoot[sizeof(Opt.RegRoot)];
  strcpy(SaveRegRoot,Opt.RegRoot);

  TVar V = VMStack.Pop();
  // принудительно параметр ставим AS string
  if(V.isInteger() && V.i() == 0)
    checkKey=true;

  TVar K = VMStack.Pop();
  // принудительно параметр ставим AS string
  if(K.isInteger() && K.i() == 0)
  {
    K=(const char *)"";
    K.toString();
  }

  TVar R = VMStack.Pop();
  int RType=(int)R.i();

  TVar RetVal=_i64(-1);
  varLook(glbConstTable, constRegError,1)->value = _i64(0);

  switch(RType)
  {
    case 0:  NewRootKey=HKEY_CURRENT_USER;  break;
    case 1:  NewRootKey=HKEY_LOCAL_MACHINE; break;
    case 2:  NewRootKey=HKEY_CLASSES_ROOT;  break;
    case 3:  NewRootKey=HKEY_CURRENT_USER;  break;
    case 4:  NewRootKey=HKEY_LOCAL_MACHINE; break;
    default: RType=-1;  break;
  }

  if(RType != -1)
  {
    SetRegRootKey(NewRootKey);
    xstrncpy(Opt.RegRoot,K.s(),sizeof(Opt.RegRoot)-1);

    //if(GetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType))
    //  checkKey

    // Restore
    SetRegRootKey(SaveRootKey);
    strcpy(Opt.RegRoot,SaveRegRoot);
    //Ret=true;
  }

  varLook(glbConstTable, constRegError,1)->value = Ret?_i64(0):_i64(1);
  VMStack.Push( RetVal );
#else
  VMStack.Pop();
  VMStack.Pop();
  VMStack.Pop();
  VMStack.Push( _i64(0) );
#endif

  return Ret;
}

// Result=replace(Str,Find,Replace[,Cnt])
static bool replaceFunc()
{
  TVar Count= VMStack.Pop();
  TVar Repl = VMStack.Pop();
  TVar Find = VMStack.Pop();
  TVar Src  = VMStack.Pop();

  __int64 Ret=_i64(1);

  char *Str=NULL;

  int lenS=(int)strlen(Src.s());
  int lenF=(int)strlen(Find.s());
  int lenR=(int)strlen(Repl.s());

  int cnt=0;
  const char *Ptr=Src.s();
  while((Ptr=LocalStrstri(Ptr,Find.s())) != NULL)
  {
    cnt++;
    Ptr+=lenF;
  }

  if(cnt)
  {
    if(lenR > lenF)
      lenS+=cnt*(lenR-lenF+1); //???

    Str=(char *)xf_malloc(lenS+1);
    if(Str)
    {
      strcpy(Str,Src.s());
      cnt=(int)Count.i();
      if(cnt <= 0)
        cnt=-1;
      ReplaceStrings(Str,Find.s(),Repl.s(),cnt,TRUE);
      VMStack.Push((const char *)Str);
      free(Str);
    }
    else
    {
      Ret=_i64(0);
      VMStack.Push(Src);
    }
  }
  else
    VMStack.Push(Src);

  return Ret?true:false;
}

// V=PanelItem(typePanel,Index,TypeInfo)
static bool panelitemFunc()
{
  TVar P2=VMStack.Pop();
  TVar P1=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();

  TVar Ret(_i64(0));

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  if(!SelPanel)
  {
    VMStack.Push(Ret);
    return false;
  }

  int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
  if(!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
  {
    VMStack.Push(Ret);
    return false;
  }

  int Index=(int)(P1.toInteger())-1;
  int TypeInfo=(int)P2.toInteger();

  struct FileListItem filelistItem;
  if(TypePanel == TREE_PANEL)
  {
    struct TreeItem treeItem;
    if(SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
    {
      VMStack.Push(TVar(treeItem.Name));
      return true;
    }
  }
  else
  {
    char Date[128], Time[64];

    if(!SelPanel->GetItem(Index,&filelistItem))
      TypeInfo=-1;
    switch(TypeInfo)
    {
      case 0:  // Name
        Ret=TVar((const char*)filelistItem.Name);
        break;
      case 1:  // ShortName
        Ret=TVar((const char*)filelistItem.ShortName);
        break;
      case 2:  // FileAttr
        Ret=TVar((__int64)(long)filelistItem.FileAttr);
        break;
      case 3:  // CreationTime
        ConvertDate(filelistItem.CreationTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        Ret=TVar((const char*)Date);
        break;
      case 4:  // AccessTime
        ConvertDate(filelistItem.AccessTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        Ret=TVar((const char*)Date);
        break;
      case 5:  // WriteTime
        ConvertDate(filelistItem.WriteTime,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        strcat(Date," ");
        strcat(Date,Time);
        Ret=TVar((const char*)Date);
        break;
      case 6:  // UnpSize
        Ret=TVar((__int64)MKUINT64(filelistItem.UnpSizeHigh,filelistItem.UnpSize));
        break;
      case 7:  // PackSize
        Ret=TVar((__int64)MKUINT64(filelistItem.PackSizeHigh,filelistItem.PackSize));
        break;
      case 8:  // Selected
        Ret=TVar((__int64)((DWORD)filelistItem.Selected));
        break;
      case 9:  // NumberOfLinks
        Ret=TVar((__int64)filelistItem.NumberOfLinks);
        break;
      case 10:  // SortGroup
        Ret=TVar((__int64)filelistItem.SortGroup);
        break;
      case 11:  // DizText
        Ret=TVar(filelistItem.DizText?filelistItem.DizText:"");
        break;
      case 12:  // Owner
        Ret=TVar((const char*)filelistItem.Owner);
        break;
      case 13:  // CRC32
        Ret=TVar((__int64)filelistItem.CRC32);
        break;
      case 14:  // Position
        Ret=TVar((__int64)filelistItem.Position);
        break;

      case 15:  // CreationTime (FILETIME)
        Ret=TVar((__int64)*(__int64*)&filelistItem.CreationTime);
        break;
      case 16:  // AccessTime (FILETIME)
        Ret=TVar((__int64)*(__int64*)&filelistItem.AccessTime);
        break;
      case 17:  // WriteTime (FILETIME)
        Ret=TVar((__int64)*(__int64*)&filelistItem.WriteTime);
        break;
    }
  }

  VMStack.Push(Ret);
  return false;
}

static bool lenFunc()
{
  VMStack.Push(TVar(strlen(VMStack.Pop().toString())));
  return true;
}

static bool ucaseFunc()
{
  TVar Val=VMStack.Pop();
  LocalStrupr((char *)Val.toString());
  VMStack.Push(Val);
  return true;
}

static bool lcaseFunc()
{
  TVar Val=VMStack.Pop();
  LocalStrlwr((char *)Val.toString());
  VMStack.Push(Val);
  return true;
}

static bool stringFunc()
{
  VMStack.Push(VMStack.Pop().toString());
  return true;
}

static bool intFunc()
{
  VMStack.Push(VMStack.Pop().toInteger());
  return true;
}

static bool absFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isInteger() )
  {
    __int64 v=tmpVar.i();
    if(v < 0)
      v=-v;
    tmpVar = v;
  }
  VMStack.Push(tmpVar);
  return true;
}

static bool ascFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isString() )
  {
    tmpVar = (__int64)((DWORD)((BYTE)*tmpVar.toString()));
    tmpVar.toInteger();
  }
  VMStack.Push(tmpVar);
  return true;
}

static bool chrFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isInteger() )
  {
    __int64 val=tmpVar.i();
    char tmp[2]={0,0};
    tmp[0]=(BYTE)(val&0xFF);
    tmpVar = (const char *)tmp;
    tmpVar.toString();
  }
  VMStack.Push(tmpVar);
  return true;
}

const char *eStackAsString(int)
{
  const char *s=__varTextDate.toString();
  return !s?"":s;
}

int KeyMacro::GetKey()
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::GetKey()"));
  //_KEYMACRO(SysLog("InternalInput=%d Executing=%d (CurrentFrame=%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
  struct MacroRecord *MR;
  TVar tmpVar;
  TVarSet *tmpVarSet=NULL;

  if (InternalInput || !FrameManager->GetCurrentFrame())
    return 0;

  int RetKey=0;  // функция должна вернуть 0 - сигнал о том, что макропоследовательности нет

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
  if((MR=Work.MacroWORK) == NULL || !MR->Buffer)
    return 0;

  _KEYMACRO(SysLog("KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

  // ВНИМАНИЕ! Возможны глюки!
  if(!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
    LockScr=new LockScreen;

begin:
  _KEYMACRO(SysLog("begin: (CurPCStack=%d), Work.ExecLIBPos=%d MR->BufferSize=%d",CurPCStack,Work.ExecLIBPos,MR->BufferSize));
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
    if(CurPCStack < 0 && (Work.MacroWORKCount-1) <= 0)  // mantis#351
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
    _KEYMACRO(SysLog(-1); SysLog("**** End Of Execute Macro **** (CurPCStack=%d)",CurPCStack));
    if(Work.MacroWORKCount <= 0 && CurPCStack >= 0)
    {
      PopState();
      goto initial;
    }
    return KEY_NONE; // Здесь ВСЕГДА!
  }

  if(Work.ExecLIBPos==0)
    Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;


  // Mantis#0000581: Добавить возможность прервать выполнение макроса
  {
    INPUT_RECORD rec;
    if(PeekInputRecord(&rec) && rec.EventType==KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
    {
    _SVS(SysLog("[%s#%d] KeyMacro::GetKey() ==> call GetInputRecord()",__FILE__,__LINE__));
      _SVS(SysLog("Mantis#0000581"));
      GetInputRecord(&rec,true);  // удаляем из очереди эту "клавишу"...
      Work.KeyProcess=0;
      goto done;                  // ...и завершаем макрос.
    }
  }


  DWORD Key=GetOpCode(MR,Work.ExecLIBPos++);
  _KEYMACRO(SysLog("[%d] IP=%d Op=%08X ==> %s",__LINE__,Work.ExecLIBPos-1,Key,((Key&KEY_MACRO_BASE)?_MCODE_ToName(Key):_FARKEY_ToName(Key))));

  if(Work.KeyProcess && Key != MCODE_OP_ENDKEYS)
  {
    _KEYMACRO(SysLog("[%d] IP=%d  %s (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)",__LINE__,Work.ExecLIBPos-1,((Key&KEY_MACRO_BASE)?_MCODE_ToName(Key):_FARKEY_ToName(Key))));
    goto return_func;
  }

  char value[2048];

  switch(Key)
  {
    case MCODE_OP_NOP:
      goto begin;

    case MCODE_OP_KEYS:                    // за этим кодом следуют ФАРовы коды клавиш
    {
      _KEYMACRO(SysLog("MCODE_OP_KEYS"));
      Work.KeyProcess++;
      goto begin;
    }

    case MCODE_OP_ENDKEYS:                 // ФАРовы коды закончились.
    {
      _KEYMACRO(SysLog("MCODE_OP_ENDKEYS"));
      Work.KeyProcess--;
      goto begin;
    }

    case KEY_ALTINS:
    {
      if(RunGraber())
        return KEY_NONE;
      break;
    }

    case MCODE_OP_XLAT:               // $XLat
    {
      return KEY_OP_XLAT;
    }

    case MCODE_OP_SELWORD:            // $SelWord
    {
      return KEY_OP_SELWORD;
    }

    case MCODE_OP_DATE:               // $Date ["format"]
    {
      __varTextDate=VMStack.Pop();
      return KEY_OP_DATE;
    }

    case MCODE_OP_PLAINTEXT:          // $Text "Text"
    {
      if(VMStack.isEmpty())
        return KEY_NONE;
      __varTextDate=VMStack.Pop();
      return KEY_OP_PLAINTEXT;
    }

    case MCODE_OP_EXIT:               // $Exit
      goto done;

    case MCODE_OP_AKEY:               // $AKey
    {
      return MR->Key;
    }

    /* $IClip
       0: MCODE_OP_ICLIP
    */
    case MCODE_OP_ICLIP:              // $IClip
    {
      UsedInternalClipboard=UsedInternalClipboard==0?1:0;
      goto begin;
    }

    case MCODE_OP_SWITCHKBD:          // $KbdSwitch
    {
      if(!hFarWnd)
        InitDetectWindowedMode();
      if(hFarWnd)
      {
        PostMessage(hFarWnd,WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
        //if(Flags & XLAT_SWITCHKEYBBEEP)
        //  MessageBeep(0);
      }
      goto begin;
    }

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
      if((Counter.i64=VMStack.Pop().toInteger()) < 0) // Peek???
        Counter.i64=0;
      SetOpCode(MR,Work.ExecLIBPos+1,Counter.Part.HighPart);
      SetOpCode(MR,Work.ExecLIBPos+2,Counter.Part.LowPart);
      goto begin;
    }

    case MCODE_OP_REP:
    {
      // получим текущее значение счетчика
      FARINT64 Counter;
      Counter.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos);
      Counter.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos+1);
      // и положим его на вершину стека
      VMStack.Push(Counter.i64);
      // уменьшим его и пойдем на MCODE_OP_JZ
      Counter.i64--;
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.HighPart);
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.LowPart);
      goto begin;
    }

    case MCODE_OP_END:
      // просто пропустим этот рудимент синтаксиса :)
      goto begin;

    case MCODE_OP_SAVE:
    {
      TVar Val0=VMStack.Pop();
      GetPlainText(value);
      // здесь проверка нужна, т.к. существует вариант вызова функции, без присвоения переменной
      if(*value)
      {
        TVarTable *t = ( *value == '%' ) ? &glbVarTable : Work.locVarTable;
        varInsert(*t, value)->value = Val0;
      }
      goto begin;
    }

    /* $MMode 1
       0: MCODE_OP_MACROMODE   - в стеке ожидается число
    */
    case MCODE_OP_MACROMODE:          // $MMode 1
      if (Work.ExecLIBPos<MR->BufferSize)
      {
        if(VMStack.Pop().toInteger() == _i64(1)) // Изменяет режим отображения ("DisableOutput").
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

    case MCODE_OP_DISCARD:    // убрать значение с вершины стека
      VMStack.Pop();
      goto begin;

    case MCODE_OP_POP:        // 0: pop 1: varname -> присвоить значение переменной и убрать из вершины стека
    {
      tmpVar=VMStack.Pop();
      GetPlainText(value);
      TVarTable *t = ( *value == '%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
        tmpVarSet->value=tmpVar;
      goto begin;
    }

    /*                               Вместо
        0: MCODE_OP_COPY                 0:   MCODE_OP_PUSHVAR
        1: VarDest                       1:   VarSrc
        ...                              ...
        N: VarSrc                        N:   MCODE_OP_DOIT
        ...                            N+1:   MCODE_OP_SAVE
                                       N+2:   VarDest
                                         ...

    */

    case MCODE_OP_COPY:       // 0: Copy 1: VarDest 2: VarSrc ==>  %a=%d
    {
      GetPlainText(value);
      TVarTable *t = ( *value == '%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
         tmpVar=tmpVarSet->value;
      GetPlainText(value);
      t = ( *value == '%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
        tmpVar=tmpVarSet->value;
      goto begin;
    }

    case MCODE_OP_PUSHINT:  // Положить целое значение на стек.
    {
      FARINT64 i64;
      i64.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos++);   //???
      i64.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos++);    //???
      VMStack.Push(i64.i64);
      goto begin;
    }

    case MCODE_OP_PUSHCONST:  // Положить на стек константу.
    {
      GetPlainText(value);
      tmpVarSet=varLook(glbConstTable, value);
      if(tmpVarSet)
        VMStack.Push(tmpVarSet->value);
      else
        VMStack.Push(_i64(0));
      goto begin;
    }

    case MCODE_OP_PUSHVAR:  // Положить на стек переменную.
    {
      GetPlainText(value);
      TVarTable *t = ( *value == '%' ) ? &glbVarTable : Work.locVarTable;
      // %%name - глобальная переменная
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
        VMStack.Push(tmpVarSet->value);
      else
        VMStack.Push(_i64(0));
      goto begin;
    }

    case MCODE_OP_PUSHSTR: // Положить на стек строку-константу.
      GetPlainText(value);
      VMStack.Push(TVar(value));
      goto begin;

    // переходы
    case MCODE_OP_JMP:
      Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      goto begin;

    case MCODE_OP_JZ:
      if ( VMStack.Pop().toInteger() == 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JNZ:
      if ( VMStack.Pop().toInteger() != 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JLT:
      if ( VMStack.Pop().toInteger() < 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JLE:
      if ( VMStack.Pop().toInteger() <= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JGT:
      if ( VMStack.Pop().toInteger() > 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JGE:
      if ( VMStack.Pop().toInteger() >= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    // операции
    case MCODE_OP_NEGATE: VMStack.Push(-VMStack.Pop()); goto begin;
    case MCODE_OP_NOT:    VMStack.Push(!VMStack.Pop()); goto begin;

    case MCODE_OP_LT:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() <  tmpVar); goto begin;
    case MCODE_OP_LE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() <= tmpVar); goto begin;
    case MCODE_OP_GT:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >  tmpVar); goto begin;
    case MCODE_OP_GE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >= tmpVar); goto begin;
    case MCODE_OP_EQ:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() == tmpVar); goto begin;
    case MCODE_OP_NE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() != tmpVar); goto begin;

    case MCODE_OP_ADD:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() +  tmpVar); goto begin;
    case MCODE_OP_SUB:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() -  tmpVar); goto begin;
    case MCODE_OP_MUL:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() *  tmpVar); goto begin;
    case MCODE_OP_DIV:
      if(VMStack.Peek() == _i64(0)) //???
      {
        _KEYMACRO(SysLog("[%d] IP=%d/0x%08X Error: Divide by zero",__LINE__,Work.ExecLIBPos,Work.ExecLIBPos));
        goto done;
      }
      tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() /  tmpVar);
      goto begin;

    // Logical
    case MCODE_OP_AND:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() && tmpVar); goto begin;
    case MCODE_OP_OR:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() || tmpVar); goto begin;

    // Bit Op
    case MCODE_OP_BITAND: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() &  tmpVar); goto begin;
    case MCODE_OP_BITOR:  tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() |  tmpVar); goto begin;
    case MCODE_OP_BITXOR: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() ^  tmpVar); goto begin;
    case MCODE_OP_BITSHR: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >> tmpVar); goto begin;
    case MCODE_OP_BITSHL: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() << tmpVar); goto begin;

    case MCODE_OP_BITNOT: VMStack.Push(~VMStack.Pop()); goto begin;

    // Function
    case MCODE_F_EVAL: // N=eval(S)
    {
      if(evalFunc())
        goto initial; // т.к.
      goto begin;
    }

    case MCODE_F_AKEY: // V=akey(N)
    {
      tmpVar=VMStack.Pop();
      if(tmpVar.i() == 0)
         tmpVar=(__int64)MR->Key;
      else
      {
         ::KeyToText(MR->Key,value);
         tmpVar=(const char *)value;
         tmpVar.toString();
      }
      VMStack.Push(tmpVar);
      goto begin;
    }

    case MCODE_F_BM_ADD:              // N=BM.Add()
    case MCODE_F_BM_CLEAR:            // N=BM.Clear()
    case MCODE_F_BM_NEXT:             // N=BM.Next()
    case MCODE_F_BM_PREV:             // N=BM.Prev()
    case MCODE_F_BM_STAT:             // N=BM.Stat([N])
    case MCODE_F_BM_DEL:              // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=0...), -1 - удаляет текущую закладку
    case MCODE_F_BM_GET:              // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=0...)
    {
       TVar p1, p2;
       if(Key == MCODE_F_BM_GET)
         p2=VMStack.Pop();
       if(Key == MCODE_F_BM_GET || Key == MCODE_F_BM_DEL || Key == MCODE_F_BM_GET)
         p1=VMStack.Pop();

       __int64 Result=_i64(0);
       Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
       while(f)
       {
         fo=f;
         f=f->GetTopModal();
       }
       if(!f)
         f=fo;

       if(f)
         Result=f->VMProcess(Key,(void*)(INT_PTR)p2.i(),p1.i());

       VMStack.Push(Result);
       goto begin;
    }

    case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
    {
       _KEYMACRO(CleverSysLog Clev("MCODE_F_MENU_GETHOTKEY"));
       tmpVar=VMStack.Pop();
       if(!tmpVar.isInteger())
         tmpVar=_i64(0);

       int CurMMode=CtrlObject->Macro.GetMode();
       if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS || CurMMode == MACRO_USERMENU)
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

         __int64 Result;

         if(f && (Result=f->VMProcess(MCODE_F_MENU_GETHOTKEY,NULL,tmpVar.i()-1)) != 0)
         {
           value[0]=value[1]=0;
           if(Result)
             value[0]=(char)Result;
           tmpVar=(const char *)value;
         }
         else
           tmpVar=(const char *)"";
       }
       else
         tmpVar=(const char *)"";

       VMStack.Push(tmpVar);
       goto begin;
    }

    case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N])
    case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S)
    {
       _KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_CHECKHOTKEY? "MCODE_F_MENU_CHECKHOTKEY":"MCODE_F_MENU_SELECT"));
       __int64 Result=_i64(-1);
       __int64 tmpMode=_i64(0);
       if(Key == MCODE_F_MENU_SELECT)
       {
         tmpMode=VMStack.Pop().toInteger();
       }
       tmpVar=VMStack.Pop();
       const char *checkStr=tmpVar.toString();
       int CurMMode=CtrlObject->Macro.GetMode();
       if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS || CurMMode == MACRO_USERMENU)
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
           Result=f->VMProcess(Key,(void*)checkStr,tmpMode);
       }
       VMStack.Push(Result);
       goto begin;
    }

    case MCODE_F_PROMPT:  // S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
    case MCODE_F_MSGBOX:  // N=msgbox("Title","Text"[,flags])
    {
        _KEYMACRO(CleverSysLog Clev(Key == MCODE_F_PROMPT?"MCODE_F_PROMPT":"MCODE_F_MSGBOX"));
        DWORD Flags=MR->Flags;
        if(Flags&MFLAGS_DISABLEOUTPUT) // если был - удалим
        {
          if(LockScr) delete LockScr;
          LockScr=NULL;
        }
        InternalInput++; // InternalInput - ограничитель того, чтобы макрос не продолжал свое исполнение
        if(Key == MCODE_F_PROMPT)
          promptFunc();
        else
          msgBoxFunc();
        InternalInput--;
        if(Flags&MFLAGS_DISABLEOUTPUT) // если стал - залочим
        {
          if(LockScr) delete LockScr;
          LockScr=new LockScreen;
        }
        goto begin;
    }

    //
    default:
    {
      static struct TMCode2Func{
        DWORD Op;
        bool (*Func)();
      } MCode2Func[]={
        {MCODE_F_WAITKEY,waitkeyFunc},  // V=waitkey([N,[T]])
        {MCODE_F_ITOA,itoaFunc}, // S=itoa(N[,radix])
        {MCODE_F_ATOI,atoiFunc}, // N=atoi(S[,radix])
        {MCODE_F_MIN,minFunc},  // N=min(N1,N2)
        {MCODE_F_MOD,modFunc},  // N=mod(N1,N2)
        {MCODE_F_MAX,maxFunc},  // N=max(N1,N2)
        {MCODE_F_IIF,iifFunc},  // V=iif(Condition,V1,V2)
        {MCODE_F_SUBSTR,substrFunc}, // S=substr(S,N1,N2)
        {MCODE_F_TRIM,trimFunc}, // S=trim(S[,N])
        {MCODE_F_RINDEX,rindexFunc}, // S=rindex(S1,S2)
        {MCODE_F_INDEX,indexFunc}, // S=index(S1,S2)
        {MCODE_F_PANELITEM,panelitemFunc},  // V=panelitem(Panel,Index,TypeInfo)
        {MCODE_F_PANEL_SETPOS,panelsetposFunc}, // N=Panel.SetPos(panelType,fileName)
        {MCODE_F_PANEL_SETPATH,panelsetpathFunc}, // N=panel.SetPath(panelType,pathName,fileName)
        {MCODE_F_PANEL_SETPOSIDX,panelsetposidxFunc}, // N=Panel.SetPosIdx(panelType,Idx)
        {MCODE_F_PANEL_FATTR,panelfattrFunc},         // N=Panel.FAttr(panelType,fileMask)
        {MCODE_F_PANEL_FEXIST,panelfexistFunc},        // N=Panel.FExist(panelType,fileMask)
        {MCODE_F_SLEEP,sleepFunc}, // N=Sleep(N)
        {MCODE_F_ENVIRON,environFunc}, // S=env(S)
        {MCODE_F_LEN,lenFunc},  // N=len(S)
        {MCODE_F_UCASE,ucaseFunc}, // S=ucase(S1)
        {MCODE_F_LCASE,lcaseFunc}, // S=lcase(S1)
        {MCODE_F_FEXIST,fexistFunc},  // S=fexist(S)
        {MCODE_F_FLOCK,flockFunc},  // N=FLock(N,N)
        {MCODE_F_FSPLIT,fsplitFunc},  // S=fsplit(S,N)
        {MCODE_F_FATTR,fattrFunc},   // N=fattr(S)
        {MCODE_F_MSAVE,msaveFunc},   // N=msave(S)
        {MCODE_F_DLG_GETVALUE,dlggetvalueFunc},        // V=Dlg.GetValue(ID,N)
        {MCODE_F_EDITOR_SEL,editorselFunc}, // V=Editor.Sel(Action[,Opt])
        {MCODE_F_EDITOR_SET,editorsetFunc}, // N=Editor.Set(N,Var)
        {MCODE_F_STRING,stringFunc},  // S=string(V)
        {MCODE_F_CLIP,clipFunc}, // V=Clip(N[,S])
        {MCODE_F_INT,intFunc}, // N=int(V)
        {MCODE_F_DATE,dateFunc},  // // S=date(S)
        {MCODE_F_XLAT,xlatFunc}, // S=xlat(S)
        {MCODE_F_ABS,absFunc}, // N=abs(N)
        {MCODE_F_ASC,ascFunc}, // N=asc(S)
        {MCODE_F_CHR,chrFunc}, // S=chr(N)
        {MCODE_F_REPLACE,replaceFunc}, // S=replace(sS,sF,sR)
        {MCODE_F_CALLPLUGIN,callpluginFunc}, // V=callplugin(SysID[,param])
        {MCODE_F_REG_GET,reggetFunc}, // V=reg.get(iRoot, "Key"[, "Value"])
        {MCODE_F_KEY,keyFunc}, // S=key(V)
      };
      int J;
      for(J=0; J < sizeof(MCode2Func)/sizeof(MCode2Func[0]); ++J)
        if(MCode2Func[J].Op == Key)
        {
          MCode2Func[J].Func();
          break;
        }

      if(J >= sizeof(MCode2Func)/sizeof(MCode2Func[0]))
      {
        DWORD Err=0;
        tmpVar=FARPseudoVariable(MR->Flags, Key, Err);
        if(!Err)
          VMStack.Push(tmpVar);
        else
        {
          break; // клавиши будем возвращать
        }
      }
      goto begin;
    } // END default

  } // END switch(Key)

return_func:

  if(Work.KeyProcess && (Key&KEY_ALTDIGIT)) // "подтасовка" фактов ;-)
  {
    Key&=~KEY_ALTDIGIT;
    ReturnAltValue=1;
  }

#if 0
  if(MR==Work.MacroWORK &&
      ( Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE && Mode==MACRO_DIALOG)
    )
  {
    RetKey=Key;
    goto done;
  }
#else
  if(MR==Work.MacroWORK && Work.ExecLIBPos>=MR->BufferSize)
  {
    _KEYMACRO(SysLog(-1);SysLog("[%d] **** End Of Execute Macro ****",__LINE__));
    ReleaseWORKBuffer();
    Work.Executing=MACROMODE_NOMACRO;
    if(TitleModified)
      SetFarTitle(NULL);
  }
#endif

  return(Key);
}

// Проверить - есть ли еще клавиша?
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

  if(!Buffer)
    return NULL;

  // выделяем заведомо большой кусок
  if((TextBuffer=(char*)xf_malloc(BufferSize*64+16)) == NULL)
    return NULL;

  TextBuffer[0]=0;
#if 0
  if(BufferSize == 1)
  {
    if(
        (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
        (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) >= KEY_OP_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE
      )
    {
      xf_free(TextBuffer);
      return Src?xf_strdup(Src):NULL;
    }

    if(KeyToText((DWORD)(DWORD_PTR)Buffer,MacroKeyText))
      strcpy(TextBuffer,MacroKeyText);
    return TextBuffer;
  }
#endif
  if(Buffer[0] == MCODE_OP_KEYS)
    for (J=1; J < BufferSize; J++)
    {
      Key=Buffer[J];

      if(Key == MCODE_OP_ENDKEYS)
        continue;

      if(
         /*(
           (Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
           (Key&KEY_OP_ENDBASE)>=KEY_OP_BASE && (Key&KEY_OP_ENDBASE) <=KEY_OP_ENDBASE
         ) ||*/
         !KeyToText(Key,MacroKeyText))
      {
        xf_free(TextBuffer);
        return Src?xf_strdup(Src):NULL;
      }
      if(J > 1)
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
  //WriteVarsConst(MACRO_CONSTS);

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
    BOOL Ok=TRUE;
    if(MacroLIB[I].Flags&MFLAGS_REG_MULTI_SZ)
    {
      int Len=(int)strlen(MacroLIB[I].Src)+2;
      char *ptrSrc=(char *)xf_malloc(Len);
      if(ptrSrc)
      {
        strcpy(ptrSrc,MacroLIB[I].Src);
        for(int J=0; ptrSrc[J]; ++J)
          if(ptrSrc[J] == '\n')
            ptrSrc[J]=0;
        ptrSrc[Len-1]=0;
        SetRegKey(RegKeyName,"Sequence",ptrSrc,Len,REG_MULTI_SZ);
        free(ptrSrc);
        Ok=FALSE;
      }
    }

    if(Ok)
      SetRegKey(RegKeyName,"Sequence",MacroLIB[I].Src);

    if(MacroLIB[I].Description)
      SetRegKey(RegKeyName,"Description",MacroLIB[I].Description);
    else
      DeleteRegValue(RegKeyName,"Description");

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


int KeyMacro::WriteVarsConst(int WriteMode)
{
  char UpKeyName[100];
  char ValueName[129];
  char *ptrValueName=ValueName;

  *ValueName=0;
  if(WriteMode==MACRO_VARS)
  {
    *ValueName='%';
    ptrValueName=ValueName+1;
  }


  sprintf(UpKeyName,"KeyMacros\\%s",(WriteMode==MACRO_VARS?"Vars":"Consts"));

  TVarTable *t = (WriteMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

  for (int I=0;I < V_TABLE_SIZE;I++)
    for(int J=0;;++J)
    {
      TVarSet *var=varEnum(*t,I,J);
      if(!var)
        break;
      xstrncpy(ptrValueName,var->str,sizeof(ValueName)-2);
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
  int I;
  char UpKeyName[100];
  char ValueName[129];
  char *ptrValueName=ValueName;
  long IData;
  __int64 IData64;

  if(ReadMode==MACRO_VARS)
    ptrValueName=ValueName+1;

  sprintf(UpKeyName,"KeyMacros\\%s",(ReadMode==MACRO_VARS?"Vars":"Consts"));
  TVarTable *t = (ReadMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

  for (I=0;;I++)
  {
    IData=0;
    *ValueName=0;
    *SData=0;

    int Type=EnumRegValue(UpKeyName,I,ValueName,sizeof(ValueName),(LPBYTE)SData,SDataSize,(LPDWORD)&IData,(__int64*)&IData64);

    if (Type == REG_NONE)
      break;

    if(ReadMode==MACRO_VARS && *ValueName != '%' && ValueName[1] != '%')
      continue;

    if (Type == REG_SZ)
      varInsert(*t, ptrValueName)->value = SData;
    else if (Type == REG_DWORD)
      varInsert(*t, ptrValueName)->value = (__int64)IData;
    else if (Type == REG_QWORD)
      varInsert(*t, ptrValueName)->value = IData64;
  }

  if(ReadMode == MACRO_CONSTS)
  {
    SetMacroConst(constRegError,_i64(0));
    SetMacroConst(constMsX,_i64(0));
    SetMacroConst(constMsY,_i64(0));
    SetMacroConst(constMsButton,_i64(0));
    SetMacroConst(constMsCtrlState,_i64(0));
  }
  return TRUE;
}


void KeyMacro::SetMacroConst(const char *ConstName, const TVar Value)
{
  varLook(glbConstTable, ConstName,1)->value = Value;
}

/*
   KeyMacros\Function
*/
int KeyMacro::ReadMacroFunction(int ReadMode, char *SData, int SDataSize)
{
  if(ReadMode != MACRO_FUNC) // пока так :-)
    return FALSE;
#if 0
  int I;
  char UpKeyName[100];
  char ValueName[129];
  long IData;
  __int64 IData64;

  strcpy(UpKeyName,"KeyMacros\\Function");

  for (I=0;;I++)
  {
    IData=0;
    *ValueName=0;
    *SData=0;

    int Type=EnumRegValue(UpKeyName,I,ValueName,sizeof(ValueName),(LPBYTE)SData,SDataSize,(LPDWORD)&IData,(__int64*)&IData64);
  }
#endif
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

    DWORD regType=0;
    if(GetRegKey(RegKeyName,"Sequence",Buffer,"",BufferSize,&regType) && regType == REG_MULTI_SZ)
    {
      // Различаем так же REG_MULTI_SZ
      char *ptrBuffer=Buffer;
      while(1)
      {
        ptrBuffer+=(int)strlen(ptrBuffer);
        if(!ptrBuffer[0] && !ptrBuffer[1])
          break;
        *ptrBuffer='\n';
      }
    }

    RemoveExternalSpaces(Buffer);
    if(!strlen(Buffer))
      continue;

    CurMacro.Key=KeyCode;
    CurMacro.Buffer=NULL;
    CurMacro.Src=NULL;
    CurMacro.Description=NULL;
    CurMacro.BufferSize=0;
    CurMacro.Flags=MFlags|(ReadMode&MFLAGS_MODEMASK)|(regType == REG_MULTI_SZ?MFLAGS_REG_MULTI_SZ:0);

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
    J=GetRegKeySize(RegKeyName,"Description");
    if(J)
    {
      if((CurMacro.Description=(char *)xf_malloc(J+1)) != NULL)
      {
        if(!GetRegKey(RegKeyName,"Description",CurMacro.Description,"",J,&regType))
        {
          xf_free(CurMacro.Description);
          CurMacro.Description=NULL;
        }
      }
    }
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
  if(Opt.DisableMacro&MDOL_ALL)
    return;

  if(Opt.DisableMacro&MDOL_AUTOSTART)
    return;

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
  if (Work.Executing == MACROMODE_NOMACRO)
    Work.ExecLIBPos=0;  // А надо ли?
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
    };

    for(I=0; I < sizeof(PreDefKeyName)/sizeof(PreDefKeyName[0]); ++I)
      Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(LONG_PTR)PreDefKeyName[I]);

    static DWORD PreDefKey[]={
      KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
      KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
      #if 0
      KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
      #endif
    };
    static DWORD PreDefModKey[]={
      0,KEY_CTRL,KEY_SHIFT,KEY_ALT,KEY_CTRLSHIFT,KEY_CTRLALT,KEY_ALTSHIFT,
    };

    for(I=0; I < sizeof(PreDefKey)/sizeof(PreDefKey[0]); ++I)
    {
      Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(LONG_PTR)"\1");
      for(int J=0; J < sizeof(PreDefModKey)/sizeof(PreDefModKey[0]); ++J)
      {
        KeyToText(PreDefKey[I]|PreDefModKey[J],KeyText);
        Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(LONG_PTR)KeyText);
      }
    }
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
       (Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && (LastKey == KEY_ENTER||LastKey == KEY_NUMENTER) ||
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
    if((Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && LastKey && !(LastKey == KEY_ENTER||LastKey == KEY_NUMENTER))
      return FALSE;
    // </Обработка особых клавиш: F1 & Enter>
M1:
    KeyMacro *MacroDlg=KMParam->Handle;

    if((Param2&0x00FFFFFF) > 0x7F && (Param2&0x00FFFFFF) < 0xFF)
      Param2=LocalKeyToKey((int)(Param2&0x000000FF)|(int)(Param2&(~0x000000FF)));

    KMParam->Key=(DWORD)Param2;
    KeyToText((int)Param2,KeyText);

    // если УЖЕ есть такой макрос...
    if((Index=MacroDlg->GetIndex((int)Param2,KMParam->Mode)) != -1)
    {
      struct MacroRecord *Mac=MacroDlg->MacroLIB+Index;

      // общие макросы учитываем только при удалении.
      if(!MacroDlg->RecBuffer || !MacroDlg->RecBufferSize || (Mac->Flags&0xFF)!=MACRO_COMMON)
      {
        DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
        char Buf[512];
        char BufKey[512];
        char RegKeyName[150];
        MacroDlg->MkRegKeyName(Index,RegKeyName);

#if 0
        char *TextBuffer;

        // берем из памяти.
        if((TextBuffer=MacroDlg->MkTextSequence(Mac->Buffer,Mac->BufferSize)) != NULL)
        {
          int F=0;
          I=strlen(TextBuffer);
          if(I > sizeof(Buf)-6) { I=sizeof(Buf)-6; F++; }
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
          I=(int)strlen(Mac->Src);
          if(I > sizeof(Buf)-6) { I=sizeof(Buf)-6; F++; }
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
             Mac->BufferSize == 1 && (DWORD)(DWORD_PTR)Mac->Buffer == (DWORD)(DWORD_PTR)MacroDlg->RecBuffer
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
    }
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)KeyText);
//    if(Param2 == KEY_F1 && LastKey == KEY_F1)
//      LastKey=-1;
//    else
      LastKey=(int)Param2;
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
  /* 01 */ DI_TEXT,-1,2,0,2,0,0,DIF_BOXCOLOR|DIF_READONLY,0,(char *)MDefineMacro,
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
          FileEditor ShellEditor(MacroFileName,FFILEEDIT_DISABLEHISTORY,-1,-1,NULL);
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
  /* 01 */DI_CHECKBOX,5,2,0,2,1,0,0,0,(char *)MMacroSettingsEnableOutput,
  /* 02 */DI_CHECKBOX,5,3,0,3,0,0,0,0,(char *)MMacroSettingsRunAfterStart,
  /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_CHECKBOX,5,5,0,5,0,0,0,0,(char *)MMacroSettingsActivePanel,
  /* 05 */DI_CHECKBOX,7,6,0,6,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsPluginPanel,
  /* 06 */DI_CHECKBOX,7,7,0,7,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsFolders,
  /* 07 */DI_CHECKBOX,7,8,0,8,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsSelectionPresent,
  /* 08 */DI_CHECKBOX,37,5,0,5,0,0,0,0,(char *)MMacroSettingsPassivePanel,
  /* 09 */DI_CHECKBOX,39,6,0,6,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsPluginPanel,
  /* 10 */DI_CHECKBOX,39,7,0,7,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsFolders,
  /* 11 */DI_CHECKBOX,39,8,0,8,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)MMacroSettingsSelectionPresent,
  /* 12 */DI_TEXT,3,9,0,9,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 13 */DI_CHECKBOX,5,10,0,10,0,2,DIF_3STATE,0,(char *)MMacroSettingsCommandLine,
  /* 14 */DI_CHECKBOX,5,11,0,11,0,2,DIF_3STATE,0,(char *)MMacroSettingsSelectionBlockPresent,
  /* 15 */DI_TEXT,3,12,0,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 16 */DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 17 */DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,0,(char *)MCancel
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

int KeyMacro::PostNewMacro(const char *PlainText,DWORD Flags,DWORD AKey,BOOL onlyCheck)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::PostNewMacro(char *PlainText,DWORD Flags)"));
  _KEYMACRO(SysLog("Param: PlainText=\"%s\"",PlainText));
  struct MacroRecord NewMacroWORK2={0};
  char *Buffer=(char *)PlainText;
  bool allocBuffer=false;

  if(Flags&MFLAGS_REG_MULTI_SZ) // Различаем так же REG_MULTI_SZ
  {
    int lenPlainText=0;
    while(1)
    {
      if(!PlainText[lenPlainText] && !PlainText[lenPlainText+1])
      {
        lenPlainText+=2;
        break;
      }
      lenPlainText++;
    }
    //lenPlainText++;

    Buffer=(char*)xf_malloc(lenPlainText+1);
    if(Buffer)
    {
      allocBuffer=true;
      memmove(Buffer,PlainText,lenPlainText);
      Buffer[lenPlainText]=0; // +1
      char *ptrBuffer=Buffer;
      while(1)
      {
        ptrBuffer+=strlen(ptrBuffer);
        if(!ptrBuffer[0] && !ptrBuffer[1])
          break;
        *ptrBuffer='\n';
      }
    }
    else
      return FALSE;
  }

  // сначала смотрим на парсер
  BOOL parsResult=ParseMacroString(&NewMacroWORK2,Buffer);
  if(allocBuffer && Buffer)
    free(Buffer);
  if(!parsResult)
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }

  if(onlyCheck)
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return TRUE;
  }

  NewMacroWORK2.Flags=Flags;
  NewMacroWORK2.Key=AKey;

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

int KeyMacro::PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag,BOOL IsPluginSend)
{
  _KEYMACRO(CleverSysLog Clev("KeyMacro::PostNewMacro(struct MacroRecord *MRec)"));
  _KEYMACRO(SysLog("Param: MRec=%p",MRec));
  if(!MRec)
    return FALSE;

  struct MacroRecord NewMacroWORK2={0};
  memcpy(&NewMacroWORK2,MRec,sizeof(struct MacroRecord));
  NewMacroWORK2.Src=NULL;
  NewMacroWORK2.Description=NULL;

//  if(MRec->BufferSize > 1)
  {
    if((NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+3)*sizeof(DWORD))) == NULL)
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
  if(IsPluginSend)
    NewMacroWORK2.Buffer[0]=MCODE_OP_KEYS;

  if((MRec->BufferSize+1) > 2)
    memcpy(&NewMacroWORK2.Buffer[IsPluginSend?1:0],MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
  else if(MRec->Buffer)
    NewMacroWORK2.Buffer[IsPluginSend?1:0]=(DWORD)(DWORD_PTR)MRec->Buffer;

  if(IsPluginSend)
    NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize+1]=MCODE_OP_ENDKEYS;
  //NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize]=MCODE_OP_NOP; // доп.клавиша/пустышка

  if(IsPluginSend)
    NewMacroWORK2.BufferSize+=2;

  Work.MacroWORK=NewMacroWORK;
  NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
  memcpy(NewMacroWORK,&NewMacroWORK2,sizeof(struct MacroRecord));
  Work.MacroWORKCount++;

//  Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
  if(Work.ExecLIBPos == Work.MacroWORK->BufferSize)
    Work.ExecLIBPos=0;
  return TRUE;
}


int KeyMacro::ParseMacroString(struct MacroRecord *CurMacro,const char *BufPtr)
{
  if ( CurMacro )
    return __parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);
  return FALSE;
}


// Функция получения индекса нужного макроса в массиве
// Ret=-1 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
int KeyMacro::GetIndex(int Key, int ChechMode, bool UseCommon)
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
      else if(ChechMode >= MACRO_OTHER && ChechMode < MACRO_LAST)
      {
        Len=IndexMode[ChechMode][1];
        if(Len)
          MPtr=MacroLIB+IndexMode[ChechMode][0];
  //_SVS(SysLog("ChechMode=%d (%d,%d)",ChechMode,IndexMode[ChechMode][0],IndexMode[ChechMode][1]));
      }
      else
        Len=0;

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
      if(ChechMode != -1 && !I && UseCommon)
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

int KeyMacro::GetMacroKeyInfo(bool FromReg,int Mode,int Pos,char *KeyName,char *Description,int DescriptionSize)
{
  if(Mode >= MACRO_OTHER && Mode < MACRO_LAST)
  {
    char UpKeyName[100];
    char RegKeyName[150];
    if(FromReg)
    {
      sprintf(UpKeyName,"KeyMacros\\%s",GetSubKey(Mode));

      if (!EnumRegKey(UpKeyName,Pos,RegKeyName,sizeof(RegKeyName)))
        return -1;

      char *KeyNamePtr=strrchr(RegKeyName,'\\');
      if (KeyNamePtr!=NULL)
        strcpy(KeyName,KeyNamePtr+1);
      GetRegKey(RegKeyName,"Description",Description,"",DescriptionSize);
      return Pos+1;
    }
    else
    {
      int Len=CtrlObject->Macro.IndexMode[Mode][1];
      if(Len && Pos < Len)
      {
        struct MacroRecord *MPtr=CtrlObject->Macro.MacroLIB+CtrlObject->Macro.IndexMode[Mode][0]+Pos;
        ::KeyToText(MPtr->Key,KeyName);
        xstrncpy(Description,NullToEmpty(MPtr->Description),DescriptionSize);
        return Pos+1;
      }
    }
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
    int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
    Frame* CurFrame=FrameManager->GetCurrentFrame();
    if (CurFrame && CurFrame->GetType()==NeedType)
    {
      int CurSelected;
      if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
        CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
      else
        CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

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
  DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)(DWORD_PTR)MR->Buffer;
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
    OldOpCode=(DWORD)(DWORD_PTR)MR->Buffer;
    MR->Buffer=(DWORD*)(DWORD_PTR)OpCode;
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


void MacroState::Init(TVarTable *tbl)
{
  KeyProcess=Executing=MacroPC=ExecLIBPos=MacroWORKCount=0;
  MacroWORK=NULL;
  if(!tbl)
  {
    AllocVarTable=true;
    locVarTable=(TVarTable*)xf_malloc(sizeof(TVarTable));
    initVTable(*locVarTable);
  }
  else
  {
    AllocVarTable=false;
    locVarTable=tbl;
  }
}


int KeyMacro::PushState(bool CopyLocalVars)
{
  if(CurPCStack+1 >= STACKLEVEL)
    return FALSE;
  ++CurPCStack;
  Work.UsedInternalClipboard=::UsedInternalClipboard;
  memcpy(PCStack+CurPCStack,&Work,sizeof(struct MacroState));
  Work.Init(CopyLocalVars?PCStack[CurPCStack].locVarTable:NULL);
  return TRUE;
}

int KeyMacro::PopState()
{
  if(CurPCStack < 0)
    return FALSE;
  memcpy(&Work,PCStack+CurPCStack,sizeof(struct MacroState));
  ::UsedInternalClipboard=Work.UsedInternalClipboard;
  CurPCStack--;
  return TRUE;
}


bool checkMacroConst(const char *name)
{
   _KEYMACRO_PARSE(CleverSysLog Clev("checkMacroConst"));
   _KEYMACRO_PARSE(SysLog("name='%s'",name));
   return !varLook(glbConstTable, name)?false:true;
}

void initMacroVarTable(int global)
{
  if(global)
  {
    initVTable(glbVarTable);
    initVTable(glbConstTable); //???
  }
}

void doneMacroVarTable(int global)
{
  if(global)
  {
    deleteVTable(glbVarTable);
    deleteVTable(glbConstTable); //???
  }
}

BOOL KeyMacro::GetMacroParseError(char *ErrMsg1,char *ErrMsg2,char *ErrMsg3)
{
  return __getMacroParseError(ErrMsg1,ErrMsg2,ErrMsg3);
}

// это OpCode (за исключением MCODE_OP_ENDKEYS)?
bool KeyMacro::IsOpCode(DWORD p)
{
  return (!(p&KEY_MACRO_BASE) || p == MCODE_OP_ENDKEYS)?false:true;
}
