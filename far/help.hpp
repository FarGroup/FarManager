#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.04 18.12.2000 $ */

/*
Modify:
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

class Help:public Modal
{
  private:
    char *HelpData;
    char HelpTopic[512];
    char SelTopic[512];
    char HelpPath[NM];

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
    void ReadHelp(char *Mask=NULL);
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
    Help(char *Topic,int &ShowPrev,int PrevFullScreen,DWORD Flags=0);
    ~Help();

  public:
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
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
