#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.30 17.07.2005 $ */

/*
Modify:
  17.07.2005 SVS
    ! долой DHELP2
  18.12.2004 WARP
    ! Спецификатор переноса строки в .hlf файлах (BugZ#1084)
  13.10.2003 SVS
    ! Заготовка для мультисимвольного CtrlColorChar
  03.06.2003 SVS
    + HELPMODE_CLICKOUTSIDE  - было нажатие мыши вне хелпа?
  22.04.2003 SVS
    + FHELPOBJ_ERRCANNOTOPENHELP - индикатор ошибки
  14.07.2002 IS
    ! внедрение const
  24.12.2001 SVS
    ! HelpMask переехала в StackHelpData.
    + Математика поиска в хелпе (зачатки, серия первая)
  29.11.2001 DJ
    ! отрисовка рамки хелпа вытащена в отдельную функцию
    + помним PluginContents для текущего хелпа
  26.11.2001 VVM
    ! Теперь хелп не реагирует на отпускание клавиши мышки, если клавиша была нажата не в хелпе.
  01.11.2001 SVS
    + немного про "типы" - GetType*()
  01.10.2001 SVS
    + CtrlTabSize - опция! размер табуляции - резерв на будущее!
  07.08.2001 SVS
    ! косметика - для собственных нужд (по поводу help2.?pp)
  05.08.2001 SVS
    + AddTitle() - добавить титл.
  01.08.2001 SVS
    + MkTopic() - создание топика
    ! ReadPluginsHelp() переименована в ReadDocumentsHelp, т.к.
      предполагается отображать индекс справки не только по плагинам
  22.07.2001 SVS
    ! Переделка number two - пытаемся при возврате показать привычное
      расположение хелпа (но пока, увы)
  20.07.2001 SVS
    ! PluginPanelHelp переехала к плагинам (не место ей здесь)
    ! Удалены за ненадобностью Get/Set-FullScreenMode
  20.07.2001 SVS
    ! "Перетрях мозглей" Help API. Part I.
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  31.05.2001 OT
    + ResizeConsole() - как реакция на изменившийся размер консоли.
  15.05.2001 OT
    ! NWZ -> NFZ
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  12.04.2001 SVS
    + сохранение значения Mask, переданного в конструктор
    + передача сохраненной Mask в конструктор с ShowPrev
  26.03.2001 SVS
    ! ReadHelp возвращает TRUE/FALSE
  30.12.2000 SVS
    + KeyBar в Help`е
  18.12.2000 SVS
    + Дополнительный параметр у конструктора - DWORD Flags.
    + Член класса - Flags
  12.09.2000 SVS
    + Параметры у функции ReadHelp и конструктора, задающие маску поиска
      файлов.
    + GetError() - возвращает ErrorHelp.
  01.09.2000 SVS
    + CtrlColorChar - опция! для спецсимвола-символа - для атрибутов
    + CurColor - текущий цвет отрисовки
  28.06.2000 tran
    - NT Console resize bug
      adding SetScreenPosition method
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "frame.hpp"
#include "keybar.hpp"
#include "farconst.hpp"

class CallBackStack;

#define HELPMODE_CLICKOUTSIDE  0x20000000 // было нажатие мыши вне хелпа?

struct StackHelpData
{
  DWORD Flags;                  // флаги
  int   TopStr;                 // номер верхней видимой строки темы
  int   CurX,CurY;              // координаты (???)

  char  HelpMask[NM];           // значение маски
  char  HelpPath[NM];           // путь к хелпам
  char  HelpTopic[512];         // текущий топик
  char  SelTopic[512];          // выделенный топик (???)
};

enum HELPDOCUMENTSHELPTYPE{
  HIDX_PLUGINS,                 // Индекс плагинов
  HIDX_DOCUMS,                  // Индекс документов
};

enum {
  FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class Help:public Frame
{
  private:
    BOOL  ErrorHelp;            // TRUE - ошибка! Например - нет такого топика
    SaveScreen *TopScreen;      // область сохранения под хелпом
    KeyBar      HelpKeyBar;     // кейбар
    CallBackStack *Stack;       // стек возврата

    struct StackHelpData StackData;
    char *HelpData;             // "хелп" в памяти.
    int   StrCount;             // количество строк в теме
    int   FixCount;             // количество строк непрокручиваемой области
    int   FixSize;              // Размер непрокручиваемой области
    int   TopicFound;           // TRUE - топик найден
    int   IsNewTopic;           // это новый топик?
    int   MouseDown;

    int   DisableOut;           // TRUE - не выводить на экран
    char  CtrlColorChar[16];    // CtrlColorChar - опция! для спецсимвола-
                                //   символа - для атрибутов
    int   CurColor;             // CurColor - текущий цвет отрисовки
    int   CtrlTabSize;          // CtrlTabSize - опция! размер табуляции

    int   PrevMacroMode;        // предыдущий режим макроса

    /* $ 29.11.2001 DJ
       помним PluginContents (для отображения в заголовке)
    */
    char CurPluginContents[NM];
    /* DJ $ */

    DWORD LastStartPos;
    DWORD StartPos;

    char  CtrlStartPosChar[16];

#if defined(WORK_HELP_FIND)
  private:
    DWORD LastSearchPos;
    unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse;

  private:
    int Search(int Next);
    void KeepInitParameters();
#endif

  private:
    void DisplayObject();
    int  ReadHelp(const char *Mask=NULL);
    void AddLine(const char *Line);
    void AddTitle(const char *Title);
    void HighlightsCorrection(char *Str);
    void FastShow();
    /* $ 29.11.2001 DJ
       вытащена из FastShow
    */
    void DrawWindowFrame();
    /* DJ $ */
    void OutString(const char *Str);
    int  StringLen(const char *Str);
    void CorrectPosition();
    int  IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadDocumentsHelp(int TypeIndex);
    int  JumpTopic(const char *JumpTopic=NULL);

  public:
    Help(const char *Topic,const char *Mask=NULL,DWORD Flags=0);
    ~Help();

  public:
    void Hide();
    int  ProcessKey(int Key);
    int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void InitKeyBar(void);
    BOOL GetError() {return ErrorHelp;}
    /* $ 28.06.2000 tran NT Console resize - resize help */
    virtual void SetScreenPosition();
    void OnChangeFocus(int focus); // вызывается при смене фокуса
    void ResizeConsole();
    /* $ Введена для нужд CtrlAltShift OT */
    int  FastHide();

    virtual const char *GetTypeName() {return "[Help]";}
    virtual int GetTypeAndName(char *Type,char *Name);
    virtual int GetType() { return MODALTYPE_HELP; }

    static char *MkTopic(int PluginNumber,const char *HelpTopic,char *Topic);
};

#endif  // __HELP_HPP__
