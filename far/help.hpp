#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.33 15.03.2006 $ */

#include "frame.hpp"
#include "keybar.hpp"
#include "farconst.hpp"
#include "unicodestring.hpp"

class CallBackStack;

#define HELPMODE_CLICKOUTSIDE  0x20000000 // было нажатие мыши вне хелпа?

struct StackHelpData
{
  DWORD Flags;                  // флаги
  int   TopStr;                 // номер верхней видимой строки темы
  int   CurX,CurY;              // координаты (???)

  string strHelpMask;           // значение маски
  string strHelpPath;           // путь к хелпам
  string strHelpTopic;         // текущий топик
  string strSelTopic;          // выделенный топик (???)
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
    string  strFullHelpPathName;

    struct StackHelpData StackData;
    wchar_t *HelpData;             // "хелп" в памяти.
    int   StrCount;             // количество строк в теме
    int   FixCount;             // количество строк непрокручиваемой области
    int   FixSize;              // Размер непрокручиваемой области
    int   TopicFound;           // TRUE - топик найден
    int   IsNewTopic;           // это новый топик?
    int   MouseDown;

    string strCtrlColorChar;    // CtrlColorChar - опция! для спецсимвола-
                                //   символа - для атрибутов
    int   CurColor;             // CurColor - текущий цвет отрисовки
    int   CtrlTabSize;          // CtrlTabSize - опция! размер табуляции

    int   PrevMacroMode;        // предыдущий режим макроса

    /* $ 29.11.2001 DJ
       помним PluginContents (для отображения в заголовке)
    */
    string strCurPluginContents;
    /* DJ $ */

    DWORD LastStartPos;
    DWORD StartPos;

    string strCtrlStartPosChar;

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
    int  ReadHelp(const wchar_t *Mask=NULL);
    void AddLine(const wchar_t *Line);
    void AddTitle(const wchar_t *Title);
    void HighlightsCorrection(wchar_t *Str);
    void FastShow();
    /* $ 29.11.2001 DJ
       вытащена из FastShow
    */
    void DrawWindowFrame();
    /* DJ $ */
    void OutString(const wchar_t *Str);
    int  StringLen(const wchar_t *Str);
    void CorrectPosition();
    int  IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadDocumentsHelp(int TypeIndex);
    int  JumpTopic(const wchar_t *JumpTopic=NULL);

  public:
    Help(const wchar_t *Topic,const wchar_t *Mask=NULL,DWORD Flags=0);
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

    virtual const wchar_t *GetTypeName() {return L"[Help]";}
    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_HELP; }

    static string &MkTopic(int PluginNumber,const wchar_t *HelpTopic,string &strTopic);
};

#endif  // __HELP_HPP__
