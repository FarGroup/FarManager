#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.09 06.05.2001 $ */

/*
Modify:
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

class Help:public Frame
{
  private:
    KeyBar HelpKeyBar;

    char *HelpData;
    char HelpTopic[512];
    char SelTopic[512];
    char HelpPath[NM];
    /* $ 12.04.2001 SVS
       Значение маски, переданной в конструктор
    */
    char *HelpMask;
    /* SVS $ */

    DWORD Flags;
    /* $ 01.09.2000 SVS
      CurColor - текущий цвет отрисовки
    */
    int CurColor;
    /* SVS $ */

    int TopLevel;
    int PrevFullScreen;
    int StrCount,FixCount,FixSize;
    int TopStr;
    int CurX,CurY;
    int ShowPrev;
    int DisableOut;
    int TopicFound;
    int PrevMacroMode;

    /* $ 01.09.2000 SVS
      CtrlColorChar - опция! для спецсимвола-символа - для атрибутов
    */
    BYTE CtrlColorChar;
    /* SVS $ */
    BOOL ErrorHelp;

  private:
    void DisplayObject();
    int  ReadHelp(char *Mask=NULL);
    void AddLine(char *Line);
    void HighlightsCorrection(char *Str);
    void FastShow();
    void OutString(char *Str);
    int StringLen(char *Str);
    void CorrectPosition();
    int IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadPluginsHelp();

  public:
    Help(char *Topic,char *Mask=NULL,DWORD Flags=0);
    /* $ 12.04.2001 SVS
       передача Mask
    */
    Help(char *Topic,int &ShowPrev,int PrevFullScreen,DWORD Flags=0,char *Mask=NULL);
    /* SVS $ */
    ~Help();

  public:
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void InitKeyBar(void);
    BOOL GetError() {return ErrorHelp;}
    static int GetFullScreenMode();
    static void SetFullScreenMode(int Mode);
    static int PluginPanelHelp(HANDLE hPlugin);

    /* $ 28.06.2000 tran
       NT Console resize - resize help */
    virtual void SetScreenPosition();
    /* tran $ */
};

#endif	// __HELP_HPP__
