#if defined(DHELP2)
#include "help2.cpp"
#else
/*
help.cpp

Помощь

*/

/* Revision: 1.62 25.12.2001 $ */

/*
Modify:
  25.12.2001 SVS
    - Работа над ошибками (Help::MkTopic для ModuleName)
  24.12.2001 SVS
    ! HelpMask переехала в StackHelpData.
    ! Уточнения для "документов" (в принципе уже можно открывать!)
    + Математика поиска в хелпе (зачатки!).
      Не обижусь, если кто-нить поможет ;-) Все равно до релиза
      (а может и дальше) поисковика не будет (но терять идеи надоело!!!)
  24.12.2001 SVS
    ! Уточнение.
  21.12.2001 SVS
    - Bug: не работала ссылка вида
      "@<c:\program files\far\plugins\multiarc\multiarc.dll>"
      Нужно было удавить "multiarc.dll". Плюс к этому теперь для
      __PluginContents__ путь заканчивается на слеш (для унификации)
  19.12.2001 VVM
    ! Если JumpTopic() не выполнился, то данные из стека удалим.
  03.12.2001 SVS
    ! Для "обрезания" :-) есть спец-функция.
  03.12.2001 DJ
    - если PluginContents очень длинный, надо его обрезать
  29.11.2001 DJ
    + в заголовке окна хелпа показываем, чей это хелп
  27.11.2001 DJ
    - забыли инициализировать CtrlTabSize
  27.11.2001 SVS
    ! ОпЯчатка в пред. патче
  26.11.2001 SVS
    + DoubliClock - свернуть/развернуть хелп.
    ! F1 в хелпе - всегда вызывать "Help"
  26.11.2001 VVM
    ! Теперь хелп не реагирует на отпускание клавиши мышки, если клавиша была нажата не в хелпе.
  01.11.2001 SVS
    + немного про "типы" - GetType*()
  26.10.2001 VVM
    + Считать нажатие средней кнопки за ЕНТЕР
  15.10.2001 SVS
    ! вместо strcmp применяем LCStricmp
  15.10.2001 SVS
    + Сортируем индекс хелпов от плагинов перед выдачей на экран.
  07.10.2001 SVS
    + Opt.HelpTabSize - размер табуляции по умолчанию.
  01.10.2001 SVS
    ! Временно отключим "KEY_SHIFTF3"
    + CtrlTabSize - опция! размер табуляции - резерв на будущее!
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 VVM
    ! Обрежем длинные строки при показе. Такое будет только при длинных ссылках...
  19.09.2001 VVM
    + Заменить ТАБУЛЯЦИЮ на пробел.
  21.08.2001 KM
    - Неверно создавался топик с учётом нового правила,
      в котором путь для топика должен заканчиваться "/".
  20.08.2001 VVM
    ! Обработка прокрутки с альтом.
  07.08.2001 SVS
    ! косметика - для собственных нужд (по поводу help2.?pp)
  05.08.2001 SVS
    + AddTitle() - добавить титл.
    ! В Help::MkTopic() исключим возможность повторного формирования топика,
      если он уже сформирован. Здесь еще нужно поковыряться, т.к. пока есть
      непонятки:
        К примеру, если принять как аксиому, что имя модуля всегда содержит
        ".ext" (точку), то все решается на ура.
  03.08.2001 SVS
    - бага с вызовом хелпа.
  01.08.2001 SVS
    + Новый взгляд на линки
    + Понятие - "Коллекция документов" (часть первая)
  26.07.2001 VVM
    + С альтом скролим всегда по 1
  22.07.2001 SVS
    ! Переделка number two - пытаемся при возврате показать привычное
      расположение хелпа (но пока, увы)
  20.07.2001 SVS
    - "То-ро-пыш-ка" :-( (или увеличение номера билда)
    ! PluginPanelHelp переехала к плагинам (не место ей здесь)
    ! Удалены за ненадобностью Get/Set-FullScreenMode
  20.07.2001 SVS
    - F1 Esc - проблемы
  20.07.2001 SVS
    ! "Перетрях мозглей" Help API. Part I.
  11.07.2001 SVS
    - Исправляем ситуацию с приганием курсора на следующую позицию при
      редраве окна хелпа
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  10.07.2001 OT
    - Возвращены переменные static SaveScreen *TopScreen и TopLevel :)
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
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"


// Стек возврата
class CallBackStack
{
  private:
    struct ListNode
    {
      ListNode *Next;

      DWORD Flags;             // флаги
      int   TopStr;            // номер верхней видимой строки темы
      int   CurX,CurY;         // координаты (???)

      char  *HelpTopic;        // текущий топик
      char  *HelpPath;         // путь к хелпам
      char  *SelTopic;         // текущее выделение
      char  *HelpMask;         // маска

      ListNode(const struct StackHelpData *Data, ListNode* n=NULL)
      {
        HelpTopic=strdup(Data->HelpTopic);
        HelpPath=strdup(Data->HelpPath);
        SelTopic=strdup(Data->SelTopic);
        HelpMask=strdup(Data->HelpMask);
        Flags=Data->Flags;
        TopStr=Data->TopStr;
        CurX=Data->CurX;
        CurY=Data->CurY;

        Next=n;
      }
      ~ListNode()
      {
        if(HelpTopic) free(HelpTopic);
        if(HelpPath)  free(HelpPath);
        if(SelTopic)  free(SelTopic);
        if(HelpMask)  free(HelpMask);
      }
    };

    ListNode *topOfStack;

  public:
    CallBackStack() {topOfStack=NULL;};
   ~CallBackStack() {ClearStack();};

  public:
    void ClearStack();
    BOOL isEmpty() const {return topOfStack==NULL;};

    void Push(const struct StackHelpData *Data);
    int Pop(struct StackHelpData *Data=NULL);

    void PrintStack(const char *Title);
};


#define MAX_HELP_STRING_LENGTH 300

static const char *PluginContents="__PluginContents__";
#if defined(WORK_HELP_DOCUMS)
static const char *DocumentContents="__DocumentContents__";
#endif
static const char *HelpOnHelpTopic=":Help";
static const char *HelpContents="Contents";

static int RunURL(char *Protocol, char *URLPath);

Help::Help(char *Topic, char *Mask,DWORD Flags)
{
  /* $ OT По умолчанию все хелпы создаются статически*/
  SetDynamicallyBorn(FALSE);
  CanLoseFocus=FALSE;
  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_HELP);

  ErrorHelp=TRUE;
  IsNewTopic=TRUE;

  MouseDown = FALSE;

  Stack=new CallBackStack;

  memset(&StackData,0,sizeof(StackData));
  StackData.Flags=Flags;

  /* $ 01.09.2000 SVS
     Установим по умолчанию текущий цвет отрисовки...
  */
  CurColor=COL_HELPTEXT;
  /* SVS $ */

  /* $ 27.11.2001 DJ
     не забудем инициализировать
  */
  CtrlTabSize = 8;
  /* DJ $ */

  strncpy(StackData.HelpMask,NullToEmpty(Mask),sizeof(StackData.HelpMask)-1); // сохраним маску файла

  KeyBarVisible = TRUE;  // Заставим обновлятся кейбар
  TopScreen=new SaveScreen;
  HelpData=NULL;
  strcpy(StackData.HelpTopic,Topic);
  *StackData.HelpPath=0;
  if (Opt.FullScreenHelp)
    SetPosition(0,0,ScrX,ScrY);
  else
    SetPosition(4,2,ScrX-4,ScrY-2);

  if(!ReadHelp(StackData.HelpMask) && (Flags&FHELP_USECONTENTS))
  {
    strcpy(StackData.HelpTopic,Topic);
    if(*StackData.HelpTopic == HelpBeginLink)
    {
      char *Ptr=strrchr(StackData.HelpTopic,HelpEndLink);
      if(Ptr)
        strcpy(++Ptr,HelpContents);
    }
    *StackData.HelpPath=0;
    ReadHelp(StackData.HelpMask);
  }

  if (HelpData!=NULL)
  {
    InitKeyBar();
    MacroMode = MACRO_HELP;
    MoveToReference(1,1);
    FrameManager->ExecuteModal (this);//OT
  }
  else
  {
    if(!(Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ErrorHelp=TRUE;
  }

#if defined(WORK_HELP_FIND)
  strncpy((char *)LastSearchStr,GlobalSearchString,sizeof(LastSearchStr));
  LastSearchPos=0;
  LastSearchCase=GlobalSearchCase;
  LastSearchWholeWords=GlobalSearchWholeWords;
  LastSearchReverse=GlobalSearchReverse;
#endif

}

Help::~Help()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
  SetRestoreScreenMode(FALSE);

  if(HelpData)     free(HelpData);
  if(Stack)        delete Stack;
  if(TopScreen)    delete TopScreen;

#if defined(WORK_HELP_FIND)
  KeepInitParameters();
#endif
}


#if defined(WORK_HELP_FIND)
void Help::KeepInitParameters()
{
  strcpy(GlobalSearchString,(char *)LastSearchStr);
  LastSearchPos=0;
  GlobalSearchCase=LastSearchCase;
  GlobalSearchWholeWords=LastSearchWholeWords;
  GlobalSearchReverse=LastSearchReverse;
}
#endif


void Help::Hide()
{
  ScreenObject::Hide();
}


int Help::ReadHelp(char *Mask)
{
  char FileName[NM],ReadStr[2*MAX_HELP_STRING_LENGTH];
  char SplitLine[2*MAX_HELP_STRING_LENGTH+8],*Ptr;
  int Formatting=TRUE,RepeatLastLine,PosTab;
  const int MaxLength=X2-X1-1;
  char TabSpace[32];

  DisableOut=0;

  char Path[NM],*TopicPtr;
  if (*StackData.HelpTopic==HelpBeginLink)
  {
    strcpy(Path,StackData.HelpTopic+1);
    if ((TopicPtr=strchr(Path,HelpEndLink))==NULL)
      return FALSE;
    strcpy(StackData.HelpTopic,TopicPtr+1);
    *TopicPtr=0;
    strcpy(StackData.HelpPath,Path);
  }
  else
    strcpy(Path,*StackData.HelpPath ? StackData.HelpPath:FarPath);

  if (!strcmp(StackData.HelpTopic,PluginContents))
  {
    ReadDocumentsHelp(HIDX_PLUGINS);
    return TRUE;
  }

#if defined(WORK_HELP_DOCUMS)
  if (!strcmp(StackData.HelpTopic,DocumentContents))
  {
    ReadDocumentsHelp(HIDX_DOCUMS);
    return TRUE;
  }
#endif

  FILE *HelpFile=Language::OpenLangFile(Path,(!*Mask?HelpFileMask:Mask),Opt.HelpLanguage,FileName);

  if (HelpFile==NULL)
  {
    if(!(StackData.Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MHelpTitle),MSG(MCannotOpenHelp),FileName,MSG(MOk));
    ErrorHelp=TRUE;
    return FALSE;
  }

  *ReadStr=0;
  if (Language::GetOptionsParam(HelpFile,"TabSize",ReadStr))
  {
    CtrlTabSize=atoi(ReadStr);
  }
  if(CtrlTabSize < 0 || CtrlTabSize > 16)
    CtrlTabSize=Opt.HelpTabSize;

  *ReadStr=0;
  if (Language::GetOptionsParam(HelpFile,"CtrlColorChar",ReadStr))
  {
    CtrlColorChar=ReadStr[0];
  }
  else
    CtrlColorChar=0;

  /* $ 29.11.2001 DJ
     запомним, чего там написано в PluginContents
  */
  if (!Language::GetLangParam (HelpFile,"PluginContents",CurPluginContents, NULL))
    *CurPluginContents = '\0';
  /* DJ $ */

  *SplitLine=0;
  if (HelpData)
    free(HelpData);
  HelpData=NULL;
  StrCount=0;
  FixCount=0;
  TopicFound=0;
  RepeatLastLine=FALSE;
  int NearTopicFound=0;
  char PrevSymbol=0;

  memset(TabSpace,' ',sizeof(TabSpace)-1);
  TabSpace[sizeof(TabSpace)-1]=0;

  while (TRUE)
  {
    if (!RepeatLastLine && fgets(ReadStr,sizeof(ReadStr)/2,HelpFile)==NULL)
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

    while((Ptr=strchr(ReadStr,'\t')) != NULL)
    {
      *Ptr=' ';
      PosTab=Ptr-ReadStr+1;
      if(CtrlTabSize > 1) // заменим табулятор по всем праивилам
        InsertString(ReadStr,PosTab,TabSpace,
           ((PosTab%CtrlTabSize)==0?CtrlTabSize:(PosTab%CtrlTabSize))-1);
      if(strlen(ReadStr) > sizeof(ReadStr)/2)
        break;
    }

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
        if (LocalStricmp(ReadStr+1,StackData.HelpTopic)==0)
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
                memmove(SplitLine+1,SplitLine+I+1,strlen(SplitLine+I+1)+1);
                *SplitLine=' ';
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
  if(IsNewTopic)
  {
    StackData.CurX=StackData.CurY=0;
    StackData.TopStr=0;
  }
  return TopicFound != 0;
}


void Help::AddLine(char *Line)
{
  char *NewHelpData=(char *)realloc(HelpData,(StrCount+1)*MAX_HELP_STRING_LENGTH);
  if (NewHelpData==NULL)
    return;
  HelpData=NewHelpData;
  strncpy(HelpData+StrCount*MAX_HELP_STRING_LENGTH,Line,MAX_HELP_STRING_LENGTH-1);
  StrCount++;
}

void Help::AddTitle(char *Title)
{
  char IndexHelpTitle[100];
  sprintf(IndexHelpTitle,"^ #%s#",Title);
  AddLine(IndexHelpTitle);
}

void Help::HighlightsCorrection(char *Str)
{
  int I,Count;
  for (I=0,Count=0;Str[I]!=0;I++)
    if (Str[I]=='#')
      Count++;
  if ((Count & 1) && *Str!='$')
  {
    memmove(Str+1,Str,strlen(Str)+1);
    *Str='#';
  }
}


void Help::DisplayObject()
{
  if(!TopScreen)
    TopScreen=new SaveScreen;
  if (!TopicFound)
  {
    if(!(StackData.Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ProcessKey(KEY_ALTF1);
    ErrorHelp=TRUE;
    return;
  }
  SetCursorType(0,10);
  if (*StackData.SelTopic==0)
    MoveToReference(1,1);
  FastShow();
  if (!Opt.FullScreenHelp)
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

  /* $ 29.11.2001 DJ
     отрисовка рамки -> в отдельную функцию
  */
  if (!DisableOut)
    DrawWindowFrame();
  /* DJ $ */

  CorrectPosition();
  *StackData.SelTopic=0;
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
        StrPos=I+StackData.TopStr;
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
    ScrollBar(X2,Y1+FixSize+1,ScrollLength,StackData.TopStr,Scrolled);
  }
}

/* $ 29.11.2001 DJ
   вытащена из FastShow; добавлен показ того, чей у нас хелп
*/

void Help::DrawWindowFrame()
{
  SetScreen(X1,Y1,X2,Y2,' ',COL_HELPTEXT);
  Box(X1,Y1,X2,Y2,COL_HELPBOX,DOUBLE_BOX);
  SetColor(COL_HELPBOXTITLE);

  char HelpTitleBuf [256];
  strcpy (HelpTitleBuf, MSG(MHelpTitle));
  strcat (HelpTitleBuf, " - ");
  if (*CurPluginContents)
    strcat (HelpTitleBuf, CurPluginContents);
  else
    strcat (HelpTitleBuf, "FAR");
  /* $ 03.12.2001 DJ
     обрежем длинный заголовок
  */
  TruncStrFromEnd(HelpTitleBuf,X2-X1-3);
  /* DJ $ */
  GotoXY(X1+(X2-X1+1-strlen(HelpTitleBuf)-2)/2,Y1);
  mprintf(" %s ",HelpTitleBuf);
}

/* DJ $ */

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

    if (*Str=='~' || *Str=='#' || *Str==HelpBeginLink || *Str==0 || *Str == CtrlColorChar)
    {
      OutStr[OutPos]=0;
      if (Topic)
      {
        int RealCurX,RealCurY;
        RealCurX=X1+StackData.CurX+1;
        RealCurY=Y1+StackData.CurY+FixSize+1;
        if (WhereY()==RealCurY && RealCurX>=WhereX() &&
                RealCurX<WhereX()+(Str-StartTopic)-1)
        {
          SetColor(COL_HELPSELECTEDTOPIC);
          if (Str[1]=='@')
          {
            strncpy(StackData.SelTopic,Str+2,sizeof(StackData.SelTopic)-1);
            char *EndPtr=strchr(StackData.SelTopic,'@');
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
      /* $ 24.09.2001 VVM
        ! Обрежем длинные строки при показе. Такое будет только при длинных ссылках... */
      if ((strlen(OutStr) + WhereX()) > X2)
        OutStr[X2 - WhereX()] = 0;
      /* VVM $ */
      if (DisableOut)
        GotoXY(WhereX()+strlen(OutStr),WhereY());
      else
        Text(OutStr);
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
  if (StackData.CurX>X2-X1-2)
    StackData.CurX=X2-X1-2;
  if (StackData.CurX<0)
    StackData.CurX=0;
  if (StackData.CurY>Y2-Y1-2-FixSize)
  {
    StackData.TopStr+=StackData.CurY-(Y2-Y1-2-FixSize);
    StackData.CurY=Y2-Y1-2-FixSize;
  }
  if (StackData.CurY<0)
  {
    StackData.TopStr+=StackData.CurY;
    StackData.CurY=0;
  }
  if (StackData.TopStr>StrCount-FixCount-(Y2-Y1-1-FixSize))
    StackData.TopStr=StrCount-FixCount-(Y2-Y1-1-FixSize);
  if (StackData.TopStr<0)
    StackData.TopStr=0;
}

#if defined(WORK_HELP_FIND)
/* SVS:
   ВНИМАНИЕ!!!!!
   Эта функция совсем сырая (мягко сказано!)
   Если у кого хватит ума немного ее поправить - милости просим ;-)
*/
int Help::Search(int Next)
{
  unsigned char SearchStr[SEARCHSTRINGBUFSIZE]
  char MsgStr[512];
  int SearchLength,Case,WholeWords,ReverseSearch,Match;

  if (Next && *LastSearchStr==0)
    return TRUE;

  Match=FALSE;
  strncpy((char *)SearchStr,(char *)LastSearchStr,sizeof(SearchStr));
  Case=LastSearchCase;
  WholeWords=LastSearchWholeWords;
  ReverseSearch=LastSearchReverse;

  if (!Next)
    if(!GetSearchReplaceString(FALSE,SearchStr,sizeof(SearchStr),
                   NULL,0,NULL,NULL,
                   NULL/*&Case*/,NULL/*&WholeWords*/,NULL/*&ReverseSearch*/))
      return FALSE;

  strncpy((char *)LastSearchStr,(char *)SearchStr,sizeof(LastSearchStr));
  LastSearchCase=Case;
  LastSearchWholeWords=WholeWords;
  LastSearchReverse=ReverseSearch;

  if ((SearchLength=strlen((char *)SearchStr))==0)
    return TRUE;
  else
  {
    SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    char Buf[8192];

    sprintf(MsgStr,"\"%s\"",SearchStr);
    Message(0,0,MSG(MHelpSearchTitle),MSG(MHelpSearchingFor),MsgStr);

    if (!Case)
      for (int I=0;I<SearchLength;I++)
        SearchStr[I]=LocalUpper(SearchStr[I]);
/*
    //if(!ReadHelp(HelpMask))
    //fseek(ViewFile,LastSearchPos,SEEK_SET);

    while (!Match)
    {

    }
*/
  }
  if (!Match)
  {
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MHelpFindTitle),
              MSG(MHelpSearchCannotFind),MsgStr,MSG(MOk));
    return FALSE;
  }

  // перехд к найденному топику.
  return TRUE;
}
#endif

int Help::ProcessKey(int Key)
{
  if (*StackData.SelTopic==0)
    StackData.CurX=StackData.CurY=0;
  switch(Key)
  {
    case KEY_NONE:
    case KEY_IDLE:
      break;
    case KEY_F5:
      Opt.FullScreenHelp=!Opt.FullScreenHelp;
      ResizeConsole();
      return(TRUE);
    case KEY_ESC:
    case KEY_F10:
      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      return(TRUE);
    case KEY_HOME:
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr=0;
      FastShow();
      if (*StackData.SelTopic==0)
        MoveToReference(1,1);
      return(TRUE);
    case KEY_END:
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr=StrCount;
      FastShow();
      if (*StackData.SelTopic==0)
      {
        StackData.CurX=0;
        StackData.CurY=Y2-Y1-2-FixSize;
        MoveToReference(0,1);
      }
      return(TRUE);
    case KEY_UP:
      if (StackData.TopStr>0)
      {
        StackData.TopStr--;
        if (StackData.CurY<Y2-Y1-2-FixSize)
        {
          StackData.CurX=X2-X1-2;
          StackData.CurY++;
        }
        FastShow();
        if (*StackData.SelTopic==0)
          MoveToReference(0,1);
      }
      else
        ProcessKey(KEY_SHIFTTAB);
      return(TRUE);
    case KEY_DOWN:
      if (StackData.TopStr<StrCount-FixCount-(Y2-Y1-1-FixSize))
      {
        StackData.TopStr++;
        if (StackData.CurY>0)
          StackData.CurY--;
        StackData.CurX=0;
        FastShow();
        if (*StackData.SelTopic==0)
          MoveToReference(1,1);
      }
      else
        ProcessKey(KEY_TAB);
      return(TRUE);
    /* $ 26.07.2001 VVM
      + С альтом скролим по 1 */
    /* $ 07.05.2001 DJ
      + Обработка KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_UP:
    case (KEY_MSWHEEL_UP | KEY_ALT):
      {
        int Roll = Key & KEY_ALT?1:Opt.MsWheelDelta;
        for (int i=0; i<Roll; i++)
          ProcessKey(KEY_UP);
        return(TRUE);
      }
    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
      {
        int Roll = Key & KEY_ALT?1:Opt.MsWheelDelta;
        for (int i=0; i<Roll; i++)
          ProcessKey(KEY_DOWN);
        return(TRUE);
      }
    /* DJ $ */
    /* VVM $ */
    case KEY_PGUP:
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr-=Y2-Y1-2-FixSize;
      FastShow();
      if (*StackData.SelTopic==0)
      {
        StackData.CurX=StackData.CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    case KEY_PGDN:
      {
        int PrevTopStr=StackData.TopStr;
        StackData.TopStr+=Y2-Y1-2-FixSize;
        FastShow();
        if (StackData.TopStr==PrevTopStr)
        {
          ProcessKey(KEY_CTRLPGDN);
          return(TRUE);
        }
        else
          StackData.CurX=StackData.CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    case KEY_RIGHT:
    case KEY_TAB:
      MoveToReference(1,0);
      return(TRUE);
    case KEY_LEFT:
    case KEY_SHIFTTAB:
      MoveToReference(0,0);
      return(TRUE);

    case KEY_F1:
      // не поганим SelTopic, если и так в Help on Help
      if(LocalStricmp(StackData.HelpTopic,HelpOnHelpTopic)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(HelpOnHelpTopic);
        IsNewTopic=FALSE;
      }
      return(TRUE);
    case KEY_SHIFTF1:
      //   не поганим SelTopic, если и так в теме Contents
      if(LocalStricmp(StackData.HelpTopic,HelpContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(HelpContents);
        IsNewTopic=FALSE;
      }
      return(TRUE);
    case KEY_SHIFTF2:
      //   не поганим SelTopic, если и так в PluginContents
      if(LocalStricmp(StackData.HelpTopic,PluginContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(PluginContents);
        IsNewTopic=FALSE;
      }
      return(TRUE);

#if defined(WORK_HELP_DOCUMS)
    case KEY_SHIFTF3: // Для "документов" :-)
      //   не поганим SelTopic, если и так в DocumentContents
      if(LocalStricmp(StackData.HelpTopic,DocumentContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(DocumentContents);
        IsNewTopic=FALSE;
      }
      return(TRUE);
#endif

    case KEY_ALTF1:
    case KEY_BS:
      // Если стек возврата пуст - выходим их хелпа
      if(!Stack->isEmpty())
      {
        Stack->Pop(&StackData);
        JumpTopic(StackData.HelpTopic);
        return(TRUE);
      }
      return ProcessKey(KEY_ESC);

    case KEY_ENTER:
      if (*StackData.SelTopic && LocalStricmp(StackData.HelpTopic,StackData.SelTopic)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        if (!JumpTopic())
          Stack->Pop(&StackData);
        IsNewTopic=FALSE;
      }
      return(TRUE);

#if defined(WORK_HELP_FIND)
    case KEY_F7:
      Search(0);
      return(TRUE);
    case KEY_SHIFTF7:
      Search(1);
      return(TRUE);
#endif

  }
  return(FALSE);
}

int Help::JumpTopic(const char *JumpTopic)
{
  char  OldTopic[512];
  char NewTopic[512];
  char *p;

  Stack->PrintStack(JumpTopic);

  if(JumpTopic)
    strcpy(StackData.SelTopic,JumpTopic);
//_SVS(SysLog("JumpTopic() = SelTopic=%s",StackData.SelTopic));
  // URL активатор - это ведь так просто :-)))
  {
    strcpy(NewTopic,StackData.SelTopic);
    p=strchr(NewTopic,':');
    if(p && NewTopic[0] != ':') // наверное подразумевается URL
    {
      *p=0;
      if(RunURL(NewTopic,StackData.SelTopic))
        return(FALSE);
      *p=':';
    }
  }
  // а вот теперь попробуем...

//_SVS(SysLog("JumpTopic() = SelTopic=%s, StackData.HelpPath=%s",StackData.SelTopic,StackData.HelpPath));
  if (*StackData.HelpPath && *StackData.SelTopic!=HelpBeginLink && strcmp(StackData.SelTopic,HelpOnHelpTopic)!=0)
  {
    if (*StackData.SelTopic==':')
      strcpy(NewTopic,StackData.SelTopic+1);
    else if(StackData.Flags&FHELP_CUSTOMFILE)
      strcpy(NewTopic,StackData.SelTopic);
    else
      sprintf(NewTopic,HelpFormatLink,StackData.HelpPath,StackData.SelTopic);
  }
  else
  {
    strcpy(NewTopic,StackData.SelTopic+(!strcmp(StackData.SelTopic,HelpOnHelpTopic)?1:0));
  }

  // удалим ссылку на .DLL
  p=strrchr(NewTopic,'>');
  if(p && *(p-1) != '\\')
  {
    char *p2=p;
    while(p >= NewTopic)
    {
      if(*p == '\\')
      {
        ++p;
        if(*p)
        {
          StackData.Flags|=FHELP_CUSTOMFILE;
          strcpy(StackData.HelpMask,p);
          *strrchr(StackData.HelpMask,'>')=0;
        }
        memmove(p,p2,strlen(p2)+1);
        break;
      }
      --p;
    }
  }

//_SVS(SysLog("HelpMask=%s NewTopic=%s",StackData.HelpMask,NewTopic));
  if(*StackData.SelTopic != ':' &&
     LocalStricmp(StackData.SelTopic,PluginContents)
#if defined(WORK_HELP_DOCUMS)
     && LocalStricmp(StackData.SelTopic,DocumentContents)
#endif
    )
  {
    if(!(StackData.Flags&FHELP_CUSTOMFILE) && strrchr(NewTopic,'>'))
    {
      if(StackData.HelpMask)
        *StackData.HelpMask=0;
    }
  }
  else
  {
    if(StackData.HelpMask)
      *StackData.HelpMask=0;
  }
  strcpy(StackData.HelpTopic,NewTopic);
  if(!(StackData.Flags&FHELP_CUSTOMFILE))
    *StackData.HelpPath=0;
  if(!ReadHelp(StackData.HelpMask))
  {
    strcpy(StackData.HelpTopic,NewTopic);
    if(*StackData.HelpTopic == HelpBeginLink)
    {
      char *Ptr=strrchr(StackData.HelpTopic,HelpEndLink);
      if(Ptr)
        strcpy(++Ptr,HelpContents);
    }
    *StackData.HelpPath=0;
    ReadHelp(StackData.HelpMask);
  }
  if (!HelpData)
  {
    if(!(StackData.Flags&FHELP_NOSHOWERROR))
      Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),MSG(MOk));
    ErrorHelp=TRUE;
    return FALSE;
  }
//  ResizeConsole();
  if(IsNewTopic
    || !LocalStricmp(StackData.SelTopic,PluginContents) // Это неприятный костыль :-((
#if defined(WORK_HELP_DOCUMS)
    || !LocalStricmp(StackData.SelTopic,DocumentContents)
#endif
    )
    MoveToReference(1,1);
  //FrameManager->ImmediateHide();
  FrameManager->RefreshFrame();

  return TRUE;
}



int Help::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (MouseEvent->dwEventFlags==MOUSE_MOVED &&
     (MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED)==0)
    return(FALSE);
  /* $ 26.10.2001 VVM
    + Считать нажатие средней кнопки за ЕНТЕР */
  if (MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
  {
    ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  /* VVM $ */
  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2)
  {
    // Вываливаем если предыдущий эвент не был двойным кликом
    if(PreMouseEventFlags != DOUBLE_CLICK)
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
  // DoubliClock - свернуть/развернуть хелп.
  if (MouseEvent->dwEventFlags==DOUBLE_CLICK &&
      (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
      MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
  {
    ProcessKey(KEY_F5);
    return(TRUE);
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
  StackData.CurX=MouseEvent->dwMousePosition.X-X1-1;
  StackData.CurY=MouseEvent->dwMousePosition.Y-Y1-1-FixSize;
  FastShow();
  /* $ 26.11.2001 VVM
    + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
  if (MouseEvent->dwEventFlags==0 &&
     (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
    MouseDown = TRUE;
  if (MouseEvent->dwEventFlags==0 &&
     (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))==0 &&
      MouseDown &&
      *StackData.SelTopic)
  {
    MouseDown = FALSE;
    ProcessKey(KEY_ENTER);
  }
  /* VVM $ */
//  if ((MouseEvent->dwButtonState & 3)==0 && *StackData.SelTopic)
//    ProcessKey(KEY_ENTER);
  return(TRUE);
}


int Help::IsReferencePresent()
{
  CorrectPosition();
  int StrPos=FixCount+StackData.TopStr+StackData.CurY;
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
  int StartSelection=*StackData.SelTopic;
  int SaveCurX=StackData.CurX;
  int SaveCurY=StackData.CurY;
  int SaveTopStr=StackData.TopStr;

  *StackData.SelTopic=0;
  DisableOut=TRUE;

  while (*StackData.SelTopic==0)
  {
    if (Forward)
    {
      if (StackData.CurX==0 && !IsReferencePresent())
        StackData.CurX=X2-X1-2;
      if (++StackData.CurX >= X2-X1-2)
      {
        StartSelection=0;
        StackData.CurX=0;
        StackData.CurY++;
        if (StackData.TopStr+StackData.CurY>=StrCount-FixCount ||
            CurScreen && StackData.CurY>Y2-Y1-2-FixSize)
          break;
      }
    }
    else
    {
      if (StackData.CurX==X2-X1-2 && !IsReferencePresent())
        StackData.CurX=0;
      if (--StackData.CurX < 0)
      {
        StartSelection=0;
        StackData.CurX=X2-X1-2;
        StackData.CurY--;
        if (StackData.TopStr+StackData.CurY<0 ||
            CurScreen && StackData.CurY<0)
          break;
      }
    }

    FastShow();
    if (*StackData.SelTopic==0)
      StartSelection=0;
    else
      if (StartSelection)
        *StackData.SelTopic=0;
  }
  DisableOut=FALSE;
  if (*StackData.SelTopic==0)
  {
    StackData.CurX=SaveCurX;
    StackData.CurY=SaveCurY;
    StackData.TopStr=SaveTopStr;
  }
  FastShow();
}

void Help::ReadDocumentsHelp(int TypeIndex)
{
  if(HelpData)
    free(HelpData);
  HelpData=NULL;
  /* $ 29.11.2001 DJ
     это не плагин -> чистим CurPluginContents
  */
  *CurPluginContents = '\0';
  /* DJ $ */

  StrCount=0;
  FixCount=1;
  FixSize=2;
  StackData.TopStr=0;
  TopicFound=TRUE;
  StackData.CurX=StackData.CurY=0;
  CtrlColorChar=0;

  char *PtrTitle, *ContentsName;
  char Path[NM],FullFileName[NM],*PtrPath,*Slash;
  char EntryName[512],HelpLine[512],SecondParam[512];

  switch(TypeIndex)
  {
    case HIDX_PLUGINS:
      PtrTitle=MSG(MPluginsHelpTitle);
      ContentsName="PluginContents";
      break;
#if defined(WORK_HELP_DOCUMS)
    case HIDX_DOCUMS:
      PtrTitle=MSG(MDocumentsHelpTitle);
      ContentsName="DocumentContents";
      break;
#endif
  }

  AddTitle(PtrTitle);

  /* TODO:
     1. Поиск (для "документов") не только в каталоге Documets, но
        и в плагинах
  */
  int OldStrCount=StrCount;
  switch(TypeIndex)
  {
    case HIDX_PLUGINS:
    {
      for (int I=0;I<CtrlObject->Plugins.PluginsCount;I++)
      {
        strcpy(Path,CtrlObject->Plugins.PluginsData[I].ModuleName);
        if ((Slash=strrchr(Path,'\\'))!=NULL)
          *++Slash=0;
        FILE *HelpFile=Language::OpenLangFile(Path,HelpFileMask,Opt.HelpLanguage,FullFileName);
        if (HelpFile!=NULL)
        {
          char EntryName[512],HelpLine[512],SecondParam[512];
          if (Language::GetLangParam(HelpFile,ContentsName,EntryName,SecondParam))
          {
            if (*SecondParam)
              sprintf(HelpLine,"   ~%s,%s~@" HelpFormatLink "@",EntryName,SecondParam,Path,HelpContents);
            else
              sprintf(HelpLine,"   ~%s~@" HelpFormatLink "@",EntryName,Path,HelpContents);
            AddLine(HelpLine);
          }

          fclose(HelpFile);
        }
      }
      break;
    }

#if defined(WORK_HELP_DOCUMS)
    case HIDX_DOCUMS:
    {
      // в плагинах.
      for (int I=0;I<CtrlObject->Plugins.PluginsCount;I++)
      {
        strcpy(Path,CtrlObject->Plugins.PluginsData[I].ModuleName);
        if ((Slash=strrchr(Path,'\\'))!=NULL)
          *++Slash=0;
        FILE *HelpFile=Language::OpenLangFile(Path,HelpFileMask,Opt.HelpLanguage,FullFileName);
        if (HelpFile!=NULL)
        {
          if (Language::GetLangParam(HelpFile,ContentsName,EntryName,SecondParam))
          {
            if (*SecondParam)
              sprintf(HelpLine,"   ~%s,%s~@<%s>%s@",EntryName,SecondParam,FullFileName,HelpContents);
            else
              sprintf(HelpLine,"   ~%s~@<%s>%s@",EntryName,FullFileName,HelpContents);
            AddLine(HelpLine);
          }

          fclose(HelpFile);
        }
      }

      // в документах.
      {
        WIN32_FIND_DATA FindData;
        char FMask[NM];
        AddEndSlash(strcpy(Path,FarPath));
        strcat(Path,"Doc");
        ScanTree ScTree(FALSE,FALSE);
        ScTree.SetFindPath(Path,HelpFileMask);
        while (ScTree.GetNextName(&FindData,FullFileName))
        {
          if((PtrPath=strrchr(FullFileName,'\\')) != NULL)
          {
            strncpy(FMask,PtrPath+1,sizeof(FMask)-1);
            *++PtrPath=0;
          }
          else
            strncpy(FMask,HelpFileMask,sizeof(FMask)-1);

          FILE *HelpFile=Language::OpenLangFile(Path,FMask,Opt.HelpLanguage,FullFileName,TRUE);
          if (HelpFile!=NULL)
          {
            if (Language::GetLangParam(HelpFile,ContentsName,EntryName,SecondParam))
            {
              if (*SecondParam)
                sprintf(HelpLine,"   ~%s,%s~@<%s>%s@",EntryName,SecondParam,FullFileName,HelpContents);
              else
                sprintf(HelpLine,"   ~%s~@<%s>%s@",EntryName,FullFileName,HelpContents);

              AddLine(HelpLine);
            }
            fclose(HelpFile);
          }
        }
      }
      StackData.Flags|=FHELP_CUSTOMFILE;
      break;
    }
#endif
  }
  // сортируем по алфавиту
  qsort(HelpData+OldStrCount*MAX_HELP_STRING_LENGTH,StrCount-OldStrCount,MAX_HELP_STRING_LENGTH,(int (*)(const void *,const void *))LCStricmp);
  /* $ 26.06.2000 IS
   Устранение глюка с хелпом по f1, shift+f2, end (решение предложил IG)
  */
  AddLine("");
  /* IS $ */
}

// Формирование топика с учетом разных факторов
char *Help::MkTopic(int PluginNumber,const char *HelpTopic,char *Topic)
{
  *Topic=0;
  if (HelpTopic && *HelpTopic)
  {
    if (*HelpTopic==':')
      strcpy(Topic,HelpTopic+1);
    else
    {
      if(PluginNumber != -1 && *HelpTopic!=HelpBeginLink)
      {
         sprintf(Topic,HelpFormatLinkModule,
                CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName,
                HelpTopic);
      }
      else
        strncpy(Topic,HelpTopic,511);

      if(*Topic==HelpBeginLink)
      {
        char *Ptr, *Ptr2;
        if((Ptr=strchr(Topic,HelpEndLink)) == NULL)
          *Topic=0;
        else
        {
          if(!Ptr[1]) // Вона как поперло то...
            strcat(Topic,HelpContents); // ... значит покажем основную тему.

          /* А вот теперь разгребем...
             Формат может быть :
               "<FullPath>Topic" или "<FullModuleName>Topic"
             Для случая "FullPath" путь ДОЛЖЕН заканчиваться СЛЕШЕМ!
             Т.о. мы отличим ЧТО ЭТО - имя модуля или путь!
          */
          Ptr2=Ptr-1;
          if(*Ptr2 != '\\') // Это имя модуля?
          {
            // значит удалим это чертово имя :-)
            if((Ptr2=strrchr(Topic,'\\')) == NULL) // ВО! Фигня какая-то :-(
              *Topic=0;
          }
          if(*Topic)
          {
            /* $ 21.08.2001 KM
              - Неверно создавался топик с учётом нового правила,
                в котором путь для топика должен заканчиваться "/".
            */
            memmove(Ptr2+1,Ptr,strlen(Ptr)+1); //???
            /* KM $ */

            // А вот ЗДЕСЬ теперь все по правилам Help API!
          }
        }
      }
    }
  }
  return *Topic?Topic:NULL;
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize help*/
void Help::SetScreenPosition()
{
  if (Opt.FullScreenHelp)
  {
    HelpKeyBar.Hide();
    SetPosition(0,0,ScrX,ScrY);
  }
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

  // Уберем лишнее с глаз долой
#if !defined(WORK_HELP_DOCUMS)
  FHelpShiftKeys[3-1][0]=0;
#endif
#if !defined(WORK_HELP_FIND)
  FHelpKeys[7-1][0]=0;
  FHelpShiftKeys[7-1][0]=0;
#endif

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

void Help::OnChangeFocus(int Focus)
{
  if (Focus)
  {
    DisplayObject();
  }
}

void Help::ResizeConsole()
{
  int OldIsNewTopic=IsNewTopic;
  IsNewTopic=FALSE;
  delete TopScreen;
  TopScreen=NULL;
  Hide();
  if (Opt.FullScreenHelp)
  {
    HelpKeyBar.Hide();
    SetPosition(0,0,ScrX,ScrY);
  }
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  ReadHelp(StackData.HelpMask);
  StackData.CurY--; // ЭТО ЕСМЬ КОСТЫЛЬ (пусть пока будет так!)
  MoveToReference(1,1);
  IsNewTopic=OldIsNewTopic;
  FrameManager->ImmediateHide();
  FrameManager->RefreshFrame();
}

int Help::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_HELP;
}


int Help::GetTypeAndName(char *Type,char *Name)
{
  if(Type)
    strcpy(Type,MSG(MHelpType));
  if(Name)
    strcpy(Name,"");
  return(MODALTYPE_HELP);
}

/* ------------------------------------------------------------------ */
void CallBackStack::ClearStack()
{
  while(!isEmpty())
    Pop();
}

int CallBackStack::Pop(struct StackHelpData *Dest)
{
  if(!isEmpty())
  {
    ListNode *oldTop = topOfStack;
    topOfStack = topOfStack->Next;
    if(Dest!=NULL)
    {
      strcpy(Dest->HelpTopic,oldTop->HelpTopic);
      strcpy(Dest->HelpPath,oldTop->HelpPath);
      strcpy(Dest->SelTopic,oldTop->SelTopic);
      strcpy(Dest->HelpMask,oldTop->HelpMask);

      Dest->Flags=oldTop->Flags;
      Dest->TopStr=oldTop->TopStr;
      Dest->CurX=oldTop->CurX;
      Dest->CurY=oldTop->CurY;
    }
    delete oldTop;
    return TRUE;
  }
  return FALSE;
}

void CallBackStack::Push(const struct StackHelpData *Data)
{
  topOfStack=new ListNode(Data,topOfStack);
}

void CallBackStack::PrintStack(const char *Title)
{
#if defined(SYSLOG)
  int I=0;
  ListNode *Ptr = topOfStack;
  SysLog("Return Stack (%s)",Title);
  SysLog(1);
  while(Ptr)
  {
    SysLog("%03d HelpTopic='%s' HelpPath='%s' HelpMask='%s'",I++,Ptr->HelpTopic,Ptr->HelpPath,Ptr->HelpMask);
    Ptr=Ptr->Next;
  }
  SysLog(-1);
#endif
}
#endif // defined(DHELP2)
