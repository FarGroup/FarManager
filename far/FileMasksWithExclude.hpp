#ifndef __FileMasksWithExclude_HPP
#define __FileMasksWithExclude_HPP

/*
FileMasksWithExclude.hpp

����� ��� ������ �� �������� ������� ������ (����������� ������� �����
����������).
*/

/* Revision: 1.03 16.03.2006 $ */

#include "FileMasksProcessor.hpp"

class FileMasksWithExcludeW:public BaseFileMaskW
{
private:
    void Free();

public:
    FileMasksWithExcludeW();
    ~FileMasksWithExcludeW() {}

public:
    BOOL Set(const wchar_t *Masks, DWORD Flags);
    BOOL Compare(const wchar_t *Name);
    BOOL IsEmpty(void);

private:
    FileMasksProcessorW Include, Exclude;

private:
  FileMasksWithExcludeW& operator=(const FileMasksWithExcludeW& rhs); /* ����� �� */
  FileMasksWithExcludeW(const FileMasksWithExcludeW& rhs); /* �������������� �� ��������� */
};

#endif // __FileMasksWithExclude_HPP
