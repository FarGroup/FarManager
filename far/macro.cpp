/*
macro.cpp

Макросы

*/

/* Revision: 1.15 26.12.2000 $ */

/*
Modify:
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

BOOL WINAPI KeyMacroToText(int Key,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;
  char KeyText[32]="";
  struct TKeyCodeName{
    int Key;
    char *Name;
  } KeyCodes[]={
     { KEY_MACRODAY,                           "$DAY"   },
     { KEY_MACROMONTH,                         "$MONTH" },
     { KEY_MACROYEAR,                          "$YEAR"  },

     { KEY_MACROSTOP,                          "$STOP"  },
     { KEY_MACROMODE,                          "$MMODE" },
  };
  for (int I=0;I<sizeof(KeyCodes)/sizeof(KeyCodes[0]);I++)
    if (Key==KeyCodes[I].Key)
    {
      strcpy(KeyText,KeyCodes[I].Name);
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
    struct TKeyNames *KeyNames=new TKeyNames[END_FARKEY_BASE];
    if(KeyNames)
    {
      int CountKeyNames, I;

      for (CountKeyNames=I=0; CountKeyNames < END_FARKEY_BASE;++I)
      {
        if(I == END_FARKEY_BASE)
          break;
        if(::KeyToText(I,KeyNames[CountKeyNames].Name))
        {
          KeyNames[CountKeyNames].Code=I;
          CountKeyNames++;
        }
      }

      Ret+=ReadMacros(MACRO_SHELL,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_VIEWER,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_EDITOR,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_DIALOG,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_SEARCH,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_DISKS,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_MAINMENU,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_HELP,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);
      Ret+=ReadMacros(MACRO_OTHER,KeyNames,CountKeyNames,Buffer,TEMP_BUFFER_SIZE);

      Ret=(Ret < 9)?FALSE:TRUE;

      delete KeyNames;
    }
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

      {
        SaveScreen SaveScr;
        SetCursorType(FALSE,0);
        Message(0,0,MSG(MDefineMacroTitle),MSG(MDefineMacro));
        InternalInput=TRUE;
        do {
          INPUT_RECORD rec;
          MacroKey=GetInputRecord(&rec);
        } while (MacroKey>=KEY_NONE);
        InternalInput=FALSE;
      }

      int DisableOutput=TRUE,EmptyCommandLine=FALSE,NotEmptyCommandLine=FALSE;
      int FilePanels=FALSE,PluginPanels=FALSE;
      int RunAfterStart=FALSE;

      if (Key==KEY_CTRLSHIFTDOT || Recording==2)
      {
        InternalInput=TRUE;
        if (!GetMacroSettings(DisableOutput,RunAfterStart,
                              EmptyCommandLine,NotEmptyCommandLine,
                              FilePanels,PluginPanels))
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
            return(FALSE);
          }
          MacrosNumber++;
        }
        else
          delete Macros[Pos].Buffer;
        Macros[Pos].Key=MacroKey;
        Macros[Pos].Buffer=RecBuffer;
        Macros[Pos].BufferSize=RecBufferSize;
        Macros[Pos].Flags=StartMode&MFLAGS_MODEMASK;
        Macros[Pos].Flags|=DisableOutput?MFLAGS_DISABLEOUTPUT:0;
        Macros[Pos].Flags|=RunAfterStart?MFLAGS_RUNAFTERSTART:0;
        Macros[Pos].Flags|=EmptyCommandLine?MFLAGS_EMPTYCOMMANDLINE:0;
        Macros[Pos].Flags|=NotEmptyCommandLine?MFLAGS_NOTEMPTYCOMMANDLINE:0;
        Macros[Pos].Flags|=FilePanels?MFLAGS_NOFILEPANELS:0;
        Macros[Pos].Flags|=PluginPanels?MFLAGS_NOPLUGINPANELS:0;
      }

      Recording=FALSE;
      RecBuffer=NULL;
      RecBufferSize=0;
      if (Opt.AutoSaveSetup)
        SaveMacros();
      ScrBuf.RestoreMacroChar();
      return(TRUE);
    }
    else
    {
      if (Key>=KEY_NONE)
        return(FALSE);
      RecBuffer=(int *)realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+1));
      if (RecBuffer==NULL)
        return(FALSE);
      RecBuffer[RecBufferSize]=Key;
      RecBufferSize++;
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
            if(CtrlObject->ActivePanel!=NULL)// && (Macros[I].Flags&MFLAGS_MODEMASK)==MACRO_SHELL)
            {
              int PanelMode=CtrlObject->ActivePanel->GetMode();
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
    delete TextBuffer;
  }
}


/* $ 10.09.2000 SVS
  ! Исправим ситуацию с макросами в связи с переработаными кодами клавиш
  ! Функция ReadMacros имеет дополнительные аргументы
*/
int KeyMacro::ReadMacros(int ReadMode,
                          struct TKeyNames *KeyNames,
                          int CountKeyNames,
                          char *Buffer,
                          int BufferSize)
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
    int KeyCode=-1;
    for (J=0; J < CountKeyNames; J++)
      if (stricmp(KeyText,KeyNames[J].Name)==0)
      {
        KeyCode=KeyNames[J].Code;
        break;
      }
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
      int KeyCode=-1;
      for (J=0; J < CountKeyNames; J++)
        if (Length==1 && strcmp(CurKeyText,KeyNames[J].Name)==0 ||
            Length>1 && stricmp(CurKeyText,KeyNames[J].Name)==0)
        {
          KeyCode=KeyNames[J].Code;
          break;
        }
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
      if(CtrlObject->ActivePanel!=NULL)
      {
        int PanelMode=CtrlObject->ActivePanel->GetMode();
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


int KeyMacro::GetMacroSettings(
         int &DisableOutput,
         int &RunAfterStart,
         int &EmptyCommandLine,
         int &NotEmptyCommandLine,
         int &FilePanels,
         int &PluginPanels)
{
  static struct DialogData MacroSettingsDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,62,14,0,0,0,0,(char *)MMacroSettingsTitle,
  /* 01 */ DI_CHECKBOX,5,2,0,0,1,1,0,0,(char *)MMacroSettingsDisableOutput,
  /* 02 */ DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MMacroSettingsRunAfterStart,
  /* 03 */ DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */ DI_RADIOBUTTON,5,5,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnoreCommandLine,
  /* 05 */ DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MMacroSettingsEmptyCommandLine,
  /* 06 */ DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MMacroSettingsNotEmptyCommandLine,
  /* 07 */ DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 08 */ DI_RADIOBUTTON,5,9,0,0,0,1,DIF_GROUP,0,(char *)MMacroSettingsIgnorePanels,
  /* 09 */ DI_RADIOBUTTON,5,10,0,0,0,0,0,0,(char *)MMacroSettingsFilePanels,
  /* 10 */ DI_RADIOBUTTON,5,11,0,0,0,0,0,0,(char *)MMacroSettingsPluginPanels,
  /* 11 */ DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 12 */ DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 13 */ DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(MacroSettingsDlgData,MacroSettingsDlg);

  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]));
  Dlg.SetPosition(-1,-1,66,16);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  if (Dlg.GetExitCode()!=12)
    return(FALSE);
  DisableOutput=MacroSettingsDlg[1].Selected;
  RunAfterStart=MacroSettingsDlg[2].Selected;
  EmptyCommandLine=MacroSettingsDlg[5].Selected;
  NotEmptyCommandLine=MacroSettingsDlg[6].Selected;
  if(!MacroSettingsDlg[8].Selected)
  {
    FilePanels=MacroSettingsDlg[10].Selected;
    PluginPanels=MacroSettingsDlg[9].Selected;
  }
  else
    FilePanels=PluginPanels=1;

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


