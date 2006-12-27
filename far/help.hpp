#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

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
    char  FullHelpPathName[512];

    struct StackHelpData StackData;
    char *HelpData;             // "хелп" в памяти.
    int   StrCount;             // количество строк в теме
    int   FixCount;             // количество строк непрокручиваемой области
    int   FixSize;              // Размер непрокручиваемой области
    int   TopicFound;           // TRUE - топик найден
    int   IsNewTopic;           // это новый топик?
    int   MouseDown;

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
