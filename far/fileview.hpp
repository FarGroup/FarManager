#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.01 29.06.2000 $ */

/*
Modify:
  28.06.2000 tran
    - NT Console resize bug
      adding SetScreenPosition method
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class FileViewer:public Modal
{
  private:
    void Process();
    void Show();
    void DisplayObject();
    Viewer View;
    KeyBar ViewKeyBar;
    char NewTitle[NM];
    int F3KeyOnly;
    int FullScreen;
    int ExitCode;
    int DisableEdit;
  public:
    FileViewer(char *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
               int DisableEdit=FALSE,long ViewStartPos=-1,char *PluginData=NULL,
               NamesList *ViewNamesList=NULL);
    FileViewer(char *Name,int EnableSwitch,char *Title,
               int X1,int Y1,int X2,int Y2);
    void Init(char *Name,int EnableSwitch,int DisableHistory,
              long ViewStartPos,char *PluginData,NamesList *ViewNamesList);
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
};

#endif	// __FILEVIEWER_HPP__
