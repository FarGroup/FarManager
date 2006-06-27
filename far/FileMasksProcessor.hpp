#ifndef __FileMasksProcessor_HPP
#define __FileMasksProcessor_HPP

/*
FileMasksProcessor.hpp

����� ��� ������ � �������� ������� ������ (�� ����������� ������� �����
����������).
*/

/* Revision: 1.03 16.03.2006 $ */

#include "BaseFileMask.hpp"
#include  "udlist.hpp"

enum FMP_FLAGS
{
  FMPF_ADDASTERISK = 0x00000001 // ��������� '*', ���� ����� �� ��������
                                // �� ������ �� ���������
                                // ��������: '*', '?', '.'
};

class FileMasksProcessorW:public BaseFileMaskW
{
public:
    FileMasksProcessorW();
    ~FileMasksProcessorW() {}

public:
    BOOL Set(const wchar_t *Masks, DWORD Flags);
    BOOL Compare(const wchar_t *Name);
    BOOL IsEmpty(void);
    void Free();

private:
    UserDefinedListW Masks; // ������ ����� ������
    const wchar_t *MaskPtr;   // ��������� �� ������� ����� � ������

private:
  FileMasksProcessorW& operator=(const FileMasksProcessorW& rhs); /* ����� �� */
  FileMasksProcessorW(const FileMasksProcessorW& rhs); /* �������������� �� ��������� */

};

#endif // __FileMasksProcessor_HPP
