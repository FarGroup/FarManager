#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

Помощь

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class Help:public Modal
{
  private:
    void DisplayObject();
    void ReadHelp();
    void AddLine(char *Line);
    void HighlightsCorrection(char *Str);
    void FastShow();
    void OutString(char *Str);
    int StringLen(char *Str);
    void CorrectPosition();
    int IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadPluginsHelp();
    char *HelpData;
    char HelpTopic[512];
    char SelTopic[512];
    char HelpPath[NM];
    int TopLevel;
    int PrevFullScreen;
    int StrCount,FixCount,FixSize;
    int TopStr;
    int CurX,CurY;
    int ShowPrev;
    int DisableOut;
    int TopicFound;
    int PrevMacroMode;
  public:
    Help(char *Topic);
    Help(char *Topic,int &ShowPrev,int PrevFullScreen);
    ~Help();
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    static int GetFullScreenMode();
    static void SetFullScreenMode(int Mode);
    static int PluginPanelHelp(HANDLE hPlugin);
};

#endif	// __HELP_HPP__
