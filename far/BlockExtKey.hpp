#ifndef __BLOCKEXTKEY_HPP__
#define __BLOCKEXTKEY_HPP__
/*
BlockExtKey.hpp

������� � ����������� - ���������� �������� ��������

���� ����� � �������� ������������ ������ � ����������� � ���
�������� �������� ��� ������ ����. ���� - �������������
��������� ���������� ������� (���� Alt-F9 � ��� � ���), �.�.
� ��� ����� �������� ��������� ������ � �����������, ��������������
� ������� ����������...

*/

/* Revision: 1.00 21.02.2002 $ */

/*
Modify:
  21.02.2002 SVS
    ! Created
*/
#include "global.hpp"

class BlockExtKey{
   int OldIsProcessAssignMacroKey, OldIsProcessVE_FindFile;
 public:
   BlockExtKey()
   {
     OldIsProcessAssignMacroKey=IsProcessAssignMacroKey;
     IsProcessAssignMacroKey=1;
     OldIsProcessVE_FindFile=IsProcessVE_FindFile;
     IsProcessVE_FindFile=0;
   }
  ~BlockExtKey(){IsProcessAssignMacroKey=OldIsProcessAssignMacroKey; IsProcessVE_FindFile=OldIsProcessVE_FindFile;}
};

#endif // __BLOCKEXTKEY_HPP__
