/*
macro.cpp

Макросы

*/

/* Revision: 1.18 09.01.2001 $ */

/*
Modify:
  09.01.2001 SVS
    + Учтем правило Opt.ShiftsKeyRules (WaitInFastFind)
  04.01.2001 SVS
    ! Конкретная переделка некоторых интересненьких функций :-)
    ! Новый диалог назначений клавиши
    ! дополнительные макро - про часы
    + Исключения "NoFolders" и "NoFiles"
  28.12.2000 SVS
    - Бага с исключениями про панели.
  26.12.2000 SVS
    + KeyMacroToText()
    ! Скинируем до END_FARKEY_BASE
    + Обработка спец-макроклавиш.
  25.12.2000 SVS
    ! MFLAGS_ вернулись из plugin.hpp
  23.12.2000 SVS
    ! MFLAGS_ переехали в plugin.hpp
    + int KeyMacro::ParseMacroString(struct MacroRecord *CurMacro,char *BufPtr)
    + int KeyMacro::PlayKeyMacro(struct MacroRecord *MRec)
    + int KeyMacro::PlayKeyMacro(char *KeyBuffer)
  22.12.2000 SVS
    - Отвлекли - забыл сбросить новые 2 флага :-(
  22.12.2000 SVS
    - При неправильно выбранных названиях начинается суматоха :-(
      После 333 патча перестали работать макросы ВЕЗДЕ!
  21.12.2000 SVS
    ! 3-е состояние для типа панелей.
    + LoadMacros(), InitVars(), ReleaseTempBuffer()
    ! ReadMacros - возвращает TRUE или FALSE (все зависит от выделения памяти)
    + TempMacroType, TempMacro - будут использоваться для команд
      MCMD_PLAYRECORD, MCMD_PLAYSTRING.
  21.12.2000 SVS
    - неверно работало считывание параметров по новым ключам
      FilePanels и PluginPanels
  21.12.2000 SVS
    + "убираем утечку памяти  (LockScr)
    ! Функция KeyToText удалена за ненадобностью
    ! структура MacroRecord "сжата"
    + 2 режима работы макросов в панелях:
       MFLAGS_PLUGINPANEL - работаем на панели плагинов
       MFLAGS_FILEPANEL - работаем на файловой панели
      по умолчанию они включены.
  27.09.2000 SKV
    - Don't redraw editor after macro finished if it is hidden.
  10.09.2000 SVS
    ! Исправим ситуацию с макросами в связи с переработаными кодами клавиш
    ! Функция ReadMacros имеет дополнительные аргументы
  10.08.2000 skv
    ! Изменение а GetKey для отработки окончания макро в Едиторе.
  25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав
      FSF
  23.07.2000 SVS
    + Клавиши:
       Ctrl- Shift- Alt- CtrlShift- AltShift- CtrlAlt- Apps :-)
       KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

#define MFLAGS_MODEMASK            0x0000FFFF
#define MFLAGS_DISABLEOUTPUT       0x00010000
#define MFLAGS_RUNAFTERSTART       0x00020000
#define MFLAGS_EMPTYCOMMANDLINE    0x00040000
#define MFLAGS_NOTEMPTYCOMMANDLINE 0x00080000
#define MFLAGS_NOFILEPANELS        0x00100000
#define MFLAGS_NOPLUGINPANELS      0x00200000
#define MFLAGS_NOFOLDERS           0x00400000
#define MFLAGS_NOFILES             0x00800000


static const char *MacroModeName[]={
  "Shell", "Viewer", "Editor", "Dialog", "Search",
  "Disks", "MainMenu", "Help"
};

static const char *MacroModeNameOther="Other";

enum MacroTempType{
  MTEMP_POINTER,  // передано откуда то
  MTEMP_DYNAMIC,  // использовалось выделение памяти
};

enum{
  KEY_MACROSTOP=KEY_MACROSPEC_BASE,
  KEY_MACROMODE,
};

static struct TKeyCodeName{
  int Key;
  int Len;
  char *Name;
} KeyMacroCodes[]={
   { KEY_MACRODAY,                           4, "$DAY"   },
   { KEY_MACROMONTH,                         6, "$MONTH" },
   { KEY_MACROYEAR,                          5, "$YEAR"  },
   { KEY_MACROHOUR,                          5, "$HOUR"  },
   { KEY_MACROMIN,                           4, "$MIN"   },
   { KEY_MACROSEC,                           4, "$SEC"   },
   { KEY_MACROSTOP,                          5, "$STOP"  },
   { KEY_MACROMODE,                          6, "$MMODE" },
};

BOOL WINAPI KeyMacroToText(int Key,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;
  char KeyText[32]="";
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
    strncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);

  return TRUE;
}

int WINAPI KeyNameMacroToKey(char *Name)
{
  // пройдемся по всем модификаторам
  for(int I=0; I < sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0]); ++I)
    if(!memicmp(Name,KeyMacroCodes[I].Name,KeyMacroCodes[I].Len))
      return KeyMacroCodes[I].Key;
  return -1;
}

KeyMacro::KeyMacro()
{
  TempMacroType=MTEMP_POINTER;
  TempMacro=NULL;
  LockScr=NULL;
  Macros=NULL;
  RecBuffer=NULL;
  LoadMacros();
}

void KeyMacro::InitVars()
{
  if(Macros)
  {
    for (int I=0;I<MacrosNumber;I++)
      if(Macros[I].Buffer)
        free(Macros[I].Buffer);
    free(Macros);
  }
  if(RecBuffer) delete[] RecBuffer;

  if(LockScr)
  {
    delete LockScr;
    LockScr=NULL;
  }

  ReleaseTempBuffer();

  MacrosNumber=0;
  StartMacroPos=0;
  Recording=FALSE;
  Executing=FALSE;
  Macros=NULL;
  RecBuffer=NULL;
  RecBufferSize=0;
  InternalInput=FALSE;
}

// удаление временного буфера
void KeyMacro::ReleaseTempBuffer()
{
  if(TempMacroType == MTEMP_DYNAMIC && TempMacro)
  {
    if(TempMacro->Buffer)
      free(TempMacro->Buffer);
    free(TempMacro);
  }
  TempMacro=NULL;
  TempMacroType=MTEMP_POINTER;
}

int KeyMacro::LoadMacros()
{
  int Ret=FALSE;
  InitVars();

  #define TEMP_BUFFER_SIZE 32768
  char *Buffer=new char[TEMP_BUFFER_SIZE];

  if(Buffer)
  {
    Ret+=ReadMacros(MACRO_SHELL,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_VIEWER,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_EDITOR,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_DIALOG,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_SEARCH,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_DISKS,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_MAINMENU,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_HELP,Buffer,TEMP_BUFFER_SIZE);
    Ret+=ReadMacros(MACRO_OTHER,Buffer,TEMP_BUFFER_SIZE);

    Ret=(Ret < 9)?FALSE:TRUE;
    delete Buffer;
  }
  Mode=MACRO_SHELL;
  return Ret;
}


KeyMacro::~KeyMacro()
{
  InitVars();
}


int KeyMacro::ProcessKey(int Key)
{
  if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE)
    return(FALSE);
  if (Recording)
  {
    if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT)
    {
      int MacroKey;

#if 0
      {
        SaveScreen SaveScr;
        SetCursorType(FALSE,0);
        Message(0,0,MSG(MDefineMacroTitle),MSG(MDefineMacro));
        InternalInput=TRUE;
        do {
          INPUT_RECORD rec;
          MacroKey=GetInputRecord(&rec);
        } while (MacroKey>=KEY_NONE && MacroKey<=KEY_END_SKEY);
        InternalInput=FALSE;
      }
#else
      InternalInput=TRUE;
      MacroKey=AssignMacroKey();
      InternalInput=FALSE;
#endif
/*
{
char KeyText[50];
::KeyToText(MacroKey,KeyText);
SysLog("ProcessKey) MacroKey=0x%08X  %s",MacroKey,KeyText);
}

*/

      int DisableOutput=TRUE,EmptyCommandLine=FALSE,NotEmptyCommandLine=FALSE;
      int NoFilePanels=FALSE,NoPluginPanels=FALSE;
      int RunAfterStart=FALSE;
      int NoFolders=FALSE, NoFiles=FALSE;

      if (MacroKey != KEY_ESC && (Key==KEY_CTRLSHIFTDOT || Recording==2))
      {
        InternalInput=TRUE;
        if (!GetMacroSettings(MacroKey,DisableOutput,RunAfterStart,
                              EmptyCommandLine,NotEmptyCommandLine,
                              NoFilePanels,NoPluginPanels,
                              NoFolders,NoFiles))
          MacroKey=KEY_ESC;
        InternalInput=FALSE;
      }

      if (MacroKey==KEY_ESC)
        delete RecBuffer;
      else
      {
        int Pos;
        for (Pos=0;Pos<MacrosNumber;Pos++)
          if (Macros[Pos].Key==MacroKey && (Macros[Pos].Flags&MFLAGS_MODEMASK)==StartMode)
            break;
        if (Pos==MacrosNumber)
        {
          Macros=(struct MacroRecord *)realloc(Macros,sizeof(*Macros)*(MacrosNumber+1));
          if (Macros==NULL)
          {
            MacrosNumber=0;
            WaitInFastFind++;
            return(FALSE);
          }
          MacrosNumber++;
        }
        else
          delete Macros[Pos].Buffer;
        Macros[Pos].Key=MacroKey;
        Macros[Pos].Buffer=RecBuffer;
        Macros[Pos].BufferSize=RecBufferSize;
/*{
char KeyText[50];
int J;
for(J=0; J < RecBufferSize; ++J)
{
  ::KeyToText(RecBuffer[J],KeyText);
  SysLog("Save RecBuffer[%d]=0x%08X  %s\n",J,RecBuffer[J],KeyText);
}
} */
        Macros[Pos].Flags=StartMode&MFLAGS_MODEMASK;
        Macros[Pos].Flags|=DisableOutput?MFLAGS_DISABLEOUTPUT:0;
        Macros[Pos].Flags|=RunAfterStart?MFLAGS_RUNAFTERSTART:0;
        Macros[Pos].Flags|=EmptyCommandLine?MFLAGS_EMPTYCOMMANDLINE:0;
        Macros[Pos].Flags|=NotEmptyCommandLine?MFLAGS_NOTEMPTYCOMMANDLINE:0;
        Macros[Pos].Flags|=NoFilePanels?MFLAGS_NOFILEPANELS:0;
        Macros[Pos].Flags|=NoPluginPanels?MFLAGS_NOPLUGINPANELS:0;
        Macros[Pos].Flags|=NoFolders?MFLAGS_NOFOLDERS:0;
        Macros[Pos].Flags|=NoFiles?MFLAGS_NOFILES:0;
      }

      Recording=FALSE;
      RecBuffer=NULL;
      RecBufferSize=0;
      if (Opt.AutoSaveSetup)
        SaveMacros();
      ScrBuf.RestoreMacroChar();
      WaitInFastFind++;
      return(TRUE);
    }
    else
    {
      if (Key>=KEY_NONE && Key<=KEY_END_SKEY)
        return(FALSE);
      RecBuffer=(int *)realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+1));
      if (RecBuffer==NULL)
        return(FALSE);
      RecBuffer[RecBufferSize++]=Key;
      return(FALSE);
    }
  }
  else
    if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT)
    {
      if(LockScr) delete LockScr;
      LockScr=NULL;

      if (Mode==MACRO_SHELL && !WaitInMainLoop)
        StartMode=MACRO_OTHER;
      else
        StartMode=Mode;
      Recording=(Key==KEY_CTRLSHIFTDOT) ? 2:1;
      delete RecBuffer;
      RecBuffer=NULL;
      RecBufferSize=0;
      ScrBuf.ResetShadow();
      ScrBuf.Flush();
      WaitInFastFind--;
      return(TRUE);
    }
    else
    {
      if (!Executing)
      {
        int CurMode=(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode;
        for (int I=0;I<MacrosNumber;I++)
          if (LocalUpper(Macros[I].Key)==LocalUpper(Key) &&
              (Macros[I].Flags&MFLAGS_MODEMASK)==CurMode &&
              Macros[I].BufferSize>0)
          {
            int CmdLength=CtrlObject->CmdLine.GetLength();
            if ((Macros[I].Flags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength!=0 ||
                (Macros[I].Flags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0)
              return(FALSE);
            Panel *ActivePanel=CtrlObject->ActivePanel;
            if(ActivePanel!=NULL)// && (Macros[I].Flags&MFLAGS_MODEMASK)==MACRO_SHELL)
            {
              int PanelMode=ActivePanel->GetMode();
              char FileName[NM*2];
              int FileAttr=-1;
              if(PanelMode == PLUGIN_PANEL)
              {
                if(Macros[I].Flags&MFLAGS_NOPLUGINPANELS)
                  return FALSE;
              }
              if(PanelMode == NORMAL_PANEL)
              {
                if(Macros[I].Flags&MFLAGS_NOFILEPANELS)
                  return FALSE;
              }
              ActivePanel->GetFileName(FileName,ActivePanel->GetCurrentPos(),FileAttr);
              if(FileAttr != -1)
              {
                if((FileAttr&FA_DIREC) && (Macros[I].Flags&MFLAGS_NOFOLDERS) ||
                   !(FileAttr&FA_DIREC) && (Macros[I].Flags&MFLAGS_NOFILES))
                  return FALSE;
              }
            }
            if (Macros[I].Flags&MFLAGS_DISABLEOUTPUT)
            {
              if(LockScr) delete LockScr;
              LockScr=new LockScreen;
            }
            Executing=TRUE;
            ExecMacroPos=I;
            ExecKeyPos=0;
            return(TRUE);
          }
      }
      return(FALSE);
    }
}


int KeyMacro::GetKey()
{
  if (InternalInput || !Executing)
    return(FALSE);

  struct MacroRecord *MR=!TempMacro?Macros+ExecMacroPos:TempMacro;

begin:
  if (ExecKeyPos>=MR->BufferSize || MR->Buffer==NULL)
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
    if(Mode==MACRO_EDITOR)
    {
      if(CtrlObject->Plugins.CurEditor)
      {
        /*$ 27.09.2000 skv
          Don't redraw editor if it is hidden.
        */
        if(CtrlObject->Plugins.CurEditor->IsVisible())
        {

          CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
          CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
          CtrlObject->Plugins.CurEditor->Show();
        }
        /* skv$*/
      }
    }
    /* skv$*/
    if(LockScr) delete LockScr;
    LockScr=NULL;
    Executing=FALSE;
    ReleaseTempBuffer();
    return(FALSE);
  }
  int Key=MR->Buffer[ExecKeyPos++];
  switch(Key)
  {
    case KEY_MACROSTOP:
      goto done;

    case KEY_MACROMODE:
      if (ExecKeyPos<MR->BufferSize)
      {
        Key=MR->Buffer[ExecKeyPos++];
        if(Key == '1')
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
  }
  return(Key);
}


int KeyMacro::PeekKey()
{
  struct MacroRecord *MR=!TempMacro?Macros+ExecMacroPos:TempMacro;
  if (InternalInput || !Executing || ExecKeyPos >= MR->BufferSize)
    return(0);
  int Key=MR->Buffer[ExecKeyPos];
  return(Key);
}

DWORD KeyMacro::SwitchFlags(DWORD& Flags,DWORD Value)
{
  if(Flags&Value) Flags&=~Value;
  else Flags|=Value;
  return Flags;
}


void KeyMacro::SaveMacros()
{
  for (int I=0;I<MacrosNumber;I++)
  {
    char *TextBuffer=NULL;
    int TextBufferSize=0;
    char KeyText[50],RegKeyName[50];
    ::KeyToText(Macros[I].Key,KeyText);
    sprintf(RegKeyName,"KeyMacros\\%s\\%s",
       GetSubKey(Macros[I].Flags&MFLAGS_MODEMASK),KeyText);

    if (Macros[I].BufferSize==0)
    {
      DeleteRegKey(RegKeyName);
      continue;
    }
    for (int J=0;J<Macros[I].BufferSize;J++)
    {
      char MacroKeyText[50];
      ::KeyToText(Macros[I].Buffer[J],MacroKeyText);
      int NewSize=TextBufferSize+strlen(MacroKeyText);
      if (TextBufferSize!=0)
        NewSize++;
      TextBuffer=(char *)realloc(TextBuffer,NewSize+1);
      if (TextBuffer==NULL)
        continue;
      sprintf(TextBuffer+TextBufferSize,"%s%s",
              TextBufferSize==0 ? "":" ",MacroKeyText);
      TextBufferSize=NewSize;
    }
    SetRegKey(RegKeyName,"Sequence",TextBuffer);
    if (Macros[I].Flags&MFLAGS_DISABLEOUTPUT)
      SetRegKey(RegKeyName,"DisableOutput",1);
    else
      DeleteRegValue(RegKeyName,"DisableOutput");
    if (Macros[I].Flags&MFLAGS_RUNAFTERSTART)
      SetRegKey(RegKeyName,"RunAfterFARStart",1);
    else
      DeleteRegValue(RegKeyName,"RunAfterFARStart");
    if (Macros[I].Flags&MFLAGS_EMPTYCOMMANDLINE)
      SetRegKey(RegKeyName,"EmptyCommandLine",1);
    else
      DeleteRegValue(RegKeyName,"EmptyCommandLine");
    if (Macros[I].Flags&MFLAGS_NOTEMPTYCOMMANDLINE)
      SetRegKey(RegKeyName,"NotEmptyCommandLine",1);
    else
      DeleteRegValue(RegKeyName,"NotEmptyCommandLine");
    if (Macros[I].Flags&MFLAGS_NOFILEPANELS)
      SetRegKey(RegKeyName,"NoFilePanels",1);
    else
      DeleteRegValue(RegKeyName,"NoFilePanels");
    if (Macros[I].Flags&MFLAGS_NOPLUGINPANELS)
      SetRegKey(RegKeyName,"NoPluginPanels",1);
    else
      DeleteRegValue(RegKeyName,"NoPluginPanels");
    if (Macros[I].Flags&MFLAGS_NOFOLDERS)
      SetRegKey(RegKeyName,"NoFolders",1);
    else
      DeleteRegValue(RegKeyName,"NoFolders");
    if (Macros[I].Flags&MFLAGS_NOFILES)
      SetRegKey(RegKeyName,"NoFiles",1);
    else
      DeleteRegValue(RegKeyName,"NoFiles");
    delete TextBuffer;
  }
}


/* $ 10.09.2000 SVS
  ! Исправим ситуацию с макросами в связи с переработаными кодами клавиш
  ! Функция ReadMacros имеет дополнительные аргументы
*/
int KeyMacro::ReadMacros(int ReadMode,char *Buffer,int BufferSize)
{
  int I, J;

  for (I=0;;I++)
  {
    char RegKeyName[50],KeyText[50];
    char UpKeyName[100];
    sprintf(UpKeyName,"KeyMacros\\%s",GetSubKey(ReadMode));
    if (!EnumRegKey(UpKeyName,I,RegKeyName,sizeof(RegKeyName)))
      break;
    char *KeyNamePtr=strrchr(RegKeyName,'\\');
    if (KeyNamePtr!=NULL)
      strcpy(KeyText,KeyNamePtr+1);
    else
      *KeyText=0;
    int KeyCode=KeyNameToKey(KeyText);
    if (KeyCode==-1)
      continue;
    Macros=(struct MacroRecord *)realloc(Macros,sizeof(*Macros)*(MacrosNumber+1));
    if (Macros==NULL)
    {
      MacrosNumber=0;
      return FALSE;
    }
    struct MacroRecord *CurMacro=&Macros[MacrosNumber];
    CurMacro->Key=KeyCode;
    CurMacro->Buffer=NULL;
    CurMacro->BufferSize=0;
    CurMacro->Flags=ReadMode&MFLAGS_MODEMASK;
    GetRegKey(RegKeyName,"Sequence",Buffer,"",BufferSize);
    CurMacro->Flags|=GetRegKey(RegKeyName,"DisableOutput",0)?MFLAGS_DISABLEOUTPUT:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"RunAfterFARStart",0)?MFLAGS_RUNAFTERSTART:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"EmptyCommandLine",0)?MFLAGS_EMPTYCOMMANDLINE:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"NotEmptyCommandLine",0)?MFLAGS_NOTEMPTYCOMMANDLINE:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"NoFilePanels",0)?MFLAGS_NOFILEPANELS:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"NoPluginPanels",0)?MFLAGS_NOPLUGINPANELS:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"NoFolders",0)?MFLAGS_NOFOLDERS:0;
    CurMacro->Flags|=GetRegKey(RegKeyName,"NoFiles",0)?MFLAGS_NOFILES:0;

    char *BufPtr=Buffer;
    while (1)
    {
      while (isspace(*BufPtr))
        BufPtr++;
      if (*BufPtr==0)
        break;
      char *CurBufPtr=BufPtr;
      while (*BufPtr && !isspace(*BufPtr))
        BufPtr++;
      int Length=BufPtr-CurBufPtr;
      char CurKeyText[50];
      memcpy(CurKeyText,CurBufPtr,Length);
      CurKeyText[Length]=0;
      KeyCode=KeyNameToKey(CurKeyText);
      if (KeyCode!=-1)
      {
        CurMacro->Buffer=(int *)realloc(CurMacro->Buffer,sizeof(*CurMacro->Buffer)*(CurMacro->BufferSize+1));
        if (CurMacro->Buffer==NULL)
        {
          return FALSE;
        }
        CurMacro->Buffer[CurMacro->BufferSize]=KeyCode;
        CurMacro->BufferSize++;
      }
    }
    MacrosNumber++;
  }
  return TRUE;
}
/* SVS $ */

// Функция, запускающая макросы при старте ФАРа
// если уж вставлять предупреждение о недопустимости выполения
// подобных макросов, то именно сюды!
void KeyMacro::RunStartMacro()
{
  if (StartMacroPos==-1)
    return;
  while (StartMacroPos<MacrosNumber)
  {
    int CurPos=StartMacroPos++;
    if ((Macros[CurPos].Flags&MFLAGS_MODEMASK)==MACRO_SHELL &&
        Macros[CurPos].BufferSize>0 &&
        (Macros[CurPos].Flags&MFLAGS_RUNAFTERSTART))
    {
      int CmdLength=CtrlObject->CmdLine.GetLength();
      if ((Macros[CurPos].Flags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength!=0 ||
          (Macros[CurPos].Flags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0)
        return;
      Panel *ActivePanel=CtrlObject->ActivePanel;
      if(ActivePanel!=NULL)
      {
        char FileName[NM*2];
        int FileAttr=-1;
        int PanelMode=ActivePanel->GetMode();
        if(PanelMode == PLUGIN_PANEL)
        {
          if(Macros[CurPos].Flags&MFLAGS_NOPLUGINPANELS)
            return;
        }
        if(PanelMode == NORMAL_PANEL)
        {
          if(Macros[CurPos].Flags&MFLAGS_NOFILEPANELS)
            return;
        }
        ActivePanel->GetFileName(FileName,ActivePanel->GetCurrentPos(),FileAttr);
        if(FileAttr != -1)
        {
          if((FileAttr&FA_DIREC) && (Macros[CurPos].Flags&MFLAGS_NOFOLDERS) ||
             !(FileAttr&FA_DIREC) && (Macros[CurPos].Flags&MFLAGS_NOFILES))
            return;
        }
      }
      if (Macros[CurPos].Flags&MFLAGS_DISABLEOUTPUT)
      {
        if(LockScr) delete LockScr;
        LockScr=new LockScreen;
      }
      Executing=TRUE;
      ExecMacroPos=CurPos;
      ExecKeyPos=0;
      return;
    }
  }
  StartMacroPos=-1;
}

/* $ 21.12.2000 SVS
   Подсократим код.
*/
char* KeyMacro::GetSubKey(int Mode)
{
  return (char *)((Mode >= MACRO_SHELL && Mode <= MACRO_HELP)?
            MacroModeName[Mode]:
            (Mode == MACRO_OTHER?MacroModeNameOther:""));
}

int KeyMacro::GetSubKey(char *Mode)
{
  if(!stricmp(MacroModeNameOther,Mode))
    return MACRO_OTHER;

  int I;
  for(I=MACRO_SHELL; I <= MACRO_HELP; ++I)
    if(!stricmp(MacroModeName[I],Mode))
      return I;
  return -1;
}


// обработчик диалогового окна назначения клавиши
long WINAPI AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  char KeyText[50];
  static int *Key, LastKey;
  if(Msg == DN_INITDIALOG)
    Key=(int *)Param2;
  else if(Msg == DM_KEY)
  {
    if(Param2 == KEY_ESC || Param2 == KEY_ENTER)
      return FALSE;
    if(Param2 == KEY_F1 && LastKey!=KEY_F1)
    {
      LastKey=KEY_F1;
      return FALSE;
    }

    *Key=Param2;
    KeyToText(Param2,KeyText);
//SysLog("0x%08X (%s)",Param2,KeyText);
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)KeyText);
    LastKey=Param2;
    if(Param2 == KEY_F1 && LastKey==KEY_F1)
      LastKey=0;
    return TRUE;
  }
  else if(Msg == DN_CTLCOLORDLGITEM) // сбросим Unchanged
  {
    Param2&=0xFF00FFFFU;
    Param2|=(Param2&0xFF)<<16;
    return Param2;
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::AssignMacroKey()
{
/*
  ╔══════ Define macro ══════╗
  ║ Press the desired key    ║
  ║ ________________________ ║
  ╚══════════════════════════╝
*/

  static struct DialogData MacroAssignDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,30,4,0,0,0,0,(char *)MDefineMacroTitle,
  /* 01 */ DI_TEXT,-1,2,0,0,0,0,DIF_BOXCOLOR|DIF_READONLY,0,(char *)MDefineMacro,
  /* 02 */ DI_EDIT,5,3,28,3,1,0,0,1,"",
  };
  MakeDialogItems(MacroAssignDlgData,MacroAssignDlg);
  int Key=0;

  Dialog Dlg(MacroAssignDlg,sizeof(MacroAssignDlg)/sizeof(MacroAssignDlg[0]),AssignMacroDlgProc,(long)&Key);
  Dlg.SetPosition(-1,-1,34,6);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  return Key;
}

int KeyMacro::GetMacroSettings(
         int Key,
         int &DisableOutput,
         int &RunAfterStart,
         int &EmptyCommandLine,
         int &NotEmptyCommandLine,
         int &NoFilePanels,
         int &NoPluginPanels,
         int &NoFolders,
         int &NoFiles)
{
  static struct DialogData MacroSettingsDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,62,19,0,0,0,0,"",
  /* 01 */ DI_CHECKBOX,5,2,0,0,1,1,0,0,(char *)MMacroSettingsDisableOutput,
  /* 02 */ DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MMacroSettingsRunAfterStart,
//  /* 03 */ DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MMacroSettingsExactCollation,
  /* 03 */ DI_TEXT,5,4,0,0,0,0,0,0,"",
  /* 04 */ DI_TEXT,3,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 05 */ DI_RADIOBUTTON,5,6,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnoreCommandLine,
  /* 06 */ DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MMacroSettingsEmptyCommandLine,
  /* 07 */ DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(char *)MMacroSettingsNotEmptyCommandLine,
  /* 08 */ DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 09 */ DI_RADIOBUTTON,5,10,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnorePanels,
  /* 10 */ DI_RADIOBUTTON,5,11,0,0,0,0,0,0,(char *)MMacroSettingsFilePanels,
  /* 11 */ DI_RADIOBUTTON,5,12,0,0,0,0,0,0,(char *)MMacroSettingsPluginPanels,

  /* 12 */ DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 13 */ DI_RADIOBUTTON,5,14,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnoreFileFolders,
  /* 14 */ DI_RADIOBUTTON,5,15,0,0,0,0,0,0,(char *)MMacroSettingsFolders,
  /* 15 */ DI_RADIOBUTTON,5,16,0,0,0,0,0,0,(char *)MMacroSettingsFiles,

  /* 16 */ DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 17 */ DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 18 */ DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(MacroSettingsDlgData,MacroSettingsDlg);

  char KeyText[66];
  KeyToText(Key,KeyText);
  sprintf(MacroSettingsDlg[0].Data,MSG(MMacroSettingsTitle),KeyText);
  if(!(Key&0xFF000000))
    MacroSettingsDlg[3].Flags|=DIF_DISABLE;

  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]));
  Dlg.SetPosition(-1,-1,66,21);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  if (Dlg.GetExitCode()!=17)
    return(FALSE);
  DisableOutput=MacroSettingsDlg[1].Selected;
  RunAfterStart=MacroSettingsDlg[2].Selected;
  EmptyCommandLine=MacroSettingsDlg[6].Selected;
  NotEmptyCommandLine=MacroSettingsDlg[7].Selected;
  NoFilePanels=MacroSettingsDlg[11].Selected;
  NoPluginPanels=MacroSettingsDlg[10].Selected;
  NoFolders=MacroSettingsDlg[15].Selected;
  NoFiles=MacroSettingsDlg[14].Selected;

  return(TRUE);
}

int KeyMacro::PlayKeyMacro(char *KeyBuffer)
{
  ReleaseTempBuffer();

  if((TempMacro=(struct MacroRecord *)malloc(sizeof(MacroRecord))) == NULL)
    return FALSE;
  TempMacro->Buffer=NULL;
  TempMacro->Flags=0;
  TempMacro->Key=0;
  TempMacro->BufferSize=0;

  if(!ParseMacroString(TempMacro,KeyBuffer))
  {
    ReleaseTempBuffer();
    return FALSE;
  }

  TempMacroType=MTEMP_DYNAMIC;
  if (TempMacro->Flags&MFLAGS_DISABLEOUTPUT)
  {
    if(LockScr) delete LockScr;
    LockScr=new LockScreen;
  }
  Executing=TRUE;
  ExecKeyPos=0;
  return TRUE;
}

int KeyMacro::PlayKeyMacro(struct MacroRecord *MRec)
{
  ReleaseTempBuffer();

  TempMacro=MRec;

  if(!TempMacro)
    return FALSE;

  if (TempMacro->Flags&MFLAGS_DISABLEOUTPUT)
  {
    if(LockScr) delete LockScr;
    LockScr=new LockScreen;
  }
  Executing=TRUE;
  ExecKeyPos=0;
  return TRUE;
}


int KeyMacro::ParseMacroString(struct MacroRecord *CurMacro,char *BufPtr)
{
  int J;
  if(!CurMacro || !BufPtr || !*BufPtr)
    return FALSE;
  // здесь структура сформирована, начинаем разбор последовательности,
  // которая находится в Buffer
  while (1)
  {
    // пропускаем ведущие пробельные символы
    while (isspace(*BufPtr))
      BufPtr++;
    if (*BufPtr==0)
      break;

    char *CurBufPtr=BufPtr;

    // ищем конец очередного названия клавиши
    while (*BufPtr && !isspace(*BufPtr))
      BufPtr++;
    int Length=BufPtr-CurBufPtr;
    char CurKeyText[50];
    memcpy(CurKeyText,CurBufPtr,Length);
    CurKeyText[Length]=0;

    // в CurKeyText - название клавиши. Попробуем отыскать ее код...
    int KeyCode=KeyNameToKey(CurKeyText);
    // код найден, добавим этот код в буфер последовательности.
    if (KeyCode!=-1)
    {
      CurMacro->Buffer=(int *)realloc(CurMacro->Buffer,sizeof(*CurMacro->Buffer)*(CurMacro->BufferSize+1));
      if (CurMacro->Buffer==NULL)
      {
        return FALSE;
      }
      CurMacro->Buffer[CurMacro->BufferSize]=KeyCode;
      CurMacro->BufferSize++;
    }
  }
  return TRUE;
}


