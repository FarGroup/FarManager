/*
help.cpp

Помощь

*/

/* Revision: 1.28 20.06.2001 $ */

/*
Modify:
  20.06.2001 SVS
    - Исправляем проблемы с Alt-F9
  14.06.2001 OT
    ! "Бунт" ;-)
  04.06.2001 OT
    - Окончательное исправление F1->AltF9.
      Истреблены переменные static SaveScreen *TopScreen и TopLevel за ненадобностью
  31.05.2001 OT
    + ResizeConsole()
    - Исправление F1->AltF9-> ?? Остались некоторые артефакты,
      связанные со ScreenSaveом, но... это чуть позже :)
  26.05.2001 OT
    - Выпрямление логики вызовов в NFZ
    - По умолчанию хелпы создаются статически.
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  07.05.2001 DJ
    ! поддержка mouse wheel
    - кейбар не обновлялся
  06.05.2001 DJ
    ! перетрях #include
  26.04.2001 DJ
    - используем сохраненный Mask при обработке F5
  16.04.2001 SVS
    - не поганим SelTopic, если и так в "Help on Help"
  12.04.2001 SVS
    + сохранение значения Mask, переданного в конструктор (для корректной
      работы HlfViewer)
    - не работало последовательное нажатие F1, Shift-F1, Enter
  26.03.2001 SVS
    + FHELP_USECONTENTS - если не найден требует топик, то отобразить "Contents"
    ! ReadHelp возвращает TRUE/FALSE
  21.03.2001 VVM
    ! уточнение поведения символа '$'
  16.03.2001 VVM
    ! Если топик не найден - остаемся, где были
    - В функции ReadPluginsHelp инициализировать CtrlColorChar
  22.02.2001 SVS
    ! в активаторе замена двойных символов ~~ и ## на одинарные эквиваленты
  06.02.2001 SVS
    - Исправлен(?) баг с активатором...
      (новый кусок пока не трогать - возможно потом исключим его вообще)
  20.01.2001 SVS
    - Пропадал курсор при вызове справки.
      Бяка появилась на 354-м патче, когда была введена поддержка кеёбар
  18.12.2000 SVS
    + Дополнительный параметр у конструктора - DWORD Flags.
    + учитываем флаг FHELP_NOSHOWERROR
  18.12.2000 SVS
    - ExpandEnv забыл поставить в активаторе :-(
  07.12.2000 SVS
    ! Изменен механизм запуска URL приложения - были нарекания со стороны
      владельцев оутглюка.
  27.09.2000 SVS
    ! Разрешения для активизации URL-ссылок.
    ! Ctrl-Alt-Shift - реагируем, если надо.
  19.09.2000 OT
    - Ошибка при отрисовки хелпа
  12.09.2000 SVS
    + Параметры у функции ReadHelp и конструктора, задающие маску поиска
      файлов.
  01.09.2000 SVS
    + Мои любимые цветовые атрибуты - Учтем символ CtrlColorChar
  25.08.2000 SVS
    + CtrlAltShift - спрятать/показать помощь...
    + URL активатор - это ведь так просто :-)))
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  28.06.2000
    - NT Console resize
      adding SetScreenPosition method
  26.06.2000 IS
    - Глюк с хелпом по f1, shift+f2, end
      (решение предложил IG)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "help.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "plugin.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"

#define MAX_HELP_STRING_LENGTH 300

static int FullScreenHelp=0;

static char *PluginContents="__PluginContents__";
static char *HelpOnHelpTopic="Help";

static int RunURL(char *Protocol, char *URLPath);

Help::Help(char *Topic, char *Mask,DWORD Flags)
{
  /* $ OT По умолчанию все хелпы создаются статически*/
  SetDynamicallyBorn(FALSE);

  CanLoseFocus=FALSE;
  Help::Flags=Flags;
  /* $ 12.04.2001 SVS
     в конструкторе PrevMacroMode не инициализирован!
  */
  //if (PrevMacroMode!=MACRO_HELP) {
  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_HELP);
  //}
  /* $ SVS */

  ErrorHelp=TRUE;
  /* $ 01.09.2000 SVS
     Установим по умолчанию текущий цвет отрисовки...
  */
  CurColor=COL_HELPTEXT;
  /* SVS $ */
  /* $ 12.04.2001 SVS
     сохраним Mask
  */
  if(Mask)
  {
    HelpMask=new char[strlen(Mask)+1];
    strcpy(HelpMask,Mask);
  }
  else
    HelpMask=NULL;
  /* SVS $ */
  /* $ 07.05.2001 DJ */
  KeyBarVisible = TRUE;
  /* DJ $ */
  HelpData=NULL;
  strcpy(HelpTopic,Topic);
  *HelpPath=0;
  if (FullScreenHelp)
    SetPosition(0,0,ScrX,ScrY);
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  if(!ReadHelp(Mask) && (Flags&FHELP_USECONTENTS))
  {
    strcpy(HelpTopic,Topic);
    if(*HelpTopic == '#')
    {
      char *Ptr=strrchr(HelpTopic,'#');
      if(Ptr)
        strcpy(++Ptr,"Contents");
    }
    *HelpPath=0;
    ReadHelp(Mask);
  }
  if (HelpData!=NULL)
  {
    InitKeyBar();
    MacroMode = MACRO_HELP;
    FrameManager->ExecuteModal (this);//OT
  }
  else
  {
    if(!(Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ErrorHelp=TRUE;
  }
}

/* $ 12.04.2001 SVS
   передаем в конструктор Mask, запоминаем
*/
Help::Help(char *Topic,int &ShowPrev,int PrevFullScreen,DWORD Flags,char *Mask)
{
  /* $ OT По умолчанию все хелпы создаются статически*/
  SetDynamicallyBorn(FALSE);

  CanLoseFocus=FALSE;
  Help::Flags=Flags;
  //if (PrevMacroMode!=MACRO_HELP) {
  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_HELP);
  //}
  ErrorHelp=TRUE;
  if(Mask)
  {
    HelpMask=new char[strlen(Mask)+1];
    strcpy(HelpMask,Mask);
  }
  else
    HelpMask=NULL;
  /* $ 07.05.2001 DJ */
  KeyBarVisible = TRUE;
  /* DJ $ */
  HelpData=NULL;
  Help::PrevFullScreen=PrevFullScreen;
  strcpy(HelpTopic,Topic);
  *HelpPath=0;
  if (FullScreenHelp)
    SetPosition(0,0,ScrX,ScrY);
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  if(!ReadHelp(Mask) && (Flags&FHELP_USECONTENTS))
  {
    strcpy(HelpTopic,Topic);
    if(*HelpTopic == '#')
    {
      char *Ptr=strrchr(HelpTopic,'#');
      if(Ptr)
        strcpy(++Ptr,"Contents");
    }
    *HelpPath=0;
    ReadHelp(Mask);
  }
  /* $ 16.03.2001 VVM
    ! Если топик не найден - остаемся, где были */
  if (HelpData!=NULL)
  {
    InitKeyBar();
    MacroMode = MACRO_HELP;
    FrameManager->ExecuteModal (this);//OT
    ShowPrev=Help::ShowPrev;
  }
  else
  {
    if(!(Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ErrorHelp=TRUE;
    ShowPrev=TRUE;
    FrameManager->DeleteFrame();

  }
  /* VVM $ */
}
/* SVS $ */


Help::~Help()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
  SetRestoreScreenMode(FALSE);
  /* $ 13.07.2000
    для выделения памяти использовалась функция realloc
  */
  free(HelpData);
  /* SVS $ */
  /* $ 12.04.2001 SVS
     удаляем HelpMask
  */
  if(HelpMask)
    delete HelpMask;
  /* SVS $ */

}


void Help::Hide()
{
  ScreenObject::Hide();
}


int Help::ReadHelp(char *Mask)
{
  char FileName[NM],ReadStr[MAX_HELP_STRING_LENGTH];
  char SplitLine[2*MAX_HELP_STRING_LENGTH];
  int Formatting=TRUE,RepeatLastLine;
  const int MaxLength=X2-X1-1;

  ShowPrev=0;
  DisableOut=0;

  char Path[NM],*TopicPtr;
  if (*HelpTopic=='#')
  {
    strcpy(Path,HelpTopic+1);
    if ((TopicPtr=strchr(Path,'#'))==NULL)
      return FALSE;
    strcpy(HelpTopic,TopicPtr+1);
    *TopicPtr=0;
    strcpy(HelpPath,Path);
  }
  else
    strcpy(Path,*HelpPath ? HelpPath:FarPath);

  if (strcmp(HelpTopic,PluginContents)==0)
  {
    ReadPluginsHelp();
    return TRUE;
  }

  FILE *HelpFile=Language::OpenLangFile(Path,(!Mask?HelpFileMask:Mask),Opt.HelpLanguage,FileName);

  if (HelpFile==NULL)
  {
    if(!(Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MHelpTitle),MSG(MCannotOpenHelp),FileName,MSG(MOk));
    ErrorHelp=TRUE;
    return FALSE;
  }
  if (Language::GetOptionsParam(HelpFile,"CtrlColorChar",ReadStr))
  {
    CtrlColorChar=ReadStr[0];
  }
  else
    CtrlColorChar=0;
  *SplitLine=0;
  CurX=CurY=0;
  if (HelpData!=NULL)
    /* $ 13.07.2000
      для выделения памяти использовалась функция realloc
    */
    free(HelpData);
    /* SVS $ */
  HelpData=NULL;
  StrCount=0;
  FixCount=0;
  TopStr=0;
  TopicFound=0;
  RepeatLastLine=FALSE;
  int NearTopicFound=0;
  char PrevSymbol=0;
  while (TRUE)
  {
    if (!RepeatLastLine && fgets(ReadStr,sizeof(ReadStr),HelpFile)==NULL)
    {
      if (StringLen(SplitLine)<MaxLength)
      {
        if (*SplitLine)
          AddLine(SplitLine);
      }
      else
      {
        *ReadStr=0;
        RepeatLastLine=TRUE;
        continue;
      }
      break;
    }
    RepeatLastLine=FALSE;

    RemoveTrailingSpaces(ReadStr);

    if (TopicFound)
      HighlightsCorrection(ReadStr);

    if (*ReadStr=='@')
    {
      if (TopicFound)
      {
        if (strcmp(ReadStr,"@+")==0)
        {
          Formatting=TRUE;
          PrevSymbol=0;
          continue;
        }
        if (strcmp(ReadStr,"@-")==0)
        {
          Formatting=FALSE;
          PrevSymbol=0;
          continue;
        }
        if (*SplitLine)
          AddLine(SplitLine);
        break;
      }
      else
        if (LocalStricmp(ReadStr+1,HelpTopic)==0)
        {
          TopicFound=1;
          NearTopicFound=1;
        }
    }
    else
      if (TopicFound)
      {
        /* $<text> в начале строки, определение темы
           Определяет не прокручиваемую область помощи
           Если идут несколько подряд сразу после строки обозначения темы...
        */
        if (*ReadStr=='$' && NearTopicFound && (PrevSymbol == '$' || PrevSymbol == '@'))
        {
          AddLine(ReadStr+1);
          FixCount++;
        }
        else
        {
          NearTopicFound=0;
          if (*ReadStr==0 || !Formatting)
            if (*SplitLine)
              if (StringLen(SplitLine)<MaxLength)
              {
                AddLine(SplitLine);
                *SplitLine=0;
                if (StringLen(ReadStr)<MaxLength)
                {
                  AddLine(ReadStr);
                  continue;
                }
              }
              else
                RepeatLastLine=TRUE;
            else
              if (StringLen(ReadStr)<MaxLength)
              {
                AddLine(ReadStr);
                continue;
              }
          if (isspace(*ReadStr) && Formatting)
            if (StringLen(SplitLine)<MaxLength)
            {
              if (*SplitLine)
                AddLine(SplitLine);
              strcpy(SplitLine,ReadStr);
              *ReadStr=0;
              continue;
            }
            else
              RepeatLastLine=TRUE;
          if (!RepeatLastLine)
          {
            if (*SplitLine)
              strcat(SplitLine," ");
            strcat(SplitLine,ReadStr);
          }
          if (StringLen(SplitLine)<MaxLength)
            continue;
          int Splitted=0;
          for (int I=strlen(SplitLine)-1;I>0;I--)
          {
            if (I>0 && SplitLine[I]=='~' && SplitLine[I-1]=='~')
            {
              I--;
              continue;
            }
            if (I>0 && SplitLine[I]=='~' && SplitLine[I-1]!='~')
            {
              do {
                I--;
              } while (I>0 && SplitLine[I]!='~');
              continue;
            }
            if (SplitLine[I]==' ')
            {
              SplitLine[I]=0;
              if (StringLen(SplitLine)<MaxLength)
              {
                AddLine(SplitLine);
                char CopyLine[2*MAX_HELP_STRING_LENGTH];
                strcpy(CopyLine,SplitLine+I+1);
                *SplitLine=' ';
                strcpy(SplitLine+1,CopyLine);
                HighlightsCorrection(SplitLine);
                Splitted=TRUE;
                break;
              }
              else
                SplitLine[I]=' ';
            }
          }
          if (!Splitted)
          {
            AddLine(SplitLine);
            *SplitLine=0;
          }
        }
      }
    PrevSymbol=*ReadStr;
  }

  fclose(HelpFile);
  FixSize=FixCount+(FixCount!=0);
  ErrorHelp=FALSE;
  return TopicFound != 0;
}


void Help::AddLine(char *Line)
{
  char *NewHelpData=(char *)realloc(HelpData,(StrCount+1)*MAX_HELP_STRING_LENGTH);
  if (NewHelpData==NULL)
    return;
  HelpData=NewHelpData;
  strncpy(HelpData+StrCount*MAX_HELP_STRING_LENGTH,Line,MAX_HELP_STRING_LENGTH);
  StrCount++;
}


void Help::HighlightsCorrection(char *Str)
{
  int I,Count;
  for (I=0,Count=0;Str[I]!=0;I++)
    if (Str[I]=='#')
      Count++;
  if ((Count & 1) && *Str!='$')
  {
    char CopyLine[MAX_HELP_STRING_LENGTH];
    strcpy(CopyLine,Str);
    *Str='#';
    strcpy(Str+1,CopyLine);
  }
}


void Help::DisplayObject()
{
  if (!TopicFound)
  {
    if(!(Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ProcessKey(KEY_ALTF1);
    ErrorHelp=TRUE;
    return;
  }
  SetCursorType(0,10);
  MoveToReference(1,1);
  FastShow();
  if (!FullScreenHelp)
  {
    HelpKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
    if(Opt.ShowKeyBar)
       HelpKeyBar.Show();
  }
  else
    HelpKeyBar.Hide();
}


void Help::FastShow()
{
  int I;
  if (!DisableOut)
  {
    SetScreen(X1,Y1,X2,Y2,' ',COL_HELPTEXT);
    Box(X1,Y1,X2,Y2,COL_HELPBOX,DOUBLE_BOX);
    SetColor(COL_HELPBOXTITLE);
    GotoXY(X1+(X2-X1+1-strlen(MSG(MHelpTitle))-2)/2,Y1);
    mprintf(" %s ",MSG(MHelpTitle));
  }
  CorrectPosition();
  *SelTopic=0;
  /* $ 01.09.2000 SVS
     Установим по умолчанию текущий цвет отрисовки...
     чтобы новая тема начиналась с нормальными атрибутами
  */
  CurColor=COL_HELPTEXT;
  /* SVS $ */
  for (I=0;I<Y2-Y1-1;I++)
  {
    int StrPos;
    if (I<FixCount)
      StrPos=I;
    else
      if (I==FixCount && FixCount>0)
      {
        if (!DisableOut)
        {
          GotoXY(X1,Y1+I+1);
          SetColor(COL_HELPBOX);
          ShowSeparator(X2-X1+1);
        }
        continue;
      }
      else
      {
        StrPos=I+TopStr;
        if (FixCount>0)
          StrPos--;
      }
    if (StrPos<StrCount)
    {
      char *OutStr=HelpData+StrPos*MAX_HELP_STRING_LENGTH;
      if (*OutStr=='^')
      {
        GotoXY(X1+(X2-X1+1-StringLen(OutStr))/2,Y1+I+1);
        OutStr++;
      }
      else
        GotoXY(X1+1,Y1+I+1);
      OutString(OutStr);
    }
  }

  const int ScrollLength=Y2-Y1-FixSize-1;
  if (!DisableOut && StrCount-FixCount > ScrollLength)
  {
    int Scrolled=StrCount-FixCount-ScrollLength;
    SetColor(COL_HELPSCROLLBAR);
    ScrollBar(X2,Y1+FixSize+1,ScrollLength,TopStr,Scrolled);
  }
}

/* $ 01.09.2000 SVS
  Учтем символ CtrlColorChar & CurColor
*/
void Help::OutString(char *Str)
{
  char OutStr[512],*StartTopic=NULL;
  int OutPos=0,Highlight=0,Topic=0;
  while (OutPos<sizeof(OutStr)-10)
  {
    if (Str[0]=='~' && Str[1]=='~' ||
        Str[0]=='#' && Str[1]=='#' ||
        Str[0]=='@' && Str[1]=='@' ||
        (CtrlColorChar && Str[0]==CtrlColorChar && Str[1]==CtrlColorChar)
        )
    {
      OutStr[OutPos++]=*Str;
      Str+=2;
      continue;
    }

    if (*Str=='~' || *Str=='#' || *Str==0 || *Str == CtrlColorChar)
    {
      OutStr[OutPos]=0;
      if (Topic)
      {
        int RealCurX,RealCurY;
        RealCurX=X1+CurX+1;
        RealCurY=Y1+CurY+FixSize+1;
        if (WhereY()==RealCurY && RealCurX>=WhereX() &&
                RealCurX<WhereX()+(Str-StartTopic)-1)
        {
          SetColor(COL_HELPSELECTEDTOPIC);
          if (Str[1]=='@')
          {
            strncpy(SelTopic,Str+2,sizeof(SelTopic));
            char *EndPtr=strchr(SelTopic,'@');
            /* $ 25.08.2000 SVS
               учтем, что может быть такой вариант: @@ или \@
               этот вариант только для URL!
            */
            if (EndPtr!=NULL)
            {
              if(*(EndPtr+1) == '@')
              {
                memmove(EndPtr,EndPtr+1,strlen(EndPtr)+1);
                EndPtr++;
              }
              EndPtr=strchr(EndPtr,'@');
              if (EndPtr!=NULL)
                *EndPtr=0;
            }
            /* SVS $ */
          }
        }
        else
          SetColor(COL_HELPTOPIC);
      }
      else
        if (Highlight)
          SetColor(COL_HELPHIGHLIGHTTEXT);
        else
          SetColor(CurColor);
      if (DisableOut)
        GotoXY(WhereX()+strlen(OutStr),WhereY());
      else
        Text(OutStr);
//if(WhereX()>=X2-10) MessageBeep(0);
      OutPos=0;
    }

    if (*Str==0)
      break;

    if (*Str=='~')
    {
      if (!Topic)
        StartTopic=Str;
      Topic=!Topic;
      Str++;
      continue;
    }
    if (*Str=='@')
    {
      /* $ 25.08.2000 SVS
         учтем, что может быть такой вариант: @@
         этот вариант только для URL!
      */
      while (*Str)
        if (*(++Str)=='@' && *(Str-1)!='@')
          break;
      /* SVS $ */
      Str++;
      continue;
    }
    if (*Str=='#')
    {
      Highlight=!Highlight;
      Str++;
      continue;
    }
    if (*Str == CtrlColorChar)
    {
      WORD Chr;

      Chr=(BYTE)Str[1];
      if(Chr == '-') // "\-" - установить дефолтовый цвет
      {
        Str+=2;
        CurColor=COL_HELPTEXT;
        continue;
      }
      if(isxdigit(Chr) && isxdigit(Str[2]))
      {
        WORD Attr;

        if(Chr >= '0' && Chr <= '9') Chr-='0';
        else { Chr&=~0x20; Chr=Chr-'A'+10; }
        Attr=(Chr<<4)&0x00F0;

        // next char
        Chr=Str[2];
        if(Chr >= '0' && Chr <= '9') Chr-='0';
        else { Chr&=~0x20; Chr=Chr-'A'+10; }
        Attr|=(Chr&0x000F);
        CurColor=Attr;
        Str+=3;
        continue;
      }
    }
    OutStr[OutPos++]=*(Str++);
  }
  if (!DisableOut && WhereX()<X2)
  {
    SetColor(CurColor);
    mprintf("%*s",X2-WhereX(),"");
  }
}


int Help::StringLen(char *Str)
{
  int Length=0;
  while (*Str)
  {
    if (Str[0]=='~' && Str[1]=='~' ||
        Str[0]=='#' && Str[1]=='#' ||
        Str[0]=='@' && Str[1]=='@' ||
        (CtrlColorChar && Str[0]==CtrlColorChar && Str[1]==CtrlColorChar)
       )
    {
      Length++;
      Str+=2;
      continue;
    }
    if (*Str=='@')
    {
      /* $ 25.08.2000 SVS
         учтем, что может быть такой вариант: @@
         этот вариант только для URL!
      */
      while (*Str)
        if (*(++Str)=='@' && *(Str-1)!='@')
          break;
      /* SVS $ */
      Str++;
      continue;
    }
    /* $ 01.09.2000 SVS
       учтем наше нововведение \XX или \-
    */
    if(*Str == CtrlColorChar)
    {
      if(Str[1] == '-')
      {
        Str+=2;
        continue;
      }

      if(isxdigit(Str[1]) && isxdigit(Str[2]))
      {
        Str+=3;
        continue;
      }
    }
    /* SVS $ */

    if (*Str!='#' && *Str!='~')
      Length++;
    Str++;
  }
  return(Length);
}


void Help::CorrectPosition()
{
  if (CurX>X2-X1-2)
    CurX=X2-X1-2;
  if (CurX<0)
    CurX=0;
  if (CurY>Y2-Y1-2-FixSize)
  {
    TopStr+=CurY-(Y2-Y1-2-FixSize);
    CurY=Y2-Y1-2-FixSize;
  }
  if (CurY<0)
  {
    TopStr+=CurY;
    CurY=0;
  }
  if (TopStr>StrCount-FixCount-(Y2-Y1-1-FixSize))
    TopStr=StrCount-FixCount-(Y2-Y1-1-FixSize);
  if (TopStr<0)
    TopStr=0;
}


int Help::ProcessKey(int Key)
{
  if (*SelTopic==0)
    CurX=CurY=0;
  switch(Key)
  {
    case KEY_NONE:
    case KEY_IDLE:
      break;
    /* $ 25.08.2000 SVS
       + CtrlAltShift - спрятать/показать помощь...
       // "ХАчу глянуть на то, что под диалогом..."
    */
    case KEY_CTRLALTSHIFTPRESS:
    {
       if(Opt.AllCtrlAltShiftRule & CASR_HELP)
       {
         Hide();
         WaitKey(KEY_CTRLALTSHIFTRELEASE);
         Show();
       }
       return(TRUE);
    }
    /* SVS $ */
    case KEY_F1:
      /* $ 16.04.2001 SVS
         - не поганим SelTopic, если и так в Help on Help
      */
      if(LocalStricmp(HelpTopic,HelpOnHelpTopic)!=0)
      {
        strcpy(SelTopic,HelpOnHelpTopic);
        ProcessKey(KEY_ENTER);
      }
      /* SVS $ */
      return(TRUE);

    case KEY_F5:
      FullScreenHelp=!FullScreenHelp;
      ResizeConsole();
      return(TRUE);

    case KEY_ESC:
    case KEY_F10:
      ShowPrev=FALSE;
      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      return(TRUE);
    case KEY_ALTF1:
    case KEY_BS:
      ShowPrev=TRUE;
      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      return(TRUE);
    case KEY_HOME:
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      CurX=CurY=0;
      TopStr=0;
      FastShow();
      if (*SelTopic==0)
        MoveToReference(1,1);
      return(TRUE);
    case KEY_END:
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      CurX=CurY=0;
      TopStr=StrCount;
      FastShow();
      if (*SelTopic==0)
      {
        CurX=0;
        CurY=Y2-Y1-2-FixSize;
        MoveToReference(0,1);
      }
      return(TRUE);
    case KEY_UP:
      if (TopStr>0)
      {
        TopStr--;
        if (CurY<Y2-Y1-2-FixSize)
        {
          CurX=X2-X1-2;
          CurY++;
        }
        FastShow();
        if (*SelTopic==0)
          MoveToReference(0,1);
      }
      else
        ProcessKey(KEY_SHIFTTAB);
      return(TRUE);
    case KEY_DOWN:
      if (TopStr<StrCount-FixCount-(Y2-Y1-1-FixSize))
      {
        TopStr++;
        if (CurY>0)
          CurY--;
        CurX=0;
        FastShow();
        if (*SelTopic==0)
          MoveToReference(1,1);
      }
      else
        ProcessKey(KEY_TAB);
      return(TRUE);
    /* $ 07.05.2001 DJ
      + Обработка KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_UP:
      {
        for (int i=0; i<Opt.MsWheelDelta; i++)
          ProcessKey(KEY_UP);
        return(TRUE);
      }
    case KEY_MSWHEEL_DOWN:
      {
        for (int i=0; i<Opt.MsWheelDelta; i++)
          ProcessKey(KEY_DOWN);
        return(TRUE);
      }
    /* DJ $ */
    case KEY_PGUP:
      CurX=CurY=0;
      TopStr-=Y2-Y1-2-FixSize;
      FastShow();
      if (*SelTopic==0)
      {
        CurX=CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    case KEY_PGDN:
      {
        int PrevTopStr=TopStr;
        TopStr+=Y2-Y1-2-FixSize;
        FastShow();
        if (TopStr==PrevTopStr)
        {
          ProcessKey(KEY_CTRLPGDN);
          return(TRUE);
        }
        else
          CurX=CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    case KEY_ENTER:
      if (*SelTopic && LocalStricmp(HelpTopic,SelTopic)!=0)
      {
        int KeepHelp;
        int CurFullScreen=FullScreenHelp;
        {
          char NewTopic[512];
          /* $ 25.08.2000 SVS
             URL активатор - это ведь так просто :-)))
          */
          {
            strcpy(NewTopic,SelTopic);
            char *p=strchr(NewTopic,':');
            if(p && NewTopic[0] != ':') // наверное подразумевается URL
            {
              *p=0;
              if(RunURL(NewTopic,SelTopic))
                return(TRUE);
            }
            // а вот теперь попробуем...
          }
          /* SVS $ */
          if (*HelpPath && *SelTopic!='#' && strcmp(SelTopic,HelpOnHelpTopic)!=0)
          {
            if (*SelTopic==':')
              strcpy(NewTopic,SelTopic+1);
            else
              sprintf(NewTopic,"#%s#%s",HelpPath,SelTopic);
          }
          else
            strcpy(NewTopic,SelTopic);

          /* $ 12.04.2001 SVS
             передаем запомненный HelpMask
          */
          char *NewHelpMask=NULL;
          if(*SelTopic != ':' && LocalStricmp(SelTopic,PluginContents) != 0)
            NewHelpMask=HelpMask;
          Help *NewHelp=new Help(NewTopic,KeepHelp,FullScreenHelp,0,NewHelpMask);
          /* SVS $ */
        }
        if (!KeepHelp)
        {
          FrameManager->DeleteFrame();
          SetExitCode (XC_QUIT);
        }
        else
        {
          if (CurFullScreen!=FullScreenHelp)
          {
            FullScreenHelp=!FullScreenHelp;
            ProcessKey(KEY_F5);
          }
        }
      }
      return(TRUE);
    case KEY_SHIFTF1:
      /* $ 12.04.2001 SVS
         не поганим SelTopic, если и так в теме Contents
      */
      if(LocalStricmp(HelpTopic,"Contents")!=0)
      {
        strcpy(SelTopic,"Contents");
        ProcessKey(KEY_ENTER);
      }
      /* SVS $ */
      return(TRUE);
    case KEY_SHIFTF2:
      /* $ 12.04.2001 SVS
         не поганим SelTopic, если и так в PluginContents
      */
      if(LocalStricmp(HelpTopic,PluginContents)!=0)
      {
        strcpy(SelTopic,PluginContents);
        ProcessKey(KEY_ENTER);
      }
      /* SVS $ */
      return(TRUE);
    case KEY_RIGHT:
    case KEY_TAB:
      MoveToReference(1,0);
      return(TRUE);
    case KEY_LEFT:
    case KEY_SHIFTTAB:
      MoveToReference(0,0);
      return(TRUE);
  }
  return(FALSE);
}


int Help::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (MouseEvent->dwEventFlags==MOUSE_MOVED && (MouseEvent->dwButtonState & 3)==0)
    return(FALSE);
  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2)
  {
    ProcessKey(KEY_ESC);
    return(TRUE);
  }
  if (MouseX==X2 && (MouseEvent->dwButtonState & 1))
  {
    int ScrollY=Y1+FixSize+1;
    int Height=Y2-Y1-FixSize-1;
    if (MouseY==ScrollY)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_UP);
      return(TRUE);
    }
    if (MouseY==ScrollY+Height-1)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
  }
  if (MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
  {
    while (IsMouseButtonPressed() && MouseY<Y1+1+FixSize)
      ProcessKey(KEY_UP);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y>=Y2)
  {
    while (IsMouseButtonPressed() && MouseY>=Y2)
      ProcessKey(KEY_DOWN);
    return(TRUE);
  }
  CurX=MouseEvent->dwMousePosition.X-X1-1;
  CurY=MouseEvent->dwMousePosition.Y-Y1-1-FixSize;
  FastShow();
  if ((MouseEvent->dwButtonState & 3)==0 && *SelTopic)
    ProcessKey(KEY_ENTER);
  return(TRUE);
}


int Help::IsReferencePresent()
{
  CorrectPosition();
  int StrPos=FixCount+TopStr+CurY;
  /* $ 19.09.2000 OT
    Ошибка при отрисовки хелпа
    */
  if (StrPos >= StrCount) {
    return FALSE;
  }
  /* OT 19.09.2000 $ */
  char *OutStr=HelpData+StrPos*MAX_HELP_STRING_LENGTH;
  return (strchr(OutStr,'@')!=NULL && strchr(OutStr,'~')!=NULL);
}


void Help::MoveToReference(int Forward,int CurScreen)
{
  int StartSelection=*SelTopic;
  int SaveCurX=CurX,SaveCurY=CurY,SaveTopStr=TopStr;
  *SelTopic=0;
  DisableOut=TRUE;
  while (*SelTopic==0)
  {
    if (Forward)
    {
      if (CurX==0 && !IsReferencePresent())
        CurX=X2-X1-2;
      if (++CurX >= X2-X1-2)
      {
        StartSelection=0;
        CurX=0;
        CurY++;
        if (TopStr+CurY>=StrCount-FixCount ||
            CurScreen && CurY>Y2-Y1-2-FixSize)
          break;
      }
    }
    else
    {
      if (CurX==X2-X1-2 && !IsReferencePresent())
        CurX=0;
      if (--CurX < 0)
      {
        StartSelection=0;
        CurX=X2-X1-2;
        CurY--;
        if (TopStr+CurY<0 || CurScreen && CurY<0)
          break;
      }
    }
    FastShow();
    if (*SelTopic==0)
      StartSelection=0;
    else
      if (StartSelection)
        *SelTopic=0;
  }
  DisableOut=FALSE;
  if (*SelTopic==0)
  {
    CurX=SaveCurX;
    CurY=SaveCurY;
    TopStr=SaveTopStr;
  }
  FastShow();
}


int Help::GetFullScreenMode()
{
  return(FullScreenHelp);
}


void Help::SetFullScreenMode(int Mode)
{
  FullScreenHelp=Mode;
}


void Help::ReadPluginsHelp()
{
  /* $ 13.07.2000
    для выделения памяти использовалась функция realloc
  */
  free(HelpData);
  /* SVS $ */
  HelpData=NULL;
  StrCount=0;
  FixCount=1;
  FixSize=2;
  TopStr=0;
  TopicFound=TRUE;
  CurX=CurY=0;
  CtrlColorChar=0;
  char PluginsHelpTitle[100];
  sprintf(PluginsHelpTitle,"^ #%s#",MSG(MPluginsHelpTitle));
  AddLine(PluginsHelpTitle);

  for (int I=0;I<CtrlObject->Plugins.PluginsCount;I++)
  {
    char Path[NM],FileName[NM],*Slash;
    strcpy(Path,CtrlObject->Plugins.PluginsData[I].ModuleName);
    if ((Slash=strrchr(Path,'\\'))!=NULL)
      *Slash=0;
    FILE *HelpFile=Language::OpenLangFile(Path,HelpFileMask,Opt.HelpLanguage,FileName);
    if (HelpFile!=NULL)
    {
      char EntryName[512],HelpLine[512],SecondParam[512];
      if (Language::GetLangParam(HelpFile,"PluginContents",EntryName,SecondParam))
      {
        if (*SecondParam)
          sprintf(HelpLine,"   ~%s,%s~@#%s#Contents@",EntryName,SecondParam,Path);
        else
          sprintf(HelpLine,"   ~%s~@#%s#Contents@",EntryName,Path);
        AddLine(HelpLine);
      }

      fclose(HelpFile);
    }
  }
  /* $ 26.06.2000 IS
   Устранение глюка с хелпом по f1, shift+f2, end (решение предложил IG)
  */
  AddLine("");
  /* IS $ */
}


int Help::PluginPanelHelp(HANDLE hPlugin)
{
  int PluginNumber=((struct PluginHandle *)hPlugin)->PluginNumber;
  char Path[NM],FileName[NM],StartTopic[256],*Slash;
  strcpy(Path,CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
  if ((Slash=strrchr(Path,'\\'))!=NULL)
    *Slash=0;
  FILE *HelpFile=Language::OpenLangFile(Path,HelpFileMask,Opt.HelpLanguage,FileName);
  if (HelpFile==NULL)
    return(FALSE);
  fclose(HelpFile);
  sprintf(StartTopic,"#%s#Contents",Path);
  Help PanelHelp(StartTopic);
  return(TRUE);
}

/* $ 28.06.2000 tran
 (NT Console resize)
 resize help*/
void Help::SetScreenPosition()
{
  if (FullScreenHelp)
    SetPosition(0,0,ScrX,ScrY);
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  Show();
}
/* tran $ */

/* $ 30.12.2000 SVS
  Функция инициализации KeyBar Labels
*/
void Help::InitKeyBar(void)
{
  char *FHelpKeys[]={MSG(MHelpF1),MSG(MHelpF2),MSG(MHelpF3),MSG(MHelpF4),MSG(MHelpF5),MSG(MHelpF6),MSG(MHelpF7),MSG(MHelpF8),MSG(MHelpF9),MSG(MHelpF10),MSG(MHelpF11),MSG(MHelpF12)};
  char *FHelpShiftKeys[]={MSG(MHelpShiftF1),MSG(MHelpShiftF2),MSG(MHelpShiftF3),MSG(MHelpShiftF4),MSG(MHelpShiftF5),MSG(MHelpShiftF6),MSG(MHelpShiftF7),MSG(MHelpShiftF8),MSG(MHelpShiftF9),MSG(MHelpShiftF10),MSG(MHelpShiftF11),MSG(MHelpShiftF12)};
  char *FHelpAltKeys[]={MSG(MHelpAltF1),MSG(MHelpAltF2),MSG(MHelpAltF3),MSG(MHelpAltF4),MSG(MHelpAltF5),MSG(MHelpAltF6),MSG(MHelpAltF7),MSG(MHelpAltF8),MSG(MHelpAltF9),MSG(MHelpAltF10),MSG(MHelpAltF11),MSG(MHelpAltF12)};
  char *FHelpCtrlKeys[]={MSG(MHelpCtrlF1),MSG(MHelpCtrlF2),MSG(MHelpCtrlF3),MSG(MHelpCtrlF4),MSG(MHelpCtrlF5),MSG(MHelpCtrlF6),MSG(MHelpCtrlF7),MSG(MHelpCtrlF8),MSG(MHelpCtrlF9),MSG(MHelpCtrlF10),MSG(MHelpCtrlF11),MSG(MHelpCtrlF12)};
//  char *FHelpAltShiftKeys[]={MSG(MHelpAltShiftF1),MSG(MHelpAltShiftF2),MSG(MHelpAltShiftF3),MSG(MHelpAltShiftF4),MSG(MHelpAltShiftF5),MSG(MHelpAltShiftF6),MSG(MHelpAltShiftF7),MSG(MHelpAltShiftF8),MSG(MHelpAltShiftF9),MSG(MHelpAltShiftF10),MSG(MHelpAltShiftF11),MSG(MHelpAltShiftF12)};
//  char *FHelpCtrlShiftKeys[]={MSG(MHelpCtrlShiftF1),MSG(MHelpCtrlShiftF2),MSG(MHelpCtrlShiftF3),MSG(MHelpCtrlShiftF4),MSG(MHelpCtrlShiftF5),MSG(MHelpCtrlShiftF6),MSG(MHelpCtrlShiftF7),MSG(MHelpCtrlShiftF8),MSG(MHelpCtrlShiftF9),MSG(MHelpCtrlShiftF10),MSG(MHelpCtrlShiftF11),MSG(MHelpCtrlShiftF12)};
//  char *FHelpCtrlAltKeys[]={MSG(MHelpCtrlAltF1),MSG(MHelpCtrlAltF2),MSG(MHelpCtrlAltF3),MSG(MHelpCtrlAltF4),MSG(MHelpCtrlAltF5),MSG(MHelpCtrlAltF6),MSG(MHelpCtrlAltF7),MSG(MHelpCtrlAltF8),MSG(MHelpCtrlAltF9),MSG(MHelpCtrlAltF10),MSG(MHelpCtrlAltF11),MSG(MHelpCtrlAltF12)};
  char *FHelpAltShiftKeys[]={"","","","","","","","","","","",""};
  char *FHelpCtrlShiftKeys[]={"","","","","","","","","","","",""};
  char *FHelpCtrlAltKeys[]={"","","","","","","","","","","",""};

  HelpKeyBar.Set(FHelpKeys,sizeof(FHelpKeys)/sizeof(FHelpKeys[0]));
  HelpKeyBar.SetShift(FHelpShiftKeys,sizeof(FHelpShiftKeys)/sizeof(FHelpShiftKeys[0]));
  HelpKeyBar.SetAlt(FHelpAltKeys,sizeof(FHelpAltKeys)/sizeof(FHelpAltKeys[0]));
  HelpKeyBar.SetCtrl(FHelpCtrlKeys,sizeof(FHelpCtrlKeys)/sizeof(FHelpCtrlKeys[0]));
  HelpKeyBar.SetCtrlAlt(FHelpCtrlAltKeys,sizeof(FHelpCtrlAltKeys)/sizeof(FHelpCtrlAltKeys[0]));
  HelpKeyBar.SetCtrlShift(FHelpCtrlShiftKeys,sizeof(FHelpCtrlShiftKeys)/sizeof(FHelpCtrlShiftKeys[0]));
  HelpKeyBar.SetAltShift(FHelpAltShiftKeys,sizeof(FHelpAltShiftKeys)/sizeof(FHelpAltShiftKeys[0]));

  SetKeyBar(&HelpKeyBar);
}
/* SVS $ */

/* $ 25.08.2000 SVS
   Запуск URL-ссылок... ;-)
   Это ведь так просто... ась?
   Вернет:
     0 - это не URL ссылка (не похожа)
     1 - CreateProcess вернул FALSE
     2 - Все Ок

   Параметры (например):
     Protocol="mailto"
     URLPath ="mailto:vskirdin@mail.ru?Subject=Reversi"
*/
#if 1
static int RunURL(char *Protocol, char *URLPath)
{
  int EditCode=0;
  if(Protocol && *Protocol && URLPath && *URLPath && (Opt.HelpURLRules&0xFF))
  {
    char *Buf=(char*)malloc(2048);
    if(Buf)
    {
      HKEY hKey;
      DWORD Disposition, DataSize=250;
      strcpy(Buf,Protocol);
      strcat(Buf,"\\shell\\open\\command");
      if(RegOpenKeyEx(HKEY_CLASSES_ROOT,Buf,0,KEY_READ,&hKey) == ERROR_SUCCESS)
      {
        Disposition=RegQueryValueEx(hKey,"",0,&Disposition,(LPBYTE)Buf,&DataSize);
        ExpandEnvironmentStr(Buf, Buf, 2048);
        RegCloseKey(hKey);
        if(Disposition == ERROR_SUCCESS)
        {
          char *pp=strrchr(Buf,'%');
          if(pp) *pp='\0'; else strcat(Buf," ");

          // удалим два идущих в подряд ~~
          pp=URLPath;
          while(*pp && (pp=strstr(pp,"~~")) != NULL)
          {
            memmove(pp,pp+1,strlen(pp+1)+1);
            ++pp;
          }
          // удалим два идущих в подряд ##
          pp=URLPath;
          while(*pp && (pp=strstr(pp,"##")) != NULL)
          {
            memmove(pp,pp+1,strlen(pp+1)+1);
            ++pp;
          }

          Disposition=0;
          if(Opt.HelpURLRules == 2 || Opt.HelpURLRules == 2+256)
            Disposition=Message(MSG_WARNING,2,MSG(MHelpTitle),
                        MSG(MHelpActivatorURL),
                        Buf,
                        MSG(MHelpActivatorFormat),
                        URLPath,
                        "\x01",
                        MSG(MHelpActivatorQ),
                        MSG(MYes),MSG(MNo));

          EditCode=2; // Все Ok!
          if(Disposition == 0)
          {
            /*
              СЮДЫ НУЖНО ВПИНДЮЛИТЬ МЕНЮХУ С ВОЗМОЖНОСТЬЮ ВЫБОРА
              ТОГО ИЛИ ИНОГО АКТИВАТОРА - ИХ МОЖЕТ БЫТЬ НЕСКОЛЬКО!!!!!
            */
            if(Opt.HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
            {
#if 0
              SHELLEXECUTEINFO sei;

              OemToChar(URLPath,Buf);
              memset(&sei,0,sizeof(sei));
              sei.cbSize=sizeof(sei);
              sei.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
              sei.lpFile=RemoveExternalSpaces(Buf);
              sei.nShow=SW_SHOWNORMAL;
              SetFileApisToANSI();
              if(ShellExecuteEx(&sei))
                EditCode=1;
              SetFileApisToOEM();
#else
              OemToChar(URLPath,Buf);
              SetFileApisToANSI();
              EditCode=ShellExecute(0, 0, RemoveExternalSpaces(Buf), 0, 0, SW_SHOWNORMAL)?1:2;
              SetFileApisToOEM();
#endif
            }
            else
            {
              STARTUPINFO si={0};
              PROCESS_INFORMATION pi={0};
              si.cb=sizeof(si);
              strcat(Buf,URLPath);
              OemToChar(Buf,Buf);
              SetFileApisToANSI(); //????
              if(!CreateProcess(NULL,Buf,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
                 EditCode=1;
              SetFileApisToOEM(); //????
            }
          }
        }
      }
      free(Buf);
    }
  }
  return EditCode;
}
#else
// ЕЩЕ ОДИН ВАРИАНТ
static int RunURL(char *Protocol, char *URLPath)
{
  if (!(Protocol && *Protocol && URLPath && *URLPath && (Opt.HelpURLRules&0xFF)))
    return 0;
  char *Buf;
  if (!(Buf=(char*)malloc(2048)))
    return 0;
  int EditCode=0;
  if(Opt.HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
  {
    if(((Opt.HelpURLRules&0xFF) != 2) || Message(MSG_WARNING,2,MSG(MHelpTitle),
                                                 MSG(MHelpActivatorURL),
                                                 URLPath,
                                                 "\x01",
                                                 MSG(MHelpActivatorQ),
                                                 MSG(MYes),MSG(MNo)) == 0)
    {
      SHELLEXECUTEINFO sei;
      strcpy(Buf,URLPath);
      OemToChar(Buf,Buf);
      memset(&sei,0,sizeof(sei));
      sei.cbSize=sizeof(sei);
      sei.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
      sei.lpFile=RemoveExternalSpaces(Buf);
      sei.nShow=SW_SHOWNORMAL;
      SetFileApisToANSI();
      EditCode=ShellExecuteEx(&sei)?1:2;
      SetFileApisToOEM();
    }
  }
  else // CREATEPROCESS_METHOD
  {
    HKEY hKey;
    strcat(strcpy(Buf,Protocol),"\\shell\\open\\command");
    if(RegOpenKeyEx(HKEY_CLASSES_ROOT,Buf,0,KEY_READ,&hKey) == ERROR_SUCCESS)
    {
      DWORD Disposition, DataSize=250;
      Disposition=RegQueryValueEx(hKey,"",0,&Disposition,(LPBYTE)Buf,&DataSize);
      RegCloseKey(hKey);
      if(Disposition != ERROR_SUCCESS)
        return 0;
      ExpandEnvironmentStr(Buf, Buf, 2048);
      char *pp=strrchr(Buf,'%');
      if(pp) *pp='\0'; else pp=Buf+strlen(strcat(Buf," "));
      if(((Opt.HelpURLRules&0xFF) != 2) ||
           Message(MSG_WARNING,2,MSG(MHelpTitle),
                   MSG(MHelpActivatorURL),
                   Buf,
                   MSG(MHelpActivatorFormat),
                   URLPath,
                   "\x01",
                   MSG(MHelpActivatorQ),
                   MSG(MYes),MSG(MNo)) == 0)
      {
        strcpy(pp, URLPath);
        OemToChar(pp,pp);
        STARTUPINFO si={0};
        PROCESS_INFORMATION pi={0};
        si.cb=sizeof(si);
        SetFileApisToANSI(); //????
        EditCode=CreateProcess(NULL,Buf,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)?2:1;
        SetFileApisToOEM(); //????
      }
    }
  }
  free(Buf);
  return EditCode;
}
#endif
/* SVS $ */

void Help::OnChangeFocus(int focus)
{
  if (focus) {
    DisplayObject();
  }
}

void Help::ResizeConsole()
{
  Hide();
  if (FullScreenHelp)
  {
    HelpKeyBar.Hide();
    SetPosition(0,0,ScrX,ScrY);
  }
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  ReadHelp(HelpMask);
  FrameManager->RefreshFrame();
}
