#ifndef __INFOLIST_HPP__
#define __INFOLIST_HPP__
/*
infolist.hpp

Информационная панель

*/

/* Revision: 1.01 03.04.2001 $ */

/*
Modify:
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

  public:
    InfoList();
    ~InfoList();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode) {Redraw();};
};

#endif	// __INFOLIST_HPP__
