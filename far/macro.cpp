/*
macro.cpp

Макросы

*/

/* Revision: 1.23 30.01.2001 $ */

/*
Modify:
  30.01.2001 SVS
    - Забыл сделать проверку на код возврата из диалога назначения
  21.01.2001 SVS
    - ошибка в назначении клавиши - учитывался текущий контекст макроса, а
      не стартовый при окончании записи макроса.
  19.01.2001 SVS
    + Зарезервировано: MFLAGS_REUSEMACRO - повторное использование макросов
      Это чуть позже, когда будет механизм линеризации...
      А пока пусть будет :-)
  18.01.2001 SVS
    ! немного оптимизации кода
    + Функции поиска макроклавиши и определения размера этой клавиши
    + Хотелка - выдавать предупреждения, что такая клавиша существует
      или подтверждение желания удалить макроклавишу.
  09.01.2001 SVS
    - Бага с пропаданием диалога назначения во время изменения файловой
      структуры в панелях.
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
#define MFLAGS_REUSEMACRO          0x01000000


static const char *MacroModeName[]={
  "Shell", "Viewer", "Editor", "Dialog", "Search",
  "Disks", "MainMenu", "Help"
};

static const char *MacroModeNameOther="Other";

// тип временного буфера
enum MacroTempType{
  MTEMP_POINTER,  // передано откуда то
  MTEMP_DYNAMIC,  // использовалось выделение памяти
};

// для диалога назначения клавиши
struct DlgParam{
  KeyMacro *Handle;
  DWORD Key;
  int Mode;
};

// Коды МАКРОКЛАВИШ
enum{
  KEY_MACROSTOP=KEY_MACROSPEC_BASE,
  KEY_MACROMODE,
};

// транслирующая таблица - имя <-> код макроклавиши
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

// функция преобразования кода макроклавиши в текст
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

KeyMacro::KeyMacro()
{
  TempMacroType=MTEMP_POINTER;
  TempMacro=NULL;
  LockScr=NULL;
  Macros=NULL;
  RecBuffer=NULL;
  LoadMacros();
}

KeyMacro::~KeyMacro()
{
  InitVars();
}

// инициализация всех переменных
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

// удаление временного буфера, если он создавался динамически
// (динамически - значит в PlayMacros передали строку.
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

// загрузка ВСЕХ макросов из реестра
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

    // выставим код возврата - если не все ВСЕ загрузились, то
    // будет FALSE
    Ret=(Ret < 9)?FALSE:TRUE;
    delete Buffer;
  }
  Mode=MACRO_SHELL;
  return Ret;
}

// функция преобразования названия в код макроклавиши
// вернет -1, если нет эквивалента!
int WINAPI KeyNameMacroToKey(char *Name)
{
  // пройдемся по всем модификаторам
  for(int I=0; I < sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0]); ++I)
    if(!memicmp(Name,KeyMacroCodes[I].Name,KeyMacroCodes[I].Len))
      return KeyMacroCodes[I].Key;
  return -1;
}

int KeyMacro::ProcessKey(int Key)
{
  if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE)
    return(FALSE);

  if (Recording) // Идет запись?
  {
    if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT) // признак конца записи?
    {
      DWORD MacroKey;
      int WaitInMainLoop0=WaitInMainLoop;
      InternalInput=TRUE;
      WaitInMainLoop=FALSE;
      MacroKey=AssignMacroKey();
      WaitInMainLoop=WaitInMainLoop0;
      InternalInput=FALSE;

      DWORD Flags=MFLAGS_DISABLEOUTPUT;

      // добавим проверку на удаление
      // если удаляем, то не нужно выдавать диалог настройки.
      if (MacroKey != KEY_ESC && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
      {
        InternalInput=TRUE;
        if (!GetMacroSettings(MacroKey,Flags))
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
        Macros[Pos].Buffer=(DWORD*)RecBuffer;
        Macros[Pos].BufferSize=RecBufferSize;
        Macros[Pos].Flags=Flags|(StartMode&MFLAGS_MODEMASK);
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
    else // процесс записи продолжается.
    {
      if (Key>=KEY_NONE && Key<=KEY_END_SKEY) // специальные клавиши прокинем
        return(FALSE);
      RecBuffer=(int *)realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+1));
      if (RecBuffer==NULL)
        return(FALSE);
      RecBuffer[RecBufferSize++]=Key;
      return(FALSE);
    }
  }
  else if (Key==KEY_CTRLSHIFTDOT || Key==KEY_CTRLDOT) // Начало записи?
  {
    if(LockScr) delete LockScr;
    LockScr=NULL;

    // Где мы?
    StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
    // тип записи - с вызовом диалога настроек или...
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
    if (!Executing) // Это еще не режим исполнения?
    {
      int I=GetIndex(LocalUpper(Key),
                    (Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);
      if(I != -1)
      {
//SysLog("KeyMacro: %d (I=%d Key=0x%08X)",__LINE__,I,Key);
        // проверка на пусто/не пусто в ком.строке (а в редакторе? :-)
        int CmdLength=CtrlObject->CmdLine.GetLength();
        if ((Macros[I].Flags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength!=0 ||
            (Macros[I].Flags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0)
          return(FALSE);

        // проверки панели и типа файла
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
        // Подавлять вывод?
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

// Получить очередной код клавиши из макроса
int KeyMacro::GetKey()
{
  struct MacroRecord *MR;

  if (InternalInput || !Executing)
    return(FALSE);

initial:
  MR=!TempMacro?Macros+ExecMacroPos:TempMacro;

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

  DWORD Key=MR->Buffer[ExecKeyPos++];
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
//SysLog("Key=0x%08X ExecMacroPos=%d", Key,ExecMacroPos);
  return(Key);
}

// Проверить - еслть ли еще клавиша?
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


// Сохранение ВСЕХ макросов
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
      DWORD Key=Macros[I].Buffer[J];
      KeyToText(Key,MacroKeyText);
      int NewSize=TextBufferSize+strlen(MacroKeyText);
      if (TextBufferSize!=0)
        NewSize++;
      TextBuffer=(char *)realloc(TextBuffer,NewSize+1);
      if (TextBuffer==NULL)
        continue;
      sprintf(TextBuffer+TextBufferSize,"%s%s",
              (TextBufferSize==0 ? "":" "),
              MacroKeyText);
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
    if (Macros[I].Flags&MFLAGS_REUSEMACRO)
      SetRegKey(RegKeyName,"ReuseMacro",1);
    else
      DeleteRegValue(RegKeyName,"ReuseMacro");

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
    CurMacro->Flags|=GetRegKey(RegKeyName,"ReuseMacro",0)?MFLAGS_REUSEMACRO:0;

    if(!ParseMacroString(CurMacro,Buffer))
      return FALSE;
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

// обработчик диалогового окна назначения клавиши
long WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  char KeyText[50];
  static int LastKey;
  static struct DlgParam *KMParam=NULL;

  if(Msg == DN_INITDIALOG)
    KMParam=(struct DlgParam *)Param2;
  else if(Msg == DM_KEY)
  {
    // Esc & Enter - не обрабатываем
    if(Param2 == KEY_ESC || Param2 == KEY_ENTER)
      return FALSE;
    // F1 - особый случай - нужно жать 2 раза
    // первый раз будет выведен хелп,
    // а второй раз - второй раз уже назначение
    if(Param2 == KEY_F1 && LastKey!=KEY_F1)
    {
      LastKey=KEY_F1;
      return FALSE;
    }
    KeyMacro *MacroDlg=KMParam->Handle;

    KMParam->Key=(DWORD)Param2;
    KeyToText(Param2,KeyText);

    // если УЖЕ есть такой макрос...
    if(MacroDlg->GetIndex(Param2,KMParam->Mode) != -1)
    {
      char Buf[256];

      // выведем сообщение.
      // Если размер буфера нулевой - удаление
      //  Иначе - перезапись
      sprintf(Buf,
        MSG(!MacroDlg->RecBufferSize?MMacroDeleteKey:MMacroReDefinedKey),
        KeyText);
      if(!Message(MSG_WARNING,2,MSG(MWarning),
            Buf,
            MSG(!MacroDlg->RecBufferSize?MMacroDeleteKey2:MMacroReDefinedKey2),
            MSG(MYes),MSG(MNo)))
      {
        // в любом случае - вываливаемся
        Dialog::SendDlgMessage(hDlg,DM_CLOSE,1,0);
        return TRUE;
      }
      // здесь - здесь мы нажимали "Нет", ну а на нет и суда нет
      //  и значит очистим поле ввода.
      KeyText[0]=0;
    }
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)KeyText);
    LastKey=Param2;
    if(Param2 == KEY_F1 && LastKey==KEY_F1)
      LastKey=0;
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
  struct DlgParam Param={this,0,StartMode};

  Dialog Dlg(MacroAssignDlg,sizeof(MacroAssignDlg)/sizeof(MacroAssignDlg[0]),AssignMacroDlgProc,(long)&Param);
  Dlg.SetPosition(-1,-1,34,6);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  /* $ 30.01.2001 SVS
     Забыл сделать проверку на код возврата из диалога назначения
  */
  if(Dlg.GetExitCode() == -1)
    return KEY_ESC;
  /* SVS $ */
  return Param.Key;
}

int KeyMacro::GetMacroSettings(int Key,DWORD &Flags)
{

  static struct DialogData MacroSettingsDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,62,19,0,0,0,0,"",
  /* 01 */ DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MMacroSettingsDisableOutput,
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
  if(!(Key&0x7F000000))
    MacroSettingsDlg[3].Flags|=DIF_DISABLE;

  MacroSettingsDlg[1].Selected=Flags&MFLAGS_DISABLEOUTPUT?1:0;
  MacroSettingsDlg[2].Selected=Flags&MFLAGS_RUNAFTERSTART?1:0;
  MacroSettingsDlg[6].Selected=Flags&MFLAGS_EMPTYCOMMANDLINE?1:0;
  MacroSettingsDlg[7].Selected=Flags&MFLAGS_NOTEMPTYCOMMANDLINE?1:0;
  MacroSettingsDlg[11].Selected=Flags&MFLAGS_NOFILEPANELS?1:0;
  MacroSettingsDlg[10].Selected=Flags&MFLAGS_NOPLUGINPANELS?1:0;
  MacroSettingsDlg[15].Selected=Flags&MFLAGS_NOFOLDERS?1:0;
  MacroSettingsDlg[14].Selected=Flags&MFLAGS_NOFILES?1:0;

  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]));
  Dlg.SetPosition(-1,-1,66,21);
  Dlg.SetHelp("KeyMacro");
  Dlg.Process();
  if (Dlg.GetExitCode()!=17)
    return(FALSE);

  Flags=MacroSettingsDlg[1].Selected?MFLAGS_DISABLEOUTPUT:0;
  Flags|=MacroSettingsDlg[2].Selected?MFLAGS_RUNAFTERSTART:0;
  Flags|=MacroSettingsDlg[6].Selected?MFLAGS_EMPTYCOMMANDLINE:0;
  Flags|=MacroSettingsDlg[7].Selected?MFLAGS_NOTEMPTYCOMMANDLINE:0;
  Flags|=MacroSettingsDlg[11].Selected?MFLAGS_NOFILEPANELS:0;
  Flags|=MacroSettingsDlg[10].Selected?MFLAGS_NOPLUGINPANELS:0;
  Flags|=MacroSettingsDlg[15].Selected?MFLAGS_NOFOLDERS:0;
  Flags|=MacroSettingsDlg[14].Selected?MFLAGS_NOFILES:0;

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

// Парсер строковых эквивалентов в коды клавиш
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
    DWORD KeyCode;
    KeyCode=KeyNameToKey(CurKeyText);
    // код найден, добавим этот код в буфер последовательности.
    if (KeyCode!=-1)
    {
      CurMacro->Buffer=(DWORD *)realloc(CurMacro->Buffer,sizeof(*CurMacro->Buffer)*(CurMacro->BufferSize+1));
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


// Функция получения индекса нужного макроса в массиве
// Ret=-1 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
int KeyMacro::GetIndex(int Key, int ChechMode)
{
  int Pos;
  if(Macros)
    for(Pos=0; Pos < MacrosNumber; ++Pos)
//      if (Macros[Pos].Key==Key && Macros[Pos].BufferSize)
      if (LocalUpper(Macros[Pos].Key)==LocalUpper(Key) && Macros[Pos].BufferSize > 0)
        if((Macros[Pos].Flags&MFLAGS_MODEMASK) == ChechMode || ChechMode == -1)
          return Pos;
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
  return sizeof(struct MacroRecord)+Macros[Pos].BufferSize;
}

/* $ 21.12.2000 SVS
   Подсократим код.
*/
// получить название моды по коду
char* KeyMacro::GetSubKey(int Mode)
{
  return (char *)((Mode >= MACRO_SHELL && Mode <= MACRO_HELP)?
            MacroModeName[Mode]:
            (Mode == MACRO_OTHER?MacroModeNameOther:""));
}

// получить код моды по имени
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
