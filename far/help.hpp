#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.14 20.07.2001 $ */

/*
Modify:
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

class Help:public Frame
{
  private:
    DWORD Flags;                // флаги
    BOOL  ErrorHelp;            // TRUE - ошибка! Например - нет такого топика
    SaveScreen *TopScreen;      // область сохранения под хелпом
    KeyBar      HelpKeyBar;     // кейбар
    CallBackStack *Stack;       // стек возврата

    char *HelpData;             // "хелп" в памяти.
    int   StrCount;             // количество строк в теме
    int   FixCount;             // количество строк непрокручиваемой области
    int   TopicFound;           // TRUE - топик найден
    int   TopStr;               // номер верхней видимой строки темы
    int   CurX,CurY;            // координаты (???)
    int   FixSize;              // Размер непрокручиваемой области

    char  HelpTopic[512];       // текущий топик
    char  SelTopic[512];        // выделенный топик (???)
    char  HelpPath[NM];         // путь к хелпам
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
    void HighlightsCorrection(char *Str);
    void FastShow();
    void OutString(char *Str);
    int  StringLen(char *Str);
    void CorrectPosition();
    int  IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadPluginsHelp();
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

    static int GetFullScreenMode();
    static void SetFullScreenMode(int Mode);
    static int PluginPanelHelp(HANDLE hPlugin);
};

#endif	// __HELP_HPP__
