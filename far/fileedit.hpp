#ifndef __FILEEDITOR_HPP__
#define __FILEEDITOR_HPP__
/*
fileedit.hpp

+õôðú²ø¨þòðýøõ ´ðùûð - ýðô¸²¨þùúð ýðô editor.cpp

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

class FileEditor:public Frame
{
  private:
    void Show();
    void DisplayObject();
    int ProcessQuitKey();

    Editor FEdit;
    KeyBar EditKeyBar;
    char FileName[NM];
    char FullFileName[NM];
    char StartDir[NM];
    char NewTitle[NM];
    int FullScreen;
    int ExitCode;
  public:
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine=-1,int StartChar=-1,int DisableHistory=FALSE,
               char *PluginData=NULL);
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine,int StartChar,char *Title,
               int X1,int Y1,int X2,int Y2);
    void Init(char *Name,int CreateNewFile,int EnableSwitch,
              int StartLine,int StartChar,int DisableHistory,char *PluginData);
    /* $ 07.08.2000 SVS
       +³ýú¶ø¿ øýø¶øðûø÷ð¶øø KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int GetTypeAndName(char *Type,char *Name);
    void ShowConsoleTitle();
    int IsFileChanged() {return(FEdit.IsFileChanged());};
    int IsFileModified() {return(FEdit.IsFileModified());};
    int GetExitCode();
    /* $ 28.06.2000 tran
       NT Console resize - resize editor */
    virtual void SetScreenPosition();
    /* tran $ */
    virtual char *GetTypeName(){return "[FileEdit]";}; ///
    virtual int GetType() { return MODALTYPE_EDITOR; }

    virtual void OnChangeFocus(int i); ///
};

#endif	// __FILEEDITOR_HPP__
