#ifndef __GRABBER_HPP__
#define __GRABBER_HPP__
/*
grabber.hpp

Screen grabber

*/

/* Revision: 1.03 05.09.2003 $ */

/*
Modify:
  05.09.2003 SVS
    + Grabber::Reset() - ����� ���������
  24.05.2002 SVS
    ! CopyGrabbedArea ����� ���.��������
  06.05.2001 DJ
    ! �������� #include
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "modal.hpp"

struct GrabberArea
{
  int X1,Y1,X2,Y2;
  int CurX,CurY;
};

class Grabber:Modal
{
  private:
    SaveScreen *SaveScr;
    struct GrabberArea PrevArea;
    struct GrabberArea GArea;
    int ResetArea;
    int PrevMacroMode;
    int VerticalBlock;

  private:
    void DisplayObject();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void CopyGrabbedArea(int Append, int VerticalBlock);
    void Reset();

  public:
    Grabber();
    ~Grabber();
};


#endif	// __GRABBER_HPP__
