#ifndef __REFRESHFRAMEMANAGER_HPP__
#define __REFRESHFRAMEMANAGER_HPP__
/*
RefreshFrameManager.hpp

Класс для решрешки

*/

/* Revision: 1.0 20.05.2004 $ */

/*
Modify:
  20.05.2004 SVS
    ! Типа... сделано
*/

class SaveScreen;

class UndoGlobalSaveScrPtr{
  public:
    UndoGlobalSaveScrPtr(SaveScreen *SaveScr);
   ~UndoGlobalSaveScrPtr();
};

class RefreshFrameManager{
  private:
    int OScrX,OScrY;
    long MsgWaitTime;
    BOOL DontRedrawFrame;

  public:
    RefreshFrameManager(int OScrX,int OScrY, int MsgWaitTime, BOOL DontRedrawFrame=FALSE);
    ~RefreshFrameManager();
};

#endif  // __REFRESHFRAMEMANAGER_HPP__
