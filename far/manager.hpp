#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

+õ¨õúû¾·õıøõ üõöô³ ıõ¸úşû¼úøüø file panels, viewers, editors

*/

/* Revision: 1.05 06.05.2001 $ */

/*
Modify:
  06.05.2001 ++
    ! +õ¨õøüõışòğıøõ Window ò Frame :)
  04.05.2001 DJ
    + ôşôõûúğ ø ÿõ¨õôõûúğ NWZ
  29.04.2001 ++
    + +ıõô¨õıøõ NWZ ş² +¨õ²¼¿úşòğ
  29.12.2000 IS
    + +õ²şô ExitAll
  28.06.2000 tran
    - NT Console resize bug
      add class member ActiveModal
  25.06.2000 SVS
    ! +şôóş²şòúğ Master Copy
    ! +»ôõûõıøõ ò úğ·õ¸²òõ ¸ğüş¸²ş¿²õû¼ışóş üşô³û¿
*/

class Manager
{
  private:

    void ActivateNextFrame();

    Frame **FrameList;
    Frame *DestroyedFrame;

    int  EndLoop;

    int  FrameCount,
         FrameListSize;
    int  FramePos;
    int  UpdateRequired;

    int  NextViewer;
    char NextName[NM];
    int  NextPos;

    INPUT_RECORD LastInputRecord;

    void SetCurrentFrame (Frame *NewCurFrame);

  public:

    Manager();
    ~Manager();

    void AddFrame(Frame *NewFrame);
    void DestroyFrame(Frame *Killed);
    int ExecuteModal(Frame &ModalFrame);

    void NextFrame(int Increment);
    void SelectFrame(); // show window menu (F12)

    void CloseAll();
    /* $ 29.12.2000 IS
         +ığûşó CloseAll, ış ¨ğ÷¨õ¸ğõ² ÿ¨şôşûöõıøõ ÿşûış¶õıışù ¨ğñş²» ò ´ğ¨õ,
         õ¸ûø ÿşû¼÷şòğ²õû¼ ÿ¨şôşûöøû ¨õôğú²ø¨şòğ²¼ ´ğùû.
         +ş÷ò¨ğ¹ğõ² TRUE, õ¸ûø ò¸õ ÷ğú¨»ûø ø üşöış ò»µşôø²¼ ø÷ ´ğ¨ğ.
    */
    BOOL ExitAll();
    /* IS $ */
    BOOL IsAnyFrameModified(int Activate);

    int  GetFrameCount() {return(FrameCount);};
    void GetFrameTypesCount(int &Viewers,int &Editors);
    int  GetFrameCountByType(int Type);

    BOOL IsPanelsActive(); // ø¸ÿşû¼÷³õ²¸¿ úğú ÿ¨ø÷ığú WaitInMainLoop

    void SetFramePos(int NewPos);

    int  FindFrameByFile(int ModalType,char *FileName);

    void ShowBackground();

    void SetNextFrame(int Viewer,char *Name,long Pos);

    // new methods
    void EnterMainLoop();
    void ProcessMainLoop();
    void ExitMainLoop(int Ask);
    int ProcessKey(int key);
    int ProcessMouse(MOUSE_EVENT_RECORD *me);

    void PluginsMenu(); // ò»÷»òğõü üõı¾ ÿş F11
    void CheckExited();

    Frame *CurrentFrame;  // ²õú³¹øù üşôğû,
                          // ÿ¨ø¸³²¸ò³õ² ò ¸ÿø¸úõ, ış üşöõ² ñ»²¼ ıõ ğú²øòı»ü
    int    EnableSwitch;  // ¨ğ÷¨õ¸õış ûø ÿõ¨õúû¾·õıøõ ø÷ üşôğûğ

    INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
};

#endif  // __MANAGER_HPP__
