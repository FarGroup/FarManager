#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.04 05.05.2001 $ */

/*
Modify:
  05.05.2001 DJ
    + Перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  07.08.2000 SVS
    + Функция инициализации KeyBar Labels - InitKeyBar()
  28.06.2000 tran
    - NT Console resize bug
      adding SetScreenPosition method
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в каµестве самостоятельного модуля
*/

class FileViewer:public Window
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
       Функция инициализации KeyBar Labels
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
