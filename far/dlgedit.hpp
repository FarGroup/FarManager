#ifndef __DLGEDIT_HPP__
#define __DLGEDIT_HPP__
/*
dlgedit.hpp

�����筠� ��ப� ।���஢���� ��� �������
(��� ��᫥���� ����� Edit)

*/

/* Revision: 1.00 15.05.2002 $ */

/*
Modify:
  15.05.2002 SVS
    ! ���� ᮧ���. � �㦭� ������� ��  edit.hpp �� ���,
      ��ᠥ�� ��᮪ �.. �� �� �⭮���� ⮫쪮 � ��������
      �� ���� ⮫쪮 蠡���, ����⮢�� ��� ���饣� ���室�
*/

#include "edit.hpp"

class DlgEdit:public Edit
{
  friend class Dialog;

  private: // �ਢ��� �����

  public:  // �㡫��� �����

  private: // �ਢ��� ��⮤�

  public:
    DlgEdit();
    ~DlgEdit();

  public: // �㡫��� ��⮤�
};

#endif  // __DLGEDIT_HPP__
