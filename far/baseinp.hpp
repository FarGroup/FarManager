#ifndef __BASEINPUT_HPP__
#define __BASEINPUT_HPP__
/*
baseinp.hpp

������� ����� (����������� �����)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

class BaseInput
{
  public:
    virtual int ProcessKey(int Key) { return(0); };
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) { return(0); };
};


#endif	// __BASEINPUT_HPP__
