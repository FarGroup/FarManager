#ifndef __INFOLIST_HPP__
#define __INFOLIST_HPP__
/*
infolist.hpp

Информационная панель

*/

/* Revision: 1.02 05.04.2001 $ */

/*
Modify:
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_INFOPANEL
  03.04.2001 VVM
    + Используется Viewer для просмотра описаний.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class InfoList:public Panel
{
  private:
    Viewer *DizView;
    int  PrevMacroMode;
    int  OldWrapMode;
    int  OldWrapType;
    char DizFileName[NM];

  private:
    void DisplayObject();
    void ShowDirDescription();
    void ShowPluginDescription();
    void PrintText(char *Str);
    void PrintText(int MsgID);
    void PrintInfo(char *Str);
    void PrintInfo(int MsgID);
    void CloseDizFile();
    int  OpenDizFile(char *DizFile);
    void SetMacroMode(int Restore = FALSE);

  public:
    InfoList();
    ~InfoList();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode) {Redraw();};
    virtual void SetFocus();
    virtual void KillFocus();
};

#endif	// __INFOLIST_HPP__
