#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

+¨þ¸üþ²¨ ´ðùûð - ýðô¸²¨þùúð ýðô viewer.cpp

*/

/* Revision: 1.05 06.05.2001 $ */

/*
Modify:
  06.05.2001 ++
    ! +õ¨õøüõýþòðýøõ Window ò Frame :)
  05.05.2001 DJ
    + +õ¨õ²¨¿µ NWZ
  29.04.2001 ++
    + +ýõô¨õýøõ NWZ þ² +¨õ²¼¿úþòð
  07.08.2000 SVS
    + +³ýú¶ø¿ øýø¶øðûø÷ð¶øø KeyBar Labels - InitKeyBar()
  28.06.2000 tran
    - NT Console resize bug
      adding SetScreenPosition method
  25.06.2000 SVS
    ! +þôóþ²þòúð Master Copy
    ! +»ôõûõýøõ ò úð…õ¸²òõ ¸ðüþ¸²þ¿²õû¼ýþóþ üþô³û¿
*/

class FileViewer:public Frame
{
  private:
    void Show();
    void DisplayObject();
    Viewer View;
    KeyBar ViewKeyBar;
    char NewTitle[NM];
    int F3KeyOnly;
    int FullScreen;
    int ExitCode;
    int DisableEdit;
    int DisableHistory;
    char Name[NM];
  public:
    FileViewer(char *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
               int DisableEdit=FALSE,long ViewStartPos=-1,char *PluginData=NULL,
               NamesList *ViewNamesList=NULL);
    FileViewer(char *Name,int EnableSwitch,char *Title,
               int X1,int Y1,int X2,int Y2);
    void Init(char *Name,int EnableSwitch,int DisableHistory,
              long ViewStartPos,char *PluginData,NamesList *ViewNamesList);
    /* $ 07.08.2000 SVS
       +³ýú¶ø¿ øýø¶øðûø÷ð¶øø KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int GetTypeAndName(char *Type,char *Name);
    void ShowConsoleTitle();
    void SetTempViewName(char *Name);
    int GetExitCode();
    /* $ 28.06.2000 tran
       NT Console resize - resize viewer */
    virtual void SetScreenPosition();
    /* tran $ */
    virtual void OnCreate(); ///
    virtual void OnDestroy(); ///
    virtual void OnChangeFocus(int f); ///
    virtual char *GetTypeName(){return "[FileView]";}; ///
    virtual int GetType() { return MODALTYPE_VIEWER; }
};

#endif	// __FILEVIEWER_HPP__
