#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.18 05.08.2001 $ */

/*
Modify:
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

struct StackHelpData
{
  DWORD Flags;                  // флаги
  int   TopStr;                 // номер верхней видимой строки темы
  int   CurX,CurY;              // координаты (???)

  char  HelpPath[NM];           // путь к хелпам
  char  HelpTopic[512];         // текущий топик
  char  SelTopic[512];          // выделенный топик (???)
};

enum HELPDOCUMENTSHELPTYPE{
  HIDX_PLUGINS,                 // Индекс плагинов
  HIDX_DOCUMS,                  // Индекс документов
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
    char *HelpMask;             // значение маски, переданной в конструктор

    int   DisableOut;           // TRUE - не выводить на экран
    BYTE  CtrlColorChar;        // CtrlColorChar - опция! для спецсимвола-
                                //   символа - для атрибутов
    int   CurColor;             // CurColor - текущий цвет отрисовки

    int   PrevMacroMode;        // предыдущий режим макроса

  private:
    void DisplayObject();
    int  ReadHelp(char *Mask=NULL);
    void AddLine(char *Line);
    void AddTitle(char *Title);
    void HighlightsCorrection(char *Str);
    void FastShow();
    void OutString(char *Str);
    int  StringLen(char *Str);
    void CorrectPosition();
    int  IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadDocumentsHelp(int TypeIndex);
    int  JumpTopic(const char *JumpTopic=NULL);

  public:
    Help(char *Topic,char *Mask=NULL,DWORD Flags=0);
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

    static char *MkTopic(int PluginNumber,const char *HelpTopic,char *Topic);
};

#endif	// __HELP_HPP__
